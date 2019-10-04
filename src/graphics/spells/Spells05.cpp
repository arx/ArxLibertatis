/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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
#include "game/effect/ParticleSystems.h"

#include "graphics/Math.h"
#include "graphics/Raycast.h"
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


CPoisonProjectile::CPoisonProjectile()
	: eSrc(0.f)
	, eCurPos(0.f)
	, lightIntensityFactor(1.f)
	, fTrail(-1.f)
	, eMove(0.f)
{
	SetDuration(GameDurationMs(2000));
	m_elapsed = m_duration + GameDurationMs(1);
}

void CPoisonProjectile::Create(Vec3f _eSrc, float _fBeta)
{
	SetDuration(m_duration);
	
	float fBetaRad = glm::radians(_fBeta);
	float fBetaRadCos = glm::cos(fBetaRad);
	float fBetaRadSin = glm::sin(fBetaRad);

	eSrc = _eSrc;

	eMove = Vec3f(-fBetaRadSin * 2, 0.f, fBetaRadCos * 2);
	
	Vec3f rayEnd = eSrc + eMove * 500.f;
	
	RaycastResult ray = RaycastLine(eSrc, rayEnd);
	Vec3f dest = ray.hit ? ray.pos : rayEnd;
	
	pathways[0] = eSrc;
	pathways[9] = dest;
	
	Split(pathways, 0, 9, Vec3f(10 * fBetaRadCos, 10, 10 * fBetaRadSin));
	
	fTrail = -1;
}

void CPoisonProjectile::Update(GameDuration timeDelta)
{
	if(m_elapsed <= GameDurationMs(2000)) {
		m_elapsed += timeDelta;
	}

	// on passe de 5 à 100 partoches en 1.5secs
	if(m_elapsed >= GameDurationMs(750)) {
		fTrail = ((m_elapsed - GameDurationMs(750)) / (m_duration - GameDurationMs(750))) * 9 * (BEZIERPrecision + 2);
	}

	if(m_elapsed >= m_duration)
		lightIntensityFactor = 0.f;
	else
		lightIntensityFactor = 1.f;
}

void CPoisonProjectile::Render() {
	
	if(m_elapsed >= m_duration)
		return;
	
	int n = BEZIERPrecision;
	float delta = 1.0f / n;
	
	Vec3f lastpos = pathways[0];
	
	int i = 0;
	for(i = 0; i < 9; i++) {
		
		int kpprec = std::max(i - 1, 0);
		int kpsuiv = i + 1;
		int kpsuivsuiv = (i < (9 - 2)) ? kpsuiv + 1 : kpsuiv;
		
		for(int toto = 1; toto < n; toto++) {
			
			if(fTrail < i * n + toto) {
				break;
			}
			
			float t = toto * delta;
			
			const Vec3f prevPos = pathways[kpprec];
			const Vec3f currentPos = pathways[i];
			const Vec3f nextPos = pathways[kpsuiv];
			const Vec3f next2Pos = pathways[kpsuivsuiv];
			
			lastpos = arx::catmullRom(prevPos, currentPos, nextPos, next2Pos, t);
		}
	}
	
	eCurPos = lastpos;
}
