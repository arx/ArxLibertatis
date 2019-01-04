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

#ifndef ARX_GRAPHICS_DRAWDEBUG_H
#define ARX_GRAPHICS_DRAWDEBUG_H

#include "core/TimeTypes.h"
#include "graphics/Color.h"
#include "math/Types.h"

void drawDebugInitialize();
void drawDebugRelease();

void drawDebugCycleViews();

void drawDebugRender();

namespace debug {

void drawRay(Vec3f start, Vec3f dir, Color color = Color::white, PlatformDuration duration = 0);

} // namespace debug

#endif // ARX_GRAPHICS_DRAWDEBUG_H
