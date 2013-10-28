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

#include "core/Application.h"
#include "core/GameTime.h"
#include "core/Core.h"

#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/Inventory.h"

#include "graphics/Math.h"
#include "graphics/Draw.h"
#include "graphics/DrawLine.h"

#include "scene/Object.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"

static const float GLOBAL_LIGHT_FACTOR=0.85f;

EERIE_LIGHT * GLight[MAX_LIGHTS];
EERIE_LIGHT DynLight[MAX_DYNLIGHTS];

EERIE_LIGHT * PDL[MAX_DYNLIGHTS];
long TOTPDL = 0;

EERIE_LIGHT * IO_PDL[MAX_DYNLIGHTS];
long TOTIOPDL = 0;

void ColorMod::updateFromEntity(Entity *io, bool inBook) {
	factor = Color3f::white;
	term = Color3f::black;
	if(io) {
	   factor *= io->special_color;
	   term += io->highlightColor;
	}

	if(Project.improve) {
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
	ambientColor = ACTIVEBKG->ambient255;
	if(io && (io->ioflags & (IO_NPC | IO_ITEM)))
	   ambientColor = Color3f::gray(NPC_ITEMS_AMBIENT_VALUE_255);
}

void RecalcLight(EERIE_LIGHT * el) {
	el->rgb255 = el->rgb * 255.f;
	el->falldiff = el->fallend - el->fallstart;
	el->falldiffmul = 1.f / el->falldiff;
	el->precalc = el->intensity * GLOBAL_LIGHT_FACTOR;
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
			if(GLight[i]->tl > 0) {
				DynLight[GLight[i]->tl].exist = 0;
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
			GLight[i]->tl = -1;
			return i;
		}
	}
	
	return -1;
}


long EERIE_LIGHT_Count() {
	
	long count = 0;
	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		if(GLight[i] && !(GLight[i]->type & TYP_SPECIAL1)) {
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
		GLight[num]->tl = -1;
		GLight[num]->sample = audio::INVALID_ID;
	}
}

void EERIE_LIGHT_MoveAll(const Vec3f * trans) {
	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		if(GLight[i]) {
			GLight[i]->pos += *trans;
		}
	}
}

void ComputeLight2DPos(EERIE_LIGHT * _pL) {
	
	TexturedVertex in, out;
	in.p = _pL->pos;
	EE_RTP(&in, &out);
	
	if(out.p.z > 0.f && out.p.z < 1000.f && out.rhw > 0) {
		float siz = 50;
		float fMaxdist = 300;

		if(Project.telekinesis)
			fMaxdist = 850;

		float t = siz * (1.0f - 1.0f / (out.rhw * fMaxdist)) + 10;

		_pL->maxs.x = out.p.x + t;
		_pL->mins.x = out.p.x - t;
		_pL->maxs.y = out.p.y + t;
		_pL->mins.y = out.p.y - t;

		if (0)
			if ((_pL->mins.x >= -200.f) && (_pL->mins.x <= 1000.f))
				if ((_pL->mins.y >= -200.f) && (_pL->mins.y <= 1000.f))
				{

					EERIEDraw2DLine(_pL->mins.x, _pL->mins.y, _pL->maxs.x, _pL->mins.y, 0.00001f, Color::white);
					EERIEDraw2DLine(_pL->maxs.x, _pL->mins.y, _pL->maxs.x, _pL->maxs.y, 0.00001f, Color::white);
					EERIEDraw2DLine(_pL->maxs.x, _pL->maxs.y, _pL->mins.x, _pL->maxs.y, 0.00001f, Color::white);
					EERIEDraw2DLine(_pL->mins.x, _pL->maxs.y, _pL->mins.x, _pL->mins.y, 0.00001f, Color::white);
				}
	}
}

void TreatBackgroundDynlights()
{
	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		EERIE_LIGHT *light = GLight[i];

		if(light && (light->extras & EXTRAS_SEMIDYNAMIC)) {

			float fMaxdist = 300;
			if(Project.telekinesis)
				fMaxdist = 850;

			if(!fartherThan(light->pos, ACTIVECAM->orgTrans.pos, fMaxdist)) {
				ComputeLight2DPos(light);
			}

			if(light->status == 0) {
				// just extinguished
				if(light->tl > 0) {
					DynLight[light->tl].exist = 0;
					light->tl = -1;
					Vec3f _pos2;

					for(size_t l = 0; l < entities.size(); l++) {
						if(entities[l] && (entities[l]->ioflags & IO_MARKER)) {
							GetItemWorldPosition(entities[l], &_pos2);
							if(!fartherThan(light->pos, _pos2, 300.f)) {
								SendIOScriptEvent(entities[l], SM_CUSTOM, "douse");
							}
						}
					}
				}
			} else {
				// just light up
				if(light->tl <= 0) {
					Vec3f _pos2;

					for(size_t l = 0; l < entities.size(); l++) {
						if(entities[l] && (entities[l]->ioflags & IO_MARKER)) {
							GetItemWorldPosition(entities[l], &_pos2);
							if(!fartherThan(light->pos, _pos2, 300.f)) {
								SendIOScriptEvent(entities[l], SM_CUSTOM, "fire");
							}
						}
					}

					light->tl = GetFreeDynLight();
				}
				
				long n = light->tl;
				if(n != -1) {
					EERIE_LIGHT *dynamicLight = &DynLight[n];

					dynamicLight->pos = light->pos;
					dynamicLight->exist		=	1;
					dynamicLight->fallstart	=	light->fallstart;
					dynamicLight->fallend		=	light->fallend;
					dynamicLight->type		=	TYP_SPECIAL1;
					dynamicLight->intensity	=	light->intensity;
					dynamicLight->ex_flaresize =	light->ex_flaresize;
					dynamicLight->extras		=	light->extras;
					dynamicLight->duration = std::numeric_limits<long>::max();

					dynamicLight->rgb.r = light->rgb.r - light->rgb.r * light->ex_flicker.r * rnd() * ( 1.0f / 2 );
					dynamicLight->rgb.g = light->rgb.g - light->rgb.g * light->ex_flicker.g * rnd() * ( 1.0f / 2 );
					dynamicLight->rgb.b = light->rgb.b - light->rgb.b * light->ex_flicker.b * rnd() * ( 1.0f / 2 );
					
					dynamicLight->rgb = componentwise_max(dynamicLight->rgb, Color3f::black);
					RecalcLight(dynamicLight);
				}
			}
		}
	}

	for(size_t i = 0; i < MAX_DYNLIGHTS; i++) {
		EERIE_LIGHT * el = &DynLight[i];

		if(el->exist && el->duration) {
			float tim = (float)float(arxtime) - (float)el->time_creation;
			float duration = (float)el->duration;

			if(tim >= duration) {
				float sub = framedelay * 0.001f;

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
					el->duration = 0;
				}
			}
		}
	}
}

void PrecalcDynamicLighting(long x0, long z0, long x1, long z1) {

	TOTPDL = 0;
	
	float fx0 = ACTIVEBKG->Xdiv * (float)x0;
	float fz0 = ACTIVEBKG->Zdiv * (float)z0;
	float fx1 = ACTIVEBKG->Xdiv * (float)x1;
	float fz1 = ACTIVEBKG->Zdiv * (float)z1;

	for(size_t i = 0; i < MAX_DYNLIGHTS; i++) {
		EERIE_LIGHT * el = &DynLight[i];

		if(el->exist && el->rgb.r >= 0.f) {
			if ((el->pos.x >= fx0) && (el->pos.x <= fx1)
			        && (el->pos.z >= fz0) && (el->pos.z <= fz1)
			        && closerThan(el->pos, ACTIVECAM->orgTrans.pos, ACTIVECAM->cdepth))
			{
				el->treat = 1;
				RecalcLight(el);
				PDL[TOTPDL] = el;
				TOTPDL++;

				if((size_t)TOTPDL >= MAX_DYNLIGHTS)
					TOTPDL--;
			}
			else if(el->treat)
				el->treat = 0;
		}
	}
}

void PrecalcIOLighting(const Vec3f * pos, float radius) {

	TOTIOPDL = 0;

	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		EERIE_LIGHT * el = GLight[i];

		if(el && el->exist && el->status && !(el->extras & EXTRAS_SEMIDYNAMIC)) {
			if((el->pos.x >= pos->x - radius) && (el->pos.x <= pos->x + radius)
					&& (el->pos.z >= pos->z - radius) && (el->pos.z <= pos->z + radius))
			{
				RecalcLight(el);
				IO_PDL[TOTIOPDL] = el;

				TOTIOPDL++;

				if((size_t)TOTIOPDL >= MAX_DYNLIGHTS)
					TOTIOPDL--;
			}
		}
	}
}

bool ValidDynLight(long num)
{
	return num >= 0 && ((size_t)num < MAX_DYNLIGHTS) && DynLight[num].exist;
}

long GetFreeDynLight() {

	for(size_t i = 1; i < MAX_DYNLIGHTS; i++) {
		if(!(DynLight[i].exist)) {
			DynLight[i].type = 0;
			DynLight[i].intensity = 1.3f;
			DynLight[i].treat = 1;
			DynLight[i].time_creation = (unsigned long)(arxtime);
			DynLight[i].duration = 0;
			DynLight[i].extras = 0;
			return i;
		}
	}

	return -1;
}

void ClearDynLights() {

	for(size_t i = 0; i < MAX_DYNLIGHTS; i++) {
		if(DynLight[i].exist) {
			DynLight[i].exist = 0;
		}
	}

	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		if(GLight[i] && GLight[i]->tl > 0) {
			GLight[i]->tl = 0;
		}
	}

	TOTPDL = 0;
	TOTIOPDL = 0;
}

long MAX_LLIGHTS = 18;
EERIE_LIGHT * llights[32];
float dists[32];
float values[32];

void llightsInit() {
	for(long i = 0; i < MAX_LLIGHTS; i++) {
		llights[i] = NULL;
		dists[i] = 999999999.f;
		values[i] = 999999999.f;
	}
}

// Inserts Light in the List of Nearest Lights
void Insertllight(EERIE_LIGHT * el,float dist)
{
	if(!el)
		return;

	float threshold = el->fallend + 560.f;
	if(dist > threshold)
		return;

	float val = dist - el->fallend;

	if(val < 0)
		val=0;

	for(long i=0; i < MAX_LLIGHTS; i++) {
		if(!llights[i]) {
			llights[i]=el;
			dists[i]=dist;
			values[i]=val;
			return;
		} else if (val <= values[i]) { // Inserts light at the right place
			for(long j = MAX_LLIGHTS - 2; j >= i; j--) {
				if(llights[j]) {
					llights[j+1]=llights[j];
					dists[j+1]=dists[j];
					values[j+1]=values[j];
				}
			}

			llights[i]=el;
			dists[i]=dist;
			values[i]=val;
			return;
		}
	}
}

void UpdateLlights(Vec3f & tv) {
	llightsInit();

	for(int i = 0; i < TOTIOPDL; i++) {
		Insertllight(IO_PDL[i], glm::distance(IO_PDL[i]->pos, tv));
	}

	for(int i = 0; i < TOTPDL; i++) {
		Insertllight(PDL[i], glm::distance(PDL[i]->pos, tv));
	}
}

struct TILE_LIGHTS
{
	std::vector<EERIE_LIGHT *> el;
};

TILE_LIGHTS tilelights[MAX_BKGX][MAX_BKGZ];

void InitTileLights()
{
	for (long j=0;j<MAX_BKGZ;j++)
	for (long i=0;i<MAX_BKGZ;i++)
	{
		tilelights[i][j].el.resize(0);
	}
}

void ResetTileLights() {
	for(long j=0; j<ACTIVEBKG->Zsize; j++) {
		for(long i=0; i<ACTIVEBKG->Xsize; i++) {
			tilelights[i][j].el.clear();
		}
	}
}

void ComputeTileLights(short x,short z)
{
	tilelights[x][z].el.clear();
	float xx=((float)x+0.5f)*ACTIVEBKG->Xdiv;
	float zz=((float)z+0.5f)*ACTIVEBKG->Zdiv;

	for(long i=0; i < TOTPDL; i++) {
		if(closerThan(Vec2f(xx, zz), Vec2f(PDL[i]->pos.x, PDL[i]->pos.z), PDL[i]->fallend + 60.f)) {

			tilelights[x][z].el.push_back(PDL[i]);
		}
	}
}

void ClearTileLights() {
	for(long j = 0; j < MAX_BKGZ; j++) {
		for(long i = 0; i < MAX_BKGZ; i++) {
			tilelights[i][j].el.resize(0);
		}
	}
}

float GetColorz(const Vec3f &pos) {

	llightsInit();

	for(long i = 0; i < TOTIOPDL; i++) {
		if(IO_PDL[i]->fallstart > 10.f && IO_PDL[i]->fallend > 100.f)
			Insertllight(IO_PDL[i], fdist(IO_PDL[i]->pos, pos) - IO_PDL[i]->fallstart);
	}

	for(int i = 0; i < TOTPDL; i++) {
		if(PDL[i]->fallstart > 10.f && PDL[i]->fallend > 100.f)
			Insertllight(PDL[i], fdist(PDL[i]->pos, pos) - PDL[i]->fallstart);
	}

	float ffr = 0;
	float ffg = 0;
	float ffb = 0;

	for(long k = 0; k < MAX_LLIGHTS; k++) {
		EERIE_LIGHT * el = llights[k];

		if(el) {
			float dd = fdist(el->pos, pos);

			if(dd < el->fallend) {
				float dc;

				if(dd <= el->fallstart) {
					dc = el->intensity * GLOBAL_LIGHT_FACTOR;
				} else {
					float p = ((el->fallend - dd) * el->falldiffmul);

					if(p <= 0.f)
						dc = 0.f;
					else
						dc = p * el->intensity * GLOBAL_LIGHT_FACTOR;
				}

				dc *= 0.4f * 255.f;
				ffr = std::max(ffr, el->rgb.r * dc);
				ffg = std::max(ffg, el->rgb.g * dc);
				ffb = std::max(ffb, el->rgb.b * dc);
			}
		}
	}


	EERIEPOLY * ep;
	float needy;
	ep = EECheckInPoly(&pos, &needy);

	if(ep != NULL) {
		float _ffr = 0;
		float _ffg = 0;
		float _ffb = 0;

		long to = (ep->type & POLY_QUAD) ? 4 : 3;
		float div = (1.0f / to);

		EP_DATA & epdata = portals->room[ep->room].epdata[0];
		ApplyTileLights(ep, epdata.px, epdata.py);

		for(long i = 0; i < to; i++) {
			Color col = Color::fromBGR(ep->tv[i].color);
			_ffr += float(col.r);
			_ffg += float(col.g);
			_ffb += float(col.b);
		}

		_ffr *= div;
		_ffg *= div;
		_ffb *= div;
		float ratio, ratio2;
		ratio = EEfabs(needy - pos.y) * ( 1.0f / 300 );
		ratio = (1.f - ratio);
		ratio2 = 1.f - ratio;
		ffr = ffr * ratio2 + _ffr * ratio;
		ffg = ffg * ratio2 + _ffg * ratio;
		ffb = ffb * ratio2 + _ffb * ratio;
	}

	return (std::min(ffr, 255.f) + std::min(ffg, 255.f) + std::min(ffb, 255.f)) * (1.f/3);
}

ColorBGRA ApplyLight(const EERIE_QUAT * quat, const Vec3f & position, const Vec3f & normal, const ColorMod & colorMod, float materialDiffuse) {

	Color3f tempColor = colorMod.ambientColor;

	// Dynamic lights
	for(int l = 0; l != MAX_LLIGHTS; l++) {
		EERIE_LIGHT * light = llights[l];

		if(!light)
			break;

		Vec3f vLight = glm::normalize(light->pos - position);

		Vec3f Cur_vLights;
		TransformInverseVertexQuat(quat, &vLight, &Cur_vLights);

		float cosangle = glm::dot(normal, Cur_vLights);

		// If light visible
		if(cosangle > 0.f) {
			float distance = fdist(position, light->pos);

			// Evaluate its intensity depending on the distance Light<->Object
			if(distance <= light->fallstart) {
				cosangle *= light->precalc;
			} else {
				float p = ((light->fallend - distance) * light->falldiffmul);

				if(p <= 0.f)
					cosangle = 0.f;
				else
					cosangle *= p * light->precalc;
			}

			cosangle *= materialDiffuse;

			tempColor += light->rgb255 * cosangle;
		}
	}

	tempColor *= colorMod.factor;
	tempColor += colorMod.term;

	u8 ir = clipByte255(tempColor.r);
	u8 ig = clipByte255(tempColor.g);
	u8 ib = clipByte255(tempColor.b);

	return (0xFF000000L | (ir << 16) | (ig << 8) | (ib));
}

void ApplyTileLights(EERIEPOLY * ep, short x, short y)
{

	Color3f lightInfraFactor = Color3f::white;
	if(Project.improve) {
		lightInfraFactor.r = 4.f;
	}

	TILE_LIGHTS * tls = &tilelights[x][y];
	size_t nbvert = (ep->type & POLY_QUAD) ? 4 : 3;

	for(size_t j = 0; j < nbvert; j++) {

		if(tls->el.size() == 0) {
			ep->tv[j].color = ep->v[j].color;
			continue;
		}

		Color3f tempColor;
		long c = ep->v[j].color;
		tempColor.r = (float)((c >> 16) & 255);
		tempColor.g = (float)((c >> 8) & 255);
		tempColor.b = (float)(c & 255);

		Vec3f & position = ep->v[j].p;
		Vec3f & normal = ep->nrml[j];

		for(size_t i = 0; i < tls->el.size(); i++) {
			EERIE_LIGHT * light = tls->el[i];

			Vec3f vLight = glm::normalize(light->pos - position);

			float cosangle = glm::dot(normal, vLight);

			if(cosangle > 0.f) {
				float distance = fdist(light->pos, position);

				if(distance <= light->fallstart) {
					cosangle *= light->precalc;
				} else {
					float p = ((light->fallend - distance) * light->falldiffmul);

					if(p <= 0.f)
						cosangle = 0.f;
					else
						cosangle *= p * light->precalc;
				}
				cosangle *= 0.5f;

				tempColor += light->rgb255 * lightInfraFactor * cosangle;
			}
		}

		u8 ir = clipByte255(tempColor.r);
		u8 ig = clipByte255(tempColor.g);
		u8 ib = clipByte255(tempColor.b);
		ep->tv[j].color = (0xFF000000L | (ir << 16) | (ig << 8) | (ib));
	}
}


/**
 * @addtogroup Light Baking
 * @{
 */

float my_CheckInPoly(float x, float y, float z, EERIEPOLY * mon_ep, EERIE_LIGHT * light)
{
	long px = x * ACTIVEBKG->Xmul;
	long pz = z * ACTIVEBKG->Zmul;

	if(px < 2 || px > ACTIVEBKG->Xsize - 3 || pz < 2 || pz > ACTIVEBKG->Zsize - 3)
		return 0;

	float nb_shadowvertexinpoly = 0.0f;
	float nb_totalvertexinpoly = 0.0f;

	EERIEPOLY * ep;
	EERIE_BKG_INFO * eg;

	Vec3f dest;
	Vec3f hit;

	Vec3f orgn = light->pos;

	for (long j = pz - 2; j <= pz + 2; j++)
		for (long i = px - 2; i <= px + 2; i++)
		{
			eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

			for (long k = 0; k < eg->nbpoly; k++)
			{
				ep = &eg->polydata[k];

				if (!(ep->type & POLY_WATER) &&  !(ep->type & POLY_TRANS))
				{

					long nbvert = (ep->type & POLY_QUAD) ? 4 : 3;

					long a, b;

					for (a = 0; a < nbvert; a++)
					{
						float fDiff = 5.f;

						if ((fabs(ep->v[a].p.x - x) <= fDiff) &&
								(fabs(ep->v[a].p.y - y) <= fDiff) &&
								(fabs(ep->v[a].p.z - z) <= fDiff))
						{

							if(glm::dot(*mon_ep->nrml, *ep->nrml) > 0.0f) {
								nb_totalvertexinpoly += nbvert;
								for(b = 0; b < nbvert; b++) {
									dest = ep->v[b].p;
									if(Visible(&orgn, &dest, ep, &hit)) {
										nb_shadowvertexinpoly ++;
									}
								}
							}
						}
					}
				}
			}
		}

	return nb_shadowvertexinpoly / nb_totalvertexinpoly;
}

static void ARX_EERIE_LIGHT_Make(EERIEPOLY * ep, float * epr, float * epg, float * epb, EERIE_LIGHT * light)
{
	float	distance[4];	// distance from light to each vertex

	if (ep->type & POLY_IGNORE)
		return;

	// number or vertices per face (3 or 4)
	int nbvert = (ep->type & POLY_QUAD) ? 4 : 3;

	// compute light - vertex distance
	for(int i = 0; i < nbvert; i++) {
		distance[i] = glm::distance(light->pos, ep->v[i].p);
	}

	for(int i = 0; i < nbvert; i++) {
		// value of light intensity for a given vertex
		float fRes = 1.0f;

		if(distance[i] < light->fallend) {
			//MODE_NORMALS
			if(ModeLight & MODE_NORMALS) {
				Vec3f vLight = glm::normalize(light->pos - ep->v[i].p); // vector (light to vertex)

				fRes = glm::dot(vLight, ep->nrml[i]);

				if(fRes < 0.0f)
					fRes = 0.0f;
			}

			//MODE_RAYLAUNCH
			if((ModeLight & MODE_RAYLAUNCH) && !(light->extras & EXTRAS_NOCASTED)) {
				Vec3f orgn = light->pos, dest = ep->v[i].p, hit;

				if(ModeLight & MODE_SMOOTH)
					fRes *= my_CheckInPoly(ep->v[i].p.x, ep->v[i].p.y, ep->v[i].p.z, ep, light);
				else
					fRes *= Visible(&orgn, &dest, ep, &hit);
			}

			float fTemp1 = light->intensity * fRes * GLOBAL_LIGHT_FACTOR;

			if(distance[i] > light->fallstart) {
				float intensity = (light->falldiff - (distance[i] - light->fallstart)) * light->falldiffmul;
				fTemp1 *= intensity;
			}

			float fr = light->rgb.r * fTemp1;
			float fg = light->rgb.g * fTemp1;
			float fb = light->rgb.b * fTemp1;

			epr[i] += fr;
			epg[i] += fg;
			epb[i] += fb;
		}
	}
}

void EERIE_LIGHT_Apply(EERIEPOLY * ep) {

	if(ep->type & POLY_IGNORE)
		return;

	float epr[4];
	float epg[4];
	float epb[4];

	epr[3] = epr[2] = epr[1] = epr[0] = 0;
	epg[3] = epg[2] = epg[1] = epg[0] = 0;
	epb[3] = epb[2] = epb[1] = epb[0] = 0;

	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		EERIE_LIGHT * el = GLight[i];

		if(el && el->treat && el->exist && el->status && !(el->extras & EXTRAS_SEMIDYNAMIC)) {
			if(closerThan(el->pos, ep->center, el->fallend + 100.f)) {
				ARX_EERIE_LIGHT_Make(ep, epr, epg, epb, el);
			}
		}
	}

	long nbvert = (ep->type & POLY_QUAD) ? 4 : 3;

	for(long i = 0; i < nbvert; i++) {
		epr[i] = clamp(epr[i], ACTIVEBKG->ambient.r, 1.f);
		epg[i] = clamp(epg[i], ACTIVEBKG->ambient.g, 1.f);
		epb[i] = clamp(epb[i], ACTIVEBKG->ambient.b, 1.f);
		ep->v[i].color = Color3f(epr[i], epg[i], epb[i]).toBGR();
	}
}

void EERIEPrecalcLights(long minx, long minz, long maxx, long maxz)
{
	minx = clamp(minx, 0, ACTIVEBKG->Xsize - 1);
	maxx = clamp(maxx, 0, ACTIVEBKG->Xsize - 1);
	minz = clamp(minz, 0, ACTIVEBKG->Zsize - 1);
	maxz = clamp(maxz, 0, ACTIVEBKG->Zsize - 1);

	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		if(GLight[i]) {
			if((GLight[i]->extras & EXTRAS_SEMIDYNAMIC)) {
				GLight[i]->treat = 0;
			} else if (!GLight[i]->treat) {
				GLight[i]->treat = 1;
			}
			RecalcLight(GLight[i]);
		}
	}

	for(long j = minz; j <= maxz; j++) {
		for(long i = minx; i <= maxx; i++) {
			EERIE_BKG_INFO *eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

			for(long k = 0; k < eg->nbpoly; k++) {
				EERIEPOLY * ep = &eg->polydata[k];

				if(ep) {
					ep->type &= ~POLY_IGNORE;
					EERIE_LIGHT_Apply(ep);
				}
			}
		}
	}
}

void RecalcLightZone(float x, float z, long siz) {

	long px = x * ACTIVEBKG->Xmul;
	long pz = z * ACTIVEBKG->Zmul;

	long x0 = std::max(px - siz, 0L);
	long x1 = std::min(px + siz, ACTIVEBKG->Xsize - 1L);
	long z0 = std::max(pz - siz, 0L);
	long z1 = std::min(pz + siz, ACTIVEBKG->Zsize - 1L);

	LightMode oldml = ModeLight;
	ModeLight &= ~MODE_RAYLAUNCH;
	EERIEPrecalcLights(x0, z0, x1, z1);
	ModeLight = oldml;
}

void EERIERemovePrecalcLights() {

	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		if(GLight[i] != NULL)
			GLight[i]->treat = 1;
	}

	for(int j = 0; j < ACTIVEBKG->Zsize; j++) {
		for(int i = 0; i < ACTIVEBKG->Xsize; i++) {
			EERIE_BKG_INFO *eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

			for(long k = 0; k < eg->nbpoly; k++) {
				EERIEPOLY *ep = &eg->polydata[k];
				ep->v[3].color = ep->v[2].color = ep->v[1].color = ep->v[0].color = Color::white.toBGR();
			}
		}
	}
}

/** @}*/
