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

#include "core/Time.h"

#include "io/Logger.h"

#include "platform/String.h"

namespace audio {

static Lock * mutex = NULL;
static const aalULong MUTEX_TIMEOUT(500);

aalError aalInit(bool enableEAX) {
	
	//Clean any initialized data
	aalClean();
	
	LogDebug << "Init";
	
	stream_limit_bytes = AAL_DEFAULT_STREAMLIMIT;
	
	//Initialize random number generator
	InitSeed();
	
	DSoundBackend * _backend = new DSoundBackend();
	if(aalError error = _backend->init(enableEAX)) {
		return error;
	}
	backend = _backend;
	
	mutex = new Lock();
	
	session_time = Time::GetMs();
	
	return AAL_OK;
}

aalError aalClean() {
	
	if(!backend) {
		return AAL_OK;
	}
	
	LogDebug << "Clean";
	
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
	
	session_time = Time::GetMs();
	
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
	
	aalError ret = backend->updateDeferred();
	
	AAL_EXIT
	
	return ret;
}

// Resource creation

aalSLong aalCreateMixer() {
	
	AAL_ENTRY
	
	Mixer * mixer = new Mixer();
	
	aalSLong id = _mixer.Add(mixer);
	if(id == AAL_SFALSE) {
		delete mixer;
		AAL_EXIT
		return AAL_SFALSE;
	}
	
	AAL_EXIT
	
	return id;
}

aalSLong aalCreateSample(const string & name) {
	
	AAL_ENTRY
	
	Sample * sample = new Sample();
	
	aalSLong s_id;
	if(sample->load(name) || (s_id = _sample.Add(sample)) == AAL_SFALSE) {
		delete sample;
		LogWarning << "Sample " << name << " not found";
		AAL_EXIT
		return AAL_SFALSE;
	}
	
	sample->Catch();
	
	AAL_EXIT
	
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

	    if(mutex) {
		    mutex->unlock();
	    }

		return a_id;
	}

aalSLong aalCreateEnvironment(const string & name) {
	
	AAL_ENTRY
	
	Environment * env = new Environment();
	aalSLong e_id = AAL_SFALSE;
	if(env->load(name) || (e_id = _env.Add(env)) == AAL_SFALSE) {
		delete env;
		LogError << "Environment " << name << " not found";
	}
	
	AAL_EXIT
	
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
	
	LogDebug << "SetListenerEnvironment " << _env[e_id]->name;
	
	aalError ret = backend->setListenerEnvironment(*_env[e_id]);
	
	AAL_EXIT
	
	return ret;
}

// Mixer setup

aalError aalSetMixerVolume(aalSLong m_id, float volume) {
	
	AAL_ENTRY
	
	if(!_mixer.IsValid(m_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug << "SetMixerVolume " << m_id << " volume=" << volume;
	
	aalError ret = _mixer[m_id]->setVolume(volume);
	
	AAL_EXIT
	
	return ret;
}

aalError aalSetMixerParent(aalSLong m_id, aalSLong pm_id) {
	
	AAL_ENTRY
	
	if(m_id == pm_id || !_mixer.IsValid(m_id) || !_mixer.IsValid(pm_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug << "SetMixerParent " << m_id << " parent=" << pm_id;
	
	aalError ret = _mixer[m_id]->setParent(_mixer[pm_id]);
	
	AAL_EXIT
	
	return ret;
}

// Mixer status

aalError aalGetMixerVolume(aalSLong m_id, aalFloat * volume) {
	
	*volume = AAL_DEFAULT_VOLUME;
	
	AAL_ENTRY
	
	if(!_mixer.IsValid(m_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	*volume = _mixer[m_id]->getVolume();
	
	AAL_EXIT
	
	return AAL_OK;
}

// Mixer control 

aalError aalMixerStop(aalSLong m_id) {
	
	AAL_ENTRY
	
	if(!_mixer.IsValid(m_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug << "MixerStop " << m_id;
	
	aalError ret = _mixer[m_id]->stop();
	
	AAL_EXIT
	
	return ret;
}

aalError aalMixerPause(aalSLong m_id) {
	
	AAL_ENTRY;
	
	if(!_mixer.IsValid(m_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug << "MixerPause " << m_id;
	
	aalError ret = _mixer[m_id]->pause();
	
	AAL_EXIT
	
	return ret;
}

aalError aalMixerResume(aalSLong m_id) {
	
	AAL_ENTRY
	
	if(!_mixer.IsValid(m_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug << "MixerResume " << m_id;
	
	aalError ret = _mixer[m_id]->resume();
	
	AAL_EXIT
	
	return ret;
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

aalError aalGetSampleName(aalSLong sample_id, string & name) {
	
	name.clear();
	
	AAL_ENTRY
	
	aalSLong s_id = Backend::getSampleId(sample_id);
	if(!_sample.IsValid(s_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	name = _sample[s_id]->getName();
	
	AAL_EXIT
	
	return AAL_OK;
}

aalError aalGetSampleLength(aalSLong sample_id, aalULong & _length, aalUnit unit) {
	
	_length = 0;
	
	AAL_ENTRY
	
	aalSLong s_id = Backend::getSampleId(sample_id);
	if(!_sample.IsValid(s_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	Sample * sample = _sample[s_id];
	_length = BytesToUnits(sample->getLength(), sample->getFormat(), unit);
	
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
	
	LogDebug << "SamplePlay " << _sample[s_id]->getName() << " play_count=" << play_count;
	
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
	
	LogDebug << "SampleStop " << source->getSample()->getName();
	
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
		
		LogDebug << "MuteAmbianceTrack " << _amb[a_id]->name << " " << t_id << " " << mute;

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
		
		LogDebug << "SetAmbianceUserData " << _amb[a_id]->name << " " << data;

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
		
		LogDebug << "SetAmbianceVolume " << _amb[a_id]->name << " " << volume;

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
		
		LogDebug << "AmbiancePlay " << _amb[a_id]->name << " " << play_count << " " << fade_interval;

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
		
		LogDebug << "AmbianceStop " << _amb[a_id]->name << " " << fade_interval;

		_amb[a_id]->Stop(fade_interval);

		if (mutex) mutex->unlock();

		return AAL_OK;
	}

} // namespace audio
