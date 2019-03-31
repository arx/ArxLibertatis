/*
 * Copyright 2019 Arx Libertatis Team (see the AUTHORS file)
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

#include "core/FpsCounter.h"

#include "core/GameTime.h"

FpsCounter g_fpsCounter;

void FpsCounter::CalcFPS(bool reset) {
	
	if(reset) {
		dwFrames = 0;
		fLastTime = 0;
		FPS = 7.f * FPS;
	} else {
		// Keep track of the time lapse and frame count
		PlatformInstant fTime = g_platformTime.frameStart();
		++dwFrames;

		float tmp = toS(fTime - fLastTime);

		// Update the frame rate once per second
		if(tmp > 1.f) {
			FPS = float(dwFrames) / tmp;
			fLastTime = fTime;
			dwFrames  = 0;
		}
	}
}
