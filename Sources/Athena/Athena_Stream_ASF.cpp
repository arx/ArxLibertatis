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
#include <malloc.h>
#include <memory.h>
#include <stdlib.h>
#include "Athena_Stream_ASF.h"
#include "Athena_FileIO.h"

namespace ATHENA
{

	static const AAF_MAGIC(0x41414646);      //'AAFF'
	static const AAF_VERSION(0x01000000);

	// Output frequency
	//  0 :  8000 Hz
	//  1 : 11025 Hz
	//  2 : 12000 Hz
	//  3 : 16000 Hz
	//  4 : 22050 Hz
	//  5 : 24000 Hz
	//  6 : 32000 Hz
	//  7 : 44100 Hz
	//  8 : 48000 Hz
	//

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Constrcutor and destructor                                                //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	StreamASF::StreamASF() :
		stream(NULL), offset(0), cursor(0)
	{
		frame.vector = NULL;
		frame.vtable = NULL;
	}

	StreamASF::~StreamASF()
	{
		free(frame.vector);
		free(frame.vtable);
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Setup                                                                     //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	aalError StreamASF::SetStream(FILE * _stream)
	{
		if (!_stream) return AAL_ERROR_FILEIO;

		free(frame.vector), frame.vector = NULL;
		free(frame.vtable), frame.vtable = NULL;

		stream = _stream;

		if (!FileRead(&header.magic, 4, 1, stream)) return AAL_ERROR_FILEIO;

		if (header.magic != AAF_MAGIC) return AAL_ERROR_FORMAT;

		if (!FileRead(&header.version, 4, 1, stream)) return AAL_ERROR_FILEIO;

		if (header.version > AAF_VERSION) return AAL_ERROR_FORMAT;

		if (!FileRead(&header.f_size, 4, 1, stream)) return AAL_ERROR_FILEIO;

		if (!FileRead(&header.o_freq, 4, 1, stream)) return AAL_ERROR_FILEIO;

		if (!FileRead(&header.o_qual, 4, 1, stream)) return AAL_ERROR_FILEIO;

		if (!FileRead(&header.o_chnl, 4, 1, stream)) return AAL_ERROR_FILEIO;

		if (!FileRead(&header.o_size, 4, 1, stream)) return AAL_ERROR_FILEIO;

		if (!FileRead(&header.frame_c, 4, 1, stream)) return AAL_ERROR_FILEIO;

		offset = FileTell(stream);

		return AAL_OK;
	}

	aalError StreamASF::SetFormat(const aalFormat &)
	{
		return AAL_ERROR;
	}

	aalError StreamASF::SetLength(const aalULong &)
	{
		return AAL_ERROR;
	}

	aalError StreamASF::SetPosition(const aalULong &)
	{
		return AAL_ERROR;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Status                                                                    //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	aalVoid StreamASF::GetStream(FILE *&file)
	{
		file = stream;
	}

	aalVoid StreamASF::GetFormat(aalFormat & _format)
	{
		_format.frequency = header.o_freq;
		_format.quality = header.o_qual;
		_format.channels = header.o_chnl;
	}

	aalVoid StreamASF::GetLength(aalULong & _length)
	{
		_length = header.o_size;
	}

	aalVoid StreamASF::GetPosition(aalULong &)
	{
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// I/O                                                                       //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	aalVoid StreamASF::Read(aalVoid * buffer, const aalULong & to_read, aalULong & read)
	{
		read = 0;

		while (read < to_read && frame_i < header.frame_c)
		{
			aalULong max, count;

			max = to_read - read;

			if (remaining)
			{
				if (remaining < max)
					count = remaining, remaining = 0;
				else
					count = max, remaining -= max;
			}
			else
			{
				if (c_vector >= frame.d_size) NextFrame();
				else NextVector();

				if (max < frame.v_size)
					count = max, remaining = frame.v_size - max;
				else
					count = frame.v_size, remaining = 0;
			}

			memcpy(&((aalUByte *)buffer)[read], &frame.vector[cursor], count);

			cursor += count;
			read += count;
		}
	}

	aalVoid StreamASF::Write(aalVoid *, const aalULong &, aalULong &)
	{
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Tools                                                                     //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	aalVoid StreamASF::NextFrame()
	{
	}

	aalVoid StreamASF::NextVector()
	{
	}

}//ATHENA::