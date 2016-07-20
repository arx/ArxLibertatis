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
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#include "scene/Light.h"

#include <boost/array.hpp>

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

static const float GLOBAL_LIGHT_FACTOR=0.85f;

static const Color3f defaultAmbient = Color3f(0.09f, 0.09f, 0.09f);
static const int NPC_ITEMS_AMBIENT_VALUE_255 = 35;

EERIE_LIGHT * GLight[MAX_LIGHTS];
EERIE_LIGHT DynLight[MAX_DYNLIGHTS];

EERIE_LIGHT * PDL[MAX_DYNLIGHTS];
size_t TOTPDL = 0;

static EERIE_LIGHT * IO_PDL[MAX_DYNLIGHTS];
size_t TOTIOPDL = 0;

void ColorMod::updateFromEntity(Entity *io, bool inBook) {
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
		memset(GLight, 0, sizeof(*GLight) * MAX_LIGHTS);
		init = 1;
		return;
	}
	
	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		if(GLight[i]) {
			if(lightHandleIsValid(GLight[i]->m_ignitionLightHandle)) {
				lightHandleGet(GLight[i]->m_ignitionLightHandle)->exist = 0;
			}
			free(GLight[i]);
			GLight[i] = NULL;
		}
	}
}

long EERIE_LIGHT_GetFree() {
	
	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		if(!GLight[i]) {
			return i;
		}
	}
	
	return -1;
}

long EERIE_LIGHT_Create() {
	
	for (size_t i = 0; i < MAX_LIGHTS; i++) {
		if(!GLight[i]) {
			
			GLight[i] = (EERIE_LIGHT *)malloc(sizeof(EERIE_LIGHT));
			if(!GLight[i]) {
				return -1;
			}
			
			memset(GLight[i], 0, sizeof(EERIE_LIGHT));
			GLight[i]->sample = audio::INVALID_ID;
			GLight[i]->m_ignitionLightHandle = LightHandle();
			return i;
		}
	}
	
	return -1;
}


long EERIE_LIGHT_Count() {
	
	long count = 0;
	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		if(GLight[i] && !GLight[i]->m_isIgnitionLight) {
			count++;
		}
	}
	
	return count;
}

void EERIE_LIGHT_GlobalAdd(const EERIE_LIGHT * el)
{
	long num = EERIE_LIGHT_GetFree();

	if (num > -1)
	{
		GLight[num] = (EERIE_LIGHT *)malloc(sizeof(EERIE_LIGHT));
		memcpy(GLight[num], el, sizeof(EERIE_LIGHT));
		GLight[num]->m_ignitionLightHandle = LightHandle();
		GLight[num]->sample = audio::INVALID_ID;
	}
}

void EERIE_LIGHT_MoveAll(const Vec3f & trans) {
	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		if(GLight[i]) {
			GLight[i]->pos += trans;
		}
	}
}

static void ComputeLight2DPos(EERIE_LIGHT * _pL) {
	
	TexturedVertex out;
	EE_RTP(_pL->pos, out);
	
	if(out.p.z > 0.f && out.p.z < 1000.f && out.rhw > 0) {
		float siz = 50;
		float fMaxdist = player.m_telekinesis ? 850 : 300;
		
		float t = siz * (1.0f - 1.0f / (out.rhw * fMaxdist)) + 10;

		_pL->m_screenRect = Rectf(out.p.x - t, out.p.y - t, out.p.x + t, out.p.y + t);
	}
}

void TreatBackgroundDynlights() {
	
	ARX_PROFILE_FUNC();
	
	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		EERIE_LIGHT *light = GLight[i];

		if(light && (light->extras & EXTRAS_SEMIDYNAMIC)) {
			
			float fMaxdist = player.m_telekinesis ? 850 : 300;
			
			if(!fartherThan(light->pos, ACTIVECAM->orgTrans.pos, fMaxdist)) {
				ComputeLight2DPos(light);
			} else {
				light->m_screenRect = Rectf(1, 0, -1, 0); // Intentionally invalid
			}

			if(!light->m_ignitionStatus) {
				// just extinguished
				if(lightHandleIsValid(light->m_ignitionLightHandle)) {
					lightHandleGet(light->m_ignitionLightHandle)->exist = 0;
					light->m_ignitionLightHandle = LightHandle();
					
					for(size_t l = 0; l < entities.size(); l++) {
						const EntityHandle handle = EntityHandle(l);
						Entity * e = entities[handle];
						
						if(e && (e->ioflags & IO_MARKER)) {
							Vec3f _pos2 = GetItemWorldPosition(e);
							if(!fartherThan(light->pos, _pos2, 300.f)) {
								SendIOScriptEvent(e, SM_CUSTOM, "douse");
							}
						}
					}
				}
			} else {
				// just light up
				if(!lightHandleIsValid(light->m_ignitionLightHandle)) {
					for(size_t l = 0; l < entities.size(); l++) {
						const EntityHandle handle = EntityHandle(l);
						Entity * e = entities[handle];
						
						if(e && (e->ioflags & IO_MARKER)) {
							Vec3f _pos2 = GetItemWorldPosition(e);
							if(!fartherThan(light->pos, _pos2, 300.f)) {
								SendIOScriptEvent(e, SM_CUSTOM, "fire");
							}
						}
					}

					light->m_ignitionLightHandle = GetFreeDynLight();
				}
				
				if(lightHandleIsValid(light->m_ignitionLightHandle)) {
					EERIE_LIGHT *dynamicLight = lightHandleGet(light->m_ignitionLightHandle);

					dynamicLight->pos          = light->pos;
					dynamicLight->fallstart    = light->fallstart;
					dynamicLight->fallend      = light->fallend;
					dynamicLight->m_isIgnitionLight  = true;
					dynamicLight->intensity    = light->intensity;
					dynamicLight->ex_flaresize = light->ex_flaresize;
					dynamicLight->extras       = light->extras;
					dynamicLight->duration     = ArxDurationMs(std::numeric_limits<long>::max());
					
					dynamicLight->rgb = light->rgb - light->rgb * light->ex_flicker * randomColor3f() * 0.5f;
					
					dynamicLight->rgb = componentwise_max(dynamicLight->rgb, Color3f::black);
					RecalcLight(dynamicLight);
				}
			}
		}
	}

	for(size_t i = 0; i < MAX_DYNLIGHTS; i++) {
		EERIE_LIGHT * el = &DynLight[i];

		if(el->exist && el->duration) {
			const ArxDuration elapsed = arxtime.now() - el->creationTime;
			const ArxDuration duration = el->duration;

			if(elapsed >= duration) {
				float sub = g_framedelay * 0.001f;

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
					el->exist = 0;
					el->duration = ArxDuration_ZERO;
				}
			}
		}
	}
}

void PrecalcDynamicLighting(long x0, long z0, long x1, long z1, const Vec3f & camPos, float camDepth) {
	
	ARX_PROFILE_FUNC();
	
	TOTPDL = 0;
	
	float fx0 = ACTIVEBKG->Xdiv * x0;
	float fz0 = ACTIVEBKG->Zdiv * z0;
	float fx1 = ACTIVEBKG->Xdiv * x1;
	float fz1 = ACTIVEBKG->Zdiv * z1;
	
	for(size_t i = 0; i < MAX_DYNLIGHTS; i++) {
		EERIE_LIGHT * el = &DynLight[i];

		if(el->exist && el->rgb.r >= 0.f) {
			if(   el->pos.x >= fx0
			   && el->pos.x <= fx1
			   && el->pos.z >= fz0
			   && el->pos.z <= fz1
			   && closerThan(el->pos, camPos, camDepth)
			) {
				el->treat = 1;
				RecalcLight(el);
				PDL[TOTPDL] = el;
				TOTPDL++;

				if(TOTPDL >= MAX_DYNLIGHTS)
					TOTPDL--;
			}
			else if(el->treat)
				el->treat = 0;
		}
	}
}

void PrecalcIOLighting(const Vec3f & pos, float radius) {

	TOTIOPDL = 0;

	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		EERIE_LIGHT * el = GLight[i];

		if(   el
		   && el->exist
		   && el->m_ignitionStatus
		   && !(el->extras & EXTRAS_SEMIDYNAMIC)
		   && (el->pos.x >= pos.x - radius)
		   && (el->pos.x <= pos.x + radius)
		   && (el->pos.z >= pos.z - radius)
		   && (el->pos.z <= pos.z + radius)
		) {
			RecalcLight(el);
			IO_PDL[TOTIOPDL] = el;
			
			TOTIOPDL++;
			
			if(TOTIOPDL >= MAX_DYNLIGHTS)
				TOTIOPDL--;
		}
	}
}

EERIE_LIGHT * lightHandleGet(LightHandle lightHandle) {
	return &DynLight[lightHandle.handleData()];
}

bool lightHandleIsValid(LightHandle num)
{
	return (long)num.handleData() >= 0 && ((size_t)num.handleData() < MAX_DYNLIGHTS) && DynLight[num.handleData()].exist;
}

void lightHandleDestroy(LightHandle & handle) {
	if(lightHandleIsValid(handle)) {
		lightHandleGet(handle)->exist = 0;
	}
	handle = LightHandle();
}

void endLightDelayed(LightHandle & handle, ArxDuration delay) {
	
	if(lightHandleIsValid(handle)) {
		EERIE_LIGHT * light = lightHandleGet(handle);
		
		light->duration = delay;
		light->creationTime = arxtime.now();
	}
}


LightHandle GetFreeDynLight() {

	for(size_t i = 1; i < MAX_DYNLIGHTS; i++) {
		if(!(DynLight[i].exist)) {
			DynLight[i].exist = 1;
			DynLight[i].m_isIgnitionLight = false;
			DynLight[i].intensity = 1.3f;
			DynLight[i].treat = 1;
			DynLight[i].creationTime = arxtime.now();
			DynLight[i].duration = ArxDuration_ZERO;
			DynLight[i].extras = 0;
			DynLight[i].m_storedFlameTime.reset();
			return LightHandle(i);
		}
	}

	return LightHandle();
}

void ClearDynLights() {

	for(size_t i = 0; i < MAX_DYNLIGHTS; i++) {
		if(DynLight[i].exist) {
			DynLight[i].exist = 0;
		}
	}

	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		if(GLight[i]) {
			GLight[i]->m_ignitionLightHandle = LightHandle();
		}
	}

	TOTPDL = 0;
	TOTIOPDL = 0;
}



static int MAX_LLIGHTS = llightsSize;

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
	if(dist > threshold)
		return;

	float val = dist - el->fallend;

	if(val < 0)
		val=0;

	for(int i = 0; i < MAX_LLIGHTS; i++) {
		if(!llights[i]) {
			llights[i]=el;
			values[i]=val;
			return;
		} else if (val <= values[i]) { // Inserts light at the right place
			for(int j = MAX_LLIGHTS - 2; j >= i; j--) {
				if(llights[j]) {
					llights[j+1]=llights[j];
					values[j+1]=values[j];
				}
			}

			llights[i]=el;
			values[i]=val;
			return;
		}
	}
}

void setMaxLLights(int count) {
	MAX_LLIGHTS = glm::clamp(count, 6, llightsSize);
}

void UpdateLlights(ShaderLight lights[], int & lightsCount, const Vec3f pos, bool forPlayerColor) {
	
	boost::array<EERIE_LIGHT *, llightsSize> llights;
	llights.fill(NULL);
	boost::array<float, llightsSize> values;
	values.fill(999999999.f);
	
	for(size_t i = 0; i < TOTIOPDL; i++) {
		Insertllight(llights, values, IO_PDL[i], pos, forPlayerColor);
	}

	for(size_t i = 0; i < TOTPDL; i++) {
		Insertllight(llights, values, PDL[i], pos, forPlayerColor);
	}
	
	lightsCount = 0;
	for(int i = 0; i < MAX_LLIGHTS; i++) {
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

void ResetTileLights() {
	
	ARX_PROFILE_FUNC();
	
	for(long z = 0; z < ACTIVEBKG->Zsize; z++)
	for(long x = 0; x < ACTIVEBKG->Xsize; x++) {
		tilelights[x][z].el.clear();
	}
}

void ComputeTileLights(short x,short z)
{
	tilelights[x][z].el.clear();
	float xx = (x + 0.5f) * ACTIVEBKG->Xdiv;
	float zz = (z + 0.5f) * ACTIVEBKG->Zdiv;

	for(size_t i = 0; i < TOTPDL; i++) {
		EERIE_LIGHT * light = PDL[i];
		
		if(closerThan(Vec2f(xx, zz), Vec2f(light->pos.x, light->pos.z), light->fallend + 60.f)) {

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

float GetColorz(const Vec3f &pos) {

	ShaderLight lights[llightsSize];
	int lightsCount;
	UpdateLlights(lights, lightsCount, pos, true);
	
	Color3f ff = Color3f(0.f, 0.f, 0.f);
	
	for(long k = 0; k < lightsCount; k++) {
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
		ApplyTileLights(ep, epdata.p);

		for(long i = 0; i < to; i++) {
			Color col = Color::fromRGBA(ep->tv[i].color);
			_ff.r += float(col.r);
			_ff.g += float(col.g);
			_ff.b += float(col.b);
		}

		_ff.r *= div;
		_ff.g *= div;
		_ff.b *= div;
		float ratio, ratio2;
		ratio = glm::abs(needy - pos.y) * ( 1.0f / 300 );
		ratio = (1.f - ratio);
		ratio2 = 1.f - ratio;
		ff.r = ff.r * ratio2 + _ff.r * ratio;
		ff.g = ff.g * ratio2 + _ff.g * ratio;
		ff.b = ff.b * ratio2 + _ff.b * ratio;
	}

	return (std::min(ff.r, 255.f) + std::min(ff.g, 255.f) + std::min(ff.b, 255.f)) * (1.f/3);
}

ColorRGBA ApplyLight(const ShaderLight lights[],
                     const int lightsCount,
                     const glm::quat & quat,
                     const Vec3f & position,
                     const Vec3f & normal,
                     const ColorMod & colorMod,
                     float materialDiffuse
) {
	Color3f tempColor = colorMod.ambientColor;
	
	glm::quat inv = glm::inverse(quat);
	
	// Dynamic lights
	for(int l = 0; l != lightsCount; l++) {
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

	u8 ir = clipByte255(tempColor.r);
	u8 ig = clipByte255(tempColor.g);
	u8 ib = clipByte255(tempColor.b);

	return Color(ir, ig, ib, 255).toRGBA();
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
			ep->tv[j].color = ep->v[j].color;
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

		u8 ir = clipByte255(tempColor.r);
		u8 ig = clipByte255(tempColor.g);
		u8 ib = clipByte255(tempColor.b);
		ep->tv[j].color = Color(ir, ig, ib, 255).toRGBA();
	}
}


/*!
 * \addtogroup Light Baking
 * \{
 */

void EERIERemovePrecalcLights() {

	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		if(GLight[i] != NULL)
			GLight[i]->treat = 1;
	}
	
	// TODO copy-paste poly iteration
	for(short z = 0; z < ACTIVEBKG->Zsize; z++)
	for(short x = 0; x < ACTIVEBKG->Xsize; x++) {
		EERIE_BKG_INFO & eg = ACTIVEBKG->fastdata[x][z];
		for(short l = 0; l < eg.nbpoly; l++) {
			EERIEPOLY & ep = eg.polydata[l];
			
			ep.v[3].color = ep.v[2].color = ep.v[1].color = ep.v[0].color = Color::white.toRGB();
		}
	}
}

/*! \} */
