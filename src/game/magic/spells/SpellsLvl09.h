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

#ifndef ARX_GAME_MAGIC_SPELLS_SPELLSLVL09_H
#define ARX_GAME_MAGIC_SPELLS_SPELLSLVL09_H

#include "game/magic/Spell.h"

#include "graphics/effects/Fissure.h"
#include "platform/Platform.h"

class SummonCreatureSpell arx_final : public SpellBase {
	
public:
	
	SummonCreatureSpell();
	
	bool CanLaunch();
	void Launch();
	void End();
	void Update();
	
private:
	
	void GetTargetAndBeta(Vec3f & target, float & beta);
	
	LightHandle m_light;
	CSummonCreature m_fissure;
	Vec3f m_targetPos;
	bool m_megaCheat;
	bool m_requestSummon;
	EntityHandle m_summonedEntity;
	
};

class FakeSummonSpell arx_final : public SpellBase {
	
public:
	
	FakeSummonSpell()
		: m_targetPos(0.f)
	{ }
	
	bool CanLaunch();
	void Launch();
	void End();
	void Update();
	
private:
	
	LightHandle m_light;
	CSummonCreature m_fissure;
	Vec3f m_targetPos;
	
};

class NegateMagicSpell arx_final : public SpellBase {
	
public:
	
	NegateMagicSpell();
	
	void Launch();
	void End();
	void Update();
	
private:
	
	Vec3f m_pos;
	TextureContainer * tex_p2;
	TextureContainer * tex_sol;
	
	void LaunchAntiMagicField();
	
};

class IncinerateSpell arx_final : public SpellBase {
	
public:
	
	bool CanLaunch();
	void Launch();
	void End();
	void Update();
	
	Vec3f getPosition();
	
};

class MassParalyseSpell arx_final : public SpellBase {
	
public:
	
	void Launch();
	void End();
	
};

#endif // ARX_GAME_MAGIC_SPELLS_SPELLSLVL09_H
