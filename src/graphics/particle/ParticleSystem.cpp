/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#include <boost/foreach.hpp>

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


ParticleSystem::ParticleSystem() {
	
	int i;
	for(i = 0; i < 20; i++) {
		tex_tab[i] = NULL;
	}
	
	m_parameters.m_nbMax = 50;
	
	iParticleNbAlive = 0;
	iNbTex = 0;
	iTexTime = 500;
	bTexLoop = true;
	m_parameters.m_rotation = 0;
	
	m_parameters.m_freq = -1;
	m_parameters.m_spawnFlags = 0;
	
	// default settings for EDITOR MODE only
	Vec3f eVect = m_parameters.m_direction = -Vec3f_Y_AXIS;
	eVect.y = -eVect.y;
	GenerateMatrixUsingVector(eMat, eVect, 0);
	
	m_parameters.m_startSegment.m_size = 1;
	m_parameters.m_endSegment.m_size = 1;
	m_parameters.m_startSegment.m_color = Color4f(0.1f, 0.1f, 0.1f, 0.1f);
	m_parameters.m_endSegment.m_color = Color4f(0.1f, 0.1f, 0.1f, 0.1f);
	m_parameters.m_speed = 10;
	m_parameters.m_life = 1000;
	m_parameters.m_pos = Vec3f_ZERO;
	m_parameters.m_flash = 0;
	m_parameters.m_rotation = 0;
	m_parameters.m_rotationRandomDirection = false;
	m_parameters.m_rotationRandomStart = false;
	m_parameters.m_gravity = Vec3f_ZERO;
	m_parameters.m_lifeRandom = 1000;
	m_parameters.m_angle = 0;
	m_parameters.m_speedRandom = 10;

	m_parameters.m_startSegment.m_sizeRandom = 1;
	m_parameters.m_startSegment.m_colorRandom = Color4f(0.1f, 0.1f, 0.1f, 0.1f);

	m_parameters.m_endSegment.m_sizeRandom = 1;
	m_parameters.m_endSegment.m_colorRandom = Color4f(0.1f, 0.1f, 0.1f, 0.1f);

	m_parameters.m_blendMode = RenderMaterial::Additive;
}

ParticleSystem::~ParticleSystem() {
	
	BOOST_FOREACH(Particle * p, listParticle) {
		delete p;
	}
	
	listParticle.clear();
}

void ParticleSystem::SetPos(const Vec3f & pos) {
	
	m_nextPosition = pos;
}

void ParticleSystem::SetParams(const ParticleParams & _pp) {
	
	m_parameters = _pp;
	
	m_parameters.m_direction = glm::normalize(m_parameters.m_direction);
	Vec3f eVect(m_parameters.m_direction.x, -m_parameters.m_direction.y, m_parameters.m_direction.z);
	GenerateMatrixUsingVector(eMat, eVect, 0);
	
	{
		ParticleParams::TextureInfo & texInfo = m_parameters.m_texture;
		SetTexture(texInfo.m_texName, texInfo.m_texNb, texInfo.m_texTime);
	}
}

void ParticleSystem::SetTexture(const char * _pszTex, int _iNbTex, int _iTime) {

	if(_iNbTex == 0) {
		tex_tab[0] = TextureContainer::Load(_pszTex);
		iNbTex = 0;
	} else {
		_iNbTex = std::min(_iNbTex, 20);
		char cBuf[256];

		for(int i = 0; i < _iNbTex; i++) {
			memset(cBuf, 0, 256);
			sprintf(cBuf, "%s_%04d", _pszTex, i + 1);
			tex_tab[i] = TextureContainer::Load(cBuf);
		}

		iNbTex = _iNbTex;
		iTexTime = _iTime;
		bTexLoop = true;
	}
}

void ParticleSystem::SetParticleParams(Particle * pP) {

	pP->p3Pos = Vec3f_ZERO;
	
	if((m_parameters.m_spawnFlags & PARTICLE_CIRCULAR) == PARTICLE_CIRCULAR) {
		
		Vec2f pos = arx::circularRand(1.f);
		pP->p3Pos.x = pos.x;
		pP->p3Pos.z = pos.y;
		
		pP->p3Pos.y = Random::getf();
		
		if((m_parameters.m_spawnFlags & PARTICLE_BORDER) != PARTICLE_BORDER) {
			pP->p3Pos *= Vec3f(Random::getf(), 1.f, Random::getf());
		}
	} else {
		pP->p3Pos = arx::randomVec(-1.f, 1.f);
	}
	
	pP->p3Pos *= m_parameters.m_pos;
	
	float fTTL = m_parameters.m_life + Random::getf() * m_parameters.m_lifeRandom;
	pP->m_timeToLive = ArxDurationMs(fTTL);
	pP->fOneOnTTL = 1.0f / toMs(pP->m_timeToLive);

	float fAngleX = Random::getf() * m_parameters.m_angle; //*0.5f;
 
	Vec3f vv1, vvz;
	
	// ici modifs ----------------------------------
	
	vv1 = -Vec3f_Y_AXIS;
	
	vvz = VRotateZ(vv1, glm::degrees(fAngleX));
	vv1 = VRotateY(vvz, Random::getf(0.f, 360.0f));
	
	vvz = Vec3f(eMat * Vec4f(vv1, 1.f));

	float fSpeed = m_parameters.m_speed + Random::getf() * m_parameters.m_speedRandom;

	pP->p3Velocity = vvz * fSpeed;
	pP->fSizeStart = m_parameters.m_startSegment.m_size + Random::getf() * m_parameters.m_startSegment.m_sizeRandom;

	{
	Color4f rndColor = Color4f(Random::getf(), Random::getf(), Random::getf(), Random::getf());
	pP->fColorStart = m_parameters.m_startSegment.m_color + rndColor * m_parameters.m_startSegment.m_colorRandom;
	}

	pP->fSizeEnd = m_parameters.m_endSegment.m_size + Random::getf() * m_parameters.m_endSegment.m_sizeRandom;

	{
	Color4f rndColor = Color4f(Random::getf(), Random::getf(), Random::getf(), Random::getf());
	pP->fColorEnd = m_parameters.m_endSegment.m_color + rndColor * m_parameters.m_endSegment.m_colorRandom;
	}
	
	if(m_parameters.m_rotationRandomDirection) {
		pP->iRot = Random::get(-1, 1);

		if(pP->iRot < 0)
			pP->iRot = -1;

		if(pP->iRot >= 0)
			pP->iRot = 1;
	} else {
		pP->iRot = 1;
	}

	if(m_parameters.m_rotationRandomStart) {
		pP->fRotStart = Random::getf(0.f, 360.0f);
	} else {
		pP->fRotStart = 0;
	}
}


void ParticleSystem::StopEmission() {
	m_parameters.m_nbMax = 0;
}

bool ParticleSystem::IsAlive() {

	if((iParticleNbAlive == 0) && (m_parameters.m_nbMax == 0))
		return false;

	return true;
}

void ParticleSystem::Update(ArxDuration delta) {

	if(arxtime.is_paused())
		return;

	float fTimeSec = toMs(delta) * ( 1.0f / 1000 );
	
	iParticleNbAlive = 0;
	
	std::list<Particle *>::iterator i;
	for(i = listParticle.begin(); i != listParticle.end(); ) {
		Particle * pP = *i;
		
		if(pP->isAlive()) {
			pP->Update(delta);
			pP->p3Velocity += m_parameters.m_gravity * fTimeSec;
			iParticleNbAlive ++;
			++i;
		} else {
			if(iParticleNbAlive >= m_parameters.m_nbMax) {
				delete pP;
				i = listParticle.erase(i);
			} else {
				pP->Regen();
				SetParticleParams(pP);
				pP->Validate();
				pP->Update(ArxDuration_ZERO);
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
			pP->Validate();
			pP->Update(ArxDuration_ZERO);
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

	int inumtex = 0;

	std::list<Particle *>::iterator i;

	for(i = listParticle.begin(); i != listParticle.end(); ++i) {
		Particle * p = *i;

		if(p->isAlive()) {
			if(m_parameters.m_flash > 0) {
				if(Random::getf() < m_parameters.m_flash)
					continue;
			}

			if(iNbTex > 0) {
				inumtex = p->iTexNum;

				if(iTexTime == 0) {
					float fNbTex = (toMs(p->m_age) * p->fOneOnTTL) * (iNbTex);

					inumtex = checked_range_cast<int>(fNbTex);
					if(inumtex >= iNbTex) {
						inumtex = iNbTex - 1;
					}
				} else {
					if(p->iTexTime > iTexTime) {
						p->iTexTime -= iTexTime;
						p->iTexNum++;

						if(p->iTexNum > iNbTex - 1) {
							if(bTexLoop) {
								p->iTexNum = 0;
							} else {
								p->iTexNum = iNbTex - 1;
							}
						}

						inumtex = p->iTexNum;
					}
				}
			}
			
			Vec3f p3pos;
			p3pos = p->p3Pos;
			p3pos += m_nextPosition;
            
			mat.setTexture(tex_tab[inumtex]);
			
			if(m_parameters.m_rotation != 0) {
				float fRot;
				if(p->iRot == 1)
					fRot = (m_parameters.m_rotation) * toMs(p->m_age) + p->fRotStart;
				else
					fRot = (-m_parameters.m_rotation) * toMs(p->m_age) + p->fRotStart;

				float size = std::max(p->fSize, 0.f);
				
				if(tex_tab[inumtex])
					EERIEAddSprite(mat, p3pos, size, p->ulColor, 2, fRot);
			} else {
				if(tex_tab[inumtex])
					EERIEAddSprite(mat, p3pos, p->fSize, p->ulColor, 2);
			}
		}
	}
}
