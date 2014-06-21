/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_PHYSICS_PROJECTILE_H
#define ARX_PHYSICS_PROJECTILE_H

#include "game/GameTypes.h"
#include "graphics/GraphicsTypes.h"

void ARX_THROWN_OBJECT_Throw(EntityHandle source, const Vec3f & position, const Vec3f & vect, const glm::quat & quat, float velocity, float damages, float poisonous);

void ARX_THROWN_OBJECT_KillAll();
void ARX_THROWN_OBJECT_Manage(unsigned long time_offset);
void ARX_THROWN_OBJECT_Render();

#endif // ARX_PHYSICS_PROJECTILE_H
