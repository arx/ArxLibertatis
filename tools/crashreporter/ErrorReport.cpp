/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "crashreporter/ErrorReport.h"

#include "platform/Platform.h"

#include <algorithm>

#include <QApplication>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QDir>
#include <QProcess>
#include <QTime>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QThread>
#include <QByteArray>

#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QWindow>
#endif

#include <boost/date_time/posix_time/posix_time_types.hpp>

#include "Configure.h"

#include "core/Version.h"

#include "crashreporter/tbg/TBG.h"

#include "platform/Architecture.h"
#include "platform/Process.h"

#include "util/String.h"

ErrorReport::ErrorReport(const QString & sharedMemoryName)
	: m_SharedMemoryName(sharedMemoryName)
	, m_pCrashInfo()
	, m_Username("CrashBot")
	, m_Password("WbAtVjS9")
{
	#if ARX_HAVE_PRCTL && defined(DEBUG)
	// Allow debuggers to be attached to this process, for development purpose...
	prctl(PR_SET_PTRACER, 1, 0, 0, 0);
	#endif
}

bool ErrorReport::Initialize() {
	
	try {
		
		// Create a shared memory object.
		m_SharedMemory = boost::interprocess::shared_memory_object(boost::interprocess::open_only, m_SharedMemoryName.toStdString().c_str(), boost::interprocess::read_write);
		
		// Map the whole shared memory in this process
		m_MemoryMappedRegion = boost::interprocess::mapped_region(m_SharedMemory, boost::interprocess::read_write);
		
		// Our SharedCrashInfo will be stored in this shared memory.
		m_pCrashInfo = static_cast<CrashInfo *>(m_MemoryMappedRegion.get_address());
		
		#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
		if(m_pCrashInfo->window) {
			QWindow * window = QWindow::fromWinId(m_pCrashInfo->window);
			if(window) {
				window->showMinimized();
				window->hide();
				window->setMouseGrabEnabled(true);
				window->setMouseGrabEnabled(false);
			}
		}
		#endif
		
	} catch(...) {
		m_pCrashInfo = NULL;
		m_DetailedError = "We encountered an unexpected error while collecting crash information!";
		return false;
	}
	
	if(!m_pCrashInfo || m_MemoryMappedRegion.get_size() != sizeof(CrashInfo)) {
		m_DetailedError = "The size of the memory mapped region does not match the size of the CrashInfo structure.";
		return false;
	}
	
	m_pCrashInfo->reporterStarted.post();
	
	return true;
}

void ErrorReport::getCrashInfo() {
	
	m_ReportDescription = util::loadString(m_pCrashInfo->description).c_str();
	if(m_ReportDescription.isEmpty()) {
		m_ReportDescription = "Could not collect crash information!";
	}
	
	if(m_pCrashInfo->crashId != 0) {
		m_ReportUniqueID = QString("[%1]").arg(QString::number(m_pCrashInfo->crashId, 16).toUpper());
	}
	
	std::string title = util::loadString(m_pCrashInfo->title);
	m_ReportTitle = QString("%1 %2").arg(m_ReportUniqueID, title.c_str());

	m_ProcessArchitecture = m_pCrashInfo->architecture;
	
	// Add attached files from the report
	size_t nbFilesAttached = std::min(size_t(m_pCrashInfo->nbFilesAttached),
	                                  size_t(CrashInfo::MaxNbFiles));
	for(size_t i = 0; i < nbFilesAttached; i++) {
		AddFile(util::loadString(m_pCrashInfo->attachedFiles[i]).c_str());
	}
	
}

bool ErrorReport::GenerateReport(ErrorReport::IProgressNotifier * pProgressNotifier) {
	
	ErrorReport * report = this;
	BOOST_SCOPE_EXIT((report)) {
		// Allow the crashed process to exit
		try {
			report->ReleaseApplicationLock();
		} catch(...) {
			qWarning("Could not terminate crashed process");
		}
	} BOOST_SCOPE_EXIT_END
	
	pProgressNotifier->taskStarted("Generating crash report", 3);
	
	// Initialize shared memory
	pProgressNotifier->taskStepStarted("Connecting to crashed application");
	bool bInit = Initialize();
	pProgressNotifier->taskStepEnded();
	if(!bInit) {
		pProgressNotifier->setError("Could not prepare the crash report.");
		pProgressNotifier->setDetailedError(m_DetailedError);
		return false;
	}
	
	// Wait for crash to be processed
	pProgressNotifier->taskStepStarted("Processing crash information");
	while(platform::isProcessRunning(m_pCrashInfo->processorProcessId)) {
		boost::posix_time::ptime timeout
		 = boost::posix_time::microsec_clock::universal_time()
		 + boost::posix_time::milliseconds(100);
		if(m_pCrashInfo->processorDone.timed_wait(timeout)) {
			break;
		}
	}
	pProgressNotifier->taskStepEnded();
	
	// Generate minidump
	pProgressNotifier->taskStepStarted("Retrieving crash information");
	getCrashInfo();
	pProgressNotifier->taskStepEnded();
	
	return true;
}

bool ErrorReport::SendReport(ErrorReport::IProgressNotifier * pProgressNotifier) {
	
	int nbFilesToSend = 0;
	for(FileList::const_iterator it = m_AttachedFiles.begin(); it != m_AttachedFiles.end(); ++it) {
		if(it->attachToReport) {
			nbFilesToSend++;
		}
	}
	
	pProgressNotifier->taskStarted("Sending crash report", 3 + nbFilesToSend);
	
	std::string userAgent = arx_name + " Crash Reporter " + arx_version;
	TBG::Server server("https://bugs.arx-libertatis.org", userAgent);
	
	// Login to TBG server
	pProgressNotifier->taskStepStarted("Connecting to the bug tracker");
	bool bLoggedIn = server.login(m_Username, m_Password);
	pProgressNotifier->taskStepEnded();
	if(!bLoggedIn) {
		pProgressNotifier->setError("Could not connect to the bug tracker");
		pProgressNotifier->setDetailedError(server.getErrorString());
		return false;
	}
	
	// Look for existing issue
	int issue_id = -1;
	pProgressNotifier->taskStepStarted("Searching for existing issue");
	if(!m_ReportUniqueID.isEmpty()) {
		m_IssueLink = server.findIssue(m_ReportUniqueID, issue_id);
	}
	pProgressNotifier->taskStepEnded();
	
	// Create new issue if no match was found
	if(issue_id == -1) {
		
		pProgressNotifier->taskStepStarted("Creating new issue");
		m_IssueLink = server.createCrashReport(m_ReportTitle, m_ReportDescription,
		                                       m_ReproSteps, tbg_version_id, issue_id);
		if(m_IssueLink.isNull()) {
			pProgressNotifier->taskStepEnded();
			pProgressNotifier->setError("Could not create a new issue on the bug tracker");
			pProgressNotifier->setDetailedError(server.getErrorString());
			return false;
		}
		
		// Set OS
		#if ARX_PLATFORM == ARX_PLATFORM_WIN32
		int os_id = TBG::Server::OS_Windows;
		#elif ARX_PLATFORM == ARX_PLATFORM_LINUX
		int os_id = TBG::Server::OS_Linux;
		#elif ARX_PLATFORM == ARX_PLATFORM_MACOS
		int os_id = TBG::Server::OS_macOS;
		#elif ARX_PLATFORM == ARX_PLATFORM_BSD
		#if defined(__FreeBSD__)
		int os_id = TBG::Server::OS_FreeBSD;
		#else
		int os_id = TBG::Server::OS_BSD;
		#endif
		#else
		int os_id = TBG::Server::OS_Other;
		#endif
		server.setOperatingSystem(issue_id, os_id);
		
		// Set Architecture
		int arch_id = TBG::Server::Arch_Other;
		if(m_ProcessArchitecture == ARX_ARCH_X86_64) {
			arch_id = TBG::Server::Arch_Amd64;
		} else if(m_ProcessArchitecture == ARX_ARCH_X86) {
			arch_id = TBG::Server::Arch_x86;
		}
		server.setArchitecture(issue_id, arch_id);
		
		pProgressNotifier->taskStepEnded();
		
	} else if(!m_ReproSteps.isEmpty()) {
		pProgressNotifier->taskStepStarted("Duplicate issue found, adding information");
		bool bCommentAdded = server.addComment(issue_id, m_ReproSteps);
		pProgressNotifier->taskStepEnded();
		if(!bCommentAdded) {
			pProgressNotifier->setError("Failure occured when trying to add information to an existing issue");
			pProgressNotifier->setDetailedError(server.getErrorString());
			return false;
		}
	}
	
	// Send files
	QString commonPath;
	for(FileList::const_iterator it = m_AttachedFiles.begin(); it != m_AttachedFiles.end(); ++it) {
		
		// Ignore files that were removed by the user.
		if(!it->attachToReport) {
			continue;
		}
		
		QFileInfo path(it->path);
		
		// One more check to verify that the file still exists.
		if(!path.exists()) {
			continue;
		}
		
		pProgressNotifier->taskStepStarted(QString("Sending file \"%1\"").arg(path.fileName()));
		if(server.attachFile(issue_id, it->path, path.fileName(), m_SharedMemoryName)) {
			commonPath.clear();
		} else {
			m_failedFiles.append(it->path);
			QString dir = path.dir().path();
			if(it == m_AttachedFiles.begin()) {
				commonPath = dir;
			} else if(dir != commonPath) {
				commonPath.clear();
			}
		}
		pProgressNotifier->taskStepEnded();
	}
	if(!commonPath.isEmpty() && m_failedFiles.count() > 1) {
		m_failedFiles.clear();
		m_failedFiles.append(commonPath);
	}
	
	return true;
}

void ErrorReport::ReleaseApplicationLock() {
	
	if(m_pCrashInfo) {
		// Kill the original, busy-waiting process.
		platform::killProcess(m_pCrashInfo->processorProcessId);
		m_pCrashInfo->exitLock.post();
		platform::killProcess(m_pCrashInfo->processId);
	}
	
}

void ErrorReport::AddFile(const QString & fileName) {
	
	QFileInfo file(fileName);
	
	// Do not include files that can't be found, and empty files...
	if(file.exists() && file.size() != 0) {
		m_AttachedFiles.push_back(File(fileName, true));
	}
	
}

ErrorReport::FileList & ErrorReport::GetAttachedFiles() {
	return m_AttachedFiles;
}

const QString & ErrorReport::GetErrorDescription() const {
	return m_ReportDescription;
}

const QString & ErrorReport::GetIssueLink() const {
	return m_IssueLink;
}

void ErrorReport::SetLoginInfo(const QString & username, const QString & password) {
	m_Username = username;
	m_Password = password;
}

void ErrorReport::SetReproSteps(const QString & reproSteps) {
	m_ReproSteps = reproSteps;
}
