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

#ifndef ARX_AUDIO_CODEC_ADPCM_H
#define ARX_AUDIO_CODEC_ADPCM_H

#include "audio/AudioTypes.h"
#include "audio/codec/Codec.h"

struct PakFileHandle;

#include <windows.h> // needed by mmreg.h
#include <mmreg.h> // for ADPCMWAVEFORMAT

namespace audio {
	
	class CodecADPCM : public Codec {
		
	public:
		
		CodecADPCM();
		~CodecADPCM();
		
		// Setup
		aalError SetHeader(void * header);
		aalError SetStream(PakFileHandle * stream);
		aalError SetPosition(size_t position);
		
		// Status
		aalError GetHeader(void *& header);
		aalError GetStream(PakFileHandle *& stream);
		aalError GetPosition(size_t & position);
		
		// File I/O
		aalError Read(void * buffer, size_t to_read, size_t & read);
		
	private:
		
		void GetSample(size_t channel_i, s8 nybble);
		aalError GetNextBlock();
		
		PakFileHandle * stream;
		ADPCMWAVEFORMAT * header;
		u32 padding;
		u32 shift;
		u32 sample_i;
		char * predictor;
		s16 * delta;
		s16 * samp1;
		s16 * samp2;
		s16 * coef1;
		s16 * coef2;
		s8 * nybble_l;
		u32 nybble_c, nybble_i;
		s8 nybble;
		bool odd;
		u8 cache_c, cache_i;
		void * cache_l;
		size_t cursor;
		
	};

} // namespace audio

#endif // ARX_AUDIO_CODEC_ADPCM_H
