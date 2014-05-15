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

#ifndef ARX_GRAPHICS_PARTICLE_PARTICLESYSTEM_H
#define ARX_GRAPHICS_PARTICLE_PARTICLESYSTEM_H

#include <list>

#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Renderer.h"
#include "graphics/Draw.h"
#include "graphics/particle/ParticleParams.h"
#include "math/Types.h"
#include "math/Vector.h"
#include "math/Quantizer.h"
#include "platform/Flags.h"
#include "scene/Light.h"
 
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
	
	// these are used for the particles it creates
	ParticleParams m_parameters;
	
	ParticleSpawn ulParticleSpawn;
	
	bool bParticleFollow;
	
	LightHandle lLightId;
	
	ParticleSystem();
	~ParticleSystem();
	
	void SetParams(const ParticleParams & app);
	
	void SetTexture(const char *, int, int);
	void SetPos(const Vec3f & ap3);
	
	void Render();
	bool IsAlive();
	void Update(long);
	void RecomputeDirection();
	
private:
	Vec3f p3Pos;
	int iParticleNbAlive;
	
	TextureContainer * tex_tab[20];
	int iNbTex;
	int iTexTime;
	bool bTexLoop;
	
	glm::mat4x4 eMat;
	
	unsigned long ulNbParticleGen;
	
	math::Quantizer m_storedTime;
	
	void SpawnParticle(Particle * particle);
	void SetParticleParams(Particle * particle);
	
	void SetColor(float, float, float);
};

#endif // ARX_GRAPHICS_PARTICLE_PARTICLESYSTEM_H
