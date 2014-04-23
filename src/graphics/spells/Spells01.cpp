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

void LaunchMagicMissileExplosion(const Vec3f & _ePos, int t = 0, long spellinstance = -1)
{
	// système de partoches pour l'explosion
	ParticleSystem * pPS = new ParticleSystem();
	ParticleParams cp;
	memset(&cp, 0, sizeof(cp));
	cp.iNbMax = 100 + t * 50;
	cp.fLife = 1500;
	cp.fLifeRandom = 0;
	cp.p3Pos = Vec3f(10.f);
	cp.p3Direction = Vec3f(0.f, -10.f, 0.f);
	cp.fAngle = radians(360);
	cp.fSpeed = 130;
	cp.fSpeedRandom = 100;
	cp.p3Gravity = Vec3f(0.f, 10.f, 0.f);
	cp.fFlash = 0;
	cp.fRotation = 16;

	cp.fStartSize = 5;
	cp.fStartSizeRandom = 10;


	cp.fEndSize = 0;
	cp.fEndSizeRandom = 2;

	if(spellinstance >= 0 && spells[spellinstance].m_caster == 0 && cur_mr == 3) {
		cp.fStartSize = 20;
		cp.fSpeed = 13;
		cp.fSpeedRandom = 10;
		cp.fStartColorRandom[0] = 0;
		cp.fStartColorRandom[1] = 0;
		cp.fStartColorRandom[2] = 0;
		cp.fStartColorRandom[3] = 0;

		cp.fStartColor[0] = 0;
		cp.fStartColor[1] = 0;
		cp.fStartColor[2] = 0;
		cp.fStartColor[3] = 0;
		cp.fEndColor[0] = 255;
		cp.fEndColor[1] = 40;
		cp.fEndColor[2] = 120;
		cp.fEndColor[3] = 10;//55;
		pPS->SetTexture("graph/particles/(fx)_mr", 0, 500);
	} else {
		cp.fStartColorRandom[0] = 100;
		cp.fStartColorRandom[1] = 100;
		cp.fStartColorRandom[2] = 100;
		cp.fStartColorRandom[3] = 100;

		cp.fStartColor[0] = 110;
		cp.fStartColor[1] = 110;
		cp.fStartColor[2] = 110;
		cp.fStartColor[3] = 110;
		cp.fEndColor[0] = 0;
		cp.fEndColor[1] = 0;
		cp.fEndColor[2] = 120;
		cp.fEndColor[3] = 10;
		pPS->SetTexture("graph/particles/magicexplosion", 0, 500);
	}

	cp.fEndColorRandom[0] = 50;
	cp.fEndColorRandom[1] = 50;
	cp.fEndColorRandom[2] = 50;
	cp.fEndColorRandom[3] = 50;

	cp.blendMode = RenderMaterial::Additive;

	pPS->SetParams(cp);
	pPS->ulParticleSpawn = 0;
	
	Vec3f eP = _ePos;
	
	pPS->SetPos(eP);
	pPS->Update(0);
	pPS->iParticleNbMax = 0;

	LightHandle id = GetFreeDynLight();

	if(lightHandleIsValid(id)) {
		EERIE_LIGHT * light = lightHandleGet(id);
		
		light->intensity = 2.3f;
		light->fallstart = 250.f;
		light->fallend   = 420.f;

		if(spellinstance >= 0 && spells[spellinstance].m_caster == 0 && cur_mr == 3) {
			light->rgb.r = 1.f;
			light->rgb.g = 0.3f;
			light->rgb.b = .8f;
		} else {
			light->rgb.r = 0.f;
			light->rgb.g = 0.f;
			light->rgb.b = .8f;
		}

		light->pos = eP;
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

	lLightId = -1;

	ARX_SOUND_Stop(snd_loop);
}

void CMagicMissile::Create(const Vec3f & aeSrc, const Anglef & angles)
{
	SetDuration(ulDuration);
	SetAngle(angles.getPitch());

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
	
	lLightId = -1;
}

void CMagicMissile::Update(unsigned long aulTime)
{
	ARX_SOUND_RefreshPosition(snd_loop, eCurPos);

	ulCurrentTime += aulTime;

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
		if(spells[spellinstance].m_caster == 0 && cur_mr == 3)
			GRenderer->ResetTexture(0);
		else
			GRenderer->SetTexture(0, tex_mm);
	}

	if(bMove)
		fTrail = (ulCurrentTime * fOneOnDuration) * (iBezierPrecision + 2) * 5;
	
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
				Draw3DLineTex(lastpos, newpos, color, fs, fe);
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
	if(spells[spellinstance].m_caster == 0 && cur_mr == 3) {
		stitecolor.r = 1.f;
		stitecolor.g = 0.f;
		stitecolor.b = 0.2f;
	} else {
		stitecolor.r = 0.3f;
		stitecolor.g = 0.3f;
		stitecolor.b = 0.5f;
	}

	Vec3f stitescale = Vec3f_ONE;

	if(smissile)
		DrawEERIEObjEx(smissile, stiteangle, stitepos, stitescale, stitecolor);

	eCurPos = lastpos;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CMultiMagicMissile::CMultiMagicMissile(long nbmissiles) : CSpellFx()
{
	SetDuration(2000);
	uiNumber = nbmissiles;
	pTab = NULL;
	pTab = new CMagicMissile*[uiNumber]();

	if(pTab) {
		for(unsigned int i = 0; i < uiNumber; i++) {
			pTab[i] = NULL;
			pTab[i] = new CMagicMissile();
			pTab[i]->spellinstance = this->spellinstance;
		}
	}
}

CMultiMagicMissile::~CMultiMagicMissile()
{
	for(unsigned int i = 0; i < uiNumber; i++) {
		if(pTab[i]) {
			// no need to kill it because it's a duration light !
			pTab[i]->lLightId = -1;

			delete pTab[i];
		}
	}

	delete [] pTab;
}

void CMultiMagicMissile::Create()
{
	
	long lMax = 0;

	if(pTab) {
		spells[spellinstance].m_hand_group = GetActionPointIdx(entities[spells[spellinstance].m_caster]->obj, "primary_attach");
		
		if(spells[spellinstance].m_hand_group != -1) {
			Entity * caster = entities[spells[spellinstance].m_caster];
			long group = spells[spellinstance].m_hand_group;
			spells[spellinstance].m_hand_pos = caster->obj->vertexlist3[group].v;
		}
		
		Vec3f aePos;
		float afAlpha, afBeta;
		if(spells[spellinstance].m_caster == 0) { // player
			afBeta = player.angle.getPitch();
			afAlpha = player.angle.getYaw();
			Vec3f vector;
			vector.x = -std::sin(radians(afBeta)) * std::cos(radians(afAlpha)) * 60.f;
			vector.y = std::sin(radians(afAlpha)) * 60.f;
			vector.z = std::cos(radians(afBeta)) * std::cos(radians(afAlpha)) * 60.f;

			if(spells[spellinstance].m_hand_group != -1) {
				aePos = spells[spellinstance].m_hand_pos + vector;
			} else {
				aePos.x = player.pos.x - std::sin(radians(afBeta)) + vector.x; 
				aePos.y = player.pos.y + vector.y; //;
				aePos.z = player.pos.z + std::cos(radians(afBeta)) + vector.z; 
			}
		} else {
			afAlpha = 0;
			afBeta = entities[spells[spellinstance].m_caster]->angle.getPitch();
			Vec3f vector;
			vector.x = -std::sin(radians(afBeta)) * std::cos(radians(afAlpha)) * 60;
			vector.y =  std::sin(radians(afAlpha)) * 60;
			vector.z =  std::cos(radians(afBeta)) * std::cos(radians(afAlpha)) * 60;

			if(spells[spellinstance].m_hand_group != -1) {
				aePos = spells[spellinstance].m_hand_pos + vector;
			} else {
				aePos = entities[spells[spellinstance].m_caster]->pos + vector;
			}

			Entity * io = entities[spells[spellinstance].m_caster];

			if(ValidIONum(io->targetinfo)) {
				Vec3f * p1 = &spells[spellinstance].m_caster_pos;
				Vec3f * p2 = &entities[io->targetinfo]->pos;
				afAlpha = -(degrees(getAngle(p1->y, p1->z, p2->y, p2->z + glm::distance(Vec2f(p2->x, p2->z), Vec2f(p1->x, p1->z))))); //alpha entre orgn et dest;
			} else if (ValidIONum(spells[spellinstance].m_target)) {
				Vec3f * p1 = &spells[spellinstance].m_caster_pos;
				Vec3f * p2 = &entities[spells[spellinstance].m_target]->pos;
				afAlpha = -(degrees(getAngle(p1->y, p1->z, p2->y, p2->z + glm::distance(Vec2f(p2->x, p2->z), Vec2f(p1->x, p1->z))))); //alpha entre orgn et dest;
			}
		}

		for(unsigned int i = 0; i < uiNumber; i++) {
			if(pTab[i]) {
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

				if(spells[spellinstance].m_caster == 0 && cur_mr == 3) {
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

					if(spells[spellinstance].m_caster == 0 && cur_mr == 3) {
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
		}
	}

	SetDuration(lMax + 1000);
}

void CMultiMagicMissile::CheckCollision()
{
	if(!pTab)
		return;

		for(unsigned int i = 0; i < uiNumber; i++) {
			if(pTab[i]) {
				CMagicMissile * pMM = (CMagicMissile *) pTab[i];

				if(!pMM->bExplo) {
					
					EERIE_SPHERE sphere;
					sphere.origin = pMM->eCurPos;
					sphere.radius	= 10.f;

					if(spellinstance != -1 && (CheckAnythingInSphere(&sphere, spells[spellinstance].m_caster, CAS_NO_SAME_GROUP)))
					{
						LaunchMagicMissileExplosion(pMM->eCurPos, 0, spellinstance);
						ARX_NPC_SpawnAudibleSound(pMM->eCurPos, entities[spells[spellinstance].m_caster]);

						pMM->SetTTL(1000);
						pMM->bExplo = true;
						pMM->bMove  = false;

						pMM->lLightId = -1;

						long ttt = ARX_DAMAGES_GetFree();

						if(ttt != -1) {
							damages[ttt].pos = pMM->eCurPos;
							damages[ttt].radius	= 80.f;
							damages[ttt].damages = (4 + spells[spellinstance].m_caster_level * ( 1.0f / 5 )) * .8f; 
							damages[ttt].area	= DAMAGE_FULL;
							damages[ttt].duration = -1;
							damages[ttt].source	= spells[spellinstance].m_caster;
							damages[ttt].flags	= DAMAGE_FLAG_DONT_HURT_SOURCE;
							damages[ttt].type	= DAMAGE_TYPE_MAGICAL;
							damages[ttt].exist	= true;
						}

						Color3f rgb(.3f, .3f, .45f);
						ARX_PARTICLES_Add_Smoke(&pMM->eCurPos, 0, 6, &rgb);
					}
				}
			}
		}
}

bool CMultiMagicMissile::CheckAllDestroyed()
{
	if(!pTab)
		return true;

	long nbmissiles	= 0;

	for(unsigned int i = 0; i < uiNumber; i++) {
		CMagicMissile *pMM = pTab[i];
		if(pMM && pMM->bMove)
			nbmissiles++;
	}

	return nbmissiles == 0;
}

void CMultiMagicMissile::Update(unsigned long _ulTime)
{
	if(pTab) {
		for(unsigned int i = 0 ; i < uiNumber ; i++) {
			if(pTab[i]) {
				pTab[i]->Update(_ulTime);
			}
		}
	}
}

void CMultiMagicMissile::Render()
{ 
	if(pTab) {
		for(unsigned int i = 0; i < uiNumber; i++) {
			if(pTab[i]) {
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
	}
}

//-----------------------------------------------------------------------------
// IGNIT
//-----------------------------------------------------------------------------
CIgnit::CIgnit()
	: pos(Vec3f_ZERO)
	, perimetre(0.f)
	, key(0)
	, duration(0)
	, currduration(0)
	, interp(0.f)
	, tp(NULL)
	, rgb(0.f, 0.f, 0.f)
	, mask(0)
{}

CIgnit::~CIgnit()
{
	this->Kill();
}

void CIgnit::Kill(void)
{
	int nb = this->nblight;

	while(nb--) {
		lightHandleDestroy(tablight[nb].idl);
	}

	this->nblight = 0;
}

void CIgnit::Create(Vec3f * posc, float perim, int speed)
{
	this->pos = *posc;
	this->perimetre = perim;
	this->nblight = 0;
	this->duration = speed;
	this->currduration = 0;
	this->key = 0;

	int	nb = 256;

	while(nb--) {
		this->tablight[nb].actif = 0;
		this->tablight[nb].idl = -1;
	}

	this->ChangeTexture(TextureContainer::Load("graph/particles/fire_hit"));
	this->ChangeRGBMask(1.f, 1.f, 1.f, Color(255, 200, 0).toBGRA());
}

void CIgnit::Action(int aiMode)
{
	short sMode = checked_range_cast<short>(aiMode);

	for(int i = 0; i < nblight; i++) {
		GLight[tablight[i].iLightNum]->status = sMode;

		if(aiMode == 1) {
			ARX_SOUND_PlaySFX(SND_SPELL_IGNITE, &spells[spellinstance].m_caster_pos);
		} else if(aiMode == 0) {
			ARX_SOUND_PlaySFX(SND_SPELL_DOUSE, &spells[spellinstance].m_caster_pos);
		}
	}
}

void CIgnit::AddLight(int aiLight)
{
	if(arxtime.is_paused())
		return;

	this->tablight[this->nblight].actif = 1;
	this->tablight[this->nblight].iLightNum = aiLight;
	this->tablight[this->nblight].poslight = GLight[aiLight]->pos;

	this->tablight[this->nblight].idl = GetFreeDynLight();

	if(lightHandleIsValid(tablight[this->nblight].idl)) {
		EERIE_LIGHT * light = lightHandleGet(this->tablight[this->nblight].idl);
		
		light->intensity = 0.7f + 2.f * rnd();
		light->fallend = 400.f;
		light->fallstart = 300.f;
		light->rgb = rgb;
		light->pos = this->tablight[this->nblight].poslight;
	}

	this->nblight++;
}

void CIgnit::Update(unsigned long _ulTime) 
{
	float	a;
	int		nb;

	if(this->currduration >= this->duration) {
		this->key++;
	}

	switch(this->key) {
		case 0:
			a = (((float)this->currduration)) / ((float)this->duration);

			if(a >= 1.f)
				a = 1.f;

			nb = this->nblight;

			while(nb--) {
				if(this->tablight[nb].actif) {
					this->tablight[nb].posfx = this->pos + (this->tablight[nb].poslight - this->pos) * a;

					LightHandle id = this->tablight[nb].idl;

					if(lightHandleIsValid(id)) {
						EERIE_LIGHT * light = lightHandleGet(id);
						
						light->intensity = 0.7f + 2.f * rnd();
						light->pos = this->tablight[nb].posfx;
					}
				}
			}

			this->interp = a;
			break;
	}

	if(!arxtime.is_paused())
		this->currduration += _ulTime;
}

void CDoze::CreateDoze(Vec3f * posc, float perim, int speed) {
	this->Create(posc, perim, speed);
	this->ChangeTexture(TextureContainer::Load("graph/particles/doze_hit"));
	this->ChangeRGBMask(0.f, .7f, 1.f, 0xFF0000FF);
}

void CDoze::AddLightDoze(int aiLight)
{
	if(arxtime.is_paused())
		return;

	this->tablight[this->nblight].actif = 1;
	this->tablight[this->nblight].iLightNum = aiLight;
	this->tablight[this->nblight].poslight = GLight[aiLight]->pos;
	this->tablight[this->nblight].idl = -1;

	this->nblight++;
}

void CIgnit::Render() {
	
	switch(this->key) {
		case 0:

			if(this->currduration > this->duration)
				this->key++;

			if(!arxtime.is_paused()) {
				float unsuri = (1.f - this->interp);
				int nb = this->nblight;

				while(nb--) {
					if(this->tablight[nb].actif && rnd() > .5f) {
						createSphericalSparks(tablight[nb].posfx, rnd() * 20.f * unsuri, tp, rgb, mask);
					}
				}
			}

			break;
		default:
			break;
	}
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
