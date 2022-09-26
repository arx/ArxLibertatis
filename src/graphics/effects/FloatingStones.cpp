/*
 * Copyright 2015-2021 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/effects/FloatingStones.h"

#include "core/GameTime.h"
#include "graphics/RenderBatcher.h"
#include "graphics/particle/ParticleEffects.h"
#include "math/Random.h"
#include "math/RandomVector.h"
#include "util/Range.h"


void FloatingStones::Init(float radius) {
	
	m_baseRadius = radius;
	
	m_timestone = 0;
	
}

void FloatingStones::Update(GameDuration timeDelta, Vec3f pos) {
	
	m_timestone -= timeDelta;
	if(m_timestone <= 0) {
		m_timestone = std::chrono::milliseconds(Random::get(50, 150));
		AddStone(pos + arx::randomOffsetXZ(m_baseRadius));
	}
	
	if(m_quantizer.update(toMsf(timeDelta) * 0.03f)) {
		for(Stone & stone : m_stones) {
			stone.yvel *= 1.f - (1.f / 100.f);
			PARTICLE_DEF * pd = createParticle(true);
			if(!pd) {
				continue;
			}
			pd->ov = stone.pos;
			pd->move = Vec3f(0.f, Random::getf(0.f, 3.f), 0.f);
			pd->siz = Random::getf(3.f, 6.f);
			pd->tolive = 1000;
			pd->timcreation = -(toMsi(g_gameTime.now()) + 1000l); // TODO WTF
			pd->m_flags = FIRE_TO_SMOKE | FADE_IN_AND_OUT | ROTATING | DISSIPATING;
			pd->m_rotation = 0.0000001f;
		}
	}
	
	for(Stone & stone : m_stones) {
		float a = timeDelta / stone.time * 100.f;
		stone.pos.y += stone.yvel * a;
		stone.ang += stone.angvel * a;
		stone.currtime += timeDelta;
	}
	
	util::unordered_remove_if(m_stones, [](const Stone & stone) { return stone.currtime > stone.time; });
	
}

void FloatingStones::AddStone(const Vec3f & pos) {
	
	Stone & stone = m_stones.emplace_back();
	
	stone.numstone = Random::get(0, 1);
	stone.pos = pos;
	stone.yvel = Random::getf(-5.f, 0.f);
	stone.ang = Anglef(Random::getf(), Random::getf(), Random::getf()) * Anglef(360.f, 360.f, 360.f);
	stone.angvel = Anglef(Random::getf(), Random::getf(), Random::getf()) * Anglef(5.f, 6.f, 3.f);
	stone.scale = Vec3f(Random::getf(0.2f, 0.5f));
	stone.time = std::chrono::milliseconds(Random::get(2000, 2500));
	stone.currtime = 0;
	
}

void FloatingStones::DrawStone() {
	
	RenderMaterial mat;
	mat.setDepthTest(true);
	mat.setBlendType(RenderMaterial::Screen);
	
	for(Stone & stone : m_stones) {
		float age = stone.currtime / stone.time;
		Color4f col = Color4f::white;
		if(age < 0.2f) {
			col = Color4f::gray(age * 5.f);
		} else if(age > 0.5f) {
			col = Color4f::gray(2.f - age * 2.f);
		}
		EERIE_3DOBJ * obj = (stone.numstone == 0) ? stone0 : stone1;
		Draw3DObject(obj, stone.ang, stone.pos, stone.scale, col, mat);
	}
	
}
