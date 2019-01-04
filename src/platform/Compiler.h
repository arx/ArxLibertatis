/*
 * Copyright 2013-2019 Arx Libertatis Team (see the AUTHORS file)
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

/*!
 * Compiler name and version identification. The macros defined here should not be used
 * to conditionally enable code, but they are useful as additional debug information to
 * include in crash reports.
 */
#ifndef ARX_PLATFORM_COMPILER_H
#define ARX_PLATFORM_COMPILER_H

#include <ostream>

#include "platform/Platform.h" // needed for ARX_STR

#if defined(__INTEL_COMPILER)
	#define ARX_COMPILER_NAME           "Intel C++"
	struct arx_icc_vername { };
	inline std::ostream & operator<<(std::ostream & os, const arx_icc_vername & /* tag */) {
		return os << ARX_COMPILER_NAME << ' ' << (__INTEL_COMPILER / 100) << '.'
		          << (__INTEL_COMPILER % 100);
	}
	#define ARX_COMPILER_VERNAME        arx_icc_vername()
#elif defined(__PATHSCALE__)
	#define ARX_COMPILER_NAME           "EKOPath"
	#define ARX_COMPILER_VERNAME        ARX_COMPILER_NAME " " __PATHSCALE__
#elif defined(__clang_version__)
	#define ARX_COMPILER_NAME           "Clang"
	#define ARX_COMPILER_VERNAME        ARX_COMPILER_NAME " " __clang_version__
#elif defined(__MINGW32__)
	#define ARX_COMPILER_NAME           "MinGW"
	#define ARX_COMPILER_VERNAME        ARX_COMPILER_NAME " " \
	                                    ARX_STR(__MINGW32_MAJOR_VERSION) \
	                                    "." ARX_STR(__MINGW32_MINOR_VERSION) \
	                                    " (GCC " __VERSION__ ")"
#elif defined(__GNUC__)
	#define ARX_COMPILER_NAME           "GCC"
	#define ARX_COMPILER_VERNAME        ARX_COMPILER_NAME " " __VERSION__
#elif defined(_MSC_VER)
	#define ARX_COMPILER_NAME           "MSVC"
	struct arx_msvc_vername { };
	inline std::ostream & operator<<(std::ostream & os, const arx_msvc_vername & /* tag */) {
		#if _MSC_VER >= 1900
		return os << ARX_COMPILER_NAME << ' ' << (_MSC_VER / 100 - 5) << '.' << (_MSC_VER % 100);
		#else
		return os << ARX_COMPILER_NAME << ' ' << (_MSC_VER / 100 - 6) << '.' << (_MSC_VER % 100);
		#endif
	}
	#define ARX_COMPILER_VERNAME        arx_msvc_vername()
#else
	#define ARX_COMPILER_NAME           "Unknown"
	#define ARX_COMPILER_VERNAME        ARX_COMPILER_NAME
#endif

#if defined(__INTEL_CXXLIB_ICC)
	#define ARX_STDLIB_NAME           "Intel C++"
	#define ARX_STDLIB_VERNAME        ARX_STDLIB_NAME
#elif defined(_LIBCPP_VERSION)
	#define ARX_STDLIB_NAME           "libc++"
	struct arx_libcxx_vername { };
	inline std::ostream & operator<<(std::ostream & os, const arx_libcxx_vername & /* tag */) {
		return os << ARX_STDLIB_NAME << ' ' << (_LIBCPP_VERSION / 1000) << '.' << (_LIBCPP_VERSION % 1000);
	}
	#define ARX_STDLIB_VERNAME        arx_libcxx_vername()
#elif defined(__GLIBCXX__)
	#define ARX_STDLIB_NAME           "libstdc++"
	#define ARX_STDLIB_VERNAME        ARX_STDLIB_NAME " " ARX_STR(__GLIBCXX__)
#elif defined(__GLIBCPP__)
	#define ARX_STDLIB_NAME           "libstdc++"
	#define ARX_STDLIB_VERNAME        ARX_STDLIB_NAME " " ARX_STR(__GLIBCPP__)
#elif defined(_CPPLIB_VER)
	#define ARX_STDLIB_NAME           "Dinkumware C++ stdlib"
	struct arx_dinkumware_vername { };
	inline std::ostream & operator<<(std::ostream & os, const arx_dinkumware_vername & /* tag */) {
		return os << ARX_STDLIB_NAME << ' ' << (_CPPLIB_VER / 100) << '.' << (_CPPLIB_VER % 100);
	}
	#define ARX_STDLIB_VERNAME        arx_dinkumware_vername()
#else
	#define ARX_STDLIB_NAME           "Unknown"
	#define ARX_STDLIB_VERNAME        ARX_STDLIB_NAME
#endif

#endif // ARX_PLATFORM_COMPILER_H
