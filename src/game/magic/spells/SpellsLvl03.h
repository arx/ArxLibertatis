/*
 * Copyright 2014-2018 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GAME_MAGIC_SPELLS_SPELLSLVL03_H
#define ARX_GAME_MAGIC_SPELLS_SPELLSLVL03_H

#include "game/magic/Spell.h"

#include "graphics/effects/Trail.h"
#include "graphics/particle/ParticleSystem.h"
#include "platform/Platform.h"


class SpeedSpell arx_final : public SpellBase {
	
public:
	
	~SpeedSpell();
	
	void Launch();
	void End();
	void Update();
	
	Vec3f getPosition();
	
private:
	
	struct SpeedTrail {
		short vertexIndex;
		Trail * trail;
	};
	
	std::vector<SpeedTrail> m_trails;
	
};

class DispellIllusionSpell arx_final : public SpellBase {
	
public:
	
	void Launch();
	void Update();
	
};

class FireballSpell arx_final : public SpellBase {
	
public:
	
	FireballSpell();
	~FireballSpell();
	
	void Launch();
	void End();
	void Update();
	
	Vec3f getPosition();
	
private:
	
	LightHandle m_light;
	
	Vec3f eCurPos;
	Vec3f eMove;
	bool bExplo;
	
	GameDuration m_createBallDuration;
	
};

class CreateFoodSpell arx_final : public SpellBase {
	
public:
	
	CreateFoodSpell();
	
	void Launch();
	void End();
	void Update();
	
private:
	
	Vec3f m_pos;
	ParticleSystem m_particles;
	
};


class IceProjectileSpell arx_final : public SpellBase {
	
public:
	
	IceProjectileSpell();
	
	void Launch();
	void End();
	void Update();
	
private:
	
	int iNumber;
	TextureContainer * tex_p1;
	TextureContainer * tex_p2;
	
	struct Icicle {
		int type;
		Vec3f pos;
		Vec3f size;
		Vec3f sizeMax;
	};
	
	static const int MAX_ICE = 150;
	Icicle m_icicles[MAX_ICE];
	
};

#endif // ARX_GAME_MAGIC_SPELLS_SPELLSLVL03_H
