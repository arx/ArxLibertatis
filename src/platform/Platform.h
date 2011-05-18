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

#ifndef ARX_PLATFORM_PLATFORM_H
#define ARX_PLATFORM_PLATFORM_H

#include <string>
#include <climits>

const std::string arxVersion = "0.1";

/* ---------------------------------------------------------
                          Platforms
------------------------------------------------------------*/

#define	ARX_PLATFORM_UNKNOWN	0
#define	ARX_PLATFORM_WIN32		1
#define	ARX_PLATFORM_PS3_PPU	2
#define	ARX_PLATFORM_LINUX		3

#if defined(__PPU__)
	#define ARX_PLATFORM ARX_PLATFORM_PS3_PPU
#elif defined(__linux)
	#define ARX_PLATFORM ARX_PLATFORM_LINUX
#elif defined(_WIN32)
	#define ARX_PLATFORM ARX_PLATFORM_WIN32
#endif

#ifndef ARX_PLATFORM
	#warning "Unknown target platform"
	#define ARX_PLATFORM ARX_PLATFORM_UNKNOWN
#endif

/* ---------------------------------------------------------
                          Compilers
------------------------------------------------------------*/
#define ARX_COMPILER_UNKNOWN 0
#define ARX_COMPILER_VC9     1
#define ARX_COMPILER_VC10    2
#define ARX_COMPILER_GCC     3
#define ARX_COMPILER_CLANG   4
#define ARX_COMPILER_MINGW   4

#if defined(__clang__)
	#define ARX_COMPILER ARX_COMPILER_CLANG
#elif defined(__MINGW32__)
	#define ARX_COMPILER ARX_COMPILER_MINGW
#elif defined(__GNUC__)
	#define ARX_COMPILER ARX_COMPILER_GCC
#elif defined(_MSC_VER)
	#if _MSC_VER < 1600
		#define ARX_COMPILER ARX_COMPILER_VC9
	#elif _MSC_VER < 1700
		#define ARX_COMPILER ARX_COMPILER_VC10
	#endif
#endif

#ifndef ARX_COMPILER
	#warning "Unknown compiler"
	#define ARX_COMPILER		ARX_COMPILER_UNKNOWN
#endif

#define ARX_COMPILER_MSVC ((ARX_COMPILER == ARX_COMPILER_VC9) || (ARX_COMPILER == ARX_COMPILER_VC10))

// TODO: Move this in a platform specific file
#if ARX_COMPILER_MSVC
	#include <direct.h>
	// Windows like to be non-standard... sigh
	inline int strcasecmp(const char* str1, const char* str2) { return _stricmp(str1, str2); }
	inline int strncasecmp(const char* str1, const char* str2, size_t maxCount) { return _strnicmp(str1, str2, maxCount); }
	inline int chdir(const char* path) { return _chdir(path); }
#endif

// TODO check for these in CMakeLists.txt
#if ARX_COMPILER_MSVC || ARX_COMPILER == ARX_COMPILER_MINGW
	#define PRINT_SIZE_T "%Iu"
#elif ARX_COMPILER == ARX_COMPILER_GCC || ARX_COMPILER == ARX_COMPILER_CLANG
	#define PRINT_SIZE_T "%zu"
#else
	#define PRINT_SIZE_T "%lu"
#endif

/* ---------------------------------------------------------
                           Types
------------------------------------------------------------*/

#if ARX_COMPILER_MSVC
	
	typedef signed char s8;         //  8 bits integer
	typedef unsigned char u8;       //  8 bits unsigned integer
	
	typedef signed short s16;       // 16 bits signed integer
	typedef unsigned short u16;     // 16 bits unsigned integer
	
	typedef signed long s32;        // 32 bits signed integer
	typedef unsigned long u32;      // 32 bits unsigned integer
	
	typedef signed long long s64;   // 64 bits signed integer
	typedef unsigned long long u64; // 64 bits unsigned integer
	
#else // ARX_COMPILER_MSVC
	
	#include <stdint.h>
	
	typedef int8_t s8;    //  8 bits integer
	typedef uint8_t u8;   //  8 bits unsigned integer
	
	typedef int16_t s16;  // 16 bits signed integer
	typedef uint16_t u16; // 16 bits unsigned integer
	
	typedef int32_t s32;  // 32 bits signed integer
	typedef uint32_t u32; // 32 bits unsigned integer
	
	typedef int64_t s64;  // 64 bits signed integer
	typedef uint64_t u64; // 64 bits unsigned integer
	
#endif // ARX_COMPILER_MSVC

typedef float f32; // 32 bits float
typedef double f64; // 64 bits double float


/* ---------------------------------------------------------
                          Break
------------------------------------------------------------*/

#if ARX_COMPILER_MSVC
	#define ARX_DEBUG_BREAK() __debugbreak()
#elif ARX_COMPILER == ARX_COMPILER_GCC || ARX_COMPILER == ARX_COMPILER_CLANG || ARX_COMPILER == ARX_COMPILER_MINGW
	#define ARX_DEBUG_BREAK() __builtin_trap()
#else
	// TODO we should check for existence of these functions in CMakeLists.txt and provide a fallback (divide by zero...)
	#define ARX_DEBUG_BREAK() ((void)0)
#endif


/* ---------------------------------------------------------
                     Maccro for assertion
------------------------------------------------------------*/

void assertionFailed(const char * _sExpression, const char * _sFile, unsigned _iLine, const char * _sMessage = NULL);

#ifdef _DEBUG
	#define arx_assert_msg(_Expression, _Message) (void) ((_Expression) ||  (assertionFailed((#_Expression), (__FILE__), __LINE__, _Message),  ARX_DEBUG_BREAK(), 0))
#else // _DEBUG
	#if ARX_COMPILER_MSVC  // MS compilers support noop which discards everything inside the parens
		#define arx_assert_msg(_Expression, _Message) __noop
	#else
		#define arx_assert_msg(_Expression, _Message) ((void)0)
	#endif
#endif // _DEBUG
#define arx_assert(_Expression) arx_assert_msg(_Expression, NULL)

/* ---------------------------------------------------------
                            Define
------------------------------------------------------------*/

// Remove asserts about unused but necessary variable (unused params, variables only used for asserts...)
#define ARX_UNUSED(var)		((void)&var)

#define ARX_CHECK_NOT_NEG( _x )  (arx_assert((_x) >= 0))

#define ARX_CAST_LONG( _x )	( static_cast<long>( _x ) )
#define ARX_CAST_ULONG( _x )( static_cast<unsigned long>( _x ) )
#define ARX_CAST_UINT( _x ) ( static_cast<unsigned int>( _x ) )
#define ARX_CAST_USHORT( _x ) ( static_cast<unsigned short>( _x ) )

#define ARX_OPAQUE_WHITE 0xFFFFFFFF

// TODO use arx_assert directly
#define ARX_CHECK_NO_ENTRY( ) (arx_assert(false))
#define ARX_CHECK( _expr ) (arx_assert( _expr ))

/* ---------------------------------------------------------
                           Pragma
------------------------------------------------------------*/

#define ARX_DEAD_CODE() {arx_assert( false );}

#define ARX_CHECK_UCHAR(_x)	{arx_assert( static_cast<u16>(_x) <= UCHAR_MAX	&& _x >= 0 ) ;}
#define ARX_CHECK_BYTE(_x) ARX_CHECK_UCHAR(_x)
#define ARX_CHECK_CHAR(_x)	{arx_assert( static_cast<s16>(_x) <= CHAR_MAX	&& static_cast<s16>(_x) >= CHAR_MIN ) ;}
#define ARX_CHECK_SHORT(_x)	{arx_assert( static_cast<s32>(_x) <= SHRT_MAX	&& static_cast<s32>(_x) >= SHRT_MIN ) ;}
#define ARX_CHECK_USHORT(_x){arx_assert( static_cast<u32>(_x) <= USHRT_MAX	&& _x >= 0 ) ;}
#define ARX_CHECK_WORD(_x)	ARX_CHECK_USHORT(_x)
#define ARX_CHECK_INT(_x)	{arx_assert( static_cast<s64>(_x) <= INT_MAX	&& static_cast<s64>(_x) >= INT_MIN ) ;}
#define ARX_CHECK_UINT(_x)  {arx_assert( static_cast<u64>(_x) <= UINT_MAX	&& _x >= 0 ) ;}
#define ARX_CHECK_SIZET(_x) ARX_CHECK_UINT(_x)
#define ARX_CHECK_LONG(_x)	{arx_assert( static_cast<s64>(_x) <= LONG_MAX	&& static_cast<s64>(_x) >= LONG_MIN ) ;}
#define ARX_CHECK_ULONG(_x) {arx_assert( static_cast<u64>(_x) <= ULONG_MAX	&& _x >= 0	);}
#define ARX_CHECK_DWORD(_x) ARX_CHECK_ULONG(_x)

// TODO remove
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

/* ---------------------------------------------------------
                      String utilities
------------------------------------------------------------*/

// TODO move into a separate header

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

#endif // ARX_PLATFORM_PLATFORM_H
