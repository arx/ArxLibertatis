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
		m_angle = radians(360);
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

void LaunchMagicMissileExplosion(const Vec3f & _ePos, SpellHandle spellinstance = InvalidSpellHandle)
{
	ParticleParams cp = MagicMissileExplosionParticle();
	
	if(spellinstance != InvalidSpellHandle && spells[spellinstance]->m_caster == PlayerEntityHandle && cur_mr == 3) {
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

		if(spellinstance != InvalidSpellHandle && spells[spellinstance]->m_caster == PlayerEntityHandle && cur_mr == 3) {
			light->rgb.r = 1.f;
			light->rgb.g = 0.3f;
			light->rgb.b = .8f;
		} else {
			light->rgb.r = 0.f;
			light->rgb.g = 0.f;
			light->rgb.b = .8f;
		}

		light->pos = _ePos;
		light->duration = 1500;
	}

	arx_assert(pParticleManager);
	pParticleManager->AddSystem(pPS);

	ARX_SOUND_PlaySFX(SND_SPELL_MM_HIT, &_ePos);
}

CMagicMissile::CMagicMissile()
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
	, angles()
	, tex_mm()
	, snd_loop()
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

	lLightId = InvalidLightHandle;

	ARX_SOUND_Stop(snd_loop);
}

void CMagicMissile::Create(const Vec3f & aeSrc, const Anglef & angles)
{
	SetDuration(ulDuration);
	
	float fBetaRad = radians(angles.getPitch());
	float fBetaRadCos = (float) cos(fBetaRad);
	float fBetaRadSin = (float) sin(fBetaRad);
	
	this->angles = angles;
	eCurPos = eSrc = aeSrc;

	Vec3f e = eSrc;

	int i = 40;
	e.x -= fBetaRadSin * 50 * i;
	e.y += sin(radians(MAKEANGLE(this->angles.getYaw()))) * 50 * i;
	e.z += fBetaRadCos * 50 * i;

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
	
	lLightId = InvalidLightHandle;
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

	// Set Appropriate Renderstates
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	// Set Texture
	if(tex_mm) {
		if(spells[spellinstance]->m_caster == PlayerEntityHandle && cur_mr == 3)
			GRenderer->ResetTexture(0);
		else
			GRenderer->SetTexture(0, tex_mm);
	}

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
				Draw3DLineTexNew(lastpos, newpos, color, color, fs, fe);
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
	stiteangle.setPitch(-degrees(bubu));
	stiteangle.setYaw(0);
	stiteangle.setRoll(-(degrees(bubu1)));

	if(av.x < 0)
		stiteangle.setRoll(stiteangle.getRoll() - 90);

	if(av.x > 0)
		stiteangle.setRoll(stiteangle.getRoll() + 90);

	if(stiteangle.getRoll() < 0)
		stiteangle.setRoll(stiteangle.getRoll() + 360.0f);

	Color3f stitecolor;
	if(spells[spellinstance]->m_caster == PlayerEntityHandle && cur_mr == 3) {
		stitecolor.r = 1.f;
		stitecolor.g = 0.f;
		stitecolor.b = 0.2f;
	} else {
		stitecolor.r = 0.3f;
		stitecolor.g = 0.3f;
		stitecolor.b = 0.5f;
	}

	Vec3f stitescale = Vec3f_ONE;

	Draw3DObject(smissile, stiteangle, stitepos, stitescale, stitecolor);

	eCurPos = lastpos;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CMultiMagicMissile::CMultiMagicMissile(size_t nbmissiles, SpellHandle spellHandle)
	: CSpellFx()
	, spellinstance(spellHandle)
{
	SetDuration(2000);
	
	pTab.reserve(nbmissiles);
	
	for(size_t i = 0; i < nbmissiles; i++) {
		CMagicMissile * missile = new CMagicMissile();
		missile->spellinstance = spellinstance;
		
		pTab.push_back(missile);
	}
}

CMultiMagicMissile::~CMultiMagicMissile()
{
	for(size_t i = 0; i < pTab.size(); i++) {
		// no need to kill it because it's a duration light !
		pTab[i]->lLightId = InvalidLightHandle;

		delete pTab[i];
	}

	pTab.clear();
}

void CMultiMagicMissile::Create()
{
	
	long lMax = 0;
	
	SpellBase * spell = spells[spellinstance];
	
	spell->m_hand_group = GetActionPointIdx(entities[spell->m_caster]->obj, "primary_attach");
	
	if(spell->m_hand_group != -1) {
		Entity * caster = entities[spell->m_caster];
		long group = spell->m_hand_group;
		spell->m_hand_pos = caster->obj->vertexlist3[group].v;
	}
	
	Vec3f aePos;
	float afAlpha, afBeta;
	if(spell->m_caster == PlayerEntityHandle) {
		afBeta = player.angle.getPitch();
		afAlpha = player.angle.getYaw();
		Vec3f vector;
		vector.x = -std::sin(radians(afBeta)) * std::cos(radians(afAlpha)) * 60.f;
		vector.y = std::sin(radians(afAlpha)) * 60.f;
		vector.z = std::cos(radians(afBeta)) * std::cos(radians(afAlpha)) * 60.f;
		
		if(spell->m_hand_group != -1) {
			aePos = spell->m_hand_pos + vector;
		} else {
			aePos.x = player.pos.x - std::sin(radians(afBeta)) + vector.x; 
			aePos.y = player.pos.y + vector.y; //;
			aePos.z = player.pos.z + std::cos(radians(afBeta)) + vector.z; 
		}
	} else {
		afAlpha = 0;
		afBeta = entities[spell->m_caster]->angle.getPitch();
		Vec3f vector;
		vector.x = -std::sin(radians(afBeta)) * std::cos(radians(afAlpha)) * 60;
		vector.y =  std::sin(radians(afAlpha)) * 60;
		vector.z =  std::cos(radians(afBeta)) * std::cos(radians(afAlpha)) * 60;
		
		if(spell->m_hand_group != -1) {
			aePos = spell->m_hand_pos + vector;
		} else {
			aePos = entities[spell->m_caster]->pos + vector;
		}
		
		Entity * io = entities[spell->m_caster];
		
		if(ValidIONum(io->targetinfo)) {
			Vec3f * p1 = &spell->m_caster_pos;
			Vec3f * p2 = &entities[io->targetinfo]->pos;
			afAlpha = -(degrees(getAngle(p1->y, p1->z, p2->y, p2->z + glm::distance(Vec2f(p2->x, p2->z), Vec2f(p1->x, p1->z))))); //alpha entre orgn et dest;
		} else if (ValidIONum(spell->m_target)) {
			Vec3f * p1 = &spell->m_caster_pos;
			Vec3f * p2 = &entities[spell->m_target]->pos;
			afAlpha = -(degrees(getAngle(p1->y, p1->z, p2->y, p2->z + glm::distance(Vec2f(p2->x, p2->z), Vec2f(p1->x, p1->z))))); //alpha entre orgn et dest;
		}
	}
	
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
		
		if(spell->m_caster == PlayerEntityHandle && cur_mr == 3) {
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
			
			if(spell->m_caster == PlayerEntityHandle && cur_mr == 3) {
				el->rgb.r = 1;
				el->rgb.g = 0.3f;
				el->rgb.b = 0.8f;
			} else {
				el->rgb.r = 0;
				el->rgb.g = 0;
				el->rgb.b = 1;
			}
			
			el->pos	 = pMM->eSrc;
			el->duration = 300;
		}
	}
	
	SetDuration(lMax + 1000);
}

void CMultiMagicMissile::CheckCollision()
{
	for(size_t i = 0; i < pTab.size(); i++) {
		CMagicMissile * missile = (CMagicMissile *) pTab[i];
		
		if(missile->bExplo)
			continue;
			
		Sphere sphere;
		sphere.origin = missile->eCurPos;
		sphere.radius	= 10.f;
		
		if(spellinstance != SpellHandle(-1) && (CheckAnythingInSphere(sphere, spells[spellinstance]->m_caster, CAS_NO_SAME_GROUP)))
		{
			LaunchMagicMissileExplosion(missile->eCurPos, spellinstance);
			ARX_NPC_SpawnAudibleSound(missile->eCurPos, entities[spells[spellinstance]->m_caster]);
			
			missile->SetTTL(1000);
			missile->bExplo = true;
			missile->bMove  = false;
			
			missile->lLightId = InvalidLightHandle;
			
			DamageParameters damage;
			damage.pos = missile->eCurPos;
			damage.radius = 80.f;
			damage.damages = (4 + spells[spellinstance]->m_level * ( 1.0f / 5 )) * .8f;
			damage.area	= DAMAGE_FULL;
			damage.duration = -1;
			damage.source = spells[spellinstance]->m_caster;
			damage.flags = DAMAGE_FLAG_DONT_HURT_SOURCE;
			damage.type = DAMAGE_TYPE_MAGICAL;
			DamageCreate(damage);
			
			Color3f rgb(.3f, .3f, .45f);
			ARX_PARTICLES_Add_Smoke(&missile->eCurPos, 0, 6, &rgb);
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

//-----------------------------------------------------------------------------
// IGNIT
//-----------------------------------------------------------------------------
CIgnit::CIgnit()
	: pos(Vec3f_ZERO)
	, m_active(false)
	, duration(0)
	, currduration(0)
	, rgb(0.f, 0.f, 0.f)
{}

CIgnit::~CIgnit()
{
	Kill();
}

void CIgnit::Kill(void)
{
	
	std::vector<T_LINKLIGHTTOFX>::iterator itr;
	for(itr = tablight.begin(); itr != tablight.end(); ++itr) {
		lightHandleDestroy(itr->idl);
	}
	
	tablight.clear();
}

void CIgnit::Create(Vec3f * posc, int speed)
{
	pos = *posc;
	tablight.clear();
	duration = speed;
	currduration = 0;
	m_active = true;
	
	rgb = Color3f(1.f, 1.f, 1.f);
}

void CIgnit::Action(bool enable)
{
	std::vector<T_LINKLIGHTTOFX>::const_iterator itr;
	for(itr = tablight.begin(); itr != tablight.end(); ++itr) {
		GLight[itr->iLightNum]->status = enable;

		if(enable) {
			ARX_SOUND_PlaySFX(SND_SPELL_IGNITE, &itr->poslight);
		} else {
			ARX_SOUND_PlaySFX(SND_SPELL_DOUSE, &itr->poslight);
		}
	}
}

void CIgnit::AddLight(int aiLight)
{
	if(arxtime.is_paused())
		return;
	
	T_LINKLIGHTTOFX entry;
	
	entry.iLightNum = aiLight;
	entry.poslight = GLight[aiLight]->pos;

	entry.idl = GetFreeDynLight();

	if(lightHandleIsValid(entry.idl)) {
		EERIE_LIGHT * light = lightHandleGet(entry.idl);
		
		light->intensity = 0.7f + 2.f * rnd();
		light->fallend = 400.f;
		light->fallstart = 300.f;
		light->rgb = rgb;
		light->pos = entry.poslight;
	}

	tablight.push_back(entry);
}

void CIgnit::Update(float timeDelta) 
{
	if(currduration >= duration) {
		m_active = false;
	}

	if(m_active) {
		float a = (((float)currduration)) / ((float)duration);
		
		if(a >= 1.f)
			a = 1.f;
		
		std::vector<T_LINKLIGHTTOFX>::iterator itr;
		for(itr = tablight.begin(); itr != tablight.end(); ++itr) {
		
				itr->posfx = pos + (itr->poslight - pos) * a;
				
				LightHandle id = itr->idl;
				
				if(lightHandleIsValid(id)) {
					EERIE_LIGHT * light = lightHandleGet(id);
					
					light->intensity = 0.7f + 2.f * rnd();
					light->pos = itr->posfx;
				}
		}
	}
	
	if(!arxtime.is_paused())
		currduration += timeDelta;
}

void CDoze::CreateDoze(Vec3f * posc, int speed) {
	Create(posc, speed);
	
	rgb = Color3f(0.f, .7f, 1.f);
}

void CDoze::AddLightDoze(int aiLight)
{
	if(arxtime.is_paused())
		return;
	
	T_LINKLIGHTTOFX entry;
	
	entry.iLightNum = aiLight;
	entry.poslight = GLight[aiLight]->pos;
	entry.idl = InvalidLightHandle;

	tablight.push_back(entry);
}

void CIgnit::Render() {

}

//-----------------------------------------------------------------------------
//							PORTALS
//-----------------------------------------------------------------------------
void Split(Vec3f * v, int a, int b, float yo)
{
	if(a != b) {
		int i = (int)((a + b) * 0.5f);

		if(i != a && i != b) {
			v[i].x = (v[a].x + v[b].x) * 0.5f + yo * frand2(); 
			v[i].y = (v[a].y + v[b].y) * 0.5f; 
			v[i].z = (v[a].z + v[b].z) * 0.5f + yo * frand2();
			Split(v, a, i, yo * 0.7f);
			Split(v, i, b, yo * 0.7f);
		}
	}
}
