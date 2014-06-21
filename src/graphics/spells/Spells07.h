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
class CLightning : public CSpellFx {
	
public:
	CLightning();
	
	void SetColor(long, long);
	void SetPosSrc(Vec3f);
	void SetPosDest(Vec3f);

	void SetColor1(float, float, float);
	void SetColor2(float, float, float);

	void Create(Vec3f, Vec3f);
	void Update(float timeDelta);
	void Render();
	
	Vec3f m_pos;
	float m_beta;
	float m_alpha;
	EntityHandle m_caster;
	float m_level;
	
	float	fDamage;
	bool m_isMassLightning;
	
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
	TextureContainer * tex_light;
	int iTTL;
	
	struct CLightningNode {
		Vec3f pos;
		float size;
		int parent;
		Vec3f f;
	};
	
	CLightningNode	cnodetab[2000];
	
	struct LIGHTNING;
	void BuildS(LIGHTNING * lightingInfo);
	void ReCreate(float rootSize);
};

// Done By : Didier Pedreno
class CConfuse : public CSpellFx {
	
public:
	CConfuse();
	~CConfuse();
	
	void Create();
	void SetPos(const Vec3f & pos);
	
	void Update(float timeDelta);
	void Render();
	
private:
	TextureContainer * tex_p1;
	TextureContainer * tex_trail;
	ANIM_USE au;
	Vec3f eCurPos;
};

class CFireField : public CSpellFx {
	
public:
	CFireField();
	~CFireField();

	void Create(float largeur, const Vec3f & pos, int duration);
	void Update(float timeDelta);
	void Render();
	
	Vec3f pos;
	
private:
	ParticleSystem pPSStream;
	ParticleSystem pPSStream1;
};

// Done By : did
class CIceField : public CSpellFx {
	
public:
	CIceField();
	~CIceField();
	
	void Create(Vec3f);
	void Update(float timeDelta);
	void Render();
	
	Vec3f eSrc;
	
private:
	Vec3f eTarget;
	TextureContainer * tex_p1;
	TextureContainer * tex_p2;
	
	int iMax;
	int tType[50];
	Vec3f tPos[50];
	Vec3f tSize[50];
	Vec3f tSizeMax[50];
};

#endif // ARX_GRAPHICS_SPELLS_SPELLS07_H
