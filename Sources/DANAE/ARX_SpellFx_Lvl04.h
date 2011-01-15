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
// CSpellFx_Lvl04.h
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
// ARX Spells FX Level 04
//
// Updates: (date) (person) (update)
//////////////////////////////////////////////////////////////////////////////////////
// Refer to CSpellFx.h for details
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#ifndef ARX_CSPELLFX_LVL04_H
#define ARX_CSPELLFX_LVL04_H

#include "ARX_CParticles.h"

class CParticleSystem;
class CSpellFx;

//-----------------------------------------------------------------------------
// Done By : Didier Pédreno
// Status  :
//-----------------------------------------------------------------------------
class CBless: public CSpellFx
{
	public:
		bool bDone;
		int iNumber;
		EERIE_3D eSrc;
		EERIE_3D eTarget;
		TextureContainer * tex_p1;
		TextureContainer * tex_sol;
		float fRot;
		float fRotPerMSec;

		int iNbPS;
		CParticleSystem psTab[256];
		int iNpcTab[256];

		int iMax;
		float fSize;

		CParticleSystem		pPS;

	public:
		CBless(LPDIRECT3DDEVICE7 m_pd3dDevice);

		// accesseurs
	public:
		void SetPos(EERIE_3D);
		void Set_Angle(EERIE_3D);
		// surcharge
	public:
		void	Create(EERIE_3D, float afBeta = 0);
		void	Update(unsigned long);
		float	Render(LPDIRECT3DDEVICE7);
		void	Kill();
};

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Done By : Didier Pédreno
// Status  :
//-----------------------------------------------------------------------------
class CDispellField: public CSpellFx
{
	public:
		bool bDone;
		int iNumber;
		EERIE_3D eSrc;
		EERIE_3D eTarget;
		TextureContainer * tex_p1;
		TextureContainer * tex_p2;
		int iMax;
		float fSize;

	public:
		// accesseurs
	public:
		void SetPos(EERIE_3D);

		// surcharge
	public:
		void	Create(EERIE_3D, float afBeta = 0);
		void	Kill();
		void	Update(unsigned long);
		float	Render(LPDIRECT3DDEVICE7);
};

//-----------------------------------------------------------------------------
class CFireProtection: public CSpellFx
{
	private:
		int			iNpc;

	public:
		CFireProtection();
		~CFireProtection();

		// surcharge
		void	Create(long, int);
		void	Update(unsigned long);
		float	Render(LPDIRECT3DDEVICE7);
 
};
//-----------------------------------------------------------------------------
class CColdProtection: public CSpellFx
{
	private:
		int			iNpc;

	public:
		CColdProtection();
		~CColdProtection();

		// surcharge
		void	Create(long, int);
		void	Update(unsigned long);
		float	Render(LPDIRECT3DDEVICE7);
 
};

//-----------------------------------------------------------------------------
// Done By :
// Status  :
//-----------------------------------------------------------------------------
class CTelekinesis: public CSpellFx
{
	public:
		bool bDone;
		int iNumber;
		EERIE_3D eSrc;
		EERIE_3D eTarget;
		TextureContainer * tex_p1;
		TextureContainer * tex_p2;

		int iMax;
		float fSize;

	public:
		~CTelekinesis();

		// accesseurs
	public:
		void SetPos(EERIE_3D);

		// surcharge
	public:
		void	Create(EERIE_3D, float afBeta = 0);
		void	Kill();
		void	Update(unsigned long);
		float	Render(LPDIRECT3DDEVICE7);
};
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Done By :
// Status  :
//-----------------------------------------------------------------------------
class CCurse: public CSpellFx
{
	public:
		bool bDone;
		int iNumber;
		EERIE_3D eSrc;
		EERIE_3D eTarget;
		TextureContainer * tex_p1;
		TextureContainer * tex_p2;
		float fRot;
		float fRotPerMSec;

	public:
		CCurse(LPDIRECT3DDEVICE7 m_pd3dDevice);
		~CCurse();
		// accesseurs
	public:
		void SetPos(EERIE_3D);

		// surcharge
	public:
		void	Create(EERIE_3D, float afBeta = 0);
		void	Kill();
		void	Update(unsigned long);
		float	Render(LPDIRECT3DDEVICE7, EERIE_3D * pos);
};
//-----------------------------------------------------------------------------


#endif
