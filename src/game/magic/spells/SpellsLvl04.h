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

#ifndef ARX_GAME_MAGIC_SPELLS_SPELLSLVL04_H
#define ARX_GAME_MAGIC_SPELLS_SPELLSLVL04_H

#include "game/magic/Spell.h"
#include "math/Quantizer.h"
#include "platform/Platform.h"


class BlessSpell final : public Spell {
	
public:
	
	BlessSpell();
	
	bool CanLaunch() override;
	void Launch() override;
	void End() override;
	void Update() override;
	
	Vec3f getPosition() const override;
	
private:
	
	TextureContainer * tex_p1;
	TextureContainer * tex_sol;
	
	Vec3f m_pos;
	float m_yaw;
	float m_scale;
	float fRot;
	math::Quantizer m_quantizer;
	
};

class DispellFieldSpell final : public Spell {
	
public:
	
	void Launch() override;
	
};

class FireProtectionSpell final : public Spell {
	
public:
	
	void Launch() override;
	void End() override;
	void Update() override;
	
	Vec3f getPosition() const override;
	
};

class ColdProtectionSpell final : public Spell {
	
public:
	
	void Launch() override;
	void End() override;
	void Update() override;
	
	Vec3f getPosition() const override;
	
};

class TelekinesisSpell final : public Spell {
	
public:
	
	bool CanLaunch() override;
	void Launch() override;
	void End() override;
	
};

class CurseSpell final : public Spell {
	
public:
	
	CurseSpell();
	
	void Launch() override;
	void End() override;
	void Update() override;
	
	Vec3f getPosition() const override;
	
private:
	
	TextureContainer * tex_p1;
	Vec3f m_pos;
	float fRot;
	math::Quantizer m_quantizer;
	
};

#endif // ARX_GAME_MAGIC_SPELLS_SPELLSLVL04_H
