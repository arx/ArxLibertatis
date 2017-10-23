/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
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
#include "util/Flags.h"


class PlatformTime {
	
	PlatformInstant m_frameStartTime;
	PlatformDuration m_lastFrameDuration;
	
public:
	
	PlatformTime()
		: m_frameStartTime(0)
		, m_lastFrameDuration(0)
	{ }
	
	void updateFrame();
	
	void overrideFrameDuration(PlatformDuration duration) {
		m_lastFrameDuration = duration;
	}
	
	PlatformInstant frameStart() {
		return m_frameStartTime;
	}
	
	PlatformDuration lastFrameDuration() {
		return m_lastFrameDuration;
	}
	
};

extern PlatformTime g_platformTime;


class GameTime {
	
public:
	
	enum PauseFlag {
		PauseInitial   = (1 << 0),
		PauseMenu      = (1 << 1),
		PauseCinematic = (1 << 2),
		PauseUser      = (1 << 3),
	};
	DECLARE_FLAGS(PauseFlag, PauseFlags)
	
private:
	
	ArxInstant m_now;
	ArxDuration m_lastFrameDuration;
	float m_speed;
	PauseFlags m_paused;
	
public:
	
	GameTime();
	
	void pause(PauseFlags flags) {
		m_paused |= flags;
	}
	
	void resume(PauseFlags flags) {
		m_paused &= ~flags;
	}
	
	void reset(ArxInstant time);
	
	ArxInstant now() const {
		return m_now;
	}
	
	void update(PlatformDuration frameDuration);
	
	PauseFlags isPaused() const {
		return m_paused;
	}
	
	ArxDuration lastFrameDuration() const {
		return m_lastFrameDuration;
	}
	
	float speed() const { return m_speed; }
	void setSpeed(float speed) { m_speed = speed; }
	
};

DECLARE_FLAGS_OPERATORS(GameTime::PauseFlags)

extern GameTime g_gameTime;

#endif // ARX_CORE_GAMETIME_H
