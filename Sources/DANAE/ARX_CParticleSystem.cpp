/*
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

//-----------------------------------------------------------------------------

#include "eerielight.h"
#include "eeriedraw.h"
#include "eerietypes.h"
#include "arx_time.h"
#include "arx_cspellfx.h"
#include "arx_cparticles.h"
#include "arx_cparticle.h"
#include "ARX_CParticleParams.h"

#include <d3d.h>

#include <list>
#include <vector>
using namespace std;

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

void CParticleSystem::RecomputeDirection()
{
	EERIE_3D eVect;
	eVect.x = p3ParticleDirection.x;
	eVect.y = -p3ParticleDirection.y;
	eVect.z = p3ParticleDirection.z;
	GenerateMatrixUsingVector(&eMat, &eVect, 0);
}
//-----------------------------------------------------------------------------
CParticleSystem::CParticleSystem()
{
	int i;

	for (i = 0; i < 20; i++)
	{
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
	iSrcBlend = D3DBLEND_ONE;
	iDstBlend = D3DBLEND_ONE;
	ulParticleSpawn = 0;

	// default settings for EDITOR MODE only
	p3ParticleDirection.x = 0;
	p3ParticleDirection.y = -1;
	p3ParticleDirection.z = 0;
	EERIE_3D eVect;
	eVect.x = p3ParticleDirection.x;
	eVect.y = -p3ParticleDirection.y;
	eVect.z = p3ParticleDirection.z;
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

	p3ParticlePos.x = 0;
	p3ParticlePos.y = 0;
	p3ParticlePos.z = 0;

	bParticleFollow = true;

	fParticleFlash = 0;
	fParticleRotation = 0;
	bParticleRotationRandomDirection = false;
	bParticleRotationRandomStart = false;

	p3ParticleGravity.x = 0;
	p3ParticleGravity.y = 0;
	p3ParticleGravity.z = 0;
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

	iSrcBlend = 0;
	iDstBlend = 0;
	iSrcBlend = D3DBLEND_ONE;
	iDstBlend = D3DBLEND_ONE;
}

//-----------------------------------------------------------------------------
CParticleSystem::~CParticleSystem()
{
	list<CParticle *>::iterator i = listParticle.begin();
	CParticle * pP;

	while (i != listParticle.end())
	{
		pP = *i;
		++i;

		if (pP)
		{
			delete pP;
			listParticle.remove(pP);
		}
	}

	listParticle.clear();
}

//-----------------------------------------------------------------------------
void CParticleSystem::SetPos(Point3 _p3)
{
	p3Pos.x = _p3.x;
	p3Pos.y = _p3.y;
	p3Pos.z = _p3.z;

	if (lLightId != -1)
	{
		DynLight[lLightId].pos.x = p3Pos.x;
		DynLight[lLightId].pos.y = p3Pos.y;
		DynLight[lLightId].pos.z = p3Pos.z;
	}
}

//-----------------------------------------------------------------------------
void CParticleSystem::SetColor(float _fR, float _fG, float _fB)
{
	if (lLightId != -1)
	{
		DynLight[lLightId].rgb.r = _fR;
		DynLight[lLightId].rgb.g = _fG;
		DynLight[lLightId].rgb.b = _fB;
	}
}

//-----------------------------------------------------------------------------
void CParticleSystem::SetParams(CParticleParams & _pp)
{
	iParticleNbMax		= _pp.iNbMax;
	fParticleLife		= _pp.fLife;
	fParticleLifeRandom = _pp.fLifeRandom;

	p3ParticlePos.x = _pp.p3Pos.x;
	p3ParticlePos.y = _pp.p3Pos.y;
	p3ParticlePos.z = _pp.p3Pos.z;
	p3ParticleDirection.x = _pp.p3Direction.x * DIV10;
	p3ParticleDirection.y = _pp.p3Direction.y * DIV10;
	p3ParticleDirection.z = _pp.p3Direction.z * DIV10;
	fParticleAngle = _pp.fAngle;
	fParticleSpeed = _pp.fSpeed;
	fParticleSpeedRandom = _pp.fSpeedRandom;
	p3ParticleGravity.x = _pp.p3Gravity.x;
	p3ParticleGravity.y = _pp.p3Gravity.y;
	p3ParticleGravity.z = _pp.p3Gravity.z;

	fParticleFlash = _pp.fFlash * DIV100;

	if (_pp.fRotation >= 2)
	{
		fParticleRotation = 1.0f / (101 - _pp.fRotation);
	}
	else
	{
		fParticleRotation = 0.0f;
	}

	bParticleRotationRandomDirection = _pp.bRotationRandomDirection;
	bParticleRotationRandomStart = _pp.bRotationRandomStart;

	fParticleStartSize = _pp.fStartSize;
	fParticleStartSizeRandom = _pp.fStartSizeRandom;

	for (int i = 0; i < 4; i++)
	{
		fParticleStartColor[i] = _pp.fStartColor[i] / 255.0f;
		fParticleStartColorRandom[i] = _pp.fStartColorRandom[i] / 255.0f;
	}

	bParticleStartColorRandomLock = _pp.bStartLock;

	fParticleEndSize = _pp.fEndSize;
	fParticleEndSizeRandom = _pp.fEndSizeRandom;

	for (int i = 0; i < 4; i++)
	{
		fParticleEndColor[i] = _pp.fEndColor[i] / 255.0f;
		fParticleEndColorRandom[i] = _pp.fEndColorRandom[i] / 255.0f;
	}

	bParticleEndColorRandomLock = _pp.bEndLock;

	TRUEVector_Normalize(&p3ParticleDirection);
	EERIE_3D eVect;
	eVect.x = p3ParticleDirection.x;
	eVect.y = -p3ParticleDirection.y;
	eVect.z = p3ParticleDirection.z;
	GenerateMatrixUsingVector(&eMat, &eVect, 0);

	float r = (fParticleStartColor[0]  + fParticleEndColor[0] ) * 0.5f;
	float g = (fParticleStartColor[1]  + fParticleEndColor[1] ) * 0.5f;
	float b = (fParticleStartColor[2]  + fParticleEndColor[2] ) * 0.5f;
	SetColor(r, g, b);

	switch (_pp.iBlendMode)
	{
		case 0:
			iSrcBlend = D3DBLEND_ONE;
			iDstBlend = D3DBLEND_ONE;
			break;
		case 1:
			iSrcBlend = D3DBLEND_ZERO;
			iDstBlend = D3DBLEND_INVSRCCOLOR;
			break;
		case 2:
			iSrcBlend = D3DBLEND_SRCCOLOR;
			iDstBlend = D3DBLEND_DESTCOLOR;
			break;
		case 3:
			iSrcBlend = D3DBLEND_SRCALPHA;
			iDstBlend = D3DBLEND_ONE;
			break;
		case 4:
			iSrcBlend = 0;
			iDstBlend = 0;
			break;
		case 5:
			iSrcBlend = D3DBLEND_INVDESTCOLOR;
			iDstBlend = D3DBLEND_ONE;
			break;
		default:
			iSrcBlend = D3DBLEND_ONE;
			iDstBlend = D3DBLEND_ONE;
			break;

	}

	if (_pp.bTexInfo)
	{
		SetTexture(_pp.lpszTexName, _pp.iTexNb, _pp.iTexTime, _pp.bTexLoop);
	}
}

//-----------------------------------------------------------------------------
void CParticleSystem::SetTexture(char * _pszTex, int _iNbTex, int _iTime, bool _bLoop)
{
	if (_iNbTex == 0)
	{
		tex_tab[0] = MakeTCFromFile(_pszTex);
		iNbTex = 0;
	}
	else
	{
		_iNbTex = min(_iNbTex, 20);
		char cBuf[256];

		for (int i = 0; i < _iNbTex; i++)
		{
			ZeroMemory(cBuf, 256);
			sprintf(cBuf, "%s_%04d.bmp", _pszTex, i + 1);
			tex_tab[i] = MakeTCFromFile(cBuf);
		}

		iNbTex = _iNbTex;
		iTexTime = _iTime;
		bTexLoop = _bLoop;
	}
}

//-----------------------------------------------------------------------------
void CParticleSystem::SpawnParticle(CParticle * pP)
{
	pP->p3Pos.x = 0;
	pP->p3Pos.y = 0;
	pP->p3Pos.z = 0;

	// CIRCULAR BORDER
	if (((ulParticleSpawn & PARTICLE_CIRCULAR) == PARTICLE_CIRCULAR) && ((ulParticleSpawn & PARTICLE_BORDER) == PARTICLE_BORDER))
	{
		float randd = rnd() * 360.f;
		pP->p3Pos.x = EEsin(randd) * p3ParticlePos.x;
		pP->p3Pos.y = rnd() * p3ParticlePos.y;
		pP->p3Pos.z = EEcos(randd) * p3ParticlePos.z;
	}
	// CIRCULAR
	else if ((ulParticleSpawn & PARTICLE_CIRCULAR) == PARTICLE_CIRCULAR)
	{
		float randd = rnd() * 360.f;
		pP->p3Pos.x = EEsin(randd) * rnd() * p3ParticlePos.x;
		pP->p3Pos.y = rnd() * p3ParticlePos.y;
		pP->p3Pos.z = EEcos(randd) * rnd() * p3ParticlePos.z;
	}
	else
	{
		pP->p3Pos.x = frand2() * p3ParticlePos.x;
		pP->p3Pos.y = frand2() * p3ParticlePos.y;
		pP->p3Pos.z = frand2() * p3ParticlePos.z;
	}

	if (bParticleFollow == false)
	{
		pP->p3Pos.x = p3Pos.x;
		pP->p3Pos.y = p3Pos.y;
		pP->p3Pos.z = p3Pos.z;
	}
}

//-----------------------------------------------------------------------------

void VectorRotateY(EERIE_3D & _eIn, EERIE_3D & _eOut, float _fAngle)
{
	register float c = EEcos(_fAngle);
	register float s = EEsin(_fAngle);
	_eOut.x = (_eIn.x * c) + (_eIn.z * s);
	_eOut.y =  _eIn.y;
	_eOut.z = (_eIn.z * c) - (_eIn.x * s);
}

//-----------------------------------------------------------------------------
void VectorRotateZ(EERIE_3D & _eIn, EERIE_3D & _eOut, float _fAngle)
{
	register float c = EEcos(_fAngle);
	register float s = EEsin(_fAngle);
	_eOut.x = (_eIn.x * c) + (_eIn.y * s);
	_eOut.y = (_eIn.y * c) - (_eIn.x * s);
	_eOut.z =  _eIn.z;
}

//-----------------------------------------------------------------------------
void CParticleSystem::SetParticleParams(CParticle * pP)
{
	SpawnParticle(pP);

	float	fTTL	=	fParticleLife + rnd() * fParticleLifeRandom;
	ARX_CHECK_LONG(fTTL);
	pP->ulTTL		=	ARX_CLEAN_WARN_CAST_LONG(fTTL);
	pP->fOneOnTTL = 1.0f / (float)pP->ulTTL;

	float fAngleX = rnd() * fParticleAngle; //*0.5f;
 
	EERIE_3D vv1, vvz;
	vv1.x = p3ParticleDirection.x;
	vv1.y = p3ParticleDirection.y;
	vv1.z = p3ParticleDirection.z;

	// ici modifs ----------------------------------

	vv1.x = 0;
	vv1.y = -1;
	vv1.z = 0;

	VectorRotateZ(vv1, vvz, fAngleX); 
	VectorRotateY(vvz, vv1, DEG2RAD(rnd() * 360.0f));
	VectorMatrixMultiply(&vvz, &vv1, &eMat);

	float fSpeed = fParticleSpeed + rnd() * fParticleSpeedRandom;

	pP->p3Velocity.x = vvz.x * fSpeed;
	pP->p3Velocity.y = vvz.y * fSpeed;
	pP->p3Velocity.z = vvz.z * fSpeed;
	pP->fSizeStart = fParticleStartSize + rnd() * fParticleStartSizeRandom;

	if (bParticleStartColorRandomLock)
	{
		float t = rnd() * fParticleStartColorRandom[0];
		pP->fColorStart[0] = fParticleStartColor[0] + t;
		pP->fColorStart[1] = fParticleStartColor[1] + t;
		pP->fColorStart[2] = fParticleStartColor[2] + t;
	}
	else
	{
		pP->fColorStart[0] = fParticleStartColor[0] + rnd() * fParticleStartColorRandom[0];
		pP->fColorStart[1] = fParticleStartColor[1] + rnd() * fParticleStartColorRandom[1];
		pP->fColorStart[2] = fParticleStartColor[2] + rnd() * fParticleStartColorRandom[2];
	}

	pP->fColorStart[3] = fParticleStartColor[3] + rnd() * fParticleStartColorRandom[3];

	pP->fSizeEnd = fParticleEndSize + rnd() * fParticleEndSizeRandom;

	if (bParticleEndColorRandomLock)
	{
		float t = rnd() * fParticleEndColorRandom[0];
		pP->fColorEnd[0] = fParticleEndColor[0] + t;
		pP->fColorEnd[1] = fParticleEndColor[1] + t;
		pP->fColorEnd[2] = fParticleEndColor[2] + t;
	}
	else
	{
		pP->fColorEnd[0] = fParticleEndColor[0] + rnd() * fParticleEndColorRandom[0];
		pP->fColorEnd[1] = fParticleEndColor[1] + rnd() * fParticleEndColorRandom[1];
		pP->fColorEnd[2] = fParticleEndColor[2] + rnd() * fParticleEndColorRandom[2];
	}

	pP->fColorEnd[3] = fParticleEndColor[3] + rnd() * fParticleEndColorRandom[3];

	if (bParticleRotationRandomDirection)
	{


		float fRandom	= frand2() ;
		ARX_CHECK_INT(fRandom);

		pP->iRot = ARX_CLEAN_WARN_CAST_INT(fRandom);


		if (pP->iRot < 0)
			pP->iRot = -1;

		if (pP->iRot >= 0)
			pP->iRot = 1;
	}
	else
	{
		pP->iRot = 1;
	}

	if (bParticleRotationRandomStart)
	{
		pP->fRotStart = rnd() * 360.0f;
	}
	else
	{
		pP->fRotStart = 0;
	}
}

//-----------------------------------------------------------------------------
bool CParticleSystem::IsAlive()
{
	if ((iParticleNbAlive == 0) && (iParticleNbMax == 0))
		return false;

	return true;
}

//-----------------------------------------------------------------------------
void CParticleSystem::Update(long _lTime)
{
	if (ARXPausedTimer) return;

	ulTime += _lTime;
	int nbtotal = 0;
	int iNb;
	float fTimeSec = _lTime * DIV1000;
	CParticle * pP;

	list<CParticle *>::iterator i;
	iParticleNbAlive = 0;

	i = listParticle.begin();

	while (i != listParticle.end())
	{
		pP = *i;
		++i;
		nbtotal++;

		if (pP->isAlive())
		{
			pP->Update(_lTime);
			pP->p3Velocity.x += p3ParticleGravity.x * fTimeSec;
			pP->p3Velocity.y += p3ParticleGravity.y * fTimeSec;
			pP->p3Velocity.z += p3ParticleGravity.z * fTimeSec;
			iParticleNbAlive ++;
		}
		else
		{
			if (iParticleNbAlive >= iParticleNbMax)
			{
				delete pP;
				listParticle.remove(pP);
			}
			else
			{
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
	if (iParticleNbAlive < iParticleNbMax)
	{
		long t = iParticleNbMax - iParticleNbAlive;

		if (fParticleFreq != -1)
		{

			ARX_CHECK_LONG(fTimeSec * fParticleFreq);
			t = min(ARX_CLEAN_WARN_CAST_LONG(fTimeSec * fParticleFreq), t);


			if (t < 1) t = 1;
		}

		for (iNb = 0; iNb < t; iNb++)
		{
			CParticle * pP  = new CParticle();
			SetParticleParams(pP);
			pP->Validate();
			pP->Update(0);
			listParticle.insert(listParticle.end(), pP);
			ulNbParticleGen ++;
			iParticleNbAlive++;
		}
	}
}

//-----------------------------------------------------------------------------
void CParticleSystem::Render(LPDIRECT3DDEVICE7 _lpD3DDevice, int _iSRCBLEND, int _iDESTBLEND)
{
	SETCULL(_lpD3DDevice, D3DCULL_NONE);
	SETZWRITE(_lpD3DDevice, FALSE);

	_lpD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  iSrcBlend);
	_lpD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, iDstBlend);
	SETALPHABLEND(_lpD3DDevice, TRUE);

	int inumtex = 0;

	list<CParticle *>::iterator i;

	for (i = listParticle.begin(); i != listParticle.end(); ++i)
	{
		CParticle * p = *i;

		if (p->isAlive())
		{
			if (fParticleFlash > 0)
			{
				if (rnd() < fParticleFlash)
					continue;
			}

			if (iNbTex > 0)
			{
				inumtex = p->iTexNum;

				if (iTexTime == 0)
				{

					float fNbTex	= (p->ulTime * p->fOneOnTTL) * (iNbTex);
					ARX_CHECK_INT(fNbTex);

					inumtex = ARX_CLEAN_WARN_CAST_INT(fNbTex);


					if (inumtex >= iNbTex)
					{
						inumtex = iNbTex - 1;
					}
				}
				else
				{
					if (p->iTexTime > iTexTime)
					{
						p->iTexTime -= iTexTime;
						p->iTexNum++;

						if (p->iTexNum > iNbTex - 1)
						{
							if (bTexLoop)
							{
								p->iTexNum = 0;
							}
							else
							{
								p->iTexNum = iNbTex - 1;
							}
						}

						inumtex = p->iTexNum;
					}
				}
			}

			D3DTLVERTEX p3pos;
			p3pos.sx = p->p3Pos.x;
			p3pos.sy = p->p3Pos.y;
			p3pos.sz = p->p3Pos.z;

			if (bParticleFollow)
			{
				p3pos.sx += p3Pos.x;
				p3pos.sy += p3Pos.y;
				p3pos.sz += p3Pos.z;
			}

			float fRot = 0;

			if (fParticleRotation != 0)
			{
				if (p->iRot == 1)
					fRot = (fParticleRotation) * p->ulTime + p->fRotStart;
				else
					fRot = (-fParticleRotation) * p->ulTime + p->fRotStart;

				if ((tex_tab[inumtex] && tex_tab[inumtex]->m_pddsSurface))
					EERIEDrawRotatedSprite(_lpD3DDevice, &p3pos, p->fSize, tex_tab[inumtex], p->ulColor, 2, fRot);
			}
			else
			{
				if ((tex_tab[inumtex] && tex_tab[inumtex]->m_pddsSurface))
					EERIEDrawSprite(_lpD3DDevice, &p3pos, p->fSize, tex_tab[inumtex], p->ulColor, 2);
			}
		}
	}
}

