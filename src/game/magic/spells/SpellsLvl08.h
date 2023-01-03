/*
 * Copyright 2014-2022 Arx Libertatis Team (see the AUTHORS file)
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

class InvisibilitySpell final : public Spell {
	
public:
	
	bool CanLaunch() override;
	void Launch() override;
	void End() override;
	void Update() override;
	
	Vec3f getPosition() const override;
	
};

class ManaDrainSpell final : public Spell {
	
public:
	
	bool CanLaunch() override;
	void Launch() override;
	void End() override;
	void Update() override;
	
	Vec3f getPosition() const override;
	
private:
	
	DamageHandle m_damage;
	CabalFx m_cabal;
	
};

class ExplosionSpell final : public Spell {
	
public:
	
	void Launch() override;
	void Update() override;
	
private:
	
	LightHandle m_light;
	DamageHandle m_damage;
	
};

class EnchantWeaponSpell final : public Spell {
	
public:
	
	void Launch() override;
	void End() override;
	void Update() override;
	
};

class LifeDrainSpell final : public Spell {
	
public:
	
	bool CanLaunch() override;
	void Launch() override;
	void End() override;
	void Update() override;
	
	Vec3f getPosition() const override;
	
private:
	
	DamageHandle m_damage;
	CabalFx m_cabal;
	
};

#endif // ARX_GAME_MAGIC_SPELLS_SPELLSLVL08_H
