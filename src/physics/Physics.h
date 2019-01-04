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

#ifndef ARX_PHYSICS_PHYSICS_H
#define ARX_PHYSICS_PHYSICS_H

#include "game/GameTypes.h"
#include "graphics/GraphicsTypes.h"

struct EERIEPOLY;
struct EERIE_3DOBJ;

struct PhysicsParticle {
	
	Vec3f initpos;
	Vec3f pos;
	Vec3f velocity;
	Vec3f force;
	float mass;
	
	PhysicsParticle()
		: initpos(0.f)
		, pos(0.f)
		, velocity(0.f)
		, force(0.f)
		, mass(0.f)
	{ }
	
};

struct PHYSICS_BOX_DATA {
	
	boost::array<PhysicsParticle, 15> vert;
	short active;
	short stopcount;
	float radius; //!< Radius around vert[0].pos for spherical collision
	float storedtiming;
	float surface;
	
	PHYSICS_BOX_DATA()
		: active(0)
		, stopcount(0)
		, radius(0.f)
		, storedtiming(0.f)
		, surface(0.f)
	{ }
	
};

bool EERIE_PHYSICS_BOX_IsValidPosition(const Vec3f & pos);

void EERIE_PHYSICS_BOX_Create(EERIE_3DOBJ * obj);
void EERIE_PHYSICS_BOX_Release(EERIE_3DOBJ * obj);
void EERIE_PHYSICS_BOX_Launch(EERIE_3DOBJ * obj, const Vec3f & pos, const Anglef & angle, const Vec3f & vect);
void ARX_PHYSICS_BOX_ApplyModel(PHYSICS_BOX_DATA & pbox, float framediff, float rubber, Entity & source);

#endif // ARX_PHYSICS_PHYSICS_H
