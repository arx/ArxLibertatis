/*
 * Copyright 2013-2021 Arx Libertatis Team (see the AUTHORS file)
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

#include "gui/LoadLevelScreen.h"

#include <cstdio>

#include "core/Application.h"
#include "core/Core.h"
#include "platform/Time.h"
#include "game/Levels.h"
#include "graphics/Draw.h"
#include "graphics/Renderer.h"
#include "graphics/texture/TextureStage.h"
#include "graphics/data/TextureContainer.h"
#include "window/RenderWindow.h"

static float PROGRESS_BAR_TOTAL = 0;
static float PROGRESS_BAR_COUNT = 0;
static float OLD_PROGRESS_BAR_COUNT = 0;

static AreaId g_lastLoadedArea;
static TextureContainer * g_loadingScreenImage = nullptr;
static TextureContainer * pbar = nullptr;
static AreaId g_lastArea;

void progressBarReset() {
	OLD_PROGRESS_BAR_COUNT = PROGRESS_BAR_COUNT = 0;
}

void progressBarSetTotal(float total) {
	PROGRESS_BAR_TOTAL = total;
}

void progressBarAdvance(float delta) {
	PROGRESS_BAR_COUNT += delta;
}

void LoadLevelScreen(AreaId area ) {
	
	if(area) {
		g_lastArea = area;
	} else if(g_lastArea) {
		area = g_lastArea;
	} else {
		return;
	}
	
	static PlatformInstant last_progress_bar_update = platform::getTime();
	
	// only update if time since last update to progress bar > 16ms
	// and progress bar's value has actually changed
	PlatformInstant now = platform::getTime();
	if(now - last_progress_bar_update > 16ms && OLD_PROGRESS_BAR_COUNT != PROGRESS_BAR_COUNT) {
		
		UseTextureState textureState(TextureStage::FilterLinear, TextureStage::WrapClamp);
		
		float ratio = (PROGRESS_BAR_TOTAL > 0.f ? PROGRESS_BAR_COUNT / PROGRESS_BAR_TOTAL : 0);
		
		ratio = glm::clamp(ratio, 0.f, 1.f);

		GRenderer->Clear(Renderer::ColorBuffer);
		
		UseRenderState state(render2D().noBlend());
		
		if(area == AreaId(10)) {
			pbar = TextureContainer::LoadUI("graph/interface/menus/load_full");
		} else {
			pbar = TextureContainer::LoadUI("graph/interface/menus/load_full_level");
		}
		
		if(area != g_lastLoadedArea) {
			delete g_loadingScreenImage, g_loadingScreenImage = nullptr;
			g_lastLoadedArea = area;
			res::path image = "graph/levels/level" + std::to_string(u32(area)) + "/loading";
			g_loadingScreenImage = TextureContainer::LoadUI(image, TextureContainer::NoColorKey);
		}
		
		float scale = minSizeRatio();
		
		if(g_loadingScreenImage) {
			
			Vec2f size = (area == AreaId(10)) ? Vec2f(640, 480) : Vec2f(320, 390);
			size *= scale;
			
			Vec2f pos = Vec2f(g_size.center());
			pos += -size * 0.5f;
			
			EERIEDrawBitmap(Rectf(pos, size.x, size.y), 0.001f, g_loadingScreenImage, Color::white);
			
		}
		
		if(pbar) {
			Vec2f pos = Vec2f(g_size.center());
			pos += Vec2f(-100 * scale, ((area == AreaId(10)) ? 221 : 35) * scale);

			Vec2f size = Vec2f(ratio * 200 * scale, 8 * scale);
			EERIEDrawBitmap_uv(Rectf(pos, size.x, size.y), 0.f, pbar, Color::white, 0.f, 0.f, ratio, 1.f);
		}
		
		mainApp->getWindow()->showFrame();
		
		OLD_PROGRESS_BAR_COUNT = PROGRESS_BAR_COUNT;
		last_progress_bar_update = now;
	}
}

void LoadLevelScreenDestroy() {
	
	delete g_loadingScreenImage, g_loadingScreenImage = nullptr;
	g_lastLoadedArea = { };
	g_lastArea = { };
	PROGRESS_BAR_TOTAL = 0;
	
}
