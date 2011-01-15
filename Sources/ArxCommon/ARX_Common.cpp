/*
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// ARX_Common
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		All preprocessor directives set for all the solution.
//
// Updates: (date) (person) (update)
//
// Code:	Jean-Yves CORBEL
//			  Xavier RICHTER
//
// Copyright (c) 1999-2010 ARKANE Studios SA. All rights reserved
/////////////////////////////////////////////////////////////////////////////////////


//-----------------------------------------------------------------------------------//
#include <ARX_Common.h>
#include <windows.h>
#include <stdio.h>
#include <signal.h>
//#include <ARX_StackLogger.h>
#include <time.h>
#include <fcntl.h>
#include <io.h>




/*
	static var initialize
*/
ArxDebug		*	ArxDebug::m_pInstance				= NULL	;



/*
	Convert wchar to char
*/
void ArxDebug::cpy_wstr(char * buf, const wchar_t * src, size_t max)
{
	if (src)
	{
		while (max > 0 && *src != 0)
		{
			*buf++ = (char) src[0];
			--max;
			src++;
		}
	}

	*buf = 0;
}


/*
	Get the callstack informations and create the log file with CreateLogFile function
*/

void ArxDebug::Assert(const wchar_t * _sMessage, const wchar_t * _sFile, unsigned int _iLine)
{
	char msgbuf[8192];
	char fn[MAX_PATH + 1], msg[MAX_PATH + 1], iFile[MAX_PATH + 1];


	cpy_wstr(msg, _sMessage, MAX_PATH);
	cpy_wstr(iFile, _sFile, MAX_PATH);

	if (iFile[0] == 0)
	{
		strcpy(iFile, "<unknown>");
	}

	if (msg[0] == 0)
	{
		strcpy(msg, "?");
	}

	fn[MAX_PATH] = 0;

	if (! GetModuleFileNameA(NULL, fn, MAX_PATH))
	{
		strcpy(fn, "<unknown>");
	}

	sprintf(msgbuf, "Assertation failed!\n\nProgram: %s\nFile: %s, Line %u\n\nExpression: %s",
	        fn, iFile, _iLine, msg);

	/*std::string stackTrace ;
	  ArxStackLogger::StackLogger s ;
	  s.GetStackTrace(stackTrace);
	  CreateCrashFile(fn, msg, iFile, _iLine, stackTrace);
	  */
}

/*
	Use to create Log directory
*/
bool ArxDebug::CreateLogDirectory()
{
	bool bReturn = true ;

	//Verify if the log folder exist, otherwise create him
	char * sLogReposiriry = "..\\Log";

	if (CreateDirectoryA(sLogReposiriry, NULL) == 0)
	{
		DWORD iCodeError = GetLastError();

		switch (iCodeError)
		{
			case ERROR_ALREADY_EXISTS :
				break ;

			case ERROR_PATH_NOT_FOUND :
				bReturn = false ;
				MessageBoxA(NULL, "Folder Path not found", "File ", MB_OK | MB_ICONHAND | MB_SETFOREGROUND | MB_TASKMODAL);
				break ;

			default:
				bReturn = false ;
				MessageBoxA(NULL, "Folder Log cannot be create. May be you dont have the permission or enought space disk.", "File ", MB_OK | MB_ICONHAND | MB_SETFOREGROUND | MB_TASKMODAL);

		}
	}

	return bReturn ;
}


/*
	Use to create the log file into the Log folder.
*/
/*
void ArxDebug::CreateCrashFile(const char * _sFn , const char * _sMsg , const char * _sFile , unsigned int _iLine, const std::string & _sStackTrace)
{
	DWORD nCode;

	//Create the message bow for ask if we want a report log
	std::ostringstream oss ;
	oss << "Executable : " << _sFn << "\nFile : " << _sFile << "	Line : " << _iLine << "\nCause : " << _sMsg;
	std::string _sFileOss = oss.str().c_str();

	oss << "\n Do you want to generate a report into your Log folder ? ";

	nCode = MessageBoxA(NULL, oss.str().c_str(), "Arx Runtime Assertation ", MB_YESNO |
	                    MB_ICONHAND | MB_SETFOREGROUND | MB_TASKMODAL);

	if ((nCode == IDYES) && CreateLogDirectory())
	{
		//Open an outpout stream and write inside
		std::ofstream fsFile ;
		std::ostringstream ossFileName ;
		ossFileName << "..\\Log\\CrashReport__" << time(NULL) << ".txt";

		fsFile.open(ossFileName.str().c_str(), std::ios::out | std::ios::trunc);

		if (!fsFile)
		{
			MessageBoxA(NULL, "Crash report cannot be create. May be you don't have the permission or enough space disk.", "Crash Report ", MB_OK | MB_ICONHAND | MB_SETFOREGROUND | MB_TASKMODAL);
			return ;
		}

		fsFile << __DATE__ << "\n\n" << _sFileOss << "\n\n ------------------------------------------ CallStack Log --------------------------------- \n\n" << _sStackTrace;

		fsFile.close();
	}
}
*/

/*
	Singleton Getter/Cleaner
*/
ArxDebug * ArxDebug::GetInstance(bool _bLogIntoFile /*= true*/)
{
	if (!m_pInstance)
	{
		m_pInstance = new ArxDebug(_bLogIntoFile);
	}

	return m_pInstance;
}


void ArxDebug::CleanInstance()
{
	if (m_pInstance)
	{
		delete m_pInstance ;
	}
}


/*
	Constructor & Destructor
*/
ArxDebug::ArxDebug(bool _bLogIntoFile /*= true*/)
{

	m_bConsoleInitialize	= false ;
	m_bOpenLogFile			= false ;
	m_uiTabulation			= 0		;

	RedirectIOToConsole();

	if (_bLogIntoFile)
	{
		StartLogSession();
	}
}

ArxDebug::~ArxDebug()
{
	CleanConsole();
	EndLogSession();

	m_pInstance = NULL ;
	m_uiTabulation = 0	;
}


/*
	Create and Clean Console for log
*/

void ArxDebug::RedirectIOToConsole()
{
	int hConHandle;
	long lStdHandle;
	FILE * fp;

	if ((m_bConsoleInitialize = CreateDebugConsole()))
	{
		freopen("CONOUT$", "wt", stdout);
		freopen("CONERR$", "wt", stderr);
		freopen("CONIN$",  "r",  stdin) ;
	}

}

bool ArxDebug::CreateDebugConsole()
{
	CONSOLE_SCREEN_BUFFER_INFO coninfo;

	// allocate a console for this app
	if (!AllocConsole())
	{
		MessageBoxA(NULL, "Cannot create Log Console ", "Log Console Error ", MB_OK | MB_ICONHAND | MB_SETFOREGROUND | MB_TASKMODAL);
		return false ;
	}

	SetConsoleTitleA("Arx Fatalis Debug Window");

	// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.X = ARXCOMMON_MAX_CONSOLE_ROWS ;
	coninfo.dwSize.Y = ARXCOMMON_MAX_CONSOLE_LINES;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);
	return true ;
}





void ArxDebug::CleanConsole()
{
	if (m_bConsoleInitialize)
	{
		FreeConsole();
	}
}


/*
	Log Functions
*/
void ArxDebug::StartLogSession()
{
	if (CreateLogDirectory())
	{
		unsigned int uiID = static_cast<unsigned int>(time(NULL));

		std::ostringstream ossFileName ;
		ossFileName << "..\\Log\\Log__" << uiID << ".txt";
		m_fsFile.open(ossFileName.str().c_str(), std::ios::out | std::ios::app);

		if (m_fsFile)
		{
			m_bOpenLogFile = true ;
		}
		else
		{
			m_bOpenLogFile = false ;
			MessageBoxA(NULL, "Log file cannot be create. May be you dont have the permission or enought space disk.", "Log File ", MB_OK | MB_ICONHAND | MB_SETFOREGROUND | MB_TASKMODAL);
		}
	}
}



/*
	Use to manage color inside the console
*/
void ArxDebug::LogTypeManager(ARX_DEBUG_LOG_TYPE eType)
{

	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	time_t timestamp;
	struct tm * t;

	timestamp = time(NULL);
	t = localtime(&timestamp);
	
	switch (eType)
	{
		case eLogWarning :
			SetConsoleTextAttribute(hStdOut, ARXDEBUG_COLOR_WARNING);
			m_ossBuffer << "[Warning : " << t->tm_hour << "h " << t->tm_min << "m " << t->tm_sec << "s] : ";
			break;
		case eLogError :
			SetConsoleTextAttribute(hStdOut, ARXDEBUG_COLOR_ERROR);
			m_ossBuffer << "[Error : " << t->tm_hour << "h " << t->tm_min << "m " << t->tm_sec << "s] : ";
			break;
		case eLog :
		default:
			SetConsoleTextAttribute(hStdOut, ARXDEBUG_COLOR_DEFAULT);
			m_ossBuffer << "[Log : " << t->tm_hour << "h " << t->tm_min << "m " << t->tm_sec << "s] : ";
	}
}


/*
	Add tabulation for a better look
*/
void ArxDebug::AddTabulation(std::ostringstream & _ossBuffer)
{
	for (unsigned int i = 0 ; i < m_uiTabulation ; ++i)
	{
		_ossBuffer << "\t";
	}
}


/*
	Use to log a message
*/



void ArxDebug::Log(ARX_DEBUG_LOG_TYPE eType, const char * _sMessage, ...)
{
	//Use to stack the message and the params
	char sBuffer[ARXCOMMON_BUFFERSIZE];

	//Treat the ... Params
	va_list arg_ptr ;
	va_start(arg_ptr, _sMessage);
	_vsnprintf(sBuffer, ARXCOMMON_BUFFERSIZE, _sMessage, arg_ptr);
	va_end(arg_ptr) ;

	LogTypeManager(eType);

	//Use to Add the tabulation inside the oss
	AddTabulation(m_ossBuffer);
	m_ossBuffer << sBuffer << "\n";

	//We want to write log inside a file
	if (m_bOpenLogFile)
	{
		m_fsFile << m_ossBuffer.str().c_str();
		m_fsFile.flush();
	}

	//Write the log on the console
	if (m_bConsoleInitialize)
	{
		std::cout << m_ossBuffer.str().c_str();
		std::cout.flush();
	}
}


/*
	Close the stream
*/
void ArxDebug::EndLogSession()
{
	if (m_bOpenLogFile)
	{
		m_fsFile.close();
		m_bOpenLogFile = false ;
	}
}




/*
	Add a new session into the log
*/
void ArxDebug::OpenTag(const char * _sTag)
{
	++m_uiTabulation;

	m_ossBuffer << "[Log : " << __TIME__ << "] :";
	AddTabulation(m_ossBuffer);
	m_ossBuffer << " ---------------- " << _sTag << " ----------------" << "\n";

	if (m_bOpenLogFile)
	{
		m_fsFile << m_ossBuffer.str().c_str();
		m_fsFile.flush();
	}

	//Write tag inside the console
	if (m_bConsoleInitialize)
	{
		std::cout << m_ossBuffer.str().c_str();
	}
}


/*
	Close a tag
*/
void ArxDebug::CloseTag()
{
	(m_uiTabulation == 0) ? m_uiTabulation : --m_uiTabulation;
}





