/*
 * Copyright 2019-2022 Arx Libertatis Team (see the AUTHORS file)
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
	
	Vec2i resolution = Vec2i(0);
	s32 refresh = 0;
	
	constexpr DisplayMode() arx_noexcept_default
	/* implicit */ constexpr DisplayMode(Vec2i res, s32 rate = 0) noexcept : resolution(res), refresh(rate) { }
	[[nodiscard]] bool operator<(const DisplayMode & o) const noexcept;
	[[nodiscard]] constexpr bool operator==(const DisplayMode & o) const noexcept {
		return resolution == o.resolution && refresh == o.refresh;
	}
	[[nodiscard]] constexpr bool operator!=(const DisplayMode & o) const noexcept {
		return !(*this == o);
	}
	
};

#endif // ARX_WINDOW_DISPLAYMODE_H
