/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/spells/Spells01.h"

#include "animation/AnimationRender.h"

#include "core/GameTime.h"

#include "game/Spells.h"
#include "game/NPC.h"
#include "game/Damage.h"
#include "game/Player.h"

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
//-----------------------------------------------------------------------------
void LaunchMagicMissileExplosion(Vec3f & _ePos, int t = 0, long spellinstance = -1)
{
	// système de partoches pour l'explosion
	ParticleSystem * pPS = new ParticleSystem();
	ParticleParams cp;
	cp.iNbMax = 100 + t * 50;
	cp.fLife = 1500;
	cp.fLifeRandom = 0;
	cp.p3Pos.x = 10;
	cp.p3Pos.y = 10;
	cp.p3Pos.z = 10;
	cp.p3Direction.x = 0;
	cp.p3Direction.y = -10;
	cp.p3Direction.z = 0;
	cp.fAngle = radians(360);
	cp.fSpeed = 130;
	cp.fSpeedRandom = 100;
	cp.p3Gravity.x = 0;
	cp.p3Gravity.y = 10;
	cp.p3Gravity.z = 0;
	cp.fFlash = 0;
	cp.fRotation = 16;

	cp.fStartSize = 5;
	cp.fStartSizeRandom = 10;


	cp.fEndSize = 0;
	cp.fEndSizeRandom = 2;

	if ((spellinstance >= 0) && (spells[spellinstance].caster == 0) && (cur_mr == 3))
	{
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
	}
	else
	{
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


	Vec3f eP;
	eP.x = _ePos.x;
	eP.y = _ePos.y;
	eP.z = _ePos.z;

	pPS->SetPos(eP);
	pPS->Update(0);
	pPS->iParticleNbMax = 0;

	long id = GetFreeDynLight();

	if (id != -1)
	{
		DynLight[id].exist = 1;
		DynLight[id].intensity = 2.3f;
		DynLight[id].fallstart = 250.f;
		DynLight[id].fallend   = 420.f;

		if ((spellinstance >= 0) && (spells[spellinstance].caster == 0) && (cur_mr == 3))
		{
			DynLight[id].rgb.r = 1.f;
			DynLight[id].rgb.g = 0.3f;
			DynLight[id].rgb.b = .8f;
		}
		else
		{
			DynLight[id].rgb.r = 0.f;
			DynLight[id].rgb.g = 0.f;
			DynLight[id].rgb.b = .8f;
		}

		DynLight[id].pos.x = eP.x;
		DynLight[id].pos.y = eP.y;
		DynLight[id].pos.z = eP.z;
		DynLight[id].duration = 1500;
	}

	if (pParticleManager)
	{
		pParticleManager->AddSystem(pPS);
	}

	ARX_SOUND_PlaySFX(SND_SPELL_MM_HIT, &_ePos);
}

CMagicMissile::CMagicMissile() : CSpellFx(), fColor(Color3f::white), eSrc(Vec3f::ZERO) {
	
	SetDuration(2000);
	ulCurrentTime = ulDuration + 1;

	tex_mm = TextureContainer::Load("graph/obj3d/textures/(fx)_bandelette_blue");

	if (!smissile)
		smissile = _LoadTheObj("graph/obj3d/interactive/fix_inter/fx_magic_missile/fx_magic_missile.teo");

	smissile_count++;

	bExplo = false;
	bMove = true;
}

//-----------------------------------------------------------------------------
CMagicMissile::~CMagicMissile()
{
	smissile_count--;

	if (smissile && (smissile_count <= 0))
	{
		smissile_count = 0;
		delete smissile;
		smissile = NULL;
	}

	if (this->lLightId != -1)
	{
		this->lLightId = -1;
	}

	ARX_SOUND_Stop(snd_loop);
}

//-----------------------------------------------------------------------------
void CMagicMissile::Create(const Vec3f & aeSrc, const Anglef & angles)
{
	Vec3f s, e;

	SetDuration(ulDuration);
	SetAngle(angles.b);

	this->angles = angles;
	eCurPos = eSrc = aeSrc;

	s = eSrc;
	e = eSrc;

	int i = 40;
	e.x -= fBetaRadSin * 50 * i;
	e.y += sin(radians(MAKEANGLE(this->angles.a))) * 50 * i;
	e.z += fBetaRadCos * 50 * i;

	pathways[0].p.x = eSrc.x;
	pathways[0].p.y = eSrc.y;
	pathways[0].p.z = eSrc.z;
	pathways[5].p.x = e.x;
	pathways[5].p.y = e.y;
	pathways[5].p.z = e.z;
	Split(pathways, 0, 5, 50, 0.5f);

	for (i = 0; i < 6; i++)
	{
		if (pathways[i].p.y >= eSrc.y + 150)
		{
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

//-----------------------------------------------------------------------------
void CMagicMissile::SetTTL(unsigned long aulTTL)
{
	unsigned long t = ulCurrentTime;
	ulDuration = std::min(ulCurrentTime + aulTTL, ulDuration);
	SetDuration(ulDuration);
	ulCurrentTime = t;

	// Light
	if (lLightId != -1)
	{
		lLightId = -1;
	}
}


//-----------------------------------------------------------------------------
void CMagicMissile::Update(unsigned long aulTime)
{
	ARX_SOUND_RefreshPosition(snd_loop, &eCurPos);

	ulCurrentTime += aulTime;
}

//-----------------------------------------------------------------------------
float CMagicMissile::Render()
{
	int i = 0;
 
	Vec3f lastpos, newpos;
	Vec3f v;
	Anglef stiteangle;
	Vec3f stitepos;
	Vec3f stitescale;
	Color3f stitecolor;
	Vec3f av;

	if (ulCurrentTime >= ulDuration)
	{
		return 0.f;
	}

	// Set Appropriate Renderstates -------------------------------------------
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	// Set Texture ------------------------------------------------------------
	if (tex_mm)
	{
		if ((spells[spellinstance].caster == 0) && (cur_mr == 3))
			GRenderer->ResetTexture(0);
		else
			GRenderer->SetTexture(0, tex_mm);
	}

	// ------------------------------------------------------------------------

	if (bMove)
	{
		fTrail = (ulCurrentTime * fOneOnDuration) * (iBezierPrecision + 2) * 5;
	}

	lastpos.x = pathways[0].p.x;
	lastpos.y = pathways[0].p.y;
	lastpos.z = pathways[0].p.z;

	newpos = lastpos;

	for (i = 0; i < 5; i++)
	{
		int kp = i;
		int kpprec = (i > 0) ? kp - 1 : kp ;
		int kpsuiv = kp + 1 ;
		int kpsuivsuiv = (i < (5 - 2)) ? kpsuiv + 1 : kpsuiv;

		for (int toto = 1; toto < iBezierPrecision; toto++)
		{
			if (fTrail < i * iBezierPrecision + toto) break;

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

			if (!((fTrail - (i * iBezierPrecision + toto)) > iLength))
			{
				float c;

				if (fTrail < iLength)
				{
					c = 1.0f - ((fTrail - (i * iBezierPrecision + toto)) / fTrail);
				}
				else
				{
					c = 1.0f - ((fTrail - (i * iBezierPrecision + toto)) / (float)iLength);
				}

				float fsize = c;
				float alpha = c - 0.2f;

				if (alpha < 0.2f) alpha = 0.2f;

				c += frand2() * 0.1f;

				if (c < 0) c = 0;
				else if (c > 1) c = 1;

				Color color = (fColor * (c * alpha)).to<u8>();

				if (fsize < 0.5f)
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

	av.x = newpos.x - lastpos.x;
	av.y = newpos.y - lastpos.y;
	av.z = newpos.z - lastpos.z;

	float bubu = getAngle(av.x, av.z, 0, 0);
	float bubu1 = getAngle(av.x, av.y, 0, 0);

	stitepos = lastpos;

	stiteangle.b = -degrees(bubu);
	stiteangle.a = 0;
	stiteangle.g = -(degrees(bubu1));

	if (av.x < 0)
		stiteangle.g -= 90;

	if (av.x > 0)
		stiteangle.g += 90;

	if (stiteangle.g < 0)
		stiteangle.g += 360.0f;

	if ((spells[spellinstance].caster == 0) && (cur_mr == 3))
	{
		stitecolor.r = 1.f;
		stitecolor.g = 0.f;
		stitecolor.b = 0.2f;
	}
	else
	{
		stitecolor.r = 0.3f;
		stitecolor.g = 0.3f;
		stitecolor.b = 0.5f;
	}

	stitescale = Vec3f(1, 1, 1);
	{
		if ((smissile))
			DrawEERIEObjEx(smissile, &stiteangle, &stitepos, &stitescale, &stitecolor);
	}

	eCurPos = lastpos;

	return 1 - 0.5f * rnd();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CMultiMagicMissile::CMultiMagicMissile(long nbmissiles) : CSpellFx()
{
	SetDuration(2000);
	uiNumber = nbmissiles;
	pTab = NULL;
	pTab = new CMagicMissile*[uiNumber]();


	if (pTab)
	{
		for (unsigned int i = 0 ; i < uiNumber ; i++)
		{
			pTab[i] = NULL;
			pTab[i] = new CMagicMissile();
			pTab[i]->spellinstance = this->spellinstance;
		}
	}
}

//-----------------------------------------------------------------------------
CMultiMagicMissile::~CMultiMagicMissile()
{
	for (unsigned int i = 0 ; i < uiNumber ; i++)
	{
		if (pTab[i])
		{
			if (pTab[i]->lLightId != -1)
			{
				// no need to kill it because it's a duration light !
				pTab[i]->lLightId = -1;
			}

			delete pTab[i];
		}
	}

	delete [] pTab;
}

//-----------------------------------------------------------------------------
void CMultiMagicMissile::Create()
{
	
	long lMax = 0;

	if (pTab)
	{
		

		spells[spellinstance].hand_group = GetActionPointIdx(inter.iobj[spells[spellinstance].caster]->obj, "primary_attach");

		if (spells[spellinstance].hand_group != -1)
		{
			spells[spellinstance].hand_pos.x = inter.iobj[spells[spellinstance].caster]->obj->vertexlist3[spells[spellinstance].hand_group].v.x;
			spells[spellinstance].hand_pos.y = inter.iobj[spells[spellinstance].caster]->obj->vertexlist3[spells[spellinstance].hand_group].v.y;
			spells[spellinstance].hand_pos.z = inter.iobj[spells[spellinstance].caster]->obj->vertexlist3[spells[spellinstance].hand_group].v.z;
		}

		Vec3f aePos;
		float afAlpha, afBeta;
		if (spells[spellinstance].caster == 0) // player
		{
			afBeta = player.angle.b;
			afAlpha = player.angle.a;
			Vec3f vector;
			vector.x = -EEsin(radians(afBeta)) * EEcos(radians(afAlpha)) * 60.f;
			vector.y = EEsin(radians(afAlpha)) * 60.f;
			vector.z = EEcos(radians(afBeta)) * EEcos(radians(afAlpha)) * 60.f;

			if (spells[spellinstance].hand_group != -1)
			{
				aePos = spells[spellinstance].hand_pos + vector;
			}
			else
			{
				aePos.x = player.pos.x - EEsin(radians(afBeta)) + vector.x; 
				aePos.y = player.pos.y + vector.y; //;
				aePos.z = player.pos.z + EEcos(radians(afBeta)) + vector.z; 
			}
		}
		else
		{
			afAlpha = 0;
			afBeta = inter.iobj[spells[spellinstance].caster]->angle.b;
			Vec3f vector;
			vector.x = -EEsin(radians(afBeta)) * EEcos(radians(afAlpha)) * 60;
			vector.y = EEsin(radians(afAlpha)) * 60;
			vector.z = EEcos(radians(afBeta)) * EEcos(radians(afAlpha)) * 60;

			if (spells[spellinstance].hand_group != -1)
			{
				aePos = spells[spellinstance].hand_pos + vector;
			}
			else
			{
				aePos = inter.iobj[spells[spellinstance].caster]->pos + vector;
			}

			INTERACTIVE_OBJ * io = inter.iobj[spells[spellinstance].caster];

			if (ValidIONum(io->targetinfo))
			{
				Vec3f * p1 = &spells[spellinstance].caster_pos;
				Vec3f * p2 = &inter.iobj[io->targetinfo]->pos;
				afAlpha = -(degrees(getAngle(p1->y, p1->z, p2->y, p2->z + dist(Vec2f(p2->x, p2->z), Vec2f(p1->x, p1->z))))); //alpha entre orgn et dest;
			}
			else if (ValidIONum(spells[spellinstance].target))
			{
				Vec3f * p1 = &spells[spellinstance].caster_pos;
				Vec3f * p2 = &inter.iobj[spells[spellinstance].target]->pos;
				afAlpha = -(degrees(getAngle(p1->y, p1->z, p2->y, p2->z + dist(Vec2f(p2->x, p2->z), Vec2f(p1->x, p1->z))))); //alpha entre orgn et dest;
			}
		}

		for (unsigned int i = 0 ; i < uiNumber ; i++)
		{
			if (pTab[i])
			{
				Anglef angles(afAlpha, afBeta, 0.f);

				if (i > 0)
				{
					angles.a += frand2() * 4.0f;
					angles.b += frand2() * 6.0f;
				}

				pTab[i]->Create(aePos, angles);  

				float	fTime	= ulDuration + frand2() * 1000.0f;
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

				if (pTab[i]->lLightId != -1)
				{
					EERIE_LIGHT * el = &DynLight[pTab[i]->lLightId];
					el->exist		= 1;
					el->intensity	= 0.7f + 2.3f;
					el->fallend		= 190.f;
					el->fallstart	= 80.f;

					if ((spells[spellinstance].caster == 0) && (cur_mr == 3))
					{
						el->rgb.r = 1;
						el->rgb.g = 0.3f;
						el->rgb.b = 0.8f;
					}
					else
					{
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

//-----------------------------------------------------------------------------
void CMultiMagicMissile::CheckCollision()
{
	if (pTab)
	{
		for (unsigned int i = 0 ; i < uiNumber ; i++)
		{
			if (pTab[i])
			{
				CMagicMissile * pMM = (CMagicMissile *) pTab[i];

				if(!pMM->bExplo) {
					
					EERIE_SPHERE sphere;
					sphere.origin = pMM->eCurPos;
					sphere.radius	= 10.f;

					if ((spellinstance != -1) && (CheckAnythingInSphere(&sphere, spells[spellinstance].caster, CAS_NO_SAME_GROUP)))
					{
 
						LaunchMagicMissileExplosion(pMM->eCurPos, 0, spellinstance);
						ARX_NPC_SpawnAudibleSound(&pMM->eCurPos, inter.iobj[spells[spellinstance].caster]);

						pMM->SetTTL(1000);
						pMM->bExplo = true;
						pMM->bMove  = false;

						if (pMM->lLightId != -1)
						{
							pMM->lLightId = -1;
						}

						long ttt = ARX_DAMAGES_GetFree();

						if (ttt != -1)
						{
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
}

//-----------------------------------------------------------------------------
void CMultiMagicMissile::Update(unsigned long _ulTime)
{
	if (pTab)
	{
		for (unsigned int i = 0 ; i < uiNumber ; i++)
		{
			if (pTab[i])
			{
				pTab[i]->Update(_ulTime);
			}
		}
	}
}

//-----------------------------------------------------------------------------
float CMultiMagicMissile::Render()
{
	long nbmissiles	= 0;
 

	if (pTab)
	{
		for (unsigned int i = 0 ; i < uiNumber ; i++)
		{
			if (pTab[i])
			{
				float fa = pTab[i]->Render();

				CMagicMissile * pMM = (CMagicMissile *) pTab[i];

				if (pMM->lLightId != -1)
				{
					EERIE_LIGHT * el	= &DynLight[pMM->lLightId];
					el->intensity		= 0.7f + 2.3f * fa;
					el->pos = pMM->eCurPos;
					el->time_creation	= ARXTimeUL();
				}

				if (pMM->bMove) nbmissiles++;
			}
		}
	}

	if (nbmissiles == 0) return -1;

	return 1;
}

//-----------------------------------------------------------------------------
// IGNIT
//-----------------------------------------------------------------------------
CIgnit::CIgnit()
{
}

//-----------------------------------------------------------------------------
CIgnit::~CIgnit()
{
	this->Kill();
}

//-----------------------------------------------------------------------------
void CIgnit::Kill(void)
{
	int nb = this->nblight;

	while (nb--)
	{
		int id = this->tablight[nb].idl;

		if (ValidDynLight(id))
			DynLight[id].exist = 0;

		this->tablight[nb].idl = -1;
	}

	this->nblight = 0;
}

//-----------------------------------------------------------------------------
void CIgnit::Create(Vec3f * posc, float perim, int speed)
{
	this->pos = *posc;
	this->perimetre = perim;
	this->nblight = 0;
	this->duration = speed;
	this->currduration = 0;
	this->key = 0;

	int	nb = 256;

	while (nb--)
	{
		this->tablight[nb].actif = 0;
		this->tablight[nb].idl = -1;
	}

	this->ChangeTexture(TextureContainer::Load("graph/particles/fire_hit"));
	this->ChangeRGBMask(1.f, 1.f, 1.f, Color(255, 200, 0).toBGRA());
}

//-----------------------------------------------------------------------------
void CIgnit::Action(int aiMode)
{

	short sMode = checked_range_cast<short>(aiMode);

	for (int i = 0; i < nblight; i++)
	{
		GLight[tablight[i].iLightNum]->status = sMode;


		if (aiMode == 1)
		{
			ARX_SOUND_PlaySFX(SND_SPELL_IGNITE, &spells[spellinstance].caster_pos);
		}
		else if (aiMode == 0)
		{
			ARX_SOUND_PlaySFX(SND_SPELL_DOUSE, &spells[spellinstance].caster_pos);
		}
	}
}

//-----------------------------------------------------------------------------
void CIgnit::AddLight(int aiLight)
{
	if (ARXPausedTimer)  return;

	this->tablight[this->nblight].actif = 1;
	this->tablight[this->nblight].iLightNum = aiLight;
	this->tablight[this->nblight].poslight = GLight[aiLight]->pos;

	this->tablight[this->nblight].idl = GetFreeDynLight();

	if (this->tablight[this->nblight].idl > 0)
	{
		int id = this->tablight[this->nblight].idl;
		EERIE_LIGHT * el = &DynLight[id];
		el->exist = 1;
		el->intensity = 0.7f + 2.f * rnd();
		el->fallend = 400.f;
		el->fallstart = 300.f;
		el->rgb.r = this->r;
		el->rgb.g = this->g;
		el->rgb.b = this->b;
		el->pos = this->tablight[this->nblight].poslight;
	}

	this->nblight++;
}

//-----------------------------------------------------------------------------
void CIgnit::Update(unsigned long _ulTime) 
{
	float	a;
	int		nb;

	if (this->currduration >= this->duration)
	{
		this->key++;
	}

	switch (this->key)
	{
		case 0:
			a = (((float)this->currduration)) / ((float)this->duration);

			if (a >= 1.f) a = 1.f;

			nb = this->nblight;

			while (nb--)
			{
				if (this->tablight[nb].actif)
				{
					this->tablight[nb].posfx = this->pos + (this->tablight[nb].poslight - this->pos) * a;

					int id = this->tablight[nb].idl;

					if (id > 0)
					{
						DynLight[id].intensity = 0.7f + 2.f * rnd();
						DynLight[id].pos = this->tablight[nb].posfx;
					}
				}
			}

			this->interp = a;
			break;
	}

	if (!ARXPausedTimer) this->currduration += _ulTime;
}

void CDoze::CreateDoze(Vec3f * posc, float perim, int speed) {
	this->Create(posc, perim, speed);
	this->ChangeTexture(TextureContainer::Load("graph/particles/doze_hit"));
	this->ChangeRGBMask(0.f, .7f, 1.f, 0xFF0000FF);
}

//-----------------------------------------------------------------------------
void CDoze::AddLightDoze(int aiLight)
{
	if (ARXPausedTimer)  return;

	this->tablight[this->nblight].actif = 1;
	this->tablight[this->nblight].iLightNum = aiLight;
	this->tablight[this->nblight].poslight = GLight[aiLight]->pos;
	this->tablight[this->nblight].idl = -1;

	this->nblight++;
}

//-----------------------------------------------------------------------------
float CIgnit::Render() {
	
	int nb;

	switch (this->key)
	{
		case 0:

			if (this->currduration > this->duration) this->key++;

			if (!ARXPausedTimer)
			{
				float unsuri = (1.f - this->interp);
				nb = this->nblight;

				while (nb--)
				{
					if ((this->tablight[nb].actif) && (rnd() > .5f))
					{
						ARX_GenereSpheriqueEtincelles(&this->tablight[nb].posfx, rnd() * 20.f * unsuri, this->tp, this->r, this->g, this->b, this->mask);
					}
				}
			}

			break;
		default:
			break;
	}

	return 0;
}

//-----------------------------------------------------------------------------
//							PORTALS
//-----------------------------------------------------------------------------
void Split(Vec3f * v, int a, int b, float yo)
{
	if (a != b)
	{
		int i = (int)((a + b) * 0.5f);

		if ((i != a) && (i != b))
		{
			v[i].x = (v[a].x + v[b].x) * 0.5f + yo * frand2(); 
			v[i].y = (v[a].y + v[b].y) * 0.5f; 
			v[i].z = (v[a].z + v[b].z) * 0.5f + yo * frand2();
			Split(v, a, i, yo * 0.7f);
			Split(v, i, b, yo * 0.7f);
		}
	}
}

//-----------------------------------------------------------------------------
void GenereArcElectrique(Vec3f * pos, Vec3f * end, Vec3f * tabdef, int nbseg)
{
	tabdef[0] = *pos;

	tabdef[nbseg-1] = *end;

	Split(tabdef, 0, nbseg - 1, 20);
}

//-----------------------------------------------------------------------------
void DrawArcElectrique(Vec3f * tabdef, int nbseg, TextureContainer * tex, float fBeta, int tsp)
{
	TexturedVertex v[4];
	TexturedVertex v2[4];

	long i;

	//-------------------------------------------------------------------------
	// rendu
	//	GRenderer->ResetTexture(0);
	GRenderer->SetCulling(Renderer::CullNone);

	GRenderer->SetTexture(0, tex);
	
	v2[0].color = v2[1].color = v2[2].color = v2[3].color = Color::grayb(tsp).toBGR();

	float xx;
	float zz;

	for (i = 0; i < nbseg - 2; i++)
	{
		Vec3f astart;

		astart = tabdef[i];

		zz = 5; // size
		xx = (float)(5 * cos(radians(fBeta)));

		float ax = tabdef[i+1].x;
		float ay = tabdef[i+1].y;
		float az = tabdef[i+1].z;

		// version 2 faces
		v2[0].uv.x = 0;
		v2[0].uv.y = 0;
		v2[1].uv.x = 1;
		v2[1].uv.y = 0;
		v2[2].uv.x = 1;
		v2[2].uv.y = 1;
		v2[3].uv.x = 0;
		v2[3].uv.y = 1;

		v[0].p.x = astart.x;
		v[0].p.y = astart.y + zz;
		v[0].p.z = astart.z;

		v[1].p.x = astart.x;
		v[1].p.y = astart.y - zz;
		v[1].p.z = astart.z;

		v[2].p.x = ax;
		v[2].p.y = ay - zz;
		v[2].p.z = az;

		v[3].p.x = ax;
		v[3].p.y = ay + zz;
		v[3].p.z = az;

		EE_RT2(&v[0], &v2[0]);
		EE_RT2(&v[1], &v2[1]);
		EE_RT2(&v[2], &v2[2]);
		EE_RT2(&v[3], &v2[3]);
		ARX_DrawPrimitive(&v2[0],
		                             &v2[1],
		                             &v2[2]);
		ARX_DrawPrimitive(&v2[0],
		                             &v2[2],
		                             &v2[3]);

		zz *= (float) sin(radians(fBeta));

		v[0].p.x = astart.x + xx;
		v[0].p.y = astart.y;
		v[0].p.z = astart.z + zz;

		v[1].p.x = astart.x - xx;
		v[1].p.y = astart.y;
		v[1].p.z = astart.z - zz;

		v[2].p.x = ax - xx;
		v[2].p.y = ay;
		v[2].p.z = az - zz;

		v[3].p.x = ax + xx;
		v[3].p.y = ay;
		v[3].p.z = az + zz;

		EE_RT2(&v[0], &v2[0]);
		EE_RT2(&v[1], &v2[1]);
		EE_RT2(&v[2], &v2[2]);
		EE_RT2(&v[3], &v2[3]);
		ARX_DrawPrimitive(&v2[0],
		                             &v2[1],
		                             &v2[2]);
		ARX_DrawPrimitive(&v2[0],
		                             &v2[2],
		                             &v2[3]);
	}
}

CPortal::~CPortal() {
	
	if(sphereind) {
		free(sphereind);
	}
	if(spherevertex) {
		free(spherevertex);
	}
	if(sphered3d) {
		free(sphered3d);
	}
	
	int nb = 256;
	while(nb--) {
		if(tabeclair[nb].seg) {
			free(tabeclair[nb].seg);
		}
	}
}

//-----------------------------------------------------------------------------
void CPortal::AddNewEclair(Vec3f * endpos, int nbseg, int duration, int numpt)
{
	if (ARXPausedTimer) return;

	int	nb = 256;

	if ((this->nbeclair > 255) || (nbseg > 256)) return;


	short sNbSeg = static_cast<short>(nbseg);

	while (nb--)
	{
		if (!this->tabeclair[nb].actif)
		{
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
/*--------------------------------------------------------------------------*/
void CPortal::DrawAllEclair()
{
	int nb = 256;

	while (nb--)
	{
		if (this->tabeclair[nb].actif)
		{
			float a;

			a = 1.f - ((float)this->tabeclair[nb].currduration / (float)this->tabeclair[nb].duration);

			if (a < 0.f) a = 0.f;

			DrawArcElectrique(this->tabeclair[nb].seg, this->tabeclair[nb].nbseg, this->te, rnd() * 360.f, (int)(255.f * a));

			if (!ARXPausedTimer) this->tabeclair[nb].currduration += this->currframe;

			if (this->tabeclair[nb].currduration >= this->tabeclair[nb].duration)
			{
				this->tabeclair[nb].actif = 0;
				this->nbeclair--;
			}
		}
	}
}
/*--------------------------------------------------------------------------*/
void CPortal::Update(unsigned long _ulTime)
{
	float a;

	switch (this->key)
	{
		case 0:
			a = (float)this->currduration / (float)this->duration;

			if (a > 1.f)
			{
				a = 1.f;
				this->key++;
			}

			this->pos = this->sphereposdep + (this->sphereposend - this->sphereposdep) * a;
			this->spherealpha = a * .5f;

			if (!ARXPausedTimer) this->currduration += _ulTime;

			break;
		case 1:
			this->spherealpha = 0.5f + rnd();

			if (this->spherealpha > 1.f) this->spherealpha = 1.f;

			this->spherealpha *= .25f;

			//getion eclair dans boule
			this->currframe = _ulTime;

			if (!ARXPausedTimer) this->timeneweclair -= _ulTime;

			if (this->timeneweclair <= 0)
			{
				this->timeneweclair = (int)(100.f + rnd() * 200.f);

				Vec3f endpos;
				int	numpt = (int)(rnd() * (float)(this->spherenbpt - 1));
				endpos = this->spherevertex[numpt] + this->pos;

				this->AddNewEclair(&endpos, 32, (int)(1000.f + 1000.f * rnd()), numpt);
			}

			break;
	}

	if (this->lLightId >= 0)
	{
		DynLight[this->lLightId].pos = this->pos;
		DynLight[this->lLightId].intensity = 0.7f + 2.f * rnd();
	}
}
/*--------------------------------------------------------------------------*/
float CPortal::Render()
{
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);

	//calcul sphere
	int			nb = this->spherenbpt;
	TexturedVertex * v = this->sphered3d, d3dvs;
	Vec3f	* pt = this->spherevertex;
	int col = Color(0, (int)(200.f * this->spherealpha), (int)(255.f * this->spherealpha)).toBGRA();

	while (nb)
	{
		d3dvs.p.x = pt->x + this->pos.x;	//pt du bas
		d3dvs.p.y = pt->y + this->pos.y;
		d3dvs.p.z = pt->z + this->pos.z;
		EE_RTP(&d3dvs, v);

		if (!ARXPausedTimer) v->color = col;

		v++;
		pt++;
		nb--;
	}

	//update les couleurs aux impacts
	nb = 256;

	while (nb--)
	{
		if (this->tabeclair[nb].actif)
		{
			float a;

			a = 1.f - ((float)this->tabeclair[nb].currduration / (float)this->tabeclair[nb].duration);

			if (a < 0.f) a = 0.f;

			if (this->tabeclair[nb].numpt >= 0)
			{
				int r = (int)((0.f + (255.f - 0.f) * a) * this->spherealpha * 3.f);

				if (r > 255) r = 255;

				int g = (int)((200.f + (255.f - 200.f) * a) * this->spherealpha * 3.f);

				if (g > 255) g = 255;

				int b = (int)(255.f * this->spherealpha * 3.f);

				if (b > 255) b = 255;

				if (!ARXPausedTimer) this->sphered3d[this->tabeclair[nb].numpt].color = Color(r, g, b).toBGRA();
			}

		}
	}


	//affichage de la sphere back
	GRenderer->SetCulling(Renderer::CullCW);
	GRenderer->ResetTexture(0);
	GRenderer->drawIndexed(Renderer::TriangleList, this->sphered3d, this->spherenbpt, this->sphereind, this->spherenbfaces * 3);

	//affichage eclair
	this->DrawAllEclair();

	//affichage des particules à l'interieur
	if (rnd() > .25f)
	{
		int j = ARX_PARTICLES_GetFree();

		if ((j != -1) && (!ARXPausedTimer))
		{
			ParticleCount++;
			particle[j].exist = 1;
			particle[j].zdec = 0;

			float a = rnd() * 360.f;
			float b = rnd() * 360.f;
			float rr = this->r * (rnd() + .25f) * 0.05f;

			particle[j].ov.x	=	this->pos.x;
			particle[j].ov.y	=	this->pos.y;
			particle[j].ov.z	=	this->pos.z;
			particle[j].move.x	=	rr * EEsin(radians(a)) * EEcos(radians(b));
			particle[j].move.y	=	rr * EEcos(radians(a));
			particle[j].move.z	=	rr * EEsin(radians(a)) * EEsin(radians(b));
			particle[j].siz		=	10.f;
			particle[j].tolive	=	1000 + (unsigned long)(float)(rnd() * 1000.f);
			particle[j].scale.x	=	1.f;
			particle[j].scale.y	=	1.f;
			particle[j].scale.z	=	1.f;
			particle[j].timcreation	=	lARXTime;
			particle[j].tc		=	tp;
			particle[j].special	=	FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
			particle[j].fparam	=	0.0000001f;
			particle[j].rgb = Color3f::white;
		}
	}

	//affichage de la sphere front
	GRenderer->SetCulling(Renderer::CullCCW);
	GRenderer->ResetTexture(0);
	GRenderer->drawIndexed(Renderer::TriangleList, this->sphered3d, this->spherenbpt, this->sphereind, this->spherenbfaces * 3);

	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendZero);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);

	return 0;
}
/*--------------------------------------------------------------------------*/
