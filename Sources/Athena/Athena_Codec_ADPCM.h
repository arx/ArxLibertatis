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
#ifndef __ATHENA_CODEC_ADPCM_H__
#define __ATHENA_CODEC_ADPCM_H__

#include <stdio.h>
#include <Athena_Types.h>
#include "Athena_Codec.h"

namespace ATHENA
{

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Class CodecADPCM                                                          //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	class CodecADPCM : public Codec
	{
		public:
			// Constructor Destructor                                                    //
			CodecADPCM();
			~CodecADPCM();
			// Setup                                                                     //
			aalError SetHeader(aalVoid * header);
			aalError SetStream(FILE * stream);
			aalError SetPosition(const aalULong & position);
			// Status                                                                    //
			aalError GetHeader(aalVoid *&header);
			aalError GetStream(FILE *&stream);
			aalError GetPosition(aalULong & position);
			// File I/O                                                                  //
			aalError Read(aalVoid * buffer, const aalULong & to_read, aalULong & read);
			aalError Write(aalVoid * buffer, const aalULong & to_write, aalULong & write);
		private:
			// Macros!                                                                   //
			aalVoid GetSample(const aalULong & channel_i, aalSByte nybble);
			aalError GetNextBlock();
			// Data                                                                      //
			FILE * stream;
			ADPCMWAVEFORMAT * header;
			aalULong padding;
			aalULong shift;
			aalULong sample_i;
			char * predictor;
			aalSWord * delta;
			aalSWord * samp1;
			aalSWord * samp2;
			aalSWord * coef1;
			aalSWord * coef2;
			aalSByte * nybble_l;
			aalULong nybble_c, nybble_i;
			aalSByte nybble;
			aalUByte odd;
			aalUByte cache_c, cache_i;
			aalVoid * cache_l;
			aalULong cursor;
	};

}//ATHENA::

#endif//__ATHENA_CODEC_ADPCM_H__