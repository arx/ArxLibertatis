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

#include "errorreport.h"

// Win32
#include <winsock2.h>
#include <windows.h>
#include <DbgHelp.h>
#include <Psapi.h>

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

// CrashReporter
#include "utilities_win32.h"

// Zip
#include "minizip/zip.h"

// CSmtp
#include "csmtp/csmtp.h"

ErrorReport::ErrorReport(const std::string& crashesFolder, const std::string& sharedMemoryName)
	: m_RunningTimeSec()
	, m_ProcessIs64Bit()
	, m_pCrashInfo()
	, m_SharedMemoryName(sharedMemoryName)
{
	m_CrashesFolder = crashesFolder.c_str();
	m_CrashesFolder += QDir::separator();
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
		m_AttachedFiles.push_back(m_pCrashInfo->attachedFiles[i]);

	m_CurrentReportFolder = m_CrashesFolder;
	m_CurrentReportFolder += m_CrashDateTime.toString("yyyy.MM.dd hh.mm.ss");
	m_CurrentReportFolder += QDir::separator();

	QDir dir;
	bool bMkPath = dir.mkpath(m_CurrentReportFolder);
	if(!bMkPath)
		return false;

	return true;
}

QString	ErrorReport::GetFilePath(const std::string& fileName) const
{
	return m_CurrentReportFolder + fileName.c_str();
}

bool ErrorReport::GetScreenshot(const std::string& fileName, int quality, bool bGrayscale)
{
	QString fullPath = GetFilePath(fileName);

	WId mainWindow = GetMainWindow(m_pCrashInfo->processId);
	
	if(mainWindow == 0)
		return false;

	RECT r;
	GetWindowRect(mainWindow, &r);

	QPixmap pixmap = QPixmap::grabWindow(QApplication::desktop()->winId(), r.left, r.top, r.right - r.left, r.bottom - r.top);

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

	bool bSaved = pixmap.save(fullPath, 0, quality);
	if(bSaved)
		m_AttachedFiles.push_back(fullPath);

	return bSaved;
}

// This callbask function is called by MinidumpWriteDump
BOOL CALLBACK MiniDumpCallback(PVOID CallbackParam, PMINIDUMP_CALLBACK_INPUT CallbackInput, PMINIDUMP_CALLBACK_OUTPUT CallbackOutput)
{
	return TRUE;
}

bool ErrorReport::GetCrashDump(const std::string& fileName)
{
	QString fullPath = GetFilePath(fileName);

	HMODULE hDbgHelp = LoadLibrary("dbghelp.dll");
	if(hDbgHelp==NULL)
		return false;
	
	typedef LPAPI_VERSION (WINAPI* LPIMAGEHLPAPIVERSIONEX)(LPAPI_VERSION AppVersion);
	typedef BOOL (WINAPI *LPMINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD ProcessId, HANDLE hFile, MINIDUMP_TYPE DumpType, CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, CONST PMINIDUMP_USER_STREAM_INFORMATION UserEncoderParam, CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

	// Check if we have a valid dbghelp API version
	LPIMAGEHLPAPIVERSIONEX lpImagehlpApiVersionEx = (LPIMAGEHLPAPIVERSIONEX)GetProcAddress(hDbgHelp, "ImagehlpApiVersionEx");
	if(lpImagehlpApiVersionEx == NULL)
		return false;

	API_VERSION CompiledApiVer;
	CompiledApiVer.MajorVersion = 6;
	CompiledApiVer.MinorVersion = 1;
	CompiledApiVer.Revision = 11;
	CompiledApiVer.Reserved = 0;
	LPAPI_VERSION pActualApiVer = lpImagehlpApiVersionEx(&CompiledApiVer);
	if( CompiledApiVer.MajorVersion != pActualApiVer->MajorVersion ||
		CompiledApiVer.MinorVersion != pActualApiVer->MinorVersion ||
		CompiledApiVer.Revision != pActualApiVer->Revision )
		return false;

	LPMINIDUMPWRITEDUMP pfnMiniDumpWriteDump = (LPMINIDUMPWRITEDUMP)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");
	if(pfnMiniDumpWriteDump == NULL)
		return false;

	MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
	MINIDUMP_CALLBACK_INFORMATION callbackInfo;

	// Write minidump to the file
	exceptionInfo.ThreadId = m_pCrashInfo->threadId;
	exceptionInfo.ExceptionPointers = m_pCrashInfo->pExceptionPointers;
	exceptionInfo.ClientPointers = TRUE;
  
	callbackInfo.CallbackRoutine = MiniDumpCallback;
	callbackInfo.CallbackParam = 0;

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_pCrashInfo->processId);
	if(hProcess == INVALID_HANDLE_VALUE)
		return false;

	// Create the minidump file
	HANDLE hFile = CreateFile(fullPath.toAscii(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return false;

	BOOL bWriteDump = pfnMiniDumpWriteDump(hProcess, m_pCrashInfo->processId, hFile, m_pCrashInfo->miniDumpType, &exceptionInfo, NULL, &callbackInfo);

	CloseHandle(hFile);
	FreeLibrary(hDbgHelp);

	if(bWriteDump)
		m_AttachedFiles.push_back(fullPath);

	return bWriteDump;
}

class QtSleeper : public QThread
{
public:
	static void usleep(unsigned long usecs){QThread::usleep(usecs);}
	static void msleep(unsigned long msecs){QThread::msleep(msecs);}
	static void sleep(unsigned long secs){QThread::sleep(secs);}
};

bool ErrorReport::GetMachineInfo(const std::string& fileName)
{
	QString fullPath = GetFilePath(fileName);

	QString program = "dxdiag.exe";
	QStringList arguments;
	arguments << "/whql:off"
			  << "/64bit"
			  << "/t" << fullPath.toAscii();

	QStringList env = QProcess::systemEnvironment();

	QTime timeWatch;
	timeWatch.start();

	QProcess *myProcess = new QProcess();
	myProcess->start(program, arguments);
	bool bStarted = myProcess->waitForStarted(10000);
	if(!bStarted)
		return false;

	// Writing the DXDiag file can take quite some time...
	QFile dxdiagReport(fullPath);
	while(!dxdiagReport.exists() && timeWatch.elapsed() < 120000)
	{
		QtSleeper::msleep(1000);
	}

	bool bExists = dxdiagReport.exists();
	if(bExists)
		m_AttachedFiles.push_back(fullPath);

	return bExists;
}

bool ErrorReport::GetMiscCrashInfo()
{
	// Get crash time
	m_CrashDateTime = QDateTime::currentDateTime();

	// Open parent process handle
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, m_pCrashInfo->processId);
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
	m_OSIs64Bit = Is64BitWindows();

	return true;
}

bool ErrorReport::WriteReport(const std::string& fileName)
{
	QString fullPath = GetFilePath(fileName);

	QFile file(fullPath);
	if(!file.open(QIODevice::WriteOnly))
		return false;

	m_AttachedFiles.push_back(fullPath);

	QXmlStreamWriter xml;
	xml.setDevice(&file);
	xml.setAutoFormatting(true);
	xml.writeStartDocument();

		xml.writeComment("Information related to the crashed process");
		xml.writeStartElement("Process");
			xml.writeTextElement("Name", m_ProcessName);
			xml.writeTextElement("Path", m_ProcessPath);
			xml.writeTextElement("MemoryUsage", QString::number(m_ProcessMemoryUsage));
			xml.writeTextElement("Is64Bit", m_ProcessIs64Bit ? "True" : "False");
			xml.writeTextElement("RunningTime", QString::number(m_RunningTimeSec));
			xml.writeTextElement("CrashDateTime", m_CrashDateTime.toString("dd.MM.yyyy hh:mm:ss"));
		xml.writeEndElement();

		xml.writeComment("Information related to the OS");
		xml.writeStartElement("OS");
			xml.writeTextElement("Name", m_OSName);
			xml.writeTextElement("Is64Bit", m_OSIs64Bit ? "True" : "False");
		xml.writeEndElement();

		xml.writeComment("List of files attached to the crash report");
		xml.writeStartElement("Files");
		for(QList<QString>::const_iterator it = m_AttachedFiles.begin(); it != m_AttachedFiles.end(); ++it)
		{
			xml.writeTextElement("File", *it);
		}
		xml.writeEndElement();

	xml.writeEndDocument();

	file.close();

	return true;
}

bool ErrorReport::GenerateArchive()
{
	std::string fileName = m_SharedMemoryName + ".zip";
	QString fullPath = GetFilePath(fileName);

	int bufferSize = 64*1024;
	void* buf = malloc(bufferSize);

	zipFile zf = zipOpen(fullPath.toAscii(), 0);

	if(zf == 0) 
	{
		free(buf);
		return false;
	}

	int err = ZIP_OK;
	foreach (QString fn, m_AttachedFiles) 
	{
		zip_fileinfo zi;
		memset(&zi, sizeof(zi), 0);

		int opt_compress_level = Z_BEST_COMPRESSION;
		//filetime(fn.toAscii(), &zi.dosDate);

		zipOpenNewFileInZip(zf, 
							fn.toAscii(),
							&zi,
							NULL,
							0,
							NULL,
							0,
							NULL,
							(opt_compress_level != 0) ? Z_DEFLATED : 0,
							opt_compress_level);

		// Open source file
		FILE * fin = fopen(fn.toAscii(), "r+b");
		if(!fin)
		{
			err = ZIP_INTERNALERROR;
			break;
		}

		// Copy data from source file into the zip file
		int size_read;
		do
		{
			size_read = fread(buf,1,bufferSize,fin);
			if (size_read < bufferSize)
			{
				if (feof(fin)==0)
				{
					err = ZIP_INTERNALERROR;
					break;
				}
			}

			if (err == ZIP_OK && size_read>0)
			{
				err = zipWriteInFileInZip (zf,buf,size_read);
				if (err < 0)
				{
					break;
				}
			}
		}
		while (err==ZIP_OK && size_read>0);
		if(err != ZIP_OK)
			break;

		fclose(fin);

		err = zipCloseFileInZip(zf);
		if(err != ZIP_OK)
			break;
	}

	free(buf);
	
	if(err != ZIP_OK)
	{
		zipClose(zf, "Generated by Arx Libertatis crash reporter");
		return false;
	}

	err = zipClose(zf, "Generated by Arx Libertatis crash reporter");
	return err == ZIP_OK;
}

bool ErrorReport::GenerateReport(ErrorReport::IProgressNotifier* pProgressNotifier)
{
	ErrorReport* report = this;
	BOOST_SCOPE_EXIT((report))
	{
		// Allow the crashed process to exit
		report->ReleaseApplicationLock();
	} BOOST_SCOPE_EXIT_END

	pProgressNotifier->taskStarted("Generating crash report", 5);
	
	// Initialize shared memory
	pProgressNotifier->taskStepStarted("Connecting to crashed application");
	Initialize();
	pProgressNotifier->taskStepEnded();

	// Take screenshot - non critical
	pProgressNotifier->taskStepStarted("Grabbing screenshot");
	GetScreenshot("screenshot.jpg");
	pProgressNotifier->taskStepEnded();

	// Generate minidump
	pProgressNotifier->taskStepStarted("Generating minidump");
	bool bCrashDump = GetCrashDump("crash.dmp");
	pProgressNotifier->taskStepEnded();
	if(!bCrashDump)
	{
		pProgressNotifier->setError("Could not generate the minidump.");
		return false;
	}

	// Gather machine info - SLOW!
	//taskStepStarted("Gathering system information");
	//bool bMachineInfo = m_errorReport.GetMachineInfo("machineInfo.txt");
	//taskStepEnded();

	// Generate xml
	pProgressNotifier->taskStepStarted("Generating report manifest");
	bool bCrashXml = WriteReport("crash.xml");
	pProgressNotifier->taskStepEnded();
	if(!bCrashXml)
	{
		pProgressNotifier->setError("Could not generate the manifest.");
		return false;
	}

	// Generate archive
	pProgressNotifier->taskStepStarted("Compressing report");
	bool bCrashArchive = GenerateArchive();
	pProgressNotifier->taskStepEnded();
	if(!bCrashArchive)
	{
		pProgressNotifier->setError("Could not generate the error archive.");
		return false;
	}

	return true;
}

bool ErrorReport::SendReport(ErrorReport::IProgressNotifier* pProgressNotifier)
{
	pProgressNotifier->taskStarted("Sending crash report", 2);

	std::string fileName = m_SharedMemoryName + ".zip";
	QString fullPath = GetFilePath(fileName);

	CSmtp smptClient;
	smptClient.SetSenderName("Arx Libertatis Crashes");
	smptClient.SetSenderMail("arxlibertatis.crashes@gmail.com");
	smptClient.AddRecipient("arxlibertatis.crashes@gmail.com");
	smptClient.SetSubject("Arx Libertatis Crash Report");
	smptClient.AddMsgLine("Hello");
	smptClient.AddAttachment(fullPath.toAscii());
	
	// Connect to server
	bool connected = false;
	pProgressNotifier->taskStepStarted("Connecting to server");
	{
		try
		{
			connected = smptClient.ConnectRemoteServer("smtp.gmail.com", 465, USE_SSL, true, "arxlibertatis.crashes@gmail.com", "yu8pnioo");
		}
		catch(const ECSmtp& err)
		{
			pProgressNotifier->setError(err.GetErrorText());
		}
	}
	pProgressNotifier->taskStepEnded();
	if(!connected)
		return false;

	// Send report
	bool sent = true;
	pProgressNotifier->taskStepStarted("Sending report");
	{
		try
		{
			smptClient.Send();
		}
		catch(const ECSmtp& err)
		{
			pProgressNotifier->setError(err.GetErrorText());
		}
	}
	pProgressNotifier->taskStepEnded();

	return sent;
}

void ErrorReport::ReleaseApplicationLock()
{
	m_pCrashInfo->exitLock.post();
}

const QStringList& ErrorReport::GetAttachedFiles() const
{
	return m_AttachedFiles;
}