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

#ifndef ARX_GRAPHICS_SPELLS_SPELLS05_H
#define ARX_GRAPHICS_SPELLS_SPELLS05_H

#include "graphics/effects/SpellEffects.h"
#include "graphics/particle/ParticleSystem.h"

// Done By : did
class CRuneOfGuarding : public CSpellFx {
	
public:
	CRuneOfGuarding();
	~CRuneOfGuarding();
	
	void SetPos(Vec3f);
	
	void Create(Vec3f);
	void Update(float timeDelta);
	void Render();
	
	Vec3f eSrc;
	
private:
	
	Vec3f eTarget;
	TextureContainer * tex_p2;
};

// Done By : Sébastien Scieux
class CLevitate : public CSpellFx {
	
public:
	CLevitate();
	~CLevitate();
	
	void ChangePos(Vec3f * pos)
	{
		this->m_pos = *pos;
	};
	
	void Create(int def, float rout, float rhaut, float hauteur, Vec3f * m_pos, unsigned long);
	void Update(float timeDelta);
	void Render();
	
private:
	short key;
	short def;
	Vec3f m_pos;
	float m_baseRadius;
	float rhaut;
	float hauteur;
	float m_coneScale;
	float ang;
	int currdurationang;
	int currframetime;
	TextureContainer * tsouffle;
	
	struct T_CONE {
		int conenbvertex;
		int conenbfaces;
		Vec3f * conevertex;
		TexturedVertex * coned3d;
		unsigned short * coneind;
	};
	
	T_CONE cone[2];
	
	struct T_STONE {
		short actif;
		short numstone;
		Vec3f pos;
		float yvel;
		Anglef ang;
		Anglef angvel;
		Vec3f scale;
		int time;
		int currtime;
	};
	
	int m_stoneDelay;
	int nbstone;
	T_STONE tstone[256];
	
	void AddStone(const Vec3f & pos);
	void DrawStone();
	
	void CreateConeStrip(float rout, float rhaut, float hauteur, int def, int numcone);
	void createDustParticle();
};

// Done By : Didier Pédreno
class CCurePoison : public CSpellFx {
	
public:
	CCurePoison();
	~CCurePoison();
	
	void Create();
	void SetPosition(const Vec3f & pos);
	
	void Update(float timeDelta);
	void Render();
	
private:
	Vec3f eSrc;
	ParticleSystem * pPS;
};

// Done By : Didier Pédreno
class CPoisonProjectile : public CSpellFx {
	
public:
	CPoisonProjectile();
	
	void Create(Vec3f, float afBeta = 0);
	void Update(float timeDelta);
	void Render();
	
	Vec3f eSrc;
	Vec3f eCurPos;
	float lightIntensityFactor;
	
private:
	float	fBetaRadCos;
	float	fBetaRadSin;
	void SetAngle(float afAngle) {
		float fBetaRad = glm::radians(afAngle);
		fBetaRadCos = glm::cos(fBetaRad);
		fBetaRadSin = glm::sin(fBetaRad);
	}
	
	bool  bOk;
	float fTrail;
	
	Vec3f eMove;
	Vec3f pathways[40];
	ParticleSystem pPS;
	ParticleSystem pPSStream;
};

class CMultiPoisonProjectile : public CSpellFx {
	
public:
	explicit CMultiPoisonProjectile(long nb);
	~CMultiPoisonProjectile();
	
	void Create(Vec3f srcPos, float afBeta);
	void Kill();
	void Update(float timeDelta);
	void Render();
	
	EntityHandle m_caster;
	float m_level;
	unsigned long m_timcreation;
	
private:
	std::vector<CPoisonProjectile *> m_projectiles;
	void AddPoisonFog(const Vec3f & pos, float power);
};

// Done By : did
class CRepelUndead : public CSpellFx {
	
public:
	CRepelUndead();
	~CRepelUndead();
	
	void Create(Vec3f);
	void SetPos(const Vec3f & pos);
	void SetRotation(float rotation);
	
	void Update(float timeDelta);
	void Render();
	
private:
	Vec3f eSrc;
	float m_yaw;
	TextureContainer * tex_p2;
};

#endif // ARX_GRAPHICS_SPELLS_SPELLS05_H
