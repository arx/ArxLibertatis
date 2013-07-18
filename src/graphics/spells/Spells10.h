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

#ifndef ARX_GRAPHICS_SPELLS_SPELLS10_H
#define ARX_GRAPHICS_SPELLS_SPELLS10_H

#include "game/Entity.h"
#include "graphics/data/Mesh.h"
#include "graphics/effects/SpellEffects.h"

class CLightning;
class CIncinerate;

// Done By : Didier Pedreno
class CMassLightning: public CSpellFx
{
	public:
		long number;

	private:
		CLightning ** pTab;

	public:
		explicit CMassLightning(long nb);
		~CMassLightning() { }

	public:
		void	Create(Vec3f, float);
		void	Update(unsigned long);
		void Render();
 
};

// Done By : did
class CControlTarget: public CSpellFx
{
	public:
		bool bDone;
		int iNumber;
		Vec3f eSrc;
		Vec3f eTarget;
		TextureContainer * tex_mm;
		TexturedVertex v1a[40];
		TexturedVertex pathways[40];
		ANIM_USE au;
		int end;
		float fColor[3];
		float fColor1[3];
		Vec3f eCurPos;

		int iMax;
		float fSize;
		float fTrail;

	public:
		CControlTarget();

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

// Done By : did
class CMassIncinerate: public CSpellFx
{
	private:
		CIncinerate ** pTabIncinerate;

	public:
 
		~CMassIncinerate();

	public:
		void	Create(Vec3f, float);
		void	Update(unsigned long);
		void Render();
 
};

#endif // ARX_GRAPHICS_SPELLS_SPELLS10_H
