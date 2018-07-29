/*
 * Copyright 2011-2016 Arx Libertatis Team (see the AUTHORS file)
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

#include "audio/Audio.h"

#include "Configure.h"

#include "audio/AudioResource.h"
#include "audio/Mixer.h"
#include "audio/Sample.h"
#include "audio/Ambiance.h"
#include "audio/AudioGlobal.h"
#include "audio/AudioBackend.h"
#include "audio/AudioSource.h"
#include "audio/AudioEnvironment.h"
#if ARX_HAVE_OPENAL
	#include "audio/openal/OpenALBackend.h"
#endif

#include "io/log/Logger.h"

#include "platform/Lock.h"
#include "platform/Time.h"
#include "platform/profiler/Profiler.h"

namespace audio {

static Lock * mutex = NULL;

aalError init(const std::string & backendName, const std::string & deviceName, HRTFAttribute hrtf) {
	
	// Clean any initialized data
	clean();
	
	LogDebug("Init");
	
	bool autoBackend = (backendName == "auto");
	aalError error = AAL_ERROR_INIT;
	
	for(int i = 0; i < 2 && !backend; i++) {
		bool first = (i == 0);
		
		bool matched = false;
		
		#if ARX_HAVE_OPENAL
		if(!backend && first == (autoBackend || backendName == "OpenAL")) {
			matched = true;
			LogDebug("initializing OpenAL backend");
			OpenALBackend * _backend = new OpenALBackend();
			if(!deviceName.empty() && deviceName != "auto") {
				error = _backend->init(deviceName.c_str(), hrtf);
				if(error) {
					LogWarning << "Could not open device \"" << deviceName
					           << "\", retrying with default device";
				}
			}
			if(error) {
				error = _backend->init(NULL, hrtf);
			}
			if(!error) {
				backend = _backend;
			} else {
				delete _backend;
			}
		}
		#endif
		
		if(first && !matched) {
			LogError << "Unknown backend: " << backendName;
		}
	}
	
	#if !ARX_HAVE_OPENAL
	ARX_UNUSED(autoBackend);
	#endif
	
	if(!backend) {
		LogError << "No working backend available";
		return error;
	}
	
	mutex = new Lock();
	
	session_time = platform::getTime();
	
	return AAL_OK;
}

aalError clean() {
	
	if(!backend) {
		return AAL_OK;
	}
	
	LogDebug("Clean");
	
	g_ambiances.clear();
	g_samples.clear();
	g_mixers.clear();
	g_environments.clear();
	
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
	Autolock lock(mutex);

#define AAL_ENTRY_V(value) \
	if(!backend) { \
		return (value); \
	} \
	Autolock lock(mutex);

#define AAL_ENTRY_VOID \
	if(!backend) { \
		return; \
	} \
	Autolock lock(mutex);

std::vector<std::string> getDevices() {
	
	AAL_ENTRY_V(std::vector<std::string>())
	
	return backend->getDevices();
}

aalError setSamplePath(const res::path & path) {
	
	AAL_ENTRY
	
	sample_path = path;
	
	return AAL_OK;
}

aalError setAmbiancePath(const res::path & path) {
	
	AAL_ENTRY
	
	ambiance_path = path;
	
	return AAL_OK;
}

aalError setEnvironmentPath(const res::path & path) {
	
	AAL_ENTRY
	
	environment_path = path;
	
	return AAL_OK;
}

aalError setReverbEnabled(bool enable) {
	
	AAL_ENTRY
	
	return backend->setReverbEnabled(enable);
}

bool isReverbSupported() {
	
	AAL_ENTRY_V(false)
	
	return backend->isReverbSupported();
}

aalError setHRTFEnabled(HRTFAttribute enable) {
	
	AAL_ENTRY
	
	return backend->setHRTFEnabled(enable);
}

HRTFStatus getHRTFStatus() {
	
	AAL_ENTRY_V(HRTFUnavailable)
	
	return backend->getHRTFStatus();
}

aalError update() {
	
	ARX_PROFILE_FUNC();
	
	AAL_ENTRY
	
	session_time = platform::getTime();
	
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
	for(AmbianceList::iterator i = g_ambiances.begin(); i != g_ambiances.end();) {
		Ambiance * ambiance = *i;
		if(ambiance) {
			ambiance->update();
			if(ambiance->getChannel().flags & FLAG_AUTOFREE && ambiance->isIdle()) {
				i = g_ambiances.remove(i);
			} else {
				++i;
			}
		} else {
			++i;
		}
	}
	
	// Update samples
	for(SampleList::iterator i = g_samples.begin(); i != g_samples.end();) {
		Sample * sample = *i;
		if(sample && sample->isReferenced() < 1) {
			i = g_samples.remove(i);
		} else {
			++i;
		}
	}
	
	return AAL_OK;
}

// Resource creation

static MixerId createMixer_common() {
	
	Mixer * mixer = new Mixer();
	
	MixerId id = g_mixers.add(mixer);
	if(id == MixerId()) {
		delete mixer;
	}
	
	return id;
}

MixerId createMixer() {
	
	AAL_ENTRY_V(MixerId())
	
	return createMixer_common();
}

MixerId createMixer(MixerId parent) {
	
	AAL_ENTRY_V(MixerId())
	
	if(!g_mixers.isValid(parent)) {
		return MixerId();
	}
	
	MixerId mixer = createMixer_common();
	if(mixer != MixerId()) {
		Mixer * parentMixer = g_mixers[parent];
		g_mixers[mixer]->setParent(parentMixer);
		LogDebug("createMixer " << mixer.handleData() << " parent=" << parent.handleData());
	}
	
	return mixer;
}


SampleHandle createSample(const res::path & name) {
	
	AAL_ENTRY_V(SampleHandle())
	
	Sample * sample = new Sample(name);
	
	SampleHandle sampleHandle;
	if(sample->load() || (sampleHandle = g_samples.add(sample)) == SampleHandle()) {
		delete sample;
	} else {
		sample->reference();
	}
	
	return sampleHandle;
}

AmbianceId createAmbiance(const res::path & name) {
	
	AAL_ENTRY_V(AmbianceId())
	
	Ambiance * ambiance = new Ambiance(name);
	AmbianceId a_id = AmbianceId();
	if(ambiance->load() || (a_id = g_ambiances.add(ambiance)) == AmbianceId()) {
		delete ambiance;
		LogError << "Ambiance " << name << " not found";
	}
	
	return a_id;
}

EnvId createEnvironment(const res::path & name) {
	
	AAL_ENTRY_V(EnvId())
	
	Environment * env = new Environment(name);
	EnvId e_id = EnvId();
	if(env->load() || (e_id = g_environments.add(env)) == EnvId()) {
		delete env;
		LogError << "Environment " << name << " not found";
	}
	
	return e_id;
}

// Resource destruction

void deleteSample(SampleHandle sampleHandle) {
	
	AAL_ENTRY_VOID
	
	if(!g_samples.isValid(sampleHandle)) {
		return;
	}
	
	g_samples.remove(sampleHandle);
}

aalError deleteAmbiance(AmbianceId ambianceId) {
	
	AAL_ENTRY
	
	if(!g_ambiances.isValid(ambianceId)) {
		return AAL_ERROR_HANDLE;
	}
	
	g_ambiances.remove(ambianceId);
	
	return AAL_OK;
}

AmbianceId getAmbiance(const res::path & name) {
	
	AAL_ENTRY_V(AmbianceId())
	
	for(size_t i = 0; i < g_ambiances.size(); i++) {
		Ambiance * ambiance = g_ambiances[AmbianceId(i)];
		if(ambiance && ambiance->getName() == name) {
			return AmbianceId(i);
		}
	}
	
	return AmbianceId();
}

EnvId getEnvironment(const res::path & name) {
	
	AAL_ENTRY_V(EnvId())
	
	for(size_t i = 0; i < g_environments.size(); i++) {
		Environment * environment = g_environments[EnvId(i)];
		if(environment && environment->name == name) {
			return EnvId(i);
		}
	}
	
	return EnvId();
}

// Retrieve next resource by ID

AmbianceId getNextAmbiance(AmbianceId ambianceId) {
	
	AAL_ENTRY_V(AmbianceId())
	
	size_t i = (ambianceId != AmbianceId() && g_ambiances.isValid(ambianceId))
	           ? ambianceId.handleData() + 1 : 0;
	
	for(; i < g_ambiances.size(); i++) {
		if(g_ambiances[AmbianceId(i)]) {
			return AmbianceId(i);
		}
	}
	
	return AmbianceId();
}

// Listener settings

aalError setUnitFactor(float factor) {
	
	AAL_ENTRY
	
	LogDebug("SetUnitFactor " << factor);
	
	return backend->setUnitFactor(factor);
}

aalError setRolloffFactor(float factor) {
	
	AAL_ENTRY
	
	LogDebug("SetRolloffFactor " << factor);
	
	return backend->setRolloffFactor(factor);
}

aalError setListenerPosition(const Vec3f & position) {
	
	AAL_ENTRY
	
	return backend->setListenerPosition(position);
}

aalError setListenerDirection(const Vec3f & front, const Vec3f & up) {
	
	AAL_ENTRY
	
	return backend->setListenerOrientation(front, up);
}

aalError setListenerEnvironment(EnvId environmentId) {
	
	AAL_ENTRY
	
	if(!g_environments.isValid(environmentId)) {
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug("SetListenerEnvironment " << g_environments[environmentId]->name);
	
	return backend->setListenerEnvironment(*g_environments[environmentId]);
}

// Mixer setup

aalError setMixerVolume(MixerId mixerId, float volume) {
	
	AAL_ENTRY
	
	if(!g_mixers.isValid(mixerId)) {
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug("SetMixerVolume " << mixerId.handleData() << " volume=" << volume);
	
	return g_mixers[mixerId]->setVolume(volume);
}

// Mixer control

aalError mixerStop(MixerId mixerId) {
	
	AAL_ENTRY
	
	if(!g_mixers.isValid(mixerId)) {
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug("MixerStop " << mixerId.handleData());
	
	return g_mixers[mixerId]->stop();
}

aalError mixerPause(MixerId mixerId) {
	
	AAL_ENTRY;
	
	if(!g_mixers.isValid(mixerId)) {
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug("MixerPause " << mixerId.handleData());
	
	return g_mixers[mixerId]->pause();
}

aalError mixerResume(MixerId mixerId) {
	
	AAL_ENTRY
	
	if(!g_mixers.isValid(mixerId)) {
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug("MixerResume " << mixerId.handleData());
	
	return g_mixers[mixerId]->resume();
}

// Sample setup

aalError setSampleVolume(SourcedSample sourceId, float volume) {
	
	AAL_ENTRY
	
	Source * source = backend->getSource(sourceId);
	if(!source) {
		return AAL_ERROR_HANDLE;
	}
	
	return source->setVolume(volume);
}

aalError setSamplePitch(SourcedSample sourceId, float pitch) {
	
	AAL_ENTRY
	
	Source * source = backend->getSource(sourceId);
	if(!source) {
		return AAL_ERROR_HANDLE;
	}
	
	return source->setPitch(pitch);
}

aalError setSamplePosition(SourcedSample sourceId, const Vec3f & position) {
	
	AAL_ENTRY
	
	Source * source = backend->getSource(sourceId);
	if(!source) {
		return AAL_ERROR_HANDLE;
	}
	
	return source->setPosition(position);
}

// Sample status

aalError getSampleName(SampleHandle sampleHandle, res::path & name) {
	
	name.clear();
	
	AAL_ENTRY
	
	if(!g_samples.isValid(sampleHandle)) {
		return AAL_ERROR_HANDLE;
	}
	
	name = g_samples[sampleHandle]->getName();
	
	return AAL_OK;
}

aalError getSampleLength(SampleHandle sampleHandle, size_t & length) {
	
	length = 0;
	
	AAL_ENTRY
	
	if(!g_samples.isValid(sampleHandle)) {
		return AAL_ERROR_HANDLE;
	}
	
	Sample * sample = g_samples[sampleHandle];
	const PCMFormat & format = sample->getFormat();
	
	length = size_t(u64(sample->getLength()) * 1000 / (format.frequency * format.channels * (format.quality >> 3)));
	
	return AAL_OK;
}

bool isSamplePlaying(SourcedSample sourceId) {
	
	AAL_ENTRY_V(false)
	
	Source * source = backend->getSource(sourceId);
	if(!source) {
		return false;
	}
	
	bool ret = source->isPlaying();
	
	return ret;
}

// Sample control

SourcedSample samplePlay(SampleHandle s_id, const Channel & channel, unsigned play_count) {
	
	AAL_ENTRY_V(SourcedSample())
	
	if(!g_samples.isValid(s_id) || !g_mixers.isValid(channel.mixer)) {
		return SourcedSample(SourceHandle(), s_id);
	}
	
	LogDebug("SamplePlay " << g_samples[s_id]->getName() << " play_count=" << play_count);
	
	Source * source = backend->createSource(s_id, channel);
	if(!source) {
		return SourcedSample(SourceHandle(), s_id);
	}
	
	if(source->play(play_count)) {
		return SourcedSample(SourceHandle(), s_id);
	}
	
	if(channel.flags & FLAG_AUTOFREE) {
		g_samples[s_id]->dereference();
	}
	
	return source->getId();
}

aalError sampleStop(SourcedSample & sourceId) {
	
	AAL_ENTRY
	
	Source * source = backend->getSource(sourceId);
	if(!source) {
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug("SampleStop " << source->getSample()->getName());
	
	sourceId.clearSource();
	
	return source->stop();
}

aalError getSourceInfos(std::vector<SourceInfo> & infos) {
	AAL_ENTRY
	
	for(audio::Backend::source_iterator p = audio::backend->sourcesBegin(); p != audio::backend->sourcesEnd(); ++p) {
		if(*p) {
			audio::Source *s = *p;
			SourceInfo si;
			si.source = s->getId().source();
			si.status = s->getStatus();
			si.sample = s->getId().getSampleId();
			si.sampleName = s->getSample()->getName().string();
			infos.push_back(si);
		}
	}
	
	return AAL_OK;
}


// Ambiance setup

aalError setAmbianceType(AmbianceId ambianceId, PlayingAmbianceType type) {
	
	AAL_ENTRY
	
	if(!g_ambiances.isValid(ambianceId)) {
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug("SetAmbianceUserData " << g_ambiances[ambianceId]->getName() << " " << type);
	
	g_ambiances[ambianceId]->setType(type);
	
	return AAL_OK;
}

aalError setAmbianceVolume(AmbianceId ambianceId, float volume) {
	
	AAL_ENTRY
	
	if(!g_ambiances.isValid(ambianceId)) {
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug("SetAmbianceVolume " << g_ambiances[ambianceId]->getName() << " " << volume);
	
	return g_ambiances[ambianceId]->setVolume(volume);
}

// Ambiance status

aalError getAmbianceName(AmbianceId ambianceId, res::path & name) {
	
	name.clear();
	
	AAL_ENTRY
	
	if(!g_ambiances.isValid(ambianceId)) {
		return AAL_ERROR_HANDLE;
	}
	
	name = g_ambiances[ambianceId]->getName();
	
	return AAL_OK;
}

aalError getAmbianceType(AmbianceId ambianceId, PlayingAmbianceType * type) {
	
	AAL_ENTRY
	
	if(!g_ambiances.isValid(ambianceId)) {
		return AAL_ERROR_HANDLE;
	}
	
	*type = g_ambiances[ambianceId]->getType();
	
	return AAL_OK;
}

aalError getAmbianceVolume(AmbianceId ambianceId, float & volume) {
	
	volume = DEFAULT_VOLUME;
	
	AAL_ENTRY
	
	if(!g_ambiances.isValid(ambianceId)) {
		return AAL_ERROR_HANDLE;
	}
	
	if(!(g_ambiances[ambianceId]->getChannel().flags & FLAG_VOLUME)) {
		return AAL_ERROR_INIT;
	}
	
	volume = g_ambiances[ambianceId]->getChannel().volume;
	
	return AAL_OK;
}

bool isAmbianceLooped(AmbianceId ambianceId) {
	
	AAL_ENTRY_V(false)
	
	if(!g_ambiances.isValid(ambianceId)) {
		return false;
	}
	
	return g_ambiances[ambianceId]->isLooped();
}

// Ambiance control

aalError ambiancePlay(AmbianceId ambianceId, const Channel & channel, bool loop, PlatformDuration fadeInterval) {
	
	AAL_ENTRY
	
	if(!g_ambiances.isValid(ambianceId) || !g_mixers.isValid(channel.mixer)) {
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug("AmbiancePlay " << g_ambiances[ambianceId]->getName() << " loop=" << loop
	         << " fade=" << toMs(fadeInterval));
	
	return g_ambiances[ambianceId]->play(channel, loop, fadeInterval);
}

aalError ambianceStop(AmbianceId ambianceId, PlatformDuration fadeInterval) {
	
	AAL_ENTRY
	
	if(!g_ambiances.isValid(ambianceId)) {
		return AAL_ERROR_HANDLE;
	}
	
	LogDebug("AmbianceStop " << g_ambiances[ambianceId]->getName() << " " << toMs(fadeInterval));
	
	g_ambiances[ambianceId]->stop(fadeInterval);
	
	return AAL_OK;
}

} // namespace audio
