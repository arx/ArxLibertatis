/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GRAPHICS_SPELLS_SPELLS04_H
#define ARX_GRAPHICS_SPELLS_SPELLS04_H

#include "graphics/effects/SpellEffects.h"
#include "graphics/particle/ParticleSystem.h"

// Done By : Didier Pedreno
class CBless: public CSpellFx
{
	public:
		bool bDone;
		int iNumber;
		Vec3f eSrc;
		Vec3f eTarget;
		TextureContainer * tex_p1;
		TextureContainer * tex_sol;
		float fRot;
		float fRotPerMSec;

		int iNbPS;
		ParticleSystem psTab[256];
		int iNpcTab[256];

		int iMax;
		float fSize;

		ParticleSystem		pPS;

	public:
		CBless();

		// accesseurs
	public:
		void Set_Angle(const Anglef &);
		// surcharge
	public:
		void	Create(Vec3f, float afBeta = 0);
		void	Update(unsigned long);
		void Render();
		void	Kill();
};

// Done By : Didier Pedreno
class CDispellField: public CSpellFx
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

		// surcharge
	public:
		void	Create(Vec3f, float afBeta = 0);
		void	Kill();
		void	Update(unsigned long);
		void Render();
};

class CFireProtection: public CSpellFx {
	
public:
	
	CFireProtection();
	~CFireProtection();
	
	void Create(long);
	void Update(unsigned long);
	void Render();
	
};

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
		void Render();
 
};

class CTelekinesis: public CSpellFx
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
		~CTelekinesis();

		// surcharge
	public:
		void	Create(Vec3f, float afBeta = 0);
		void	Kill();
		void	Update(unsigned long);
		void Render();
};

class CCurse: public CSpellFx
{
	public:
		bool bDone;
		int iNumber;
		Vec3f eSrc;
		Vec3f eTarget;
		TextureContainer * tex_p1;
		TextureContainer * tex_p2;
		float fRot;
		float fRotPerMSec;

	public:
		CCurse();
		~CCurse();

		// surcharge
	public:
		void	Create(Vec3f, float afBeta = 0);
		void	Kill();
		void	Update(unsigned long);
		void Render();
};

#endif // ARX_GRAPHICS_SPELLS_SPELLS04_H
