/*
 * Copyright 2016 Arx Libertatis Team (see the AUTHORS file)
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
#include "game/Player.h"
#include "graphics/Draw.h"
#include "graphics/GraphicsModes.h"
#include "graphics/Renderer.h"
#include "graphics/effects/Fade.h"
#include "gui/CinematicBorder.h"
#include "platform/profiler/Profiler.h"
#include "scene/Interactive.h"
#include "scene/Light.h"

void update2DFX() {
	
	ARX_PROFILE_FUNC();
	
	TexturedVertex ltvv;

	Entity* pTableIO[256];
	size_t nNbInTableIO = 0;

	float temp_increase = g_framedelay * (1.0f/1000) * 4.f;

	bool bComputeIO = false;

	for(size_t i = 0; i < g_culledDynamicLightsCount; i++) {
		EERIE_LIGHT *el = g_culledDynamicLights[i];

		EERIE_BKG_INFO * bkgData = getFastBackgroundData(el->pos.x, el->pos.z);

		if(!bkgData || !bkgData->treat) {
			el->treat=0;
			continue;
		}

		if(el->extras & EXTRAS_FLARE) {
			Vec3f lv = el->pos;
			EE_RTP(lv, ltvv);
			el->m_flareFader -= temp_increase;

			if(!(player.Interface & INTER_COMBATMODE) && (player.Interface & INTER_MAP))
				continue;

			if(ltvv.rhw > 0.f &&
				ltvv.p.x > 0.f &&
				ltvv.p.y > (cinematicBorder.CINEMA_DECAL * g_sizeRatio.y) &&
				ltvv.p.x < g_size.width() &&
				ltvv.p.y < (g_size.height()-(cinematicBorder.CINEMA_DECAL * g_sizeRatio.y))
				)
			{
				Vec3f vector = lv - ACTIVECAM->orgTrans.pos;
				lv -= vector * (50.f / glm::length(vector));

				float fZFar=ACTIVECAM->ProjectionMatrix[2][2]*(1.f/(ACTIVECAM->cdepth*fZFogEnd))+ACTIVECAM->ProjectionMatrix[3][2];

				Vec3f hit;
				Vec2s ees2dlv;
				Vec3f ee3dlv = lv;

				ees2dlv.x = checked_range_cast<short>(ltvv.p.x);
				ees2dlv.y = checked_range_cast<short>(ltvv.p.y);

				if(!bComputeIO) {
					GetFirstInterAtPos(ees2dlv, 2, &ee3dlv, pTableIO, &nNbInTableIO);
					bComputeIO = true;
				}

				if(ltvv.p.z > fZFar ||
					EERIELaunchRay3(ACTIVECAM->orgTrans.pos, ee3dlv, hit, 1) ||
					GetFirstInterAtPos(ees2dlv, 3, &ee3dlv, pTableIO, &nNbInTableIO )
					)
				{
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
	
	GRenderer->SetRenderState(Renderer::Fog, true);
	GRenderer->SetBlendFunc(BlendOne, BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetCulling(CullNone);
	GRenderer->SetRenderState(Renderer::DepthTest, false);
	GRenderer->SetFogColor(Color::none);

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

	GRenderer->SetRenderState(Renderer::DepthTest, true);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::Fog, false);
}
