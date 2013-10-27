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
	fBeta(0),
	fManaCostToLaunch(10),
	fManaCostPerSecond(10),
	lLightId(-1),
	lSrc(-1),
	spellinstance(-1)
{
	SetDuration(1000);
	SetAngle(fBeta);
}

void CSpellFx::SetDuration(const unsigned long ulaDuration) {
	ulDuration = ulaDuration;

	if(ulDuration <= 0)
		ulDuration = 100;

	fOneOnDuration = 1.f / (float)(ulDuration);

	ulCurrentTime = 0;
}

unsigned long CSpellFx::getCurrentTime() {
	return ulCurrentTime;
}

unsigned long CSpellFx::GetDuration() {
	return ulDuration;
}

void CSpellFx::SetAngle(float afAngle) {
	fBeta = afAngle;
	fBetaRad = radians(fBeta);
	fBetaRadCos = (float) cos(fBetaRad);
	fBetaRadSin = (float) sin(fBetaRad);
}

void CSpellFx::Update(float _fParam) {
	Update(checked_range_cast<unsigned long>(_fParam));
}

//-----------------------------------------------------------------------------
void Draw3DLineTex(Vec3f s, Vec3f e, Color color, float fStartSize, float fEndSize) {
	
	Draw3DLineTexNew(s, e, color, color, fStartSize, fEndSize);
}

void Draw3DLineTex2(Vec3f s, Vec3f e, float fSize, Color color, Color color2) {

	Draw3DLineTexNew(s, e, color, color2, fSize, fSize);
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

	EE_RT2(&v[0], &v2[0]);
	EE_RT2(&v[1], &v2[1]);
	EE_RT2(&v[2], &v2[2]);
	EE_RT2(&v[3], &v2[3]);
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

	EE_RT2(&v[0], &v2[0]);
	EE_RT2(&v[1], &v2[1]);
	EE_RT2(&v[2], &v2[2]);
	EE_RT2(&v[3], &v2[3]);
	ARX_DrawPrimitive(&v2[0], &v2[1], &v2[3]);
	ARX_DrawPrimitive(&v2[1], &v2[2], &v2[3]);
}


void Split(TexturedVertex * v, int a, int b, float fX, float fMulX, float fY, float fMulY, float fZ, float fMulZ)
{
	if (a != b)
	{
		int i = (int)((a + b) * 0.5f);

		if ((i != a) && (i != b))
		{
			v[i].p.x = (v[a].p.x + v[b].p.x) * 0.5f + fX * frand2();
			v[i].p.y = (v[a].p.y + v[b].p.y) * 0.5f + fY * frand2(); 
			v[i].p.z = (v[a].p.z + v[b].p.z) * 0.5f + fZ * frand2(); 
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

