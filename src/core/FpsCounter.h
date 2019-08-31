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

#ifndef ARX_CORE_FPSCOUNTER_H
#define ARX_CORE_FPSCOUNTER_H

#include "core/TimeTypes.h"

class FpsCounter {
	PlatformInstant fLastTime;
	u32 dwFrames;
	
public:
	FpsCounter()
		: fLastTime(0)
		, dwFrames(0)
		, FPS(0.f)
	{ }
	
	float FPS;
	void CalcFPS(bool reset = false);
};

extern FpsCounter g_fpsCounter;

#endif // ARX_CORE_FPSCOUNTER_H
