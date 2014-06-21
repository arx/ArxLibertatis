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
	
	void SetTTL(unsigned long);
	void SetColor(Color3f);
	
	void Create(const Vec3f &, const Anglef &);
	void Update(float timeDelta);
	void Render();
	
	bool bExplo;
	bool bMove;
	Vec3f eSrc;
	Vec3f eCurPos;
	
	float lightIntensityFactor;
	SpellHandle spellinstance;
	
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
	
public:
	explicit CMultiMagicMissile(size_t nb, SpellHandle spellHandle);
	~CMultiMagicMissile();
	
	void CheckCollision(float level, EntityHandle caster);
	bool CheckAllDestroyed();
	
	void Create();
	void Update(float timeDelta);
	void Render();
	
	SpellHandle spellinstance;
	
private:
	std::vector<CMagicMissile *> pTab;
};

// Done By : Sébastien Scieux
class CIgnit : public CSpellFx {
	
protected:
	Vec3f pos;
	bool m_active;
	int duration;
	int currduration;
	
	Color3f rgb;
	
	struct T_LINKLIGHTTOFX {
		Vec3f poslight;
		Vec3f posfx;
		LightHandle idl;
		int iLightNum;
	};
	
public:
	std::vector<T_LINKLIGHTTOFX> tablight;
	
	CIgnit();
	~CIgnit();
	
	void Create(Vec3f * posc, int speed);
	void Update(float timeDelta);
	void Render();
	void Kill();
	void AddLight(int iLight);
	void Action(bool enable);
};

// Done By : Sébastien Scieux
class CDoze: public CIgnit {
	
public:
	
	void CreateDoze(Vec3f * posc, int speed);
	
	void AddLightDoze(int iLight);
	
};

#endif // ARX_GRAPHICS_SPELLS_SPELLS01_H
