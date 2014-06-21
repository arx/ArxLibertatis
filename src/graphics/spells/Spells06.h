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

// Done By : Didier Pedreno
class CCreateField : public CSpellFx {
	
public:
	CCreateField();
	
	void Create(Vec3f);
	void Update(float timeDelta);
	void Render();
	
	Vec3f eSrc;
	
private:
	TextureContainer * tex_jelly;
	bool youp;
	float fColor1[3];
	float fColor2[3];
	float fwrap;
	float ysize;
	float size;
	float ft;
	float fglow ;
	TexturedVertex b[4];
	TexturedVertex t[4];
	
	float falpha;
	
	void RenderQuad(TexturedVertex p1, TexturedVertex p2, TexturedVertex p3, TexturedVertex p4,  int rec, Vec3f);
	void RenderSubDivFace(TexturedVertex * b, TexturedVertex * t, int b1, int b2, int t1, int t2);
};

class CSlowDown : public CSpellFx {
	
public:
	CSlowDown();
	~CSlowDown();
	
	void SetPos(Vec3f);
	
	void Create(Vec3f);
	void Update(float timeDelta);
	void Render();
	
private:
	Vec3f eSrc;
	Vec3f eTarget;
	TextureContainer * tex_p2;
};

// Done By : Didier Pedreno
class CRiseDead : public CSpellFx {
	
public:
	CRiseDead();
	~CRiseDead();
	
	void SetDuration(const unsigned long duration);
	void SetDuration(unsigned long, unsigned long, unsigned long);
	
	void SetPos(Vec3f);
	
	void SetColorBorder(float, float, float);
	void SetColorRays1(float, float, float);
	void SetColorRays2(float, float, float);
	unsigned long GetDuration();
	
	void Create(Vec3f, float afBeta = 0);
	void Update(float timeDelta);
	void Render();
	
	Vec3f eSrc;
	
	float	fBetaRadCos;
	float	fBetaRadSin;
	void SetAngle(float afAngle) {
		float fBetaRad = radians(afAngle);
		fBetaRadCos = (float) cos(fBetaRad);
		fBetaRadSin = (float) sin(fBetaRad);
	}
	
private:
	void Split(Vec3f * v, int a, int b, float yo);
	void RenderFissure();
	
	float	fColorRays1[3];
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
	Vec3f va[40];
	Vec3f vb[40];
	Vec3f v1a[40];
	Vec3f v1b[40];
	
	EERIE_3DOBJ	*	stone[2];
	
	struct T_STONE {
		short actif;
		short numstone;
		Vec3f pos;
		float yvel;
		Anglef ang;
		Anglef angvel;
		Vec3f scale;
		int time;
		int currtime;
	};
	
	int currframetime;
	int timestone;
	int nbstone;
	T_STONE tstone[256];
	
	void AddStone(Vec3f * pos);
	void DrawStone();
};

#endif // ARX_GRAPHICS_SPELLS_SPELLS06_H
