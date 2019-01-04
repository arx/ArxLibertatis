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

#ifndef ARX_PHYSICS_COLLISIONS_H
#define ARX_PHYSICS_COLLISIONS_H

#include <stddef.h>
#include <vector>

#include "game/Entity.h"
#include "graphics/data/Mesh.h"
#include "math/Types.h"
#include "util/Flags.h"

// ARX_COLLISIONS flags (cylinder move)
enum CollisionFlag {
	CFLAG_LEVITATE          = 1 << 0,
	CFLAG_NO_INTERCOL       = 1 << 1,
	CFLAG_EASY_SLIDING      = 1 << 3,
	CFLAG_CLIMBING          = 1 << 4,
	CFLAG_JUST_TEST         = 1 << 5,
	CFLAG_NPC               = 1 << 6,
	CFLAG_PLAYER            = 1 << 7,
	CFLAG_RETURN_HEIGHT     = 1 << 8,
	CFLAG_EXTRA_PRECISION   = 1 << 9,
	CFLAG_COLLIDE_NOCOL     = 1 << 12,
	CFLAG_NO_NPC_COLLIDE    = 1 << 13,
	CFLAG_NO_HEIGHT_MOD     = 1 << 14
};
DECLARE_FLAGS(CollisionFlag, CollisionFlags)
DECLARE_FLAGS_OPERATORS(CollisionFlags)

const size_t MAX_IN_SPHERE = 20;

extern size_t EXCEPTIONS_LIST_Pos;
extern EntityHandle EXCEPTIONS_LIST[MAX_IN_SPHERE + 1];

extern bool DIRECT_PATH;

bool ARX_COLLISION_Move_Cylinder(IO_PHYSICS * ip, Entity * io, float MOVE_CYLINDER_STEP, CollisionFlags flags = 0);
float CheckAnythingInCylinder(const Cylinder & cyl, Entity * ioo, long flags = 0);

enum CheckAnythingInSphereFlag {
	 CAS_NO_NPC_COL        = 1 << 0,
	 CAS_NO_SAME_GROUP     = 1 << 1,
	 CAS_NO_BACKGROUND_COL = 1 << 2,
	 CAS_NO_ITEM_COL       = 1 << 3,
	 CAS_NO_FIX_COL        = 1 << 4,
	 CAS_NO_DEAD_COL       = 1 << 5
};
DECLARE_FLAGS(CheckAnythingInSphereFlag, CASFlags)
DECLARE_FLAGS_OPERATORS(CASFlags)

bool CheckAnythingInSphere(const Sphere & sphere, EntityHandle source, CASFlags flags = 0, EntityHandle * num = NULL);

bool CheckEverythingInSphere(const Sphere & sphere, EntityHandle source, EntityHandle targ, std::vector<EntityHandle> & sphereContent);

const EERIEPOLY * CheckBackgroundInSphere(const Sphere & sphere);
 
bool IsCollidingIO(Entity * io, Entity * ioo);

/*!
 * \param ignoreNoCollisionFlag true if the IO_NO_COLLISIONS on the interactive object should be ignored
 */
bool CheckIOInSphere(const Sphere & sphere, const Entity & entity, bool ignoreNoCollisionFlag = false);

bool AttemptValidCylinderPos(Cylinder & cyl, Entity * io, CollisionFlags flags);
bool IO_Visible(const Vec3f & orgn, const Vec3f & dest, Vec3f * hit);

bool CylinderPlatformCollide(const Cylinder & cyl, Entity * io);
bool IsAnyNPCInPlatform(Entity * pfrm);
void PushIO_ON_Top(Entity * ioo, float ydec);

#endif // ARX_PHYSICS_COLLISIONS_H
