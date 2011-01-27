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
//#include <io.h>
#include <hermes/Logger.h>




/*
	static var initialize
*/
ArxDebug		*	ArxDebug::m_pInstance				= NULL	;


/*
	Get the callstack informations and create the log file with CreateLogFile function
*/

void ArxDebug::Assert(const char * _sMessage, const char * _sFile, unsigned int _iLine)
{
	char msgbuf[8192];
	char fn[MAX_PATH + 1], msg[MAX_PATH + 1], iFile[MAX_PATH + 1];


	strcpy(msg, _sMessage);
	strcpy(iFile, _sFile);

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

	LogError << msgbuf;
}

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
ArxDebug::ArxDebug(bool _bLogIntoFile /*= true*/) {
}

ArxDebug::~ArxDebug() {
}

/*
	Use to log a message
*/
// TODO use Logger directly
void ArxDebug::Log(ARX_DEBUG_LOG_TYPE eType, const char * _sMessage, ...) {
	//Use to stack the message and the params
	char sBuffer[ARXCOMMON_BUFFERSIZE];

	//Treat the ... Params
	va_list arg_ptr ;
	va_start(arg_ptr, _sMessage);
	vsnprintf(sBuffer, ARXCOMMON_BUFFERSIZE, _sMessage, arg_ptr);
	va_end(arg_ptr) ;
	
	switch (eType)
	{
		case eLogWarning :
			LogWarning << sBuffer;
			break;
		case eLogError :
			LogError << sBuffer;
			break;
		case eLog :
		default:
			LogInfo << sBuffer;
	}
	
}
