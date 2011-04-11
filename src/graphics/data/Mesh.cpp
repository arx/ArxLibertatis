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
//////////////////////////////////////////////////////////////////////////////////////
// EERIEPoly																	    //
//////////////////////////////////////////////////////////////////////////////////////
//																		    		//
// Description:																		//
//																					//
// Updates: (date) (person) (update)												//
//																					//
// Code: Cyril Meynier																//
//																					//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved						//
//////////////////////////////////////////////////////////////////////////////////////

#include "graphics/data/Mesh.h"

#include <cstdlib>
#include <cstdio>
#include <map>

#include "ai/PathFinder.h"
#include "ai/PathFinderManager.h"

#include "animation/Animation.h"

#include "core/Time.h"
#include "core/Core.h"

#include "game/Player.h"

#include "gui/MenuWidgets.h"

#include "graphics/Draw.h"
#include "graphics/GraphicsUtility.h"
#include "graphics/GraphicsEnum.h"
#include "graphics/data/Texture.h"
#include "graphics/data/FastSceneFormat.h"
#include "graphics/particle/ParticleEffects.h"

#include "io/IO.h"
#include "io/String.h"
#include "io/FilePath.h"
#include "io/PakManager.h"
#include "io/Filesystem.h"
#include "io/Logger.h"
#include "io/Blast.h"
#include "io/Implode.h"

#include "physics/Anchors.h"
#include "physics/Physics.h"

#include "scene/Scene.h"
#include "scene/Light.h"
#include "scene/Object.h"
#include "scene/Interactive.h"

using std::min;
using std::max;
using std::copy;

void ComputeFastBkgData(EERIE_BACKGROUND * eb);
extern long ParticleCount;
extern bool ARXPausedTimer;
extern EERIE_LIGHT * PDL[MAX_DYNLIGHTS];
extern EERIE_LIGHT * GLight[MAX_LIGHTS];
extern EERIE_LIGHT DynLight[MAX_DYNLIGHTS];
void EERIE_PORTAL_Release();
long NEED_ANCHORS = 1;
float Xratio = 1.f;
float Yratio = 1.f;
extern long CYRIL_VERSION;
extern CMenuConfig * pMenuConfig;
long COMPUTE_PORTALS = 1;




 
static int RayIn3DPolyNoCull(EERIE_3D * orgn, EERIE_3D * dest, EERIEPOLY * epp);

D3DMATRIX ProjectionMatrix;

bool bGMergeVertex = false;

void ReleaseAnimFromIO(INTERACTIVE_OBJ * io, long num)
{
	for (long count = 0; count < MAX_ANIM_LAYERS; count++)
	{
		if (io->animlayer[count].cur_anim == io->anims[num])
		{
			memset(&io->animlayer[count], 0, sizeof(ANIM_USE));
			io->animlayer[count].cur_anim = NULL;
		}

		if (io->animlayer[count].next_anim == io->anims[num])
			io->animlayer[count].next_anim = NULL;
	}

	EERIE_ANIMMANAGER_ReleaseHandle(io->anims[num]);
	io->anims[num] = NULL;
}

//*************************************************************************************
//*************************************************************************************
void DebugSphere(float x, float y, float z, float siz, long tim, D3DCOLOR color)
{
	long	j = ARX_PARTICLES_GetFree();

	if ((j != -1) && (!ARXPausedTimer))
	{
		ParticleCount++;
		particle[j].exist		=	true;
		particle[j].zdec		=	0;
		particle[j].ov.x		=	x;
		particle[j].ov.y		=	y;
		particle[j].ov.z		=	z;
		particle[j].move.x		=	0.f;
		particle[j].move.y		=	0.f;
		particle[j].move.z		=	0.f;
		particle[j].scale.x		=	0.f;
		particle[j].scale.y		=	0.f;
		particle[j].scale.z		=	0.f;
		float	fArx_Time_Get	=	ARX_TIME_Get();
		ARX_CHECK_LONG(fArx_Time_Get);
		particle[j].timcreation	=	ARX_CLEAN_WARN_CAST_LONG(fArx_Time_Get);
		particle[j].tolive		=	tim;
		particle[j].tc			=	EERIE_DRAW_sphere_particle;
		particle[j].siz			=	siz;
		particle[j].r			=	(float)((color >> 16) & 255) * ( 1.0f / 255 );
		particle[j].g			=	(float)((color >> 8)  & 255) * ( 1.0f / 255 );
		particle[j].b			=	(float)((color) & 255) * ( 1.0f / 255 );
	}
}

//*************************************************************************************
//*************************************************************************************
float fK3;

void EERIE_CreateMatriceProj(float _fWidth, float _fHeight, float _fFOV, float _fZNear, float _fZFar)
{
	float fAspect = _fHeight / _fWidth;
	float fFOV = radians(_fFOV);
	float fFarPlane = _fZFar;
	float fNearPlane = _fZNear;
	float w = fAspect * (cosf(fFOV / 2) / sinf(fFOV / 2));
	float h =   1.0f  * (cosf(fFOV / 2) / sinf(fFOV / 2));
	float Q = fFarPlane / (fFarPlane - fNearPlane);

	fK3 = (_fZFar - _fZNear);

	ZeroMemory(&ProjectionMatrix, sizeof(D3DMATRIX));
	ProjectionMatrix._11 = w;
	ProjectionMatrix._22 = h;
	ProjectionMatrix._33 = Q;
	ProjectionMatrix._43 = (-Q * fNearPlane);
	ProjectionMatrix._34 = 1.f;

	D3DMATRIX mat;
	mat._11 = 1.f;
	mat._12 = 0.f;
	mat._13 = 0.f;
	mat._14 = 0.f;
	mat._21 = 0.f;
	mat._22 = 1.f;
	mat._23 = 0.f;
	mat._24 = 0.f;
	mat._31 = 0.f;
	mat._32 = 0.f;
	mat._33 = 1.f;
	mat._34 = 0.f;
	mat._41 = 0.f;
	mat._42 = 0.f;
	mat._43 = 0.f;
	mat._44 = 1.f;
	GDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &mat);
	GDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, &mat);
	GDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &ProjectionMatrix);

	ProjectionMatrix._11 *= _fWidth * .5f;
	ProjectionMatrix._22 *= _fHeight * .5f;
	ProjectionMatrix._33 = -(fFarPlane * fNearPlane) / (fFarPlane - fNearPlane);	//HYPERBOLIC
	ProjectionMatrix._43 = Q;

	D3DVIEWPORT7 vp;
	vp.dwX		=	0;
	vp.dwY		=	0;
	vp.dwWidth	=	ARX_CLEAN_WARN_CAST_DWORD(_fWidth);
	vp.dwHeight	=	ARX_CLEAN_WARN_CAST_DWORD(_fHeight);
	vp.dvMinZ = 0.f;
	vp.dvMaxZ = 1.f;
	GDevice->SetViewport(&vp);
}

void specialEE_RTP(D3DTLVERTEX * in, D3DTLVERTEX * out)
{
	register EERIE_TRANSFORM * et = (EERIE_TRANSFORM *)&ACTIVECAM->transform;
	out->sx = in->sx - et->posx;
	out->sy = in->sy - et->posy;
	out->sz = in->sz - et->posz;

	register float temp = (out->sz * et->ycos) - (out->sx * et->ysin);
	out->sx = (out->sz * et->ysin) + (out->sx * et->ycos);
	out->sz = (out->sy * et->xsin) + (temp * et->xcos);
	out->sy = (out->sy * et->xcos) - (temp * et->xsin);

	float fZTemp = 1.f / out->sz;
	out->sz = fZTemp * ProjectionMatrix._33 + ProjectionMatrix._43; //HYPERBOLIC

	out->sx = out->sx * ProjectionMatrix._11 * fZTemp + et->xmod;
	out->sy = out->sy * ProjectionMatrix._22 * fZTemp + et->ymod;
	out->rhw = fZTemp;
}

static bool IntersectLinePlane(const EERIE_3D & l1, const EERIE_3D & l2, const EERIEPOLY * ep, EERIE_3D * intersect) {
	
	EERIE_3D v = l2 - l1;
	
	float d = ScalarProduct(&v, &ep->norm);
	
	if (d != 0.0f) {
		EERIE_3D v1 = ep->center - l2;
		d = ScalarProduct(&v1, &ep->norm) / d;
		
		*intersect = (v * d) + l2;
		
		return true;
	}

	return false;
}

bool RayCollidingPoly(EERIE_3D * orgn, EERIE_3D * dest, EERIEPOLY * ep, EERIE_3D * hit)
{
	if (IntersectLinePlane(*orgn, *dest, ep, hit))
	{
		if (RayIn3DPolyNoCull(orgn, dest, ep)) return true;
	}

	return false;
}

long MakeTopObjString(INTERACTIVE_OBJ * io,  string & dest) {
	
	EERIE_3D boxmin;
	EERIE_3D boxmax;

	if (io == NULL) return -1;

	boxmin.x = 999999999.f;
	boxmin.y = 999999999.f;
	boxmin.z = 999999999.f;
	boxmax.x = -999999999.f;
	boxmax.y = -999999999.f;
	boxmax.z = -999999999.f;

	for (size_t i = 0; i < io->obj->vertexlist.size(); i++)
	{
		boxmin.x = min(boxmin.x, io->obj->vertexlist3[i].v.x);
		boxmin.y = min(boxmin.y, io->obj->vertexlist3[i].v.y);
		boxmin.z = min(boxmin.z, io->obj->vertexlist3[i].v.z);

		boxmax.x = max(boxmax.x, io->obj->vertexlist3[i].v.x);
		boxmax.y = max(boxmax.y, io->obj->vertexlist3[i].v.y);
		boxmax.z = max(boxmax.z, io->obj->vertexlist3[i].v.z);
	}

	boxmin.y -= 5.f;
	boxmax.y -= 5.f;
	dest = "";

	if ((player.pos.x > boxmin.x)
			&& (player.pos.x < boxmax.x)
			&& (player.pos.z > boxmin.z)
			&& (player.pos.z < boxmax.z))
	{
		{
			if (EEfabs(player.pos.y + 160.f - boxmin.y) < 50.f)
				dest += " PLAYER";
		}
	}

	for (long i = 0; i < inter.nbmax; i++)
	{
		if (inter.iobj[i] != NULL)
		{
			if (inter.iobj[i] != io)
				if (inter.iobj[i]->show == SHOW_FLAG_IN_SCENE)
					if ((inter.iobj[i]->ioflags & IO_NPC) || (inter.iobj[i]->ioflags & IO_ITEM))
					{
						if (((inter.iobj[i]->pos.x) > boxmin.x)
								&& ((inter.iobj[i]->pos.x) < boxmax.x)
								&& ((inter.iobj[i]->pos.z) > boxmin.z)
								&& ((inter.iobj[i]->pos.z) < boxmax.z))
						{
							if (EEfabs(inter.iobj[i]->pos.y - boxmin.y) < 40.f)
							{
								dest += ' ' + inter.iobj[i]->long_name();
								
							}
						}

				}
		}
	}

	if (dest.length() == 0) dest = "NONE";

	return -1;
}


EERIEPOLY * CheckInPoly(float x, float y, float z, float * needY)
{
	long px, pz;
	EERIE_3D poss(x, y, z);

	px = poss.x * ACTIVEBKG->Xmul;
	pz = poss.z * ACTIVEBKG->Zmul;

	if ((pz >= ACTIVEBKG->Zsize - 1)
			||	(pz <= 0)
			||	(px >= ACTIVEBKG->Xsize - 1)
			||	(px <= 0))
		return NULL;

	float rx, rz;
	rx = poss.x - ((float)px * ACTIVEBKG->Xdiv);
	rz = poss.z - ((float)pz * ACTIVEBKG->Zdiv);

	EERIEPOLY * ep;
	FAST_BKG_DATA * feg;
	EERIEPOLY * found = NULL;

	float foundY = 0.f;
	short pzi, pza, pxi, pxa;

	ARX_CHECK_SHORT(pz - 1);
	ARX_CHECK_SHORT(pz + 1);
	short sPz = ARX_CLEAN_WARN_CAST_SHORT(pz);

	if (rz < -40.f)
	{
		pzi = sPz - 1;
		pza = sPz - 1;
	}
	else if (rz < 40.f)
	{
		pzi = sPz - 1;
		pza = sPz;
	}
	else if (rz > 60.f)
	{
		pzi = sPz;
		pza = sPz + 1;
	}
	else
	{
		pzi = sPz;
		pza = sPz;
	}

	ARX_CHECK_SHORT(px - 1);
	ARX_CHECK_SHORT(px + 1);
	short sPx = ARX_CLEAN_WARN_CAST_SHORT(px);

	if (rx < -40.f)
	{
		pxi = sPx - 1;
		pxa = sPx - 1;
	}

	else if (rx < 40.f)
	{
		pxi = sPx - 1;
		pxa = sPx;
	}
	else if (rx > 60.f)
	{
		pxi = sPx;
		pxa = sPx + 1;
	}
	else
	{
		pxi = sPx;
		pxa = sPx;
	}

	int i, j, k;

	for (j = pzi; j <= pza; j++)
		for (i = pxi; i <= pxa; i++)
		{
			feg = &ACTIVEBKG->fastdata[i][j];

			for (k = 0; k < feg->nbpolyin; k++)
			{
				ep = feg->polyin[k];

				if (
					(poss.x >= ep->min.x) && (poss.x <= ep->max.x)
					&&	(poss.z >= ep->min.z) && (poss.z <= ep->max.z)
					&& !(ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL))
					&& (ep->max.y >= poss.y)
					&&	(ep != found)
					&&	(PointIn2DPolyXZ(ep, poss.x, poss.z))
				)
				{
					if ((GetTruePolyY(ep, &poss, &rz))
							&&	(rz >= poss.y)
							&&	((found == NULL) || ((found != NULL) && (rz <= foundY)))
					   )
					{
						found = ep;
						foundY = rz;
					}
				}
			}
		}

	if (needY) *needY = foundY;

	return found;
}
EERIEPOLY * CheckInPolyPrecis(float x, float y, float z, float * needY)
{
	long px, pz;
	EERIE_3D poss(x, y, z);

	px = poss.x * ACTIVEBKG->Xmul;
	pz = poss.z * ACTIVEBKG->Zmul;

	if ((pz >= ACTIVEBKG->Zsize - 1)
			||	(pz <= 0)
			||	(px >= ACTIVEBKG->Xsize - 1)
			||	(px <= 0))
		return NULL;

	float rx, rz;
	rx = poss.x - ((float)px * ACTIVEBKG->Xdiv);
	rz = poss.z - ((float)pz * ACTIVEBKG->Zdiv);

	EERIEPOLY * ep;
	FAST_BKG_DATA * feg;
	EERIEPOLY * found = NULL;

	float foundY = 0.f;
	short pzi, pza, pxi, pxa;

	ARX_CHECK_SHORT(pz - 1);
	ARX_CHECK_SHORT(pz + 1);
	short sPz = ARX_CLEAN_WARN_CAST_SHORT(pz);

	if (rz < -40.f)
	{
		pzi = sPz - 1;
		pza = sPz - 1;
	}
	else if (rz < 40.f)
	{
		pzi = sPz - 1;
		pza = sPz;
	}
	else if (rz > 60.f)
	{
		pzi = sPz;
		pza = sPz + 1;
	}
	else
	{
		pzi = sPz;
		pza = sPz;
	}

	ARX_CHECK_SHORT(px + 1);
	ARX_CHECK_SHORT(px - 1);
	short sPx = ARX_CLEAN_WARN_CAST_SHORT(px);

	if (rx < -40.f)
	{
		pxi = sPx - 1;
		pxa = sPx - 1;
	}
	else if (rx < 40.f)
	{
		pxi = sPx - 1;
		pxa = sPx;
	}
	else if (rx > 60.f)
	{
		pxi = sPx;
		pxa = sPx + 1;
	}
	else
	{
		pxi = sPx;
		pxa = sPx;
	}

	int i, j, k;

	for (j = pzi; j <= pza; j++)
		for (i = pxi; i <= pxa; i++)
		{
			feg = &ACTIVEBKG->fastdata[i][j];

			for (k = 0; k < feg->nbpolyin; k++)
			{
				ep = feg->polyin[k];

				if (
					(poss.x >= ep->min.x) && (poss.x <= ep->max.x)
					&&	(poss.z >= ep->min.z) && (poss.z <= ep->max.z)
					&& !(ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL))
					&& (ep->max.y >= poss.y)
					&&	(ep != found)
					&&	(PointIn2DPolyXZ(ep, poss.x, poss.z))
				)
				{
					if ((GetTruePolyY(ep, &poss, &rz))
							&&	(rz >= poss.y)
							&&	((found == NULL) || ((found != NULL) && (rz <= foundY)))
					   )
					{
						found = ep;
						foundY = rz;
					}
				}
			}
		}

	if (needY) *needY = foundY;

	return found;
}

EERIEPOLY * EECheckInPoly(const EERIE_3D * pos, float * needY) {
	return CheckInPoly(pos->x, pos->y, pos->z, needY);
}

static FAST_BKG_DATA * getFastBackgroundData(float x, float z) {
	
	long px = x * ACTIVEBKG->Xmul;
	if(px < 0 || px >= ACTIVEBKG->Xsize) {
		return NULL;
	}
	
	long pz = z * ACTIVEBKG->Zmul;
	if(pz < 0 || pz >= ACTIVEBKG->Zsize) {
		return NULL;
	}
	
	return &ACTIVEBKG->fastdata[px][pz];
}

EERIEPOLY * CheckTopPoly(float x, float y, float z) {
	
	FAST_BKG_DATA * feg = getFastBackgroundData(x, z);
	if(!feg) {
		return false;
	}
	
	EERIEPOLY * found = NULL;
	for (long k = 0; k < feg->nbpolyin; k++) {
		
		EERIEPOLY * ep = feg->polyin[k];
		
		if((!(ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL)))
		   && (ep->min.y < y)
		   && (x >= ep->min.x) && (x <= ep->max.x)
		   && (z >= ep->min.z) && (z <= ep->max.z)
		   && (PointIn2DPolyXZ(ep, x, z))) {
			
			if((EEfabs(ep->max.y - ep->min.y) > 50.f) && (y - ep->center.y < 60.f)) {
				continue;
			}
			
			if(ep->tex != NULL) {
				if(found == NULL || ep->min.y > found->min.y) {
					found = ep;
				}
			}
		}
	}
	
	return found;
}

bool IsAnyPolyThere(float x, float z) {
	
	FAST_BKG_DATA * feg = getFastBackgroundData(x, z);
	if(!feg) {
		return false;
	}
	
	for(long k = 0; k < feg->nbpolyin; k++) {
		
		EERIEPOLY * ep = feg->polyin[k];
		
		if(PointIn2DPolyXZ(ep, x, z)) {
			return true;
		}
	}
	
	return false;
}

float FirstPolyPosY(float x, float z)
{
	EERIEPOLY * ep = GetMinPoly(x, 0.f, z);

	if (ep == NULL) return 99999999.f;

	return ep->max.y;
}

EERIEPOLY * GetMinPoly(float x, float y, float z) {
	
	FAST_BKG_DATA * feg = getFastBackgroundData(x, z);
	if(!feg) {
		return NULL;
	}
	
	EERIE_3D pos(x, y, z);
	
	EERIEPOLY * found = NULL;
	float foundy;
	for (long k = 0; k < feg->nbpolyin; k++) {
		
		EERIEPOLY * ep = feg->polyin[k];
		
		if(ep->type & POLY_WATER) continue;
		
		if(ep->type & POLY_TRANS) continue;
		
		if(ep->type & POLY_NOCOL) continue;
		
		if(PointIn2DPolyXZ(ep, x, z)) {
			float ret;
			if(GetTruePolyY(ep, &pos, &ret)) {
				if(!found || ret > foundy) {
					found = ep;
					foundy = ret;
				}
			}
		}
	}
	
	return found;
}

EERIEPOLY * GetMaxPoly(float x, float y, float z) {
	
	FAST_BKG_DATA * feg = getFastBackgroundData(x, z);
	if(!feg) {
		return NULL;
	}
	
	EERIE_3D pos(x, y, z);
	
	EERIEPOLY * found = NULL;
	float foundy;
	for(long k = 0; k < feg->nbpolyin; k++) {
		
		EERIEPOLY * ep = feg->polyin[k];
		
		if(ep->type & POLY_WATER) continue;
		
		if(ep->type & POLY_TRANS) continue;
		
		if(ep->type & POLY_NOCOL) continue;
		
		if(PointIn2DPolyXZ(ep, x, z)) {
			float ret;
			if(GetTruePolyY(ep, &pos, &ret)) {
				if(!found || ret < foundy) {
					found = ep;
					foundy = ret;
				}
			}
		}
	}
	
	return found;
}

EERIEPOLY * EEIsUnderWater(const EERIE_3D * pos) {
	
	FAST_BKG_DATA * feg = getFastBackgroundData(pos->x, pos->z);
	if(!feg) {
		return NULL;
	}
	
	EERIEPOLY * found = NULL;
	for(short k = 0; k < feg->nbpolyin; k++) {
		
		EERIEPOLY * ep = feg->polyin[k];
		
		if(ep->type & POLY_WATER) {
			if(ep->max.y < pos->y && PointIn2DPolyXZ(ep, pos->x, pos->z)) {
				if(!found || ep->max.y < found->max.y) {
					found = ep;
				}
			}
		}
	}
	return found;
}

bool GetTruePolyY(const EERIEPOLY * ep, const EERIE_3D * pos, float * ret)
{
	EERIE_3D	n, s21, s31;

	s21.x = ep->v[1].sx - ep->v[0].sx;
	s21.y = ep->v[1].sy - ep->v[0].sy;
	s21.z = ep->v[1].sz - ep->v[0].sz;
	s31.x = ep->v[2].sx - ep->v[0].sx;
	s31.y = ep->v[2].sy - ep->v[0].sy;
	s31.z = ep->v[2].sz - ep->v[0].sz;

	n.y = (s21.z * s31.x) - (s21.x * s31.z);

	if (n.y == 0.f) return false; 

	n.x = (s21.y * s31.z) - (s21.z * s31.y);
	n.z = (s21.x * s31.y) - (s21.y * s31.x);

	// uses s21.x instead of d
	s21.x = ep->v[0].sx * n.x + ep->v[0].sy * n.y + ep->v[0].sz * n.z;

	s21.x = (s21.x - (n.x * pos->x) - (n.z * pos->z)) / n.y;

	// Perhaps we can remove the two following lines... (need to test)
	if (s21.x < ep->min.y) s21.x = ep->min.y;
	else if (s21.x > ep->max.y) s21.x = ep->max.y;

	*ret = s21.x;
	return true;
}

//*************************************************************************************
//*************************************************************************************
EERIE_BACKGROUND * ACTIVEBKG = NULL;

//*************************************************************************************
// Selects Active Background Structure
//*************************************************************************************
EERIE_CAMERA * ACTIVECAM = NULL;

//*************************************************************************************
// Selects Active Camera
//*************************************************************************************
void SetActiveCamera(EERIE_CAMERA * cam)
{
	if (ACTIVECAM != cam) ACTIVECAM = cam;
}

//*************************************************************************************
//*************************************************************************************
void EERIETreatPoint(D3DTLVERTEX * in, D3DTLVERTEX * out)
{
	out->sx = in->sx - ACTIVECAM->pos.x;
	out->sy = in->sy - ACTIVECAM->pos.y;
	out->sz = in->sz - ACTIVECAM->pos.z;
	in->sx = (out->sx * ACTIVECAM->Ycos) + (out->sz * ACTIVECAM->Ysin);
	in->sz = (out->sz * ACTIVECAM->Ycos) - (out->sx * ACTIVECAM->Ysin);
	out->sz = (out->sy * ACTIVECAM->Xsin) + (in->sz * ACTIVECAM->Xcos);
	out->sy = (out->sy * ACTIVECAM->Xcos) - (in->sz * ACTIVECAM->Xsin);

	if (ACTIVECAM->Zsin == 0)
	{
		in->sy = out->sy;
	}
	else
	{
		in->sy = (out->sy * ACTIVECAM->Zcos) - (in->sx * ACTIVECAM->Zsin);
		in->sx = (in->sx * ACTIVECAM->Zcos) + (out->sy * ACTIVECAM->Zsin);
	}

	float fZTemp;
	fZTemp = 1.f / out->sz;
	out->sz = fZTemp * ProjectionMatrix._33 + ProjectionMatrix._43; //HYPERBOLIC
	out->sx = in->sx * ProjectionMatrix._11 * fZTemp + ACTIVECAM->posleft;
	out->sy = in->sy * ProjectionMatrix._22 * fZTemp + ACTIVECAM->postop;
	out->rhw = fZTemp;
}

void EERIETreatPoint2(D3DTLVERTEX * in, D3DTLVERTEX * out)
{
	out->sx = in->sx - ACTIVECAM->pos.x;
	out->sy = in->sy - ACTIVECAM->pos.y;
	out->sz = in->sz - ACTIVECAM->pos.z;
	in->sx = (out->sx * ACTIVECAM->Ycos) + (out->sz * ACTIVECAM->Ysin);
	in->sz = (out->sz * ACTIVECAM->Ycos) - (out->sx * ACTIVECAM->Ysin);
	out->sz = (out->sy * ACTIVECAM->Xsin) + (in->sz * ACTIVECAM->Xcos);
	out->sy = (out->sy * ACTIVECAM->Xcos) - (in->sz * ACTIVECAM->Xsin);

	if (ACTIVECAM->Zsin == 0)
	{
		in->sy = out->sy;
	}
	else
	{
		in->sy = (out->sy * ACTIVECAM->Zcos) - (in->sx * ACTIVECAM->Zsin);
		in->sx = (in->sx * ACTIVECAM->Zcos) + (out->sy * ACTIVECAM->Zsin);
	}

	float fZTemp;
	fZTemp = 1.f / out->sz;
	out->sz = fZTemp * ProjectionMatrix._33 + ProjectionMatrix._43; //HYPERBOLIC
	out->sx = in->sx * ProjectionMatrix._11 * fZTemp + ACTIVECAM->posleft;
	out->sy = in->sy * ProjectionMatrix._22 * fZTemp + ACTIVECAM->postop;
	out->rhw = fZTemp * 3000.f;

}

//*************************************************************************************
//*************************************************************************************
void EE_RT(D3DTLVERTEX * in, EERIE_3D * out)
{
	out->x = in->sx - ACTIVECAM->pos.x;
	out->y = in->sy - ACTIVECAM->pos.y;
	out->z = in->sz - ACTIVECAM->pos.z;

	register float temp;
	temp = (out->z * ACTIVECAM->Ycos) - (out->x * ACTIVECAM->Ysin);
	out->x = (out->x * ACTIVECAM->Ycos) + (out->z * ACTIVECAM->Ysin);

	out->z = (out->y * ACTIVECAM->Xsin) + (temp * ACTIVECAM->Xcos);
	out->y = (out->y * ACTIVECAM->Xcos) - (temp * ACTIVECAM->Xsin);

	// Might Prove Usefull one day...
	temp = (out->y * ACTIVECAM->Zcos) - (out->x * ACTIVECAM->Zsin);
	out->x = (out->x * ACTIVECAM->Zcos) + (out->y * ACTIVECAM->Zsin);
	out->y = temp;
}

void EE_RT2(D3DTLVERTEX * in, D3DTLVERTEX * out)
{

	out->sx = in->sx - ACTIVECAM->pos.x;
	out->sy = in->sy - ACTIVECAM->pos.y;
	out->sz = in->sz - ACTIVECAM->pos.z;

	register float temp;
	temp = (out->sz * ACTIVECAM->Ycos) - (out->sx * ACTIVECAM->Ysin);
	out->sx = (out->sx * ACTIVECAM->Ycos) + (out->sz * ACTIVECAM->Ysin);

	out->sz = (out->sy * ACTIVECAM->Xsin) + (temp * ACTIVECAM->Xcos);
	out->sy = (out->sy * ACTIVECAM->Xcos) - (temp * ACTIVECAM->Xsin);

	// Might Prove Usefull one day...
	temp = (out->sy * ACTIVECAM->Zcos) - (out->sx * ACTIVECAM->Zsin);
	out->sx = (out->sx * ACTIVECAM->Zcos) + (out->sy * ACTIVECAM->Zsin);
	out->sy = temp;
}

void EE_P(EERIE_3D * in, D3DTLVERTEX * out)
{
	float fZTemp;
	fZTemp = 1.f / in->z;
	out->sz = fZTemp * ProjectionMatrix._33 + ProjectionMatrix._43; //HYPERBOLIC
	out->sx = in->x * ProjectionMatrix._11 * fZTemp + ACTIVECAM->posleft;
	out->sy = in->y * ProjectionMatrix._22 * fZTemp + ACTIVECAM->postop;
	out->rhw = fZTemp;
}

void EE_P2(D3DTLVERTEX * in, D3DTLVERTEX * out)
{
	float fZTemp;
	fZTemp = 1.f / in->sz;
	out->sz = fZTemp * ProjectionMatrix._33 + ProjectionMatrix._43; //HYPERBOLIC
	out->sx = in->sx * ProjectionMatrix._11 * fZTemp + ACTIVECAM->posleft;
	out->sy = in->sy * ProjectionMatrix._22 * fZTemp + ACTIVECAM->postop;
	out->rhw = fZTemp;
}

void EE_RTP(D3DTLVERTEX * in, D3DTLVERTEX * out)
{
	//register float rhw;
	out->sx = in->sx - ACTIVECAM->pos.x;
	out->sy = in->sy - ACTIVECAM->pos.y;
	out->sz = in->sz - ACTIVECAM->pos.z;

	register float temp;
	temp = (out->sz * ACTIVECAM->Ycos) - (out->sx * ACTIVECAM->Ysin);
	out->sx = (out->sx * ACTIVECAM->Ycos) + (out->sz * ACTIVECAM->Ysin);

	out->sz = (out->sy * ACTIVECAM->Xsin) + (temp * ACTIVECAM->Xcos);
	out->sy = (out->sy * ACTIVECAM->Xcos) - (temp * ACTIVECAM->Xsin);

	// Might Prove Usefull one day...
	temp = (out->sy * ACTIVECAM->Zcos) - (out->sx * ACTIVECAM->Zsin);
	out->sx = (out->sx * ACTIVECAM->Zcos) + (out->sy * ACTIVECAM->Zsin);
	out->sy = temp;

	float fZTemp;
	fZTemp = 1.f / out->sz;
	out->sz = fZTemp * ProjectionMatrix._33 + ProjectionMatrix._43;
	out->sx = out->sx * ProjectionMatrix._11 * fZTemp + ACTIVECAM->posleft;
	out->sy = out->sy * ProjectionMatrix._22 * fZTemp + ACTIVECAM->postop;
	out->rhw = fZTemp;
}
//*************************************************************************************
//*************************************************************************************
__inline void camEE_RTP(D3DTLVERTEX * in, D3DTLVERTEX * out, EERIE_CAMERA * cam)
{
	D3DTLVERTEX tout;
	out->sx = in->sx - cam->pos.x;
	out->sy = in->sy - cam->pos.y;
	out->sz = in->sz - cam->pos.z;

	tout.sx = (out->sx * cam->Ycos) + (out->sz * cam->Ysin);
	tout.sz = (out->sz * cam->Ycos) - (out->sx * cam->Ysin);

	out->sz = (out->sy * cam->Xsin) + (tout.sz * cam->Xcos);
	out->sy = (out->sy * cam->Xcos) - (tout.sz * cam->Xsin);

	if (ACTIVECAM->Zsin == 0)
	{
		tout.sy = out->sy;
	}
	else
	{
		tout.sy = (out->sy * cam->Zcos) - (tout.sx * cam->Zsin);
		tout.sx = (tout.sx * cam->Zcos) + (out->sy * cam->Zsin);
	}

	if (out->sz <= 0.f)
	{
		out->rhw = 1.f - out->sz;
	}
	else
	{
		out->rhw = 1.f / out->sz;
	}

	tout.rhw = cam->use_focal * out->rhw;
	out->sz = out->sz * cam->Zmul;
	out->sx = cam->posleft + (tout.sx * tout.rhw);
	out->sy = cam->postop + (tout.sy * tout.rhw) ;
}

//*************************************************************************************
//*************************************************************************************
void EE_RTT(D3DTLVERTEX * in, D3DTLVERTEX * out)
{
	specialEE_RTP(in, out);
}

//*************************************************************************************
//*************************************************************************************
__inline void EERIERTPPolyCam(EERIEPOLY * ep, EERIE_CAMERA * cam)
{
	camEE_RTP(&ep->v[0], &ep->tv[0], cam);
	camEE_RTP(&ep->v[1], &ep->tv[1], cam);
	camEE_RTP(&ep->v[2], &ep->tv[2], cam);

	if (ep->type & POLY_QUAD) camEE_RTP(&ep->v[3], &ep->tv[3], cam);
}

extern float GLOBAL_LIGHT_FACTOR;
//*************************************************************************************
//*************************************************************************************

D3DCOLOR GetColorz(float x, float y, float z)
{
	EERIE_3D pos(x, y, z);
	llightsInit();
	float ffr, ffg, ffb;
	long lfr, lfg, lfb;
	float dd, dc;
	float p;
	D3DCOLOR color;

	for (long i = 0; i < TOTIOPDL; i++)
	{
		if ((IO_PDL[i]->fallstart > 10.f) && (IO_PDL[i]->fallend > 100.f))
			Insertllight(IO_PDL[i], EEDistance3D(&IO_PDL[i]->pos, &pos) - IO_PDL[i]->fallstart);
	}

	for (int i = 0; i < TOTPDL; i++)
	{
		if ((PDL[i]->fallstart > 10.f) && (PDL[i]->fallend > 100.f))
			Insertllight(PDL[i], EEDistance3D(&PDL[i]->pos, &pos) - PDL[i]->fallstart);
	}

	Preparellights(&pos);
	ffr = 0;
	ffg = 0;
	ffb = 0;

	for (long k = 0; k < MAX_LLIGHTS; k++)
	{
		EERIE_LIGHT * el = llights[k];

		if (el)
		{
			dd = EEDistance3D(&el->pos, &pos);

			if (dd < el->fallend)
			{
				if (dd <= el->fallstart)
					dc = el->intensity * GLOBAL_LIGHT_FACTOR;
				else
				{
					p = ((el->fallend - dd) * el->falldiffmul);

					if (p <= 0.f)
						dc = 0.f;
					else
						dc = p * el->intensity * GLOBAL_LIGHT_FACTOR;
				}

				dc *= 0.4f * 255.f; 
				ffr = max(ffr, el->rgb.r * dc);
				ffg = max(ffg, el->rgb.g * dc);
				ffb = max(ffb, el->rgb.b * dc);
			}
		}
	}


	EERIEPOLY * ep;
	float needy;
	ep = CheckInPoly(x, y , z, &needy);

	if (ep != NULL)
	{
		float _ffr = 0;
		float _ffg = 0;
		float _ffb = 0;
		long to;
		float div;

		if (ep->type & POLY_QUAD)
		{
			to = 4;
			div = ( 1.0f / 4 );
		}
		else
		{
			to = 3;
			div = ( 1.0f / 3 );
		}

		ApplyDynLight(ep);

		for (long i = 0; i < to; i++)
		{
			D3DCOLOR col = ep->tv[i].color;
			_ffr += (float)(long)((col >> 16) & 255);
			_ffg += (float)(long)((col >> 8) & 255);
			_ffb += (float)(long)((col) & 255);
		}

		_ffr *= div;
		_ffg *= div;
		_ffb *= div;
		float ratio, ratio2;
		ratio = EEfabs(needy - y) * ( 1.0f / 300 );
		ratio = (1.f - ratio); 
		ratio2 = 1.f - ratio;
		ffr = ffr * ratio2 + _ffr * ratio;
		ffg = ffg * ratio2 + _ffg * ratio;
		ffb = ffb * ratio2 + _ffb * ratio;
	}

	ffr = min(ffr, 255.f);
	lfr = ffr;

	ffg = min(ffg, 255.f);
	lfg = ffg;

	ffb = min(ffb, 255.f);
	lfb = ffb;
	color = (0xFF000000L | ((lfr & 255) << 16) |	((lfg & 255) << 8) | (lfb & 255));
	return color;
}

//*************************************************************************************
//*************************************************************************************

extern float GetIOHeight(INTERACTIVE_OBJ * io);
extern float GetIORadius(INTERACTIVE_OBJ * io);

long GetVertexPos(INTERACTIVE_OBJ * io, long id, EERIE_3D * pos)
{
	if (!io) return 0;

	if (id != -1)
	{
		*pos = io->obj->vertexlist3[id].v;
		return 1;
	}
	else
	{
		pos->x = io->pos.x;
		pos->y = io->pos.y + GetIOHeight(io);
		pos->z = io->pos.z;
		return 2;
	}
}

long EERIEDrawnPolys = 0;

extern EERIE_CAMERA * Kam;

//*************************************************************************************
// Checks if point (x,y) is in a 2D poly defines by ep
//*************************************************************************************
int PointIn2DPoly(EERIEPOLY * ep, float x, float y)
{
	register int i, j, c = 0;

	for (i = 0, j = 2; i < 3; j = i++)
	{
		if ((((ep->tv[i].sy <= y) && (y < ep->tv[j].sy)) ||
				((ep->tv[j].sy <= y) && (y < ep->tv[i].sy))) &&
				(x < (ep->tv[j].sx - ep->tv[i].sx) *(y - ep->tv[i].sy) / (ep->tv[j].sy - ep->tv[i].sy) + ep->tv[i].sx))
			c = !c;
	}

	if (c) return c;
	else if (ep->type & POLY_QUAD)
		for (i = 1, j = 3; i < 4; j = i++)
		{
			if ((((ep->tv[i].sy <= y) && (y < ep->tv[j].sy)) ||
					((ep->tv[j].sy <= y) && (y < ep->tv[i].sy))) &&
					(x < (ep->tv[j].sx - ep->tv[i].sx) *(y - ep->tv[i].sy) / (ep->tv[j].sy - ep->tv[i].sy) + ep->tv[i].sx))
				c = !c;
		}

	return c;
}

//*************************************************************************************
//*************************************************************************************

float PtIn2DPolyProj(EERIE_3DOBJ * obj, EERIE_FACE * ef, float x, float z)
{
	register int i, j, c = 0;

	for (i = 0, j = 2; i < 3; j = i++)
	{
		if ((((obj->vertexlist[ef->vid[i]].vert.sy <= z) && (z < obj->vertexlist[ef->vid[j]].vert.sy)) ||
				((obj->vertexlist[ef->vid[j]].vert.sy <= z) && (z < obj->vertexlist[ef->vid[i]].vert.sy))) &&
				(x < (obj->vertexlist[ef->vid[j]].vert.sx - obj->vertexlist[ef->vid[i]].vert.sx) *(z - obj->vertexlist[ef->vid[i]].vert.sy) / (obj->vertexlist[ef->vid[j]].vert.sy - obj->vertexlist[ef->vid[i]].vert.sy) + obj->vertexlist[ef->vid[i]].vert.sx))
			c = !c;
	}

	if (c)
		return obj->vertexlist[ef->vid[0]].vert.sz;
	else
		return 0.f;
}

//*************************************************************************************
//*************************************************************************************

float CEDRIC_PtIn2DPolyProjV2(EERIE_3DOBJ * obj, EERIE_FACE * ef, float x, float z)
{
	register int i, j, c = 0;

	for (i = 0, j = 2; i < 3; j = i++)
	{
		if ((((obj->vertexlist3[ef->vid[i]].vert.sy <= z) && (z < obj->vertexlist3[ef->vid[j]].vert.sy)) ||
				((obj->vertexlist3[ef->vid[j]].vert.sy <= z) && (z < obj->vertexlist3[ef->vid[i]].vert.sy))) &&
				(x < (obj->vertexlist3[ef->vid[j]].vert.sx - obj->vertexlist3[ef->vid[i]].vert.sx) *(z - obj->vertexlist3[ef->vid[i]].vert.sy) / (obj->vertexlist3[ef->vid[j]].vert.sy - obj->vertexlist3[ef->vid[i]].vert.sy) + obj->vertexlist3[ef->vid[i]].vert.sx))
			c = !c;
	}

	if (c) return obj->vertexlist3[ef->vid[0]].vert.sz;
	else return 0.f;
}

//*************************************************************************************
//*************************************************************************************
int PointIn2DPolyXZ(const EERIEPOLY * ep, float x, float z)
{
	register int i, j, c = 0, d = 0;

	for (i = 0, j = 2; i < 3; j = i++)
	{
		if ((((ep->v[i].sz <= z) && (z < ep->v[j].sz)) ||
				((ep->v[j].sz <= z) && (z < ep->v[i].sz))) &&
				(x < (ep->v[j].sx - ep->v[i].sx) *(z - ep->v[i].sz) / (ep->v[j].sz - ep->v[i].sz) + ep->v[i].sx))
			c = !c;
	}

	if (ep->type & POLY_QUAD)
		for (i = 1, j = 3; i < 4; j = i++)
		{
			if ((((ep->v[i].sz <= z) && (z < ep->v[j].sz)) ||
					((ep->v[j].sz <= z) && (z < ep->v[i].sz))) &&
					(x < (ep->v[j].sx - ep->v[i].sx) *(z - ep->v[i].sz) / (ep->v[j].sz - ep->v[i].sz) + ep->v[i].sx))
				d = !d;
		}

	return c + d;
}

//*************************************************************************************
// Sets the target of a camera...
//*************************************************************************************

void SetTargetCamera(EERIE_CAMERA * cam, float x, float y, float z)
{
	if ((cam->pos.x == x) && (cam->pos.y == y) && (cam->pos.z == z)) return;

	cam->angle.a = (degrees(GetAngle(cam->pos.y, cam->pos.z, y, cam->pos.z + TRUEDistance2D(x, z, cam->pos.x, cam->pos.z)))); //alpha entre orgn et dest;
	cam->angle.b = (180.f + degrees(GetAngle(cam->pos.x, cam->pos.z, x, z))); //beta entre orgn et dest;
	cam->angle.g = 0.f;
}


//*************************************************************************************
//*************************************************************************************

int BackFaceCull2D(D3DTLVERTEX * tv)
{

	if ((tv[0].sx - tv[1].sx)*(tv[2].sy - tv[1].sy) - (tv[0].sy - tv[1].sy)*(tv[2].sx - tv[1].sx) > 0.f)
		return 0;
	else return 1;
}

extern EERIE_CAMERA raycam;

//*************************************************************************************
//*************************************************************************************

static int RayIn3DPolyNoCull(EERIE_3D * orgn, EERIE_3D * dest, EERIEPOLY * epp) {

	EERIEPOLY ep;
	memcpy(&ep, epp, sizeof(EERIEPOLY));
	raycam.pos = *orgn;
	SetTargetCamera(&raycam, dest->x, dest->y, dest->z);
	SP_PrepareCamera(&raycam);
	EERIERTPPolyCam(&ep, &raycam);

	if (PointIn2DPoly(&ep, 320.f, 320.f))	return 1;

	return 0;
}

int EERIELaunchRay3(EERIE_3D * orgn, EERIE_3D * dest,  EERIE_3D * hit, EERIEPOLY * epp, long flag)
{
	float x, y, z; //current ray pos
	float dx, dy, dz; // ray incs
	float adx, ady, adz; // absolute ray incs
	float ix, iy, iz;
	long lpx, lpz;
	long voidlast;
	long px, pz;
	EERIEPOLY * ep;
	EERIE_BKG_INFO * eg;
	float pas = 1.5f;

	long iii = 0;
	float maxstepp = 20000.f / pas;
	hit->x = x = orgn->x;
	hit->y = y = orgn->y;
	hit->z = z = orgn->z;

	voidlast = 0;
	lpx = lpz = -1;
	dx = (dest->x - orgn->x);
	adx = EEfabs(dx);
	dy = (dest->y - orgn->y);
	ady = EEfabs(dy);
	dz = (dest->z - orgn->z);
	adz = EEfabs(dz);

	if ((adx >= ady) && (adx >= adz))
	{
		if (adx != dx) ix = -1.f * pas;
		else ix = 1.f * pas;

		iy = dy / (adx / pas);
		iz = dz / (adx / pas);
	}
	else if ((ady >= adx) && (ady >= adz))
	{
		if (ady != dy) iy = -1.f * pas;
		else iy = 1.f * pas;

		ix = dx / (ady / pas);
		iz = dz / (ady / pas);
	}
	else
	{
		if (adz != dz) iz = -1.f * pas;
		else iz = 1.f * pas;

		ix = dx / (adz / pas);
		iy = dy / (adz / pas);
	}

	for (;;)
	{
		x += ix;
		y += iy;
		z += iz;

		if ((ix == -1.f * pas) && (x <= dest->x))
		{
			hit->x = x;
			hit->y = y;
			hit->z = z;
			return 0;
		}

		if ((ix == 1.f * pas) && (x >= dest->x))
		{
			hit->x = x;
			hit->y = y;
			hit->z = z;
			return 0;
		}

		if ((iy == -1.f * pas) && (y <= dest->y))
		{
			hit->x = x;
			hit->y = y;
			hit->z = z;
			return 0;
		}

		if ((iy == 1.f * pas) && (y >= dest->y))
		{
			hit->x = x;
			hit->y = y;
			hit->z = z;
			return 0;
		}

		if ((iz == -1.f * pas) && (z <= dest->z))
		{
			hit->x = x;
			hit->y = y;
			hit->z = z;
			return 0;
		}

		if ((iz == 1.f * pas) && (z >= dest->z))
		{
			hit->x = x;
			hit->y = y;
			hit->z = z;
			return 0;
		}

		iii++;

		if (iii > maxstepp)
		{
			hit->x = x;
			hit->y = y;
			hit->z = z;
			epp = NULL;
			return -1;
		}

		px = (long)(x * ACTIVEBKG->Xmul);
		pz = (long)(z * ACTIVEBKG->Zmul);

		if (px > ACTIVEBKG->Xsize - 1)
		{
			hit->x = x;
			hit->y = y;
			hit->z = z;
			epp = NULL;
			return -1;
		}

		if (px < 0)
		{
			hit->x = x;
			hit->y = y;
			hit->z = z;
			epp = NULL;
			return -1;
		}

		if (pz > ACTIVEBKG->Zsize - 1)
		{
			hit->x = x;
			hit->y = y;
			hit->z = z;
			epp = NULL;
			return -1;
		}

		if (pz < 0)
		{
			hit->x = x;
			hit->y = y;
			hit->z = z;
			epp = NULL;
			return -1;
		}

		if (((lpx == px) && (lpz == pz)) && voidlast)
		{
		}
		else
		{
			lpx = px;
			lpz = pz;
			voidlast = !flag;
			long jx1 = px - 1;
			long jx2 = px + 1;
			long jz1 = pz - 1;
			long jz2 = pz + 1;

			if (jx1 < 0) jx1 = 0;

			if (jx2 < 0) jx2 = 0;

			if (jz1 < 0) jz1 = 0;

			if (jz2 < 0) jz2 = 0;

			if (jx1 > ACTIVEBKG->Xsize - 1) jx1 = ACTIVEBKG->Xsize - 1;

			if (jx2 > ACTIVEBKG->Xsize - 1) jx2 = ACTIVEBKG->Xsize - 1;

			if (jz1 > ACTIVEBKG->Zsize - 1) jz1 = ACTIVEBKG->Zsize - 1;

			if (jz2 > ACTIVEBKG->Zsize - 1) jz2 = ACTIVEBKG->Zsize - 1;


			eg = &ACTIVEBKG->Backg[px+pz*ACTIVEBKG->Xsize];

			if (eg->nbpoly == 0)
			{
				hit->x = x;
				hit->y = y;
				hit->z = z;
				return 1;
			}

			for (pz = jz1; pz < jz2; pz++)
				for (px = jx1; px < jx2; px++)
				{
					eg = &ACTIVEBKG->Backg[px+pz*ACTIVEBKG->Xsize];

					for (long k = 0; k < eg->nbpoly; k++)
					{
						ep = &eg->polydata[k];

						if (!(ep->type & POLY_TRANS))
						{
							if ((y >= ep->min.y - 10.f) && (y <= ep->max.y + 10.f)
									&& (x >= ep->min.x - 10.f) && (x <= ep->max.x + 10.f)
									&& (z >= ep->min.z - 10.f) && (z <= ep->max.z + 10.f))
							{
								voidlast = 0;

								if (RayIn3DPolyNoCull(orgn, dest, ep))
								{
									hit->x = x;
									hit->y = y;
									hit->z = z;

									if (ep == epp) return 0;

									epp = ep;
									return 1;
								}
							}
						}

					}
				}
		}
	}
}

//*************************************************************************************
// Computes the visibility from a point to another... (sort of...)
//*************************************************************************************
bool Visible(EERIE_3D * orgn, EERIE_3D * dest, EERIEPOLY * epp, EERIE_3D * hit)
{
	float			x, y, z; //current ray pos
	float			dx, dy, dz; // ray incs
	float			adx, ady, adz; // absolute ray incs
	float			ix, iy, iz;
	long			px, pz;
	EERIEPOLY	*	ep;
	EERIE_BKG_INFO	* eg;
	float			pas			=	35.f;
	EERIE_3D		found_hit;
	EERIEPOLY	*	found_ep	=	NULL;
	float iter, t;

	found_hit.x = found_hit.y = found_hit.z = 0.f;

	x	=	orgn->x;
	y	=	orgn->y;
	z	=	orgn->z;

	float			distance;
	float			nearest		=	distance	=	EEDistance3D(orgn, dest);

	if (distance < pas) pas	=	distance * ( 1.0f / 2 );

	dx	=	(dest->x - orgn->x);
	adx	=	EEfabs(dx);
	dy	=	(dest->y - orgn->y);
	ady	=	EEfabs(dy);
	dz	=	(dest->z - orgn->z);
	adz	=	EEfabs(dz);

	if ((adx >= ady) && (adx >= adz))
	{
		if (adx != dx)
		{
			ix = -pas;
		}
		else
		{
			ix = pas;
		}

		iter = adx / pas;
		t = 1.f / (iter);
		iy = dy * t;
		iz = dz * t;
	}
	else if ((ady >= adx) && (ady >= adz))
	{
		if (ady != dy)
		{
			iy = -pas;
		}
		else
		{
			iy = pas;
		}

		iter = ady / pas;
		t = 1.f / (iter);
		ix = dx * t;
		iz = dz * t;
	}
	else
	{
		if (adz != dz)
		{
			iz = -pas;
		}
		else
		{
			iz = pas;
		}

		iter = adz / pas;
		t = 1.f / (iter);
		ix = dx * t;
		iy = dy * t;
	}

	float dd;
	x -= ix;
	y -= iy;
	z -= iz;

	while (iter > 0.f)
	{
		iter -= 1.f;
		x += ix;
		y += iy;
		z += iz;

		px = (long)(x * ACTIVEBKG->Xmul);
		pz = (long)(z * ACTIVEBKG->Zmul);

		if (px > ACTIVEBKG->Xsize - 1)		goto fini;
		else if (px < 0)					goto fini;

		if (pz > ACTIVEBKG->Zsize - 1)		goto fini;
		else if (pz < 0)					goto fini;

		{
			eg = &ACTIVEBKG->Backg[px+pz*ACTIVEBKG->Xsize];

			for (long k = 0; k < eg->nbpolyin; k++)
			{
				ep = eg->polyin[k];

				if (ep)
					if ((ep->min.y - pas < y) && (ep->max.y + pas > y))
						if ((ep->min.x - pas < x) && (ep->max.x + pas > x))
							if ((ep->min.z - pas < z) && (ep->max.z + pas > z))
							{
								if (RayCollidingPoly(orgn, dest, ep, hit))
								{
									dd = EEDistance3D(orgn, hit);

									if (dd < nearest)
									{
										nearest		=	dd;
										found_ep	=	ep;
										found_hit.x	=	hit->x;
										found_hit.y	=	hit->y;
										found_hit.z	=	hit->z;
									}
								}
							}
			}
		}
	}

fini:
	;

	if (!found_ep) return true;

	if (found_ep == epp) return true;

	hit->x	=	found_hit.x;
	hit->y	=	found_hit.y;
	hit->z	=	found_hit.z;

	return false;
}


//*************************************************************************************
// Counts total number of polys in a background
//*************************************************************************************
long BKG_CountPolys(EERIE_BACKGROUND * eb)
{
	long count = 0;
	EERIE_BKG_INFO * eg;

	for (long i = 0; i < eb->Xsize * eb->Zsize; i++)
	{
		eg = &eb->Backg[i];
		count += eg->nbpoly;
	}

	return count;
}

//*************************************************************************************
// Counts number of ignored polys in a background
//*************************************************************************************

long BKG_CountIgnoredPolys(EERIE_BACKGROUND * eb)
{
	long count = 0;
	EERIE_BKG_INFO * eg;
	EERIEPOLY * pol = NULL;

	for (long i = 0; i < eb->Xsize * eb->Zsize; i++)
	{
		eg = &eb->Backg[i];

		for (long k = 0; k < eg->nbpoly; k++)
		{
			pol = &eg->polydata[k];

			if (pol->type & POLY_IGNORE)
				count++;
		}
	}

	return count;
}
//*************************************************************************************
// Releases BKG_INFO from a tile
//*************************************************************************************
void ReleaseBKG_INFO(EERIE_BKG_INFO * eg)
{
	if (eg->polydata) free(eg->polydata);

	eg->polydata = NULL;

	if (eg->polyin) free(eg->polyin);

	eg->polyin = NULL;
	eg->nbpolyin = 0;
	memset(eg, 0, sizeof(EERIE_BKG_INFO));
}

void ARX_PORTALS_SWAP_EPs(short px, short py, short ep_idx, short ep_idx2)
{
	if (!portals) return;

	for (long room_num = 0; room_num <= portals->nb_rooms; room_num++)
	{
		for (long  lll = 0; lll < portals->room[room_num].nb_polys; lll++)
		{
			if ((portals->room[room_num].epdata[lll].px == px)
					&& (portals->room[room_num].epdata[lll].py == py))
			{
				if (portals->room[room_num].epdata[lll].idx == ep_idx)
					portals->room[room_num].epdata[lll].idx = ep_idx2;
				else if (portals->room[room_num].epdata[lll].idx == ep_idx2)
					portals->room[room_num].epdata[lll].idx = ep_idx;
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////
// Sorts Background tile polys from roof to top
/////////////////////////////////////////////////////////////////////////////////////
void BKG_VerticalReOrder(EERIE_BACKGROUND * eb)
{
	return;
	EERIE_BKG_INFO * eg;
	EERIEPOLY * ep;
	EERIEPOLY * ep2;
	EERIEPOLY tep;

	for (short j = 0; j < eb->Zsize; j++)
		for (short i = 0; i < eb->Xsize; i++)
		{
			eg = &eb->Backg[i+j*eb->Xsize];

			if (eg->nbpoly > 1)
			{
				bool Reordered = false;

				while (!Reordered)
				{
					Reordered = true;

					for (short k = 0; k < eg->nbpoly - 1; k++)
					{
						ep = &eg->polydata[k];
						ep2 = &eg->polydata[k+1];

						if (ep->center.y < ep2->center.y)
						{
							ARX_PORTALS_SWAP_EPs(i, j, k, k + 1);
							memcpy(&tep, ep2, sizeof(EERIEPOLY));
							memcpy(ep2, ep, sizeof(EERIEPOLY));
							memcpy(ep, &tep, sizeof(EERIEPOLY));
							Reordered = false;
						}
					}
				}
			}
		}
}

//*************************************************************************************
//*************************************************************************************

void AddAData(_ANCHOR_DATA * ad, long linked)
{
	for (long i=0;i<ad->nblinked;i++)
		if (ad->linked[i] == linked) return;

	ad->linked = (long *)realloc(ad->linked, sizeof(long) * (ad->nblinked + 1));

	if (!ad->linked) HERMES_Memory_Emergency_Out();

	ad->linked[ad->nblinked] = linked;
	ad->nblinked++;
}

void UpdateIORoom(INTERACTIVE_OBJ * io)
{
	EERIE_3D pos;
	Vector_Copy(&pos, &io->pos);
	pos.y -= 60.f;

	long roo = ARX_PORTALS_GetRoomNumForPosition(&pos, 2);
	ARX_CHECK_SHORT(roo);

	if (roo >= 0)
		io->room = ARX_CLEAN_WARN_CAST_SHORT(roo);

	io->room_flags &= ~1;
}

bool GetRoomCenter(long room_num, EERIE_3D * center)
{
	if (!portals) return false;

	if (room_num > portals->nb_rooms) return false;

	if (portals->room[room_num].nb_polys <= 0) return false;

	EERIE_3D_BBOX bbox;
	bbox.min.x = 99999999.f;
	bbox.min.y = 99999999.f;
	bbox.min.z = 99999999.f;
	bbox.max.x = -99999999.f;
	bbox.max.y = -99999999.f;
	bbox.max.z = -99999999.f;

	for (long  lll = 0; lll < portals->room[room_num].nb_polys; lll++)
	{
		FAST_BKG_DATA * feg;
		feg = &ACTIVEBKG->fastdata[portals->room[room_num].epdata[lll].px][portals->room[room_num].epdata[lll].py];
		EERIEPOLY * ep = &feg->polydata[portals->room[room_num].epdata[lll].idx];
		bbox.min.x = min(bbox.min.x, ep->center.x);
		bbox.min.y = min(bbox.min.y, ep->center.y);
		bbox.min.z = min(bbox.min.z, ep->center.z);
		bbox.max.x = max(bbox.max.x, ep->center.x);
		bbox.max.y = max(bbox.max.y, ep->center.y);
		bbox.max.z = max(bbox.max.z, ep->center.z);
	}

	*center = (bbox.max + bbox.min) * ( 1.0f / 2 );

	Vector_Copy(&portals->room[room_num].center, center);
	portals->room[room_num].radius = EEDistance3D(center, &bbox.max);
	return true;
}

ROOM_DIST_DATA * RoomDistance = NULL;
long NbRoomDistance = 0;

static void SetRoomDistance(long i, long j, float val, const EERIE_3D * p1, const EERIE_3D * p2) {
	
	if((i < 0) || (j < 0) || (i >= NbRoomDistance) || (j >= NbRoomDistance) || !RoomDistance) {
		return;
	}
	
	long offs = i + j * NbRoomDistance;
	
	if (p1) Vector_Copy(&RoomDistance[offs].startpos, p1);

	if (p2) Vector_Copy(&RoomDistance[offs].endpos, p2);

	RoomDistance[offs].distance = val;
}
static float GetRoomDistance(long i, long j, EERIE_3D * p1, EERIE_3D * p2)
{
	if ((i < 0) || (j < 0) || (i >= NbRoomDistance) || (j >= NbRoomDistance))
		return -1.f;

	long offs = i + j * NbRoomDistance;

	if (p1) Vector_Copy(p1, &RoomDistance[offs].startpos);

	if (p2) Vector_Copy(p2, &RoomDistance[offs].endpos);

	return (RoomDistance[offs].distance);
}
float SP_GetRoomDist(EERIE_3D * pos, EERIE_3D * c_pos, long io_room, long Cam_Room)
{
	float dst = EEDistance3D(pos, c_pos);

	if (dst < 150.f) return dst;

	if ((!portals) || (!RoomDistance))
		return dst;

	long Room=io_room;

	if (Room >= 0)
	{
		EERIE_3D p1, p2;
		float v = GetRoomDistance(Cam_Room, Room, &p1, &p2);

		if (v > 0.f)
		{
			v += EEDistance3D(pos, &p2);
			v += EEDistance3D(c_pos, &p1);
			return v;
		}
	}

	return dst;
}
void ComputeRoomDistance()
{
	if (RoomDistance)
		free(RoomDistance);

	RoomDistance = NULL;
	NbRoomDistance = 0;

	if (portals == NULL) return;

	NbRoomDistance = portals->nb_rooms + 1;
	RoomDistance =
		(ROOM_DIST_DATA *)malloc(sizeof(ROOM_DIST_DATA) * (NbRoomDistance) * (NbRoomDistance));

	for (long n = 0; n < NbRoomDistance; n++)
		for (long m = 0; m < NbRoomDistance; m++)
			SetRoomDistance(m, n, -1.f, NULL, NULL);

	long nb_anchors = NbRoomDistance + (portals->nb_total * 9);
	_ANCHOR_DATA * ad = (_ANCHOR_DATA *)malloc(sizeof(_ANCHOR_DATA) * nb_anchors);

	if (!ad) HERMES_Memory_Emergency_Out();

	memset(ad, 0, sizeof(_ANCHOR_DATA)*nb_anchors);

	void ** ptr = NULL;
	ptr = (void **)malloc(sizeof(void *) * nb_anchors);
	memset(ptr, 0, sizeof(void *)*nb_anchors);


	for (long i = 0; i < NbRoomDistance; i++)
	{
		GetRoomCenter(i, &ad[i].pos);
		ptr[i] = (void *)&portals->room[i];
	}

	long curpos = NbRoomDistance;

	for (int i = 0; i < portals->nb_total; i++)
	{
		// Add 4 portal vertices
		for (int nn = 0; nn < 4; nn++)
		{
			ad[curpos].pos.x = portals->portals[i].poly.v[nn].sx;
			ad[curpos].pos.y = portals->portals[i].poly.v[nn].sy;
			ad[curpos].pos.z = portals->portals[i].poly.v[nn].sz;
			ptr[curpos] = (void *)&portals->portals[i];
			curpos++;
		}

		// Add center;
		ad[curpos].pos = portals->portals[i].poly.center;
		ptr[curpos] = (void *)&portals->portals[i];
		curpos++;

		// Add V centers;
		for (int nn = 0, nk = 3; nn < 4; nk = nn++)
		{
			ad[curpos].pos.x = (portals->portals[i].poly.v[nn].sx + portals->portals[i].poly.v[nk].sx) * ( 1.0f / 2 );
			ad[curpos].pos.y = (portals->portals[i].poly.v[nn].sy + portals->portals[i].poly.v[nk].sy) * ( 1.0f / 2 );
			ad[curpos].pos.z = (portals->portals[i].poly.v[nn].sz + portals->portals[i].poly.v[nk].sz) * ( 1.0f / 2 );
			ptr[curpos] = (void *)&portals->portals[i];
			curpos++;
		}
	}

	// Link Room Centers to all its Room portals...
	for (int i = 0; i <= portals->nb_rooms; i++)
	{
		for (long j = 0; j < portals->nb_total; j++)
		{
			if ((portals->portals[j].room_1 == i)
					||	(portals->portals[j].room_2 == i))
			{
				for (long tt = 0; tt < nb_anchors; tt++)
				{
					if (ptr[tt] == (void *)(&portals->portals[j]))
					{
						AddAData(&ad[tt], i);
						AddAData(&ad[i], tt);
					}
				}
			}
		}
	}

	// Link All portals of a room to all other portals of that room
	for (int i = 0; i <= portals->nb_rooms; i++)
	{
		for (long j = 0; j < portals->nb_total; j++)
		{
			if (((portals->portals[j].room_1 == i)
					|| (portals->portals[j].room_2 == i)))
				for (long jj = 0; jj < portals->nb_total; jj++)
				{
					if ((jj != j)
							&&	((portals->portals[jj].room_1 == i)
								 ||	(portals->portals[jj].room_2 == i)))
					{
						long p1 = -1;
						long p2 = -1;

						for (long tt = 0; tt < nb_anchors; tt++)
						{
							if (ptr[tt] == (void *)(&portals->portals[jj]))
								p1 = tt;

							if (ptr[tt] == (void *)(&portals->portals[j]))
								p2 = tt;
						}

						if ((p1 >= 0) && (p2 >= 0))
						{
							AddAData(&ad[p1], p2);
							AddAData(&ad[p2], p1);
						}
					}
				}
		}
	}

	PathFinder pathfinder(NbRoomDistance, ad, 0, NULL);

	for (int i = 0; i < NbRoomDistance; i++)
		for (long j = 0; j < NbRoomDistance; j++)
		{
			if (i == j)
			{
				SetRoomDistance(i, j, -1, NULL, NULL);
				continue;
			}
			
			PathFinder::Result rl;

			bool found = pathfinder.move(i, j, rl);

			if (found)
			{
				float dist = 0.f;

				for (size_t id = 1; id < rl.size() - 1; id++)
				{
					dist += TRUEEEDistance3D(&ad[rl[id-1]].pos, &ad[rl[id]].pos);
				}

				if (dist < 0.f) dist = 0.f;

				float old = GetRoomDistance(i, j, NULL, NULL);

				if (((dist < old) || (old < 0.f)) && rl.size() >= 2)
					SetRoomDistance(i, j, dist, &ad[rl[1]].pos, &ad[rl[rl.size()-2]].pos);
			}

		}

	// Don't use this for contiguous rooms !
	for (int i = 0; i < portals->nb_total; i++)
	{
		SetRoomDistance(portals->portals[i].room_1, portals->portals[i].room_2, -1, NULL, NULL);
		SetRoomDistance(portals->portals[i].room_2, portals->portals[i].room_1, -1, NULL, NULL);
	}

	// Release our temporary Pathfinder data
	for (int ii = 0; ii < nb_anchors; ii++)
	{
		if (ad[ii].nblinked)
		{
			free(ad[ii].linked);
		}
	}

	free(ad);
}

// Clears a background of its infos
void ClearBackground(EERIE_BACKGROUND * eb)
{
	EERIE_BKG_INFO * eg;

	if (eb == NULL) return;

	AnchorData_ClearAll(eb);

	if (eb->minmax) free(eb->minmax);

	eb->minmax = NULL;

	for (long i = 0; i < eb->Xsize * eb->Zsize; i++)
	{
		eg = &eb->Backg[i];
		ReleaseBKG_INFO(eg);
	}

	if (eb->Backg) free(eb->Backg);

	eb->Backg = NULL;

	if (RoomDistance)
	{
		free(RoomDistance);
		RoomDistance = NULL;
		NbRoomDistance = 0;
	}
}

//*************************************************************************************
//*************************************************************************************
int InitBkg(EERIE_BACKGROUND * eb, short sx, short sz, short Xdiv, short Zdiv)
{

	EERIE_BKG_INFO * eg;

	if (eb == NULL) return 0;

	if (eb->exist)
	{
		EERIE_PORTAL_Release();
		ClearBackground(eb);
	}

	eb->exist = 1;
	eb->anchors = NULL;
	eb->nbanchors = 0;
	eb->Xsize = sx;
	eb->Zsize = sz;

	if (Xdiv < 0) Xdiv = 1;

	if (Zdiv < 0) Xdiv = 1;

	eb->Xdiv = Xdiv;
	eb->Zdiv = Zdiv;
	eb->Xmul = 1.f / (float)eb->Xdiv;
	eb->Zmul = 1.f / (float)eb->Zdiv;

	//todo free
	eb->Backg = (EERIE_BKG_INFO *)malloc(sizeof(EERIE_BKG_INFO) * sx * sz);

	if (!eb->Backg) HERMES_Memory_Emergency_Out();

	memset(eb->Backg, 0, sizeof(EERIE_BKG_INFO)*sx * sz);

	for (int i = 0; i < eb->Xsize * eb->Zsize; i++)
	{
		eg = &eb->Backg[i];
		eg->treat = 0;
		eg->nothing = 1;
		eg->nbianchors = 0;
		eg->ianchors = NULL;
	}

	for (long j = 0; j < eb->Zsize; j++)
		for (int i = 0; i < eb->Xsize; i++)
		{
			FAST_BKG_DATA * feg = &eb->fastdata[i][j];
			memset(feg, 0, sizeof(FAST_BKG_DATA));
		}

	//todo free
	eb->minmax = (EERIE_SMINMAX *)malloc(sizeof(EERIE_SMINMAX) * eb->Zsize);

	if (!eb->minmax) HERMES_Memory_Emergency_Out();

	for (int i = 0; i < eb->Zsize; i++)
	{
		eb->minmax[i].min = 9999;
		eb->minmax[i].max = -1;
	}

	return 1;
}
//*************************************************************************************
// Checks for angular difference between normals
//*************************************************************************************
bool LittleAngularDiff(EERIE_3D * norm, EERIE_3D * norm2)
{
	if (Distance3D(norm->x, norm->y, norm->z,
				   norm2->x, norm2->y, norm2->z) < 1.41421f) return true;

	return false;
}
extern void ARX_PrepareBackgroundNRMLs();

//*************************************************************************************
//*************************************************************************************

void PrepareBackgroundNRMLs()
{
	ARX_PrepareBackgroundNRMLs();
	return;
	long i, j, k, mai, maj, mii, mij;
	long i2, j2, k2;
	EERIE_BKG_INFO * eg;
	EERIE_BKG_INFO * eg2;
	EERIEPOLY * ep;
	EERIEPOLY * ep2;
	EERIE_3D nrml;
	EERIE_3D cur_nrml;
	float count;
	long nbvert;
	long nbvert2;

	for (j = 0; j < ACTIVEBKG->Zsize; j++)
		for (i = 0; i < ACTIVEBKG->Xsize; i++)
		{
			eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

			for (long l = 0; l < eg->nbpoly; l++)
			{
				ep = &eg->polydata[l];

				if (ep->type & POLY_QUAD) nbvert = 4;
				else nbvert = 3;

				for (k = 0; k < nbvert; k++)
				{
					float ttt = 1.f;

					if (k == 3)
					{
						nrml = ep->norm2;
						count = 1.f;
					}
					else if ((k > 0) && (nbvert > 3))
					{
						nrml = (ep->norm + ep->norm2) * ( 1.0f / 2 );
						count = 1.f; 
					}
					else
					{
						nrml = ep->norm;
						count = 1.f;
					}

					cur_nrml = nrml * ttt;
					
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

							for (long kr = 0; kr < eg2->nbpoly; kr++)
							{
								ep2 = &eg2->polydata[kr];

								if (ep2->type & POLY_QUAD) nbvert2 = 4;
								else nbvert2 = 3;

								if (ep != ep2)

									for (k2 = 0; k2 < nbvert2; k2++)
									{
										if ((EEfabs(ep2->v[k2].sx - ep->v[k].sx) < 2.f)
												&&	(EEfabs(ep2->v[k2].sy - ep->v[k].sy) < 2.f)
												&&	(EEfabs(ep2->v[k2].sz - ep->v[k].sz) < 2.f))
										{
											if (k2 == 3)
											{
												if (LittleAngularDiff(&cur_nrml, &ep2->norm2))
												{
													nrml += ep2->norm2;
													count += 1.f;
													nrml += cur_nrml;
													count += 1.f;
												}
												else
												{
													ep->type |= POLY_ANGULAR;
													ep2->type |= POLY_ANGULAR;
													ep2->type |= POLY_ANGULAR_IDX3;
												}
											}
											else if ((k2 > 0) && (nbvert2 > 3))
											{
												EERIE_3D tnrml = (ep2->norm + ep2->norm2) * ( 1.0f / 2 );

												if (LittleAngularDiff(&cur_nrml, &tnrml))
												{
													nrml += tnrml; 
													count += 1; 
												}
												else
												{
													ep->type |= POLY_ANGULAR;
													ep2->type |= POLY_ANGULAR;

													if (k2 == 1)
														ep2->type |= POLY_ANGULAR_IDX1;
													else
														ep2->type |= POLY_ANGULAR_IDX2;
												}
											}
											else
											{
												if (LittleAngularDiff(&cur_nrml, &ep2->norm))
												{
													nrml += ep2->norm;
													count += 1.f;
												}
												else
												{
													ep->type |= POLY_ANGULAR;
													ep2->type |= POLY_ANGULAR;
													ep2->type |= POLY_ANGULAR_IDX0;
												}
											}
										}
									}
							}
						}

					count = 1.f / count;
					ep->tv[k].sx = nrml.x * count;
					ep->tv[k].sy = nrml.y * count;
					ep->tv[k].sz = nrml.z * count;
				}
			}
		}

	for (j = 0; j < ACTIVEBKG->Zsize; j++)
		for (i = 0; i < ACTIVEBKG->Xsize; i++)
		{
			eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

			for (long l = 0; l < eg->nbpoly; l++)
			{
				ep = &eg->polydata[l];

				if (ep->type & POLY_QUAD) nbvert = 4;
				else nbvert = 3;

				for (k = 0; k < nbvert; k++)
				{
					ep->nrml[k].x = ep->tv[k].sx;
					ep->nrml[k].y = ep->tv[k].sy;
					ep->nrml[k].z = ep->tv[k].sz;
				}

				float dist = 0.f;

				for (long ii = 0; ii < nbvert; ii++)
				{
					dist = max(dist, TRUEEEDistance3D((EERIE_3D *)&ep->v[ii], &ep->center));
				}

				ep->v[0].rhw = dist;
			}
		}

}

//*************************************************************************************
//*************************************************************************************

void DeclareEGInfo(float x, float z)
{
	long posx = x * ACTIVEBKG->Xmul;

	if (posx < 0) return;
	else if (posx >= ACTIVEBKG->Xsize) return;

	long posz = (long)(float)(z * ACTIVEBKG->Zmul);

	if (posz < 0) return;
	else if (posz >= ACTIVEBKG->Zsize) return;

	EERIE_BKG_INFO * eg;
	eg = &ACTIVEBKG->Backg[posx+posz*ACTIVEBKG->Xsize];
	eg->nothing = 0;
}

void EERIEPOLY_Add_PolyIn(EERIE_BKG_INFO * eg, EERIEPOLY * ep)
{
	for (long i = 0; i < eg->nbpolyin; i++)
	{
		if (eg->polyin[i] == ep) return;
	}

	eg->polyin = (EERIEPOLY **)realloc(eg->polyin, sizeof(EERIEPOLY) * (eg->nbpolyin + 1));

	if (!eg->polyin) HERMES_Memory_Emergency_Out();

	eg->polyin[eg->nbpolyin] = ep;
	eg->nbpolyin++;
}

bool PointInBBox(EERIE_3D * point, EERIE_2D_BBOX * bb)
{
	if ((point->x > bb->max.x)
			||	(point->x < bb->min.x)
			||	(point->z > bb->max.y)
			||	(point->z < bb->min.y)
	   )
		return false;

	return true;
}

void EERIEPOLY_Compute_PolyIn()
{
	EERIE_BKG_INFO * eg;
	EERIE_BKG_INFO * eg2;
	EERIEPOLY * ep2;

	long ii, ij;
	long ai, aj;
	long nbvert;

	for (long j = 0; j < ACTIVEBKG->Zsize; j++)
		for (long i = 0; i < ACTIVEBKG->Xsize; i++)
		{
			eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

			if (eg->polyin) free(eg->polyin);

			eg->polyin = NULL;
			eg->nbpolyin = 0;

			ii = max(i - 2, 0L);
			ij = max(j - 2, 0L);
			ai = min(i + 2, ACTIVEBKG->Xsize - 1L);
			aj = min(j + 2, ACTIVEBKG->Zsize - 1L);

			EERIE_2D_BBOX bb;
			bb.min.x = (float)i * ACTIVEBKG->Xdiv - 10;
			bb.max.x = (float)bb.min.x + ACTIVEBKG->Xdiv + 20;
			bb.min.y = (float)j * ACTIVEBKG->Zdiv - 10;
			bb.max.y = (float)bb.min.y + ACTIVEBKG->Zdiv + 20;
			EERIE_3D bbcenter;
			bbcenter.x = (bb.min.x + bb.max.x) * ( 1.0f / 2 );
			bbcenter.z = (bb.min.y + bb.max.y) * ( 1.0f / 2 );

			for (long cj = ij; cj < aj; cj++)
				for (long ci = ii; ci < ai; ci++)
				{

					eg2 = &ACTIVEBKG->Backg[ci+cj*ACTIVEBKG->Xsize];

					for (long l = 0; l < eg2->nbpoly; l++)
					{

						ep2 = &eg2->polydata[l];

						if (Distance2D(bbcenter.x, bbcenter.z, ep2->center.x, ep2->center.z) > 120.f)
							continue;

						if (ep2->type & POLY_QUAD) nbvert = 4;
						else nbvert = 3;

						if (PointInBBox(&ep2->center, &bb))
						{
							EERIEPOLY_Add_PolyIn(eg, ep2);
						}
						else
							for (long k = 0; k < nbvert; k++)
							{

								if (PointInBBox((EERIE_3D *)&ep2->v[k], &bb))
								{
									EERIEPOLY_Add_PolyIn(eg, ep2);
									break;
								}
								else
								{
									EERIE_3D pt;
									pt.x = (ep2->v[k].sx + ep2->center.x) * ( 1.0f / 2 );
									pt.y = (ep2->v[k].sy + ep2->center.y) * ( 1.0f / 2 );
									pt.z = (ep2->v[k].sz + ep2->center.z) * ( 1.0f / 2 );

									if (PointInBBox((EERIE_3D *)&pt, &bb))
									{
										EERIEPOLY_Add_PolyIn(eg, ep2);
										break;
									}
								}
							}
					}
				}

			if (eg->nbpolyin) eg->nothing = 0;
			else eg->nothing = 1;
		}

	for (int j = 0; j < ACTIVEBKG->Zsize; j++)
		for (long i = 0; i < ACTIVEBKG->Xsize; i++)
		{
			eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];
			eg->tile_miny = 999999999.f;
			eg->tile_maxy = -999999999.f;

			for (long kk = 0; kk < eg->nbpolyin; kk++)
			{
				EERIEPOLY * ep = eg->polyin[kk];
				eg->tile_miny = min(eg->tile_miny, ep->min.y);
				eg->tile_maxy = max(eg->tile_maxy, ep->max.y);
			}

			FAST_BKG_DATA * fbd = &ACTIVEBKG->fastdata[i][j];
			fbd->treat = eg->treat;
			fbd->nothing = eg->nothing;
			fbd->nbpoly = eg->nbpoly;
			fbd->nbianchors = eg->nbianchors;
			fbd->nbpolyin = eg->nbpolyin;
			fbd->frustrum_miny = eg->frustrum_miny;
			fbd->frustrum_maxy = eg->frustrum_maxy;
			fbd->polydata = eg->polydata;
			fbd->polyin = eg->polyin;
			fbd->ianchors = eg->ianchors;
		}
}

float GetTileMinY(long i, long j)
{
	float minf = 9999999999.f;
	EERIE_BKG_INFO * eg;
	eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

	for (long kk = 0; kk < eg->nbpolyin; kk++)
	{
		EERIEPOLY * ep = eg->polyin[kk];
		minf = min(minf, ep->min.y);
	}

	return minf;
}
float GetTileMaxY(long i, long j)
{
	float maxf = -9999999999.f;
	EERIE_BKG_INFO * eg;
	eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

	for (long kk = 0; kk < eg->nbpolyin; kk++)
	{
		EERIEPOLY * ep = eg->polyin[kk];
		maxf = max(maxf, ep->max.y);
	}

	return maxf;
}

//*************************************************************************************
//*************************************************************************************

#define TYPE_PORTAL	1
#define TYPE_ROOM	2
bool GetNameInfo( const std::string& name1, long& type, long& val1, long& val2)
{
	std::string name;
	name = name1;
	MakeUpcase(name);

	if (name[0] == 'R')
	{
		if (name[1] == '_')
		{
			type = TYPE_ROOM;
			val1 = atoi(name.c_str() + 2);
			val2 = 0;
			return true;
		}

		if ((name[1] == 'O') && (name[2] == 'O')
				&& (name[3] == 'M') && (name[4] == '_'))
		{
			type = TYPE_ROOM;
			val1 = atoi(name.c_str() + 5);
			val2 = 0;
			return true;
		}
	}

	if ((name[0] == 'P') && (name[1] == 'O') && (name[2] == 'R')
			&& (name[3] == 'T') && (name[4] == 'A') && (name[5] == 'L')
			&& (name[6] == '_'))
	{
		type = TYPE_PORTAL;
		char temp[16];
		strcpy(temp, name.c_str() + 7);
		temp[3] = 0;
		val1 = atoi(temp);
		val2 = atoi(name.c_str() + 11);
		return true;
	}

	return false;
}

extern long COMPUTE_PORTALS;

void EERIE_PORTAL_Blend_Portals_And_Rooms()
{
	if (!COMPUTE_PORTALS) return;

	if (!portals) return;

	for (long num = 0; num < portals->nb_total; num++)
	{
		CalcFaceNormal(&portals->portals[num].poly, portals->portals[num].poly.v);
		EERIEPOLY * ep = &portals->portals[num].poly;
		ep->center.x = ep->v[0].sx;
		ep->center.y = ep->v[0].sy;
		ep->center.z = ep->v[0].sz;
		long to = 3;
		float divide = ( 1.0f / 3 );

		if (ep->type & POLY_QUAD)
		{
			to = 4;
			divide = ( 1.0f / 4 );
		}

		ep->min.x = ep->v[0].sx;
		ep->min.y = ep->v[0].sy;
		ep->min.z = ep->v[0].sz;
		ep->max.x = ep->v[0].sx;
		ep->max.y = ep->v[0].sy;
		ep->max.z = ep->v[0].sz;

		for (long i = 1; i < to; i++)
		{
			ep->center.x += ep->v[i].sx;
			ep->center.y += ep->v[i].sy;
			ep->center.z += ep->v[i].sz;

			ep->min.x = min(ep->min.x, ep->v[i].sx);
			ep->min.y = min(ep->min.y, ep->v[i].sy);
			ep->min.z = min(ep->min.z, ep->v[i].sz);
			ep->max.x = max(ep->max.x, ep->v[i].sx);
			ep->max.y = max(ep->max.y, ep->v[i].sy);
			ep->max.z = max(ep->max.z, ep->v[i].sz);
		}

		ep->center *= divide;
		float dist = 0.f;

		for (long ii = 0; ii < to; ii++)
		{
			dist = max(dist, TRUEEEDistance3D((EERIE_3D *)&ep->v[ii], &ep->center));
		}

		ep->norm2.x = dist;

		for (long nroom = 0; nroom <= portals->nb_rooms; nroom++)
		{
			if ((nroom == portals->portals[num].room_1)
					||	(nroom == portals->portals[num].room_2))
			{
				portals->room[nroom].portals = (long *)realloc(portals->room[nroom].portals, sizeof(long) * (portals->room[nroom].nb_portals + 1));
				portals->room[nroom].portals[portals->room[nroom].nb_portals] = num;
				portals->room[nroom].nb_portals++;
			}
		}
	}
}

void EERIE_PORTAL_Room_Poly_Add(EERIEPOLY * ep, long nr, long px, long py, long idx)
{
	ARX_CHECK_SHORT(idx);
	ARX_CHECK_SHORT(px);
	ARX_CHECK_SHORT(py);
	ARX_CHECK_SHORT(nr);

	portals->room[nr].epdata = (EP_DATA *)realloc(portals->room[nr].epdata, sizeof(EP_DATA) * (portals->room[nr].nb_polys + 1));
	portals->room[nr].epdata[portals->room[nr].nb_polys].idx = ARX_CLEAN_WARN_CAST_SHORT(idx);
	portals->room[nr].epdata[portals->room[nr].nb_polys].px = ARX_CLEAN_WARN_CAST_SHORT(px);
	portals->room[nr].epdata[portals->room[nr].nb_polys].py = ARX_CLEAN_WARN_CAST_SHORT(py);
	ep->room = ARX_CLEAN_WARN_CAST_SHORT(nr);
	portals->room[nr].nb_polys++;
}
void EERIE_PORTAL_Release()
{
	if (portals)
	{
		if (portals->portals)
		{
			free(portals->portals);
			portals->portals = NULL;
		}

		if (portals->room)
		{
			if (portals->nb_rooms > 0)
			{
				for (long nn = 0; nn < portals->nb_rooms + 1; nn++)
				{
					if (portals->room[nn].epdata)
						free(portals->room[nn].epdata);

					if (portals->room[nn].portals)
						free(portals->room[nn].portals);

					portals->room[nn].epdata = NULL;
					portals->room[nn].portals = NULL;

					if (portals->room[nn].pVertexBuffer)
					{
						portals->room[nn].pVertexBuffer->Release();
						portals->room[nn].pVertexBuffer = NULL;
					}

					if (portals->room[nn].pussIndice)
					{
						free((void *)portals->room[nn].pussIndice);
						portals->room[nn].pussIndice = NULL;
					}

					if (portals->room[nn].ppTextureContainer)
					{
						free((void *)portals->room[nn].ppTextureContainer);
						portals->room[nn].ppTextureContainer = NULL;
					}

				}
			}

			free(portals->room);
			portals->room = NULL;
		}

		free(portals);
		portals = NULL;
	}
}

extern long COMPUTE_PORTALS;

void EERIE_PORTAL_Poly_Add(EERIEPOLY * ep, const std::string& name, long px, long py, long idx)
{
	if (!COMPUTE_PORTALS) return;

	long type, val1, val2;

	if (!GetNameInfo(name, type, val1, val2)) return;

	if (portals == NULL)
	{
		portals = (EERIE_PORTAL_DATA *)malloc(sizeof(EERIE_PORTAL_DATA));

		if (!portals) return;

		portals->nb_rooms = 0;
		portals->room = NULL;
		portals->nb_total = 0;
		portals->portals = NULL;
		USE_PORTALS = 4;
	}

	if (type == TYPE_PORTAL) //portal_def
	{
		portals->portals = (EERIE_PORTALS *)realloc(portals->portals, sizeof(EERIE_PORTALS) * (portals->nb_total + 1));
		portals->portals[portals->nb_total].room_1 = val1;
		portals->portals[portals->nb_total].room_2 = val2;
		memcpy(&portals->portals[portals->nb_total].poly, ep, sizeof(EERIEPOLY));

		float fDistMin = FLT_MAX;
		float fDistMax = FLT_MIN;
		int nbvert = (ep->type & POLY_QUAD) ? 4 : 3;

		ep->center.x = ep->v[0].sx;
		ep->center.y = ep->v[0].sy;
		ep->center.z = ep->v[0].sz;

		for (long ii = 1; ii < nbvert; ii++)
		{
			ep->center.x += ep->v[ii].sx;
			ep->center.y += ep->v[ii].sy;
			ep->center.z += ep->v[ii].sz;
		}

		ep->center /= nbvert;

		for (int ii = 0; ii < nbvert; ii++)
		{
			float fDist = TRUEEEDistance3D((EERIE_3D *)&ep->v[ii], &ep->center);
			fDistMin = min(fDistMin, fDist);
			fDistMax = max(fDistMax, fDist);
		}

		fDistMin = (fDistMax + fDistMin) * .5f;
		portals->portals[portals->nb_total].poly.v[0].rhw = fDistMin;

		portals->nb_total++;
	}
	else if (type == TYPE_ROOM)
	{
		if (val1 > portals->nb_rooms)
		{
			portals->room = (EERIE_ROOM_DATA *)realloc(portals->room, sizeof(EERIE_ROOM_DATA) * (val1 + 1));

			if (portals->nb_rooms == 0)
			{
				memset(portals->room, 0, sizeof(EERIE_ROOM_DATA)*(val1 + 1));
			}
			else for (long i = portals->nb_rooms + 1; i <= val1; i++)
				{
					memset(&portals->room[i], 0, sizeof(EERIE_ROOM_DATA));
				}

			portals->nb_rooms = val1;
		}

		EERIE_PORTAL_Room_Poly_Add(ep, val1, px, py, idx);
	}
}

int BkgAddPoly(EERIEPOLY * ep, EERIE_3DOBJ * eobj)
{
	long j, posx, posz, posy;
	float cx, cy, cz;
	EERIE_BKG_INFO * eg;
	long type, val1;
	type = -1;
	val1 = -1;

	if (TryToQuadify(ep, eobj)) return 0;

	cx = (ep->v[0].sx + ep->v[1].sx + ep->v[2].sx);
	cy = (ep->v[0].sy + ep->v[1].sy + ep->v[2].sy);
	cz = (ep->v[0].sz + ep->v[1].sz + ep->v[2].sz);
	posx = (long)(float)(cx * ( 1.0f / 3 ) * ACTIVEBKG->Xmul); 
	posz = (long)(float)(cz * ( 1.0f / 3 ) * ACTIVEBKG->Zmul); 
	posy = (long)(float)(cy * ( 1.0f / 3 ) * ACTIVEBKG->Xmul + ACTIVEBKG->Xsize * ( 1.0f / 2 )); 

	if (posy < 0) return 0;
	else if (posy >= ACTIVEBKG->Xsize) return 0;

	if (posx < 0) return 0;
	else if (posx >= ACTIVEBKG->Xsize) return 0;

	if (posz < 0) return 0;
	else if (posz >= ACTIVEBKG->Zsize) return 0;

	eg = &ACTIVEBKG->Backg[posx+posz*ACTIVEBKG->Xsize];

	DeclareEGInfo(cx * ( 1.0f / 3 ), cz * ( 1.0f / 3 ));
	DeclareEGInfo(ep->v[0].sx, ep->v[0].sz);
	DeclareEGInfo(ep->v[1].sx, ep->v[1].sz);
	DeclareEGInfo(ep->v[2].sx, ep->v[2].sz);

	cx *= ( 1.0f / 3 );
	cy *= ( 1.0f / 3 );
	cz *= ( 1.0f / 3 );
	long t = (((eg->nbpoly) >> 1) << 1) + 2; 
	long tt = (((eg->nbpoly - 1) >> 1) << 1) + 2; 

	if (eg->polydata == NULL)
	{
		eg->polydata = (EERIEPOLY *)malloc(sizeof(EERIEPOLY) * t);

		if (!eg->polydata) HERMES_Memory_Emergency_Out();
	}
	else if (tt != t)
	{
		eg->polydata = (EERIEPOLY *)realloc(eg->polydata, sizeof(EERIEPOLY) * t);

		if (!eg->polydata) HERMES_Memory_Emergency_Out();
	}

	memcpy(&eg->polydata[eg->nbpoly], ep, sizeof(EERIEPOLY));

	EERIEPOLY * epp = (EERIEPOLY *)&eg->polydata[eg->nbpoly];

	for (j = 0; j < 3; j++)
	{
		epp->tv[j].tu	= epp->v[j].tu;
		epp->tv[j].tv	= epp->v[j].tv;
		epp->tv[j].color = epp->v[j].color;
		epp->tv[j].rhw	= 1.f;
	}

	epp->center.x = cx; 
	epp->center.y = cy; 
	epp->center.z = cz; 
	epp->max.x = max(epp->v[0].sx, epp->v[1].sx);
	epp->max.x = max(epp->max.x, epp->v[2].sx);
	epp->min.x = min(epp->v[0].sx, epp->v[1].sx);
	epp->min.x = min(epp->min.x, epp->v[2].sx);

	epp->max.y = max(epp->v[0].sy, epp->v[1].sy);
	epp->max.y = max(epp->max.y, epp->v[2].sy);
	epp->min.y = min(epp->v[0].sy, epp->v[1].sy);
	epp->min.y = min(epp->min.y, epp->v[2].sy);

	epp->max.z = max(epp->v[0].sz, epp->v[1].sz);
	epp->max.z = max(epp->max.z, epp->v[2].sz);
	epp->min.z = min(epp->v[0].sz, epp->v[1].sz);
	epp->min.z = min(epp->min.z, epp->v[2].sz);
	epp->type = ep->type;
	epp->type &= ~POLY_QUAD;

	CalcFaceNormal(epp, epp->v);
	epp->area = Distance3D((epp->v[0].sx + epp->v[1].sx) * ( 1.0f / 2 ),
	                       (epp->v[0].sy + epp->v[1].sy) * ( 1.0f / 2 ),
	                       (epp->v[0].sz + epp->v[1].sz) * ( 1.0f / 2 ),
	                       epp->v[2].sx, epp->v[2].sy, epp->v[2].sz)
	            * Distance3D(epp->v[0].sx, epp->v[0].sy, epp->v[0].sz,
	                         epp->v[1].sx, epp->v[1].sy, epp->v[1].sz) * ( 1.0f / 2 );
	
	ARX_CHECK_SHORT(val1);

	if (type == TYPE_ROOM)
		epp->room = ARX_CLEAN_WARN_CAST_SHORT(val1);
	else
		epp->room = -1;

	eg->nbpoly++;

	eg->treat = 0;

	if (ep != NULL)
		if (ep->tex != NULL)
			if ( !ep->tex->m_texName.empty() )
			{
				if ( ep->tex->m_texName.find("STONE") != std::string::npos )         ep->type |= POLY_STONE;
				else if ( ep->tex->m_texName.find("PIERRE") != std::string::npos )   ep->type |= POLY_STONE;
				else if ( ep->tex->m_texName.find("WOOD") != std::string::npos )     ep->type |= POLY_WOOD;
				else if ( ep->tex->m_texName.find("BOIS") != std::string::npos )     ep->type |= POLY_STONE;
				else if ( ep->tex->m_texName.find("GAVIER") != std::string::npos )   ep->type |= POLY_GRAVEL;
				else if ( ep->tex->m_texName.find("EARTH") != std::string::npos )    ep->type |= POLY_EARTH;
			}

	EERIE_PORTAL_Poly_Add(epp, eobj->name, posx, posz, eg->nbpoly - 1);
	return 1;
}

//-----------------------------------------------------------------------------
float EERIE_TransformOldFocalToNewFocal(float _fOldFocal)
{
	if (_fOldFocal < 200)
	{
		return (-.34f * _fOldFocal + 168.5f);
	}
	else
	{
		if (_fOldFocal < 300)
		{
			return (-.25f * _fOldFocal + 150.5f);
		}
		else
		{
			if (_fOldFocal < 400)
			{
				return (-.155f * _fOldFocal + 124.f);
			}
			else
			{
				if (_fOldFocal < 500)
				{
					return (-.11f * _fOldFocal + 106.f);
				}
				else
				{
					if (_fOldFocal < 600)
					{
						return (-.075f * _fOldFocal + 88.5f);
					}
					else
					{
						if (_fOldFocal < 700)
						{
							return (-.055f * _fOldFocal + 76.5f);
						}
						else
						{
							if (_fOldFocal < 800)
							{
								return (-.045f * _fOldFocal + 69.5f);
							}
							else
							{
								return 33.5f;
							}
						}
					}
				}
			}
		}
	}
}

void PrepareActiveCamera()
{
	register float tmp = radians(ACTIVECAM->angle.a);
	ACTIVECAM->Xcos = (float)EEcos(tmp);
	ACTIVECAM->Xsin = (float)EEsin(tmp);
	tmp = radians(ACTIVECAM->angle.b);
	ACTIVECAM->Ycos = (float)EEcos(tmp);
	ACTIVECAM->Ysin = (float)EEsin(tmp);
	tmp = radians(ACTIVECAM->angle.g);
	ACTIVECAM->Zcos = (float)EEcos(tmp);
	ACTIVECAM->Zsin = (float)EEsin(tmp);
	ACTIVECAM->posleft = (float)(ACTIVECAM->centerx + ACTIVECAM->clip.left);
	ACTIVECAM->postop = (float)(ACTIVECAM->centery + ACTIVECAM->clip.top);

	MatrixReset(&ACTIVECAM->matrix);

	float cx = ACTIVECAM->Xcos;
	float sx = ACTIVECAM->Xsin;
	float cy = ACTIVECAM->Ycos;
	float sy = ACTIVECAM->Ysin;
	float cz = ACTIVECAM->Zcos;
	float sz = ACTIVECAM->Zsin;
	float const1, const2, const3, const4 ;

	const1 = -sz * cx ;
	const2 = cx * cz ;
	const3 = sx * sz ;
	const4 = -sx * cz ;

	ACTIVECAM->matrix._11 = cz * cy ;
	ACTIVECAM->matrix._21 = const4 * sy + const1 ;
	ACTIVECAM->matrix._31 = const3 - const2 * sy ;
	ACTIVECAM->matrix._12 = cy * sz ;
	ACTIVECAM->matrix._22 = const2 - const3 * sy ;
	ACTIVECAM->matrix._32 = const1 * sy + const4 ;
	ACTIVECAM->matrix._13 = sy ;
	ACTIVECAM->matrix._23 = sx * cy ;
	ACTIVECAM->matrix._33 = cx * cy ;

	EERIE_CreateMatriceProj(ARX_CLEAN_WARN_CAST_FLOAT(DANAESIZX),
							ARX_CLEAN_WARN_CAST_FLOAT(DANAESIZY),
							EERIE_TransformOldFocalToNewFocal(ACTIVECAM->focal),
							1.f,
							ACTIVECAM->cdepth);


}
void F_PrepareCamera(EERIE_CAMERA * cam)
{
	float tmp = radians(cam->angle.a);
	cam->use_focal = cam->focal * Xratio;
	cam->Xcos = (float)EEcos(tmp);
	cam->Xsin = (float)EEsin(tmp);
	tmp = radians(cam->angle.b);
	cam->Ycos = (float)EEcos(tmp);
	cam->Ysin = (float)EEsin(tmp);
	cam->Zcos = 1;
	cam->Zsin = 0.f;
}
//*************************************************************************************
//*************************************************************************************
void PrepareCamera(EERIE_CAMERA * cam)
{
	float tmp = radians(cam->angle.a);
	cam->transform.use_focal = cam->use_focal = cam->focal * Xratio;
	cam->transform.xcos = cam->Xcos = (float)EEcos(tmp);
	cam->transform.xsin = cam->Xsin = (float)EEsin(tmp);
	tmp = radians(cam->angle.b);
	cam->transform.ycos = cam->Ycos = (float)EEcos(tmp);
	cam->transform.ysin = cam->Ysin = (float)EEsin(tmp);
	tmp = radians(cam->angle.g);
	cam->Zcos = (float)EEcos(tmp);
	cam->Zsin = (float)EEsin(tmp);
	cam->transform.xmod = cam->xmod = cam->posleft = (float)(cam->centerx + cam->clip.left);
	cam->transform.ymod = cam->ymod = cam->postop = (float)(cam->centery + cam->clip.top);
	cam->transform.zmod = cam->Zmul;
	cam->transform.posx = cam->pos.x;
	cam->transform.posy = cam->pos.y;
	cam->transform.posz = cam->pos.z;

	EERIE_CreateMatriceProj(ARX_CLEAN_WARN_CAST_FLOAT(DANAESIZX),
							ARX_CLEAN_WARN_CAST_FLOAT(DANAESIZY),
							EERIE_TransformOldFocalToNewFocal(cam->focal),
							1.f,
							cam->cdepth);

}

void SP_PrepareCamera(EERIE_CAMERA * cam)
{
	float tmp = radians(cam->angle.a);
	cam->transform.use_focal = cam->use_focal = cam->focal * Xratio;
	cam->transform.xcos = cam->Xcos = (float)EEcos(tmp);
	cam->transform.xsin = cam->Xsin = (float)EEsin(tmp);
	tmp = radians(cam->angle.b);
	cam->transform.ycos = cam->Ycos = (float)EEcos(tmp);
	cam->transform.ysin = cam->Ysin = (float)EEsin(tmp);
	tmp = radians(cam->angle.g);
	cam->Zcos = (float)EEcos(tmp);
	cam->Zsin = (float)EEsin(tmp);
	cam->transform.xmod = cam->xmod = cam->posleft = (float)(cam->centerx + cam->clip.left);
	cam->transform.ymod = cam->ymod = cam->postop = (float)(cam->centery + cam->clip.top);
	cam->transform.zmod = cam->Zmul;
	cam->transform.posx = cam->pos.x;
	cam->transform.posy = cam->pos.y;
	cam->transform.posz = cam->pos.z;
}

extern long EDITION;

//*************************************************************************************
//*************************************************************************************

void SetCameraDepth(float depth)
{
	ACTIVECAM->cdepth = depth;
	ACTIVECAM->Zdiv = depth * 1.2f;
	ACTIVECAM->Zmul = 1.f / ACTIVECAM->Zdiv;
	long l = depth * 0.42f;
	ACTIVECAM->clip3D = (l / (long)BKG_SIZX) + 1;
}

extern TextureContainer * tflare;
extern float _framedelay;
long TRUECLIPPING = 1;


//*************************************************************************************
//*************************************************************************************

void RecalcLight(EERIE_LIGHT * el)
{
	el->rgb255.r = el->rgb.r * 255.f;
	el->rgb255.g = el->rgb.g * 255.f;
	el->rgb255.b = el->rgb.b * 255.f;
	el->falldiff = el->fallend - el->fallstart;
	el->falldiffmul = 1.f / el->falldiff;
	el->precalc = el->intensity * GLOBAL_LIGHT_FACTOR;
}

void ClearDynLights()
{
	long i;

	for (i = 0; i < MAX_DYNLIGHTS; i++)
	{
		if (DynLight[i].exist)
		{
			DynLight[i].exist = 0;
		}
	}

	for (i = 0; i < MAX_LIGHTS; i++)
	{
		if ((GLight[i]) && (GLight[i]->tl > 0))
			GLight[i]->tl = 0;
	}

	TOTPDL = 0;
	TOTIOPDL = 0;
}

//*************************************************************************************
//*************************************************************************************
long GetFreeDynLight()
{
	long i;

	for (i = 1; i < MAX_DYNLIGHTS; i++)
	{
		if (!(DynLight[i].exist))
		{
			DynLight[i].type = 0;
			DynLight[i].intensity = 1.3f;
			DynLight[i].treat = 1;

			DynLight[i].time_creation = ARXTimeUL();

			DynLight[i].duration = 0;
			DynLight[i].extras = 0;

			return i;
		}
	}

	return -1;
}

//*************************************************************************************
//*************************************************************************************
long CountBkgVertex()
{
	long i, j;
	EERIEPOLY * ep;
	EERIE_BKG_INFO * eg;
	long count = 0;

	for (j = 0; j < ACTIVEBKG->Zsize; j++)
		for (i = 0; i < ACTIVEBKG->Xsize; i++)
		{
			eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

			for (long l = 0; l < eg->nbpoly; l++)
			{
				ep = &eg->polydata[l];

				if (ep != NULL)
				{
					if (ep->type & POLY_QUAD) count += 4;
					else count += 3;
				}
			}
		}

	return count;
}




//*************************************************************************************
//*************************************************************************************
void DrawEERIEObjEx(EERIE_3DOBJ * eobj,
					EERIE_3D * angle, EERIE_3D  * pos, EERIE_3D * scale, EERIE_RGB * col)
{
	if (eobj == NULL) return;

	float    Xcos, Ycos, Zcos, Xsin, Ysin, Zsin;
	D3DTLVERTEX v;
	D3DTLVERTEX rv;
	D3DTLVERTEX vert_list[4];


	Zsin = radians(angle->a);
	Xcos = (float)EEcos(Zsin);
	Xsin = (float)EEsin(Zsin);
	Zsin = radians(angle->b);
	Ycos = (float)EEcos(Zsin);
	Ysin = (float)EEsin(Zsin);
	Zsin = radians(angle->g);
	Zcos = (float)EEcos(Zsin);
	Zsin = (float)EEsin(Zsin);

	for (size_t i = 0; i < eobj->vertexlist.size(); i++)
	{
		v.sx = eobj->vertexlist[i].v.x * scale->x;
		v.sy = eobj->vertexlist[i].v.y * scale->y;
		v.sz = eobj->vertexlist[i].v.z * scale->z;

		_YRotatePoint((EERIE_3D *)&v, (EERIE_3D *)&rv, Ycos, Ysin);
		_XRotatePoint((EERIE_3D *)&rv, (EERIE_3D *)&v, Xcos, Xsin);
		_ZRotatePoint((EERIE_3D *)&v, (EERIE_3D *)&rv, Zcos, Zsin);

		eobj->vertexlist3[i].v.x = rv.sx += pos->x;
		eobj->vertexlist3[i].v.y = rv.sy += pos->y;
		eobj->vertexlist3[i].v.z = rv.sz += pos->z;
		
		EE_RT(&rv, &eobj->vertexlist[i].vworld);
		EE_P(&eobj->vertexlist[i].vworld, &eobj->vertexlist[i].vert);
	}

	D3DCOLOR coll = EERIERGB(col->r, col->g, col->b);

	for (size_t i = 0; i < eobj->facelist.size(); i++)
	{
		vert_list[0].sx = eobj->vertexlist[eobj->facelist[i].vid[0]].vworld.x;
		vert_list[0].sy = eobj->vertexlist[eobj->facelist[i].vid[0]].vworld.y;
		vert_list[0].sz = eobj->vertexlist[eobj->facelist[i].vid[0]].vworld.z;
		vert_list[1].sx = eobj->vertexlist[eobj->facelist[i].vid[1]].vworld.x;
		vert_list[1].sy = eobj->vertexlist[eobj->facelist[i].vid[1]].vworld.y;
		vert_list[1].sz = eobj->vertexlist[eobj->facelist[i].vid[1]].vworld.z;
		vert_list[2].sx = eobj->vertexlist[eobj->facelist[i].vid[2]].vworld.x;
		vert_list[2].sy = eobj->vertexlist[eobj->facelist[i].vid[2]].vworld.y;
		vert_list[2].sz = eobj->vertexlist[eobj->facelist[i].vid[2]].vworld.z;
		vert_list[0].tu = eobj->facelist[i].u[0];
		vert_list[0].tv = eobj->facelist[i].v[0];
		vert_list[1].tu = eobj->facelist[i].u[1];
		vert_list[1].tv = eobj->facelist[i].v[1];
		vert_list[2].tu = eobj->facelist[i].u[2];
		vert_list[2].tv = eobj->facelist[i].v[2];
		vert_list[0].color = vert_list[1].color = vert_list[2].color = coll;

		if ((eobj->facelist[i].facetype == 0)
				|| (eobj->texturecontainer[eobj->facelist[i].texid] == NULL))
		{
			SETTC(NULL);
		}
		else
		{
			SETTC(eobj->texturecontainer[eobj->facelist[i].texid]);
		}

		if (eobj->facelist[i].facetype & POLY_DOUBLESIDED)
			GRenderer->SetCulling(Renderer::CullNone);
		else GRenderer->SetCulling(Renderer::CullCW);

		ARX_DrawPrimitive_SoftClippZ(&vert_list[0],
									 &vert_list[1],
									 &vert_list[2]);
	}
}
//*************************************************************************************
//routine qui gere l'alpha au vertex SEB
//*************************************************************************************
void DrawEERIEObjExEx(EERIE_3DOBJ * eobj,
					  EERIE_3D * angle, EERIE_3D  * pos, EERIE_3D * scale, int coll)
{
	if (eobj == NULL) return;

	float    Xcos, Ycos, Zcos, Xsin, Ysin, Zsin;
	D3DTLVERTEX v;
	D3DTLVERTEX rv;
	D3DTLVERTEX vert_list[4];


	Zsin = radians(angle->a);
	Xcos = (float)EEcos(Zsin);
	Xsin = (float)EEsin(Zsin);
	Zsin = radians(angle->b);
	Ycos = (float)EEcos(Zsin);
	Ysin = (float)EEsin(Zsin);
	Zsin = radians(angle->g);
	Zcos = (float)EEcos(Zsin);
	Zsin = (float)EEsin(Zsin);

	for (size_t i = 0; i < eobj->vertexlist.size(); i++)
	{
		v.sx = eobj->vertexlist[i].v.x * scale->x;
		v.sy = eobj->vertexlist[i].v.y * scale->y;
		v.sz = eobj->vertexlist[i].v.z * scale->z;

		_YRotatePoint((EERIE_3D *)&v, (EERIE_3D *)&rv, Ycos, Ysin);
		_XRotatePoint((EERIE_3D *)&rv, (EERIE_3D *)&v, Xcos, Xsin);
		_ZRotatePoint((EERIE_3D *)&v, (EERIE_3D *)&rv, Zcos, Zsin);

		eobj->vertexlist3[i].v.x = rv.sx += pos->x;
		eobj->vertexlist3[i].v.y = rv.sy += pos->y;
		eobj->vertexlist3[i].v.z = rv.sz += pos->z;

		EE_RT(&rv, &eobj->vertexlist[i].vworld);
		EE_P(&eobj->vertexlist[i].vworld, &eobj->vertexlist[i].vert);
	}

	for (size_t i = 0; i < eobj->facelist.size(); i++)
	{
		vert_list[0].sx = eobj->vertexlist[eobj->facelist[i].vid[0]].vworld.x;
		vert_list[0].sy = eobj->vertexlist[eobj->facelist[i].vid[0]].vworld.y;
		vert_list[0].sz = eobj->vertexlist[eobj->facelist[i].vid[0]].vworld.z;
		vert_list[1].sx = eobj->vertexlist[eobj->facelist[i].vid[1]].vworld.x;
		vert_list[1].sy = eobj->vertexlist[eobj->facelist[i].vid[1]].vworld.y;
		vert_list[1].sz = eobj->vertexlist[eobj->facelist[i].vid[1]].vworld.z;
		vert_list[2].sx = eobj->vertexlist[eobj->facelist[i].vid[2]].vworld.x;
		vert_list[2].sy = eobj->vertexlist[eobj->facelist[i].vid[2]].vworld.y;
		vert_list[2].sz = eobj->vertexlist[eobj->facelist[i].vid[2]].vworld.z;

		vert_list[0].tu = eobj->facelist[i].u[0];
		vert_list[0].tv = eobj->facelist[i].v[0];
		vert_list[1].tu = eobj->facelist[i].u[1];
		vert_list[1].tv = eobj->facelist[i].v[1];
		vert_list[2].tu = eobj->facelist[i].u[2];
		vert_list[2].tv = eobj->facelist[i].v[2];
		vert_list[0].color = vert_list[1].color = vert_list[2].color = coll;

		if ((eobj->facelist[i].facetype == 0)
				|| (eobj->texturecontainer[eobj->facelist[i].texid] == NULL))
		{
			SETTC(NULL);
		}
		else
		{
			SETTC(eobj->texturecontainer[eobj->facelist[i].texid]);
		}

		if (eobj->facelist[i].facetype & POLY_DOUBLESIDED)
			GRenderer->SetCulling(Renderer::CullNone);
		else GRenderer->SetCulling(Renderer::CullCW);

		ARX_DrawPrimitive_SoftClippZ(&vert_list[0],
									 &vert_list[1],
									 &vert_list[2]);
	}
}

extern float FLOATTEST;

EERIE_3D BBOXMIN, BBOXMAX;

extern long USE_CEDRIC_ANIM;
//*************************************************************************************
// Memorizes information for animation to animation smoothing interpolation
//*************************************************************************************
void AcquireLastAnim(INTERACTIVE_OBJ * io)
{
	if ((!io->animlayer[0].cur_anim)
			&&	(!io->animlayer[1].cur_anim)
			&&	(!io->animlayer[2].cur_anim)
			&&	(!io->animlayer[3].cur_anim)) return;

	// Stores Frametime and number of vertex for later interpolation
	ARX_CHECK_ULONG(FrameTime);
	io->lastanimtime = ARX_CLEAN_WARN_CAST_ULONG(FrameTime);
	io->nb_lastanimvertex = 1;
}

//*************************************************************************************
// Declares an Animation as finished.
// Usefull to update object true position with object virtual pos.
//*************************************************************************************
void FinishAnim(INTERACTIVE_OBJ * io, ANIM_HANDLE * eanim)
{

	if (io == NULL) return;

	if (eanim == NULL) return;

	// Only layer 0 controls movement...
	if ((eanim == io->animlayer[0].cur_anim) && (io->ioflags & IO_NPC))
	{
		io->move.x = io->lastmove.x = 0;
		io->move.y = io->lastmove.y = 0;
		io->move.z = io->lastmove.z = 0;
	}

	return;
}

bool IsVertexIdxInGroup(EERIE_3DOBJ * eobj, long idx, long grs)
{
	
	if (eobj == NULL) return false;

	for (size_t i = 0; i < eobj->grouplist[grs].indexes.size(); i++)
	{
		long ii = eobj->grouplist[grs].indexes[i];

		if (ii == idx) return true;
	}

	return false;
}

extern void LoadLevelScreen();
extern void LoadLevelScreen(long lev);

extern float PROGRESS_BAR_COUNT;
long NOCHECKSUM = 0;
long USE_FAST_SCENES = 1;


bool FastSceneLoad(const string & partial_path) {
	
	// TODO bounds checking
	
	LogDebug << "Fast Scene Load " << partial_path;
	if(!USE_FAST_SCENES) {
		LogDebug << "Not using fast scenes.";
		return false;
	}
	
	string path = "Game\\" + partial_path;
	string file = path + "fast.fts";
	
	size_t size;
	char * dat = (char *)PAK_FileLoadMalloc(file, size);
	if(!dat) {
		LogError << "FastSceneLoad: could not find " << file;
	}
	
	size_t pos = 0;
	UNIQUE_HEADER * uh = (UNIQUE_HEADER *)dat;
	pos += sizeof(UNIQUE_HEADER);
	
	if(!NOCHECKSUM && strcasecmp(uh->path, path)) {
		LogError << "FastSceneLoad path mismatch: \"" << path << "\" and \"" << uh->path << "\"";
		free(dat);
		return false;
	}
	
	if(uh->version != FTS_VERSION) {
		LogError << "FastSceneLoad version mistmatch: got " << uh->version << " expected " << FTS_VERSION;
		free(dat);
		return false;
	}
	
	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen();
	
	// Skip .scn file list.
	pos += uh->count * (512 + sizeof(UNIQUE_HEADER2));
	
	InitBkg(ACTIVEBKG, MAX_BKGX, MAX_BKGZ, BKG_SIZX, BKG_SIZZ);
	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen();
	
	char * rawdata = new char[uh->uncompressedsize];
	if(rawdata == NULL) {
		LogError << "FastSceneLoad: can't allocate buffer for uncompressed data";
		free(dat);
		return false;
	}
	
	size_t rawsize = blastMem(dat + pos, size - pos, rawdata, uh->uncompressedsize);
	free(dat);
	if(!rawsize) {
		LogDebug << "FastSceneLoad: blastMem didn't return anything " << size << " " << pos;
		delete[] rawdata;
		return false;
	}
	
	PROGRESS_BAR_COUNT += 3.f;
	LoadLevelScreen();
	pos = 0;
	
	FAST_SCENE_HEADER * fsh = (FAST_SCENE_HEADER *)(rawdata + pos);
	pos += sizeof(FAST_SCENE_HEADER);
	
	if(fsh->version != FTS_VERSION) {
		LogError << "FastSceneLoad: version mismatch in FAST_SCENE_HEADER";
		delete[] rawdata;
		return false;
	}
	
	if(fsh->sizex != ACTIVEBKG->Xsize || fsh->sizez != ACTIVEBKG->Zsize) {
		LogError << "FastSceneLoad: size mismatch in FAST_SCENE_HEADER";
		delete[] rawdata;
		return false;
	}
	
	player.pos = fsh->playerpos;
	Mscenepos = fsh->Mscenepos;
	
	// load textures
	typedef std::map<s32, TextureContainer *> TextureContainerMap;
	TextureContainerMap textures;
	for(long k = 0; k < fsh->nb_textures; k++) {
		FAST_TEXTURE_CONTAINER * ftc = (FAST_TEXTURE_CONTAINER *)(rawdata + pos);
		TextureContainer * tmpTC = D3DTextr_CreateTextureFromFile(ftc->fic, 0, 0, EERIETEXTUREFLAG_LOADSCENE_RELEASE);
		if(tmpTC) {
			textures[ftc->tc] = tmpTC;
			if(!tmpTC->m_pddsSurface) {
				tmpTC->Restore();
			}
		}
		pos += sizeof(FAST_TEXTURE_CONTAINER);
	}
	
	PROGRESS_BAR_COUNT += 4.f;
	LoadLevelScreen();
	
	for(long j = 0; j < fsh->sizez; j++) {
		for(long i = 0; i < fsh->sizex; i++) {
			
			FAST_SCENE_INFO * fsi = (FAST_SCENE_INFO *)(rawdata + pos);
			pos += sizeof(FAST_SCENE_INFO);
			
			EERIE_BKG_INFO & bkg = ACTIVEBKG->Backg[i + (j * fsh->sizex)];
			
			bkg.nbianchors = (short)fsi->nbianchors;
			bkg.nbpoly = (short)fsi->nbpoly;
			
			if(fsi->nbpoly > 0) {
				bkg.polydata = (EERIEPOLY *)malloc(sizeof(EERIEPOLY) * fsi->nbpoly);
				if(!bkg.polydata) {
					HERMES_Memory_Emergency_Out();
				}
			} else {
				bkg.polydata = NULL;
			}
			
			bkg.treat = 0;
			bkg.nothing = fsi->nbpoly ? 0 : 1;
			
			bkg.frustrum_maxy = -99999999.f;
			bkg.frustrum_miny = 99999999.f;
			
			for(long k = 0; k < fsi->nbpoly; k++) {
				
				FAST_EERIEPOLY * ep = (FAST_EERIEPOLY *)(rawdata + pos);
				pos += sizeof(FAST_EERIEPOLY);
				
				EERIEPOLY * ep2 = &bkg.polydata[k];
				memset(ep2, 0, sizeof(EERIEPOLY));
				ep2->room = ep->room;
				ep2->area = ep->area;
				ep2->norm = ep->norm;
				ep2->norm2 = ep->norm2;
				copy(ep->nrml, ep->nrml + 4, ep2->nrml);
				
				if(ep->tex != 0) {
					TextureContainerMap::const_iterator cit = textures.find(ep->tex);
					ep2->tex = (cit != textures.end()) ? cit->second : NULL;
				} else {
					ep2->tex = NULL;
				}
				
				ep2->transval = ep->transval;
				ep2->type = ep->type;
				
				for(size_t kk = 0; kk < 4; kk++) {
					ep2->v[kk].color = 0xFFFFFFFF;
					ep2->v[kk].rhw = 1;
					ep2->v[kk].specular = 1;
					ep2->v[kk].sx = ep->v[kk].ssx;
					ep2->v[kk].sy = ep->v[kk].sy;
					ep2->v[kk].sz = ep->v[kk].ssz;
					ep2->v[kk].tu = ep->v[kk].stu;
					ep2->v[kk].tv = ep->v[kk].stv;
				}
				
				memcpy(ep2->tv, ep2->v, sizeof(D3DTLVERTEX) * 4);
				
				for(size_t kk = 0; kk < 4; kk++) {
					ep2->tv[kk].color = 0xFF000000;
				}
				
				long to;
				float div;
				if(ep->type & POLY_QUAD) {
					to = 4;
					div = 0.25f;
				} else {
					to = 3;
					div = 0.333333333333f;
				}
				
				ep2->center.x = 0.f;
				ep2->center.y = 0.f;
				ep2->center.z = 0.f;
				
				for(long h = 0; h < to; h++) {
					
					ep2->center.x += ep2->v[h].sx;
					ep2->center.y += ep2->v[h].sy;
					ep2->center.z += ep2->v[h].sz;
					
					if(h != 0) {
						ep2->max.x = max(ep2->max.x, ep2->v[h].sx);
						ep2->min.x = min(ep2->min.x, ep2->v[h].sx);
						ep2->max.y = max(ep2->max.y, ep2->v[h].sy);
						ep2->min.y = min(ep2->min.y, ep2->v[h].sy);
						ep2->max.z = max(ep2->max.z, ep2->v[h].sz);
						ep2->min.z = min(ep2->min.z, ep2->v[h].sz);
					} else {
						ep2->min.x = ep2->max.x = ep2->v[0].sx;
						ep2->min.y = ep2->max.y = ep2->v[0].sy;
						ep2->min.z = ep2->max.z = ep2->v[0].sz;
					}
				}
				
				ep2->center *= div;
				
				float dist = 0.f;
				for(int h = 0; h < to; h++) {
					float x = ep2->v[h].sx - ep2->center.x;
					float y = ep2->v[h].sy - ep2->center.y;
					float z = ep2->v[h].sz - ep2->center.z;
					float d = sqrt((x * x) + (y * y) + (z * z));
					dist = max(dist, d);
				}
				ep2->v[0].rhw = dist;
				
				DeclareEGInfo(ep2->center.x, ep2->center.z);
				DeclareEGInfo(ep2->v[0].sx, ep2->v[0].sz);
				DeclareEGInfo(ep2->v[1].sx, ep2->v[1].sz);
				DeclareEGInfo(ep2->v[2].sx, ep2->v[2].sz);
				if(ep->type & POLY_QUAD) {
					DeclareEGInfo(ep2->v[3].sx, ep2->v[3].sz);
				}
			}
			
			if(fsi->nbianchors <= 0) {
				bkg.ianchors = NULL;
			} else {
				bkg.ianchors = (long *)malloc(sizeof(long) * fsi->nbianchors);
				if(!bkg.ianchors) {
					HERMES_Memory_Emergency_Out();
				}
				memset(bkg.ianchors, 0, sizeof(long)*fsi->nbianchors);
			}
			
			for(long k = 0; k < fsi->nbianchors; k++) {
				s32 * ianch = (s32 *)(rawdata + pos);
				pos += sizeof(s32);
				ACTIVEBKG->Backg[i+j * fsh->sizex].ianchors[k] = *ianch;
			}
		}
	}
	
	PROGRESS_BAR_COUNT += 4.f;
	LoadLevelScreen();
	ACTIVEBKG->nbanchors = fsh->nb_anchors;
	
	if(fsh->nb_anchors > 0) {
		ACTIVEBKG->anchors = (_ANCHOR_DATA *)malloc(sizeof(_ANCHOR_DATA) * fsh->nb_anchors);
		if(!ACTIVEBKG->anchors) {
			HERMES_Memory_Emergency_Out();
		}
		memset(ACTIVEBKG->anchors, 0, sizeof(_ANCHOR_DATA)*fsh->nb_anchors);
	} else {
		ACTIVEBKG->anchors = NULL;
	}
	
	for(long i = 0; i < fsh->nb_anchors; i++) {
		
		FAST_ANCHOR_DATA * fad = (FAST_ANCHOR_DATA *)(rawdata + pos);
		pos += sizeof(FAST_ANCHOR_DATA);
		
		_ANCHOR_DATA & anchor = ACTIVEBKG->anchors[i];
		anchor.flags = fad->flags;
		anchor.pos = fad->pos;
		anchor.nblinked = fad->nb_linked;
		anchor.height = fad->height;
		anchor.radius = fad->radius;
		
		if(fad->nb_linked > 0) {
			anchor.linked = (long *)malloc(sizeof(long) * fad->nb_linked);
			if(!ACTIVEBKG->anchors[i].linked) {
				HERMES_Memory_Emergency_Out();
			}
		} else {
			anchor.linked = NULL;
		}
		
		for(long kk = 0; kk < fad->nb_linked; kk++) {
			s32 * lng = (s32 *)(rawdata + pos);
			pos += sizeof(s32);
			anchor.linked[kk] = *lng;
		}
	}
	
	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen();
	
	if(fsh->nb_rooms > 0) {
		
		if(portals != NULL) {
			EERIE_PORTAL_Release();
		}
		
		portals = (EERIE_PORTAL_DATA *)malloc(sizeof(EERIE_PORTAL_DATA));
		portals->nb_rooms = fsh->nb_rooms;
		portals->room = (EERIE_ROOM_DATA *)malloc(sizeof(EERIE_ROOM_DATA) * (portals->nb_rooms + 1));
		portals->nb_total = fsh->nb_portals;
		portals->portals = (EERIE_PORTALS *)malloc(sizeof(EERIE_PORTALS) * portals->nb_total);
		
		for(long i = 0; i < portals->nb_total; i++) {
			
			EERIE_SAVE_PORTALS * epo = (EERIE_SAVE_PORTALS *)(rawdata + pos);
			pos += sizeof(EERIE_SAVE_PORTALS);
			
			EERIE_PORTALS & portal = portals->portals[i];
			
			memset(&portal, 0, sizeof(EERIE_PORTALS));
			portal.room_1 = epo->room_1;
			portal.room_2 = epo->room_2;
			portal.useportal = epo->useportal;
			portal.paddy = epo->paddy;
			portal.poly.area = epo->poly.area;
			portal.poly.type = epo->poly.type;
			portal.poly.transval = epo->poly.transval;
			portal.poly.room = epo->poly.room;
			portal.poly.misc = epo->poly.misc;
			portal.poly.center = epo->poly.center;
			portal.poly.max = epo->poly.max;
			portal.poly.min = epo->poly.min;
			portal.poly.norm = epo->poly.norm;
			portal.poly.norm2 = epo->poly.norm2;
			
			copy(epo->poly.nrml, epo->poly.nrml + 4, portal.poly.nrml);
			copy(epo->poly.v, epo->poly.v + 4, portal.poly.v);
			copy(epo->poly.tv, epo->poly.tv + 4, portal.poly.tv);
		}
		
		for(long i = 0; i < portals->nb_rooms + 1; i++) {
			
			EERIE_SAVE_ROOM_DATA * erd = (EERIE_SAVE_ROOM_DATA *)(rawdata + pos);
			pos += sizeof(EERIE_SAVE_ROOM_DATA);
			
			EERIE_ROOM_DATA & room = portals->room[i];
			
			memset(&room, 0, sizeof(EERIE_ROOM_DATA));
			room.nb_portals = erd->nb_portals;
			room.nb_polys = erd->nb_polys;
			
			if(room.nb_portals) {
				room.portals = (long *)malloc(sizeof(long) * room.nb_portals);
				s32 * start = (s32 *)(rawdata + pos);
				pos += sizeof(s32) * portals->room[i].nb_portals;
				copy(start, (s32 *)(rawdata + pos), room.portals);
			} else {
				room.portals = NULL;
			}
			
			if(room.nb_polys) {
				room.epdata = (EP_DATA *)malloc(sizeof(EP_DATA) * room.nb_polys);
				FAST_EP_DATA * ed = (FAST_EP_DATA *)(rawdata + pos);
				pos += sizeof(FAST_EP_DATA) * portals->room[i].nb_polys;
				copy(ed, (FAST_EP_DATA *)(rawdata + pos), room.epdata);
			} else {
				portals->room[i].epdata = NULL;
			}
			
		}
		
		USE_PORTALS = (COMPUTE_PORTALS == 0) ? 0 : 4;
	} else {
		USE_PORTALS = 0;
	}
	
	if(RoomDistance) {
		free(RoomDistance);
	}
	RoomDistance = NULL;
	NbRoomDistance = 0;
	
	if(portals) {
		NbRoomDistance = portals->nb_rooms + 1;
		RoomDistance = (ROOM_DIST_DATA *)malloc(sizeof(ROOM_DIST_DATA) * (NbRoomDistance) * (NbRoomDistance));
		for(long n = 0; n < NbRoomDistance; n++) {
			for(long m = 0; m < NbRoomDistance; m++) {
				ROOM_DIST_DATA_SAVE * rdds = (ROOM_DIST_DATA_SAVE *)(rawdata + pos);
				pos += sizeof(ROOM_DIST_DATA_SAVE);
				EERIE_3D start = rdds->startpos;
				EERIE_3D end = rdds->endpos;
				SetRoomDistance(m, n, rdds->distance, &start, &end);
			}
		}
	}
	
	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen();
	
	EERIEPOLY_Compute_PolyIn();
	PROGRESS_BAR_COUNT += 3.f;
	LoadLevelScreen();
	EERIE_PATHFINDER_Create();
	EERIE_PORTAL_Blend_Portals_And_Rooms();
	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen();
	
	ComputePortalVertexBuffer();
	
	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen();
	delete[] rawdata;
	
	LogDebug << "FastSceneLoad: done loading.";
	return true;
	
}

#define checkalloc if(pos >= allocsize - 100000) { free(dat); return false; }

/*!
 * Save the currently loaded scene.
 * @param partal_path Where to save the scene to.
 */
bool FastSceneSave(const string & partial_path) {
	
	string path = "Game\\" + partial_path;
	
	LogDebug << "FastSceneSave" << path;
	
	if(!CreateFullPath(path)) {
		return false;
	}
	
	size_t allocsize = (256) * 60 + 1000000 + sizeof(FAST_SCENE_HEADER)
	                   + sizeof(FAST_TEXTURE_CONTAINER) * 1000
	                   + sizeof(FAST_ANCHOR_DATA) * ACTIVEBKG->nbanchors * 2;
	
	if(portals) {
		
		for(long i = 0; i < portals->nb_total + 1; i++) {
			allocsize += sizeof(EERIE_SAVE_PORTALS);
		}
		
		for(long i = 0; i < portals->nb_rooms + 1; i++) {
			allocsize += sizeof(EERIE_SAVE_ROOM_DATA);
			allocsize += sizeof(s32) * portals->room[i].nb_portals;
			allocsize += sizeof(FAST_EP_DATA) * portals->room[i].nb_polys;
		}
		
		allocsize += sizeof(ROOM_DIST_DATA_SAVE) * (portals->nb_rooms + 1) * (portals->nb_rooms + 1);
	}
	
	for(long i = 0; i < ACTIVEBKG->nbanchors; i++) {
		allocsize += ACTIVEBKG->anchors[i].nblinked * sizeof(s32);
	}
	
	for(long j = 0; j < ACTIVEBKG->Zsize; j++) {
		for(long i = 0; i < ACTIVEBKG->Xsize; i++) {
			allocsize += sizeof(FAST_SCENE_INFO);
			allocsize += ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize].nbpoly * (sizeof(EERIEPOLY) + 1);
		}
	}
	
	
	size_t pos = 0;
	unsigned char * dat = (unsigned char *)malloc(allocsize);
	if(!dat) {
		HERMES_Memory_Emergency_Out();
	}
	
	memset(dat, 0, allocsize);
	UNIQUE_HEADER * uh = (UNIQUE_HEADER *)dat;
	strcpy(uh->path, path.c_str());
	uh->version = FTS_VERSION;
	pos += sizeof(UNIQUE_HEADER);
	
	string path2 = partial_path + "*.scn";
	
	long count = 0;
//	TODO: find
//	struct _finddata_t fd;
//  long idx;
//	if ((idx = _findfirst(path2, &fd)) != -1)
//	{
//		do
//		{
//			if (!(fd.attrib & _A_SUBDIR))
//			{
//				char * text = GetExt(fd.name);
//
//				if (!strcasecmp(text, ".SCN"))
//				{
//					char path3[256];
//					sprintf(path3, "%s%s", partial_path, fd.name);
//					SetExt(path3, ".scn");
//					UNIQUE_HEADER2 * uh2 = (UNIQUE_HEADER2 *)(dat + pos);
//					strcpy(uh2->path, fd.name);
//					pos += sizeof(UNIQUE_HEADER2);
//
//					char check[512];
//					HERMES_CreateFileCheck(path3, check, 512, UNIQUE_VERSION);
//					memcpy(dat + pos, check, 512);
//					pos += 512;
//					count++;
//
//					if (count > 60)
//					{
//						free(dat);
//						return false;
//					}
//				}
//			}
//		}
//		while (!(_findnext(idx, &fd)));
//
//		_findclose(idx);
//	}
	
	uh->count = count;
	string file = path + "fast.fts";
	long compressedstart = pos;
	
	FAST_SCENE_HEADER * fsh = (FAST_SCENE_HEADER *)(dat + pos);
	pos += sizeof(FAST_SCENE_HEADER);
	
	fsh->version = FTS_VERSION;
	fsh->sizex = ACTIVEBKG->Xsize;
	fsh->sizez = ACTIVEBKG->Zsize;
	fsh->nb_textures = 0;
	fsh->playerpos = player.pos;
	fsh->Mscenepos = Mscenepos;
	fsh->nb_anchors = ACTIVEBKG->nbanchors;
	fsh->nb_portals = 0;
	fsh->nb_rooms = 0;
	
	if(portals) {
		fsh->nb_portals = portals->nb_total;
		fsh->nb_rooms = portals->nb_rooms;
	}
	
	fsh->nb_polys = 0;
	
	//Count textures...
	typedef std::map<TextureContainer *, s32> TextureContainerMap;
	TextureContainerMap textures;
	s32 texid = 0;
	for(long j = 0; j < fsh->sizez; j++) {
		for(long i = 0; i < fsh->sizex; i++) {
			for(long k = 0; k < ACTIVEBKG->Backg[i+j*fsh->sizex].nbpoly; k++) {
				
				EERIEPOLY * ep = (EERIEPOLY *)&ACTIVEBKG->Backg[i+j*fsh->sizex].polydata[k];
				
				if(ep && ep->tex) {
					
					if(textures.find(ep->tex) == textures.end()) {
						textures[ep->tex] = ++texid;
						
						FAST_TEXTURE_CONTAINER * ftc = (FAST_TEXTURE_CONTAINER *)(dat + pos);
						ftc->tc = texid;
						strcpy(ftc->fic, ep->tex->m_texName.c_str());
						ftc->temp = 0;
						fsh->nb_textures++;
						pos += sizeof(FAST_TEXTURE_CONTAINER);
						
						checkalloc
					}
				}
			}
		}
	}
	
	for(long j = 0; j < fsh->sizez; j++) {
		for(long i = 0; i < fsh->sizex; i++) {
			FAST_SCENE_INFO * fsi = (FAST_SCENE_INFO *)(dat + pos);
			pos += sizeof(FAST_SCENE_INFO);
			
			checkalloc
			
			fsi->nbianchors = ACTIVEBKG->Backg[i+j*fsh->sizex].nbianchors;
			fsi->nbpoly = ACTIVEBKG->Backg[i+j*fsh->sizex].nbpoly;
			
			for(long k = 0; k < fsi->nbpoly; k++) {
				fsh->nb_polys++;
				
				FAST_EERIEPOLY * ep = (FAST_EERIEPOLY *)(dat + pos);
				EERIEPOLY * ep2 = &ACTIVEBKG->Backg[i+j*fsh->sizex].polydata[k];
				pos += sizeof(FAST_EERIEPOLY);
				
				checkalloc
				
				ep->room = ep2->room;
				ep->paddy = 0;
				ep->area = ep2->area;
				ep->norm = ep2->norm;
				ep->norm2 = ep2->norm2;
				std::copy(ep2->nrml, ep2->nrml + 4, ep->nrml);
				ep->tex = textures[ep2->tex];
				ep->transval = ep2->transval;
				ep->type = ep2->type;
				
				for(long kk = 0; kk < 4; kk++) {
					ep->v[kk].ssx = ep2->v[kk].sx;
					ep->v[kk].sy = ep2->v[kk].sy;
					ep->v[kk].ssz = ep2->v[kk].sz;
					ep->v[kk].stu = ep2->v[kk].tu;
					ep->v[kk].stv = ep2->v[kk].tv;
				}
			}
			
			for(long k = 0; k < fsi->nbianchors; k++) {
				s32 * ianch = (s32 *)(dat + pos);
				pos += sizeof(s32);
				checkalloc
				*ianch = ACTIVEBKG->Backg[i+j*fsh->sizex].ianchors[k];
			}
		}
	}
	
	for(long i = 0; i < ACTIVEBKG->nbanchors; i++) {
		
		FAST_ANCHOR_DATA * fad = (FAST_ANCHOR_DATA *)(dat + pos);
		pos += sizeof(FAST_ANCHOR_DATA);
		
		checkalloc
		
		fad->flags = ACTIVEBKG->anchors[i].flags;
		fad->pos = ACTIVEBKG->anchors[i].pos;
		fad->nb_linked = ACTIVEBKG->anchors[i].nblinked;
		fad->radius = ACTIVEBKG->anchors[i].radius;
		fad->height = ACTIVEBKG->anchors[i].height;
		
		for(long kk = 0; kk < fad->nb_linked; kk++) {
			s32 * lng = (s32 *)(dat + pos);
			pos += sizeof(s32);
			checkalloc
			*lng = ACTIVEBKG->anchors[i].linked[kk];
		}
	}
	
	if(portals) {
		
		for(long i = 0; i < portals->nb_total; i++) {
			
			EERIE_SAVE_PORTALS * epo = (EERIE_SAVE_PORTALS *)(dat + pos);
			pos += sizeof(EERIE_SAVE_PORTALS);
			
			const EERIE_PORTALS & portal = portals->portals[i];
			
			epo->room_1 = portal.room_1;
			epo->room_2 = portal.room_2;
			epo->useportal = portal.useportal;
			epo->paddy = portal.paddy;
			epo->poly.area = portal.poly.area;
			epo->poly.type = portal.poly.type;
			epo->poly.transval = portal.poly.transval;
			epo->poly.room = portal.poly.room;
			epo->poly.misc = portal.poly.misc;
			epo->poly.center = portal.poly.center;
			epo->poly.max = portal.poly.max;
			epo->poly.min = portal.poly.min;
			epo->poly.norm = portal.poly.norm;
			epo->poly.norm2 = portal.poly.norm2;
			
			copy(portal.poly.nrml, portal.poly.nrml + 4, epo->poly.nrml);
			copy(portal.poly.v, portal.poly.v + 4, epo->poly.v);
			copy(portal.poly.tv, portal.poly.tv + 4, epo->poly.tv);
		}
		
		for(long i = 0; i < portals->nb_rooms + 1; i++) {
			
			EERIE_SAVE_ROOM_DATA * erd = (EERIE_SAVE_ROOM_DATA *)(dat + pos);
			pos += sizeof(EERIE_SAVE_ROOM_DATA);
			
			memset(erd, 0, sizeof(EERIE_SAVE_ROOM_DATA));
			erd->nb_polys = portals->room[i].nb_polys;
			erd->nb_portals = portals->room[i].nb_portals;
			
			for(long jj = 0; jj < portals->room[i].nb_portals; jj++) {
				s32 * lng = (s32 *)(dat + pos);
				pos += sizeof(s32);
				*lng = portals->room[i].portals[jj];
			}
			
			if(portals->room[i].nb_polys) {
				EP_DATA * ed = (EP_DATA *)(dat + pos);
				pos += sizeof(EP_DATA) * portals->room[i].nb_polys;
				memcpy(ed, portals->room[i].epdata, sizeof(EP_DATA)*portals->room[i].nb_polys);
			}
		}
	}
	
	if(!RoomDistance) {
		ComputeRoomDistance();
	}
	
	if(portals && RoomDistance && NbRoomDistance > 0) {
		for(long n = 0; n < NbRoomDistance; n++) {
			for(long m = 0; m < NbRoomDistance; m++) {
				ROOM_DIST_DATA_SAVE * rdds = (ROOM_DIST_DATA_SAVE *)(dat + pos);
				pos += sizeof(ROOM_DIST_DATA_SAVE);
				EERIE_3D start;
				EERIE_3D end;
				rdds->distance = GetRoomDistance(m, n, &start, &end);
				rdds->startpos = start;
				rdds->endpos = end;
			}
		}
	}
	
	// Now Saving Whole Buffer
	uh->uncompressedsize = pos - compressedstart;
	
	FileHandle handle = FileOpenWrite(file);
	if(!handle) {
		free(dat);
		return false;
	}
	
	if(FileWrite(handle, dat, compressedstart) != compressedstart) {
		free(dat);
		return false;
	}
	
	size_t compressedSize;
	char * compressed;
	compressed = implodeAlloc((char *)(dat + compressedstart), pos - compressedstart, compressedSize);
	if(!compressed) {
		LogError << "error compressing scene";
		free(dat);
		return false;
	}
	
	if((size_t)FileWrite(handle, compressed, compressedSize) != compressedSize) {
		FileClose(handle);
		free(dat);
		return false;
	}
	
	delete[] compressed;
	FileClose(handle);
	free(dat);
	
	return true;
}

void SceneAddMultiScnToBackground(EERIE_MULTI3DSCENE * ms) {
	
	std::string ftemp = LastLoadedScene;
	RemoveName(ftemp);
	
	// First Release Any Portal Data
	EERIE_PORTAL_Release();
	
	// Try to Load Fast Scene
	if(!FastSceneLoad(ftemp)) {
		
		//failure: Not-Fast Load
		
		for(long j = 0; j < ms->nb_scenes; j++) {
			EERIE_3DSCENE * escn = ms->scenes[j];
			for(long i = 0; i < escn->nbobj; i++) {
				escn->objs[i]->pos += ms->pos;
				SceneAddObjToBackground(escn->objs[i]);
			}
		}
		
		EERIE_LIGHT_MoveAll(&ms->pos);
		BKG_VerticalReOrder(ACTIVEBKG);
		PrepareBackgroundNRMLs();
		EERIEPOLY_Compute_PolyIn();
		EERIE_PORTAL_Blend_Portals_And_Rooms();
		
		if(NEED_ANCHORS) {
			AnchorData_Create(ACTIVEBKG);
		}
		
		FastSceneSave(ftemp);
		ComputePortalVertexBuffer();
		ComputeRoomDistance();
	}
	
}

//*************************************************************************************
//*************************************************************************************
static void EERIEAddPolyToBackground(D3DTLVERTEX * vert2, TextureContainer * tex, long render, float transval, EERIE_3DOBJ * eobj)
{
	EERIEPOLY ep;

	memset(ep.tv, 0, sizeof(D3DTLVERTEX) * 3);

	if (vert2 != NULL)
	{
		memcpy(ep.v, vert2, sizeof(D3DTLVERTEX) * 3);
	}
	else memset(ep.tv, 0, sizeof(D3DTLVERTEX) * 3);

	ep.type = render;
	ep.tex = tex;
	ep.transval = transval;
	BkgAddPoly(&ep, eobj);
}
void EERIEPOLY_FillMissingVertex(EERIEPOLY * po, EERIEPOLY * ep)
{
	long missing = -1;

	for (long i = 0; i < 3; i++)
	{
		long same = 0;

		for (long j = 0; j < 3; j++)
		{
			if	((po->v[j].sx == ep->v[i].sx)
					&&	(po->v[j].sy == ep->v[i].sy)
					&&	(po->v[j].sz == ep->v[i].sz))
				same = 1;
		}

		if (!same) missing = i;
	}

	if (missing >= 0)
	{
		EERIE_3D temp;
		temp.x = po->v[2].sx;
		temp.y = po->v[2].sy;
		temp.z = po->v[2].sz;

		po->v[2].sx = ep->v[missing].sx;
		po->v[2].sy = ep->v[missing].sy;
		po->v[2].sz = ep->v[missing].sz;

		po->v[3].sx = temp.x;
		po->v[3].sy = temp.y;
		po->v[3].sz = temp.z;
		po->type |= POLY_QUAD;
	}
}

struct SINFO_TEXTURE_VERTEX
{
	int					iNbVertex;
	int					iNbIndiceCull;
	int					iNbIndiceNoCull;
	int					iNbIndiceCull_TMultiplicative;
	int					iNbIndiceNoCull_TMultiplicative;
	int					iNbIndiceCull_TAdditive;
	int					iNbIndiceNoCull_TAdditive;
	int					iNbIndiceCull_TNormalTrans;
	int					iNbIndiceNoCull_TNormalTrans;
	int					iNbIndiceCull_TSubstractive;
	int					iNbIndiceNoCull_TSubstractive;
	TextureContainer	* pTex;
	int					iMin;
	int					iMax;
};

void SceneAddObjToBackground(EERIE_3DOBJ * eobj)
{
	float       Xcos, Ycos, Zcos, Xsin, Ysin, Zsin;
	EERIE_3D      p, rp;

	D3DTLVERTEX vlist[3];
	Zsin = radians(eobj->angle.a);
	Xcos = (float)EEcos(Zsin);
	Xsin = (float)EEsin(Zsin);
	Zsin = radians(eobj->angle.b);
	Ycos = (float)EEcos(Zsin);
	Ysin = (float)EEsin(Zsin);
	Zsin = radians(eobj->angle.g);
	Zcos = (float)EEcos(Zsin);
	Zsin = (float)EEsin(Zsin);

	for (size_t i = 0; i < eobj->vertexlist.size(); i++)
	{
		//Local Transform
		p = eobj->vertexlist[i].v - eobj->point0;
		_YRotatePoint(&p, &rp, Ycos, Ysin);
		_XRotatePoint(&rp, &p, Xcos, Xsin);
		_ZRotatePoint(&p, &rp, Zcos, Zsin);
		eobj->vertexlist[i].vert.sx = rp.x + eobj->pos.x + eobj->point0.x;
		eobj->vertexlist[i].vert.sy = rp.y + eobj->pos.y + eobj->point0.y;
		eobj->vertexlist[i].vert.sz = rp.z + eobj->pos.z + eobj->point0.z;
	}

	long type, val1, val2;

	if (COMPUTE_PORTALS)
	{
		if (GetNameInfo(eobj->name, type, val1, val2))
		{
			if (type == TYPE_PORTAL)
			{
				EERIEPOLY ep;
				EERIEPOLY epp;

				for (size_t i = 0; i < eobj->facelist.size(); i++)
				{
					for (long kk = 0; kk < 3; kk++)
					{
						memcpy(&ep.v[kk], &eobj->vertexlist[eobj->facelist[i].vid[kk]].vert, sizeof(D3DTLVERTEX));
					}

					if (i == 0)
					{
						memcpy(&epp, &ep, sizeof(EERIEPOLY));
						epp.type = 0;
					}
					else if (i == 1)
					{
						EERIEPOLY_FillMissingVertex(&epp, &ep);
					}
					else break;
				}

				if(!eobj->facelist.empty()) {
					EERIE_PORTAL_Poly_Add(&epp, eobj->name, -1, -1, -1);
				}

				return;
			}
		}
	}
	else
	{
		if (GetNameInfo(eobj->name, type, val1, val2))
		{
			if (type == TYPE_PORTAL)
			{
				return;
			}
		}
	}

	for (size_t i = 0; i < eobj->facelist.size(); i++)
	{
		vlist[0] = eobj->vertexlist[eobj->facelist[i].vid[0]].vert;
		vlist[1] = eobj->vertexlist[eobj->facelist[i].vid[1]].vert;
		vlist[2] = eobj->vertexlist[eobj->facelist[i].vid[2]].vert;

		if (eobj->facelist[i].facetype & 1)
		{
			vlist[0].color = vlist[1].color = vlist[2].color = D3DCOLORWHITE;
			vlist[0].tu = eobj->facelist[i].u[0];
			vlist[0].tv = eobj->facelist[i].v[0];
			vlist[1].tu = eobj->facelist[i].u[1];
			vlist[1].tv = eobj->facelist[i].v[1];
			vlist[2].tu = eobj->facelist[i].u[2];
			vlist[2].tv = eobj->facelist[i].v[2];

			if (eobj->facelist[i].texid >= 0)
				EERIEAddPolyToBackground(vlist,eobj->texturecontainer[eobj->facelist[i].texid],eobj->facelist[i].facetype,eobj->facelist[i].transval,eobj);
		}
		else
		{
			vlist[0].color = 0xFFFFFFFF;
			vlist[1].color = 0xFFFFFFFF;
			vlist[2].color = 0xFFFFFFFF;
			EERIEAddPolyToBackground(vlist, NULL, eobj->facelist[i].facetype, eobj->facelist[i].transval, eobj);
		}
	}
}


void EERIE_PORTAL_ReleaseOnlyVertexBuffer()
{
	if (portals)
	{
		if (portals->room)
		{
			if (portals->nb_rooms > 0)
			{
				for (long nn = 0; nn < portals->nb_rooms + 1; nn++)
				{
					portals->room[nn].usNbTextures = 0;

					if (portals->room[nn].pVertexBuffer)
					{
						portals->room[nn].pVertexBuffer->Release();
						portals->room[nn].pVertexBuffer = NULL;
					}

					if (portals->room[nn].pussIndice)
					{
						free((void *)portals->room[nn].pussIndice);
						portals->room[nn].pussIndice = NULL;
					}

					if (portals->room[nn].ppTextureContainer)
					{
						free((void *)portals->room[nn].ppTextureContainer);
						portals->room[nn].ppTextureContainer = NULL;
					}
				}
			}
		}
	}
}

extern DANAE danaeApp;

struct COPY3D
{
	float	x, y, z;
	int		color;
	float	u, v;
};

vector<COPY3D> vCopy3d;

//-----------------------------------------------------------------------------
void IncrementPolyWithNormal(EERIEPOLY * _pPoly, float _fFactor)
{
	if ((pMenuConfig) &&
			(pMenuConfig->bForceZBias))
	{
		_pPoly->v[0].sx += _pPoly->norm.x * _fFactor;
		_pPoly->v[0].sy += _pPoly->norm.y * _fFactor;
		_pPoly->v[0].sz += _pPoly->norm.z * _fFactor;
		_pPoly->v[1].sx += _pPoly->norm.x * _fFactor;
		_pPoly->v[1].sy += _pPoly->norm.y * _fFactor;
		_pPoly->v[1].sz += _pPoly->norm.z * _fFactor;
		_pPoly->v[2].sx += _pPoly->norm.x * _fFactor;
		_pPoly->v[2].sy += _pPoly->norm.y * _fFactor;
		_pPoly->v[2].sz += _pPoly->norm.z * _fFactor;

		if (_pPoly->type & POLY_QUAD)
		{
			_pPoly->v[3].sx += _pPoly->norm2.x * _fFactor;
			_pPoly->v[3].sy += _pPoly->norm2.y * _fFactor;
			_pPoly->v[3].sz += _pPoly->norm2.z * _fFactor;
		}
	}
}
//-----------------------------------------------------------------------------
void ComputePortalVertexBuffer()
{
	if (!portals) return;

	if (COMPUTE_PORTALS)
	{
		EERIE_PORTAL_ReleaseOnlyVertexBuffer();

		vector<SINFO_TEXTURE_VERTEX *> vTextureVertex;

		int iMaxRoom = min(portals->nb_rooms, 255L);

		if (portals->nb_rooms > 255)
		{
			char tTxt[256];
			sprintf(tTxt, "rooms > 255");
			LogError<<tTxt<<" Error Portals";
			return;
		}

		for (int iNb = 0; iNb <= iMaxRoom; iNb++)
		{
			EERIE_ROOM_DATA * pRoom = &portals->room[iNb];

			if (!pRoom->nb_polys)
			{
				continue;
			}

			vTextureVertex.clear();

			//check le nombre de vertexs par room + liste de texturecontainer*
			int iNbVertexForRoom = 0;
			int iNbIndiceForRoom = 0;

			for (int iNbPoly = 0; iNbPoly < pRoom->nb_polys; iNbPoly++)
			{
				int iPx = pRoom->epdata[iNbPoly].px;
				int iPy = pRoom->epdata[iNbPoly].py;
				EERIE_BKG_INFO * pBkgInfo = &ACTIVEBKG->Backg[iPx+iPy*ACTIVEBKG->Xsize];

				EERIEPOLY * pPoly = &pBkgInfo->polydata[pRoom->epdata[iNbPoly].idx];

				if ((pPoly->type & POLY_IGNORE) ||
						(pPoly->type & POLY_HIDE) ||
						(!pPoly->tex)) continue;

				if (!pPoly->tex->tMatRoom)
				{
					pPoly->tex->tMatRoom = (SMY_ARXMAT *)malloc(sizeof(SMY_ARXMAT) * (iMaxRoom + 1));
				}

				SINFO_TEXTURE_VERTEX * pTextureVertex = NULL;
				vector<SINFO_TEXTURE_VERTEX *>::iterator it;

				for (it = vTextureVertex.begin(); it < vTextureVertex.end(); it++)
				{
					if (((*it)->pTex) == pPoly->tex)
					{
						pTextureVertex = *it;
						break;
					}
				}

				if (!pTextureVertex)
				{
					pTextureVertex = (SINFO_TEXTURE_VERTEX *)malloc(sizeof(SINFO_TEXTURE_VERTEX));
					memset(pTextureVertex, 0, sizeof(SINFO_TEXTURE_VERTEX));
					pTextureVertex->pTex = pPoly->tex;
					vTextureVertex.insert(vTextureVertex.end(), pTextureVertex);
				}

				int iNbVertex;
				int iNbIndice;

				if (pPoly->type & POLY_QUAD)
				{
					iNbVertex = 4;
					iNbIndice = 6;
				}
				else
				{
					iNbVertex = 3;
					iNbIndice = 3;
				}

				pTextureVertex->iNbVertex += iNbVertex;

				if (pPoly->type & POLY_DOUBLESIDED)
				{
					if (pPoly->type & POLY_TRANS)
					{
						float fTransp = pPoly->transval;

						if (pPoly->transval >= 2.f) //MULTIPLICATIVE
						{
							pTextureVertex->iNbIndiceNoCull_TMultiplicative += iNbIndice;
							fTransp *= ( 1.0f / 2 );
							fTransp += .5f;
						}
						else
						{
							if (pPoly->transval >= 1.f) //ADDITIVE
							{
								pTextureVertex->iNbIndiceNoCull_TAdditive += iNbIndice;
								fTransp -= 1.f;
							}
							else
							{
								if (pPoly->transval > 0.f) //NORMAL TRANS
								{
									pTextureVertex->iNbIndiceNoCull_TNormalTrans += iNbIndice;
									fTransp = 1.f - fTransp;
								}
								else
								{
									//SUBTRACTIVE
									pTextureVertex->iNbIndiceNoCull_TSubstractive += iNbIndice;
									fTransp = 1.f - fTransp;

									IncrementPolyWithNormal(pPoly, 2.f);
								}
							}
						}

						pPoly->v[3].color = pPoly->v[2].color = pPoly->v[1].color = pPoly->v[0].color = D3DRGB(fTransp, fTransp, fTransp);
					}
					else
					{
						pTextureVertex->iNbIndiceNoCull += iNbIndice;
					}
				}
				else
				{
					if (pPoly->type & POLY_TRANS)
					{
						float fTransp = pPoly->transval;

						if (pPoly->transval >= 2.f) //MULTIPLICATIVE
						{
							pTextureVertex->iNbIndiceCull_TMultiplicative += iNbIndice;
							fTransp *= ( 1.0f / 2 );
							fTransp += .5f;
						}
						else
						{
							if (pPoly->transval >= 1.f) //ADDITIVE
							{
								pTextureVertex->iNbIndiceCull_TAdditive += iNbIndice;
								fTransp -= 1.f;
							}
							else
							{
								if (pPoly->transval > 0.f) //NORMAL TRANS
								{
									pTextureVertex->iNbIndiceCull_TNormalTrans += iNbIndice;
									fTransp = 1.f - fTransp;
								}
								else
								{
									//SUBTRACTIVE
									pTextureVertex->iNbIndiceCull_TSubstractive += iNbIndice;
									fTransp = 1.f - fTransp;

									IncrementPolyWithNormal(pPoly, 2.f);
								}
							}
						}

						pPoly->v[3].color = pPoly->v[2].color = pPoly->v[1].color = pPoly->v[0].color = D3DRGB(fTransp, fTransp, fTransp);
					}
					else
					{
						pTextureVertex->iNbIndiceCull += iNbIndice;
					}
				}

				iNbVertexForRoom += iNbVertex;
				iNbIndiceForRoom += iNbIndice;
			}

			if (!iNbVertexForRoom)
			{
				char tTxt[256];
				sprintf(tTxt, "portals %d - Zero Vertex", iNb);
				LogError << tTxt<< " Error Portals";

				vector<SINFO_TEXTURE_VERTEX *>::iterator it;

				for (it = vTextureVertex.begin(); it < vTextureVertex.end(); it++)
				{
					free((void *)*it);
				}

				continue;
			}

			pRoom->pussIndice = (unsigned short *)malloc(sizeof(unsigned short) * iNbIndiceForRoom);
			int iFlag = D3DVBCAPS_WRITEONLY;

			if (!(danaeApp.m_pDeviceInfo->ddDeviceDesc.dwDevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT))
			{
				iFlag |= D3DVBCAPS_SYSTEMMEMORY;
			}

			D3DVERTEXBUFFERDESC d3dvbufferdesc;
			d3dvbufferdesc.dwSize = sizeof(D3DVERTEXBUFFERDESC);
			d3dvbufferdesc.dwCaps = iFlag;
			d3dvbufferdesc.dwFVF = FVF_D3DVERTEX;
			d3dvbufferdesc.dwNumVertices = iNbVertexForRoom;

			pRoom->pVertexBuffer = NULL;

			if (FAILED(danaeApp.m_pD3D->CreateVertexBuffer(&d3dvbufferdesc,
					   &pRoom->pVertexBuffer,
					   0)))
			{
				if (pRoom->pussIndice)
				{
					free((void *)pRoom->pussIndice);
					pRoom->pussIndice = NULL;
				}

				char tTxt[256];
				sprintf(tTxt, "CreateVertexBuffer - NbVertexs %d", iNbVertexForRoom);
				LogError << tTxt<< " Error TransForm";
				return;
			}


			SMY_D3DVERTEX * pVertex;

			if (FAILED(pRoom->pVertexBuffer->Lock(DDLOCK_WRITEONLY | DDLOCK_NOOVERWRITE,
												  (void **)&pVertex,
												  NULL)))
			{
				pRoom->pVertexBuffer->Release();
				pRoom->pVertexBuffer = NULL;

				if (pRoom->pussIndice)
				{
					free((void *)pRoom->pussIndice);
					pRoom->pussIndice = NULL;
				}

				return;
			}

			int iStartVertex = 0;
			int iStartCull = 0;
			std::vector<SINFO_TEXTURE_VERTEX *>::iterator it;

			for (it = vTextureVertex.begin(); it < vTextureVertex.end(); it++)
			{
				TextureContainer * pTextureContainer = (*it)->pTex;

				unsigned short iIndiceInVertex = 0;

				for (int iNbPoly = 0; iNbPoly < pRoom->nb_polys; iNbPoly++)
				{
					int iPx = pRoom->epdata[iNbPoly].px;
					int iPy = pRoom->epdata[iNbPoly].py;
					EERIE_BKG_INFO * pBkgInfo = &ACTIVEBKG->Backg[iPx+iPy*ACTIVEBKG->Xsize];

					EERIEPOLY * pPoly = &pBkgInfo->polydata[pRoom->epdata[iNbPoly].idx];

					if ((pPoly->type & POLY_IGNORE) ||
							(pPoly->type & POLY_HIDE) ||
							(!pPoly->tex)) continue;

					if (pPoly->tex == pTextureContainer)
					{
						pVertex->x = pPoly->v[0].sx;
						pVertex->y = -(pPoly->v[0].sy);
						pVertex->z = pPoly->v[0].sz;
						pVertex->color = pPoly->v[0].color;
						pVertex->tu = pPoly->v[0].tu + pTextureContainer->m_hdx;
						pVertex->tv = pPoly->v[0].tv + pTextureContainer->m_hdy;
						pVertex++;

						pVertex->x = pPoly->v[1].sx;
						pVertex->y = -(pPoly->v[1].sy);
						pVertex->z = pPoly->v[1].sz;
						pVertex->color = pPoly->v[1].color;
						pVertex->tu = pPoly->v[1].tu + pTextureContainer->m_hdx;
						pVertex->tv = pPoly->v[1].tv + pTextureContainer->m_hdy;
						pVertex++;

						pVertex->x = pPoly->v[2].sx;
						pVertex->y = -(pPoly->v[2].sy);
						pVertex->z = pPoly->v[2].sz;
						pVertex->color = pPoly->v[2].color;
						pVertex->tu = pPoly->v[2].tu + pTextureContainer->m_hdx;
						pVertex->tv = pPoly->v[2].tv + pTextureContainer->m_hdy;
						pVertex++;

						pPoly->uslInd[0] = iIndiceInVertex++;
						pPoly->uslInd[1] = iIndiceInVertex++;
						pPoly->uslInd[2] = iIndiceInVertex++;

						if (pPoly->type & POLY_QUAD)
						{
							pVertex->x = pPoly->v[3].sx;
							pVertex->y = -(pPoly->v[3].sy);
							pVertex->z = pPoly->v[3].sz;
							pVertex->color = pPoly->v[3].color;
							pVertex->tu = pPoly->v[3].tu + pTextureContainer->m_hdx;
							pVertex->tv = pPoly->v[3].tv + pTextureContainer->m_hdy;
							pVertex++;

							pPoly->uslInd[3] = iIndiceInVertex++;
						}
					}
				}

				//texturecontainer
				pRoom->usNbTextures++;
                pRoom->ppTextureContainer = (TextureContainer **)realloc(pRoom->ppTextureContainer, sizeof(TextureContainer *) * pRoom->usNbTextures);
				pRoom->ppTextureContainer[pRoom->usNbTextures-1] = pTextureContainer;

				pTextureContainer->tMatRoom[iNb].uslStartVertex = iStartVertex;
				pTextureContainer->tMatRoom[iNb].uslNbVertex = iIndiceInVertex;

				pTextureContainer->tMatRoom[iNb].uslStartCull = iStartCull;
				pTextureContainer->tMatRoom[iNb].uslNbIndiceCull = 0;
				pTextureContainer->tMatRoom[iNb].uslStartNoCull = pTextureContainer->tMatRoom[iNb].uslStartCull + (*it)->iNbIndiceCull;
				pTextureContainer->tMatRoom[iNb].uslNbIndiceNoCull = 0;

				pTextureContainer->tMatRoom[iNb].uslStartCull_TNormalTrans = pTextureContainer->tMatRoom[iNb].uslStartNoCull + (*it)->iNbIndiceNoCull;
				pTextureContainer->tMatRoom[iNb].uslNbIndiceCull_TNormalTrans = 0;
				pTextureContainer->tMatRoom[iNb].uslStartNoCull_TNormalTrans = pTextureContainer->tMatRoom[iNb].uslStartCull_TNormalTrans + (*it)->iNbIndiceCull_TNormalTrans;
				pTextureContainer->tMatRoom[iNb].uslNbIndiceNoCull_TNormalTrans = 0;

				pTextureContainer->tMatRoom[iNb].uslStartCull_TMultiplicative = pTextureContainer->tMatRoom[iNb].uslStartNoCull_TNormalTrans + (*it)->iNbIndiceNoCull_TNormalTrans;
				pTextureContainer->tMatRoom[iNb].uslNbIndiceCull_TMultiplicative = 0;
				pTextureContainer->tMatRoom[iNb].uslStartNoCull_TMultiplicative = pTextureContainer->tMatRoom[iNb].uslStartCull_TMultiplicative + (*it)->iNbIndiceCull_TMultiplicative;
				pTextureContainer->tMatRoom[iNb].uslNbIndiceNoCull_TMultiplicative = 0;

				pTextureContainer->tMatRoom[iNb].uslStartCull_TAdditive = pTextureContainer->tMatRoom[iNb].uslStartNoCull_TMultiplicative + (*it)->iNbIndiceNoCull_TMultiplicative;
				pTextureContainer->tMatRoom[iNb].uslNbIndiceCull_TAdditive = 0;
				pTextureContainer->tMatRoom[iNb].uslStartNoCull_TAdditive = pTextureContainer->tMatRoom[iNb].uslStartCull_TAdditive + (*it)->iNbIndiceCull_TAdditive;
				pTextureContainer->tMatRoom[iNb].uslNbIndiceNoCull_TAdditive = 0;

				pTextureContainer->tMatRoom[iNb].uslStartCull_TSubstractive = pTextureContainer->tMatRoom[iNb].uslStartNoCull_TAdditive + (*it)->iNbIndiceNoCull_TAdditive;
				pTextureContainer->tMatRoom[iNb].uslNbIndiceCull_TSubstractive = 0;
				pTextureContainer->tMatRoom[iNb].uslStartNoCull_TSubstractive = pTextureContainer->tMatRoom[iNb].uslStartCull_TSubstractive + (*it)->iNbIndiceCull_TSubstractive;
				pTextureContainer->tMatRoom[iNb].uslNbIndiceNoCull_TSubstractive = 0;

				if (((*it)->iNbIndiceCull > 65535) ||
						((*it)->iNbIndiceNoCull > 65535) ||
						((*it)->iNbIndiceCull_TNormalTrans > 65535) ||
						((*it)->iNbIndiceNoCull_TNormalTrans > 65535) ||
						((*it)->iNbIndiceCull_TMultiplicative > 65535) ||
						((*it)->iNbIndiceNoCull_TMultiplicative > 65535) ||
						((*it)->iNbIndiceCull_TAdditive > 65535) ||
						((*it)->iNbIndiceNoCull_TAdditive > 65535) ||
						((*it)->iNbIndiceCull_TSubstractive > 65535) ||
						((*it)->iNbIndiceNoCull_TSubstractive > 65535))
				{
					char tTxt[256];
					sprintf(tTxt, "CreateVertexBuffer - Indices>65535");
					LogError << tTxt<<" Error TransForm";
				}

				iStartCull +=	(*it)->iNbIndiceCull +
								(*it)->iNbIndiceNoCull +
								(*it)->iNbIndiceCull_TNormalTrans +
								(*it)->iNbIndiceNoCull_TNormalTrans +
								(*it)->iNbIndiceCull_TMultiplicative +
								(*it)->iNbIndiceNoCull_TMultiplicative +
								(*it)->iNbIndiceCull_TAdditive +
								(*it)->iNbIndiceNoCull_TAdditive +
								(*it)->iNbIndiceCull_TSubstractive +
								(*it)->iNbIndiceNoCull_TSubstractive;

				iStartVertex += iIndiceInVertex;

				free((void *)(*it));
			}

			if (FAILED(pRoom->pVertexBuffer->Unlock()))
			{
				pRoom->pVertexBuffer->Release();
				pRoom->pVertexBuffer = NULL;

				if (pRoom->pussIndice)
				{
					free((void *)pRoom->pussIndice);
					pRoom->pussIndice = NULL;
				}
			}
		}

		vTextureVertex.clear();
	}
}

long EERIERTPPoly(EERIEPOLY *ep)
{
	specialEE_RTP(&ep->v[0],&ep->tv[0]);
	specialEE_RTP(&ep->v[1],&ep->tv[1]);
	specialEE_RTP(&ep->v[2],&ep->tv[2]);	

	if (ep->type & POLY_QUAD) 
	{
		specialEE_RTP(&ep->v[3],&ep->tv[3]);	

		if ((ep->tv[0].sz<=0.f) &&
			(ep->tv[1].sz<=0.f) &&
			(ep->tv[2].sz<=0.f) &&
			(ep->tv[3].sz<=0.f) ) 
		{
			return 0;
		}
	}
	else
	{
		if ((ep->tv[0].sz<=0.f) &&
			(ep->tv[1].sz<=0.f) &&
			(ep->tv[2].sz<=0.f)  ) 
		{
			return 0;
		}
	}

	return 1;
}
