/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_PLATFORM_PLATFORM_H
#define ARX_PLATFORM_PLATFORM_H

#include <stddef.h>

#include "platform/PlatformConfig.h"

#define ARX_STR_HELPER(x) # x
#define ARX_STR(x) ARX_STR_HELPER(x)

/* ---------------------------------------------------------
                          Platforms
------------------------------------------------------------*/

#define ARX_PLATFORM_UNKNOWN 0
#define ARX_PLATFORM_WIN32   1
#define ARX_PLATFORM_LINUX   2
#define ARX_PLATFORM_MACOSX  3
#define ARX_PLATFORM_BSD     100 // Generic BSD system
#define ARX_PLATFORM_UNIX    101 // Generic UNIX system

#if defined(__linux)
	#define ARX_PLATFORM ARX_PLATFORM_LINUX
#elif defined(_WIN32)
	#define ARX_PLATFORM ARX_PLATFORM_WIN32
#elif defined(__MACH__)
	#define ARX_PLATFORM ARX_PLATFORM_MACOSX
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) \
      || defined(__bsdi__) || defined(__DragonFly__)
	#define ARX_PLATFORM ARX_PLATFORM_BSD
#elif defined(__unix__) || defined(__unix) || defined(unix)
	#define ARX_PLATFORM ARX_PLATFORM_UNIX
#else
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
#define ARX_COMPILER_MINGW   5

#if defined(__clang__)
	#define ARX_COMPILER                ARX_COMPILER_CLANG
	#define ARX_COMPILER_NAME           "Clang"
	#define ARX_COMPILER_VERSION        (__clang_major__ * 10000 + __clang_minor__ * 100 \
	                                     + __clang_patchlevel__)
	#define ARX_COMPILER_VERNAME        ARX_COMPILER_NAME " " __clang_version__
#elif defined(__MINGW32__)
	#define ARX_COMPILER                ARX_COMPILER_MINGW
	#define ARX_COMPILER_NAME           "MinGW32"
	#define ARX_COMPILER_VERSION        (__MINGW32_MAJOR_VERSION * 10000 + __MINGW32_MINOR_VERSION * 100)
	#define ARX_COMPILER_VERNAME        ARX_COMPILER_NAME " " \
	                                    ARX_STR(__MINGW32_MAJOR_VERSION) \
	                                    "." ARX_STR(__MINGW32_MINOR_VERSION)
#elif defined(__GNUC__)
	#define ARX_COMPILER                ARX_COMPILER_GCC
	#define ARX_COMPILER_NAME           "GCC"
	#define ARX_COMPILER_VERSION        (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
	#define ARX_COMPILER_VERNAME        ARX_COMPILER_NAME " " __VERSION__
#elif defined(_MSC_VER)
	#if _MSC_VER < 1600
		#define ARX_COMPILER              ARX_COMPILER_VC9
		#define ARX_COMPILER_NAME         "VC9"
	#elif _MSC_VER < 1700
		#define ARX_COMPILER              ARX_COMPILER_VC10
		#define ARX_COMPILER_NAME         "VC10"
	#endif
	#define ARX_COMPILER_VERSION        _MSC_VER
	#define ARX_COMPILER_VERNAME        ARX_COMPILER_NAME " " ARX_STR(ARX_COMPILER_VERSION)
#else
	#define ARX_COMPILER                ARX_COMPILER_UNKNOWN
	#define ARX_COMPILER_NAME           "Unknown"
	#define ARX_COMPILER_VERSION        0
	#define ARX_COMPILER_VERNAME        ARX_COMPILER_NAME
#endif

#define ARX_COMPILER_MSVC ((ARX_COMPILER == ARX_COMPILER_VC9) || (ARX_COMPILER == ARX_COMPILER_VC10))

#if ARX_COMPILER_MSVC
	#include <direct.h>
	#define __func__ __FUNCTION__	// MSVC doesn't know about C99 __func__
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
                Compiler-specific attributes
------------------------------------------------------------*/

#if ARX_COMPILER_MSVC  // MS compilers support noop which discards everything inside the parens
	#define ARX_DISCARD(...) __noop
#else
	#define ARX_DISCARD(...) ((void)0)
#endif

#ifdef ARX_HAVE_ATTRIBUTE_FORMAT_PRINTF

#define ARX_FORMAT_PRINTF(message_arg, param_vararg) \
	__attribute__((format(printf, message_arg, param_vararg)))

#else // ARX_HAVE_ATTRIBUTE_FORMAT_PRINTF

#define ARX_FORMAT_PRINTF(message_arg, param_vararg)

#endif // ARX_HAVE_ATTRIBUTE_FORMAT_PRINTF

/* ---------------------------------------------------------
                     Macro for assertion
------------------------------------------------------------*/

void assertionFailed(const char * expression, const char * file, unsigned line,
                     const char * message = NULL, ...) ARX_FORMAT_PRINTF(4, 5);

#ifdef ARX_DEBUG
	#define arx_assert_impl(Expression, File, Line, ...) { \
			if(!(Expression)) { \
				assertionFailed(#Expression, File, Line, ##__VA_ARGS__); \
				ARX_DEBUG_BREAK(); \
			} \
		}
#else // ARX_DEBUG
	#define arx_assert_impl(Expression, File, Line, ...) \
		ARX_DISCARD(Expression, File, Line, Message, ##__VA_ARGS__)
#endif // ARX_DEBUG

#define arx_assert_msg(Expression, Message, ...) \
	arx_assert_impl(Expression, (__FILE__), __LINE__, Message, ##__VA_ARGS__)
#define arx_assert(Expression) \
	arx_assert_impl(Expression, (__FILE__), __LINE__)

#define arx_error_msg(Message, ...) arx_assert_msg(false, Message, ##__VA_ARGS__)
#define arx_error() arx_assert(false)

/* ---------------------------------------------------------
                            Define
------------------------------------------------------------*/

// Get the number of items in a static array
#define ARRAY_SIZE(a) ((sizeof(a) / sizeof(*(a))) / static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

/* ---------------------------------------------------------
                           Pragma
------------------------------------------------------------*/

#define ARX_DEAD_CODE() arx_assert(false)

// Remove warnings about unused but necessary variable (unused params, variables only used for asserts...)
#define ARX_UNUSED(x) ((void)&x)

/* ---------------------------------------------------------
                      String utilities
------------------------------------------------------------*/

// TODO move into a separate header

template <class CTYPE, class STYPE>
inline CTYPE * safeGetString(CTYPE * & pos, STYPE & size) {	
	
	CTYPE * begin = pos;
	
	for(size_t i = 0; i < size; i++) {
		if(pos[i] == 0) {
			size -= i + 1;
			pos += i + 1;
			return begin;
		}
	}
	
	return NULL;
}

template <class T, class CTYPE, class STYPE>
inline bool safeGet(T & data, CTYPE * & pos, STYPE & size) {
	
	if(size < sizeof(T)) {
		return false;
	}
	data = *reinterpret_cast<const T *>(pos);
	pos += sizeof(T);
	size -= sizeof(T);
	return true;
}

#endif // ARX_PLATFORM_PLATFORM_H
