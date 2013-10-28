/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#include "audio/openal/OpenALSource.h"

#include <cmath>
#include <algorithm>

#include "audio/openal/OpenALUtils.h"
#include "audio/AudioGlobal.h"
#include "audio/AudioResource.h"
#include "audio/Stream.h"
#include "audio/Sample.h"
#include "audio/Mixer.h"
#include "io/resource/ResourcePath.h"
#include "io/log/Logger.h"
#include "math/Vector.h"
#include "platform/Platform.h"

namespace audio {

#define ALPREFIX "[" << (s16)(((id)&0xffff0000)>>16) << "," << (s16)((id)&0xffff) << "," << (sample ? sample->getName() : "(none)") << "," << nbsources << "," << nbbuffers << "," << loadCount << "] "

#undef ALError
#define ALError LogError << ALPREFIX
#define ALWarning LogWarning << ALPREFIX
#define LogAL(x) LogDebug(ALPREFIX << x)
#define TraceAL(x) LogDebug(ALPREFIX << x)

static size_t nbsources = 0;
static size_t nbbuffers = 0;

// How often to queue the buffer when looping but not streaming.
#define MAXLOOPBUFFERS std::max((size_t)NBUFFERS, NBUFFERS * stream_limit_bytes / sample->getLength())

aalError OpenALSource::sourcePlay() {
	
	ALint val;
	alGetSourcei(source, AL_SOURCE_STATE, &val);
	AL_CHECK_ERROR("getting source state")
	
	if(val == AL_STOPPED) {
			return updateBuffers();
	} else if(val == AL_INITIAL || val == AL_PAUSED) {
		alSourcePlay(source);
		AL_CHECK_ERROR("playing source")
		return AAL_OK;
	} else if(val == AL_PLAYING) {
		return AAL_OK;
	} else {
		ALError << "unexpected source state: " << val;
		return AAL_ERROR;
	}
	
}

aalError OpenALSource::sourcePause() {
	
	alSourcePause(source);
	AL_CHECK_ERROR("pausing source")
	
	ALint val;
	alGetSourcei(source, AL_SOURCE_STATE, &val);
	AL_CHECK_ERROR("getting source state")
	
	if(val == AL_STOPPED) {
		return updateBuffers();
	}
	
	return AAL_OK;
}

OpenALSource::OpenALSource(Sample * _sample) :
	Source(_sample),
	tooFar(false),
	streaming(false), loadCount(0), written(0), stream(NULL),
	read(0),
	source(0),
	refcount(NULL) {
	for(size_t i = 0; i < NBUFFERS; i++) {
		buffers[i] = 0;
	}
}

OpenALSource::~OpenALSource() {
	
	LogAL("clean");
	
	if(alIsSource(source)) {
		
		alSourceStop(source);
		AL_CHECK_ERROR_N("stopping source",)
		
		alDeleteSources(1, &source);
		nbsources--;
		AL_CHECK_ERROR_N("deleting source",)
		
		source = 0;
	} else {
		arx_assert(!source);
	}
	
	if(streaming) {
		for(size_t i = 0; i < NBUFFERS; i++) {
			if(buffers[i] && alIsBuffer(buffers[i])) {
				TraceAL("deleting buffer " << buffers[i]);
				alDeleteBuffers(1, &buffers[i]);
				nbbuffers--;
				AL_CHECK_ERROR_N("deleting buffer",)
				buffers[i] = 0;
			}
		}
		arx_assert(!refcount);
	} else {
		if(buffers[0]) {
			arx_assert(!refcount || *refcount > 0);
			if(!refcount || !--*refcount) {
				delete refcount, refcount = NULL;
				TraceAL("deleting buffer " << buffers[0]);
				alDeleteBuffers(1, &buffers[0]);
				nbbuffers--;
				AL_CHECK_ERROR_N("deleting buffer",)
			}
		} else {
			arx_assert(!refcount);
		}
		for(size_t i = 1; i < NBUFFERS; i++) {
			arx_assert(!buffers[i]);
		}
	}
	
	if(stream) {
		deleteStream(stream), stream = NULL;
	}
	
}

bool OpenALSource::convertStereoToMono() {
	return ((channel.flags & FLAG_ANY_3D_FX) && sample->getFormat().channels == 2);
}

aalError OpenALSource::init(SourceId _id, OpenALSource * inst, const Channel & _channel) {
	
	arx_assert(!source);
	
	id = _id;
	
	channel = _channel;
	if(channel.flags & FLAG_ANY_3D_FX) {
		channel.flags &= ~FLAG_PAN;
	}
	
	if(inst && !inst->streaming && convertStereoToMono() == inst->convertStereoToMono()) {
		
		arx_assert(inst->sample == sample);
		
		arx_assert(inst->buffers[0] != 0);
		buffers[0] = inst->buffers[0];
		bufferSizes[0] = inst->bufferSizes[0];
		if(!inst->refcount) {
			inst->refcount = new unsigned int;
			*inst->refcount = 1;
		}
		refcount = inst->refcount;
		(*refcount)++;
		
	}
	
	alGenSources(1, &source);
	nbsources++;
	alSourcei(source, AL_LOOPING, AL_FALSE);
	AL_CHECK_ERROR("generating source")
	
	streaming = (sample->getLength() > (stream_limit_bytes * NBUFFERS));
	
	LogAL("init: length=" << sample->getLength() << " " << (streaming ? "streaming" : "static") << (buffers[0] ? " (copy)" : ""));
	
	if(!streaming && !buffers[0]) {
		stream = createStream(sample->getName());
		if(!stream) {
			ALError << "error creating stream";
			return AAL_ERROR_FILEIO;
		}
		alGenBuffers(1, &buffers[0]);
		nbbuffers++;
		AL_CHECK_ERROR("generating buffer")
		arx_assert(buffers[0] != 0);
		loadCount = 1;
		if(aalError error = fillBuffer(0, sample->getLength())) {
			return error;
		}
		arx_assert(!stream && !loadCount);
	}
	
	setVolume(channel.volume);
	setPitch(channel.pitch);
	
	if(!(channel.flags & FLAG_POSITION) || (channel.flags & FLAG_RELATIVE)) {
		alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
		AL_CHECK_ERROR("setting relative flag")
	}
	
	// Create 3D interface if required
	if(channel.flags & FLAG_ANY_3D_FX) {
		
		if(sample->getFormat().channels != 1) {
			// TODO(broken-assets) this is not supported by OpenAL so we will need to convert the sample
		}
		
		setPosition(channel.position);
		setVelocity(channel.velocity);
		setDirection(channel.direction);
		setCone(channel.cone);
		setFalloff(channel.falloff);
		
	} else {
		setPan(channel.pan);
	}
	
	return AAL_OK;
}

aalError OpenALSource::fillAllBuffers() {
	
	arx_assert(streaming);
	
	if(!loadCount) {
		return AAL_OK;
	}
	
	if(!stream) {
		stream = createStream(sample->getName());
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
		
		if(aalError error = fillBuffer(i, stream_limit_bytes)) {
			return error;
		}
		
		TraceAL("queueing buffer " << buffers[i]);
		alSourceQueueBuffers(source, 1, &buffers[i]);
		AL_CHECK_ERROR("queueing buffer")
		
	}
	
	return AAL_OK;
}

/*!
 * Convert a stereo buffer to mono in-place.
 * @param T The type of one (mono) sound sample.
 * @return the size of the converted buffer
 */
template <class T>
static size_t stereoToMono(char * data, size_t size) {
	
	T * buf = reinterpret_cast<T *>(data);
	
	size_t nbsamples = size / sizeof(T);
	arx_assert(nbsamples % 2 == 0);
	
	for(size_t in = 0, out = 0; in < nbsamples - 1; in += 2, out++) {
		buf[out] = T((int(buf[in]) + int(buf[in + 1])) / 2);
	}
	
	return size / 2;
}

aalError OpenALSource::fillBuffer(size_t i, size_t size) {
	
	arx_assert(loadCount > 0);
	
	size_t left = std::min(size, (size_t)sample->getLength() - written);
	if(loadCount == 1) {
		size = left;
	}
	
	TraceAL("filling buffer " << buffers[i] << " with " << size << " bytes");
	
	char * data = new char[size];
	if(!data) {
		return AAL_ERROR_MEMORY;
	}
	
	size_t read;
	stream->read(data, left, read);
	if(read != left) {
		delete[] data;
		return AAL_ERROR_SYSTEM;
	}
	written += read;
	arx_assert(written <= sample->getLength());
	if(written == sample->getLength()) {
		written = 0;
		if(!markAsLoaded()) {
			deleteStream(stream);
			stream = NULL;
		} else {
			stream->setPosition(0);
			if(size > left) {
				stream->read(data + left, size - left, read);
				if(read != size - left) {
					delete[] data;
					return AAL_ERROR_SYSTEM;
				}
				written += read;
				arx_assert(written < sample->getLength());
			}
		}
	}
	
	const PCMFormat & f = sample->getFormat();
	if((f.channels != 1 && f.channels != 2) || (f.quality != 8 && f.quality != 16)) {
		LogError << "Unsupported audio format: quality=" << f.quality << " channels=" << f.channels;
		return AAL_ERROR_SYSTEM;
	}
	
	ALenum alformat;
	if(f.channels == 1 || convertStereoToMono()) {
		alformat = (f.quality == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
	} else {
		alformat = (f.quality == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
	}
	
	size_t alsize = size;
	if(convertStereoToMono()) {
		alsize = (f.quality == 8) ? stereoToMono<s8>(data, size) : stereoToMono<s16>(data, size);
	}
	
	alBufferData(buffers[i], alformat, data, alsize, f.frequency);
	delete[] data;
	AL_CHECK_ERROR("setting buffer data")
	
	bufferSizes[i] = size;
	
	return AAL_OK;
}

aalError OpenALSource::updateVolume() {
	
	if(!alIsSource(source) || !(channel.flags & FLAG_VOLUME)) {
		return AAL_ERROR_INIT;
	}
	
	const Mixer * mixer = _mixer[channel.mixer];
	float volume = mixer ? mixer->getFinalVolume() : 1.f;
	
	if(volume) {
		// LogToLinearVolume(LinearToLogVolume(volume) * channel.volume)
		volume = std::pow(100000.f * volume, channel.volume) / 100000.f;
	}
	
	alSourcef(source, AL_GAIN, volume);
	AL_CHECK_ERROR("setting source gain")
	
	return AAL_OK;
}

aalError OpenALSource::setPitch(float p) {
	
	if(!alIsSource(source) || !(channel.flags & FLAG_PITCH)) {
		return AAL_ERROR_INIT;
	}
	
	channel.pitch = clamp(p, 0.1f, 2.f);
	
	alSourcef(source, AL_PITCH, channel.pitch);
	AL_CHECK_ERROR("setting source pitch")
	
	return AAL_OK;
}

aalError OpenALSource::setPan(float p) {
	
	if(!alIsSource(source) || !(channel.flags & FLAG_PAN)) {
		return AAL_ERROR_INIT;
	}
	
	float oldPan = channel.pan;
	
	channel.pan = clamp(p, -1.f, 1.f);
	
	if(channel.pan != 0.f && oldPan == 0.f) {
		// TODO OpenAL doesn't have a pan feature, but it isn't used much (only in abiances?)
		ALWarning << "paning not supported";
	}
	
	return AAL_OK;
}

aalError OpenALSource::setPosition(const Vec3f & position) {
	
	if(!alIsSource(source) || !(channel.flags & FLAG_POSITION)) {
		return AAL_ERROR_INIT;
	}
	
	channel.position = position;
	
	if(!isallfinite(position)) {
		return AAL_ERROR; // OpenAL soft will lock up if given NaN or +-Inf here
	}
	
	alSource3f(source, AL_POSITION, position.x, position.y, position.z);
	AL_CHECK_ERROR("setting source position")
	
	return AAL_OK;
}

aalError OpenALSource::setVelocity(const Vec3f & velocity) {
	
	if(!alIsSource(source) || !(channel.flags & FLAG_VELOCITY)) {
		return AAL_ERROR_INIT;
	}
	
	channel.velocity = velocity;
	
	if(!isallfinite(velocity)) {
		return AAL_ERROR; // OpenAL soft will lock up if given NaN or +-Inf here
	}
	
	alSource3f(source, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
	AL_CHECK_ERROR("setting source velocity")
	
	return AAL_OK;
}

aalError OpenALSource::setDirection(const Vec3f & direction) {
	
	if(!alIsSource(source) || !(channel.flags & FLAG_DIRECTION)) {
		return AAL_ERROR_INIT;
	}
	
	channel.direction = direction;
	
	alSource3f(source, AL_DIRECTION, direction.x, direction.y, direction.z);
	AL_CHECK_ERROR("setting source direction")
	
	return AAL_OK;
}

aalError OpenALSource::setCone(const SourceCone & cone) {
	
	if(!alIsSource(source) || !(channel.flags & FLAG_CONE)) {
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

aalError OpenALSource::setFalloff(const SourceFalloff & falloff) {
	
	if(!alIsSource(source) || !(channel.flags & FLAG_FALLOFF)) {
		return AAL_ERROR_INIT;
	}
	
	channel.falloff = falloff;
	
	alSourcef(source, AL_MAX_DISTANCE, falloff.end);
	alSourcef(source, AL_REFERENCE_DISTANCE, falloff.start);
	AL_CHECK_ERROR("setting source falloff")
	
	return AAL_OK;
}

aalError OpenALSource::play(unsigned play_count) {
	
	if(status != Playing) {
		
		LogAL("play(" << play_count << ") vol=" << channel.volume);
		
		status = Playing;
		
		read = written = 0;
		reset();
		
		alSourcei(source, AL_SEC_OFFSET, 0);
		AL_CHECK_ERROR("set source offset")
		
	} else {
		TraceAL("play(+" << play_count << ") vol=" << channel.volume);
	}
	
	if(play_count && loadCount != (unsigned)-1) {
		loadCount += play_count;
	} else {
		loadCount = (unsigned)-1;
	}
	
	if(streaming) {
		if(aalError error = fillAllBuffers()) {
			return error;
		}
	} else {
		ALint queuedBuffers;
		alGetSourcei(source, AL_BUFFERS_QUEUED, &queuedBuffers);
		AL_CHECK_ERROR("getting queued buffer count")
		size_t nbuffers = MAXLOOPBUFFERS;
		for(size_t i = queuedBuffers; i < nbuffers && loadCount; i++) {
			TraceAL("queueing buffer " << buffers[0]);
			alSourceQueueBuffers(source, 1, &buffers[0]);
			AL_CHECK_ERROR("queueing buffer")
			markAsLoaded();
		}
	}
	
	return sourcePlay();
}

aalError OpenALSource::stop() {
	
	if(status == Idle) {
		return AAL_OK;
	}
	
	LogAL("stop");
	
	alSourceStop(source);
	alSourceRewind(source);
	alSourcei(source, AL_BUFFER, 0);
	AL_CHECK_ERROR("stopping source")
	
	if(streaming) {
		for(size_t i = 0; i < NBUFFERS; i++) {
			if(buffers[i] && alIsBuffer(buffers[i])) {
				TraceAL("deleting buffer " << buffers[i]);
				alDeleteBuffers(1, &buffers[i]);
				nbbuffers--;
				AL_CHECK_ERROR("deleting buffer")
				buffers[i] = 0;
			}
		}
	}
	
	status = Idle;
	
	return AAL_OK;
}

aalError OpenALSource::pause() {
	
	if(status == Idle || status == Paused) {
		return AAL_OK;
	}
	
	LogAL("pause");
	
	status = Paused;
	
	sourcePause();
	
	return AAL_OK;
}

aalError OpenALSource::resume() {
	
	if(status == Idle || status == Playing) {
		return AAL_OK;
	}
	
	LogAL("resume");
	
	status = Playing;
	
	if(updateCulling()) {
		return AAL_OK;
	}
	
	return sourcePlay();
}

bool OpenALSource::updateCulling() {
	
	arx_assert(status == Playing);
	
	if(!(channel.flags & FLAG_POSITION) || !alIsSource(source)) {
		return false;
	}
	
	ALfloat max;
	alGetSourcef(source, AL_MAX_DISTANCE, &max);
	AL_CHECK_ERROR_N("getting source max distance", return tooFar;)
	
	Vec3f listener_pos;
	if(channel.flags & FLAG_RELATIVE) {
		listener_pos = Vec3f_ZERO;
	} else {
		alGetListener3f(AL_POSITION, &listener_pos.x, &listener_pos.y, &listener_pos.z);
		AL_CHECK_ERROR_N("getting listener position", return tooFar;)
	}
	
	float d = glm::distance(channel.position, listener_pos);
	
	if(tooFar) {
		
		if(d > max) {
			return true;
		}
		
		LogAL("in range");
		tooFar = false;
		sourcePlay();
		return false;
		
	} else {
		
		if(d <= max) {
			return false;
		}
		
		LogAL("out of range");
		tooFar = true;
		sourcePause();
		if(loadCount <= 1) {
			stop();
		}
		return true;
		
	}
}

aalError OpenALSource::updateBuffers() {
	
	// Stream data / queue buffers.
	
	// We need to get the source state before the number of processed buffers to prevent a race condition with the source reaching the end of the last buffer.
	ALint sourceState;
	alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
	AL_CHECK_ERROR("getting source state")
	arx_assert(sourceState != AL_INITIAL && sourceState != AL_PAUSED);
	
	ALint nbuffersProcessed;
	alGetSourcei(source, AL_BUFFERS_PROCESSED, &nbuffersProcessed);
	AL_CHECK_ERROR("getting processed buffer count")
	arx_assert(nbuffersProcessed >= 0);
	
	ALint maxbuffers = (streaming ? (ALint)NBUFFERS : MAXLOOPBUFFERS);
	arx_assert(nbuffersProcessed <= maxbuffers);
	if(loadCount && nbuffersProcessed == maxbuffers) {
		ALWarning << "buffer underrun detected";
	}
	
	unsigned oldLoadCount = loadCount;
	
	size_t oldTime = time;
	
	for(ALint c = 0; c < nbuffersProcessed; c++) {
		
		ALuint buffer;
		alSourceUnqueueBuffers(source, 1, &buffer);
		AL_CHECK_ERROR("unqueueing buffer")
		
		size_t i = 0;
		if(streaming) {
			for(; buffers[i] != buffer; i++) {
				arx_assert(i + 1 < NBUFFERS);
			}
		}
		
		TraceAL("done playing buffer " << buffer << " (" << i << ") with " << bufferSizes[i] << " bytes");
		
		/*
		 * We can't use the AL_SIZE buffer attribute here as it describes the internal buffer size,
		 * which might differ from the original size as the OpenAL implementation may convert the data.
		 */
		time += bufferSizes[i];
		
		if(streaming) {
			if(loadCount) {
				fillBuffer(i, stream_limit_bytes);
				TraceAL("queueing buffer " << buffer);
				alSourceQueueBuffers(source, 1, &buffer);
				AL_CHECK_ERROR("queueing buffer")
			} else {
				TraceAL("deleting buffer " << buffer);
				alDeleteBuffers(1, &buffer);
				buffers[i] = 0;
				nbbuffers--;
				AL_CHECK_ERROR("deleting buffer")
			}
		} else if(loadCount) {
			TraceAL("re-queueing buffer " << buffer);
			alSourceQueueBuffers(source, 1, &buffer);
			AL_CHECK_ERROR("queueing buffer")
			markAsLoaded();
		}
		
	}
	
	
	// Check if we are done playing.
	
	aalError ret = AAL_OK;
	if(oldLoadCount == 0) {
		ALint buffersQueued;
		alGetSourcei(source, AL_BUFFERS_QUEUED, &buffersQueued);
		AL_CHECK_ERROR("getting queued buffer count")
		if(!buffersQueued) {
			LogAL("done playing");
			ret = stop();
		}
	}
	
	if(status == Playing) {
		if(sourceState == AL_STOPPED) {
			if(nbuffersProcessed != maxbuffers) {
				ALWarning << "buffer underrun detected";
			}
			alSourcePlay(source);
			AL_CHECK_ERROR("playing source")
		} else if(sourceState != AL_PLAYING) {
			ALError << "unexpected source state: " << sourceState;
			ret = AAL_ERROR;
		}
	}
	
	
	// Inform callbacks about the time played.
	
	ALint newRead;
	alGetSourcei(source, AL_BYTE_OFFSET, &newRead);
	AL_CHECK_ERROR("getting source byte offset")
	arx_assert(newRead >= 0);
	
	arx_assert(status == Playing || newRead == 0);
	
	if(convertStereoToMono()) {
		newRead *= 2;
	}
	
	if(newRead == 0 && read != 0 && nbuffersProcessed == 0) {
		/*
		 * OAL reached the end of the last buffer between the alGetSourcei(AL_BUFFERS_PROCESSED) and
		 * alGetSourcei(AL_BYTE_OFFSET) calls and was so nice to reset the AL_BYTE_OFFSET to 0
		 * even though we haven't yet unqueued the buffer.
		 * We need to process played buffers before we can replace old 'read' with 'newRead.
		 * This will be done in the next updateBuffers() call.
		 */
		ALint newSourceState;
		alGetSourcei(source, AL_SOURCE_STATE, &newSourceState);
		AL_CHECK_ERROR("getting source state")
		arx_assert(newSourceState == AL_STOPPED);
		arx_assert(status == Playing);
		return ret;
	}
	
	time = time - read + newRead;
	TraceAL("update: read " << read << " -> " << newRead << "  time " << oldTime << " -> " << time);
	
	arx_assert_msg(time >= oldTime, " oldTime=%lu time=%lu read=%lu newRead=%d nbuffersProcessed=%d status=%d sourceState=%d" , (unsigned long)oldTime, (unsigned long)time, (unsigned long)read, newRead, nbuffersProcessed, (int)status, sourceState);
	ARX_UNUSED(oldTime);
	read = newRead;
	
	return ret;
}

bool OpenALSource::markAsLoaded() {
	return (loadCount == (unsigned)-1 || --loadCount);
}

aalError OpenALSource::setRolloffFactor(float factor) {
	
	alSourcef(source, AL_ROLLOFF_FACTOR, factor);
	AL_CHECK_ERROR("setting rolloff factor");
	
	return AAL_OK;
}

} // namespace audio
