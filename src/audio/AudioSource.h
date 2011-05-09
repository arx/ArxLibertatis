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

#ifndef ARX_AUDIO_AUDIOSOURCE_H
#define ARX_AUDIO_AUDIOSOURCE_H

#include "audio/AudioTypes.h"

namespace audio {

class Sample;
class Stream;

/*!
 * An audio source that can play one sample.
 */
class Source {
	
public:
	
	enum Status {
		Idle,
		Playing,
		Paused
	};
	
	/*!
	 * Set the volume of this source and update the volume calculated from the sources mixers.
	 * @param volume The new source volume. The volume will be clamped to the range [0,1].
	 */
	virtual aalError setVolume(float volume) = 0;
	
	/*!
	 * Set the pitch of this source and update the pitch calculated from the sources mixers.
	 * @param pitch The new source pitch. The pitch will be clamped to the range [0,1].
	 */
	virtual aalError setPitch(float pitch) = 0;
	
	/*!
	 * Set the panning of this source.
	 * @param pitch The new source panning. The pan will be clamped to the range [-1,1].
	 */
	virtual aalError setPan(float pan) = 0;
	
	virtual aalError setPosition(const Vec3f & position) = 0;
	virtual aalError setVelocity(const Vec3f & velocity) = 0;
	virtual aalError setDirection(const Vec3f & direction) = 0;
	virtual aalError setCone(const SourceCone & cone) = 0;
	virtual aalError setFalloff(const SourceFalloff & falloff) = 0;
	virtual aalError setMixer(MixerId mixer) = 0;
	
	/*!
	 * Get the current play position in the sample.
	 * Updates to the return value may be deferred to calles to the update() methos.
	 */
	virtual size_t getTime(TimeUnit unit = UNIT_MS) const = 0;
	
	/*!
	 * Play the source. A source that is already playing is not stopped / rewinded, but the playCount increased by the provided amount.
	 * @param playCount How often to play the sample. 0 means loop forever.
	 */
	virtual aalError play(unsigned playCount = 1) = 0;
	
	/*!
	 * Stop the source and rewind the sample position.
	 */
	virtual aalError stop() = 0;
	virtual aalError pause() = 0;
	virtual aalError resume() = 0;
	virtual aalError update() = 0;
	
	inline SourceId getId() const { return id; }
	inline Sample * getSample() const { return sample; }
	inline const Channel & getChannel() const { return channel; }
	inline Status getStatus() const { return status; }
	inline bool isPlaying() const { return status == Playing; }
	inline bool isIdle() const { return status == Idle; }
	
	/*!
	 * Re-calculate the final volume from the channel and mixer volumes.
	 */
	virtual aalError updateVolume() = 0;
	
protected:
	
	Source(Sample * sample);
	virtual ~Source();
	
	SourceId id;
	
	Channel channel;
	
	Sample * sample;
	Status status;
	
};

} // namespace audio

#endif // ARX_AUDIO_AUDIOSOURCE_H
