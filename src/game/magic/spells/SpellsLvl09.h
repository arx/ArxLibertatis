/*
 * Copyright 2014 Arx Libertatis Team (see the AUTHORS file)
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

class SummonCreatureSpell : public SpellBase {
public:
	SummonCreatureSpell();
	
	bool CanLaunch();
	void Launch();
	void End();
	void Update(float timeDelta);
	
private:
	void GetTargetAndBeta(Vec3f & target, float & beta);
	
	Vec3f m_targetPos;
	bool m_megaCheat;
	long m_longinfo_summon_creature;
	EntityHandle m_longinfo2_entity;
};

class FakeSummonSpell : public SpellBase {
public:
	bool CanLaunch();
	void Launch();
	void End();
	void Update(float timeDelta);
	
private:
	Vec3f m_targetPos;
};

class NegateMagicSpell : public SpellBase {
public:
	void Launch();
	void Update(float timeDelta);
	
private:
	void LaunchAntiMagicField();
};

class IncinerateSpell : public SpellBase {
public:
	bool CanLaunch();
	void Launch();
	void End();
	void Update(float timeDelta);
	
	Vec3f getPosition();
};

class MassParalyseSpell : public SpellBase {
public:
	void Launch();
	void End();
};

#endif // ARX_GAME_MAGIC_SPELLS_SPELLSLVL09_H
