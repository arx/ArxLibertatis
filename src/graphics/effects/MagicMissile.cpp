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

#include "graphics/effects/MagicMissile.h"

#include "animation/AnimationRender.h"

#include "core/GameTime.h"

#include "game/Damage.h"
#include "game/EntityManager.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/Spells.h"

#include "graphics/Math.h"
#include "graphics/effects/SpellEffects.h"
#include "graphics/particle/ParticleParams.h"
#include "graphics/particle/ParticleManager.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/spells/Spells05.h"
#include "graphics/data/TextureContainer.h"

#include "physics/Collisions.h"

#include "scene/Light.h"
#include "scene/GameSound.h"
#include "scene/Object.h"
#include "scene/Interactive.h"

CMagicMissile::CMagicMissile()
	: bExplo(false)
	, bMove(true)
	, eCurPos(0.f)
	, m_trailColor(Color3f(0.9f, 0.9f, 0.7f) + Color3f(0.1f, 0.1f, 0.3f) * randomColor3f())
	, m_projectileColor(0.3f, 0.3f, 0.5f)
	, tex_mm(TextureContainer::Load("graph/obj3d/textures/(fx)_bandelette_blue"))
	, iLength(0)
	, iBezierPrecision(0)
	, fTrail(0.f)
{
	SetDuration(GameDurationMs(2000));
	m_elapsed = m_duration + GameDurationMs(1);
}

CMagicMissile::~CMagicMissile() { }

void CMagicMissile::Create(const Vec3f & startPos, const Anglef & angles)
{
	SetDuration(m_duration);
	
	eCurPos = startPos;
	
	size_t i = 40;
	Vec3f endPos = startPos;
	endPos += angleToVectorXZ(angles.getYaw()) * (50.f * float(i));
	endPos.y += std::sin(glm::radians(MAKEANGLE(angles.getPitch()))) * (50.f * float(i));
	
	pathways[0] = startPos;
	pathways[5] = endPos;
	Split(pathways, 0, 5, 50, 0.5f);

	for(i = 0; i < 6; i++) {
		if(pathways[i].y >= startPos.y + 150) {
			pathways[i].y = startPos.y + 150;
		}
	}

	fTrail = 0;

	iLength = 50;
	iBezierPrecision = BEZIERPrecision;
	bExplo = false;
	bMove = true;
}

void CMagicMissile::SetTTL(GameDuration aulTTL)
{
	GameDuration t = m_elapsed;
	m_duration = std::min(m_elapsed + aulTTL, m_duration);
	SetDuration(m_duration);
	m_elapsed = t;
}

void CMagicMissile::Update(GameDuration timeDelta)
{
	m_elapsed += timeDelta;
}

void CMagicMissile::Render() {
	
	if(m_elapsed >= m_duration) {
		return;
	}
	
	RenderMaterial mat;
	mat.setCulling(CullNone);
	mat.setDepthTest(true);
	mat.setBlendType(RenderMaterial::Additive);
	
	if(tex_mm)
		mat.setTexture(tex_mm);
	
	if(bMove) {
		fTrail = (m_elapsed / m_duration) * (iBezierPrecision + 2) * 5;
	}
	
	Vec3f lastpos = pathways[0];
	Vec3f newpos = pathways[0];
	
	for(int i = 0; i < 5; i++) {
		
		const Vec3f v1 = pathways[std::max(0, i - 1)];
		const Vec3f v2 = pathways[i];
		const Vec3f v3 = pathways[i + 1];
		const Vec3f v4 = pathways[std::min(5, i + 2)];
		
		for(int toto = 1; toto < iBezierPrecision; toto++) {
			if(fTrail < i * iBezierPrecision + toto)
				break;

			float t = toto * (1.0f / iBezierPrecision);
			
			newpos = arx::catmullRom(v1, v2, v3, v4, t);

			if(!((fTrail - (i * iBezierPrecision + toto)) > iLength)) {
				
				float fsize = 1.f - (fTrail - i * iBezierPrecision + toto) / std::min(fTrail, float(iLength));
				float alpha = std::max(fsize - 0.2f, 0.2f);
				Color color(m_trailColor * (glm::clamp(fsize + Random::getf(-0.1f, 0.1f), 0.f, 1.f) * alpha));
				
				if(fsize < 0.5f) {
					fsize = fsize * 6.f;
				} else {
					fsize = (1.f - fsize + 0.5f) * 3.f;
				}
				
				float fs = fsize * 6.f + Random::getf(0.f, 0.3f);
				float fe = fsize * 6.f + Random::getf(0.f, 0.3f);
				Draw3DLineTexNew(mat, lastpos, newpos, color, color, fs, fe);
			}

			Vec3f temp_vector = lastpos;
			lastpos = newpos;
			newpos = temp_vector;
		}
	}
	
	Vec3f av = newpos - lastpos;
	
	float bubu = getAngle(av.x, av.z, 0, 0);
	float bubu1 = getAngle(av.x, av.y, 0, 0);
	
	eCurPos = lastpos;
	
	Anglef stiteangle;
	stiteangle.setYaw(-glm::degrees(bubu));
	stiteangle.setPitch(0);
	stiteangle.setRoll(-(glm::degrees(bubu1)));

	if(av.x < 0)
		stiteangle.setRoll(stiteangle.getRoll() - 90);

	if(av.x > 0)
		stiteangle.setRoll(stiteangle.getRoll() + 90);

	if(stiteangle.getRoll() < 0)
		stiteangle.setRoll(stiteangle.getRoll() + 360.0f);

	Draw3DObject(smissile, stiteangle, eCurPos, Vec3f(1.f), m_projectileColor, mat);
}


MrMagicMissileFx::MrMagicMissileFx() {
	m_trailColor = Color3f(0.9f, 0.2f, 0.5f);
	m_projectileColor = Color3f(1.f, 0.f, 0.2f);
	tex_mm = NULL;
}
