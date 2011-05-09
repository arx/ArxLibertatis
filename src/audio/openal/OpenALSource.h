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

#ifndef ARX_AUDIO_OPENAL_OPENALSOURCE_H
#define ARX_AUDIO_OPENAL_OPENALSOURCE_H

#include <stddef.h>

#include "audio/openal/alwrapper.h"
#include "audio/AudioTypes.h"
#include "audio/AudioSource.h"

namespace audio {

class Sample;
class Stream;

class OpenALSource : public Source {
	
public:
	
	OpenALSource(Sample * sample);
	~OpenALSource();
	
	aalError init(SourceId id, OpenALSource * instance, const Channel & channel);
	
	aalError setVolume(float volume);
	aalError setPitch(float pitch);
	aalError setPan(float pan);
	
	aalError setPosition(const Vec3f & position);
	aalError setVelocity(const Vec3f & velocity);
	aalError setDirection(const Vec3f & direction);
	aalError setCone(const SourceCone & cone);
	aalError setFalloff(const SourceFalloff & falloff);
	aalError setMixer(MixerId mixer);
	
	size_t getTime(TimeUnit unit = UNIT_MS) const;
	
	aalError play(unsigned playCount = 1);
	aalError stop();
	aalError pause();
	aalError resume();
	aalError update();
	
	aalError updateVolume();
	
	aalError setRolloffFactor(float factor);
	
private:
	
	aalError sourcePlay();
	aalError sourcePause();
	
	/**
	 * Check if this source is too far from the listener and play/pause it accordingly.
	 * @return true if the source is too far and should not be played
	 */
	bool updateCulling();
	
	aalError updateBuffers();
	
	/*!
	 * Create buffers for all unused entries of the buffers array and fill them.
	 */
	aalError fillAllBuffers();
	
	/*!
	 * Fills the given buffer with the next size bytes of audio data from the current stream.
	 * Adjusts written and loadCount and closes the stream once loadCount reaches 0.
	 * @param i The index of the buffer to fill.
	 */
	aalError fillBuffer(size_t i, size_t size);
	
	bool markAsLoaded();
	
	bool tooFar; // True if the listener is too far from this source.
	
	/*
	 * Remaining play count, excluding queued buffers.
	 * For stream mode, the loadCount is decremented after the whole sample has been loaded.
	 * In that case, written will hold the amount ob bytes already loaded.
	 */
	bool streaming;
	unsigned loadCount;
	size_t written; // Streaming status
	Stream * stream;
	
	size_t time; // Elapsed 'time'
	size_t read;
	size_t callb_i; // Next callback index
	
	ALuint source;
	
	const static size_t NBUFFERS = 2;
	ALuint buffers[NBUFFERS];
	size_t bufferSizes[NBUFFERS];
	unsigned int * refcount; // reference count for shared buffers
	
};

} // namespace audio

#endif // ARX_AUDIO_OPENAL_OPENALSOURCE_H
