/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "math/Vector.h"
#include "platform/Platform.h"
#include "util/Flags.h"
#include "util/HandleType.h"

namespace audio {

const float DEFAULT_VOLUME = 1.f; // Original gain

// Flags
enum ChannelFlag {
	FLAG_VOLUME        = 1 << 0,  // Enable volume control
	FLAG_PITCH         = 1 << 1,  // Enable pitch control
	FLAG_PAN           = 1 << 2,  // Enable pan control
	FLAG_POSITION      = 1 << 3,  // Enable position control
	FLAG_VELOCITY      = 1 << 4,  // Enable velocity control
	FLAG_FALLOFF       = 1 << 7,  // Enable intensity control
	FLAG_REVERBERATION = 1 << 8,  // Enable environment reverberation
	FLAG_RELATIVE      = 1 << 9,  // Compute position relative to the listener
	FLAG_AUTOFREE      = 1 << 10, // Free resource when playing is finished
};
DECLARE_FLAGS(ChannelFlag, ChannelFlags)
DECLARE_FLAGS_OPERATORS(ChannelFlags)

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

enum HRTFAttribute {
	HRTFDisable = 0,
	HRTFEnable = 1,
	HRTFDefault = -1,
};

enum HRTFStatus {
	HRTFDisabled,
	HRTFEnabled,
	HRTFRequired,
	HRTFForbidden,
	HRTFUnavailable
};

enum PlayingAmbianceType {
	PLAYING_AMBIANCE_MENU,
	PLAYING_AMBIANCE_SCRIPT,
	PLAYING_AMBIANCE_ZONE
};

// Output format
struct PCMFormat {
	u32 frequency; // Samples per second
	u16 quality; // Bits per sample
	u16 channels; // Output channels count
};

// Source falloff
struct SourceFalloff {
	
	float start;
	float end;
	
	SourceFalloff()
		: start(0.f)
		, end(0.f)
	{ }
	
};


typedef HandleType<struct SampleHandleTag,    s32, -1> SampleHandle;
typedef HandleType<struct SourceHandleTag,    s32, -1> SourceHandle;

struct SourcedSample {
	
	SourcedSample()
	{ }
	
	SourcedSample(SourceHandle source, SampleHandle sample)
		: m_source(source)
		, m_sample(sample)
	{ }
	
	bool operator==(const SourcedSample & rhs) const {
		return m_source == rhs.m_source && m_sample == rhs.m_sample;
	}
	bool operator!=(const SourcedSample & rhs) const {
		return m_source != rhs.m_source || m_sample != rhs.m_sample;
	}
	
	SourceHandle source() const {
		return m_source;
	}
	
	SampleHandle getSampleId() const {
		return m_sample;
	}
	
	void clearSource() {
		m_source = SourceHandle();
	}
	
private:
	
	SourceHandle m_source;
	SampleHandle m_sample;
	
};


typedef HandleType<struct MixerIdTag,    s32, -1> MixerId;
typedef HandleType<struct EnvIdTag,      s32, -1> EnvId;
typedef HandleType<struct AmbianceIdTag, s32, -1> AmbianceId;

// Play channel initialization parameters
struct Channel {
	
	ChannelFlags flags;
	MixerId mixer;
	float volume;
	float pitch;
	float pan;
	Vec3f position;
	Vec3f velocity;
	SourceFalloff falloff;
	
	explicit Channel(MixerId _mixer)
		: flags(0)
		, mixer(_mixer)
		, volume(0.f)
		, pitch(0.f)
		, pan(0.f)
		, position(0.f)
		, velocity(0.f)
	{ }
	
};

enum SourceStatus {
	Idle,
	Playing,
	Paused
};

} // namespace audio

#endif // ARX_AUDIO_AUDIOTYPES_H
