/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/particle/Particle.h"

#include "graphics/Math.h"
#include "graphics/effects/SpellEffects.h"
#include "math/RandomVector.h"

Particle::Particle()
	: p3Pos(arx::randomVec(-5.f, 5.f))
	, p3Velocity(arx::randomVec(-10.f, 10.f))
	, m_age(0)
	, fSize(1.f)
	, fSizeStart(1.f)
	, fSizeEnd(1.f)
	, iRot(1)
	, fRotStart(0.f)
	, iTexTime(0)
	, iTexNum(0)
{
	
	m_timeToLive = GameDurationMs(Random::get(2000, 5000));
	
	fColorStart = Color4f(1, 1, 1, 0.5f);
	fColorEnd = Color4f(1, 1, 1, 0.1f);
}

Particle::~Particle() { }

void Particle::Regen() {
	p3Pos = Vec3f(0.f);
	m_age = 0;
	fSize = 1;
	iTexTime = 0;
	iTexNum = 0;
}

void Particle::Validate() {
	
	fSize = std::max(fSize, 1.f);
	fSizeStart = std::max(fSizeStart, 1.f);
	fSizeEnd = std::max(fSizeEnd, 1.f);
	
	fColorStart.r = glm::clamp(fColorStart.r, 0.f, 1.f);
	fColorStart.g = glm::clamp(fColorStart.g, 0.f, 1.f);
	fColorStart.b = glm::clamp(fColorStart.b, 0.f, 1.f);
	fColorStart.a = glm::clamp(fColorStart.a, 0.f, 1.f);
	
	fColorEnd.r = glm::clamp(fColorEnd.r, 0.f, 1.f);
	fColorEnd.g = glm::clamp(fColorEnd.g, 0.f, 1.f);
	fColorEnd.b = glm::clamp(fColorEnd.b, 0.f, 1.f);
	fColorEnd.a = glm::clamp(fColorEnd.a, 0.f, 1.f);
	
	if(m_timeToLive < GameDurationMs(100)) {
		m_timeToLive = GameDurationMs(100);
	}
}

void Particle::Update(GameDuration delta) {
	
	m_age += delta;
	iTexTime += toMsi(delta); // FIXME time, this will break with sub ms deltas
	float fTimeSec = delta / GameDurationMs(1000);
	
	if(m_age < m_timeToLive) {
		
		float ft = m_age / m_timeToLive;
		
		// update new pos
		p3Pos += p3Velocity * fTimeSec;
		
		fSize = fSizeStart + (fSizeEnd - fSizeStart) * ft;
		
		ulColor = Color(fColorStart + (fColorEnd - fColorStart) * ft);
	}
}
