/*
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
// Code: Didier P�dreno

#ifndef ARX_LOC_H
#define ARX_LOC_H

#include <string>

#include "core/Common.h"

void Localisation_Init();
void Localisation_Close();


bool PAK_UNICODE_GetPrivateProfileString( const std::string& section,
                                          const std::string& default_return,
                                          std::string& buffer);

long HERMES_UNICODE_GetProfileSectionKeyCount( const std::string& sectionname);
long HERMES_UNICODE_GetProfileString( const std::string& sectionname,
                                      const std::string& defaultstring,
                                      std::string& destination );

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
inline std::size_t GetUTF16Length(In Begin, In End)
{
    std::size_t Length = 0;
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

#endif // ARX_LOC_H
