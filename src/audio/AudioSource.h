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

#ifndef ARX_AUDIO_AUDIOSOURCE_H
#define ARX_AUDIO_AUDIOSOURCE_H

#include <stddef.h>
#include <vector>
#include <utility>

#include "audio/AudioTypes.h"
#include "math/Types.h"

namespace audio {

class Sample;

/*!
 * An audio source that can play one sample.
 */
class Source {
	
public:
	
	class Callback {
		
	private:
		
		virtual void onSamplePosition(Source & source, size_t position) = 0;
		
		friend class Source;
	};
	
	/*!
	 * Set the volume of this source and update the volume calculated from the sources mixers.
	 * \param volume The new source volume. The volume will be clamped to the range [0,1].
	 */
	aalError setVolume(float volume);
	
	/*!
	 * Set the pitch of this source and update the pitch calculated from the sources mixers.
	 * \param pitch The new source pitch. The pitch will be clamped to the range [0,1].
	 */
	virtual aalError setPitch(float pitch) = 0;
	
	/*!
	 * Set the panning of this source.
	 * \param pan The new source panning. The pan will be clamped to the range [-1,1].
	 */
	virtual aalError setPan(float pan) = 0;
	
	virtual aalError setPosition(const Vec3f & position) = 0;
	virtual aalError setVelocity(const Vec3f & velocity) = 0;
	virtual aalError setFalloff(const SourceFalloff & falloff) = 0;
	aalError setMixer(MixerId mixer);
	
	/*!
	 * Play the source. A source that is already playing is not stopped / rewinded, but the playCount increased by the provided amount.
	 * \param playCount How often to play the sample. 0 means loop forever.
	 */
	virtual aalError play(unsigned playCount = 1) = 0;
	
	/*!
	 * Stop the source and rewind the sample position.
	 */
	virtual aalError stop() = 0;
	virtual aalError pause() = 0;
	virtual aalError resume() = 0;
	aalError update();
	
	SourcedSample getId() const { return m_id; }
	Sample * getSample() const { return m_sample; }
	const Channel & getChannel() const { return m_channel; }
	SourceStatus getStatus() const { return status; }
	bool isPlaying() const { return status == Playing; }
	bool isIdle() const { return status == Idle; }
	
	/*!
	 * Re-calculate the final volume from the channel and mixer volumes.
	 */
	virtual aalError updateVolume() = 0;
	
	void addCallback(Callback * callback, size_t position);
	
protected:
	
	explicit Source(Sample * sample);
	virtual ~Source();
	
	SourcedSample m_id;
	
	Channel m_channel;
	
	Sample * m_sample;
	SourceStatus status;
	
	size_t time; // Elapsed 'time'
	
	void reset() { time = 0, callback_i = 0; }
	
	/*!
	 * Check if this source is too far from the listener and play/pause it accordingly.
	 * \return true if the source is too far and should not be played
	 */
	virtual bool updateCulling() = 0;
	
	virtual aalError updateBuffers() = 0;
	
private:
	
	typedef std::vector<std::pair<Callback *, size_t> > CallbackList;
	CallbackList callbacks;
	size_t callback_i;
	
	void updateCallbacks();
	
};

} // namespace audio

#endif // ARX_AUDIO_AUDIOSOURCE_H
