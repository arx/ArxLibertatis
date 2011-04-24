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
	
	// Setup
	aalError aalInit(bool enableEAX);
	aalError aalClean();
	aalError aalSetStreamLimit(const aalULong & size);
	aalError aalSetSamplePath(const std::string & path);
	aalError aalSetAmbiancePath(const std::string & path);
	aalError aalSetEnvironmentPath(const std::string & path);
	aalError setReverbEnabled(bool enable);
	aalError aalUpdate();
	
	// Resource
	
	// Creation
	aalSLong aalCreateMixer(const char * name = NULL);
	aalSLong aalCreateSample(const char * name = NULL);
	aalSLong aalCreateAmbiance(const char * name = NULL);
	aalSLong aalCreateEnvironment(const char * name = NULL);
	// Destruction
	aalError aalDeleteSample(aalSLong sample_id);
	aalError aalDeleteAmbiance(aalSLong ambiance_id);
	
	// Checking
	aalUBool aalIsEnvironment(aalSLong environment_id);
	// Retrieving by name (If resource_name == NULL, return first found)
	aalSLong aalGetMixer(const char * mixer_name = NULL);
	
	aalSLong aalGetAmbiance(const char * ambiance_name = NULL);
	aalSLong aalGetEnvironment(const std::string & environment_name);
	// Retrieving by ID (If resource_id == AAL_SFALSE, return first found)
	aalSLong aalGetNextAmbiance(aalSLong ambiance_id = AAL_SFALSE);
	
	// Environment
	
	aalError aalSetRoomRolloffFactor(float factor);
	
	// Listener
	
	aalError aalSetListenerName(char * name, const aalULong & max_char = AAL_DEFAULT_STRINGSIZE);
	aalError aalSetListenerUnitFactor(float factor);
	aalError aalSetListenerRolloffFactor(float factor);
	aalError aalSetListenerPosition(const aalVector & position);
	aalError aalSetListenerDirection(const aalVector & front, const aalVector & up);
	aalError aalSetListenerEnvironment(aalSLong environment_id);
	
	aalError aalGetListenerName(char * name, const aalULong & max_char = AAL_DEFAULT_STRINGSIZE);
	aalError aalGetListenerUnitFactor(aalFloat & factor);
	aalError aalGetListenerDopplerFactor(aalFloat & factor);
	aalError aalGetListenerRolloffFactor(aalFloat & factor);
	aalError aalGetListenerPosition(aalVector & position);
	aalError aalGetListenerDirection(aalVector & front, aalVector * up);
	aalError aalGetListenerVelocity(aalVector & velocity);
	aalError aalGetListenerEnvironment(aalSLong & environment_id);
	
	// Mixer
	
	// Setup
	aalError aalSetMixerVolume(aalSLong mixer_id, float volume);
	aalError aalSetMixerParent(aalSLong mixer_id, aalSLong parent_mixer_id);
	// Status
	aalError aalGetMixerVolume(aalSLong mixer_id, aalFloat * volume);
	aalUBool IsMixerEnabled(aalSLong mixer_id, const aalFlag & flag);
	aalUBool IsMixerDisabled(aalSLong mixer_id, const aalFlag & flag);
	aalUBool IsMixerPaused(aalSLong mixer_id);
	// Control
	aalError aalMixerStop(aalSLong mixer_id);
	aalError aalMixerPause(aalSLong mixer_id);
	aalError aalMixerResume(aalSLong mixer_id);
	
	// Sample
	
	// Setup
	aalError aalSetSampleVolume(aalSLong sample_id, float volume);
	aalError aalSetSamplePitch(aalSLong sample_id, float pitch);
	aalError aalSetSamplePosition(aalSLong sample_id, const aalVector & position);
	// Status
	aalError aalGetSampleName(aalSLong sample_id, char * name, const aalULong & max_char = AAL_DEFAULT_STRINGSIZE);
	aalError aalGetSampleLength(aalSLong sample_id, aalULong & length, const aalUnit & unit = AAL_UNIT_MS);
	aalError aalGetSamplePan(aalSLong sample_id, aalFloat * pan);
	aalError aalGetSampleCone(aalSLong sample_id, aalCone * cone);
	bool aalIsSamplePlaying(aalSLong sample_id);
	// Control
	//play_count == 0 -> infinite loop, play_count > 0 -> play play_count times
	aalError aalSamplePlay(aalSLong & sample_id, const aalChannel & channel, const aalULong & play_count = 1);
	aalError aalSampleStop(aalSLong & sample_id);
	
	// Track setup
	aalError aalMuteAmbianceTrack(aalSLong ambiance_id, aalSLong track_id, const aalUBool & mute);
	
	// Ambiance
	
	// Setup
	aalError aalSetAmbianceUserData(aalSLong ambiance_id, void * data);
	aalError aalSetAmbianceVolume(aalSLong ambiance_id, float volume);
	// Status
	aalError aalGetAmbianceName(aalSLong ambiance_id, char * name, const aalULong & max_char = AAL_DEFAULT_STRINGSIZE);
	aalError aalGetAmbianceUserData(aalSLong ambiance_id, void ** data);
	aalError aalGetAmbianceTrackID(aalSLong ambiance_id, const char * name, aalSLong & track_id);
	aalError aalGetAmbianceVolume(aalSLong ambiance_id, aalFloat & volume);
	aalUBool aalIsAmbianceLooped(aalSLong ambiance_id);
	// Control
	//play_count == 0 -> infinite loop, play_count == 1 -> play once
	aalError aalAmbiancePlay(aalSLong ambiance_id, const aalChannel & channel, const aalULong & play_count = 1, const aalULong & fade_interval = 0);
	aalError aalAmbianceStop(aalSLong ambiance_id, const aalULong & fade_interval = 0);
	
} // namespace audio

#endif // ARX_AUDIO_AUDIO_H
