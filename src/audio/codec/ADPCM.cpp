/*
 * Copyright 2011-2016 Arx Libertatis Team (see the AUTHORS file)
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
/* Based on:
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

#include "audio/codec/ADPCM.h"

#include <algorithm>

#include "audio/AudioTypes.h"
#include "audio/codec/WAVFormat.h"
#include "io/resource/PakReader.h"

namespace audio {

// Fixed point delta adaption table
static const short gai_p4[] = {
	230, 230, 230, 230, 307, 409, 512, 614,
	768, 614, 512, 409, 307, 230, 230, 230
};

CodecADPCM::CodecADPCM() :
	stream(NULL), header(NULL), padding(0), shift(0),
	sample_i(0xffffffff), predictor(NULL), delta(NULL),
	samp1(NULL), samp2(NULL), coef1(NULL), coef2(NULL),
	nybble_l(NULL), nybble_c(0), nybble_i(0), nybble(0), odd(false),
	cache_c(0), cache_i(0), cache_l(NULL), cursor(0) {
}

CodecADPCM::~CodecADPCM() {
	delete[] predictor;
	delete[] delta;
	delete[] samp1;
	delete[] samp2;
	delete[] coef1;
	delete[] coef2;
	delete[] (s8 *)cache_l;
	delete[] nybble_l;
}

aalError CodecADPCM::setHeader(void * _header) {
	
	if(header || !_header) {
		return AAL_ERROR_SYSTEM;
	}
	
	header = (ADPCMHeader *)_header;
	
	if(header->wfx.channels != 1 && header->wfx.channels != 2) {
		return AAL_ERROR_FORMAT;
	}
	
	shift = header->wfx.channels - 1;
	padding = 0;
	sample_i = 0xffffffff;
	
	size_t nb = size_t(1) << shift;
	
	predictor = new char[nb];
	delta = new s16[nb];
	coef1 = new s16[nb];
	coef2 = new s16[nb];
	samp1 = new s16[nb];
	samp2 = new s16[nb];
	if(!predictor || !delta || !coef1 || !coef2 || !samp1 || !samp2) {
		return AAL_ERROR_MEMORY;
	}
	
	cache_c = cache_i = (u8)(sizeof(s16) << shift);
	cache_l = new s8[cache_c];
	if(!cache_l) {
		return AAL_ERROR_MEMORY;
	}
	
	nybble_c = header->samplesPerBlock - 2;
	if(!shift) {
		nybble_c >>= 1;
	}
	nybble_l = new s8[nybble_c];
	if(!nybble_l) {
		return AAL_ERROR_MEMORY;
	}
	
	padding = ((header->wfx.blockAlign - (7 << shift)) << 3) -
	          (header->samplesPerBlock - 2) * (header->wfx.bitsPerSample << shift);
	
	if(aalError error = getNextBlock()) {
		return error;
	}
	
	sample_i++;
	
	return AAL_OK;
}

void CodecADPCM::setStream(PakFileHandle * _stream) {
	stream = _stream;
}

aalError CodecADPCM::setPosition(size_t _position) {
	
	size_t i = (_position >> shift) / header->samplesPerBlock;
	
	if(stream->seek(SeekCur, i * header->wfx.blockAlign) == -1) {
		return AAL_ERROR_FILEIO;
	}
	
	i = _position - i * (header->samplesPerBlock << shift);
	
	// Get header from current block
	if(aalError error = getNextBlock()) {
		return error;
	}
	
	sample_i++;
	cache_i = 0;
	
	while(i) {
		char buffer[256];
		size_t nRead;
		size_t to_read = std::min(i, (size_t)256);
		if(aalError error = read(buffer, to_read, nRead)) {
			return error;
		}
		i -= nRead;
	}
	
	cursor = _position;
	
	return AAL_OK;
}

size_t CodecADPCM::getPosition() {
	return cursor;
}

void CodecADPCM::getSample(size_t i, s8 adpcm_sample) {
	
	// Update delta
	s32 old_delta = delta[i];
	delta[i] = s16((gai_p4[adpcm_sample] * old_delta) >> 8);
	
	if(delta[i] < 16) {
		delta[i] = 16;
	}
	
	// Sign-extend adpcm_sample
	if(adpcm_sample & 0x08) {
		adpcm_sample -= 16;
	}
	
	// Predict next sample
	s32 predict = ((s32)samp1[i] * coef1[i] + (s32)samp2[i] * coef2[i]) >> 8;
	
	// Reconstruct original PCM
	s32 pcm_sample = adpcm_sample * old_delta + predict;
	
	// Clip value to signed 16 bits limits
	if(pcm_sample > 32767) {
		pcm_sample = 32767;
	} else if(pcm_sample < -32768) {
		pcm_sample = -32768;
	}
	
	// Update samples
	samp2[i] = samp1[i];
	samp1[i] = (s16)pcm_sample;
}

aalError CodecADPCM::read(void * buffer, size_t to_read, size_t & read) {
	
	read = 0;
	while(read < to_read) {
		
		// If prefetched bytes are remaining, put the next one into the buffer
		if(cache_i < cache_c) {
			((s8 *)buffer)[read++] = ((s8 *)cache_l)[cache_i++];
			continue;
		}
		
		// Load next block header if there is no more sample in current one
		if(sample_i >= header->samplesPerBlock) {
			
			if(padding) {
				stream->seek(SeekCur, padding);
			}
			
			if(aalError error = getNextBlock()) {
				return error;
			}
			
		} else if(sample_i == 1) {
			for(size_t i = 0; i < header->wfx.channels; i++) {
				((s16 *)cache_l)[i] = samp1[i];
			}
		} else {
			
			// Get new sample for each channel
			for(size_t i = 0; i < header->wfx.channels; i++) {
				if(odd) {
					getSample(i, (s8)(nybble & 0x0f));
					odd = false;
				} else {
					nybble = nybble_l[nybble_i++];
					getSample(i, s8((nybble >> 4) & 0x0f));
					odd = true;
				}
				((s16 *)cache_l)[i] = samp1[i];
			}
		}
		
		sample_i++;
		cache_i = 0;
	}
	
	return AAL_OK;
}

aalError CodecADPCM::getNextBlock() {
	
	// Load and check block header
	if(!stream->read(predictor, sizeof(*predictor) << shift)) {
		return AAL_ERROR_FILEIO;
	}
	
	if(!stream->read(delta, sizeof(*delta) << shift)) {
		return AAL_ERROR_FILEIO;
	}
	
	if(!stream->read(samp1, sizeof(*samp1) << shift)) {
		return AAL_ERROR_FILEIO;
	}
	
	if(!stream->read(samp2, sizeof(*samp2) << shift)) {
		return AAL_ERROR_FILEIO;
	}
	
	odd = false;
	sample_i = 0;
	nybble_i = 0;
	
	for(size_t i = 0; i < header->wfx.channels; i++) {
		if(predictor[i] >= header->coefficientCount) {
			return AAL_ERROR_FORMAT;
		}
		coef1[i] = header->coefficients[(size_t)predictor[i]].coef1;
		coef2[i] = header->coefficients[(size_t)predictor[i]].coef2;
		((s16 *)cache_l)[i] = samp2[i];
	}
	
	if(!stream->read(nybble_l, nybble_c)) {
		return AAL_ERROR_FILEIO;
	}
	
	return AAL_OK;
}

} // namespace audio
