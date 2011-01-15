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
// CSpellFx_Lvl07.h
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
// ARX Spells FX Level 07
//
// Updates: (date) (person) (update)
//////////////////////////////////////////////////////////////////////////////////////
// Refer to CSpellFx.h for details
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#ifndef ARX_CSPELLFX_LVL07_H
#define ARX_CSPELLFX_LVL07_H

#include "ARX_CParticles.h"
#include <EERIETypes.h>
#include <EERIEPoly.h>

class CParticleSystem;

class CSpellFx;

//-----------------------------------------------------------------------------
// Done By :
// Status  :
//-----------------------------------------------------------------------------
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

//-----------------------------------------------------------------------------
// Done By : Didier Pédreno
// Status  :
//-----------------------------------------------------------------------------
class CLightningNode
{
	public:
		float x, y, z; // 3*4 = 12
		float size;
		//short nbfils;  // 2
		int parent;
		//list<CNode> *fils;
		float fx, fy, fz;
};

//-----------------------------------------------------------------------------
typedef struct
{
	EERIE_3D eStart;
	EERIE_3D eEnd;
	EERIE_3D eVect;
	//	float ax;
	//	float ay;
	//	float az;
	int anb;
	int anbrec;
	bool abFollow;
	int aParent;
	float fSize;
	float fAngleXMin;
	float fAngleXMax;
	float fAngleYMin;
	float fAngleYMax;
	float fAngleZMin;
	float fAngleZMax;
} LIGHTNING;

//-----------------------------------------------------------------------------
class CLightning: public CSpellFx
{
	private:
		int		nbtotal;
		// tests du nombre de segments
		//	int		nbmin;
		//	int		nbmax;
		long	lNbSegments;
		float	fColor1[3];
		float	fColor2[3];
		float	invNbSegments;
		float	fSize;
		float	fSizeMin;
		float	fSizeMax;
		float	fLengthMin;
		float	fLengthMax;
		float	fAngleXMin;
		float	fAngleXMax;
		float	fAngleYMin;
		float	fAngleYMax;
		float	fAngleZMin;
		float	fAngleZMax;
		EERIE_3D eSrc;
		EERIE_3D eDest;
		float	fbeta;
		TextureContainer * tex_light;
		int iTTL;

		//LIGHTNING lInfo;

		CLightningNode	cnodetab[2000];
		// long lightsidx[10]

	private:
		void BuildS(LIGHTNING *);
		void ReCreate();

	public:
		float	falpha;
		float	fDamage;
		//CLightning();
		CLightning(TextureContainer * aTC = NULL);

		//CLightning(LPDIRECT3DDEVICE7);
		//~CLightning();

		// accesseurs
	public:
		void SetColor(long, long);
		void SetPosSrc(EERIE_3D);
		void SetPosDest(EERIE_3D);

 

		void SetColor1(float, float, float);
		void SetColor2(float, float, float);

 
 

 
 
 
 
 
 

		// surcharge
	public:
		void	Create(EERIE_3D, EERIE_3D, float beta = 0);
		void	Update(unsigned long);
		float	Render(LPDIRECT3DDEVICE7);
		void	Kill();
};

//-----------------------------------------------------------------------------
// Ex Make Friend
// Done By : Didier Pédreno
// Status  :
//-----------------------------------------------------------------------------
class CConfuse: public CSpellFx
{
	public:
		bool bDone;
		int iNumber;
		EERIE_3D eSrc;
		EERIE_3D eTarget;
		EERIE_ANIM	* anim_papi;
		TextureContainer * tex_p1;
		TextureContainer * tex_p2;
		TextureContainer * tex_trail;
		TextureContainer * tex_light;
		D3DTLVERTEX pathways[80];
		ANIM_USE au;
		int end;
		float fColor[3];
		int iElapsedTime;
		EERIE_3D eCurPos;

		int iMax;
		float fSize;
		float fTrail;

	public:
		CConfuse(LPDIRECT3DDEVICE7 m_pd3dDevice);
		~CConfuse();
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
class CFireField: public CSpellFx
{
	public:
		short		key;
		EERIE_3D	pos;
		float		demilargeur;
		float		interp;
		//	TextureContainer *tp,*tp2;
		CParticleSystem pPSStream;
		CParticleSystem pPSStream1;

	public:
		CFireField();
		~CFireField();

	public:
		void	Create(float largeur, EERIE_3D * pos, int duration);
		void	Update(unsigned long);
		float	Render(LPDIRECT3DDEVICE7);
 
};

//-----------------------------------------------------------------------------
// Done By : did
// Status  :
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CIceField: public CSpellFx
{
	public:
		int iNumber;
		EERIE_3D eSrc;
		EERIE_3D eTarget;
		//	EERIE_3DOBJ * stite;
		//	EERIE_3DOBJ * smotte;
		TextureContainer * tex_p1;
		TextureContainer * tex_p2;

		int iMax;
		float fSize;
		int		 tType[50];
		EERIE_3D tPos[50];
		EERIE_3D tSize[50];
		EERIE_3D tSizeMax[50];

	public:
		CIceField(LPDIRECT3DDEVICE7 m_pd3dDevice);
		~CIceField();

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

#endif

