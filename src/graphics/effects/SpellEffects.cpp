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
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include "graphics/effects/SpellEffects.h"

#include "animation/AnimationRender.h"
#include "game/Player.h"
#include "graphics/Math.h"


CSpellFx::CSpellFx() :
	lLightId(-1)
{
	SetDuration(1000);
}

void CSpellFx::SetDuration(const unsigned long ulaDuration) {
	ulDuration = ulaDuration;

	if(ulDuration <= 0)
		ulDuration = 100;
	
	ulCurrentTime = 0;
}

unsigned long CSpellFx::getCurrentTime() {
	return ulCurrentTime;
}

unsigned long CSpellFx::GetDuration() {
	return ulDuration;
}

void Draw3DLineTexNew(Vec3f startPos, Vec3f endPos, Color startColor, Color endColor, float startSize, float endSize) {

	float fBeta = MAKEANGLE(player.angle.getPitch());
	float xxs = (float)(startSize * cos(radians(fBeta)));
	float xxe = (float)(endSize * cos(radians(fBeta)));
	float zzs = startSize;
	float zze = endSize;

	TexturedVertex v[4];
	TexturedVertex v2[4];

	v2[0].color = v2[1].color = startColor.toBGRA();
	v2[2].color = v2[3].color = endColor.toBGRA();

	// version 2 faces
	v2[0].uv = Vec2f_ZERO;
	v2[1].uv = Vec2f_X_AXIS;
	v2[2].uv = Vec2f_ONE;
	v2[3].uv = Vec2f_Y_AXIS;

	v[0].p.x = startPos.x;
	v[0].p.y = startPos.y + zzs;
	v[0].p.z = startPos.z;

	v[1].p.x = startPos.x;
	v[1].p.y = startPos.y - zzs;
	v[1].p.z = startPos.z;

	v[2].p.x = endPos.x;
	v[2].p.y = endPos.y - zze;
	v[2].p.z = endPos.z;

	v[3].p.x = endPos.x;
	v[3].p.y = endPos.y + zze;
	v[3].p.z = endPos.z;

	EE_RT(v[0].p, v2[0].p);
	EE_RT(v[1].p, v2[1].p);
	EE_RT(v[2].p, v2[2].p);
	EE_RT(v[3].p, v2[3].p);
	ARX_DrawPrimitive(&v2[0], &v2[1], &v2[3]);
	ARX_DrawPrimitive(&v2[1], &v2[2], &v2[3]);

	zzs *= (float) sin(radians(fBeta));
	zze *= (float) sin(radians(fBeta));

	v[0].p.x = startPos.x + xxs;
	v[0].p.y = startPos.y;
	v[0].p.z = startPos.z + zzs;

	v[1].p.x = startPos.x - xxs;
	v[1].p.y = startPos.y;
	v[1].p.z = startPos.z - zzs;

	v[2].p.x = endPos.x - xxe;
	v[2].p.y = endPos.y;
	v[2].p.z = endPos.z - zze;

	v[3].p.x = endPos.x + xxe;
	v[3].p.y = endPos.y;
	v[3].p.z = endPos.z + zze;

	EE_RT(v[0].p, v2[0].p);
	EE_RT(v[1].p, v2[1].p);
	EE_RT(v[2].p, v2[2].p);
	EE_RT(v[3].p, v2[3].p);
	ARX_DrawPrimitive(&v2[0], &v2[1], &v2[3]);
	ARX_DrawPrimitive(&v2[1], &v2[2], &v2[3]);
}


void Split(Vec3f * v, int a, int b, float fX, float fMulX, float fY, float fMulY, float fZ, float fMulZ)
{
	if (a != b)
	{
		int i = (int)((a + b) * 0.5f);

		if ((i != a) && (i != b))
		{
			v[i].x = (v[a].x + v[b].x) * 0.5f + fX * frand2();
			v[i].y = (v[a].y + v[b].y) * 0.5f + fY * frand2(); 
			v[i].z = (v[a].z + v[b].z) * 0.5f + fZ * frand2(); 
			Split(v, a, i, fX, fMulX, fY, fMulY, fZ, fMulZ);
			Split(v, i, b, fX, fMulX, fY, fMulY, fZ, fMulZ);
		}
	}
}

void Split(TexturedVertex * v, int a, int b, float yo, float fMul)
{
	if (a != b)
	{
		int i = (int)((a + b) * 0.5f);

		if ((i != a) && (i != b))
		{
			v[i].p = (v[a].p + v[b].p) * 0.5f + randomVec(-yo, yo);
			Split(v, a, i, yo * fMul);
			Split(v, i, b, yo * fMul);
		}
	}
}


EERIE_3DOBJ * ssol = NULL;
long ssol_count = 0;
EERIE_3DOBJ * slight = NULL;
long slight_count = 0;
EERIE_3DOBJ * srune = NULL;
long srune_count = 0;
EERIE_3DOBJ * smotte = NULL;
long smotte_count = 0;
EERIE_3DOBJ * stite = NULL;
long stite_count = 0;
EERIE_3DOBJ * smissile = NULL;
long smissile_count = 0;
EERIE_3DOBJ * spapi = NULL;
long spapi_count = 0;
EERIE_3DOBJ * svoodoo = NULL;
long svoodoo_count = 0;
EERIE_3DOBJ * stone1 = NULL;
long stone1_count = 0;
EERIE_3DOBJ * stone0 = NULL;
long stone0_count = 0;

void DANAE_ReleaseAllDatasDynamic() {
	delete ssol, ssol = NULL, ssol_count = 0;
	delete slight, slight = NULL, slight_count = 0;
	delete srune, srune = NULL, srune_count = 0;
	delete smotte, smotte = NULL, smotte_count = 0;
	delete stite, stite = NULL, stite_count = 0;
	delete smissile, smissile = NULL, smissile_count = 0;
	delete spapi, spapi = NULL, spapi_count = 0;
	delete svoodoo, svoodoo = NULL, svoodoo_count = 0;
	delete stone0, stone0 = NULL, stone0_count = 0;
	delete stone1, stone1 = NULL, stone1_count = 0;
}

