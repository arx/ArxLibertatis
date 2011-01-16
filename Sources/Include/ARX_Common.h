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
#pragma once

/* ---------------------------------------------------------
						Include
------------------------------------------------------------*/

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdarg.h>
#include <wtypes.h>
#include <time.h>

/* ---------------------------------------------------------
					     Break
------------------------------------------------------------*/

#define appIsDebuggerPresent	IsDebuggerPresent
#define appDebugBreak()			( appIsDebuggerPresent() ? (DebugBreak(),1) : 1 )
#define ARXCOMMON_BUFFERSIZE	512
#define DEBUG_INSIDEAFILE       true

/* ---------------------------------------------------------
					  Maccro for assertion
------------------------------------------------------------*/

enum ARX_DEBUG_LOG_TYPE
{
	eLog,
	eLogError ,
	eLogWarning
};


#ifdef _DEBUG
#define	TEST						__LINE__
#define arx_assert(_Expression) (void)	( (_Expression) ||  (ArxDebug::Assert(_CRT_WIDE(#_Expression), _CRT_WIDE(__FILE__), __LINE__),  DebugBreak() , 0) )

//Use only at game's beginning
#define ARX_LOG_INIT()					ArxDebug::GetInstance(DEBUG_INSIDEAFILE)
#define ARX_LOG_CLEAN()					ArxDebug::GetInstance(false)->CleanInstance()

#define ARX_LOG(_sFmt,...)				ArxDebug::GetInstance(false)->Log(eLog,_sFmt,__VA_ARGS__)
#define ARX_LOG_ERROR(_sFmt,...)		ArxDebug::GetInstance(false)->Log(eLogError,_sFmt,__VA_ARGS__)
#define ARX_LOG_WARNING(_sFmt,...)		ArxDebug::GetInstance(false)->Log(eLogWarning,_sFmt,__VA_ARGS__)
#define ARX_LOG_TAG(_iId,_sTag)			ArxLogTag	_logTag_##_iId	(_sTag)


#else //DO_CHECK
#ifndef NDEBUG
	#define NDEBUG	//For suppress assert.
#endif
#define ARX_LOG(_sFmt,...)				ArxDebug::NullFunc(_sFmt,__VA_ARGS__)
#define ARX_LOG_ERROR(_sFmt,...)		ArxDebug::NullFunc(_sFmt,__VA_ARGS__)
#define ARX_LOG_WARNING(_sFmt,...)		ArxDebug::NullFunc(_sFmt,__VA_ARGS__)

#if _MSC_VER  // MS compilers support noop which discards everything inside the parens
#define ARX_LOG_INIT()							__noop
#define ARX_LOG_CLEAN()							__noop
#define arx_assert(_Expression)					__noop
#define ARX_LOG_TAG(_iId,_sTag)					__noop
#else
#define arx_assert(_Expression)						{}
#define ARX_LOG_INIT()								{}
#define ARX_LOG_CLEAN()								{}
#define ARX_LOG_TAG(_iId,_sTag)						{}
#endif
#endif

/* ---------------------------------------------------------
						Define
------------------------------------------------------------*/

#define _CRT_SECURE_NO_DEPRECATE //ARX: xrichter (2010-07-02) - treat warnings for depreciate functions

//ARX_BEGIN: jycorbel (2010-06-23) - clear warning signed/unsigned mismatch : add macros + assert
#define ARX_CHECK_NOT_NEG( _x )  ( arx_assert( ( _x ) >= 0 ) ) //ARX_BEGIN: jycorbel (2010-06-28) - Add description in assert

#define ARX_CAST_LONG( _x )	( static_cast<long>( _x ) )
#define ARX_CAST_ULONG( _x )( static_cast<unsigned long>( _x ) )
#define ARX_CAST_UINT( _x ) ( static_cast<unsigned int>( _x ) )
#define ARX_CAST_USHORT( _x ) ( static_cast<unsigned short>( _x ) )

# define ARX_OPAQUE_WHITE 0xFFFFFFFF
//ARX_END: jycorbel (2010-06-23)


//ARX_BEGIN: jycorbel (2010-06-28) - clean warning 'variable used without having been initialized'.
#define ARX_CHECK_NO_ENTRY( ) ( arx_assert( FALSE ) )	//ARX: xrichter (2010-07-26) - Modified to use arx_assert
#define ARX_CHECK( _expr ) ( arx_assert( _expr ) )		//ARX: xrichter (2010-07-26) - Modified to use arx_assert
#define ARX_WARN( _str ) ( printf("WARN: %s=%s", #_str, ( _str ) ) )
#define ARX_WARN_F( _f ) ( printf("WARN: %s=%f", #_f, ( _f ) ) )
#define ARX_WARN_I( _i ) ( printf("WARN: %s=%d", #_i, ( _i ) ) )
//ARX_END: jycorbel (2010-06-28)



/* ---------------------------------------------------------
						Pragma
------------------------------------------------------------*/

#pragma warning(disable : 4100) //ARX - jycorbel - 2010/06/22 - Disable warning "unreferenced formal parameter"
#pragma warning(disable : 4101) //ARX - jycorbel - 2010/06/22 - Disable warning "unreferenced local variable"
#pragma warning(disable : 4127) //ARX - jycorbel - 2010/06/22 - Disable warning "conditional expression is constant"
#pragma warning(disable : 4189) //ARX - jycorbel - 2010/06/22 - Disable warning "local variable is initialized but not referenced"
#pragma warning(disable : 4201) //ARX - jycorbel - 2010/06/22 - Disable warning "nonstandard extension used : nameless struct/union"
#pragma warning(disable : 4702) //ARX - jycorbel - 2010/06/22 - Disable warning "unreachable code"
#pragma warning(disable : 4706) //ARX - jycorbel - 2010/06/22 - Disable warning "assignment within conditional expression"
#pragma warning(disable : 4995) //ARX: xrichter (2010-07-02) - treat warnings for depreciate functions
#pragma warning(disable : 4996) //ARX: xrichter (2010-07-02) - treat warnings for depreciate functions

//ARX_BEGIN: jycorbel (2010-07-02) - Assure compatibility with C code (Mercury).
#define ARX_DEAD_CODE() {arx_assert( FALSE );} //ARX: xrichter (2010-06-23) - treat warnings C4700  Disable warning "local variable % used without having been initialized"
//ARX_END: jycorbel (2010-07-02)

//ARX_BEGIN: xrichter (2010-06-24) - Use to verify no overflow value by data type
#define ARX_CHECK_UCHAR(_x)	{arx_assert( static_cast<short>(_x) <= UCHAR_MAX		&& _x >= 0 ) ;}
#define ARX_CHECK_BYTE(_x)	ARX_CHECK_UCHAR(_x)
#define ARX_CHECK_CHAR(_x)	{arx_assert( static_cast<short>(_x) <= CHAR_MAX			&& static_cast<short>(_x) >= CHAR_MIN ) ;}
#define ARX_CHECK_SHORT(_x)	{arx_assert( static_cast<int>(_x) <= SHRT_MAX			&& static_cast<int>(_x) >= SHRT_MIN ) ;}
#define ARX_CHECK_USHORT(_x){arx_assert( static_cast<int>(_x) <= USHRT_MAX			&& _x >= 0 ) ;}
#define ARX_CHECK_WORD(_x)	ARX_CHECK_USHORT(_x)		//typedef unsigned short      WORD;
#define ARX_CHECK_INT(_x)	{arx_assert( static_cast<long long>(_x) <= INT_MAX		&& static_cast<long long>(_x) >= INT_MIN ) ;}
#define ARX_CHECK_UINT(_x)  {arx_assert( static_cast<long long>(_x) <= UINT_MAX		&& _x >= 0 ) ;}
#define ARX_CHECK_SIZET(_x) ARX_CHECK_UINT(_x)
#define ARX_CHECK_LONG(_x)	{arx_assert( static_cast<long long>(_x) <= LONG_MAX		&& static_cast<long long>(_x) >= LONG_MIN ) ;}
#define ARX_CHECK_ULONG(_x) {arx_assert( static_cast<long long>(_x) <= ULONG_MAX	&& _x >= 0	);}
#define ARX_CHECK_DWORD(_x) ARX_CHECK_ULONG(_x)
//ARX_END: xrichter (2010-06-24)


//ARX_BEGIN: xrichter (2010-06-23) - treat warnings C4244 for convertion problem
#define ARX_CLEAN_WARN_CAST_FLOAT(_x) (static_cast<float>( _x ))
#define ARX_CLEAN_WARN_CAST_UCHAR(_x) (static_cast<unsigned char>( _x ))
#define ARX_CLEAN_WARN_CAST_CHAR(_x) (static_cast<char>( _x ))
#define ARX_CLEAN_WARN_CAST_BYTE(_x) (static_cast<BYTE>( _x ))
#define ARX_CLEAN_WARN_CAST_SHORT(_x) (static_cast<short>( _x ))
#define ARX_CLEAN_WARN_CAST_USHORT(_x) (static_cast<unsigned short>( _x ))
#define ARX_CLEAN_WARN_CAST_WORD(_x)	(static_cast<WORD>( _x ))
#define ARX_CLEAN_WARN_CAST_INT(_x)	  (static_cast<int>( _x ))
#define ARX_CLEAN_WARN_CAST_UINT(_x)  (static_cast<unsigned int>( _x ))
#define ARX_CLEAN_WARN_CAST_SIZET(_x) (static_cast<size_t>( _x ))
#define ARX_CLEAN_WARN_CAST_LONG(_x)  (static_cast<long>( _x ))
#define ARX_CLEAN_WARN_CAST_ULONG(_x) (static_cast<unsigned long>( _x ))
#define ARX_CLEAN_WARN_CAST_DWORD(_x) (static_cast<DWORD>( _x ))
#define ARX_CLEAN_WARN_CAST_D3DVALUE(_x) (static_cast<D3DVALUE>( _x ))
//ARX_END: xrichter (2010-06-23)



/* ---------------------------------------------------------
						Arx Common class
------------------------------------------------------------*/
// maximum mumber of lines the output console should have
#define ARXCOMMON_MAX_CONSOLE_LINES  1500
#define ARXCOMMON_MAX_CONSOLE_ROWS	 200

#define ARXDEBUG_COLOR_ERROR	FOREGROUND_RED
#define ARXDEBUG_COLOR_WARNING	FOREGROUND_GREEN
#define ARXDEBUG_COLOR_DEFAULT  FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED


class ArxDebug
{
		friend class ArxLogTag;

	public :
		static ArxDebug * GetInstance(bool _bLogIntoFile = true);
		static void CleanInstance();
		static void NullFunc(const char * _sMessage, ...) {};
		static void Assert(const wchar_t * _sMessage, const wchar_t * _sFile, unsigned _iLine);

		void Log(ARX_DEBUG_LOG_TYPE eType, const char * _sMessage, ...);

		~ArxDebug();

	private :
		static ArxDebug * m_pInstance ;

		bool				m_bConsoleInitialize	;
		bool				m_bOpenLogFile	;
		std::ofstream		m_fsFile ;
		unsigned int		m_uiTabulation ;
		std::ostringstream	m_ossBuffer ;

	private :
		ArxDebug(bool _bLogIntoFile);
		void RedirectIOToConsole();
		bool CreateDebugConsole();
		void CleanConsole();

		void OpenTag(const char * _sTag); //Used to specify a session inside the log file
		void CloseTag();

		void StartLogSession();
		void EndLogSession();

		void AddTabulation(std::ostringstream & _ossBuffer);
		void LogTypeManager(ARX_DEBUG_LOG_TYPE eType);
		static void cpy_wstr(char * buf, const wchar_t * src, size_t max);
		static bool CreateLogDirectory();
		static void CreateCrashFile(const char * _sFn , const char * _sMsg , const char * _sFile , unsigned int _iLine, const std::string & _sStackTrace);
};


/*
	Use to have an automatic indentation without scope problems
*/
class ArxLogTag
{
	public :
		//Increment ArxDebug::m_uiTabulation
		ArxLogTag(const char * _sTag = "")
		{
			ArxDebug::GetInstance(false)->OpenTag(_sTag);
		}

		//Decrement ArxDebug::m_uiTabulation
		~ArxLogTag()
		{
			ArxDebug::GetInstance()->CloseTag();
		}

};



















