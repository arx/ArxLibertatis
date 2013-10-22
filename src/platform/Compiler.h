/*
 * Copyright 2013 Arx Libertatis Team (see the AUTHORS file)
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

#if defined(__clang__)
	#define ARX_COMPILER_NAME           "Clang"
	#define ARX_COMPILER_VERNAME        ARX_COMPILER_NAME " " __clang_version__
#elif defined(__MINGW32__)
	#define ARX_COMPILER_NAME           "MinGW32"
	#define ARX_COMPILER_VERNAME        ARX_COMPILER_NAME " " \
	                                    ARX_STR(__MINGW32_MAJOR_VERSION) \
	                                    "." ARX_STR(__MINGW32_MINOR_VERSION)
#elif defined(__GNUC__)
	#define ARX_COMPILER_NAME           "GCC"
	#define ARX_COMPILER_VERNAME        ARX_COMPILER_NAME " " __VERSION__
#elif defined(_MSC_VER)
	#if _MSC_VER < 1500 || _MSC_VER >= 1900
		#define ARX_COMPILER_NAME         "MSVC"
	#elif _MSC_VER < 1600
		#define ARX_COMPILER_NAME         "VC9"
	#elif _MSC_VER < 1700
		#define ARX_COMPILER_NAME         "VC10"
	#elif _MSC_VER < 1800
		#define ARX_COMPILER_NAME         "VC11"
	#elif _MSC_VER < 1900
		#define ARX_COMPILER_NAME         "VC12"
	#endif
	#define ARX_COMPILER_VERNAME        ARX_COMPILER_NAME " " ARX_STR(_MSC_VER)
#else
	#define ARX_COMPILER_NAME           "Unknown"
	#define ARX_COMPILER_VERNAME        ARX_COMPILER_NAME
#endif

#endif // ARX_PLATFORM_COMPILER_H
