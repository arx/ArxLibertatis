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

#include "gui/Logo.h"

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

static TextureContainer * FISHTANK_img = NULL;
static TextureContainer * ARKANE_img = NULL;

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

static void ARX_INTERFACE_ShowLogo(TextureContainer * logo) {
	
	if(logo == NULL) {
		return;
	}
	
	GRenderer->Clear(Renderer::ColorBuffer);
	
	UseRenderState state(render2D().noBlend());
	UseTextureState textureState(TextureStage::FilterLinear, TextureStage::WrapClamp);
	
	Vec2f size = Vec2f(logo->size());
	
	Vec2f pos = Vec2f(g_size.center()) - size / 2.f;
	
	EERIEDrawBitmap(Rectf(pos, size.x, size.y), 0.001f, logo, Color::white);
	
}

void ARX_INTERFACE_ShowFISHTANK() {
	ARX_INTERFACE_ShowLogo(FISHTANK_img);
}

void ARX_INTERFACE_ShowARKANE() {
	ARX_INTERFACE_ShowLogo(ARKANE_img);
}
