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

#include "scene/Light.h"

using std::list;

void ParticleSystem::RecomputeDirection() {
	Vec3f eVect = m_parameters.m_direction;
	eVect.y = -eVect.y;
	GenerateMatrixUsingVector(eMat, eVect, 0);
}

ParticleSystem::ParticleSystem() {
	
	int i;
	for(i = 0; i < 20; i++) {
		tex_tab[i] = NULL;
	}
	
	m_lightHandle = InvalidLightHandle;
	
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
	if(lightHandleIsValid(m_lightHandle)) {
		EERIE_LIGHT * light = lightHandleGet(m_lightHandle);
		
		light->pos = m_nextPosition;
	}
}

void ParticleSystem::SetColor(float _fR, float _fG, float _fB) {
	if(lightHandleIsValid(m_lightHandle)) {
		EERIE_LIGHT * light = lightHandleGet(m_lightHandle);
		
		light->rgb = Color3f(_fR, _fG, _fB);
	}
}

void ParticleSystem::SetParams(const ParticleParams & _pp) {
	
	m_parameters = _pp;
	
	m_parameters.m_direction = glm::normalize(m_parameters.m_direction);
	Vec3f eVect(m_parameters.m_direction.x, -m_parameters.m_direction.y, m_parameters.m_direction.z);
	GenerateMatrixUsingVector(eMat, eVect, 0);

	float r = (m_parameters.m_startSegment.m_color.r  + m_parameters.m_endSegment.m_color.r ) * 0.5f;
	float g = (m_parameters.m_startSegment.m_color.g  + m_parameters.m_endSegment.m_color.g ) * 0.5f;
	float b = (m_parameters.m_startSegment.m_color.b  + m_parameters.m_endSegment.m_color.b ) * 0.5f;
	SetColor(r, g, b);
	
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
		_iNbTex = min(_iNbTex, 20);
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

void VectorRotateY(Vec3f & _eIn, Vec3f & _eOut, float _fAngle) {
	float c = std::cos(_fAngle);
	float s = std::sin(_fAngle);
	_eOut.x = (_eIn.x * c) + (_eIn.z * s);
	_eOut.y =  _eIn.y;
	_eOut.z = (_eIn.z * c) - (_eIn.x * s);
}

void VectorRotateZ(Vec3f & _eIn, Vec3f & _eOut, float _fAngle) {
	float c = std::cos(_fAngle);
	float s = std::sin(_fAngle);
	_eOut.x = (_eIn.x * c) + (_eIn.y * s);
	_eOut.y = (_eIn.y * c) - (_eIn.x * s);
	_eOut.z =  _eIn.z;
}

void ParticleSystem::SetParticleParams(Particle * pP) {

	pP->p3Pos = Vec3f_ZERO;
	
	if((m_parameters.m_spawnFlags & PARTICLE_CIRCULAR) == PARTICLE_CIRCULAR
	   && (m_parameters.m_spawnFlags & PARTICLE_BORDER) == PARTICLE_BORDER) {
		float randd = rnd() * 360.f;
		pP->p3Pos.x = std::sin(randd) * m_parameters.m_pos.x;
		pP->p3Pos.y = rnd() * m_parameters.m_pos.y;
		pP->p3Pos.z = std::cos(randd) * m_parameters.m_pos.z;
	} else if((m_parameters.m_spawnFlags & PARTICLE_CIRCULAR) == PARTICLE_CIRCULAR) {
		float randd = rnd() * 360.f;
		pP->p3Pos.x = std::sin(randd) * rnd() * m_parameters.m_pos.x;
		pP->p3Pos.y = rnd() * m_parameters.m_pos.y;
		pP->p3Pos.z = std::cos(randd) * rnd() * m_parameters.m_pos.z;
	} else {
		pP->p3Pos = m_parameters.m_pos * randomVec(-1.f, 1.f);
	}

	float fTTL = m_parameters.m_life + rnd() * m_parameters.m_lifeRandom;
	pP->m_timeToLive = checked_range_cast<long>(fTTL);
	pP->fOneOnTTL = 1.0f / (float)pP->m_timeToLive;

	float fAngleX = rnd() * m_parameters.m_angle; //*0.5f;
 
	Vec3f vv1, vvz;
	
	// ici modifs ----------------------------------
	
	vv1 = -Vec3f_Y_AXIS;
	
	VectorRotateZ(vv1, vvz, fAngleX); 
	VectorRotateY(vvz, vv1, radians(rnd() * 360.0f));
	
	vvz = Vec3f(eMat * Vec4f(vv1, 1.f));

	float fSpeed = m_parameters.m_speed + rnd() * m_parameters.m_speedRandom;

	pP->p3Velocity = vvz * fSpeed;
	pP->fSizeStart = m_parameters.m_startSegment.m_size + rnd() * m_parameters.m_startSegment.m_sizeRandom;

	{
	Color4f rndColor = Color4f(rnd(), rnd(), rnd(), rnd());
	pP->fColorStart = m_parameters.m_startSegment.m_color + rndColor * m_parameters.m_startSegment.m_colorRandom;
	}

	pP->fSizeEnd = m_parameters.m_endSegment.m_size + rnd() * m_parameters.m_endSegment.m_sizeRandom;

	{
	Color4f rndColor = Color4f(rnd(), rnd(), rnd(), rnd());
	pP->fColorEnd = m_parameters.m_endSegment.m_color + rndColor * m_parameters.m_endSegment.m_colorRandom;
	}
	
	if(m_parameters.m_rotationRandomDirection) {
		float fRandom	= frand2();

		pP->iRot = checked_range_cast<int>(fRandom);

		if(pP->iRot < 0)
			pP->iRot = -1;

		if(pP->iRot >= 0)
			pP->iRot = 1;
	} else {
		pP->iRot = 1;
	}

	if(m_parameters.m_rotationRandomStart) {
		pP->fRotStart = rnd() * 360.0f;
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

void ParticleSystem::Update(long _lTime) {

	if(arxtime.is_paused())
		return;

	float fTimeSec = _lTime * ( 1.0f / 1000 );
	
	iParticleNbAlive = 0;
	
	list<Particle *>::iterator i;
	for(i = listParticle.begin(); i != listParticle.end(); ) {
		Particle * pP = *i;
		
		if(pP->isAlive()) {
			pP->Update(_lTime);
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
			pP->Validate();
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

	int inumtex = 0;

	list<Particle *>::iterator i;

	for(i = listParticle.begin(); i != listParticle.end(); ++i) {
		Particle * p = *i;

		if(p->isAlive()) {
			if(m_parameters.m_flash > 0) {
				if(rnd() < m_parameters.m_flash)
					continue;
			}

			if(iNbTex > 0) {
				inumtex = p->iTexNum;

				if(iTexTime == 0) {
					float fNbTex	= (p->m_age * p->fOneOnTTL) * (iNbTex);

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
					fRot = (m_parameters.m_rotation) * p->m_age + p->fRotStart;
				else
					fRot = (-m_parameters.m_rotation) * p->m_age + p->fRotStart;

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
