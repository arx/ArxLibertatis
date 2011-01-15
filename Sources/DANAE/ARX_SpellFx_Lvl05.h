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
// CSpellFx_Lvl05.h
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
// ARX Spells FX Level 05
//
// Updates: (date) (person) (update)
//////////////////////////////////////////////////////////////////////////////////////
// Refer to CSpellFx.h for details
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#ifndef ARX_CSPELLFX_LVL05_H
#define ARX_CSPELLFX_LVL05_H

#include "ARX_CParticles.h"

class CParticleSystem;
class CSpellFx;

// Cyril = Global Resources
extern EERIE_3DOBJ * ssol;
extern long ssol_count;
extern EERIE_3DOBJ * slight;
extern long slight_count;
extern EERIE_3DOBJ * srune;
extern long srune_count;
extern EERIE_3DOBJ * smotte;
extern long smotte_count;
extern EERIE_3DOBJ * stone1;
extern long stone1_count;
extern EERIE_3DOBJ * stone0;
extern long stone0_count;
extern EERIE_3DOBJ * stite;
extern long stite_count;
extern EERIE_3DOBJ * smissile;
extern long smissile_count;
extern EERIE_3DOBJ * spapi;
extern long spapi_count;
extern EERIE_3DOBJ * sfirewave;
extern long sfirewave_count;
extern EERIE_3DOBJ * svoodoo;
extern long svoodoo_count;
//-----------------------------------------------------------------------------
// Done By : did
// Status  :
//-----------------------------------------------------------------------------
class CRuneOfGuarding: public CSpellFx
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
		CRuneOfGuarding(LPDIRECT3DDEVICE7 m_pd3dDevice);
		~CRuneOfGuarding();
		// accesseurs
	public:
		void	SetPos(EERIE_3D);

		// surcharge
	public:
		void	Create(EERIE_3D, float afBeta = 0);
		void	Kill();
		void	Update(unsigned long);
		float	Render(LPDIRECT3DDEVICE7);
};
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// LEVITATION
// Done By : Sébastien Scieux
// Status  :
//-----------------------------------------------------------------------------
class CLevitate: public CSpellFx
{
	private:
		short		key;
		short		def;
		EERIE_3D	pos;
		float		rbase, rhaut, hauteur, scale;
		float		ang;
		int			currdurationang;
		int			currframetime;
		TextureContainer * tsouffle;

		typedef struct
		{
			int				conenbvertex;
			int				conenbfaces;
			EERIE_3D	*	conevertex;
			D3DTLVERTEX	*	coned3d;
			unsigned short	* coneind;
		} T_CONE;

		T_CONE		cone[2];

		EERIE_3DOBJ	*	stone[2];

		typedef struct
		{
			short		actif;
			short		numstone;
			EERIE_3D	pos;
			float		yvel;
			EERIE_3D	ang;
			EERIE_3D	angvel;
			EERIE_3D	scale;
			int			time;
			int			currtime;
		} T_STONE;

		int				timestone;
		int				nbstone;
		T_STONE			tstone[256];

		void AddStone(EERIE_3D * pos);
		void DrawStone(LPDIRECT3DDEVICE7 device);

		void CreateConeStrip(float rout, float rhaut, float hauteur, int def, int numcone);
	public:
		CLevitate();
		~CLevitate();

		void ChangePos(EERIE_3D * pos)
		{
			this->pos = *pos;
		};

		void	Create(int def, float rout, float rhaut, float hauteur, EERIE_3D * pos, unsigned long);
		void	Update(unsigned long);
		float	Render(LPDIRECT3DDEVICE7);
 
};
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Done By : Didier Pédreno
// Status  :
//-----------------------------------------------------------------------------
class CCurePoison: public CSpellFx
{
	public:
		EERIE_3D eSrc;
		float	fSize;
		CParticleSystem * pPS;
		TextureContainer * tex_sol;
		TextureContainer * tex_heal;

	public:
		CCurePoison(LPDIRECT3DDEVICE7 m_pd3dDevice);
		~CCurePoison();

	public:
		void	Create();
		void	Update(unsigned long);
		float	Render(LPDIRECT3DDEVICE7);
 
};

//-----------------------------------------------------------------------------
// Done By : Didier Pédreno
// Status  :
//-----------------------------------------------------------------------------
class CPoisonProjectile: public CSpellFx
{
	public:
		int end;
		int iMax;
		float fSize;
		float fTrail;
		float fColor[3];
		float fColor1[3];
		bool  bOk;
		bool bDone;


		EERIE_3D eSrc;
		EERIE_3D eTarget;
		EERIE_3D eCurPos;
		EERIE_3D eMove;
		D3DTLVERTEX pathways[40];
		CParticleSystem pPS;
		CParticleSystem pPSStream;

	public:
		CPoisonProjectile(LPDIRECT3DDEVICE7 m_pd3dDevice);

		// accesseurs
	public:
		void	SetPos(EERIE_3D);
		void	SetColor(float, float, float);
		void	SetColor1(float, float, float);

		// surcharge
	public:
		void	Create(EERIE_3D, float afBeta = 0);
		void	Kill();
		void	Update(unsigned long);
		float	Render(LPDIRECT3DDEVICE7);
};

//-----------------------------------------------------------------------------
class CMultiPoisonProjectile: public CSpellFx
{
	public:
		unsigned int uiNumber;

	private:
		CSpellFx ** pTab;

	public:
		CMultiPoisonProjectile(LPDIRECT3DDEVICE7 m_pd3dDevice, long nb);
		~CMultiPoisonProjectile();

		// surcharge
	public:
		void	Create(EERIE_3D, float);
		void	Kill();
		void	Update(unsigned long);
		float	Render(LPDIRECT3DDEVICE7);
};

//-----------------------------------------------------------------------------
// Done By : did
// Status  :
//-----------------------------------------------------------------------------
class CRepelUndead: public CSpellFx
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
		CRepelUndead(LPDIRECT3DDEVICE7 m_pd3dDevice);
		~CRepelUndead();

	public:
		void SetPos(EERIE_3D);

	public:
		void	Create(EERIE_3D, float afBeta = 0);
		void	Kill();
		void	Update(unsigned long);
		float	Render(LPDIRECT3DDEVICE7);
};
//-----------------------------------------------------------------------------

#endif
