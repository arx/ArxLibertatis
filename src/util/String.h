/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_UTIL_STRING_H
#define ARX_UTIL_STRING_H

#include <stddef.h>
#include <string>

namespace util {

/*!
 * Load an std::string from a const char * that may not be null-terminated.
 */
std::string loadString(const char * data, size_t maxLength);

template <size_t N>
std::string loadString(const char (&data)[N]) {
	return loadString(data, N);
}

/*!
 * Escape a string containing the specified characters to escape
 * @param text The string to escape
 * @param escapeChars String containing the characters you wish to escape
 * @return The escaped string
 */
std::string escapeString(const std::string & text, const char * escapeChars);

//! Remove surrounding quotes and replace escape code with their values.
std::string unescapeString(const std::string & text);

} // namespace util

#endif // ARX_UTIL_STRING_H
