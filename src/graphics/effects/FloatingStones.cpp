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
#include "graphics/particle/ParticleTextures.h"
#include "math/Random.h"
#include "math/RandomVector.h"
#include "util/Range.h"


void FloatingStones::Init(float radius) {
	
	m_baseRadius = radius;
	
	m_timestone = 0;
	
}

void FloatingStones::Update(ShortGameDuration timeDelta, Vec3f pos, bool addStones) {
	
	arx_assume(timeDelta >= 0 && timeDelta <= GameTime::MaxFrameDuration);
	
	if(addStones) {
		m_timestone -= timeDelta;
		if(m_timestone <= 0) {
			m_timestone = Random::get(50ms, 150ms);
			AddStone(pos + arx::randomOffsetXZ(m_baseRadius));
		}
	}
	
	if(m_quantizer.update(toMsf(timeDelta) * 0.03f)) {
		for(Stone & stone : m_stones) {
			stone.yvel *= 1.f - (1.f / 100.f);
			if(Random::getf() <= 0.7f) {
				continue;
			}
			PARTICLE_DEF * pd = createParticle(true);
			if(!pd) {
				continue;
			}
			Vec3f move = Vec3f(0.f, Random::getf(0.f, 3.f), 0.f);
			pd->ov = stone.pos + move;
			pd->move = move * 0.5f;
			pd->size = Random::getf(1.f, 2.f);
			pd->sizeDelta = 2.4f;
			pd->duration = 1375ms;
			pd->m_flags = FADE_IN_AND_OUT | ROTATING | DISSIPATING;
			pd->m_rotation = 0.0000001f;
			pd->tc = g_particleTextures.smoke;
			pd->rgb = Color3f::gray(.45f);
		}
	}
	
	for(Stone & stone : m_stones) {
		arx_assume(stone.duration > 0 && stone.duration <= ShortGameDuration::max() / 2);
		float a = timeDelta / stone.duration * 100.f;
		stone.pos.y += stone.yvel * a;
		stone.ang += stone.angvel * a;
		stone.elapsed += timeDelta;
	}
	
	util::unordered_remove_if(m_stones, [](const Stone & stone) { return stone.elapsed > stone.duration; });
	
}

void FloatingStones::AddStone(const Vec3f & pos) {
	
	Stone & stone = m_stones.emplace_back();
	
	stone.numstone = Random::get(0, 1);
	stone.pos = pos;
	stone.yvel = Random::getf(-5.f, 0.f);
	stone.ang = Anglef(Random::getf(), Random::getf(), Random::getf()) * Anglef(360.f, 360.f, 360.f);
	stone.angvel = Anglef(Random::getf(), Random::getf(), Random::getf()) * Anglef(5.f, 6.f, 3.f);
	stone.scale = Vec3f(Random::getf(0.2f, 0.5f));
	stone.duration = Random::get(2000ms, 2500ms);
	
}

void FloatingStones::DrawStone() {
	
	RenderMaterial mat;
	mat.setDepthTest(true);
	mat.setBlendType(RenderMaterial::Screen);
	
	for(Stone & stone : m_stones) {
		arx_assume(stone.duration > 0 && stone.duration <= ShortGameDuration::max() / 2);
		arx_assume(stone.elapsed >= 0 && stone.elapsed <= stone.duration);
		float age = stone.elapsed / stone.duration;
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
