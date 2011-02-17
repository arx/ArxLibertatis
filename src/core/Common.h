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
#ifndef ARX_COMMON_H
#define ARX_COMMON_H

/* ---------------------------------------------------------
						Include
------------------------------------------------------------*/

// TODO remove these
#include <climits>
#include <cstdio>
#include <cstdarg>
#include <ctime>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using std::string;

const string arxVersion = "0.1";

/* ---------------------------------------------------------
						Platforms
------------------------------------------------------------*/
#define	ARX_PLATFORM_UNKNOWN	0
#define	ARX_PLATFORM_WIN32		1
#define	ARX_PLATFORM_PS3_PPU	2
#define	ARX_PLATFORM_LINUX		3

#if defined(__PPU__)
	#define ARX_PLATFORM		ARX_PLATFORM_PS3_PPU
#elif defined(_WIN32)
	#define	ARX_PLATFORM		ARX_PLATFORM_WIN32
#elif defined(__linux)
    #define	ARX_PLATFORM		ARX_PLATFORM_LINUX
#endif

#ifndef ARX_PLATFORM
	#warning "Unknown target platform"
	#define	ARX_PLATFORM		ARX_PLATFORM_UNKNOWN
#endif

/* ---------------------------------------------------------
						Compilers
------------------------------------------------------------*/
#define	ARX_COMPILER_UNKNOWN	0
#define	ARX_COMPILER_VC9		1
#define	ARX_COMPILER_VC10		2
#define	ARX_COMPILER_GCC		3

#if defined(__GNUC__)
	#define ARX_COMPILER		ARX_COMPILER_GCC
#elif defined(_MSC_VER)
	#if _MSC_VER < 1600
		#define	ARX_COMPILER	ARX_COMPILER_VC9
	#elif _MSC_VER < 1700
		#define	ARX_COMPILER	ARX_COMPILER_VC10
	#endif
#endif

#ifndef ARX_COMPILER
	#warning "Unknown compiler"
	#define ARX_COMPILER		ARX_COMPILER_UNKNOWN
#endif

#define ARX_COMPILER_MSVC	((ARX_COMPILER == ARX_COMPILER_VC9) || (ARX_COMPILER == ARX_COMPILER_VC10))

// TODO: Move this in a platform specific file
#if ARX_COMPILER_MSVC
    #include <direct.h>

    // Windows like to be non-standard... sigh
	inline int strcasecmp(const char* str1, const char* str2) { return _stricmp(str1, str2); }
	inline int strncasecmp(const char* str1, const char* str2, size_t maxCount) { return _strnicmp(str1, str2, maxCount); }
	inline int chdir(const char* path) { return _chdir(path); }

    char* strcasestr(const char *haystack, const char *needle);
#endif


/* ---------------------------------------------------------
					     Types
------------------------------------------------------------*/

typedef signed char         s8;		//  8 bits integer
typedef unsigned char       u8;     //  8 bits unsigned integer

typedef signed short        s16;    // 16 bits signed integer
typedef unsigned short      u16;    // 16 bits unsigned integer

#if ARX_COMPILER_MSVC
	typedef signed long     s32;    // 32 bits signed integer
	typedef unsigned long   u32;    // 32 bits unsigned integer
#else
	typedef signed int      s32;    // 32 bits signed integer
	typedef unsigned int    u32;    // 32 bits unsigned integer
#endif

typedef signed long long    s64;    // 64 bits signed integer
typedef unsigned long long  u64;    // 64 bits unsigned integer

typedef float               f32;    // 32 bits float
typedef double              f64;    // 64 bits double float


/* ---------------------------------------------------------
						 Break
------------------------------------------------------------*/

#define ARXCOMMON_BUFFERSIZE	512
#define DEBUG_INSIDEAFILE       true

#if ARX_COMPILER_MSVC
	#define ARX_DEBUG_BREAK()	__debugbreak()
#elif ARX_COMPILER == ARX_COMPILER_GCC
	#define ARX_DEBUG_BREAK()	__builtin_trap()
#else
	#define ARX_DEBUG_BREAK()
#endif


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
#define	TEST						    __LINE__

#define arx_assert(_Expression)	\
	(void)( (_Expression) || (ArxDebug::Assert((#_Expression), (__FILE__), __LINE__),  ARX_DEBUG_BREAK() , 0) )

#define arx_assert_msg(_Expression, _Message) \
	(void)( (_Expression) || (ArxDebug::Assert((#_Expression), (__FILE__), __LINE__, _Message),  ARX_DEBUG_BREAK() , 0) )


#else //DO_CHECK
#ifndef NDEBUG
	#define NDEBUG	//For suppress assert.
#endif

#if _MSC_VER  // MS compilers support noop which discards everything inside the parens
#define arx_assert(_Expression)					__noop
#else
#define arx_assert(_Expression)						{}
#endif
#endif

/* ---------------------------------------------------------
						Define
------------------------------------------------------------*/

//ARX_BEGIN: jycorbel (2010-06-23) - clear warning signed/unsigned mismatch : add macros + assert
#define ARX_CHECK_NOT_NEG( _x )  ( arx_assert( ( _x ) >= 0 ) ) //ARX_BEGIN: jycorbel (2010-06-28) - Add description in assert

#define ARX_CAST_LONG( _x )	( static_cast<long>( _x ) )
#define ARX_CAST_ULONG( _x )( static_cast<unsigned long>( _x ) )
#define ARX_CAST_UINT( _x ) ( static_cast<unsigned int>( _x ) )
#define ARX_CAST_USHORT( _x ) ( static_cast<unsigned short>( _x ) )

# define ARX_OPAQUE_WHITE 0xFFFFFFFF
//ARX_END: jycorbel (2010-06-23)


//ARX_BEGIN: jycorbel (2010-06-28) - clean warning 'variable used without having been initialized'.
#define ARX_CHECK_NO_ENTRY( ) ( arx_assert( false ) )	//ARX: xrichter (2010-07-26) - Modified to use arx_assert
#define ARX_CHECK( _expr ) ( arx_assert( _expr ) )		//ARX: xrichter (2010-07-26) - Modified to use arx_assert
#define ARX_WARN( _str ) ( std::printf("WARN: %s=%s", #_str, ( _str ) ) )
#define ARX_WARN_F( _f ) ( std::printf("WARN: %s=%f", #_f, ( _f ) ) )
#define ARX_WARN_I( _i ) ( std::printf("WARN: %s=%d", #_i, ( _i ) ) )
//ARX_END: jycorbel (2010-06-28)



/* ---------------------------------------------------------
						Pragma
------------------------------------------------------------*/

//ARX_BEGIN: jycorbel (2010-07-02) - Assure compatibility with C code (Mercury).
#define ARX_DEAD_CODE() {arx_assert( false );} //ARX: xrichter (2010-06-23) - treat warnings C4700  Disable warning "local variable % used without having been initialized"
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
	public :
		static void Assert(const char * _sExpression, const char * _sFile, unsigned _iLine, const char * _sMessage = 0);
	
};


/* ---------------------------------------------------------
					String utilities
------------------------------------------------------------*/
template <class STYPE>
inline const char * safeGetString(const char * & pos, STYPE & size) {	
	const char * begin = pos;
	
	for(size_t i = 0; i < size; i++) {
		if(pos[i] == 0) {
			size -= i + 1;
			pos += i + 1;
			return begin;
		}
	}
	
	return NULL;
}

template <class T, class STYPE>
inline bool safeGet(T & data, const char * & pos, STYPE & size) {
	if(size < sizeof(T)) {
		return false;
	}
	data = *reinterpret_cast<const T *>(pos);
	pos += sizeof(T);
	size -= sizeof(T);
	return true;
}

#endif // ARX_COMMON_H
