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
#ifndef __ATHENA_STREAM_ASF_H__
#define __ATHENA_STREAM_ASF_H__

#include <Athena_Types.h>
#include "Athena_Stream.h"

namespace ATHENA
{

	// AAFHeader
	//
	typedef struct
	{
		aalULong magic;
		aalULong version;
		aalULong f_size;       //File size (with this header)
		aalULong o_freq;       //Output frequency
		aalUWord o_qual;       //Output quality (bits per sample)
		aalUWord o_chnl;       //Output channels count
		aalULong o_size;       //Output data size (uncompressed)
		aalULong frame_c;      //Frame count
	} AAFHeader;

	static const aalULong AAFFreqTable[] =
	{
		8000,
		11025,
		12000,
		16000,
		22050,
		24000,
		32000,
		44100,
		48000
	};

	typedef struct
	{
		aalUWord v_size;       //Vector size
		aalULong d_size;       //Frame data size
		aalSByte * vector;     //Last decoded vector
		aalSByte * vtable;     //Vector table
	} AAFFrame;

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Class StreamASF                                                           //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	class StreamASF : public Stream
	{
		public:
			// Constructor and destructor                                                //
			StreamASF();
			~StreamASF();
			// Setup                                                                     //
			aalError SetStream(FILE * stream);
			aalError SetFormat(const aalFormat & format);
			aalError SetLength(const aalULong & length);
			aalError SetPosition(const aalULong & position);
			// Status                                                                    //
			aalVoid GetStream(FILE *&stream);
			aalVoid GetFormat(aalFormat & format);
			aalVoid GetLength(aalULong & length);
			aalVoid GetPosition(aalULong & position);
			// File I/O                                                                  //
			aalVoid Read(aalVoid * buffer, const aalULong & to_read, aalULong & read);
			aalVoid Write(aalVoid * buffer, const aalULong & to_write, aalULong & write);
		private:
			aalVoid NextFrame();
			aalVoid NextVector();
			// Data                                                                      //
			FILE * stream;

			AAFHeader header;        //File header
			aalULong offset;            //Offset of first framen

			AAFFrame frame;          //Current frame header
			aalULong frame_i;           //Current frame index

			aalULong c_vector;          //Current vector index in frame
			aalULong cursor, remaining; //Cursor and remaining bytes to read in current vector
	};

}//ATHENA::

#endif//__ATHENA_STREAM_ASF_H__