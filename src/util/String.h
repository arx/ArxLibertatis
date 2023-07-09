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

#ifndef ARX_UTIL_STRING_H
#define ARX_UTIL_STRING_H

#include "platform/Platform.h"

#include <stddef.h>
#include <cstring>
#include <algorithm>
#include <string>
#include <string_view>

namespace util {

//! Avoid using this function directly, use the templated versions!
[[nodiscard]] std::string_view loadString(const char * data, size_t maxLength);

/*!
 * Load an std::string from a const char * that may not be null-terminated.
 */
template <size_t N>
[[nodiscard]] std::string_view loadString(const char (&data)[N]) {
	return loadString(data, N);
}

//! Avoid using this function directly, use the templated versions!
void storeString(char * dst, size_t maxLength, std::string_view src);

//! Copy an std::string into a char array, the array may not be null terminated
template <size_t N>
void storeString(char (&dst)[N], std::string_view src) {
	storeString(dst, N, src);
}

//! Copy an std::string into a char array, the array will be null terminated
template <size_t N>
void storeStringTerminated(char (&dst)[N], std::string_view src) {
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

constexpr const std::string_view Whitespace = " \t\r\n";

[[nodiscard]] inline std::string_view trimLeft(std::string_view string, std::string_view totrim = Whitespace) {
	std::string_view::size_type pos = string.find_first_not_of(totrim);
	return string.substr((pos == std::string_view::npos) ? string.size() : pos);
}

[[nodiscard]] inline std::string_view trimRight(std::string_view string, std::string_view totrim = Whitespace) {
	std::string_view::size_type pos = string.find_last_not_of(totrim);
	return string.substr(0, (pos == std::string_view::npos) ? 0 : pos + 1);
}

[[nodiscard]] inline std::string_view trim(std::string_view string, std::string_view totrim = Whitespace) {
	return trimLeft(trimRight(string, totrim), totrim);
}

/*!
 * Escape a string containing the specified characters to escape
 * \param text The string to escape
 * \param escapeChars String containing the characters you wish to escape
 * \return The escaped string
 */
[[nodiscard]] std::string escapeString(std::string text, std::string_view escapeChars = "\\\" '$!");

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

[[nodiscard]] std::string getDateTimeString();

struct SplitStringSentinel { };

template <typename Separator, bool SkipEmpty>
class SplitStringIterator {
	
	std::string_view m_string;
	Separator m_separator;
	std::string_view::size_type m_pos;
	
public:
	
	SplitStringIterator(std::string_view string, Separator separator)
		: m_string(string), m_separator(separator), m_pos(std::string_view::size_type(-1))
	{
		operator++();
	}
	
	void operator++() {
		if(m_pos == m_string.size()) {
			m_pos = std::string_view::npos;
			return;
		}
		do {
			m_string.remove_prefix(m_pos + 1);
			m_pos = m_string.find_first_of(m_separator);
			if(m_pos == std::string_view::npos) {
				if(!SkipEmpty || !m_string.empty()) {
					m_pos = m_string.size();
				}
				break;
			}
		} while(SkipEmpty && m_pos == 0);
	}
	
	[[nodiscard]] std::string_view operator*() const {
		return m_string.substr(0, m_pos);
	}
	
	[[nodiscard]] bool operator!=(SplitStringSentinel /* ignored */) const {
		return m_pos != std::string_view::npos;
	}
	
};

template <typename Separator, bool SkipEmpty>
class SplitStringView {
	
	std::string_view m_string;
	Separator m_separator;
	
public:
	
	SplitStringView(std::string_view string, Separator separator)
		: m_string(string), m_separator(separator)
	{ }
	
	[[nodiscard]] SplitStringIterator<Separator, SkipEmpty> begin() const {
		return { m_string, m_separator };
	}
	
	[[nodiscard]] SplitStringSentinel end() const {
		return { };
	}
	
};

/*!
 * \brief Split a string at each occurrence of a character.
 *
 * Returns an iterable view of all substrings between occurrences of the separator even if they are empty.
 */
[[nodiscard]] inline SplitStringView<char, false> split(std::string_view string, char separator) {
    return { string, separator };
}

/*!
 * \brief Split a string at each occurrence of any of the characters in the separator string.
 *
 * Returns an iterable view of all substrings between occurrences of a separator even if they are empty.
 */
[[nodiscard]] inline SplitStringView<std::string_view, false> split(std::string_view string,
                                                                    std::string_view separators) {
    return { string, separators };
}

/*!
 * \brief Split a string at each occurrence of a character.
 *
 * Returns an iterable view of all non-empty substrings between occurrences of the separator
 */
[[nodiscard]] inline SplitStringView<char, true> splitIgnoreEmpty(std::string_view string, char separator) {
    return { string, separator };
}

/*!
 * \brief Split a string at each occurrence of any of the characters in the separator string.
 *
 * Returns an iterable view of all non-empty substrings between occurrences of a separator.
 */
[[nodiscard]] inline SplitStringView<std::string_view, true> splitIgnoreEmpty(std::string_view string,
                                                                              std::string_view separators) {
    return { string, separators };
}

} // namespace util

#endif // ARX_UTIL_STRING_H
