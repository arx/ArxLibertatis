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
	
	SetAngle(MAKEANGLE(player.angle.getPitch()));
	
	if(spells[spellinstance].caster == 0) {
		eSrc = player.pos;
	} else {
		eSrc = entities[spells[spellinstance].caster]->pos;
	}
	
	pPS->lLightId = GetFreeDynLight();

	if(pPS->lLightId != -1) {
		long id = pPS->lLightId;
		DynLight[id].exist = 1;
		DynLight[id].intensity = 2.3f;
		DynLight[id].fallstart = 200.f;
		DynLight[id].fallend   = 350.f;
		DynLight[id].rgb.r = 0.4f;
		DynLight[id].rgb.g = 0.4f;
		DynLight[id].rgb.b = 1.0f;
		DynLight[id].pos.x = eSrc.x;
		DynLight[id].pos.y = eSrc.y - 50.f;
		DynLight[id].pos.z = eSrc.z;
		DynLight[id].duration = 200;
		DynLight[id].extras = 0;
	}

	pPS->SetPos(eSrc);
	ParticleParams cp;
	memset(&cp, 0, sizeof(cp));
	cp.iNbMax = 350;
	cp.fLife = 800;
	cp.fLifeRandom = 2000;
	cp.p3Pos.x = 100;
	cp.p3Pos.y = 200;
	cp.p3Pos.z = 100;
	cp.p3Direction.x = 0;
	cp.p3Direction.y = -10;
	cp.p3Direction.z = 0;
	cp.fAngle = radians(5);
	cp.fSpeed = 120; 
	cp.fSpeedRandom = 84; 
	cp.p3Gravity.x = 0;
	cp.p3Gravity.y = -10;
	cp.p3Gravity.z = 0;
	cp.fFlash = 0;
	cp.fRotation = 80;

	cp.fStartSize = 8;
	cp.fStartSizeRandom = 8;
	cp.fStartColor[0] = 205;
	cp.fStartColor[1] = 205;
	cp.fStartColor[2] = 255;
	cp.fStartColor[3] = 245;
	cp.fStartColorRandom[0] = 50;
	cp.fStartColorRandom[1] = 50;
	cp.fStartColorRandom[2] = 0;
	cp.fStartColorRandom[3] = 10;

	cp.fEndSize = 6;
	cp.fEndSizeRandom = 4;
	cp.fEndColor[0] = 20;
	cp.fEndColor[1] = 20;
	cp.fEndColor[2] = 30;
	cp.fEndColor[3] = 0;
	cp.fEndColorRandom[0] = 0;
	cp.fEndColorRandom[1] = 0;
	cp.fEndColorRandom[2] = 40;
	cp.fEndColorRandom[3] = 0;
	
	cp.iBlendMode = 0;

	pPS->SetParams(cp);
	pPS->ulParticleSpawn = PARTICLE_CIRCULAR | PARTICLE_BORDER;
	pPS->SetTexture("graph/particles/heal_0005", 0, 100); 

	fSize = 1;
}

void CHeal::Update(unsigned long aulTime)
{
	ulCurrentTime += aulTime;

	if(ulCurrentTime >= ulDuration)
		return;
	
	if(spells[spellinstance].caster == 0) {
		eSrc = player.pos;
	} else if(ValidIONum(spells[spellinstance].target)) {
		eSrc = entities[spells[spellinstance].target]->pos;
	}
	
	if(pPS->lLightId == -1)
		pPS->lLightId = GetFreeDynLight();

	if(pPS->lLightId != -1) {
		long id = pPS->lLightId;
		DynLight[id].exist = 1;
		DynLight[id].intensity = 2.3f;
		DynLight[id].fallstart = 200.f;
		DynLight[id].fallend   = 350.f;
		DynLight[id].rgb.r = 0.4f;
		DynLight[id].rgb.g = 0.4f;
		DynLight[id].rgb.b = 1.0f;
		DynLight[id].pos.x = eSrc.x;
		DynLight[id].pos.y = eSrc.y - 50.f;
		DynLight[id].pos.z = eSrc.z;
		DynLight[id].duration = 200;
		DynLight[id].extras = 0;
	}

	unsigned long ulCalc = ulDuration - ulCurrentTime ;
	arx_assert(ulCalc <= LONG_MAX);
	long ff = static_cast<long>(ulCalc);

	if(ff < 1500) {
		pPS->uMaxParticles = 0;
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

	pPS->SetPos(eSrc);
	pPS->Update(aulTime);
}

void CHeal::Render() {
	if(ulCurrentTime >= ulDuration)
		return;

	pPS->Render();
}

//-----------------------------------------------------------------------------
// ARMOR
//-----------------------------------------------------------------------------
CArmor::CArmor()
{
}

CArmor::~CArmor()
{
}

void CArmor::Create(long _ulDuration) {
	
	SetDuration(_ulDuration);

	long iNpc = spells[spellinstance].target;

	if(ValidIONum(iNpc)) {
		Entity *io = entities[iNpc];
		io->halo.flags = HALO_ACTIVE;
		io->halo.color.r = 0.5f;
		io->halo.color.g = 0.5f;
		io->halo.color.b = 0.25f;
		io->halo.radius = 45.f;
		io->halo.dynlight = -1;
	}
}

void CArmor::Update(unsigned long _ulTime)
{
	if(!arxtime.is_paused())
		ulCurrentTime += _ulTime;
	
	long iNpc = spells[spellinstance].target;

	if(ValidIONum(iNpc)) {
		Entity *io = entities[iNpc];
		io->halo.flags = HALO_ACTIVE;
		io->halo.color.r = 0.5f;
		io->halo.color.g = 0.5f;
		io->halo.color.b = 0.25f;
		io->halo.radius = 45.f;
		io->halo.dynlight = -1;
	}
}

void CArmor::Render() {
	
}

//-----------------------------------------------------------------------------
// LOWER ARMOR
//-----------------------------------------------------------------------------
CLowerArmor::CLowerArmor()
{
}

CLowerArmor::~CLowerArmor()
{
}

void CLowerArmor::Create(long _ulDuration) {
	
	SetDuration(_ulDuration);

	if(spellinstance != -1) {
		Entity * io = entities[spells[spellinstance].target];

		if(io && !(io->halo.flags & HALO_ACTIVE)) {
			io->halo.flags |= HALO_ACTIVE;
			io->halo.color.r = 1.f;
			io->halo.color.g = 0.05f;
			io->halo.color.b = 0.0f;
			io->halo.radius = 45.f;
			io->halo.dynlight = -1;
			spells[spellinstance].longinfo = 1;
		}
		else
			spells[spellinstance].longinfo = 0;
	}
}

void CLowerArmor::Update(unsigned long _ulTime)
{
	if(!arxtime.is_paused())
		ulCurrentTime += _ulTime;
	
	if(spellinstance != -1) {
		Entity * io = entities[spells[spellinstance].target];

		if(io && !(io->halo.flags & HALO_ACTIVE)) {
			io->halo.flags |= HALO_ACTIVE;
			io->halo.color.r = 1.f;
			io->halo.color.g = 0.05f;
			io->halo.color.b = 0.0f;
			io->halo.radius = 45.f;
			io->halo.dynlight = -1;
			spells[spellinstance].longinfo = 1;
		}
	}
}

void CLowerArmor::Render() {

}
