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

#include "platform/Platform.h"

#include <stdio.h>
#include <stdarg.h>
#include <cstdio>

#include "Configure.h"

#if ARX_HAVE_LIBCPP_VERBOSE_ABORT || ARX_HAVE_GLIBCXX_ASSERT_FAIL
#include <version>
#endif

#include "io/log/Logger.h"

#if ARX_PLATFORM == ARX_PLATFORM_UNKNOWN
#warning "Unknown target platform"
#endif

#ifdef ARX_DEBUG

typedef void(*AssertHandler)(const char * expr, const char * file, unsigned int line,
                             const char * msg);
AssertHandler g_assertHandler = 0;

void assertionFailed(const char * expr, const char * file, unsigned int line,
                     const char * msg, ...) {
	
	if(!file || file[0] == '\0') {
		file = "<unknown>";
	}
	
	Logger(file, line, Logger::Critical) << "Assertion Failed: " << expr;
	if(msg) {
		char formattedmsgbuf[4096];
		va_list args;
		va_start(args, msg);
		std::vsnprintf(formattedmsgbuf, sizeof(formattedmsgbuf) - 1, msg, args);
		va_end(args);
		Logger(file, line, Logger::Critical) << "Message: " << formattedmsgbuf;
		if(g_assertHandler) {
			g_assertHandler(expr, file, line, formattedmsgbuf);
		}
	} else {
		if(g_assertHandler) {
			g_assertHandler(expr, file, line, nullptr);
		}
	}
	
}

#if ARX_HAVE_LIBCPP_VERBOSE_ABORT
void std::__libcpp_verbose_abort(char const * format, ...) {
	char message[4096];
	va_list args;
	va_start(args, format);
	std::vsnprintf(message, sizeof(message) - 1, format, args);
	va_end(args);
	assertionFailed(message, "libc++", 0, nullptr);
	arx_trap();
}
#endif

#if ARX_HAVE_GLIBCXX_ASSERT_FAIL
_GLIBCXX_NORETURN void std::__glibcxx_assert_fail(const char * file, int line, const char * function,
                                                  const char * condition) _GLIBCXX_NOEXCEPT {
	assertionFailed(condition, file ? file : "libstdc++", line, function ? "in %s" : nullptr,  function);
	arx_trap();
}
#endif

#endif // ARX_DEBUG

// When building without exceptions, there's a chance the boost precompiled library are built with exception
// handling turned on... in that case we would get an undefined symbol at link time.
// In order to solve this, we define this symbol here:
#ifdef BOOST_NO_EXCEPTIONS
void boost::throw_exception(const std::exception & e) {
	#ifdef ARX_DEBUG
	assertionFailed(e.what(), "Boost", 0, nullptr);
	arx_trap();
	#else
	ARX_UNUSED(e);
	arx_unreachable();
	#endif
}
#endif // BOOST_NO_EXCEPTIONS
