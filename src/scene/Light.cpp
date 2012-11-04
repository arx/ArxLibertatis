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
#include "scene/Object.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"

extern float GLOBAL_LIGHT_FACTOR;
EERIE_LIGHT * GLight[MAX_LIGHTS];
EERIE_LIGHT DynLight[MAX_DYNLIGHTS];

EERIE_LIGHT * PDL[MAX_DYNLIGHTS];
long TOTPDL = 0;

EERIE_LIGHT * IO_PDL[MAX_DYNLIGHTS];
long TOTIOPDL = 0;

static void ARX_EERIE_LIGHT_Make(EERIEPOLY * ep, float * epr, float * epg, float * epb, EERIE_LIGHT * light);

bool ValidDynLight(long num)
{
	if (	(num >= 0)
        &&	((size_t)num < MAX_DYNLIGHTS)
        &&	DynLight[num].exist)
		return true;

	return false;
}

void PrecalcIOLighting(const Vec3f * pos, float radius, long flags) {
	
	static Vec3f lastpos;
	if(flags & 1) {
		lastpos = Vec3f::repeat(99999.f) + lastpos;
		return;
	}
	
	// Lastpos optim
	if(closerThan(*pos, lastpos, 100.f)) {
		return;
	}
	
	lastpos = *pos;
	
	TOTIOPDL = 0;
	
	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		
		EERIE_LIGHT * el = GLight[i];

		if ((el)  && (el->exist) && (el->status)
		        && !(el->extras & EXTRAS_SEMIDYNAMIC))
		{
			if ((el->pos.x >= pos->x - radius) && (el->pos.x <= pos->x + radius)
			        && (el->pos.z >= pos->z - radius) && (el->pos.z <= pos->z + radius))
			{
				
				el->rgb255 = el->rgb * 255.f;
				el->falldiff = el->fallend - el->fallstart;
				el->falldiffmul = 1.f / el->falldiff;
				el->precalc = el->intensity * GLOBAL_LIGHT_FACTOR;
				IO_PDL[TOTIOPDL] = el;
			
				TOTIOPDL++;

				if ((size_t)TOTIOPDL >= MAX_DYNLIGHTS) TOTIOPDL--;
			}
		}
	}
}

void EERIE_LIGHT_Apply(EERIEPOLY * ep) {
	
	if (ep->type & POLY_IGNORE)  return;

	float epr[4];
	float epg[4];
	float epb[4];

	epr[3] = epr[2] = epr[1] = epr[0] = 0; 
	epg[3] = epg[2] = epg[1] = epg[0] = 0; 
	epb[3] = epb[2] = epb[1] = epb[0] = 0; 

	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		
		EERIE_LIGHT * el = GLight[i];

		if ((el) && (el->treat) && (el->exist) && (el->status) 
		        && !(el->extras & EXTRAS_SEMIDYNAMIC))
		{
			if(closerThan(el->pos, ep->center, el->fallend + 100.f)) {
				ARX_EERIE_LIGHT_Make(ep, epr, epg, epb, el);
			}
		}
	}

	for (size_t i = 0; i < MAX_ACTIONS; i++)
	{
		if ((actions[i].exist) && ((actions[i].type == ACT_FIRE2) || (actions[i].type == ACT_FIRE)))
		{
			if(closerThan(actions[i].light.pos, ep->center, actions[i].light.fallend + 100.f)) {
				ARX_EERIE_LIGHT_Make(ep, epr, epg, epb, &actions[i].light);
			}
		}
	}

	long nbvert;

	if (ep->type & POLY_QUAD) nbvert = 4;
	else nbvert = 3;

	for(long i = 0; i < nbvert; i++) {
		epr[i] = clamp(epr[i], ACTIVEBKG->ambient.r, 1.f);
		epg[i] = clamp(epg[i], ACTIVEBKG->ambient.g, 1.f);
		epb[i] = clamp(epb[i], ACTIVEBKG->ambient.b, 1.f);
		ep->v[i].color = Color3f(epr[i], epg[i], epb[i]).toBGR();
	}
}

void EERIE_LIGHT_TranslateSelected(const Vec3f * trans) {
	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		if(GLight[i] && GLight[i]->selected) {
			if(GLight[i]->tl > 0) {
				DynLight[GLight[i]->tl].exist = 0;
			}
			GLight[i]->tl = -1;
			GLight[i]->pos += *trans;
		}
	}
}

void EERIE_LIGHT_UnselectAll() {
	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		if(GLight[i] && GLight[i]->exist && GLight[i]->treat) {
			GLight[i]->selected = 0;
		}
	}
}

void EERIE_LIGHT_ClearByIndex(long num)
{
	if ((num >= 0) && ((size_t)num < MAX_LIGHTS))
	{
		if (GLight[num] != NULL)
		{
			if (GLight[num]->tl != -1) DynLight[GLight[num]->tl].exist = 0;

			free(GLight[num]);
			GLight[num] = NULL;
		}
	}
}

void EERIE_LIGHT_ClearAll() {
	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		EERIE_LIGHT_ClearByIndex(i);
	}
}

void EERIE_LIGHT_ClearSelected() {
	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		if(GLight[i] && GLight[i]->selected) {
			EERIE_LIGHT_ClearByIndex(i);
		}
	}
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

//*************************************************************************************
//*************************************************************************************
float my_CheckInPoly(float x, float y, float z, EERIEPOLY * mon_ep, EERIE_LIGHT * light)
{
	long px, pz;
	px = x * ACTIVEBKG->Xmul;


	if (px > ACTIVEBKG->Xsize - 3)
	{
		return 0;
	}

	if (px < 2)
	{
		return 0;
	}

	pz = z * ACTIVEBKG->Zmul;

	if (pz > ACTIVEBKG->Zsize - 3)
	{
		return 0;
	}

	if (pz < 2)
	{
		return 0;
	}

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

					long nbvert;
					(ep->type & POLY_QUAD) ? nbvert = 4 : nbvert = 3;

					long a, b;

					for (a = 0; a < nbvert; a++)
					{
						float fDiff = 5.f;

						if ((fabs(ep->v[a].p.x - x) <= fDiff) &&
								(fabs(ep->v[a].p.y - y) <= fDiff) &&
								(fabs(ep->v[a].p.z - z) <= fDiff))
						{

							if(dot(*mon_ep->nrml, *ep->nrml) > 0.0f) {
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
	int		i;				// iterator
	int		nbvert;			// number or vertices per face (3 or 4)
	float	distance[4];	// distance from light to each vertex
	float	fRes;			// value of light intensity for a given vertex

	if (ep->type & POLY_IGNORE)
		return;

	(ep->type & POLY_QUAD) ? nbvert = 4 : nbvert = 3;

	// compute light - vertex distance
	for(i = 0; i < nbvert; i++) {
		distance[i] = dist(light->pos, ep->v[i].p);
	}

	for (i = 0; i < nbvert; i++)
	{
		fRes = 1.0f;

		if (distance[i] < light->fallend)
		{
			//---------------------- start MODE_NORMALS
			if (ModeLight & MODE_NORMALS)
			{
				Vec3f vLight = (light->pos - ep->v[i].p).getNormalized(); // vector (light to vertex)

				fRes = dot(vLight, ep->nrml[i]);

				if (fRes < 0.0f)
				{
					fRes = 0.0f;
				}
			}

			//---------------------- end MODE_NORMALS

			//---------------------- start MODE_RAYLAUNCH
			if ((ModeLight & MODE_RAYLAUNCH) && !(light->extras & EXTRAS_NOCASTED))
			{
				Vec3f orgn = light->pos, dest = ep->v[i].p, hit;

				if (ModeLight & MODE_SMOOTH)
					fRes *= my_CheckInPoly(ep->v[i].p.x, ep->v[i].p.y, ep->v[i].p.z, ep, light);
				else
					fRes *= Visible(&orgn, &dest, ep, &hit);
			}

			//---------------------- fin MODE_RAYLAUNCH

			float fTemp1 = light->intensity * fRes * GLOBAL_LIGHT_FACTOR;
			float fr, fg, fb;

			if (distance[i] <= light->fallstart)
			{
				fr = light->rgb.r * fTemp1;
				fg = light->rgb.g * fTemp1;
				fb = light->rgb.b * fTemp1;
			}
			else
			{
				float intensity = (light->falldiff - (distance[i] - light->fallstart)) * light->falldiffmul;
				float fTemp2 = fTemp1 * intensity;
				fr = light->rgb.r * fTemp2;
				fg = light->rgb.g * fTemp2;
				fb = light->rgb.b * fTemp2;
			}

			epr[i] += fr; 
			epg[i] += fg; 
			epb[i] += fb; 
		}
	}
}

void ComputeLight2DPos(EERIE_LIGHT * _pL) {
	
	TexturedVertex in, out;
	in.p = _pL->pos;
	EERIETreatPoint(&in, &out);
	
	if ((out.p.z > 0.f) && (out.p.z < 1000.f) && (out.rhw > 0))
	{
		float t;
		float siz = 50;
		float fMaxdist = 300;

		if (Project.telekinesis) fMaxdist = 850;

		t = siz * (1.0f - 1.0f / (out.rhw * fMaxdist)) + 10;

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

//*************************************************************************************
//*************************************************************************************
void TreatBackgroundDynlights()
{
	long n;

	for (size_t i = 0; i < MAX_LIGHTS; i++)
	{
		if ((GLight[i] != NULL) && (GLight[i]->extras & EXTRAS_SEMIDYNAMIC))
		{
			float fMaxdist = 300;

			if (Project.telekinesis) fMaxdist = 850;

			if(!fartherThan(GLight[i]->pos, ACTIVECAM->pos, fMaxdist)) {
				ComputeLight2DPos(GLight[i]);
			}

			if (GLight[i]->status == 0)
			{
				// vient de s'éteindre
				if (GLight[i]->tl > 0)
				{
					DynLight[GLight[i]->tl].exist = 0;
					GLight[i]->tl = -1;
					Vec3f _pos2;

					for(size_t l = 0; l < entities.size(); l++) {
						if(entities[l] && (entities[l]->ioflags & IO_MARKER)) {
							GetItemWorldPosition(entities[l], &_pos2);
							if(!fartherThan(GLight[i]->pos, _pos2, 300.f)) {
								SendIOScriptEvent(entities[l], SM_CUSTOM, "douse");
							}
						}
					}
				}
			}
			else
			{
				// vient de s'allumer
				if (GLight[i]->tl <= 0)
				{
					Vec3f _pos2;

					for(size_t l = 0; l < entities.size(); l++) {
						if(entities[l] && (entities[l]->ioflags & IO_MARKER)) {
							GetItemWorldPosition(entities[l], &_pos2);
							if(!fartherThan(GLight[i]->pos, _pos2, 300.f)) {
								SendIOScriptEvent(entities[l], SM_CUSTOM, "fire");
							}
						}
					}

					GLight[i]->tl = GetFreeDynLight();
				}
				
				n = GLight[i]->tl;
				if(n != -1) {
					DynLight[n].pos = GLight[i]->pos;
					DynLight[n].exist		=	1;
					DynLight[n].fallstart	=	GLight[i]->fallstart;
					DynLight[n].fallend		=	GLight[i]->fallend;
					DynLight[n].type		=	TYP_SPECIAL1;
					DynLight[n].intensity	=	GLight[i]->intensity;
					DynLight[n].ex_flaresize =	GLight[i]->ex_flaresize;
					DynLight[n].extras		=	GLight[i]->extras;
					DynLight[n].duration = std::numeric_limits<long>::max();

					DynLight[n].rgb.r = GLight[i]->rgb.r - GLight[i]->rgb.r * GLight[i]->ex_flicker.r * rnd() * ( 1.0f / 2 ); 
					DynLight[n].rgb.g = GLight[i]->rgb.g - GLight[i]->rgb.g * GLight[i]->ex_flicker.g * rnd() * ( 1.0f / 2 ); 
					DynLight[n].rgb.b = GLight[i]->rgb.b - GLight[i]->rgb.b * GLight[i]->ex_flicker.b * rnd() * ( 1.0f / 2 ); 
					
					DynLight[n].rgb = componentwise_max(DynLight[n].rgb, Color3f::black);
					DynLight[n].rgb255 = DynLight[n].rgb * 255.f;
					DynLight[n].falldiff = DynLight[n].fallend - DynLight[n].fallstart;
					DynLight[n].falldiffmul = 1.f / DynLight[n].falldiff;
					DynLight[n].precalc = DynLight[n].intensity * GLOBAL_LIGHT_FACTOR;
				}
			}
		}

	}
}


void PrecalcDynamicLighting(long x0, long z0, long x1, long z1) {
	
	TreatBackgroundDynlights();
	TOTPDL = 0;
	
	float fx0 = ACTIVEBKG->Xdiv * (float)x0;
	float fz0 = ACTIVEBKG->Zdiv * (float)z0;
	float fx1 = ACTIVEBKG->Xdiv * (float)x1;
	float fz1 = ACTIVEBKG->Zdiv * (float)z1;

	for (size_t i = 0; i < MAX_DYNLIGHTS; i++)
	{
		EERIE_LIGHT * el = &DynLight[i];

		if ((el->exist) && (el->rgb.r >= 0.f))
		{

			bool bDist = (distSqr(el->pos, ACTIVECAM->pos) < square(ACTIVECAM->cdepth));

			if ((el->pos.x >= fx0) && (el->pos.x <= fx1)
			        && (el->pos.z >= fz0) && (el->pos.z <= fz1)
			        && bDist)
			{
				el->treat = 1;
				el->rgb255 = el->rgb * 255.f;
				el->falldiff = el->fallend - el->fallstart;
				el->falldiffmul = 1.f / el->falldiff;
				el->precalc = el->intensity * GLOBAL_LIGHT_FACTOR;
				PDL[TOTPDL] = el;
				TOTPDL++;

				if ((size_t)TOTPDL >= MAX_DYNLIGHTS) TOTPDL--;
			}
			else if (el->treat) el->treat = 0;

			if (el->duration)
			{
				float tim = ((float)float(arxtime) - (float)el->time_creation);
				float duration = (float)el->duration;

				if (tim >= duration)
				{


					float sub = framedelay * 0.001f;
					el->rgb.r -= sub;

					if (el->rgb.r < 0) el->rgb.r = 0.f;

					el->rgb.g -= sub;

					if (el->rgb.g < 0) el->rgb.g = 0.f;

					el->rgb.b -= sub;

					if (el->rgb.b < 0) el->rgb.b = 0.f;

					if (el->rgb.r + el->rgb.g + el->rgb.b == 0)
					{
						el->exist = 0;
						el->duration = 0;
					}
				}
			}
		}
	}
}

void EERIE_LIGHT_ChangeLighting()
{
	long i, j;
	EERIEPOLY * ep;
	EERIE_BKG_INFO * eg;

	for (j = 0; j < ACTIVEBKG->Zsize; j++)
		for (i = 0; i < ACTIVEBKG->Xsize; i++)
		{
			eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

			for (long k = 0; k < eg->nbpoly; k++)
			{
				ep = &eg->polydata[k];
				ep->tv[0].color = ep->v[0].color;
				ep->tv[1].color = ep->v[1].color;
				ep->tv[2].color = ep->v[2].color;

				if (ep->type & POLY_QUAD) ep->tv[3].color = ep->v[3].color;
			}
		}
}

//*************************************************************************************
//*************************************************************************************

void EERIEPrecalcLights(long minx, long minz, long maxx, long maxz)
{
	EERIE_BKG_INFO * eg;
 
	if (minx < 0) minx = 0;
	else if (minx >= ACTIVEBKG->Xsize) minx = ACTIVEBKG->Xsize - 1;

	if (maxx < 0) maxx = 0;
	else if (maxx >= ACTIVEBKG->Xsize) maxx = ACTIVEBKG->Xsize - 1;

	if (minz < 0) minz = 0;
	else if (minz >= ACTIVEBKG->Zsize) minz = ACTIVEBKG->Zsize - 1;

	if (maxz < 0) maxz = 0;
	else if (maxz >= ACTIVEBKG->Zsize) maxz = ACTIVEBKG->Zsize - 1;

	for (size_t i = 0; i < MAX_LIGHTS; i++)
	{
		if (GLight[i] != NULL)
		{
			if ((GLight[i]->extras & EXTRAS_SEMIDYNAMIC))
			{
				GLight[i]->treat = 0;
			}
			else if (!GLight[i]->treat)
			{
				GLight[i]->treat = 1;
			}

			GLight[i]->falldiff = GLight[i]->fallend - GLight[i]->fallstart;
			GLight[i]->falldiffmul = 1.f / GLight[i]->falldiff;
			GLight[i]->rgb255 = GLight[i]->rgb * 255.f;
			GLight[i]->precalc = GLight[i]->intensity * GLOBAL_LIGHT_FACTOR;
		}
	}
	
	for (long j = minz; j <= maxz; j++)
	{
		for (long i = minx; i <= maxx; i++)
		{
			eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

			for (long k = 0; k < eg->nbpoly; k++)
			{
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
	
	long i, j, x0, x1, z0, z1;

	i = x * ACTIVEBKG->Xmul;
	j = z * ACTIVEBKG->Zmul;
	
	x0 = i - siz;
	x1 = i + siz;
	z0 = j - siz;
	z1 = j + siz;
	
	if (x0 < 2) x0 = 2;
	else if (x0 >= ACTIVEBKG->Xsize - 2) x0 = ACTIVEBKG->Xsize - 3;
	
	if (x1 < 2) x1 = 0;
	else if (x1 >= ACTIVEBKG->Xsize - 2) x1 = ACTIVEBKG->Xsize - 3;
	
	if (z0 < 2) z0 = 0;
	else if (z0 >= ACTIVEBKG->Zsize - 2) z0 = ACTIVEBKG->Zsize - 3;
	
	if (z1 < 2) z1 = 0;
	else if (z1 >= ACTIVEBKG->Zsize - 2) z1 = ACTIVEBKG->Zsize - 3;
	
	LightMode oldml = ModeLight;
	ModeLight &= ~MODE_RAYLAUNCH;
	EERIEPrecalcLights(x0, z0, x1, z1);
	ModeLight = oldml;
}

void EERIERemovePrecalcLights() {
	
	EERIEPOLY * ep;
	EERIE_BKG_INFO * eg;
 
	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		if (GLight[i] != NULL) GLight[i]->treat = 1;
	}
	
	for(int j = 0; j < ACTIVEBKG->Zsize; j++) {
		for(int i = 0; i < ACTIVEBKG->Xsize; i++) {
			
			eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];
			
			for (long k = 0; k < eg->nbpoly; k++) {
				ep = &eg->polydata[k];
				ep->v[3].color = ep->v[2].color = ep->v[1].color = ep->v[0].color = Color::white.toBGR();
			}
		}
	}
}
