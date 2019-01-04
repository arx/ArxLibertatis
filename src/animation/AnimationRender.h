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

#ifndef ARX_ANIMATION_ANIMATIONRENDER_H
#define ARX_ANIMATION_ANIMATIONRENDER_H

#include "core/TimeTypes.h"
#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Color.h"
#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "math/Types.h"

struct EERIE_3DOBJ;
struct AnimLayer;
class Entity;
struct RenderMaterial;
struct TexturedQuad;

float Cedric_GetInvisibility(Entity * io);

void Cedric_ApplyLightingFirstPartRefactor(Entity & io);

void PopAllTriangleListOpaque(RenderState baseState = render3D(), bool clear = true);
void PopAllTriangleListTransparency();

void drawQuadRTP(const RenderMaterial & mat, TexturedQuad quat);
void drawTriangle(const RenderMaterial & mat, const TexturedVertexUntransformed * vertices);

struct TransformInfo {
	
	Vec3f pos;
	glm::quat rotation;
	float scale;
	Vec3f offset;
	
	TransformInfo()
		: pos(0.f)
		, rotation(quat_identity())
		, scale(1.f)
		, offset(0.f)
	{ }
	
	TransformInfo(Vec3f pos_, glm::quat rotation_, float scale_ = 1.f, Vec3f offset_ = Vec3f(0.f))
		: pos(pos_)
		, rotation(rotation_)
		, scale(scale_)
		, offset(offset_)
	{ }
	
};

EERIE_2D_BBOX UpdateBbox2d(const EERIE_3DOBJ & eobj);

void DrawEERIEInter_ModelTransform(EERIE_3DOBJ * eobj, const TransformInfo & t);
void DrawEERIEInter_ViewProjectTransform(EERIE_3DOBJ * eobj);

void DrawEERIEInter_Render(EERIE_3DOBJ * eobj, const TransformInfo & t, Entity * io, float invisibility = 0.f);

void DrawEERIEInter(EERIE_3DOBJ * eobj,
                    const TransformInfo & t,
                    Entity * io,
                    bool forceDraw,
                    float invisibility);

void EERIEDrawAnimQuatUpdate(EERIE_3DOBJ * eobj,
                             AnimLayer * animlayer,
                             const Anglef & angle,
                             const Vec3f & pos,
                             AnimationDuration time,
                             Entity * io,
                             bool update_movement);

void EERIEDrawAnimQuatRender(EERIE_3DOBJ * eobj, const Vec3f & pos, Entity * io, float invisibility);

void AnimatedEntityRender(Entity * entity, float invisibility);

#endif // ARX_ANIMATION_ANIMATIONRENDER_H
