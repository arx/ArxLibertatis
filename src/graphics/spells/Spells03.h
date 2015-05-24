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

#ifndef ARX_GRAPHICS_SPELLS_SPELLS03_H
#define ARX_GRAPHICS_SPELLS_SPELLS03_H

#include "graphics/effects/SpellEffects.h"
#include "graphics/effects/Trail.h"
#include "graphics/particle/ParticleSystem.h"
#include "graphics/particle/ParticleParams.h"

// Done By : Didier Pedreno
class CFireBall : public CSpellFx {
	
public:
	CFireBall();
	~CFireBall();
	
	void SetTTL(unsigned long);
	
	void Create(Vec3f, float afBeta, float afAlpha);
	void Kill();
	
	void Update(float timeDelta);
	void Render();
	
	Vec3f eSrc;
	Vec3f eCurPos;
	Vec3f eMove;
	bool bExplo;
	
	unsigned long m_createBallDuration;
private:

};

class CSpeed: public CSpellFx {
	
public:
	~CSpeed();
	
	void Create(EntityHandle numinteractive);
	void Update(float timeDelta);
	void Render();
	
private:
	EntityHandle num;
	
	struct SpeedTrail {
		short vertexIndex;
		Trail * trail;
	};
	
	std::vector<SpeedTrail> m_trails;
};

#define MAX_ICE 150
// Done By : did
class CIceProjectile : public CSpellFx {
	
public:
	CIceProjectile();
	~CIceProjectile();
	
	void Create(Vec3f, float, float, EntityHandle caster);
	void Update(float timeDelta);
	void Render();
	
private:
	int iNumber;
	int iMax;
	float fStep;
	float fColor;
	TextureContainer * tex_p1;
	TextureContainer * tex_p2;
	
	struct Icicle {
		int type;
		Vec3f pos;
		Vec3f size;
		Vec3f sizeMax;
	};
	Icicle m_icicles[MAX_ICE];
};

// Done By : did
class CCreateFood : public CSpellFx {
	
public:
	CCreateFood();
	~CCreateFood();
	
	void Create();
	void Update(float timeDelta);
	void Render();
	
private:
	Vec3f eSrc;
	ParticleSystem * pPS;
};

#endif // ARX_GRAPHICS_SPELLS_SPELLS03_H
