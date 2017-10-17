/*
 * Copyright 2011-2016 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_AUDIO_AMBIANCE_H
#define ARX_AUDIO_AMBIANCE_H

#include <stddef.h>
#include <string>
#include <vector>

#include "core/TimeTypes.h"
#include "audio/AudioTypes.h"
#include "io/resource/ResourcePath.h"
#include "platform/Platform.h"

namespace audio {

class Ambiance {
	
public:
	
	explicit Ambiance(const res::path & name);
	~Ambiance();
	
	aalError load();
	
	void setType(PlayingAmbianceType type) { m_type = type; }
	PlayingAmbianceType getType() const { return m_type; }
	
	const Channel & getChannel() const { return channel; }
	const res::path & getName() const { return name; }
	
	aalError setVolume(float volume);
	
	bool isPaused() const { return status == Paused; }
	bool isPlaying() const { return status == Playing; }
	bool isIdle() const { return status == Idle; }
	bool isLooped() const { return loop; }
	
	aalError play(const Channel & channel, bool loop = true, PlatformDuration fade_interval = 0);
	aalError stop(PlatformDuration fade_interval = 0);
	aalError pause();
	aalError resume();
	aalError update();
	
	struct Track;
	typedef std::vector<Track> TrackList;
	
private:
	
	enum Fade {
		None,
		FadeUp,
		FadeDown
	};
	
	enum Status {
		Idle,
		Playing,
		Paused
	};
	
	Status status;
	bool loop;
	Fade fade;
	
	Channel channel;
	PlatformDuration fade_time;
	PlatformDuration fade_interval;
	float fade_max;
	PlatformInstant start;
	PlatformDuration m_time;
	
	TrackList tracks;
	
	res::path name;
	
	PlayingAmbianceType m_type;
	
};

} // namespace audio

#endif // ARX_AUDIO_AMBIANCE_H
