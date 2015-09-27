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

#include "graphics/effects/MagicMissile.h"

#include "animation/AnimationRender.h"

#include "core/GameTime.h"

#include "game/Damage.h"
#include "game/EntityManager.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/Spells.h"

#include "graphics/Math.h"
#include "graphics/effects/SpellEffects.h"
#include "graphics/particle/ParticleParams.h"
#include "graphics/particle/ParticleManager.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/spells/Spells05.h"
#include "graphics/data/TextureContainer.h"

#include "physics/Collisions.h"

#include "scene/Light.h"
#include "scene/GameSound.h"
#include "scene/Object.h"
#include "scene/Interactive.h"


CMagicMissile::CMagicMissile()
	: CSpellFx()
	, bExplo(false)
	, bMove(true)
	, eSrc(Vec3f_ZERO)
	, eCurPos()
	, lightIntensityFactor()
	, iLength()
	, iBezierPrecision()
	, fTrail()
	, fOneOnBezierPrecision()
	, snd_loop()
{
	SetDuration(2000);
	ulCurrentTime = ulDuration + 1;
	
	m_trailColor = Color3f(0.9f, 0.9f, 0.7f) + Color3f(0.1f, 0.1f, 0.3f) * randomColor3f();
	m_projectileColor = Color3f(0.3f, 0.3f, 0.5f);
	tex_mm = TextureContainer::Load("graph/obj3d/textures/(fx)_bandelette_blue");
}

CMagicMissile::~CMagicMissile() {

	lLightId = LightHandle::Invalid;

	ARX_SOUND_Stop(snd_loop);
}

void CMagicMissile::Create(const Vec3f & aeSrc, const Anglef & angles)
{
	SetDuration(ulDuration);
	
	eCurPos = eSrc = aeSrc;
	
	short i = 40.f;
	Vec3f e = eSrc;
	e += angleToVectorXZ(angles.getPitch()) * (50.f * i);
	e.y += std::sin(glm::radians(MAKEANGLE(angles.getYaw()))) * (50.f * i);
	
	pathways[0] = eSrc;
	pathways[5] = e;
	Split(pathways, 0, 5, 50, 0.5f);

	for(i = 0; i < 6; i++) {
		if(pathways[i].y >= eSrc.y + 150) {
			pathways[i].y = eSrc.y + 150;
		}
	}

	fTrail = 0;

	iLength = 50;
	iBezierPrecision = BEZIERPrecision;
	fOneOnBezierPrecision = 1.0f / (float) iBezierPrecision;
	bExplo = false;
	bMove = true;

	ARX_SOUND_PlaySFX(SND_SPELL_MM_CREATE, &eCurPos);
	ARX_SOUND_PlaySFX(SND_SPELL_MM_LAUNCH, &eCurPos);
	snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_MM_LOOP, &eCurPos, 1.0F, ARX_SOUND_PLAY_LOOPED);
}

void CMagicMissile::SetTTL(unsigned long aulTTL)
{
	unsigned long t = ulCurrentTime;
	ulDuration = std::min(ulCurrentTime + aulTTL, ulDuration);
	SetDuration(ulDuration);
	ulCurrentTime = t;
	
	lLightId = LightHandle::Invalid;
}

void CMagicMissile::Update(float timeDelta)
{
	ARX_SOUND_RefreshPosition(snd_loop, eCurPos);

	ulCurrentTime += timeDelta;

	if(ulCurrentTime >= ulDuration)
		lightIntensityFactor = 0.f;
	else
		lightIntensityFactor = Random::getf(0.5f, 1.0f);
}

void CMagicMissile::Render()
{ 
	Vec3f lastpos, newpos;
	Vec3f v;

	if(ulCurrentTime >= ulDuration)
		return;
	
	RenderMaterial mat;
	mat.setCulling(Renderer::CullNone);
	mat.setDepthTest(true);
	mat.setBlendType(RenderMaterial::Additive);
	
	if(tex_mm)
		mat.setTexture(tex_mm);
	
	if(bMove) {
		float fOneOnDuration = 1.f / (float)(ulDuration);
		fTrail = (ulCurrentTime * fOneOnDuration) * (iBezierPrecision + 2) * 5;
	}
	
	newpos = lastpos = pathways[0];
	
	for(int i = 0; i < 5; i++) {
		int kp = i;
		int kpprec = (i > 0) ? kp - 1 : kp ;
		int kpsuiv = kp + 1 ;
		int kpsuivsuiv = (i < (5 - 2)) ? kpsuiv + 1 : kpsuiv;

		for(int toto = 1; toto < iBezierPrecision; toto++) {
			if(fTrail < i * iBezierPrecision + toto)
				break;

			float t = toto * fOneOnBezierPrecision;

			float t1 = t;
			float t2 = t1 * t1 ;
			float t3 = t2 * t1 ;
			float f0 = 2.f * t3 - 3.f * t2 + 1.f ;
			float f1 = -2.f * t3 + 3.f * t2 ;
			float f2 = t3 - 2.f * t2 + t1 ;
			float f3 = t3 - t2 ;

			float val = pathways[kpsuiv].x;
			float p0 = 0.5f * (val - pathways[kpprec].x) ;
			float p1 = 0.5f * (pathways[kpsuivsuiv].x - pathways[kp].x) ;
			v.x = f0 * pathways[kp].x + f1 * val + f2 * p0 + f3 * p1 ;

			val = pathways[kpsuiv].y ;
			p0 = 0.5f * (val - pathways[kpprec].y) ;
			p1 = 0.5f * (pathways[kpsuivsuiv].y - pathways[kp].y) ;
			v.y = f0 * pathways[kp].y + f1 * val + f2 * p0 + f3 * p1 ;

			val = pathways[kpsuiv].z ;
			p0 = 0.5f * (val - pathways[kpprec].z) ;
			p1 = 0.5f * (pathways[kpsuivsuiv].z - pathways[kp].z) ;
			v.z = f0 * pathways[kp].z + f1 * val + f2 * p0 + f3 * p1 ;

			newpos = v;

			if(!((fTrail - (i * iBezierPrecision + toto)) > iLength)) {
				float c;

				if(fTrail < iLength) {
					c = 1.0f - ((fTrail - (i * iBezierPrecision + toto)) / fTrail);
				} else {
					c = 1.0f - ((fTrail - (i * iBezierPrecision + toto)) / (float)iLength);
				}

				float fsize = c;
				float alpha = c - 0.2f;

				if(alpha < 0.2f)
					alpha = 0.2f;

				c += Random::getf(-0.1f, 0.1f);
				
				c = glm::clamp(c, 0.f, 1.f);
				
				Color color = (m_trailColor * (c * alpha)).to<u8>();

				if(fsize < 0.5f)
					fsize = fsize * 2 * 3;
				else
					fsize = (1.0f - fsize + 0.5f) * 2 * (3 * 0.5f);

				float fs = fsize * 6 + Random::getf(0.f, 0.3f);
				float fe = fsize * 6 + Random::getf(0.f, 0.3f);
				Draw3DLineTexNew(mat, lastpos, newpos, color, color, fs, fe);
			}

			Vec3f temp_vector = lastpos;
			lastpos = newpos;
			newpos = temp_vector;
		}
	}
	
	Vec3f av = newpos - lastpos;
	
	float bubu = getAngle(av.x, av.z, 0, 0);
	float bubu1 = getAngle(av.x, av.y, 0, 0);
	
	eCurPos = lastpos;
	
	Anglef stiteangle;
	stiteangle.setPitch(-glm::degrees(bubu));
	stiteangle.setYaw(0);
	stiteangle.setRoll(-(glm::degrees(bubu1)));

	if(av.x < 0)
		stiteangle.setRoll(stiteangle.getRoll() - 90);

	if(av.x > 0)
		stiteangle.setRoll(stiteangle.getRoll() + 90);

	if(stiteangle.getRoll() < 0)
		stiteangle.setRoll(stiteangle.getRoll() + 360.0f);

	Draw3DObject(smissile, stiteangle, eCurPos, Vec3f_ONE, m_projectileColor, mat);
}


MrMagicMissileFx::MrMagicMissileFx()
	: CMagicMissile()
{
	m_trailColor = Color3f(0.9f, 0.2f, 0.5f);
	m_projectileColor = Color3f(1.f, 0.f, 0.2f);
	tex_mm = NULL;
}
