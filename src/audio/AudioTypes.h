/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_AUDIO_AUDIOTYPES_H
#define ARX_AUDIO_AUDIOTYPES_H

#include "platform/Platform.h"
#include "platform/Flags.h"
#include "math/Vector.h"

namespace audio {

// Default values
const size_t DEFAULT_STREAMLIMIT = 88200; // in Bytes; ~1 second for the correct format

const float DEFAULT_ENVIRONMENT_SIZE = 7.5f;
const float DEFAULT_ENVIRONMENT_DIFFUSION = 1.f; // High density echoes
const float DEFAULT_ENVIRONMENT_ABSORPTION = 0.05f; // Air-like absorbtion
const float DEFAULT_ENVIRONMENT_REFLECTION_VOLUME = 0.8f;
const float DEFAULT_ENVIRONMENT_REFLECTION_DELAY = 7.f;
const float DEFAULT_ENVIRONMENT_REVERBERATION_VOLUME = 1.f;
const float DEFAULT_ENVIRONMENT_REVERBERATION_DELAY = 10.f;
const float DEFAULT_ENVIRONMENT_REVERBERATION_DECAY = 1500.f;
const float DEFAULT_ENVIRONMENT_REVERBERATION_HFDECAY = 1200.f;

const float DEFAULT_VOLUME = 1.f; // Original gain

// Flags
enum ChannelFlag {
	FLAG_RESTART       = 0x00000001, // Force restart sample if already playing
	FLAG_ENQUEUE       = 0x00000002, // Enqueue sample if already playing
	FLAG_VOLUME        = 0x00000004, // Enable volume control
	FLAG_PITCH         = 0x00000008, // Enable pitch control
	FLAG_PAN           = 0x00000010, // Enable pan control
	FLAG_POSITION      = 0x00000020, // Enable position control
	FLAG_VELOCITY      = 0x00000040, // Enable velocity control
	FLAG_DIRECTION     = 0x00000080, // Enable orientation control
	FLAG_CONE          = 0x00000100, // Enable cone control
	FLAG_FALLOFF       = 0x00000200, // Enable intensity control
	FLAG_REVERBERATION = 0x00000400, // Enable environment reverberation
	FLAG_RELATIVE      = 0x00001000, // Compute position relative to the listener
	FLAG_AUTOFREE      = 0x00008000, // Free resource when playing is finished
};
DECLARE_FLAGS(ChannelFlag, ChannelFlags)
DECLARE_FLAGS_OPERATORS(ChannelFlags)

// Length units
enum TimeUnit {
	UNIT_MS,
	UNIT_SAMPLES,
	UNIT_BYTES
};

// Errors
enum aalError {
	AAL_OK = 0,
	AAL_ERROR, // General error
	AAL_ERROR_INIT, // Not initialized
	AAL_ERROR_MEMORY, // Not enough memory
	AAL_ERROR_FILEIO, // File input/output error
	AAL_ERROR_FORMAT, // Invalid or corrupted file format
	AAL_ERROR_SYSTEM, // Internal system error
	AAL_ERROR_HANDLE // Invalid resource handle
};

// Output format
struct PCMFormat {
	size_t frequency; // Samples per second
	size_t quality; // Bits per sample
	size_t channels; // Output channels count
};

// Source cone
struct SourceCone {
	float inner_angle;
	float outer_angle;
	float outer_volume;
};

// Source falloff
struct SourceFalloff {
	float start;
	float end;
};

const s32 INVALID_ID = -1;

typedef s32 SourceId;
typedef s32 SampleId;
typedef s32 MixerId;
typedef s32 EnvId;
typedef s32 AmbianceId;

// Play channel initialization parameters
struct Channel {
	ChannelFlags flags;
	MixerId mixer;
	float volume;
	float pitch;
	float pan;
	Vec3f position;
	Vec3f velocity;
	Vec3f direction;
	SourceCone cone;
	SourceFalloff falloff;
};

} // namespace audio

#endif // ARX_AUDIO_AUDIOTYPES_H
