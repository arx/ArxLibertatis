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

#include "graphics/effects/SpellEffects.h"

// Done By : Didier Pedreno
class CRiseDead : public CSpellFx {
	
public:
	CRiseDead();
	~CRiseDead();
	
	void SetDuration(const unsigned long duration);
	void SetDuration(unsigned long, unsigned long, unsigned long);
	
	void SetPos(Vec3f);
	
	void SetColorBorder(Color3f color);
	void SetColorRays1(Color3f color);
	void SetColorRays2(Color3f color);
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
	
	Color3f m_colorBorder;
	Color3f m_colorRays1;
	Color3f m_colorRays2;
	
	TextureContainer * tex_light;
	int		end;
	int		iSize;
	bool	bIntro;
	float	fOneOnDurationIntro;
	float	fOneOnDurationRender;
	float	fOneOnDurationOuttro;
	float	sizeF;
	float	fSizeIntro;
	float	fTexWrap;
	float	tfRaysa[40];
	float	tfRaysb[40];
	unsigned long ulDurationIntro;
	unsigned long ulDurationRender;
	unsigned long ulDurationOuttro;
	Vec3f va[40];
	Vec3f vb[40];
	Vec3f v1a[40];
	Vec3f v1b[40];
	
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
	
	int m_currframetime;
	int m_timestone;
	int m_nbstone;
	T_STONE m_tstone[256];
	
	void AddStone(const Vec3f & pos);
	void DrawStone();
};


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
	
	LightHandle lLightId;
private:
	float	fBetaRadCos;
	float	fBetaRadSin;
	
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

#endif // ARX_GRAPHICS_EFFECTS_FISSURE_H
