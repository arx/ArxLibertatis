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

#include "game/GameTypes.h"
#include "graphics/Color.h"
#include "graphics/Vertex.h"
#include "math/Types.h"
#include "math/Vector.h"
#include "math/Angle.h"
#include "util/Flags.h"

struct EERIEPOLY;
struct EERIE_CAMERA;
struct Sphere;
class TextureContainer;
class Entity;
struct EERIE_3DOBJ;

enum ParticlesTypeFlag {
	FIRE_TO_SMOKE       = (1<<0),
	ROTATING            = (1<<1),
	FADE_IN_AND_OUT     = (1<<2),
	MODULATE_ROTATION   = (1<<3),
	DISSIPATING         = (1<<4),
	GRAVITY             = (1<<5),
	SUBSTRACT           = (1<<6),
	FIRE_TO_SMOKE2      = (1<<7),  // TODO unused
	PARTICLE_SPARK2     = (1<<8),  // TODO unused
	FOLLOW_SOURCE       = (1<<9),  // TODO unused
	FOLLOW_SOURCE2      = (1<<10), // TODO unused
	DELAY_FOLLOW_SOURCE = (1<<11),
	NO_TRANS            = (1<<12),
	PARTICLE_ANIMATED   = (1<<13),
	PARTICLE_SPARK      = (1<<14),
	SPLAT_GROUND        = (1<<15),
	SPLAT_WATER         = (1<<16),
	PARTICLE_SUB2       = (1<<17),
	PARTICLE_GOLDRAIN   = (1<<18),
	PARTICLE_NOZBUFFER  = (1<<19)
};

DECLARE_FLAGS(ParticlesTypeFlag, ParticlesTypeFlags)
DECLARE_FLAGS_OPERATORS(ParticlesTypeFlags)

struct PARTICLE_DEF {
	bool exist;
	bool is2D;
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
	ParticlesTypeFlags special;
	float fparam;
	long mask;
	Vec3f * source;
	EntityHandle sourceionum;
	short sval;
	char cval1;
	char cval2;
	
	PARTICLE_DEF()
		: exist(false)
		, is2D(false)
		, ov(Vec3f_ZERO)
		, move(Vec3f_ZERO)
		, scale(Vec3f_ZERO)
		, oldpos(Vec3f_ZERO)
		, siz(0.f)
		, zdec(false)
		, timcreation(0)
		, tolive(0)
		, delay(0)
		, tc(NULL)
		, rgb(Color3f::black)
		, special(0)
		, fparam(0.f)
		, mask(0)
		, source(NULL)
		, sourceionum()
		, sval(0)
		, cval1(0)
		, cval2(0)
	{ }
};

//-----------------------------------------------------------------------------


static const float FLARE_MUL = 2.f;

extern TextureContainer * fire2;
extern long NewSpell;

void MagFX(const Vec3f & pos, float size);
void RestoreAllLightsInitialStatus();
void TreatBackgroundActions();

void Add3DBoom(const Vec3f & position);
void AddRandomSmoke(Entity * io, long amount = 1);

void ManageTorch();

void MakePlayerAppearsFX(Entity * io);
void MakeCoolFx(const Vec3f & pos);
void SpawnGroundSplat(const Sphere & sp, const Color3f & col, long flags);

PARTICLE_DEF * createParticle(bool allocateWhilePaused = false);
long getParticleCount();

void ARX_PARTICLES_FirstInit();
void ARX_PARTICLES_ClearAll();
void ARX_PARTICLES_Update(EERIE_CAMERA * cam);
void ARX_PARTICLES_Spawn_Blood(const Vec3f & pos, float dmgs, EntityHandle source);
void ARX_PARTICLES_Spawn_Blood2(const Vec3f & pos, float dmgs, Color col, Entity * io);
void ARX_PARTICLES_Spawn_Lava_Burn(Vec3f pos, Entity * io = NULL);
void ARX_PARTICLES_Add_Smoke(const Vec3f & pos, long flags, long amount, Color3f * rgb = NULL); // flag 1 = randomize pos

enum SpawnSparkType {
	SpawnSparkType_Default = 0,
	SpawnSparkType_Failed = 1,
	SpawnSparkType_Success = 2
};

void ARX_PARTICLES_Spawn_Spark(const Vec3f & pos, unsigned int count, SpawnSparkType type);

void ARX_PARTICLES_Spawn_Splat(const Vec3f & pos, float dmgs, Color col);
void ARX_PARTICLES_SpawnWaterSplash(const Vec3f & pos);

void createFireParticles(Vec3f & pos, int perPos, int delay);

void createObjFireParticles(const EERIE_3DOBJ * obj, int particlePositions, int perPos, int delay);

void LaunchFireballBoom(const Vec3f & poss, float level, Vec3f * direction = NULL, Color3f * rgb = NULL);
void spawnFireHitParticle(const Vec3f & poss, long type);
void spawn2DFireParticle(const Vec2f & pos, float scale);

void SpawnFireballTail(const Vec3f &, const Vec3f &, float, long);

#endif // ARX_GRAPHICS_PARTICLE_PARTICLEEFFECTS_H
