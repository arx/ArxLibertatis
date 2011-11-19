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

#ifndef ARX_CORE_GAMETIME_H
#define ARX_CORE_GAMETIME_H

#include "graphics/Math.h"

#include "platform/Time.h"

extern u64 ARXPausedTime;
extern u64 ARXStartTime;

// TODO this sometimes respects pause and sometimes not! [adejr]: is this TODO still valid?

// TODO an absolute time value stored in a floating point number will lose precision 
// when large numbers are stored. 
extern float ARXTime; 
extern bool ARXPausedTimer;

#define lARXTime (static_cast<long>( ARXTime ))
#define dwARX_TIME_Get() (static_cast<DWORD>(ARX_TIME_Get()))

void ARX_TIME_Pause();
void ARX_TIME_UnPause();
void ARX_TIME_Init();
void ARX_TIME_Force_Time_Restore(float time);

inline float ARX_TIME_Get(bool _bUsePause = true) {
	
	if(ARXPausedTimer && _bUsePause) {
		ARXTime = float(Time::getElapsedUs(ARXStartTime, ARXPausedTime)) / 1000;
	} else {
		ARXTime = float(Time::getElapsedUs(ARXStartTime)) / 1000;
	}
	
	return ARXTime;
}

inline unsigned long ARX_TIME_GetUL(bool _bUsePause = true) {
	float time = ARX_TIME_Get(_bUsePause);
	return checked_range_cast<unsigned long>(time);
}

inline unsigned long ARXTimeUL() {
	return checked_range_cast<unsigned long>(ARXTime);
}

#endif // ARX_CORE_GAMETIME_H
