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

#ifndef ARX_GAME_MAGIC_SPELLS_SPELLSLVL05_H
#define ARX_GAME_MAGIC_SPELLS_SPELLSLVL05_H

#include "game/magic/Spell.h"

#include "graphics/spells/Spells05.h"
#include "platform/Platform.h"

class RuneOfGuardingSpell arx_final : public SpellBase {
	
public:
	
	RuneOfGuardingSpell();
	
	void Launch();
	void End();
	void Update();
	
	Vec3f getPosition();
	
private:
	
	Vec3f m_pos;
	LightHandle m_light;
	
	TextureContainer * tex_p2;
	
};

class LevitateSpell arx_final : public SpellBase {
	
public:
	
	LevitateSpell();
	
	void Launch();
	void End();
	void Update();
	
private:
	
	Vec3f m_pos;
	float m_baseRadius;
	RotatingCone cone1;
	RotatingCone cone2;
	FloatingStones m_stones;
	
	void createDustParticle();
	
};

class CurePoisonSpell arx_final : public SpellBase {
	
public:
	
	CurePoisonSpell();
	
	void Launch();
	void End();
	void Update();
	
private:
	
	Vec3f m_pos;
	LightHandle m_light;
	ParticleSystem m_particles;
	
};

class RepelUndeadSpell arx_final : public SpellBase {
	
public:
	
	RepelUndeadSpell();
	
	void Launch();
	void End();
	void Update();
	
private:
	
	LightHandle m_light;
	Vec3f m_pos;
	float m_yaw;
	TextureContainer * tex_p2;
	
};

class PoisonProjectileSpell arx_final : public SpellBase {
	
public:
	
	~PoisonProjectileSpell();
	
	void Launch();
	void End();
	void Update();
	
private:
	
	LightHandle lLightId;
	std::vector<CPoisonProjectile *> m_projectiles;
	
	void AddPoisonFog(const Vec3f & pos, float power);
	
};

#endif // ARX_GAME_MAGIC_SPELLS_SPELLSLVL05_H
