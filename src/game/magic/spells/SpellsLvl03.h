/*
 * Copyright 2014-2020 Arx Libertatis Team (see the AUTHORS file)
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

#include <vector>

#include "game/magic/Spell.h"

#include "graphics/effects/Trail.h"
#include "graphics/particle/ParticleSystem.h"
#include "platform/Platform.h"


class SpeedSpell final : public Spell {
	
public:
	
	void Launch() override;
	void End() override;
	void Update() override;
	
	Vec3f getPosition() const override;
	
private:
	
	struct SpeedTrail : public Trail {
		short vertexIndex;
		explicit SpeedTrail(short vertex);
	};
	
	std::vector<SpeedTrail> m_trails;
	
};

class DispellIllusionSpell final : public Spell {
	
public:
	
	void Launch() override;
	void Update() override;
	
};

class FireballSpell final : public Spell {
	
public:
	
	FireballSpell();
	
	void Launch() override;
	void End() override;
	void Update() override;
	
	Vec3f getPosition() const override;
	
private:
	
	LightHandle m_light;
	
	Vec3f eCurPos;
	Vec3f eMove;
	bool bExplo;
	
	GameDuration m_createBallDuration;
	
};

class CreateFoodSpell final : public Spell {
	
public:
	
	CreateFoodSpell();
	
	void Launch() override;
	void End() override;
	void Update() override;
	
private:
	
	Vec3f m_pos;
	ParticleSystem m_particles;
	
};


class IceProjectileSpell final : public Spell {
	
public:
	
	IceProjectileSpell();
	
	void Launch() override;
	void End() override;
	void Update() override;
	
private:
	
	TextureContainer * tex_p1;
	TextureContainer * tex_p2;
	
	struct Icicle {
		int type;
		Vec3f pos;
		Vec3f size;
		Vec3f sizeMax;
	};
	
	std::vector<Icicle> m_icicles;
	
};

#endif // ARX_GAME_MAGIC_SPELLS_SPELLSLVL03_H
