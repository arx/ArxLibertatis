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

#include "platform/Flags.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/effects/Trail.h"

enum ThrownObjectFlag {
	ATO_EXIST      = (1<<0),
	ATO_MOVING     = (1<<1),
	ATO_UNDERWATER = (1<<2),
	ATO_FIERY      = (1<<3)
};
DECLARE_FLAGS(ThrownObjectFlag, ThrownObjectFlags)
DECLARE_FLAGS_OPERATORS(ThrownObjectFlags)

struct ARX_THROWN_OBJECT {
	ThrownObjectFlags flags;
	Vec3f vector;
	EERIE_QUAT quat;
	Vec3f initial_position;
	float velocity;
	Vec3f position;
	float damages;
	EERIE_3DOBJ * obj;
	long source;
	unsigned long creation_time;
	float poisonous;
	Trail * pRuban;
};

const size_t MAX_THROWN_OBJECTS = 100;

extern ARX_THROWN_OBJECT Thrown[MAX_THROWN_OBJECTS];



long ARX_THROWN_OBJECT_GetFree();
long ARX_THROWN_OBJECT_Throw(long source, Vec3f * position, Vec3f * vect, EERIE_QUAT * quat, float velocity, float damages, float poisonous);
void ARX_THROWN_OBJECT_KillAll();
void ARX_THROWN_OBJECT_Manage(unsigned long time_offset);
void ARX_THROWN_OBJECT_Render();

#endif // ARX_PHYSICS_PROJECTILE_H
