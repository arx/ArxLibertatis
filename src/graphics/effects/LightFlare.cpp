/*
 * Copyright 2016-2017 Arx Libertatis Team (see the AUTHORS file)
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

void update2DFX() {
	
	RaycastDebugClear();
	
	ARX_PROFILE_FUNC();
	
	Entity* pTableIO[256];
	size_t nNbInTableIO = 0;

	float temp_increase = toMs(g_platformTime.lastFrameDuration()) * (1.0f/1000) * 4.f;
	
	const Vec3f camPos = ACTIVECAM->orgTrans.pos;
	
	bool bComputeIO = false;

	for(size_t i = 0; i < g_culledDynamicLightsCount; i++) {
		EERIE_LIGHT *el = g_culledDynamicLights[i];

		BackgroundTileData * bkgData = getFastBackgroundData(el->pos.x, el->pos.z);

		if(!bkgData || !bkgData->treat) {
			el->treat=0;
			continue;
		}

		if(el->extras & EXTRAS_FLARE) {
			Vec3f lv = el->pos;
			
			Vec4f p = worldToClipSpace(lv);
			Vec3f pos2d = Vec3f(p) / p.w;
			
			el->m_flareFader -= temp_increase;

			if(!(player.Interface & INTER_COMBATMODE) && (player.Interface & INTER_PLAYERBOOK))
				continue;

			if(p.w > 0.f &&
				pos2d.x > 0.f &&
				pos2d.y > (cinematicBorder.CINEMA_DECAL * g_sizeRatio.y) &&
				pos2d.x < g_size.width() &&
				pos2d.y < (g_size.height()-(cinematicBorder.CINEMA_DECAL * g_sizeRatio.y))
				)
			{
				Vec3f vector = lv - camPos;
				lv -= vector * (50.f / glm::length(vector));

				float fZFar=ACTIVECAM->ProjectionMatrix[3][2]*(1.f/(ACTIVECAM->cdepth*fZFogEnd))+ACTIVECAM->ProjectionMatrix[2][2];

				Vec2s ees2dlv;
				Vec3f ee3dlv = lv;

				ees2dlv.x = checked_range_cast<short>(pos2d.x);
				ees2dlv.y = checked_range_cast<short>(pos2d.y);

				if(!bComputeIO) {
					GetFirstInterAtPos(ees2dlv, 2, &ee3dlv, pTableIO, &nNbInTableIO);
					bComputeIO = true;
				}

				
				if(   pos2d.z > fZFar
				   || RaycastLightFlare(camPos, el->pos)
				   || GetFirstInterAtPos(ees2dlv, 3, &ee3dlv, pTableIO, &nNbInTableIO)
				) {
					el->m_flareFader -= temp_increase * 2.f;
				} else {
					el->m_flareFader += temp_increase * 2.f;
				}
			}

			el->m_flareFader = glm::clamp(el->m_flareFader, 0.f, .8f);
		}
	}
}

extern TextureContainer * tflare;



void goFor2DFX() {
	
	ARX_PROFILE_FUNC();
	
	if(g_debugToggles[6]) {
		RaycastDebugDraw();
	}
	
	GRenderer->SetFogColor(Color::none);
	UseRenderState state(render3D().blend(BlendOne, BlendOne).depthWrite(false).depthTest(false));
	
	for(size_t i = 0; i < g_culledDynamicLightsCount; i++) {
		const EERIE_LIGHT & el = *g_culledDynamicLights[i];

		if(!el.exist || !el.treat)
			continue;

		if(el.extras & EXTRAS_FLARE) {
			if(el.m_flareFader > 0.f) {
				Vec3f ltvv = EE_RT(el.pos);
				
				float v = el.m_flareFader;

				if(FADEDIR) {
					v *= 1.f - LAST_FADEVALUE;
				}

				float siz;

				if(el.extras & EXTRAS_FIXFLARESIZE)
					siz = el.ex_flaresize;
				else
					siz = -el.ex_flaresize;

				EERIEDrawSprite(el.pos, siz, tflare, (el.rgb * v).to<u8>(), ltvv.z);

			}
		}
	}
	
}
