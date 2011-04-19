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

#ifndef ARX_AUDIO_AUDIOINSTANCE_H
#define ARX_AUDIO_AUDIOINSTANCE_H

#include "audio/AudioTypes.h"
#include "audio/Sample.h"
#include "audio/Stream.h"
#include "audio/alwrapper.h"

namespace ATHENA {

class Instance {
	
public:
	
	Instance();
	~Instance();
	
	// Setup
	aalError Init(Sample * sample, const aalChannel & channel);
	aalError Init(Instance * instance, const aalChannel & channel);
	aalError Clean();
	aalError SetVolume(float volume);
	aalError SetPitch(float pitch);
	aalError SetPan(float pan);
	aalError SetPosition(const aalVector & position);
	aalError SetVelocity(const aalVector & velocity);
	aalError SetDirection(const aalVector & direction);
	aalError SetCone(const aalCone & cone);
	aalError SetFalloff(const aalFalloff & falloff);
	aalError SetMixer(aalSLong mixer);
	aalError SetEnvironment(aalSLong environment);
	
	//Status
	aalError GetPosition(aalVector & position) const;
	aalError GetFalloff(aalFalloff & falloff) const;
	bool IsIdled();
	bool IsPlaying();
	aalULong Time(const aalUnit & unit = AAL_UNIT_MS);
	
	// Control
	aalError Play(const aalULong & play_count = 1);
	aalError Stop();
	aalError Pause();
	aalError Resume();
	aalError Update();
	
	aalSLong id;
	
	inline Sample * getSample() { return sample; }
	inline const aalChannel & getChannel() { return channel; }
	
private:
	
	
	enum Status {
		PLAYING = 0, // New or playing
		IDLED  = 1,
		PAUSED = 2,
	};
	
	aalError sourcePlay();
	
	/**
	 * Check if this source is too far from the listener and play/pause it accordingly.
	 */
	bool isTooFar();
	
	aalError init();
	
	/*!
	 * Create buffers for all unused entries of the buffers array and fill them.
	 */
	aalError fillAllBuffers();
	
	/*!
	 * Fills the given buffer with the next size bytes of audio data from the current stream.
	 * Adjusts written and loadCount and closes the stream once loadCount reaches 0.
	 */
	aalError fillBuffer(ALuint buffer, size_t size);
	
	bool load();
	
	aalChannel channel;
	
	Sample * sample;
	Status status;
	bool tooFar; // True if the listener is too far from this source.
	
	/*
	 * Remaining play count, excluding queued buffers.
	 * For stream mode, the loadCount is decremented after the whole sample has been loaded.
	 * In that case, written will hold the amount ob bytes already loaded.
	 */
	bool streaming;
	aalULong loadCount;
	size_t written; // Streaming status
	Stream * stream;
	
	aalULong time; // Elapsed 'time'
	aalULong read;
	aalULong callb_i; // Next callback index
	
	ALuint source;
	
	const static size_t NBUFFERS = 2;
	ALuint buffers[NBUFFERS];
	
};

} // namespace ATHENA

#endif // ARX_AUDIO_AUDIOINSTANCE_H
