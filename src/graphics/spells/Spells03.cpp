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

#include "graphics/spells/Spells03.h"

#include <climits>

#include "core/GameTime.h"

#include "game/Damage.h"
#include "game/EntityManager.h"
#include "game/Player.h"
#include "game/Spells.h"

#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/SpellEffects.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/Particle.h"
#include "graphics/particle/ParticleParams.h"
#include "graphics/spells/Spells05.h"

#include "scene/Object.h"
#include "scene/Interactive.h"

CFireBall::CFireBall() : CSpellFx() {
	
	eSrc = Vec3f_ZERO;
	
	SetDuration(2000);
	ulCurrentTime = ulDuration + 1;
	
	bExplo = false;
	
	// TODO using memset on a class is naughty
	
	memset(&fire_1, 0, sizeof(fire_1));
	fire_1.m_nbMax = 200;
	fire_1.m_life = 550;
	fire_1.m_lifeRandom = 500;
	fire_1.m_pos = Vec3f_ZERO;
	fire_1.m_angle = radians(3);
	fire_1.m_speed = 0;
	fire_1.m_speedRandom = 0;
	fire_1.m_gravity = Vec3f_ZERO;
	fire_1.m_flash = 0;
	fire_1.m_rotation = 50;
	fire_1.fStartColor[0] = 22;
	fire_1.fStartColor[1] = 30;
	fire_1.fStartColor[2] = 30;
	fire_1.fStartColor[3] = 0;
	fire_1.fStartColorRandom[0] = 22;
	fire_1.fStartColorRandom[1] = 0;
	fire_1.fStartColorRandom[2] = 0;
	fire_1.fStartColorRandom[3] = 2;
	fire_1.fEndSize = 0;
	fire_1.fEndSizeRandom = 2;
	fire_1.fEndColor[0] = 25;
	fire_1.fEndColor[1] = 25;
	fire_1.fEndColor[2] = 0;
	fire_1.fEndColor[3] = 50;
	fire_1.fEndColorRandom[0] = 50; 
	fire_1.fEndColorRandom[1] = 0;
	fire_1.fEndColorRandom[2] = 0;
	fire_1.fEndColorRandom[3] = 120; 
	fire_1.blendMode = RenderMaterial::Additive;

	memset(&fire_2, 0, sizeof(fire_2));
	fire_2.m_nbMax = 20;
	fire_2.m_life = 550;
	fire_2.m_lifeRandom = 500;
	fire_2.m_pos = Vec3f_ZERO;
	fire_2.m_angle = radians(3);
	fire_2.m_speed = 0;
	fire_2.m_speedRandom = 0;
	fire_2.m_gravity = Vec3f_ZERO;
	fire_2.m_flash = 0;
	fire_2.m_rotation = 50;
	fire_2.fStartColor[0] = 22;
	fire_2.fStartColor[1] = 30;
	fire_2.fStartColor[2] = 30;
	fire_2.fStartColor[3] = 0;
	fire_2.fStartColorRandom[0] = 22;
	fire_2.fStartColorRandom[1] = 0;
	fire_2.fStartColorRandom[2] = 0;
	fire_2.fStartColorRandom[3] = 2;
	fire_2.fEndColor[0] = 25;
	fire_2.fEndColor[1] = 25;
	fire_2.fEndColor[2] = 0;
	fire_2.fEndColor[3] = 0;
	fire_2.fEndColorRandom[0] = 50; 
	fire_2.fEndColorRandom[1] = 0;
	fire_2.fEndColorRandom[2] = 0;
	fire_2.fEndColorRandom[3] = 120;
	fire_2.blendMode = RenderMaterial::Additive;

	memset(&smoke, 0, sizeof(smoke));
	smoke.m_nbMax = 30;
	smoke.m_life = 2000;
	smoke.m_lifeRandom = 3000;
	smoke.m_pos = Vec3f(0, 10, 10);
	smoke.m_angle = radians(9);
	smoke.m_speed = 150;
	smoke.m_speedRandom = 150;
	smoke.m_gravity = Vec3f(0, -10, 0);
	smoke.m_flash = 0;
	smoke.m_rotation = 90;
	smoke.fStartSize = 0;
	smoke.fStartSizeRandom = 2;
	smoke.fStartColor[0] = 70;
	smoke.fStartColor[1] = 70;
	smoke.fStartColor[2] = 51;
	smoke.fStartColor[3] = 50;
	smoke.fStartColorRandom[0] = 0;
	smoke.fStartColorRandom[1] = 0;
	smoke.fStartColorRandom[2] = 0;
	smoke.fStartColorRandom[3] = 0;
	smoke.fEndColor[0] = 0;
	smoke.fEndColor[1] = 0;
	smoke.fEndColor[2] = 0;
	smoke.fEndColor[3] = 27; 
	smoke.fEndColorRandom[0] = 0;
	smoke.fEndColorRandom[1] = 0;
	smoke.fEndColorRandom[2] = 0;
	smoke.fEndColorRandom[3] = 20;
	smoke.blendMode = RenderMaterial::Additive;
}

CFireBall::~CFireBall()
{
}

void CFireBall::SetTTL(unsigned long aulTTL)
{
	unsigned long t = ulCurrentTime;
	ulDuration = min(ulCurrentTime + aulTTL, ulDuration);
	SetDuration(ulDuration);
	ulCurrentTime = t;

	std::list<Particle *>::iterator i;

	unsigned long ulCalc = ulDuration - ulCurrentTime ;
	arx_assert(ulCalc <= LONG_MAX);
	long ff = static_cast<long>(ulCalc);

	for(i = pPSSmoke.listParticle.begin(); i != pPSSmoke.listParticle.end(); ++i) {
		Particle * pP = *i;

		if(pP->isAlive()) {
			if(pP->ulTime + ff < pP->ulTTL) {
				pP->ulTime = pP->ulTTL - ff;
			}
		}
	}
	
	lLightId = -1;
}

void CFireBall::Create(Vec3f aeSrc, float afBeta, float afAlpha, float _fLevel)
{
	SetDuration(ulDuration);
	SetAngle(afBeta);

	eSrc.x = aeSrc.x - fBetaRadSin * 60;
	eSrc.y = aeSrc.y;
	eSrc.z = aeSrc.z + fBetaRadCos * 60;

	eMove.x = - fBetaRadSin * 80 * cos(radians(MAKEANGLE(afAlpha)));
	eMove.y = sin(radians(MAKEANGLE(afAlpha))) * 80;
	eMove.z = + fBetaRadCos * 80 * cos(radians(MAKEANGLE(afAlpha)));
	
	//FIRE
	fire_1.m_direction = -eMove;
	fire_1.fStartSize = 1 * _fLevel; 
	fire_1.fStartSizeRandom = 2 * _fLevel; 

	pPSFire.SetParams(fire_1);
	pPSFire.fParticleFreq = 100.0f;
	pPSFire.ulParticleSpawn = 0;
	pPSFire.SetTexture("graph/particles/fire", 0, 200);

	pPSFire.Update(0);

	//FIRE
	fire_2.m_direction = -eMove;
	fire_2.fStartSize = 1 * _fLevel; 
	fire_2.fStartSizeRandom = 2 * _fLevel; 
	fire_2.fEndSize = 3 * _fLevel; 
	fire_2.fEndSizeRandom = 2 * _fLevel; 

	pPSFire2.SetParams(fire_2);
	pPSFire2.fParticleFreq = 20.0f;
	pPSFire2.ulParticleSpawn = 0;
	pPSFire2.SetTexture("graph/particles/fire", 0, 200);
	pPSFire2.Update(0);

	// Smoke
	smoke.m_direction = -eMove;
	smoke.fEndSize = 7 * _fLevel;
	smoke.fEndSizeRandom = 2 * _fLevel; 

	pPSSmoke.SetParams(smoke);
	pPSSmoke.ulParticleSpawn = 0;
	pPSSmoke.fParticleFreq = 20.0f;

	pPSSmoke.SetTexture("graph/particles/big_greypouf", 0, 0);
	pPSSmoke.Update(0);

	pPSFire.SetPos(eSrc);
	pPSFire2.SetPos(eSrc);
	pPSSmoke.SetPos(eSrc);

	// Light
	lLightId = -1; 
	eCurPos = eSrc;
}

#define MIN_TIME_FIREBALL 2000  //750
void CFireBall::Update(unsigned long aulTime)
{
	ulCurrentTime += aulTime;

	if(ulCurrentTime > MIN_TIME_FIREBALL) {
		// smoke en retard
		pPSSmoke.SetPos(eCurPos);
		pPSSmoke.Update(aulTime);
		eCurPos += eMove * (aulTime * 0.0045f);
		pPSFire.SetPos(eCurPos);
		pPSFire.fParticleSpeed = 100;
		pPSFire.fParticleSpeedRandom = 200;
		pPSFire.p3ParticleGravity = -eMove * 2.f;
		pPSFire2.p3ParticleGravity = -eMove * 2.f;
		pPSFire2.SetPos(eCurPos);
		pPSFire2.fParticleSpeed = 100;
		pPSFire2.fParticleSpeedRandom = 100;
	} else {
		float afAlpha = 0.f;
	
		if(spells[spellinstance].m_caster == 0) {
			SetAngle(player.angle.getPitch());
			afAlpha = player.angle.getYaw();
			long idx = GetGroupOriginByName(entities[spells[spellinstance].m_caster]->obj, "chest");

			if(idx) {
				eCurPos.x = entities[spells[spellinstance].m_caster]->obj->vertexlist3[idx].v.x - fBetaRadSin * 60;
				eCurPos.y = entities[spells[spellinstance].m_caster]->obj->vertexlist3[idx].v.y;
				eCurPos.z = entities[spells[spellinstance].m_caster]->obj->vertexlist3[idx].v.z + fBetaRadCos * 60;
			} else {
				eCurPos.x = player.pos.x - fBetaRadSin * 60;
				eCurPos.y = player.pos.y;
				eCurPos.z = player.pos.z + fBetaRadCos * 60;
			}
		} else {
			SetAngle(entities[spells[spellinstance].m_caster]->angle.getPitch());

			eCurPos.x = entities[spells[spellinstance].m_caster]->pos.x - fBetaRadSin * 60;
			eCurPos.y = entities[spells[spellinstance].m_caster]->pos.y;
			eCurPos.z = entities[spells[spellinstance].m_caster]->pos.z + fBetaRadCos * 60;

			if ((ValidIONum(spells[spellinstance].m_caster))
			        && (entities[spells[spellinstance].m_caster]->ioflags & IO_NPC))
			{
				eCurPos.x -= std::sin(radians(entities[spells[spellinstance].m_caster]->angle.getPitch())) * 30.f;
				eCurPos.y -= 80.f;
				eCurPos.z += std::cos(radians(entities[spells[spellinstance].m_caster]->angle.getPitch())) * 30.f;
			}
			
			Entity * io = entities[spells[spellinstance].m_caster];

			if(ValidIONum(io->targetinfo)) {
				Vec3f * p1 = &eCurPos;
				Vec3f p2 = entities[io->targetinfo]->pos;
				p2.y -= 60.f;
				afAlpha = 360.f - (degrees(getAngle(p1->y, p1->z, p2.y, p2.z + glm::distance(Vec2f(p2.x, p2.z), Vec2f(p1->x, p1->z))))); //alpha entre orgn et dest;
			}
		}

		eMove.x = - fBetaRadSin * 100 * cos(radians(MAKEANGLE(afAlpha)));
		eMove.y = sin(radians(MAKEANGLE(afAlpha))) * 100;
		eMove.z = + fBetaRadCos * 100 * cos(radians(MAKEANGLE(afAlpha)));

		Vec3f vMove = glm::normalize(eMove);

		// smoke en retard
		pPSSmoke.p3ParticleDirection = -vMove;
		pPSSmoke.SetPos(eCurPos);
		pPSSmoke.RecomputeDirection();
		eCurPos = eCurPos + eMove * (aulTime * 0.0045f);
		pPSFire.p3ParticleDirection = -vMove;
		pPSFire.RecomputeDirection();
		pPSFire.SetPos(eCurPos);
		pPSFire.p3ParticleGravity = -eMove * 2.f;
		pPSFire2.p3ParticleDirection = -vMove;
		pPSFire2.p3ParticleGravity = -eMove * 2.f;
		pPSFire2.RecomputeDirection();
		pPSFire2.SetPos(eCurPos);
	}
	
	pPSFire.Update(aulTime);
	pPSFire2.Update(aulTime);
}

void CFireBall::Render() {
	
	if(ulCurrentTime >= ulDuration)
		return;
	
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	
	pPSFire.Render();
	pPSFire2.Render();
	pPSSmoke.Render();
}

CIceProjectile::~CIceProjectile()
{
	stite_count--;

	if(stite && (stite_count <= 0)) {
		stite_count = 0;
		delete stite;
		stite = NULL;
	}

	smotte_count--;

	if(smotte && (smotte_count <= 0)) {
		smotte_count = 0;
		delete smotte;
		smotte = NULL;
	}
}

CIceProjectile::CIceProjectile()
	: fColor(1.f)
{
	SetDuration(1000);
	ulCurrentTime = ulDuration + 1;

	iNumber = MAX_ICE;

	tex_p1 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_blueting");
	tex_p2 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_bluepouf");

	if (!stite)
		stite = LoadTheObj("graph/obj3d/interactive/fix_inter/stalagmite/stalagmite.teo");

	stite_count++;

	if (!smotte)
		smotte = LoadTheObj("graph/obj3d/interactive/fix_inter/stalagmite/motte.teo");

	smotte_count++;

	iMax = MAX_ICE;
	fStep = 20;
}

void CIceProjectile::Create(Vec3f aeSrc, float afBeta, float fLevel)
{
	iMax = (int)(30 + fLevel * 5.2f);

	Create(aeSrc, afBeta);
}

void CIceProjectile::Create(Vec3f aeSrc, float afBeta)
{
	SetDuration(ulDuration);
	SetAngle(afBeta);
	
	float xmin, ymin, zmin;

	Vec3f s, e, h;

	s.x					= aeSrc.x;
	s.y					= aeSrc.y - 100;
	s.z					= aeSrc.z;

	float fspelldist	= static_cast<float>(iMax * 15);

	fspelldist = min(fspelldist, 200.0f);
	fspelldist = max(fspelldist, 450.0f);
	e.x = aeSrc.x - fBetaRadSin * fspelldist;
	e.y = aeSrc.y - 100;
	e.z = aeSrc.z + fBetaRadCos * fspelldist;

	float fd;

	if(!Visible(s, e, NULL, &h)) {
		e.x = h.x + fBetaRadSin * 20;
		e.y = h.y;
		e.z = h.z - fBetaRadCos * 20;
	}

	fd = fdist(s, e);

	float fCalc = ulDuration * (fd / fspelldist);
	SetDuration(checked_range_cast<unsigned long>(fCalc));

	float fDist = (fd / fspelldist) * iMax ;

	iNumber = checked_range_cast<int>(fDist);

	int end = iNumber / 2;
	tv1a[0].p = s + Vec3f(0.f, 100.f, 0.f);
	tv1a[end].p = e + Vec3f(0.f, 100.f, 0.f);

	Split(tv1a, 0, end, 80, 0.5f, 0, 1, 80, 0.5f);

	for(int i = 0; i < iNumber; i++) {
		float t = rnd();

		if (t < 0.5f)
			tType[i] = 0;
		else
			tType[i] = 1;

		tSize[i] = Vec3f_ZERO;
		tSizeMax[i] = randomVec() + Vec3f(0.f, 0.2f, 0.f);

		if(tType[i] == 0) {
			xmin = 1.2f;
			ymin = 1;
			zmin = 1.2f;
		} else {
			xmin = 0.4f;
			ymin = 0.3f;
			zmin = 0.4f;
		}

		if(tSizeMax[i].x < xmin)
			tSizeMax[i].x = xmin;

		if(tSizeMax[i].y < ymin)
			tSizeMax[i].y = ymin;

		if(tSizeMax[i].z < zmin)
			tSizeMax[i].z = zmin;

		int iNum = static_cast<int>(i / 2);

		if(tType[i] == 0) {
			tPos[i].x = tv1a[iNum].p.x + frand2() * 80;
			tPos[i].y = tv1a[iNum].p.y;
			tPos[i].z = tv1a[iNum].p.z + frand2() * 80;
		} else {
			tPos[i].x = tv1a[iNum].p.x + frand2() * 40;
			tPos[i].y = tv1a[iNum].p.y;
			tPos[i].z = tv1a[iNum].p.z + frand2() * 40;
		}

		long ttt = ARX_DAMAGES_GetFree();
		if(ttt != -1) {
			damages[ttt].pos = tPos[i];
			damages[ttt].radius = 60.f;
			damages[ttt].damages = 0.1f * spells[spellinstance].m_caster_level;
			damages[ttt].area = DAMAGE_FULL;
			damages[ttt].duration = ulDuration;
			damages[ttt].source = spells[spellinstance].m_caster;
			damages[ttt].flags = DAMAGE_FLAG_DONT_HURT_SOURCE;
			damages[ttt].type = DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_COLD;
			damages[ttt].exist = true;
		}
	}

	fColor = 1;
}

void CIceProjectile::Update(unsigned long aulTime)
{
	ulCurrentTime += aulTime;

	if(ulDuration - ulCurrentTime < 1000) {
		fColor = (ulDuration - ulCurrentTime) * ( 1.0f / 1000 );

		for(int i = 0; i < iNumber; i++) {
			tSize[i].y *= fColor;
		}
	}
}

void CIceProjectile::Render()
{
	int i = 0;

	if(ulCurrentTime >= ulDuration)
		return;

	GRenderer->SetCulling(Renderer::CullCW);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetBlendFunc(Renderer::BlendInvDstColor, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	iMax = (int)((iNumber * 2) * fOneOnDuration * ulCurrentTime);

	if(iMax > iNumber)
		iMax = iNumber;

	for(i = 0; i < iMax; i++) {
		if(tSize[i].x < tSizeMax[i].x)
			tSize[i].x += 0.1f;

		if(tSize[i].x > tSizeMax[i].x)
			tSize[i].x = tSizeMax[i].x;

		if(tSize[i].y < tSizeMax[i].y)
			tSize[i].y += 0.1f;

		if(tSize[i].y < 0)
			tSize[i].y = 0;

		if(tSize[i].y > tSizeMax[i].y)
			tSize[i].y = tSizeMax[i].y;

		if(tSize[i].z < tSizeMax[i].z)
			tSize[i].z += 0.1f;

		if(tSize[i].z > tSizeMax[i].z)
			tSize[i].z = tSizeMax[i].z;

		Anglef stiteangle;
		Vec3f stitepos;
		Vec3f stitescale;
		Color3f stitecolor;

		stiteangle.setPitch((float) cos(radians(tPos[i].x)) * 360);
		stiteangle.setYaw(0);
		stiteangle.setRoll(0);
		stitepos = tPos[i];

		float tt;
		tt = tSizeMax[i].y * fColor;
		stitecolor.g = stitecolor.r = tt * 0.7f;
		stitecolor.b = tt * 0.9f;

		if(stitecolor.r > 1)
			stitecolor.r = 1;

		if(stitecolor.g > 1)
			stitecolor.g = 1;

		if(stitecolor.b > 1)
			stitecolor.b = 1;

		stitescale = tSize[i];

		if(tType[i] == 0)
			Draw3DObject(smotte, stiteangle, stitepos, stitescale, stitecolor);
		else
			Draw3DObject(stite, stiteangle, stitepos, stitescale, stitecolor);
	}
	
	for(i = 0; i < min(iNumber, iMax + 1); i++) {
		
		float t = rnd();
		if(t < 0.01f) {
			
			PARTICLE_DEF * pd = createParticle();
			if(pd) {
				pd->ov = tPos[i] + randomVec(-5.f, 5.f);
				pd->move = randomVec(-2.f, 2.f);
				pd->siz = 20.f;
				float t = min(2000.f + rnd() * 2000.f,
				              ulDuration - ulCurrentTime + 500.0f * rnd());
				pd->tolive = checked_range_cast<unsigned long>(t);
				pd->tc = tex_p2;
				pd->special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
				pd->fparam = 0.0000001f;
				pd->rgb = Color3f(0.7f, 0.7f, 1.f);
			}
			
		} else if(t > 0.095f) {
			
			PARTICLE_DEF * pd = createParticle();
			if(pd) {
				pd->ov = tPos[i] + randomVec(-5.f, 5.f) - Vec3f(0.f, 50.f, 0.f);
				pd->move = Vec3f(0.f, 2.f - 4.f * rnd(), 0.f);
				pd->siz = 0.5f;
				float t = min(2000.f + rnd() * 1000.f,
				              ulDuration - ulCurrentTime + 500.0f * rnd());
				pd->tolive = checked_range_cast<unsigned long>(t);
				pd->tc = tex_p1;
				pd->special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
				pd->fparam = 0.0000001f;
				pd->rgb = Color3f(0.7f, 0.7f, 1.f);
			}
			
		}
	}
}

//-----------------------------------------------------------------------------
// SPEED
//-----------------------------------------------------------------------------
CSpeed::~CSpeed()
{
	for(size_t i = 0; i < m_trails.size(); i++) {
		delete m_trails[i].trail;
	}
}

void CSpeed::Create(int numinteractive)
{
	this->num = numinteractive;
	
	std::vector<VertexGroup> & grouplist = entities[this->num]->obj->grouplist;
	
	bool skip = true;
	std::vector<VertexGroup>::const_iterator itr;
	for(itr = grouplist.begin(); itr != grouplist.end(); ++itr) {
		skip = !skip;
		
		if(skip) {
			continue;
		}
		
		float col = 0.05f + (rnd() * 0.05f);
		float size = 1.f + (0.5f * rnd());
		int taille = Random::get(130, 260);
		
		SpeedTrail trail;
		trail.vertexIndex = itr->origin;	
		trail.trail = new Trail(taille, Color4f::gray(col), Color4f::black, size, 0.f);
		
		m_trails.push_back(trail);
	}
}

void CSpeed::Update(unsigned long _ulTime)
{
	ARX_UNUSED(_ulTime);
	
	for(size_t i = 0; i < m_trails.size(); i++) {
		Vec3f pos = entities[this->num]->obj->vertexlist3[m_trails[i].vertexIndex].v;
		
		m_trails[i].trail->SetNextPosition(pos);
		m_trails[i].trail->Update(_ulTime);
	}
}

void CSpeed::Render()
{
	for(size_t i = 0; i < m_trails.size(); i++) {
		m_trails[i].trail->Render();
	}
}

CCreateFood::CCreateFood() {
	SetDuration(1000);
	ulCurrentTime = ulDuration + 1;
	pPS = new ParticleSystem();
}

CCreateFood::~CCreateFood() {
	delete pPS;
}

void CCreateFood::Create() {
	
	eSrc = player.pos;
	
	pPS->SetPos(eSrc);
	ParticleParams cp;
	memset(&cp, 0, sizeof(cp));
	cp.m_nbMax = 350;
	cp.m_life = 800;
	cp.m_lifeRandom = 2000;
	cp.m_pos = Vec3f(100, 200, 100);
	cp.m_direction = Vec3f(0, -10, 0);
	cp.m_angle = radians(5);
	cp.m_speed = 120;
	cp.m_speedRandom = 84;
	cp.m_gravity = Vec3f(0, -10, 0);
	cp.m_flash = 0;
	cp.m_rotation = 80;

	cp.fStartSize = 8;
	cp.fStartSizeRandom = 8;
	cp.fStartColor[0] = 105; 
	cp.fStartColor[1] = 105; 
	cp.fStartColor[2] = 20;
	cp.fStartColor[3] = 145; 
	cp.fStartColorRandom[0] = 50;
	cp.fStartColorRandom[1] = 50;
	cp.fStartColorRandom[2] = 0;
	cp.fStartColorRandom[3] = 10;

	cp.fEndSize = 6;
	cp.fEndSizeRandom = 4;
	cp.fEndColor[0] = 20;
	cp.fEndColor[1] = 20;
	cp.fEndColor[2] = 5;
	cp.fEndColor[3] = 0;
	cp.fEndColorRandom[0] = 40;
	cp.fEndColorRandom[1] = 40;
	cp.fEndColorRandom[2] = 0;
	cp.fEndColorRandom[3] = 0;

	cp.blendMode = RenderMaterial::Additive;

	pPS->SetParams(cp);
	pPS->ulParticleSpawn = PARTICLE_CIRCULAR | PARTICLE_BORDER;
	pPS->SetTexture("graph/particles/create_food", 0, 100); //5
}

void CCreateFood::Update(unsigned long aulTime)
{
	ulCurrentTime += aulTime;

//ARX_BEGIN: jycorbel (2010-07-20) - @TBE : dead code
/*
if (ulCurrentTime >= ulDuration)
*/
//ARX_END: jycorbel (2010-07-20)

	eSrc = entities.player()->pos;


//ARX_BEGIN: jycorbel (2010-07-20) - Correct bug when this spell is cast, the function update particule after-life
	if( ulCurrentTime <= ulDuration ) //ulCalc (time before end spell) must be positive !
//ARX_END: jycorbel (2010-07-20)
	{
	unsigned long ulCalc = ulDuration - ulCurrentTime ;
	arx_assert(ulCalc <= LONG_MAX);
	long ff =  static_cast<long>(ulCalc);

		if(ff < 1500) {
			pPS->ulParticleSpawn = PARTICLE_CIRCULAR;
			pPS->p3ParticleGravity = Vec3f_ZERO;

		std::list<Particle *>::iterator i;

		for(i = pPS->listParticle.begin(); i != pPS->listParticle.end(); ++i) {
			Particle * pP = *i;

			if(pP->isAlive()) {
				pP->fColorEnd[3] = 0;

					if(pP->ulTime + ff < pP->ulTTL) {
						pP->ulTime = pP->ulTTL - ff;
					}
				}
			}
		}
	}

	pPS->SetPos(eSrc);
	pPS->Update(aulTime);
}

void CCreateFood::Render()
{
	pPS->Render();
}
