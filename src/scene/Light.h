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
// Code: Cyril Meynier
//       Didier Pedreno (update Light Model version 1)
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#ifndef ARX_SCENE_LIGHT_H
#define ARX_SCENE_LIGHT_H

#include <stddef.h>

#include "math/Types.h"
#include "graphics/Color.h"
#include "platform/Flags.h"
#include "audio/AudioTypes.h"

struct EERIE_LIGHT;
struct EERIEPOLY;
struct EERIE_QUAT;
struct SMY_VERTEX;
class Entity;

const size_t MAX_LIGHTS = 1200;
const size_t MAX_DYNLIGHTS = 500;

extern long MAX_LLIGHTS;

extern EERIE_LIGHT * PDL[MAX_DYNLIGHTS];
extern EERIE_LIGHT * GLight[MAX_LIGHTS];
extern EERIE_LIGHT DynLight[MAX_DYNLIGHTS];
extern EERIE_LIGHT * IO_PDL[MAX_DYNLIGHTS];
extern long TOTPDL;
extern long TOTIOPDL;


enum EERIE_TYPES_EXTRAS_MODE
{
	EXTRAS_SEMIDYNAMIC       = 0x00000001,
	EXTRAS_EXTINGUISHABLE    = 0x00000002,
	EXTRAS_STARTEXTINGUISHED = 0x00000004,
	EXTRAS_SPAWNFIRE         = 0x00000008,
	EXTRAS_SPAWNSMOKE        = 0x00000010,
	EXTRAS_OFF               = 0x00000020,
	EXTRAS_COLORLEGACY       = 0x00000040,
	EXTRAS_NOCASTED          = 0x00000080,
	EXTRAS_FIXFLARESIZE      = 0x00000100,
	EXTRAS_FIREPLACE         = 0x00000200,
	EXTRAS_NO_IGNIT          = 0x00000400,
	EXTRAS_FLARE	         = 0x00000800
};
DECLARE_FLAGS(EERIE_TYPES_EXTRAS_MODE, ExtrasType)
DECLARE_FLAGS_OPERATORS(ExtrasType)

struct EERIE_LIGHT {
	char exist;
	char type;
	char treat;
	char selected;
	ExtrasType extras;
	short status; // on/off 1/0
	Vec3f pos;
	float fallstart;
	float fallend;
	float falldiff;
	float falldiffmul;
	float precalc;
	Color3f rgb255;
	float intensity;
	Color3f rgb;
	float i;
	Vec3f mins;
	Vec3f maxs;
	float temp;
	long ltemp;
	Color3f ex_flicker;
	float ex_radius;
	float ex_frequency;
	float ex_size;
	float ex_speed;
	float ex_flaresize;
	long tl;
	unsigned long time_creation;
	long duration; // will start to fade before the end of duration...
	audio::SourceId sample;
};

struct ColorMod {

	void updateFromEntity(Entity * io, bool inBook = false);

	Color3f ambientColor;
	Color3f factor;
	Color3f term;
};

void RecalcLight(EERIE_LIGHT * el);

void EERIE_LIGHT_GlobalInit();
long EERIE_LIGHT_GetFree();
long EERIE_LIGHT_Count();
void EERIE_LIGHT_GlobalAdd(const EERIE_LIGHT * el);
void EERIE_LIGHT_MoveAll(const Vec3f * trans);
long EERIE_LIGHT_Create();
void PrecalcIOLighting(const Vec3f * pos, float radius);

bool ValidDynLight(long num);
long GetFreeDynLight();
void ClearDynLights();
void PrecalcDynamicLighting(long x0,long x1,long z0,long z1);

void UpdateLlights(Vec3f & tv);

void InitTileLights();
void ResetTileLights();
void ComputeTileLights(short x,short z);
void ClearTileLights();

float GetColorz(const Vec3f &pos);
ColorBGRA ApplyLight(const EERIE_QUAT * quat, const Vec3f & position, const Vec3f & normal, const ColorMod & colorMod, float materialDiffuse = 1.f);
void ApplyTileLights(EERIEPOLY * ep, short x, short y);


void RecalcLightZone(float x, float z, long siz);
void EERIERemovePrecalcLights();

#endif // ARX_SCENE_LIGHT_H
