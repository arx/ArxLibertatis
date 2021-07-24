/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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

#include "platform/Platform.h"

#include <stddef.h>
#include <cstring>
#include <string>
#include <string_view>

namespace util {

//! Avoid using this function directly, use the templated versions!
std::string loadString(const char * data, size_t maxLength);

/*!
 * Load an std::string from a const char * that may not be null-terminated.
 */
template <size_t N>
std::string loadString(const char (&data)[N]) {
	return loadString(data, N);
}

//! Avoid using this function directly, use the templated versions!
void storeString(char * dst, size_t maxLength, const std::string & src);

//! Copy an std::string into a char array, the array may not be null terminated
template <size_t N>
void storeString(char (&dst)[N], const std::string & src) {
	storeString(dst, N, src);
}

//! Copy an std::string into a char array, the array will be null terminated
template <size_t N>
void storeStringTerminated(char (&dst)[N], const std::string & src) {
	storeString(dst, src);
	dst[N - 1] = 0x00;
}

//! Convert a character to lowercase (ASCII-only)
[[nodiscard]] inline char toLowercase(char character) {
	return u8(u8(character) - (u8(character) >= u8('A') && u8(character) <= u8('Z')) * (u8('A') - u8('a')));
}

//! Convert a string to lowercase in-place (ASCII-only)
void makeLowercase(std::string & string);

//! Convert a string to lowercase (ASCII-only)
[[nodiscard]] inline std::string toLowercase(std::string string) {
	makeLowercase(string);
	return string;
}

//! Convert a string to lowercase (ASCII-only)
[[nodiscard]] inline std::string toLowercase(std::string_view string) {
	return toLowercase(std::string(string));
}

//! Convert a string to lowercase (ASCII-only)
[[nodiscard]] inline std::string toLowercase(const char * string) {
	return toLowercase(std::string(string));
}

/*!
 * Escape a string containing the specified characters to escape
 * \param text The string to escape
 * \param escapeChars String containing the characters you wish to escape
 * \return The escaped string
 */
std::string escapeString(std::string text, std::string_view escapeChars = "\\\" '$!");

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
	
	return nullptr;
}

template <class T, class CTYPE, class STYPE>
inline bool safeGet(T & data, CTYPE * & pos, STYPE & size) {
	
	if(size < sizeof(T)) {
		return false;
	}
	
	std::memcpy(&data, pos, sizeof(T));
	
	pos += sizeof(T);
	size -= sizeof(T);
	return true;
}

std::string getDateTimeString();

} // namespace util

#endif // ARX_UTIL_STRING_H
