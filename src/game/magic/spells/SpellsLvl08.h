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

#ifndef ARX_GAME_MAGIC_SPELLS_SPELLSLVL08_H
#define ARX_GAME_MAGIC_SPELLS_SPELLSLVL08_H

#include "game/magic/Spell.h"
#include "graphics/effects/Cabal.h"
#include "platform/Platform.h"

class InvisibilitySpell arx_final : public SpellBase {
	
public:
	
	bool CanLaunch();
	void Launch();
	void End();
	void Update();
	
	Vec3f getPosition();
	
};

class ManaDrainSpell arx_final : public SpellBase {
	
public:
	
	ManaDrainSpell();
	
	bool CanLaunch();
	void Launch();
	void End();
	void Update();
	
	Vec3f getPosition();
	
private:
	
	DamageHandle m_damage;
	CabalFx m_cabal;
	
};

class ExplosionSpell arx_final : public SpellBase {
	
public:
	
	ExplosionSpell();
	
	void Launch();
	void Update();
	
private:
	
	LightHandle m_light;
	DamageHandle m_damage;
	
};

class EnchantWeaponSpell arx_final : public SpellBase {
	
public:
	
	void Launch();
	void End();
	void Update();
	
};

class LifeDrainSpell arx_final : public SpellBase {
	
public:
	
	LifeDrainSpell();
	
	bool CanLaunch();
	void Launch();
	void End();
	void Update();
	
	Vec3f getPosition();
	
private:
	
	DamageHandle m_damage;
	CabalFx m_cabal;
	
};

#endif // ARX_GAME_MAGIC_SPELLS_SPELLSLVL08_H
