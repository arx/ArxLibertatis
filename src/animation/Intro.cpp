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

// TODO header file

#include <algorithm>
#include <cstdio>

#include "core/Application.h"
#include "core/Core.h"

#include "game/Levels.h"

#include "gui/Menu.h"

#include "graphics/Color.h"
#include "graphics/Draw.h"
#include "graphics/Renderer.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/texture/TextureStage.h"

#include "platform/Time.h"

#include "scene/GameSound.h"

#include "window/RenderWindow.h"

using std::min;
using std::max;

extern float PROGRESS_BAR_TOTAL;
extern float PROGRESS_BAR_COUNT;
extern float OLD_PROGRESS_BAR_COUNT;

static TextureContainer * FISHTANK_img = NULL;
static TextureContainer * ARKANE_img = NULL;

void LoadScreen() {
	GRenderer->Clear(Renderer::ColorBuffer | Renderer::DepthBuffer);
}

bool ARX_INTERFACE_InitFISHTANK() {
	if(FISHTANK_img == NULL) {
		FISHTANK_img = TextureContainer::LoadUI("misc/logo", TextureContainer::NoColorKey);
	}
	return FISHTANK_img != NULL;
}

bool ARX_INTERFACE_InitARKANE() {
	if(ARKANE_img == NULL) {
		ARKANE_img = TextureContainer::LoadUI("graph/interface/misc/arkane",
		                                      TextureContainer::NoColorKey);
	}
	return ARKANE_img != NULL;
}

void ARX_INTERFACE_KillFISHTANK() {
	delete FISHTANK_img;
	FISHTANK_img = NULL;
}

void ARX_INTERFACE_KillARKANE() {
	delete ARKANE_img;
	ARKANE_img = NULL;
}

static void DrawCenteredImage(TextureContainer * tc) {
	EERIEDrawBitmap2(g_size.center().x - (tc->m_dwWidth * 0.5f),
					 g_size.center().y - (tc->m_dwHeight * 0.5f),
	                 static_cast<float>((int)(tc->m_dwWidth)),
	                 static_cast<float>((int)(tc->m_dwHeight)),
	                 0.001f, tc, Color::white);
}

void ARX_INTERFACE_ShowLogo(TextureContainer * logo) {
	
	if(logo == NULL) {
		return;
	}
	
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapClamp); 
	GRenderer->SetRenderState(Renderer::ColorKey, false);
	
	GRenderer->Clear(Renderer::ColorBuffer | Renderer::DepthBuffer);
	
	GRenderer->BeginScene();
	
	GRenderer->SetRenderState(Renderer::Fog, false);
	DrawCenteredImage(logo);
	
	GRenderer->EndScene();
	
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
	GRenderer->SetRenderState(Renderer::ColorKey, true);
}

void ARX_INTERFACE_ShowFISHTANK() {
	ARX_INTERFACE_ShowLogo(FISHTANK_img);
}

void ARX_INTERFACE_ShowARKANE() {
	ARX_INTERFACE_ShowLogo(ARKANE_img);
}

static long lastloadednum = -1;
static TextureContainer * tc = NULL;
static TextureContainer * pbar = NULL;
static long nopbar = -1;
static long lastnum = -1;

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
	
	static u32 last_progress_bar_update = Time::getMs();
	
	// only update if time since last update to progress bar > 16ms
	// and progress bar's value has actually changed
	if (Time::getElapsedMs(last_progress_bar_update) > 16 &&
		 OLD_PROGRESS_BAR_COUNT != PROGRESS_BAR_COUNT)
	{
		GRenderer->GetTextureStage(0)->setMinFilter(TextureStage::FilterLinear);
		GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterLinear);

		float ratio = (PROGRESS_BAR_TOTAL > 0.f ? PROGRESS_BAR_COUNT / PROGRESS_BAR_TOTAL : 0); 

		if (ratio > 1.f) ratio = 1.f;
		else if (ratio < 0.f) ratio = 0.f;

		GRenderer->Clear(Renderer::ColorBuffer | Renderer::DepthBuffer);
		
		GRenderer->BeginScene();
		
		GRenderer->SetRenderState(Renderer::DepthTest, true);
		GRenderer->SetCulling(Renderer::CullNone);
		GRenderer->SetRenderState(Renderer::DepthWrite, true);
		GRenderer->SetRenderState(Renderer::Fog, false);
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		
		if (num == 10) {
			pbar = TextureContainer::LoadUI("graph/interface/menus/load_full");
		} else {
			pbar = TextureContainer::LoadUI("graph/interface/menus/load_full_level");
		}
		
		nopbar = 1;
		
		if(num != lastloadednum) {
			delete tc, tc = NULL;
			lastloadednum = num;
			char temp[256];
			char tx[256];
			GetLevelNameByNum(num, tx);
			sprintf(temp, "graph/levels/level%s/loading", tx);
			tc = TextureContainer::LoadUI(temp, TextureContainer::NoColorKey);
		}
		
		float scale = minSizeRatio();
		
		if(tc) {
			GRenderer->SetRenderState(Renderer::ColorKey, false);
			
			Vec2f size = (num == 10) ? Vec2f(640, 480) : Vec2f(320, 390);
			size *= scale;
			EERIEDrawBitmap2(g_size.center().x - size.x * 0.5f, g_size.center().y - size.y * 0.5f,
												size.x, size.y, 0.001f, tc, Color::white);
			
			GRenderer->SetRenderState(Renderer::ColorKey, true);
		}
		
		if(pbar) {
			float px = g_size.center().x - 100 * scale;
			float py = g_size.center().y + ((num == 10) ? 221 : 35) * scale;
			float px2 = ratio * 200 * scale;
			float py2 = 8 * scale;
			EERIEDrawBitmap_uv(px, py, px2, py2, 0.f, pbar, Color::gray(1.0f), 0.f, 0.f, ratio, 1.f);
		}
		
		GRenderer->EndScene();
		mainApp->getWindow()->showFrame();
		
		OLD_PROGRESS_BAR_COUNT = PROGRESS_BAR_COUNT;
		last_progress_bar_update = Time::getMs();
	}
}

void LoadLevelScreen() {
	LoadLevelScreen(-1);
}

//-----------------------------------------------------------------------------
void ARX_INTERFACE_EndIntro()
{
	ARX_SOUND_MixerStop(ARX_SOUND_MixerGame);
	ARX_MENU_Launch(false);
}
