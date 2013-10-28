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

#ifndef ARX_GRAPHICS_DRAW_H
#define ARX_GRAPHICS_DRAW_H

#include "graphics/Renderer.h"
#include "math/Types.h"

struct EERIEPOLY;
struct EERIE_3DOBJ;
struct EERIE_SPHERE;
struct EERIE_CYLINDER;
struct TexturedVertex;

extern Vec3f SPRmins;
extern Vec3f SPRmaxs;

void EERIEDRAWPRIM(Renderer::Primitive primitive, const TexturedVertex * vertices, size_t count = 3, bool nocount = false);

void EERIEDrawBitmap(Rect rect, float z, TextureContainer * tex, Color color);
void EERIEDrawBitmap(float x, float y, float sx, float sy, float z, TextureContainer * tex, Color color);


void EERIEDrawBitmap2DecalY(float x, float y, float sx, float sy, float z, TextureContainer * tex, Color col, float _fDeltaY);

void EERIEDrawSprite(TexturedVertex * in, float siz, TextureContainer * tex, Color col, float Zpos);
void EERIEDrawRotatedSprite(TexturedVertex * in, float siz, TextureContainer * tex, Color col, float Zpos, float rot);

void EERIEDrawBitmap2(float x, float y, float sx, float sy, float z, TextureContainer * tex, Color col);

void EERIEDrawBitmap_uv(float x, float y, float sx, float sy, float z, TextureContainer * tex, Color col, float u0, float v0, float u1, float v1);

void EERIEDrawBitmapUVs(float x, float y, float sx, float sy, float z, TextureContainer * tex, Color col, float u0, float v0, float u1, float v1, float u2, float v2, float u3, float v3);

#endif // ARX_GRAPHICS_DRAW_H
