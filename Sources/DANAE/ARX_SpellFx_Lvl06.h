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
// CSpellFx_Lvl06.h
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
// ARX Spells FX Level 06
//
// Updates: (date) (person) (update)
//////////////////////////////////////////////////////////////////////////////////////
// Refer to CSpellFx.h for details
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#ifndef ARX_CSPELLFX_LVL06_H
#define ARX_CSPELLFX_LVL06_H

class CSpellFx;

//-----------------------------------------------------------------------------
// Done By : seb
// Status  :
//-----------------------------------------------------------------------------
class CParalyse : public CSpellFx
{
	private:
		short	key;
		int		duration;
		int		currduration;
		EERIE_3D	pos;
		float		scale;
		float		r;

		int			colduration;
		float		prisminterpcol;
		float		prismrd, prismgd, prismbd;
		float		prismre, prismge, prismbe;

		EERIE_3D		*	prismvertex;
		D3DTLVERTEX		*	prismd3d;
		unsigned short	*	prismind;
		int					prismnbpt;
		int					prismnbface;
		TextureContainer	* tex_prism;

		TextureContainer	* tex_p, *tex_p1, *tex_p2;

		typedef struct
		{
			EERIE_3D	pos;
			EERIE_3D	offset;
			EERIE_3D	* vertex;
		} T_PRISM;

		T_PRISM		tabprism[100];

		void	CreatePrismTriangleList(float, float, float, int);
		void	CreateLittlePrismTriangleList();
	public:
		CParalyse();
		~CParalyse();

		void SetColPrismDep(float r, float g, float b)
		{
			prismrd = r;
			prismgd = g;
			prismbd = b;
		};
		void SetColPrismEnd(float r, float g, float b)
		{
			prismre = r;
			prismge = g;
			prismbe = b;
		};
		void InversePrismCol(void)
		{
			float t;
			t = prismrd;
			prismrd = prismre;
			prismre = t;
			t = prismgd;
			prismgd = prismge;
			prismge = t;
			t = prismbd;
			prismbd = prismbe;
			prismbe = t;
		};

		unsigned long GetDuration(void)
		{
			return this->duration;
		};


		void	Create(int, float, float, float, EERIE_3D *, int);
		void	Update(unsigned long);
		float	Render(LPDIRECT3DDEVICE7);
		void	Kill();
};
/*--------------------------------------------------------------------------*/


//-----------------------------------------------------------------------------
// Done By : Didier Pédreno
// Status  :
//-----------------------------------------------------------------------------
class CCreateField: public CSpellFx
{
	public:
		EERIE_3D eSrc;

	private:
		TextureContainer * tex_jelly;
		bool youp;
		float	fColor1[3];
		float	fColor2[3];
		float	fSize;
		float	fbeta;
		float	fwrap;
		float ysize;
		float size;
		float ft;
		float fglow ;
		D3DTLVERTEX b[4];
		D3DTLVERTEX t[4];

	public:
		float	falpha;
		CCreateField(LPDIRECT3DDEVICE7 m_pd3dDevice);

	private:
		void RenderQuad(LPDIRECT3DDEVICE7 m_pd3dDevice, D3DTLVERTEX p1, D3DTLVERTEX p2, D3DTLVERTEX p3, D3DTLVERTEX p4,  int rec, EERIE_3D);
		void RenderSubDivFace(LPDIRECT3DDEVICE7 m_pd3dDevice, D3DTLVERTEX * b, D3DTLVERTEX * t, int b1, int b2, int t1, int t2);

	public:
		void SetPos(EERIE_3D);
		void SetSize(float);

	public:
		void	Create(EERIE_3D, float afBeta = 0);
		void	Kill();
		void	Update(unsigned long);
		float	Render(LPDIRECT3DDEVICE7);
};

//-----------------------------------------------------------------------------
// Done By :
// Status  :
//-----------------------------------------------------------------------------
class CDisarmTrap: public CSpellFx
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
		CDisarmTrap(LPDIRECT3DDEVICE7 m_pd3dDevice);
		~CDisarmTrap();

	public:
		void SetPos(EERIE_3D);

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
class CSlowDown: public CSpellFx
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
		CSlowDown(LPDIRECT3DDEVICE7 m_pd3dDevice);
		~CSlowDown();

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
// Done By : Didier Pédreno
// Status  :
//-----------------------------------------------------------------------------
class CRiseDead: public CSpellFx
{
	public:
		EERIE_3D eSrc;
		float	fColorRays1[3];

	private:
		TextureContainer * tex_light;
		int		end;
		int		iSize;
		bool	bIntro;
		float	fOneOniSize;
		float	fOneOnDurationIntro;
		float	fOneOnDurationRender;
		float	fOneOnDurationOuttro;
		float	sizeF;
		float	fSizeIntro;
		float	fRand;
		float	fTexWrap;
		float	fColorBorder[3];
		float	fColorRays2[3];
		float	tfRaysa[40];
		float	tfRaysb[40];
		unsigned long ulDurationIntro;
		unsigned long ulDurationRender;
		unsigned long ulDurationOuttro;
		D3DTLVERTEX va[40];
		D3DTLVERTEX vb[40];
		D3DTLVERTEX v1a[40];
		D3DTLVERTEX v1b[40];

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

		int				currframetime;
		int				timestone;
		int				nbstone;
		T_STONE			tstone[256];

		void AddStone(EERIE_3D * pos);
		void DrawStone(LPDIRECT3DDEVICE7 device);
	public:
		CRiseDead(LPDIRECT3DDEVICE7 m_pd3dDevice);
		~CRiseDead();

	private:
		void Split(D3DTLVERTEX * v, int a, int b, float yo);
		void RenderFissure(LPDIRECT3DDEVICE7 m_pd3dDevice);

		// accesseurs
	public:
		void SetDuration(const unsigned long duration);
		void SetDuration(unsigned long, unsigned long, unsigned long);
 
		void SetPos(EERIE_3D);
 
		void SetColorBorder(float, float, float);
		void SetColorRays1(float, float, float);
		void SetColorRays2(float, float, float);
		unsigned long GetDuration();

		// surcharge
	public:
		void	Create(EERIE_3D, float afBeta = 0);
		void	Kill();
		void	Update(unsigned long);
		float	Render(LPDIRECT3DDEVICE7);
};
//-----------------------------------------------------------------------------


#endif
