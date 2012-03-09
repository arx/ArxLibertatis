/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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

#include "crashreporter/errorreport.h"

#ifdef HAVE_WINAPI
// Win32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/wait.h>
#endif

#ifdef HAVE_UNAME
#include <sys/utsname.h>
#endif

#ifdef HAVE_GETRUSAGE
#include <sys/resource.h>
#include <sys/time.h>
#endif

// Qt
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
#include <QXmlStreamWriter>

#include "Configure.h"

#include "io/fs/Filesystem.h"
#include "io/fs/FileStream.h"

#include "crashreporter/utilities_win32.h"
#include "crashreporter/tbg/tbg.h"

#include "platform/Architecture.h"
#include "platform/String.h"
#include "platform/Thread.h"

ErrorReport::ErrorReport(const std::string& sharedMemoryName)
	: m_RunningTimeSec()
	, m_SharedMemoryName(sharedMemoryName)
	, m_pCrashInfo()
	, m_Username("CrashBot")
	, m_Password("WbAtVjS9")
{
}

bool ErrorReport::Initialize()
{	
	// Create a shared memory object.
	m_SharedMemory = boost::interprocess::shared_memory_object(boost::interprocess::open_only, m_SharedMemoryName.c_str(), boost::interprocess::read_write);

	// Map the whole shared memory in this process
	m_MemoryMappedRegion = boost::interprocess::mapped_region(m_SharedMemory, boost::interprocess::read_write);

	// Our SharedCrashInfo will be stored in this shared memory.
	m_pCrashInfo = (CrashInfo*)m_MemoryMappedRegion.get_address();

	if(m_MemoryMappedRegion.get_size() != sizeof(CrashInfo))
		return false;

	bool bMiscCrashInfo = GetMiscCrashInfo();
	if(!bMiscCrashInfo)
		return false;

	// Add attached files from the report
	for(int i = 0; i < m_pCrashInfo->nbFilesAttached; i++)
		AddFile(m_pCrashInfo->attachedFiles[i]);

	m_ReportFolder = fs::path(m_pCrashInfo->crashReportFolder) / fs::path(m_CrashDateTime.toString("yyyy.MM.dd hh.mm.ss").toAscii());

	if(!fs::create_directories(m_ReportFolder))
		return false;

	return true;
}

bool ErrorReport::GetCrashDump(const fs::path& fileName) {
	
#ifdef HAVE_WINAPI
	bool bHaveDump = false;

	if(fs::exists(m_pCrashInfo->miniDumpTmpFile))
	{
		fs::path fullPath = m_ReportFolder / fileName;
		if(fs::rename(m_pCrashInfo->miniDumpTmpFile, fullPath))
		{
			AddFile(fullPath);
			bHaveDump = true;
		}
	}

	return bHaveDump;
	
#else // !HAVE_WINAPI
	
	ARX_UNUSED(fileName);
	// TODO: Write core dump to 
	// fs::path fullPath = m_ReportFolder / fileName;
	
#if defined(HAVE_FORK) && defined(HAVE_EXECLP) && defined(HAVE_DUP2)
	
	fs::path tracePath = m_ReportFolder / "gdbtrace.txt";
	
	// Fork so we retain control after launching GDB.
	int childPID = fork();
	if(childPID) {
		// Wait for GDB to exit.
		waitpid(childPID, NULL, 0);
	} else {
		
		// Redirect output to a file
		int fd = open(tracePath.string().c_str(), O_WRONLY|O_CREAT, 0666);
		dup2(fd, 1);
		close(fd);
		
		// Prepare pid argument for GDB.
		char pid_buf[30];
		memset(&pid_buf, 0, sizeof(pid_buf));
		sprintf(pid_buf, "%d", m_pCrashInfo->processId);
		
		// Try to execute gdb to get a very detailed stack trace.
		execlp("gdb", "gdb", "--batch", "-n", "-ex", "thread", "-ex", "set confirm off", "-ex", "set print frame-arguments all", "-ex", "set print static-members off", "-ex", "info threads", "-ex", "thread apply all bt full", m_pCrashInfo->execFullName, pid_buf, NULL);
		
		// GDB failed to start.
		exit(1);
	}
#endif // defined(HAVE_EXECLP) && defined(HAVE_DUP2)
	
	bool bWroteDump = fs::exists(tracePath) && fs::file_size(tracePath) > 0;
	if(bWroteDump) {
		AddFile(tracePath);
		return true;
	}
	
	tracePath = m_ReportFolder / "trace.txt";
	fs::ofstream ofs(tracePath, std::ios_base::trunc);
	ofs << safestring(m_pCrashInfo->backtrace);
	ofs.flush();
	ofs.close();
	
	AddFile(tracePath);
	
	return true;
	
#endif // !HAVE_WINAPI
}

bool ErrorReport::GetMiscCrashInfo()
{
	// Copy the detailed description to an std::string for easier manipulation
	m_ReportDescription = m_pCrashInfo->detailedCrashInfo;

	// Get crash time
	m_CrashDateTime = QDateTime::currentDateTime();
	
	m_ProcessArchitecture = ARX_ARCH_NAME;
	
#ifdef HAVE_WINAPI
	
	// Open parent process handle
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_pCrashInfo->processId);
	if(hProcess != NULL)
	{
		// Get memory usage info
		PROCESS_MEMORY_COUNTERS meminfo;
		BOOL bGetMemInfo = GetProcessMemoryInfo(hProcess, &meminfo, sizeof(PROCESS_MEMORY_COUNTERS));
		if(bGetMemInfo)
			m_ProcessMemoryUsage = meminfo.WorkingSetSize;

		// Determine the period of time the process is working.
		FILETIME CreationTime, ExitTime, KernelTime, UserTime;
		BOOL bGetTimes = GetProcessTimes(hProcess, &CreationTime, &ExitTime, &KernelTime, &UserTime);
		if(bGetTimes)
		{
			SYSTEMTIME AppStartTime;
			FileTimeToSystemTime(&CreationTime, &AppStartTime);

			SYSTEMTIME CurTime;
			GetSystemTime(&CurTime);
			ULONG64 uCurTime = ConvertSystemTimeToULONG64(CurTime);
			ULONG64 uStartTime = ConvertSystemTimeToULONG64(AppStartTime);

			// Check that the application works for at least one minute before crash.
			// This might help to avoid cyclic error report generation when the applciation
			// crashes on startup.
			m_RunningTimeSec = (double)(uCurTime-uStartTime)*10E-08;
		}
	}

	// Get operating system friendly name from registry.
	char OSNameBuf[256];
	if(!GetWindowsVersionName(OSNameBuf, 256))
		return false;
	m_OSName = OSNameBuf;

	// Determine if Windows is 64-bit.
	m_OSArchitecture = Is64BitWindows() ? ARX_ARCH_NAME_X86_64 : ARX_ARCH_NAME_X86;
	
	if(m_pCrashInfo->exceptionCode != 0)
	{
		std::string exceptionStr = GetExceptionString(m_pCrashInfo->exceptionCode);
		if(!exceptionStr.empty())
		{
			m_ReportDescription += "\nException code:\n  ";
			m_ReportDescription += exceptionStr.c_str();
			m_ReportDescription += "\n";
		}
	}

	std::string callStack, callstackTop;
	u32 callstackCrc;

	bool bCallstack = GetCallStackInfo(hProcess, m_pCrashInfo->threadHandle, &m_pCrashInfo->contextRecord, callStack, callstackTop, callstackCrc);
	if(!bCallstack)
		return false;
	
	m_ReportUniqueID = QString("[%1]").arg(QString::number(callstackCrc, 16).toUpper());
	m_ReportDescription += "\nCallstack:\n";
	m_ReportDescription += callStack.c_str();
	m_ReportTitle = QString("%1 %2").arg(m_ReportUniqueID, callstackTop.c_str());

	std::string registers = GetRegisters(&m_pCrashInfo->contextRecord);
	if(!registers.empty())
	{
		m_ReportDescription += "\nRegisters:\n";
		m_ReportDescription += registers.c_str();
	}

	CloseHandle(hProcess);
	
#else
	
	m_ProcessMemoryUsage = 0;
	m_RunningTimeSec = 0.0;
#ifdef HAVE_GETRUSAGE
	if(m_pCrashInfo->have_rusage) {
		m_ProcessMemoryUsage = m_pCrashInfo->rusage.ru_maxrss * 1024;
		m_RunningTimeSec += m_pCrashInfo->rusage.ru_utime.tv_sec;
		m_RunningTimeSec += m_pCrashInfo->rusage.ru_utime.tv_usec * (1.0 / 1000000);
		m_RunningTimeSec += m_pCrashInfo->rusage.ru_stime.tv_sec;
		m_RunningTimeSec += m_pCrashInfo->rusage.ru_stime.tv_usec * (1.0 / 1000000);
	}
#endif
	
#ifdef HAVE_UNAME
	struct utsname buf;
	if(uname(&buf) == 0) {
		m_OSName = QString(buf.sysname) + " " + buf.release;
		m_OSArchitecture = buf.machine;
	}
#endif
	
	// TODO
	
#endif

	return true;
}

bool ErrorReport::WriteReport(const fs::path & fileName) {
	
	fs::path fullPath = m_ReportFolder / fileName;
	
	QFile file(fullPath.string().c_str());
	if(!file.open(QIODevice::WriteOnly)) {
		return false;
	}
	
	AddFile(fullPath);
	
	QXmlStreamWriter xml;
	xml.setDevice(&file);
	xml.setAutoFormatting(true);
	xml.writeStartDocument();
	xml.writeStartElement("CrashReport");
		
		xml.writeComment("Information related to the crashed process");
		xml.writeStartElement("Process");
			xml.writeTextElement("Path", m_pCrashInfo->executablePath);
			xml.writeTextElement("Version", m_pCrashInfo->executableVersion);
			xml.writeTextElement("MemoryUsage", QString::number(m_ProcessMemoryUsage));
			xml.writeTextElement("Architecture", m_ProcessArchitecture);
			xml.writeTextElement("RunningTime", QString::number(m_RunningTimeSec));
			xml.writeTextElement("CrashDateTime", m_CrashDateTime.toString("dd.MM.yyyy hh:mm:ss"));
		xml.writeEndElement();

		xml.writeComment("Information related to the OS");
		xml.writeStartElement("OS");
			xml.writeTextElement("Name", m_OSName);
			xml.writeTextElement("Architecture", m_OSArchitecture);
		xml.writeEndElement();

		xml.writeComment("List of files generated by the crash reporter");
		xml.writeComment("Note that some of these files could have been manually excluded from the report");
		xml.writeStartElement("Files");
		for(FileList::const_iterator it = m_AttachedFiles.begin();
		    it != m_AttachedFiles.end(); ++it) {
			xml.writeTextElement("File", it->path.string().c_str());
		}
		xml.writeEndElement();

		xml.writeComment("Variables attached by the crashed process");
		xml.writeStartElement("Variables");
		for(int i = 0; i < m_pCrashInfo->nbVariables; ++i)
		{
			xml.writeStartElement("Variable");
			xml.writeAttribute("Name", m_pCrashInfo->variables[i].name);
			xml.writeAttribute("Value", m_pCrashInfo->variables[i].value);
			xml.writeEndElement();
		}
		xml.writeEndElement();
		
	xml.writeEndElement();
	xml.writeEndDocument();
	
	file.close();
	
	return true;
}

bool ErrorReport::GenerateReport(ErrorReport::IProgressNotifier* pProgressNotifier)
{
	ErrorReport* report = this;
	BOOST_SCOPE_EXIT((report))
	{
		// Allow the crashed process to exit
		report->ReleaseApplicationLock();
	} BOOST_SCOPE_EXIT_END

	pProgressNotifier->taskStarted("Generating crash report", 3);
	
	// Initialize shared memory
	pProgressNotifier->taskStepStarted("Connecting to crashed application");
	Initialize();
	pProgressNotifier->taskStepEnded();
	
	if(m_pCrashInfo->architecture != ARX_ARCH) {
		// TODO architecture mismatch - display an error
		exit(0);
	}
	
	// Generate minidump
	pProgressNotifier->taskStepStarted("Generating crash dump");
	bool bCrashDump = GetCrashDump("crash.dmp");
	pProgressNotifier->taskStepEnded();
	if(!bCrashDump)
	{
		pProgressNotifier->setError("Could not generate the crash dump.");
		return false;
	}

	// Generate manifest
	pProgressNotifier->taskStepStarted("Generating report manifest");
	bool bCrashXml = WriteReport("crash.xml");
	pProgressNotifier->taskStepEnded();
	if(!bCrashXml)
	{
		pProgressNotifier->setError("Could not generate the manifest.");
		return false;
	}

	return true;
}

bool ErrorReport::SendReport(ErrorReport::IProgressNotifier* pProgressNotifier)
{
	int nbFilesToSend = 0;
	for(FileList::const_iterator it = m_AttachedFiles.begin(); it != m_AttachedFiles.end(); ++it) 
	{
		if(it->attachToReport)
			nbFilesToSend++;
	}

	pProgressNotifier->taskStarted("Sending crash report", 3 + nbFilesToSend);

	TBG::Server server("https://bugs.arx-libertatis.org");

	// Login to TBG server
	pProgressNotifier->taskStepStarted("Connecting to the bug tracker");
	bool bLoggedIn = server.login(m_Username, m_Password);
	pProgressNotifier->taskStepEnded();
	if(!bLoggedIn)
	{
		pProgressNotifier->setError("Could not connect to the bug tracker");
		return false;
	}

	// Look for existing issue
	int issue_id;
	pProgressNotifier->taskStepStarted("Searching for existing issue");
	bool bSearchSuccessful = server.findIssue(m_ReportUniqueID, issue_id);
	pProgressNotifier->taskStepEnded();
	if(!bSearchSuccessful)
	{
		pProgressNotifier->setError("Failure occured when searching issue on the bug tracker");
		return false;
	}

	// Create new issue if no match was found
	if(issue_id == -1)
	{
		pProgressNotifier->taskStepStarted("Creating new issue");
		bool bCreatedIssue = server.createCrashReport(m_ReportTitle, m_ReportDescription, m_ReproSteps, issue_id);
		if(!bCreatedIssue)
		{
			pProgressNotifier->taskStepEnded();
			pProgressNotifier->setError("Could not create a new issue on the bug tracker");
			return false;
		}

		// Set OS
#if   ARX_PLATFORM == ARX_PLATFORM_WIN32
		int os_id = TBG::Server::OS_Windows;
#elif ARX_PLATFORM == ARX_PLATFORM_LINUX
		int os_id = TBG::Server::OS_Linux;
#elif ARX_PLATFORM == ARX_PLATFORM_MACOSX
		int os_id = TBG::Server::OS_MacOSX;
#elif ARX_PLATFORM == ARX_PLATFORM_BSD
		int os_id = TBG::Server::OS_FreeBSD;
#else
		int os_id = TBG::Server::OS_Other;
#endif
		server.setOperatingSystem(issue_id, os_id);

		// Set Architecture
		int arch_id;
		if(m_ProcessArchitecture == ARX_ARCH_NAME_X86_64)
			arch_id = TBG::Server::Arch_Amd64;
		else if(m_ProcessArchitecture == ARX_ARCH_NAME_X86)
			arch_id = TBG::Server::Arch_x86;
		else
			arch_id = TBG::Server::Arch_Other;
		server.setArchitecture(issue_id, arch_id);

		pProgressNotifier->taskStepEnded();
	}
	else
	{
		if(!m_ReproSteps.isEmpty())
		{
			pProgressNotifier->taskStepStarted("Duplicate issue found, adding information");
			bool bCommentAdded = server.addComment(issue_id, m_ReproSteps);
			pProgressNotifier->taskStepEnded();
			if(!bCommentAdded)
			{
				pProgressNotifier->setError("Failure occured when trying to add information to an existing issue");
				return false;
			}
		}
	}

	// Send files
	for(FileList::const_iterator it = m_AttachedFiles.begin(); it != m_AttachedFiles.end(); ++it) 
	{
		if(!it->attachToReport)
			continue;

		pProgressNotifier->taskStepStarted(std::string("Sending file \"") + it->path.filename() + "\"");
		bool bAttached = server.attachFile(issue_id, it->path.string().c_str(), it->path.filename().c_str(), m_SharedMemoryName.c_str());
		pProgressNotifier->taskStepEnded();
		if(!bAttached)
		{
			pProgressNotifier->setError(std::string("Could not send file \"") + it->path.filename() + "\"");
			return false;
		}
	}

	m_IssueLink = server.getUrl().toString();
	
	return true;
}

void ErrorReport::ReleaseApplicationLock()
{
	m_pCrashInfo->exitLock.post();
}

void ErrorReport::AddFile(const fs::path& fileName)
{
	m_AttachedFiles.push_back(File(fileName, true));
}

ErrorReport::FileList& ErrorReport::GetAttachedFiles()
{
	return m_AttachedFiles;
}

const QString& ErrorReport::GetErrorDescription() const
{
	return m_ReportDescription;
}

const QString& ErrorReport::GetIssueLink() const
{
	return m_IssueLink;
}

void ErrorReport::SetLoginInfo(const QString& username, const QString& password)
{
	m_Username = username;
	m_Password = password;
}

void ErrorReport::SetReproSteps(const QString& reproSteps)
{
	m_ReproSteps = reproSteps;
}
