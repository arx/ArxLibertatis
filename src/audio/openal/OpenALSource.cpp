/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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

#if ARX_HAVE_OPENAL_EFX
#include <efx.h>
#endif

#include "audio/openal/OpenALUtils.h"
#include "audio/AudioGlobal.h"
#include "audio/AudioResource.h"
#include "audio/Stream.h"
#include "audio/Sample.h"
#include "audio/Mixer.h"
#include "graphics/Math.h"
#include "io/resource/ResourcePath.h"
#include "io/log/Logger.h"
#include "math/Vector.h"
#include "platform/Platform.h"

namespace audio {

const size_t StreamLimitBytes = 176400;

#define ALPREFIX "[" << (s16)(m_id.source().handleData()) << \
                 "," << (s16)(m_id.getSampleId().handleData()) << \
                 "," << (m_sample ? m_sample->getName() : "(none)") << \
                 "," << nbsources << "," << nbbuffers << "," << m_loadCount << "] "

#undef ALError
#define ALError LogError << ALPREFIX
#define ALWarning LogWarning << ALPREFIX
#define LogAL(x) LogDebug(ALPREFIX << x)
#define TraceAL(x) LogDebug(ALPREFIX << x)

static size_t nbsources = 0;
static size_t nbbuffers = 0;

// How often to queue the buffer when looping but not streaming.
#define MAXLOOPBUFFERS std::max((size_t)NBUFFERS, NBUFFERS * StreamLimitBytes / m_sample->getLength())

aalError OpenALSource::sourcePlay() {
	
	ALint val;
	alGetSourcei(m_source, AL_SOURCE_STATE, &val);
	AL_CHECK_ERROR("getting source state")
	
	if(val == AL_STOPPED) {
		return updateBuffers();
	} else if(val == AL_INITIAL || val == AL_PAUSED) {
		alSourcePlay(m_source);
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
	
	alSourcePause(m_source);
	AL_CHECK_ERROR("pausing source")
	
	ALint val;
	alGetSourcei(m_source, AL_SOURCE_STATE, &val);
	AL_CHECK_ERROR("getting source state")
	
	if(val == AL_STOPPED) {
		return updateBuffers();
	}
	
	return AAL_OK;
}

OpenALSource::OpenALSource(Sample * sample) :
	Source(sample),
	m_tooFar(false),
	m_streaming(false), m_loadCount(0), m_written(0), m_stream(NULL),
	m_read(0),
	m_source(0),
	m_refcount(NULL),
	m_volume(1.f) {
	for(size_t i = 0; i < NBUFFERS; i++) {
		m_buffers[i] = 0;
	}
}

OpenALSource::~OpenALSource() {
	
	LogAL("clean");
	
	if(alIsSource(m_source)) {
		
		alSourceStop(m_source);
		AL_CHECK_ERROR_N("stopping source")
		
		alDeleteSources(1, &m_source);
		nbsources--;
		AL_CHECK_ERROR_N("deleting source")
		
		m_source = 0;
	} else {
		arx_assert(!m_source);
	}
	
	if(m_streaming) {
		for(size_t i = 0; i < NBUFFERS; i++) {
			if(m_buffers[i] && alIsBuffer(m_buffers[i])) {
				TraceAL("deleting buffer " << m_buffers[i]);
				alDeleteBuffers(1, &m_buffers[i]);
				nbbuffers--;
				AL_CHECK_ERROR_N("deleting buffer")
				m_buffers[i] = 0;
			}
		}
		arx_assert(!m_refcount);
	} else {
		if(m_buffers[0]) {
			arx_assert(!m_refcount || *m_refcount > 0);
			if(!m_refcount || !--*m_refcount) {
				delete m_refcount, m_refcount = NULL;
				TraceAL("deleting buffer " << m_buffers[0]);
				alDeleteBuffers(1, &m_buffers[0]);
				nbbuffers--;
				AL_CHECK_ERROR_N("deleting buffer")
			}
		} else {
			arx_assert(!m_refcount);
		}
		for(size_t i = 1; i < NBUFFERS; i++) {
			arx_assert(!m_buffers[i]);
		}
	}
	
	if(m_stream) {
		deleteStream(m_stream), m_stream = NULL;
	}
	
}

bool OpenALSource::convertStereoToMono() {
	return ((m_channel.flags & FLAG_ANY_3D_FX) && m_sample->getFormat().channels == 2);
}

aalError OpenALSource::init(SourcedSample id, OpenALSource * instance, const Channel & channel) {
	
	arx_assert(!m_source);
	
	m_id = id;
	
	m_channel = channel;
	if(m_channel.flags & FLAG_ANY_3D_FX) {
		m_channel.flags &= ~FLAG_PAN;
	}
	
	if(instance && !instance->m_streaming && convertStereoToMono() == instance->convertStereoToMono()) {
		
		arx_assert(instance->m_sample == m_sample);
		
		arx_assert(instance->m_buffers[0] != 0);
		m_buffers[0] = instance->m_buffers[0];
		m_bufferSizes[0] = instance->m_bufferSizes[0];
		if(!instance->m_refcount) {
			instance->m_refcount = new unsigned int;
			*instance->m_refcount = 1;
		}
		m_refcount = instance->m_refcount;
		(*m_refcount)++;
		
	}
	
	alGenSources(1, &m_source);
	nbsources++;
	alSourcei(m_source, AL_LOOPING, AL_FALSE);
	AL_CHECK_ERROR("generating source")
	
	m_streaming = (m_sample->getLength() > (StreamLimitBytes * NBUFFERS));
	
	LogAL("init: length=" << m_sample->getLength() << " " << (m_streaming ? "m_streaming" : "static")
	      << (m_buffers[0] ? " (copy)" : ""));
	
	if(!m_streaming && !m_buffers[0]) {
		m_stream = createStream(m_sample->getName());
		if(!m_stream) {
			ALError << "error creating stream";
			return AAL_ERROR_FILEIO;
		}
		alGenBuffers(1, &m_buffers[0]);
		nbbuffers++;
		AL_CHECK_ERROR("generating buffer")
		arx_assert(m_buffers[0] != 0);
		m_loadCount = 1;
		if(aalError error = fillBuffer(0, m_sample->getLength())) {
			return error;
		}
		arx_assert(!m_stream && !m_loadCount);
	}
	
	setVolume(m_channel.volume);
	setPitch(m_channel.pitch);
	
	if(!(m_channel.flags & FLAG_POSITION) || (m_channel.flags & FLAG_RELATIVE)) {
		alSourcei(m_source, AL_SOURCE_RELATIVE, AL_TRUE);
		AL_CHECK_ERROR("setting relative flag")
	}
	
	// Create 3D interface if required
	if(m_channel.flags & FLAG_ANY_3D_FX) {
		setPosition(m_channel.position);
		setVelocity(m_channel.velocity);
		setFalloff(m_channel.falloff);
	} else {
		setPan(m_channel.pan);
	}
	
	return AAL_OK;
}

aalError OpenALSource::fillAllBuffers() {
	
	arx_assert(m_streaming);
	
	if(!m_loadCount) {
		return AAL_OK;
	}
	
	if(!m_stream) {
		m_stream = createStream(m_sample->getName());
		if(!m_stream) {
			ALError << "error creating stream";
			return AAL_ERROR_FILEIO;
		}
	}
	
	for(size_t i = 0; i < NBUFFERS && m_loadCount; i++) {
		
		if(m_buffers[i] && alIsBuffer(m_buffers[i])) {
			continue;
		}
		
		alGenBuffers(1, &m_buffers[i]);
		nbbuffers++;
		AL_CHECK_ERROR("generating buffer")
		arx_assert(m_buffers[i] != 0);
		
		if(aalError error = fillBuffer(i, StreamLimitBytes)) {
			return error;
		}
		
		TraceAL("queueing buffer " << m_buffers[i]);
		alSourceQueueBuffers(m_source, 1, &m_buffers[i]);
		AL_CHECK_ERROR("queueing buffer")
		
	}
	
	return AAL_OK;
}

/*!
 * Convert a stereo buffer to mono in-place.
 * \tparam T The type of one (mono) sound sample.
 * \return the size of the converted buffer
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
	
	arx_assert(m_loadCount > 0);
	
	size_t left = std::min(size, m_sample->getLength() - m_written);
	if(m_loadCount == 1) {
		size = left;
	}
	
	TraceAL("filling buffer " << m_buffers[i] << " with " << size << " bytes");
	
	char data[StreamLimitBytes * NBUFFERS];
	
	arx_assert(size <= sizeof(data));
	
	size_t read;
	m_stream->read(data, left, read);
	if(read != left) {
		return AAL_ERROR_SYSTEM;
	}
	m_written += read;
	arx_assert(m_written <= m_sample->getLength());
	if(m_written == m_sample->getLength()) {
		m_written = 0;
		if(!markAsLoaded()) {
			deleteStream(m_stream);
			m_stream = NULL;
		} else {
			m_stream->setPosition(0);
			if(size > left) {
				m_stream->read(data + left, size - left, read);
				if(read != size - left) {
					return AAL_ERROR_SYSTEM;
				}
				m_written += read;
				arx_assert(m_written < m_sample->getLength());
			}
		}
	}
	
	const PCMFormat & f = m_sample->getFormat();
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
	
	alBufferData(m_buffers[i], alformat, data, alsize, f.frequency);
	AL_CHECK_ERROR("setting buffer data")
	
	m_bufferSizes[i] = size;
	
	return AAL_OK;
}

aalError OpenALSource::updateVolume() {
	
	if(!alIsSource(m_source)) {
		return AAL_ERROR_INIT;
	}
	
	const Mixer * mixer = g_mixers[m_channel.mixer];
	float volume = mixer ? mixer->getFinalVolume() : 1.f;
	
	if(volume > 0.f && (m_channel.flags & FLAG_VOLUME)) {
		// LogToLinearVolume(LinearToLogVolume(volume) * m_channel.volume)
		volume = std::pow(100000.f * volume, m_channel.volume) / 100000.f;
	}
	
	alSourcef(m_source, AL_GAIN, volume * m_volume);
	AL_CHECK_ERROR("setting source gain")
	
	return AAL_OK;
}

aalError OpenALSource::setPitch(float pitch) {
	
	if(!alIsSource(m_source) || !(m_channel.flags & FLAG_PITCH)) {
		return AAL_ERROR_INIT;
	}
	
	m_channel.pitch = glm::clamp(pitch, 0.1f, 2.f);
	
	alSourcef(m_source, AL_PITCH, m_channel.pitch);
	AL_CHECK_ERROR("setting source pitch")
	
	return AAL_OK;
}

aalError OpenALSource::setPan(float pan) {
	
	if(!alIsSource(m_source) || !(m_channel.flags & FLAG_PAN)) {
		return AAL_ERROR_INIT;
	}
	
	m_channel.pan = glm::clamp(pan, -1.f, 1.f);
	
	if(m_channel.pan != 0.f && m_sample->getFormat().channels != 1) {
		ALWarning << "panning only supported for mono samples";
		return AAL_ERROR_SYSTEM;
	}
	
	// Emulate pan using a listener-relative position
	float distance = 0.1f; // something within the min attenuation distance;
	Vec3f position = distance * angleToVectorXZ(-m_channel.pan * 90.f);
	alSource3f(m_source, AL_POSITION, position.x, position.z, position.y); // xzy swizzle
	AL_CHECK_ERROR("setting source pan")
	
	return AAL_OK;
}

aalError OpenALSource::setPosition(const Vec3f & position) {
	
	if(!alIsSource(m_source) || !(m_channel.flags & FLAG_POSITION)) {
		return AAL_ERROR_INIT;
	}
	
	arx_assert(isallfinite(position));
	
	m_channel.position = position;
	
	if(!isallfinite(position)) {
		return AAL_ERROR; // OpenAL soft will lock up if given NaN or +-Inf here
	}
	
	alSource3f(m_source, AL_POSITION, position.x, position.y, position.z);
	AL_CHECK_ERROR("setting source position")
	
	return AAL_OK;
}

aalError OpenALSource::setVelocity(const Vec3f & velocity) {
	
	if(!alIsSource(m_source) || !(m_channel.flags & FLAG_VELOCITY)) {
		return AAL_ERROR_INIT;
	}
	
	arx_assert(isallfinite(velocity));
	
	m_channel.velocity = velocity;
	
	if(!isallfinite(velocity)) {
		return AAL_ERROR; // OpenAL soft will lock up if given NaN or +-Inf here
	}
	
	alSource3f(m_source, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
	AL_CHECK_ERROR("setting source velocity")
	
	return AAL_OK;
}

aalError OpenALSource::setFalloff(const SourceFalloff & falloff) {
	
	if(!alIsSource(m_source) || !(m_channel.flags & FLAG_FALLOFF)) {
		return AAL_ERROR_INIT;
	}
	
	m_channel.falloff = falloff;
	
	alSourcef(m_source, AL_MAX_DISTANCE, falloff.end);
	alSourcef(m_source, AL_REFERENCE_DISTANCE, falloff.start);
	AL_CHECK_ERROR("setting source falloff")
	
	return AAL_OK;
}

aalError OpenALSource::play(unsigned playCount) {
	
	if(status != Playing) {
		
		LogAL("play(" << playCount << ") vol=" << m_channel.volume);
		
		status = Playing;
		
		m_read = m_written = 0;
		reset();
		
		alSourcei(m_source, AL_SEC_OFFSET, 0);
		AL_CHECK_ERROR("set source offset")
		
	} else {
		TraceAL("play(+" << playCount << ") vol=" << m_channel.volume);
	}
	
	if(playCount && m_loadCount != unsigned(-1)) {
		m_loadCount += playCount;
	} else {
		m_loadCount = unsigned(-1);
	}
	
	if(m_streaming) {
		if(aalError error = fillAllBuffers()) {
			return error;
		}
	} else {
		ALint queuedBuffers;
		alGetSourcei(m_source, AL_BUFFERS_QUEUED, &queuedBuffers);
		AL_CHECK_ERROR("getting queued buffer count")
		size_t nbuffers = MAXLOOPBUFFERS;
		for(size_t i = queuedBuffers; i < nbuffers && m_loadCount; i++) {
			TraceAL("queueing buffer " << m_buffers[0]);
			alSourceQueueBuffers(m_source, 1, &m_buffers[0]);
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
	
	alSourceStop(m_source);
	alSourceRewind(m_source);
	alSourcei(m_source, AL_BUFFER, 0);
	AL_CHECK_ERROR("stopping source")
	
	if(m_streaming) {
		for(size_t i = 0; i < NBUFFERS; i++) {
			if(m_buffers[i] && alIsBuffer(m_buffers[i])) {
				TraceAL("deleting buffer " << m_buffers[i]);
				alDeleteBuffers(1, &m_buffers[i]);
				nbbuffers--;
				AL_CHECK_ERROR("deleting buffer")
				m_buffers[i] = 0;
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
	
	if(!(m_channel.flags & FLAG_POSITION) || !(m_channel.flags & FLAG_FALLOFF)
	   || !alIsSource(m_source)) {
		return false;
	}
	
	Vec3f listener_pos(0.f);
	if(!(m_channel.flags & FLAG_RELATIVE)) {
		alGetListener3f(AL_POSITION, &listener_pos.x, &listener_pos.y, &listener_pos.z);
		AL_CHECK_ERROR_C("getting listener position", return m_tooFar;)
	}
	
	float d = glm::distance(m_channel.position, listener_pos);
	
	if(m_tooFar) {
		if(d <= m_channel.falloff.end) {
			LogAL("in range");
			m_tooFar = false;
			sourcePlay();
		}
	} else {
		if(d > m_channel.falloff.end) {
			LogAL("out of range");
			m_tooFar = true;
			sourcePause();
			if(m_loadCount <= 1) {
				stop();
			}
		}
	}
	
	if(!m_tooFar) {
		d = (d - m_channel.falloff.start) / (m_channel.falloff.end - m_channel.falloff.start);
		float v = 1.f - glm::clamp((d - 0.75f) / (1.f - 0.75f), 0.f, 1.f);
		if(m_volume != v) {
			m_volume = v;
			updateVolume();
		}
	}
	
	return m_tooFar;
}

aalError OpenALSource::updateBuffers() {
	
	// Stream data / queue buffers.
	
	// We need to get the source state before the number of processed buffers to prevent a race condition with the source reaching the end of the last buffer.
	ALint sourceState;
	alGetSourcei(m_source, AL_SOURCE_STATE, &sourceState);
	AL_CHECK_ERROR("getting source state")
	arx_assert(sourceState != AL_INITIAL && sourceState != AL_PAUSED);
	
	ALint nbuffersProcessed;
	alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &nbuffersProcessed);
	AL_CHECK_ERROR("getting processed buffer count")
	arx_assert(nbuffersProcessed >= 0);
	
	ALint maxbuffers = (m_streaming ? ALint(NBUFFERS) : ALint(MAXLOOPBUFFERS));
	arx_assert(nbuffersProcessed <= maxbuffers);
	if(m_loadCount && nbuffersProcessed == maxbuffers) {
		ALWarning << "buffer underrun detected";
	}
	
	unsigned oldLoadCount = m_loadCount;
	
	size_t oldTime = time;
	
	for(ALint c = 0; c < nbuffersProcessed; c++) {
		
		ALuint buffer;
		alSourceUnqueueBuffers(m_source, 1, &buffer);
		AL_CHECK_ERROR("unqueueing buffer")
		
		size_t i = 0;
		if(m_streaming) {
			for(; m_buffers[i] != buffer; i++) {
				arx_assert(i + 1 < NBUFFERS);
			}
		}
		
		TraceAL("done playing buffer " << buffer << " (" << i << ") with " << m_bufferSizes[i] << " bytes");
		
		/*
		 * We can't use the AL_SIZE buffer attribute here as it describes the internal buffer size,
		 * which might differ from the original size as the OpenAL implementation may convert the data.
		 */
		time += m_bufferSizes[i];
		
		if(m_streaming) {
			if(m_loadCount) {
				fillBuffer(i, StreamLimitBytes);
				TraceAL("queueing buffer " << buffer);
				alSourceQueueBuffers(m_source, 1, &buffer);
				AL_CHECK_ERROR("queueing buffer")
			} else {
				TraceAL("deleting buffer " << buffer);
				alDeleteBuffers(1, &buffer);
				m_buffers[i] = 0;
				nbbuffers--;
				AL_CHECK_ERROR("deleting buffer")
			}
		} else if(m_loadCount) {
			TraceAL("re-queueing buffer " << buffer);
			alSourceQueueBuffers(m_source, 1, &buffer);
			AL_CHECK_ERROR("queueing buffer")
			markAsLoaded();
		}
		
	}
	
	
	// Check if we are done playing.
	
	aalError ret = AAL_OK;
	if(oldLoadCount == 0) {
		ALint buffersQueued;
		alGetSourcei(m_source, AL_BUFFERS_QUEUED, &buffersQueued);
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
			alSourcePlay(m_source);
			AL_CHECK_ERROR("playing source")
		} else if(sourceState != AL_PLAYING) {
			ALError << "unexpected source state: " << sourceState;
			ret = AAL_ERROR;
		}
	}
	
	
	// Inform callbacks about the time played.
	
	ALint newRead;
	alGetSourcei(m_source, AL_BYTE_OFFSET, &newRead);
	AL_CHECK_ERROR("getting source byte offset")
	arx_assert(newRead >= 0);
	
	arx_assert(status == Playing || newRead == 0);
	
	if(convertStereoToMono()) {
		newRead *= 2;
	}
	
	if(newRead == 0 && m_read != 0 && nbuffersProcessed == 0) {
		/*
		 * OAL reached the end of the last buffer between the alGetSourcei(AL_BUFFERS_PROCESSED) and
		 * alGetSourcei(AL_BYTE_OFFSET) calls and was so nice to reset the AL_BYTE_OFFSET to 0
		 * even though we haven't yet unqueued the buffer.
		 * We need to process played buffers before we can replace old 'm_read' with 'newRead.
		 * This will be done in the next updateBuffers() call.
		 */
		ALint newSourceState;
		alGetSourcei(m_source, AL_SOURCE_STATE, &newSourceState);
		AL_CHECK_ERROR("getting source state")
		arx_assert(newSourceState == AL_STOPPED);
		arx_assert(status == Playing);
		return ret;
	}
	
	time = time - m_read + newRead;
	TraceAL("update: read " << m_read << " -> " << newRead << "  time " << oldTime << " -> " << time);
	
	arx_assert_msg(time >= oldTime, "oldTime=%lu time=%lu read=%lu newRead=%d"
	               " nbuffersProcessed=%d status=%d sourceState=%d", (unsigned long)oldTime,
	               (unsigned long)time, (unsigned long)m_read, newRead, nbuffersProcessed,
	               (int)status, sourceState);
	ARX_UNUSED(oldTime);
	m_read = newRead;
	
	return ret;
}

bool OpenALSource::markAsLoaded() {
	return (m_loadCount == unsigned(-1) || --m_loadCount);
}

aalError OpenALSource::setRolloffFactor(float factor) {
	
	alSourcef(m_source, AL_ROLLOFF_FACTOR, factor);
	AL_CHECK_ERROR("setting rolloff factor");
	
	return AAL_OK;
}

#if ARX_HAVE_OPENAL_EFX
void OpenALSource::setEffectSlot(ALuint slot) {
	if(m_channel.flags & FLAG_REVERBERATION) {
		alSource3i(m_source, AL_AUXILIARY_SEND_FILTER, slot, 0, AL_FILTER_NULL);
	}
}
#endif

} // namespace audio
