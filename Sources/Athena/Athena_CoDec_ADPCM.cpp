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
#include <windows.h>
#include <mmreg.h>
#include <Athena_Types.h>
#include "Athena_Codec_ADPCM.h"
#include "Athena_FileIO.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>


namespace ATHENA
{

	// Fixed point delta adaption table                                          //
	static const short gai_p4[] =
	{
		230, 230, 230, 230, 307, 409, 512, 614,
		768, 614, 512, 409, 307, 230, 230, 230
	};

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Constructor and destructor                                                //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	CodecADPCM::CodecADPCM() :
		header(NULL),
		stream(NULL),
		padding(0),
		shift(0),
		sample_i(0xffffffff),
		predictor(NULL),
		delta(NULL),
		samp1(NULL), samp2(NULL),
		coef1(NULL), coef2(NULL),
		nybble_c(0), nybble_i(0), nybble_l(NULL),
		nybble(0),
		odd(0),
		cache_c(0), cache_i(0), cache_l(NULL),
		cursor(0)
	{
	}

	CodecADPCM::~CodecADPCM()
	{
		free(predictor);
		free(delta);
		free(samp1);
		free(samp2);
		free(coef1);
		free(coef2);
		free(cache_l);
		free(nybble_l);
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Setup                                                                     //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	aalError CodecADPCM::SetHeader(aalVoid * _header)
	{
		header = (ADPCMWAVEFORMAT *)_header;

		if (header->wfx.nChannels != 1 && header->wfx.nChannels != 2)
			return AAL_ERROR_FORMAT;

		shift = header->wfx.nChannels - 1;
		padding = 0;
		sample_i = 0xffffffff;

		aalVoid * ptr;

		ptr = realloc(predictor, sizeof(char) << shift);

		if (!ptr) return AAL_ERROR_MEMORY;

		predictor = (char *)ptr;

		ptr = realloc(delta, sizeof(aalSWord) << shift);

		if (!ptr) return AAL_ERROR_MEMORY;

		delta = (aalSWord *)ptr;

		ptr = realloc(coef1, sizeof(aalSWord) << shift);

		if (!ptr) return AAL_ERROR_MEMORY;

		coef1 = (aalSWord *)ptr;

		ptr = realloc(coef2, sizeof(aalSWord) << shift);

		if (!ptr) return AAL_ERROR_MEMORY;

		coef2 = (aalSWord *)ptr;

		ptr = realloc(samp1, sizeof(aalSWord) << shift);

		if (!ptr) return AAL_ERROR_MEMORY;

		samp1 = (aalSWord *)ptr;

		ptr = realloc(samp2, sizeof(aalSWord) << shift);

		if (!ptr) return AAL_ERROR_MEMORY;

		samp2 = (aalSWord *)ptr;

		ptr = realloc(cache_l, cache_c = cache_i = (aalUByte)(sizeof(aalSWord) << shift));

		if (!ptr) return AAL_ERROR_MEMORY;

		cache_l = ptr;

		nybble_c = header->wSamplesPerBlock - 2;

		if (!shift) nybble_c >>= 1;

		ptr = realloc(nybble_l, nybble_c);

		if (!ptr) return AAL_ERROR_MEMORY;

		nybble_l = (aalSByte *)ptr;

		padding = ((header->wfx.nBlockAlign - (7 << shift)) << 3) -
		          (header->wSamplesPerBlock - 2) * (header->wfx.wBitsPerSample << shift);

		aalError error(GetNextBlock());

		if (error) return error;

		sample_i++;

		return AAL_OK;
	}

	aalError CodecADPCM::SetStream(FILE * _stream)
	{
		stream = _stream;

		return AAL_OK;
	}

	// The stream cursor must be at the begining of waveform data                //
	aalError CodecADPCM::SetPosition(const aalULong & _position)
	{
		aalError error;

		aalULong i = (_position >> shift) / header->wSamplesPerBlock;

		if (FileSeek(stream, i * header->wfx.nBlockAlign, SEEK_CUR))
			return AAL_ERROR_FILEIO;

		i = _position - i * (header->wSamplesPerBlock << shift);

		// Get header from current block
		error = GetNextBlock();

		if (error) return error;

		sample_i++;
		cache_i = 0;

		char buffer[256];
		aalULong to_read, read;

		while (i)
		{
			to_read = i >= 256 ? 256 : i;

			error = Read(buffer, to_read, read);

			if (error) return error;

			i -= read;
		}

		cursor = _position;

		return AAL_OK;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Status                                                                    //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	aalError CodecADPCM::GetHeader(aalVoid *&_header)
	{
		_header = header;

		return AAL_OK;
	}

	aalError CodecADPCM::GetStream(FILE *&_stream)
	{
		_stream = stream;

		return AAL_OK;
	}

	aalError CodecADPCM::GetPosition(aalULong & _position)
	{
		_position = cursor;

		return AAL_OK;
	}
	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Macros!                                                                   //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	__forceinline aalVoid CodecADPCM::GetSample(const aalULong & i, aalSByte adpcm_sample)
	{
		aalSLong predict, pcm_sample, old_delta;

		// Update delta
		old_delta = delta[i];
		delta[i] = aalSWord((gai_p4[adpcm_sample] * old_delta) >> 8);

		if (delta[i] < 16) delta[i] = 16;

		// Sign-extend adpcm_sample
		if (adpcm_sample & 0x08) adpcm_sample -= 16;

		// Predict next sample
		predict = ((aalSLong)samp1[i] * coef1[i] + (aalSLong)samp2[i] * coef2[i]) >> 8;

		// Reconstruct original PCM
		pcm_sample = adpcm_sample * old_delta + predict;

		// Clip value to signed 16 bits limits
		if (pcm_sample > 32767) pcm_sample = 32767;
		else if (pcm_sample < -32768) pcm_sample = -32768;

		// Update samples
		samp2[i] = samp1[i];
		samp1[i] = (aalSWord)pcm_sample;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// File I/O                                                                  //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	aalError CodecADPCM::Read(aalVoid * buffer, const aalULong & to_read, aalULong & read)
	{
		read = 0;

		while (read < to_read)
		{
			// If prefetched bytes are remaining, put the next one into the buffer
			if (cache_i < cache_c)
			{
				((aalSByte *)buffer)[read++] = ((aalSByte *)cache_l)[cache_i++];
				continue;
			}

			// Load next block header if there is no more sample in current one
			if (sample_i >= header->wSamplesPerBlock)
			{
				aalError error;

				if (padding) FileSeek(stream, padding, SEEK_CUR);

				error = GetNextBlock();

				if (error) return error;
			}
			else if (sample_i == 1)
			{
				for (aalULong i(0); i < header->wfx.nChannels; i++)
					((aalSWord *)cache_l)[i] = samp1[i];
			}
			else
			{
				// Get new sample for each channel
				for (aalULong i(0); i < header->wfx.nChannels; i++)
				{
					if (odd)
					{
						GetSample(i, (aalSByte)(nybble & 0x0f));
						odd = AAL_UFALSE;
					}
					else
					{
						nybble = nybble_l[nybble_i++];

						GetSample(i, aalSByte((nybble >> 4) & 0x0f));
						odd = AAL_UTRUE;
					}

					((aalSWord *)cache_l)[i] = samp1[i];
				}
			}

			sample_i++;
			cache_i = 0;
		}

		return AAL_OK;
	}

	aalError CodecADPCM::Write(aalVoid *, const aalULong &, aalULong & write)
	{
		write = 0;

		return AAL_ERROR;
	}


	aalError CodecADPCM::GetNextBlock()
	{
		// Load and check block header
		if (!FileRead(predictor, sizeof(aalUByte) << shift, 1, stream)) return AAL_ERROR_FILEIO;

		if (!FileRead(delta, sizeof(aalSWord) << shift, 1, stream)) return AAL_ERROR_FILEIO;

		if (!FileRead(samp1, sizeof(aalSWord) << shift, 1, stream)) return AAL_ERROR_FILEIO;

		if (!FileRead(samp2, sizeof(aalSWord) << shift, 1, stream)) return AAL_ERROR_FILEIO;

		odd = AAL_UFALSE;
		sample_i = 0;
		nybble_i = 0;

		for (aalULong i(0); i < header->wfx.nChannels; i++)
		{
			if (predictor[i] >= header->wNumCoef) return AAL_ERROR_FORMAT;

			coef1[i] = header->aCoef[predictor[i]].iCoef1;
			coef2[i] = header->aCoef[predictor[i]].iCoef2;

			((aalSWord *)cache_l)[i] = samp2[i];
		}

		if (!FileRead(nybble_l, nybble_c, 1, stream)) return AAL_ERROR_FILEIO;

		return AAL_OK;
	}

}//ATHENA::