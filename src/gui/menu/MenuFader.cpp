/*
 * Copyright 2016-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "gui/menu/MenuFader.h"

#include "core/Core.h"
#include "core/GameTime.h"
#include "graphics/Draw.h"
#include "graphics/Renderer.h"
#include "gui/MenuWidgets.h"

bool g_menuFadeActive = false;
Fade bFadeInOut = Fade_Out;
int iFadeAction = -1;

static PlatformDuration menuFadeElapsed = 0;

void MenuFader_reset() {
	iFadeAction = -1;
	g_menuFadeActive = false;
	menuFadeElapsed = 0;
}

static void FadeInOut(float _fVal) {
	
	TexturedVertex vertices[4];
	
	ColorRGBA iColor = Color::gray(_fVal).toRGB();
	
	vertices[0].p = Vec3f(0.f);
	vertices[0].w = 1.f;
	vertices[0].color = iColor;
	
	vertices[1].p = Vec3f(float(g_size.width()), 0.f, 0.f);
	vertices[1].w = 1.f;
	vertices[1].color = iColor;
	
	vertices[2].p = Vec3f(0.f, float(g_size.height()), 0.f);
	vertices[2].w = 1.f;
	vertices[2].color = iColor;
	
	vertices[3].p = Vec3f(float(g_size.width()), float(g_size.height()), 0.f);
	vertices[3].w = 1.f;
	vertices[3].color = iColor;
	
	UseRenderState state(render2D().blend(BlendZero, BlendInvSrcColor));
	GRenderer->ResetTexture(0);
	EERIEDRAWPRIM(Renderer::TriangleStrip, vertices, 4, true);
	
}

bool MenuFader_process() {
	
	const PlatformDuration fadeDuration = PlatformDurationMs(1000);
	
	float alpha = menuFadeElapsed / fadeDuration;
	FadeInOut(alpha);

	if(!g_menuFadeActive)
		return true;

	if(bFadeInOut == Fade_In) {
		menuFadeElapsed = menuFadeElapsed + g_platformTime.lastFrameDuration();
		
		if(menuFadeElapsed > fadeDuration) {
			menuFadeElapsed = fadeDuration;
			g_menuFadeActive = false;
		}
	} else {
		menuFadeElapsed = menuFadeElapsed - g_platformTime.lastFrameDuration();
		
		if(menuFadeElapsed < 0) {
			menuFadeElapsed = 0;
			g_menuFadeActive = false;
		}
	}

	return false;
}

void MenuFader_start(Fade fadeInOut, int fadeAction) {
	g_menuFadeActive = true;
	bFadeInOut = fadeInOut;
	iFadeAction = fadeAction;
}
