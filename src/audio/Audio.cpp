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
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// TODO                                                                      //
//                                                                           //
// Finish reverb implementation                                              //
// Keep finished instances a while before deleting in case we need it again  //
// Abstract driver API for testing other libs than DirectSound               //
// Finish ASF format implementation                                          //
//                                                                           //
// Ambiance                                                                  //
// Make sure global 3D localisation and multiple keys / track works properly //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "audio/Audio.h"

#include <cmath>
#include <cstring>

#include <AL/al.h>

#include "audio/AudioResource.h"
#include "audio/Mixer.h"
#include "audio/Sample.h"
#include "audio/Ambient.h"
#include "audio/AudioInstance.h"
#include "audio/AudioGlobal.h"
#include "audio/Stream.h"
#include "audio/Lock.h"

#include "io/Logger.h"

namespace ATHENA {

//Multithread data

Lock * mutex = NULL;

static const aalULong MUTEX_TIMEOUT(500);
static aalError EnableEnvironmentalAudio();

static struct timespec start_timespec;

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Global setup                                                              //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	aalError aalInit(void * param)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		//Clean any initialized data
		aalClean();

		//Initialize random number generator
		InitSeed();

		// Create OpenAL interface
		device = alcOpenDevice(NULL);
		if(!device) {
			if(mutex) {
				mutex->unlock();
			}
			return AAL_ERROR_SYSTEM;
		}
		
		context = alcCreateContext(device, NULL);
		alcMakeContextCurrent(context);
		
		alGenEffects = (LPALGENEFFECTS)alGetProcAddress("alGenEffects");
		alDeleteEffects = (LPALDELETEEFFECTS)alGetProcAddress("alDeleteEffects");
		alEffectf = (LPALEFFECTF)alGetProcAddress("alEffectf");
		if(!alGenEffects || !alDeleteEffects || !alEffectf) {
			LogError << "Missing OpenAL EFX extension.";
			if(mutex) {
				mutex->unlock();
			}
			return AAL_ERROR_SYSTEM;
		}
		
		alGetError(); // clear error code
		alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
		is_reverb_present = alcIsExtensionPresent(device, "ALC_EXT_EFX") ? AAL_UTRUE : AAL_UFALSE;
		
		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	aalError aalInitForceNoEAX(void * param)
	{
		aalError ret = aalInit(param);
		is_reverb_present = AAL_UFALSE;

		return ret;
	}

	aalError aalClean()
	{
		if (mutex) mutex->lock(MUTEX_TIMEOUT);

		_mixer.Clean();
		_amb.Clean();
		_inst.Clean();
		_sample.Clean();
		_env.Clean();

		if (context) alcDestroyContext(context), context = NULL;

		if (device) alcCloseDevice(device), device = NULL;

		free(sample_path), sample_path = NULL;
		free(ambiance_path), ambiance_path = NULL;
		free(environment_path), environment_path = NULL;
		stream_limit_ms = AAL_DEFAULT_STREAMLIMIT;
		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		session_start = (ts.tv_sec * 1000000000 + ts.tv_nsec) / 1000000; // session_start shouldn't really be used any more
		start_timespec.tv_sec = ts.tv_sec;
		start_timespec.tv_nsec = ts.tv_nsec;
		session_time = 0;
		is_reverb_present = AAL_UFALSE;

		if (mutex) delete mutex, mutex = NULL;

		return AAL_OK;
	}

	aalError aalSetOutputFormat(const aalFormat & f)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		alDeleteBuffers(1, primary);

		//Get new buffer and set its format
		alGenBuffers(1, primary);
		if (alGetError() != AL_NO_ERROR) {
			if (mutex) mutex->unlock();
			return AAL_ERROR_SYSTEM;
		}

		global_format.frequency = f.frequency;
		global_format.quality = f.quality;
		global_format.channels = f.channels;

		stream_limit_bytes = UnitsToBytes(stream_limit_ms, global_format, AAL_UNIT_MS);

		//Must restore all buffers here?!

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	aalError aalSetStreamLimit(const aalULong & limit)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		stream_limit_ms = limit < 500 ? 500 : limit;
		stream_limit_bytes = UnitsToBytes(stream_limit_ms, global_format, AAL_UNIT_MS);

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	aalError aalSetSamplePath(const char * _path)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		if (_path)
		{
			void * ptr;
			const char * temp = _path;
			aalULong len(strlen(_path) + 1);

			if (len <= 1) free(sample_path), sample_path = NULL;
			else
			{
				ptr = realloc(sample_path, len);

				if (!ptr)
				{
					if (mutex) mutex->unlock();

					return AAL_ERROR_MEMORY;
				}

				sample_path = (char *)ptr;

				memcpy(sample_path, temp, len);

				if (len && sample_path[len - 2] != '\\') strcat(sample_path, "\\");
			}
		}
		else free(sample_path), sample_path = NULL;

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	aalError aalSetAmbiancePath(const char * _path)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		if (_path)
		{
			void * ptr;
			const char * temp = _path;
			aalULong len(strlen(_path) + 1);

			if (len <= 1) free(ambiance_path), ambiance_path = NULL;
			else
			{
				ptr = realloc(ambiance_path, len);

				if (!ptr)
				{
					if (mutex) mutex->unlock();

					return AAL_ERROR_MEMORY;
				}

				ambiance_path = (char *)ptr;

				memcpy(ambiance_path, temp, len);

				if (len && ambiance_path[len - 2] != '\\') strcat(ambiance_path, "\\");
			}
		}
		else free(ambiance_path), ambiance_path = NULL;

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	aalError aalSetEnvironmentPath(const char * _path)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		if (_path)
		{
			void * ptr;
			const char * temp = _path;
			aalULong len(strlen(_path) + 1);

			if (len <= 1) free(environment_path), environment_path = NULL;
			else
			{
				ptr = realloc(environment_path, len);

				if (!ptr)
				{
					if (mutex) mutex->unlock();

					return AAL_ERROR_MEMORY;
				}

				environment_path = (char *)ptr;

				memcpy(environment_path, temp, len);

				if (len && environment_path[len - 2] != '\\') strcat(environment_path, "\\");
			}
		}
		else free(environment_path), environment_path = NULL;

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	aalError aalEnable(const aalULong & flags)
	{
		aalError _error(AAL_OK);


		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		if (flags & AAL_FLAG_MULTITHREAD && !mutex)
		{
			mutex = new Lock();

			if (!mutex) return AAL_ERROR_SYSTEM;

			if (mutex && !mutex->lock(MUTEX_TIMEOUT))
				return AAL_ERROR_TIMEOUT;

		}

		// I don't think we need to worry about this
		if (flags & FLAG_ANY_3D_FX)
		{
			// if (primary->QueryInterface(IID_IDirectSound3DListener, (aalVoid **)&listener))
			// {
			// 	if (mutex) mutex->unlock();

			// 	return AAL_ERROR_SYSTEM;
			// }

			global_status |= FLAG_ANY_3D_FX & ~FLAG_ANY_ENV_FX;
		}

		if (flags & FLAG_ANY_ENV_FX && is_reverb_present)
		{
			_error = EnableEnvironmentalAudio();

			if (_error)
			{
				if (mutex) mutex->unlock();

				return _error;
			}

			global_status |= FLAG_ANY_ENV_FX;
		}

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	aalError aalDisable(const aalULong & flags)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		if (flags & AAL_FLAG_MULTITHREAD && mutex)
			delete mutex, mutex = NULL;

		return AAL_OK;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Global status                                                             //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////

	aalUBool aalIsEnabled(const aalFlag & flag)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_UFALSE;

		aalUBool status;

		switch (flag)
		{
			case AAL_FLAG_MULTITHREAD   :
				status = mutex ? AAL_UTRUE : AAL_UFALSE;
				break;

			case AAL_FLAG_POSITION      :
			case AAL_FLAG_VELOCITY      :
			case AAL_FLAG_DIRECTION     :
			case AAL_FLAG_CONE          :
			case AAL_FLAG_FALLOFF       :
			        status = AAL_UTRUE;
				break;

			case AAL_FLAG_REVERBERATION :
			case AAL_FLAG_OBSTRUCTION   :
				status = AAL_UTRUE;
				break;
			default:
				// cannot query for this flag
				status = AAL_UFALSE;
		}

		if (mutex) mutex->unlock();

		return status;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Global control                                                            //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////


	aalError aalUpdate()
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		// Update global timer
		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		unsigned long elapsed_seconds = ts.tv_sec - start_timespec.tv_sec;
		unsigned long elapsed_nseconds = ts.tv_nsec - start_timespec.tv_nsec;
		session_time = elapsed_seconds * 1000 + elapsed_nseconds / 1000000;

		aalULong i;

		unsigned long ulNb = 0;

		// Update instances
		for (i = 0; i < _inst.Size(); i++)
		{
			Instance * instance = _inst[i];

			if (instance)
			{
				instance->Update();

				if (instance->IsIdled())
				{
					_inst.Delete(i);
				}
				else
				{
					ulNb ++;
				}
			}
		}

		// Update ambiances
		for (i = 0; i < _amb.Size(); i++)
		{
			Ambiance * ambiance = _amb[i];

			if (ambiance)
			{
				ambiance->Update();

				if (ambiance->channel.flags & AAL_FLAG_AUTOFREE &&
				        !ambiance->IsPaused() && !ambiance->IsPlaying())
					_amb.Delete(i);
			}
		}

		// Update samples
		Sample * sample = NULL;

		for (i = 0; i < _sample.Size(); i++)
		{
			sample = _sample[i];

			if (sample && sample->IsHandled() < 1)
			{
				//Console_Log("- %s", sample->name);
				_sample.Delete(i);
			}
		}

		// Update output buffer with new 3D positional settings
		// FIXME -- I don't think we need this
		//if (listener) listener->CommitDeferredSettings();

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Resource creation                                                         //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	aalSLong aalCreateMixer(const char * name)
	{
		Mixer * mixer = NULL;
		aalSLong id;

		if (mutex && !mutex->lock(MUTEX_TIMEOUT)) return AAL_SFALSE;

		mixer = new Mixer;

		if ((name && mixer->SetName(name)) || (id = _mixer.Add(mixer)) == AAL_SFALSE)
		{
			delete mixer;

			if (mutex) mutex->unlock();

			return AAL_SFALSE;
		}

		if (mutex) mutex->unlock();

		return id;
	}

	aalSLong aalCreateSample(const char * name)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_SFALSE;

		Sample * sample = NULL;
		aalSLong s_id;

		if (!sample)
		{
			sample = new Sample();
		}

		if ((name && sample->Load(name)) || (s_id = _sample.Add(sample)) == AAL_SFALSE)
		{
			delete sample;
			
			LogDebug << "Sample " << name << " not found";

			if (mutex) mutex->unlock();

			return AAL_SFALSE;
		}

		sample->Catch();

		if (mutex) mutex->unlock();

		return s_id | 0xffff0000;
	}

	aalSLong aalCreateAmbiance(const char * name)
	{
		Ambiance * ambiance = NULL;
		aalSLong a_id;

		if (mutex && !mutex->lock(MUTEX_TIMEOUT)) return AAL_SFALSE;

		ambiance = new Ambiance;

		if ((name && ambiance->Load(name)) || (a_id = _amb.Add(ambiance)) == AAL_SFALSE)
		{
			delete ambiance;
			
			LogError << "Ambiance " << name << " not found";

			if (mutex) mutex->unlock();

			return AAL_SFALSE;
		}

		//MISERY
		Track * track = &ambiance->track_l[ambiance->track_c];

		while (track > ambiance->track_l)
		{
			--track;
			track->a_id = a_id;
		}

		if (mutex) mutex->unlock();

		return a_id;
	}

	aalSLong aalCreateEnvironment(const char * name)
	{
		Environment * env = NULL;
		aalSLong e_id;

		if (mutex && !mutex->lock(MUTEX_TIMEOUT)) return AAL_SFALSE;

		env = new Environment;

		if ((name && env->Load(name)) || (e_id = _env.Add(env)) == AAL_SFALSE)
		{
			delete env;
			
			LogError << "Environment " << name << " not found";

			if (mutex) mutex->unlock();

			return AAL_SFALSE;
		}

		if (mutex) mutex->unlock();

		return e_id;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Resource destruction                                                      //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////

	aalError aalDeleteSample(const aalSLong & sample_id)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		aalSLong s_id(GetSampleID(sample_id));

		if (_sample.IsNotValid(s_id))
		{
			if (mutex) mutex->unlock();

			return AAL_ERROR_HANDLE;
		}

		_sample.Delete(s_id);

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	aalError aalDeleteAmbiance(const aalSLong & a_id)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		_amb.Delete(a_id);

		if (mutex) mutex->unlock();

		return AAL_OK;
	}


	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Retrieve resource by name                                                 //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	aalSLong aalGetMixer(const char * name)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_SFALSE;

		for (aalULong i(0); i < _mixer.Size(); i++)
			if (_mixer[i] && (!name || !strcasecmp(name, _mixer[i]->name)))
			{
				if (mutex) mutex->unlock();

				return i;
			}

		if (mutex) mutex->unlock();

		return AAL_SFALSE;
	}

	aalSLong aalGetAmbiance(const char * name)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_SFALSE;

		for (aalULong i(0); i < _amb.Size(); i++)
			if (_amb[i] && (!name || !strcasecmp(name, _amb[i]->name)))
			{
				if (mutex) mutex->unlock();

				return i;
			}

		if (mutex) mutex->unlock();

		return AAL_SFALSE;
	}

	aalSLong aalGetEnvironment(const char * name)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_SFALSE;

		for (aalULong i(0); i < _env.Size(); i++)
			if (_env[i] && (!name || !strcasecmp(name, _env[i]->name)))
			{
				if (mutex) mutex->unlock();

				return i;
			}

		if (mutex) mutex->unlock();

		return AAL_SFALSE;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Retrieve next resource by ID                                              //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	aalSLong aalGetNextAmbiance(const aalSLong & ambiance_id)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_SFALSE;

		aalULong i(_amb.IsValid(ambiance_id) ? ambiance_id + 1 : 0);

		while (i < _amb.Size())
		{
			if (_amb[i])
			{
				if (mutex) mutex->unlock();

				return i;
			}

			++i;
		}

		if (mutex) mutex->unlock();

		return AAL_SFALSE;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Environment setup                                                         //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////

	aalError aalSetEnvironmentRolloffFactor(const aalSLong & e_id, const aalFloat & factor)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		if (_env.IsNotValid(e_id))
		{
			if (mutex) mutex->unlock();

			return AAL_ERROR_HANDLE;
		}

		_env[e_id]->SetRolloffFactor(factor);

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Listener settings                                                         //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	aalError aalSetListenerUnitFactor(const aalFloat & factor)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		/*
		if (!listener)
		{
			if (mutex) mutex->unlock();

			return AAL_ERROR_INIT;
		}

		if (listener->SetDistanceFactor(factor, DS3D_DEFERRED))
		{
			if (mutex) mutex->unlock();

			return AAL_ERROR_SYSTEM;
		}
		*/

		// FIXME -- this doesn't exist in OpenAL

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	aalError aalSetListenerRolloffFactor(const aalFloat & factor)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		/*
		if (!listener)
		{
			if (mutex) mutex->unlock();

			return AAL_ERROR_INIT;
		}

		if (listener->SetRolloffFactor(factor, DS3D_DEFERRED))
		{
			if (mutex) mutex->unlock();

			return AAL_ERROR_SYSTEM;
		}
		*/

		// FIXME -- this property doesn't exist in OpenAL

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	aalError aalSetListenerPosition(const aalVector & position)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		alListener3f(AL_POSITION, position.x, position.y, position.z);

		if (alGetError() != AL_NO_ERROR)
		{
			if (mutex) mutex->unlock();

			return AAL_ERROR_SYSTEM;
		}

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	aalError aalSetListenerDirection(const aalVector & front, const aalVector & up)
	{
		ALfloat orientation[] = {front.x, front.y, front.z, up.x, up.y, up.z};
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		alListenerfv(AL_ORIENTATION, orientation);

		if (alGetError() != AL_NO_ERROR)
		{
			if (mutex) mutex->unlock();

			return AAL_ERROR_SYSTEM;
		}

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	aalError aalSetListenerEnvironment(const aalSLong & e_id)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		if (_env.IsNotValid(e_id))
		{
			if (mutex) mutex->unlock();

			return AAL_ERROR_HANDLE;
		}

		environment_id = e_id;
		Environment * env = _env[environment_id];

		// EAXLISTENERPROPERTIES props;

		// props.dwEnvironment = 0;
		// props.dwFlags = EAXLISTENERFLAGS_DECAYHFLIMIT;
		// props.flRoomRolloffFactor = 1.0F;
		// props.lRoom = 0;
		// props.lRoomHF = 0;
		// props.flEnvironmentSize = env->size;
		env->SetEffect(AL_REVERB_DIFFUSION, env->diffusion);
		env->SetEffect(AL_REVERB_AIR_ABSORPTION_GAINHF, env->absorption * -100.0F);

		aalFloat reverb_volume = std::max(env->reverb_volume, 0.0f);
		reverb_volume = std::min(reverb_volume, 1.0f);
		env->SetEffect(AL_REVERB_LATE_REVERB_GAIN, reverb_volume);

		if (env->reverb_delay >= 100.0F) env->SetEffect(AL_REVERB_LATE_REVERB_DELAY, 0.1F);
		else env->SetEffect(AL_REVERB_LATE_REVERB_DELAY, aalFloat(env->reverb_delay) * 0.001F);

		if (env->reverb_decay <= 100.0F) env->SetEffect(AL_REVERB_DECAY_TIME, 0.1F);
		else if (env->reverb_decay >= 20000.0F) env->SetEffect(AL_REVERB_DECAY_TIME, 20.0F);
		else env->SetEffect(AL_REVERB_DECAY_TIME, aalFloat(env->reverb_decay) * 0.001F);

		aalFloat flDecayHFRatio = env->reverb_hf_decay / env->reverb_decay;

		if (flDecayHFRatio <= 0.1F) flDecayHFRatio = 0.1F;
		else if (flDecayHFRatio >= 2.0F) flDecayHFRatio = 2.0F;
		env->SetEffect(AL_REVERB_DECAY_HFRATIO, flDecayHFRatio);

		aalFloat reflect_volume = std::max(env->reflect_volume, 0.0f);
		reflect_volume = std::min(env->reflect_volume, 1.0f);
		env->SetEffect(AL_REVERB_REFLECTIONS_GAIN, reflect_volume);

		if (env->reflect_delay >= 300.0F) env->SetEffect(AL_REVERB_REFLECTIONS_DELAY, 0.3F);
		else env->SetEffect(AL_REVERB_REFLECTIONS_DELAY, aalFloat(env->reflect_delay) * 0.001F);

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Mixer setup                                                               //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////

	aalError aalSetMixerVolume(const aalSLong & m_id, const aalFloat & volume)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		if (_mixer.IsNotValid(m_id))
		{
			if (mutex) mutex->unlock();

			return AAL_ERROR_HANDLE;
		}

		_mixer[m_id]->SetVolume(volume);

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	aalError aalSetMixerParent(const aalSLong & m_id, const aalSLong & pm_id)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		if (m_id == pm_id || _mixer.IsNotValid(m_id) || _mixer.IsNotValid(pm_id))
		{
			if (mutex) mutex->unlock();

			return AAL_ERROR_HANDLE;
		}

		_mixer[m_id]->SetParent(_mixer[pm_id]);

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Mixer status                                                              //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////

	aalError aalGetMixerVolume(const aalSLong & m_id, aalFloat * volume)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
		{
			*volume = AAL_DEFAULT_VOLUME;
			return AAL_ERROR_TIMEOUT;
		}

		if (_mixer.IsNotValid(m_id))
		{
			*volume = AAL_DEFAULT_VOLUME;

			if (mutex) mutex->unlock();

			return AAL_ERROR_HANDLE;
		}

		_mixer[m_id]->GetVolume(*volume);

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Mixer control                                                             //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	
	aalError aalMixerStop(const aalSLong & m_id)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		if (_mixer.IsNotValid(m_id))
		{
			if (mutex) mutex->unlock();

			return AAL_ERROR_HANDLE;
		}

		_mixer[m_id]->Stop();

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	aalError aalMixerPause(const aalSLong & m_id)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		if (_mixer.IsNotValid(m_id))
		{
			if (mutex) mutex->unlock();

			return AAL_ERROR_HANDLE;
		}

		_mixer[m_id]->Pause();

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	aalError aalMixerResume(const aalSLong & m_id)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		if (_mixer.IsNotValid(m_id))
		{
			if (mutex) mutex->unlock();

			return AAL_ERROR_HANDLE;
		}

		_mixer[m_id]->Resume();

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Sample setup                                                              //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////

	aalError aalSetSampleVolume(const aalSLong & sample_id, const aalFloat & volume)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		aalSLong s_id(GetSampleID(sample_id));
		aalSLong i_id(GetInstanceID(sample_id));

		if (_sample.IsNotValid(s_id) || _inst.IsNotValid(i_id) || _inst[i_id]->sample != _sample[s_id])
		{
			if (mutex) mutex->unlock();

			return AAL_ERROR_HANDLE;
		}

		_inst[i_id]->SetVolume(volume);

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	aalError aalSetSamplePitch(const aalSLong & sample_id, const aalFloat & pitch)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		aalSLong s_id(GetSampleID(sample_id));
		aalSLong i_id(GetInstanceID(sample_id));

		if (_sample.IsNotValid(s_id) || _inst.IsNotValid(i_id) || _inst[i_id]->sample != _sample[s_id])
		{
			if (mutex) mutex->unlock();

			return AAL_ERROR_HANDLE;
		}

		_inst[i_id]->SetPitch(pitch);

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	aalError aalSetSamplePosition(const aalSLong & sample_id, const aalVector & position)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		aalSLong s_id(GetSampleID(sample_id));
		aalSLong i_id(GetInstanceID(sample_id));

		if (_sample.IsNotValid(s_id) || _inst.IsNotValid(i_id) || _inst[i_id]->sample != _sample[s_id])
		{
			if (mutex) mutex->unlock();

			return AAL_ERROR_HANDLE;
		}

		_inst[i_id]->SetPosition(position);

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Sample status                                                             //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	aalError aalGetSampleName(const aalSLong & s_id, char * name, const aalULong & max_char)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
		{
			*name = 0;
			return AAL_ERROR_TIMEOUT;
		}

		if (_sample.IsNotValid(GetSampleID(s_id)))
		{
			*name = 0;

			if (mutex) mutex->unlock();

			return AAL_ERROR_HANDLE;
		}

		_sample[GetSampleID(s_id)]->GetName(name, max_char);

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	aalError aalGetSampleLength(const aalSLong & s_id, aalULong & _length, const aalUnit & unit)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
		{
			_length = 0;
			return AAL_ERROR_TIMEOUT;
		}

		if (_sample.IsNotValid(GetSampleID(s_id)))
		{
			_length = 0;

			if (mutex) mutex->unlock();

			return AAL_ERROR_HANDLE;
		}

		_sample[GetSampleID(s_id)]->GetLength(_length, unit);

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	aalUBool aalIsSamplePlaying(const aalSLong & sample_id)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_UFALSE;

		aalSLong s_id(GetSampleID(sample_id));
		aalSLong i_id(GetInstanceID(sample_id));

		if (_sample.IsNotValid(s_id) || _inst.IsNotValid(i_id) || _inst[i_id]->sample != _sample[s_id])
		{
			if (mutex) mutex->unlock();

			return AAL_UFALSE;
		}

		aalUBool status(_inst[i_id]->IsPlaying());

		if (mutex) mutex->unlock();

		return status;
	}























	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Sample control                                                            //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////

	aalError aalSamplePlay(aalSLong & sample_id, const aalChannel & channel, const aalULong & play_count)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		aalSLong s_id(GetSampleID(sample_id));

		sample_id = s_id;

		if (_sample.IsNotValid(s_id) || _mixer.IsNotValid(channel.mixer))
		{
			if (mutex) mutex->unlock();

			return AAL_ERROR_HANDLE;
		}

		aalSLong i_id(GetInstanceID(sample_id));
		Instance * instance = NULL;

		if (_inst.IsValid(i_id) && _inst[i_id]->sample == _sample[s_id] &&
		        !(channel.flags ^ _inst[i_id]->channel.flags))
		{

			instance = _inst[i_id];

			if (channel.flags & AAL_FLAG_RESTART)
			{
				instance->Stop();
			}
			else if (channel.flags & AAL_FLAG_ENQUEUE && instance->channel.flags == channel.flags)
			{
				instance->Play(play_count);
			}
			else if (instance->IsIdled())
			{
				instance->channel.mixer = channel.mixer;
				instance->channel.environment = channel.environment;
				instance->SetVolume(channel.volume);
				instance->SetPitch(channel.pitch);
				instance->SetPan(channel.pan);
				instance->SetPosition(channel.position);
				instance->SetVelocity(channel.velocity);
				instance->SetDirection(channel.direction);
				instance->SetCone(channel.cone);
				instance->SetFalloff(channel.falloff);
			}
			else instance = NULL;
		}

		if (!instance)
		{
			Sample * sample = _sample[s_id];

			aalULong i(0);

			for (; i < _inst.Size(); i++)
			{
				if (_inst[i] && _inst[i]->sample == sample)
					break;
			}

			instance = new Instance;

			if ((i < _inst.Size() ? instance->Init(_inst[i], channel) : instance->Init(_sample[s_id], channel)) ||
			        (i_id = _inst.Add(instance)) == AAL_SFALSE)
			{
				delete instance;

				if (mutex) mutex->unlock();

				return AAL_ERROR_SYSTEM;
			}
		}

		//if (listener && channel.flags & FLAG_ANY_3D_FX) listener->CommitDeferredSettings();
		// FIXME -- I don't think we need the above

		if (instance->Play(play_count))
		{
			_inst.Delete(i_id);

			if (mutex) mutex->unlock();

			return AAL_ERROR_SYSTEM;
		}

		sample_id = instance->id = (i_id << 16) | s_id;

		if (channel.flags & AAL_FLAG_AUTOFREE) _sample[s_id]->Release();

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	aalError aalSampleStop(aalSLong & s_id)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		if (_sample.IsNotValid(GetSampleID(s_id)) || _inst.IsNotValid(GetInstanceID(s_id)))
		{
			if (mutex) mutex->unlock();

			return AAL_ERROR_HANDLE;
		}

		_inst[GetInstanceID(s_id)]->Stop();
		s_id |= 0xffff0000;

		if (mutex) mutex->unlock();

		return AAL_OK;
	}


	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Track setup                                                               //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////

	aalError aalMuteAmbianceTrack(const aalSLong & a_id, const aalSLong & t_id, const aalUBool & mute)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		if (_amb.IsNotValid(a_id))
		{
			if (mutex) mutex->unlock();

			return AAL_ERROR_HANDLE;
		}

		_amb[a_id]->MuteTrack(t_id, mute);

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Ambiance setup                                                            //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////

	aalError aalSetAmbianceUserData(const aalSLong & a_id, void * data)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		if (_amb.IsNotValid(a_id))
		{
			if (mutex) mutex->unlock();

			return AAL_ERROR_HANDLE;
		}

		_amb[a_id]->SetUserData(data);

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	aalError aalSetAmbianceVolume(const aalSLong & a_id, const aalFloat & volume)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		if (_amb.IsNotValid(a_id))
		{
			if (mutex) mutex->unlock();

			return AAL_ERROR_HANDLE;
		}

		_amb[a_id]->SetVolume(volume);

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Ambiance status                                                           //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////

	aalError aalGetAmbianceName(const aalSLong & a_id, char * name, const aalULong & max_char)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
		{
			*name = 0;
			return AAL_ERROR_TIMEOUT;
		}

		if (_amb.IsNotValid(a_id))
		{
			*name = 0;

			if (mutex) mutex->unlock();

			return AAL_ERROR_HANDLE;
		}

		_amb[a_id]->GetName(name, max_char);

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	aalError aalGetAmbianceUserData(const aalSLong & a_id, void ** data)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		if (_amb.IsNotValid(a_id))
		{
			if (mutex) mutex->unlock();

			return AAL_ERROR_HANDLE;
		}

		_amb[a_id]->GetUserData(data);

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	aalError aalGetAmbianceTrackID(const aalSLong & a_id, const char * name, aalSLong & _t_id)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
		{
			_t_id = AAL_SFALSE;
			return AAL_ERROR_TIMEOUT;
		}

		if (_amb.IsNotValid(a_id))
		{
			_t_id = AAL_SFALSE;

			if (mutex) mutex->unlock();

			return AAL_ERROR_HANDLE;
		}

		_amb[a_id]->GetTrackID(name, _t_id);

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	aalError aalGetAmbianceVolume(const aalSLong & a_id, aalFloat & _volume)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
		{
			_volume = AAL_DEFAULT_VOLUME;
			return AAL_ERROR_TIMEOUT;
		}

		if (_amb.IsNotValid(a_id))
		{
			_volume = AAL_DEFAULT_VOLUME;

			if (mutex) mutex->unlock();

			return AAL_ERROR_HANDLE;
		}

		_amb[a_id]->GetVolume(_volume);

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	aalUBool aalIsAmbianceLooped(const aalSLong & a_id)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_UFALSE;

		if (_amb.IsNotValid(a_id))
		{
			if (mutex) mutex->unlock();

			return AAL_UFALSE;
		}

		aalUBool status(_amb[a_id]->IsLooped());

		if (mutex) mutex->unlock();

		return status;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Ambiance control                                                          //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////

	aalError aalAmbiancePlay(const aalSLong & a_id, const aalChannel & channel, const aalULong & play_count, const aalULong & fade_interval)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		if (_amb.IsNotValid(a_id) || _mixer.IsNotValid(channel.mixer))
		{
			if (mutex) mutex->unlock();

			return AAL_ERROR_HANDLE;
		}

		_amb[a_id]->Play(channel, play_count, fade_interval);

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	aalError aalAmbianceStop(const aalSLong & a_id, const aalULong & fade_interval)
	{
		if (mutex && !mutex->lock(MUTEX_TIMEOUT))
			return AAL_ERROR_TIMEOUT;

		if (_amb.IsNotValid(a_id))
		{
			if (mutex) mutex->unlock();

			return AAL_ERROR_HANDLE;
		}

		_amb[a_id]->Stop(fade_interval);

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Misc                                                                      //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	static aalError EnableEnvironmentalAudio()
	{
		/*
		WAVEFORMATEX formatex;
		DSBUFFERDESC desc;
		LPDIRECTSOUNDBUFFER lpdsbtmp(NULL);
		LPDIRECTSOUND3DBUFFER lpds3dbtmp(NULL);

		memset(&formatex, 0, sizeof(WAVEFORMATEX));
		formatex.wFormatTag = WAVE_FORMAT_PCM;
		formatex.nChannels = (aalUWord)(global_format.channels);
		formatex.nSamplesPerSec = global_format.frequency;
		formatex.wBitsPerSample = (aalUWord)(global_format.quality);
		formatex.nBlockAlign = (aalUWord)(global_format.channels * global_format.quality / 8);
		formatex.nAvgBytesPerSec = formatex.nBlockAlign * global_format.frequency;

		memset(&desc, 0, sizeof(DSBUFFERDESC));
		desc.dwSize = sizeof(DSBUFFERDESC);
		desc.dwBufferBytes = 10000;
		desc.dwFlags = DSBCAPS_CTRL3D;
		desc.lpwfxFormat = &formatex;

		if (device->CreateSoundBuffer(&desc, &lpdsbtmp, NULL) ||
		        lpdsbtmp->QueryInterface(IID_IDirectSound3DBuffer, (void **)&lpds3dbtmp) ||
		        lpds3dbtmp->QueryInterface(IID_IKsPropertySet, (void **)&environment))
		{
			if (lpdsbtmp) lpdsbtmp->Release();

			if (lpds3dbtmp) lpds3dbtmp->Release();

			return AAL_ERROR_SYSTEM;
		}

		lpdsbtmp->Release();
		lpds3dbtmp->Release();

		aalULong support(0);

		if (environment->QuerySupport(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ALLPARAMETERS, &support) ||
		        ((support & (KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET)) != (KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET)))
		{
			environment->Release(), environment = NULL;
			return AAL_ERROR_SYSTEM;
		}

		if (environment->QuerySupport(DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_ALLPARAMETERS, &support) ||
		        ((support & (KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET)) != (KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET)))
		{
			environment->Release(), environment = NULL;
			return AAL_ERROR_SYSTEM;
		}

		EAXLISTENERPROPERTIES props;

		props.dwEnvironment = 0;
		props.dwFlags = EAXLISTENERFLAGS_DECAYHFLIMIT;
		props.flRoomRolloffFactor = 1.0F;
		props.lRoom = 0;
		props.lRoomHF = 0;
		props.flEnvironmentSize = AAL_DEFAULT_ENVIRONMENT_SIZE;
		props.flEnvironmentDiffusion = AAL_DEFAULT_ENVIRONMENT_DIFFUSION;
		props.flAirAbsorptionHF = AAL_DEFAULT_ENVIRONMENT_ABSORPTION * -100.0F;
		props.lReflections = aalSLong(2000 * log10(AAL_DEFAULT_ENVIRONMENT_REFLECTION_VOLUME));
		props.flReflectionsDelay = aalFloat(AAL_DEFAULT_ENVIRONMENT_REFLECTION_DELAY) * 0.001F;
		props.lReverb = aalSLong(2000 * log10(AAL_DEFAULT_ENVIRONMENT_REVERBERATION_VOLUME));
		props.flReverbDelay = aalFloat(AAL_DEFAULT_ENVIRONMENT_REVERBERATION_DELAY) * 0.001F;
		props.flDecayTime = aalFloat(AAL_DEFAULT_ENVIRONMENT_REVERBERATION_DECAY) * 0.001F;
		props.flDecayHFRatio = AAL_DEFAULT_ENVIRONMENT_REVERBERATION_HFDECAY / AAL_DEFAULT_ENVIRONMENT_REVERBERATION_DECAY;

		if (environment->Set(DSPROPSETID_EAX_ListenerProperties,
		                     DSPROPERTY_EAXLISTENER_ALLPARAMETERS | DSPROPERTY_EAXLISTENER_DEFERRED,
		                     NULL, 0, &props, sizeof(EAXLISTENERPROPERTIES)))
		{
			environment->Release(), environment = NULL;
			return AAL_ERROR_SYSTEM;
		}
		*/

		return AAL_OK;
	}

}//ATHENA::
