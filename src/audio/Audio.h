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

#ifndef ARX_AUDIO_AUDIO_H
#define ARX_AUDIO_AUDIO_H

#include "AudioTypes.h"

namespace audio {

// Global

/*!
 * Initialize the audio system.
 * If the audio system was already initialized, it is cleaned first, removing all loaded resources.
 * This is not threadsafe: The caller must ensure that no other audio methods are called at the same time.
 */
aalError aalInit(const std::string & backend, bool enableEAX);

/*!
 * Cleanup the audio system.
 * This is not threadsafe: The caller must ensure that no other audio methods are called at the same time.
 */
aalError aalClean();
aalError aalSetStreamLimit(size_t size);
aalError aalSetSamplePath(const std::string & path);
aalError aalSetAmbiancePath(const std::string & path);
aalError aalSetEnvironmentPath(const std::string & path);
aalError setReverbEnabled(bool enable);
aalError aalUpdate();

// Resource

MixerId aalCreateMixer();
SampleId aalCreateSample(const std::string & name);
AmbianceId aalCreateAmbiance(const std::string & name);
EnvId aalCreateEnvironment(const std::string & name);
aalError aalDeleteSample(SampleId sample_id);
aalError aalDeleteAmbiance(AmbianceId ambiance_id);

AmbianceId aalGetAmbiance(const std::string & ambiance_name);
EnvId aalGetEnvironment(const std::string & environment_name);

//! Retrieving by ID (If resource_id == INVALID_ID, return first found)
AmbianceId aalGetNextAmbiance(AmbianceId ambiance_id = INVALID_ID);

// Environment

aalError aalSetRoomRolloffFactor(float factor);

// Listener

aalError aalSetUnitFactor(float factor);
aalError aalSetRolloffFactor(float factor);
aalError aalSetListenerPosition(const Vec3f & position);
aalError aalSetListenerDirection(const Vec3f & front, const Vec3f & up);
aalError aalSetListenerEnvironment(EnvId environment_id);

// Mixer

aalError aalSetMixerVolume(MixerId mixer_id, float volume);
aalError aalSetMixerParent(MixerId mixer_id, MixerId parent_mixer_id);

aalError aalGetMixerVolume(MixerId mixer_id, float * volume);

aalError aalMixerStop(MixerId mixer_id);
aalError aalMixerPause(MixerId mixer_id);
aalError aalMixerResume(MixerId mixer_id);

// Sample

aalError aalSetSampleVolume(SourceId sample_id, float volume);
aalError aalSetSamplePitch(SourceId sample_id, float pitch);
aalError aalSetSamplePosition(SourceId sample_id, const Vec3f & position);

aalError aalGetSampleName(SampleId sample_id, std::string & name);
aalError aalGetSampleLength(SampleId sample_id, size_t & length, TimeUnit unit = UNIT_MS);
aalError aalGetSamplePan(SourceId sample_id, float * pan);
aalError aalGetSampleCone(SourceId sample_id, SourceCone * cone);
bool aalIsSamplePlaying(SourceId sample_id);

//! play_count == 0 -> infinite loop, play_count > 0 -> play play_count times
aalError aalSamplePlay(SampleId & sample_id, const Channel & channel, unsigned play_count = 1);
aalError aalSampleStop(SourceId & sample_id);

// Ambiance

aalError aalMuteAmbianceTrack(AmbianceId ambiance_id, const std::string & track, bool mute);

aalError aalSetAmbianceUserData(AmbianceId ambiance_id, void * data);
aalError aalSetAmbianceVolume(AmbianceId ambiance_id, float volume);

aalError aalGetAmbianceName(AmbianceId ambiance_id, std::string & name);
aalError aalGetAmbianceUserData(AmbianceId ambiance_id, void ** data);
aalError aalGetAmbianceVolume(AmbianceId ambiance_id, float & volume);
bool aalIsAmbianceLooped(AmbianceId ambiance_id);

//! play_count == 0 -> infinite loop, play_count == 1 -> play once
aalError aalAmbiancePlay(AmbianceId ambiance_id, const Channel & channel, bool loop = false, size_t fade_interval = 0);
aalError aalAmbianceStop(AmbianceId ambiance_id, size_t fade_interval = 0);

} // namespace audio

#endif // ARX_AUDIO_AUDIO_H
