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

#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/Player.h"
#include "core/GameTime.h"
#include "graphics/RenderBatcher.h"
#include "core/Core.h"
#include "scene/GameSound.h"
#include "graphics/effects/Cabal.h"
#include "io/log/Logger.h"

CabalFx::CabalFx()
	: m_yaw(0)
	, m_scaleY(90)
	, m_offset(60)
	, m_hasTwoRingSets(true)
	, m_ringCount(0)
{
}

void CabalFx::setLowerColorRndRange(Color3f range) {
	m_colorRangeLower = range;
}

void CabalFx::setUpperColorRndRange(Color3f range) {
	m_colorRangeUpper = range;
}

void CabalFx::setStartLightColor(Color3f color) {
	m_startLightColor = color;
}

void CabalFx::setYScale(float scale) {
	m_scaleY = scale;
}

void CabalFx::setOffset(float offset) {
	m_offset = offset;
}

void CabalFx::addRing(Color3f color) {
	if(m_ringCount < 4) {
		m_ringColors[m_ringCount++] = color;
	}
}

void CabalFx::disableSecondRingSet() {
	m_hasTwoRingSets = false;
}

Color3f CabalFx::randomizeLightColor() {
	
	Color3f color;
	
	if(m_colorRangeLower.r < m_colorRangeUpper.r) {
		color.r = Random::getf(m_colorRangeLower.r, m_colorRangeUpper.r);
	} else {
		color.r = m_colorRangeLower.r;
	}
	if(m_colorRangeLower.b < m_colorRangeUpper.b) {
		color.b = Random::getf(m_colorRangeLower.b, m_colorRangeUpper.b);
	} else {
		color.b = m_colorRangeLower.b;
	}
	if(m_colorRangeLower.g < m_colorRangeUpper.g) {
		color.g = Random::getf(m_colorRangeLower.g, m_colorRangeUpper.g);
	} else {
		color.g = m_colorRangeLower.g;
	}
	return color;
}

Vec3f CabalFx::update(Vec3f casterPos) {
	
	const float frametime = toMsf(g_gameTime.now());
	float mov = std::sin(frametime * (1.0f / 800)) * m_scaleY;
	
	Vec3f cabalpos = casterPos + Vec3f(0.f, m_offset - mov, 0.f);
	float refpos = casterPos.y + m_offset;
	
	float Es = std::sin(frametime * (1.0f / 800) + glm::radians(m_scaleY));
	
	EERIE_LIGHT * light = lightHandleGet(m_lightHandle);
	if (light) {
		light->pos.x = cabalpos.x;
		light->pos.y = refpos;
		light->pos.z = cabalpos.z;
		light->rgb = randomizeLightColor();
		light->fallstart = Es * 1.5f;
	}

	RenderMaterial mat;
	mat.setCulling(CullNone);
	mat.setDepthTest(true);
	mat.setBlendType(RenderMaterial::Additive);

	Anglef cabalangle(0.f, 0.f, 0.f);
	cabalangle.setYaw(m_yaw + g_framedelay * 0.1f);
	m_yaw = cabalangle.getYaw();

	Vec3f cabalscale = Vec3f(Es);
	Color3f cabalcolor;

	float timeOffset = 30.0;

	for(size_t i = 0; i < m_ringCount; i++) {
		if(i > 0) {
			mov = std::sin((frametime - timeOffset) * (1.0f / 800)) * m_scaleY;
			cabalpos.y = refpos - mov;
			timeOffset *= 2.0f;
		}
		cabalcolor = m_ringColors[i];
		Draw3DObject(cabal, cabalangle, cabalpos, cabalscale, cabalcolor, mat);
	}
	
	if(m_hasTwoRingSets) {
		
		cabalangle.setYaw(-cabalangle.getYaw());
		timeOffset = 30.0f;

		for(size_t i = 0; i < m_ringCount; i++) {
			if(i > 0) {
				mov = std::sin((frametime + timeOffset) * (1.0f / 800)) * m_scaleY;
				cabalpos.y = refpos + mov;
				timeOffset *= 2.0f;
			}
			cabalcolor = m_ringColors[m_ringCount - 1 - i];
			Draw3DObject(cabal, cabalangle, cabalpos, cabalscale, cabalcolor, mat);
		}
	}
	return cabalpos;
}

void CabalFx::create(Vec3f casterPos) {
	EERIE_LIGHT * light = dynLightCreate(m_lightHandle);
	if(light) {
		light->intensity = 2.3f;
		light->fallend = 700.f;
		light->fallstart = 500.f;
		light->rgb = m_startLightColor;
		light->pos = casterPos;
		light->duration = GameDurationMs(900);
	}
}

void CabalFx::end() {
	endLightDelayed(m_lightHandle, GameDurationMs(600));
}
