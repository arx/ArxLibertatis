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

#include "graphics/particle/Particle.h"

#include "graphics/Math.h"
#include "graphics/effects/SpellEffects.h"

Particle::Particle()
	: p3Pos(frand2() * 5, frand2() * 5, frand2() * 5),
	  p3Velocity(frand2() * 10, frand2() * 10, frand2() * 10),
	  ulTime(0), fSize(1.f), fSizeStart(1.f), fSizeEnd(1.f),
	  iTexTime(0), iTexNum(0) {
	
	ulTTL = checked_range_cast<long>(2000 + rnd() * 3000);
	fOneOnTTL = 1.0f / float(ulTTL);
	
	fColorStart[0] = 1;
	fColorStart[1] = 1;
	fColorStart[2] = 1;
	fColorStart[3] = 0.5f;
	fColorEnd[0] = 1;
	fColorEnd[1] = 1;
	fColorEnd[2] = 1;
	fColorEnd[3] = 0.1f;
}

Particle::~Particle() { }

void Particle::Regen() {
	p3Pos = Vec3f_ZERO;
	ulTime = 0;
	fSize = 1;
	iTexTime = 0;
	iTexNum = 0;
}

void Particle::Validate() {
	
	fSize = std::max(fSize, 1.f);
	fSizeStart = std::max(fSizeStart, 1.f);
	fSizeEnd = std::max(fSizeEnd, 1.f);
	
	for(int i = 0; i < 4; i++) {
		fColorStart[i] = clamp(fColorStart[i], 0.f, 1.f);
		fColorEnd[i] = clamp(fColorEnd[i], 0.f, 1.f);
	}
	
	if(ulTTL < 100) {
		ulTTL = 100;
		fOneOnTTL = 1.0f / float(ulTTL);
	}
}

void Particle::Update(long _lTime) {
	
	ulTime += _lTime;
	iTexTime += _lTime;
	float fTimeSec = _lTime * (1.f / 1000);
	
	if(ulTime < ulTTL) {
		
		float ft = fOneOnTTL * ulTime;
		
		// update new pos
		p3Pos += p3Velocity * fTimeSec;
		
		fSize = fSizeStart + (fSizeEnd - fSizeStart) * ft;
		
		Color4f fColor;
		fColor.r = fColorStart[0] + (fColorEnd[0] - fColorStart[0]) * ft;
		fColor.g = fColorStart[1] + (fColorEnd[1] - fColorStart[1]) * ft;
		fColor.b = fColorStart[2] + (fColorEnd[2] - fColorStart[2]) * ft;
		fColor.a = fColorStart[3] + (fColorEnd[3] - fColorStart[3]) * ft;
		ulColor = fColor.to<u8>();
	}
}
