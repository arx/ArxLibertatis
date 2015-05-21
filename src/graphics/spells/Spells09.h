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

#ifndef ARX_GRAPHICS_SPELLS_SPELLS09_H
#define ARX_GRAPHICS_SPELLS_SPELLS09_H

#include "graphics/effects/SpellEffects.h"
#include "graphics/particle/ParticleSystem.h"

// Done By : Didier Pedreno
class CSummonCreature : public CSpellFx {
public:
	Vec3f eSrc;
	
	CSummonCreature();
	
	void SetDuration(const unsigned long duration);
	void SetDuration(unsigned long, unsigned long, unsigned long);
	void SetPos(Vec3f);
	
	void SetColorBorder(Color3f);
	void SetColorRays1(Color3f);
	void SetColorRays2(Color3f);
	
	unsigned long GetDuration();
	
	void Create(Vec3f, float afBeta = 0);
	void Kill();
	void Update(float timeDelta);
	void Render();
	
private:
	float	fBetaRadCos;
	float	fBetaRadSin;
	void SetAngle(float afAngle) {
		float fBetaRad = glm::radians(afAngle);
		fBetaRadCos = glm::cos(fBetaRad);
		fBetaRadSin = glm::sin(fBetaRad);
	}
	
	Color3f fColorRays1;
	
	void Split(Vec3f * v, int a, int b, float yo);
	void RenderFissure();
	
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
	float	fTexWrap;
	Color3f fColorBorder;
	Color3f fColorRays2;
	float tfRaysa[40];
	float tfRaysb[40];
	unsigned long ulDurationIntro;
	unsigned long ulDurationRender;
	unsigned long ulDurationOuttro;
	Vec3f va[40];
	Vec3f vb[40];
	Vec3f v1a[40];
	Vec3f v1b[40];
};

// Done By : Didier Pedreno
class CNegateMagic: public CSpellFx {
	
public:
	CNegateMagic();
	~CNegateMagic();
	
	void Create(Vec3f);
	void Update(float timeDelta);
	void Render();
	
	void SetPos(Vec3f pos);
	
private:
	Vec3f eSrc;
	TextureContainer * tex_p2;
	TextureContainer * tex_sol;
};

#endif // ARX_GRAPHICS_SPELLS_SPELLS09_H
