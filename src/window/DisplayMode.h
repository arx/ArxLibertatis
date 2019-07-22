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

#ifndef ARX_WINDOW_DISPLAYMODE_H
#define ARX_WINDOW_DISPLAYMODE_H

#include "math/Vector.h"
#include "platform/Platform.h"

struct DisplayMode {
	
	Vec2i resolution;
	s32 refresh;
	
	DisplayMode() : resolution(0), refresh(0) { }
	/* implicit */ DisplayMode(Vec2i res, s32 rate = 0) : resolution(res), refresh(rate) { }
	bool operator<(const DisplayMode & o) const;
	bool operator==(const DisplayMode & o) const {
		return resolution == o.resolution && refresh == o.refresh;
	}
	bool operator!=(const DisplayMode & o) const {
		return !(*this == o);
	}
	
};

#endif // ARX_WINDOW_DISPLAYMODE_H
