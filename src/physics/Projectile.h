/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "core/TimeTypes.h"
#include "game/GameTypes.h"
#include "graphics/GraphicsTypes.h"

class Trail;

enum ProjectileFlag {
	ATO_EXIST      = 1 << 0,
	ATO_MOVING     = 1 << 1,
	ATO_UNDERWATER = 1 << 2,
	ATO_FIERY      = 1 << 3
};
DECLARE_FLAGS(ProjectileFlag, ProjectileFlags)
DECLARE_FLAGS_OPERATORS(ProjectileFlags)

struct Projectile {
	
	ProjectileFlags flags;
	Vec3f vector;
	glm::quat quat;
	Vec3f initial_position;
	float velocity;
	Vec3f position;
	float damages;
	EERIE_3DOBJ * obj;
	EntityHandle source;
	GameInstant creation_time;
	float poisonous;
	Trail * m_trail;
	
	Projectile()
		: flags(0)
		, vector(0.f)
		, quat(quat_identity())
		, initial_position(0.f)
		, velocity(0.f)
		, position(0.f)
		, damages(0)
		, obj(NULL)
		, creation_time(0)
		, poisonous(0.f)
		, m_trail(NULL)
	{ }
	
};

void ARX_THROWN_OBJECT_Throw(EntityHandle source, const Vec3f & position, const Vec3f & vect, const glm::quat & quat, float velocity, float damages, float poisonous);

void ARX_THROWN_OBJECT_KillAll();
void ARX_THROWN_OBJECT_Manage(GameDuration timeDelta);
void ARX_THROWN_OBJECT_Render();

#endif // ARX_PHYSICS_PROJECTILE_H
