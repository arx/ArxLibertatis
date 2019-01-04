/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_AUDIO_CODEC_WAVFORMAT_H
#define ARX_AUDIO_CODEC_WAVFORMAT_H

#include "platform/Platform.h"

#pragma pack(push, 1)

enum WavFormatTag {
	WAV_FORMAT_PCM = 1,
	WAV_FORMAT_ADPCM = 2,
};

struct WaveHeader {
	u16 formatTag;
	u16 channels;
	u32 samplesPerSec;
	u32 avgBytesPerSec;
	u16 blockAlign;
	u16 bitsPerSample;
	u16 size;
};

struct ADPCMCoefficientPair {
	s16 coef1;
	s16 coef2;
};

struct ADPCMHeader {
	WaveHeader wfx;
	u16 samplesPerBlock;
	u16 coefficientCount;
	ADPCMCoefficientPair coefficients[1];
};

#pragma pack(pop)

#endif // ARX_AUDIO_CODEC_WAVFORMAT_H
