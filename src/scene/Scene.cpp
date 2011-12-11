/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#include "scene/Scene.h"

#include <cstdio>

#include "ai/Paths.h"

#include "animation/Animation.h"

#include "core/Application.h"
#include "core/GameTime.h"
#include "core/Core.h"

#include "game/Spells.h"
#include "game/Player.h"
#include "game/Inventory.h"

#include "gui/Interface.h"

#include "graphics/VertexBuffer.h"
#include "graphics/Draw.h"
#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/DrawEffects.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/texture/TextureStage.h"

#include "input/Input.h"

#include "io/log/Logger.h"

#include "scene/Light.h"
#include "scene/Interactive.h"

using std::vector;

//-----------------------------------------------------------------------------
#define MAX_OUT 4 
#define VAL_THRESHOLD 100.f
#define PASSS 0.5f 
#define PASS 50.f 

//-----------------------------------------------------------------------------
extern long USE_LIGHT_OPTIM;
extern EERIE_3DOBJ * eyeballobj;
extern long NEED_TEST_TEXT;
extern long EXTERNALVIEW;
extern long WATERFX;
extern long REFLECTFX;
extern bool ARXPausedTimer;
long LAST_PORTALS_COUNT=0;
//-----------------------------------------------------------------------------
extern TextureContainer *enviro;
extern long ZMAPMODE;
extern Color ulBKGColor;
//-----------------------------------------------------------------------------
EERIEPOLY VF_Center;
EERIEPOLY VF_Top;
EERIEPOLY VF_Bottom;
EERIEPOLY VF_Front;

EERIEPOLY * TransPol[MAX_TRANSPOL];
TexturedVertex InterTransPol[MAX_INTERTRANSPOL][4];
EERIE_FACE * InterTransFace[MAX_INTERTRANSPOL];
TextureContainer * InterTransTC[MAX_INTERTRANSPOL];

EERIE_PORTAL_DATA * portals = NULL;

float WATEREFFECT=0.f;
long INTERTRANSPOLYSPOS=0;

long TRANSPOLYSPOS=0;
long SHOWSHADOWS=1;
long TESTMODE=1;
long FRAME_COUNT=0;

long LAST_ROOM=-1;

float fZFogStart=0.3f;
float fZFogEnd=.5f;

long iTotPoly;
unsigned long FrameCount;

CircularVertexBuffer<SMY_VERTEX3> * pDynamicVertexBuffer;

namespace {

struct DynamicVertexBuffer {
	
private:
	
	SMY_VERTEX3 * vertices;
	size_t start;
	
public:
	
	size_t nbindices;
	unsigned short * indices;
	size_t offset;
	
	DynamicVertexBuffer() : vertices(NULL), nbindices(0), indices(NULL) { }
	
	void lock() {
		
		arx_assert(!vertices);
		
		if(!indices) {
			indices = new unsigned short[4 * pDynamicVertexBuffer->vb->capacity()];
			start = 0;
		}
		
		BufferFlags flags = (pDynamicVertexBuffer->pos == 0) ? DiscardBuffer : NoOverwrite | DiscardRange;
		
		vertices =  pDynamicVertexBuffer->vb->lock(flags, pDynamicVertexBuffer->pos);
		offset = 0;
	}
	
	SMY_VERTEX3 * append(size_t nbvertices) {
		
		arx_assert(vertices);
		
		if(pDynamicVertexBuffer->pos + nbvertices > pDynamicVertexBuffer->vb->capacity()) {
			return NULL;
		}
		
		SMY_VERTEX3 * pos = vertices + offset;
		
		pDynamicVertexBuffer->pos += nbvertices, offset += nbvertices;
		
		return pos;
	}
	
	void unlock() {
		arx_assert(vertices);
		pDynamicVertexBuffer->vb->unlock(), vertices = NULL;
	}
	
	void draw(Renderer::Primitive primitive) {
		arx_assert(!vertices);
		pDynamicVertexBuffer->vb->drawIndexed(primitive, pDynamicVertexBuffer->pos - start, start, indices, nbindices);
	}
	
	void done() {
		arx_assert(!vertices);
		start = pDynamicVertexBuffer->pos;
		nbindices = 0;
	}
	
	void reset() {
		arx_assert(!vertices);
		start = pDynamicVertexBuffer->pos = 0;
		nbindices = 0;
	}
	
	~DynamicVertexBuffer() {
		if(indices) {
			delete[] indices;
		}
		
	}
	
} dynamicVertices;

}

EERIE_FRUSTRUM_PLANE efpPlaneNear;
EERIE_FRUSTRUM_PLANE efpPlaneFar;

vector<EERIEPOLY*> vPolyWater;
vector<EERIEPOLY*> vPolyLava;

bool bOLD_CLIPP=false;

void PopAllTriangleListTransparency();

extern long TSU_TEST;

//*************************************************************************************
//*************************************************************************************
void ApplyWaterFXToVertex(Vec3f * odtv,TexturedVertex * dtv,float power)
{
	power=power*0.05f;
	dtv->uv.x+=EEsin((WATEREFFECT+odtv->x))*power;
	dtv->uv.y+=EEcos((WATEREFFECT+odtv->z))*power;
}

static void ApplyLavaGlowToVertex(Vec3f * odtv,TexturedVertex * dtv, float power) {
	register float f;
	register long lr, lg, lb;
	power = 1.f - (EEsin((WATEREFFECT+odtv->x+odtv->z)) * 0.05f) * power;
	f = ((dtv->color >> 16) & 255) * power;
	lr = clipByte(f);

	f = ((dtv->color >> 8) & 255) * power;
	lg = clipByte(f);

	f = ((dtv->color) & 255) * power;
	lb = clipByte(f);

	dtv->color=0xFF000000L | (lr << 16) | (lg << 8) | lb;
}

void ManageLavaWater(EERIEPOLY * ep, const long to, const unsigned long tim)
{
	if ((ep->type & POLY_WATER) || (ep->type & POLY_LAVA) )
	{
		for (long k=0;k<to;k++) 
		{
			ep->tv[k].uv.x=ep->v[k].uv.x;
			ep->tv[k].uv.y=ep->v[k].uv.y;
			
			
			if (ep->type & POLY_LAVA)
			{
				ApplyWaterFXToVertex(&ep->v[k].p, &ep->tv[k], 0.35f); 
				ApplyLavaGlowToVertex(&ep->v[k].p,&ep->tv[k],0.6f);

				if (rnd()>0.995f) 
				{
					if (ep->type & POLY_FALL)
					{
						Vec3f pos;
						pos.x=ep->v[k].p.x+ep->norm.x*20.f;
						pos.y=ep->v[k].p.y+ep->norm.y*20.f;
						pos.z=ep->v[k].p.z+ep->norm.z*20.f;
					}

					
				}
			}
			else ApplyWaterFXToVertex(&ep->v[k].p,&ep->tv[k],0.35f);
		}					
	}	

	if (ep->type & POLY_FALL)
	{
		if (ep->type & POLY_LAVA)
			for (long k=0;k<to;k++) 
			{
				ep->tv[k].uv.y-=(float)(tim)*( 1.0f / 12000 );
			}
			else
				for (long k=0;k<to;k++) 
				{
					ep->tv[k].uv.y-=(float)(tim)*( 1.0f / 1000 );
				}
	}
}

void ManageWater_VertexBuffer(EERIEPOLY * ep, const long to, const unsigned long tim, SMY_VERTEX * _pVertex) {
	
	for (long k=0;k<to;k++) 
	{
		ep->tv[k].uv = ep->v[k].uv;
		
		ApplyWaterFXToVertex(&ep->v[k].p,&ep->tv[k],0.35f);
			
		if(ep->type&POLY_FALL)
		{
			ep->tv[k].uv.y-=(float)(tim)*( 1.0f / 1000 );
		}
		
		_pVertex[ep->uslInd[k]].uv.x = ep->tv[k].uv.x;
		_pVertex[ep->uslInd[k]].uv.y = ep->tv[k].uv.y;
	}					
}

void ManageLava_VertexBuffer(EERIEPOLY * ep, const long to, const unsigned long tim, SMY_VERTEX * _pVertex) {
	
	for (long k=0;k<to;k++) 
	{
		ep->tv[k].uv = ep->v[k].uv;
		
		ApplyWaterFXToVertex(&ep->v[k].p, &ep->tv[k], 0.35f); //0.25f
		ApplyLavaGlowToVertex(&ep->v[k].p, &ep->tv[k], 0.6f);
			
		if(ep->type&POLY_FALL)
		{
			ep->tv[k].uv.y-=(float)(tim)*( 1.0f / 12000 );
		}
		
		_pVertex[ep->uslInd[k]].uv.x=ep->tv[k].uv.x;
		_pVertex[ep->uslInd[k]].uv.y=ep->tv[k].uv.y;
	}					
}



extern EERIEMATRIX ProjectionMatrix;
void specialEE_RTP2(TexturedVertex *in,TexturedVertex *out)
{
	register EERIE_TRANSFORM * et=(EERIE_TRANSFORM *)&ACTIVECAM->transform;
	out->p.x = in->p.x - et->posx;	
	out->p.y = in->p.y - et->posy;
	out->p.z = in->p.z - et->posz;

	register float temp =(out->p.z*et->ycos) - (out->p.x*et->ysin);
	out->p.x = (out->p.z*et->ysin) + (out->p.x*et->ycos);	
	out->p.z = (out->p.y*et->xsin) + (temp*et->xcos);	
	out->p.y = (out->p.y*et->xcos) - (temp*et->xsin);

	float fZTemp = 1.f / out->p.z;
		
	out->p.z = fZTemp * ProjectionMatrix._33 + ProjectionMatrix._43; 
		out->p.x=out->p.x*ProjectionMatrix._11*fZTemp+et->xmod;
		out->p.y=out->p.y*ProjectionMatrix._22*fZTemp+et->ymod;
		out->rhw=fZTemp;
	
}

long EERIERTPPoly2(EERIEPOLY *ep)
{
	specialEE_RTP2(&ep->v[0],&ep->tv[0]);	
	specialEE_RTP2(&ep->v[1],&ep->tv[1]);
	specialEE_RTP2(&ep->v[2],&ep->tv[2]);	

	if (ep->type & POLY_QUAD) specialEE_RTP2(&ep->v[3],&ep->tv[3]);	
	else ep->tv[3].p.z=1.f;

	if ((ep->tv[0].p.z<=0.f) &&
		(ep->tv[1].p.z<=0.f) &&
		(ep->tv[2].p.z<=0.f) &&
		(ep->tv[3].p.z<=0.f) ) return 0;

	return 1;
}


bool IsSphereInFrustrum(float radius,Vec3f * point,EERIE_FRUSTRUM * frustrum);
bool FrustrumsClipSphere(EERIE_FRUSTRUM_DATA * frustrums,EERIE_SPHERE * sphere)
{
	float dists=sphere->origin.x*efpPlaneNear.a + sphere->origin.y*efpPlaneNear.b + sphere->origin.z*efpPlaneNear.c + efpPlaneNear.d;

	if (dists+sphere->radius>0)
	{	
		for (long i=0;i<frustrums->nb_frustrums;i++)
		{
			if (IsSphereInFrustrum(sphere->radius, &sphere->origin, &frustrums->frustrums[i]))
				return false;
		}
	}

	return true;
}

bool VisibleSphere(float x, float y, float z, float radius) {
	
	Vec3f pos(x, y, z);
	if(distSqr(pos, ACTIVECAM->pos) > square(ACTIVECAM->cdepth*0.5f + radius))
		return false;

	long room_num = ARX_PORTALS_GetRoomNumForPosition(&pos);

	if (room_num>=0)
	{
		EERIE_SPHERE sphere;
		sphere.origin = pos;
		sphere.radius = radius;
							
		EERIE_FRUSTRUM_DATA * frustrums=&RoomDraw[room_num].frustrum;

		if (FrustrumsClipSphere(frustrums,&sphere))
			return false;
	}

	return true;
}
bool IsInFrustrum(Vec3f * point,EERIE_FRUSTRUM * frustrum);
bool IsBBoxInFrustrum(EERIE_3D_BBOX * bbox,EERIE_FRUSTRUM * frustrum)
{
	Vec3f point;
	point.x=bbox->min.x;
	point.y=bbox->min.y;
	point.z=bbox->min.z;

	if (!IsInFrustrum(&point,frustrum))
	{
		Vec3f point;
		point.x=bbox->max.x;
		point.y=bbox->min.y;
		point.z=bbox->min.z;

		if (!IsInFrustrum(&point,frustrum))
		{
			Vec3f point;
			point.x=bbox->max.x;
			point.y=bbox->max.y;
			point.z=bbox->min.z;

			if (!IsInFrustrum(&point,frustrum))
			{
				Vec3f point;
				point.x=bbox->min.x;
				point.y=bbox->max.y;
				point.z=bbox->min.z;

				if (!IsInFrustrum(&point,frustrum))
				{
					Vec3f point;
					point.x=bbox->min.x;
					point.y=bbox->min.y;
					point.z=bbox->max.z;

					if (!IsInFrustrum(&point,frustrum))
					{
						Vec3f point;
						point.x=bbox->max.x;
						point.y=bbox->min.y;
						point.z=bbox->max.z;

						if (!IsInFrustrum(&point,frustrum))
						{
							Vec3f point;
							point.x=bbox->max.x;
							point.y=bbox->max.y;
							point.z=bbox->max.z;

							if (!IsInFrustrum(&point,frustrum))
							{
								Vec3f point;
								point.x=bbox->min.x;
								point.y=bbox->max.y;
								point.z=bbox->max.z;

								if (!IsInFrustrum(&point,frustrum))
								{
									return	false;
								}
							}
						}
					}	
				}
			}
		}
	}	

	return true;
}

bool FrustrumsClipBBox3D(EERIE_FRUSTRUM_DATA * frustrums,EERIE_3D_BBOX * bbox)
{
	for (long i=0;i<frustrums->nb_frustrums;i++)
	{
		if (IsBBoxInFrustrum(bbox,&frustrums->frustrums[i]))
			return false;
	}

	return false;
}
PORTAL_ROOM_DRAW * RoomDraw=NULL;
long NbRoomDraw=0;
long * RoomDrawList=NULL;
long NbRoomDrawList=0;
long TotalRoomDrawList=0;

bool ARX_SCENE_PORTAL_Basic_ClipIO(INTERACTIVE_OBJ * io)
{
	if (EDITMODE) return false;

	if (io==inter.iobj[0]) return false;

	if ((io) && (io->ioflags & IO_FORCEDRAW)) return false;

	if (USE_PORTALS && portals)
	{
		Vec3f posi;
		posi.x=io->pos.x;
		posi.y=io->pos.y-20;
		posi.z=io->pos.z;

		if (io->room_flags & 1)
			UpdateIORoom(io);

		long room_num = io->room; 

		{
			if (room_num==-1)
			{
				posi.y=io->pos.y-120;
				room_num=ARX_PORTALS_GetRoomNumForPosition(&posi);
			}

			if (	(room_num>=0) 
				&&	(RoomDraw)
				&&	(RoomDraw[room_num].count))				
			{

			switch (USE_PORTALS)
			{
				case 2:
				case 3:
				case 4:
					EERIE_SPHERE sphere;

					if (io->ioflags & IO_ITEM)
					{
						sphere.origin.x=io->pos.x;
						sphere.origin.y=io->pos.y-40.f;
						sphere.origin.z=io->pos.z;

						if (io->ioflags & IO_MOVABLE)
							sphere.radius=160.f;
							else sphere.radius = 75.f; 
					}
					else if (io->ioflags & IO_FIX)
					{
						sphere.origin.x=io->pos.x;
						sphere.origin.y=io->pos.y-60.f;
						sphere.origin.z=io->pos.z;
						sphere.radius=340.f;
					}
					else if (io->ioflags & IO_NPC)
					{
						sphere.origin.x=io->pos.x;
						sphere.origin.y=io->pos.y-120.f;
						sphere.origin.z=io->pos.z;
						sphere.radius=120.f;
					}
						
					EERIE_FRUSTRUM_DATA * frustrums=&RoomDraw[room_num].frustrum;

					if (FrustrumsClipSphere(frustrums,&sphere))
					{
						if (io)
						{
							io->bbox1.x=(short)-1;
							io->bbox2.x=(short)-1;
							io->bbox1.y=(short)-1;
							io->bbox2.y=(short)-1;		
						}

						return true;
					}
			}
			}
			else return false;
		}
	}

	return false;
}
//*********************************************************************************************************************
// bool ARX_SCENE__PORTAL_ClipIO(INTERACTIVE_OBJ * io,EERIE_3DOBJ * eobj,EERIE_3D * position,EERIE_3D * bboxmin,EERIE_3D * bboxmax)
//---------------------------------------------------------------------------------------------------------------------
// USAGE/FUNCTION
//   io can be NULL if io is valid io->bbox3D contains 3D world-bbox
//   bboxmin & bboxmax ARE in fact 2D-screen BBOXes using only (x,y).
// RETURN:
//   return true if IO cannot be seen, false if visible
//---------------------------------------------------------------------------------------------------------------------
// TODO:
//   Implement all Portal Methods
//   Return a reduced clipbox which can be used for polys clipping in the case of partial visibility
//*********************************************************************************************************************
bool ARX_SCENE_PORTAL_ClipIO(INTERACTIVE_OBJ * io, Vec3f * position) {
	
	if (EDITMODE) return false;

	if (io==inter.iobj[0]) return false;

	if ((io) && (io->ioflags & IO_FORCEDRAW)) return false;

	if (USE_PORTALS && portals)
	{
		Vec3f posi;
		posi.x=position->x;
		posi.y=position->y-60; //20
		posi.z=position->z;
		long room_num;

		if (io)
		{
			if (io->room_flags & 1)
				UpdateIORoom(io);

			room_num=io->room;//
		}
		else
		{
			room_num=ARX_PORTALS_GetRoomNumForPosition(&posi);
		}

		if (room_num==-1)
		{
			posi.y=position->y-120;
			room_num=ARX_PORTALS_GetRoomNumForPosition(&posi);
		}

		if ((room_num >= 0) && (RoomDraw)) 
		{
			if (RoomDraw[room_num].count==0)
			{
				if (io)
				{
					io->bbox1.x=(short)-1;
					io->bbox2.x=(short)-1;
					io->bbox1.y=(short)-1;
					io->bbox2.y=(short)-1;		
				}

				return true;
			}

			switch (USE_PORTALS)
			{
				case 1: // 2D portal
				{
					EERIE_2D_BBOX * bbox=&RoomDraw[room_num].bbox;

					if (	bbox->min.x > BBOXMAX.x || BBOXMIN.x > bbox->max.x
						||	bbox->min.y > BBOXMAX.y || BBOXMIN.y > bbox->max.y)
					{
						if (io)
						{
							io->bbox1.x=(short)-1;
							io->bbox2.x=(short)-1;
							io->bbox1.y=(short)-1;
							io->bbox2.y=(short)-1;		
						}

						return true;
					}
				}
				break;
				case 2:
				case 3:
				case 4:

					if(io) {
						
						EERIE_SPHERE sphere;
						sphere.origin = (io->bbox3D.min + io->bbox3D.max) * .5f;
						sphere.radius = dist(sphere.origin, io->bbox3D.min) + 10.f;
						
						EERIE_FRUSTRUM_DATA * frustrums=&RoomDraw[room_num].frustrum;

						if (FrustrumsClipSphere(frustrums,&sphere))
						{
							if (io)
							{
								io->bbox1.x=(short)-1;
								io->bbox2.x=(short)-1;
								io->bbox1.y=(short)-1;
								io->bbox2.y=(short)-1;		
							}

							return true;
						}

						if (FrustrumsClipBBox3D(frustrums,&io->bbox3D))
						{
							if (io)
							{
								io->bbox1.x=(short)-1;
								io->bbox2.x=(short)-1;
								io->bbox1.y=(short)-1;
								io->bbox2.y=(short)-1;		
							}

							return true;
						}
					}

				break;
			}
		}
	}

	return false;
}

long ARX_PORTALS_GetRoomNumForPosition2(Vec3f * pos,long flag,float * height)
{
	
	EERIEPOLY * ep; 

	if (flag & 1)
	{
		ep=CheckInPolyPrecis(pos->x,pos->y-150.f,pos->z);

		if (!ep)
			ep=CheckInPolyPrecis(pos->x,pos->y-1.f,pos->z);
	}
	else
		ep=CheckInPoly(pos->x,pos->y,pos->z);

	if ((ep) && (ep->room>-1))	
	{
		if (height) *height=ep->center.y;

		return ep->room;
	}

	// Security... ?
	ep=GetMinPoly(pos->x,pos->y,pos->z);

	if ((ep) && (ep->room>-1))	
	{
		if (height) *height=ep->center.y;

		return ep->room;
	}
	else if (!(flag & 1))
	{
		ep=CheckInPolyPrecis(pos->x,pos->y,pos->z);

		if ((ep) && (ep->room>-1))	
		{
			if (height) *height=ep->center.y;

			return ep->room;
		}
	}

	if (flag & 2)
	{
		float off=20.f;
		ep=CheckInPolyPrecis(pos->x-off,pos->y-off,pos->z);

		if ((ep) && (ep->room>-1))	
		{
			if (height) *height=ep->center.y;

			return ep->room;
		}

		ep=CheckInPolyPrecis(pos->x-off,pos->y-20,pos->z-off);

		if ((ep) && (ep->room>-1))	
		{
			if (height) *height=ep->center.y;

			return ep->room;
		}

		ep=CheckInPolyPrecis(pos->x-off,pos->y-20,pos->z+off);

		if ((ep) && (ep->room>-1))	
		{
			if (height) *height=ep->center.y;

			return ep->room;
		}

		ep=CheckInPolyPrecis(pos->x+off,pos->y-20,pos->z);

		if ((ep) && (ep->room>-1))	
		{
			if (height) *height=ep->center.y;

			return ep->room;
		}

		ep=CheckInPolyPrecis(pos->x+off,pos->y-20,pos->z+off);

		if ((ep) && (ep->room>-1))	
		{
			if (height) *height=ep->center.y;

			return ep->room;
		}

		ep=CheckInPolyPrecis(pos->x+off,pos->y-20,pos->z-off);

		if ((ep) && (ep->room>-1))	
		{
			if (height) *height=ep->center.y;

			return ep->room;
		}

	}

	return -1;
}
long ARX_PORTALS_GetRoomNumForCamera(float * height)
{
	EERIEPOLY * ep; 
	ep=CheckInPolyPrecis(ACTIVECAM->pos.x,ACTIVECAM->pos.y,ACTIVECAM->pos.z);

	if ((ep) && (ep->room>-1))	
	{
		if (height) *height=ep->center.y;

		return ep->room;
	}

	ep=GetMinPoly(ACTIVECAM->pos.x,ACTIVECAM->pos.y,ACTIVECAM->pos.z);

	if ((ep) && (ep->room>-1))	
	{
		if (height) *height=ep->center.y;

		return ep->room;
	}

	float dist=0.f;

	while (dist<=20.f)
	{		
		float vvv=radians(ACTIVECAM->angle.b);
		ep=CheckInPolyPrecis(	ACTIVECAM->pos.x+EEsin(vvv)*dist,
								ACTIVECAM->pos.y,
								ACTIVECAM->pos.z-EEcos(vvv)*dist);

		if ((ep) && (ep->room>-1))	
		{
			if (height) *height=ep->center.y;

			return ep->room;
		}

		dist+=5.f;
	}

	return -1;
}
// flag==1 for player
long ARX_PORTALS_GetRoomNumForPosition(Vec3f * pos,long flag)
{
	long num;
	float height;

	if (flag & 1)
		num=ARX_PORTALS_GetRoomNumForCamera(&height);
	else
		num=ARX_PORTALS_GetRoomNumForPosition2(pos,flag,&height);

	if (num > -1)
	{
		long nearest=-1;
		float nearest_dist=99999.f;

		for (long n=0;n<portals->nb_rooms;n++)
		{
			for (long lll=0;lll<portals->room[n].nb_portals;lll++)
			{
				EERIE_PORTALS * po=	&portals->portals[portals->room[n].portals[lll]];
				EERIEPOLY *		epp=&po->poly;

				if (PointIn2DPolyXZ(epp, pos->x, pos->z)) 
				{
					float yy;

					if (GetTruePolyY(epp,pos,&yy))
					{
						if (height>yy)
						{
							if ((yy>=pos->y) && (yy-pos->y<nearest_dist))
							{
								if (epp->norm.y>0)
									nearest=po->room_2;
								else
									nearest=po->room_1;

								nearest_dist=yy-pos->y;						
							}
						}
					}
				}
			}
		}

		if (nearest>-1)
		num=nearest;
	}
	
	return num;
			}
			
void ARX_PORTALS_InitDrawnRooms()
{
	if (!portals) return;

	EERIE_PORTALS *ep = &portals->portals[0];

	for (long i=0;i<portals->nb_total;i++)
	{
		ep->useportal=0;
		ep++;
	}


	if ((RoomDraw==NULL) || (NbRoomDraw<portals->nb_rooms+1))
	{
		RoomDraw=(PORTAL_ROOM_DRAW *)realloc(RoomDraw,sizeof(PORTAL_ROOM_DRAW)*(portals->nb_rooms+1));

		if (RoomDraw)
		{
			NbRoomDraw=portals->nb_rooms+1;
		}
	}

	if (RoomDraw)
	{
		for (long i=0;i<NbRoomDraw;i++)
		{
			RoomDraw[i].count=0;		
			RoomDraw[i].flags=0;
			RoomDraw[i].frustrum.nb_frustrums=0;
		}
	}

	vPolyWater.clear();
	vPolyLava.clear();

	iTotPoly=0;

	if(pDynamicVertexBuffer) {
		pDynamicVertexBuffer->vb->setData(NULL, 0, 0, DiscardBuffer);
		dynamicVertices.reset();
	}
	
}
bool BBoxClipPoly(EERIE_2D_BBOX * bbox,EERIEPOLY * ep)
{
	EERIE_2D_BBOX n_bbox;
	long nbv;

	if (ep->type & POLY_QUAD)
		nbv=4;
	else
		nbv=3;

	n_bbox.max.x=n_bbox.min.x=ep->tv[0].p.x;
	n_bbox.max.y=n_bbox.min.y=ep->tv[0].p.y;	

	for (long i=1;i<nbv;i++)
	{
		n_bbox.min.x=min(n_bbox.min.x , ep->tv[i].p.x);
		n_bbox.min.y=min(n_bbox.min.y , ep->tv[i].p.y);
		n_bbox.max.x=max(n_bbox.max.x , ep->tv[i].p.x);
		n_bbox.max.y=max(n_bbox.max.y , ep->tv[i].p.y);
	}

	if (	bbox->min.x > n_bbox.max.x || n_bbox.min.x > bbox->max.x
		||	bbox->min.y > n_bbox.max.y || n_bbox.min.y > bbox->max.y)
		return true;

	return false;

}
bool IsInFrustrum(Vec3f * point,EERIE_FRUSTRUM * frustrum)
{
	if (	((point->x*frustrum->plane[0].a + point->y*frustrum->plane[0].b + point->z*frustrum->plane[0].c + frustrum->plane[0].d)>0)
		&&	((point->x*frustrum->plane[1].a + point->y*frustrum->plane[1].b + point->z*frustrum->plane[1].c + frustrum->plane[1].d)>0)
		&&	((point->x*frustrum->plane[2].a + point->y*frustrum->plane[2].b + point->z*frustrum->plane[2].c + frustrum->plane[2].d)>0)
		&&	((point->x*frustrum->plane[3].a + point->y*frustrum->plane[3].b + point->z*frustrum->plane[3].c + frustrum->plane[3].d)>0) )
		return true;

	return false;
}


bool IsSphereInFrustrum(float radius,Vec3f * point,EERIE_FRUSTRUM * frustrum)
{
	float dists[4];
	dists[0]=point->x*frustrum->plane[0].a + point->y*frustrum->plane[0].b + point->z*frustrum->plane[0].c + frustrum->plane[0].d;
	dists[1]=point->x*frustrum->plane[1].a + point->y*frustrum->plane[1].b + point->z*frustrum->plane[1].c + frustrum->plane[1].d;
	dists[2]=point->x*frustrum->plane[2].a + point->y*frustrum->plane[2].b + point->z*frustrum->plane[2].c + frustrum->plane[2].d;
	dists[3]=point->x*frustrum->plane[3].a + point->y*frustrum->plane[3].b + point->z*frustrum->plane[3].c + frustrum->plane[3].d;

	if (	(dists[0]+radius>0)
		&&	(dists[1]+radius>0)
		&&	(dists[2]+radius>0)
		&&	(dists[3]+radius>0) )
		return true;

	return false;
	
}

bool FrustrumsClipPoly(EERIE_FRUSTRUM_DATA * frustrums,EERIEPOLY * ep)
{
	for (long i=0;i<frustrums->nb_frustrums;i++)
	{
		if (IsSphereInFrustrum(ep->v[0].rhw, &ep->center, &frustrums->frustrums[i]))
			return false;
			}

	return true;
}
 
 
void ARX_PORTALS_BlendBBox(long room_num,EERIE_2D_BBOX * bbox)
{
	if (RoomDraw[room_num].count==0)
	{
		RoomDraw[room_num].bbox.min.x=bbox->min.x;
		RoomDraw[room_num].bbox.min.y=bbox->min.y;
		RoomDraw[room_num].bbox.max.x=bbox->max.x;
		RoomDraw[room_num].bbox.max.y=bbox->max.y;		
	}
	else
	{
		RoomDraw[room_num].bbox.min.x=min(RoomDraw[room_num].bbox.min.x, bbox->min.x);
		RoomDraw[room_num].bbox.min.y=min(RoomDraw[room_num].bbox.min.y, bbox->min.y);
		RoomDraw[room_num].bbox.max.x=max(RoomDraw[room_num].bbox.max.x, bbox->max.x);
		RoomDraw[room_num].bbox.max.y=max(RoomDraw[room_num].bbox.max.y, bbox->max.y);		
	}
}
void Frustrum_Set(EERIE_FRUSTRUM * fr,long plane,float a,float b,float c,float d)
{
	fr->plane[plane].a=a;
	fr->plane[plane].b=b;
	fr->plane[plane].c=c;
	fr->plane[plane].d=d;
}

void CreatePlane(EERIE_FRUSTRUM * frustrum,long numplane,Vec3f * orgn,Vec3f * pt1,Vec3f * pt2)
{
	register float Ax,Ay,Az,Bx,By,Bz,epnlen;
	Ax=pt1->x-orgn->x;
	Ay=pt1->y-orgn->y;
	Az=pt1->z-orgn->z;

	Bx=pt2->x-orgn->x;
	By=pt2->y-orgn->y;
	Bz=pt2->z-orgn->z;

	frustrum->plane[numplane].a=Ay*Bz-Az*By;
	frustrum->plane[numplane].b=Az*Bx-Ax*Bz;
	frustrum->plane[numplane].c=Ax*By-Ay*Bx;

	epnlen = (float)sqrt(	frustrum->plane[numplane].a * frustrum->plane[numplane].a
						+	frustrum->plane[numplane].b * frustrum->plane[numplane].b
						+	frustrum->plane[numplane].c * frustrum->plane[numplane].c	);
	epnlen=1.f/epnlen;
	frustrum->plane[numplane].a*=epnlen;
	frustrum->plane[numplane].b*=epnlen;
	frustrum->plane[numplane].c*=epnlen;
	frustrum->plane[numplane].d=-(	orgn->x * frustrum->plane[numplane].a +
									orgn->y * frustrum->plane[numplane].b +
									orgn->z * frustrum->plane[numplane].c		);

	
}
void CreateScreenFrustrum(EERIE_FRUSTRUM * frustrum);
void CreateFrustrum(EERIE_FRUSTRUM * frustrum, EERIEPOLY * ep, long cull) {
	
	long to = (ep->type & POLY_QUAD) ? 4 : 3;
	
	if(cull) {
		CreatePlane(frustrum, 0, &ACTIVECAM->pos, &ep->v[0].p, &ep->v[1].p);
		CreatePlane(frustrum, 1, &ACTIVECAM->pos, &ep->v[3].p, &ep->v[2].p);
		CreatePlane(frustrum, 2, &ACTIVECAM->pos, &ep->v[1].p, &ep->v[3].p);
		CreatePlane(frustrum, 3, &ACTIVECAM->pos, &ep->v[2].p, &ep->v[0].p);
	} else {
		CreatePlane(frustrum, 0, &ACTIVECAM->pos, &ep->v[1].p, &ep->v[0].p);
		CreatePlane(frustrum, 1, &ACTIVECAM->pos, &ep->v[2].p, &ep->v[3].p);
		CreatePlane(frustrum, 2, &ACTIVECAM->pos, &ep->v[3].p, &ep->v[1].p);
		CreatePlane(frustrum, 3, &ACTIVECAM->pos, &ep->v[0].p, &ep->v[2].p);
	}
	
	frustrum->nb = to;
}

void CreateScreenFrustrum(EERIE_FRUSTRUM * frustrum) {
	
	Vec3f vEyePt(ACTIVECAM->pos.x, -ACTIVECAM->pos.y, ACTIVECAM->pos.z);
	Vec3f vTout(0.0f, 0.0f, 10000.0f);
	
	Vec3f vTarget;
	vTarget.y = -(vTout.z * ACTIVECAM->Xsin);
	vTarget.z = -(vTout.z * ACTIVECAM->Xcos);
	vTarget.x =  (vTarget.z * ACTIVECAM->Ysin);
	vTarget.z = -(vTarget.z * ACTIVECAM->Ycos);
	vTarget.x += ACTIVECAM->pos.x;
	vTarget.y -= ACTIVECAM->pos.y;
	vTarget.z += ACTIVECAM->pos.z;
	
	Vec3f vUpVec(0.f, 1.f, 0.f);
	
	// Set the app view matrix for normal viewing
	GRenderer->SetViewMatrix(vEyePt, vTarget, vUpVec);
	
	EERIEMATRIX matProj;
	GRenderer->GetProjectionMatrix(matProj);
	
	EERIEMATRIX matView;
	GRenderer->GetViewMatrix(matView);
	
	EERIEMATRIX matres;
	MatrixMultiply(&matres, &matView, &matProj);

	float a,b,c,d,n;
	a=matres._14-matres._11;
	b=matres._24-matres._21;
	c=matres._34-matres._31;
	d=matres._44-matres._41;
 b=-b;
	n = (float)(1.f /sqrt(a*a+b*b+c*c));

	Frustrum_Set(frustrum,0,a*n,b*n,c*n,d*n);
	a=matres._14+matres._11;
	b=matres._24+matres._21;
	c=matres._34+matres._31;
	d=matres._44+matres._41;
 b=-b;
	n = (float)(1.f/sqrt(a*a+b*b+c*c));

	Frustrum_Set(frustrum,1,a*n,b*n,c*n,d*n);
	a=matres._14-matres._12;
	b=matres._24-matres._22;
	c=matres._34-matres._32;
	d=matres._44-matres._42;
 b=-b;
	n = (float)(1.f/sqrt(a*a+b*b+c*c));

	Frustrum_Set(frustrum,2,a*n,b*n,c*n,d*n);
	a=matres._14+matres._12;
	b=matres._24+matres._22;
	c=matres._34+matres._32;
	d=matres._44+matres._42;
 b=-b;
	n = (float)(1.f/sqrt(a*a+b*b+c*c));

	Frustrum_Set(frustrum,3,a*n,b*n,c*n,d*n);
 
	frustrum->nb=4;

	//Ajout du plan Near & Far
	a=matres._14-matres._13;
	b=matres._24-matres._23;
	c=matres._34-matres._33;
	d=matres._44-matres._43;
	b=-b;
 	n = (float)(1.f/sqrt(a*a+b*b+c*c));
	efpPlaneFar.a=a*n;
	efpPlaneFar.b=b*n;
	efpPlaneFar.c=c*n;
	efpPlaneFar.d=d*n;
	
	a=matres._14+matres._13;
	b=matres._24+matres._23;
	c=matres._34+matres._33;
	d=matres._44+matres._43;
 b=-b;
	n = (float)(1.f/sqrt(a*a+b*b+c*c));
	efpPlaneNear.a=a*n;
	efpPlaneNear.b=b*n;
	efpPlaneNear.c=c*n;
	efpPlaneNear.d=d*n;
}

void RoomDrawRelease()
{
	if (RoomDrawList)
		free(RoomDrawList);

	RoomDrawList=NULL;

	if (RoomDraw)
		free(RoomDraw);

	RoomDraw=NULL;
}
void RoomDrawListAdd(long num)
{
	if (TotalRoomDrawList<=NbRoomDrawList)
	{
		RoomDrawList=(long *)realloc(RoomDrawList,sizeof(long)*(NbRoomDrawList+1));
		TotalRoomDrawList=NbRoomDrawList+1;
	}

	RoomDrawList[NbRoomDrawList]=num;	
	NbRoomDrawList++;
}
void RoomFrustrumAdd(long num,EERIE_FRUSTRUM * fr)
{
	if (RoomDraw[num].frustrum.nb_frustrums<MAX_FRUSTRUMS-1)
	{
		memcpy(&RoomDraw[num].frustrum.frustrums
			[RoomDraw[num].frustrum.nb_frustrums],fr,sizeof(EERIE_FRUSTRUM));		
		RoomDraw[num].frustrum.nb_frustrums++;
		
	}	
}
void ARX_PORTALS_RenderRoom(long room_num,EERIE_2D_BBOX * bbox,long prec,long tim);
void ARX_PORTALS_RenderRooms(long prec,long tim)
{
	for (long i=0;i<NbRoomDrawList;i++)
	{
		ARX_PORTALS_RenderRoom(RoomDrawList[i],&RoomDraw[RoomDrawList[i]].bbox,prec,tim);
	}

	NbRoomDrawList=0;
}
void ARX_PORTALS_Frustrum_RenderRoom(long room_num,EERIE_FRUSTRUM_DATA * frustrums,long prec,long tim);
void ARX_PORTALS_Frustrum_RenderRooms(long prec,long tim)
{
	for (long i=0;i<NbRoomDrawList;i++)
	{
		ARX_PORTALS_Frustrum_RenderRoom(RoomDrawList[i],&RoomDraw[RoomDrawList[i]].frustrum,prec,tim);
	}

	NbRoomDrawList=0;
}

static void RenderWaterBatch() {
	
	if(!dynamicVertices.nbindices) {
		return;
	}
	
	GRenderer->GetTextureStage(1)->SetColorOp(TextureStage::OpModulate4X, TextureStage::ArgTexture, TextureStage::ArgCurrent);
	GRenderer->GetTextureStage(1)->DisableAlpha();
	
	GRenderer->GetTextureStage(2)->SetColorOp(TextureStage::OpModulate, TextureStage::ArgTexture, TextureStage::ArgCurrent);
	GRenderer->GetTextureStage(2)->DisableAlpha();
	
	dynamicVertices.draw(Renderer::TriangleList);
	
	GRenderer->GetTextureStage(1)->DisableColor();
	GRenderer->GetTextureStage(2)->DisableColor();
	
}

static void RenderWater() {
	
	if(vPolyWater.empty()) {
		return;
	}
	
	size_t iNbIndice = 0;
	int iNb = vPolyWater.size();
	
	dynamicVertices.lock();
	
	GRenderer->SetBlendFunc(Renderer::BlendDstColor, Renderer::BlendOne);
	GRenderer->SetTexture(0, enviro);
	GRenderer->SetTexture(2, enviro);
	
	unsigned short * indices = dynamicVertices.indices;
	
	while(iNb--) {
		EERIEPOLY * ep = vPolyWater[iNb];
		
		unsigned short iNbVertex = (ep->type & POLY_QUAD) ? 4 : 3;
		SMY_VERTEX3 * pVertex = dynamicVertices.append(iNbVertex);
		
		if(!pVertex) {
			dynamicVertices.unlock();
			RenderWaterBatch();
			dynamicVertices.reset();
			dynamicVertices.lock();
			iNbIndice = 0;
			indices = dynamicVertices.indices;
			pVertex = dynamicVertices.append(iNbVertex);
		}
		
		pVertex->p.x = ep->v[0].p.x;
		pVertex->p.y = -ep->v[0].p.y;
		pVertex->p.z = ep->v[0].p.z;
		pVertex->color = 0xFF505050;
		float fTu = ep->v[0].p.x*(1.f/1000) + sin(ep->v[0].p.x*(1.f/200) + arxtime.get_frame_time()*(1.f/1000)) * (1.f/32);
		float fTv = ep->v[0].p.z*(1.f/1000) + cos(ep->v[0].p.z*(1.f/200) + arxtime.get_frame_time()*(1.f/1000)) * (1.f/32);
		if(ep->type & POLY_FALL) {
			fTv += arxtime.get_frame_time() * (1.f/4000);
		}
		pVertex->uv[0].x = fTu;
		pVertex->uv[0].y = fTv;
		fTu = (ep->v[0].p.x + 30.f)*(1.f/1000) + sin((ep->v[0].p.x + 30)*(1.f/200) + arxtime.get_frame_time()*(1.f/1000))*(1.f/28);
		fTv = (ep->v[0].p.z + 30.f)*(1.f/1000) - cos((ep->v[0].p.z + 30)*(1.f/200) + arxtime.get_frame_time()*(1.f/1000))*(1.f/28);
		if (ep->type & POLY_FALL) {
			fTv += arxtime.get_frame_time() * (1.f/4000);
		}
		pVertex->uv[1].x=fTu;
		pVertex->uv[1].y=fTv;
		fTu=(ep->v[0].p.x+60.f)*( 1.0f / 1000 )-EEsin((ep->v[0].p.x+60)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 1000 ))*( 1.0f / 40 );
		fTv=(ep->v[0].p.z+60.f)*( 1.0f / 1000 )-EEcos((ep->v[0].p.z+60)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 1000 ))*( 1.0f / 40 );
		
		if (ep->type & POLY_FALL) fTv+=arxtime.get_frame_time()*( 1.0f / 4000 );
		
		pVertex->uv[2].x=fTu;
		pVertex->uv[2].y=fTv;
		
		pVertex++;
		pVertex->p.x=ep->v[1].p.x;
		pVertex->p.y=-ep->v[1].p.y;
		pVertex->p.z=ep->v[1].p.z;
		pVertex->color=0xFF505050;
		fTu=ep->v[1].p.x*( 1.0f / 1000 )+EEsin((ep->v[1].p.x)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 1000 ))*( 1.0f / 32 );
		fTv=ep->v[1].p.z*( 1.0f / 1000 )+EEcos((ep->v[1].p.z)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 1000 ))*( 1.0f / 32 );
		
		if(ep->type&POLY_FALL) fTv+=arxtime.get_frame_time()*( 1.0f / 4000 );
		
		pVertex->uv[0].x=fTu;
		pVertex->uv[0].y=fTv;
		fTu=(ep->v[1].p.x+30.f)*( 1.0f / 1000 )+EEsin((ep->v[1].p.x+30)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 1000 ))*( 1.0f / 28 );
		fTv=(ep->v[1].p.z+30.f)*( 1.0f / 1000 )-EEcos((ep->v[1].p.z+30)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 1000 ))*( 1.0f / 28 );
		
		if (ep->type & POLY_FALL) fTv+=arxtime.get_frame_time()*( 1.0f / 4000 );
		
		pVertex->uv[1].x=fTu;
		pVertex->uv[1].y=fTv;
		fTu=(ep->v[1].p.x+60.f)*( 1.0f / 1000 )-EEsin((ep->v[1].p.x+60)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 1000 ))*( 1.0f / 40 );
		fTv=(ep->v[1].p.z+60.f)*( 1.0f / 1000 )-EEcos((ep->v[1].p.z+60)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 1000 ))*( 1.0f / 40 );
		
		if (ep->type & POLY_FALL) fTv+=arxtime.get_frame_time()*( 1.0f / 4000 );
		
		pVertex->uv[2].x=fTu;
		pVertex->uv[2].y=fTv;
		pVertex++;
		pVertex->p.x=ep->v[2].p.x;
		pVertex->p.y=-ep->v[2].p.y;
		pVertex->p.z=ep->v[2].p.z;
		pVertex->color=0xFF505050;
		fTu=ep->v[2].p.x*( 1.0f / 1000 )+EEsin((ep->v[2].p.x)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 1000 ))*( 1.0f / 32 );
		fTv=ep->v[2].p.z*( 1.0f / 1000 )+EEcos((ep->v[2].p.z)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 1000 ))*( 1.0f / 32 );
		
		if(ep->type&POLY_FALL) fTv+=arxtime.get_frame_time()*( 1.0f / 4000 );
		
		pVertex->uv[0].x=fTu;
		pVertex->uv[0].y=fTv;
		fTu=(ep->v[2].p.x+30.f)*( 1.0f / 1000 )+EEsin((ep->v[2].p.x+30)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 1000 ))*( 1.0f / 28 );
		fTv=(ep->v[2].p.z+30.f)*( 1.0f / 1000 )-EEcos((ep->v[2].p.z+30)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 1000 ))*( 1.0f / 28 );
		
		if (ep->type & POLY_FALL) fTv+=arxtime.get_frame_time()*( 1.0f / 4000 );
		
		pVertex->uv[1].x=fTu;
		pVertex->uv[1].y=fTv;
		fTu=(ep->v[2].p.x+60.f)*( 1.0f / 1000 )-EEsin((ep->v[2].p.x+60)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 1000 ))*( 1.0f / 40 );
		fTv=(ep->v[2].p.z+60.f)*( 1.0f / 1000 )-EEcos((ep->v[2].p.z+60)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 1000 ))*( 1.0f / 40 );
		
		if (ep->type & POLY_FALL) fTv+=arxtime.get_frame_time()*( 1.0f / 4000 );
		
		pVertex->uv[2].x=fTu;
		pVertex->uv[2].y=fTv;
		pVertex++;
		
		*indices++ = iNbIndice++; 
		*indices++ = iNbIndice++; 
		*indices++ = iNbIndice++; 
		dynamicVertices.nbindices += 3;
		
		if(iNbVertex == 4)
		{
			pVertex->p.x=ep->v[3].p.x;
			pVertex->p.y=-ep->v[3].p.y;
			pVertex->p.z=ep->v[3].p.z;
			pVertex->color=0xFF505050;
			fTu=ep->v[3].p.x*( 1.0f / 1000 )+EEsin((ep->v[3].p.x)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 1000 ))*( 1.0f / 32 );
			fTv=ep->v[3].p.z*( 1.0f / 1000 )+EEcos((ep->v[3].p.z)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 1000 ))*( 1.0f / 32 );
			
			if(ep->type&POLY_FALL) fTv+=arxtime.get_frame_time()*( 1.0f / 4000 );
			
			pVertex->uv[0].x=fTu;
			pVertex->uv[0].y=fTv;
			fTu=(ep->v[3].p.x+30.f)*( 1.0f / 1000 )+EEsin((ep->v[3].p.x+30)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 1000 ))*( 1.0f / 28 );
			fTv=(ep->v[3].p.z+30.f)*( 1.0f / 1000 )-EEcos((ep->v[3].p.z+30)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 1000 ))*( 1.0f / 28 );
			
			if (ep->type & POLY_FALL) fTv+=arxtime.get_frame_time()*( 1.0f / 4000 );
			
			pVertex->uv[1].x=fTu;
			pVertex->uv[1].y=fTv;
			fTu=(ep->v[3].p.x+60.f)*( 1.0f / 1000 )-EEsin((ep->v[3].p.x+60)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 1000 ))*( 1.0f / 40 );
			fTv=(ep->v[3].p.z+60.f)*( 1.0f / 1000 )-EEcos((ep->v[3].p.z+60)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 1000 ))*( 1.0f / 40 );
			
			if (ep->type & POLY_FALL) fTv+=arxtime.get_frame_time()*( 1.0f / 4000 );
			
			pVertex->uv[2].x=fTu;
			pVertex->uv[2].y=fTv;
			pVertex++;
			
			*indices++ = iNbIndice++; 
			*indices++ = iNbIndice - 2; 
			*indices++ = iNbIndice - 3; 
			dynamicVertices.nbindices += 3;
		}
		
	}
	
	dynamicVertices.unlock();
	RenderWaterBatch();
	dynamicVertices.done();
	
	vPolyWater.clear();
	
}

void RenderLavaBatch() {
	
	GRenderer->SetBlendFunc(Renderer::BlendDstColor, Renderer::BlendOne);
	GRenderer->GetTextureStage(0)->SetColorOp(TextureStage::OpModulate2X, TextureStage::ArgTexture, TextureStage::ArgDiffuse);
	
	if(!dynamicVertices.nbindices) {
		return;
	}
	
	GRenderer->GetTextureStage(1)->SetColorOp(TextureStage::OpModulate4X, TextureStage::ArgTexture, TextureStage::ArgCurrent);
	GRenderer->GetTextureStage(1)->DisableAlpha();
	
	GRenderer->GetTextureStage(2)->SetColorOp(TextureStage::OpModulate, TextureStage::ArgTexture, TextureStage::ArgCurrent);
	GRenderer->GetTextureStage(2)->DisableAlpha();
	
	dynamicVertices.draw(Renderer::TriangleList);
	
	GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
	GRenderer->GetTextureStage(0)->SetColorOp(TextureStage::OpModulate);
	
	dynamicVertices.draw(Renderer::TriangleList);
	
	GRenderer->GetTextureStage(1)->DisableColor();
	GRenderer->GetTextureStage(2)->DisableColor();
	
}

void RenderLava() {
	
	if(vPolyLava.empty()) {
		return;
	}
	
	size_t iNbIndice = 0;
	int iNb=vPolyLava.size();
	
	dynamicVertices.lock();
	
	GRenderer->SetBlendFunc(Renderer::BlendDstColor, Renderer::BlendOne);
	GRenderer->SetTexture(0, enviro);
	GRenderer->SetTexture(2, enviro);
	
	unsigned short * indices = dynamicVertices.indices;
	
	while(iNb--) {
		EERIEPOLY * ep = vPolyLava[iNb];
		
		unsigned short iNbVertex = (ep->type & POLY_QUAD) ? 4 : 3;
		SMY_VERTEX3 * pVertex = dynamicVertices.append(iNbVertex);
		
		if(!pVertex) {
			dynamicVertices.unlock();
			RenderLavaBatch();
			dynamicVertices.reset();
			dynamicVertices.lock();
			iNbIndice = 0;
			indices = dynamicVertices.indices;
			pVertex = dynamicVertices.append(iNbVertex);
		}
		
		pVertex->p.x=ep->v[0].p.x;
		pVertex->p.y=-ep->v[0].p.y;
		pVertex->p.z=ep->v[0].p.z;
		pVertex->color=0xFF666666;
		float fTu=ep->v[0].p.x*( 1.0f / 1000 )+EEsin((ep->v[0].p.x)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 2000 ))*( 1.0f / 20 );
		float fTv=ep->v[0].p.z*( 1.0f / 1000 )+EEcos((ep->v[0].p.z)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 2000 ))*( 1.0f / 20 );
		pVertex->uv[0].x=fTu;
		pVertex->uv[0].y=fTv;
		fTu=ep->v[0].p.x*( 1.0f / 1000 )+EEsin((ep->v[0].p.x)*( 1.0f / 100 )+arxtime.get_frame_time()*( 1.0f / 2000 ))*( 1.0f / 10 );
		fTv=ep->v[0].p.z*( 1.0f / 1000 )+EEcos((ep->v[0].p.z)*( 1.0f / 100 )+arxtime.get_frame_time()*( 1.0f / 2000 ))*( 1.0f / 10 );
		pVertex->uv[1].x=fTu;
		pVertex->uv[1].y=fTv;
		fTu=ep->v[0].p.x*( 1.0f / 600 )+EEsin((ep->v[0].p.x)*( 1.0f / 160 )+arxtime.get_frame_time()*( 1.0f / 2000 ))*( 1.0f / 11 );
		fTv=ep->v[0].p.z*( 1.0f / 600 )+EEcos((ep->v[0].p.z)*( 1.0f / 160 )+arxtime.get_frame_time()*( 1.0f / 2000 ))*( 1.0f / 11 );
		
		pVertex->uv[2].x=fTu;
		pVertex->uv[2].y=fTv;
		pVertex++;
		pVertex->p.x=ep->v[1].p.x;
		pVertex->p.y=-ep->v[1].p.y;
		pVertex->p.z=ep->v[1].p.z;
		pVertex->color=0xFF666666;
		fTu=ep->v[1].p.x*( 1.0f / 1000 )+EEsin((ep->v[1].p.x)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 2000 ))*( 1.0f / 20 );
		fTv=ep->v[1].p.z*( 1.0f / 1000 )+EEcos((ep->v[1].p.z)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 2000 ))*( 1.0f / 20 );
		pVertex->uv[0].x=fTu;
		pVertex->uv[0].y=fTv;
		fTu=ep->v[1].p.x*( 1.0f / 1000 )+EEsin((ep->v[1].p.x)*( 1.0f / 100 )+arxtime.get_frame_time()*( 1.0f / 2000 ))*( 1.0f / 10 );
		fTv=ep->v[1].p.z*( 1.0f / 1000 )+EEcos((ep->v[1].p.z)*( 1.0f / 100 )+arxtime.get_frame_time()*( 1.0f / 2000 ))*( 1.0f / 10 );
		pVertex->uv[1].x=fTu;
		pVertex->uv[1].y=fTv;
		fTu=ep->v[1].p.x*( 1.0f / 600 )+EEsin((ep->v[1].p.x)*( 1.0f / 160 )+arxtime.get_frame_time()*( 1.0f / 2000 ))*( 1.0f / 11 );
		fTv=ep->v[1].p.z*( 1.0f / 600 )+EEcos((ep->v[1].p.z)*( 1.0f / 160 )+arxtime.get_frame_time()*( 1.0f / 2000 ))*( 1.0f / 11 );
		
		pVertex->uv[2].x=fTu;
		pVertex->uv[2].y=fTv;
		pVertex++;
		pVertex->p.x=ep->v[2].p.x;
		pVertex->p.y=-ep->v[2].p.y;
		pVertex->p.z=ep->v[2].p.z;
		pVertex->color=0xFF666666;
		fTu=ep->v[2].p.x*( 1.0f / 1000 )+EEsin((ep->v[2].p.x)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 2000 ))*( 1.0f / 20 );
		fTv=ep->v[2].p.z*( 1.0f / 1000 )+EEcos((ep->v[2].p.z)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 2000 ))*( 1.0f / 20 );
		pVertex->uv[0].x=fTu;
		pVertex->uv[0].y=fTv;
		fTu=ep->v[2].p.x*( 1.0f / 1000 )+EEsin((ep->v[2].p.x)*( 1.0f / 100 )+arxtime.get_frame_time()*( 1.0f / 2000 ))*( 1.0f / 10 );
		fTv=ep->v[2].p.z*( 1.0f / 1000 )+EEcos((ep->v[2].p.z)*( 1.0f / 100 )+arxtime.get_frame_time()*( 1.0f / 2000 ))*( 1.0f / 10 );
		pVertex->uv[1].x=fTu;
		pVertex->uv[1].y=fTv;
		fTu=ep->v[2].p.x*( 1.0f / 600 )+EEsin((ep->v[2].p.x)*( 1.0f / 160 )+arxtime.get_frame_time()*( 1.0f / 2000 ))*( 1.0f / 11 );
		fTv=ep->v[2].p.z*( 1.0f / 600 )+EEcos((ep->v[2].p.z)*( 1.0f / 160 )+arxtime.get_frame_time()*( 1.0f / 2000 ))*( 1.0f / 11 );
		
		pVertex->uv[2].x=fTu;
		pVertex->uv[2].y=fTv;
		pVertex++;
		
		*indices++ = iNbIndice++; 
		*indices++ = iNbIndice++; 
		*indices++ = iNbIndice++; 
		dynamicVertices.nbindices += 3;
		
		if(iNbVertex&4)
		{
			pVertex->p.x=ep->v[3].p.x;
			pVertex->p.y=-ep->v[3].p.y;
			pVertex->p.z=ep->v[3].p.z;
			pVertex->color=0xFF666666;
			fTu=ep->v[3].p.x*( 1.0f / 1000 )+EEsin((ep->v[3].p.x)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 2000 ))*( 1.0f / 20 );
			fTv=ep->v[3].p.z*( 1.0f / 1000 )+EEcos((ep->v[3].p.z)*( 1.0f / 200 )+arxtime.get_frame_time()*( 1.0f / 2000 ))*( 1.0f / 20 );
			pVertex->uv[0].x=fTu;
			pVertex->uv[0].y=fTv;
			fTu=ep->v[3].p.x*( 1.0f / 1000 )+EEsin((ep->v[3].p.x)*( 1.0f / 100 )+arxtime.get_frame_time()*( 1.0f / 2000 ))*( 1.0f / 10 );
			fTv=ep->v[3].p.z*( 1.0f / 1000 )+EEcos((ep->v[3].p.z)*( 1.0f / 100 )+arxtime.get_frame_time()*( 1.0f / 2000 ))*( 1.0f / 10 );
			pVertex->uv[1].x=fTu;
			pVertex->uv[1].y=fTv;
			fTu=ep->v[3].p.x*( 1.0f / 600 )+EEsin((ep->v[3].p.x)*( 1.0f / 160 )+arxtime.get_frame_time()*( 1.0f / 2000 ))*( 1.0f / 11 );
			fTv=ep->v[3].p.z*( 1.0f / 600 )+EEcos((ep->v[3].p.z)*( 1.0f / 160 )+arxtime.get_frame_time()*( 1.0f / 2000 ))*( 1.0f / 11 );
			
			pVertex->uv[2].x=fTu;
			pVertex->uv[2].y=fTv;
			pVertex++;
			
			*indices++ = iNbIndice++; 
			*indices++ = iNbIndice - 2; 
			*indices++ = iNbIndice - 3; 
			dynamicVertices.nbindices += 3;
		}
		
	}
	
	dynamicVertices.unlock();
	RenderLavaBatch();
	dynamicVertices.done();
	
	vPolyLava.clear();
	
}

void ARX_PORTALS_Frustrum_RenderRoom_TransparencyTSoftCull(long room_num);
void ARX_PORTALS_Frustrum_RenderRooms_TransparencyT() {
	
	GRenderer->SetFogColor(Color::none);

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);

	GRenderer->SetAlphaFunc(Renderer::CmpGreater, .5f);
	
	for (long i=0;i<NbRoomDrawList;i++)
	{
		if(USE_PORTALS==4)
		{
			ARX_PORTALS_Frustrum_RenderRoom_TransparencyTSoftCull(RoomDrawList[i]);
		}
		else
		{
			LogWarning << "unimplemented";
		}
	}
	
	GRenderer->SetAlphaFunc(Renderer::CmpNotEqual, 0.f);

	NbRoomDrawList=0;


	SetZBias(8);

	GRenderer->SetRenderState(Renderer::DepthWrite, false);

	//render all fx!!
	GRenderer->SetCulling(Renderer::CullCW);
	
	RenderWater();
	RenderLava();
	
	SetZBias(0);
	GRenderer->SetFogColor(ulBKGColor);
	GRenderer->GetTextureStage(0)->SetColorOp(TextureStage::OpModulate);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}

void ARX_PORTALS_Frustrum_RenderRoomTCullSoft(long room_num,EERIE_FRUSTRUM_DATA * frustrums,long prec,long tim);
void ARX_PORTALS_Frustrum_RenderRoomsTCullSoft(long prec,long tim)
{
	GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);	

	for (long i=0;i<NbRoomDrawList;i++)
	{
		ARX_PORTALS_Frustrum_RenderRoomTCullSoft(RoomDrawList[i],&RoomDraw[RoomDrawList[i]].frustrum,prec,tim);
	}
}
void ARX_PORTALS_RenderRoom(long room_num,EERIE_2D_BBOX * bbox,long prec,long tim)
{
	
	if (RoomDraw[room_num].count)
	{
		EERIEDraw2DRect(bbox->min.x, bbox->min.y ,bbox->max.x, bbox->max.y, 0.0001f, Color::blue);

	for (long  lll=0;lll<portals->room[room_num].nb_polys;lll++)
	{
			
		FAST_BKG_DATA * feg;
		feg=&ACTIVEBKG->fastdata[portals->room[room_num].epdata[lll].px][portals->room[room_num].epdata[lll].py];

		if (!feg->treat)
			continue;

		EERIEPOLY * ep=&feg->polydata[portals->room[room_num].epdata[lll].idx];

		if (ep->type & (POLY_IGNORE | POLY_NODRAW))		
			continue;
			
			// GO for 3D Backface Culling
			if (ep->type & POLY_DOUBLESIDED)
				GRenderer->SetCulling(Renderer::CullNone);
			else
			{
				Vec3f nrm;
				nrm.x=ep->v[2].p.x-ACTIVECAM->pos.x;
				nrm.y=ep->v[2].p.y-ACTIVECAM->pos.y;
				nrm.z=ep->v[2].p.z-ACTIVECAM->pos.z;

				if ( ep->type & POLY_QUAD) 
				{
					if ((dot(ep->norm , nrm) > 0.f) &&
						 (dot(ep->norm2 , nrm) > 0.f) )	
						continue;
				}
				else if (dot(ep->norm , nrm) > 0.f)
						continue;

				GRenderer->SetCulling(Renderer::CullCW);
			}
			 
			if (!EERIERTPPoly(ep)) // RotTransProject Vertices
				continue; 

			if (BBoxClipPoly(bbox,ep))
				continue;

			long to;

			if ( ep->type & POLY_QUAD) 
			{
				if (FRAME_COUNT<=0) 
					ep->tv[3].color=ep->v[3].color;	

				to=4;
			}
			else to=3;

			if (ep->type & POLY_TRANS) 
			{
				ManageLavaWater(ep,to,tim);
				TransPol[TRANSPOLYSPOS++]=ep;

				if (TRANSPOLYSPOS>=MAX_TRANSPOL) TRANSPOLYSPOS=MAX_TRANSPOL-1;

				if (ViewMode)
				{
					if (ViewMode & VIEWMODE_WIRE) 
						EERIEPOLY_DrawWired(ep);
					
					if (ViewMode & VIEWMODE_NORMALS) 
						EERIEPOLY_DrawNormals(ep);
				}	

				continue;
			}

			if (!Project.improve)  // Normal View...			
			{
				if (ep->type & POLY_GLOW) 
					ep->tv[0].color=ep->tv[1].color=ep->tv[2].color=ep->tv[3].color=0xFFFFFFFF;
				else 
				{
					if (FRAME_COUNT<=0)
					{
						if (ModeLight & MODE_DYNAMICLIGHT) 	ApplyDynLight(ep);					
						else
						{
							ep->tv[0].color=ep->v[0].color;	
							ep->tv[1].color=ep->v[1].color;	
							ep->tv[2].color=ep->v[2].color;		
						}						
					}
				}

				ManageLavaWater(ep,to,tim);
				
				Delayed_EERIEDRAWPRIM(ep);

				if (ViewMode)
				{
					if (ViewMode & VIEWMODE_WIRE) 
						EERIEPOLY_DrawWired(ep);
					
					if (ViewMode & VIEWMODE_NORMALS) 
						EERIEPOLY_DrawNormals(ep);
				}	
			}
			else // Improve Vision Activated
			{			
				if (FRAME_COUNT<=0)
				{
					if ( ModeLight & MODE_DYNAMICLIGHT ) 	ApplyDynLight(ep);										
					else 
					{	
						ep->tv[0].color=ep->v[0].color;	
						ep->tv[1].color=ep->v[1].color;	
						ep->tv[2].color=ep->v[2].color;				
					}
				

					for (long k=0;k<to;k++) 
					{
						long lfr,lfb;
						float fr,fb;
						long lr=(ep->tv[k].color>>16) & 255;
						float ffr=(float)(lr);
							
						float dd=(ep->tv[k].p.z*prec);

						if (dd>1.f) dd=1.f;

						if (dd<0.f) dd=0.f;
						
						fb=((1.f-dd)*6.f + (EEfabs(ep->nrml[k].x)+EEfabs(ep->nrml[k].y)))*0.125f;
						fr=((0.6f-dd)*6.f + (EEfabs(ep->nrml[k].z)+EEfabs(ep->nrml[k].y)))*0.125f;//(1.f-dd);						

						if (fr<0.f) fr=0.f;
						else fr=max(ffr,fr*255.f);

						fb*=255.f;
						lfr = fr;
						lfb = fb;
						ep->tv[k].color=( 0xff001E00L | ( (lfr & 255) << 16) | (lfb & 255) );
						//GG component locked at 0x1E
				}
			}

			Delayed_EERIEDRAWPRIM(ep);
		}			
	}			
	}
}
void ARX_PORTALS_Frustrum_RenderRoom(long room_num,EERIE_FRUSTRUM_DATA * frustrums,long prec,long tim)
{
	
	if (RoomDraw[room_num].count)
	{
		
	for (long  lll=0;lll<portals->room[room_num].nb_polys;lll++)
	{
		
		FAST_BKG_DATA * feg;
		feg=&ACTIVEBKG->fastdata[portals->room[room_num].epdata[lll].px][portals->room[room_num].epdata[lll].py];

		if (!feg->treat)
			continue;

		EERIEPOLY * ep=&feg->polydata[portals->room[room_num].epdata[lll].idx];

		if (ep->type & (POLY_IGNORE | POLY_NODRAW))		
			continue;
			
		if (FrustrumsClipPoly(frustrums,ep))
				continue;

			// GO for 3D Backface Culling
			if (ep->type & POLY_DOUBLESIDED)
				GRenderer->SetCulling(Renderer::CullNone);
			else
			{
				Vec3f nrm;
				nrm.x=ep->v[2].p.x-ACTIVECAM->pos.x;
				nrm.y=ep->v[2].p.y-ACTIVECAM->pos.y;
				nrm.z=ep->v[2].p.z-ACTIVECAM->pos.z;

				if ( ep->type & POLY_QUAD) 
				{
					if ( (dot( ep->norm , nrm )>0.f) &&
						 (dot( ep->norm2 , nrm )>0.f) )	
						continue;
				}
				else if ( dot( ep->norm , nrm )>0.f)
						continue;

				GRenderer->SetCulling(Renderer::CullCW);
			}
			 
			if (!EERIERTPPoly(ep)) // RotTransProject Vertices
				continue; 
			
			long to;

			if ( ep->type & POLY_QUAD) 
			{
				if (FRAME_COUNT<=0) ep->tv[3].color=ep->v[3].color;	

				to=4;
			}
			else to=3;

			if (ep->type & POLY_TRANS) 
			{
				ManageLavaWater(ep,to,tim);
				TransPol[TRANSPOLYSPOS++]=ep;

				if (TRANSPOLYSPOS>=MAX_TRANSPOL) TRANSPOLYSPOS=MAX_TRANSPOL-1;

				if (ViewMode)
				{
					if (ViewMode & VIEWMODE_WIRE) 
						EERIEPOLY_DrawWired(ep);
					
					if (ViewMode & VIEWMODE_NORMALS) 
						EERIEPOLY_DrawNormals(ep);
				}	

				continue;
			}

			if (!Project.improve)  // Normal View...			
			{
				if (ep->type & POLY_GLOW) ep->tv[0].color=ep->tv[1].color=ep->tv[2].color=ep->tv[3].color=0xFFFFFFFF;
				else 
				{
					if (FRAME_COUNT<=0)
					{
						if (ModeLight & MODE_DYNAMICLIGHT) 	ApplyDynLight(ep);					
						else
						{
							ep->tv[0].color=ep->v[0].color;	
							ep->tv[1].color=ep->v[1].color;	
							ep->tv[2].color=ep->v[2].color;		
						}						
					}
				}

				ManageLavaWater(ep,to,tim);
				
				Delayed_EERIEDRAWPRIM(ep);

				if (ViewMode)
				{
					if (ViewMode & VIEWMODE_WIRE) 
						EERIEPOLY_DrawWired(ep);
					
					if (ViewMode & VIEWMODE_NORMALS) 
						EERIEPOLY_DrawNormals(ep);
				}	
			}
			else // Improve Vision Activated
			{			
				if (FRAME_COUNT<=0)
				{
					if ( ModeLight & MODE_DYNAMICLIGHT ) 	ApplyDynLight(ep);										
					else 
					{	
						ep->tv[0].color=ep->v[0].color;	
						ep->tv[1].color=ep->v[1].color;	
						ep->tv[2].color=ep->v[2].color;				
					}
				

					for (long k=0;k<to;k++) 
					{
						long lfr,lfb;
						float fr,fb;
						long lr=(ep->tv[k].color>>16) & 255;
						float ffr=(float)(lr);
							
						float dd=(ep->tv[k].p.z*prec);

						if (dd>1.f) dd=1.f;

						if (dd<0.f) dd=0.f;
						
						fb=((1.f-dd)*6.f + (EEfabs(ep->nrml[k].x)+EEfabs(ep->nrml[k].y)))*0.125f;
						fr=((0.6f-dd)*6.f + (EEfabs(ep->nrml[k].z)+EEfabs(ep->nrml[k].y)))*0.125f;//(1.f-dd);						

						if (fr<0.f) fr=0.f;
						else fr=max(ffr,fr*255.f);

						fb*=255.f;
						lfr = fr;
						lfb = fb;
						ep->tv[k].color=( 0xff001E00L | ( (lfr & 255) << 16) | (lfb & 255) );
						//GG component locked at 0x1E
				}
			}

			Delayed_EERIEDRAWPRIM(ep);
		}			
	}			
	}
}

void ApplyDynLight_VertexBuffer(EERIEPOLY *ep,SMY_VERTEX *_pVertex,unsigned short _usInd0,unsigned short _usInd1,unsigned short _usInd2,unsigned short _usInd3);
void ApplyDynLight_VertexBuffer_2(EERIEPOLY *ep,short x,short y,SMY_VERTEX *_pVertex,unsigned short _usInd0,unsigned short _usInd1,unsigned short _usInd2,unsigned short _usInd3);

TILE_LIGHTS tilelights[MAX_BKGX][MAX_BKGZ];

void InitTileLights()
{
	for (long j=0;j<MAX_BKGZ;j++)
	for (long i=0;i<MAX_BKGZ;i++)
	{
		tilelights[i][j].el=NULL;
		tilelights[i][j].max=0;
		tilelights[i][j].num=0;
	}
}

void ComputeTileLights(short x,short z)
{
	tilelights[x][z].num=0;
	float xx=((float)x+0.5f)*ACTIVEBKG->Xdiv;
	float zz=((float)z+0.5f)*ACTIVEBKG->Zdiv;

	for (long i=0;i<TOTPDL;i++)
	{
		if(closerThan(Vec2f(xx, zz), Vec2f(PDL[i]->pos.x, PDL[i]->pos.z), PDL[i]->fallend + 60.f)) {
			
			if (tilelights[x][z].num>=tilelights[x][z].max)
			{
				tilelights[x][z].max++;
				tilelights[x][z].el=(EERIE_LIGHT **)realloc(tilelights[x][z].el,sizeof(EERIE_LIGHT *)*(tilelights[x][z].max));
			}

			tilelights[x][z].el[tilelights[x][z].num]=PDL[i];
			tilelights[x][z].num++;
		}
	}
}

void ClearTileLights()
{
	for (long j=0;j<MAX_BKGZ;j++)
	for (long i=0;i<MAX_BKGZ;i++)
	{
		tilelights[i][j].max=0;
		tilelights[i][j].num=0;

		if (tilelights[i][j].el != NULL)
		{
			free (tilelights[i][j].el);
			tilelights[i][j].el=NULL;
		}
	}
}

//-----------------------------------------------------------------------------

void ARX_PORTALS_Frustrum_RenderRoomTCullSoft(long room_num,EERIE_FRUSTRUM_DATA * frustrums,long prec,long tim)
{

	
	if (RoomDraw[room_num].count)
	{
		if(!portals->room[room_num].pVertexBuffer)
		{
			char tTxt[256];
			sprintf(tTxt,"portals %ld - Zero Polys",room_num);
			LogError<< tTxt<<" Error Portals";
			return;
		}
		
		SMY_VERTEX * pMyVertex = portals->room[room_num].pVertexBuffer->lock(NoOverwrite);
		
		unsigned short *pIndices=portals->room[room_num].pussIndice;

		FAST_BKG_DATA * feg;
		EERIEPOLY * ep;
		EP_DATA *pEPDATA = &portals->room[room_num].epdata[0];

		for (long  lll=0; lll<portals->room[room_num].nb_polys; lll++, pEPDATA++)
		{

			feg = &ACTIVEBKG->fastdata[pEPDATA->px][pEPDATA->py];

			if (!feg->treat)
			{
				long ix=max(0,pEPDATA->px-1);
				long ax=min(ACTIVEBKG->Xsize-1,pEPDATA->px+1);
				long iz=max(0,pEPDATA->py-1);
				long az=min(ACTIVEBKG->Zsize-1,pEPDATA->py+1);
				
				(void)checked_range_cast<short>(iz);
				(void)checked_range_cast<short>(ix);
				(void)checked_range_cast<short>(az);
				(void)checked_range_cast<short>(ax);

				for (long nz=iz;nz<=az;nz++)
				for (long nx=ix;nx<=ax;nx++)

				{
					FAST_BKG_DATA * feg2 = &ACTIVEBKG->fastdata[nx][nz];

					if (!feg2->treat)
					{
						feg2->treat=1;

						if (USE_LIGHT_OPTIM) 
								ComputeTileLights(static_cast<short>(nx), static_cast<short>(nz));
					}
				}
			}

			ep=&feg->polydata[pEPDATA->idx];

			if	(!ep->tex)
			{
				continue;
			}

			if (ep->type & (POLY_IGNORE | POLY_NODRAW| POLY_HIDE))
			{
				continue;
			}
			
			if (FrustrumsClipPoly(frustrums,ep))
			{
				continue;
			}

			//Clipp ZNear + Distance pour les ZMapps!!!
			float fDist=(ep->center.x*efpPlaneNear.a + ep->center.y*efpPlaneNear.b + ep->center.z*efpPlaneNear.c + efpPlaneNear.d);

			if(ep->v[0].rhw<-fDist)
			{
				continue;
			}

			fDist-=ep->v[0].rhw;

			Vec3f nrm;
			nrm.x=ep->v[2].p.x-ACTIVECAM->pos.x;
			nrm.y=ep->v[2].p.y-ACTIVECAM->pos.y;
			nrm.z=ep->v[2].p.z-ACTIVECAM->pos.z;

			int to;

			if(ep->type&POLY_QUAD)
			{
				if(	(!(ep->type&POLY_DOUBLESIDED))&&
					(dot( ep->norm , nrm )>0.f)&&
					(dot( ep->norm2 , nrm )>0.f) )
				{
					continue;
				}

				to=4;
			}
			else
			{
				if(	(!(ep->type&POLY_DOUBLESIDED))&&
					(dot( ep->norm , nrm )>0.f) )
				{
					continue;
				}

				to=3;
			}

			unsigned short *pIndicesCurr;
			unsigned long *pNumIndices;

			if(ep->type&POLY_TRANS)
			{
				if(ep->transval>=2.f)  //MULTIPLICATIVE
				{
					pIndicesCurr=pIndices+ep->tex->tMatRoom[room_num].uslStartCull_TMultiplicative+ep->tex->tMatRoom[room_num].uslNbIndiceCull_TMultiplicative;
					pNumIndices=&ep->tex->tMatRoom[room_num].uslNbIndiceCull_TMultiplicative;
				}
				else
				{
					if(ep->transval>=1.f) //ADDITIVE
					{
						pIndicesCurr=pIndices+ep->tex->tMatRoom[room_num].uslStartCull_TAdditive+ep->tex->tMatRoom[room_num].uslNbIndiceCull_TAdditive;
						pNumIndices=&ep->tex->tMatRoom[room_num].uslNbIndiceCull_TAdditive;
					}
					else
					{
						if(ep->transval>0.f) //NORMAL TRANS
						{
							pIndicesCurr=pIndices+ep->tex->tMatRoom[room_num].uslStartCull_TNormalTrans+ep->tex->tMatRoom[room_num].uslNbIndiceCull_TNormalTrans;
							pNumIndices=&ep->tex->tMatRoom[room_num].uslNbIndiceCull_TNormalTrans;
						}
						else
						{
							//SUBTRACTIVE
							pIndicesCurr=pIndices+ep->tex->tMatRoom[room_num].uslStartCull_TSubstractive+ep->tex->tMatRoom[room_num].uslNbIndiceCull_TSubstractive;
							pNumIndices=&ep->tex->tMatRoom[room_num].uslNbIndiceCull_TSubstractive;
						}
					}
				}
			}
			else
			{
				pIndicesCurr=pIndices+ep->tex->tMatRoom[room_num].uslStartCull+ep->tex->tMatRoom[room_num].uslNbIndiceCull;
				pNumIndices=&ep->tex->tMatRoom[room_num].uslNbIndiceCull;
				
				if(ZMAPMODE)
				{
					if((fDist<200)&&(ep->tex->TextureRefinement))
					{
						ep->tex->TextureRefinement->vPolyZMap.push_back(ep);
					}
				}
			}

			SMY_VERTEX *pMyVertexCurr;

				*pIndicesCurr++=ep->uslInd[0];
				*pIndicesCurr++=ep->uslInd[1];
				*pIndicesCurr++=ep->uslInd[2];

				if(to&4)
				{
					*pIndicesCurr++=ep->uslInd[3];
					*pIndicesCurr++=ep->uslInd[2];
					*pIndicesCurr++=ep->uslInd[1];
					*pNumIndices+=6;
				}
				else
				{
					*pNumIndices+=3;
				}
				pMyVertexCurr=&pMyVertex[ep->tex->tMatRoom[room_num].uslStartVertex];
		

			if (!Project.improve)  // Normal View...			
			{
				if(ep->type&POLY_GLOW) 
				{
					pMyVertexCurr[ep->uslInd[0]].color=pMyVertexCurr[ep->uslInd[1]].color=pMyVertexCurr[ep->uslInd[2]].color=0xFFFFFFFF;

					if(to&4)
					{
						pMyVertexCurr[ep->uslInd[3]].color=0xFFFFFFFF;
					}
				}
				else 
				{
					if(ep->type&POLY_LAVA)
					{
						if((FRAME_COUNT<=0)&&(!(ep->type&POLY_TRANS)))
						{
							if(ModeLight & MODE_DYNAMICLIGHT)
							{
								ApplyDynLight(ep);
							}
							else
							{
								ep->tv[0].color=ep->v[0].color;	
								ep->tv[1].color=ep->v[1].color;	
								ep->tv[2].color=ep->v[2].color;		

								if(to&4)
								{
									ep->tv[3].color=ep->v[3].color;
								}
							}
						}

						ManageLava_VertexBuffer(ep,to,tim,pMyVertexCurr);

						vPolyLava.push_back(ep);

						pMyVertexCurr[ep->uslInd[0]].color=ep->tv[0].color;
						pMyVertexCurr[ep->uslInd[1]].color=ep->tv[1].color;
						pMyVertexCurr[ep->uslInd[2]].color=ep->tv[2].color;

						if(to&4)
						{
							pMyVertexCurr[ep->uslInd[3]].color=ep->tv[3].color;
						}
					}
					else
					{
						if((FRAME_COUNT<=0)&&(!(ep->type&POLY_TRANS)))
						{
							if(ModeLight & MODE_DYNAMICLIGHT)
							{
																		
									if (USE_LIGHT_OPTIM)
									ApplyDynLight_VertexBuffer_2(	ep,pEPDATA->px,pEPDATA->py,
																pMyVertexCurr,
																ep->uslInd[0],
																ep->uslInd[1],
																ep->uslInd[2],
																ep->uslInd[3]);
																
									else
									ApplyDynLight_VertexBuffer(	ep,
																pMyVertexCurr,
																ep->uslInd[0],
																ep->uslInd[1],
																ep->uslInd[2],
																ep->uslInd[3]);
								
								}
							else
							{
										pMyVertexCurr[ep->uslInd[0]].color=ep->v[0].color;
										pMyVertexCurr[ep->uslInd[1]].color=ep->v[1].color;
										pMyVertexCurr[ep->uslInd[2]].color=ep->v[2].color;

									if(to&4)
									{
											pMyVertexCurr[ep->uslInd[3]].color=ep->v[3].color;
										}
									}
								}

						if(ep->type&POLY_WATER)
						{
							ManageWater_VertexBuffer(ep,to,tim,pMyVertexCurr);
							vPolyWater.push_back(ep);
						}
					}
				}

				if ((ViewMode & VIEWMODE_WIRE))
				{
					if (EERIERTPPoly(ep))
						EERIEPOLY_DrawWired(ep);
				}

					}
			else // Improve Vision Activated
			{			
				//!!!!!!!!! NOT OPTIMIZED T&L !!!!!!!!!!
				if ((FRAME_COUNT<=0)&&(!(ep->type&POLY_TRANS)))
				{
					if (!EERIERTPPoly(ep)) // RotTransProject Vertices
					{
						continue; 
					}

					if ( ModeLight & MODE_DYNAMICLIGHT ) 	ApplyDynLight(ep);										
					else 
					{	
						ep->tv[0].color=ep->v[0].color;	
						ep->tv[1].color=ep->v[1].color;	
						ep->tv[2].color=ep->v[2].color;				

						if(to&4)
						{
							ep->tv[3].color=ep->v[3].color;
						}
					}
				
					for (long k=0;k<to;k++) 
					{
						long lfr,lfb;
						float fr,fb;
						long lr=(ep->tv[k].color>>16) & 255;
						float ffr=(float)(lr);
						
						float dd=(ep->tv[k].rhw*prec);

						if (dd>1.f) dd=1.f;

						if (dd<0.f) dd=0.f;
						
						fb=((1.f-dd)*6.f + (EEfabs(ep->nrml[k].x)+EEfabs(ep->nrml[k].y)))*0.125f;
						fr = ((0.6f - dd) * 6.f + (EEfabs(ep->nrml[k].z) + EEfabs(ep->nrml[k].y))) * 0.125f;

						if (fr<0.f) fr=0.f;
						else fr=max(ffr,fr*255.f);

						fr=min(fr,255.f);
						fb*=255.f;
						fb=min(fb,255.f);
						lfr = fr;
						lfb = fb;
				
						ep->tv[k].color=( 0xff001E00L | ( (lfr & 255) << 16) | (lfb & 255) );
					
					}

					pMyVertexCurr[ep->uslInd[0]].color=ep->tv[0].color;
					pMyVertexCurr[ep->uslInd[1]].color=ep->tv[1].color;
					pMyVertexCurr[ep->uslInd[2]].color=ep->tv[2].color;

					if(to&4)
					{
						pMyVertexCurr[ep->uslInd[3]].color=ep->tv[3].color;
					}
				}
			}
		}

		portals->room[room_num].pVertexBuffer->unlock();
	
		//render opaque
		GRenderer->SetCulling(Renderer::CullNone);
		int iNbTex=portals->room[room_num].usNbTextures;
		TextureContainer **ppTexCurr=portals->room[room_num].ppTextureContainer;

		while(iNbTex--)
		{
			TextureContainer *pTexCurr=*ppTexCurr;

			if (ViewMode & VIEWMODE_FLAT) 
				GRenderer->ResetTexture(0);
			else
				GRenderer->SetTexture(0, pTexCurr);

			if(pTexCurr->userflags&POLY_METAL)
			{
				GRenderer->GetTextureStage(0)->SetColorOp(TextureStage::OpModulate2X);
			}
			else
			{
				GRenderer->GetTextureStage(0)->SetColorOp(TextureStage::OpModulate);
			}
			
			if(pTexCurr->tMatRoom[room_num].uslNbIndiceCull)
			{
				GRenderer->SetAlphaFunc(Renderer::CmpGreater, .5f);
				portals->room[room_num].pVertexBuffer->drawIndexed(Renderer::TriangleList, pTexCurr->tMatRoom[room_num].uslNbVertex, pTexCurr->tMatRoom[room_num].uslStartVertex,
					&portals->room[room_num].pussIndice[pTexCurr->tMatRoom[room_num].uslStartCull],
					pTexCurr->tMatRoom[room_num].uslNbIndiceCull);
				GRenderer->SetAlphaFunc(Renderer::CmpNotEqual, 0.f);
				
				EERIEDrawnPolys+=pTexCurr->tMatRoom[room_num].uslNbIndiceCull;
				pTexCurr->tMatRoom[room_num].uslNbIndiceCull=0;
						}
						
			ppTexCurr++;
		}

		//////////////////////////////
		//ZMapp
		GRenderer->GetTextureStage(0)->SetColorOp(TextureStage::OpModulate);

		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		GRenderer->SetRenderState(Renderer::DepthWrite, false);

		iNbTex=portals->room[room_num].usNbTextures;
		ppTexCurr=portals->room[room_num].ppTextureContainer;
		
		while ( iNbTex-- ) //For each tex in portals->room[room_num]
		{
			TextureContainer * pTexCurr	= *ppTexCurr;

			if ( pTexCurr->TextureRefinement && pTexCurr->TextureRefinement->vPolyZMap.size() )
			{
					//---------------------------------------------------------------------------
					//																		 INIT
				
				GRenderer->SetTexture(0, pTexCurr->TextureRefinement);
				
				dynamicVertices.lock();
				unsigned short * pussInd = dynamicVertices.indices;
				unsigned short iNbIndice = 0;

				vector<EERIEPOLY *>::iterator it		=	pTexCurr->TextureRefinement->vPolyZMap.begin();
	

				
				//---------------------------------------------------------------------------
				//																		 LOOP
				for (; it != pTexCurr->TextureRefinement->vPolyZMap.end(); ++it)
				{
					EERIEPOLY * ep = *it;
					
					unsigned short iNbVertex = (ep->type & POLY_QUAD) ? 4 : 3;
					SMY_VERTEX3 * pVertex = dynamicVertices.append(iNbVertex);
					
					if(!pVertex) {
						dynamicVertices.unlock();
						if(dynamicVertices.nbindices) {
							dynamicVertices.draw(Renderer::TriangleList);
						}
						dynamicVertices.reset();
						dynamicVertices.lock();
						iNbIndice = 0;
						pussInd = dynamicVertices.indices;
						pVertex = dynamicVertices.append(iNbVertex);
					}
					
					//-----------------------------------------------------------------------
					//																PRECALCUL
					float tu[4];
					float tv[4];
					float _fTransp[4];
					unsigned short nu;
					long nrm=0;
					
					if	(	(EEfabs(ep->nrml[0].y)>=0.9f)
						||	(EEfabs(ep->nrml[1].y)>=0.9f)
						||	(EEfabs(ep->nrml[2].y)>=0.9f)	)
						nrm=1;
					
					for (nu=0;nu<iNbVertex;nu++)
					{
						if (nrm)
						{
							tu[nu]=(ep->v[nu].p.x*( 1.0f / 50 ));
							tv[nu]=(ep->v[nu].p.z*( 1.0f / 50 ));
						}
						else
						{
							tu[nu]=ep->v[nu].uv.x*4.f;
							tv[nu]=ep->v[nu].uv.y*4.f;						
						}
						
						float t = max(10.f, fdist(ACTIVECAM->pos, ep->v[nu].p) - 80.f);
						
						_fTransp[nu] = (150.f - t) * 0.006666666f;
						
						if (_fTransp[nu] < 0.f)
							_fTransp[nu]		=	0.f;
						// t cannot be greater than 1.f (b should be negative for that)
					}
					
					//-----------------------------------------------------------------------
					//																FILL DATA
					for ( int idx = 0  ; idx < iNbVertex ; ++idx )
					{
						pVertex->p.x				=	ep->v[idx].p.x;
						pVertex->p.y				=	- ep->v[idx].p.y;
						pVertex->p.z				=	ep->v[idx].p.z;
						pVertex->color = Color::gray(_fTransp[idx]).toBGR();
						pVertex->uv[0].x				=	tu[idx]; 
						pVertex->uv[0].y				=	tv[idx]; 
						pVertex++;
						
						*pussInd++				=	iNbIndice++;
						dynamicVertices.nbindices++;
					}
					
					if(iNbVertex&4) {
						*pussInd++=iNbIndice-2;
						*pussInd++=iNbIndice-3;
						dynamicVertices.nbindices += 2;
					}
					
				}

					//---------------------------------------------------------------------------
					//														   CLEAR CURRENT ZMAP
				pTexCurr->TextureRefinement->vPolyZMap.clear();
				
				dynamicVertices.unlock();
				if(dynamicVertices.nbindices) {
					dynamicVertices.draw(Renderer::TriangleList);
				}
				dynamicVertices.done();
				
			}
			
			ppTexCurr++;
		} //END  while ( iNbTex-- ) ----------------------------------------------------------
		
		GRenderer->SetRenderState(Renderer::DepthWrite, true);
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	}
}

//-----------------------------------------------------------------------------

void ARX_PORTALS_Frustrum_RenderRoom_TransparencyTSoftCull(long room_num)
{
	if (RoomDraw[room_num].count)
	{
		//render transparency
		int iNbTex=portals->room[room_num].usNbTextures;
		TextureContainer **ppTexCurr=portals->room[room_num].ppTextureContainer;

		while(iNbTex--) {
			
			TextureContainer * pTexCurr = *ppTexCurr;
			GRenderer->SetTexture(0, pTexCurr);

			//NORMAL TRANS
			if(pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TNormalTrans)
			{
				SetZBias(2);
				GRenderer->SetBlendFunc(Renderer::BlendSrcColor, Renderer::BlendDstColor);
			
				portals->room[room_num].pVertexBuffer->drawIndexed(Renderer::TriangleList, pTexCurr->tMatRoom[room_num].uslNbVertex, pTexCurr->tMatRoom[room_num].uslStartVertex, &portals->room[room_num].pussIndice[pTexCurr->tMatRoom[room_num].uslStartCull_TNormalTrans], pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TNormalTrans);
				
				EERIEDrawnPolys+=pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TNormalTrans;
				pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TNormalTrans=0;
			}

			//MULTIPLICATIVE
			if(pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TMultiplicative)
			{
				SetZBias(2);
				GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
				
				portals->room[room_num].pVertexBuffer->drawIndexed(Renderer::TriangleList, pTexCurr->tMatRoom[room_num].uslNbVertex, pTexCurr->tMatRoom[room_num].uslStartVertex, &portals->room[room_num].pussIndice[pTexCurr->tMatRoom[room_num].uslStartCull_TMultiplicative], pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TMultiplicative);
				
				EERIEDrawnPolys+=pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TMultiplicative;
				pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TMultiplicative=0;
			}

			//ADDITIVE
			if(pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TAdditive)
			{
				SetZBias(2);
				GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
				
				portals->room[room_num].pVertexBuffer->drawIndexed(Renderer::TriangleList, pTexCurr->tMatRoom[room_num].uslNbVertex, pTexCurr->tMatRoom[room_num].uslStartVertex, &portals->room[room_num].pussIndice[pTexCurr->tMatRoom[room_num].uslStartCull_TAdditive], pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TAdditive);
				
				EERIEDrawnPolys+=pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TAdditive;
				pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TAdditive=0;
			}

			//SUBSTRACTIVE
			if(pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TSubstractive)
			{
				SetZBias(8);

				GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
				
				portals->room[room_num].pVertexBuffer->drawIndexed(Renderer::TriangleList, pTexCurr->tMatRoom[room_num].uslNbVertex, pTexCurr->tMatRoom[room_num].uslStartVertex, &portals->room[room_num].pussIndice[pTexCurr->tMatRoom[room_num].uslStartCull_TSubstractive], pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TSubstractive);
				
				EERIEDrawnPolys+=pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TSubstractive;
				pTexCurr->tMatRoom[room_num].uslNbIndiceCull_TSubstractive=0;
			}

			ppTexCurr++;
		}
	}
}


void ARX_PORTALS_ComputeRoom(long room_num,EERIE_2D_BBOX * bbox,long prec,long tim)
{
	if (portals==NULL) return;

	if (bbox->min.x>=DANAESIZX) return;

	if (bbox->min.y>=DANAESIZY) return;

	if (bbox->max.x<0) return;

	if (bbox->max.y<0) return;

	if (bbox->min.x>=bbox->max.x) return;

	if (bbox->min.y>=bbox->max.y) return;

	if (RoomDraw[room_num].count==0)
		RoomDrawListAdd(room_num);

	ARX_PORTALS_BlendBBox(room_num,bbox);
	RoomDraw[room_num].count++;
	
		// Now Checks For room Portals !!!
	for (long lll=0;lll<portals->room[room_num].nb_portals;lll++)
	{
		if (portals->portals[portals->room[room_num].portals[lll]].useportal) continue;

		EERIE_PORTALS * po=&portals->portals[portals->room[room_num].portals[lll]];
		EERIEPOLY * epp=&po->poly;
		
		float threshold = square(ACTIVECAM->cdepth - ACTIVECAM->cdepth * fZFogEnd);
		if((distSqr(ACTIVECAM->pos, epp->v[0].p) > threshold)
		   && (distSqr(ACTIVECAM->pos, epp->v[2].p) > threshold) 
		   && (distSqr(ACTIVECAM->pos, epp->center) > threshold)) {
			portals->portals[portals->room[room_num].portals[lll]].useportal=2;
			continue;
		}
		
		if (!EERIERTPPoly2(epp)) continue;

		
		int Cull=BackFaceCull2D(epp->tv);

		
		EERIE_2D_BBOX n_bbox;
		n_bbox.max.x=n_bbox.min.x=epp->tv[0].p.x;
		n_bbox.max.y=n_bbox.min.y=epp->tv[0].p.y;
		long to;

		if (epp->type & POLY_QUAD)
			to=4;
		else
			to=3;

		float minz=epp->tv[0].p.z;
		float maxz=epp->tv[0].p.z;

		for (long nn=1;nn<to;nn++)
		{
			n_bbox.min.x=min(n_bbox.min.x , epp->tv[nn].p.x);
			n_bbox.min.y=min(n_bbox.min.y , epp->tv[nn].p.y);
			n_bbox.max.x=max(n_bbox.max.x , epp->tv[nn].p.x);
			n_bbox.max.y=max(n_bbox.max.y , epp->tv[nn].p.y);
			minz=min(minz,epp->tv[0].p.z);
			maxz=max(maxz,epp->tv[0].p.z);
		}

		if (minz>0.5f) continue;

		if (	bbox->min.x > n_bbox.max.x || n_bbox.min.x > bbox->max.x
			||	bbox->min.y > n_bbox.max.y || n_bbox.min.y > bbox->max.y)
			continue;

		if (Cull)
			EERIEPOLY_DrawWired(epp, Color::red);
		else
			EERIEPOLY_DrawWired(epp, Color::green);
		
		n_bbox.min.x=max(n_bbox.min.x,bbox->min.x);
		n_bbox.min.y=max(n_bbox.min.y,bbox->min.y);
		n_bbox.max.x=min(n_bbox.max.x,bbox->max.x);
		n_bbox.max.y=min(n_bbox.max.y,bbox->max.y);
		
		if (po->room_1==room_num)
		{
			if (!Cull)
			{
				portals->portals[portals->room[room_num].portals[lll]].useportal=1;
				ARX_PORTALS_ComputeRoom(po->room_2,&n_bbox,prec,tim);
			}
		}
		else if (po->room_2==room_num)
		{
			if (Cull)
			{
				portals->portals[portals->room[room_num].portals[lll]].useportal=1;
				ARX_PORTALS_ComputeRoom(po->room_1,&n_bbox,prec,tim);
			}			
		}	
	}
}

long ARX_PORTALS_Frustrum_ComputeRoom(long room_num,EERIE_FRUSTRUM * frustrum,long prec,long tim)
{
	long portals_count=0;

	if (portals==NULL) return 0;

	if (RoomDraw[room_num].count==0)
	{
		RoomDrawListAdd(room_num);
	}

	RoomFrustrumAdd(room_num,frustrum);
	RoomDraw[room_num].count++;

	float fClippZFar=ACTIVECAM->cdepth*(fZFogEnd*1.1f);

	// Now Checks For room Portals !!!
	for (long lll=0;lll<portals->room[room_num].nb_portals;lll++)
	{
		if (portals->portals[portals->room[room_num].portals[lll]].useportal) continue;

		EERIE_PORTALS * po=&portals->portals[portals->room[room_num].portals[lll]];
		EERIEPOLY * epp=&po->poly;
	
		//clipp NEAR & FAR
		unsigned char ucVisibilityNear=0;
		unsigned char ucVisibilityFar=0;
		float fDist0=(efpPlaneNear.a*epp->v[0].p.x)+(efpPlaneNear.b*epp->v[0].p.y)+(efpPlaneNear.c*epp->v[0].p.z)+efpPlaneNear.d;

		if(fDist0<0.f) ucVisibilityNear++;
		else
		{
			if(fDist0>fClippZFar) ucVisibilityFar++;
		}

		fDist0=(efpPlaneNear.a*epp->v[1].p.x)+(efpPlaneNear.b*epp->v[1].p.y)+(efpPlaneNear.c*epp->v[1].p.z)+efpPlaneNear.d;

		if(fDist0<0.f) ucVisibilityNear++;
		else
		{
			if(fDist0>fClippZFar) ucVisibilityFar++;
		}

		fDist0=(efpPlaneNear.a*epp->v[2].p.x)+(efpPlaneNear.b*epp->v[2].p.y)+(efpPlaneNear.c*epp->v[2].p.z)+efpPlaneNear.d;

		if(fDist0<0.f) ucVisibilityNear++;
		else
		{
			if(fDist0>fClippZFar) ucVisibilityFar++;
		}

		fDist0=(efpPlaneNear.a*epp->v[3].p.x)+(efpPlaneNear.b*epp->v[3].p.y)+(efpPlaneNear.c*epp->v[3].p.z)+efpPlaneNear.d;

		if(fDist0<0.f) ucVisibilityNear++;
		else
		{
			if(fDist0>fClippZFar) ucVisibilityFar++;
		}

		if(	(ucVisibilityFar&4)||(ucVisibilityNear&4) )
		{
			portals->portals[portals->room[room_num].portals[lll]].useportal=2;
			continue;
		}

		Vec3f pos;
		pos.x=epp->center.x-ACTIVECAM->pos.x;
		pos.y=epp->center.y-ACTIVECAM->pos.y;
		pos.z=epp->center.z-ACTIVECAM->pos.z;
		float fRes = dot(pos, epp->norm);

		long ret=1;

		if(IsSphereInFrustrum(epp->v[0].rhw,&epp->center,frustrum))
		{
			ret=0;
		}

		if (ret) 
		{
			EERIERTPPoly2(epp);

			if (NEED_TEST_TEXT)
				EERIEPOLY_DrawWired(epp, Color::magenta);

			continue;
		}

		portals_count++;

		EERIERTPPoly2(epp);

		int Cull;

		if (fRes<0.f) Cull=0;
		else Cull=1;

		
		EERIE_FRUSTRUM fd;
		CreateFrustrum(&fd,epp,Cull);
		
		if (NEED_TEST_TEXT)
		{
			if (Cull)
				EERIEPOLY_DrawWired(epp, Color::red);
			else
				EERIEPOLY_DrawWired(epp, Color::blue);
		}
		

		if (po->room_1==room_num)
		{
			if (!Cull)
			{
				portals->portals[portals->room[room_num].portals[lll]].useportal=1;
				ARX_PORTALS_Frustrum_ComputeRoom(po->room_2,&fd,prec,tim);
			}
		}
		else if (po->room_2==room_num)
		{
			if (Cull)
			{
				portals->portals[portals->room[room_num].portals[lll]].useportal=1;
				ARX_PORTALS_Frustrum_ComputeRoom(po->room_1,&fd,prec,tim);
			}			
		}	
	}

	return portals_count; 
			}

		
bool Clip_Visible(const Vec3f * orgn, Vec3f * dest)
{
	register float dx,dy,dz,adx,ady,adz,ix,iz;
	register float x0,z0;
	register float forr,temp;
 
	dx=(dest->x-orgn->x);
	adx=EEfabs(dx);
	dy=(dest->y-orgn->y);
	ady=EEfabs(dy);
	dz=(dest->z-orgn->z);
	adz=EEfabs(dz);

	x0=orgn->x;
	z0=orgn->z;

	if ( (adx>=ady) && (adx>=adz)) 
	{
		if (adx != dx)
		{
			ix = -1.f * PASS;
		}
		else
		{
			ix = 1.f * PASS;
		}

		forr=adx;
		temp=1.f/(adx/PASS);
		iz=dz*temp;
	}
	else if ( (ady>=adx) && (ady>=adz)) 
	{
		forr=ady;
		temp=1.f/(ady/PASS);
		ix=dx*temp;
		iz=dz*temp;
	}
	else 
	{
		if (adz != dz)
		{
			iz = -1.f * PASS;
		}
		else
		{
			iz = 1.f * PASS;
		}

		forr=adz;
		temp=1.f/(adz/PASS);
		ix=dx*temp;
	}
	

long curpixel;
	long tot;
	tot=0;
	
	long x,y;
	FAST_BKG_DATA * LAST_eg=NULL;
	curpixel=2;
	x0+=ix*2;
		z0+=iz*2;
		forr-=PASS*2;		

	while (forr>PASSS)
	{
			FAST_BKG_DATA * feg;
			x = x0 * ACTIVEBKG->Xmul;
			y = z0 * ACTIVEBKG->Zmul;
			feg=&ACTIVEBKG->fastdata[x][y];

		if (feg!=LAST_eg)
		{
				
			LAST_eg=feg;
			
			if (feg->nothing)	tot += 2; 
		
			if (tot>MAX_OUT) return false;
		}

		float v=(float)curpixel*( 1.0f / 5 );

		if (v<1.f) v=1.f;

		x0+=ix*v;
		z0+=iz*v;
		forr-=PASS*v;		
	}

	return true;//hard;
}



bool spGetTruePolyY(const EERIEPOLY * ep, const Vec3f * pos, float * ret)
	{
		
	Vec3f n,s21,s31;

	s21.x=ep->v[1].p.x-ep->v[0].p.x;
	s21.y=ep->v[1].p.y-ep->v[0].p.y;
	s21.z=ep->v[1].p.z-ep->v[0].p.z;
	s31.x=ep->v[2].p.x-ep->v[0].p.x;
	s31.y=ep->v[2].p.y-ep->v[0].p.y;
	s31.z=ep->v[2].p.z-ep->v[0].p.z;

	n.y=(s21.z*s31.x)-(s21.x*s31.z);
	n.x=(s21.y*s31.z)-(s21.z*s31.y);
	n.z=(s21.x*s31.y)-(s21.y*s31.x);

	// uses s21.x instead of d
	s21.x=ep->v[0].p.x*n.x+ep->v[0].p.y*n.y+ep->v[0].p.z*n.z;

	s21.x=(s21.x-(n.x*pos->x)-(n.z*pos->z))/n.y;
	*ret=s21.x;
	return true;

}

extern long SPECIAL_DRAGINTER_RENDER;
long MAX_FRAME_COUNT=0;
//*************************************************************************************
// Main Background Rendering Proc.
// ie: Big Mess
//*************************************************************************************
///////////////////////////////////////////////////////////
void ARX_SCENE_Render(long flag) {
	
	FrameCount++;

	FRAME_COUNT++;

	if (FRAME_COUNT>MAX_FRAME_COUNT) FRAME_COUNT=0;

	if (!TESTMODE) FRAME_COUNT=0;

	if (EDITMODE) FRAME_COUNT=0;

	if ((player.Interface & INTER_MAP ) &&  (!(player.Interface & INTER_COMBATMODE))) 
		FRAME_COUNT=0;
		
	static long x0=0;
	static long x1=0;
	static long z0=0;
	static long z1=0;
	long i;
	EERIEPOLY * ep;
	FAST_BKG_DATA * feg;

	unsigned long tim = (unsigned long)(arxtime);
	
	WATEREFFECT+=0.0005f*_framedelay;

	if (flag == 3)
		return;
	
	float cval=(float)ACTIVECAM->clip3D+4;
	long lcval = cval;

	//TODO(lubosz): no if / loop ?

	{

		PrepareActiveCamera();
		float xx=(float)(ACTIVECAM->pos.x*ACTIVEBKG->Xmul);
		float yy=(float)(ACTIVECAM->pos.z*ACTIVEBKG->Zmul);
		
		ACTIVECAM->Xsnap = xx;
		ACTIVECAM->Zsnap = yy;
		ACTIVECAM->Xsnap = clamp(ACTIVECAM->Xsnap,0,ACTIVEBKG->Xsize-1);
		ACTIVECAM->Zsnap = clamp(ACTIVECAM->Zsnap,0,ACTIVEBKG->Zsize-1);
		
		x0=ACTIVECAM->Xsnap-lcval;
		x1=ACTIVECAM->Xsnap+lcval;
		z0=ACTIVECAM->Zsnap-lcval;
		z1=ACTIVECAM->Zsnap+lcval;
		x0 = clamp(x0,0,ACTIVEBKG->Xsize-1);
		x1 = clamp(x1,0,ACTIVEBKG->Xsize-1);
		z0 = clamp(z0,0,ACTIVEBKG->Zsize-2);
		z1 = clamp(z1,0,ACTIVEBKG->Xsize-2);
			
			
		ACTIVEBKG->Backg[ACTIVECAM->Xsnap+ACTIVECAM->Zsnap * ACTIVEBKG->Xsize].treat = 1;
		float prec = 1.f / (ACTIVECAM->cdepth * ACTIVECAM->Zmul);

			

	long lll;
	
	// Temporary Hack...
	long LAST_FC=FRAME_COUNT;
	FRAME_COUNT=0;

	if ((FRAME_COUNT<=0) && (ModeLight & MODE_DYNAMICLIGHT)) PrecalcDynamicLighting(x0,z0,x1,z1);

	float temp0=radians(ACTIVECAM->angle.b);
	ACTIVECAM->norm.x=-(float)EEsin(temp0);
	ACTIVECAM->norm.y= (float)EEsin(radians(ACTIVECAM->angle.a));
	ACTIVECAM->norm.z= (float)EEcos(temp0);
	
	fnormalize(ACTIVECAM->norm);
	
	// Go for a growing-square-spirallike-render around the camera position
	// (To maximize Z-Buffer efficiency)
	temp0=0;
	Vec3f nrm;

	long zsnap=ACTIVECAM->Zsnap;
	zsnap=min((int)zsnap,ACTIVEBKG->Zsize-1);
	zsnap=max((int)zsnap,1);
	long xsnap=ACTIVECAM->Xsnap;
	xsnap=min((int)xsnap,ACTIVEBKG->Xsize-1);
	xsnap=max((int)xsnap,1);

 

	if (!USE_LIGHT_OPTIM)
	{
 		for (long j=0;j<ACTIVEBKG->Zsize;j++)
		{
			feg=&ACTIVEBKG->fastdata[0][j];

			for (i=0; i<ACTIVEBKG->Xsize; i++, feg++)
			{
				if (feg->treat)
					feg->treat=0;			
			}
		}
	}
	else
	{
		for (long j=z0;j<=z1;j++)
		{			
			for (i=x0; i<x1; i++)
			{
				feg=&ACTIVEBKG->fastdata[i][j];
				feg->treat=0;												
			}
		}

		for (long j=0;j<ACTIVEBKG->Zsize;j++)
		for (i=0; i<ACTIVEBKG->Xsize; i++)
		{
			if (tilelights[i][j].num)
				tilelights[i][j].num=0;
		}
			}


if (USE_PORTALS && portals)
{
	long room_num=ARX_PORTALS_GetRoomNumForPosition(&ACTIVECAM->pos,1);
	LAST_ROOM=room_num;

	if (room_num>-1)
	{
		ARX_PORTALS_InitDrawnRooms();
		
		long lprec = checked_range_cast<long>(prec);

		switch (USE_PORTALS)
		{
			case 1: {
				EERIE_2D_BBOX bbox;
				bbox.min.x=0;
				bbox.min.y=0;

				bbox.max.x = static_cast<float>( DANAESIZX );
				bbox.max.y = static_cast<float>( DANAESIZY );

				ARX_PORTALS_ComputeRoom(room_num,&bbox,lprec,tim);
				ARX_PORTALS_RenderRooms(lprec,tim);
				break;
			}
			case 2: {
				EERIE_FRUSTRUM frustrum;
				CreateScreenFrustrum(&frustrum);
				LAST_PORTALS_COUNT=ARX_PORTALS_Frustrum_ComputeRoom(room_num,&frustrum,lprec,tim);
				ARX_PORTALS_Frustrum_RenderRooms(lprec,tim);
				break;
			}
			case 3: {
				EERIE_FRUSTRUM frustrum;
				CreateScreenFrustrum(&frustrum);
				LAST_PORTALS_COUNT=ARX_PORTALS_Frustrum_ComputeRoom(room_num,&frustrum,lprec,tim);
				LogWarning << "unimplemented";
				break;
			}
			case 4: {
				EERIE_FRUSTRUM frustrum;
				CreateScreenFrustrum(&frustrum);
				LAST_PORTALS_COUNT=ARX_PORTALS_Frustrum_ComputeRoom(room_num,&frustrum,lprec,tim);
				ARX_PORTALS_Frustrum_RenderRoomsTCullSoft(lprec,tim);
				break;
			}
		}


		//ARX_SCENE_DilateBackground();
	}
}
else
{
	for (long n=0;n<=lcval;n++)
	{
		temp0+=100.f;

	for (long j=zsnap-n;j<=zsnap+n;j++)
	{
	for (i=xsnap-n;i<=xsnap+n;i++)
	{
		if ( (i!=xsnap-n) && (i!=xsnap+n) && (j!=zsnap-n) && (j!=zsnap+n) )
		{
			continue;
		}

		if ( (i<0) || (j<0) || (i>=ACTIVEBKG->Xsize) || (j>=ACTIVEBKG->Zsize) ) continue;

		if (i<x0) continue;

		if (i>x1) continue;
	
						feg = &ACTIVEBKG->fastdata[i][j]; 

		if (!feg->treat) continue;		

		for ( lll=0;lll<feg->nbpoly;lll++)
		{
			//SPECIFIC INTEL COMPILER  
			ep=&feg->polydata[lll];

			if (ep->type & (POLY_IGNORE | POLY_NODRAW))		
				continue;

			if ((ep->min.y > feg->frustrum_maxy)	
				|| (ep->max.y < feg->frustrum_miny))
				continue;
			
			// GO for 3D Backface Culling
			if (ep->type & POLY_DOUBLESIDED)
				GRenderer->SetCulling(Renderer::CullNone);
			else
			{
				
				nrm.x=ep->v[2].p.x-ACTIVECAM->pos.x;
				nrm.y=ep->v[2].p.y-ACTIVECAM->pos.y;
				nrm.z=ep->v[2].p.z-ACTIVECAM->pos.z;

				if ( ep->type & POLY_QUAD) 
				{
					if ( (dot( ep->norm , nrm )>0.f) &&
						 (dot( ep->norm2 , nrm )>0.f) )	
						continue;
				}
				else if ( dot( ep->norm , nrm )>0.f)
						continue;

				GRenderer->SetCulling(Renderer::CullCW);
			}
			 
							if (!EERIERTPPoly(ep)) 
				continue; 

			long to;
			if ( ep->type & POLY_QUAD) 
			{
				if (FRAME_COUNT<=0) ep->tv[3].color=ep->v[3].color;	

				to=4;
			}
			else to=3;

			if (ep->type & POLY_TRANS) 
			{
				ManageLavaWater(ep,to,tim);
				TransPol[TRANSPOLYSPOS++]=ep;

				if (TRANSPOLYSPOS>=MAX_TRANSPOL) TRANSPOLYSPOS=MAX_TRANSPOL-1;

				if (ViewMode)
				{
					if (ViewMode & VIEWMODE_WIRE) 
						EERIEPOLY_DrawWired(ep);
					
					if (ViewMode & VIEWMODE_NORMALS) 
						EERIEPOLY_DrawNormals(ep);
				}	

				continue;
			}

			if (!Project.improve)  // Normal View...			
			{
				if (ep->type & POLY_GLOW) ep->tv[0].color=ep->tv[1].color=ep->tv[2].color=ep->tv[3].color=0xFFFFFFFF;
				else 
				{
					if (FRAME_COUNT<=0)
					{
						if (ModeLight & MODE_DYNAMICLIGHT) 	ApplyDynLight(ep);					
						else
						{
							ep->tv[0].color=ep->v[0].color;	
							ep->tv[1].color=ep->v[1].color;	
							ep->tv[2].color=ep->v[2].color;		
						}
						
					}
				}

				ManageLavaWater(ep,to,tim);
				
				Delayed_EERIEDRAWPRIM(ep);

				if (ViewMode)
				{
		
					if (ViewMode & VIEWMODE_WIRE) 
						EERIEPOLY_DrawWired(ep);
					
					if (ViewMode & VIEWMODE_NORMALS) 
						EERIEPOLY_DrawNormals(ep);
				}	
			}
			else // Improve Vision Activated
			{			
				if (FRAME_COUNT<=0)
				{
					if ( ModeLight & MODE_DYNAMICLIGHT ) 	ApplyDynLight(ep);										
					else 
					{	
						ep->tv[0].color=ep->v[0].color;	
						ep->tv[1].color=ep->v[1].color;	
						ep->tv[2].color=ep->v[2].color;				
					}
				

					for (long k=0;k<to;k++) 
					{
						long lr=(ep->tv[k].color>>16) & 255;
						float ffr=(float)(lr);
							
						float dd=(ep->tv[k].p.z*prec);

						if (dd>1.f) dd=1.f;

						if (dd<0.f) dd=0.f;
						
						float fb=((1.f-dd)*6.f + (EEfabs(ep->nrml[k].x)+EEfabs(ep->nrml[k].y)))*0.125f;
						float fr=((0.6f-dd)*6.f + (EEfabs(ep->nrml[k].z)+EEfabs(ep->nrml[k].y)))*0.125f;//(1.f-dd);						

						if (fr<0.f) fr=0.f;
						else fr=max(ffr,fr*255.f);

						fb*=255.f;
						long lfb = fb;
						long lfr = fr;
						ep->tv[k].color=( 0xff001E00L | ( (lfr & 255) << 16) | (lfb & 255) );
						//GG component locked at 0x1E
					}
								}

				Delayed_EERIEDRAWPRIM(ep);
			}			
			}			
		}
	}
	}
}

	if(GInput->isKeyPressedNowPressed(Keyboard::Key_J))
		bOLD_CLIPP=!bOLD_CLIPP;

	if ((SHOWSHADOWS) && (!Project.improve))
		ARXDRAW_DrawInterShadows();

	FRAME_COUNT=LAST_FC;

	if(USE_PORTALS<3)
			Delayed_FlushAll();
		
		ARX_THROWN_OBJECT_Manage(checked_range_cast<unsigned long>(FrameDiff));
		
		RenderInter(0.f, 3200.f);
		

	if (DRAGINTER) // To render Dragged objs
	{
		SPECIAL_DRAGINTER_RENDER=1;
		ARX_INTERFACE_RenderCursor();

		if(USE_PORTALS<3)
			Delayed_FlushAll();

		SPECIAL_DRAGINTER_RENDER=0;
	}

	PopAllTriangleList();
	
					}

	if (ACTIVECAM->type!=CAM_TOPVIEW) 
	{
		
		if ((eyeball.exist!=0) && eyeballobj)
			ARXDRAW_DrawEyeBall();

		
		GRenderer->SetRenderState(Renderer::DepthWrite, false);

		if (BoomCount) 
			ARXDRAW_DrawPolyBoom();

		PopAllTriangleListTransparency();
		
		if(	(USE_PORTALS>2)&&
			(portals) )
		{
			ARX_PORTALS_Frustrum_RenderRooms_TransparencyT();
		}
		else
		{
			if (TRANSPOLYSPOS)
				ARXDRAW_DrawAllTransPolysPos();
		}
	}

if (HALOCUR>0)
{
	GRenderer->ResetTexture(0);
	GRenderer->SetBlendFunc(Renderer::BlendSrcColor, Renderer::BlendOne);	
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);			
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);

	for (i=0;i<HALOCUR;i++)
	{
		//blue halo rendering (keyword : BLUE HALO RENDERING HIGHLIGHT AURA)
		TexturedVertex * vert=&LATERDRAWHALO[(i<<2)];

		if (vert[2].color == 0)
		{
			GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);									
			vert[2].color =0xFF000000;
			EERIEDRAWPRIM(Renderer::TriangleFan, vert, 4);
			GRenderer->SetBlendFunc(Renderer::BlendSrcColor, Renderer::BlendOne);	
		}
		else EERIEDRAWPRIM(Renderer::TriangleFan, vert, 4);
	}

		 HALOCUR = 0; 
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);			
}

	GRenderer->SetCulling(Renderer::CullCCW);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);	
	GRenderer->SetRenderState(Renderer::DepthWrite, true);

#ifdef BUILD_EDITOR
	if (EDITION==EDITION_LIGHTS)
		ARXDRAW_DrawAllLights(x0,z0,x1,z1);
#endif

}

