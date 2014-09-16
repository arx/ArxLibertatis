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

#include "graphics/spells/Spells02.h"

#include <climits>

#include "core/Core.h"
#include "core/GameTime.h"

#include "game/EntityManager.h"
#include "game/Player.h"
#include "game/Spells.h"

#include "graphics/particle/Particle.h"
#include "graphics/particle/ParticleParams.h"
#include "graphics/particle/ParticleSystem.h"

#include "scene/Light.h"
#include "scene/Interactive.h"

CHeal::CHeal() {
	SetDuration(1000);
	ulCurrentTime = ulDuration + 1;

	pPS = new ParticleSystem();
}

CHeal::~CHeal() {
	delete pPS;
	pPS = NULL;
}

void CHeal::Create() {
	
	pPS->m_lightHandle = GetFreeDynLight();

	if(lightHandleIsValid(pPS->m_lightHandle)) {
		EERIE_LIGHT * light = lightHandleGet(pPS->m_lightHandle);
		
		light->intensity = 2.3f;
		light->fallstart = 200.f;
		light->fallend   = 350.f;
		light->rgb = Color3f(0.4f, 0.4f, 1.0f);
		light->pos = eSrc + Vec3f(0.f, -50.f, 0.f);
		light->duration = 200;
		light->extras = 0;
	}

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
	cp.m_startSegment.m_color = Color(205, 205, 255, 245).to<float>();
	cp.m_startSegment.m_colorRandom = Color(50, 50, 0, 10).to<float>();

	cp.m_endSegment.m_size = 6;
	cp.m_endSegment.m_sizeRandom = 4;
	cp.m_endSegment.m_color = Color(20, 20, 30, 0).to<float>();
	cp.m_endSegment.m_colorRandom = Color(0, 0, 40, 0).to<float>();
	
	cp.m_blendMode = RenderMaterial::Additive;
	cp.m_texture.set("graph/particles/heal_0005", 0, 100);
	cp.m_spawnFlags = PARTICLE_CIRCULAR | PARTICLE_BORDER;
	
	pPS->SetParams(cp);
}

void CHeal::setPos(const Vec3f & pos)
{
	eSrc = pos;
}

void CHeal::Update(float timeDelta)
{
	ulCurrentTime += timeDelta;

	if(ulCurrentTime >= ulDuration)
		return;
	
	if(!lightHandleIsValid(pPS->m_lightHandle))
		pPS->m_lightHandle = GetFreeDynLight();

	if(lightHandleIsValid(pPS->m_lightHandle)) {
		EERIE_LIGHT * light = lightHandleGet(pPS->m_lightHandle);
		
		light->intensity = 2.3f;
		light->fallstart = 200.f;
		light->fallend   = 350.f;
		light->rgb = Color3f(0.4f, 0.4f, 1.0f);
		light->pos = eSrc + Vec3f(0.f, -50.f, 0.f);
		light->duration = 200;
		light->extras = 0;
	}

	unsigned long ulCalc = ulDuration - ulCurrentTime ;
	arx_assert(ulCalc <= LONG_MAX);
	long ff = static_cast<long>(ulCalc);

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

	pPS->SetPos(eSrc);
	pPS->Update(timeDelta);
}

void CHeal::Render() {
	if(ulCurrentTime >= ulDuration)
		return;

	pPS->Render();
}

