/*
 * Copyright 2011-2016 Arx Libertatis Team (see the AUTHORS file)
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
#include <cstdlib>

#include "platform/PlatformConfig.h"

#include <boost/preprocessor/cat.hpp>

/* ---------------------------------------------------------
                          Platforms
------------------------------------------------------------*/

#define ARX_PLATFORM_UNKNOWN 0
#define ARX_PLATFORM_WIN32   1
#define ARX_PLATFORM_LINUX   2
#define ARX_PLATFORM_MACOS   3
#define ARX_PLATFORM_BSD     100 // Generic BSD system
#define ARX_PLATFORM_UNIX    101 // Generic UNIX system

#if defined(__linux)
	#define ARX_PLATFORM ARX_PLATFORM_LINUX
#elif defined(_WIN32)
	#define ARX_PLATFORM ARX_PLATFORM_WIN32
#elif defined(__MACH__)
	#define ARX_PLATFORM ARX_PLATFORM_MACOS
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

// This is used in many places, keep it for now
#if defined(_MSC_VER)
	#define ARX_COMPILER_MSVC 1
#else
	#define ARX_COMPILER_MSVC 0
#endif

#if ARX_COMPILER_MSVC
	#define __func__ __FUNCTION__ // MSVC doesn't know about C99 __func__
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
	
#else
	
	#include <stdint.h>
	
	typedef int8_t s8;    //  8 bits integer
	typedef uint8_t u8;   //  8 bits unsigned integer
	
	typedef int16_t s16;  // 16 bits signed integer
	typedef uint16_t u16; // 16 bits unsigned integer
	
	typedef int32_t s32;  // 32 bits signed integer
	typedef uint32_t u32; // 32 bits unsigned integer
	
	typedef int64_t s64;  // 64 bits signed integer
	typedef uint64_t u64; // 64 bits unsigned integer
	
#endif

typedef float f32; // 32 bits float
typedef double f64; // 64 bits double float


/* ---------------------------------------------------------
                          Break
------------------------------------------------------------*/

/*!
 * \def arx_trap()
 * \brief Halt execution and notify any attached debugger
 */
#if ARX_COMPILER_MSVC
	#define arx_trap() (__debugbreak(), std::abort())
#elif ARX_HAVE_BUILTIN_TRAP
	#define arx_trap() __builtin_trap()
#else
	#define arx_trap() std::abort()
#endif

/* ---------------------------------------------------------
                Compiler-specific attributes
------------------------------------------------------------*/

/*!
 * \def ARX_FORMAT_PRINTF(message_arg, param_vararg)
 * \brief Declare that a function argument is a printf-like format string
 *
 * Usage: T function(args, message, ...) ARX_FORMAT_PRINTF(message_arg, param_vararg)
 *
 * \param message_arg index of the format string arg (1 for the first)
 * \param param_vararg index of the vararg for the parameters
 *
 * This is useful to
 *  a) Let the compiler check the format string and parameters when calling the function
 *  b) Prevent warnings due to a non-literal format string in the implementation
 */
#if ARX_HAVE_ATTRIBUTE_FORMAT_PRINTF
	#define ARX_FORMAT_PRINTF(message_arg, param_vararg) \
		__attribute__((format(printf, message_arg, param_vararg)))
#else
	#define ARX_FORMAT_PRINTF(message_arg, param_vararg)
#endif

/* ---------------------------------------------------------
                Helper macros
------------------------------------------------------------*/

/*!
 * \def ARX_DISCARD(...)
 * \brief Discard parameters from a macro
 */
#if ARX_COMPILER_MSVC
	// MS compilers support noop which discards everything inside the parens
	#define ARX_DISCARD(...) __noop
#else
	#define ARX_DISCARD(...) ((void)0)
#endif

/*!
 * \def ARX_FILE
 * \brief Path to the current source file
 * In release builds this will be defined to the relative path to the current
 * translation unit to avoid compiling the full source path into release executables.
 */
#if !defined(ARX_FILE)
	#define ARX_FILE __FILE__
#endif

/*!
 * \def ARX_STR(x)
 * \brief Turn argument into a string constant
 */
#define ARX_STR_HELPER(x) # x
#define ARX_STR(x) ARX_STR_HELPER(x)

/*!
 * \def ARX_UNUSED(x)
 * \brief Remove warnings about unused but necessary variable
 *
 * (unused params, variables only used for asserts...)
 */
#define ARX_UNUSED(x) ((void)(x))

/*!
 * \def ARX_ANONYMOUS_SYMBOL(Name)
 * \brief Make a symbol name specific to the current "translation unit"
 * The constructed symbol will still be unique for each source file in unity builds.
 * This should be used for anonymous namespaces or static variables / functions.
 */
#if defined(ARX_TRANSLATION_UNIT)
	#define ARX_ANONYMOUS_SYMBOL(Name) \
		BOOST_PP_CAT(BOOST_PP_CAT(Name, _), ARX_TRANSLATION_UNIT)
#else
	#define ARX_ANONYMOUS_SYMBOL(Name) Name
#endif

/*!
 * \def ARX_UNIQUE_SYMBOL(Name)
 * \brief Make a symbol unique to the current translation unit.
 * The constructed symbol will still be unique for each source file in unity builds.
 * May only be used once on each line with the same \a Name.
 */
#define ARX_UNIQUE_SYMBOL(Name) \
	BOOST_PP_CAT(BOOST_PP_CAT(ARX_ANONYMOUS_SYMBOL(Name), _), __LINE__)

/*!
 * \def ARX_ANONYMOUS_NAMESPACE
 * \brief Name for an "anonymous namespace"
 * This ensures the namespace is still unique for each source file in unity builds.
 * Usage: \code
namespace ARX_ANONYMOUS_NAMESPACE {
	…
} ARX_END_ANONYMOUS_NAMESPACE \endcode
 * This is required to avoid name collision and because GCC comlains if a type from
 * an anonymous namespace is used in a file that isn't the main source file
 */
#if defined(ARX_TRANSLATION_UNIT)
	#define ARX_ANONYMOUS_NAMESPACE ARX_ANONYMOUS_SYMBOL(arx_tu)
	#define ARX_ANONYMOUS_NAMESPACE_DECL namespace ARX_ANONYMOUS_SYMBOL(arx_tu)
	#define ARX_END_ANONYMOUS_NAMESPACE using ARX_ANONYMOUS_NAMESPACE_DECL;
#else
	#define ARX_ANONYMOUS_NAMESPACE
	#define ARX_END_ANONYMOUS_NAMESPACE
#endif

/*!
 * \def ARRAY_SIZE(a)
 * \brief Get the number of items in a static array
 * This should only be used if the array size needs to be known as a compile-time
 * constant. For other uses, prefer \ref boost::size()!
 * TODO add ARX_ prefix
 */
#define ARRAY_SIZE(a) \
	((sizeof(a) / sizeof(*(a))) / static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

/*!
 * \def ARX_NOEXCEPT
 * \brief Declare that a function never throws exceptions.
 */
#if ARX_HAVE_CXX11_NOEXCEPT
	#define ARX_NOEXCEPT noexcept
#else
	#define ARX_NOEXCEPT throw()
#endif

/*!
 * \def ARX_STATIC_ASSERT
 * \brief Declare that a function never throws exceptions.
 */
#if ARX_HAVE_CXX11_STATIC_ASSERT
	#define ARX_STATIC_ASSERT(Condition, Message) static_assert(Condition, Message)
#else
	#include <boost/static_assert.hpp>
	#define ARX_STATIC_ASSERT(Condition, Message) BOOST_STATIC_ASSERT_MSG(Condition, Message)
#endif

/* ---------------------------------------------------------
                          Assertions
------------------------------------------------------------*/

#ifdef ARX_DEBUG
	/*!
	 * \brief Log that an assertion has failed
	 *
	 * This is a low-level implementation, use arx_assert() instead!
	 */
	void assertionFailed(const char * expression, const char * file, unsigned line,
	                     const char * message, ...) ARX_FORMAT_PRINTF(4, 5);
#define arx_assert_impl(Expression, ExpressionString, ...) \
	((Expression) ? (void)0 : (assertionFailed(ExpressionString, ARX_FILE, __LINE__, __VA_ARGS__), arx_trap()))
#else // ARX_DEBUG
	#define arx_assert_impl(Expression, ExpressionString, ...) \
		ARX_DISCARD(Expression, ExpressionString, __VA_ARGS__)
#endif // ARX_DEBUG

/*!
 * \def arx_assert(Expression)
 * \brief Abort if \a Expression evaluates to false
 *
 * Does nothing in release builds.
 */
#define arx_assert(Expression)          arx_assert_impl(Expression, #Expression, NULL)

/*!
 * \def arx_assert_msg(Expression, Message, MessageArguments...)
 * \brief Abort and print a message if \a Expression evaluates to false
 * You must provide a failure message in printf-like syntax and arguments for it as
 * as additional arguments after the expression.
 * Does nothing in release builds.
 */
#define arx_assert_msg(Expression, ...) arx_assert_impl(Expression, #Expression, __VA_ARGS__)

/*!
 * \def ARX_DEAD_CODE()
 * \brief Assert that a code branch cannot be reached.
 *
 * Unlike arx_assert(false) this macro also tells the compiler to assume that the branch is unreachable
 * in release builds. Therefore there should never be any code after uses of this macro, including
 * return statements, as wether this code is executed would be undefined.
 *
 * This macro can be used to avoid dummy values when switching over enums, ie if only A and B are possible:
 * \code
 * const char * enumToString(Enum value) {
 *   switch(value) {
 *     case A: return "A";
 *     case B: return "B";
 *     // default: return "invalid value";  -- this is not needed with ARX_DEAD_CODE()
 *   }
 *   ARX_DEAD_CODE();
 * }
 * \endcode
 *
 * If a switch is supposed to handle all values of an enum it is important to not add a default label
 * for the ARX_DEAD_CODE() macro but instead place it after the switch. This allows tools to warn when
 * ther is an enum value added that is not handled in the switch.
 */
#ifdef ARX_DEBUG
#define ARX_DEAD_CODE() arx_assert_impl(false, "unreachable code", NULL)
#elif ARX_COMPILER_MSVC
#define ARX_DEAD_CODE() __assume(0)
#elif ARX_HAVE_BUILTIN_UNREACHABLE
#define ARX_DEAD_CODE() __builtin_unreachable()
#else
#define ARX_DEAD_CODE() do { } while(true)
#endif


#endif // ARX_PLATFORM_PLATFORM_H
