/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#include "scene/Light.h"

#include <boost/array.hpp>
#include <boost/foreach.hpp>
#include <boost/range/size.hpp>

#include "core/Application.h"
#include "core/GameTime.h"
#include "core/Core.h"

#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/Inventory.h"
#include "game/Player.h"

#include "graphics/Math.h"
#include "graphics/Draw.h"
#include "graphics/DrawLine.h"

#include "platform/profiler/Profiler.h"

#include "scene/Object.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"

static const float GLOBAL_LIGHT_FACTOR = 0.85f;

static const Color3f defaultAmbient = Color3f(0.09f, 0.09f, 0.09f);
static const int NPC_ITEMS_AMBIENT_VALUE_255 = 35;

EERIE_LIGHT * g_staticLights[g_staticLightsMax];
EERIE_LIGHT g_dynamicLights[g_dynamicLightsMax];

EERIE_LIGHT * g_culledDynamicLights[g_dynamicLightsMax];
size_t g_culledDynamicLightsCount = 0;

static EERIE_LIGHT * g_culledStaticLights[g_dynamicLightsMax];
size_t g_culledStaticLightsCount = 0;

void culledStaticLightsReset() {
	g_culledStaticLightsCount = 0;
}

void ColorMod::updateFromEntity(Entity * io, bool inBook) {
	
	factor = Color3f::white;
	term = Color3f::black;
	if(io) {
		factor *= io->special_color;
		term += io->highlightColor;
	}
	
	if(player.m_improve) {
		Color3f infra = (io) ? io->infracolor : Color3f(0.6f, 0.f, 1.f);
		
		factor *= infra;
		
		// Special case for drawing runes in book
		if(inBook) {
			term.r += infra.r * 512.f;
			term.g += infra.g;
			term.b += infra.b * 400.f;
		}
	}
	
	// Ambient light
	ambientColor = defaultAmbient * 255.f;
	if(io && (io->ioflags & (IO_NPC | IO_ITEM)))
		ambientColor = Color3f::gray(NPC_ITEMS_AMBIENT_VALUE_255);
}

void RecalcLight(EERIE_LIGHT * el) {
	el->rgb255 = el->rgb * 255.f;
	el->falldiffmul = 1.f / (el->fallend - el->fallstart);
}

void EERIE_LIGHT_GlobalInit() {
	
	static long init = 0;
	
	if(!init) {
		memset(g_staticLights, 0, sizeof(*g_staticLights) * g_staticLightsMax);
		init = 1;
		return;
	}
	
	for(size_t i = 0; i < g_staticLightsMax; i++) {
		if(g_staticLights[i]) {
			if(EERIE_LIGHT * dynLight = lightHandleGet(g_staticLights[i]->m_ignitionLightHandle)) {
				dynLight->m_exists = false;
			}
			delete g_staticLights[i];
			g_staticLights[i] = NULL;
		}
	}
}

long EERIE_LIGHT_Create() {
	
	for (size_t i = 0; i < g_staticLightsMax; i++) {
		if(!g_staticLights[i]) {
			g_staticLights[i] = new EERIE_LIGHT;
			return i;
		}
	}
	
	return -1;
}

static void ComputeLight2DPos(EERIE_LIGHT * _pL) {
	
	Vec4f p = worldToClipSpace(_pL->pos);
	if(p.w <= 0.f) {
		return;
	}
	
	Vec3f pos2d = Vec3f(p) / p.w;
	
	if(pos2d.z > 0.f && pos2d.z < 1000.f) {
		float siz = 50;
		float fMaxdist = player.m_telekinesis ? 850 : 300;
		
		float t = siz * (1.0f - 1.0f * p.w / fMaxdist) + 10;

		_pL->m_screenRect = Rectf(pos2d.x - t, pos2d.y - t, pos2d.x + t, pos2d.y + t);
	}
}

void TreatBackgroundDynlights() {
	
	ARX_PROFILE_FUNC();
	
	for(size_t i = 0; i < g_staticLightsMax; i++) {
		EERIE_LIGHT * light = g_staticLights[i];
		
		if(light && (light->extras & EXTRAS_SEMIDYNAMIC)) {
			
			float fMaxdist = player.m_telekinesis ? 850 : 300;
			
			if(!fartherThan(light->pos, g_camera->m_pos, fMaxdist)) {
				ComputeLight2DPos(light);
			} else {
				light->m_screenRect = Rectf(1, 0, -1, 0); // Intentionally invalid
			}
			
			if(!light->m_ignitionStatus) {
				
				// just extinguished
				EERIE_LIGHT * dynLight = lightHandleGet(light->m_ignitionLightHandle);
				if(dynLight) {
					dynLight->m_exists = false;
					light->m_ignitionLightHandle = LightHandle();
					for(size_t l = 0; l < entities.size(); l++) {
						const EntityHandle handle = EntityHandle(l);
						Entity * e = entities[handle];
						if(e && (e->ioflags & IO_MARKER)) {
							Vec3f _pos2 = GetItemWorldPosition(e);
							if(!fartherThan(light->pos, _pos2, 300.f)) {
								SendIOScriptEvent(NULL, e, SM_CUSTOM, "douse");
							}
						}
					}
				}
				
			} else {
				
				// just light up
				if(!lightHandleGet(light->m_ignitionLightHandle)) {
					for(size_t l = 0; l < entities.size(); l++) {
						const EntityHandle handle = EntityHandle(l);
						Entity * e = entities[handle];
						if(e && (e->ioflags & IO_MARKER)) {
							Vec3f _pos2 = GetItemWorldPosition(e);
							if(!fartherThan(light->pos, _pos2, 300.f)) {
								SendIOScriptEvent(NULL, e, SM_CUSTOM, "fire");
							}
						}
					}
					light->m_ignitionLightHandle = GetFreeDynLight();
				}
				
				EERIE_LIGHT * dynamicLight = lightHandleGet(light->m_ignitionLightHandle);
				if(dynamicLight) {
					dynamicLight->pos          = light->pos;
					dynamicLight->fallstart    = light->fallstart;
					dynamicLight->fallend      = light->fallend;
					dynamicLight->m_isIgnitionLight  = true;
					dynamicLight->intensity    = light->intensity;
					dynamicLight->ex_flaresize = light->ex_flaresize;
					dynamicLight->extras       = light->extras;
					dynamicLight->duration     = GameDurationMs(std::numeric_limits<long>::max());
					
					dynamicLight->rgb = light->rgb - light->rgb * light->ex_flicker * randomColor3f() * 0.5f;
					
					dynamicLight->rgb = componentwise_max(dynamicLight->rgb, Color3f::black);
					RecalcLight(dynamicLight);
				}
			}
		}
	}

	for(size_t i = 0; i < g_dynamicLightsMax; i++) {
		EERIE_LIGHT * el = &g_dynamicLights[i];

		if(el->m_exists && el->duration != 0) {
			const GameDuration elapsed = g_gameTime.now() - el->creationTime;
			const GameDuration duration = el->duration;

			if(elapsed >= duration) {
				float sub = g_gameTime.lastFrameDuration() / GameDurationMs(1000);

				el->rgb.r -= sub;
				el->rgb.g -= sub;
				el->rgb.b -= sub;

				if(el->rgb.r < 0)
					el->rgb.r = 0.f;

				if(el->rgb.g < 0)
					el->rgb.g = 0.f;

				if(el->rgb.b < 0)
					el->rgb.b = 0.f;

				if(el->rgb.r + el->rgb.g + el->rgb.b == 0) {
					el->m_exists = false;
					el->duration = 0;
				}
			}
		}
	}
}

void PrecalcDynamicLighting(const Vec3f & camPos, float camDepth) {
	
	ARX_PROFILE_FUNC();
	
	g_culledDynamicLightsCount = 0;
	
	BOOST_FOREACH(EERIE_LIGHT & light, g_dynamicLights) {
		if(light.m_exists && light.rgb != Color3f::black) {
			light.m_isVisible = closerThan(light.pos, camPos, camDepth + light.fallend);
			if(light.m_isVisible) {
				RecalcLight(&light);
				arx_assert(g_culledDynamicLightsCount < size_t(boost::size(g_culledDynamicLights)));
				g_culledDynamicLights[g_culledDynamicLightsCount++] = &light;
			}
		}
	}
	
}

void PrecalcIOLighting(const Vec3f & pos, float radius) {

	g_culledStaticLightsCount = 0;

	for(size_t i = 0; i < g_staticLightsMax; i++) {
		EERIE_LIGHT * el = g_staticLights[i];

		if(   el
		   && el->m_exists
		   && el->m_ignitionStatus
		   && !(el->extras & EXTRAS_SEMIDYNAMIC)
		   && (el->pos.x >= pos.x - radius)
		   && (el->pos.x <= pos.x + radius)
		   && (el->pos.z >= pos.z - radius)
		   && (el->pos.z <= pos.z + radius)
		) {
			RecalcLight(el);
			g_culledStaticLights[g_culledStaticLightsCount] = el;
			
			g_culledStaticLightsCount++;
			
			if(g_culledStaticLightsCount >= g_dynamicLightsMax)
				g_culledStaticLightsCount--;
		}
	}
}

static bool lightHandleIsValid(LightHandle num) {
	return num.handleData() >= 0 && size_t(num.handleData()) < g_dynamicLightsMax
	       && g_dynamicLights[num.handleData()].m_exists;
}

EERIE_LIGHT * lightHandleGet(LightHandle lightHandle) {
	if(lightHandleIsValid(lightHandle) || lightHandle == torchLightHandle) {
		return &g_dynamicLights[lightHandle.handleData()];
	} else {
		return NULL;
	}
}

void lightHandleDestroy(LightHandle & handle) {
	
	if(EERIE_LIGHT * light = lightHandleGet(handle)) {
		light->m_exists = false;
	}
	
	handle = LightHandle();
}

void endLightDelayed(LightHandle & handle, GameDuration delay) {
	
	if(EERIE_LIGHT * light = lightHandleGet(handle)) {
		light->duration = delay;
		light->creationTime = g_gameTime.now();
	}
	
}

void resetDynLights() {
	for(size_t i = 0; i < g_dynamicLightsMax; i++) {
		g_dynamicLights[i].m_exists = false;
	}
}


LightHandle GetFreeDynLight() {

	for(size_t i = 1; i < g_dynamicLightsMax; i++) {
		EERIE_LIGHT & light = g_dynamicLights[i];
		if(!light.m_exists) {
			light.m_exists = true;
			light.m_isIgnitionLight = false;
			light.intensity = 1.3f;
			light.m_isVisible = true;
			light.creationTime = g_gameTime.now();
			light.duration = 0;
			light.extras = 0;
			light.m_storedFlameTime.reset();
			return LightHandle(i);
		}
	}

	return LightHandle();
}

EERIE_LIGHT * dynLightCreate(LightHandle & handle) {
	if(!lightHandleGet(handle)) {
		handle = GetFreeDynLight();
	}
	
	return lightHandleGet(handle);
}

EERIE_LIGHT * dynLightCreate() {
	LightHandle handle = GetFreeDynLight();
	return lightHandleGet(handle);
}


void ClearDynLights() {
	
	for(size_t i = 0; i < g_dynamicLightsMax; i++) {
		g_dynamicLights[i].m_exists = false;
	}
	
	for(size_t i = 0; i < g_staticLightsMax; i++) {
		if(g_staticLights[i]) {
			g_staticLights[i]->m_ignitionLightHandle = LightHandle();
		}
	}

	g_culledDynamicLightsCount = 0;
	g_culledStaticLightsCount = 0;
}



static size_t MAX_LLIGHTS = llightsSize;

// Inserts Light in the List of Nearest Lights
static void Insertllight(boost::array<EERIE_LIGHT *, llightsSize> & llights,
                         boost::array<float, llightsSize> & values,
                         EERIE_LIGHT * el,
                         const Vec3f & pos,
                         bool forPlayerColor
) {
	if(!el)
		return;
	
	float dist = glm::distance(el->pos, pos);
	
	if(forPlayerColor) {
		if(!(el->fallstart > 10.f && el->fallend > 100.f)) {
			return;
		}
		
		dist -= el->fallstart;
	}
	
	float threshold = el->fallend + 560.f;
	if(dist > threshold) {
		return;
	}
	
	float val = dist - el->fallend;
	if(val < 0) {
		val = 0;
	}
	
	for(size_t i = 0; i < MAX_LLIGHTS; i++) {
		if(!llights[i]) {
			llights[i] = el;
			values[i] = val;
			return;
		} else if(val <= values[i]) { // Inserts light at the right place
			for(size_t j = MAX_LLIGHTS - 1; j > i; j--) {
				if(llights[j - 1]) {
					llights[j] = llights[j - 1];
					values[j] = values[j - 1];
				}
			}
			llights[i] = el;
			values[i] = val;
			return;
		}
	}
	
}

void setMaxLLights(size_t count) {
	MAX_LLIGHTS = glm::clamp(count, size_t(6), llightsSize);
}

void UpdateLlights(ShaderLight lights[], size_t & lightsCount, const Vec3f pos, bool forPlayerColor) {
	
	ARX_PROFILE_FUNC();
	
	boost::array<EERIE_LIGHT *, llightsSize> llights;
	llights.fill(NULL);
	boost::array<float, llightsSize> values;
	values.fill(999999999.f);
	
	for(size_t i = 0; i < g_culledStaticLightsCount; i++) {
		Insertllight(llights, values, g_culledStaticLights[i], pos, forPlayerColor);
	}

	for(size_t i = 0; i < g_culledDynamicLightsCount; i++) {
		Insertllight(llights, values, g_culledDynamicLights[i], pos, forPlayerColor);
	}
	
	lightsCount = 0;
	for(size_t i = 0; i < MAX_LLIGHTS; i++) {
		if(llights[i]) {
			EERIE_LIGHT * el = llights[i];
			ShaderLight & sl = lights[i];
			
			sl.pos = el->pos;
			sl.fallstart = el->fallstart;
			sl.fallend = el->fallend;
			sl.falldiffmul = el->falldiffmul;
			sl.intensity = el->intensity;
			sl.rgb = el->rgb;
			sl.rgb255 = el->rgb255;
			
			lightsCount = i + 1;
		} else {
			break;
		}
	}
}

struct TILE_LIGHTS
{
	std::vector<EERIE_LIGHT *> el;
};

static TILE_LIGHTS tilelights[MAX_BKGX][MAX_BKGZ];

void InitTileLights()
{
	for(long z = 0; z < MAX_BKGZ; z++)
	for(long x = 0; x < MAX_BKGX; x++) {
		tilelights[x][z].el.clear();
	}
}

void ComputeTileLights(short x, short z) {
	
	tilelights[x][z].el.clear();
	
	Vec2f tileCenter = (Vec2f(x, z) + 0.5f) * g_backgroundTileSize;
	
	for(size_t i = 0; i < g_culledDynamicLightsCount; i++) {
		EERIE_LIGHT * light = g_culledDynamicLights[i];
		
		if(closerThan(tileCenter, Vec2f(light->pos.x, light->pos.z), light->fallend + 60.f)) {

			tilelights[x][z].el.push_back(light);
		}
	}
}

void ClearTileLights() {
	
	for(long z = 0; z < MAX_BKGZ; z++)
	for(long x = 0; x < MAX_BKGX; x++) {
		tilelights[x][z].el.clear();
	}
}

float GetColorz(const Vec3f & pos) {

	ShaderLight lights[llightsSize];
	size_t lightsCount;
	UpdateLlights(lights, lightsCount, pos, true);
	
	Color3f ff = Color3f(0.f, 0.f, 0.f);
	
	for(size_t k = 0; k < lightsCount; k++) {
		const ShaderLight & light = lights[k];
		
		float dd = fdist(light.pos, pos);
		
		if(dd < light.fallend) {
			float dc;
			
			if(dd <= light.fallstart) {
				dc = light.intensity * GLOBAL_LIGHT_FACTOR;
			} else {
				float p = ((light.fallend - dd) * light.falldiffmul);
				
				if(p <= 0.f)
					dc = 0.f;
				else
					dc = p * light.intensity * GLOBAL_LIGHT_FACTOR;
			}
			
			dc *= 0.4f * 255.f;
			ff.r = std::max(ff.r, light.rgb.r * dc);
			ff.g = std::max(ff.g, light.rgb.g * dc);
			ff.b = std::max(ff.b, light.rgb.b * dc);
		}
	}


	EERIEPOLY * ep;
	float needy;
	ep = CheckInPoly(pos, &needy);

	if(ep != NULL) {
		Color3f _ff = Color3f(0.f, 0.f, 0.f);
		
		long to = (ep->type & POLY_QUAD) ? 4 : 3;
		float div = (1.0f / to);

		EP_DATA & epdata = portals->rooms[ep->room].epdata[0];
		ApplyTileLights(ep, epdata.tile);

		for(long i = 0; i < to; i++) {
			Color col = Color::fromRGBA(ep->color[i]);
			_ff.r += float(col.r);
			_ff.g += float(col.g);
			_ff.b += float(col.b);
		}

		_ff.r *= div;
		_ff.g *= div;
		_ff.b *= div;
		float ratio, ratio2;
		ratio = glm::abs(needy - pos.y) * (1.0f / 300);
		ratio = (1.f - ratio);
		ratio2 = 1.f - ratio;
		ff.r = ff.r * ratio2 + _ff.r * ratio;
		ff.g = ff.g * ratio2 + _ff.g * ratio;
		ff.b = ff.b * ratio2 + _ff.b * ratio;
	}
	
	return (std::min(ff.r, 255.f) + std::min(ff.g, 255.f) + std::min(ff.b, 255.f)) * (1.f / 3);
}

ColorRGBA ApplyLight(ShaderLight lights[], size_t lightsCount, const glm::quat & quat, const Vec3f & position,
                     const Vec3f & normal, const ColorMod & colorMod, float materialDiffuse) {
	
	Color3f tempColor = colorMod.ambientColor;
	
	glm::quat inv = glm::inverse(quat);
	
	// Dynamic lights
	for(size_t l = 0; l != lightsCount; l++) {
		const ShaderLight & light = lights[l];
		
		Vec3f vLight = glm::normalize(light.pos - position);
		
		Vec3f Cur_vLights = inv * vLight;
		
		float cosangle = glm::dot(normal, Cur_vLights);
		
		// If light visible
		if(cosangle > 0.f) {
			float distance = fdist(position, light.pos);
			
			// Evaluate its intensity depending on the distance Light<->Object
			if(distance <= light.fallstart) {
				cosangle *= light.intensity * GLOBAL_LIGHT_FACTOR;
			} else {
				float p = ((light.fallend - distance) * light.falldiffmul);
				
				if(p <= 0.f)
					cosangle = 0.f;
				else
					cosangle *= p * (light.intensity * GLOBAL_LIGHT_FACTOR);
			}
			
			cosangle *= materialDiffuse;

			tempColor += light.rgb255 * cosangle;
		}
	}

	tempColor *= colorMod.factor;
	tempColor += colorMod.term;

	u8 ir = clipByte255(int(tempColor.r));
	u8 ig = clipByte255(int(tempColor.g));
	u8 ib = clipByte255(int(tempColor.b));

	return Color(ir, ig, ib).toRGBA();
}

void ApplyTileLights(EERIEPOLY * ep, const Vec2s & pos)
{

	Color3f lightInfraFactor = Color3f::white;
	if(player.m_improve) {
		lightInfraFactor.r = 4.f;
	}

	TILE_LIGHTS * tls = &tilelights[pos.x][pos.y];
	size_t nbvert = (ep->type & POLY_QUAD) ? 4 : 3;

	for(size_t j = 0; j < nbvert; j++) {

		if(tls->el.empty()) {
			ep->color[j] = ep->v[j].color;
			continue;
		}

		Color3f tempColor;
		Color c = Color::fromRGBA(ep->v[j].color);
		tempColor.r = c.r;
		tempColor.g = c.g;
		tempColor.b = c.b;

		Vec3f & position = ep->v[j].p;
		Vec3f & normal = ep->nrml[j];

		for(size_t i = 0; i < tls->el.size(); i++) {
			EERIE_LIGHT * light = tls->el[i];

			Vec3f vLight = glm::normalize(light->pos - position);

			float cosangle = glm::dot(normal, vLight);

			if(cosangle > 0.f) {
				float distance = fdist(light->pos, position);

				if(distance <= light->fallstart) {
					cosangle *= light->intensity * GLOBAL_LIGHT_FACTOR;
				} else {
					float p = ((light->fallend - distance) * light->falldiffmul);

					if(p <= 0.f)
						cosangle = 0.f;
					else
						cosangle *= p * (light->intensity * GLOBAL_LIGHT_FACTOR);
				}
				cosangle *= 0.5f;

				tempColor += light->rgb255 * lightInfraFactor * cosangle;
			}
		}

		u8 ir = clipByte255(int(tempColor.r));
		u8 ig = clipByte255(int(tempColor.g));
		u8 ib = clipByte255(int(tempColor.b));
		ep->color[j] = Color(ir, ig, ib).toRGBA();
	}
}
