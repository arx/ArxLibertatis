/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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
// Code: Didier Pédreno - Sébastien Scieux
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#ifndef ARX_GRAPHICS_EFFECTS_SPELLEFFECTS_H
#define ARX_GRAPHICS_EFFECTS_SPELLEFFECTS_H

#include "game/GameTypes.h"
#include "graphics/Color.h"
#include "graphics/Math.h"
#include "math/Random.h"
#include "math/Types.h"
#include "platform/Alignment.h"
#include "scene/Light.h"

struct RenderMaterial;

const int BEZIERPrecision = 32;

class ARX_ALIGNAS(16) CSpellFx {
	
public:
	
	// We can't use ARX_ALIGNOF(glm::mat4x4) directly because MSVC sucks
	ARX_STATIC_ASSERT(ARX_ALIGNOF(glm::mat4x4) <= 16, "need to increase alignment");
	
		GameDuration m_duration;
		GameDuration m_elapsed;

		CSpellFx();
		virtual ~CSpellFx() { }
		
		virtual void SetDuration(GameDuration duration);
		
		virtual void Update(GameDuration timeDelta) = 0;
		virtual void Render() = 0;
	
	ARX_USE_ALIGNED_NEW(CSpellFx)
};

void Draw3DLineTexNew(const RenderMaterial & mat, Vec3f startPos, Vec3f endPos, Color startColor, Color endColor, float startSize, float endSize);

void Split(Vec3f * v, int a, int b, Vec3f f);
void Split(Vec3f * v, int a, int b, float yo, float fMul = 0.5f);

extern EERIE_3DOBJ * cabal;
extern EERIE_3DOBJ * ssol;
extern EERIE_3DOBJ * slight;
extern EERIE_3DOBJ * srune;
extern EERIE_3DOBJ * smotte;
extern EERIE_3DOBJ * stite;
extern EERIE_3DOBJ * smissile;
extern EERIE_3DOBJ * spapi;
extern EERIE_3DOBJ * svoodoo;
extern EERIE_3DOBJ * stone0;
extern EERIE_3DOBJ * stone1;

void LoadSpellModels();
void ReleaseSpellModels();

#endif // ARX_GRAPHICS_EFFECTS_SPELLEFFECTS_H
