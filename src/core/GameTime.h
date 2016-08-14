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
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#ifndef ARX_CORE_GAMETIME_H
#define ARX_CORE_GAMETIME_H

#include "core/TimeTypes.h"
#include "graphics/Math.h"

#include "platform/Time.h"


class PlatformTime {
	
	u64 m_frameStartTime;
	u64 m_lastFrameStartTime;
	u64 m_lastFrameDuration;
	
public:
	PlatformTime()
		: m_frameStartTime(0)
		, m_lastFrameStartTime(0)
		, m_lastFrameDuration(0)
	{ }
	
	inline void updateFrame() {
		u64 currentTime = platform::getTimeUs();
		
		m_frameStartTime = currentTime;
		if(m_lastFrameStartTime == 0) {
			m_lastFrameStartTime = currentTime;
		}
		arx_assert(m_frameStartTime >= m_lastFrameStartTime);
		
		m_lastFrameDuration = m_frameStartTime - m_lastFrameStartTime;
		
		m_lastFrameStartTime = currentTime;
	}
	
	inline PlatformInstant frameStart() {
		return PlatformInstant(m_frameStartTime);
	}

	inline PlatformDuration lastFrameDuration() {
		return PlatformDuration(m_lastFrameDuration);
	}
};

extern PlatformTime g_platformTime;


class GameTime {
	
public:
	
	GameTime();
	~GameTime() {}
	
	void init();
	
	void pause();
	void resume();
	
	void force_time_restore(ArxInstant time);
	
	// TODO probably the source of the need to clip frame_delay
	void force_frame_time_restore(const ArxInstant v) {
		frame_time_us = v * 1000;
		last_frame_time_us = v * 1000;
	}
	
	float now_f() const {
		return m_now_us / 1000.0f;
	}
	
	ArxInstant now() const {
		return checked_range_cast<ArxInstant>(m_now_us / 1000);
	}
	
	void update(bool use_pause = true) {
		if (is_paused() && use_pause) {
			m_now_us = platform::getElapsedUs(start_time, pause_time);
		} else {
			m_now_us = platform::getElapsedUs(start_time);
		}
	}
	
	bool is_paused() const { 
		return paused; 
	}
	
	// used only for "slow time" spell
	void increment_start_time(u64 inc) {
		start_time += inc;
	}
	
	float get_frame_time() const { 
		return frame_time_us / 1000.0f; 
	}
	
	float get_frame_delay() const {
		return frame_delay_ms;
	}
	
	void update_frame_time() {
		update();
		frame_time_us = m_now_us;
		frame_delay_ms = (frame_time_us - last_frame_time_us) / 1000.0f;
	}
	
	void update_last_frame_time() {
		last_frame_time_us = frame_time_us;
	}
	
private:
	
	bool paused;
	
	// these values are expected to wrap
	u64 pause_time;
	u64 start_time;
	
	// TODO this sometimes respects pause and sometimes not!
	u64 m_now_us;
	
	u64 last_frame_time_us;
	u64 frame_time_us;
	float frame_delay_ms;
};

extern GameTime arxtime;

#endif // ARX_CORE_GAMETIME_H
