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

#include "audio/AudioInstance.h"

#include <cstdio>

#include "audio/AudioGlobal.h"
#include "audio/Stream.h"

#include "io/Logger.h"
#include "platform/Platform.h"

namespace ATHENA {

#define ALPREFIX << "[" << id << "," << (sample ? sample->name : "(none)") << "," << nbsources << "," << nbbuffers << "] "

#undef ALError
#define ALError LogError ALPREFIX
#define ALWarning LogWarning ALPREFIX
#define LogAL(x) LogDebug ALPREFIX << x

static size_t nbsources = 0;
static size_t nbbuffers = 0;

// How often to queue the buffer when looping but not streaming.
#define MAXLOOPBUFFERS std::max(NBUFFERS, NBUFFERS * stream_limit_bytes / sample->length)

const size_t Instance::NBUFFERS;

aalError Instance::sourcePlay() {
	
	ALint val;
	AL_CLEAR_ERROR();
	alGetSourcei(source, AL_SOURCE_STATE, &val);
	AL_CHECK_ERROR("getting source state")
	
	if(val == AL_STOPPED || val == AL_INITIAL || val == AL_PAUSED) {
		alSourcePlay(source);
		AL_CHECK_ERROR("playing source")
		return AAL_OK;
	} else if(val == AL_PLAYING) {
		return AAL_OK;
	} else {
		return AAL_ERROR;
	}
	
}

Instance::Instance() :
	id((aalSLong)-1), // Otherwise id might be uninitialized for InstanceDebugLog
	sample(NULL),
	status(IDLED),
	tooFar(false),
	streaming(false), loadCount(0), written(0), stream(NULL),
	time(0), read(0), callb_i(0),
	source(0),
	refcount(NULL) {
	for(size_t i = 0; i < NBUFFERS; i++) {
		buffers[i] = 0;
	}
}

Instance::~Instance() {
	clean();
}

static ALenum getALFormat(const aalFormat & format) {
	arx_assert(format.channels == 1 || format.channels == 2);
	switch(format.quality) {
		case 8:
			return format.channels == 1 ? AL_FORMAT_MONO8 : AL_FORMAT_STEREO8;
		case 16:
			return format.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
		default:
			arx_assert(false);
			return 0;
	}
}

aalError Instance::Init(Sample * _sample, const aalChannel & _channel) {
	
	if(aalError error = clean())  {
		return error;
	}
	
	sample = _sample;
	LogAL("Init");
	sample->Catch();
	channel = _channel;
	
	if(_mixer[channel.mixer]->flags & AAL_FLAG_VOLUME) {
		channel.flags |= AAL_FLAG_VOLUME;
		channel.volume = 1.0F;
	}
	
	if(_mixer[channel.mixer]->flags & AAL_FLAG_PITCH) {
		channel.flags |= AAL_FLAG_PITCH;
		channel.pitch = 1.0F;
	}
	
	if(_mixer[channel.mixer]->flags & AAL_FLAG_PAN) {
		channel.flags |= AAL_FLAG_PAN;
		channel.pan = 0.0F;
	}
	
	if(channel.flags & FLAG_ANY_3D_FX) {
		channel.flags &= ~AAL_FLAG_PAN;
	}
	
	return init();
}

aalError Instance::init() {
	
	alGenSources(1, &source);
	nbsources++;
	alSourcei(source, AL_LOOPING, AL_FALSE);
	AL_CHECK_ERROR("generating source")
	
	streaming = (sample->length > (stream_limit_bytes * NBUFFERS));
	
	LogAL("length=" << sample->length << " streaming=" << streaming);
	
	if(!streaming && !buffers[0]) {
		LogAL("opening sample");
		stream = CreateStream(sample->name);
		if(!stream) {
			ALError << "error creating stream";
			return AAL_ERROR_FILEIO;
		}
		alGenBuffers(1, &buffers[0]);
		nbbuffers++;
		AL_CHECK_ERROR("generating buffer")
		arx_assert(buffers[0] != 0);
		loadCount = 1;
		fillBuffer(buffers[0], sample->length);
		arx_assert(!stream && !loadCount);
	}
	
	SetVolume(channel.volume);
	SetPitch(channel.pitch);
	
	// Create 3D interface if required
	if(channel.flags & FLAG_ANY_3D_FX) {
		
		if(channel.flags & AAL_FLAG_RELATIVE) {
			alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
			AL_CHECK_ERROR("setting relative flag")
		}
		
		SetPosition(channel.position);
		SetVelocity(channel.velocity);
		SetDirection(channel.direction);
		SetCone(channel.cone);
		SetFalloff(channel.falloff);
		
	} else {
		
		alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
		AL_CHECK_ERROR("setting relative flag")
		
		SetPan(channel.pan);
	}
	
	return AAL_OK;
}

aalError Instance::fillAllBuffers() {
	
	arx_assert(streaming);
	
	if(!loadCount) {
		return AAL_OK;
	}
	
	if(!stream) {
		LogAL("opening sample");
		stream = CreateStream(sample->name);
		if(!stream) {
			ALError << "error creating stream";
			return AAL_ERROR_FILEIO;
		}
	}
	
	for(size_t i = 0; i < NBUFFERS && loadCount; i++) {
		
		if(buffers[i] && alIsBuffer(buffers[i])) {
			continue;
		}
		
		alGenBuffers(1, &buffers[i]);
		nbbuffers++;
		AL_CHECK_ERROR("generating buffer")
		arx_assert(buffers[i] != 0);
		
		aalError error = fillBuffer(buffers[i], stream_limit_bytes);
		if(error != AAL_OK) {
			return error;
		}
		
		LogAL("queueing buffer " << buffers[i]);
		alSourceQueueBuffers(source, 1, &buffers[i]);
		AL_CHECK_ERROR("queueing buffer")
		
	}
	
	return AAL_OK;
}

aalError Instance::fillBuffer(ALuint buffer, size_t size) {
	
	arx_assert(loadCount > 0);
	
	size_t left = std::min(size, sample->length - written);
	if(loadCount == 1) {
		size = left;
	}
	
	LogAL("filling buffer " << buffer << " with " << size << " bytes");
	
	char * data = new char[size];
	if(!data) {
		return AAL_ERROR_MEMORY;
	}
	
	aalULong read;
	stream->Read(data, left, read);
	if(read != left) {
		delete[] data;
		return AAL_ERROR_SYSTEM;
	}
	written += read;
	arx_assert(written <= sample->length);
	if(written == sample->length) {
		written = 0;
		if(!load()) {
			LogAL("closing sample");
			DeleteStream(stream);
			stream = NULL;
		} else {
			stream->SetPosition(0);
			if(size > left) {
				stream->Read(data + left, size - left, read);
				if(read != size - left) {
					delete[] data;
					return AAL_ERROR_SYSTEM;
				}
				written += read;
				arx_assert(written < sample->length);
			}
		}
	}
	
	alBufferData(buffer, getALFormat(sample->format), data, size, sample->format.frequency);
	delete[] data;
	AL_CHECK_ERROR("setting buffer data")
	
	return AAL_OK;
}

aalError Instance::Init(Instance * instance, const aalChannel & _channel) {
	
	if(instance->stream || _channel.flags != instance->channel.flags) {
		return Init(instance->sample, _channel);
	}
	
	if(aalError error = clean()) {
		return error;
	}
	
	sample = instance->sample;
	LogAL("Init(copy)");
	sample->Catch();
	channel = _channel;
	
	arx_assert(instance->buffers[0] != 0);
	buffers[0] = instance->buffers[0];
	if(!instance->refcount) {
		instance->refcount = new unsigned int;
		*instance->refcount = 1;
	}
	refcount = instance->refcount;
	(*refcount)++;
	
	return init();
}

aalError Instance::clean() {
	
	AL_CLEAR_ERROR()
	
	if(sample) {
		LogAL("clean");
	}
	
	if(alIsSource(source)) {
		
		alSourceStop(source);
		AL_CHECK_ERROR("stopping source")
		
		alDeleteSources(1, &source);
		nbsources--;
		AL_CHECK_ERROR("deleting source")
		
		source = 0;
	} else {
		arx_assert(!source);
	}
	
	if(streaming) {
		for(size_t i = 0; i < NBUFFERS; i++) {
			if(buffers[i] && alIsBuffer(buffers[i])) {
				LogAL("deleting buffer " << buffers[i]);
				alDeleteBuffers(1, &buffers[i]);
				nbbuffers--;
				AL_CHECK_ERROR("deleting buffer")
				buffers[i] = 0;
			}
		}
		arx_assert(!refcount);
	} else {
		if(buffers[0]) {
			arx_assert(!refcount || *refcount > 0);
			if(!refcount || !--*refcount) {
				if(refcount) {
					delete refcount;
					refcount = NULL;
				}
				LogAL("deleting buffer " << buffers[0]);
				alDeleteBuffers(1, &buffers[0]);
				nbbuffers--;
				AL_CHECK_ERROR("deleting buffer")
			}
		} else {
			arx_assert(!refcount);
		}
		for(size_t i = 1; i < NBUFFERS; i++) {
			arx_assert(!buffers[i]);
		}
	}
	
	if(stream) {
		DeleteStream(stream);
		stream = NULL;
	}
	
	if(sample) {
		sample->Release(), sample = NULL;
	}
	
	status = IDLED;
	tooFar = false;
	
	loadCount = 0;
	written = 0;
	
	callb_i = time = read = 0;
	
	return AAL_OK;
}

static inline float clamp(float v, float min, float max) {
	return std::min(max, std::max(min, v));
}

aalError Instance::SetVolume(float v) {
	
	if(!alIsSource(source) || !(channel.flags & AAL_FLAG_VOLUME)) {
		return AAL_ERROR_INIT;
	}
	
	channel.volume = clamp(v, 0.f, 1.f);
	
	aalFloat volume = 1.f;
	const Mixer * mixer = _mixer[channel.mixer];
	while(mixer) {
		volume *= mixer->volume, mixer = mixer->parent;
	}
	
	alSourcef(source, AL_GAIN, channel.volume * volume);
	AL_CHECK_ERROR("setting source gain")
	
	return AAL_OK;
}

aalError Instance::SetPitch(float p) {
	
	if(!alIsSource(source) || !(channel.flags & AAL_FLAG_PITCH)) {
		return AAL_ERROR_INIT;
	}
	
	channel.pitch = clamp(p, 0.1f, 2.f);
	
	float pitch = 1.f;
	const Mixer * mixer = _mixer[channel.mixer];
	while(mixer) {
		pitch *= mixer->pitch, mixer = mixer->parent;
	}
	pitch = clamp(pitch, 0.1f, 2.f);
	
	alSourcef(source, AL_PITCH, channel.pitch * pitch);
	AL_CHECK_ERROR("setting source pitch")
	
	return AAL_OK;
}

aalError Instance::SetPan(float p) {
	
	if(!alIsSource(source) || !(channel.flags & AAL_FLAG_PAN)) {
		return AAL_ERROR_INIT;
	}
	
	channel.pan = clamp(p, -1.f, 1.f);
	
	if(channel.pan != 0.f) {
		ALError << "setting channel pan not supported: " << p;
		// FIXME -- OpenAL doesn't have a pan feature
	}
	
	return AAL_OK;
}

aalError Instance::SetPosition(const aalVector & position) {
	
	if(!alIsSource(source) || !(channel.flags & AAL_FLAG_POSITION)) {
		return AAL_ERROR_INIT;
	}
	
	channel.position = position;
	
	alSource3f(source, AL_POSITION, position.x, position.y, position.z);
	AL_CHECK_ERROR("setting source position")
	
	return AAL_OK;
}

aalError Instance::SetVelocity(const aalVector & velocity) {
	
	if(!alIsSource(source) || !(channel.flags & AAL_FLAG_VELOCITY)) {
		return AAL_ERROR_INIT;
	}
	
	channel.velocity = velocity;
	
	alSource3f(source, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
	AL_CHECK_ERROR("setting source velocity")
	
	return AAL_OK;
}

aalError Instance::SetDirection(const aalVector & direction) {
	
	if(!alIsSource(source) || !(channel.flags & AAL_FLAG_DIRECTION)) {
		return AAL_ERROR_INIT;
	}
	
	channel.direction = direction;
	
	alSource3f(source, AL_DIRECTION, direction.x, direction.y, direction.z);
	AL_CHECK_ERROR("setting source direction")
	
	return AAL_OK;
}

aalError Instance::SetCone(const aalCone & cone) {
	
	if(!alIsSource(source) || !(channel.flags & AAL_FLAG_CONE)) {
		return AAL_ERROR_INIT;
	}
	
	channel.cone.inner_angle = cone.inner_angle;
	channel.cone.outer_angle = cone.outer_angle;
	channel.cone.outer_volume = clamp(cone.outer_volume, 0.f, 1.f);
	
	alSourcef(source, AL_CONE_INNER_ANGLE, channel.cone.inner_angle);
	alSourcef(source, AL_CONE_OUTER_ANGLE, channel.cone.outer_angle);
	alSourcef(source, AL_CONE_OUTER_GAIN, channel.cone.outer_volume);
	AL_CHECK_ERROR("setting source cone")
	
	return AAL_OK;
}

aalError Instance::SetFalloff(const aalFalloff & falloff) {
	
	if(!alIsSource(source) || !(channel.flags & AAL_FLAG_FALLOFF)) {
		return AAL_ERROR_INIT;
	}
	
	channel.falloff = falloff;
	
	alSourcef(source, AL_MAX_DISTANCE, falloff.end);
	alSourcef(source, AL_REFERENCE_DISTANCE, falloff.start);
	AL_CHECK_ERROR("setting source falloff")
	
	return AAL_OK;
}

aalError Instance::SetMixer(aalSLong mixer) {
	
	channel.mixer = mixer;
	
	return SetVolume(channel.volume);
}

aalError Instance::SetEnvironment(aalSLong environment) {
	
	channel.environment = environment;
	
	// TODO
	return AAL_ERROR_SYSTEM;
}

aalError Instance::GetPosition(aalVector & position) const {
	if(!(channel.flags & AAL_FLAG_POSITION)) {
		return AAL_ERROR_INIT;
	}
	position = channel.position;
	return AAL_OK;
}

aalError Instance::GetFalloff(aalFalloff & falloff) const {
	if(!(channel.flags & AAL_FLAG_FALLOFF)) {
		return AAL_ERROR_INIT;
	}
	falloff = channel.falloff;
	return AAL_OK;
}

bool Instance::IsPlaying() {
	return (status == PLAYING);
}

bool Instance::IsIdled() {
	return (status == IDLED);
}

aalULong Instance::Time(const aalUnit & unit) {
	return BytesToUnits(time, sample->format, unit);
}

aalError Instance::Play(const aalULong & play_count) {
	
	if(status != PLAYING) {
		
 		LogAL("Play(" << play_count << ") vol=" << channel.volume);
		
		status = PLAYING;
		time = read = written = 0;
		
		callb_i = channel.flags & AAL_FLAG_CALLBACK ? 0 : 0xffffffff;
		
		alSourcei(source, AL_SEC_OFFSET, 0);
		AL_CHECK_ERROR("set source offset")
	}
	
	if(play_count && loadCount != (aalULong)-1) {
		loadCount += play_count;
	} else {
		loadCount = (aalULong)-1;
	}
	
	if(streaming) {
		if(aalError error = fillAllBuffers()) {
			return error;
		}
	} else {
		ALint queuedBuffers;
		alGetSourcei(source, AL_BUFFERS_QUEUED, &queuedBuffers);
		AL_CHECK_ERROR("getting queued buffer count")
		aalULong nbuffers = MAXLOOPBUFFERS;
		for(aalULong i = queuedBuffers; i < nbuffers && loadCount ; i++) {
			LogAL("queueing buffer " << buffers[0]);
			alSourceQueueBuffers(source, 1, &buffers[0]);
			AL_CHECK_ERROR("queueing buffer")
			load();
		}
	}
	
	if(aalError error = sourcePlay()) {
		return error;
	}
	
	return AAL_OK;
}

aalError Instance::Stop() {
	
	if(status == IDLED) {
		return AAL_OK;
	}
	
	LogAL("Stop");
	
	alSourceStop(source);
	alSourceRewind(source);
	alSourcei(source, AL_BUFFER, 0);
	AL_CHECK_ERROR("stopping source")
	
	if(streaming) {
		for(size_t i = 0; i < NBUFFERS; i++) {
			if(buffers[i] && alIsBuffer(buffers[i])) {
				LogAL("deleting buffer " << buffers[i]);
				alDeleteBuffers(1, &buffers[i]);
				nbbuffers--;
				AL_CHECK_ERROR("deleting buffer")
				buffers[i] = 0;
			}
		}
	}
	
	status = IDLED;
	
	return AAL_OK;
}

aalError Instance::Pause() {
	
	if(status == IDLED) {
		return AAL_OK;
	}
	
	LogAL("Pause");
	
	alSourcePause(source);
	AL_CHECK_ERROR("pausing source")
	status = PAUSED;
	
	return AAL_OK;
}

aalError Instance::Resume() {
	
	if(status == IDLED) {
		return AAL_OK;
	}
	
	LogAL("Resume");
	
	status = PLAYING;
	
	if(isTooFar()) {
		return AAL_OK;
	}
	
	return sourcePlay();
}

bool Instance::isTooFar() {
	
	if(!(channel.flags & AAL_FLAG_POSITION) || !alIsSource(source)) {
		return false;
	}
	
	ALfloat max;
	alGetSourcef(source, AL_MAX_DISTANCE, &max);
	AL_CHECK_ERROR("getting source max distance")
	
	aalVector listener_pos;
	if(channel.flags & AAL_FLAG_RELATIVE) {
		listener_pos.x = listener_pos.y = listener_pos.z = 0.0F;
	} else {
		alGetListener3f(AL_POSITION, &listener_pos.x, &listener_pos.y, &listener_pos.z);
		AL_CHECK_ERROR("getting listener position")
	}
	
	float dist = Distance(listener_pos, channel.position);
	
	if(tooFar) {
		
		if(dist > max) {
			return true;
		}
		tooFar = false;
		AL_CLEAR_ERROR();
		sourcePlay();
		return false;
		
	} else {
		
		if(dist <= max) {
			return false;
		}
		tooFar = true;
		alSourcePause(source);
		return true;
		
	}
}

aalError Instance::Update() {
	
	if(status != PLAYING) {
		return AAL_OK;
	}
	
	if(isTooFar()) {
		return AAL_OK;
	}
	
	AL_CLEAR_ERROR();
	
	
	// Stream data / queue buffers.
	
	ALint nbuffers;
	alGetSourcei(source, AL_BUFFERS_PROCESSED, &nbuffers);
	AL_CHECK_ERROR("getting processed buffer count")
	arx_assert(nbuffers >= 0);
	
	ALint maxbuffers = (streaming ? (ALint)NBUFFERS : MAXLOOPBUFFERS);
	arx_assert(nbuffers <= maxbuffers);
	if(loadCount && nbuffers == maxbuffers) {
		ALWarning << "buffer underrun detected";
	}
	
	bool oldLoadCount = loadCount;
	
	aalULong oldTime = time;
	
	for(ALint i = 0; i < nbuffers; i++) {
		
		ALuint buffer;
		alSourceUnqueueBuffers(source, 1, &buffer);
		AL_CHECK_ERROR("unqueueing buffer")
		
		ALint bufsize;
		alGetBufferi(buffer, AL_SIZE, &bufsize);
		AL_CHECK_ERROR("getting buffer size")
		
		LogAL("done playing buffer " << buffer << " with " << bufsize << " bytes");
		
		time += bufsize;
		
		if(streaming) {
			if(loadCount) {
				fillBuffer(buffer, stream_limit_bytes);
				LogAL("queueing buffer " << buffer);
				alSourceQueueBuffers(source, 1, &buffer);
				AL_CHECK_ERROR("queueing buffer")
			} else {
				int deleted = 0;
				for(size_t i = 0; i < NBUFFERS; i++) {
					if(buffers[i] == buffer) {
						buffers[i] = 0;
						deleted++;
					}
				}
				arx_assert(deleted == 1);
				LogAL("deleting buffer " << buffer);
				alDeleteBuffers(1, &buffer);
				nbbuffers--;
				AL_CHECK_ERROR("deleting buffer")
			}
		} else if(loadCount) {
			LogAL("re-queueing buffer " << buffer);
			alSourceQueueBuffers(source, 1, &buffer);
			AL_CHECK_ERROR("queueing buffer")
			load();
		}
		
	}
	
	
	// Check if we are done playing.
	
	aalError ret;
	if(!oldLoadCount) {
		ALint buffersQueued;
		alGetSourcei(source, AL_BUFFERS_QUEUED, &buffersQueued);
		if(!buffersQueued) {
			LogAL("done playing");
			ret = Stop();
		} else {
			ret = sourcePlay();
		}
	} else {
		ret = sourcePlay();
	}
	
	
	// Inform callbacks about the time played.
	
	ALint newRead;
	alGetSourcei(source, AL_BYTE_OFFSET, &newRead);
	AL_CHECK_ERROR("getting source byte offset")
	arx_assert(newRead >= 0);
	
	time = time - read + newRead;
	LogAL("Update: read " << read << " -> " << newRead << "  time " << oldTime << " -> " << time);
	read = newRead;
	
	if(time < oldTime) {
		ALError << "what just happened?";
	}
	
	arx_assert(time >= oldTime); // TODO this fails sometimes
	
	while(true) {
		
		// Check if it's time to launch a callback
		while(callb_i < sample->callb_c && sample->callb[callb_i].time <= time) {
			LogAL("invoking callback " << callb_i << " for time " << sample->callb[callb_i].time);
			sample->callb[callb_i].func(this, id, sample->callb[callb_i].data);
			callb_i++;
		}
		
		if(time < sample->length) {
			break;
		}
		
		time -= sample->length;
		
		if(channel.flags & AAL_FLAG_CALLBACK) {
			callb_i = 0;
		}
		
	}
	
	return ret;
}

bool Instance::load() {
	return (loadCount == (aalULong)-1 || --loadCount);
}

} // namespace ATHENA
