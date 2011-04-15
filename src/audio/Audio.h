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

namespace ATHENA {
	
	// Global
	
	// Setup
	aalError aalInit();
	aalError aalInitForceNoEAX();
	aalError aalClean();
	aalError aalSetStreamLimit(const aalULong & size);
	aalError aalSetOutputFormat(const aalFormat & format);
	aalError aalSetSamplePath(const char * path);
	aalError aalSetAmbiancePath(const char * path);
	aalError aalSetEnvironmentPath(const char * path);
	aalError aalEnable(const aalULong & flags);
	aalError aalDisable(const aalULong & flags);
	// Status
	aalError aalGetStreamLimit(aalULong & limit);
	aalUBool aalIsEnabled(const aalFlag & flag);
	// Control
	aalError aalUpdate();
	
	// Resource
	
	// Creation
	aalSLong aalCreateMixer(const char * name = NULL);
	aalSLong aalCreateSample(const char * name = NULL);
	aalSLong aalCreateAmbiance(const char * name = NULL);
	aalSLong aalCreateEnvironment(const char * name = NULL);
	// Destruction
	aalError aalDeleteSample(const aalSLong & sample_id);
	aalError aalDeleteAmbiance(const aalSLong & ambiance_id);
	
	// Checking
	aalUBool aalIsEnvironment(const aalSLong & environment_id);
	// Retrieving by name (If resource_name == NULL, return first found)
	aalSLong aalGetMixer(const char * mixer_name = NULL);
	
	aalSLong aalGetAmbiance(const char * ambiance_name = NULL);
	aalSLong aalGetEnvironment(const char * environment_name = NULL);
	// Retrieving by ID (If resource_id == AAL_SFALSE, return first found)
	aalSLong aalGetNextAmbiance(const aalSLong & ambiance_id = AAL_SFALSE);
	
	// Environment
	
	// Setup
	aalError aalSetEnvironmentRolloffFactor(const aalSLong & environment_id, const aalFloat & factor);
	// Status
	aalError aalGetEnvironmentSize(const aalSLong & environment_id, aalFloat & size);
	aalError aalGetEnvironmentRolloffFactor(const aalSLong & environment_id, aalFloat & factor);
	aalError aalGetEnvironmentDiffusion(const aalSLong & environment_id, aalFloat & diffusion);
	aalError aalGetEnvironmentAbsorption(const aalSLong & environment_id, aalFloat & absorption);
	aalError aalGetEnvironmentReflection(const aalSLong & environment_id, aalReflection & reflection);
	aalError aalGetEnvironmentReverberation(const aalSLong & environment_id, aalReverberation & reverberation);
	aalError aalGetEnvironmentCallback(const aalSLong & environment_id, aalEnvironmentCallback & callback);
	
	// Listener
	
	// Setup
	aalError aalSetListenerName(char * name, const aalULong & max_char = AAL_DEFAULT_STRINGSIZE);
	aalError aalSetListenerUnitFactor(const aalFloat & factor);
	aalError aalSetListenerRolloffFactor(const aalFloat & factor);
	aalError aalSetListenerPosition(const aalVector & position);
	aalError aalSetListenerDirection(const aalVector & front, const aalVector & up);
	aalError aalSetListenerEnvironment(const aalSLong & environment_id);
	// Status
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
	aalError aalSetMixerVolume(const aalSLong & mixer_id, const aalFloat & volume);
	aalError aalSetMixerParent(const aalSLong & mixer_id, const aalSLong & parent_mixer_id);
	// Status
	aalError aalGetMixerVolume(const aalSLong & mixer_id, aalFloat * volume);
	aalUBool IsMixerEnabled(const aalSLong & mixer_id, const aalFlag & flag);
	aalUBool IsMixerDisabled(const aalSLong & mixer_id, const aalFlag & flag);
	aalUBool IsMixerPaused(const aalSLong & mixer_id);
	// Control
	aalError aalMixerStop(const aalSLong & mixer_id);
	aalError aalMixerPause(const aalSLong & mixer_id);
	aalError aalMixerResume(const aalSLong & mixer_id);
	
	// Sample
	
	// Setup
	aalError aalSetSampleVolume(const aalSLong & sample_id, const aalFloat & volume);
	aalError aalSetSamplePitch(const aalSLong & sample_id, const aalFloat & pitch);
	aalError aalSetSamplePosition(const aalSLong & sample_id, const aalVector & position);
	// Status
	aalError aalGetSampleName(const aalSLong & sample_id, char * name, const aalULong & max_char = AAL_DEFAULT_STRINGSIZE);
	aalError aalGetSampleLength(const aalSLong & sample_id, aalULong & length, const aalUnit & unit = AAL_UNIT_MS);
	aalError aalGetSamplePan(const aalSLong & sample_id, aalFloat * pan);
	aalError aalGetSampleCone(const aalSLong & sample_id, aalCone * cone);
	aalUBool aalIsSamplePlaying(const aalSLong & sample_id);
	// Control
	//play_count == 0 -> infinite loop, play_count > 0 -> play play_count times
	aalError aalSamplePlay(aalSLong & sample_id, const aalChannel & channel, const aalULong & play_count = 1);
	aalError aalSampleStop(aalSLong & sample_id);
	
	// Track setup
	aalError aalMuteAmbianceTrack(const aalSLong & ambiance_id, const aalSLong & track_id, const aalUBool & mute);
	
	// Ambiance
	
	// Setup
	aalError aalSetAmbianceUserData(const aalSLong & ambiance_id, void * data);
	aalError aalSetAmbianceVolume(const aalSLong & ambiance_id, const aalFloat & volume);
	// Status
	aalError aalGetAmbianceName(const aalSLong & ambiance_id, char * name, const aalULong & max_char = AAL_DEFAULT_STRINGSIZE);
	aalError aalGetAmbianceUserData(const aalSLong & ambiance_id, void ** data);
	aalError aalGetAmbianceTrackID(const aalSLong & ambiance_id, const char * name, aalSLong & track_id);
	aalError aalGetAmbianceVolume(const aalSLong & ambiance_id, aalFloat & volume);
	aalUBool aalIsAmbianceLooped(const aalSLong & ambiance_id);
	// Control
	//play_count == 0 -> infinite loop, play_count == 1 -> play once
	aalError aalAmbiancePlay(const aalSLong & ambiance_id, const aalChannel & channel, const aalULong & play_count = 1, const aalULong & fade_interval = 0);
	aalError aalAmbianceStop(const aalSLong & ambiance_id, const aalULong & fade_interval = 0);
	
} // namespace ATHENA

#endif // ARX_AUDIO_AUDIO_H
