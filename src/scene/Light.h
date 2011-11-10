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
//       Didier Pedreno (update Light Model version 1)
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#ifndef ARX_SCENE_LIGHT_H
#define ARX_SCENE_LIGHT_H

#include <stddef.h>

#include "math/MathFwd.h"

struct EERIE_LIGHT;
struct EERIEPOLY;

const size_t MAX_LIGHTS = 1200;
const size_t MAX_DYNLIGHTS = 500;

extern EERIE_LIGHT * PDL[MAX_DYNLIGHTS];
extern EERIE_LIGHT * GLight[MAX_LIGHTS];
extern EERIE_LIGHT DynLight[MAX_DYNLIGHTS];
extern EERIE_LIGHT * IO_PDL[MAX_DYNLIGHTS];
extern long TOTPDL;
extern long TOTIOPDL;

void PrecalcIOLighting(const Vec3f * pos, float radius, long flags = 0);

void EERIE_LIGHT_Apply(EERIEPOLY * ep);
void EERIE_LIGHT_TranslateSelected(const Vec3f * trans);
void EERIE_LIGHT_UnselectAll();
void EERIE_LIGHT_ClearAll();
void EERIE_LIGHT_ClearSelected();
void EERIE_LIGHT_ClearByIndex(long num);
void EERIE_LIGHT_GlobalInit();
long EERIE_LIGHT_GetFree();
long EERIE_LIGHT_Count();
void EERIE_LIGHT_GlobalAdd(const EERIE_LIGHT * el);
void EERIE_LIGHT_MoveAll(const Vec3f * trans);
long EERIE_LIGHT_Create();

void _RecalcLightZone(float x, float z, long siz);
 
bool ValidDynLight(long num);

#endif // ARX_SCENE_LIGHT_H
