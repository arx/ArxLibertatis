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

#ifndef ARX_AUDIO_OPENAL_OPENALSOURCE_H
#define ARX_AUDIO_OPENAL_OPENALSOURCE_H

#include "Configure.h"

#include <stddef.h>

#include <al.h>

#include "audio/AudioTypes.h"
#include "audio/AudioSource.h"
#include "math/Types.h"
#include "platform/Platform.h"

namespace audio {

class Sample;
class Stream;

class OpenALSource arx_final : public Source {
	
public:
	
	explicit OpenALSource(Sample * sample);
	~OpenALSource();
	
	aalError init(SourcedSample id, OpenALSource * instance, const Channel & channel);
	
	aalError setPitch(float pitch);
	aalError setPan(float pan);
	
	aalError setPosition(const Vec3f & position);
	aalError setVelocity(const Vec3f & velocity);
	aalError setFalloff(const SourceFalloff & falloff);
	
	aalError play(unsigned playCount = 1);
	aalError stop();
	aalError pause();
	aalError resume();
	
	aalError updateVolume();
	
	aalError setRolloffFactor(float factor);
	
	#if ARX_HAVE_OPENAL_EFX
	void setEffectSlot(ALuint slot);
	#endif
	
protected:
	
	bool updateCulling();
	
	aalError updateBuffers();
	
private:
	
	aalError sourcePlay();
	aalError sourcePause();
	
	/*!
	 * Create buffers for all unused entries of the buffers array and fill them.
	 */
	aalError fillAllBuffers();
	
	/*!
	 * Fills the given buffer with the next size bytes of audio data from the current stream.
	 * Adjusts written and loadCount and closes the stream once loadCount reaches 0.
	 * \param i The index of the buffer to fill.
	 */
	aalError fillBuffer(size_t i, size_t size);
	
	bool markAsLoaded();
	
	/*!
	 * \return true if we need to convert a stereo sample to mono before passing it to OpenAL
	 */
	bool convertStereoToMono();
	
	bool m_tooFar; // True if the listener is too far from this source.
	
	/*
	 * Remaining play count, excluding queued buffers.
	 * For stream mode, the loadCount is decremented after the whole sample has been loaded.
	 * In that case, written will hold the amount ob bytes already loaded.
	 */
	bool m_streaming;
	unsigned m_loadCount;
	size_t m_written; // Streaming status
	Stream * m_stream;
	
	size_t m_read;
	
	ALuint m_source;
	
	enum { NBUFFERS = 2 };

	ALuint m_buffers[NBUFFERS];
	size_t m_bufferSizes[NBUFFERS];
	unsigned int * m_refcount; // Reference count for shared buffers
	
	float m_volume;
	
};

} // namespace audio

#endif // ARX_AUDIO_OPENAL_OPENALSOURCE_H
