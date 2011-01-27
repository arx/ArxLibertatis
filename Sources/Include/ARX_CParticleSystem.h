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
#ifndef ARX_CPARTICLESYSTEM_H
#define ARX_CPARTICLESYSTEM_H

#include "eerietypes.h"
#include <d3d.h>
#include <list>
#include <vector>
using namespace std;

#define PARTICLE_CIRCULAR  1
#define PARTICLE_BORDER	   2

//-----------------------------------------------------------------------------
typedef EERIE_3D Point3;
 

class CParticle;
class CParticleParams;

//-----------------------------------------------------------------------------
class CParticleSystem
{
	public:
		list<CParticle *> listParticle;

	public:
		Point3	p3Pos;

		unsigned int uMaxParticles;
		unsigned int uParticlesPerSec;

		int		iParticleNbAlive;

		TextureContainer * tex_tab[20];
		int		iNbTex;
		int		iTexTime;
		bool	bTexLoop;

		EERIEMATRIX eMat;


		unsigned long ulTime;
		unsigned long ulNbParticleGen;

		// these are used for the particles it creates
		Point3	p3ParticlePos;
		int		iParticleNbMax;
		float	fParticleFreq;

		float	fParticleFlash;
		float	fParticleRotation;
		bool	bParticleRotationRandomDirection;
		bool	bParticleRotationRandomStart;

		unsigned long ulParticleSpawn;

		Point3	p3ParticleDirection;
		Point3	p3ParticleGravity;
		float	fParticleLife;
		float	fParticleLifeRandom;
		float	fParticleAngle;
		float	fParticleSpeed;
		float	fParticleSpeedRandom;

		bool	bParticleStartColorRandomLock;
		float	fParticleStartSize;
		float	fParticleStartSizeRandom;
		float	fParticleStartColor[4];
		float	fParticleStartColorRandom[4];

		bool	bParticleEndColorRandomLock;
		float	fParticleEndSize;
		float	fParticleEndSizeRandom;
		float	fParticleEndColor[4];
		float	fParticleEndColorRandom[4];

		int		iSrcBlend;
		int		iDstBlend;

		bool	bParticleFollow;

		long	lLightId;

		// editor
		float	fMinx, fMaxx, fMiny, fMaxy;

	public:
		CParticleSystem();
		~CParticleSystem();

	private:
		void	SpawnParticle(CParticle *);
		void	SetParticleParams(CParticle *);

	public:
		void	SetParams(CParticleParams & app);
 
		void	SetTexture(char *, int, int, bool _bLoop = true);
		void	SetPos(Point3 ap3);
		void	SetColor(float, float, float);
 


	public:
		void	Render(LPDIRECT3DDEVICE7 _pD3DDevice, int _iSRCBLEND = D3DBLEND_SRCALPHA, int _iDESTBLEND = D3DBLEND_ONE/*flag post prod/genre filtre*/);
		bool	IsAlive();
		void	Update(long);
		void	RecomputeDirection();
};

#endif
