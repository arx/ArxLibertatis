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

aalError aalInit(bool enableEAX);
aalError aalClean();
aalError aalSetStreamLimit(aalULong size);
aalError aalSetSamplePath(const std::string & path);
aalError aalSetAmbiancePath(const std::string & path);
aalError aalSetEnvironmentPath(const std::string & path);
aalError setReverbEnabled(bool enable);
aalError aalUpdate();

// Resource

aalSLong aalCreateMixer();
aalSLong aalCreateSample(const std::string & name);
aalSLong aalCreateAmbiance(const std::string & name);
aalSLong aalCreateEnvironment(const std::string & name);
aalError aalDeleteSample(aalSLong sample_id);
aalError aalDeleteAmbiance(aalSLong ambiance_id);

aalSLong aalGetAmbiance(const std::string & ambiance_name);
aalSLong aalGetEnvironment(const std::string & environment_name);

//! Retrieving by ID (If resource_id == AAL_SFALSE, return first found)
aalSLong aalGetNextAmbiance(aalSLong ambiance_id = AAL_SFALSE);

// Environment

aalError aalSetRoomRolloffFactor(float factor);

// Listener

aalError aalSetListenerUnitFactor(float factor);
aalError aalSetListenerRolloffFactor(float factor);
aalError aalSetListenerPosition(const aalVector & position);
aalError aalSetListenerDirection(const aalVector & front, const aalVector & up);
aalError aalSetListenerEnvironment(aalSLong environment_id);

// Mixer

aalError aalSetMixerVolume(aalSLong mixer_id, float volume);
aalError aalSetMixerParent(aalSLong mixer_id, aalSLong parent_mixer_id);

aalError aalGetMixerVolume(aalSLong mixer_id, float * volume);
bool IsMixerEnabled(aalSLong mixer_id, const aalFlag & flag);
bool IsMixerDisabled(aalSLong mixer_id, const aalFlag & flag);
bool IsMixerPaused(aalSLong mixer_id);

aalError aalMixerStop(aalSLong mixer_id);
aalError aalMixerPause(aalSLong mixer_id);
aalError aalMixerResume(aalSLong mixer_id);

// Sample

aalError aalSetSampleVolume(aalSLong sample_id, float volume);
aalError aalSetSamplePitch(aalSLong sample_id, float pitch);
aalError aalSetSamplePosition(aalSLong sample_id, const aalVector & position);

aalError aalGetSampleName(aalSLong sample_id, std::string & name);
aalError aalGetSampleLength(aalSLong sample_id, aalULong & length, aalUnit unit = AAL_UNIT_MS);
aalError aalGetSamplePan(aalSLong sample_id, float * pan);
aalError aalGetSampleCone(aalSLong sample_id, aalCone * cone);
bool aalIsSamplePlaying(aalSLong sample_id);

//! play_count == 0 -> infinite loop, play_count > 0 -> play play_count times
aalError aalSamplePlay(aalSLong & sample_id, const aalChannel & channel, aalULong play_count = 1);
aalError aalSampleStop(aalSLong & sample_id);

// Ambiance

aalError aalMuteAmbianceTrack(aalSLong ambiance_id, const std::string & track, bool mute);

aalError aalSetAmbianceUserData(aalSLong ambiance_id, void * data);
aalError aalSetAmbianceVolume(aalSLong ambiance_id, float volume);

aalError aalGetAmbianceName(aalSLong ambiance_id, std::string & name);
aalError aalGetAmbianceUserData(aalSLong ambiance_id, void ** data);
aalError aalGetAmbianceVolume(aalSLong ambiance_id, float & volume);
bool aalIsAmbianceLooped(aalSLong ambiance_id);

//! play_count == 0 -> infinite loop, play_count == 1 -> play once
aalError aalAmbiancePlay(aalSLong ambiance_id, const aalChannel & channel, bool loop = false, aalULong fade_interval = 0);
aalError aalAmbianceStop(aalSLong ambiance_id, aalULong fade_interval = 0);

} // namespace audio

#endif // ARX_AUDIO_AUDIO_H
