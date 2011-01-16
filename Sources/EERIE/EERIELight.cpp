/*
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
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// EERIELight
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
///////////////////////////////////////////////////////////////////////////////

#include "EERIELight.h"
#include "EERIEMath.h"
#include "EERIEObject.h"
#include "EERIEDraw.h"

#include "ARX_Time.h"
#include "ARX_Sound.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
extern float GLOBAL_LIGHT_FACTOR;
EERIE_LIGHT * GLight[MAX_LIGHTS];
EERIE_LIGHT DynLight[MAX_DYNLIGHTS];
long LIGHTPOWERUP = 0;
float LPpower = 3.f;

EERIE_LIGHT * PDL[MAX_DYNLIGHTS];
long TOTPDL = 0;

HANDLE LIGHTTHREAD = NULL;
long PROGRESS_COUNT = 0;
long PROGRESS_TOTAL = 0;

EERIE_LIGHT * IO_PDL[MAX_DYNLIGHTS];
long TOTIOPDL = 0;

bool ValidDynLight(long num)
{
	if (	(num >= 0)
        &&	(num < MAX_DYNLIGHTS)
        &&	DynLight[num].exist)
		return true;

	return false;
}
extern float GLOBAL_LIGHT_FACTOR;
void PrecalcIOLighting(EERIE_3D * pos, float radius, long flags)
{
	static EERIE_3D lastpos;

	if (flags & 1)
	{
		lastpos.x = 99999.f + lastpos.x;
		lastpos.y = 99999.f + lastpos.y;
		lastpos.z = 99999.f + lastpos.z;
		return;
	}

	// Lastpos optim
	if ((EEDistance3D(pos, &lastpos) < 100)) 
	{
		return;
	}

	lastpos.x = pos->x;
	lastpos.y = pos->y;
	lastpos.z = pos->z;

	TOTIOPDL = 0;

	for (long i = 0; i < MAX_LIGHTS; i++)
	{
		EERIE_LIGHT * el = GLight[i];

		if ((el)  && (el->exist) && (el->status)
		        && !(el->extras & EXTRAS_SEMIDYNAMIC))
		{
			if ((el->pos.x >= pos->x - radius) && (el->pos.x <= pos->x + radius)
			        && (el->pos.z >= pos->z - radius) && (el->pos.z <= pos->z + radius))
			{

				el->rgb255.r = el->rgb.r * 255.f;
				el->rgb255.g = el->rgb.g * 255.f;
				el->rgb255.b = el->rgb.b * 255.f;
				el->falldiff = el->fallend - el->fallstart;
				el->falldiffmul = 1.f / el->falldiff;
				el->precalc = el->intensity * GLOBAL_LIGHT_FACTOR;
				IO_PDL[TOTIOPDL] = el;
			
				TOTIOPDL++;

				if (TOTIOPDL >= MAX_DYNLIGHTS) TOTIOPDL--;
			}
		}
	}
}

//*************************************************************************************
//*************************************************************************************
void EERIE_LIGHT_Apply(EERIEPOLY * ep, EERIEPOLY * father)
{
	if (ep->type & POLY_IGNORE)  return;

	long i;
	float epr[4];
	float epg[4];
	float epb[4];

	epr[3] = epr[2] = epr[1] = epr[0] = 0; 
	epg[3] = epg[2] = epg[1] = epg[0] = 0; 
	epb[3] = epb[2] = epb[1] = epb[0] = 0; 

	for (i = 0; i < MAX_LIGHTS; i++)
	{
		EERIE_LIGHT * el = GLight[i];

		if ((el) && (el->treat) && (el->exist) && (el->status) 
		        && !(el->extras & EXTRAS_SEMIDYNAMIC))
		{
			if (Distance3D(el->pos.x, el->pos.y, el->pos.z,
			               ep->center.x, ep->center.y, ep->center.z) < el->fallend + 100.f)
				EERIE_LIGHT_Make(ep, epr, epg, epb, el, father);
		}
	}

	for (i = 0; i < MAX_ACTIONS; i++)
	{
		if ((actions[i].exist) && ((actions[i].type == ACT_FIRE2) || (actions[i].type == ACT_FIRE)))
		{
			if (Distance3D(actions[i].light.pos.x, actions[i].light.pos.y, actions[i].light.pos.z,
			               ep->center.x, ep->center.y, ep->center.z) < actions[i].light.fallend + 100.f)
				EERIE_LIGHT_Make(ep, epr, epg, epb, &actions[i].light, father);
		}
	}

	long nbvert;

	if (ep->type & POLY_QUAD) nbvert = 4;
	else nbvert = 3;

	for (i = 0; i < nbvert; i++)
	{
		if (epr[i] > 1.f) epr[i] = 1.f;
		else if (epr[i] < ACTIVEBKG->ambient.r) epr[i] = ACTIVEBKG->ambient.r;

		if (epg[i] > 1.f) epg[i] = 1.f;
		else if (epg[i] < ACTIVEBKG->ambient.g) epg[i] = ACTIVEBKG->ambient.g;

		if (epb[i] > 1.f) epb[i] = 1.f;
		else if (epb[i] < ACTIVEBKG->ambient.b) epb[i] = ACTIVEBKG->ambient.b;

		ep->v[i].color = EERIERGB(epr[i], epg[i], epb[i]);
	}
}

void EERIE_LIGHT_TranslateSelected(EERIE_3D * trans)
{
	for (long i = 0; i < MAX_LIGHTS; i++)
	{
		if (GLight[i] != NULL)
			if (GLight[i]->selected)
			{
				if (GLight[i]->tl > 0) DynLight[GLight[i]->tl].exist = 0;

				GLight[i]->tl = -1;
				GLight[i]->pos.x += trans->x;
				GLight[i]->pos.y += trans->y;
				GLight[i]->pos.z += trans->z;
			}
	}
}
void EERIE_LIGHT_UnselectAll()
{
	for (long i = 0; i < MAX_LIGHTS; i++)
	{
		if (GLight[i] != NULL)
			if ((GLight[i]->exist) && (GLight[i]->treat))
			{
				GLight[i]->selected = 0;
			}
	}
}

void EERIE_LIGHT_ClearByIndex(long num)
{
	if ((num >= 0) && (num < MAX_LIGHTS))
	{
		if (GLight[num] != NULL)
		{
			if (GLight[num]->tl != -1) DynLight[GLight[num]->tl].exist = 0;

			free(GLight[num]);
			GLight[num] = NULL;
		}
	}
}
void EERIE_LIGHT_ClearAll()
{
	for (long i = 0; i < MAX_LIGHTS; i++)
	{
		EERIE_LIGHT_ClearByIndex(i);
	}
}
void EERIE_LIGHT_ClearSelected()
{
	for (long i = 0; i < MAX_LIGHTS; i++)
	{
		if (GLight[i] != NULL)
			if (GLight[i]->selected)
			{
				EERIE_LIGHT_ClearByIndex(i);
			}
	}
}

void EERIE_LIGHT_GlobalInit()
{
	static long init = 0;

	if (!init)
	{
		memset(GLight, 0, sizeof(EERIE_LIGHT *) * MAX_LIGHTS);

		init = 1;
		return;
	}

	for (long i = 0; i < MAX_LIGHTS; i++)
		if (GLight[i])
		{
			if (GLight[i]->tl > 0) DynLight[GLight[i]->tl].exist = 0;

			free(GLight[i]);
			GLight[i] = NULL;

		}
}

long EERIE_LIGHT_GetFree()
{
	for (long i = 0; i < MAX_LIGHTS; i++)
	{
		if (!GLight[i])
		{
			return i;
		}
	}

	return -1;
}

long EERIE_LIGHT_Create()
{
	for (unsigned long i(0); i < MAX_LIGHTS; i++)
		if (!GLight[i])
		{
			GLight[i] = (EERIE_LIGHT *)malloc(sizeof(EERIE_LIGHT));

			if (!GLight[i]) return -1;

			memset(GLight[i], 0, sizeof(EERIE_LIGHT));
			GLight[i]->sample = ARX_SOUND_INVALID_RESOURCE;
			GLight[i]->tl = -1;
			return i;
		}

	return -1;
}


long EERIE_LIGHT_Count()
{
	long count = 0;

	for (long i = 0; i < MAX_LIGHTS; i++)
		if (GLight[i] && !(GLight[i]->type & TYP_SPECIAL1)) count++;

	return count;
}

void EERIE_LIGHT_GlobalAdd(EERIE_LIGHT * el)
{
	long num = EERIE_LIGHT_GetFree();

	if (num > -1)
	{
		GLight[num] = (EERIE_LIGHT *)malloc(sizeof(EERIE_LIGHT));
		memcpy(GLight[num], el, sizeof(EERIE_LIGHT));
		GLight[num]->tl = -1;
		GLight[num]->sample = ARX_SOUND_INVALID_RESOURCE;
	}
}

void EERIE_LIGHT_MoveAll(EERIE_3D * trans)
{
	for (long i = 0; i < MAX_LIGHTS; i++)
		if (GLight[i] != NULL)
		{
			GLight[i]->pos.x += trans->x;
			GLight[i]->pos.y += trans->y;
			GLight[i]->pos.z += trans->z;
		}
}

float BIGLIGHTPOWER = 0.f;
extern long HIPOLY;

//*************************************************************************************
//*************************************************************************************
float my_CheckInPoly(float x, float y, float z, EERIEPOLY * mon_ep, EERIE_LIGHT * light)
{
	long px, pz;
	F2L(x * ACTIVEBKG->Xmul, &px);


	if (px > ACTIVEBKG->Xsize - 3)
	{
		return NULL;
	}

	if (px < 2)
	{
		return NULL;
	}

	F2L(z * ACTIVEBKG->Zmul, &pz);

	if (pz > ACTIVEBKG->Zsize - 3)
	{
		return NULL;
	}

	if (pz < 2)
	{
		return NULL;
	}

	float nb_shadowvertexinpoly = 0.0f;
	float nb_totalvertexinpoly = 0.0f;

	EERIEPOLY * ep;
	EERIE_BKG_INFO * eg;

	EERIE_3D orgn, dest;
	EERIE_3D hit;

	orgn.x = light->pos.x;
	orgn.y = light->pos.y;
	orgn.z = light->pos.z;

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

						if ((fabs(ep->v[a].sx - x) <= fDiff) &&
								(fabs(ep->v[a].sy - y) <= fDiff) &&
								(fabs(ep->v[a].sz - z) <= fDiff))
						{

							float v1[3];
							v1[0] = mon_ep->nrml->x;
							v1[1] = mon_ep->nrml->y;
							v1[2] = mon_ep->nrml->z;

							float v2[3];
							v2[0] = ep->nrml->x;
							v2[1] = ep->nrml->y;
							v2[2] = ep->nrml->z;

							if (DOT(v1, v2) > 0.0f)
							{
								nb_totalvertexinpoly += nbvert;

								for (b = 0; b < nbvert; b++)
								{
									dest.x = ep->v[b].sx;
									dest.y = ep->v[b].sy;
									dest.z = ep->v[b].sz;

									if (Visible(&orgn, &dest, ep, &hit))
									{
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
extern void ARX_EERIE_LIGHT_Make(EERIEPOLY * ep, float * epr, float * epg, float * epb, EERIE_LIGHT * light, EERIEPOLY * father);

//*************************************************************************************
void EERIE_LIGHT_Make(EERIEPOLY * ep, float * epr, float * epg, float * epb, EERIE_LIGHT * light, EERIEPOLY * father)
{
	ARX_EERIE_LIGHT_Make(ep, epr, epg, epb, light, father);
	return;
	int		i;				// iterator
	int		nbvert;			// number or vertices per face (3 or 4)
	float	distance[4];	// distance from light to each vertex
	float	fRes;			// value of light intensity for a given vertex
	EERIE_3D vLight;		// vector (light to vertex)
	EERIE_3D vNorm;			// vector (interpolated normal of vertex)

	if (ep->type & POLY_IGNORE)
		return;

	(ep->type & POLY_QUAD) ? nbvert = 4 : nbvert = 3;

	// compute light - vertex distance
	for (i = 0; i < nbvert; i++)
	{
		distance[i] = TRUEEEDistance3D(&light->pos, (EERIE_3D *)&ep->v[i]);
	}

	for (i = 0; i < nbvert; i++)
	{
		fRes = 1.0f;

		if (distance[i] < light->fallend)
		{
			//---------------------- start MODE_NORMALS
			if (ModeLight & MODE_NORMALS)
			{
				vLight.x = light->pos.x - ep->v[i].sx;
				vLight.y = light->pos.y - ep->v[i].sy;
				vLight.z = light->pos.z - ep->v[i].sz;
				TRUEVector_Normalize(&vLight);
				vNorm.x = ep->nrml[i].x;
				vNorm.y = ep->nrml[i].y;
				vNorm.z = ep->nrml[i].z;

				fRes = Vector_DotProduct(&vLight, &vNorm);

				if (fRes < 0.0f)
				{
					fRes = 0.0f;
				}
			}

			//---------------------- end MODE_NORMALS

			//---------------------- start MODE_RAYLAUNCH
			if ((ModeLight & MODE_RAYLAUNCH) && !(light->extras & EXTRAS_NOCASTED))
			{
				EERIE_3D orgn, dest, hit;
				orgn.x = light->pos.x;
				orgn.y = light->pos.y;
				orgn.z = light->pos.z;
				dest.x = ep->v[i].sx;
				dest.y = ep->v[i].sy;
				dest.z = ep->v[i].sz;

				if (ModeLight & MODE_SMOOTH)
					fRes *= my_CheckInPoly(ep->v[i].sx, ep->v[i].sy, ep->v[i].sz, ep, light);
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
extern float GLOBAL_LIGHT_FACTOR;
//*************************************************************************************
long	DYNAMIC_NORMALS = 1;

extern EERIE_CAMERA DynLightCam;

extern float BIGLIGHTPOWER;

//-----------------------------------------------------------------------------
void ComputeLight2DPos(EERIE_LIGHT * _pL)
{
	D3DTLVERTEX in, out;
	in.sx = _pL->pos.x;
	in.sy = _pL->pos.y;
	in.sz = _pL->pos.z;
	EERIETreatPoint(&in, &out);

	if ((out.sz > 0.f) && (out.sz < 1000.f) && (out.rhw > 0))
	{
		float t;
		float siz = 50;
		float fMaxdist = 300;

		if (Project.telekinesis) fMaxdist = 850;

		t = siz * (1.0f - 1.0f / (out.rhw * fMaxdist)) + 10;

		_pL->maxs.x = out.sx + t;
		_pL->mins.x = out.sx - t;
		_pL->maxs.y = out.sy + t;
		_pL->mins.y = out.sy - t;


		if (0)
			if ((_pL->mins.x >= -200.f) && (_pL->mins.x <= 1000.f))
				if ((_pL->mins.y >= -200.f) && (_pL->mins.y <= 1000.f))
				{

					EERIEDraw2DLine(GDevice, _pL->mins.x, _pL->mins.y, _pL->maxs.x, _pL->mins.y, 0.00001f, D3DRGB(1.f, 1.f, 1.f));
					EERIEDraw2DLine(GDevice, _pL->maxs.x, _pL->mins.y, _pL->maxs.x, _pL->maxs.y, 0.00001f, D3DRGB(1.f, 1.f, 1.f));
					EERIEDraw2DLine(GDevice, _pL->maxs.x, _pL->maxs.y, _pL->mins.x, _pL->maxs.y, 0.00001f, D3DRGB(1.f, 1.f, 1.f));
					EERIEDraw2DLine(GDevice, _pL->mins.x, _pL->maxs.y, _pL->mins.x, _pL->mins.y, 0.00001f, D3DRGB(1.f, 1.f, 1.f));
				}
	}
}

//*************************************************************************************
//*************************************************************************************
void TreatBackgroundDynlights()
{
	long n;

	for (long i = 0; i < MAX_LIGHTS; i++)
	{
		if ((GLight[i] != NULL) && (GLight[i]->extras & EXTRAS_SEMIDYNAMIC))
		{
			float fDist = EEDistance3D(&GLight[i]->pos, &ACTIVECAM->pos);
			float fMaxdist = 300;

			if (Project.telekinesis) fMaxdist = 850;

			if (fDist <= fMaxdist)
			{
				ComputeLight2DPos(GLight[i]);
			}

			if (GLight[i]->status == 0)
			{
				// vient de s'éteindre
				if (GLight[i]->tl > 0)
				{
					DynLight[GLight[i]->tl].exist = 0;
					GLight[i]->tl = -1;
					EERIE_3D _pos2;

					for (long l = 0; l < inter.nbmax; l++)
					{
						if ((inter.iobj[l] != NULL) &&
						        (inter.iobj[l]->ioflags & IO_MARKER))
						{
							GetItemWorldPosition(inter.iobj[l], &_pos2);

							if (EEDistance3D(&GLight[i]->pos, &_pos2) <= 300)
							{
								SendIOScriptEvent(inter.iobj[l], SM_CUSTOM, "DOUSE");
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
					EERIE_3D _pos2;

					for (long l = 0; l < inter.nbmax; l++)
					{
						if ((inter.iobj[l] != NULL) &&
						        (inter.iobj[l]->ioflags & IO_MARKER))
						{
							GetItemWorldPosition(inter.iobj[l], &_pos2);

							if (EEDistance3D(&GLight[i]->pos, &_pos2) <= 300)
							{
								SendIOScriptEvent(inter.iobj[l], SM_CUSTOM, "FIRE");
							}
						}
					}

					GLight[i]->tl = GetFreeDynLight();
				}

				n = GLight[i]->tl;

				if (n != -1)
				{
					DynLight[n].pos.x		=	GLight[i]->pos.x;
					DynLight[n].pos.y		=	GLight[i]->pos.y;
					DynLight[n].pos.z		=	GLight[i]->pos.z;
					DynLight[n].exist		=	1;
					DynLight[n].fallstart	=	GLight[i]->fallstart;
					DynLight[n].fallend		=	GLight[i]->fallend;
					DynLight[n].type		=	TYP_SPECIAL1;
					DynLight[n].intensity	=	GLight[i]->intensity;
					DynLight[n].ex_flaresize =	GLight[i]->ex_flaresize;
					DynLight[n].extras		=	GLight[i]->extras;
					DynLight[n].duration	=	(long) LONG_MAX;

					DynLight[n].rgb.r = GLight[i]->rgb.r - GLight[i]->rgb.r * GLight[i]->ex_flicker.r * rnd() * DIV2; 
					DynLight[n].rgb.g = GLight[i]->rgb.g - GLight[i]->rgb.g * GLight[i]->ex_flicker.g * rnd() * DIV2; 
					DynLight[n].rgb.b = GLight[i]->rgb.b - GLight[i]->rgb.b * GLight[i]->ex_flicker.b * rnd() * DIV2; 

					if (DynLight[n].rgb.r < 0.f) DynLight[n].rgb.r = 0.f;

					if (DynLight[n].rgb.g < 0.f) DynLight[n].rgb.g = 0.f;

					if (DynLight[n].rgb.b < 0.f) DynLight[n].rgb.b = 0.f;
					
					DynLight[n].rgb255.r = DynLight[n].rgb.r * 255.f;
					DynLight[n].rgb255.g = DynLight[n].rgb.g * 255.f;
					DynLight[n].rgb255.b = DynLight[n].rgb.b * 255.f;

					DynLight[n].falldiff = DynLight[n].fallend - DynLight[n].fallstart;
					DynLight[n].falldiffmul = 1.f / DynLight[n].falldiff;
					DynLight[n].precalc = DynLight[n].intensity * GLOBAL_LIGHT_FACTOR;
				}
			}
		}

	}
}


//-----------------------------------------------------------------------------
void PrecalcDynamicLighting(long x0, long z0, long x1, long z1)
{
	long i;		// iterator

	TreatBackgroundDynlights();
	TOTPDL = 0;

	float fx0 = ACTIVEBKG->Xdiv * (float)x0;
	float fz0 = ACTIVEBKG->Zdiv * (float)z0;
	float fx1 = ACTIVEBKG->Xdiv * (float)x1;
	float fz1 = ACTIVEBKG->Zdiv * (float)z1;

	for (i = 0; i < MAX_DYNLIGHTS; i++)
	{
		EERIE_LIGHT * el = &DynLight[i];

		if ((el->exist) && (el->rgb.r >= 0.f))
		{

			bool bDist = (EEDistance3D(&el->pos, &ACTIVECAM->pos) < ACTIVECAM->cdepth);

			if ((el->pos.x >= fx0) && (el->pos.x <= fx1)
			        && (el->pos.z >= fz0) && (el->pos.z <= fz1)
			        && bDist)
			{
				el->treat = 1;
				el->rgb255.r = el->rgb.r * 255.f;
				el->rgb255.g = el->rgb.g * 255.f;
				el->rgb255.b = el->rgb.b * 255.f;
				el->falldiff = el->fallend - el->fallstart;
				el->falldiffmul = 1.f / el->falldiff;
				el->precalc = el->intensity * GLOBAL_LIGHT_FACTOR;
				PDL[TOTPDL] = el;
				TOTPDL++;

				if (TOTPDL >= MAX_DYNLIGHTS) TOTPDL--;
			}
			else if (el->treat) el->treat = 0;

			if (el->duration)
			{
				float tim = ((float)ARXTime - (float)el->time_creation);
				float duration = (float)el->duration;

				if (tim >= duration)
				{


					float sub = _framedelay * 0.001f;
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
extern long LIGHT_THREAD_STATUS;
extern long PAUSED_PRECALC;
void EERIEPrecalcLights(long minx, long minz, long maxx, long maxz)
{
	long i, j;		// iterators
	EERIEPOLY * ep;
	EERIE_BKG_INFO * eg;
 
	if (minx < 0) minx = 0;
	else if (minx >= ACTIVEBKG->Xsize) minx = ACTIVEBKG->Xsize - 1;

	if (maxx < 0) maxx = 0;
	else if (maxx >= ACTIVEBKG->Xsize) maxx = ACTIVEBKG->Xsize - 1;

	if (minz < 0) minz = 0;
	else if (minz >= ACTIVEBKG->Zsize) minz = ACTIVEBKG->Zsize - 1;

	if (maxz < 0) maxz = 0;
	else if (maxz >= ACTIVEBKG->Zsize) maxz = ACTIVEBKG->Zsize - 1;

	{
		if (LIGHT_THREAD_STATUS == 3) return;

		for (i = 0; i < MAX_LIGHTS; i++)
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
				GLight[i]->rgb255.r = GLight[i]->rgb.r * 255.f;
				GLight[i]->rgb255.g = GLight[i]->rgb.g * 255.f;
				GLight[i]->rgb255.b = GLight[i]->rgb.b * 255.f;
				GLight[i]->precalc = GLight[i]->intensity * GLOBAL_LIGHT_FACTOR;
			}
		}

		PROGRESS_COUNT = PROGRESS_TOTAL = 0;

		if (LIGHT_THREAD_STATUS == 3) return;

		for (j = minz; j <= maxz; j++)
			for (i = minx; i <= maxx; i++)
			{
				eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

				for (long k = 0; k < eg->nbpoly; k++)
				{
					ep = &eg->polydata[k];
					PROGRESS_TOTAL++;
				}
			}

		if (LIGHT_THREAD_STATUS == 3) return;

		for (j = minz; j <= maxz; j++)
			for (i = minx; i <= maxx; i++)
			{
				if (LIGHT_THREAD_STATUS == 3) return;

				eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

				while (PAUSED_PRECALC) Sleep(1000);

				for (long k = 0; k < eg->nbpoly; k++)
				{

					if (LIGHT_THREAD_STATUS == 3) return;

					PROGRESS_COUNT++;
					ep = &eg->polydata[k];
					ep->type &= ~POLY_IGNORE;

					if (ep)	EERIE_LIGHT_Apply(ep, NULL);
				}
			}

	}

	if (0)
	{
		EERIE_BKG_INFO * eg2;
		EERIEPOLY * ep2;
		long i2, j2, k, k2, mai, maj, mii, mij;
		float totr, totg, totb;
		float tr, tg, tb, tcd, tc;

		for (j = minz; j <= maxz; j++)
			for (i = minx; i <= maxx; i++)
			{
				eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

				for (long l = 0; l < eg->nbpoly; l++)
				{
					ep = &eg->polydata[l];

					if (!(ep->type & POLY_IGNORE))
						for (k = 0; k < 3; k++)
						{
							totr = (float)((ep->v[k].color >> 16) & 255) * 2.f;
							totg = (float)((ep->v[k].color >> 8) & 255) * 2.f;
							totb = (float)((ep->v[k].color) & 255) * 2.f;
							tc = 2.f;
							mai = i + 2;
							maj = j + 2;
							mii = i - 2;
							mij = j - 2;

							if (mij < 0) mij = 0;

							if (mii < 0) mii = 0;

							if (maj >= ACTIVEBKG->Zsize) maj = ACTIVEBKG->Zsize - 1;

							if (mai >= ACTIVEBKG->Xsize) mai = ACTIVEBKG->Xsize - 1;

							for (j2 = mij; j2 < maj; j2++)
								for (i2 = mii; i2 < mai; i2++)
								{
									eg2 = &ACTIVEBKG->Backg[i2+j2*ACTIVEBKG->Xsize];

									for (long l = 0; l < eg2->nbpoly; l++)
									{
										ep2 = &eg2->polydata[l];

										if (!(ep2->type & POLY_IGNORE))
											if (ep != ep2)

												for (k2 = 0; k2 < 3; k2++)
												{
													float dist = Distance3D(ep2->v[k2].sx, ep2->v[k2].sy, ep2->v[k2].sz,
													                        ep->v[k].sx, ep->v[k].sy, ep->v[k].sz);

													if (dist < 50.f)
													{
														tr = (float)((ep2->v[k2].color >> 16) & 255);
														tg = (float)((ep2->v[k2].color >> 8) & 255);
														tb = (float)((ep2->v[k2].color) & 255);
														float r = (50.f - dist) * 0.02f;
														totr += tr * r;
														totg += tg * r;
														totb += tb * r;
														tc += r;
													}
												}
									}
								}

							tcd = 1.f / (float)tc;
							ep->v[k].color = D3DRGB(totr * tcd * DIV255, totg * tcd * DIV255, totb * tcd * DIV255);
						}

				}
			}
	}
}


//*************************************************************************************
//*************************************************************************************
void _RecalcLightZone(float x, float y, float z, long siz)
{
	long i, j, x0, x1, z0, z1;

	F2L(x * ACTIVEBKG->Xmul, &i);

	F2L(z * ACTIVEBKG->Zmul, &j);

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

	long oldml = ModeLight;
	ModeLight &= ~MODE_RAYLAUNCH;
	EERIEPrecalcLights(x0, z0, x1, z1);
	ModeLight = oldml;
}

//*************************************************************************************
//*************************************************************************************
void RecalcLightZone(float x, float y, float z, long siz)
{
	long i, j, x0, x1, z0, z1;

	F2L(x * ACTIVEBKG->Xmul, &i);

	F2L(z * ACTIVEBKG->Zmul, &j);
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

	LaunchLightThread(x0, z0, x1, z1);
}

//*************************************************************************************
//*************************************************************************************
void EERIERemovePrecalcLights()
{
	long i, j;
	EERIEPOLY * ep;
	EERIE_BKG_INFO * eg;
 
	for (i = 0; i < MAX_LIGHTS; i++) 	if (GLight[i] != NULL) GLight[i]->treat = 1;

	for (j = 0; j < ACTIVEBKG->Zsize; j++)
		for (i = 0; i < ACTIVEBKG->Xsize; i++)
		{
			eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

			for (long k = 0; k < eg->nbpoly; k++)
			{
				ep = &eg->polydata[k];
				ep->v[3].color = ep->v[2].color = ep->v[1].color = ep->v[0].color = D3DCOLORWHITE;
			}
		}
}
