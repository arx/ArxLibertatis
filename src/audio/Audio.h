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

#ifndef ARX_AUDIO_AUDIO_H
#define ARX_AUDIO_AUDIO_H

#include <stddef.h>
#include <string>

#include "audio/AudioTypes.h"
#include "math/Types.h"

namespace res { class path; }

namespace audio {

// Global

/*!
 * Initialize the audio system.
 * If the audio system was already initialized, it is cleaned first, removing all loaded resources.
 * This is not threadsafe: The caller must ensure that no other audio methods are called at the same time.
 */
aalError init(const std::string & backend, bool enableEAX);

/*!
 * Cleanup the audio system.
 * This is not threadsafe: The caller must ensure that no other audio methods are called at the same time.
 */
aalError clean();
aalError setStreamLimit(size_t size);
aalError setSamplePath(const res::path & path);
aalError setAmbiancePath(const res::path & path);
aalError setEnvironmentPath(const res::path & path);
aalError setReverbEnabled(bool enable);
aalError update();

// Resource

MixerId createMixer();
SampleId createSample(const res::path & name);
AmbianceId createAmbiance(const res::path & name);
EnvId createEnvironment(const res::path & name);
aalError deleteSample(SampleId sample_id);
aalError deleteAmbiance(AmbianceId ambiance_id);

AmbianceId getAmbiance(const res::path & ambiance_name);
EnvId getEnvironment(const res::path & environment_name);

//! Retrieving by ID (If resource_id == INVALID_ID, return first found)
AmbianceId getNextAmbiance(AmbianceId ambiance_id = INVALID_ID);

// Environment

aalError setRoomRolloffFactor(float factor);

// Listener

aalError setUnitFactor(float factor);
aalError setRolloffFactor(float factor);
aalError setListenerPosition(const Vec3f & position);
aalError setListenerDirection(const Vec3f & front, const Vec3f & up);
aalError setListenerEnvironment(EnvId environment_id);

// Mixer

aalError setMixerVolume(MixerId mixer_id, float volume);
aalError setMixerParent(MixerId mixer_id, MixerId parent_mixer_id);

aalError getMixerVolume(MixerId mixer_id, float * volume);

aalError mixerStop(MixerId mixer_id);
aalError mixerPause(MixerId mixer_id);
aalError mixerResume(MixerId mixer_id);

// Sample

aalError setSampleVolume(SourceId sample_id, float volume);
aalError setSamplePitch(SourceId sample_id, float pitch);
aalError setSamplePosition(SourceId sample_id, const Vec3f & position);

aalError getSampleName(SampleId sample_id, res::path & name);
aalError getSampleLength(SampleId sample_id, size_t & length, TimeUnit unit = UNIT_MS);
aalError getSamplePan(SourceId sample_id, float * pan);
aalError getSampleCone(SourceId sample_id, SourceCone * cone);
bool isSamplePlaying(SourceId sample_id);

//! play_count == 0 -> infinite loop, play_count > 0 -> play play_count times
aalError samplePlay(SampleId & sample_id, const Channel & channel, unsigned play_count = 1);
aalError sampleStop(SourceId & sample_id);

// Ambiance

aalError muteAmbianceTrack(AmbianceId ambiance_id, const std::string & track, bool mute);

aalError setAmbianceUserData(AmbianceId ambiance_id, void * data);
aalError setAmbianceVolume(AmbianceId ambiance_id, float volume);

aalError getAmbianceName(AmbianceId ambiance_id, res::path & name);
aalError getAmbianceUserData(AmbianceId ambiance_id, void ** data);
aalError getAmbianceVolume(AmbianceId ambiance_id, float & volume);
bool isAmbianceLooped(AmbianceId ambiance_id);

//! play_count == 0 -> infinite loop, play_count == 1 -> play once
aalError ambiancePlay(AmbianceId ambiance_id, const Channel & channel, bool loop = false, size_t fade_interval = 0);
aalError ambianceStop(AmbianceId ambiance_id, size_t fade_interval = 0);

} // namespace audio

#endif // ARX_AUDIO_AUDIO_H
