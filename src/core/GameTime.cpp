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

#include "core/GameTime.h"

#include "platform/Time.h"

PlatformTime g_platformTime;

GameTime arxtime;

GameTime::GameTime() {
	reset(ArxInstant_ZERO);
}

void GameTime::pause() {
	if(!is_paused()) {
		paused     = true;
	}
}

void GameTime::resume() {
	if(is_paused()) {
		paused     = false;
	}
}

void GameTime::reset(const ArxInstant time) {
	paused = true;
	m_now_us = time;
	m_frameDelay = ArxDuration_ZERO;
	m_speed = 1.f;
}

void GameTime::update(PlatformDuration frameDelay) {
	
	ArxDuration delta = ArxDurationUs(toUs(frameDelay));
	
	arx_assert(delta >= ArxDuration_ZERO);
	
	// Limit simulation time per frame
	delta = std::min(delta, ArxDurationMs(100));
	
	if(arxtime.speed() != 1.f) {
		delta -= ArxDurationMsf(toMsf(delta) * (1.f - arxtime.speed()));
	}
	
	arx_assert(delta >= ArxDuration_ZERO);
	
	if(is_paused()) {
		delta = ArxDuration_ZERO;
	}
	
	m_frameDelay = delta;
	m_now_us += delta;
}
