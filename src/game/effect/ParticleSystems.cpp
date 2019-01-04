/*
 * Copyright 2016-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "game/effect/ParticleSystems.h"

ParticleParams g_particleParameters[ParticleParam_MAX];


class MagicMissileExplosionParticle : public ParticleParams {
public:
	MagicMissileExplosionParticle() {
		load();
	}
	
	void load() {
		m_nbMax = 100;
		m_life = 1500;
		m_lifeRandom = 0;
		m_pos = Vec3f(10.f);
		m_direction = Vec3f(0.f, -1.f, 0.f);
		m_angle = glm::radians(360.f);
		m_speed = 130;
		m_speedRandom = 100;
		m_gravity = Vec3f(0.f, 10.f, 0.f);
		m_flash = 0;
		m_rotation = 1.0f / (101 - 16);
	
		m_startSegment.m_size = 5;
		m_startSegment.m_sizeRandom = 10;
		m_startSegment.m_color = Color4f::gray(0.43f, 0.42f);
		m_startSegment.m_colorRandom = Color4f::gray(0.39f, 0.39f);
		m_endSegment.m_size = 0;
		m_endSegment.m_sizeRandom = 2;
		m_endSegment.m_color = Color4f(Color3f::blue * 0.47f, 0.04f);
		m_endSegment.m_colorRandom = Color4f::gray(0.2f, 0.2f);
		
		m_texture.set("graph/particles/magicexplosion", 0, 500);
		
		m_blendMode = RenderMaterial::Additive;
		m_spawnFlags = 0;
		m_looping = false;
	}
};

class MagicMissileExplosionMrCheatParticle : public MagicMissileExplosionParticle {
public:
	MagicMissileExplosionMrCheatParticle() {
		load();
	}
	
	void load() {
		MagicMissileExplosionParticle::load();
		
		m_speed = 13;
		m_speedRandom = 10;
		m_startSegment.m_size = 20;
		m_startSegment.m_color = Color4f::none;
		m_startSegment.m_colorRandom = Color4f::none;
		m_endSegment.m_color = Color4f::rgba(1.f, 0.17f, 0.47f, 0.04f);
		m_texture.set("graph/particles/(fx)_mr", 0, 500);
	}
};

void particleParametersInit() {
	
	g_particleParameters[ParticleParam_MagicMissileExplosion]    = MagicMissileExplosionParticle();
	g_particleParameters[ParticleParam_MagicMissileExplosionMar] = MagicMissileExplosionMrCheatParticle();
	
	{
	ParticleParams cp;
	cp.m_nbMax = 350;
	cp.m_life = 800;
	cp.m_lifeRandom = 2000;
	cp.m_pos = Vec3f(100, 200, 100);
	cp.m_direction = Vec3f(0.f, -1.f, 0.f);
	cp.m_angle = glm::radians(5.f);
	cp.m_speed = 120;
	cp.m_speedRandom = 84;
	cp.m_gravity = Vec3f(0, -10, 0);
	cp.m_flash = 0;
	cp.m_rotation = 1.0f / (101 - 80);

	cp.m_startSegment.m_size = 8;
	cp.m_startSegment.m_sizeRandom = 8;
	cp.m_startSegment.m_color = Color4f::rgba(0.8f, 0.8f, 1.f, 0.96f);
	cp.m_startSegment.m_colorRandom = Color4f(Color3f::yellow * 0.2f, 0.04f);

	cp.m_endSegment.m_size = 6;
	cp.m_endSegment.m_sizeRandom = 4;
	cp.m_endSegment.m_color = Color4f::rgba(0.08f, 0.08f, 0.12f, 0.f);
	cp.m_endSegment.m_colorRandom = Color4f(Color3f::blue * 0.16f, 0.f);
	
	cp.m_blendMode = RenderMaterial::Additive;
	cp.m_texture.set("graph/particles/heal_0005", 0, 100);
	cp.m_spawnFlags = PARTICLE_CIRCULAR | PARTICLE_BORDER;
	g_particleParameters[ParticleParam_Heal] = cp;
	}
	
	{
	ParticleParams cp;
	cp.m_nbMax = 350;
	cp.m_life = 800;
	cp.m_lifeRandom = 2000;
	cp.m_pos = Vec3f(100, 200, 100);
	cp.m_direction = Vec3f(0.f, -1.f, 0.f);
	cp.m_angle = glm::radians(5.f);
	cp.m_speed = 120;
	cp.m_speedRandom = 84;
	cp.m_gravity = Vec3f(0, -10, 0);
	cp.m_flash = 0;
	cp.m_rotation = 1.0f / (101 - 80);

	cp.m_startSegment.m_size = 8;
	cp.m_startSegment.m_sizeRandom = 8;
	cp.m_startSegment.m_color = Color4f::rgba(0.41f, 0.41f, 0.08f, 0.57f);
	cp.m_startSegment.m_colorRandom = Color4f(Color3f::yellow * 0.2f, 0.04f);

	cp.m_endSegment.m_size = 6;
	cp.m_endSegment.m_sizeRandom = 4;
	cp.m_endSegment.m_color = Color4f::rgba(0.08f, 0.08f, 0.02f, 0.f);
	cp.m_endSegment.m_colorRandom = Color4f::rgba(0.16f, 0.16f, 0.f, 0.f);

	cp.m_blendMode = RenderMaterial::Additive;
	cp.m_texture.set("graph/particles/create_food", 0, 100);
	cp.m_spawnFlags = PARTICLE_CIRCULAR | PARTICLE_BORDER;
	
	g_particleParameters[ParticleParam_CreateFood] = cp;
	}
	
	{
	ParticleParams cp;
	cp.m_nbMax = 350;
	cp.m_life = 800;
	cp.m_lifeRandom = 2000;
	cp.m_pos = Vec3f(100, 0, 100);
	cp.m_direction = Vec3f(0.f, -1.f, 0.f);
	cp.m_angle = glm::radians(5.f);
	cp.m_speed = 120;
	cp.m_speedRandom = 84;
	cp.m_gravity = Vec3f(0, -10, 0);
	cp.m_flash = 0;
	cp.m_rotation = 1.0f / (101 - 80);

	cp.m_startSegment.m_size = 8;
	cp.m_startSegment.m_sizeRandom = 8;
	cp.m_startSegment.m_color = Color4f::rgba(0.08f, 0.8f, 0.08f, 0.96f);
	cp.m_startSegment.m_colorRandom = Color4f::gray(0.2f, 0.04f);

	cp.m_endSegment.m_size = 6;
	cp.m_endSegment.m_sizeRandom = 4;
	cp.m_endSegment.m_color = Color4f::rgba(0.02f, 0.08f, 0.02f, 0.f);
	cp.m_endSegment.m_colorRandom = Color4f(Color3f::green * 0.16f, 0.f);
	cp.m_blendMode = RenderMaterial::Additive;
	cp.m_texture.set("graph/particles/cure_poison", 0, 100);
	cp.m_spawnFlags = PARTICLE_CIRCULAR | PARTICLE_BORDER;
	g_particleParameters[ParticleParam_CurePoison] = cp;
	}
	
	{
	ParticleParams cp = ParticleParams();
	cp.m_nbMax = 100;
	cp.m_life = 2000;
	cp.m_lifeRandom = 1000;
	cp.m_pos = Vec3f(80, 10, 80);
	cp.m_direction = Vec3f(0.f, 1.f, 0.f);
	cp.m_angle = 0;
	cp.m_speed = 0;
	cp.m_speedRandom = 0;
	cp.m_gravity = Vec3f(0.f);
	cp.m_flash = 0;
	cp.m_rotation = 0;
	cp.m_rotationRandomDirection = false;
	cp.m_rotationRandomStart = false;

	cp.m_startSegment.m_size = 10;
	cp.m_startSegment.m_sizeRandom = 3;
	cp.m_startSegment.m_color = Color4f::gray(0.1f, 0.2f);
	cp.m_startSegment.m_colorRandom = Color4f::gray(0.2f, 0.4f);

	cp.m_endSegment.m_size = 10;
	cp.m_endSegment.m_sizeRandom = 3;
	cp.m_endSegment.m_color = Color4f::gray(0.1f, 0.2f);
	cp.m_endSegment.m_colorRandom = Color4f(Color3f::black, 0.4f);
	cp.m_texture.m_texLoop = true;

	cp.m_blendMode = RenderMaterial::AlphaAdditive;
	cp.m_freq = 150;
	cp.m_texture.set("graph/particles/firebase", 4, 100);
	cp.m_spawnFlags = 0;
	
	g_particleParameters[ParticleParam_FireFieldBase] = cp;
	}
	
	{
	ParticleParams cp = ParticleParams();
	cp.m_nbMax = 50;
	cp.m_life = 1000;
	cp.m_lifeRandom = 500;
	cp.m_pos = Vec3f(100, 10, 100);
	cp.m_direction = Vec3f(0.f, -1.f, 0.f);
	cp.m_angle = glm::radians(10.f);
	cp.m_speed = 0;
	cp.m_speedRandom = 0;
	cp.m_gravity = Vec3f(0.f);
	cp.m_flash = 0;
	cp.m_rotation = 0;
	cp.m_rotationRandomDirection = false;
	cp.m_rotationRandomStart = false;

	cp.m_startSegment.m_size = 10;
	cp.m_startSegment.m_sizeRandom = 10;
	cp.m_startSegment.m_color = Color4f::gray(0.16f, 0.2f);
	cp.m_startSegment.m_colorRandom = Color4f::gray(0.2f, 0.4f);

	cp.m_endSegment.m_size = 10;
	cp.m_endSegment.m_sizeRandom = 10;
	cp.m_endSegment.m_color = Color4f(Color3f::black, 0.2f);
	cp.m_endSegment.m_colorRandom = Color4f(Color3f::black, 0.4f);
	cp.m_texture.m_texLoop = false;

	cp.m_blendMode = RenderMaterial::Additive;
	cp.m_freq = 150;
	cp.m_texture.set("graph/particles/fire", 0, 500);
	cp.m_spawnFlags = 0;
	
	g_particleParameters[ParticleParam_FireFieldFlame] = cp;
	}
	
	{
	ParticleParams cp = ParticleParams();
	cp.m_nbMax = 80;
	cp.m_life = 1500;
	cp.m_lifeRandom = 500;
	cp.m_pos = Vec3f(5);
	cp.m_direction = Vec3f(0.f, 1.f, 0.f);
	cp.m_angle = glm::radians(360.f);
	cp.m_speed = 200;
	cp.m_speedRandom = 0;
	cp.m_gravity = Vec3f(0, 17, 0);
	cp.m_flash = 0;
	cp.m_rotation = 1.0f / (101 - 80);
	cp.m_rotationRandomDirection = true;
	cp.m_rotationRandomStart = true;

	cp.m_startSegment.m_size = 5;
	cp.m_startSegment.m_sizeRandom = 3;
	cp.m_startSegment.m_color = Color4f(Color3f::green * 0.3f, 0.f);
	cp.m_startSegment.m_colorRandom = Color4f(Color3f::black, 0.59f);

	cp.m_endSegment.m_size = 30;
	cp.m_endSegment.m_sizeRandom = 5;
	cp.m_endSegment.m_color = Color4f::none;
	cp.m_endSegment.m_colorRandom = Color4f(Color3f::green * 0.1f, 0.08f);

	cp.m_blendMode = RenderMaterial::AlphaAdditive;
	cp.m_freq = -1;
	cp.m_texture.set("graph/particles/big_greypouf", 0, 200);
	cp.m_spawnFlags = 0;
	cp.m_looping = false;
	g_particleParameters[ParticleParam_Poison1] = cp;
	}
	
	{
	ParticleParams cp = ParticleParams();
	cp.m_nbMax = 5;
	cp.m_life = 2000;
	cp.m_lifeRandom = 1000;
	cp.m_pos = Vec3f(0.f);
	cp.m_direction = Vec3f(0.1f);
	cp.m_angle = 0;
	cp.m_speed = 10;
	cp.m_speedRandom = 10;
	cp.m_gravity = Vec3f(0.f);
	cp.m_flash = 21 * 0.01f;
	cp.m_rotation = 1.f / (101 - 80);
	cp.m_rotationRandomDirection = true;
	cp.m_rotationRandomStart = true;

	cp.m_startSegment.m_size = 5;
	cp.m_startSegment.m_sizeRandom = 3;
	cp.m_startSegment.m_color = Color4f(Color3f::green * 0.2f, 0.16f);
	cp.m_startSegment.m_colorRandom = Color4f(Color3f::green * 0.4f, 0.2f);

	cp.m_endSegment.m_size = 8;
	cp.m_endSegment.m_sizeRandom = 13;
	cp.m_endSegment.m_color = Color4f(Color3f::green * 0.24f, 0.16f);
	cp.m_endSegment.m_colorRandom = Color4f(Color3f::green * 0.4f, 0.2f);

	cp.m_blendMode = RenderMaterial::Screen;
	cp.m_freq = -1;
	cp.m_texture.set("graph/particles/big_greypouf", 0, 200);
	cp.m_spawnFlags = 0;
	g_particleParameters[ParticleParam_Poison2] = cp;
	}
	
	{
	ParticleParams cp = ParticleParams();
	cp.m_nbMax = 100;
	cp.m_life = 500;
	cp.m_lifeRandom = 300;
	cp.m_pos = Vec3f(20, 0.f, 20);

	cp.m_direction = Vec3f(0.1f);

	cp.m_angle = glm::radians(4.f);
	cp.m_speed = 150;
	cp.m_speedRandom = 50;
	cp.m_gravity = Vec3f(0, 10, 0);
	cp.m_flash = 0;
	cp.m_rotation = 1.0f / (101 - 80);
	cp.m_rotationRandomDirection = true;
	cp.m_rotationRandomStart = true;

	cp.m_startSegment.m_size = 2;
	cp.m_startSegment.m_sizeRandom = 2;
	cp.m_startSegment.m_color = Color4f(Color3f::green * 0.15f, 0.4f);
	cp.m_startSegment.m_colorRandom = Color4f::rgba(0.2f, 0.08f, 0.f, 0.f);

	cp.m_endSegment.m_size = 7;
	cp.m_endSegment.m_sizeRandom = 5;
	cp.m_endSegment.m_color = Color4f(Color3f::green * 0.1f, 0.4f);
	cp.m_endSegment.m_colorRandom = Color4f::rgba(0.2f, 0.08f, 0.f, 0.f);

	cp.m_blendMode = RenderMaterial::Screen;
	cp.m_freq = 80;
	cp.m_texture.set("graph/particles/big_greypouf", 0, 200);
	cp.m_spawnFlags = 0;
	g_particleParameters[ParticleParam_Poison3] = cp;
	}
}
