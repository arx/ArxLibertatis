/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_AUDIO_DSOUND_DSOUNDSOURCE_H
#define ARX_AUDIO_DSOUND_DSOUNDSOURCE_H

#include "audio/dsound/dsoundfwd.h"

#include "audio/AudioTypes.h"
#include "audio/AudioSource.h"

namespace audio {

class Stream;
class Sample;
class DSoundBackend;

class DSoundSource : public Source {
	
public:
	
	DSoundSource(Sample * Sample, DSoundBackend * backend);
	~DSoundSource();
	
	aalError init(SourceId _id, const Channel & channel);
	aalError init(SourceId _id, DSoundSource * instance, const Channel & channel);
	
	aalError setPitch(float pitch);
	aalError setPan(float pan);
	
	aalError setPosition(const Vec3f & position);
	aalError setVelocity(const Vec3f & velocity);
	aalError setDirection(const Vec3f & direction);
	aalError setCone(const SourceCone & cone);
	aalError setFalloff(const SourceFalloff & falloff);
	
	// Control
	aalError play(unsigned playCount = 1);
	aalError stop();
	aalError pause();
	aalError resume();
	
	aalError updateVolume();
	
protected:
	
	bool updateCulling();
	
	aalError updateBuffers();
	
private:
	
	aalError init();
	void updateStreaming();
	aalError clean();
	bool checkPlaying();
	
	bool tooFar;
	unsigned loop; // Remaining loop count
	Stream * stream;
	size_t read, write; // Streaming status
	size_t size; // Buffer size
	LPDIRECTSOUNDBUFFER lpdsb;
	LPDIRECTSOUND3DBUFFER lpds3db;
	LPKSPROPERTYSET lpeax;
	
	DSoundBackend * backend;
	
};

} // namespace audio

#endif // ARX_AUDIO_DSOUND_DSOUNDSOURCE_H
