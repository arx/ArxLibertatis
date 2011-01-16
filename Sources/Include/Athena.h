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
#ifndef __ATHENA_H__
#define __ATHENA_H__

#define __ATHENA_VERSION__ "0000"

#include <Athena_Types.h>

#ifndef AAL_APIDLL
#define AAL_APIFUNC __declspec(dllimport)
#else
#define AAL_APIFUNC __declspec(dllexport)
#endif

namespace ATHENA
{

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Athena Audio Library                                                      //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Global                                                                    //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	// Setup                                                                     //
	AAL_APIFUNC aalError aalInit(aalVoid * param = NULL);
	AAL_APIFUNC aalError aalInitForceNoEAX(aalVoid * param = NULL);
	AAL_APIFUNC aalError aalClean();
	AAL_APIFUNC aalError aalSetStreamLimit(const aalULong & size);
	AAL_APIFUNC aalError aalSetOutputFormat(const aalFormat & format);
	AAL_APIFUNC aalError aalAddResourcePack(const char * name);
	AAL_APIFUNC aalError aalSetRootPath(const char * path);
	AAL_APIFUNC aalError aalSetSamplePath(const char * path);
	AAL_APIFUNC aalError aalSetAmbiancePath(const char * path);
	AAL_APIFUNC aalError aalSetEnvironmentPath(const char * path);
	AAL_APIFUNC aalError aalEnable(const aalULong & flags);
	AAL_APIFUNC aalError aalDisable(const aalULong & flags);
	// Status                                                                    //
	AAL_APIFUNC aalError aalGetStreamLimit(aalULong & limit);
	AAL_APIFUNC aalUBool aalIsEnabled(const aalFlag & flag);
	// Control                                                                   //
	AAL_APIFUNC aalError aalUpdate();

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Resource                                                                  //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	// Creation                                                                  //
	AAL_APIFUNC aalSLong aalCreateMixer(const char * name = NULL);
	AAL_APIFUNC aalSLong aalCreateSample(const char * name = NULL);
	AAL_APIFUNC aalSLong aalCreateAmbiance(const char * name = NULL);
	AAL_APIFUNC aalSLong aalCreateEnvironment(const char * name = NULL);
	// Destruction                                                               // 
	AAL_APIFUNC aalError aalDeleteSample(const aalSLong & sample_id);
	AAL_APIFUNC aalError aalDeleteAmbiance(const aalSLong & ambiance_id);
 
	// Checking                                                                  //
	AAL_APIFUNC aalUBool aalIsEnvironment(const aalSLong & environment_id);
	// Retrieving by name (If resource_name == NULL, return first found)         //
	AAL_APIFUNC aalSLong aalGetMixer(const char * mixer_name = NULL);
 
	AAL_APIFUNC aalSLong aalGetAmbiance(const char * ambiance_name = NULL);
	AAL_APIFUNC aalSLong aalGetEnvironment(const char * environment_name = NULL);
	// Retrieving by ID (If resource_id == AAL_SFALSE, return first found)       //
	AAL_APIFUNC aalSLong aalGetNextAmbiance(const aalSLong & ambiance_id = AAL_SFALSE);
 

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Environment                                                               //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	// Setup                                                                     //
	AAL_APIFUNC aalError aalSetEnvironmentRolloffFactor(const aalSLong & environment_id, const aalFloat & factor);
	// Status                                                                    //
	AAL_APIFUNC aalError aalGetEnvironmentSize(const aalSLong & environment_id, aalFloat & size);
	AAL_APIFUNC aalError aalGetEnvironmentRolloffFactor(const aalSLong & environment_id, aalFloat & factor);
	AAL_APIFUNC aalError aalGetEnvironmentDiffusion(const aalSLong & environment_id, aalFloat & diffusion);
	AAL_APIFUNC aalError aalGetEnvironmentAbsorption(const aalSLong & environment_id, aalFloat & absorption);
	AAL_APIFUNC aalError aalGetEnvironmentReflection(const aalSLong & environment_id, aalReflection & reflection);
	AAL_APIFUNC aalError aalGetEnvironmentReverberation(const aalSLong & environment_id, aalReverberation & reverberation);
	AAL_APIFUNC aalError aalGetEnvironmentCallback(const aalSLong & environment_id, aalEnvironmentCallback & callback);

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Listener                                                                  //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	// Setup                                                                     //
	AAL_APIFUNC aalError aalSetListenerName(char * name, const aalULong & max_char = AAL_DEFAULT_STRINGSIZE);
	AAL_APIFUNC aalError aalSetListenerUnitFactor(const aalFloat & factor);
	AAL_APIFUNC aalError aalSetListenerRolloffFactor(const aalFloat & factor);
	AAL_APIFUNC aalError aalSetListenerPosition(const aalVector & position);
	AAL_APIFUNC aalError aalSetListenerDirection(const aalVector & front, const aalVector & up);
	AAL_APIFUNC aalError aalSetListenerEnvironment(const aalSLong & environment_id);
	// Status                                                                    //
	AAL_APIFUNC aalError aalGetListenerName(char * name, const aalULong & max_char = AAL_DEFAULT_STRINGSIZE);
	AAL_APIFUNC aalError aalGetListenerUnitFactor(aalFloat & factor);
	AAL_APIFUNC aalError aalGetListenerDopplerFactor(aalFloat & factor);
	AAL_APIFUNC aalError aalGetListenerRolloffFactor(aalFloat & factor);
	AAL_APIFUNC aalError aalGetListenerPosition(aalVector & position);
	AAL_APIFUNC aalError aalGetListenerDirection(aalVector & front, aalVector * up);
	AAL_APIFUNC aalError aalGetListenerVelocity(aalVector & velocity);
	AAL_APIFUNC aalError aalGetListenerEnvironment(aalSLong & environment_id);

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Mixer                                                                     //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	// Setup                                                                     //
	AAL_APIFUNC aalError aalSetMixerVolume(const aalSLong & mixer_id, const aalFloat & volume);
	AAL_APIFUNC aalError aalSetMixerParent(const aalSLong & mixer_id, const aalSLong & parent_mixer_id);
	// Status                                                                    //
	AAL_APIFUNC aalError aalGetMixerVolume(const aalSLong & mixer_id, aalFloat * volume);
	AAL_APIFUNC aalUBool IsMixerEnabled(const aalSLong & mixer_id, const aalFlag & flag);
	AAL_APIFUNC aalUBool IsMixerDisabled(const aalSLong & mixer_id, const aalFlag & flag);
	AAL_APIFUNC aalUBool IsMixerPaused(const aalSLong & mixer_id);
	// Control                                                                   //
	AAL_APIFUNC aalError aalMixerStop(const aalSLong & mixer_id);
	AAL_APIFUNC aalError aalMixerPause(const aalSLong & mixer_id);
	AAL_APIFUNC aalError aalMixerResume(const aalSLong & mixer_id);

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Sample                                                                    //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	// Setup                                                                     //
	AAL_APIFUNC aalError aalSetSampleVolume(const aalSLong & sample_id, const aalFloat & volume);
	AAL_APIFUNC aalError aalSetSamplePitch(const aalSLong & sample_id, const aalFloat & pitch);
	AAL_APIFUNC aalError aalSetSamplePosition(const aalSLong & sample_id, const aalVector & position);
	// Status                                                                    //
	AAL_APIFUNC aalError aalGetSampleName(const aalSLong & sample_id, char * name, const aalULong & max_char = AAL_DEFAULT_STRINGSIZE);
	AAL_APIFUNC aalError aalGetSampleLength(const aalSLong & sample_id, aalULong & length, const aalUnit & unit = AAL_UNIT_MS);
	AAL_APIFUNC aalError aalGetSamplePan(const aalSLong & sample_id, aalFloat * pan);
	AAL_APIFUNC aalError aalGetSampleCone(const aalSLong & sample_id, aalCone * cone);
	AAL_APIFUNC aalUBool aalIsSamplePlaying(const aalSLong & sample_id);
	// Control                                                                   //
	//play_count == 0 -> infinite loop, play_count > 0 -> play play_count times
	AAL_APIFUNC aalError aalSamplePlay(aalSLong & sample_id, const aalChannel & channel, const aalULong & play_count = 1);
	AAL_APIFUNC aalError aalSampleStop(aalSLong & sample_id);
 
	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Track                                                                     //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	// Setup                                                                     //
	AAL_APIFUNC aalError aalMuteAmbianceTrack(const aalSLong & ambiance_id, const aalSLong & track_id, const aalUBool & mute);

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Ambiance                                                                  //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	// Setup                                                                     //
	AAL_APIFUNC aalError aalSetAmbianceUserData(const aalSLong & ambiance_id, aalVoid * data);
	AAL_APIFUNC aalError aalSetAmbianceVolume(const aalSLong & ambiance_id, const aalFloat & volume);
	// Status                                                                    //
	AAL_APIFUNC aalError aalGetAmbianceName(const aalSLong & ambiance_id, char * name, const aalULong & max_char = AAL_DEFAULT_STRINGSIZE);
	AAL_APIFUNC aalError aalGetAmbianceUserData(const aalSLong & ambiance_id, aalVoid ** data);
	AAL_APIFUNC aalError aalGetAmbianceTrackID(const aalSLong & ambiance_id, const char * name, aalSLong & track_id);
	AAL_APIFUNC aalError aalGetAmbianceVolume(const aalSLong & ambiance_id, aalFloat & volume);
	AAL_APIFUNC aalUBool aalIsAmbianceLooped(const aalSLong & ambiance_id);
	// Control                                                                   //
	//play_count == 0 -> infinite loop, play_count == 1 -> play once
	AAL_APIFUNC aalError aalAmbiancePlay(const aalSLong & ambiance_id, const aalChannel & channel, const aalULong & play_count = 1, const aalULong & fade_interval = 0);
	AAL_APIFUNC aalError aalAmbianceStop(const aalSLong & ambiance_id, const aalULong & fade_interval = 0);

}//ATHENA::

#endif//__ATHENA_H__
