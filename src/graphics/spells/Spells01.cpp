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

#include "graphics/spells/Spells01.h"

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

extern ParticleManager * pParticleManager;
 
extern long cur_mr;


class MagicMissileExplosionParticle : public ParticleParams {
public:
	MagicMissileExplosionParticle() {
		load();
	}
	
	void load() {
		m_nbMax = 100;
		m_life = 1500;
		m_lifeRandom = 0;
		m_pos = Vec3f(10.f);
		m_direction = Vec3f(0.f, -10.f, 0.f) * 0.1f;
		m_angle = glm::radians(360.f);
		m_speed = 130;
		m_speedRandom = 100;
		m_gravity = Vec3f(0.f, 10.f, 0.f);
		m_flash = 0;
		m_rotation = 1.0f / (101 - 16);
	
		m_startSegment.m_size = 5;
		m_startSegment.m_sizeRandom = 10;
		m_startSegment.m_color = Color(110, 110, 110, 110).to<float>();
		m_startSegment.m_colorRandom = Color(100, 100, 100, 100).to<float>();
		m_endSegment.m_size = 0;
		m_endSegment.m_sizeRandom = 2;
		m_endSegment.m_color = Color(0, 0, 120, 10).to<float>();
		m_endSegment.m_colorRandom = Color(50, 50, 50, 50).to<float>();
		
		m_texture.set("graph/particles/magicexplosion", 0, 500);
		
		m_blendMode = RenderMaterial::Additive;
		m_spawnFlags = 0;
		m_looping = false;
	}
};

class MagicMissileExplosionMrCheatParticle : public MagicMissileExplosionParticle {
public:
	MagicMissileExplosionMrCheatParticle() {
		load();
	}
	
	void load() {
		MagicMissileExplosionParticle::load();
		
		m_speed = 13;
		m_speedRandom = 10;
		m_startSegment.m_size = 20;
		m_startSegment.m_color = Color(0, 0, 0, 0).to<float>();
		m_startSegment.m_colorRandom = Color(0, 0, 0, 0).to<float>();
		m_endSegment.m_color = Color(255, 40, 120, 10).to<float>();
		m_texture.set("graph/particles/(fx)_mr", 0, 500);
	}
};

static void LaunchMagicMissileExplosion(const Vec3f & _ePos, bool mrCheat) {
	
	ParticleParams cp = MagicMissileExplosionParticle();
	
	if(mrCheat) {
		cp = MagicMissileExplosionMrCheatParticle();
	}
	
	ParticleSystem * pPS = new ParticleSystem();
	pPS->SetParams(cp);
	pPS->SetPos(_ePos);
	pPS->Update(0);

	LightHandle id = GetFreeDynLight();

	if(lightHandleIsValid(id)) {
		EERIE_LIGHT * light = lightHandleGet(id);
		
		light->intensity = 2.3f;
		light->fallstart = 250.f;
		light->fallend   = 420.f;

		if(mrCheat) {
			light->rgb = Color3f(1.f, 0.3f, .8f);
		} else {
			light->rgb = Color3f(0.f, 0.f, .8f);
		}

		light->pos = _ePos;
		light->duration = 1500;
	}

	arx_assert(pParticleManager);
	pParticleManager->AddSystem(pPS);

	ARX_SOUND_PlaySFX(SND_SPELL_MM_HIT, &_ePos);
}

CMagicMissile::CMagicMissile(bool mrCheat)
	: CSpellFx()
	, bExplo(false)
	, bMove(true)
	, eSrc(Vec3f_ZERO)
	, eCurPos()
	, lightIntensityFactor()
	, iLength()
	, iBezierPrecision()
	, fColor(Color3f::white)
	, fTrail()
	, fOneOnBezierPrecision()
	, tex_mm()
	, snd_loop()
	, m_mrCheat(mrCheat)
{
	SetDuration(2000);
	ulCurrentTime = ulDuration + 1;

	tex_mm = TextureContainer::Load("graph/obj3d/textures/(fx)_bandelette_blue");

	if(!smissile)
		smissile = LoadTheObj("graph/obj3d/interactive/fix_inter/fx_magic_missile/fx_magic_missile.teo");

	smissile_count++;
}

CMagicMissile::~CMagicMissile()
{
	smissile_count--;

	if(smissile && smissile_count <= 0) {
		smissile_count = 0;
		delete smissile;
		smissile = NULL;
	}

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
	
	pathways[0].p = eSrc;
	pathways[5].p = e;
	Split(pathways, 0, 5, 50, 0.5f);

	for(i = 0; i < 6; i++) {
		if(pathways[i].p.y >= eSrc.y + 150) {
			pathways[i].p.y = eSrc.y + 150;
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

void CMagicMissile::SetColor(Color3f color) {
	fColor = color;
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
		lightIntensityFactor = 1 - 0.5f * rnd();
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
	
	if(tex_mm && !m_mrCheat)
		mat.setTexture(tex_mm);
	
	if(bMove) {
		float fOneOnDuration = 1.f / (float)(ulDuration);
		fTrail = (ulCurrentTime * fOneOnDuration) * (iBezierPrecision + 2) * 5;
	}
	
	newpos = lastpos = pathways[0].p;
	
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

			float val = pathways[kpsuiv].p.x;
			float p0 = 0.5f * (val - pathways[kpprec].p.x) ;
			float p1 = 0.5f * (pathways[kpsuivsuiv].p.x - pathways[kp].p.x) ;
			v.x = f0 * pathways[kp].p.x + f1 * val + f2 * p0 + f3 * p1 ;

			val = pathways[kpsuiv].p.y ;
			p0 = 0.5f * (val - pathways[kpprec].p.y) ;
			p1 = 0.5f * (pathways[kpsuivsuiv].p.y - pathways[kp].p.y) ;
			v.y = f0 * pathways[kp].p.y + f1 * val + f2 * p0 + f3 * p1 ;

			val = pathways[kpsuiv].p.z ;
			p0 = 0.5f * (val - pathways[kpprec].p.z) ;
			p1 = 0.5f * (pathways[kpsuivsuiv].p.z - pathways[kp].p.z) ;
			v.z = f0 * pathways[kp].p.z + f1 * val + f2 * p0 + f3 * p1 ;

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

				c += frand2() * 0.1f;

				if (c < 0) c = 0;
				else if (c > 1) c = 1;

				Color color = (fColor * (c * alpha)).to<u8>();

				if(fsize < 0.5f)
					fsize = fsize * 2 * 3;
				else
					fsize = (1.0f - fsize + 0.5f) * 2 * (3 * 0.5f);

				float fs = fsize * 6 + rnd() * 0.3f;
				float fe = fsize * 6 + rnd() * 0.3f;
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
	
	Vec3f stitepos = lastpos;

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

	Color3f stitecolor;
	if(m_mrCheat) {
		stitecolor.r = 1.f;
		stitecolor.g = 0.f;
		stitecolor.b = 0.2f;
	} else {
		stitecolor.r = 0.3f;
		stitecolor.g = 0.3f;
		stitecolor.b = 0.5f;
	}

	Vec3f stitescale = Vec3f_ONE;

	Draw3DObject(smissile, stiteangle, stitepos, stitescale, stitecolor, mat);

	eCurPos = lastpos;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CMultiMagicMissile::CMultiMagicMissile(size_t nbmissiles, bool mrCheat)
	: CSpellFx()
	, m_mrCheat(mrCheat)
{
	SetDuration(2000);
	
	pTab.reserve(nbmissiles);
	
	for(size_t i = 0; i < nbmissiles; i++) {
		CMagicMissile * missile = new CMagicMissile(m_mrCheat);
		
		pTab.push_back(missile);
	}
}

CMultiMagicMissile::~CMultiMagicMissile()
{
	for(size_t i = 0; i < pTab.size(); i++) {
		// no need to kill it because it's a duration light !
		pTab[i]->lLightId = LightHandle::Invalid;

		delete pTab[i];
	}

	pTab.clear();
}

void CMultiMagicMissile::Create(Vec3f aePos, float afAlpha, float afBeta)
{
	
	long lMax = 0;
	
	for(size_t i = 0; i < pTab.size(); i++) {
		Anglef angles(afAlpha, afBeta, 0.f);
		
		if(i > 0) {
			angles.setYaw(angles.getYaw() + frand2() * 4.0f);
			angles.setPitch(angles.getPitch() + frand2() * 6.0f);
		}
		
		pTab[i]->Create(aePos, angles);  
		
		float fTime = ulDuration + frand2() * 1000.0f;
		long lTime = checked_range_cast<long>(fTime);
		
		lTime		= std::max(1000L, lTime);
		lMax		= std::max(lMax, lTime);
		
		CMagicMissile * pMM = (CMagicMissile *)pTab[i];
		
		pMM->SetDuration(lTime);
		
		if(m_mrCheat) {
			pMM->SetColor(Color3f(0.9f, 0.2f, 0.5f));
		} else {
			pMM->SetColor(Color3f(0.9f + rnd() * 0.1f, 0.9f + rnd() * 0.1f, 0.7f + rnd() * 0.3f));
		}
		
		pTab[i]->lLightId = GetFreeDynLight();
		
		if(lightHandleIsValid(pTab[i]->lLightId)) {
			EERIE_LIGHT * el = lightHandleGet(pTab[i]->lLightId);
			
			el->intensity	= 0.7f + 2.3f;
			el->fallend		= 190.f;
			el->fallstart	= 80.f;
			
			if(m_mrCheat) {
				el->rgb = Color3f(1.f, 0.3f, 0.8f);
			} else {
				el->rgb = Color3f(0.f, 0.f, 1.f);
			}
			
			el->pos	 = pMM->eSrc;
			el->duration = 300;
		}
	}
	
	SetDuration(lMax + 1000);
}

void CMultiMagicMissile::CheckCollision(float level, EntityHandle caster)
{
	for(size_t i = 0; i < pTab.size(); i++) {
		CMagicMissile * missile = (CMagicMissile *) pTab[i];
		
		if(missile->bExplo)
			continue;
			
		Sphere sphere;
		sphere.origin = missile->eCurPos;
		sphere.radius	= 10.f;
		
		if(CheckAnythingInSphere(sphere, caster, CAS_NO_SAME_GROUP))
		{
			LaunchMagicMissileExplosion(missile->eCurPos, m_mrCheat);
			ARX_NPC_SpawnAudibleSound(missile->eCurPos, entities[caster]);
			
			missile->SetTTL(1000);
			missile->bExplo = true;
			missile->bMove  = false;
			
			missile->lLightId = LightHandle::Invalid;
			
			DamageParameters damage;
			damage.pos = missile->eCurPos;
			damage.radius = 80.f;
			damage.damages = (4 + level * ( 1.0f / 5 )) * .8f;
			damage.area	= DAMAGE_FULL;
			damage.duration = -1;
			damage.source = caster;
			damage.flags = DAMAGE_FLAG_DONT_HURT_SOURCE;
			damage.type = DAMAGE_TYPE_MAGICAL;
			DamageCreate(damage);
			
			Color3f rgb(.3f, .3f, .45f);
			ARX_PARTICLES_Add_Smoke(missile->eCurPos, 0, 6, &rgb);
		}
	}
}

bool CMultiMagicMissile::CheckAllDestroyed()
{
	long nbmissiles	= 0;
	
	for(size_t i = 0; i < pTab.size(); i++) {
		CMagicMissile *pMM = pTab[i];
		if(pMM->bMove)
			nbmissiles++;
	}
	
	return nbmissiles == 0;
}

void CMultiMagicMissile::Update(float timeDelta)
{
	for(size_t i = 0 ; i < pTab.size() ; i++) {
		pTab[i]->Update(timeDelta);
	}
}

void CMultiMagicMissile::Render()
{ 
	for(size_t i = 0; i < pTab.size(); i++) {
		pTab[i]->Render();
		
		CMagicMissile * pMM = (CMagicMissile *) pTab[i];
		
		if(lightHandleIsValid(pMM->lLightId)) {
			EERIE_LIGHT * el	= lightHandleGet(pMM->lLightId);
			el->intensity		= 0.7f + 2.3f * pMM->lightIntensityFactor;
			el->pos = pMM->eCurPos;
			el->time_creation	= (unsigned long)(arxtime);
		}
	}
}
