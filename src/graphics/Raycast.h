/*
 * Copyright 2016-2019 Arx Libertatis Team (see the AUTHORS file)
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
#ifndef ARX_GRAPHICS_RAYCAST_H
#define ARX_GRAPHICS_RAYCAST_H

#include "graphics/GraphicsTypes.h"
#include "math/Types.h"
#include "util/Flags.h"

class Entity;

enum RaycastFlag {
	RaycastAnyHit,
	RaycastIgnorePlayer,
};
DECLARE_FLAGS(RaycastFlag, RaycastFlags)
DECLARE_FLAGS_OPERATORS(RaycastFlags)

struct RaycastResult {
	
	EERIEPOLY * hit;
	Vec3f pos;
	
	RaycastResult()
		: hit(NULL)
		, pos(0.f)
	{ }
	
	RaycastResult(EERIEPOLY * hit_, Vec3f pos_)
		: hit(hit_)
		, pos(pos_)
	{ }
	
	operator bool() const {
		return hit != NULL;
	}
	
};

bool RaycastLightFlare(const Vec3f & start, const Vec3f & end);
RaycastResult raycastScene(const Vec3f & start, const Vec3f & end, PolyType ignored = POLY_TRANS,
                           RaycastFlags flags = 0);

struct EntityRaycastResult {
	
	Entity * entity;
	EERIE_FACE * face;
	Vec3f pos;
	
	EntityRaycastResult()
		: entity(NULL)
		, face(NULL)
		, pos(0.f)
	{ }
	
	EntityRaycastResult(Entity * entity_, EERIE_FACE * face_, Vec3f pos_)
		: entity(entity_)
		, face(face_)
		, pos(pos_)
	{ }
	
	operator bool() const {
		return entity != NULL;
	}
	
};

EntityRaycastResult raycastEntities(const Vec3f & start, const Vec3f & end, PolyType ignored = POLY_TRANS,
                                    RaycastFlags flags = 0);

void RaycastDebugClear();
void RaycastDebugDraw();

#endif // ARX_GRAPHICS_RAYCAST_H
