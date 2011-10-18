/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved

#ifndef  ARX_ANIMATION_ANIMATION_H
#define  ARX_ANIMATION_ANIMATION_H

#include <stddef.h>
#include <string>

#include "math/MathFwd.h"

class TextureContainer;
struct INTERACTIVE_OBJ;
struct EERIE_3DOBJ;
struct EERIE_LIGHT;
struct ANIM_USE;
struct EERIEMATRIX;
struct EERIE_MOD_INFO;
struct TexturedVertex;

const size_t HALOMAX = 2000;
extern long MAX_LLIGHTS;
const size_t MAX_ANIMATIONS = 900;

extern long HALOCUR;
extern TexturedVertex LATERDRAWHALO[HALOMAX * 4];
extern EERIE_LIGHT * llights[32];

long EERIE_ANIMMANAGER_Count(std::string & tex, long * memsize);
void EERIE_ANIMMANAGER_ClearAll();
void llightsInit();
void Preparellights(Vec3f * pos);
void Insertllight(EERIE_LIGHT * el, float dist);

void PopAllTriangleList();
void PopAllTriangleListTransparency();

TexturedVertex * PushVertexInTableCull(TextureContainer * tex);
TexturedVertex * PushVertexInTableCull_TNormalTrans(TextureContainer * tex);
TexturedVertex * PushVertexInTableCull_TAdditive(TextureContainer * tex);
TexturedVertex * PushVertexInTableCull_TSubstractive(TextureContainer * tex);
TexturedVertex * PushVertexInTableCull_TMultiplicative(TextureContainer * tex);
TexturedVertex * PushVertexInTableCull_TMetal(TextureContainer * tex);

void CalculateInterZMapp(EERIE_3DOBJ * _pobj3dObj, long lIdList, long * _piInd, TextureContainer * _pTex, TexturedVertex * _pVertex);
void EERIE_ANIMMANAGER_ReloadAll();

void EERIEDrawAnimQuat(EERIE_3DOBJ * eobj, ANIM_USE * eanim, Anglef * angle, Vec3f  * pos, unsigned long time, INTERACTIVE_OBJ * io, bool render = true);

void DrawEERIEInterMatrix(EERIE_3DOBJ * eobj, EERIEMATRIX * mat, Vec3f  * pos, INTERACTIVE_OBJ * io, EERIE_MOD_INFO * modinfo = NULL);

void DrawEERIEInter(EERIE_3DOBJ * eobj, Anglef * angle, Vec3f * pos, INTERACTIVE_OBJ * io, EERIE_MOD_INFO * modinfo = NULL);

#endif // ARX_ANIMATION_ANIMATION_H
