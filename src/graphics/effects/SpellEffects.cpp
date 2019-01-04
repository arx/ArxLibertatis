/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include "graphics/effects/SpellEffects.h"

#include "animation/AnimationRender.h"
#include "game/Player.h"
#include "graphics/Math.h"
#include "graphics/RenderBatcher.h"
#include "math/RandomVector.h"
#include "scene/Object.h"


CSpellFx::CSpellFx()
{
	SetDuration(GameDurationMs(1000));
}

void CSpellFx::SetDuration(GameDuration duration) {
	m_duration = duration;

	if(m_duration <= 0)
		m_duration = GameDurationMs(100);
	
	m_elapsed = 0;
}

void Draw3DLineTexNew(const RenderMaterial & mat, Vec3f startPos, Vec3f endPos, Color startColor, Color endColor, float startSize, float endSize) {
	
	float fBeta = MAKEANGLE(player.angle.getYaw());
	float xxs = startSize * glm::cos(glm::radians(fBeta));
	float xxe = endSize * glm::cos(glm::radians(fBeta));
	float zzs = startSize;
	float zze = endSize;
	
	{
	TexturedQuad q1;
	q1.v[0].color = q1.v[1].color = startColor.toRGBA();
	q1.v[2].color = q1.v[3].color = endColor.toRGBA();
	
	q1.v[0].uv = Vec2f(0.f);
	q1.v[1].uv = Vec2f(1.f, 0.f);
	q1.v[2].uv = Vec2f(1.f);
	q1.v[3].uv = Vec2f(0.f, 1.f);
	
	q1.v[0].p = startPos + Vec3f(0.f, zzs, 0.f);
	q1.v[1].p = startPos + Vec3f(0.f, -zzs, 0.f);
	q1.v[2].p = endPos + Vec3f(0.f, -zze, 0.f);
	q1.v[3].p = endPos + Vec3f(0.f, zze, 0.f);
	
	drawQuadRTP(mat, q1);
	}
	
	zzs *= glm::sin(glm::radians(fBeta));
	zze *= glm::sin(glm::radians(fBeta));
	
	{
	TexturedQuad q2;
	
	q2.v[0].color = q2.v[1].color = startColor.toRGBA();
	q2.v[2].color = q2.v[3].color = endColor.toRGBA();
	
	q2.v[0].uv = Vec2f(0.f);
	q2.v[1].uv = Vec2f(1.f, 0.f);
	q2.v[2].uv = Vec2f(1.f);
	q2.v[3].uv = Vec2f(0.f, 1.f);
	
	q2.v[0].p = startPos + Vec3f(xxs, 0.f, zzs);
	q2.v[1].p = startPos + Vec3f(-xxs, 0.f, -zzs);
	q2.v[2].p = endPos + Vec3f(-xxe, 0.f, -zze);
	q2.v[3].p = endPos + Vec3f(xxe, 0.f, zze);
	
	drawQuadRTP(mat, q2);
	}
}


void Split(Vec3f * v, int a, int b, Vec3f f) {
	
	if(a != b) {
		int i = (a + b) / 2;
		if(i != a && i != b) {
			v[i].x = (v[a].x + v[b].x) * 0.5f + f.x * Random::getf(-1.f, 1.f);
			v[i].y = (v[a].y + v[b].y) * 0.5f + f.y * Random::getf(-1.f, 1.f);
			v[i].z = (v[a].z + v[b].z) * 0.5f + f.z * Random::getf(-1.f, 1.f);
			Split(v, a, i, f);
			Split(v, i, b, f);
		}
	}
	
}

void Split(Vec3f * v, int a, int b, float yo, float fMul) {
	
	if(a != b) {
		int i = (a + b) / 2;
		if(i != a && i != b) {
			v[i] = (v[a] + v[b]) * 0.5f + arx::randomVec(-yo, yo);
			Split(v, a, i, yo * fMul);
			Split(v, i, b, yo * fMul);
		}
	}
	
}

EERIE_3DOBJ * cabal = NULL;
EERIE_3DOBJ * ssol = NULL;
EERIE_3DOBJ * slight = NULL;
EERIE_3DOBJ * srune = NULL;
EERIE_3DOBJ * smotte = NULL;
EERIE_3DOBJ * stite = NULL;
EERIE_3DOBJ * smissile = NULL;
EERIE_3DOBJ * spapi = NULL;
EERIE_3DOBJ * svoodoo = NULL;
EERIE_3DOBJ * stone0 = NULL;
EERIE_3DOBJ * stone1 = NULL;

void LoadSpellModels() {
	// TODO Load dynamically
	cabal = loadObject("editor/obj3d/cabal.teo");
	ssol = loadObject("graph/obj3d/interactive/fix_inter/fx_rune_guard/fx_rune_guard.teo");
	slight = loadObject("graph/obj3d/interactive/fix_inter/fx_rune_guard/fx_rune_guard02.teo");
	srune = loadObject("graph/obj3d/interactive/fix_inter/fx_rune_guard/fx_rune_guard03.teo");
	smotte = loadObject("graph/obj3d/interactive/fix_inter/stalagmite/motte.teo");
	stite = loadObject("graph/obj3d/interactive/fix_inter/stalagmite/stalagmite.teo");
	smissile = loadObject("graph/obj3d/interactive/fix_inter/fx_magic_missile/fx_magic_missile.teo");
	spapi = loadObject("graph/obj3d/interactive/fix_inter/fx_papivolle/fx_papivolle.teo");
	svoodoo = loadObject("graph/obj3d/interactive/fix_inter/fx_voodoodoll/fx_voodoodoll.teo");
	stone0 = loadObject("graph/obj3d/interactive/fix_inter/fx_raise_dead/stone01.teo");
	stone1 = loadObject("graph/obj3d/interactive/fix_inter/fx_raise_dead/stone02.teo");
}

void ReleaseSpellModels() {
	delete cabal, cabal = NULL;
	delete ssol, ssol = NULL;
	delete slight, slight = NULL;
	delete srune, srune = NULL;
	delete smotte, smotte = NULL;
	delete stite, stite = NULL;
	delete smissile, smissile = NULL;
	delete spapi, spapi = NULL;
	delete svoodoo, svoodoo = NULL;
	delete stone0, stone0 = NULL;
	delete stone1, stone1 = NULL;
}
