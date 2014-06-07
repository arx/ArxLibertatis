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

#include "graphics/spells/Spells07.h"

#include "animation/Animation.h"
#include "animation/AnimationRender.h"

#include "core/GameTime.h"

#include "game/Damage.h"
#include "game/EntityManager.h"
#include "game/Player.h"
#include "game/Spells.h"

#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/SpellEffects.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/ParticleParams.h"
#include "graphics/spells/Spells05.h"

#include "physics/Collisions.h"

#include "scene/Object.h"
#include "scene/Interactive.h"
#include "scene/Light.h"

struct CLightning::LIGHTNING {
	Vec3f eStart;
	Vec3f eVect;
	int anb;
	int anbrec;
	bool abFollow;
	int aParent;
	float fAngleXMin;
	float fAngleXMax;
	float fAngleYMin;
	float fAngleYMax;
	float fAngleZMin;
	float fAngleZMax;
};

CLightning::CLightning() :
	nbtotal(0),
	lNbSegments(40),
	invNbSegments(1.0f / 40.0f),
	fSize(100.0f),
	fLengthMin(5.0f),  
	fLengthMax(40.0f),  
	fAngleXMin(5.0f),
	fAngleXMax(32.0f),
	fAngleYMin(5.0f),
	fAngleYMax(32.0f),
	fAngleZMin(5.0f),
	fAngleZMax(32.0f)
	, fbeta(0.f)
	, iTTL(0)
	, falpha(1.0f)
	, fDamage(1)
{
	SetDuration(2000);
	ulCurrentTime = ulDuration + 1;
	
	SetColor1(1.0f, 1.0f, 1.0f);
	SetColor2(0.0f, 0.0f, 0.2f);
	
	tex_light = NULL;
}

//------------------------------------------------------------------------------
// Params une méchante struct
//------------------------------------------------------------------------------

void CLightning::BuildS(LIGHTNING * pLInfo)
{
	Vec3f astart = pLInfo->eStart;
	Vec3f avect = pLInfo->eVect;

	if(pLInfo->anb > 0 && nbtotal < 2000) {
		nbtotal++;
		int moi = nbtotal;

		if(pLInfo->abFollow) {
			avect = glm::normalize(eDest - pLInfo->eStart);
		}

		float fAngleX = frand2() * (pLInfo->fAngleXMax - pLInfo->fAngleXMin) + pLInfo->fAngleXMin;
		float fAngleY = frand2() * (pLInfo->fAngleYMax - pLInfo->fAngleYMin) + pLInfo->fAngleYMin;
		float fAngleZ = frand2() * (pLInfo->fAngleZMax - pLInfo->fAngleZMin) + pLInfo->fAngleZMin;

		Vec3f av;
		av.x = (float) cos(acos(avect.x) - radians(fAngleX));
		av.y = (float) sin(asin(avect.y) - radians(fAngleY));
		av.z = (float) tan(atan(avect.z) - radians(fAngleZ));
		av = glm::normalize(av);
		avect = av;

		float ts = rnd();
		av *= ts * (fLengthMax - fLengthMin) * pLInfo->anb * invNbSegments + fLengthMin;

		astart += av;
		pLInfo->eStart = astart;
		
		cnodetab[nbtotal].pos = pLInfo->eStart;
		cnodetab[nbtotal].size = cnodetab[0].size * pLInfo->anb * invNbSegments;
		cnodetab[nbtotal].parent = pLInfo->aParent;
		
		int anb = pLInfo->anb;
		int anbrec = pLInfo->anbrec;

		float p = rnd();

		if(p <= 0.15 && pLInfo->anbrec < 7) {
			float m = rnd();

			if(pLInfo->abFollow) {
				pLInfo->eStart = astart;
				pLInfo->eVect = avect;
				pLInfo->abFollow = false;
				pLInfo->anb =  anb - (int)(10 * (1 - m));
				pLInfo->anbrec = anbrec + (int)(2 * m);
				pLInfo->aParent = moi;
				pLInfo->fAngleXMin = fAngleXMin;
				pLInfo->fAngleXMax = fAngleXMax;
				pLInfo->fAngleYMin = fAngleYMin;
				pLInfo->fAngleYMax = fAngleYMax;
				pLInfo->fAngleZMin = fAngleZMin;
				pLInfo->fAngleZMax = fAngleZMax;
				BuildS(pLInfo);

				pLInfo->eStart = astart;
				pLInfo->eVect = avect;
				pLInfo->abFollow = true;
				pLInfo->anb = anb - (int)(10 * m);
				pLInfo->anbrec = anbrec + (int)(2 * m);
				pLInfo->aParent = moi;
				pLInfo->fAngleXMin = fAngleXMin;
				pLInfo->fAngleXMax = fAngleXMax;
				pLInfo->fAngleYMin = fAngleYMin;
				pLInfo->fAngleYMax = fAngleYMax;
				pLInfo->fAngleZMin = fAngleZMin;
				pLInfo->fAngleZMax = fAngleZMax;
				BuildS(pLInfo);
			} else {
				pLInfo->abFollow = false;
				pLInfo->eStart = astart;
				pLInfo->eVect = avect;
				pLInfo->anb = anb - (int)(10 * (1 - m));
				pLInfo->anbrec = anbrec + (int)(2 * m);
				pLInfo->aParent = moi;
				pLInfo->fAngleXMin = fAngleXMin;
				pLInfo->fAngleXMax = fAngleXMax;
				pLInfo->fAngleYMin = fAngleYMin;
				pLInfo->fAngleYMax = fAngleYMax;
				pLInfo->fAngleZMin = fAngleZMin;
				pLInfo->fAngleZMax = fAngleZMax;
				BuildS(pLInfo);

				pLInfo->abFollow = false;
				pLInfo->eStart = astart;
				pLInfo->eVect = avect;
				pLInfo->anb = anb - (int)(10 * m);
				pLInfo->anbrec = anbrec + (int)(2 * m);
				pLInfo->aParent = moi;
				pLInfo->fAngleXMin = fAngleXMin;
				pLInfo->fAngleXMax = fAngleXMax;
				pLInfo->fAngleYMin = fAngleYMin;
				pLInfo->fAngleYMax = fAngleYMax;
				pLInfo->fAngleZMin = fAngleZMin;
				pLInfo->fAngleZMax = fAngleZMax;
				BuildS(pLInfo);
			}
		} else {
			if(rnd() <= 0.10) {
				pLInfo->abFollow = true;
			}

			pLInfo->eStart = astart;
			pLInfo->eVect = avect;
			pLInfo->anb = anb - 1;
			pLInfo->anbrec = anbrec;
			pLInfo->aParent = moi;
			pLInfo->fAngleXMin = fAngleXMin;
			pLInfo->fAngleXMax = fAngleXMax;
			pLInfo->fAngleYMin = fAngleYMin;
			pLInfo->fAngleYMax = fAngleYMax;
			pLInfo->fAngleZMin = fAngleZMin;
			pLInfo->fAngleZMax = fAngleZMax;
			BuildS(pLInfo);
		}
	}
}

void CLightning::SetPosSrc(Vec3f aeSrc) {
	eSrc = aeSrc;
}

void CLightning::SetPosDest(Vec3f aeDest) {
	eDest = aeDest;
}

void CLightning::SetColor1(float afR, float afG, float afB) {
	fColor1[0] = afR;
	fColor1[1] = afG;
	fColor1[2] = afB;
}

void CLightning::SetColor2(float afR, float afG, float afB) {
	fColor2[0] = afR;
	fColor2[1] = afG;
	fColor2[2] = afB;
}


float fTotoro = 0;
float fMySize = 2;

void CLightning::Create(Vec3f aeFrom, Vec3f aeTo, float beta) {
	
	(void)beta; // TODO removing this parameter makes the signature clash with method from superclass
	
	SetDuration(ulDuration);
	SetPosSrc(aeFrom);
	SetPosDest(aeTo);

	nbtotal = 0;

	if(nbtotal == 0) {
		fbeta = 0.f; 
		falpha = 1.f; 

		LIGHTNING LInfo;
		memset(&LInfo, 0, sizeof(LIGHTNING));

		LInfo.eStart = eSrc;
		LInfo.eVect = eDest - eSrc;
		LInfo.anb = lNbSegments;
		LInfo.anbrec = 0;
		LInfo.abFollow = true;
		LInfo.aParent = 0;
		LInfo.fAngleXMin = fAngleXMin;
		LInfo.fAngleXMax = fAngleXMax;
		LInfo.fAngleYMin = fAngleYMin;
		LInfo.fAngleYMax = fAngleYMax;
		LInfo.fAngleZMin = fAngleZMin;
		LInfo.fAngleZMax = fAngleZMax;
		
		cnodetab[0].pos = eSrc;
		cnodetab[0].size = 15;
		cnodetab[0].parent = 0;
		
		BuildS(&LInfo);
	}
	
	
	float fRandom	= 500 + rnd() * 1000;
	
	iTTL = checked_range_cast<int>(fRandom);
	
}

void CLightning::ReCreate()
{
	nbtotal = 0;

	if(nbtotal == 0) {
		falpha = 1.f;

		LIGHTNING LInfo;
		memset(&LInfo, 0, sizeof(LIGHTNING));

		LInfo.eStart = eSrc;
		LInfo.eVect = eDest - eSrc;
		LInfo.anb = lNbSegments;
		LInfo.anbrec = 0;
		LInfo.abFollow = true;
		LInfo.aParent = 0;
		LInfo.fAngleXMin = fAngleXMin;
		LInfo.fAngleXMax = fAngleXMax;
		LInfo.fAngleYMin = fAngleYMin;
		LInfo.fAngleYMax = fAngleYMax;
		LInfo.fAngleZMin = fAngleZMin;
		LInfo.fAngleZMax = fAngleZMax;

		cnodetab[0].pos = eSrc;
		cnodetab[0].size = 8;
		cnodetab[0].parent = 0;

		BuildS(&LInfo);
	}


	float fRandom	= 500 + rnd() * 1000;

	iTTL = checked_range_cast<int>(fRandom);
}

void CLightning::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;
	iTTL -= _ulTime;
	fTotoro += 8;

	if(fMySize > 0.3f)
		fMySize -= 0.1f;
}

void GetChestPos(EntityHandle num, Vec3f * p)
{
	if(num == 0) {
		p->x = player.pos.x;
		p->y = player.pos.y + 70.f;
		p->z = player.pos.z;
		return;
	}

	if(ValidIONum(num)) {
		long idx = GetGroupOriginByName(entities[num]->obj, "chest");

		if(idx >= 0) {
			*p = entities[num]->obj->vertexlist3[idx].v;
		} else {
			p->x = entities[num]->pos.x;
			p->y = entities[num]->pos.y - 120.f;
			p->z = entities[num]->pos.z;
		}
	}
}

void CLightning::Render()
{
	TexturedVertex v[4];
	TexturedVertex v2[4];

	if(ulCurrentTime >= ulDuration)
		return;

	float fOneOnDuration = 1.f / (float)(ulDuration);
	falpha = 1.f - (((float)(ulCurrentTime)) * fOneOnDuration); 

	if(falpha > 1.f)
		falpha = 1.f;

	if(iTTL <= 0) {
		fTotoro = 0;
		fMySize = 2;
		ReCreate();
	}

	falpha = 1;

	long i;

	Vec3f ePos;
	
	float fBeta = 0.f;
	falpha = 0.f;

	// Create hand position if a hand is defined
	//	spells[spellinstance].hand_group=entities[spells[spellinstance].caster]->obj->fastaccess.primary_attach;//GetActionPointIdx(entities[spells[spellinstance].caster]->obj,"primary_attach");
	// Player source
	if(spells[spellinstance].m_type == SPELL_MASS_LIGHTNING_STRIKE) {
		arx_assert(lSrc == -1);	//ARX: jycorbel (2010-07-19) - We really need ePos when lSrc!=-1 ; in that case lSrc should be equal to -1 !
		ePos = Vec3f_ZERO;
	} else {
		
		Entity * caster = entities[spells[spellinstance].m_caster];
		long idx = GetGroupOriginByName(caster->obj, "chest");
		if(idx >= 0) {
			spells[spellinstance].m_caster_pos = caster->obj->vertexlist3[idx].v;
		} else {
			spells[spellinstance].m_caster_pos = caster->pos;
		}
		
		if(spells[spellinstance].m_caster == 0) {
			falpha = -player.angle.getYaw();
			fBeta = player.angle.getPitch();
		} else {
			// IO source
			fBeta = caster->angle.getPitch();
			if(ValidIONum(caster->targetinfo)
			   && caster->targetinfo != spells[spellinstance].m_caster) {
				Vec3f * p1 = &spells[spellinstance].m_caster_pos;
				Vec3f p2;
				GetChestPos(caster->targetinfo, &p2); 
				falpha = MAKEANGLE(degrees(getAngle(p1->y, p1->z, p2.y, p2.z + glm::distance(Vec2f(p2.x, p2.z), Vec2f(p1->x, p1->z))))); //alpha entre orgn et dest;
			}
			else if (ValidIONum(spells[spellinstance].m_target))
			{
				Vec3f * p1 = &spells[spellinstance].m_caster_pos;
				Vec3f p2;
				GetChestPos(spells[spellinstance].m_target, &p2); //
				falpha = MAKEANGLE(degrees(getAngle(p1->y, p1->z, p2.y, p2.z + glm::distance(Vec2f(p2.x, p2.z), Vec2f(p1->x, p1->z))))); //alpha entre orgn et dest;
			}
		}
		
		ePos = spells[spellinstance].m_caster_pos;
	}

	//-------------------------------------------------------------------------
	// rendu

	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);

	float f = 1.5f * fMySize;
	cnodetab[0].f = randomVec(-f, f);

	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->ResetTexture(0);

	v2[0].color = v2[1].color = v2[2].color = v2[3].color = Color::white.toBGR();

	fbeta = fBeta + rnd() * 2 * fMySize;

	for(i = 0; i < nbtotal && i <= fTotoro; i++) {
		Vec3f astart = cnodetab[cnodetab[i].parent].pos + cnodetab[cnodetab[i].parent].f;
		float temp = 1.5f * fMySize;
		Vec3f z_z = cnodetab[cnodetab[i].parent].f + randomVec(-temp, temp);
		float zz = cnodetab[i].size + cnodetab[i].size * 0.3f * rnd();
		float xx = (float)(cnodetab[i].size * cos(radians(-fbeta)));
		cnodetab[i].f = z_z;
		
		Vec3f a = cnodetab[i].pos + z_z;
		if(lSrc != -1) {
			Vec3f vv2;
			Vec3f vv1 = astart;
			vv1 = VRotateX(vv1, (falpha));  
			vv2 = VRotateY(vv1, 180 - MAKEANGLE(fBeta));
			astart = vv2;
			vv1 = a;
			vv1 = VRotateX(vv1, (falpha)); 
			vv2 = VRotateY(vv1, 180 - MAKEANGLE(fBeta));
			a = vv2;
			astart += ePos;
			a += ePos;
		}
		
		if(i % 4 == 0) {
			Sphere sphere;
			sphere.origin = a;
			sphere.radius = std::min(cnodetab[i].size, 50.f);

			if(CheckAnythingInSphere(sphere, spells[spellinstance].m_caster, CAS_NO_SAME_GROUP)) {

				DamageParameters damage;
				damage.pos = sphere.origin;
				damage.radius = sphere.radius;
				damage.damages = fDamage * spells[spellinstance].m_caster_level * ( 1.0f / 3 );
				damage.area = DAMAGE_FULL;
				damage.duration = 1;
				damage.source = spells[spellinstance].m_caster;
				damage.flags = DAMAGE_FLAG_DONT_HURT_SOURCE | DAMAGE_FLAG_ADD_VISUAL_FX;
				damage.type = DAMAGE_TYPE_FAKEFIRE | DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_LIGHTNING;
				DamageCreate(damage);
			}
		}
		
		// version 4 faces
		v2[0].color = v2[3].color = 0xFFFFFFFF;
		v2[1].color = v2[2].color = 0xFF00005A;
		v2[0].uv = Vec2f(0.5f, 0.f);
		v2[1].uv = Vec2f_ZERO;
		v2[2].uv = Vec2f_Y_AXIS;
		v2[3].uv = Vec2f(0.5f, 1.f);
		v[0].p = astart;
		v[1].p = astart + Vec3f(0.f, zz, 0.f);
		v[2].p = a + Vec3f(0.f, zz, 0.f);
		v[3].p = a;
		EE_RT2(&v[0], &v2[0]);
		EE_RT2(&v[1], &v2[1]);
		EE_RT2(&v[2], &v2[2]);
		EE_RT2(&v[3], &v2[3]);
		ARX_DrawPrimitive(&v2[0], &v2[1], &v2[2]);
		ARX_DrawPrimitive(&v2[0], &v2[2], &v2[3]);
		
		v2[0].uv = Vec2f(0.5f, 0.f);
		v2[1].uv = Vec2f_X_AXIS;
		v2[2].uv = Vec2f_ONE;
		v2[3].uv = Vec2f(0.5f, 1.f);
		v[1].p = astart - Vec3f(0.f, zz, 0.f);
		v[2].p = a - Vec3f(0.f, zz, 0.f);
		EE_RT2(&v[1], &v2[1]);
		EE_RT2(&v[2], &v2[2]);
		ARX_DrawPrimitive(&v2[0], &v2[1], &v2[2]);
		ARX_DrawPrimitive(&v2[0], &v2[2], &v2[3]);
		
		zz *= (float) sin(radians(fbeta));
		
		v2[1].uv = Vec2f_X_AXIS;
		v2[2].uv = Vec2f_ONE;
		v[1].p = astart + Vec3f(xx, 0.f, zz);
		v[2].p = a + Vec3f(xx, 0.f, zz);
		EE_RT2(&v[1], &v2[1]);
		EE_RT2(&v[2], &v2[2]);
		ARX_DrawPrimitive(&v2[0], &v2[1], &v2[2]);
		ARX_DrawPrimitive(&v2[0], &v2[2], &v2[3]);
		
		v2[1].uv = Vec2f_ZERO;
		v2[2].uv = Vec2f_Y_AXIS;
		v[1].p = astart - Vec3f(xx, 0.f, zz);
		v[2].p = a - Vec3f(xx, 0.f, zz);
		EE_RT2(&v[1], &v2[1]);
		EE_RT2(&v[2], &v2[2]);
		ARX_DrawPrimitive(&v2[0], &v2[1], &v2[2]);
		ARX_DrawPrimitive(&v2[0], &v2[2], &v2[3]);
	}
	
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	
	//return falpha;
}

CConfuse::~CConfuse()
{
	spapi_count--;

	if(spapi && spapi_count <= 0) {
		spapi_count = 0;
		delete spapi;
		spapi = NULL;
	}
}

CConfuse::CConfuse() {
	
	SetDuration(5000);
	ulCurrentTime = ulDuration + 1;
	
	tex_p1 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_blueting");
	tex_trail = TextureContainer::Load("graph/obj3d/textures/(fx)_bandelette_blue");
	
	if(!spapi) {
		spapi = LoadTheObj("graph/obj3d/interactive/fix_inter/fx_papivolle/fx_papivolle.teo");
	}
	spapi_count++;
	
	const char tex[] = "graph/obj3d/interactive/fix_inter/fx_papivolle/fx_papivolle.tea";
	ANIM_HANDLE * anim_papii = EERIE_ANIMMANAGER_Load(tex);
	
	ANIM_Set(&au, anim_papii);
	au.next_anim = NULL;
	au.cur_anim = anim_papii;
	au.ctime = 0;
	au.flags = EA_LOOP;
	au.nextflags = 0;
	au.lastframe = 0;
	au.pour = 0;
	au.fr = 0;
	au.altidx_cur = 0;
	au.altidx_next = 0;
}

void CConfuse::Create(float afBeta) {
	
	SetDuration(ulDuration);
	SetAngle(afBeta);
}

void CConfuse::Update(unsigned long _ulTime) {
	ulCurrentTime += _ulTime;
}

void CConfuse::Render() {
	
	int i = 0;
	
	if(ulCurrentTime >= ulDuration)
		return;

	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetTexture(0, tex_trail);
	
	eCurPos = entities[spells[spellinstance].m_target]->pos;
	if(spells[spellinstance].m_target != PlayerEntityHandle) {
		eCurPos.y += entities[spells[spellinstance].m_target]->physics.cyl.height - 30.f;
	}
	
	long idx = entities[spells[spellinstance].m_target]->obj->fastaccess.head_group_origin;
	if(idx >= 0) {
		eCurPos = entities[spells[spellinstance].m_target]->obj->vertexlist3[idx].v;
		eCurPos.y -= 50.f;
	}
	
	Vec3f stitepos = eCurPos;
	Anglef stiteangle = Anglef(0.f, -degrees(arxtime.get_updated() * ( 1.0f / 500 )), 0.f);
	Color3f stitecolor = Color3f::white;
	Vec3f stitescale = Vec3f_ONE;
	Draw3DObject(spapi, stiteangle, stitepos, stitescale, stitecolor);
	
	for(i = 0; i < 6; i++) {
		
		PARTICLE_DEF * pd = createParticle();
		if(!pd) {
			break;
		}
		
		float ang = radians(rnd() * 360.f);
		float rad = rnd() * 15.f;
		pd->ov = stitepos + Vec3f(-std::sin(ang) * rad, 0.f, std::cos(ang) * rad);
		pd->move = Vec3f(0.f, rnd() * 3.f + 1.f, 0.f);
		pd->siz = 0.25f;
		pd->tolive = Random::get(2300, 3300);
		pd->tc = tex_p1;
		pd->special = PARTICLE_GOLDRAIN | FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION
		              | DISSIPATING;
		pd->fparam = 0.0000001f;
		float t1 = rnd() * 0.4f + 0.4f;
		float t2 = rnd() * 0.6f + 0.2f;
		float t3 = rnd() * 0.4f + 0.4f;
		while(EEfabs(t1 - t2) > 0.3f && EEfabs(t2 - t3) > 0.3f) {
			t1 = rnd() * 0.4f + 0.4f;
			t2 = rnd() * 0.6f + 0.2f;
			t3 = rnd() * 0.4f + 0.4f;
		}
		pd->rgb = Color3f(t1 * 0.8f, t2 * 0.8f, t3 * 0.8f);
	}
	
	if(!lightHandleIsValid(lLightId))
		this->lLightId = GetFreeDynLight();

	if(lightHandleIsValid(lLightId)) {
		EERIE_LIGHT * light = lightHandleGet(lLightId);
		
		light->intensity = 1.3f;
		light->fallstart = 180.f;
		light->fallend   = 420.f;
		light->rgb.r = 0.3f + rnd() * ( 1.0f / 5 );
		light->rgb.g = 0.3f;
		light->rgb.b = 0.5f + rnd() * ( 1.0f / 5 );
		light->pos = stitepos;
		light->duration = 200;
		light->extras = 0;
	}
}

//-----------------------------------------------------------------------------
//	FIRE FIELD
//-----------------------------------------------------------------------------
CFireField::CFireField()
{
}

CFireField::~CFireField()
{
}

void CFireField::Create(float largeur, const Vec3f & pos, int _ulDuration)
{
	ARX_UNUSED(largeur);
	
	SetDuration(_ulDuration);

	this->pos = pos;
	
	{
	ParticleParams cp = ParticleParams();
	cp.m_nbMax = 100;
	cp.m_life = 2000;
	cp.m_lifeRandom = 1000;
	cp.m_pos = Vec3f(80, 10, 80);
	cp.m_direction = Vec3f(0, 2, 0) * 0.1f;
	cp.m_angle = 0;
	cp.m_speed = 0;
	cp.m_speedRandom = 0;
	cp.m_gravity = Vec3f_ZERO;
	cp.m_flash = 0;
	cp.m_rotation = 0;
	cp.m_rotationRandomDirection = false;
	cp.m_rotationRandomStart = false;

	cp.m_startSegment.m_size = 10;
	cp.m_startSegment.m_sizeRandom = 3;
	cp.m_startSegment.m_color = Color(25, 25, 25, 50).to<float>();
	cp.m_startSegment.m_colorRandom = Color(51, 51, 51, 101).to<float>();

	cp.m_endSegment.m_size = 10;
	cp.m_endSegment.m_sizeRandom = 3;
	cp.m_endSegment.m_color = Color(25, 25, 25, 50).to<float>();
	cp.m_endSegment.m_colorRandom = Color(0, 0, 0, 100).to<float>();
	cp.m_texture.m_texLoop = true;

	cp.m_blendMode = RenderMaterial::AlphaAdditive;
	cp.m_freq = 150.0f;
	cp.m_texture.set("graph/particles/firebase", 4, 100);
	cp.m_spawnFlags = 0;
	
	pPSStream.SetParams(cp);
	}
	pPSStream.SetPos(pos);
	pPSStream.Update(0);

	//-------------------------------------------------------------------------

	{
	ParticleParams cp = ParticleParams();
	cp.m_nbMax = 50;
	cp.m_life = 1000;
	cp.m_lifeRandom = 500;
	cp.m_pos = Vec3f(100, 10, 100);
	cp.m_direction = Vec3f(0, -2, 0) * 0.1f;
	cp.m_angle = radians(10);
	cp.m_speed = 0;
	cp.m_speedRandom = 0;
	cp.m_gravity = Vec3f_ZERO;
	cp.m_flash = 0;
	cp.m_rotation = 0;
	cp.m_rotationRandomDirection = false;
	cp.m_rotationRandomStart = false;

	cp.m_startSegment.m_size = 10;
	cp.m_startSegment.m_sizeRandom = 10;
	cp.m_startSegment.m_color = Color(40, 40, 40, 50).to<float>();
	cp.m_startSegment.m_colorRandom = Color(51, 51, 51, 100).to<float>();

	cp.m_endSegment.m_size = 10;
	cp.m_endSegment.m_sizeRandom = 10;
	cp.m_endSegment.m_color = Color(0, 0, 0, 50).to<float>();
	cp.m_endSegment.m_colorRandom = Color(0, 0, 0, 100).to<float>();
	cp.m_texture.m_texLoop = false;

	cp.m_blendMode = RenderMaterial::Additive;
	cp.m_freq = 150.0f;
	cp.m_texture.set("graph/particles/fire", 0, 500);
	cp.m_spawnFlags = 0;
	
	pPSStream1.SetParams(cp);
	}
	pPSStream1.SetPos(pos + Vec3f(0, 10, 0));
	pPSStream1.Update(0);
}

void CFireField::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;

	pPSStream.Update(_ulTime);
	pPSStream1.Update(_ulTime);
}

void CFireField::Render()
{
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);

	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	pPSStream.Render();
	pPSStream1.Render();

	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendZero);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
}

CIceField::~CIceField()
{
	smotte_count--;

	if(smotte && smotte_count <= 0) {
		smotte_count = 0;
		delete smotte;
		smotte = NULL;
	}

	stite_count--;

	if(stite && stite_count <= 0) {
		stite_count = 0;
		delete stite;
		stite = NULL;
	}
}

CIceField::CIceField()
	: iMax(50)
{
	eSrc = Vec3f_ZERO;
	eTarget = Vec3f_ZERO;
	
	SetDuration(1000);
	ulCurrentTime = ulDuration + 1;
	
	tex_p1 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_blueting");
	tex_p2 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_bluepouf");
	
	if(!stite) {
		stite = LoadTheObj("graph/obj3d/interactive/fix_inter/stalagmite/motte.teo");
	}
	stite_count++;
	
	if(!smotte) {
		smotte = LoadTheObj("graph/obj3d/interactive/fix_inter/stalagmite/motte.teo");
	}
	smotte_count++;
}

void CIceField::Create(Vec3f aeSrc, float afBeta) {
	
	SetDuration(ulDuration);
	
	eSrc = aeSrc;
	SetAngle(afBeta);
	eTarget = eSrc;
	
	for(int i = 0; i < iMax; i++) {
		float t = rnd();

		if (t < 0.5f)
			tType[i] = 0;
		else
			tType[i] = 1;
		
		tSize[i] = Vec3f_ZERO;
		tSizeMax[i].x = rnd();
		tSizeMax[i].y = rnd() + 0.2f;
		tSizeMax[i].z = rnd();
		
		Vec3f minPos;
		if(tType[i] == 0) {
			minPos = Vec3f(1.2f, 1, 1.2f);
		} else {
			minPos = Vec3f(0.4f, 0.3f, 0.4f);
		}
		
		tSizeMax[i] = glm::max(tSizeMax[i], minPos);
		
		if(tType[i] == 0) {
			tPos[i].x = eSrc.x + frand2() * 80;
			tPos[i].y = eSrc.y;
			tPos[i].z = eSrc.z + frand2() * 80;
		} else {
			tPos[i].x = eSrc.x + frand2() * 120;
			tPos[i].y = eSrc.y;
			tPos[i].z = eSrc.z + frand2() * 120;
		}
	}
}

void CIceField::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;
}

extern bool VisibleSphere(const Vec3f & pos, float radius);

void CIceField::Render()
{
	if(!VisibleSphere(eSrc - Vec3f(0.f, 120.f, 0.f), 350.f))
		return;

	int i = 0;

	
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	
	for(i = 0; i < iMax; i++) {
		
		tSize[i] += Vec3f(0.1f);
		tSize[i] = glm::min(tSize[i], tSizeMax[i]);
		
		Anglef stiteangle = Anglef::ZERO;
		Vec3f stitepos;
		Vec3f stitescale;
		Color3f stitecolor;

		stiteangle.setPitch((float)cos(radians(tPos[i].x)) * 360);
		stitepos.x = tPos[i].x;
		stitepos.y = eSrc.y;
		stitepos.z = tPos[i].z;
		
		stitecolor.r = tSizeMax[i].y * 0.7f;
		stitecolor.g = tSizeMax[i].y * 0.7f;
		stitecolor.b = tSizeMax[i].y * 0.9f;

		if(stitecolor.r > 1)
			stitecolor.r = 1;

		if(stitecolor.g > 1)
			stitecolor.g = 1;

		if(stitecolor.b > 1)
			stitecolor.b = 1;

		stitescale.z = tSize[i].x;
		stitescale.y = tSize[i].y;
		stitescale.x = tSize[i].z;

		if(tType[i] == 0)
			Draw3DObject(smotte, stiteangle, stitepos, stitescale, stitecolor);
		else
			Draw3DObject(stite, stiteangle, stitepos, stitescale, stitecolor);
	}
	
	for(i = 0; i < iMax * 0.5f; i++) {
		
		float t = rnd();
		if(t < 0.01f) {
			
			PARTICLE_DEF * pd = createParticle();
			if(pd) {
				pd->ov = tPos[i] + randomVec(-5.f, 5.f);
				pd->move = randomVec(-2.f, 2.f);
				pd->siz = 20.f;
				pd->tolive = Random::get(2000, 6000);
				pd->tc = tex_p2;
				pd->special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
				pd->fparam = 0.0000001f;
				pd->rgb = Color3f(0.7f, 0.7f, 1.f);
			}
			
		} else if (t > 0.095f) {
			
			PARTICLE_DEF * pd = createParticle();
			if(pd) {
				pd->ov = tPos[i] + randomVec(-5.f, 5.f) + Vec3f(0.f, 50.f, 0.f);
				pd->move = Vec3f(0.f, 2.f - 4.f * rnd(), 0.f);
				pd->siz = 0.5f;
				pd->tolive = Random::get(2000, 6000);
				pd->tc = tex_p1;
				pd->special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
				pd->fparam = 0.0000001f;
				pd->rgb = Color3f(0.7f, 0.7f, 1.f);
			}
			
		}
	}

	GRenderer->SetCulling(Renderer::CullNone);
}
