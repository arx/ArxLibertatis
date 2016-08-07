/*
 * Copyright 2013 Arx Libertatis Team (see the AUTHORS file)
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

static long lastloadednum = -1;
static TextureContainer * tc = NULL;
static TextureContainer * pbar = NULL;
static long lastnum = -1;

void progressBarReset() {
	OLD_PROGRESS_BAR_COUNT = PROGRESS_BAR_COUNT = 0;
}

void progressBarSetTotal(float total) {
	PROGRESS_BAR_TOTAL = total;
}

void progressBarAdvance(float delta) {
	PROGRESS_BAR_COUNT += delta;
}

void LoadLevelScreen(long num) {
	
	// resets status
	if(num < -1) {
		delete tc, tc = NULL;
		lastloadednum = -1;
		lastnum = -1;
		PROGRESS_BAR_TOTAL = 0;
		return;
	}
	
	if(num == -1) {
		num = lastnum;
	}
	lastnum = num;
	
	if(num < 0) {
		return;
	}
	
	static u32 last_progress_bar_update = platform::getTimeMs();
	
	// only update if time since last update to progress bar > 16ms
	// and progress bar's value has actually changed
	if (platform::getElapsedMs(last_progress_bar_update) > 16 &&
		 OLD_PROGRESS_BAR_COUNT != PROGRESS_BAR_COUNT)
	{
		GRenderer->GetTextureStage(0)->setMinFilter(TextureStage::FilterLinear);
		GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterLinear);

		float ratio = (PROGRESS_BAR_TOTAL > 0.f ? PROGRESS_BAR_COUNT / PROGRESS_BAR_TOTAL : 0); 

		ratio = glm::clamp(ratio, 0.f, 1.f);

		GRenderer->Clear(Renderer::ColorBuffer | Renderer::DepthBuffer);
		
		GRenderer->SetRenderState(Renderer::DepthTest, true);
		GRenderer->SetCulling(CullNone);
		GRenderer->SetRenderState(Renderer::DepthWrite, true);
		GRenderer->SetRenderState(Renderer::Fog, false);
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		
		if (num == 10) {
			pbar = TextureContainer::LoadUI("graph/interface/menus/load_full");
		} else {
			pbar = TextureContainer::LoadUI("graph/interface/menus/load_full_level");
		}
		
		if(num != lastloadednum) {
			delete tc, tc = NULL;
			lastloadednum = num;
			char temp[256];
			const char * tx = GetLevelNameByNum(num);
			sprintf(temp, "graph/levels/level%s/loading", tx);
			tc = TextureContainer::LoadUI(temp, TextureContainer::NoColorKey);
		}
		
		float scale = minSizeRatio();
		
		if(tc) {
			GRenderer->SetRenderState(Renderer::ColorKey, false);
			
			Vec2f size = (num == 10) ? Vec2f(640, 480) : Vec2f(320, 390);
			size *= scale;
			
			Vec2f pos = Vec2f(g_size.center());
			pos += -size * 0.5f;
			
			EERIEDrawBitmap2(Rectf(pos, size.x, size.y), 0.001f, tc, Color::white);
			
			GRenderer->SetRenderState(Renderer::ColorKey, true);
		}
		
		if(pbar) {
			Vec2f pos = Vec2f(g_size.center());
			pos += Vec2f(-100 * scale, ((num == 10) ? 221 : 35) * scale);

			Vec2f size = Vec2f(ratio * 200 * scale, 8 * scale);
			EERIEDrawBitmap_uv(Rectf(pos, size.x, size.y), 0.f, pbar, Color::white, 0.f, 0.f, ratio, 1.f);
		}
		
		mainApp->getWindow()->showFrame();
		
		OLD_PROGRESS_BAR_COUNT = PROGRESS_BAR_COUNT;
		last_progress_bar_update = platform::getTimeMs();
	}
}

void LoadLevelScreen() {
	LoadLevelScreen(-1);
}

