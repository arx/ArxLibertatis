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

#if defined(HAVE_PRCTL)
#include <sys/prctl.h>
#ifndef PR_SET_PTRACER
#define PR_SET_PTRACER 0x59616d61
#endif
#endif

#ifdef HAVE_SYSCONF
#include <unistd.h>
#endif

#ifdef HAVE_CRASHHANDLER_POSIX
#include <signal.h>
#endif

// yes, we need stdio.h, POSIX doesn't know about cstdio
#ifdef HAVE_POPEN
#include <stdio.h>
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
#include <QByteArray>

// Boost
#include <boost/crc.hpp>

#include "Configure.h"

#include "core/Version.h"

#include "io/fs/Filesystem.h"
#include "io/fs/FileStream.h"

#include "crashreporter/utilities_win32.h"
#include "crashreporter/tbg/tbg.h"

#include "platform/Architecture.h"
#include "platform/String.h"
#include "platform/Thread.h"

ErrorReport::ErrorReport(const std::string& sharedMemoryName)
	: m_RunningTimeSec(0)
	, m_ProcessMemoryUsage(0)
	, m_SharedMemoryName(sharedMemoryName)
	, m_pCrashInfo()
	, m_Username("CrashBot")
	, m_Password("WbAtVjS9")
{
#if defined(HAVE_PRCTL)
	// Allow debuggers to be attached to this process, for development purpose...
	prctl(PR_SET_PTRACER, 1, 0, 0, 0);
#endif
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

bool ErrorReport::GetCrashDump(const fs::path & fileName) {
	
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
	
	return getCrashDescription();
	
#endif // !HAVE_WINAPI
}

#if defined(HAVE_POPEN) && defined(HAVE_PCLOSE)
static QString getOutputOf(const char * command) {
	FILE * pipe = popen(command, "r");
	if(!pipe) {
		return QString();
	}
	char buffer[1024];
	QByteArray result;
	while(!feof(pipe)) {
		if(size_t count = fread(buffer, 1, ARRAY_SIZE(buffer), pipe)) {
			result.append(buffer, count);
		}
	}
	pclose(pipe);
	return result;
}
#endif

#ifndef HAVE_WINAPI

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
	
#ifdef HAVE_GETRUSAGE
	{
		struct rusage usage;
		if(getrusage(pid, &usage) == 0) {
			memoryUsage = usage.ru_maxrss * 1024;
		}
	}
#endif
	
#if defined(HAVE_SYSCONF) && (defined(_SC_PAGESIZE) || defined(_SC_CLK_TCK))
	
	u64 rss, startTicks, endTicks, dummy;
	
	getProcessSatus(QString("/proc/%1/stat").arg(pid), rss, startTicks);
	getProcessSatus("/proc/self/stat", dummy, endTicks);
	
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
	if(startTicks != 0 && endTicks != 0) {
		u64 ticksPerSecond = sysconf(_SC_CLK_TCK);
		if(ticksPerSecond > 0) {
			runningTimeSec = double(endTicks - startTicks) / double(ticksPerSecond);
		}
	}
#endif
	
#endif
	
}

QString getLinuxDistribution() {
	
#if defined(HAVE_POPEN) && defined(HAVE_PCLOSE)
	{
		QString distro(getOutputOf("lsb_release -d").trimmed());
		QString prefix("Description:");
		if(distro.startsWith(prefix)) {
			distro = distro.mid(prefix.length()).trimmed();
		}
		if(!distro.isEmpty()) {
			return distro;
		}
	}
#endif
	
	// Fallback for older / non-LSB-compliant distros.
	// Release file list taken from http://linuxmafia.com/faq/Admin/release-files.html
	
	const char * release_files[] = {
		"/etc/annvix-release",
		"/etc/arch-release",
		"/etc/arklinux-release",
		"/etc/aurox-release",
		"/etc/blackcat-release",
		"/etc/cobalt-release",
		"/etc/conectiva-release",
		"/etc/fedora-release",
		"/etc/gentoo-release",
		"/etc/immunix-release",
		"/etc/lfs-release",
		"/etc/linuxppc-release",
		"/etc/mandriva-release",
		"/etc/mandrake-release",
		"/etc/mandakelinux-release",
		"/etc/mklinux-release",
		"/etc/nld-release",
		"/etc/pld-release",
		"/etc/slackware-release",
		"/etc/e-smith-release",
		"/etc/release",
		"/etc/sun-release",
		"/etc/SuSE-release",
		"/etc/novell-release",
		"/etc/sles-release",
		"/etc/tinysofa-release",
		"/etc/turbolinux-release",
		"/etc/ultrapenguin-release",
		"/etc/UnitedLinux-release",
		"/etc/va-release",
		"/etc/yellowdog-release",
		"/etc/debian_release",
		"/etc/redhat-release",
	};
	
	const char * version_files[][2] = {
		{ "/etc/debian_version", "Debian " },
		{ "/etc/knoppix_version", "Knoppix " },
		{ "/etc/redhat_version", "RedHat " },
		{ "/etc/slackware-version", "Slackware " },
	};
	
	const char * lsb_release = "/etc/lsb-release";
	
	for(size_t i = 0; i < ARRAY_SIZE(release_files); i++) {
		QFile file(release_files[i]);
		if(file.exists()) {
			file.open(QIODevice::ReadOnly);
			QString distro(QString(file.readAll()).trimmed());
			file.close();
			if(!distro.isEmpty()) {
				return distro;
			}
		}
	}
	
	for(size_t i = 0; i < ARRAY_SIZE(version_files); i++) {
		QFile file(version_files[i][0]);
		if(file.exists()) {
			file.open(QIODevice::ReadOnly);
			QString distro(version_files[i][1] + QString(file.readAll()).trimmed());
			file.close();
			if(!distro.isEmpty()) {
				return distro;
			}
		}
	}
	
	QFile file(lsb_release);
	if(file.exists()) {
		file.open(QIODevice::ReadOnly);
		QString distro(QString(file.readAll()).trimmed());
		file.close();
		QString prefix("DISTRIB_ID=\"");
		QString suffix("\"");
		if(distro.startsWith(prefix) && distro.endsWith(suffix)) {
			distro = distro.mid(
				prefix.length(),
				distro.length() - prefix.length() - suffix.length()
			).trimmed();
		}
		return distro;
	}
	
	return QString();
}

#endif // !defined(HAVE_WINAPI)

bool ErrorReport::getCrashDescription() {
	
#ifdef HAVE_WINAPI
	
	return true;
	
#else // !defined(HAVE_WINAPI)
	
	switch(m_pCrashInfo->signal) {
		case SIGABRT:  m_ReportDescription = "Abnormal termination"; break;
		case SIGFPE:   m_ReportDescription = "Floating-point error"; break;
		case SIGILL:   m_ReportDescription = "Illegal instruction"; break;
		case SIGSEGV:  m_ReportDescription = "Illegal storage access"; break;
		default: {
			m_ReportDescription = QString("Received signal %1").arg(m_pCrashInfo->signal);
			break;
		}
	}
	
	if(m_pCrashInfo->signal == SIGFPE) {
		// Append detailed information in case of a FPE exception
		switch(m_pCrashInfo->fpeCode) {
			#ifdef FPE_INTDIV
			case FPE_INTDIV: m_ReportDescription += ": Integer divide by zero"; break;
			#endif
			#ifdef FPE_INTOVF
			case FPE_INTOVF: m_ReportDescription += ": Integer overflow"; break;
			#endif
			#ifdef FPE_FLTDIV
			case FPE_FLTDIV: m_ReportDescription += ": Floating point divide by zero"; break;
			#endif
			#ifdef FPE_FLTOVF
			case FPE_FLTOVF: m_ReportDescription += ": Floating point overflow"; break;
			#endif
			#ifdef FPE_FLTUND
			case FPE_FLTUND: m_ReportDescription += ": Floating point underflow"; break;
			#endif
			#ifdef FPE_FLTRES
			case FPE_FLTRES: m_ReportDescription += ": Floating point inexact result"; break;
			#endif
			#ifdef FPE_FLTINV
			case FPE_FLTINV: m_ReportDescription += ": Floating point invalid operation"; break;
			#endif
			#ifdef FPE_FLTSUB
			case FPE_FLTSUB: m_ReportDescription += ": Subscript out of range"; break;
			#endif
			default: break;
		}
	}
	m_ReportDescription += "\n\n";
	
	m_ReportDescriptionText = m_ReportDescription;
	
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
	if(!bWroteDump) {
		return false;
	}
	
	AddFile(tracePath);
	
#ifdef HAVE_BACKTRACE
	
	boost::crc_32_type callstackCRC32;
	
	for(size_t i = 0; i < ARRAY_SIZE(m_pCrashInfo->backtrace); i++) {
		if(m_pCrashInfo->backtrace[i] == 0) {
			break;
		}
		callstackCRC32.process_bytes(&m_pCrashInfo->backtrace[i], sizeof(m_pCrashInfo->backtrace[i]));
	}
	
	u32 callstackCrc = callstackCRC32.checksum();
	m_ReportUniqueID = QString("[%1]").arg(QString::number(callstackCrc, 16).toUpper());
	
#endif // HAVE_BACKTRACE
	
	QFile traceFile(tracePath.string().c_str());
	traceFile.open(QIODevice::ReadOnly);
	if(!traceFile.isOpen()) {
		return false;
	}
	
	QString traceStr = traceFile.readAll();
	traceFile.close();
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
	
#endif // !defined(HAVE_WINAPI)
	
	return true;
}

bool ErrorReport::GetMiscCrashInfo() {
	
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
	
	m_ReportDescription = m_pCrashInfo->detailedCrashInfo;
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
	
	m_ReportDescriptionText = m_ReportDescription;
	
#else // !HAVE_WINAPI
	
	getResourceUsage(m_pCrashInfo->processId, m_ProcessMemoryUsage, m_RunningTimeSec);
	
#ifdef HAVE_UNAME
	struct utsname buf;
	if(uname(&buf) == 0) {
		m_OSName = QString(buf.sysname) + " " + buf.release;
		m_OSArchitecture = buf.machine;
	}
#endif
	
	m_OSDistribution = getLinuxDistribution();
	
#endif // !HAVE_WINAPI

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
	int issue_id = -1;
	pProgressNotifier->taskStepStarted("Searching for existing issue");
	bool bSearchSuccessful = m_ReportUniqueID.isEmpty() || server.findIssue(m_ReportUniqueID, issue_id);
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
		bool bCreatedIssue = server.createCrashReport(m_ReportTitle, m_ReportDescription, m_ReproSteps, tbg_version_id, issue_id);
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
