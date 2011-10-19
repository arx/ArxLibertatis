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

#ifndef ARX_GRAPHICS_PARTICLE_PARTICLESYSTEM_H
#define ARX_GRAPHICS_PARTICLE_PARTICLESYSTEM_H

#include <list>

#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Renderer.h"
#include "math/MathFwd.h"
#include "math/Vector3.h"
#include "platform/Flags.h"
 
class Particle;
class ParticleParams;
class TextureContainer;

enum ParticleSpawnFlag {
	PARTICLE_CIRCULAR = (1<<0),
	PARTICLE_BORDER   = (1<<1)
};
DECLARE_FLAGS(ParticleSpawnFlag, ParticleSpawn)
DECLARE_FLAGS_OPERATORS(ParticleSpawn)

class ParticleSystem {
	
public:
	
	std::list<Particle *> listParticle;
	
	Vec3f p3Pos;
	
	unsigned int uMaxParticles;
	unsigned int uParticlesPerSec;
	
	int iParticleNbAlive;
	
	TextureContainer * tex_tab[20];
	int iNbTex;
	int iTexTime;
	bool bTexLoop;
	
	EERIEMATRIX eMat;
	
	unsigned long ulTime;
	unsigned long ulNbParticleGen;
	
	// these are used for the particles it creates
	Vec3f p3ParticlePos;
	int iParticleNbMax;
	float fParticleFreq;
	
	float fParticleFlash;
	float fParticleRotation;
	bool bParticleRotationRandomDirection;
	bool bParticleRotationRandomStart;
	
	ParticleSpawn ulParticleSpawn;
	
	Vec3f p3ParticleDirection;
	Vec3f p3ParticleGravity;
	float fParticleLife;
	float fParticleLifeRandom;
	float fParticleAngle;
	float fParticleSpeed;
	float fParticleSpeedRandom;
	
	bool bParticleStartColorRandomLock;
	float fParticleStartSize;
	float fParticleStartSizeRandom;
	float fParticleStartColor[4];
	float fParticleStartColorRandom[4];
	
	bool bParticleEndColorRandomLock;
	float fParticleEndSize;
	float fParticleEndSizeRandom;
	float fParticleEndColor[4];
	float fParticleEndColorRandom[4];
	
	Renderer::PixelBlendingFactor iSrcBlend;
	Renderer::PixelBlendingFactor iDstBlend;
	
	bool bParticleFollow;
	
	long lLightId;
	
	// editor
	float fMinx, fMaxx, fMiny, fMaxy;
	
	ParticleSystem();
	~ParticleSystem();
	
	void SpawnParticle(Particle * particle);
	void SetParticleParams(Particle * particle);
	
	void SetParams(const ParticleParams & app);
	
	void SetTexture(const char *, int, int, bool _bLoop = true);
	void SetPos(const Vec3f & ap3);
	void SetColor(float, float, float);
	
	void Render();
	bool IsAlive();
	void Update(long);
	void RecomputeDirection();
	
};

#endif // ARX_GRAPHICS_PARTICLE_PARTICLESYSTEM_H
