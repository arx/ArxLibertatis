/*
 * Copyright 2011-2022 Arx Libertatis Team (see the AUTHORS file)
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
#include <cstring>

#include "platform/PlatformConfig.h"

#include <boost/preprocessor/cat.hpp>

/* ---------------------------------------------------------
                          Platforms
------------------------------------------------------------*/

#define ARX_PLATFORM_UNKNOWN 0
#define ARX_PLATFORM_WIN32   1
#define ARX_PLATFORM_LINUX   2
#define ARX_PLATFORM_MACOS   3
#define ARX_PLATFORM_HAIKU   4
#define ARX_PLATFORM_BSD     100 // Generic BSD system
#define ARX_PLATFORM_UNIX    101 // Generic UNIX system

#if defined(__linux)
	#define ARX_PLATFORM ARX_PLATFORM_LINUX
#elif defined(_WIN32)
	#define ARX_PLATFORM ARX_PLATFORM_WIN32
#elif defined(__MACH__)
	#define ARX_PLATFORM ARX_PLATFORM_MACOS
#elif defined(__HAIKU__)
	#define ARX_PLATFORM ARX_PLATFORM_HAIKU
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
 * \def arx_format_printf(message_arg, param_vararg)
 * \brief Declare that a function argument is a printf-like format string
 *
 * Usage: T function(args, message, ...) arx_format_printf(message_arg, param_vararg)
 *
 * \param message_arg index of the format string arg (1 for the first)
 * \param param_vararg index of the vararg for the parameters
 *
 * This is useful to
 *  a) Let the compiler check the format string and parameters when calling the function
 *  b) Prevent warnings due to a non-literal format string in the implementation
 */
#if ARX_HAVE_ATTRIBUTE_FORMAT_PRINTF
	#define arx_format_printf(message_arg, param_vararg) \
		__attribute__((format(printf, message_arg, param_vararg)))
#else
	#define arx_format_printf(message_arg, param_vararg)
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
 * constant. For other uses, prefer \ref std::size()!
 * TODO add ARX_ prefix
 */
#define ARRAY_SIZE(a) \
	((sizeof(a) / sizeof(*(a))) / static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

/*!
 * \def arx_force_inline
 * \brief Tell the compiler to always inline a function.
 */
#if ARX_COMPILER_MSVC
	#define arx_force_inline __forceinline
#elif ARX_HAVE_ATTRIBUTE_ALWAYS_INLINE
	#define arx_force_inline __attribute__((always_inline)) inline
#else
	#define arx_force_inline inline
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
	arx_format_printf(4, 5)
	void assertionFailed(const char * expression, const char * file, unsigned line, const char * message, ...);
#define arx_assert_impl(Expression, ExpressionString, ...) \
	(arx_likely(Expression) ? (void)0 : (assertionFailed(ExpressionString, ARX_FILE, __LINE__, __VA_ARGS__), arx_trap()))
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
#define arx_assert(Expression)          arx_assert_impl(Expression, #Expression, nullptr)

/*!
 * \def arx_assert_msg(Expression, Message, MessageArguments...)
 * \brief Abort and print a message if \a Expression evaluates to false
 *
 * You must provide a failure message in printf-like syntax and arguments for it as
 * as additional arguments after the expression.
 *
 * Does nothing in release builds.
 */
#define arx_assert_msg(Expression, ...) arx_assert_impl(Expression, #Expression, __VA_ARGS__)

/* ---------------------------------------------------------
                  Assumptions (Optimizer Hints)
------------------------------------------------------------*/

/*!
 * \def arx_expect(Expression, Value)
 * \brief Indicate the most likely value of an expression
 */
#if ARX_HAVE_BUILTIN_EXPECT
	#define arx_expect(Expression, ExpectedValue) __builtin_expect((Expression), (ExpectedValue))
#else
	#define arx_expect(Expression, ExpectedValue) (ARX_UNUSED(ExpectedValue), (Expression))
#endif

/*!
 * \def arx_likely(Condition)
 * \brief Indicate that a condition is likely true
 */
#define arx_likely(Condition) arx_expect(bool(Condition), 1)

/*!
 * \def arx_unlikely(Condition)
 * \brief Indicate that a condition is likely false
 */
#define arx_unlikely(Condition) arx_expect(bool(Condition), 0)

/*!
 * \def arx_assume(Expression)
 * \brief Assume that an expression is true for optimization purposes.
 *
 * In debug builds, assumptions are checked using \ref arx_assert().
 *
 * Unlike arx_assert(Expression) this macro also tells the compiler to assume that Expression is always true
 * in release builds. Expression should be simple because it may otherwise end up being executed in release
 * builds if the compiler cannot prove it does not have any side effects.
 *
 * \note Clang provides __builtin_assume() which might seem like it would be a good fit here but unfortnately
 * only works if all functions called in the expression have been manually marked with [[gnu::pure]] or
 * [[gnu::const]] - constexpr and inline definition is not enough.
 */
#ifdef ARX_DEBUG
	#define arx_assume(Expression) arx_assert(Expression)
#elif ARX_COMPILER_MSVC
	#define arx_assume(Expression) __assume(Expression)
#elif ARX_HAVE_BUILTIN_UNREACHABLE
	#define arx_assume(Expression) ((Expression) ? (void)0 : __builtin_unreachable())
#else
	#define arx_assume(Expression) ARX_DISCARD(Expression)
#endif

/*!
 * \def arx_unreachable()
 * \brief Assume that a code branch cannot be reached.
 *
 * This is similar to arx_assume(false) but falls back to a while loop instead of a no-op when we don't know
 * of a way to tell the compiler about assumptions.
 *
 * Unlike arx_assert(false) this macro also tells the compiler to assume that the branch is unreachable
 * in release builds. Therefore there should never be any code after uses of this macro, including
 * return statements, as whether this code is executed would be undefined.
 *
 * This macro can be used to avoid dummy values when switching over enums, ie if only A and B are possible:
 * \code
 * const char * enumToString(Enum value) {
 *   switch(value) {
 *     case A: return "A";
 *     case B: return "B";
 *     // default: return "invalid value";  -- this is not needed with arx_unreachable()
 *   }
 *   arx_unreachable();
 * }
 * \endcode
 *
 * If a switch is supposed to handle all values of an enum it is important to not add a default label
 * for the arx_unreachable() macro but instead place it after the switch. This allows tools to warn when
 * there is a (new) enum value that is not handled in the switch.
 *
 * While some compilers may use unreachable branches to make assumptions about variables, this should
 * not be relied upon. To teach the optimizer a fact, use
 * \code
 * arx_assume(expression)
 * \endcode
 */
#ifdef ARX_DEBUG
	#define arx_unreachable() arx_assert_impl(false, "unreachable code", nullptr)
#elif ARX_HAVE_BUILTIN_UNREACHABLE
	#define arx_unreachable() __builtin_unreachable()
#elif ARX_COMPILER_MSVC
	#define arx_unreachable() __assume(0)
#else
	#define arx_unreachable() do { } while(true)
#endif

/*!
 * Helper to cast a void * that is really a function pointer back to a function pointer
 */
class FunctionPointer {
	void * m_func;
public:
	explicit FunctionPointer(void * func) : m_func(func) { }
	template <typename FunctionType>
	operator FunctionType() {
		return reinterpret_cast<FunctionType>(reinterpret_cast<void(*)()>(m_func));
	}
};

/*!
 * \def arx_noexcept_default
 *
 * Use the default implementation for a constructor but explicitly mark it as noexcept.
 */
#if ARX_HAVE_CXX20_NOEXCEPT_DEFAULT
	#define arx_noexcept_default noexcept = default;
#else
	#define arx_noexcept_default noexcept { }
#endif

#endif // ARX_PLATFORM_PLATFORM_H
