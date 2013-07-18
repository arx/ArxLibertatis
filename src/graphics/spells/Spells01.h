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

#ifndef ARX_GRAPHICS_SPELLS_SPELLS01_H
#define ARX_GRAPHICS_SPELLS_SPELLS01_H

#include "graphics/effects/SpellEffects.h"

// Done By : Didier Pédreno
class CMagicMissile : public CSpellFx {
	
public:
	CMagicMissile();
	~CMagicMissile();

	// accesseurs
	void SetTTL(unsigned long);
	void SetColor(Color3f);

	// surcharge
	void Create(const Vec3f &, const Anglef &);
	void Update(unsigned long);
	void Render();

	bool bExplo;
	bool bMove;
	Vec3f eSrc;
	Vec3f eCurPos;

	float lightIntensityFactor;

private:
	int iLength;
	int	iBezierPrecision;
	Color3f fColor;
	float fTrail;
	float fOneOnBezierPrecision;
	Anglef angles;
	TextureContainer * tex_mm;
	TexturedVertex pathways[6];
	audio::SourceId snd_loop;
};

class CMultiMagicMissile : public CSpellFx {
	
private:
	CMagicMissile ** pTab;
	unsigned int uiNumber;
	
public:
	explicit CMultiMagicMissile(long nb);
	~CMultiMagicMissile();
	
	void CheckCollision();
	bool CheckAllDestroyed();
	
	void Create();
	void Update(unsigned long);
	void Render();
};

// Done By : Sébastien Scieux
class CIgnit : public CSpellFx {
	
private:
	Vec3f pos;
	float perimetre;
	short key;
	int duration;
	int currduration;
	float interp;
	
	TextureContainer * tp;
	Color3f rgb;
	int mask;
	
	struct T_LINKLIGHTTOFX {
		Vec3f poslight;
		Vec3f posfx;
		int actif;
		int idl;
		int iLightNum;
	};
	
public:
	unsigned char nblight;
	T_LINKLIGHTTOFX tablight[256];
	
	CIgnit();
	~CIgnit();
	
	unsigned long GetDuration(void) {
		return this->duration;
	}
	
	void Create(Vec3f * posc, float perim, int speed);
	void Update(unsigned long time);
	void Render();
	void Kill();
	void AddLight(int iLight);
	void Action(int mode);
	
	void ChangeRGBMask(float r, float g, float b, int mask) {
		rgb = Color3f(r, g, b);
		this->mask = mask;
	}
	
	void ChangeTexture(TextureContainer * tc) {
		this->tp = tc;
	}
	
	float GetPerimetre(void) {
		return this->perimetre;
	}
};

// Done By : Sébastien Scieux
class CDoze: public CIgnit {
	
public:
	
	void CreateDoze(Vec3f * posc, float perim, int speed);
	
	void AddLightDoze(int iLight);
	
};

// Done By : Sébastien Scieux
class CPortal: public CSpellFx {
	
private:
	
	short key;
	int duration;
	int currduration, currframe;
	Vec3f pos;
	float r;
	TextureContainer * tp, * te;
	
	struct T_ECLAIR {
		short actif, nbseg;
		int duration;
		int currduration;
		int numpt;
		Vec3f * seg;
	};
	
	int timeneweclair;
	int nbeclair; // eclair
	T_ECLAIR tabeclair[256];
	
	Vec3f sphereposdep;
	Vec3f sphereposend;
	Vec3f * spherevertex;
	unsigned short * sphereind;
	int spherenbpt;
	int spherenbfaces;
	float spherealpha;
	TexturedVertex * sphered3d;
	
public:
	
	~CPortal();
	
	unsigned long GetDuration(void) {
		return this->duration;
	}
	
	void AddNewEclair(Vec3f * endpos, int nbseg, int duration, int numpt);
	void DrawAllEclair();

	void Update(unsigned long);
	void Render();
};

#endif // ARX_GRAPHICS_SPELLS_SPELLS01_H
