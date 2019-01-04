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

#ifndef ARX_GRAPHICS_EFFECTS_LIGHTNING_H
#define ARX_GRAPHICS_EFFECTS_LIGHTNING_H

#include "game/Entity.h"
#include "graphics/Vertex.h"
#include "graphics/data/Mesh.h"
#include "graphics/effects/SpellEffects.h"
#include "graphics/particle/ParticleSystem.h"
#include "math/Types.h"
#include "math/Vector.h"
#include "platform/Platform.h"

// Done By : Didier Pedreno
class CLightning arx_final : public CSpellFx {
	
public:
	
	CLightning();
	
	void Create(Vec3f aeFrom, Vec3f aeTo);
	void Update(GameDuration timeDelta);
	void Render();
	
	Vec3f m_pos;
	float m_beta;
	float m_alpha;
	EntityHandle m_caster;
	float m_level;
	
	float m_fDamage;
	bool m_isMassLightning;
	
private:
	
	float fTotoro;
	float fMySize;
	size_t m_nbtotal;
	long m_lNbSegments;
	float m_invNbSegments;
	float m_fLengthMin;
	float m_fLengthMax;
	Vec3f m_fAngleMin;
	Vec3f m_fAngleMax;
	Vec3f m_eSrc;
	Vec3f m_eDest;
	GameDuration m_iTTL;
	
	struct CLightningNode {
		Vec3f pos;
		float size;
		int parent;
		Vec3f f;
	};
	
	static const size_t MAX_NODES = 2000;
	
	CLightningNode m_cnodetab[MAX_NODES];
	
	struct LIGHTNING;
	void BuildS(LIGHTNING * lightingInfo);
	void ReCreate(float rootSize);
	
};

#endif // ARX_GRAPHICS_EFFECTS_LIGHTNING_H
