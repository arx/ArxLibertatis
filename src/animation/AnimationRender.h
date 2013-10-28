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

#ifndef ARX_ANIMATION_ANIMATIONRENDER_H
#define ARX_ANIMATION_ANIMATIONRENDER_H

#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Color.h"
#include "graphics/Math.h"
#include "math/Types.h"

struct EERIE_3DOBJ;
struct ANIM_USE;
class Entity;
struct EERIEMATRIX;
struct EERIE_QUAT;
struct TexturedVertex;

float Cedric_GetInvisibility(Entity *io);

void Cedric_ApplyLightingFirstPartRefactor(Entity *io);

void PopAllTriangleList();
void PopAllTriangleListTransparency();

void ARX_DrawPrimitive(TexturedVertex *, TexturedVertex *, TexturedVertex *, float _fAdd = 0.0f);

struct TransformInfo {

	Vec3f pos;
	EERIE_QUAT rotation;
	float scale;
	Vec3f offset;

	TransformInfo()
		: pos(Vec3f_ZERO)
		, scale(1.f)
		, offset(Vec3f_ZERO)
	{
		Quat_Init(&rotation);
	}

	TransformInfo(Vec3f pos, EERIE_QUAT rotation, float scale = 1.f, Vec3f offset = Vec3f_ZERO)
		: pos(pos)
		, rotation(rotation)
		, scale(scale)
		, offset(offset)
	{}
};

void UpdateBbox2d(EERIE_3DOBJ *eobj, EERIE_2D_BBOX & box2D);

void DrawEERIEInter_ModelTransform(EERIE_3DOBJ *eobj, const TransformInfo &t);
void DrawEERIEInter_ViewProjectTransform(EERIE_3DOBJ *eobj);

void DrawEERIEInter_Render(EERIE_3DOBJ *eobj, const TransformInfo &t, Entity *io, float invisibility = 0.f);
void DrawEERIEInter(EERIE_3DOBJ *eobj, const TransformInfo & t, Entity *io, bool forceDraw = false, float invisibility = 0.f);

void EERIEDrawAnimQuatUpdate(EERIE_3DOBJ *eobj, ANIM_USE * animlayer,const Anglef & angle, const Vec3f & pos, unsigned long time, Entity *io, bool update_movement);
void EERIEDrawAnimQuatRender(EERIE_3DOBJ *eobj, const Vec3f & pos, Entity *io, bool render, float invisibility);

void EERIEDrawAnimQuat(EERIE_3DOBJ *eobj, ANIM_USE * animlayer, const Anglef & angle, const Vec3f & pos, unsigned long time, Entity *io, bool render = true, bool update_movement = true, float invisibility = 0.f);
void AnimatedEntityUpdate(Entity * entity, float time);
void AnimatedEntityRender(Entity * entity, float invisibility);

#endif // ARX_ANIMATION_ANIMATIONRENDER_H
