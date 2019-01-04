/*
 * Copyright 2015-2018 Arx Libertatis Team (see the AUTHORS file)
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

void FloatingStones::Init(float radius) {
	
	m_baseRadius = radius;
	
	// cailloux
	m_timestone = 0;
	m_nbstone = 0;

	int nb = 256;
	while(nb--) {
		m_tstone[nb].actif = 0;
	}
}

void FloatingStones::Update(GameDuration timeDelta, Vec3f pos) {
	
	m_timestone -= timeDelta;
	m_currframetime = timeDelta;
	
	if(m_timestone <= 0) {
		m_timestone = GameDurationMs(Random::get(50, 150));
		
		AddStone(pos + arx::randomOffsetXZ(m_baseRadius));
	}
}

void FloatingStones::AddStone(const Vec3f & pos) {
	
	if(g_gameTime.isPaused() || m_nbstone > 255) {
		return;
	}
	
	int nb = 256;
	while(nb--) {
		T_STONE & s = m_tstone[nb];
		
		if(!s.actif) {
			m_nbstone++;
			s.actif = 1;
			s.numstone = Random::get(0, 1);
			s.pos = pos;
			s.yvel = Random::getf(-5.f, 0.f);
			s.ang = Anglef(Random::getf(), Random::getf(), Random::getf()) * Anglef(360.f, 360.f, 360.f);
			s.angvel = Anglef(Random::getf(), Random::getf(), Random::getf()) * Anglef(5.f, 6.f, 3.f);
			s.scale = Vec3f(Random::getf(0.2f, 0.5f));
			s.time = GameDurationMs(Random::get(2000, 2500));
			s.currtime = 0;
			break;
		}
	}
}

void FloatingStones::DrawStone()
{
	RenderMaterial mat;
	mat.setDepthTest(true);
	mat.setBlendType(RenderMaterial::Screen);
	
	int nb = 256;
	while(nb--) {
		T_STONE & s = m_tstone[nb];
		
		if(s.actif) {
			float a = s.currtime / s.time;
			
			if(a > 1.f) {
				a = 1.f;
				s.actif = 0;
			}
			
			Color4f col = Color4f(Color3f::white, 1.f - a);
			EERIE_3DOBJ * obj = (s.numstone == 0) ? stone0 : stone1;
			Draw3DObject(obj, s.ang, s.pos, s.scale, col, mat);
			
			PARTICLE_DEF * pd = createParticle();
			if(pd) {
				pd->ov = s.pos;
				pd->move = Vec3f(0.f, Random::getf(0.f, 3.f), 0.f);
				pd->siz = Random::getf(3.f, 6.f);
				pd->tolive = 1000;
				pd->timcreation = -(toMsi(g_gameTime.now()) + 1000l); // TODO WTF
				pd->m_flags = FIRE_TO_SMOKE | FADE_IN_AND_OUT | ROTATING | DISSIPATING;
				pd->m_rotation = 0.0000001f;
			}
			
			// Update mvt
			if(!g_gameTime.isPaused()) {
				a = (m_currframetime * 100) / s.time;
				s.pos.y += s.yvel * a;
				s.ang += s.angvel * a;
				s.yvel *= 1.f - (1.f / 100.f);
				s.currtime += m_currframetime;
			}
			
		}
		
	}
	
}
