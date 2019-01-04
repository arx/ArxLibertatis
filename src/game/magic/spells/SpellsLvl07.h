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

#ifndef ARX_GAME_MAGIC_SPELLS_SPELLSLVL07_H
#define ARX_GAME_MAGIC_SPELLS_SPELLSLVL07_H

#include "game/magic/Spell.h"

#include "graphics/particle/ParticleSystem.h"
#include "graphics/effects/Lightning.h"
#include "platform/Platform.h"

class FlyingEyeSpell arx_final : public SpellBase {
	
public:
	
	FlyingEyeSpell();
	
	bool CanLaunch();
	void Launch();
	void End();
	void Update();
	
	Vec3f getPosition();
	
private:
	
	GameInstant m_lastupdate;
	
	LightHandle m_light1;
	LightHandle m_light2;
	
};

class FireFieldSpell arx_final : public SpellBase {
	
public:
	
	FireFieldSpell();
	
	void Launch();
	void End();
	void Update();
	
	Vec3f getPosition();
	
private:
	
	Vec3f m_pos;
	LightHandle m_light;
	DamageHandle m_damage;
	
	ParticleSystem pPSStream;
	ParticleSystem pPSStream1;
	
};

class IceFieldSpell arx_final : public SpellBase {
	
public:
	
	IceFieldSpell();
	
	void Launch();
	void End();
	void Update();
	
	Vec3f getPosition();
	
private:
	
	LightHandle m_light;
	DamageHandle m_damage;
	
	Vec3f m_pos;
	TextureContainer * tex_p1;
	TextureContainer * tex_p2;
	
	static const int iMax = 50;
	int tType[50];
	Vec3f tPos[50];
	Vec3f tSize[50];
	Vec3f tSizeMax[50];
	
};

class LightningStrikeSpell arx_final : public SpellBase {
	
public:
	
	void Launch();
	void End();
	void Update();
	
private:
	
	CLightning m_lightning;
	
};

class ConfuseSpell arx_final : public SpellBase {
	
public:
	
	ConfuseSpell();
	
	void Launch();
	void End();
	void Update();
	
	Vec3f getPosition();
	
private:
	
	LightHandle m_light;
	TextureContainer * tex_p1;
	TextureContainer * tex_trail;
	AnimLayer animlayer[MAX_ANIM_LAYERS];
	Vec3f eCurPos;
	
};

#endif // ARX_GAME_MAGIC_SPELLS_SPELLSLVL07_H
