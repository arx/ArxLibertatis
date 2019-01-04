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

#ifndef ARX_AUDIO_AUDIO_H
#define ARX_AUDIO_AUDIO_H

#include <stddef.h>
#include <string>
#include <vector>

#include "core/TimeTypes.h"
#include "audio/AudioTypes.h"
#include "io/resource/ResourcePath.h"
#include "math/Types.h"

namespace res { class path; }

namespace audio {

// Global

/*!
 * Initialize the audio system.
 * If the audio system was already initialized, it is cleaned first, removing all loaded resources.
 * This is not threadsafe: The caller must ensure that no other audio methods are called at the same time.
 */
aalError init(const std::string & backendName, const std::string & deviceName = std::string(),
              HRTFAttribute hrtf = HRTFDefault);

/*!
 * Get a list of available devices for the current backend.
 */
std::vector<std::string> getDevices();

/*!
 * Cleanup the audio system.
 * This is not threadsafe: The caller must ensure that no other audio methods are called at the same time.
 */
void clean();
void setAmbiancePath(const res::path & path);
void setEnvironmentPath(const res::path & path);
void setReverbEnabled(bool enable);
bool isReverbSupported();
void setHRTFEnabled(HRTFAttribute enable);
HRTFStatus getHRTFStatus();

// Resource

MixerId createMixer();
MixerId createMixer(MixerId parent);
SampleHandle createSample(const res::path & name);
AmbianceId createAmbiance(const res::path & name, PlayingAmbianceType type);
EnvId createEnvironment(const res::path & name);
void deleteSample(SampleHandle sampleHandle);
void deleteAmbiance(AmbianceId ambianceId);
void deleteAmbianceAll();

AmbianceId getAmbiance(const res::path & name);
EnvId getEnvironment(const res::path & name);

// Listener

void setUnitFactor(float factor);
void setRolloffFactor(float factor);
void setListenerPosition(const Vec3f & position, const Vec3f & front, const Vec3f & up);
void setListenerEnvironment(EnvId environmentId);

// Mixer

void setMixerVolume(MixerId mixerId, float volume);

void mixerStop(MixerId mixerId);
void mixerPause(MixerId mixerId);
void mixerResume(MixerId mixerId);

// Sample

void setSampleVolume(SourcedSample sourceId, float volume);
void setSamplePitch(SourcedSample sourceId, float pitch);
void setSamplePosition(SourcedSample sourceId, const Vec3f & position);

aalError getSampleName(SampleHandle sampleHandle, res::path & name);
aalError getSampleLength(SampleHandle sampleHandle, size_t & length);
bool isSamplePlaying(SourcedSample sourceId);

//! play_count == 0 -> infinite loop, play_count > 0 -> play play_count times
SourcedSample samplePlay(SampleHandle sampleHandle, const Channel & channel, unsigned playCount = 1);
void sampleStop(SourcedSample sourceId);

struct SourceInfo {
	SourceHandle source;
	SourceStatus status;
	SampleHandle sample;
	std::string sampleName;
};

void getSourceInfos(std::vector<SourceInfo> & infos);

// Ambiance

void setAmbianceVolume(AmbianceId ambianceId, float volume);

void ambiancePlay(AmbianceId ambianceId, const Channel & channel, bool loop = false, PlatformDuration fadeInterval = 0);
void ambianceStop(AmbianceId ambianceId, PlatformDuration fadeInterval = 0);

struct AmbianceInfo {
	res::path name;
	PlayingAmbianceType type;
	float volume;
	bool isLooped;
};

void getAmbianceInfos(std::vector<AmbianceInfo> & infos);


void threadStart();
void threadStop();

} // namespace audio

#endif // ARX_AUDIO_AUDIO_H
