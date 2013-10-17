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
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#include "scene/Scene.h"

#include <cstdio>

#include "ai/Paths.h"

#include "animation/AnimationRender.h"

#include "core/Application.h"
#include "core/GameTime.h"
#include "core/Core.h"

#include "game/EntityManager.h"
#include "game/Inventory.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "game/spell/FlyingEye.h"

#include "gui/Interface.h"

#include "graphics/Draw.h"
#include "graphics/DrawLine.h"
#include "graphics/GraphicsModes.h"
#include "graphics/Math.h"
#include "graphics/VertexBuffer.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/DrawEffects.h"
#include "graphics/effects/Halo.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/texture/TextureStage.h"

#include "input/Input.h"

#include "io/log/Logger.h"

#include "scene/Light.h"
#include "scene/Interactive.h"

using std::vector;

extern TextureContainer *enviro;
extern long ZMAPMODE;
extern Color ulBKGColor;

EERIE_PORTAL_DATA * portals = NULL;

static float WATEREFFECT = 0.f;

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
		delete[] indices;
	}
	
} dynamicVertices;

}

EERIE_FRUSTRUM_PLANE efpPlaneNear;

static vector<EERIEPOLY*> vPolyWater;
static vector<EERIEPOLY*> vPolyLava;

void PopAllTriangleListTransparency();

std::vector<PORTAL_ROOM_DRAW> RoomDraw;
std::vector<long> RoomDrawList;

//*************************************************************************************
//*************************************************************************************
void ApplyWaterFXToVertex(Vec3f * odtv, TexturedVertex * dtv, float power)
{
	power = power * 0.05f;
	dtv->uv.x += EEsin(WATEREFFECT + odtv->x) * power;
	dtv->uv.y += EEcos(WATEREFFECT + odtv->z) * power;
}

static void ApplyLavaGlowToVertex(Vec3f * odtv,TexturedVertex * dtv, float power) {
	float f;
	long lr, lg, lb;
	power = 1.f - EEsin(WATEREFFECT + odtv->x + odtv->z) * 0.05f * power;
	f = ((dtv->color >> 16) & 255) * power;
	lr = clipByte(f);

	f = ((dtv->color >> 8) & 255) * power;
	lg = clipByte(f);

	f = ((dtv->color) & 255) * power;
	lb = clipByte(f);

	dtv->color = (0xFF000000L | (lr << 16) | (lg << 8) | (lb));
}

void ManageWater_VertexBuffer(EERIEPOLY * ep, const long to, const unsigned long tim, SMY_VERTEX * _pVertex) {
	
	for(long k = 0; k < to; k++) {
		ep->tv[k].uv = ep->v[k].uv;
		
		ApplyWaterFXToVertex(&ep->v[k].p, &ep->tv[k], 0.35f);
			
		if(ep->type & POLY_FALL) {
			ep->tv[k].uv.y -= (float)(tim) * (1.f/1000);
		}
		
		_pVertex[ep->uslInd[k]].uv = ep->tv[k].uv;
	}
}

void ManageLava_VertexBuffer(EERIEPOLY * ep, const long to, const unsigned long tim, SMY_VERTEX * _pVertex) {
	
	for(long k = 0; k < to; k++) {
		ep->tv[k].uv = ep->v[k].uv;
		
		ApplyWaterFXToVertex(&ep->v[k].p, &ep->tv[k], 0.35f); //0.25f
		ApplyLavaGlowToVertex(&ep->v[k].p, &ep->tv[k], 0.6f);
			
		if(ep->type & POLY_FALL) {
			ep->tv[k].uv.y -= (float)(tim) * (1.f/12000);
		}
		
		_pVertex[ep->uslInd[k]].uv = ep->tv[k].uv;
	}
}

void EERIERTPPoly2(EERIEPOLY *ep)
{
	EE_RTP(&ep->v[0],&ep->tv[0]);
	EE_RTP(&ep->v[1],&ep->tv[1]);
	EE_RTP(&ep->v[2],&ep->tv[2]);

	if(ep->type & POLY_QUAD)
		EE_RTP(&ep->v[3],&ep->tv[3]);
	else
		ep->tv[3].p.z=1.f;
}

bool IsSphereInFrustrum(const Vec3f & point, const EERIE_FRUSTRUM & frustrum, float radius = 0.f);
bool FrustrumsClipSphere(const EERIE_FRUSTRUM_DATA & frustrums, const EERIE_SPHERE & sphere)
{
	float dists = efpPlaneNear.getDist(sphere.origin);

	if(dists + sphere.radius > 0) {
		for(long i = 0; i < frustrums.nb_frustrums; i++) {
			if(IsSphereInFrustrum(sphere.origin, frustrums.frustrums[i], sphere.radius))
				return false;
		}
	}

	return true;
}

bool VisibleSphere(float x, float y, float z, float radius) {
	
	Vec3f pos(x, y, z);
	if(distSqr(pos, ACTIVECAM->orgTrans.pos) > square(ACTIVECAM->cdepth*0.5f + radius))
		return false;

	long room_num = ARX_PORTALS_GetRoomNumForPosition(&pos);

	if (room_num>=0)
	{
		EERIE_SPHERE sphere;
		sphere.origin = pos;
		sphere.radius = radius;
							
		EERIE_FRUSTRUM_DATA & frustrums = RoomDraw[room_num].frustrum;

		if (FrustrumsClipSphere(frustrums, sphere))
			return false;
	}

	return true;
}

bool IsBBoxInFrustrum(const EERIE_3D_BBOX & bbox, const EERIE_FRUSTRUM & frustrum) {
	
	Vec3f point = bbox.min;
	if(IsSphereInFrustrum(point, frustrum)) {
		return true;
	}
	
	point = Vec3f(bbox.max.x, bbox.min.y, bbox.min.z);
	if(IsSphereInFrustrum(point, frustrum)) {
		return true;
	}
	
	point = Vec3f(bbox.max.x, bbox.max.y, bbox.min.z);
	if(IsSphereInFrustrum(point, frustrum)) {
		return true;
	}
	
	point = Vec3f(bbox.min.x, bbox.max.y, bbox.min.z);
	if(IsSphereInFrustrum(point, frustrum)) {
		return true;
	}
	
	point = Vec3f(bbox.min.x, bbox.min.y, bbox.max.z);
	if(IsSphereInFrustrum(point, frustrum)) {
		return true;
	}
	
	point = Vec3f(bbox.max.x, bbox.min.y, bbox.max.z);
	if(IsSphereInFrustrum(point, frustrum)) {
		return true;
	}
	
	point = bbox.max;
	if(IsSphereInFrustrum(point, frustrum)) {
		return true;
	}
	
	point = Vec3f(bbox.min.x, bbox.max.y, bbox.max.z);
	if(IsSphereInFrustrum(point, frustrum)) {
		return true;
	}
	
	return	false;
}

bool FrustrumsClipBBox3D(const EERIE_FRUSTRUM_DATA & frustrums, const EERIE_3D_BBOX & bbox)
{
	for(long i = 0; i < frustrums.nb_frustrums; i++)
	{
		if(IsBBoxInFrustrum(bbox, frustrums.frustrums[i]))
			return false;
	}

	return false;
}

bool ARX_SCENE_PORTAL_Basic_ClipIO(Entity * io) {
	arx_assert(io);
	if(io == entities.player() || (io->ioflags & IO_FORCEDRAW)) {
		return false;
	}
	
	if(!(USE_PORTALS && portals))
		return false;

	Vec3f posi = io->pos;
	posi.y -= 20.f;

	if(io->room_flags & 1)
		UpdateIORoom(io);

	long room_num = io->room;

	if(room_num == -1) {
		posi.y = io->pos.y-120;
		room_num=ARX_PORTALS_GetRoomNumForPosition(&posi);
	}

	if(room_num >= 0 && size_t(room_num) < RoomDraw.size() && RoomDraw[room_num].count) {
		float yOffset = 0.f;
		float radius = 0.f;
		if(io->ioflags & IO_ITEM) {
			yOffset = -40.f;
			if(io->ioflags & IO_MOVABLE)
				radius = 160.f;
			else
				radius = 75.f;
		} else if(io->ioflags & IO_FIX) {
			yOffset = -60.f;
			radius = 340.f;
		} else if(io->ioflags & IO_NPC) {
			yOffset = -120.f;
			radius = 120.f;
		}

		EERIE_SPHERE sphere;

		if(radius != 0.f) {
			sphere.origin.x=io->pos.x;
			sphere.origin.y=io->pos.y + yOffset;
			sphere.origin.z=io->pos.z;
			sphere.radius=radius;
		}

		EERIE_FRUSTRUM_DATA & frustrums = RoomDraw[room_num].frustrum;

		if (FrustrumsClipSphere(frustrums, sphere)) {
			io->bbox2D.min = Vec2f(-1.f, -1.f);
			io->bbox2D.max = Vec2f(-1.f, -1.f);
			return true;
		}
	}
	return false;
}

// USAGE/FUNCTION
//   io can be NULL if io is valid io->bbox3D contains 3D world-bbox
//   bboxmin & bboxmax ARE in fact 2D-screen BBOXes using only (x,y).
// RETURN:
//   return true if IO cannot be seen, false if visible
// TODO:
//   Implement all Portal Methods
//   Return a reduced clipbox which can be used for polys clipping in the case of partial visibility
bool ARX_SCENE_PORTAL_ClipIO(Entity * io, const Vec3f & position) {
	
	if(io==entities.player())
		return false;

	if(io && (io->ioflags & IO_FORCEDRAW))
		return false;

	if(USE_PORTALS && portals) {
		Vec3f posi;
		posi.x=position.x;
		posi.y=position.y-60; //20
		posi.z=position.z;
		long room_num;

		if(io) {
			if(io->room_flags & 1)
				UpdateIORoom(io);

			room_num = io->room;//
		} else {
			room_num = ARX_PORTALS_GetRoomNumForPosition(&posi);
		}

		if(room_num == -1) {
			posi.y = position.y - 120;
			room_num = ARX_PORTALS_GetRoomNumForPosition(&posi);
		}

		if(room_num >= 0 && size_t(room_num) < RoomDraw.size()) {
			if(RoomDraw[room_num].count == 0) {
				if(io) {
					io->bbox2D.min = Vec2f(-1.f, -1.f);
					io->bbox2D.max = Vec2f(-1.f, -1.f);
				}
				return true;
			}

			if(io) {
				EERIE_SPHERE sphere;
				sphere.origin = (io->bbox3D.min + io->bbox3D.max) * .5f;
				sphere.radius = dist(sphere.origin, io->bbox3D.min) + 10.f;

				EERIE_FRUSTRUM_DATA & frustrums = RoomDraw[room_num].frustrum;

				if(FrustrumsClipSphere(frustrums, sphere) ||
				   FrustrumsClipBBox3D(frustrums, io->bbox3D)
				) {
					io->bbox2D.min = Vec2f(-1.f, -1.f);
					io->bbox2D.max = Vec2f(-1.f, -1.f);
					return true;
				}
			}
		}
	}

	return false;
}

long ARX_PORTALS_GetRoomNumForPosition2(Vec3f * pos,long flag,float * height)
{
	EERIEPOLY * ep; 

	if(flag & 1) {
		ep=CheckInPoly(pos->x,pos->y-150.f,pos->z);

		if (!ep)
			ep=CheckInPoly(pos->x,pos->y-1.f,pos->z);
	} else {
		ep=CheckInPoly(pos->x,pos->y,pos->z);
	}

	if(ep && ep->room>-1) {
		if(height)
			*height=ep->center.y;

		return ep->room;
	}

	// Security... ?
	ep=GetMinPoly(pos->x,pos->y,pos->z);

	if(ep && ep->room > -1) {
		if(height)
			*height=ep->center.y;

		return ep->room;
	} else if( !(flag & 1) ) {
		ep=CheckInPoly(pos->x,pos->y,pos->z);

		if(ep && ep->room > -1) {
			if(height)
				*height=ep->center.y;

			return ep->room;
		}
	}

	if(flag & 2) {
		float off=20.f;
		ep=CheckInPoly(pos->x-off,pos->y-off,pos->z);

		if(ep && ep->room > -1) {
			if(height)
				*height=ep->center.y;

			return ep->room;
		}

		ep=CheckInPoly(pos->x-off,pos->y-20,pos->z-off);

		if(ep && ep->room > -1) {
			if(height)
				*height=ep->center.y;

			return ep->room;
		}

		ep=CheckInPoly(pos->x-off,pos->y-20,pos->z+off);

		if(ep && ep->room > -1) {
			if(height)
				*height=ep->center.y;

			return ep->room;
		}

		ep=CheckInPoly(pos->x+off,pos->y-20,pos->z);

		if(ep && ep->room>-1) {
			if(height)
				*height=ep->center.y;

			return ep->room;
		}

		ep=CheckInPoly(pos->x+off,pos->y-20,pos->z+off);

		if(ep && ep->room > -1) {
			if(height)
				*height=ep->center.y;

			return ep->room;
		}

		ep=CheckInPoly(pos->x+off,pos->y-20,pos->z-off);

		if(ep && ep->room > -1) {
			if(height)
				*height=ep->center.y;

			return ep->room;
		}
	}

	return -1;
}
long ARX_PORTALS_GetRoomNumForCamera(float * height)
{
	EERIEPOLY * ep; 
	ep = CheckInPoly(ACTIVECAM->orgTrans.pos.x,ACTIVECAM->orgTrans.pos.y,ACTIVECAM->orgTrans.pos.z);

	if(ep && ep->room > -1) {
		if(height)
			*height=ep->center.y;

		return ep->room;
	}

	ep = GetMinPoly(ACTIVECAM->orgTrans.pos.x,ACTIVECAM->orgTrans.pos.y,ACTIVECAM->orgTrans.pos.z);

	if(ep && ep->room > -1) {
		if(height)
			*height=ep->center.y;

		return ep->room;
	}

	float dist=0.f;

	while(dist<=20.f) {
		float vvv=radians(ACTIVECAM->angle.b);
		ep=CheckInPoly(	ACTIVECAM->orgTrans.pos.x+EEsin(vvv)*dist,
								ACTIVECAM->orgTrans.pos.y,
								ACTIVECAM->orgTrans.pos.z-EEcos(vvv)*dist);

		if(ep && ep->room > -1) {
			if(height)
				*height=ep->center.y;

			return ep->room;
		}

		dist += 5.f;
	}

	return -1;
}

// flag==1 for player
long ARX_PORTALS_GetRoomNumForPosition(Vec3f * pos,long flag)
{
	long num;
	float height;

	if(flag & 1)
		num=ARX_PORTALS_GetRoomNumForCamera(&height);
	else
		num=ARX_PORTALS_GetRoomNumForPosition2(pos,flag,&height);

	if(num > -1) {
		long nearest = -1;
		float nearest_dist = 99999.f;

		for(long n = 0; n < portals->nb_rooms; n++) { //TODO off by one ? (portals->nb_rooms + 1)
			for(long lll = 0; lll < portals->room[n].nb_portals; lll++) {
				EERIE_PORTALS *po = &portals->portals[portals->room[n].portals[lll]];
				EERIEPOLY *epp = &po->poly;

				if(PointIn2DPolyXZ(epp, pos->x, pos->z)) {
					float yy;

					if(GetTruePolyY(epp,pos,&yy)) {
						if(height > yy) {
							if(yy >= pos->y && yy-pos->y < nearest_dist) {
								if(epp->norm.y>0)
									nearest = po->room_2;
								else
									nearest = po->room_1;

								nearest_dist = yy - pos->y;
							}
						}
					}
				}
			}
		}

		if(nearest>-1)
			num = nearest;
	}
	
	return num;
}

void ARX_PORTALS_Frustrum_ClearIndexCount(long room_num) {

	EERIE_ROOM_DATA & room = portals->room[room_num];

	int iNbTex = room.usNbTextures;
	TextureContainer **ppTexCurr = room.ppTextureContainer;

	while(iNbTex--) {

		TextureContainer * pTexCurr = *ppTexCurr;
		GRenderer->SetTexture(0, pTexCurr);

		SMY_ARXMAT & roomMat = pTexCurr->tMatRoom[room_num];

		roomMat.count[SMY_ARXMAT::Opaque] = 0;
		roomMat.count[SMY_ARXMAT::Blended] = 0;
		roomMat.count[SMY_ARXMAT::Multiplicative] = 0;
		roomMat.count[SMY_ARXMAT::Additive] = 0;
		roomMat.count[SMY_ARXMAT::Subtractive] = 0;

		ppTexCurr++;
	}
}

			
void ARX_PORTALS_InitDrawnRooms()
{
	arx_assert(portals);

	for(size_t i = 0; i < portals->portals.size(); i++) {
		EERIE_PORTALS *ep = &portals->portals[i];

		ep->useportal = 0;
	}

	for(long i = 0; i < portals->roomsize(); i++) {
		ARX_PORTALS_Frustrum_ClearIndexCount(i);
	}

	RoomDraw.resize(portals->roomsize());

	for(size_t i = 0; i < RoomDraw.size(); i++) {
		RoomDraw[i].count=0;
		RoomDraw[i].flags=0;
		RoomDraw[i].frustrum.nb_frustrums=0;
	}

	RoomDrawList.clear();

	vPolyWater.clear();
	vPolyLava.clear();

	if(pDynamicVertexBuffer) {
		pDynamicVertexBuffer->vb->setData(NULL, 0, 0, DiscardBuffer);
		dynamicVertices.reset();
	}
}

bool IsSphereInFrustrum(const Vec3f & point, const EERIE_FRUSTRUM & frustrum, float radius)
{
	float dists[4];
	dists[0]=frustrum.plane[0].getDist(point);
	dists[1]=frustrum.plane[1].getDist(point);
	dists[2]=frustrum.plane[2].getDist(point);
	dists[3]=frustrum.plane[3].getDist(point);

	if (	(dists[0]+radius>0)
		&&	(dists[1]+radius>0)
		&&	(dists[2]+radius>0)
		&&	(dists[3]+radius>0) )
		return true;

	return false;
}

bool FrustrumsClipPoly(const EERIE_FRUSTRUM_DATA & frustrums, const EERIEPOLY & ep){
	for(long i=0; i<frustrums.nb_frustrums; i++) {
		if(IsSphereInFrustrum(ep.center, frustrums.frustrums[i], ep.v[0].rhw))
			return false;
	}

	return true;
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
	float Ax, Ay, Az, Bx, By, Bz, epnlen;
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

void CreateFrustrum(EERIE_FRUSTRUM *frustrum, Vec3f * pos, EERIEPOLY *ep, bool cull) {
	if(cull) {
		CreatePlane(frustrum, 0, pos, &ep->v[0].p, &ep->v[1].p);
		CreatePlane(frustrum, 1, pos, &ep->v[3].p, &ep->v[2].p);
		CreatePlane(frustrum, 2, pos, &ep->v[1].p, &ep->v[3].p);
		CreatePlane(frustrum, 3, pos, &ep->v[2].p, &ep->v[0].p);
	} else {
		CreatePlane(frustrum, 0, pos, &ep->v[1].p, &ep->v[0].p);
		CreatePlane(frustrum, 1, pos, &ep->v[2].p, &ep->v[3].p);
		CreatePlane(frustrum, 2, pos, &ep->v[3].p, &ep->v[1].p);
		CreatePlane(frustrum, 3, pos, &ep->v[0].p, &ep->v[2].p);
	}
}



void CreateScreenFrustrum(EERIE_FRUSTRUM * frustrum) {
		
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

void RoomDrawRelease() {
	RoomDrawList.resize(0);
	RoomDraw.resize(0);
}

void RoomFrustrumAdd(long num, const EERIE_FRUSTRUM & fr)
{
	if(RoomDraw[num].frustrum.nb_frustrums < MAX_FRUSTRUMS - 1) {
		RoomDraw[num].frustrum.frustrums[RoomDraw[num].frustrum.nb_frustrums] = fr;
		RoomDraw[num].frustrum.nb_frustrums++;
	}	
}

static void RenderWaterBatch() {
	
	if(!dynamicVertices.nbindices) {
		return;
	}
	
	GRenderer->GetTextureStage(1)->setColorOp(TextureStage::OpModulate4X, TextureStage::ArgTexture, TextureStage::ArgCurrent);
	GRenderer->GetTextureStage(1)->disableAlpha();
	
	GRenderer->GetTextureStage(2)->setColorOp(TextureStage::OpModulate, TextureStage::ArgTexture, TextureStage::ArgCurrent);
	GRenderer->GetTextureStage(2)->disableAlpha();
	
	dynamicVertices.draw(Renderer::TriangleList);
	
	GRenderer->GetTextureStage(1)->disableColor();
	GRenderer->GetTextureStage(2)->disableColor();
	
}

float FluidTextureDisplacement(bool calcSin, const TexturedVertex& v, float time, float divVar1, float divVar2, 
                               float divVar3, float divVar4, float addVar1 = 0, float addVar2 = 0, float sign = 1) {
	if(calcSin) {
		return (v.p.x + addVar1)*(1.f/divVar1) + sign * (sin((v.p.x + addVar2)*(1.f/divVar2) + time * (1.f/divVar3))) * (1.f/divVar4);
	}
	return (v.p.z + addVar1)*(1.f/divVar1) + sign * (cos((v.p.z + addVar2)*(1.f/divVar2) + time * (1.f/divVar3))) * (1.f/divVar4);
}

void CalculateWaterDisplacement(float& fTu, float& fTv, EERIEPOLY* ep, float time, int vertIndex, int step) {
	switch(step) {
	case(0):fTu = FluidTextureDisplacement(true, ep->v[vertIndex], time, 1000, 200, 1000, 32); 
			fTv = FluidTextureDisplacement(false, ep->v[vertIndex], time, 1000, 200, 1000, 32); 
			break;
	case(1):fTu = FluidTextureDisplacement(true, ep->v[vertIndex], time, 1000, 200, 1000, 28, 30.f, 30); 
			fTv = FluidTextureDisplacement(false, ep->v[vertIndex], time, 1000, 200, 1000, 28, 30.f, 30, -1.0); 
			break;
	case(2):fTu = FluidTextureDisplacement(true, ep->v[vertIndex], time, 1000, 200, 1000, 40, 60.f, 60, -1.0);
			fTv = FluidTextureDisplacement(false, ep->v[vertIndex], time, 1000, 200, 1000, 40, 60.f, 60, -1.0);
			break;
	default:break;
	}
}

void CalculateLavaDisplacement(float& fTu, float& fTv, EERIEPOLY* ep, float time, int vertIndex, int step) {
	switch(step) {
	case(0):fTu = FluidTextureDisplacement(true, ep->v[vertIndex], time, 1000, 200, 2000, 20); 
			fTv = FluidTextureDisplacement(false, ep->v[vertIndex], time, 1000, 200, 2000, 20); 
			break;
	case(1):fTu = FluidTextureDisplacement(true, ep->v[vertIndex], time, 1000, 100, 2000, 10); 
			fTv = FluidTextureDisplacement(false, ep->v[vertIndex], time, 1000, 100, 2000, 10);
			break;
	case(2):fTu = FluidTextureDisplacement(true, ep->v[vertIndex], time, 600, 160, 2000, 11); 
			fTv = FluidTextureDisplacement(false, ep->v[vertIndex], time, 600, 160, 2000, 11);
			break;
	default:break;
	}
}

const int FTVU_STEP_COUNT = 3; //For fTv and fTu calculations

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
	
	float time = arxtime.get_frame_time();

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
		
		float fTu;
		float fTv;

		for(int j = 0; j < iNbVertex; ++j) {
			pVertex->p.x = ep->v[j].p.x;
			pVertex->p.y = -ep->v[j].p.y;
			pVertex->p.z = ep->v[j].p.z;
			pVertex->color = 0xFF505050;

			for(int i = 0; i < FTVU_STEP_COUNT; ++i) {
				CalculateWaterDisplacement(fTu, fTv, ep, time, j, i);

				if(ep->type & POLY_FALL) {
					fTv += time * (1.f/4000);
				}
				pVertex->uv[i].x = fTu;
				pVertex->uv[i].y = fTv;
			}	
			pVertex++;

			if(j == 2){						
				*indices++ = iNbIndice++; 
				*indices++ = iNbIndice++; 
				*indices++ = iNbIndice++; 
				dynamicVertices.nbindices += 3;
			}
		}
		if(iNbVertex == 4) {
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
	GRenderer->GetTextureStage(0)->setColorOp(TextureStage::OpModulate2X, TextureStage::ArgTexture, TextureStage::ArgDiffuse);
	
	if(!dynamicVertices.nbindices) {
		return;
	}
	
	GRenderer->GetTextureStage(1)->setColorOp(TextureStage::OpModulate4X, TextureStage::ArgTexture, TextureStage::ArgCurrent);
	GRenderer->GetTextureStage(1)->disableAlpha();
	
	GRenderer->GetTextureStage(2)->setColorOp(TextureStage::OpModulate, TextureStage::ArgTexture, TextureStage::ArgCurrent);
	GRenderer->GetTextureStage(2)->disableAlpha();
	
	dynamicVertices.draw(Renderer::TriangleList);
	
	GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
	GRenderer->GetTextureStage(0)->setColorOp(TextureStage::OpModulate);
	
	dynamicVertices.draw(Renderer::TriangleList);
	
	GRenderer->GetTextureStage(1)->disableColor();
	GRenderer->GetTextureStage(2)->disableColor();
	
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
	
	float time = arxtime.get_frame_time();

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
		
		float fTu;
		float fTv;

		for(int j = 0; j < iNbVertex; ++j) {
			pVertex->p.x = ep->v[j].p.x;
			pVertex->p.y = -ep->v[j].p.y;
			pVertex->p.z = ep->v[j].p.z;
			pVertex->color = 0xFF666666;
			for(int i = 0; i < FTVU_STEP_COUNT; ++i) {
				CalculateLavaDisplacement(fTu, fTv, ep, time, j, i);
				pVertex->uv[i].x = fTu;
				pVertex->uv[i].y = fTv;
			}
			pVertex++;
			if(j == 2){	
				*indices++ = iNbIndice++; 
				*indices++ = iNbIndice++; 
				*indices++ = iNbIndice++; 
				dynamicVertices.nbindices += 3;
			}
		}								
		if(iNbVertex == 4) {			
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

void ARX_PORTALS_Frustrum_RenderRoomTCullSoft(long room_num, const EERIE_FRUSTRUM_DATA & frustrums, long tim) {
	if(!RoomDraw[room_num].count)
		return;

	EERIE_ROOM_DATA & room = portals->room[room_num];

	if(!room.pVertexBuffer) {
		// No need to spam this for every frame as there will already be an
		// earlier warning
		LogDebug("no vertex data for room " << room_num);
		return;
	}

	SMY_VERTEX * pMyVertex = room.pVertexBuffer->lock(NoOverwrite);

	unsigned short *pIndices=room.pussIndice;

	EP_DATA *pEPDATA = &room.epdata[0];

	for(long lll=0; lll<room.nb_polys; lll++, pEPDATA++) {
		FAST_BKG_DATA *feg = &ACTIVEBKG->fastdata[pEPDATA->px][pEPDATA->py];

		if(!feg->treat) {
			short ix = std::max(pEPDATA->px - 1, 0);
			short ax = std::min(pEPDATA->px + 1, ACTIVEBKG->Xsize - 1);
			short iz = std::max(pEPDATA->py - 1, 0);
			short az = std::min(pEPDATA->py + 1, ACTIVEBKG->Zsize - 1);

			for(short nz=iz; nz<=az; nz++)
			for(short nx=ix; nx<=ax; nx++) {
				FAST_BKG_DATA * feg2 = &ACTIVEBKG->fastdata[nx][nz];

				if(!feg2->treat) {
					feg2->treat=1;
					ComputeTileLights(nx, nz);
				}
			}
		}

		EERIEPOLY *ep = &feg->polydata[pEPDATA->idx];

		if(!ep->tex) {
			continue;
		}

		if(ep->type & (POLY_IGNORE | POLY_NODRAW| POLY_HIDE)) {
			continue;
		}

		if(FrustrumsClipPoly(frustrums, *ep)) {
			continue;
		}

		//Clipp ZNear + Distance pour les ZMapps!!!
		float fDist = efpPlaneNear.getDist(ep->center);

		if(ep->v[0].rhw < -fDist) {
			continue;
		}

		fDist -= ep->v[0].rhw;

		Vec3f nrm = ep->v[2].p - ACTIVECAM->orgTrans.pos;
		int to = (ep->type & POLY_QUAD) ? 4 : 3;

		if(!(ep->type & POLY_DOUBLESIDED) && dot(ep->norm , nrm) > 0.f) {
			if(to == 3 || dot(ep->norm2 , nrm) > 0.f) {
				continue;
			}
		}

		SMY_ARXMAT::TransparencyType transparencyType;

		if(ep->type & POLY_TRANS) {
			if(ep->transval >= 2.f) {
				transparencyType = SMY_ARXMAT::Multiplicative;
			} else if(ep->transval >= 1.f) {
				transparencyType = SMY_ARXMAT::Additive;
			} else if(ep->transval > 0.f) {
				transparencyType = SMY_ARXMAT::Blended;
			} else {
				transparencyType = SMY_ARXMAT::Subtractive;
			}
		} else {
			transparencyType = SMY_ARXMAT::Opaque;

			if(ZMAPMODE) {
				if(fDist < 200 && ep->tex->TextureRefinement) {
					ep->tex->TextureRefinement->vPolyZMap.push_back(ep);
				}
			}
		}

		SMY_ARXMAT & roomMat = ep->tex->tMatRoom[room_num];

		unsigned short * pIndicesCurr = pIndices + roomMat.offset[transparencyType] + roomMat.count[transparencyType];
		unsigned long * pNumIndices = &roomMat.count[transparencyType];

		*pIndicesCurr++ = ep->uslInd[0];
		*pIndicesCurr++ = ep->uslInd[1];
		*pIndicesCurr++ = ep->uslInd[2];
		*pNumIndices += 3;

		if(to == 4) {
			*pIndicesCurr++ = ep->uslInd[3];
			*pIndicesCurr++ = ep->uslInd[2];
			*pIndicesCurr++ = ep->uslInd[1];
			*pNumIndices += 3;
		}

		SMY_VERTEX * pMyVertexCurr = &pMyVertex[roomMat.uslStartVertex];

		if(!Project.improve) { // Normal View...
			if(ep->type & POLY_GLOW) {
				pMyVertexCurr[ep->uslInd[0]].color = 0xFFFFFFFF;
				pMyVertexCurr[ep->uslInd[1]].color = 0xFFFFFFFF;
				pMyVertexCurr[ep->uslInd[2]].color = 0xFFFFFFFF;

				if(to == 4) {
					pMyVertexCurr[ep->uslInd[3]].color = 0xFFFFFFFF;
				}
			} else {
				if(!(ep->type & POLY_TRANS)) {
					ApplyTileLights(ep, pEPDATA->px, pEPDATA->py);

					pMyVertexCurr[ep->uslInd[0]].color = ep->tv[0].color;
					pMyVertexCurr[ep->uslInd[1]].color = ep->tv[1].color;
					pMyVertexCurr[ep->uslInd[2]].color = ep->tv[2].color;

					if(to&4) {
						pMyVertexCurr[ep->uslInd[3]].color = ep->tv[3].color;
					}
				}

				if(ep->type & POLY_LAVA) {
					ManageLava_VertexBuffer(ep, to, tim, pMyVertexCurr);
					vPolyLava.push_back(ep);
				} else if(ep->type & POLY_WATER) {
					ManageWater_VertexBuffer(ep, to, tim, pMyVertexCurr);
					vPolyWater.push_back(ep);
				}
			}

			if((ViewMode & VIEWMODE_WIRE) && EERIERTPPoly(ep))
				EERIEPOLY_DrawWired(ep);

		} else { // Improve Vision Activated
			if(!(ep->type & POLY_TRANS)) {
				if(!EERIERTPPoly(ep)) { // RotTransProject Vertices
					continue;
				}

				ApplyTileLights(ep, pEPDATA->px, pEPDATA->py);

				for(int k = 0; k < to; k++) {
					long lr=(ep->tv[k].color>>16) & 255;
					float ffr=(float)(lr);

					float dd = ep->tv[k].rhw;

					dd = clamp(dd, 0.f, 1.f);

					Vec3f & norm = ep->nrml[k];

					float fb=((1.f-dd)*6.f + (EEfabs(norm.x) + EEfabs(norm.y))) * 0.125f;
					float fr=((.6f-dd)*6.f + (EEfabs(norm.z) + EEfabs(norm.y))) * 0.125f;

					if(fr < 0.f)
						fr = 0.f;
					else
						fr = std::max(ffr, fr * 255.f);

					fr=std::min(fr,255.f);
					fb*=255.f;
					fb=std::min(fb,255.f);
					u8 lfr = fr;
					u8 lfb = fb;
					u8 lfg = 0x1E;

					ep->tv[k].color = (0xff000000L | (lfr << 16) | (lfg << 8) | (lfb));
				}

				pMyVertexCurr[ep->uslInd[0]].color = ep->tv[0].color;
				pMyVertexCurr[ep->uslInd[1]].color = ep->tv[1].color;
				pMyVertexCurr[ep->uslInd[2]].color = ep->tv[2].color;

				if(to == 4) {
					pMyVertexCurr[ep->uslInd[3]].color = ep->tv[3].color;
				}
			}
		}
	}

	room.pVertexBuffer->unlock();
}


void ARX_PORTALS_Frustrum_RenderRoomTCullSoftRender(long room_num) {

	EERIE_ROOM_DATA & room = portals->room[room_num];

	//render opaque
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetAlphaFunc(Renderer::CmpGreater, .5f);

	int iNbTex = room.usNbTextures;
	TextureContainer **ppTexCurr = room.ppTextureContainer;

	while(iNbTex--) {
		TextureContainer *pTexCurr = *ppTexCurr;

		SMY_ARXMAT & roomMat = pTexCurr->tMatRoom[room_num];

		if(ViewMode & VIEWMODE_FLAT)
			GRenderer->ResetTexture(0);
		else
			GRenderer->SetTexture(0, pTexCurr);

		if(roomMat.count[SMY_ARXMAT::Opaque]) {
			if (pTexCurr->userflags & POLY_METAL)
				GRenderer->GetTextureStage(0)->setColorOp(TextureStage::OpModulate2X);
			else
				GRenderer->GetTextureStage(0)->setColorOp(TextureStage::OpModulate);

			

			room.pVertexBuffer->drawIndexed(
				Renderer::TriangleList,
				roomMat.uslNbVertex,
				roomMat.uslStartVertex,
				&room.pussIndice[roomMat.offset[SMY_ARXMAT::Opaque]],
				roomMat.count[SMY_ARXMAT::Opaque]);

			EERIEDrawnPolys += roomMat.count[SMY_ARXMAT::Opaque];
		}

		ppTexCurr++;
	}

	//////////////////////////////
	// ZMapp
	GRenderer->GetTextureStage(0)->setColorOp(TextureStage::OpModulate);

	GRenderer->SetAlphaFunc(Renderer::CmpNotEqual, 0.f);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);

	iNbTex=room.usNbTextures;
	ppTexCurr=room.ppTextureContainer;

	// For each tex in portals->room[room_num]
	while(iNbTex--) {
		TextureContainer * pTexCurr	= *ppTexCurr;

		if(pTexCurr->TextureRefinement && pTexCurr->TextureRefinement->vPolyZMap.size()) {

			GRenderer->SetTexture(0, pTexCurr->TextureRefinement);

			dynamicVertices.lock();
			unsigned short *pussInd = dynamicVertices.indices;
			unsigned short iNbIndice = 0;

			vector<EERIEPOLY *>::iterator it = pTexCurr->TextureRefinement->vPolyZMap.begin();

			for(;it != pTexCurr->TextureRefinement->vPolyZMap.end(); ++it) {
				EERIEPOLY * ep = *it;

				unsigned short iNbVertex = (ep->type & POLY_QUAD) ? 4 : 3;
				SMY_VERTEX3 *pVertex = dynamicVertices.append(iNbVertex);

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

				// PRECALCUL
				float tu[4];
				float tv[4];
				float _fTransp[4];

				bool nrm = EEfabs(ep->nrml[0].y) >= 0.9f || EEfabs(ep->nrml[1].y) >= 0.9f || EEfabs(ep->nrml[2].y) >= 0.9f;

				for(int nu = 0; nu < iNbVertex; nu++) {
					if(nrm) {
						tu[nu] = ep->v[nu].p.x * (1.0f/50);
						tv[nu] = ep->v[nu].p.z * (1.0f/50);
					} else {
						tu[nu] = ep->v[nu].uv.x * 4.f;
						tv[nu] = ep->v[nu].uv.y * 4.f;
					}

					float t = max(10.f, fdist(ACTIVECAM->orgTrans.pos, ep->v[nu].p) - 80.f);

					_fTransp[nu] = (150.f - t) * 0.006666666f;

					if(_fTransp[nu] < 0.f)
						_fTransp[nu] = 0.f;
					// t cannot be greater than 1.f (b should be negative for that)
				}

				// FILL DATA
				for(int idx = 0; idx < iNbVertex; ++idx) {
					pVertex->p.x     =  ep->v[idx].p.x;
					pVertex->p.y     = -ep->v[idx].p.y;
					pVertex->p.z     =  ep->v[idx].p.z;
					pVertex->color   = Color::gray(_fTransp[idx]).toBGR();
					pVertex->uv[0].x = tu[idx];
					pVertex->uv[0].y = tv[idx];
					pVertex++;

					*pussInd++ = iNbIndice++;
					dynamicVertices.nbindices++;
				}

				if(iNbVertex == 4) {
					*pussInd++ = iNbIndice-2;
					*pussInd++ = iNbIndice-3;
					dynamicVertices.nbindices += 2;
				}
			}

			// CLEAR CURRENT ZMAP
			pTexCurr->TextureRefinement->vPolyZMap.clear();

			dynamicVertices.unlock();
			if(dynamicVertices.nbindices) {
				dynamicVertices.draw(Renderer::TriangleList);
			}
			dynamicVertices.done();

		}

		ppTexCurr++;
	}

	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}

//-----------------------------------------------------------------------------

static const SMY_ARXMAT::TransparencyType transRenderOrder[] = {
	SMY_ARXMAT::Blended,
	SMY_ARXMAT::Multiplicative,
	SMY_ARXMAT::Additive,
	SMY_ARXMAT::Subtractive
};


void ARX_PORTALS_Frustrum_RenderRoom_TransparencyTSoftCull(long room_num)
{
	//render transparency
	EERIE_ROOM_DATA & room = portals->room[room_num];

	int iNbTex = room.usNbTextures;
	TextureContainer **ppTexCurr = room.ppTextureContainer;

	while(iNbTex--) {

		TextureContainer * pTexCurr = *ppTexCurr;
		GRenderer->SetTexture(0, pTexCurr);

		SMY_ARXMAT & roomMat = pTexCurr->tMatRoom[room_num];

		for(size_t i = 0; i < ARRAY_SIZE(transRenderOrder); i++) {
			SMY_ARXMAT::TransparencyType transType = transRenderOrder[i];

			if(!roomMat.count[transType])
				continue;

			switch(transType) {
			case SMY_ARXMAT::Opaque: {
				// This should currently not happen
				arx_assert(false);
				continue;
			}
			case SMY_ARXMAT::Blended: {
				SetZBias(2);
				GRenderer->SetBlendFunc(Renderer::BlendSrcColor, Renderer::BlendDstColor);
				break;
			}
			case SMY_ARXMAT::Multiplicative: {
				SetZBias(2);
				GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
				break;
			}
			case SMY_ARXMAT::Additive: {
				SetZBias(2);
				GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
				break;
			}
			case SMY_ARXMAT::Subtractive: {
				SetZBias(8);
				GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
				break;
			}
			}

			room.pVertexBuffer->drawIndexed(
				Renderer::TriangleList,
				roomMat.uslNbVertex,
				roomMat.uslStartVertex,
				&room.pussIndice[roomMat.offset[transType]],
				roomMat.count[transType]);

			EERIEDrawnPolys += roomMat.count[transType];
		}

		ppTexCurr++;
	}
}

void ARX_PORTALS_Frustrum_ComputeRoom(long room_num, const EERIE_FRUSTRUM & frustrum)
{
	if(RoomDraw[room_num].count == 0) {
		RoomDrawList.push_back(room_num);
	}

	RoomFrustrumAdd(room_num, frustrum);
	RoomDraw[room_num].count++;

	float fClippZFar = ACTIVECAM->cdepth * (fZFogEnd*1.1f);

	// Now Checks For room Portals !!!
	for(long lll=0; lll<portals->room[room_num].nb_portals; lll++) {
		if(portals->portals[portals->room[room_num].portals[lll]].useportal)
			continue;

		EERIE_PORTALS *po = &portals->portals[portals->room[room_num].portals[lll]];
		EERIEPOLY *epp = &po->poly;
	
		//clipp NEAR & FAR
		unsigned char ucVisibilityNear=0;
		unsigned char ucVisibilityFar=0;

		for(size_t i=0; i<ARRAY_SIZE(epp->v); i++) {
			float fDist0 = efpPlaneNear.getDist(epp->v[i].p);

			if(fDist0 < 0.f)
				ucVisibilityNear++;
			if(fDist0 > fClippZFar)
				ucVisibilityFar++;
		}

		if((ucVisibilityFar & 4) || (ucVisibilityNear & 4)) {
			po->useportal=2;
			continue;
		}

		Vec3f pos = epp->center - ACTIVECAM->orgTrans.pos;
		float fRes = dot(pos, epp->norm);

		EERIERTPPoly2(epp);

		if(!IsSphereInFrustrum(epp->center, frustrum, epp->v[0].rhw)) {
			continue;
		}

		bool Cull = !(fRes<0.f);
		
		EERIE_FRUSTRUM fd;
		CreateFrustrum(&fd, &ACTIVECAM->orgTrans.pos, epp, Cull);

		long roomToCompute = 0;
		bool computeRoom = false;

		if(po->room_1 == room_num && !Cull) {
			roomToCompute = po->room_2;
			computeRoom = true;
		}else if(po->room_2 == room_num && Cull) {
			roomToCompute = po->room_1;
			computeRoom = true;
		}

		if(computeRoom) {
			po->useportal=1;
			ARX_PORTALS_Frustrum_ComputeRoom(roomToCompute, fd);
		}
	}
}

void ARX_SCENE_Update() {
	arx_assert(USE_PORTALS && portals);

	unsigned long tim = (unsigned long)(arxtime);

	WATEREFFECT+=0.0005f*framedelay;

	long l = ACTIVECAM->cdepth * 0.42f;
	long clip3D = (l / (long)BKG_SIZX) + 1;
	long lcval = clip3D + 4;

	long camXsnap = ACTIVECAM->orgTrans.pos.x * ACTIVEBKG->Xmul;
	long camZsnap = ACTIVECAM->orgTrans.pos.z * ACTIVEBKG->Zmul;
	camXsnap = clamp(camXsnap, 0, ACTIVEBKG->Xsize - 1L);
	camZsnap = clamp(camZsnap, 0, ACTIVEBKG->Zsize - 1L);

	long x0 = std::max(camXsnap - lcval, 0L);
	long x1 = std::min(camXsnap + lcval, ACTIVEBKG->Xsize - 1L);
	long z0 = std::max(camZsnap - lcval, 0L);
	long z1 = std::min(camZsnap + lcval, ACTIVEBKG->Zsize - 1L);

	ACTIVEBKG->Backg[camXsnap + camZsnap * ACTIVEBKG->Xsize].treat = 1;
	TreatBackgroundDynlights();
	PrecalcDynamicLighting(x0, z0, x1, z1);

	// Go for a growing-square-spirallike-render around the camera position
	// (To maximize Z-Buffer efficiency)

	for(long j=z0; j<=z1; j++) {
		for(long i=x0; i<x1; i++) {
			FAST_BKG_DATA *feg = &ACTIVEBKG->fastdata[i][j];
			feg->treat = 0;
		}
	}

	ResetTileLights();

	long room_num=ARX_PORTALS_GetRoomNumForPosition(&ACTIVECAM->orgTrans.pos,1);
	if(room_num>-1) {

		ARX_PORTALS_InitDrawnRooms();
		EERIE_FRUSTRUM frustrum;
		CreateScreenFrustrum(&frustrum);
		ARX_PORTALS_Frustrum_ComputeRoom(room_num, frustrum);

		for(size_t i = 0; i < RoomDrawList.size(); i++) {
			ARX_PORTALS_Frustrum_RenderRoomTCullSoft(RoomDrawList[i], RoomDraw[RoomDrawList[i]].frustrum, tim);
		}
	}

	ARX_THROWN_OBJECT_Manage(checked_range_cast<unsigned long>(framedelay));

	UpdateInter();
}

extern short uw_mode;
extern long SPECIAL_DRAGINTER_RENDER;

//*************************************************************************************
// Main Background Rendering Proc.
// ie: Big Mess
//*************************************************************************************
///////////////////////////////////////////////////////////
void ARX_SCENE_Render() {

	if(uw_mode)
		GRenderer->GetTextureStage(0)->setMipMapLODBias(10.f);

	GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
	for(size_t i = 0; i < RoomDrawList.size(); i++) {

		ARX_PORTALS_Frustrum_RenderRoomTCullSoftRender(RoomDrawList[i]);
	}

	if(!Project.improve) {
		ARXDRAW_DrawInterShadows();
	}

	ARX_THROWN_OBJECT_Render();
		
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapClamp);
	GRenderer->GetTextureStage(0)->setMipMapLODBias(-0.6f);

	RenderInter();

	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
	GRenderer->GetTextureStage(0)->setMipMapLODBias(-0.3f);
		
	// To render Dragged objs
	if(DRAGINTER) {
		SPECIAL_DRAGINTER_RENDER=1;
		ARX_INTERFACE_RenderCursor();

		SPECIAL_DRAGINTER_RENDER=0;
	}

	PopAllTriangleList();
		
	ARXDRAW_DrawEyeBall();

	GRenderer->SetRenderState(Renderer::DepthWrite, false);

	ARXDRAW_DrawPolyBoom();

	PopAllTriangleListTransparency();

	GRenderer->SetFogColor(Color::none);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetAlphaFunc(Renderer::CmpGreater, .5f);

	for(size_t i = 0; i < RoomDrawList.size(); i++) {

		ARX_PORTALS_Frustrum_RenderRoom_TransparencyTSoftCull(RoomDrawList[i]);
	}

	SetZBias(8);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetCulling(Renderer::CullCW);
	GRenderer->SetAlphaFunc(Renderer::CmpNotEqual, 0.f);

	RenderWater();
	RenderLava();

	SetZBias(0);
	GRenderer->SetFogColor(ulBKGColor);
	GRenderer->GetTextureStage(0)->setColorOp(TextureStage::OpModulate);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);

	Halo_Render();

	GRenderer->SetCulling(Renderer::CullCCW);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);	
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
}

