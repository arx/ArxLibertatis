/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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
// Based on:
////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2009 Laurent Gomila (laurent.gom@gmail.com)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
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

#ifndef ARX_CORE_UNICODE_HPP
#define ARX_CORE_UNICODE_HPP

#include <stddef.h>

#include "platform/Platform.h"

////////////////////////////////////////////////////////////
/// Generic function to convert an UTF-16 characters range
/// to an UTF-8 characters range, using the given locale
////////////////////////////////////////////////////////////
template <typename In, typename Out>
inline Out UTF16ToUTF8(In Begin, In End, Out Output, u8 Replacement = '?' )
{
	const u8 UTF8FirstBytes[7] =
	{
		0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC
	};

	while (Begin < End)
	{
		u32 c = *Begin++;

		// If it's a surrogate pair, first convert to a single UTF-32 character
		if ((c >= 0xD800) && (c <= 0xDBFF))
		{
			if (Begin < End)
			{
				// The second element is valid : convert the two elements to a UTF-32 character
				u32 d = *Begin++;
				if ((d >= 0xDC00) && (d <= 0xDFFF))
					c = static_cast<u32>(((c - 0xD800) << 10) + (d - 0xDC00) + 0x0010000);
			}
			else
			{
				// Invalid second element
				if (Replacement)
					*Output++ = Replacement;
			}
		}

		// Then convert to UTF-8
		if (c > 0x0010FFFF)
		{

			// Invalid character (greater than the maximum unicode value)
			if (Replacement)
				*Output++ = Replacement;
		}
		else
		{
			// Valid character

			// Get number of bytes to write
			int BytesToWrite = 1;
			if      (c <  0x80)       BytesToWrite = 1;
			else if (c <  0x800)      BytesToWrite = 2;
			else if (c <  0x10000)    BytesToWrite = 3;
			else if (c <= 0x0010FFFF) BytesToWrite = 4;

			// Extract bytes to write
			u8 Bytes[4];
			switch (BytesToWrite)
			{
				case 4 : Bytes[3] = static_cast<u8>((c | 0x80) & 0xBF); c >>= 6;
				case 3 : Bytes[2] = static_cast<u8>((c | 0x80) & 0xBF); c >>= 6;
				case 2 : Bytes[1] = static_cast<u8>((c | 0x80) & 0xBF); c >>= 6;
				case 1 : Bytes[0] = static_cast<u8> (c | UTF8FirstBytes[BytesToWrite]);
			}

			// Add them to the output
			const u8* CurByte = Bytes;
			switch (BytesToWrite)
			{
				case 4 : *Output++ = *CurByte++;
				case 3 : *Output++ = *CurByte++;
				case 2 : *Output++ = *CurByte++;
				case 1 : *Output++ = *CurByte++;
			}
		}
	}

	return Output;
}


////////////////////////////////////////////////////////////
/// Get the number of characters composing an UTF-16 string
////////////////////////////////////////////////////////////
template <typename In>
inline size_t GetUTF16Length(In Begin, In End)
{
    size_t Length = 0;
    while (Begin < End)
    {
        if ((*Begin >= 0xD800) && (*Begin <= 0xDBFF))
        {
            ++Begin;
            if ((Begin < End) && ((*Begin >= 0xDC00) && (*Begin <= 0xDFFF)))
            {
                ++Length;
            }
        }
        else
        {
            ++Length;
        }

        ++Begin;
    }

    return Length;
}

#endif // ARX_CORE_UNICODE_HPP
