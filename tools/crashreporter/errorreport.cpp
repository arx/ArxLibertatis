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

#include "io/fs/Filesystem.h"
#include "io/fs/FileStream.h"
#include "platform/Thread.h"
#include "platform/String.h"

// CrashReporter
#include "crashreporter/utilities_win32.h"
#include "crashreporter/tbg/tbg.h"

#include "platform/Architecture.h"

ErrorReport::ErrorReport(const std::string& sharedMemoryName)
	: m_RunningTimeSec()
	, m_SharedMemoryName(sharedMemoryName)
	, m_pCrashInfo()
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

bool ErrorReport::GetScreenshot(const fs::path& fileName, int quality, bool bGrayscale)
{
	fs::path fullPath = m_ReportFolder / fileName;

#ifdef HAVE_WINAPI
	WId mainWindow = GetMainWindow(m_pCrashInfo->processId);
	
	if(mainWindow == 0)
		return false;
	
	RECT r;
	GetWindowRect(mainWindow, &r);

	QPixmap pixmap = QPixmap::grabWindow(QApplication::desktop()->winId(), r.left, r.top, r.right - r.left, r.bottom - r.top);
#else
	QPixmap pixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
#endif
	
	if(bGrayscale)
	{
		QImage image = pixmap.toImage();
		QRgb col;
		int gray;
		int width = pixmap.width();
		int height = pixmap.height();
		for (int i = 0; i < width; ++i)
		{
			for (int j = 0; j < height; ++j)
			{
				col = image.pixel(i, j);
				gray = qGray(col);
				image.setPixel(i, j, qRgb(gray, gray, gray));
			}
		}
		pixmap = pixmap.fromImage(image);
	}

	bool bSaved = pixmap.save(fullPath.string().c_str(), 0, quality);
	if(bSaved)
		AddFile(fullPath);

	return bSaved;
}

#ifdef HAVE_WINAPI
// This callbask function is called by MinidumpWriteDump
BOOL CALLBACK MiniDumpCallback(PVOID CallbackParam, PMINIDUMP_CALLBACK_INPUT CallbackInput, PMINIDUMP_CALLBACK_OUTPUT CallbackOutput)
{
	return TRUE;
}
#endif

bool ErrorReport::GetCrashDump(const fs::path& fileName) {
	
#ifdef HAVE_WINAPI
	
	fs::path fullPath = m_ReportFolder / fileName;
	
	MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
	MINIDUMP_CALLBACK_INFORMATION callbackInfo;

	EXCEPTION_POINTERS exceptionPointers;
	exceptionPointers.ExceptionRecord = &m_pCrashInfo->exceptionRecord;
	exceptionPointers.ContextRecord = &m_pCrashInfo->contextRecord;

	// Write minidump to the file
	exceptionInfo.ThreadId = m_pCrashInfo->threadId;
	exceptionInfo.ExceptionPointers = &exceptionPointers;
	exceptionInfo.ClientPointers = FALSE;
  
	callbackInfo.CallbackRoutine = MiniDumpCallback;
	callbackInfo.CallbackParam = 0;

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_pCrashInfo->processId);
	if(hProcess == INVALID_HANDLE_VALUE)
		return false;

	// Create the minidump file
	HANDLE hFile = CreateFile(fullPath.string().c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return false;

	BOOL bWriteDump = MiniDumpWriteDump(hProcess, m_pCrashInfo->processId, hFile, m_pCrashInfo->miniDumpType, &exceptionInfo, NULL, &callbackInfo);

	CloseHandle(hFile);

	if(bWriteDump)
		AddFile(fullPath);

	return bWriteDump;
	
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
	m_Architecture = Is64BitWindows() ? "x86_64" : "x86";
	
	if(m_pCrashInfo->exceptionRecord.ExceptionCode != 0)
	{
		std::string exceptionStr = GetExceptionString(m_pCrashInfo->exceptionRecord.ExceptionCode);
		if(!exceptionStr.empty())
		{
			m_ReportDescription += "\nException code:\n  ";
			m_ReportDescription += exceptionStr + "\n";
		}
	}

	std::string callStack = GetCallStack(hProcess, m_pCrashInfo->threadHandle, &m_pCrashInfo->contextRecord);
	if(!callStack.empty())
	{
		m_ReportDescription += "\nCallstack:\n";
		m_ReportDescription += callStack;
	}

	std::string registers = GetRegisters(&m_pCrashInfo->contextRecord);
	if(!registers.empty())
	{
		m_ReportDescription += "\nRegisters:\n";
		m_ReportDescription += registers;
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
		m_Architecture = buf.machine;
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
			xml.writeTextElement("Architecture", m_Architecture);
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

	pProgressNotifier->taskStarted("Generating crash report", 4);
	
	// Initialize shared memory
	pProgressNotifier->taskStepStarted("Connecting to crashed application");
	Initialize();
	pProgressNotifier->taskStepEnded();
	
	if(m_pCrashInfo->architecture != ARX_ARCH) {
		// TODO architecture mismatch - display an error
		exit(0);
	}
	
	// Take screenshot - non critical
	pProgressNotifier->taskStepStarted("Grabbing screenshot");
	GetScreenshot("screenshot.jpg");
	pProgressNotifier->taskStepEnded();

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

	pProgressNotifier->taskStarted("Sending crash report", 2 + nbFilesToSend);

	TBG::Server server("https://bugs.arx-libertatis.org");

	// Login to TBG server
	pProgressNotifier->taskStepStarted("Connecting to the bug tracker");
	bool bLoggedIn = server.login("TODO_USERNAME", "TODO_PASSWORD");
	pProgressNotifier->taskStepEnded();
	if(!bLoggedIn)
	{
		pProgressNotifier->setError("Could not connect to the bug tracker");
		return false;
	}
	
	// Create new issue
	int issue_id;
	pProgressNotifier->taskStepStarted("Creating new issue");
	bool bCreatedIssue = server.createCrashReport("TODO_ADD_REAL_TITLE", m_ReportDescription.c_str(), issue_id);
	pProgressNotifier->taskStepEnded();
	if(!bCreatedIssue)
	{
		pProgressNotifier->setError("Could not create a new issue on the bug tracker");
		return false;
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

const std::string& ErrorReport::GetErrorDescription() const
{
	return m_ReportDescription;
}
