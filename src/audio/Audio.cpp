/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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
#include "platform/Thread.h"
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

void clean() {
	
	if(!backend) {
		return;
	}
	
	LogDebug("Clean");
	
	g_ambiances.clear();
	g_samples.clear();
	g_mixers.clear();
	g_environments.clear();
	
	delete backend, backend = NULL;
	
	ambiance_path.clear();
	environment_path.clear();
	
	delete mutex, mutex = NULL;
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

void setAmbiancePath(const res::path & path) {
	
	AAL_ENTRY_VOID
	
	ambiance_path = path;
}

void setEnvironmentPath(const res::path & path) {
	
	AAL_ENTRY_VOID
	
	environment_path = path;
}

void setReverbEnabled(bool enable) {
	
	AAL_ENTRY_VOID
	
	backend->setReverbEnabled(enable);
}

bool isReverbSupported() {
	
	AAL_ENTRY_V(false)
	
	return backend->isReverbSupported();
}

void setHRTFEnabled(HRTFAttribute enable) {
	
	AAL_ENTRY_VOID
	
	backend->setHRTFEnabled(enable);
}

HRTFStatus getHRTFStatus() {
	
	AAL_ENTRY_V(HRTFUnavailable)
	
	return backend->getHRTFStatus();
}

// Resource creation

MixerId createMixer() {
	
	AAL_ENTRY_V(MixerId())
	
	Mixer * mixer = new Mixer(NULL);
	MixerId mixerHandle = g_mixers.add(mixer);
	
	arx_assert(mixerHandle != MixerId());
	LogDebug("createMixer " << mixerHandle << " root");
	
	return mixerHandle;
}

MixerId createMixer(MixerId parent) {
	
	AAL_ENTRY_V(MixerId())
	
	if(!g_mixers.isValid(parent)) {
		return MixerId();
	}
	
	Mixer * parentMixer = g_mixers[parent];
	Mixer * mixer = new Mixer(parentMixer);
	MixerId mixerHandle = g_mixers.add(mixer);
	
	arx_assert(mixerHandle != MixerId());
	LogDebug("createMixer " << mixerHandle << " parent=" << parent);
	
	return mixerHandle;
}


SampleHandle createSample(const res::path & name) {
	
	AAL_ENTRY_V(SampleHandle())
	
	Sample * sample = new Sample(name);
	
	SampleHandle sampleHandle;
	if(sample->load() || (sampleHandle = g_samples.add(sample)) == SampleHandle()) {
		delete sample;
		LogDebug("createSample " << sampleHandle << " " << name << " failed !");
	} else {
		sample->reference();
		LogDebug("createSample " << sampleHandle << " " << name << " len " << sample->getLength());
	}
	
	return sampleHandle;
}

AmbianceId createAmbiance(const res::path & name, PlayingAmbianceType type) {
	
	AAL_ENTRY_V(AmbianceId())
	
	Ambiance * ambiance = new Ambiance(name);
	AmbianceId a_id = AmbianceId();
	if(ambiance->load() || (a_id = g_ambiances.add(ambiance)) == AmbianceId()) {
		delete ambiance;
		LogError << "Ambiance " << name << " not found";
		return AmbianceId();
	}
	
	LogDebug("createAmbiance " << ambiance->getName() << " " << type);
	ambiance->setType(type);
	
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

void deleteAmbiance(AmbianceId ambianceId) {
	
	AAL_ENTRY_VOID
	
	if(!g_ambiances.isValid(ambianceId)) {
		return;
	}
	
	g_ambiances.remove(ambianceId);
}

void deleteAmbianceAll() {
	AAL_ENTRY_VOID
	
	g_ambiances.clear();
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

// Listener settings

void setUnitFactor(float factor) {
	
	AAL_ENTRY_VOID
	
	LogDebug("SetUnitFactor " << factor);
	
	backend->setUnitFactor(factor);
}

void setRolloffFactor(float factor) {
	
	AAL_ENTRY_VOID
	
	LogDebug("SetRolloffFactor " << factor);
	
	backend->setRolloffFactor(factor);
}

void setListenerPosition(const Vec3f & position, const Vec3f & front, const Vec3f & up) {
	
	AAL_ENTRY_VOID
	
	backend->setListenerPosition(position);
	backend->setListenerOrientation(front, up);
}

void setListenerEnvironment(EnvId environmentId) {
	
	AAL_ENTRY_VOID
	
	if(!g_environments.isValid(environmentId)) {
		return;
	}
	
	LogDebug("SetListenerEnvironment " << g_environments[environmentId]->name);
	
	backend->setListenerEnvironment(*g_environments[environmentId]);
}

// Mixer setup

void setMixerVolume(MixerId mixerId, float volume) {
	
	AAL_ENTRY_VOID
	
	if(!g_mixers.isValid(mixerId)) {
		return;
	}
	
	LogDebug("SetMixerVolume " << mixerId.handleData() << " volume=" << volume);
	
	g_mixers[mixerId]->setVolume(volume);
}

// Mixer control

void mixerStop(MixerId mixerId) {
	
	AAL_ENTRY_VOID
	
	if(!g_mixers.isValid(mixerId)) {
		return;
	}
	
	LogDebug("MixerStop " << mixerId.handleData());
	
	g_mixers[mixerId]->stop();
}

void mixerPause(MixerId mixerId) {
	
	AAL_ENTRY_VOID;
	
	if(!g_mixers.isValid(mixerId)) {
		return;
	}
	
	LogDebug("MixerPause " << mixerId.handleData());
	
	g_mixers[mixerId]->pause();
}

void mixerResume(MixerId mixerId) {
	
	AAL_ENTRY_VOID
	
	if(!g_mixers.isValid(mixerId)) {
		return;
	}
	
	LogDebug("MixerResume " << mixerId.handleData());
	
	g_mixers[mixerId]->resume();
}

// Sample setup

void setSampleVolume(SourcedSample sourceId, float volume) {
	
	AAL_ENTRY_VOID
	
	Source * source = backend->getSource(sourceId);
	if(!source) {
		return;
	}
	
	source->setVolume(volume);
}

void setSamplePitch(SourcedSample sourceId, float pitch) {
	
	AAL_ENTRY_VOID
	
	Source * source = backend->getSource(sourceId);
	if(!source) {
		return;
	}
	
	source->setPitch(pitch);
}

void setSamplePosition(SourcedSample sourceId, const Vec3f & position) {
	
	AAL_ENTRY_VOID
	
	Source * source = backend->getSource(sourceId);
	if(!source) {
		return;
	}
	
	source->setPosition(position);
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

SourcedSample samplePlay(SampleHandle sampleHandle, const Channel & channel, unsigned playCount) {
	
	AAL_ENTRY_V(SourcedSample())
	
	if(!g_samples.isValid(sampleHandle) || !g_mixers.isValid(channel.mixer)) {
		return SourcedSample(SourceHandle(), sampleHandle);
	}
	
	LogDebug("SamplePlay " << g_samples[sampleHandle]->getName() << " play_count=" << playCount);
	
	Source * source = backend->createSource(sampleHandle, channel);
	if(!source) {
		return SourcedSample(SourceHandle(), sampleHandle);
	}
	
	if(source->play(playCount)) {
		return SourcedSample(SourceHandle(), sampleHandle);
	}
	
	if(channel.flags & FLAG_AUTOFREE) {
		g_samples[sampleHandle]->dereference();
	}
	
	return source->getId();
}

void sampleStop(SourcedSample sourceId) {
	
	AAL_ENTRY_VOID
	
	Source * source = backend->getSource(sourceId);
	if(!source) {
		return;
	}
	
	LogDebug("SampleStop " << source->getSample()->getName());
	
	source->stop();
}

void getSourceInfos(std::vector<SourceInfo> & infos) {
	AAL_ENTRY_VOID
	
	for(audio::Backend::source_iterator p = audio::backend->sourcesBegin(); p != audio::backend->sourcesEnd(); ++p) {
		if(*p) {
			audio::Source * s = *p;
			SourceInfo si;
			si.source = s->getId().source();
			si.status = s->getStatus();
			si.sample = s->getId().getSampleId();
			si.sampleName = s->getSample()->getName().string();
			infos.push_back(si);
		}
	}
}


// Ambiance setup

void setAmbianceVolume(AmbianceId ambianceId, float volume) {
	
	AAL_ENTRY_VOID
	
	if(!g_ambiances.isValid(ambianceId)) {
		return;
	}
	
	LogDebug("SetAmbianceVolume " << g_ambiances[ambianceId]->getName() << " " << volume);
	
	g_ambiances[ambianceId]->setVolume(volume);
}

// Ambiance status

void getAmbianceInfos(std::vector<AmbianceInfo> & infos) {
	
	AAL_ENTRY_VOID
	
	for(AmbianceList::const_iterator p = g_ambiances.begin(); p != g_ambiances.end(); ++p) {
		if(*p) {
			const Ambiance * ambiance = *p;
			AmbianceInfo info;
			info.name = ambiance->getName();
			info.type = ambiance->getType();
			if(ambiance->getChannel().flags & FLAG_VOLUME) {
				info.volume = ambiance->getChannel().volume;
			} else {
				info.volume = DEFAULT_VOLUME;
			}
			info.isLooped = ambiance->isLooped();
			infos.push_back(info);
		}
	}
}


// Ambiance control

void ambiancePlay(AmbianceId ambianceId, const Channel & channel, bool loop, PlatformDuration fadeInterval) {
	
	arx_assert(channel.flags & FLAG_VOLUME);
	
	AAL_ENTRY_VOID
	
	if(!g_ambiances.isValid(ambianceId) || !g_mixers.isValid(channel.mixer)) {
		return;
	}
	
	LogDebug("AmbiancePlay " << g_ambiances[ambianceId]->getName() << " loop=" << loop
	         << " fade=" << toMs(fadeInterval));
	
	g_ambiances[ambianceId]->play(channel, loop, fadeInterval);
}

void ambianceStop(AmbianceId ambianceId, PlatformDuration fadeInterval) {
	
	AAL_ENTRY_VOID
	
	if(!g_ambiances.isValid(ambianceId)) {
		return;
	}
	
	LogDebug("AmbianceStop " << g_ambiances[ambianceId]->getName() << " " << toMs(fadeInterval));
	
	g_ambiances[ambianceId]->stop(fadeInterval);
}

static const PlatformDuration ARX_SOUND_UPDATE_INTERVAL = PlatformDurationMs(100);

class SoundUpdateThread : public StoppableThread {
	
	void update();
	
	void run() {
		
		while(!isStopRequested()) {
			
			ARX_PROFILE("SoundUpdate");
			
			sleep(ARX_SOUND_UPDATE_INTERVAL);
			
			update();
		}
		
	}
	
};

void SoundUpdateThread::update() {
	
	ARX_PROFILE_FUNC();
	
	AAL_ENTRY_VOID
	
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
}


static SoundUpdateThread * updateThread = NULL;

void threadStart() {
	
	arx_assert(!updateThread);
	
	updateThread = new SoundUpdateThread();
	updateThread->setThreadName("Sound Update");
	updateThread->start();
}

void threadStop() {
	
	if(!updateThread) {
		return;
	}
	
	updateThread->stop();
	delete updateThread, updateThread = NULL;
}

} // namespace audio
