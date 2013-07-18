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

#ifndef ARX_GRAPHICS_SPELLS_SPELLS03_H
#define ARX_GRAPHICS_SPELLS_SPELLS03_H

#include "graphics/effects/SpellEffects.h"
#include "graphics/particle/ParticleSystem.h"
#include "graphics/particle/ParticleParams.h"

// Done By : Didier Pedreno
class CFireBall: public CSpellFx
{
public:
	CFireBall();
	~CFireBall();

	void SetTTL(unsigned long);

	void Create(Vec3f, float afBeta, float afAlpha,  float);
	void Kill();

	void Update(unsigned long);
	void Render();

	Vec3f eSrc;
	Vec3f eCurPos;
	Vec3f eMove;
	bool bExplo;
	float fLevel;

	ParticleSystem pPSFire;
	ParticleSystem pPSFire2;
	ParticleSystem pPSSmoke;

private:
	ParticleParams fire_1;
	ParticleParams fire_2;
	ParticleParams smoke;
};

class CSpeed: public CSpellFx
{
	private:
		short		key;
		int			duration;
		int			currduration;
		int			num;

		TextureContainer * tp;

		struct T_RUBAN
		{
			int				actif;
			Vec3f		pos;
			int				next;
		};
		T_RUBAN truban[2048];

		struct T_RUBAN_DEF
		{
			int		first;
			int		origin;
			float	size;
			int		dec;
			float	r, g, b;
			float	r2, g2, b2;
		};

		int			nbrubandef;
		T_RUBAN_DEF trubandef[256];

		int GetFreeRuban(void);
		void AddRuban(int * f, int id, int dec);
		void DrawRuban(int num, float size, int dec, float r, float g, float b, float r2, float g2, float b2);
	public:

		unsigned long GetDuration(void)
		{
			return this->duration;
		}

		void	AddRubanDef(int origin, float size, int dec, float r, float g, float b, float r2, float g2, float b2);
		void	Create(int numinteractive, int duration);
		void	Update(unsigned long);
		void Render();
 
};

#define MAX_ICE 150
// Done By : did
class CIceProjectile: public CSpellFx
{
	public:
		int iNumber;
		int iMax;
		int	tType[MAX_ICE];
		float fSize;
		float fStep;
		float fColor;
		Vec3f tPos[MAX_ICE];
		Vec3f tSize[MAX_ICE];
		Vec3f tSizeMax[MAX_ICE];
		TextureContainer * tex_p1;
		TextureContainer * tex_p2;
		TexturedVertex tv1a[MAX_ICE];

	public:
		CIceProjectile();
		~CIceProjectile();

		// accesseurs
	public:
		void SetPos(Vec3f);

		// surcharge
	public:
		void	Create(Vec3f, float);
		void	Create(Vec3f, float, float);
		void	Kill();
		void	Update(unsigned long);
		void Render();
};

// Done By : did
class CCreateFood: public CSpellFx
{
	public:
		Vec3f eSrc;
		float	fSize;
		ParticleSystem * pPS;
		TextureContainer * tex_sol;
		TextureContainer * tex_heal;

	public:
		CCreateFood();
		~CCreateFood();

	public:
		void	Create();
		void	Update(unsigned long);
		void Render();
 
};

void LaunchFireballExplosion(Vec3f *, float);
void LaunchFireballExplosion2(Vec3f *, float);

#endif // ARX_GRAPHICS_SPELLS_SPELLS03_H
