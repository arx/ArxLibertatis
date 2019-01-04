/*
 * Copyright 2013-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/effects/Fade.h"

#include "core/Core.h"
#include "core/GameTime.h"

#include "graphics/Draw.h"
#include "graphics/Renderer.h"

static PlatformDuration FADEDURATION = 0;
long FADEDIR = 0;
static PlatformInstant FADESTART = 0;
float LAST_FADEVALUE = 1.f;
static Color3f FADECOLOR;


void fadeReset() {
	FADEDIR = 0;
	FADEDURATION = 0;
	FADESTART = 0;
	FADECOLOR = Color3f::black;
}

void fadeSetColor(Color3f color) {
	FADECOLOR = color;
}

void fadeRequestStart(FadeType type, const PlatformDuration duration) {
	switch(type) {
		case FadeType_In:
			FADEDIR = 1;
			break;
		case FadeType_Out:
			FADEDIR = -1;
			break;
	}
	
	FADEDURATION = duration;
	FADESTART = g_platformTime.frameStart();
}

void ManageFade() {
	
	PlatformDuration elapsed = g_platformTime.frameStart() - FADESTART;
	if(elapsed < 0)
		return;

	float Visibility = elapsed / FADEDURATION;

	if(FADEDIR > 0)
		Visibility = 1.f - Visibility;

	if(Visibility > 1.f)
		Visibility = 1.f;

	if(Visibility < 0.f) {
		FADEDIR = 0;
		return;
	}
	
	LAST_FADEVALUE = Visibility;
	
	UseRenderState state(render2D());
	
	EERIEDrawBitmap(Rectf(g_size), 0.0001f, NULL, Color(Color4f(FADECOLOR, Visibility)));
	
}
