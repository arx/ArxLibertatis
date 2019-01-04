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

#ifndef ARX_GRAPHICS_EFFECTS_FOG_H
#define ARX_GRAPHICS_EFFECTS_FOG_H

#include "math/Types.h"
#include "math/Angle.h"
#include "math/Vector.h"
#include "graphics/Color.h"

struct EERIE_3DOBJ;

struct FOG_DEF {
	
	bool exist;
	Vec3f pos;
	Color3f rgb;
	float size;
	long special;
	float scale;
	Vec3f move;
	Anglef angle;
	float speed;
	float rotatespeed;
	long tolive;
	long blend;
	float frequency;
	
	FOG_DEF()
		: exist(false)
		, pos(0.f)
		, rgb(Color3f::black)
		, size(0.f)
		, special(0)
		, scale(0.f)
		, move(0.f)
		, speed(0.f)
		, rotatespeed(0.f)
		, tolive(0)
		, blend(0)
		, frequency(0.f)
	{ }
	
};

#define FOG_DIRECTIONAL 1

static const size_t MAX_FOG = 100;
extern FOG_DEF fogs[MAX_FOG];

void ARX_FOGS_Render();
long ARX_FOGS_GetFree();
void ARX_FOGS_Clear();

#endif // ARX_GRAPHICS_EFFECTS_FOG_H
