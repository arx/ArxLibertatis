/*
 * Copyright 2015 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GRAPHICS_EFFECTS_FISSURE_H
#define ARX_GRAPHICS_EFFECTS_FISSURE_H

#include "graphics/effects/FloatingStones.h"
#include "graphics/effects/SpellEffects.h"

class FissureFx {
public:
	FissureFx();
	
	void SetDuration(unsigned long alDurationIntro, unsigned long alDurationRender, unsigned long alDurationOuttro);
	
	void SetColorBorder(Color3f color);
	void SetColorRays1(Color3f color);
	void SetColorRays2(Color3f color);
	
	unsigned long ulDuration;
	unsigned long ulCurrentTime;
	unsigned long ulDurationIntro;
	unsigned long ulDurationRender;
	unsigned long ulDurationOuttro;
	float	fOneOnDurationIntro;
	float	fOneOnDurationRender;
	float	fOneOnDurationOuttro;
	
	Color3f m_colorBorder;
	Color3f m_colorRays1;
	Color3f m_colorRays2;
};


// Done By : Didier Pedreno
class CRiseDead : public FissureFx {
	
public:
	CRiseDead();
	~CRiseDead();
	
	unsigned long GetDuration();
	
	void Create(Vec3f, float afBeta = 0);
	void Update(float timeDelta);
	void Render();
	
	Vec3f m_eSrc;
	
	LightHandle lLightId;
private:
	float	fBetaRadCos;
	float	fBetaRadSin;
	
	void Split(Vec3f * v, int a, int b, float yo);
	void RenderFissure();
	
	TextureContainer * tex_light;
	int		end;
	int		iSize;
	bool	bIntro;
	float	sizeF;
	float	fSizeIntro;
	float	tfRaysa[40];
	float	tfRaysb[40];
	Vec3f va[40];
	Vec3f vb[40];
	Vec3f v1a[40];
	Vec3f v1b[40];
	
	FloatingStones m_stones;
};


// Done By : Didier Pedreno
class CSummonCreature : public FissureFx {
public:
	Vec3f m_eSrc;
	
	CSummonCreature();
	
	void Create(Vec3f, float afBeta = 0);
	void Kill();
	void Update(float timeDelta);
	void Render();
	
	LightHandle lLightId;
private:
	float	fBetaRadCos;
	float	fBetaRadSin;
	
	void Split(Vec3f * v, int a, int b, float yo);
	void RenderFissure();
	
	TextureContainer * tex_light;
	int		end;
	int		iSize;
	bool	bIntro;
	float	fOneOniSize;

	float	sizeF;
	float	fSizeIntro;
	float tfRaysa[40];
	float tfRaysb[40];
	Vec3f va[40];
	Vec3f vb[40];
	Vec3f v1a[40];
	Vec3f v1b[40];
};

#endif // ARX_GRAPHICS_EFFECTS_FISSURE_H
