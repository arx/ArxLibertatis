
#ifndef ARX_AUDIO_CODEC_WAVFORMAT_H
#define ARX_AUDIO_CODEC_WAVFORMAT_H

#include "platform/Platform.h"

#pragma pack(push,1)

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
	short coef1;
	short coef2;
};

struct ADPCMHeader {
	WaveHeader wfx;
	u16 samplesPerBlock;
	u16 coefficientCount;
	ADPCMCoefficientPair coefficients[0];
};

#pragma pack(pop)

#endif // ARX_AUDIO_CODEC_WAVFORMAT_H
