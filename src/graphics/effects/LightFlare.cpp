/*
 * Copyright 2016-2019 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/effects/LightFlare.h"

#include "core/Core.h"
#include "core/GameTime.h"
#include "game/Player.h"
#include "graphics/Draw.h"
#include "graphics/GlobalFog.h"
#include "graphics/Raycast.h"
#include "graphics/Renderer.h"
#include "graphics/effects/Fade.h"
#include "gui/CinematicBorder.h"
#include "platform/profiler/Profiler.h"
#include "scene/Interactive.h"
#include "scene/Light.h"

static TextureContainer * tflare = NULL;

void initLightFlares() {
	tflare = TextureContainer::LoadUI("graph/particles/flare");
}

void updateLightFlares() {
	
	RaycastDebugClear();
	
	ARX_PROFILE_FUNC();
	
	float temp_increase = toMs(g_platformTime.lastFrameDuration()) * 0.004f;
	
	Vec4f zFar = g_preparedCamera.m_viewToScreen * Vec4f(0.f, 0.f, g_camera->cdepth * fZFogEnd, 1.f);
	float fZFar = zFar.z / zFar.w;

	for(size_t i = 0; i < g_culledDynamicLightsCount; i++) {
		EERIE_LIGHT * el = g_culledDynamicLights[i];
		
		if(!ACTIVEBKG->isInActiveTile(el->pos)) {
			el->m_isVisible = false;
			continue;
		}
		
		if(!(el->extras & EXTRAS_FLARE)) {
			continue;
		}
		
		Vec4f p = worldToClipSpace(el->pos);
		Vec3f pos2d = Vec3f(p) / p.w;
		
		el->m_flareFader -= temp_increase;
		
		if(p.w > 0.f && pos2d.x > 0.f && pos2d.x < g_size.width()
		   && pos2d.y > (cinematicBorder.CINEMA_DECAL * g_sizeRatio.y)
		   && pos2d.y < (g_size.height() - (cinematicBorder.CINEMA_DECAL * g_sizeRatio.y))) {
			
			Vec3f lightpos = el->pos + glm::normalize(g_camera->m_pos - el->pos) * 20.f;
			PolyType ignored = POLY_HIDE | POLY_TRANS | POLY_NODRAW;
			RaycastFlags flags = RaycastAnyHit;
			if(pos2d.z > fZFar
			   || raycastScene(g_camera->m_pos, lightpos, ignored, flags)
			   || raycastEntities(g_camera->m_pos, lightpos, ignored, flags)) {
				el->m_flareFader -= temp_increase * 2.f;
			} else {
				el->m_flareFader += temp_increase * 2.f;
			}
		}
		
		el->m_flareFader = glm::clamp(el->m_flareFader, 0.f, .8f);
		
	}
	
}

void renderLightFlares() {
	
	ARX_PROFILE_FUNC();
	
	if(g_debugToggles[6]) {
		RaycastDebugDraw();
	}
	
	GRenderer->SetFogColor(Color::none);
	UseRenderState state(render3D().blend(BlendOne, BlendOne).depthWrite(false).depthTest(false));
	
	for(size_t i = 0; i < g_culledDynamicLightsCount; i++) {
		const EERIE_LIGHT & el = *g_culledDynamicLights[i];
		
		if(!el.m_exists || !el.m_isVisible || !(el.extras & EXTRAS_FLARE) || el.m_flareFader <= 0.f) {
			continue;
		}
		
		float v = el.m_flareFader;
		
		if(FADEDIR) {
			v *= 1.f - LAST_FADEVALUE;
		}
		
		float size = -el.ex_flaresize;
		if(el.extras & EXTRAS_FIXFLARESIZE) {
			// This is only used for one light in the whole game and makes it's flare gigantic when up close
			// TODO Is this part of some puzze or a bug / obsolete workaround?
			size = el.ex_flaresize;
		}
		
		EERIEDrawSprite(el.pos, size, tflare, Color(el.rgb * v), EE_RT(el.pos).z);
		
	}
	
}
