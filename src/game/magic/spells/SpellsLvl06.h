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

#ifndef ARX_GAME_MAGIC_SPELLS_SPELLSLVL06_H
#define ARX_GAME_MAGIC_SPELLS_SPELLSLVL06_H

#include "game/magic/Spell.h"

#include "graphics/effects/Fissure.h"
#include "graphics/effects/Field.h"
#include "platform/Platform.h"

class RiseDeadSpell arx_final : public SpellBase {
	
public:
	
	RiseDeadSpell();
	
	bool CanLaunch();
	void Launch();
	void End();
	void Update();
	
private:
	
	void GetTargetAndBeta(Vec3f & target, float & beta);
	
	CRiseDead m_fissure;
	LightHandle m_light;
	Vec3f m_targetPos;
	bool m_creationFailed;
	EntityHandle m_entity;
	
};

class ParalyseSpell arx_final : public SpellBase {
	
public:
	
	void Launch();
	void End();
	
	Vec3f getPosition();
	
};

class CreateFieldSpell arx_final : public SpellBase {
	
public:
	
	CreateFieldSpell();
	
	void Launch();
	void End();
	void Update();
	
	Vec3f getPosition();
	
	// TODO this is directly used in physics and projectile
	EntityHandle m_entity;
	
private:
	
	CCreateField m_field;
	
};

class DisarmTrapSpell arx_final : public SpellBase {
	
public:
	
	void Launch();
	
};

class SlowDownSpell arx_final : public SpellBase {
	
public:
	
	bool CanLaunch();
	void Launch();
	void End();
	void Update();
	
	Vec3f getPosition();
	
};

#endif // ARX_GAME_MAGIC_SPELLS_SPELLSLVL06_H
