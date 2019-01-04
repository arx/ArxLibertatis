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

#ifndef ARX_GAME_MAGIC_SPELLS_SPELLSLVL04_H
#define ARX_GAME_MAGIC_SPELLS_SPELLSLVL04_H

#include "game/magic/Spell.h"
#include "platform/Platform.h"

class BlessSpell arx_final : public SpellBase {
	
public:
	
	BlessSpell();
	
	bool CanLaunch();
	void Launch();
	void End();
	void Update();
	
	Vec3f getPosition();
	
private:
	
	Vec3f m_pos;
	float m_yaw;
	float m_scale;
	
	TextureContainer * tex_p1;
	TextureContainer * tex_sol;
	float fRot;
	
};

class DispellFieldSpell arx_final : public SpellBase {
	
public:
	
	void Launch();
	
};

class FireProtectionSpell arx_final : public SpellBase {
	
public:
	
	void Launch();
	void End();
	void Update();
	
	Vec3f getPosition();
	
};

class ColdProtectionSpell arx_final : public SpellBase {
	
public:
	
	void Launch();
	void End();
	void Update();
	
	Vec3f getPosition();
	
};

class TelekinesisSpell arx_final : public SpellBase {
	
public:
	
	bool CanLaunch();
	void Launch();
	void End();
	
};

class CurseSpell arx_final : public SpellBase {
	
public:
	
	CurseSpell();
	
	void Launch();
	void End();
	void Update();
	
	Vec3f getPosition();
	
private:
	
	Vec3f m_pos;
	TextureContainer * tex_p1;
	float fRot;
	
};

#endif // ARX_GAME_MAGIC_SPELLS_SPELLSLVL04_H
