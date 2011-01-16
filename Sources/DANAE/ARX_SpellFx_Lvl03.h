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
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// CSpellFx_Lvl03.h
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
// ARX Spells FX Level 03
//
// Updates: (date) (person) (update)
//////////////////////////////////////////////////////////////////////////////////////
// Refer to CSpellFx.h for details
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#ifndef ARX_CSPELLFX_LVL03_H
#define ARX_CSPELLFX_LVL03_H

#include "ARX_CParticles.h"

class CParticleSystem;
class CSpellFx;

//-----------------------------------------------------------------------------
// Done By : Didier Pédreno
// Status  :
//-----------------------------------------------------------------------------
class CFireBall: public CSpellFx
{
	public:
		EERIE_3D eSrc;
		EERIE_3D eCurPos;
		EERIE_3D eMove;
		bool bExplo;
		float fLevel;
		//private:
		CParticleSystem pPSFire;
		CParticleSystem pPSFire2;
		CParticleSystem pPSSmoke;

	public:
		CFireBall();
		~CFireBall();

		// surcharge
	public:
		void	SetTTL(unsigned long);

	public:
		void	Create(EERIE_3D, float afBeta, float afAlpha,  float);
		void	Kill();
		void	Update(unsigned long);
		float	Render(LPDIRECT3DDEVICE7);
};

//-----------------------------------------------------------------------------
// Done By :
// Status  :
//-----------------------------------------------------------------------------
class CSpeed: public CSpellFx
{
	private:
		short		key;
		int			duration;
		int			currduration;
		int			num;

		TextureContainer * tp;

		typedef struct ST_RUBAN
		{
			int				actif;
			EERIE_3D		pos;
			int				next;
		} T_RUBAN;
		T_RUBAN truban[2048];

		typedef struct
		{
			int		first;
			int		origin;
			float	size;
			int		dec;
			float	r, g, b;
			float	r2, g2, b2;
		} T_RUBAN_DEF;

		int			nbrubandef;
		T_RUBAN_DEF trubandef[256];

		int GetFreeRuban(void);
		void AddRuban(int * f, int id, int dec);
		void DrawRuban(LPDIRECT3DDEVICE7 device, int num, float size, int dec, float r, float g, float b, float r2, float g2, float b2);
	public:
		CSpeed() {};
		~CSpeed() {};

		unsigned long GetDuration(void)
		{
			return this->duration;
		}

		void	AddRubanDef(int origin, float size, int dec, float r, float g, float b, float r2, float g2, float b2);
		void	Create(int numinteractive, int duration);
		void	Update(unsigned long);
		float	Render(LPDIRECT3DDEVICE7);
 
};

//-----------------------------------------------------------------------------
// Done By : did
// Status  :
//-----------------------------------------------------------------------------
#define MAX_ICE 150
//-----------------------------------------------------------------------------
class CIceProjectile: public CSpellFx
{
	public:
		int iNumber;
		int iMax;
		int	tType[MAX_ICE];
		float fSize;
		float fStep;
		float fColor;
		EERIE_3D tPos[MAX_ICE];
		EERIE_3D tSize[MAX_ICE];
		EERIE_3D tSizeMax[MAX_ICE];
		TextureContainer * tex_p1;
		TextureContainer * tex_p2;
		D3DTLVERTEX tv1a[MAX_ICE];

	public:
		CIceProjectile(LPDIRECT3DDEVICE7 m_pd3dDevice);
		~CIceProjectile();

		// accesseurs
	public:
		void SetPos(EERIE_3D);

		// surcharge
	public:
		void	Create(EERIE_3D, float);
		void	Create(EERIE_3D, float, float);
		void	Kill();
		void	Update(unsigned long);
		float	Render(LPDIRECT3DDEVICE7);
};


//-----------------------------------------------------------------------------
// Done By : did
// Status  :
//-----------------------------------------------------------------------------
class CCreateFood: public CSpellFx
{
	public:
		EERIE_3D eSrc;
		float	fSize;
		CParticleSystem * pPS;
		TextureContainer * tex_sol;
		TextureContainer * tex_heal;

	public:
		CCreateFood(LPDIRECT3DDEVICE7 m_pd3dDevice);
		~CCreateFood();

	public:
		void	Create();
		void	Update(unsigned long);
		float	Render(LPDIRECT3DDEVICE7);
 
};

//-----------------------------------------------------------------------------

void LaunchFireballExplosion(EERIE_3D *, float);
void LaunchFireballExplosion2(EERIE_3D *, float);

#endif
