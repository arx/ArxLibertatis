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

#ifndef ARX_GRAPHICS_SPELLS_SPELLS07_H
#define ARX_GRAPHICS_SPELLS_SPELLS07_H

#include "game/Entity.h"
#include "graphics/Vertex.h"
#include "graphics/data/Mesh.h"
#include "graphics/effects/SpellEffects.h"
#include "graphics/particle/ParticleSystem.h"
#include "math/Types.h"
#include "math/Vector.h"

// Done By : Didier Pedreno
struct CLightningNode {
	Vec3f pos;
	float size;
	int parent;
	Vec3f f;
};

class CLightning: public CSpellFx
{
	private:
		int		nbtotal;
		long	lNbSegments;
		float	fColor1[3];
		float	fColor2[3];
		float	invNbSegments;
		float	fSize;
		float	fLengthMin;
		float	fLengthMax;
		float	fAngleXMin;
		float	fAngleXMax;
		float	fAngleYMin;
		float	fAngleYMax;
		float	fAngleZMin;
		float	fAngleZMax;
		Vec3f eSrc;
		Vec3f eDest;
		float	fbeta;
		TextureContainer * tex_light;
		int iTTL;

		CLightningNode	cnodetab[2000];

	private:
		struct LIGHTNING;
		void BuildS(LIGHTNING * lightingInfo);
		void ReCreate();

	public:
		float	falpha;
		float	fDamage;
		CLightning();

		// accesseurs
	public:
		void SetColor(long, long);
		void SetPosSrc(Vec3f);
		void SetPosDest(Vec3f);

 

		void SetColor1(float, float, float);
		void SetColor2(float, float, float);

		// surcharge
	public:
		void	Create(Vec3f, Vec3f, float beta = 0);
		void	Update(unsigned long);
		void Render();
		void	Kill();
};

// Done By : Didier Pedreno
class CConfuse: public CSpellFx
{
	public:
		bool bDone;
		int iNumber;
		Vec3f eSrc;
		Vec3f eTarget;
		EERIE_ANIM	* anim_papi;
		TextureContainer * tex_p1;
		TextureContainer * tex_p2;
		TextureContainer * tex_trail;
		TextureContainer * tex_light;
		TexturedVertex pathways[80];
		ANIM_USE au;
		int end;
		float fColor[3];
		int iElapsedTime;
		Vec3f eCurPos;

		int iMax;
		float fSize;
		float fTrail;

	public:
		CConfuse();
		~CConfuse();
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

class CFireField: public CSpellFx
{
	public:
		short		key;
		Vec3f	pos;
		float		demilargeur;
		float		interp;
		ParticleSystem pPSStream;
		ParticleSystem pPSStream1;

	public:
		CFireField();
		~CFireField();

	public:
		void	Create(float largeur, Vec3f * pos, int duration);
		void	Update(unsigned long);
		void Render();
 
};

// Done By : did
class CIceField: public CSpellFx
{
	public:
		int iNumber;
		Vec3f eSrc;
		Vec3f eTarget;
		TextureContainer * tex_p1;
		TextureContainer * tex_p2;

		int iMax;
		float fSize;
		int		 tType[50];
		Vec3f tPos[50];
		Vec3f tSize[50];
		Vec3f tSizeMax[50];

	public:
		CIceField();
		~CIceField();

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

#endif // ARX_GRAPHICS_SPELLS_SPELLS07_H
