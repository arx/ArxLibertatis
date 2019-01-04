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

#include "graphics/GlobalFog.h"

#include <cstring>
#include <algorithm>

#include "core/Application.h"
#include "core/Config.h"
#include "core/Core.h"
#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Renderer.h"
#include "graphics/data/Mesh.h"
#include "platform/profiler/Profiler.h"

GLOBAL_MODS g_currentFogParameters;
GLOBAL_MODS g_desiredFogParameters;

// change the clipping Z max & min
static const float DEFAULT_ZCLIP = 6400.f;
static const float DEFAULT_MINZCLIP = 1200.f;

Color g_fogColor = Color::none;

void ARX_GLOBALMODS_Reset() {
	
	g_desiredFogParameters = GLOBAL_MODS();
	g_currentFogParameters = GLOBAL_MODS();
	g_currentFogParameters.zclip = DEFAULT_ZCLIP;
	g_desiredFogParameters.zclip = DEFAULT_ZCLIP;
	g_currentFogParameters.depthcolor = Color3f::black;
	g_desiredFogParameters.depthcolor = Color3f::black;
	g_fogColor = Color::none;
	
}

static float Approach(float current, float desired, float increment) {
	
	if(desired > current) {
		current = std::min(current + increment, desired);
	} else if(desired < current) {
		current = std::min(current - increment, desired);
	}
	
	return current;
}

void ARX_GLOBALMODS_Apply() {
	
	ARX_PROFILE_FUNC();
	
	float baseinc = g_framedelay;
	float incdiv1000 = g_framedelay * 0.001f;
	
	GLOBAL_MODS & current = g_currentFogParameters;
	GLOBAL_MODS & desired = g_desiredFogParameters;
	
	if (desired.flags & GMOD_ZCLIP) {
		current.zclip = Approach(current.zclip, desired.zclip, baseinc * 2);
	} else { // return to default...
		desired.zclip = current.zclip = Approach(current.zclip, DEFAULT_ZCLIP, baseinc * 2);
	}

	// Now goes for RGB mods
	if(desired.flags & GMOD_DCOLOR) {
		current.depthcolor.r = Approach(current.depthcolor.r, desired.depthcolor.r, incdiv1000);
		current.depthcolor.g = Approach(current.depthcolor.g, desired.depthcolor.g, incdiv1000);
		current.depthcolor.b = Approach(current.depthcolor.b, desired.depthcolor.b, incdiv1000);
	} else {
		current.depthcolor.r = Approach(current.depthcolor.r, 0, incdiv1000);
		current.depthcolor.g = Approach(current.depthcolor.g, 0, incdiv1000);
		current.depthcolor.b = Approach(current.depthcolor.b, 0, incdiv1000);
	}
	
	float fZclipp = config.video.fogDistance * 1.2f * (DEFAULT_ZCLIP - DEFAULT_MINZCLIP) / 10.f + DEFAULT_MINZCLIP;
	fZclipp += (g_camera->focal - 310.f) * 5.f;
	g_camera->cdepth = std::min(current.zclip, fZclipp);
	
	g_fogColor = Color(current.depthcolor);
}
