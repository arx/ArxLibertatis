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

void LaunchMagicMissileExplosion(Vec3f & _ePos, int t = 0, long spellinstance = -1)
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

	if(spellinstance >= 0 && spells[spellinstance].caster == 0 && cur_mr == 3) {
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

	pPS->SetParams(cp);
	pPS->ulParticleSpawn = 0;
	
	Vec3f eP = _ePos;
	
	pPS->SetPos(eP);
	pPS->Update(0);
	pPS->iParticleNbMax = 0;

	long id = GetFreeDynLight();

	if(id != -1) {
		DynLight[id].exist = 1;
		DynLight[id].intensity = 2.3f;
		DynLight[id].fallstart = 250.f;
		DynLight[id].fallend   = 420.f;

		if(spellinstance >= 0 && spells[spellinstance].caster == 0 && cur_mr == 3) {
			DynLight[id].rgb.r = 1.f;
			DynLight[id].rgb.g = 0.3f;
			DynLight[id].rgb.b = .8f;
		} else {
			DynLight[id].rgb.r = 0.f;
			DynLight[id].rgb.g = 0.f;
			DynLight[id].rgb.b = .8f;
		}

		DynLight[id].pos = eP;
		DynLight[id].duration = 1500;
	}

	if(pParticleManager)
		pParticleManager->AddSystem(pPS);

	ARX_SOUND_PlaySFX(SND_SPELL_MM_HIT, &_ePos);
}

CMagicMissile::CMagicMissile() :
	CSpellFx(),
	eSrc(Vec3f_ZERO),
	fColor(Color3f::white)
{
	SetDuration(2000);
	ulCurrentTime = ulDuration + 1;

	tex_mm = TextureContainer::Load("graph/obj3d/textures/(fx)_bandelette_blue");

	if(!smissile)
		smissile = LoadTheObj("graph/obj3d/interactive/fix_inter/fx_magic_missile/fx_magic_missile.teo");

	smissile_count++;

	bExplo = false;
	bMove = true;
}

CMagicMissile::~CMagicMissile()
{
	smissile_count--;

	if(smissile && smissile_count <= 0) {
		smissile_count = 0;
		delete smissile;
		smissile = NULL;
	}

	if(this->lLightId != -1) {
		this->lLightId = -1;
	}

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

	// Light
	if(lLightId != -1) {
		lLightId = -1;
	}
}

void CMagicMissile::Update(unsigned long aulTime)
{
	ARX_SOUND_RefreshPosition(snd_loop, &eCurPos);

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
		if(spells[spellinstance].caster == 0 && cur_mr == 3)
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
	if(spells[spellinstance].caster == 0 && cur_mr == 3) {
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
		DrawEERIEObjEx(smissile, &stiteangle, &stitepos, &stitescale, stitecolor);

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
			if(pTab[i]->lLightId != -1) {
				// no need to kill it because it's a duration light !
				pTab[i]->lLightId = -1;
			}

			delete pTab[i];
		}
	}

	delete [] pTab;
}

void CMultiMagicMissile::Create()
{
	
	long lMax = 0;

	if(pTab) {
		spells[spellinstance].hand_group = GetActionPointIdx(entities[spells[spellinstance].caster]->obj, "primary_attach");
		
		if(spells[spellinstance].hand_group != -1) {
			Entity * caster = entities[spells[spellinstance].caster];
			long group = spells[spellinstance].hand_group;
			spells[spellinstance].hand_pos = caster->obj->vertexlist3[group].v;
		}
		
		Vec3f aePos;
		float afAlpha, afBeta;
		if(spells[spellinstance].caster == 0) { // player
			afBeta = player.angle.getPitch();
			afAlpha = player.angle.getYaw();
			Vec3f vector;
			vector.x = -EEsin(radians(afBeta)) * EEcos(radians(afAlpha)) * 60.f;
			vector.y = EEsin(radians(afAlpha)) * 60.f;
			vector.z = EEcos(radians(afBeta)) * EEcos(radians(afAlpha)) * 60.f;

			if(spells[spellinstance].hand_group != -1) {
				aePos = spells[spellinstance].hand_pos + vector;
			} else {
				aePos.x = player.pos.x - EEsin(radians(afBeta)) + vector.x; 
				aePos.y = player.pos.y + vector.y; //;
				aePos.z = player.pos.z + EEcos(radians(afBeta)) + vector.z; 
			}
		} else {
			afAlpha = 0;
			afBeta = entities[spells[spellinstance].caster]->angle.getPitch();
			Vec3f vector;
			vector.x = -EEsin(radians(afBeta)) * EEcos(radians(afAlpha)) * 60;
			vector.y = EEsin(radians(afAlpha)) * 60;
			vector.z = EEcos(radians(afBeta)) * EEcos(radians(afAlpha)) * 60;

			if(spells[spellinstance].hand_group != -1) {
				aePos = spells[spellinstance].hand_pos + vector;
			} else {
				aePos = entities[spells[spellinstance].caster]->pos + vector;
			}

			Entity * io = entities[spells[spellinstance].caster];

			if(ValidIONum(io->targetinfo)) {
				Vec3f * p1 = &spells[spellinstance].caster_pos;
				Vec3f * p2 = &entities[io->targetinfo]->pos;
				afAlpha = -(degrees(getAngle(p1->y, p1->z, p2->y, p2->z + glm::distance(Vec2f(p2->x, p2->z), Vec2f(p1->x, p1->z))))); //alpha entre orgn et dest;
			} else if (ValidIONum(spells[spellinstance].target)) {
				Vec3f * p1 = &spells[spellinstance].caster_pos;
				Vec3f * p2 = &entities[spells[spellinstance].target]->pos;
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

				if(spells[spellinstance].caster == 0 && cur_mr == 3) {
					pMM->SetColor(Color3f(0.9f, 0.2f, 0.5f));
				} else {
					pMM->SetColor(Color3f(0.9f + rnd() * 0.1f, 0.9f + rnd() * 0.1f, 0.7f + rnd() * 0.3f));
				}

				pTab[i]->lLightId = GetFreeDynLight();

				if(pTab[i]->lLightId != -1) {
					EERIE_LIGHT * el = &DynLight[pTab[i]->lLightId];
					el->exist		= 1;
					el->intensity	= 0.7f + 2.3f;
					el->fallend		= 190.f;
					el->fallstart	= 80.f;

					if(spells[spellinstance].caster == 0 && cur_mr == 3) {
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

					if(spellinstance != -1 && (CheckAnythingInSphere(&sphere, spells[spellinstance].caster, CAS_NO_SAME_GROUP)))
					{
						LaunchMagicMissileExplosion(pMM->eCurPos, 0, spellinstance);
						ARX_NPC_SpawnAudibleSound(&pMM->eCurPos, entities[spells[spellinstance].caster]);

						pMM->SetTTL(1000);
						pMM->bExplo = true;
						pMM->bMove  = false;

						if(pMM->lLightId != -1)
							pMM->lLightId = -1;

						long ttt = ARX_DAMAGES_GetFree();

						if(ttt != -1) {
							damages[ttt].pos = pMM->eCurPos;
							damages[ttt].radius	= 80.f;
							damages[ttt].damages = (4 + spells[spellinstance].caster_level * ( 1.0f / 5 )) * .8f; 
							damages[ttt].area	= DAMAGE_FULL;
							damages[ttt].duration = -1;
							damages[ttt].source	= spells[spellinstance].caster;
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

				if(pMM->lLightId != -1) {
					EERIE_LIGHT * el	= &DynLight[pMM->lLightId];
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
{
}

CIgnit::~CIgnit()
{
	this->Kill();
}

void CIgnit::Kill(void)
{
	int nb = this->nblight;

	while(nb--) {
		int id = this->tablight[nb].idl;

		if(ValidDynLight(id))
			DynLight[id].exist = 0;

		this->tablight[nb].idl = -1;
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
			ARX_SOUND_PlaySFX(SND_SPELL_IGNITE, &spells[spellinstance].caster_pos);
		} else if(aiMode == 0) {
			ARX_SOUND_PlaySFX(SND_SPELL_DOUSE, &spells[spellinstance].caster_pos);
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

	if(this->tablight[this->nblight].idl > 0) {
		int id = this->tablight[this->nblight].idl;
		EERIE_LIGHT * el = &DynLight[id];
		el->exist = 1;
		el->intensity = 0.7f + 2.f * rnd();
		el->fallend = 400.f;
		el->fallstart = 300.f;
		el->rgb = rgb;
		el->pos = this->tablight[this->nblight].poslight;
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

					int id = this->tablight[nb].idl;

					if(id > 0) {
						DynLight[id].intensity = 0.7f + 2.f * rnd();
						DynLight[id].pos = this->tablight[nb].posfx;
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

void GenereArcElectrique(Vec3f * pos, Vec3f * end, Vec3f * tabdef, int nbseg)
{
	tabdef[0] = *pos;

	tabdef[nbseg-1] = *end;

	Split(tabdef, 0, nbseg - 1, 20);
}

void DrawArcElectrique(Vec3f * tabdef, int nbseg, TextureContainer * tex, float fBeta, int tsp)
{
	TexturedVertex v[4];
	TexturedVertex v2[4];

	//-------------------------------------------------------------------------
	// rendu
	//	GRenderer->ResetTexture(0);
	GRenderer->SetCulling(Renderer::CullNone);

	GRenderer->SetTexture(0, tex);
	
	v2[0].color = v2[1].color = v2[2].color = v2[3].color = Color::grayb(tsp).toBGR();
	
	for(long i = 0; i < nbseg - 2; i++) {
		
		Vec3f astart = tabdef[i];
		Vec3f a = tabdef[i + 1];
		
		// version 2 faces
		v2[0].uv = Vec2f_ZERO;
		v2[1].uv = Vec2f_X_AXIS;
		v2[2].uv = Vec2f_ONE;
		v2[3].uv = Vec2f_Y_AXIS;
		
		Vec3f d(0.f, 5.f, 0.f);
		v[0].p = astart + d;
		v[1].p = astart - d;
		v[2].p = a - d;
		v[3].p = a + d;
		EE_RT2(&v[0], &v2[0]);
		EE_RT2(&v[1], &v2[1]);
		EE_RT2(&v[2], &v2[2]);
		EE_RT2(&v[3], &v2[3]);
		ARX_DrawPrimitive(&v2[0], &v2[1], &v2[2]);
		ARX_DrawPrimitive(&v2[0], &v2[2], &v2[3]);
		
		d = Vec3f(5.f * (float)cos(radians(fBeta)), 0.f, 5.f * (float)sin(radians(fBeta)));
		v[0].p = astart + d;
		v[1].p = astart - d;
		v[2].p = a - d;
		v[3].p = a + d;
		EE_RT2(&v[0], &v2[0]);
		EE_RT2(&v[1], &v2[1]);
		EE_RT2(&v[2], &v2[2]);
		EE_RT2(&v[3], &v2[3]);
		ARX_DrawPrimitive(&v2[0], &v2[1], &v2[2]);
		ARX_DrawPrimitive(&v2[0], &v2[2], &v2[3]);
	}
}

CPortal::~CPortal() {
	
	free(sphereind);
	free(spherevertex);
	free(sphered3d);
	
	int nb = 256;
	while(nb--) {
		free(tabeclair[nb].seg);
	}
}

void CPortal::AddNewEclair(Vec3f * endpos, int nbseg, int duration, int numpt)
{
	if(arxtime.is_paused())
		return;

	int	nb = 256;

	if(this->nbeclair > 255 || nbseg > 256)
		return;

	short sNbSeg = static_cast<short>(nbseg);

	while(nb--) {
		if(!this->tabeclair[nb].actif) {
			this->nbeclair++;
			this->tabeclair[nb].actif = 1;
			this->tabeclair[nb].duration = duration;
			this->tabeclair[nb].currduration = 0;
			this->tabeclair[nb].nbseg = sNbSeg;
			this->tabeclair[nb].numpt = numpt;

			GenereArcElectrique(&this->pos, endpos, this->tabeclair[nb].seg, this->tabeclair[nb].nbseg);
			break;
		}
	}
}

void CPortal::DrawAllEclair()
{
	int nb = 256;

	while(nb--){
		if(this->tabeclair[nb].actif) {
			float a = 1.f - ((float)this->tabeclair[nb].currduration / (float)this->tabeclair[nb].duration);

			if(a < 0.f)
				a = 0.f;

			DrawArcElectrique(this->tabeclair[nb].seg, this->tabeclair[nb].nbseg, this->te, rnd() * 360.f, (int)(255.f * a));

			if(!arxtime.is_paused())
				this->tabeclair[nb].currduration += this->currframe;

			if(this->tabeclair[nb].currduration >= this->tabeclair[nb].duration) {
				this->tabeclair[nb].actif = 0;
				this->nbeclair--;
			}
		}
	}
}

void CPortal::Update(unsigned long _ulTime)
{
	float a;

	switch(this->key) {
		case 0:
			a = (float)this->currduration / (float)this->duration;

			if(a > 1.f) {
				a = 1.f;
				this->key++;
			}

			this->pos = this->sphereposdep + (this->sphereposend - this->sphereposdep) * a;
			this->spherealpha = a * .5f;

			if(!arxtime.is_paused())
				this->currduration += _ulTime;

			break;
		case 1:
			this->spherealpha = 0.5f + rnd();

			if(this->spherealpha > 1.f)
				this->spherealpha = 1.f;

			this->spherealpha *= .25f;

			//getion eclair dans boule
			this->currframe = _ulTime;

			if(!arxtime.is_paused())
				this->timeneweclair -= _ulTime;

			if(this->timeneweclair <= 0) {
				this->timeneweclair = Random::get(100, 300);

				int	numpt = Random::get(0, this->spherenbpt - 1);
				Vec3f endpos = this->spherevertex[numpt] + this->pos;

				this->AddNewEclair(&endpos, 32, Random::get(1000, 2000), numpt);
			}

			break;
	}

	if(this->lLightId >= 0) {
		DynLight[this->lLightId].pos = this->pos;
		DynLight[this->lLightId].intensity = 0.7f + 2.f * rnd();
	}
}

void CPortal::Render()
{
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);

	//calcul sphere
	int			nb = this->spherenbpt;
	TexturedVertex * v = this->sphered3d, d3dvs;
	Vec3f	* pt = this->spherevertex;
	int col = Color(0, (int)(200.f * this->spherealpha), (int)(255.f * this->spherealpha)).toBGRA();

	while(nb) {
		d3dvs.p.x = pt->x + this->pos.x;	//pt du bas
		d3dvs.p.y = pt->y + this->pos.y;
		d3dvs.p.z = pt->z + this->pos.z;
		EE_RTP(&d3dvs, v);

		if (!arxtime.is_paused()) v->color = col;

		v++;
		pt++;
		nb--;
	}

	//update les couleurs aux impacts
	nb = 256;

	while(nb--) {
		if(this->tabeclair[nb].actif) {
			float a = 1.f - ((float)this->tabeclair[nb].currduration / (float)this->tabeclair[nb].duration);

			if(a < 0.f)
				a = 0.f;

			if(this->tabeclair[nb].numpt >= 0) {
				int r = (int)((0.f + (255.f - 0.f) * a) * this->spherealpha * 3.f);

				if (r > 255) r = 255;

				int g = (int)((200.f + (255.f - 200.f) * a) * this->spherealpha * 3.f);

				if (g > 255) g = 255;

				int b = (int)(255.f * this->spherealpha * 3.f);

				if (b > 255) b = 255;

				if(!arxtime.is_paused())
					this->sphered3d[this->tabeclair[nb].numpt].color = Color(r, g, b).toBGRA();
			}
		}
	}

	//affichage de la sphere back
	GRenderer->SetCulling(Renderer::CullCW);
	GRenderer->ResetTexture(0);
	GRenderer->drawIndexed(Renderer::TriangleList, this->sphered3d, this->spherenbpt, this->sphereind, this->spherenbfaces * 3);
	
	// affichage eclair
	DrawAllEclair();
	
	// affichage des particules à l'interieur
	if(rnd() > .25f) {
		
		PARTICLE_DEF * pd = createParticle();
		if(pd) {
			float a = radians(rnd() * 360.f);
			float b = radians(rnd() * 360.f);
			float rr = r * (rnd() + .25f) * 0.05f;
			pd->ov = pos;
			pd->move = Vec3f(EEsin(a) * EEcos(b), EEcos(a), EEsin(a) * EEsin(b)) * rr;
			pd->siz = 10.f;
			pd->tolive = Random::get(1000, 2000);
			pd->tc = tp;
			pd->special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
			pd->fparam = 0.0000001f;
		}
	}
	
	// affichage de la sphere front
	GRenderer->SetCulling(Renderer::CullCCW);
	GRenderer->ResetTexture(0);
	GRenderer->drawIndexed(Renderer::TriangleList, this->sphered3d, this->spherenbpt, this->sphereind, this->spherenbfaces * 3);

	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendZero);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
}
