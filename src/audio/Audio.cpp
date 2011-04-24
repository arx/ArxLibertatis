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

#include <windows.h> // TODO for GetTickCount(), remove!

#include "audio/AudioResource.h"
#include "audio/Mixer.h"
#include "audio/Sample.h"
#include "audio/Ambient.h"
#include "audio/AudioGlobal.h"
#include "audio/Stream.h"
#include "audio/Lock.h"
#include "audio/AudioBackend.h"
#include "audio/AudioSource.h"
#include "audio/dsound/DSoundBackend.h"

#include "io/Logger.h"

#include "platform/String.h"

namespace audio {

static Lock * mutex = NULL;
static const aalULong MUTEX_TIMEOUT(500);

aalError aalInit(bool enableEAX) {
	
	//Clean any initialized data
	aalClean();
	
	stream_limit_bytes = AAL_DEFAULT_STREAMLIMIT;
	
	//Initialize random number generator
	InitSeed();
	
	DSoundBackend * _backend = new DSoundBackend();
	if(aalError error = _backend->init(enableEAX)) {
		return error;
	}
	backend = _backend;
	
	mutex = new Lock();
	
	session_time = GetTickCount();
	
	return AAL_OK;
}

aalError aalClean() {
	
	if(!backend) {
		return AAL_OK;
	}
	
	mutex->lock(MUTEX_TIMEOUT);
	
	_mixer.Clean();
	_amb.Clean();
	_sample.Clean();
	_env.Clean();
	
	delete backend, backend = NULL;
	
	sample_path.clear();
	ambiance_path.clear();
	environment_path.clear();
	
	delete mutex, mutex = NULL;
	
	return AAL_OK;
}

#define AAL_ENTRY \
	if(!backend) { \
		return AAL_ERROR_INIT; \
	} \
	if(!mutex->lock(MUTEX_TIMEOUT)) { \
		return AAL_ERROR_TIMEOUT; \
	}

#define AAL_ENTRY_V(value) \
	if(!backend) { \
		return (value); \
	} \
	if(!mutex->lock(MUTEX_TIMEOUT)) { \
		return (value); \
	}

#define AAL_EXIT mutex->unlock();

#define AAL_CHECK(expr) \
	if(aalError error = (expr)) { \
		AAL_EXIT \
		return error; \
	}

aalError aalSetStreamLimit(const aalULong & limit) {
	
	AAL_ENTRY
	
	stream_limit_bytes = limit;
	
	AAL_EXIT
	
	return AAL_OK;
}

aalError aalSetSamplePath(const string & path) {
	
	AAL_ENTRY
	
	sample_path = path;
	
	AAL_EXIT
	
	return AAL_OK;
}

aalError aalSetAmbiancePath(const string & path) {
	
	AAL_ENTRY
	
	ambiance_path = path;
	
	AAL_EXIT
	
	return AAL_OK;
}

aalError aalSetEnvironmentPath(const string & path) {
	
	AAL_ENTRY
	
	environment_path = path;
	
	AAL_EXIT
	
	return AAL_OK;
}

aalError setReverbEnabled(bool enable) {
	
	AAL_ENTRY
	
	aalError ret = backend->setReverbEnabled(enable);
	
	AAL_EXIT
	
	return ret;
}


aalError aalUpdate() {
	
	AAL_ENTRY
	
	session_time = GetTickCount();
	
	// Update sources
	for(Backend::source_iterator p = backend->sourcesBegin(); p != backend->sourcesEnd();) {
		Source * source = *p;
		if(source && (source->update(), source->isIdle())) {
			p = backend->deleteSource(p);
		} else {
			++p;
		}
	}
	
	// Update ambiances
	for(aalULong i = 0; i < _amb.Size(); i++) {
		Ambiance * ambiance = _amb[i];
		if(ambiance) {
			ambiance->Update();
			if(ambiance->channel.flags & AAL_FLAG_AUTOFREE &&
         !ambiance->IsPaused() && !ambiance->IsPlaying()) {
				_amb.Delete(i);
			}
		}
	}
	
	// Update samples
	for(aalULong i = 0; i < _sample.Size(); i++) {
		Sample * sample = _sample[i];
		if(sample && sample->IsHandled() < 1) {
			_sample.Delete(i);
		}
	}
	
	backend->updateDeferred();
	
	AAL_EXIT
	
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
			
			LogWarning << "Sample " << name << " not found";

			if (mutex) mutex->unlock();

			return AAL_SFALSE;
		}

		sample->Catch();

		if (mutex) mutex->unlock();

		return Backend::clearSource(s_id);
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

aalError aalDeleteSample(aalSLong sample_id) {
	
	AAL_ENTRY
	
	aalSLong s_id = Backend::getSampleId(sample_id);
	if(!_sample.IsValid(s_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	_sample.Delete(s_id);
	
	AAL_EXIT
	
	return AAL_OK;
}

	aalError aalDeleteAmbiance(aalSLong a_id)
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

aalSLong aalGetEnvironment(const string & name) {
	
	AAL_ENTRY_V(AAL_SFALSE)
	
	for(aalULong i = 0; i < _env.Size(); i++) {
		if(_env[i] && !strcasecmp(name, _env[i]->name)) {
			AAL_EXIT
			return i;
		}
	}
	
	AAL_EXIT
	
	return AAL_SFALSE;
}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Retrieve next resource by ID                                              //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	aalSLong aalGetNextAmbiance(aalSLong ambiance_id)
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

// Environment setup

aalError aalSetRoomRolloffFactor(float factor) {
	
	AAL_ENTRY
	
	aalError ret = backend->setRoomRolloffFactor(factor);
	
	AAL_EXIT
	
	return ret;
}

// Listener settings

aalError aalSetListenerUnitFactor(float factor) {
	
	AAL_ENTRY
	
	aalError ret = backend->setUnitFactor(factor);
	
	AAL_EXIT
	
	return ret;
}

aalError aalSetListenerRolloffFactor(float factor) {
	
	AAL_ENTRY
	
	aalError ret = backend->setRolloffFactor(factor);
	
	AAL_EXIT
	
	return ret;
}

aalError aalSetListenerPosition(const aalVector & position) {
	
	AAL_ENTRY
	
	aalError ret = backend->setListenerPosition(position);
	
	AAL_EXIT
	
	return ret;
}

aalError aalSetListenerDirection(const aalVector & front, const aalVector & up) {
	
	AAL_ENTRY
	
	aalError ret = backend->setListenerOrientation(front, up);
	
	AAL_EXIT
	
	return ret;
}

aalError aalSetListenerEnvironment(aalSLong e_id) {
	
	AAL_ENTRY
	
	if(!_env.IsValid(e_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	aalError ret = backend->setListenerEnvironment(*_env[e_id]);
	
	AAL_EXIT
	
	return ret;
}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Mixer setup                                                               //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////

	aalError aalSetMixerVolume(aalSLong m_id, float volume)
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

	aalError aalSetMixerParent(aalSLong m_id, aalSLong pm_id)
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

	aalError aalGetMixerVolume(aalSLong m_id, aalFloat * volume)
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
	
	aalError aalMixerStop(aalSLong m_id)
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

	aalError aalMixerPause(aalSLong m_id)
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

	aalError aalMixerResume(aalSLong m_id)
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

// Sample setup

aalError aalSetSampleVolume(aalSLong sample_id, float volume) {
	
	AAL_ENTRY
	
	Source * source = backend->getSource(sample_id);
	if(!source) {
		return AAL_ERROR_HANDLE;
	}
	
	aalError ret = source->setVolume(volume);
	
	AAL_EXIT
	
	return ret;
}

aalError aalSetSamplePitch(aalSLong sample_id, float pitch) {
	
	AAL_ENTRY
	
	Source * source = backend->getSource(sample_id);
	if(!source) {
		return AAL_ERROR_HANDLE;
	}
	
	aalError ret = source->setPitch(pitch);
	
	AAL_EXIT
	
	return ret;
}

aalError aalSetSamplePosition(aalSLong sample_id, const aalVector & position) {
	
	AAL_ENTRY
	
	Source * source = backend->getSource(sample_id);
	if(!source) {
		return AAL_ERROR_HANDLE;
	}
	
	aalError ret = source->setPosition(position);
	
	AAL_EXIT
	
	return ret;
}

// Sample status

aalError aalGetSampleName(aalSLong sample_id, char * name, const aalULong & max_char) {
	
	*name = 0;
	
	AAL_ENTRY
	
	aalSLong s_id = Backend::getSampleId(sample_id);
	if(!_sample.IsValid(s_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	_sample[s_id]->GetName(name, max_char);
	
	AAL_EXIT
	
	return AAL_OK;
}

aalError aalGetSampleLength(aalSLong sample_id, aalULong & _length, const aalUnit & unit) {
	
	_length = 0;
	
	AAL_ENTRY
	
	aalSLong s_id = Backend::getSampleId(sample_id);
	
	if(!_sample.IsValid(s_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	_sample[s_id]->GetLength(_length, unit);
	
	AAL_EXIT
	
	return AAL_OK;
}

bool aalIsSamplePlaying(aalSLong sample_id) {
	
	AAL_ENTRY_V(false)
	
	Source * source = backend->getSource(sample_id);
	if(!source) {
		return AAL_ERROR_HANDLE;
	}
	
	bool ret = source->isPlaying();
	
	AAL_EXIT
	
	return ret;
}

// Sample control

aalError aalSamplePlay(aalSLong & sample_id, const aalChannel & channel, const aalULong & play_count) {
	
	AAL_ENTRY
	
	aalSLong s_id = Backend::getSampleId(sample_id);
	sample_id = Backend::clearSource(sample_id);
	if(!_sample.IsValid(s_id) || !_mixer.IsValid(channel.mixer)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	Source * source = backend->getSource(sample_id);
	if(source) {
		if(channel.flags == source->getChannel().flags) {
			source = NULL;
		} else if(channel.flags & AAL_FLAG_RESTART) {
			source->stop();
		} else if(channel.flags & AAL_FLAG_ENQUEUE) {
			source->play(play_count);
		} else if(source->isIdle()) {
			source->setMixer(channel.mixer);
			source->setEnvironment(channel.environment);
			source->setVolume(channel.volume);
			source->setPitch(channel.pitch);
			source->setPan(channel.pan);
			source->setPosition(channel.position);
			source->setVelocity(channel.velocity);
			source->setDirection(channel.direction);
			source->setCone(channel.cone);
			source->setFalloff(channel.falloff);
		} else {
			source = NULL;
		}
	}
	
	if(!source) {
		source = backend->createSource(s_id, channel);
		if(!source) {
			AAL_EXIT
			return AAL_ERROR_SYSTEM;
		}
	}
	
	backend->updateDeferred();
	
	if(source->play(play_count)) {
		AAL_EXIT
		return AAL_ERROR_SYSTEM;
	}
	
	sample_id = source->getId();
	
	if(channel.flags & AAL_FLAG_AUTOFREE) {
		_sample[s_id]->Release();
	}
	
	AAL_EXIT
	
	return AAL_OK;
}

aalError aalSampleStop(aalSLong & sample_id) {
	
	AAL_ENTRY
	
	Source * source = backend->getSource(sample_id);
	if(!source) {
		return AAL_ERROR_HANDLE;
	}
	
	aalError ret = source->stop();
	
	AAL_EXIT
	
	sample_id = Backend::clearSource(sample_id);
	
	return ret;
}


	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Track setup                                                               //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////

	aalError aalMuteAmbianceTrack(aalSLong a_id, aalSLong t_id, const aalUBool & mute)
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

	aalError aalSetAmbianceUserData(aalSLong a_id, void * data)
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

	aalError aalSetAmbianceVolume(aalSLong a_id, float volume)
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

	aalError aalGetAmbianceName(aalSLong a_id, char * name, const aalULong & max_char)
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

	aalError aalGetAmbianceUserData(aalSLong a_id, void ** data)
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

	aalError aalGetAmbianceTrackID(aalSLong a_id, const char * name, aalSLong & _t_id)
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

	aalError aalGetAmbianceVolume(aalSLong a_id, aalFloat & _volume)
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

	aalUBool aalIsAmbianceLooped(aalSLong a_id)
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

	aalError aalAmbiancePlay(aalSLong a_id, const aalChannel & channel, const aalULong & play_count, const aalULong & fade_interval)
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

	aalError aalAmbianceStop(aalSLong a_id, const aalULong & fade_interval)
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


} // namespace audio
