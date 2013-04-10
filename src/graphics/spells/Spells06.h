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

#ifndef ARX_GRAPHICS_SPELLS_SPELLS06_H
#define ARX_GRAPHICS_SPELLS_SPELLS06_H

#include "graphics/effects/SpellEffects.h"

// Done By : seb
class CParalyse : public CSpellFx
{
	private:
		short	key;
		int		duration;
		int		currduration;
		Vec3f	pos;
		float		scale;
		float		r;

		int			colduration;
		float		prisminterpcol;
		float		prismrd, prismgd, prismbd;
		float		prismre, prismge, prismbe;

		Vec3f		*	prismvertex;
		TexturedVertex		*	prismd3d;
		unsigned short	*	prismind;
		int					prismnbpt;
		int					prismnbface;
		TextureContainer	* tex_prism;

		TextureContainer	* tex_p, *tex_p1, *tex_p2;

		struct T_PRISM
		{
			Vec3f	pos;
			Vec3f	offset;
			Vec3f	* vertex;
		};

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


		void	Create(int, float, float, float, Vec3f *, int);
		void	Update(unsigned long);
		void Render();
		void	Kill();
};

// Done By : Didier Pedreno
class CCreateField: public CSpellFx
{
	public:
		Vec3f eSrc;

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
		TexturedVertex b[4];
		TexturedVertex t[4];

	public:
		float	falpha;
		CCreateField();

	private:
		void RenderQuad(TexturedVertex p1, TexturedVertex p2, TexturedVertex p3, TexturedVertex p4,  int rec, Vec3f);
		void RenderSubDivFace(TexturedVertex * b, TexturedVertex * t, int b1, int b2, int t1, int t2);

	public:
		void	Create(Vec3f, float afBeta = 0);
		void	Kill();
		void	Update(unsigned long);
		void Render();
};

class CDisarmTrap: public CSpellFx
{
	public:
		bool bDone;
		int iNumber;
		Vec3f eSrc;
		Vec3f eTarget;

		TextureContainer * tex_p1;
		TextureContainer * tex_p2;

		int iMax;
		float fSize;

	public:
		CDisarmTrap();
		~CDisarmTrap();

	public:
		void SetPos(Vec3f);

	public:
		void	Create(Vec3f, float afBeta = 0);
		void	Kill();
		void	Update(unsigned long);
		void Render();
};

class CSlowDown: public CSpellFx
{
	public:
		bool bDone;
		int iNumber;
		Vec3f eSrc;
		Vec3f eTarget;
		TextureContainer * tex_p1;
		TextureContainer * tex_p2;

		int iMax;
		float fSize;

	public:
		CSlowDown();
		~CSlowDown();

		// accesseurs
	public:
		void SetPos(Vec3f);

		// surcharge
	public:
		void	Create(Vec3f, float afBeta = 0);
		void	Kill();
		void	Update(unsigned long);
		void Render();
};

// Done By : Didier Pedreno
class CRiseDead: public CSpellFx
{
	public:
		Vec3f eSrc;
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
		TexturedVertex va[40];
		TexturedVertex vb[40];
		TexturedVertex v1a[40];
		TexturedVertex v1b[40];

		EERIE_3DOBJ	*	stone[2];

		struct T_STONE
		{
			short		actif;
			short		numstone;
			Vec3f	pos;
			float		yvel;
			Anglef ang;
			Anglef angvel;
			Vec3f	scale;
			int			time;
			int			currtime;
		};

		int				currframetime;
		int				timestone;
		int				nbstone;
		T_STONE			tstone[256];

		void AddStone(Vec3f * pos);
		void DrawStone();
	public:
		CRiseDead();
		~CRiseDead();

	private:
		void Split(TexturedVertex * v, int a, int b, float yo);
		void RenderFissure();

		// accesseurs
	public:
		void SetDuration(const unsigned long duration);
		void SetDuration(unsigned long, unsigned long, unsigned long);
 
		void SetPos(Vec3f);
 
		void SetColorBorder(float, float, float);
		void SetColorRays1(float, float, float);
		void SetColorRays2(float, float, float);
		unsigned long GetDuration();

		// surcharge
	public:
		void	Create(Vec3f, float afBeta = 0);
		void	Kill();
		void	Update(unsigned long);
		void Render();
};

#endif // ARX_GRAPHICS_SPELLS_SPELLS06_H
