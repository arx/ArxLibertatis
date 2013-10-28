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
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#ifndef ARX_GRAPHICS_PARTICLE_PARTICLEEFFECTS_H
#define ARX_GRAPHICS_PARTICLE_PARTICLEEFFECTS_H

#include <stddef.h>
#include <vector>

#include "graphics/Color.h"
#include "graphics/Vertex.h"
#include "math/Types.h"
#include "math/Vector.h"
#include "math/Angle.h"

struct EERIEPOLY;
struct EERIE_CAMERA;
struct EERIE_SPHERE;
class TextureContainer;
class Entity;



struct POLYBOOM {
	short tx;
	short tz;
	EERIEPOLY * ep;
	float u[4];
	float v[4];
	Color3f rgb;
	TextureContainer * tc;
	unsigned long timecreation;
	unsigned long tolive;
	short type;
	short nbvert;
};

struct PARTICLE_DEF {
	bool exist;
	long type;
	Vec3f ov;
	Vec3f move;
	Vec3f scale;
	Vec3f oldpos;
	float siz;
	bool zdec;
	long timcreation;
	unsigned long tolive;
	unsigned long delay;
	TextureContainer * tc;
	Color3f rgb;
	long special; // TODO ARX_PARTICLES_TYPE_FLAG
	float fparam;
	long mask;
	Vec3f * source;
	long sourceionum;
	short sval;
	char cval1;
	char cval2;
};

//-----------------------------------------------------------------------------
struct FOG_DEF
{
	bool		exist;
	long		selected;
	Vec3f	pos;
	Color3f	rgb;
	float		size;
	long		special;
	float		scale;
	Vec3f	move;
	Anglef	angle;
	float		speed;
	float		rotatespeed;
	long		tolive;
	Vec3f	bboxmin;
	Vec3f	bboxmax;
	long		blend;
	float		frequency;
	unsigned long lastupdate;
};

#define MAX_FOG 100
#define FOG_DIRECTIONAL 1
#define MAX_POLYBOOM 4000
#define PARTICLE_2D	256

#define MAX_FLARELIFE 4000
#define FLARE_MUL 2.f

enum ARX_PARTICLES_TYPE_FLAG {
	FIRE_TO_SMOKE       = 0x00000001,
	ROTATING            = 0x00000002,
	FADE_IN_AND_OUT     = 0x00000004,
	MODULATE_ROTATION   = 0x00000008,
	DISSIPATING         = 0x00000010,
	GRAVITY             = 0x00000020,
	SUBSTRACT           = 0x00000040,
	FIRE_TO_SMOKE2      = 0x00000080,
	PARTICLE_SPARK2     = 0x00000100,
	FOLLOW_SOURCE       = 0x00000200,
	FOLLOW_SOURCE2      = 0x00000400,
	DELAY_FOLLOW_SOURCE = 0x00000800,
	NO_TRANS            = 0x00001000,
	PARTICLE_ANIMATED   = 0x00002000,
	PARTICLE_SPARK      = 0x00004000,
	SPLAT_GROUND        = 0x00008000,
	SPLAT_WATER         = 0x00010000,
	PARTICLE_SUB2       = 0x00020000,
	PARTICLE_GOLDRAIN   = 0x00040000,
	PARTICLE_NOZBUFFER  = 0x80000000
};

//-----------------------------------------------------------------------------
#define BOOM_RADIUS 420.f
#define BOOM_RADIUS2 250.f
#define MAX_EXPLO			24 

//-----------------------------------------------------------------------------
extern TextureContainer * explo[MAX_EXPLO];
extern TextureContainer * blood_splat;

extern short OPIPOrgb;
extern short PIPOrgb;
extern std::vector<POLYBOOM> polyboom;
extern FOG_DEF fogs[MAX_FOG];
extern TextureContainer * fire2;
extern long NewSpell;

void MagFX(const Vec3f & pos);
void RestoreAllLightsInitialStatus();
void TreatBackgroundActions();
void TreatBackgroundDynlights();
void MakeBookFX(const Vec3f & pos);

void Add3DBoom(Vec3f * position);
void AddRandomSmoke(Entity * io, long amount = 1);

void LaunchDummyParticle();
void ManageTorch();

void createSphericalSparks(const Vec3f & pos, float r, TextureContainer * tc,
                           const Color3f & color, int mask);
void MakePlayerAppearsFX(Entity * io);
void MakeCoolFx(Vec3f * pos);
void SpawnGroundSplat(EERIE_SPHERE * sp, Color3f * rgb, float size, long flags);

PARTICLE_DEF * createParticle(bool allocateWhilePaused = false);
long getParticleCount();

void ARX_PARTICLES_FirstInit();
void ARX_PARTICLES_ClearAll();
void ARX_PARTICLES_Render(EERIE_CAMERA * cam);
void ARX_PARTICLES_Spawn_Blood(Vec3f * pos, float dmgs, long source);
void ARX_PARTICLES_Spawn_Blood2(const Vec3f & pos, float dmgs, Color col, Entity * io);
void ARX_PARTICLES_Spawn_Lava_Burn(Vec3f * pos, Entity * io = NULL);
void ARX_PARTICLES_Add_Smoke(Vec3f * pos, long flags, long amount, Color3f * rgb = NULL); // flag 1 = randomize pos
void ARX_PARTICLES_Spawn_Spark(Vec3f * pos, float dmgs, long flags);
void ARX_PARTICLES_Spawn_Splat(const Vec3f & pos, float dmgs, Color col);
void ARX_PARTICLES_SpawnWaterSplash(const Vec3f * pos);

void ARX_BOOMS_ClearAllPolyBooms();
void ARX_BOOMS_Add(Vec3f * pos, long type = 0);



void LaunchFireballBoom(Vec3f * poss, float level, Vec3f * direction = NULL, Color3f * rgb = NULL);
void SpawnFireballTail(Vec3f *, Vec3f *, float, long);

#endif // ARX_GRAPHICS_PARTICLE_PARTICLEEFFECTS_H
