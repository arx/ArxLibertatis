/*
 * Copyright 2014-2019 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GAME_MAGIC_SPELLS_SPELLSLVL10_H
#define ARX_GAME_MAGIC_SPELLS_SPELLSLVL10_H

#include "game/magic/Spell.h"

#include "graphics/effects/Lightning.h"
#include "platform/Platform.h"

class MassLightningStrikeSpell final : public SpellBase {
	
public:
	
	MassLightningStrikeSpell();
	~MassLightningStrikeSpell();
	
	void Launch() override;
	void End() override;
	void Update() override;
	
private:
	
	Vec3f m_pos;
	bool m_soundEffectPlayed;
	LightHandle m_light;
	
	std::vector<CLightning *> pTab;
	
};

class ControlTargetSpell final : public SpellBase {
	
public:
	
	ControlTargetSpell();
	
	bool CanLaunch() override;
	void Launch() override;
	void End() override;
	void Update() override;
	
private:
	
	Vec3f eSrc;
	Vec3f eTarget;
	TextureContainer * tex_mm;
	Vec3f pathways[40];
	float fTrail;
	
};

class FreezeTimeSpell final : public SpellBase {
	
public:
	
	FreezeTimeSpell();
	
	bool CanLaunch() override;
	void Launch() override;
	void End() override;
	
private:
	
	float m_slowdown;
	
};

class MassIncinerateSpell final : public SpellBase {
	
public:
	
	void Launch() override;
	void End() override;
	void Update() override;
	
};

#endif // ARX_GAME_MAGIC_SPELLS_SPELLSLVL10_H
