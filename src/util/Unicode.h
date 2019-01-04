/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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
static const Unicode BYTE_ORDER_MARK = 0xfeff; //!< Unicode byte order mark - for UTF-16

//! Functions to read and write UTF-8 encoded data
struct UTF8 {
	
	//! \return true c is not the first byte of an UTF-8 character
	static bool isContinuationByte(u8 chr) {
		return (chr & 0xc0) == 0x80;
	}
	
	/*!
	 * Advance to the next character.
	 */
	template <typename In>
	static In next(In it, In end);
	
	/*!
	* Decode the first character from an UTF-8 string
	* \tparam In          An InputIterator type for the input string
	* \param  it          Start of the input string - will be advanced by the bytes read
	* \param  end         End of the inputstring
	* \param  replacement Replacement code point for invalid data
	* \return one Unicode code point or INVALID_CHAR if we are at the end of the range
	*/
	template <typename In>
	static Unicode read(In & it, In end, Unicode replacement = REPLACEMENT_CHAR);
	
	//! Get the number of UTF-8 bytes required for a unicode character
	static size_t length(Unicode chr);
	
	/*!
	* Encode one character to an UTF8 string
	* \tparam Out  An OutputIterator type for the input string
	* \param  it  Output iterator to write the encoded caracter to
	* \return new output iterator after the written bytes
	*/
	template <typename Out>
	static Out write(Out it, Unicode chr);
	
};

//! Functions to read UTF-16 little-endian encoded data
struct UTF16LE {
	
	//! \return true c is is the first part of an UTF-16 surrogate pair
	static bool isHighSurrogate(u16 chr) {
		return chr >= 0xd800 && chr <= 0xdbff;
	}

	//! \return true c is is the first part of an UTF-16 surrogate pair
	static bool isLowSurrogate(u16 chr) {
		return chr >= 0xdc00 && chr <= 0xdfff;
	}
	
	/*!
	* Decode the first character from an UTF-16 little-endian string
	* \tparam In          An InputIterator type for the input string
	* \param  it          Start of the input string - will be advanced by the bytes read
	* \param  end         End of the inputstring
	* \param  replacement Replacement code point for invalid data
	* \return one Unicode code point or INVALID_CHAR if we are at the end of the range
	*/
	template <typename In>
	static Unicode read(In & it, In end, Unicode replacement = REPLACEMENT_CHAR);
	
	// writing not supported
	
};

//! Functions to read ISO_8859-1  encoded data
struct ISO_8859_1 {
	
	/*!
	* Decode the first character from an ISO_8859-1 string
	* \tparam In          An InputIterator type for the input string
	* \param  it          Start of the input string - will be advanced by the bytes read
	* \param  end         End of the inputstring
	* \param  replacement Replacement code point for invalid data
	* \return one Unicode code point or INVALID_CHAR if we are at the end of the range
	*/
	template <typename In>
	static Unicode read(In & it, In end, Unicode replacement = REPLACEMENT_CHAR);
	
	// writing not supported
	
};

/*!
 * Convert a string from one encoding to another
 * \tparam InEnc       The encoding of the input string
 * \tparam OutEnc      The output encoding to write
 * \tparam In          An InputIterator type for the input string
 * \tparam Out         An OutputIterator type for the input string
 * \param  begin       Start of the string in the source endocing
 * \param  end         End of the string in the source encoding
 * \param  output      Output iterator to write the result to
 * \param  replacement Replacement code point for invalid data
 * \return new output iterator position after the written bytes
 */
template <typename InEnc, typename OutEnc, typename In, typename Out>
Out convert(In begin, In end, Out output, Unicode replacement = REPLACEMENT_CHAR) {
	
	Unicode chr;
	for(In it = begin; (chr = InEnc::read(it, end, replacement)) != INVALID_CHAR;) {
		if(chr == BYTE_ORDER_MARK) {
			// The BOM is useless and annoying in UTF-8, so strip it
		} else {
			output = OutEnc::write(output, chr);
		}
	}
	
	return output;
}

/*!
 * Get the number of bytes required to represent an string in one encoding in another one
 * \tparam InEnc       The encoding of the input string
 * \tparam OutEnc      The output encoding to count the required characters for
 * \tparam In          An InputIterator type for the input string
 * \param  begin       Start of the string in the source endocing
 * \param  end         End of the string in the source encoding
 * \param  replacement Replacement code point for invalid data
 * \return the number of bytes required to represent the input string in the output
 *         encoding
 */
template <typename InEnc, typename OutEnc, typename In>
size_t getConvertedLength(In begin, In end, Unicode replacement = REPLACEMENT_CHAR) {
	
	size_t length = 0;
	
	Unicode chr;
	for(In it = begin; (chr = InEnc::read(it, end, replacement)) != INVALID_CHAR;) {
		if(chr == BYTE_ORDER_MARK) {
			// The BOM is useless and annoying in UTF-8, so strip it
		} else {
			length += OutEnc::length(chr);
		}
	}
	
	return length;
}

/*!
 * Convert a string from one encoding to another
 * \tparam InEnc  The encoding of the input string
 * \tparam OutEnc The output encoding to return
 * \tparam In     A ForwardIterator type for the input string
 * \param  begin  Start of the string in the source endocing
 * \param  end    End of the string in the source encoding
 * \return an \ref std::string with the string in the destination encoding
 */
template <typename InEnc, typename OutEnc, typename In>
std::string convert(In begin, In end) {
	std::string buffer;
	buffer.resize(getConvertedLength<InEnc, OutEnc>(begin, end));
	std::string::iterator oend = convert<InEnc, OutEnc>(begin, end, buffer.begin());
	arx_assert(oend == buffer.end());
	ARX_UNUSED(oend);
	return buffer;
}

/*!
 * Convert a string from one encoding to another
 * \tparam InEnc  The encoding of the input string
 * \tparam OutEnc The output encoding to return
 * \param  str    The input string
 * \return an \ref std::string with the string in the destination encoding
 */
template <typename InEnc, typename OutEnc>
std::string convert(const std::string & str) {
	return convert<InEnc, OutEnc>(str.begin(), str.end());
}

/*!
 * Encode a Unicode character according to a given encoding
 * This is a small convenience wrapper around \ref OutEnc::write()
 * \tparam OutEnc    The encoding to use
 * \param  character The Unicode code point to encode
 * \return an \ref std::string containing the encoded caracter
 */
template <typename OutEnc>
std::string encode(Unicode character) {
	std::string result;
	result.resize(OutEnc::length(character));
	std::string::iterator oend = OutEnc::write(result.begin(), character);
	arx_assert(oend == result.end());
	ARX_UNUSED(oend);
	return result;
}

//---------------------------------------------------------------------------------------
// Implementation

template <typename In>
In UTF8::next(In it, In end) {
	++it;
	while(it != end && isContinuationByte(*it)) {
		++it;
	}
	return it;
}

template <typename In>
Unicode UTF8::read(In & it, In end, Unicode replacement) {
	
	if(it == end) {
		return INVALID_CHAR;
	}
	Unicode chr = u8(*it++);
	
	// For multi-byte characters, read the remaining bytes
	if(chr & (1 << 7)) {
		
		if(isContinuationByte(chr)) {
			// Bad start position
			return replacement;
		}
		
		if(it == end || !isContinuationByte(u8(*it))) {
			// Unexpected end of multi-byte sequence
			return replacement;
		}
		chr &= 0x3f, chr <<= 6, chr |= (u8(*it++) & 0x3f);
		
		if(chr & (1 << (5 + 6))) {
			
			if(it == end || !isContinuationByte(u8(*it))) {
				// Unexpected end of multi-byte sequence
				return replacement;
			}
			chr &= ~(1 << (5 + 6)), chr <<= 6, chr |= (u8(*it++) & 0x3f);
			
			if(chr & (1 << (4 + 6 + 6))) {
				
				if(it == end || !isContinuationByte(u8(*it))) {
					// Unexpected end of multi-byte sequence
					return replacement;
				}
				chr &= ~(1 << (4 + 6 + 6)), chr <<= 6, chr |= (u8(*it++) & 0x3f);
				
				if(chr & (1 << (3 + 6 + 6 + 6))) {
					// Illegal UTF-8 byte
					return replacement;
				}
				
			}
		}
	}
	
	return chr;
}

inline size_t UTF8::length(Unicode chr) {
	if     (chr <  0x80)       return 1;
	else if(chr <  0x800)      return 2;
	else if(chr <  0x10000)    return 3;
	else if(chr <= 0x0010FFFF) return 4;
	return 1;
}

template <typename Out>
Out UTF8::write(Out it, Unicode chr) {
	
	static const u8 utf8FirstBytes[7] = { 0x00, 0x00, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc };
	
	// Get number of bytes to write
	int bytesToWrite = length(chr);
	
	// Extract bytes to write
	u8 bytes[4];
	switch(bytesToWrite) {
		case 4: {
			bytes[3] = static_cast<u8>((chr | 0x80) & 0xBF);
			chr >>= 6;
		} /* fall-through */
		case 3: {
			bytes[2] = static_cast<u8>((chr | 0x80) & 0xBF);
			chr >>= 6;
		} /* fall-through */
		case 2: {
			bytes[1] = static_cast<u8>((chr | 0x80) & 0xBF);
			chr >>= 6;
		} /* fall-through */
		case 1: {
			bytes[0] = static_cast<u8>(chr | utf8FirstBytes[bytesToWrite]);
			break;
		}
		default: arx_unreachable();
	}
	
	// Add them to the output
	const u8 * curByte = bytes;
	switch(bytesToWrite) {
		case 4 : *it++ = *curByte++; /* fall-through */
		case 3 : *it++ = *curByte++; /* fall-through */
		case 2 : *it++ = *curByte++; /* fall-through */
		case 1 : *it++ = *curByte++; break;
		default: arx_unreachable();
	}
	
	return it;
}

template <typename In>
Unicode UTF16LE::read(In & it, In end, Unicode replacement) {
	
	if(it == end) {
		return INVALID_CHAR;
	}
	Unicode chr = u8(*it++);
	if(it == end) {
		return replacement;
	}
	chr |= Unicode(u8(*it++)) << 8;
	
	// If it's a surrogate pair, convert to a single UTF-32 character
	if(isHighSurrogate(chr)) {
		if(it != end) {
			Unicode d = u8(*it++);
			if(it == end) {
				return replacement;
			}
			d |= Unicode(u8(*it++)) << 8;
			if(isLowSurrogate(d)) {
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

template <typename In>
Unicode ISO_8859_1::read(In & it, In end, Unicode replacement) {
	
	if(it == end) {
		return INVALID_CHAR;
	}
	
	Unicode chr = u8(*it++);
	if(chr >= 128 && chr < 160) {
		return replacement;
	}
	
	// ISO-8859-1 maps directly to Unicode - yay!
	return chr;
}

} // namespace util

#endif // ARX_UTIL_UNICODE_H
