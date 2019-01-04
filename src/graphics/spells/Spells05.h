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

#ifndef ARX_GRAPHICS_SPELLS_SPELLS05_H
#define ARX_GRAPHICS_SPELLS_SPELLS05_H

#include "graphics/effects/FloatingStones.h"
#include "graphics/effects/RotatingCone.h"
#include "graphics/effects/SpellEffects.h"
#include "graphics/particle/ParticleSystem.h"
#include "platform/Platform.h"


// Done By : Didier Pédreno
class CPoisonProjectile arx_final : public CSpellFx {
	
public:
	
	CPoisonProjectile();
	
	void Create(Vec3f _eSrc, float _fBeta = 0);
	void Update(GameDuration timeDelta);
	void Render();
	
	Vec3f eSrc;
	Vec3f eCurPos;
	float lightIntensityFactor;
	
	LightHandle lLightId;
	
private:
	
	float fBetaRadCos;
	float fBetaRadSin;
	
	bool  bOk;
	float fTrail;
	
	Vec3f eMove;
	Vec3f pathways[40];
	ParticleSystem pPS;
	ParticleSystem pPSStream;
	
};

#endif // ARX_GRAPHICS_SPELLS_SPELLS05_H
