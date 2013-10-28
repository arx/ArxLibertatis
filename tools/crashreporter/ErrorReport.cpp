/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#include <signal.h>

#if ARX_PLATFORM == ARX_PLATFORM_WIN32
// Win32
#include <windows.h>
#include <psapi.h>
#endif

#if ARX_HAVE_GETRUSAGE
#include <sys/resource.h>
#include <sys/time.h>
#endif

#if ARX_HAVE_PRCTL
#include <sys/prctl.h>
#ifndef PR_SET_PTRACER
#define PR_SET_PTRACER 0x59616d61
#endif
#endif

#if ARX_HAVE_SYSCONF
#include <unistd.h>
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
#include <QSslSocket>
#include <QThread>
#include <QXmlStreamWriter>
#include <QByteArray>

// Boost
#include <boost/crc.hpp>

#include "Configure.h"

#include "core/Version.h"

#include "io/fs/Filesystem.h"
#include "io/fs/FileStream.h"

#include "crashreporter/Win32Utilities.h"
#include "crashreporter/tbg/TBG.h"

#include "platform/Architecture.h"
#include "platform/OS.h"
#include "platform/Process.h"

ErrorReport::ErrorReport(const QString& sharedMemoryName)
	: m_RunningTimeSec(0)
	, m_ProcessMemoryUsage(0)
	, m_SharedMemoryName(sharedMemoryName)
	, m_pCrashInfo()
	, m_Username("CrashBot")
	, m_Password("WbAtVjS9")
{
#if ARX_HAVE_PRCTL && defined(DEBUG)
	// Allow debuggers to be attached to this process, for development purpose...
	prctl(PR_SET_PTRACER, 1, 0, 0, 0);
#endif
}

bool ErrorReport::Initialize()
{	
	// Create a shared memory object.
	m_SharedMemory = boost::interprocess::shared_memory_object(boost::interprocess::open_only, m_SharedMemoryName.toStdString().c_str(), boost::interprocess::read_write);

	// Map the whole shared memory in this process
	m_MemoryMappedRegion = boost::interprocess::mapped_region(m_SharedMemory, boost::interprocess::read_write);

	// Our SharedCrashInfo will be stored in this shared memory.
	m_pCrashInfo = (CrashInfo*)m_MemoryMappedRegion.get_address();

	if(m_MemoryMappedRegion.get_size() != sizeof(CrashInfo))
	{
		m_DetailedError = "The size of the memory mapped region does not match the size of the CrashInfo structure.";
		return false;
	}

	bool bMiscCrashInfo = GetMiscCrashInfo();
	if(!bMiscCrashInfo)
		return false;

	// Add attached files from the report
	for(int i = 0; i < m_pCrashInfo->nbFilesAttached; i++)
		AddFile(m_pCrashInfo->attachedFiles[i]);

	m_ReportFolder = fs::path(m_pCrashInfo->crashReportFolder) / fs::path(m_CrashDateTime.toString("yyyy.MM.dd hh.mm.ss").toUtf8());

	if(!fs::create_directories(m_ReportFolder))
	{
		m_DetailedError = QString("Unable to create directory (%1) to store the crash report files.").arg(m_ReportFolder.string().c_str());
		return false;
	}

	return true;
}

bool ErrorReport::GetCrashDump(const fs::path & fileName) {
	
#if ARX_PLATFORM == ARX_PLATFORM_WIN32
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
	
#else //  ARX_PLATFORM != ARX_PLATFORM_WIN32
	
	ARX_UNUSED(fileName);
	
	// TODO: Write core dump to 
	// fs::path fullPath = m_ReportFolder / fileName;
	
	return getCrashDescription();
	
#endif
	
}

#if ARX_PLATFORM != ARX_PLATFORM_WIN32

void getProcessSatus(QString filename, u64 & rss, u64 & startTicks) {
	
	rss = startTicks = 0;
	
	QFile file(filename);
	
	if(!file.open(QIODevice::ReadOnly)) {
		return;
	}
	QByteArray stat = file.readAll();
	file.close();
	
	QList<QByteArray> stat_fields = stat.split(' ');
	
	const int rss_index = 23;
	if(rss_index < stat_fields.size()) {
		rss = stat_fields[rss_index].toULongLong();
	}
	
	const int starttime_index = 21;
	if(starttime_index < stat_fields.size()) {
		startTicks = stat_fields[starttime_index].toULongLong();
	}
	
}

void getResourceUsage(int pid, quint64 & memoryUsage, double & runningTimeSec) {
	
	memoryUsage = 0;
	runningTimeSec = 0.0;
	
#if ARX_HAVE_GETRUSAGE && ARX_PLATFORM != ARX_PLATFORM_MACOSX
	{
		struct rusage usage;
		if(getrusage(pid, &usage) == 0) {
			memoryUsage = usage.ru_maxrss * 1024;
		}
	}
#endif
	
#if ARX_HAVE_SYSCONF && (defined(_SC_PAGESIZE) || defined(_SC_CLK_TCK))
	
	u64 rss, startTicks, endTicks, dummy;
	
	getProcessSatus(QString("/proc/%1/stat").arg(pid), rss, startTicks);
	
	// Get the rss memory usage from /proc/pid/stat
#ifdef _SC_PAGESIZE
	if(rss != 0) {
		long pagesize = sysconf(_SC_PAGESIZE);
		if(pagesize > 0) {
			memoryUsage = rss * pagesize;
		}
	}
#endif
	
#ifdef _SC_CLK_TCK

	if(startTicks == 0) {
		return;
	}
	
	pid_t child = fork();
	if(!child) {
		while(1) {
			// wait
		}
	}
	
	getProcessSatus(QString("/proc/%1/stat").arg(child), dummy, endTicks);
	kill(child, SIGTERM);
	
	if(startTicks != 0 && endTicks != 0) {
		u64 ticksPerSecond = sysconf(_SC_CLK_TCK);
		if(ticksPerSecond > 0) {
			runningTimeSec = double(endTicks - startTicks) / double(ticksPerSecond);
		}
	}
	
#endif
	
#endif
	
}

#endif //  ARX_PLATFORM != ARX_PLATFORM_WIN32

bool ErrorReport::getCrashDescription() {
	
#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	
	return true;
	
#else // ARX_PLATFORM != ARX_PLATFORM_WIN32
	
	switch(m_pCrashInfo->signal) {
		
		case SIGILL: {
			m_ReportDescription = "Illegal instruction";
			switch(m_pCrashInfo->code) {
				#ifdef ILL_ILLOPC
				case ILL_ILLOPC: m_ReportDescription += ": illegal opcode"; break;
				#endif
				#ifdef ILL_ILLOPN
				case ILL_ILLOPN: m_ReportDescription += ": illegal operand"; break;
				#endif
				#ifdef ILL_ADR
				case ILL_ADR: m_ReportDescription += ": illegal addressing mode"; break;
				#endif
				#ifdef ILL_ILLTRP
				case ILL_ILLTRP: m_ReportDescription += ": illegal trap"; break;
				#endif
				#ifdef ILL_PRVOPC
				case ILL_PRVOPC: m_ReportDescription += ": privileged opcode"; break;
				#endif
				#ifdef ILL_PRVREG
				case ILL_PRVREG: m_ReportDescription += ": privileged register"; break;
				#endif
				#ifdef ILL_COPROC
				case ILL_COPROC: m_ReportDescription += ": coprocessor error"; break;
				#endif
				#ifdef ILL_BADSTK
				case ILL_BADSTK: m_ReportDescription += ": internal stack error"; break;
				#endif
				default: break;
			}
			break;
		}
		
		case SIGABRT: {
			m_ReportDescription = "Abnormal termination";
			break;
		}
		
		case SIGBUS: {
			m_ReportDescription = "Bus error";
			switch(m_pCrashInfo->code) {
				#ifdef BUS_ADRALN
				case BUS_ADRALN: m_ReportDescription += ": invalid address alignment"; break;
				#endif
				#ifdef BUS_ADRERR
				case BUS_ADRERR: m_ReportDescription += ": non-existent physical address"; break;
				#endif
				#ifdef BUS_OBJERR
				case BUS_OBJERR: m_ReportDescription += ": object specific hardware error"; break;
				#endif
				default: break;
			}
			break;
		}
		
		case SIGFPE: {
			m_ReportDescription = "Floating-point error";
			switch(m_pCrashInfo->code) {
				#ifdef FPE_INTDIV
				case FPE_INTDIV: m_ReportDescription += ": integer division by zero"; break;
				#endif
				#ifdef FPE_INTOVF
				case FPE_INTOVF: m_ReportDescription += ": integer overflow"; break;
				#endif
				#ifdef FPE_FLTDIV
				case FPE_FLTDIV: m_ReportDescription += ": floating point division by zero"; break;
				#endif
				#ifdef FPE_FLTOVF
				case FPE_FLTOVF: m_ReportDescription += ": floating point overflow"; break;
				#endif
				#ifdef FPE_FLTUND
				case FPE_FLTUND: m_ReportDescription += ": floating point underflow"; break;
				#endif
				#ifdef FPE_FLTRES
				case FPE_FLTRES: m_ReportDescription += ": floating point inexact result"; break;
				#endif
				#ifdef FPE_FLTINV
				case FPE_FLTINV: m_ReportDescription += ": invalid floating point operation"; break;
				#endif
				#ifdef FPE_FLTSUB
				case FPE_FLTSUB: m_ReportDescription += ": subscript out of range"; break;
				#endif
				default: break;
			}
			break;
		}
		
		case SIGSEGV: {
			m_ReportDescription = "Illegal storage access";
			switch(m_pCrashInfo->code) {
				#ifdef SEGV_MAPERR
				case SEGV_MAPERR: {
					m_ReportDescription += ": address not mapped to object";
					break;
				}
				#endif
				#ifdef SEGV_ACCERR
				case SEGV_ACCERR: {
					m_ReportDescription += ": invalid permissions for mapped object";
					break;
				}
				#endif
				default: break;
			}
			break;
		}
		
		default: {
			m_ReportDescription = QString("Received signal %1").arg(m_pCrashInfo->signal);
			break;
		}
	}
	
	m_ReportDescription += "\n\n";
	
	m_ReportDescriptionText = m_ReportDescription;
	
	// Get a stack trace via GDB
	char pid_buf[36];
	memset(&pid_buf, 0, sizeof(pid_buf));
	sprintf(pid_buf, "--pid=%d", m_pCrashInfo->processId);
	const char * args[] = {
		"gdb", "--batch", "-n",
		"-ex", "thread",
		"-ex", "set confirm off",
		"-ex", "set print frame-arguments all",
		"-ex", "set print static-members off",
		"-ex", "info threads",
		"-ex", "thread apply all bt full",
		pid_buf, NULL
	};
	std::string gdbstdout = platform::getOutputOf("gdb", args, /*unlocalized=*/ true);
	if(gdbstdout.empty()) {
		m_DetailedError = "GDB is probably not installed on your machine."
		                  " Please install it in order to obtain valuable crash reports in the future.";
		return false;
	}
	
#if ARX_HAVE_BACKTRACE
	
	boost::crc_32_type callstackCRC32;
	
	for(size_t i = 0; i < ARRAY_SIZE(m_pCrashInfo->backtrace); i++) {
		if(m_pCrashInfo->backtrace[i] == 0) {
			break;
		}
		callstackCRC32.process_bytes(&m_pCrashInfo->backtrace[i], sizeof(m_pCrashInfo->backtrace[i]));
	}
	
	u32 callstackCrc = callstackCRC32.checksum();
	m_ReportUniqueID = QString("[%1]").arg(QString::number(callstackCrc, 16).toUpper());
	
#endif
	
	QString traceStr = QString::fromUtf8(gdbstdout.c_str());
	QString callstackTop = "Unknown";
	
	// The useful stack frames are found below "<signal handler called>"
	int posStart = traceStr.indexOf("<signal handler called>");
	int posEnd = -1;
	if(posStart != -1) {
		posStart = traceStr.indexOf("#", posStart);
		if(posStart != -1)
			posEnd = traceStr.indexOf("\n", posStart);
	}
	
	// Capture the entry where the crash occured and format it
	if(posStart != -1 && posEnd != -1)	{
		callstackTop = traceStr.mid(posStart, posEnd - posStart);

		// Remove "#N 0x???????? in "
		posEnd = callstackTop.indexOf(" in ");
		if(posEnd != -1) {
			posEnd += 4;
			callstackTop.remove(0, posEnd);
		}
		
		// Remove params
		posStart = callstackTop.indexOf("(");
		posEnd = callstackTop.indexOf(")", posStart);
		if(posStart != -1 && posEnd != -1) {
			posStart++;
			callstackTop.remove(posStart, posEnd - posStart);
		}
		
		// Trim filenames
		posStart = callstackTop.lastIndexOf(") at");
		posEnd = callstackTop.lastIndexOf("/");
		if(posStart != -1 && posEnd != -1) {
			posStart += 2;
			posEnd++;
			callstackTop.remove(posStart, posEnd - posStart);
		}
	}
	
	m_ReportDescription += "\nGDB stack trace:\n";
	m_ReportDescription += "<source lang=\"gdb\">\n";
	m_ReportDescription += traceStr;
	m_ReportDescription += "</source>\n";
	
	m_ReportDescriptionText += "\nGDB stack trace:\n";
	m_ReportDescriptionText += "\n";
	m_ReportDescriptionText += traceStr;
	
	m_ReportTitle = QString("%1 %2").arg(m_ReportUniqueID, callstackTop.trimmed());
	
#endif // ARX_PLATFORM != ARX_PLATFORM_WIN32
	
	return true;
}

bool ErrorReport::GetMiscCrashInfo() {
	
	// Get crash time
	m_CrashDateTime = QDateTime::currentDateTime();
	
	m_ProcessArchitecture = ARX_ARCH_NAME;
	
	m_OSName = QString::fromUtf8(platform::getOSName().c_str());
	m_OSArchitecture = QString::fromUtf8(platform::getOSArchitecture().c_str());
	m_OSDistribution = QString::fromUtf8(platform::getOSDistribution().c_str());
	
#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	
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
	else
	{
		m_DetailedError = QString("Unable to obtain an handle to the crashed process (Error %1).").arg(QString::number(GetLastError()));
		return false;
	}
	
	if(m_pCrashInfo->exceptionCode != 0)
	{
		QString exceptionStr = GetExceptionString(m_pCrashInfo->exceptionCode).c_str();
		if(!exceptionStr.isEmpty())
		{
			m_ReportDescription += "\nException code:\n  ";
			m_ReportDescription += exceptionStr;
			m_ReportDescription += "\n";
		}
	}

	std::string callStack, callstackTop;
	u32 callstackCrc;

	bool bCallstack = GetCallStackInfo(hProcess, m_pCrashInfo->threadHandle, &m_pCrashInfo->contextRecord, callStack, callstackTop, callstackCrc);
	if(!bCallstack) 
	{
		m_DetailedError = "A failure occured when obtaining information regarding the callstack.";
		return false;
	}
	
	m_ReportUniqueID = QString("[%1]").arg(QString::number(callstackCrc, 16).toUpper());
	
	m_ReportDescription = m_pCrashInfo->detailedCrashInfo;
	m_ReportDescription += "\nCallstack:\n";
	m_ReportDescription += callStack.c_str();
	m_ReportTitle = QString("%1 %2").arg(m_ReportUniqueID, callstackTop.c_str());

	QString registers(GetRegisters(&m_pCrashInfo->contextRecord).c_str());
	if(!registers.isEmpty())
	{
		m_ReportDescription += "\nRegisters:\n";
		m_ReportDescription += registers;
	}
	
	CloseHandle(hProcess);
	
	m_ReportDescriptionText = m_ReportDescription;
	
#else // ARX_PLATFORM != ARX_PLATFORM_WIN32
	
	getResourceUsage(m_pCrashInfo->processId, m_ProcessMemoryUsage, m_RunningTimeSec);
	
#endif

	return true;
}

bool ErrorReport::WriteReport(const fs::path & fileName) {
	
	fs::path fullPath = m_ReportFolder / fileName;
	
	QFile file(fullPath.string().c_str());
	if(!file.open(QIODevice::WriteOnly)) {
		m_DetailedError = "Unable to open report manifest for writing.";
		return false;
	}
	
	QXmlStreamWriter xml;
	xml.setDevice(&file);
	xml.setAutoFormatting(true);
	xml.writeStartDocument();
	xml.writeStartElement("CrashReport");
		
		xml.writeComment("Information related to the crashed process");
		xml.writeStartElement("Process");
			xml.writeTextElement("Path", m_pCrashInfo->executablePath);
			xml.writeTextElement("Version", m_pCrashInfo->executableVersion);
			if(m_ProcessMemoryUsage != 0) {
				xml.writeTextElement("MemoryUsage", QString::number(m_ProcessMemoryUsage));
			}
			xml.writeTextElement("Architecture", m_ProcessArchitecture);
			if(m_RunningTimeSec > 0) {
				xml.writeTextElement("RunningTime", QString::number(m_RunningTimeSec));
			}
			xml.writeTextElement("CrashDateTime", m_CrashDateTime.toString("dd.MM.yyyy hh:mm:ss"));
		xml.writeEndElement();

		xml.writeComment("Information related to the OS");
		xml.writeStartElement("OS");
			if(!m_OSName.isEmpty()) {
				xml.writeTextElement("Name", m_OSName);
			}
			if(!m_OSArchitecture.isEmpty()) {
				xml.writeTextElement("Architecture", m_OSArchitecture);
			}
			if(!m_OSDistribution.isEmpty()) {
				xml.writeTextElement("Distribution", m_OSDistribution);
			}
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

	AddFile(fullPath);
	
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
	bool bInit = Initialize();
	pProgressNotifier->taskStepEnded();
	if(!bInit)
	{
		pProgressNotifier->setError("Could not generate the crash dump.");
		pProgressNotifier->setDetailedError(m_DetailedError);
		return false;
	}
	
	if(m_pCrashInfo->architecture != ARX_ARCH) {
		pProgressNotifier->setError("Architecture mismatch between the crashed process and the crash reporter.");
		pProgressNotifier->setDetailedError(m_DetailedError);
		return false;
	}
	
	// Generate minidump
	pProgressNotifier->taskStepStarted("Generating crash dump");
	bool bCrashDump = GetCrashDump("crash.dmp");
	pProgressNotifier->taskStepEnded();
	if(!bCrashDump)
	{
		pProgressNotifier->setError("Could not generate the crash dump.");
		pProgressNotifier->setDetailedError(m_DetailedError);
		return false;
	}

	// Generate manifest
	pProgressNotifier->taskStepStarted("Generating report manifest");
	bool bCrashXml = WriteReport("crash.xml");
	pProgressNotifier->taskStepEnded();
	if(!bCrashXml)
	{
		pProgressNotifier->setError("Could not generate the manifest.");
		pProgressNotifier->setDetailedError(m_DetailedError);
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

	// This must be called before creating our QNetworkAccessManager
	AddSSLCertificate();

	TBG::Server server("https://bugs.arx-libertatis.org");
	
	// Login to TBG server
	pProgressNotifier->taskStepStarted("Connecting to the bug tracker");
	bool bLoggedIn = server.login(m_Username, m_Password);
	pProgressNotifier->taskStepEnded();
	if(!bLoggedIn)
	{
		pProgressNotifier->setError("Could not connect to the bug tracker");
		pProgressNotifier->setDetailedError(server.getErrorString());
		return false;
	}

	// Look for existing issue
	int issue_id = -1;
	pProgressNotifier->taskStepStarted("Searching for existing issue");
	bool bSearchSuccessful = m_ReportUniqueID.isEmpty() || server.findIssue(m_ReportUniqueID, issue_id);
	pProgressNotifier->taskStepEnded();
	if(!bSearchSuccessful)
	{
		pProgressNotifier->setError("Failure occured when searching issue on the bug tracker");
		pProgressNotifier->setDetailedError(server.getErrorString());
		return false;
	}

	// Create new issue if no match was found
	if(issue_id == -1)
	{
		pProgressNotifier->taskStepStarted("Creating new issue");
		bool bCreatedIssue = server.createCrashReport(m_ReportTitle, m_ReportDescription, m_ReproSteps, tbg_version_id, issue_id);
		if(!bCreatedIssue)
		{
			pProgressNotifier->taskStepEnded();
			pProgressNotifier->setError("Could not create a new issue on the bug tracker");
			pProgressNotifier->setDetailedError(server.getErrorString());
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
				pProgressNotifier->setDetailedError(server.getErrorString());
				return false;
			}
		}
	}

	// Send files
	for(FileList::const_iterator it = m_AttachedFiles.begin(); it != m_AttachedFiles.end(); ++it) 
	{
		// Ignore files that were removed by the user.
		if(!it->attachToReport)
			continue;

		// One more check to verify that the file still exists.
		if(!fs::exists(it->path))
			continue;

		pProgressNotifier->taskStepStarted(QString("Sending file \"%1\"").arg(it->path.filename().c_str()));
		bool bAttached = server.attachFile(issue_id, it->path.string().c_str(), it->path.filename().c_str(), m_SharedMemoryName);
		pProgressNotifier->taskStepEnded();
		if(!bAttached)
		{
			pProgressNotifier->setError(QString("Could not send file \"%1\"").arg(it->path.filename().c_str()));
			pProgressNotifier->setDetailedError(server.getErrorString());
			return false;
		}
	}

	m_IssueLink = server.getUrl().toString();
	
	return true;
}

void ErrorReport::ReleaseApplicationLock() {
#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	m_pCrashInfo->exitLock.post();
#else
	// Kill the original, busy-waiting process.
	kill(m_pCrashInfo->processId, SIGKILL);
#endif
}

void ErrorReport::AddSSLCertificate() {
	
	QFile file(":/startcom.cer", 0);
	if(file.open(QIODevice::ReadOnly)) {
		QSslCertificate certificate(file.readAll(), QSsl::Der);
		
		if(certificate.isNull()) {
			return;
		}
		
		#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
		if(!certificate.isValid()) {
			return;
		}
		#else
		if(certificate.isBlacklisted()) {
			return;
		}
		#endif
		
		QSslSocket::addDefaultCaCertificate(certificate);
	}
}

void ErrorReport::AddFile(const fs::path& fileName)
{
	// Do not include files that can't be found, and empty files...
	if(fs::exists(fileName) && fs::file_size(fileName) != 0)
		m_AttachedFiles.push_back(File(fileName, true));
}

ErrorReport::FileList& ErrorReport::GetAttachedFiles()
{
	return m_AttachedFiles;
}

const QString& ErrorReport::GetErrorDescription() const
{
	return m_ReportDescriptionText;
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
