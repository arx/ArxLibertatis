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
// Parts based on:
////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2009 Laurent Gomila (laurent.gom@gmail.com)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////
//
// This code has been taken from SFML and altered to fit the project's needs.
//
////////////////////////////////////////////////////////////

#ifndef ARX_UTIL_UNICODE_H
#define ARX_UTIL_UNICODE_H

#include <stddef.h>
#include <string>

#include "platform/Platform.h"

namespace util {

typedef u32 Unicode;
static const Unicode INVALID_CHAR = Unicode(-1);
static const Unicode REPLACEMENT_CHAR = 0xfffd; //!< Unicode replacement character
static const Unicode BYTE_ORDER_MARK = 0xfeff;


//! @return true c is is the first part of an UTF-16 surrogate pair
inline bool isUTF16HighSurrogate(u16 chr) {
	return chr >= 0xd800 && chr <= 0xdbff;
}

//! @return true c is is the first part of an UTF-16 surrogate pair
inline bool isUTF16LowSurrogate(u16 chr) {
	return chr >= 0xdc00 && chr <= 0xdfff;
}

//! @return true c is not the first byte of an UTF-8 character
inline bool isUTF8ContinuationByte(u8 chr) {
	return (chr & 0xc0) == 0x80;
}


/*!
 * Decode the first character from an UTF-16 LE string
 * @param it Start of the string. Will be advanced by the bytes read
 * @param end End of the string
 * @param replacement Replacement code point for invalid data
 * @return one Unicode code point or INVALID_CHAR if we are at the end of the range
 */
template <typename In>
Unicode readUTF16LE(In & it, In end, Unicode replacement = REPLACEMENT_CHAR) {
	
	if(it == end) {
		return INVALID_CHAR;
	}
	Unicode chr = u8(*it++);
	if(it == end) {
		return replacement;
	}
	chr |= Unicode(u8(*it++)) << 8;
	
	// If it's a surrogate pair, first convert to a single UTF-32 character
	if(isUTF16HighSurrogate(chr)) {
		if(it != end) {
			// The second element is valid : convert the two elements to a UTF-32 character
			Unicode d = u8(*it++);
			if(it == end) {
				return replacement;
			}
			d |= Unicode(u8(*it++)) << 8;
			if(isUTF16LowSurrogate(d)) {
				chr = ((chr - 0xd800) << 10) + (d - 0xdc00) + 0x0010000;
			} else {
				return replacement;
			}
		} else {
			// Invalid second element
			return replacement;
		}
	}
	
	// Replace invalid characters
	if(chr > 0x0010FFFF) {
		// Invalid character (greater than the maximum unicode value)
		return replacement;
	}
	
	return chr;
}


/*!
 * Decode the first character from an UTF-8 string
 * @param it Start of the string. Will be advanced by the bytes read
 * @param end End of the string
 * @param replacement Replacement code point for invalid data
 * @return one Unicode code point or INVALID_CHAR if we are at the end of the range
 */
template <typename In>
Unicode readUTF8(In & it, In end, Unicode replacement = REPLACEMENT_CHAR) {
	
	if(it == end) {
		return INVALID_CHAR;
	}
	Unicode chr = u8(*it++);
	
	// For multi-byte characters, read the remaining bytes
	if(chr & (1 << 7)) {
		
		if(isUTF8ContinuationByte(chr)) {
			// Bad start position
			return replacement;
		}
		
		if(it == end || !isUTF8ContinuationByte(u8(*it))) {
			// Unexpected end of multi-byte sequence
			return replacement;
		}
		chr &= 0x3f, chr <<= 6, chr |= (u8(*it++) & 0x3f);
		
		if(chr & (1 << (5 + 6))) {
			
			if(it == end || !isUTF8ContinuationByte(u8(*it))) {
				// Unexpected end of multi-byte sequence
				return replacement;
			}
			chr &= ~(1 << (5 + 6)), chr <<= 6, chr |= (u8(*it++) & 0x3f);
			
			if(chr & (1 << (4 + 6 + 6))) {
				
				if(it == end || !isUTF8ContinuationByte(u8(*it))) {
					// Unexpected end of multi-byte sequence
					return replacement;
				}
				chr &= ~(1 << (4 + 6 + 6)), chr <<= 6, chr |= (u8(*it++) & 0x3f);
				
				if(chr & (1 << (3 + 6 + 6 + 6))) {
					// Illegal UTF-8 byte
					chr = replacement;
				}
				
			}
		}
	}
	
	return chr;
}

//! Get the number of UTF-8 bytes required for a unicode character
inline size_t getUTF8Length(Unicode chr) {
	if     (chr <  0x80)       return 1;
	else if(chr <  0x800)      return 2;
	else if(chr <  0x10000)    return 3;
	else if(chr <= 0x0010FFFF) return 4;
	return 1;
}

/*!
 * Encode one character to an UTF8 string
 * @param it Output iterator
 * @return New output iterator after the written bytes
 */
template <typename Out>
Out writeUTF8(Out it, Unicode chr) {
	
	static const u8 utf8FirstBytes[7] = { 0x00, 0x00, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc };
	
	// Get number of bytes to write
	int bytesToWrite = getUTF8Length(chr);
	
	// Extract bytes to write
	u8 bytes[4];
	switch(bytesToWrite) {
		case 4 : bytes[3] = static_cast<u8>((chr | 0x80) & 0xBF); chr >>= 6;
		case 3 : bytes[2] = static_cast<u8>((chr | 0x80) & 0xBF); chr >>= 6;
		case 2 : bytes[1] = static_cast<u8>((chr | 0x80) & 0xBF); chr >>= 6;
		case 1 : bytes[0] = static_cast<u8>( chr | utf8FirstBytes[bytesToWrite]);
	}
	
	// Add them to the output
	const u8 * curByte = bytes;
	switch(bytesToWrite) {
		case 4 : *it++ = *curByte++;
		case 3 : *it++ = *curByte++;
		case 2 : *it++ = *curByte++;
		case 1 : *it++ = *curByte++;
	}
	
	return it;
}


/*!
 * Convert an UTF-16 LE string to an UTF-8 string
 * @param begin Start of the string
 * @param end End of the string
 * @param output Output iterator
 * @param replacement Replacement code point for invalid data
 * @return New output iterator after the written bytes
 */
template <typename In, typename Out>
Out convertUTF16LEToUTF8(In begin, In end, Out output,
                         Unicode replacement = REPLACEMENT_CHAR) {
	
	Unicode chr;
	for(In it = begin; (chr = readUTF16LE(it, end, replacement)) != INVALID_CHAR;) {
		if(chr == BYTE_ORDER_MARK) {
			// The BOM is useless and annoying in UTF-8, so strip it
		} else {
			output = writeUTF8(output, chr);
		}
	}
	
	return output;
}


//! Get the number of UTF-8 bytes required to represent an UTF-16 LE string
template <typename In>
size_t getUTF16LEToUTF8Length(In begin, In end,
                              Unicode replacement = REPLACEMENT_CHAR) {
	
	size_t length = 0;
	
	Unicode chr;
	for(In it = begin; (chr = readUTF16LE(it, end, replacement)) != INVALID_CHAR;) {
		if(chr == BYTE_ORDER_MARK) {
			// The BOM is useless and annoying in UTF-8, so strip it
		} else {
			length += getUTF8Length(chr);
		}
	}
	
	return length;
}


/*!
 * Convert an UTF-16 string to an UTF-8 string
 * @param begin Start of the string
 * @param end End of the string
 */
template <typename In>
std::string convertUTF16LEToUTF8(In begin, In end) {
	std::string buffer;
	buffer.resize(getUTF16LEToUTF8Length(begin, end));
	std::string::iterator oend = convertUTF16LEToUTF8(begin, end, buffer.begin());
	arx_assert(oend == buffer.end()); ARX_UNUSED(oend);
	return buffer;
}

} // namespace util

#endif // ARX_UTIL_UNICODE_H
