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
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#include "core/GameTime.h"

#include "platform/Time.h"

arx::time arxtime;

arx::time::time() {

	// TODO can't call init from constructor Time::getUs() requires init
	// potential out-of-order construction resulting in a divide-by-zero
	start_time      = 0ull;
	pause_time      = 0ull;
	paused          = false;
	delta_time      = 0.0f;
	frame_time      = 0.0f;
	last_frame_time = 0.0f;
	frame_delay     = 0.0f;
}

void arx::time::init() {
	
	start_time      = Time::getUs();
	pause_time      = 0ull;
	paused          = false;
	delta_time      = 0.0f;
	frame_time      = 0.0f;
	last_frame_time = 0.0f;
	frame_delay     = 0.0f;
}

void arx::time::pause() {
	
	if (!is_paused())
	{
		pause_time = Time::getUs();
		paused     = true;
	}
}

void arx::time::resume() {
	
	if (is_paused()) 
	{
		start_time += Time::getElapsedUs(pause_time);

		pause_time = 0ull;
		paused     = false;
	}
}

void arx::time::force_time_restore(const float &time) {
	
	u64 requested_time = u64(time * 1000.0f);
	
	start_time = Time::getElapsedUs(requested_time);
	delta_time = float(requested_time) / 1000.0f;
	
	pause_time = 0ull;
	paused     = false;
}
