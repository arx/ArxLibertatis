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
	Vec3f eVect = p3ParticleDirection;
	eVect.y = -eVect.y;
	GenerateMatrixUsingVector(&eMat, &eVect, 0);
}

ParticleSystem::ParticleSystem() {
	
	int i;
	for(i = 0; i < 20; i++) {
		tex_tab[i] = NULL;
	}
	
	lLightId = -1;
	
	iParticleNbMax = 50;
	
	ulTime = 0;
	ulNbParticleGen = 10;
	iParticleNbAlive = 0;
	iNbTex = 0;
	iTexTime = 500;
	fParticleRotation = 0;
	
	fParticleFreq = -1;
	iSrcBlend = Renderer::BlendOne;
	iDstBlend = Renderer::BlendOne;
	ulParticleSpawn = 0;
	
	// default settings for EDITOR MODE only
	Vec3f eVect = p3ParticleDirection = -Vec3f_Y_AXIS;
	eVect.y = -eVect.y;
	GenerateMatrixUsingVector(&eMat, &eVect, 0);
	
	fParticleStartSize = 1;
	fParticleEndSize = 1;
	fParticleStartColor[0] = 0.1f;
	fParticleStartColor[1] = 0.1f;
	fParticleStartColor[2] = 0.1f;
	fParticleStartColor[3] = 0.1f;
	fParticleEndColor[0] = 0.1f;
	fParticleEndColor[1] = 0.1f;
	fParticleEndColor[2] = 0.1f;
	fParticleEndColor[3] = 0.1f;
	fParticleSpeed = 10;
	fParticleLife = 1000;
	p3ParticlePos = Vec3f_ZERO;
	bParticleFollow = true;
	fParticleFlash = 0;
	fParticleRotation = 0;
	bParticleRotationRandomDirection = false;
	bParticleRotationRandomStart = false;
	p3ParticleGravity = Vec3f_ZERO;
	fParticleLifeRandom = 1000;
	fParticleAngle = 0;
	fParticleSpeedRandom = 10;

	bParticleStartColorRandomLock = false;
	fParticleStartSizeRandom = 1;
	fParticleStartColorRandom[0] = 0.1f;
	fParticleStartColorRandom[1] = 0.1f;
	fParticleStartColorRandom[2] = 0.1f;
	fParticleStartColorRandom[3] = 0.1f;

	bParticleEndColorRandomLock = false;
	fParticleEndSizeRandom = 1;
	fParticleEndColorRandom[0] = 0.1f;
	fParticleEndColorRandom[1] = 0.1f;
	fParticleEndColorRandom[2] = 0.1f;
	fParticleEndColorRandom[3] = 0.1f;

	iSrcBlend = Renderer::BlendOne;
	iDstBlend = Renderer::BlendOne;
}

ParticleSystem::~ParticleSystem() {
	
	BOOST_FOREACH(Particle * p, listParticle) {
		delete p;
	}
	
	listParticle.clear();
}

void ParticleSystem::SetPos(const Vec3f & _p3) {
	
	p3Pos = _p3;
	if(lLightId != -1) {
		DynLight[lLightId].pos = p3Pos;
	}
}

void ParticleSystem::SetColor(float _fR, float _fG, float _fB) {
	if(lLightId != -1) {
		DynLight[lLightId].rgb = Color3f(_fR, _fG, _fB);
	}
}

void ParticleSystem::SetParams(const ParticleParams & _pp) {
	
	iParticleNbMax		= _pp.iNbMax;
	fParticleLife		= _pp.fLife;
	fParticleLifeRandom = _pp.fLifeRandom;
	
	p3ParticlePos = _pp.p3Pos;
	p3ParticleDirection = _pp.p3Direction * 0.1f;
	fParticleAngle = _pp.fAngle;
	fParticleSpeed = _pp.fSpeed;
	fParticleSpeedRandom = _pp.fSpeedRandom;
	p3ParticleGravity = _pp.p3Gravity;
	
	fParticleFlash = _pp.fFlash * ( 1.0f / 100 );
	
	if(_pp.fRotation >= 2) {
		fParticleRotation = 1.0f / (101 - _pp.fRotation);
	} else {
		fParticleRotation = 0.0f;
	}

	bParticleRotationRandomDirection = _pp.bRotationRandomDirection;
	bParticleRotationRandomStart = _pp.bRotationRandomStart;

	fParticleStartSize = _pp.fStartSize;
	fParticleStartSizeRandom = _pp.fStartSizeRandom;

	for(int i = 0; i < 4; i++) {
		fParticleStartColor[i] = _pp.fStartColor[i] / 255.0f;
		fParticleStartColorRandom[i] = _pp.fStartColorRandom[i] / 255.0f;
	}

	bParticleStartColorRandomLock = _pp.bStartLock;

	fParticleEndSize = _pp.fEndSize;
	fParticleEndSizeRandom = _pp.fEndSizeRandom;

	for(int i = 0; i < 4; i++) {
		fParticleEndColor[i] = _pp.fEndColor[i] / 255.0f;
		fParticleEndColorRandom[i] = _pp.fEndColorRandom[i] / 255.0f;
	}

	bParticleEndColorRandomLock = _pp.bEndLock;

	p3ParticleDirection = glm::normalize(p3ParticleDirection);
	Vec3f eVect(p3ParticleDirection.x, -p3ParticleDirection.y, p3ParticleDirection.z);
	GenerateMatrixUsingVector(&eMat, &eVect, 0);

	float r = (fParticleStartColor[0]  + fParticleEndColor[0] ) * 0.5f;
	float g = (fParticleStartColor[1]  + fParticleEndColor[1] ) * 0.5f;
	float b = (fParticleStartColor[2]  + fParticleEndColor[2] ) * 0.5f;
	SetColor(r, g, b);

	switch(_pp.iBlendMode) {
		case 0:
			iSrcBlend = Renderer::BlendOne;
			iDstBlend = Renderer::BlendOne;
			break;
		case 1:
			iSrcBlend = Renderer::BlendZero;
			iDstBlend = Renderer::BlendInvSrcColor;
			break;
		case 2:
			iSrcBlend = Renderer::BlendSrcColor;
			iDstBlend = Renderer::BlendDstColor;
			break;
		case 3:
			iSrcBlend = Renderer::BlendSrcAlpha;
			iDstBlend = Renderer::BlendOne;
			break;
		case 5:
			iSrcBlend = Renderer::BlendInvDstColor;
			iDstBlend = Renderer::BlendOne;
			break;
		default:
			iSrcBlend = Renderer::BlendOne;
			iDstBlend = Renderer::BlendOne;
			break;
	}

	if(_pp.bTexInfo) {
		SetTexture(_pp.lpszTexName, _pp.iTexNb, _pp.iTexTime, _pp.bTexLoop);
	}
}

void ParticleSystem::SetTexture(const char * _pszTex, int _iNbTex, int _iTime, bool _bLoop) {

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
		bTexLoop = _bLoop;
	}
}

void ParticleSystem::SpawnParticle(Particle * pP) {
	
	pP->p3Pos = Vec3f_ZERO;
	
	if((ulParticleSpawn & PARTICLE_CIRCULAR) == PARTICLE_CIRCULAR
	   && (ulParticleSpawn & PARTICLE_BORDER) == PARTICLE_BORDER) {
		float randd = rnd() * 360.f;
		pP->p3Pos.x = EEsin(randd) * p3ParticlePos.x;
		pP->p3Pos.y = rnd() * p3ParticlePos.y;
		pP->p3Pos.z = EEcos(randd) * p3ParticlePos.z;
	} else if((ulParticleSpawn & PARTICLE_CIRCULAR) == PARTICLE_CIRCULAR) {
		float randd = rnd() * 360.f;
		pP->p3Pos.x = EEsin(randd) * rnd() * p3ParticlePos.x;
		pP->p3Pos.y = rnd() * p3ParticlePos.y;
		pP->p3Pos.z = EEcos(randd) * rnd() * p3ParticlePos.z;
	} else {
		pP->p3Pos = p3ParticlePos * randomVec(-1.f, 1.f);
	}
	
	if(bParticleFollow == false) {
		pP->p3Pos = p3Pos;
	}
}

void VectorRotateY(Vec3f & _eIn, Vec3f & _eOut, float _fAngle) {
	float c = EEcos(_fAngle);
	float s = EEsin(_fAngle);
	_eOut.x = (_eIn.x * c) + (_eIn.z * s);
	_eOut.y =  _eIn.y;
	_eOut.z = (_eIn.z * c) - (_eIn.x * s);
}

void VectorRotateZ(Vec3f & _eIn, Vec3f & _eOut, float _fAngle) {
	float c = EEcos(_fAngle);
	float s = EEsin(_fAngle);
	_eOut.x = (_eIn.x * c) + (_eIn.y * s);
	_eOut.y = (_eIn.y * c) - (_eIn.x * s);
	_eOut.z =  _eIn.z;
}

void ParticleSystem::SetParticleParams(Particle * pP) {

	SpawnParticle(pP);

	float fTTL = fParticleLife + rnd() * fParticleLifeRandom;
	pP->ulTTL = checked_range_cast<long>(fTTL);
	pP->fOneOnTTL = 1.0f / (float)pP->ulTTL;

	float fAngleX = rnd() * fParticleAngle; //*0.5f;
 
	Vec3f vv1, vvz;
	vv1 = p3ParticleDirection;
	
	// ici modifs ----------------------------------
	
	vv1 = -Vec3f_Y_AXIS;
	
	VectorRotateZ(vv1, vvz, fAngleX); 
	VectorRotateY(vvz, vv1, radians(rnd() * 360.0f));
	VectorMatrixMultiply(&vvz, &vv1, &eMat);

	float fSpeed = fParticleSpeed + rnd() * fParticleSpeedRandom;

	pP->p3Velocity = vvz * fSpeed;
	pP->fSizeStart = fParticleStartSize + rnd() * fParticleStartSizeRandom;

	if(bParticleStartColorRandomLock) {
		float t = rnd() * fParticleStartColorRandom[0];
		pP->fColorStart[0] = fParticleStartColor[0] + t;
		pP->fColorStart[1] = fParticleStartColor[1] + t;
		pP->fColorStart[2] = fParticleStartColor[2] + t;
	} else {
		pP->fColorStart[0] = fParticleStartColor[0] + rnd() * fParticleStartColorRandom[0];
		pP->fColorStart[1] = fParticleStartColor[1] + rnd() * fParticleStartColorRandom[1];
		pP->fColorStart[2] = fParticleStartColor[2] + rnd() * fParticleStartColorRandom[2];
	}

	pP->fColorStart[3] = fParticleStartColor[3] + rnd() * fParticleStartColorRandom[3];

	pP->fSizeEnd = fParticleEndSize + rnd() * fParticleEndSizeRandom;

	if(bParticleEndColorRandomLock) {
		float t = rnd() * fParticleEndColorRandom[0];
		pP->fColorEnd[0] = fParticleEndColor[0] + t;
		pP->fColorEnd[1] = fParticleEndColor[1] + t;
		pP->fColorEnd[2] = fParticleEndColor[2] + t;
	} else {
		pP->fColorEnd[0] = fParticleEndColor[0] + rnd() * fParticleEndColorRandom[0];
		pP->fColorEnd[1] = fParticleEndColor[1] + rnd() * fParticleEndColorRandom[1];
		pP->fColorEnd[2] = fParticleEndColor[2] + rnd() * fParticleEndColorRandom[2];
	}

	pP->fColorEnd[3] = fParticleEndColor[3] + rnd() * fParticleEndColorRandom[3];

	if(bParticleRotationRandomDirection) {
		float fRandom	= frand2();

		pP->iRot = checked_range_cast<int>(fRandom);

		if(pP->iRot < 0)
			pP->iRot = -1;

		if(pP->iRot >= 0)
			pP->iRot = 1;
	} else {
		pP->iRot = 1;
	}

	if(bParticleRotationRandomStart) {
		pP->fRotStart = rnd() * 360.0f;
	} else {
		pP->fRotStart = 0;
	}
}

bool ParticleSystem::IsAlive() {

	if((iParticleNbAlive == 0) && (iParticleNbMax == 0))
		return false;

	return true;
}

void ParticleSystem::Update(long _lTime) {

	if(arxtime.is_paused())
		return;

	ulTime += _lTime;
	int nbtotal = 0;
	int iNb;
	float fTimeSec = _lTime * ( 1.0f / 1000 );
	Particle * pP;

	list<Particle *>::iterator i;
	iParticleNbAlive = 0;

	i = listParticle.begin();

	while(i != listParticle.end()) {
		pP = *i;
		++i;
		nbtotal++;

		if(pP->isAlive()) {
			pP->Update(_lTime);
			pP->p3Velocity += p3ParticleGravity * fTimeSec;
			iParticleNbAlive ++;
		} else {
			if(iParticleNbAlive >= iParticleNbMax) {
				delete pP;
				listParticle.remove(pP);
			} else {
				pP->Regen();
				SetParticleParams(pP);
				pP->Validate();
				pP->Update(0);
				ulNbParticleGen++;
				iParticleNbAlive++;
			}
		}
	}

	// création de particules en fct de la fréquence
	if(iParticleNbAlive < iParticleNbMax) {
		long t = iParticleNbMax - iParticleNbAlive;

		if(fParticleFreq != -1) {
			t = max(min(checked_range_cast<long>(fTimeSec * fParticleFreq), t), 1l);
		}

		for(iNb = 0; iNb < t; iNb++) {
			Particle * pP  = new Particle();
			SetParticleParams(pP);
			pP->Validate();
			pP->Update(0);
			listParticle.insert(listParticle.end(), pP);
			ulNbParticleGen ++;
			iParticleNbAlive++;
		}
	}
}

void ParticleSystem::Render() {
	
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(iSrcBlend, iDstBlend);

	int inumtex = 0;

	list<Particle *>::iterator i;

	for(i = listParticle.begin(); i != listParticle.end(); ++i) {
		Particle * p = *i;

		if(p->isAlive()) {
			if(fParticleFlash > 0) {
				if(rnd() < fParticleFlash)
					continue;
			}

			if(iNbTex > 0) {
				inumtex = p->iTexNum;

				if(iTexTime == 0) {
					float fNbTex	= (p->ulTime * p->fOneOnTTL) * (iNbTex);

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
			
			TexturedVertex p3pos;
			p3pos.p = p->p3Pos;
			if(bParticleFollow) {
				p3pos.p += p3Pos;
			}
			
			if(fParticleRotation != 0) {
				float fRot;
				if(p->iRot == 1)
					fRot = (fParticleRotation) * p->ulTime + p->fRotStart;
				else
					fRot = (-fParticleRotation) * p->ulTime + p->fRotStart;

				if(tex_tab[inumtex])
					EERIEDrawRotatedSprite(&p3pos, p->fSize, tex_tab[inumtex], p->ulColor, 2, fRot);
			} else {
				if(tex_tab[inumtex])
					EERIEDrawSprite(&p3pos, p->fSize, tex_tab[inumtex], p->ulColor, 2);
			}
		}
	}
}
