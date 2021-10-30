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

#ifndef ARX_GAME_MAGIC_SPELLS_SPELLSLVL09_H
#define ARX_GAME_MAGIC_SPELLS_SPELLSLVL09_H

#include "game/magic/Spell.h"

#include "graphics/effects/Fissure.h"
#include "platform/Platform.h"

class SummonCreatureSpell final : public Spell {
	
public:
	
	SummonCreatureSpell();
	
	bool CanLaunch() override;
	void Launch() override;
	void End() override;
	void Update() override;
	
private:
	
	void GetTargetAndBeta(Vec3f & target, float & beta);
	
	LightHandle m_light;
	CSummonCreature m_fissure;
	Vec3f m_targetPos;
	bool m_megaCheat;
	bool m_requestSummon;
	EntityHandle m_summonedEntity;
	
};

class FakeSummonSpell final : public Spell {
	
public:
	
	FakeSummonSpell()
		: m_targetPos(0.f)
	{ }
	
	bool CanLaunch() override;
	void Launch() override;
	void End() override;
	void Update() override;
	
private:
	
	LightHandle m_light;
	CSummonCreature m_fissure;
	Vec3f m_targetPos;
	
};

class NegateMagicSpell final : public Spell {
	
public:
	
	NegateMagicSpell();
	
	void Launch() override;
	void End() override;
	void Update() override;
	
private:
	
	Vec3f m_pos;
	TextureContainer * tex_p2;
	TextureContainer * tex_sol;
	
	void LaunchAntiMagicField();
	
};

class IncinerateSpell final : public Spell {
	
public:
	
	bool CanLaunch() override;
	void Launch() override;
	void End() override;
	void Update() override;
	
	Vec3f getPosition() const override;
	
};

class MassParalyseSpell final : public Spell {
	
public:
	
	void Launch() override;
	void End() override;
	
};

#endif // ARX_GAME_MAGIC_SPELLS_SPELLSLVL09_H
