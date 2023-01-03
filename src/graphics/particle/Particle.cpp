/*
 * Copyright 2011-2022 Arx Libertatis Team (see the AUTHORS file)
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
	, fSizeStart(1.f)
	, p3Velocity(arx::randomVec(-10.f, 10.f))
	, fSizeEnd(1.f)
	, fColorStart(1.f, 1.f, 1.f, 0.5f)
	, fColorEnd(1.f, 1.f, 1.f, 0.1f)
	, m_age(0)
	, m_timeToLive(Random::get(2000ms, 5000ms))
	, iRot(1)
	, fRotStart(0.f)
{ }

void Particle::Regen() {
	p3Pos = Vec3f(0.f);
	m_age = 0;
}

void Particle::Update(GameDuration delta) {
	
	m_age += delta;
	
	if(m_age < m_timeToLive) {
		
		// update new pos
		p3Pos += p3Velocity * (delta / 1s);
		
	}
	
}
