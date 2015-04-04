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

#include "scene/Interactive.h"
#include "scene/Light.h"
#include "scene/Object.h"
#include "scene/Scene.h"

struct CLightning::LIGHTNING {
	Vec3f eStart;
	Vec3f eVect;
	int anb;
	int anbrec;
	bool abFollow;
	int aParent;
	Vec3f fAngleMin;
	Vec3f fAngleMax;
};

CLightning::CLightning()
	: m_pos(Vec3f_ZERO)
	, m_beta(0.f)
	, m_alpha(0.f)
	, m_caster(EntityHandle::Invalid)
	, m_level(1.f)
	, fDamage(1)
	, m_isMassLightning(false),
	nbtotal(0),
	lNbSegments(40),
	invNbSegments(1.0f / 40.0f),
	fSize(100.0f),
	fLengthMin(5.0f),  
	fLengthMax(40.0f),  
	fAngleMin(5.0f, 5.0f, 5.0f),
	fAngleMax(32.0f, 32.0f, 32.0f)
	, iTTL(0)
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

	if(pLInfo->anb > 0 && nbtotal < (MAX_NODES - 1)) {
		nbtotal++;
		int moi = nbtotal;

		if(pLInfo->abFollow) {
			avect = glm::normalize(eDest - pLInfo->eStart);
		}

		Vec3f fAngle;
		fAngle.x = frand2() * (pLInfo->fAngleMax.x - pLInfo->fAngleMin.x) + pLInfo->fAngleMin.x;
		fAngle.y = frand2() * (pLInfo->fAngleMax.y - pLInfo->fAngleMin.y) + pLInfo->fAngleMin.y;
		fAngle.z = frand2() * (pLInfo->fAngleMax.z - pLInfo->fAngleMin.z) + pLInfo->fAngleMin.z;

		Vec3f av;
		av.x = glm::cos(glm::acos(avect.x) - glm::radians(fAngle.x));
		av.y = glm::sin(glm::asin(avect.y) - glm::radians(fAngle.y));
		av.z = glm::tan(glm::atan(avect.z) - glm::radians(fAngle.z));
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
				pLInfo->fAngleMin = fAngleMin;
				pLInfo->fAngleMax = fAngleMax;
				
				BuildS(pLInfo);

				pLInfo->eStart = astart;
				pLInfo->eVect = avect;
				pLInfo->abFollow = true;
				pLInfo->anb = anb - (int)(10 * m);
				pLInfo->anbrec = anbrec + (int)(2 * m);
				pLInfo->aParent = moi;
				pLInfo->fAngleMin = fAngleMin;
				pLInfo->fAngleMax = fAngleMax;
				
				BuildS(pLInfo);
			} else {
				pLInfo->abFollow = false;
				pLInfo->eStart = astart;
				pLInfo->eVect = avect;
				pLInfo->anb = anb - (int)(10 * (1 - m));
				pLInfo->anbrec = anbrec + (int)(2 * m);
				pLInfo->aParent = moi;
				pLInfo->fAngleMin = fAngleMin;
				pLInfo->fAngleMax = fAngleMax;
				
				BuildS(pLInfo);

				pLInfo->abFollow = false;
				pLInfo->eStart = astart;
				pLInfo->eVect = avect;
				pLInfo->anb = anb - (int)(10 * m);
				pLInfo->anbrec = anbrec + (int)(2 * m);
				pLInfo->aParent = moi;
				pLInfo->fAngleMin = fAngleMin;
				pLInfo->fAngleMax = fAngleMax;
				
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
			pLInfo->fAngleMin = fAngleMin;
			pLInfo->fAngleMax = fAngleMax;
			
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

void CLightning::Create(Vec3f aeFrom, Vec3f aeTo) {
	
	SetDuration(ulDuration);
	SetPosSrc(aeFrom);
	SetPosDest(aeTo);

	ReCreate(15);
}

void CLightning::ReCreate(float rootSize)
{
	nbtotal = 0;

	if(nbtotal == 0) {
		LIGHTNING LInfo;
		memset(&LInfo, 0, sizeof(LIGHTNING));

		LInfo.eStart = eSrc;
		LInfo.eVect = eDest - eSrc;
		LInfo.anb = lNbSegments;
		LInfo.anbrec = 0;
		LInfo.abFollow = true;
		LInfo.aParent = 0;
		LInfo.fAngleMin = fAngleMin;
		LInfo.fAngleMax = fAngleMax;
		
		cnodetab[0].pos = eSrc;
		cnodetab[0].size = rootSize;
		cnodetab[0].parent = 0;

		BuildS(&LInfo);
	}


	float fRandom	= 500 + rnd() * 1000;

	iTTL = checked_range_cast<int>(fRandom);
}

void CLightning::Update(float timeDelta)
{
	ulCurrentTime += timeDelta;
	iTTL -= timeDelta;
	fTotoro += 8;

	if(fMySize > 0.3f)
		fMySize -= 0.1f;
}

void CLightning::Render()
{
	if(ulCurrentTime >= ulDuration)
		return;
	
	if(iTTL <= 0) {
		fTotoro = 0;
		fMySize = 2;
		ReCreate(8);
	}
	
	Vec3f ePos;
	
	float fBeta = 0.f;
	float falpha = 0.f;

	// Create hand position if a hand is defined
	//	spells[spellinstance].hand_group=entities[spells[spellinstance].caster]->obj->fastaccess.primary_attach;//GetActionPointIdx(entities[spells[spellinstance].caster]->obj,"primary_attach");
	// Player source
	if(m_isMassLightning) {
		ePos = Vec3f_ZERO;
	} else {
		ePos = m_pos;
		fBeta = m_beta;
		falpha = m_alpha;
	}
	
	float f = 1.5f * fMySize;
	cnodetab[0].f = randomVec(-f, f);
	
	RenderMaterial mat;
	mat.setCulling(Renderer::CullNone);
	mat.setDepthTest(false);
	mat.setBlendType(RenderMaterial::Additive);
	
	float fbeta = fBeta + rnd() * 2 * fMySize;

	for(size_t i = 0; i < nbtotal && i <= fTotoro; i++) {
		Vec3f astart = cnodetab[cnodetab[i].parent].pos + cnodetab[cnodetab[i].parent].f;
		float temp = 1.5f * fMySize;
		Vec3f z_z = cnodetab[cnodetab[i].parent].f + randomVec(-temp, temp);
		float zz = cnodetab[i].size + cnodetab[i].size * 0.3f * rnd();
		float xx = cnodetab[i].size * glm::cos(glm::radians(-fbeta));
		cnodetab[i].f = z_z;
		
		Vec3f a = cnodetab[i].pos + z_z;
		if(!m_isMassLightning) {
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

			if(CheckAnythingInSphere(sphere, m_caster, CAS_NO_SAME_GROUP)) {

				DamageParameters damage;
				damage.pos = sphere.origin;
				damage.radius = sphere.radius;
				damage.damages = fDamage * m_level * ( 1.0f / 3 );
				damage.area = DAMAGE_FULL;
				damage.duration = 1;
				damage.source = m_caster;
				damage.flags = DAMAGE_FLAG_DONT_HURT_SOURCE | DAMAGE_FLAG_ADD_VISUAL_FX;
				damage.type = DAMAGE_TYPE_FAKEFIRE | DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_LIGHTNING;
				DamageCreate(damage);
			}
		}
		
		{
		TexturedQuad q;
		
		q.v[0].color = Color(255, 255, 255, 255).toRGBA();
		q.v[1].color = Color(0, 0, 90, 255).toRGBA();
		q.v[2].color = Color(0, 0, 90, 255).toRGBA();
		q.v[3].color = Color(255, 255, 255, 255).toRGBA();
		q.v[0].uv = Vec2f(0.5f, 0.f);
		q.v[1].uv = Vec2f_ZERO;
		q.v[2].uv = Vec2f_Y_AXIS;
		q.v[3].uv = Vec2f(0.5f, 1.f);
		q.v[0].p = astart;
		q.v[1].p = astart + Vec3f(0.f, zz, 0.f);
		q.v[2].p = a + Vec3f(0.f, zz, 0.f);
		q.v[3].p = a;
		
		drawQuadRTP(mat, q);
		}
		
		{
		TexturedQuad q;

		q.v[0].color = Color(255, 255, 255, 255).toRGBA();
		q.v[1].color = Color(0, 0, 90, 255).toRGBA();
		q.v[2].color = Color(0, 0, 90, 255).toRGBA();
		q.v[3].color = Color(255, 255, 255, 255).toRGBA();
		q.v[0].uv = Vec2f(0.5f, 0.f);
		q.v[1].uv = Vec2f_X_AXIS;
		q.v[2].uv = Vec2f_ONE;
		q.v[3].uv = Vec2f(0.5f, 1.f);
		q.v[0].p = astart;
		q.v[1].p = astart - Vec3f(0.f, zz, 0.f);
		q.v[2].p = a - Vec3f(0.f, zz, 0.f);
		q.v[3].p = a;
		
		drawQuadRTP(mat, q);
		}
		
		zz *= glm::sin(glm::radians(fbeta));
		
		{
		TexturedQuad q;
		
		q.v[0].color = Color(255, 255, 255, 255).toRGBA();
		q.v[1].color = Color(0, 0, 90, 255).toRGBA();
		q.v[2].color = Color(0, 0, 90, 255).toRGBA();
		q.v[3].color = Color(255, 255, 255, 255).toRGBA();
		q.v[0].uv = Vec2f(0.5f, 0.f);
		q.v[1].uv = Vec2f_X_AXIS;
		q.v[2].uv = Vec2f_ONE;
		q.v[3].uv = Vec2f(0.5f, 1.f);
		q.v[0].p = astart;
		q.v[1].p = astart + Vec3f(xx, 0.f, zz);
		q.v[2].p = a + Vec3f(xx, 0.f, zz);
		q.v[3].p = a;
		
		drawQuadRTP(mat, q);
		}
		
		{
		TexturedQuad q;
		
		q.v[0].color = Color(255, 255, 255, 255).toRGBA();
		q.v[1].color = Color(0, 0, 90, 255).toRGBA();
		q.v[2].color = Color(0, 0, 90, 255).toRGBA();
		q.v[3].color = Color(255, 255, 255, 255).toRGBA();
		q.v[0].uv = Vec2f(0.5f, 0.f);
		q.v[1].uv = Vec2f_ZERO;
		q.v[2].uv = Vec2f_Y_AXIS;
		q.v[3].uv = Vec2f(0.5f, 1.f);
		q.v[0].p = astart;
		q.v[1].p = astart - Vec3f(xx, 0.f, zz);
		q.v[2].p = a - Vec3f(xx, 0.f, zz);
		q.v[3].p = a;
		
		drawQuadRTP(mat, q);
		}
	}
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

void CConfuse::Create() {
	
	SetDuration(ulDuration);
}

void CConfuse::SetPos(const Vec3f & pos)
{
	eCurPos = pos;
}

void CConfuse::Update(float timeDelta) {
	ulCurrentTime += timeDelta;
}

void CConfuse::Render() {
	
	int i = 0;
	
	if(ulCurrentTime >= ulDuration)
		return;
	
	RenderMaterial mat;
	mat.setDepthTest(false);
	mat.setBlendType(RenderMaterial::Additive);
	mat.setTexture(tex_trail);
	
	Vec3f stitepos = eCurPos;
	Anglef stiteangle = Anglef(0.f, -glm::degrees(arxtime.get_updated() * ( 1.0f / 500 )), 0.f);
	Color3f stitecolor = Color3f::white;
	Vec3f stitescale = Vec3f_ONE;
	Draw3DObject(spapi, stiteangle, stitepos, stitescale, stitecolor, mat);
	
	for(i = 0; i < 6; i++) {
		
		PARTICLE_DEF * pd = createParticle();
		if(!pd) {
			break;
		}
		
		float ang = rnd() * 360.f;
		float rad = rnd() * 15.f;
		pd->ov = stitepos;
		pd->ov += angleToVectorXZ(ang) * rad;
		
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
		while(glm::abs(t1 - t2) > 0.3f && glm::abs(t2 - t3) > 0.3f) {
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
	cp.m_angle = glm::radians(10.f);
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

void CFireField::Update(float timeDelta)
{
	ulCurrentTime += timeDelta;

	pPSStream.Update(timeDelta);
	pPSStream1.Update(timeDelta);
}

void CFireField::Render() {
	pPSStream.Render();
	pPSStream1.Render();
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

void CIceField::Create(Vec3f aeSrc) {
	
	SetDuration(ulDuration);
	
	eSrc = aeSrc;
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

void CIceField::Update(float timeDelta)
{
	ulCurrentTime += timeDelta;
}

void CIceField::Render()
{
	if(!VisibleSphere(eSrc - Vec3f(0.f, 120.f, 0.f), 350.f))
		return;

	int i = 0;
	
	RenderMaterial mat;
	mat.setDepthTest(true);
	mat.setBlendType(RenderMaterial::Additive);
	
	for(i = 0; i < iMax; i++) {
		
		tSize[i] += Vec3f(0.1f);
		tSize[i] = glm::min(tSize[i], tSizeMax[i]);
		
		Anglef stiteangle = Anglef::ZERO;
		Vec3f stitepos;
		Vec3f stitescale;
		Color3f stitecolor;

		stiteangle.setPitch(glm::cos(glm::radians(tPos[i].x)) * 360);
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
			Draw3DObject(smotte, stiteangle, stitepos, stitescale, stitecolor, mat);
		else
			Draw3DObject(stite, stiteangle, stitepos, stitescale, stitecolor, mat);
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
}
