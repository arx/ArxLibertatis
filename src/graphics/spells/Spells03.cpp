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

CFireBall::CFireBall()
	: CSpellFx()
	, m_createBallDuration(2000)
{
	
	eSrc = Vec3f_ZERO;
	
	SetDuration(2000);
	ulCurrentTime = ulDuration + 1;
	
	bExplo = false;
}

CFireBall::~CFireBall()
{
}

void CFireBall::SetTTL(unsigned long aulTTL)
{
	unsigned long t = ulCurrentTime;
	ulDuration = std::min(ulCurrentTime + aulTTL, ulDuration);
	SetDuration(ulDuration);
	ulCurrentTime = t;
	
	lLightId = LightHandle::Invalid;
}

void CFireBall::Create(Vec3f aeSrc, float afBeta, float afAlpha)
{
	SetDuration(ulDuration);
	
	eSrc = aeSrc;
	eSrc += angleToVectorXZ(afBeta) * 60.f;
	
	eMove = angleToVector(Anglef(afAlpha, afBeta, 0.f)) * 80.f;
	
	// Light
	lLightId = LightHandle::Invalid;
	eCurPos = eSrc;
}

void CFireBall::Update(float timeDelta)
{
	ulCurrentTime += timeDelta;


}

void CFireBall::Render() {

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

void CIceProjectile::Create(Vec3f aeSrc, float afBeta, float fLevel, EntityHandle caster)
{
	iMax = (int)(30 + fLevel * 5.2f);
	
	SetDuration(ulDuration);
	
	float fBetaRad = glm::radians(afBeta);
	float fBetaRadCos = glm::cos(fBetaRad);
	float fBetaRadSin = glm::sin(fBetaRad);
	
	Vec3f s, e, h;

	s.x					= aeSrc.x;
	s.y					= aeSrc.y - 100;
	s.z					= aeSrc.z;

	float fspelldist	= static_cast<float>(iMax * 15);

	fspelldist = std::min(fspelldist, 200.0f);
	fspelldist = std::max(fspelldist, 450.0f);
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
	
	Vec3f tv1a[MAX_ICE];
	tv1a[0] = s + Vec3f(0.f, 100.f, 0.f);
	tv1a[end] = e + Vec3f(0.f, 100.f, 0.f);

	Split(tv1a, 0, end, Vec3f(80, 0, 80), Vec3f(0.5f, 1, 0.5f));

	for(int i = 0; i < iNumber; i++) {
		Icicle & icicle = m_icicles[i];
		
		float t = rnd();
		
		Vec3f minSize;
		int randomRange;
		
		if(t < 0.5f) {
			icicle.type = 0;
			minSize = Vec3f(1.2f, 1.f, 1.2f);
			randomRange = 80;
		} else {
			icicle.type = 1;
			minSize = Vec3f(0.4f, 0.3f, 0.4f);
			randomRange = 40;
		}

		icicle.size = Vec3f_ZERO;
		icicle.sizeMax = randomVec() + Vec3f(0.f, 0.2f, 0.f);
		icicle.sizeMax = glm::max(icicle.sizeMax, minSize);
		
		int iNum = static_cast<int>(i / 2);
		icicle.pos = tv1a[iNum] + randomOffsetXZ(randomRange);
		
		DamageParameters damage;
		damage.pos = icicle.pos;
		damage.radius = 60.f;
		damage.damages = 0.1f * fLevel;
		damage.area = DAMAGE_FULL;
		damage.duration = ulDuration;
		damage.source = caster;
		damage.flags = DAMAGE_FLAG_DONT_HURT_SOURCE;
		damage.type = DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_COLD;
		DamageCreate(damage);
	}

	fColor = 1;
}

void CIceProjectile::Update(float timeDelta)
{
	ulCurrentTime += timeDelta;

	if(ulDuration - ulCurrentTime < 1000) {
		fColor = (ulDuration - ulCurrentTime) * ( 1.0f / 1000 );

		for(int i = 0; i < iNumber; i++) {
			m_icicles[i].size.y *= fColor;
		}
	}
}

void CIceProjectile::Render()
{
	if(ulCurrentTime >= ulDuration)
		return;
	
	RenderMaterial mat;
	mat.setCulling(Renderer::CullCW);
	mat.setDepthTest(true);
	mat.setBlendType(RenderMaterial::Screen);
	
	float fOneOnDuration = 1.f / (float)(ulDuration);
	iMax = (int)((iNumber * 2) * fOneOnDuration * ulCurrentTime);

	if(iMax > iNumber)
		iMax = iNumber;

	for(int i = 0; i < iMax; i++) {
		Icicle & icicle = m_icicles[i];
		
		icicle.size += Vec3f(0.1f);
		icicle.size = glm::clamp(icicle.size, Vec3f(0.f), icicle.sizeMax);
		
		Anglef stiteangle;
		Color3f stitecolor;

		stiteangle.setPitch(glm::cos(glm::radians(icicle.pos.x)) * 360);
		stiteangle.setYaw(0);
		stiteangle.setRoll(0);
		
		float tt = icicle.sizeMax.y * fColor;
		stitecolor.g = stitecolor.r = tt * 0.7f;
		stitecolor.b = tt * 0.9f;

		if(stitecolor.r > 1)
			stitecolor.r = 1;

		if(stitecolor.g > 1)
			stitecolor.g = 1;

		if(stitecolor.b > 1)
			stitecolor.b = 1;

		if(icicle.type == 0)
			Draw3DObject(smotte, stiteangle, icicle.pos, icicle.size, stitecolor, mat);
		else
			Draw3DObject(stite, stiteangle, icicle.pos, icicle.size, stitecolor, mat);
	}
	
	for(int i = 0; i < std::min(iNumber, iMax + 1); i++) {
		Icicle & icicle = m_icicles[i];
		
		float t = rnd();
		if(t < 0.01f) {
			
			PARTICLE_DEF * pd = createParticle();
			if(pd) {
				pd->ov = icicle.pos + randomVec(-5.f, 5.f);
				pd->move = randomVec(-2.f, 2.f);
				pd->siz = 20.f;
				float t = std::min(2000.f + rnd() * 2000.f,
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
				pd->ov = icicle.pos + randomVec(-5.f, 5.f) - Vec3f(0.f, 50.f, 0.f);
				pd->move = Vec3f(0.f, 2.f - 4.f * rnd(), 0.f);
				pd->siz = 0.5f;
				float t = std::min(2000.f + rnd() * 1000.f,
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

void CSpeed::Create(EntityHandle numinteractive)
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

void CSpeed::Update(float timeDelta)
{
	ARX_UNUSED(timeDelta);
	
	for(size_t i = 0; i < m_trails.size(); i++) {
		Vec3f pos = entities[this->num]->obj->vertexlist3[m_trails[i].vertexIndex].v;
		
		m_trails[i].trail->SetNextPosition(pos);
		m_trails[i].trail->Update(timeDelta);
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
	ParticleParams cp = ParticleParams();
	cp.m_nbMax = 350;
	cp.m_life = 800;
	cp.m_lifeRandom = 2000;
	cp.m_pos = Vec3f(100, 200, 100);
	cp.m_direction = Vec3f(0, -10, 0) * 0.1f;
	cp.m_angle = glm::radians(5.f);
	cp.m_speed = 120;
	cp.m_speedRandom = 84;
	cp.m_gravity = Vec3f(0, -10, 0);
	cp.m_flash = 0;
	cp.m_rotation = 1.0f / (101 - 80);

	cp.m_startSegment.m_size = 8;
	cp.m_startSegment.m_sizeRandom = 8;
	cp.m_startSegment.m_color = Color(105, 105, 20, 145).to<float>();
	cp.m_startSegment.m_colorRandom = Color(50, 50, 0, 10).to<float>();

	cp.m_endSegment.m_size = 6;
	cp.m_endSegment.m_sizeRandom = 4;
	cp.m_endSegment.m_color = Color(20, 20, 5, 0).to<float>();
	cp.m_endSegment.m_colorRandom = Color(40, 40, 0, 0).to<float>();

	cp.m_blendMode = RenderMaterial::Additive;
	cp.m_texture.set("graph/particles/create_food", 0, 100); //5
	cp.m_spawnFlags = PARTICLE_CIRCULAR | PARTICLE_BORDER;
	
	pPS->SetParams(cp);
}

void CCreateFood::Update(float timeDelta)
{
	ulCurrentTime += timeDelta;
	
	eSrc = entities.player()->pos;
	
	if(ulCurrentTime <= ulDuration) {
		unsigned long ulCalc = ulDuration - ulCurrentTime ;
		arx_assert(ulCalc <= LONG_MAX);
		long ff =  static_cast<long>(ulCalc);
		
		if(ff < 1500) {
			pPS->m_parameters.m_spawnFlags = PARTICLE_CIRCULAR;
			pPS->m_parameters.m_gravity = Vec3f_ZERO;
			
			std::list<Particle *>::iterator i;
			
			for(i = pPS->listParticle.begin(); i != pPS->listParticle.end(); ++i) {
				Particle * pP = *i;
				
				if(pP->isAlive()) {
					pP->fColorEnd.a = 0;
						
					if(pP->m_age + ff < pP->m_timeToLive) {
						pP->m_age = pP->m_timeToLive - ff;
					}
				}
			}
		}
	}

	pPS->SetPos(eSrc);
	pPS->Update(timeDelta);
}

void CCreateFood::Render()
{
	pPS->Render();
}
