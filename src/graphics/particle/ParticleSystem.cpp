/*
 * Copyright 2011-2021 Arx Libertatis Team (see the AUTHORS file)
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
/* Based on:
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/

#include "graphics/particle/ParticleSystem.h"

#include <cstdio>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <iomanip>

#include "core/GameTime.h"

#include "graphics/Draw.h"
#include "graphics/Math.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/SpellEffects.h"
#include "graphics/particle/ParticleParams.h"
#include "graphics/particle/Particle.h"

#include "math/Random.h"
#include "math/RandomVector.h"

#include "util/Range.h"


ParticleSystem::ParticleSystem()
	: m_nextPosition(0.f)
	, iParticleNbAlive(0)
	, iNbTex(0)
	, iTexTime(500)
	, bTexLoop(true)
	, eMat(1.f)
{
	
	std::fill(tex_tab.begin(), tex_tab.end(), nullptr);
	
	m_parameters.m_nbMax = 50;
	m_parameters.m_rotation = 0;
	m_parameters.m_freq = -1;
	m_parameters.m_spawnFlags = 0;
	m_parameters.m_direction = Vec3f(0.f, -1.f, 0.f);
	m_parameters.m_startSegment.m_size = 1;
	m_parameters.m_endSegment.m_size = 1;
	m_parameters.m_startSegment.m_color = Color4f(0.1f, 0.1f, 0.1f, 0.1f);
	m_parameters.m_endSegment.m_color = Color4f(0.1f, 0.1f, 0.1f, 0.1f);
	m_parameters.m_speed = 10;
	m_parameters.m_life = 1000;
	m_parameters.m_pos = Vec3f(0.f);
	m_parameters.m_rotation = 0;
	m_parameters.m_rotationRandomDirection = false;
	m_parameters.m_rotationRandomStart = false;
	m_parameters.m_gravity = Vec3f(0.f);
	m_parameters.m_lifeRandom = 1000;
	m_parameters.m_angle = 0;
	m_parameters.m_speedRandom = 10;
	m_parameters.m_startSegment.m_sizeRandom = 1;
	m_parameters.m_startSegment.m_colorRandom = Color4f(0.1f, 0.1f, 0.1f, 0.1f);
	m_parameters.m_endSegment.m_sizeRandom = 1;
	m_parameters.m_endSegment.m_colorRandom = Color4f(0.1f, 0.1f, 0.1f, 0.1f);
	m_parameters.m_blendMode = RenderMaterial::Additive;
	
	GenerateMatrixUsingVector(eMat, Vec3f(0.f, 1.f, 0.f), 0);
	
}

ParticleSystem::~ParticleSystem() {
	
	for(Particle * p : listParticle) {
		delete p;
	}
	
	listParticle.clear();
}

void ParticleSystem::SetPos(const Vec3f & pos) {
	
	m_nextPosition = pos;
}

void ParticleSystem::SetParams(const ParticleParams & params) {
	
	m_parameters = params;
	
	m_parameters.m_direction = glm::normalize(m_parameters.m_direction);
	Vec3f eVect(m_parameters.m_direction.x, -m_parameters.m_direction.y, m_parameters.m_direction.z);
	GenerateMatrixUsingVector(eMat, eVect, 0);
	
	const ParticleParams::TextureInfo & texInfo = m_parameters.m_texture;
	SetTexture(texInfo.m_texName, texInfo.m_texNb, texInfo.m_texTime);
	
}

void ParticleSystem::SetTexture(const char * _pszTex, int _iNbTex, int _iTime) {

	if(_iNbTex == 0) {
		tex_tab[0] = TextureContainer::Load(_pszTex);
		iNbTex = 0;
	} else {
		
		_iNbTex = std::min(_iNbTex, 20);
		
		std::ostringstream oss;
		for(int i = 0; i < _iNbTex; i++) {
			oss.str(std::string());
			oss << _pszTex << std::setfill('0') << std::setw(4) << (i + 1);
			tex_tab[i] = TextureContainer::Load(oss.str());
		}
		
		iNbTex = _iNbTex;
		iTexTime = _iTime;
		bTexLoop = true;
		
	}
	
}

void ParticleSystem::SetParticleParams(Particle * particle) {
	
	particle->p3Pos = Vec3f(0.f);
	
	if((m_parameters.m_spawnFlags & PARTICLE_CIRCULAR) == PARTICLE_CIRCULAR) {
		
		Vec2f pos = arx::circularRand(1.f);
		particle->p3Pos.x = pos.x;
		particle->p3Pos.z = pos.y;
		
		particle->p3Pos.y = Random::getf();
		
		if((m_parameters.m_spawnFlags & PARTICLE_BORDER) != PARTICLE_BORDER) {
			particle->p3Pos *= Vec3f(Random::getf(), 1.f, Random::getf());
		}
	} else {
		particle->p3Pos = arx::randomVec(-1.f, 1.f);
	}
	
	particle->p3Pos *= m_parameters.m_pos;
	
	float fTTL = m_parameters.m_life + Random::getf() * m_parameters.m_lifeRandom;
	particle->m_timeToLive = std::max<GameDuration>(std::chrono::duration<float, std::milli>(fTTL), 100ms);
	
	float fAngleX = Random::getf() * m_parameters.m_angle;
	
	Vec3f vvz = VRotateZ(Vec3f(0.f, -1.f, 0.f), glm::degrees(fAngleX));
	vvz = VRotateY(vvz, Random::getf(0.f, 360.0f));
	vvz = Vec3f(eMat * Vec4f(vvz, 1.f));
	
	float fSpeed = m_parameters.m_speed + Random::getf() * m_parameters.m_speedRandom;
	particle->p3Velocity = vvz * fSpeed;
	particle->fSizeStart = std::max(m_parameters.m_startSegment.m_size +
	                                Random::getf() * m_parameters.m_startSegment.m_sizeRandom, 1.f);
	{
		Color4f rndColor = Color4f(Random::getf(), Random::getf(), Random::getf(), Random::getf());
		particle->fColorStart = clamp(m_parameters.m_startSegment.m_color +
		                              rndColor * m_parameters.m_startSegment.m_colorRandom);
	}
	
	particle->fSizeEnd = std::max(m_parameters.m_endSegment.m_size +
	                              Random::getf() * m_parameters.m_endSegment.m_sizeRandom, 1.f);
	{
		Color4f rndColor = Color4f(Random::getf(), Random::getf(), Random::getf(), Random::getf());
		particle->fColorEnd = clamp(m_parameters.m_endSegment.m_color +
		                            rndColor * m_parameters.m_endSegment.m_colorRandom);
	}
	
	if(m_parameters.m_rotationRandomDirection) {
		particle->iRot = Random::get(-1, 1);
		if(particle->iRot < 0) {
			particle->iRot = -1;
		}
		if(particle->iRot >= 0) {
			particle->iRot = 1;
		}
	} else {
		particle->iRot = 1;
	}
	
	if(m_parameters.m_rotationRandomStart) {
		particle->fRotStart = Random::getf(0.f, 360.0f);
	} else {
		particle->fRotStart = 0;
	}
	
}


void ParticleSystem::StopEmission() {
	m_parameters.m_nbMax = 0;
}

bool ParticleSystem::IsAlive() const {
	return (iParticleNbAlive != 0 || m_parameters.m_nbMax != 0);
}

void ParticleSystem::Update(GameDuration delta) {
	
	if(g_gameTime.isPaused()) {
		return;
	}
	
	float fTimeSec = delta / 1s;
	
	iParticleNbAlive = 0;
	
	std::list<Particle *>::iterator i;
	for(i = listParticle.begin(); i != listParticle.end(); ) {
		Particle * pP = *i;
		
		if(pP->isAlive()) {
			pP->Update(delta);
			pP->p3Velocity += m_parameters.m_gravity * fTimeSec;
			iParticleNbAlive++;
			++i;
		} else {
			if(iParticleNbAlive >= m_parameters.m_nbMax) {
				delete pP;
				i = listParticle.erase(i);
			} else {
				pP->Regen();
				SetParticleParams(pP);
				pP->Update(0);
				iParticleNbAlive++;
				++i;
			}
		}
	}

	// création de particules en fct de la fréquence
	if(iParticleNbAlive < m_parameters.m_nbMax) {
		size_t t = m_parameters.m_nbMax - iParticleNbAlive;
		
		if(m_parameters.m_freq != -1.f) {
			t = std::min(size_t(m_storedTime.update(fTimeSec * m_parameters.m_freq)), t);
		}
		
		for(size_t iNb = 0; iNb < t; iNb++) {
			Particle * pP  = new Particle();
			SetParticleParams(pP);
			pP->Update(0);
			listParticle.insert(listParticle.end(), pP);
			iParticleNbAlive++;
		}
	}
	
	if(!m_parameters.m_looping) {
		StopEmission();
	}
	
}

void ParticleSystem::Render() {
	
	RenderMaterial mat;
	mat.setBlendType(m_parameters.m_blendMode);
	mat.setDepthTest(true);
	
	std::list<Particle *>::iterator i;
	
	for(Particle & particle : util::dereference(listParticle)) {
		
		if(!particle.isAlive()) {
			continue;
		}
		
		float t = particle.m_age / particle.m_timeToLive;
		
		int inumtex = 0;
		if(iNbTex > 0) {
			inumtex = particle.iTexNum;
			if(iTexTime == 0) {
				inumtex = std::min(checked_range_cast<int>(t * iNbTex), iNbTex - 1);
			} else if(particle.iTexTime > iTexTime) {
				particle.iTexTime -= iTexTime;
				particle.iTexNum++;
				if(particle.iTexNum > iNbTex - 1) {
					particle.iTexNum = bTexLoop ? 0 : iNbTex - 1;
				}
				inumtex = particle.iTexNum;
			}
		}
		
		if(!tex_tab[inumtex]) {
			continue;
		}
		
		mat.setTexture(tex_tab[inumtex]);
		
		Vec3f pos = m_nextPosition + particle.p3Pos;
		float size = glm::mix(particle.fSizeStart, particle.fSizeEnd, t);
		Color color = Color(particle.fColorStart + (particle.fColorEnd - particle.fColorStart) * t);
		
		if(m_parameters.m_rotation != 0) {
			float rotation = particle.fRotStart;
			rotation += (particle.iRot == 1 ? 1.f : -1.f) * m_parameters.m_rotation * toMsf(particle.m_age);
			EERIEAddSprite(mat, pos, std::max(size, 0.f), color, 2, rotation);
		} else {
			EERIEAddSprite(mat, pos, size, color, 2);
		}
		
	}
	
}
