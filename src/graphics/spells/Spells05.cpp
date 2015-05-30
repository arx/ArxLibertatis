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

#include "graphics/spells/Spells05.h"

#include <climits>
#include <cmath>

#include "animation/AnimationRender.h"

#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"

#include "game/Damage.h"
#include "game/EntityManager.h"
#include "game/Player.h"
#include "game/Spells.h"

#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/SpellEffects.h"
#include "graphics/effects/Fog.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/Particle.h"
#include "graphics/particle/ParticleManager.h"
#include "graphics/particle/ParticleParams.h"
#include "graphics/texture/TextureStage.h"

#include "scene/Interactive.h"
#include "scene/Light.h"
#include "scene/Object.h"

#include <list>

extern ParticleManager * pParticleManager;


static void LaunchPoisonExplosion(const Vec3f & aePos) {
	
	// système de partoches pour l'explosion
	ParticleSystem * pPS = new ParticleSystem();
	ParticleParams cp = ParticleParams();
	cp.m_nbMax = 80; 
	cp.m_life = 1500;
	cp.m_lifeRandom = 500;
	cp.m_pos = Vec3f(5);
	cp.m_direction = Vec3f(0, 4, 0) * 0.1f;
	cp.m_angle = glm::radians(360.f);
	cp.m_speed = 200;
	cp.m_speedRandom = 0;
	cp.m_gravity = Vec3f(0, 17, 0);
	cp.m_flash = 0;
	cp.m_rotation = 1.0f / (101 - 80);
	cp.m_rotationRandomDirection = true;
	cp.m_rotationRandomStart = true;

	cp.m_startSegment.m_size = 5;
	cp.m_startSegment.m_sizeRandom = 3;
	cp.m_startSegment.m_color = Color(0, 76, 0, 0).to<float>();
	cp.m_startSegment.m_colorRandom = Color(0, 0, 0, 150).to<float>();

	cp.m_endSegment.m_size = 30;
	cp.m_endSegment.m_sizeRandom = 5;
	cp.m_endSegment.m_color = Color(0, 0, 0, 0).to<float>();
	cp.m_endSegment.m_colorRandom = Color(0, 25, 0, 20).to<float>();

	cp.m_blendMode = RenderMaterial::AlphaAdditive;
	cp.m_freq = -1;
	cp.m_texture.set("graph/particles/big_greypouf", 0, 200);
	cp.m_spawnFlags = 0;
	cp.m_looping = false;
	
	pPS->SetParams(cp);
	pPS->SetPos(aePos);
	pPS->Update(0);

	std::list<Particle *>::iterator i;

	for(i = pPS->listParticle.begin(); i != pPS->listParticle.end(); ++i) {
		Particle * pP = *i;

		if(pP->isAlive()) {
			pP->p3Velocity = glm::clamp(pP->p3Velocity, Vec3f(0, -100, 0), Vec3f(0, 100, 0));
		}
	}

	arx_assert(pParticleManager);
	pParticleManager->AddSystem(pPS);
}


CPoisonProjectile::CPoisonProjectile()
	: eSrc(Vec3f_ZERO)
	, lightIntensityFactor(1.f)
	, fBetaRadCos(0.f)
	, fBetaRadSin(0.f)
	, bOk(false)
	, fTrail(-1.f)
{
	SetDuration(2000);
	ulCurrentTime = ulDuration + 1;
}

void CPoisonProjectile::Create(Vec3f _eSrc, float _fBeta)
{
	SetDuration(ulDuration);
	SetAngle(_fBeta);
	eSrc = _eSrc;

	bOk = false;

	eMove = Vec3f(-fBetaRadSin * 2, 0.f, fBetaRadCos * 2); 

	Vec3f tempHit;
	Vec3f dest = eSrc;

	int i = 0;
	while(Visible(eSrc, dest, NULL, &tempHit) && i < 20) {
		dest.x -= fBetaRadSin * 50;
		dest.z += fBetaRadCos * 50;

		i++;
	}

	dest.y += 0.f;

	pathways[0] = eSrc;
	pathways[9] = dest;
	
	Split(pathways, 0, 9, Vec3f(10 * fBetaRadCos, 10, 10 * fBetaRadSin), Vec3f(10, 10, 10));
	
	fTrail = -1;

	//-------------------------------------------------------------------------
	// système de partoches
	ParticleParams cp = ParticleParams();
	cp.m_nbMax = 5;
	cp.m_life = 2000;
	cp.m_lifeRandom = 1000;
	cp.m_pos = Vec3f_ZERO;
	cp.m_direction = -eMove * 0.1f;
	cp.m_angle = 0;
	cp.m_speed = 10;
	cp.m_speedRandom = 10;
	cp.m_gravity = Vec3f_ZERO;
	cp.m_flash = 21 * (1.f/100);
	cp.m_rotation = 1.0f / (101 - 80);
	cp.m_rotationRandomDirection = true;
	cp.m_rotationRandomStart = true;

	cp.m_startSegment.m_size = 5;
	cp.m_startSegment.m_sizeRandom = 3;
	cp.m_startSegment.m_color = Color(0, 50, 0, 40).to<float>();
	cp.m_startSegment.m_colorRandom = Color(0, 100, 0, 50).to<float>();

	cp.m_endSegment.m_size = 8;
	cp.m_endSegment.m_sizeRandom = 13;
	cp.m_endSegment.m_color = Color(0, 60, 0, 40).to<float>();
	cp.m_endSegment.m_colorRandom = Color(0, 100, 0, 50).to<float>();

	cp.m_blendMode = RenderMaterial::Screen;
	cp.m_freq = -1;
	cp.m_texture.set("graph/particles/big_greypouf", 0, 200);
	cp.m_spawnFlags = 0;
	
	pPS.SetParams(cp);
	pPS.SetPos(eSrc);
	pPS.Update(0);
}

void CPoisonProjectile::Update(float timeDelta)
{
	if(ulCurrentTime <= 2000) {
		ulCurrentTime += timeDelta;
	}

	// on passe de 5 à 100 partoches en 1.5secs
	if(ulCurrentTime < 750) {
		pPS.m_parameters.m_nbMax = 2;
		pPS.Update(timeDelta);
	} else {
		if(!bOk) {
			bOk = true;

			// go
			ParticleParams cp = ParticleParams();
			cp.m_nbMax = 100;
			cp.m_life = 500;
			cp.m_lifeRandom = 300;
			cp.m_pos = Vec3f(fBetaRadSin * 20, 0.f, fBetaRadCos * 20);

			cp.m_direction = -eMove * 0.1f;

			cp.m_angle = glm::radians(4.f);
			cp.m_speed = 150;
			cp.m_speedRandom = 50;//15;
			cp.m_gravity = Vec3f(0, 10, 0);
			cp.m_flash = 0;
			cp.m_rotation = 1.0f / (101 - 80);
			cp.m_rotationRandomDirection = true;
			cp.m_rotationRandomStart = true;

			cp.m_startSegment.m_size = 2;
			cp.m_startSegment.m_sizeRandom = 2;
			cp.m_startSegment.m_color = Color(0, 39, 0, 100).to<float>();
			cp.m_startSegment.m_colorRandom = Color(50, 21, 0, 0).to<float>();

			cp.m_endSegment.m_size = 7;
			cp.m_endSegment.m_sizeRandom = 5;
			cp.m_endSegment.m_color = Color(0, 25, 0, 100).to<float>();
			cp.m_endSegment.m_colorRandom = Color(50, 20, 0, 0).to<float>();

			cp.m_blendMode = RenderMaterial::Screen;
			cp.m_freq = 80;
			cp.m_texture.set("graph/particles/big_greypouf", 0, 200);
			cp.m_spawnFlags = 0;
			
			pPSStream.SetParams(cp);
		}

		pPSStream.Update(timeDelta);
		pPSStream.SetPos(eCurPos);

		pPS.Update(timeDelta);
		pPS.SetPos(eCurPos);

		fTrail = ((ulCurrentTime - 750) * (1.0f / (ulDuration - 750.0f))) * 9 * (BEZIERPrecision + 2);
	}

	if(ulCurrentTime >= ulDuration)
		lightIntensityFactor = 0.f;
	else
		lightIntensityFactor = 1.f;
}

void CPoisonProjectile::Render() {
	
	if(ulCurrentTime >= ulDuration)
		return;
	
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	
	int n = BEZIERPrecision;
	float delta = 1.0f / n;
	
	Vec3f lastpos = pathways[0];
	
	int i = 0;
	for(i = 0; i < 9; i++) {
		
		int kpprec = std::max(i - 1, 0);
		int kpsuiv = i + 1 ;
		int kpsuivsuiv = (i < (9 - 2)) ? kpsuiv + 1 : kpsuiv;
		
		for(int toto = 1; toto < n; toto++) {
			
			if(fTrail < i * n + toto) {
				break;
			}
			
			float t = toto * delta;
			
			float t2 = t * t ;
			float t3 = t2 * t ;
			float f0 = 2.f * t3 - 3.f * t2 + 1.f ;
			float f1 = -2.f * t3 + 3.f * t2 ;
			float f2 = t3 - 2.f * t2 + t ;
			float f3 = t3 - t2 ;
			
			float val = pathways[kpsuiv].x;
			float p0 = 0.5f * (val - pathways[kpprec].x);
			float p1 = 0.5f * (pathways[kpsuivsuiv].x - pathways[i].x);
			lastpos.x = f0 * pathways[i].x + f1 * val + f2 * p0 + f3 * p1;
			
			val = pathways[kpsuiv].y;
			p0 = 0.5f * (val - pathways[kpprec].y);
			p1 = 0.5f * (pathways[kpsuivsuiv].y - pathways[i].y);
			lastpos.y = f0 * pathways[i].y + f1 * val + f2 * p0 + f3 * p1;
			
			val = pathways[kpsuiv].z;
			p0 = 0.5f * (val - pathways[kpprec].z);
			p1 = 0.5f * (pathways[kpsuivsuiv].z - pathways[i].z);
			lastpos.z = f0 * pathways[i].z + f1 * val + f2 * p0 + f3 * p1;
		}
	}
	
	eCurPos = lastpos;
	
	if(fTrail >= (i * n)) {
		LaunchPoisonExplosion(lastpos);
	}
	
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
}

//-----------------------------------------------------------------------------
//	LEVITATION
//-----------------------------------------------------------------------------

CLevitate::CLevitate()
	: m_key(0)
	, m_def(16)
	, m_pos(Vec3f_ZERO)
	, m_baseRadius(50.f)
	, m_rhaut(100.f)
	, m_hauteur(80.f)
	, m_coneScale(0.f)
	, m_ang(0.f)
	, m_currdurationang(0)
	, m_currframetime(0)
	, m_tsouffle(NULL)
	, m_stoneDelay(0)
	, m_nbstone(0)
{
	int nb = 2;

	while(nb--) {
		m_cone[nb].coned3d = NULL;
		m_cone[nb].coneind = NULL;
		m_cone[nb].conevertex = NULL;
	}
}

CLevitate::~CLevitate() {
}

void CLevitate::CreateConeStrip(float rbase, float rhaut, float hauteur, int def, int numcone) {
	
	T_CONE & c = m_cone[numcone];
	
	free(c.coned3d);
	free(c.conevertex);
	free(c.coneind);
	
	c.conenbvertex = def * 2 + 2;
	c.conenbfaces = def * 2 + 2;
	c.coned3d = (TexturedVertex *)malloc(c.conenbvertex * sizeof(TexturedVertex));
	c.conevertex = (Vec3f *)malloc(c.conenbvertex * sizeof(Vec3f));
	c.coneind = (unsigned short *)malloc(c.conenbvertex * sizeof(unsigned short));
	
	Vec3f * vertex = c.conevertex;
	unsigned short * pind = c.coneind;
	unsigned short ind = 0;
	int nb;
	float a = 0.f;
	float da = 360.f / (float)def;
	nb = m_cone[numcone].conenbvertex >> 1;
	
	while(nb) {
		*pind++ = ind++;
		*pind++ = ind++;
		*vertex++ = Vec3f(rhaut * std::cos(glm::radians(a)), -hauteur, rhaut * std::sin(glm::radians(a)));
		*vertex++ = Vec3f(rbase * std::cos(glm::radians(a)), 0.f, rbase * std::sin(glm::radians(a)));
		a += da;
		nb--;
	}
}

void CLevitate::Create(int def, float rbase, float rhaut, float hauteur, Vec3f * pos, unsigned long _ulDuration)
{
	SetDuration(_ulDuration);

	if(def < 3)
		return;

	CreateConeStrip(rbase, rhaut, hauteur, def, 0);
	CreateConeStrip(rbase, rhaut * 1.5f, hauteur * 0.5f, def, 1);

	m_key = 0;
	m_pos = *pos;
	m_baseRadius = rbase;
	m_rhaut = rhaut;
	m_hauteur = hauteur;
	m_currdurationang = 0;
	m_coneScale = 0.f;
	m_ang = 0.f;
	m_def = (short)def;
	m_tsouffle = TextureContainer::Load("graph/obj3d/textures/(fx)_sebsouffle");

	m_stoneDelay = 0;
	m_nbstone = 0;
	
	int nb = 256;

	while(nb--) {
		m_tstone[nb].actif = 0;
	}
}

void CLevitate::AddStone(const Vec3f & pos) {
	
	if(arxtime.is_paused() || m_nbstone > 255) {
		return;
	}
	
	int nb = 256;
	while(nb--) {
		if(!m_tstone[nb].actif) {
			m_nbstone++;
			
			T_STONE stone;
			stone.actif = 1;
			stone.numstone = Random::get(0, 1);
			stone.pos = pos;
			stone.yvel = rnd() * -5.f;
			stone.ang = Anglef(rnd(), rnd(), rnd()) * Anglef(360.f, 360.f, 360.f);
			stone.angvel = Anglef(rnd(), rnd(), rnd()) * Anglef(5.f, 6.f, 3.f);
			stone.scale = Vec3f(0.2f + rnd() * 0.3f);
			stone.time = Random::get(2000, 2500);
			stone.currtime = 0;
			
			m_tstone[nb] = stone;
			break;
		}
	}
}

void CLevitate::DrawStone()
{
	RenderMaterial mat;
	mat.setDepthTest(true);
	mat.setBlendType(RenderMaterial::Screen);
	
	int	nb = 256;

	while(nb--) {
		T_STONE & s = m_tstone[nb];
		
		if(s.actif) {
			float a = (float)s.currtime / (float)s.time;

			if(a > 1.f) {
				a = 1.f;
				s.actif = 0;
			}

			Color4f col = Color4f(Color3f::white, 1.f - a);

			EERIE_3DOBJ * obj = (s.numstone == 0) ? stone0 : stone1;
			
			Draw3DObject(obj, s.ang, s.pos, s.scale, Color4f(col), mat);
			
			PARTICLE_DEF * pd = createParticle();
			if(pd) {
				pd->ov = s.pos;
				pd->move = Vec3f(0.f, 3.f * rnd(), 0.f);
				pd->siz = 3.f + 3.f * rnd();
				pd->tolive = 1000;
				pd->timcreation = -(long(arxtime) + 1000l); // TODO WTF?
				pd->special = FIRE_TO_SMOKE | FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION
				              | DISSIPATING;
				pd->fparam = 0.0000001f;
			}
			
			//update mvt
			if(!arxtime.is_paused()) {
				a = (((float)m_currframetime) * 100.f) / (float)s.time;
				s.pos.y += s.yvel * a;
				s.ang += s.angvel * a;

				s.yvel *= 1.f - (1.f / 100.f);

				s.currtime += m_currframetime;
			}
		}
	}
}

void CLevitate::Update(float timeDelta) {
	
	if(!arxtime.is_paused()) {
		m_currdurationang += timeDelta;
		ulCurrentTime += timeDelta;
		m_currframetime = timeDelta;
		m_stoneDelay -= timeDelta;
	}
	
	//animation cone
	m_ang = (float)m_currdurationang / 1000.f;

	if(m_ang > 1.f) {
		m_currdurationang = 0;
		m_ang = 1.f;
	}
	
	int dustParticles = 0;
	
	switch(m_key) {
		case 0: {
			//monté du cone
			float a = (float) ulCurrentTime / 1000.f;

			if(a > 1.f) {
				a = 0.f;
				m_key++;
			}

			m_coneScale = a;
			
			dustParticles = 3;
			break;
		}
		case 1:
			//animation cone
			m_coneScale = (float)ulCurrentTime / (float)ulDuration;

			if(ulCurrentTime >= ulDuration) {
				m_coneScale = 1.f;
				m_key++;
			}
			
			dustParticles = 10;
			break;
	}
	
	for(int i = 0; i < dustParticles; i++) {
		createDustParticle();
	}
	
	if(m_stoneDelay <= 0) {
		m_stoneDelay = Random::get(50, 150);
		
		AddStone(m_pos + randomOffsetXZ(m_baseRadius));
	}
}

void CLevitate::createDustParticle()
{
	PARTICLE_DEF * pd = createParticle();
	if(!pd) {
		return;
	}
	
	float a = glm::radians(360.f * rnd());
	pd->ov = m_pos + Vec3f(m_baseRadius * std::cos(a), 0.f, m_baseRadius * std::sin(a));
	float t = fdist(pd->ov, m_pos);
	pd->move = Vec3f((5.f + 5.f * rnd()) * ((pd->ov.x - m_pos.x) / t), 3.f * rnd(),
	                 (5.f + 5.f * rnd()) * ((pd->ov.z - m_pos.z) / t));
	pd->siz = 30.f + 30.f * rnd();
	pd->tolive = 3000;
	pd->timcreation = -(long(arxtime) + 3000l); // TODO WTF
	pd->special = FIRE_TO_SMOKE | FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
	pd->fparam = 0.0000001f;
}

void CLevitate::Render()
{
	if(m_key > 1)
		return;
	
	//calcul du cone
	TexturedVertex *d3dv;
	Vec3f	* vertex;
	int			nb, nbc, col;
	float		ddu = m_ang;
	float		u = ddu, du = .99999999f / (float)m_def;

	switch(m_key) {
		case 0:
			nbc = 2;

			while(nbc--) {
				vertex = m_cone[nbc].conevertex;
				d3dv = m_cone[nbc].coned3d;
				nb = (m_cone[nbc].conenbvertex) >> 1;

				while(nb) {
					Vec3f d3dvs;
					d3dvs.x = m_pos.x + (vertex + 1)->x + ((vertex->x - (vertex + 1)->x) * m_coneScale);
					d3dvs.y = m_pos.y + (vertex + 1)->y + ((vertex->y - (vertex + 1)->y) * m_coneScale);
					d3dvs.z = m_pos.z + (vertex + 1)->z + ((vertex->z - (vertex + 1)->z) * m_coneScale);
					
					d3dv->p = EE_RT(d3dvs);
					col = Random::get(0, 80);

					if(!arxtime.is_paused())
						d3dv->color = Color::grayb(col).toRGB(col);

					d3dv->uv.x = u;
					d3dv->uv.y = 0.f;
					vertex++;
					d3dv++;

					d3dvs.x = m_pos.x + vertex->x;
					d3dvs.y = m_pos.y;
					d3dvs.z = m_pos.z + vertex->z;
					
					d3dv->p = EE_RT(d3dvs);
					col = Random::get(0, 80);

					if(!arxtime.is_paused())
						d3dv->color = Color::black.toRGB(col);

					d3dv->uv.x = u;
					d3dv->uv.y = 1.f;
					vertex++;
					d3dv++;

					u += du;
					nb--;
				}

				u = ddu;
				du = -du;
			}
			break;
		case 1:
			nbc = 2;

			while(nbc--) {
				vertex = m_cone[nbc].conevertex;
				d3dv = m_cone[nbc].coned3d;
				nb = (m_cone[nbc].conenbvertex) >> 1;

				while(nb) {
					Vec3f d3dvs = m_pos + *vertex;
	
					d3dv->p = EE_RT(d3dvs);
					col = Random::get(0, 80);

					if(!arxtime.is_paused())
						d3dv->color = Color::grayb(col).toRGB(col);

					d3dv->uv.x = u;
					d3dv->uv.y = 0.f;
					vertex++;
					d3dv++;

					d3dvs.x = m_pos.x + vertex->x;
					d3dvs.y = m_pos.y;
					d3dvs.z = m_pos.z + vertex->z;

					d3dv->p = EE_RT(d3dvs);
					col = Random::get(0, 80);

					if(!arxtime.is_paused())
						d3dv->color = Color::black.toRGB(col);

					d3dv->uv.x = u;
					d3dv->uv.y = 1.f;
					vertex++;
					d3dv++;

					u += du;
					nb--;
				}

				u = ddu;
				du = -du;
			}
			break;
	}

	//tracé du cone back
	
	RenderMaterial mat;
	mat.setDepthTest(true);
	mat.setBlendType(RenderMaterial::Additive);
	mat.setWrapMode(TextureStage::WrapMirror);
	mat.setTexture(m_tsouffle);
	mat.setCulling(Renderer::CullCW);
	
	int i = m_cone[1].conenbfaces - 2;
	int j = 0;

	while(i--) {
		drawTriangle(mat, &m_cone[1].coned3d[j]);
		j++;
	}

	i = m_cone[0].conenbfaces - 2;
	j = 0;

	while(i--) {
		drawTriangle(mat, &m_cone[0].coned3d[j]);
		j++;
	}

	//tracé du cone front
	mat.setCulling(Renderer::CullCCW);
	
	i = m_cone[1].conenbfaces - 2;
	j = 0;

	while(i--) {
		drawTriangle(mat, &m_cone[1].coned3d[j]);
		j++;
	}

	i = m_cone[0].conenbfaces - 2;
	j = 0;

	while(i--) {
		drawTriangle(mat, &m_cone[0].coned3d[j]);
		j++;
	}
	
	DrawStone();
}
