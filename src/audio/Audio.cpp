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

#include "audio/Audio.h"

#include "audio/AudioResource.h"
#include "audio/Mixer.h"
#include "audio/Sample.h"
#include "audio/Ambiance.h"
#include "audio/AudioGlobal.h"
#include "audio/Stream.h"
#include "audio/AudioBackend.h"
#include "audio/AudioSource.h"
#include "audio/AudioEnvironment.h"
#ifdef HAVE_DSOUND
	#include "audio/dsound/DSoundBackend.h"
#endif
#ifdef HAVE_OPENAL
	#include "audio/openal/OpenALBackend.h"
#endif

#include "core/Time.h"

#include "io/Logger.h"

#include "platform/String.h"
#include "platform/Lock.h"

namespace audio {

static Lock * mutex = NULL;
static const size_t MUTEX_TIMEOUT = 5000;

aalError aalInit(const string & backendName, bool enableEAX) {
	
	//Clean any initialized data
	aalClean();
	
	LogDebug << "Init";
	
	stream_limit_bytes = DEFAULT_STREAMLIMIT;
	
	//Initialize random number generator
	InitSeed();
	
	bool autoBackend = (backendName == "auto");
	aalError error;
	bool matched = false;
	
#ifdef HAVE_OPENAL
	if(autoBackend || backendName == "OpenAL") {
		matched = true;
		LogDebug << "initializing OpenAL backend";
		OpenALBackend * _backend = new OpenALBackend();
		if(!(error = _backend->init(enableEAX))) {
			backend = _backend;
		}
	}
#endif
	
#ifdef HAVE_DSOUND
	if(!backend && (autoBackend || backendName == "DirectSound")) {
		matched = true;
		LogDebug << "initializing DirectSound backend";
		DSoundBackend * _backend = new DSoundBackend();
		if(!(error = _backend->init(enableEAX))) {
			backend = _backend;
		}
	}
#endif
	
	if(!matched) {
		LogError << "unknown backend: " << backendName;
		return AAL_ERROR_SYSTEM;
	}
	
	if(!backend) {
		LogError << "no working backend available";
		return error;
	}
	
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
	
	_amb.clear();
	_sample.clear();
	_mixer.clear();
	_env.clear();
	
	delete backend, backend = NULL;
	
	sample_path.clear();
	ambiance_path.clear();
	environment_path.clear();
	
	mutex->unlock();
	delete mutex, mutex = NULL;
	
	return AAL_OK;
}

#define AAL_ENTRY \
	if(!backend) { \
		return AAL_ERROR_INIT; \
	} else if(!mutex->lock(MUTEX_TIMEOUT)) { \
		return AAL_ERROR_TIMEOUT; \
	}

#define AAL_ENTRY_V(value) \
	if(!backend || !mutex->lock(MUTEX_TIMEOUT)) { \
		return (value); \
	}

#define AAL_EXIT mutex->unlock();

aalError aalSetStreamLimit(size_t limit) {
	
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
	for(size_t i = 0; i < _amb.size(); i++) {
		Ambiance * ambiance = _amb[i];
		if(ambiance) {
			ambiance->update();
			if(ambiance->getChannel().flags & FLAG_AUTOFREE && ambiance->isIdle()) {
				_amb.remove(i);
			}
		}
	}
	
	// Update samples
	for(size_t i = 0; i < _sample.size(); i++) {
		Sample * sample = _sample[i];
		if(sample && sample->isReferenced() < 1) {
			_sample.remove(i);
		}
	}
	
	aalError ret = backend->updateDeferred();
	
	AAL_EXIT
	
	return ret;
}

// Resource creation

MixerId aalCreateMixer() {
	
	AAL_ENTRY_V(INVALID_ID)
	
	Mixer * mixer = new Mixer();
	
	MixerId id = _mixer.add(mixer);
	if(id == INVALID_ID) {
		delete mixer;
	}
	
	AAL_EXIT
	
	return id;
}

SampleId aalCreateSample(const string & name) {
	
	AAL_ENTRY_V(INVALID_ID)
	
	Sample * sample = new Sample(name);
	
	SampleId s_id = INVALID_ID;
	if(sample->load() || (s_id = _sample.add(sample)) == INVALID_ID) {
		delete sample;
	} else {
		sample->reference();
	}
	
	AAL_EXIT
	
	return Backend::clearSource(s_id);
}

AmbianceId aalCreateAmbiance(const string & name) {
	
	AAL_ENTRY_V(INVALID_ID)
	
	Ambiance * ambiance = new Ambiance(name);
	AmbianceId a_id = INVALID_ID;
	if(ambiance->load() || (a_id = _amb.add(ambiance)) == INVALID_ID) {
		delete ambiance;
		LogError << "Ambiance " << name << " not found";
	} else {
		ambiance->setId(a_id);
	}
	
	AAL_EXIT
	
	return a_id;
}

EnvId aalCreateEnvironment(const string & name) {
	
	AAL_ENTRY_V(INVALID_ID)
	
	Environment * env = new Environment(name);
	EnvId e_id = INVALID_ID;
	if(env->load() || (e_id = _env.add(env)) == INVALID_ID) {
		delete env;
		LogError << "Environment " << name << " not found";
	}
	
	AAL_EXIT
	
	return e_id;
}

// Resource destruction

aalError aalDeleteSample(SampleId sample_id) {
	
	AAL_ENTRY
	
	SampleId s_id = Backend::getSampleId(sample_id);
	if(!_sample.isValid(s_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	_sample.remove(s_id);
	
	AAL_EXIT
	
	return AAL_OK;
}

aalError aalDeleteAmbiance(AmbianceId a_id) {
	
	AAL_ENTRY
	
	_amb.remove(a_id);
	
	AAL_EXIT
	
	return AAL_OK;
}

AmbianceId aalGetAmbiance(const string & name) {
	
	AAL_ENTRY_V(INVALID_ID)
	
	for(size_t i = 0; i < _amb.size(); i++) {
		if(_amb[i] && !strcasecmp(name, _amb[i]->getName())) {
			AAL_EXIT
			return i;
		}
	}
	
	AAL_EXIT
	
	return INVALID_ID;
}

EnvId aalGetEnvironment(const string & name) {
	
	AAL_ENTRY_V(INVALID_ID)
	
	for(size_t i = 0; i < _env.size(); i++) {
		if(_env[i] && !strcasecmp(name, _env[i]->name)) {
			AAL_EXIT
			return i;
		}
	}
	
	AAL_EXIT
	
	return INVALID_ID;
}

// Retrieve next resource by ID

AmbianceId aalGetNextAmbiance(AmbianceId ambiance_id) {
	
	AAL_ENTRY_V(INVALID_ID)
	
	size_t i = _amb.isValid(ambiance_id) ? ambiance_id + 1 : 0;
	
	for(; i < _amb.size(); i++) {
		if(_amb[i]) {
			AAL_EXIT
			return i;
		}
	}
	
	AAL_EXIT
	
	return INVALID_ID;
}

// Environment setup

aalError aalSetRoomRolloffFactor(float factor) {
	
	AAL_ENTRY
	
	LogDebug << "SetRoomRolloffFactor " << factor;
	
	aalError ret = backend->setRoomRolloffFactor(factor);
	
	AAL_EXIT
	
	return ret;
}

// Listener settings

aalError aalSetUnitFactor(float factor) {
	
	AAL_ENTRY
	
	LogDebug << "SetUnitFactor " << factor;
	
	aalError ret = backend->setUnitFactor(factor);
	
	AAL_EXIT
	
	return ret;
}

aalError aalSetRolloffFactor(float factor) {
	
	AAL_ENTRY
	
	LogDebug << "SetRolloffFactor " << factor;
	
	aalError ret = backend->setRolloffFactor(factor);
	
	AAL_EXIT
	
	return ret;
}

aalError aalSetListenerPosition(const Vector3f & position) {
	
	AAL_ENTRY
	
	aalError ret = backend->setListenerPosition(position);
	
	AAL_EXIT
	
	return ret;
}

aalError aalSetListenerDirection(const Vector3f & front, const Vector3f & up) {
	
	AAL_ENTRY
	
	aalError ret = backend->setListenerOrientation(front, up);
	
	AAL_EXIT
	
	return ret;
}

aalError aalSetListenerEnvironment(EnvId e_id) {
	
	AAL_ENTRY
	
	if(!_env.isValid(e_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug << "SetListenerEnvironment " << _env[e_id]->name;
	
	aalError ret = backend->setListenerEnvironment(*_env[e_id]);
	
	AAL_EXIT
	
	return ret;
}

// Mixer setup

aalError aalSetMixerVolume(MixerId m_id, float volume) {
	
	AAL_ENTRY
	
	if(!_mixer.isValid(m_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug << "SetMixerVolume " << m_id << " volume=" << volume;
	
	aalError ret = _mixer[m_id]->setVolume(volume);
	
	AAL_EXIT
	
	return ret;
}

aalError aalSetMixerParent(MixerId m_id, MixerId pm_id) {
	
	AAL_ENTRY
	
	if(m_id == pm_id || !_mixer.isValid(m_id) || !_mixer.isValid(pm_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug << "SetMixerParent " << m_id << " parent=" << pm_id;
	
	aalError ret = _mixer[m_id]->setParent(_mixer[pm_id]);
	
	AAL_EXIT
	
	return ret;
}

// Mixer status

aalError aalGetMixerVolume(MixerId m_id, float * volume) {
	
	*volume = DEFAULT_VOLUME;
	
	AAL_ENTRY
	
	if(!_mixer.isValid(m_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	*volume = _mixer[m_id]->getVolume();
	
	AAL_EXIT
	
	return AAL_OK;
}

// Mixer control 

aalError aalMixerStop(MixerId m_id) {
	
	AAL_ENTRY
	
	if(!_mixer.isValid(m_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug << "MixerStop " << m_id;
	
	aalError ret = _mixer[m_id]->stop();
	
	AAL_EXIT
	
	return ret;
}

aalError aalMixerPause(MixerId m_id) {
	
	AAL_ENTRY;
	
	if(!_mixer.isValid(m_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug << "MixerPause " << m_id;
	
	aalError ret = _mixer[m_id]->pause();
	
	AAL_EXIT
	
	return ret;
}

aalError aalMixerResume(MixerId m_id) {
	
	AAL_ENTRY
	
	if(!_mixer.isValid(m_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug << "MixerResume " << m_id;
	
	aalError ret = _mixer[m_id]->resume();
	
	AAL_EXIT
	
	return ret;
}

// Sample setup

aalError aalSetSampleVolume(SourceId sample_id, float volume) {
	
	AAL_ENTRY
	
	Source * source = backend->getSource(sample_id);
	if(!source) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	aalError ret = source->setVolume(volume);
	
	AAL_EXIT
	
	return ret;
}

aalError aalSetSamplePitch(SourceId sample_id, float pitch) {
	
	AAL_ENTRY
	
	Source * source = backend->getSource(sample_id);
	if(!source) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	aalError ret = source->setPitch(pitch);
	
	AAL_EXIT
	
	return ret;
}

aalError aalSetSamplePosition(SourceId sample_id, const Vector3f & position) {
	
	AAL_ENTRY
	
	Source * source = backend->getSource(sample_id);
	if(!source) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	aalError ret = source->setPosition(position);
	
	AAL_EXIT
	
	return ret;
}

// Sample status

aalError aalGetSampleName(SampleId sample_id, string & name) {
	
	name.clear();
	
	AAL_ENTRY
	
	SampleId s_id = Backend::getSampleId(sample_id);
	if(!_sample.isValid(s_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	name = _sample[s_id]->getName();
	
	AAL_EXIT
	
	return AAL_OK;
}

aalError aalGetSampleLength(SampleId sample_id, size_t & length, TimeUnit unit) {
	
	length = 0;
	
	AAL_ENTRY
	
	SampleId s_id = Backend::getSampleId(sample_id);
	if(!_sample.isValid(s_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	Sample * sample = _sample[s_id];
	length = bytesToUnits(sample->getLength(), sample->getFormat(), unit);
	
	AAL_EXIT
	
	return AAL_OK;
}

bool aalIsSamplePlaying(SourceId sample_id) {
	
	AAL_ENTRY_V(false)
	
	Source * source = backend->getSource(sample_id);
	if(!source) {
		AAL_EXIT
		return false;
	}
	
	bool ret = source->isPlaying();
	
	AAL_EXIT
	
	return ret;
}

// Sample control

aalError aalSamplePlay(SampleId & sample_id, const Channel & channel, unsigned play_count) {
	
	AAL_ENTRY
	
	SampleId s_id = Backend::getSampleId(sample_id);
	sample_id = Backend::clearSource(sample_id);
	if(!_sample.isValid(s_id) || !_mixer.isValid(channel.mixer)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug << "SamplePlay " << _sample[s_id]->getName() << " play_count=" << play_count;
	
	Source * source = backend->getSource(sample_id);
	if(source) {
		if(channel.flags == source->getChannel().flags) {
			source = NULL;
		} else if(channel.flags & FLAG_RESTART) {
			source->stop();
		} else if(channel.flags & FLAG_ENQUEUE) {
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
	
	if(channel.flags & FLAG_AUTOFREE) {
		_sample[s_id]->dereference();
	}
	
	AAL_EXIT
	
	return AAL_OK;
}

aalError aalSampleStop(SourceId & sample_id) {
	
	AAL_ENTRY
	
	Source * source = backend->getSource(sample_id);
	if(!source) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug << "SampleStop " << source->getSample()->getName();
	
	aalError ret = source->stop();
	
	AAL_EXIT
	
	sample_id = Backend::clearSource(sample_id);
	
	return ret;
}

// Track setup

aalError aalMuteAmbianceTrack(AmbianceId a_id, const string & track, bool mute) {
	
	AAL_ENTRY
	
	if(!_amb.isValid(a_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug << "MuteAmbianceTrack " << _amb[a_id]->getName() << " " << track << " " << mute;
	
	aalError ret = _amb[a_id]->muteTrack(track, mute);
	
	AAL_EXIT
	
	return ret;
}

// Ambiance setup

aalError aalSetAmbianceUserData(AmbianceId a_id, void * data) {
	
	AAL_ENTRY
	
	if(!_amb.isValid(a_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug << "SetAmbianceUserData " << _amb[a_id]->getName() << " " << data;
	
	_amb[a_id]->setUserData(data);
	
	AAL_EXIT
	
	return AAL_OK;
}

aalError aalSetAmbianceVolume(AmbianceId a_id, float volume) {
	
	AAL_ENTRY
	
	if(!_amb.isValid(a_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug << "SetAmbianceVolume " << _amb[a_id]->getName() << " " << volume;
	
	aalError ret = _amb[a_id]->setVolume(volume);
	
	AAL_EXIT
	
	return ret;
}

// Ambiance status

aalError aalGetAmbianceName(AmbianceId a_id, string & name) {
	
	name.clear();
	
	AAL_ENTRY
	
	if(!_amb.isValid(a_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	name = _amb[a_id]->getName();
	
	AAL_EXIT
	
	return AAL_OK;
}

aalError aalGetAmbianceUserData(AmbianceId a_id, void ** data) {
	
	AAL_ENTRY
	
	if(!_amb.isValid(a_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	*data = _amb[a_id]->getUserData();
	
	AAL_EXIT
	
	return AAL_OK;
}

aalError aalGetAmbianceVolume(AmbianceId a_id, float & _volume) {
	
	_volume = DEFAULT_VOLUME;
	
	AAL_ENTRY
	
	if(!_amb.isValid(a_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	if(!(_amb[a_id]->getChannel().flags & FLAG_VOLUME)) {
		AAL_EXIT
		return AAL_ERROR_INIT;
	}
	
	_volume = _amb[a_id]->getChannel().volume;
	
	AAL_EXIT
	
	return AAL_OK;
}

bool aalIsAmbianceLooped(AmbianceId a_id) {
	
	AAL_ENTRY_V(false)
	
	if(!_amb.isValid(a_id)) {
		AAL_EXIT
		return false;
	}
	
	bool isLooped = _amb[a_id]->isLooped();
	
	AAL_EXIT
	
	return isLooped;
}

// Ambiance control

aalError aalAmbiancePlay(AmbianceId a_id, const Channel & channel, bool loop, size_t fade_interval) {
	
	AAL_ENTRY
	
	if(!_amb.isValid(a_id) || !_mixer.isValid(channel.mixer)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug << "AmbiancePlay " << _amb[a_id]->getName() << " loop=" << loop << " fade=" << fade_interval;
	
	aalError ret = _amb[a_id]->play(channel, loop, fade_interval);
	
	AAL_EXIT
	
	return ret;
}

aalError aalAmbianceStop(AmbianceId a_id, size_t fade_interval) {
	
	AAL_ENTRY
	
	if(!_amb.isValid(a_id)) {
		AAL_EXIT
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug << "AmbianceStop " << _amb[a_id]->getName() << " " << fade_interval;
	
	_amb[a_id]->stop(fade_interval);
	
	AAL_EXIT
	
	return AAL_OK;
}

} // namespace audio
