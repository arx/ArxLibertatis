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
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#include "scene/Scene.h"

#include <vector>
#include <cstdio>
#include <cmath>

#include <boost/foreach.hpp>

#include "ai/Paths.h"

#include "animation/AnimationRender.h"

#include "core/Application.h"
#include "core/ArxGame.h"
#include "core/GameTime.h"
#include "core/Core.h"

#include "game/EntityManager.h"
#include "game/Inventory.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "game/spell/Cheat.h"
#include "game/spell/FlyingEye.h"

#include "gui/Interface.h"
#include "gui/Cursor.h"

#include "graphics/Draw.h"
#include "graphics/DrawLine.h"
#include "graphics/GlobalFog.h"
#include "graphics/Math.h"
#include "graphics/VertexBuffer.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/BlobShadow.h"
#include "graphics/effects/PolyBoom.h"
#include "graphics/effects/Halo.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/texture/Texture.h"
#include "graphics/texture/TextureStage.h"

#include "input/Input.h"

#include "io/log/Logger.h"

#include "scene/Light.h"
#include "scene/Interactive.h"

#include "physics/Projectile.h"

#include "platform/profiler/Profiler.h"


extern bool EXTERNALVIEW; // *sigh*

EERIE_PORTAL_DATA * portals = NULL;

float WATEREFFECT = 0.f;

CircularVertexBuffer<SMY_VERTEX3> * pDynamicVertexBuffer;

namespace {

struct DynamicVertexBuffer {
	
private:
	
	SMY_VERTEX3 * vertices;
	size_t start;
	
public:
	
	size_t nbindices;
	std::vector<unsigned short> indices;
	size_t offset;
	
	DynamicVertexBuffer()
		: vertices(NULL)
		, start(0)
		, nbindices(0)
		, offset(0)
	{ }
	
	void lock(size_t count) {
		
		arx_assert(!vertices);
		
		if(indices.empty()) {
			indices.resize(4 * pDynamicVertexBuffer->vb->capacity());
			start = 0;
		}
		
		BufferFlags flags = (pDynamicVertexBuffer->pos == 0) ? DiscardBuffer : NoOverwrite | DiscardRange;
		
		vertices =  pDynamicVertexBuffer->vb->lock(flags, pDynamicVertexBuffer->pos, count);
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
		pDynamicVertexBuffer->vb->drawIndexed(primitive, pDynamicVertexBuffer->pos - start, start, &indices[0], nbindices);
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
	
} dynamicVertices;

} // anonymous namespace

static Plane efpPlaneNear;

static std::vector<EERIEPOLY *> vPolyWater;
static std::vector<EERIEPOLY *> vPolyLava;

static std::vector<PORTAL_ROOM_DRAW> RoomDraw;
static std::vector<size_t> RoomDrawList;

Vec2f getWaterFxUvOffset(float watereffect, const Vec3f & odtv) {
	return Vec2f(std::sin(watereffect + odtv.x), std::cos(watereffect + odtv.z));
}

static void ApplyLavaGlowToVertex(const Vec3f & odtv, ColorRGBA & color, float power) {
	power = 1.f - std::sin(WATEREFFECT + odtv.x + odtv.z) * 0.05f * power;
	color = (Color4f::fromRGBA(color) * power).toRGBA();
}

static void ManageWater_VertexBuffer(EERIEPOLY * ep, const long to,
                                     float uvScroll, SMY_VERTEX * _pVertex) {
	
	for(long k = 0; k < to; k++) {
		Vec2f uv = ep->v[k].uv;
		
		uv += getWaterFxUvOffset(WATEREFFECT, ep->v[k].p) * (0.35f * 0.05f);
			
		if(ep->type & POLY_FALL) {
			uv.y -= uvScroll;
		}
		
		_pVertex[ep->uslInd[k]].uv = uv;
	}
}

static void ManageLava_VertexBuffer(EERIEPOLY * ep, const long to,
                                    float uvScroll, SMY_VERTEX * _pVertex) {
	
	for(long k = 0; k < to; k++) {
		Vec2f uv = ep->v[k].uv;
		
		uv += getWaterFxUvOffset(WATEREFFECT, ep->v[k].p) * (0.35f * 0.05f);
		ApplyLavaGlowToVertex(ep->v[k].p, ep->color[k], 0.6f);
			
		if(ep->type & POLY_FALL) {
			uv.y -= uvScroll;
		}
		
		_pVertex[ep->uslInd[k]].uv = uv;
	}
}

bool IsSphereInFrustrum(const Vec3f & point, const EERIE_FRUSTRUM & frustrum, float radius = 0.f);

static bool FrustrumsClipSphere(const EERIE_FRUSTRUM_DATA & frustrums,
                                const Sphere & sphere) {
	
	float dists = distanceToPoint(efpPlaneNear, sphere.origin);

	if(dists + sphere.radius > 0) {
		for(long i = 0; i < frustrums.nb_frustrums; i++) {
			if(IsSphereInFrustrum(sphere.origin, frustrums.frustrums[i], sphere.radius))
				return false;
		}
	}

	return true;
}

bool VisibleSphere(const Sphere & sphere) {
	
	ARX_PROFILE_FUNC();
	
	if(fartherThan(sphere.origin, g_camera->m_pos, g_camera->cdepth * 0.5f + sphere.radius)) {
		return false;
	}
	
	long room_num = ARX_PORTALS_GetRoomNumForPosition(sphere.origin);
	if(room_num >= 0) {
		EERIE_FRUSTRUM_DATA & frustrums = RoomDraw[room_num].frustrum;
		if(FrustrumsClipSphere(frustrums, sphere)) {
			return false;
		}
	}
	
	return true;
}

static bool IsBBoxInFrustrum(const EERIE_3D_BBOX & bbox, const EERIE_FRUSTRUM & frustrum) {
	
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
	return IsSphereInFrustrum(point, frustrum);
}

static bool FrustrumsClipBBox3D(const EERIE_FRUSTRUM_DATA & frustrums,
                                const EERIE_3D_BBOX & bbox) {
	
	for(long i = 0; i < frustrums.nb_frustrums; i++)
	{
		if(IsBBoxInFrustrum(bbox, frustrums.frustrums[i]))
			return false;
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
	
	if(io == entities.player()) {
		return false;
	}
	
	if(io && (io->ioflags & IO_FORCEDRAW)) {
		return false;
	}
	
	if(portals) {
		Vec3f posi = position + Vec3f(0, -60, 0); // -20 ?
		long room_num;

		if(io) {
			if(io->requestRoomUpdate)
				UpdateIORoom(io);

			room_num = io->room;//
		} else {
			room_num = ARX_PORTALS_GetRoomNumForPosition(posi);
		}

		if(room_num == -1) {
			posi.y = position.y - 120;
			room_num = ARX_PORTALS_GetRoomNumForPosition(posi);
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
				Sphere sphere;
				sphere.origin = (io->bbox3D.min + io->bbox3D.max) * .5f;
				sphere.radius = glm::distance(sphere.origin, io->bbox3D.min) + 10.f;

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

static EERIEPOLY * ARX_PORTALS_GetRoomNumForPosition2(const Vec3f & pos, long flag) {
	
	EERIEPOLY * ep;
	if(flag & 1) {
		ep = CheckInPoly(pos + Vec3f(0.f, -150.f, 0.f));
		if(!ep) {
			ep = CheckInPoly(pos + Vec3f(0.f, -1.f, 0.f));
		}
	} else {
		ep = CheckInPoly(pos);
	}
	if(ep && ep->room > -1) {
		return ep;
	}
	
	// Security... ?
	ep = GetMinPoly(pos);
	if(ep && ep->room > -1) {
		return ep;
	} else if(!(flag & 1)) {
		ep = CheckInPoly(pos);
		if(ep && ep->room > -1) {
			return ep;
		}
	}
	
	if(flag & 2) {
		
		float off = 20.f;
		
		ep = CheckInPoly(pos + Vec3f(-off, -off, 0.f));
		if(ep && ep->room > -1) {
			return ep;
		}
		
		ep = CheckInPoly(pos + Vec3f(-off, -20, -off));
		if(ep && ep->room > -1) {
			return ep;
		}
		
		ep = CheckInPoly(pos + Vec3f(-off, -20, off));
		if(ep && ep->room > -1) {
			return ep;
		}
		
		ep = CheckInPoly(pos + Vec3f(off, -20, 0.f));
		if(ep && ep->room > -1) {
			return ep;
		}
		
		ep = CheckInPoly(pos + Vec3f(off, -20, off));
		if(ep && ep->room > -1) {
			return ep;
		}
		
		ep = CheckInPoly(pos + Vec3f(off, -20, -off));
		if(ep && ep->room > -1) {
			return ep;
		}
		
	}
	
	return NULL;
}

static EERIEPOLY * ARX_PORTALS_GetRoomNumForCamera(const Vec3f & pos, const Vec3f & direction) {
	
	{
		EERIEPOLY * ep = CheckInPoly(pos);
		if(ep && ep->room > -1) {
			return ep;
		}
	}
	
	{
		EERIEPOLY * ep = GetMinPoly(pos);
		if(ep && ep->room > -1) {
			return ep;
		}
	}
	
	float dist = 0.f;
	while(dist <= 20.f) {
		
		Vec3f tmpPos = pos;
		tmpPos += direction * dist;
		
		EERIEPOLY * ep = CheckInPoly(tmpPos);
		
		if(ep && ep->room > -1) {
			return ep;
		}
		
		dist += 5.f;
	}
	
	return NULL;
}

// flag==1 for player
long ARX_PORTALS_GetRoomNumForPosition(const Vec3f & pos, long flag) {
	
	ARX_PROFILE_FUNC();
	
	long num = -1;
	float height = 0.f;
	
	if(flag & 1) {
		Vec3f direction = angleToVectorXZ_180offset(g_camera->angle.getYaw());
		EERIEPOLY * ep = ARX_PORTALS_GetRoomNumForCamera(g_camera->m_pos, direction);
		if(ep) {
			num = ep->room;
			height = ep->center.y;
		} else {
			num = -1;
		}
	} else {
		EERIEPOLY * ep = ARX_PORTALS_GetRoomNumForPosition2(pos, flag);
		if(ep) {
			num = ep->room;
			height = ep->center.y;
		} else {
			num = -1;
		}
	}
	
	if(num > -1) {
		long nearest = -1;
		float nearest_dist = 99999.f;

		BOOST_FOREACH(const EERIE_ROOM_DATA & room, portals->rooms) {
			BOOST_FOREACH(long portal, room.portals) {
				const EERIE_PORTALS & po = portals->portals[portal];
				const PortalPoly * epp = &po.poly;
				if(PointIn2DPolyXZ(epp, pos.x, pos.z)) {
					float yy;
					if(GetTruePolyY(epp, pos, &yy)) {
						if(height > yy) {
							if(yy >= pos.y && yy - pos.y < nearest_dist) {
								if(epp->norm.y > 0) {
									nearest = po.room_2;
								} else {
									nearest = po.room_1;
								}
								nearest_dist = yy - pos.y;
							}
						}
					}
				}
			}
		}
		
		if(nearest > -1) {
			num = nearest;
		}
		
	}
	
	return num;
}

static void ARX_PORTALS_Frustrum_ClearIndexCount(size_t room_num) {
	
	EERIE_ROOM_DATA & room = portals->rooms[room_num];
	
	std::vector<TextureContainer *>::const_iterator itr;
	for(itr = room.ppTextureContainer.begin(); itr != room.ppTextureContainer.end(); ++itr) {

		TextureContainer * pTexCurr = *itr;
		GRenderer->SetTexture(0, pTexCurr);

		SMY_ARXMAT & roomMat = pTexCurr->m_roomBatches[room_num];

		roomMat.count[BatchBucket_Opaque] = 0;
		roomMat.count[BatchBucket_Blended] = 0;
		roomMat.count[BatchBucket_Multiplicative] = 0;
		roomMat.count[BatchBucket_Additive] = 0;
		roomMat.count[BatchBucket_Subtractive] = 0;
	}
}

static void ARX_PORTALS_InitDrawnRooms() {
	
	ARX_PROFILE_FUNC();
	
	arx_assert(portals);
	
	for(size_t i = 0; i < portals->portals.size(); i++) {
		portals->portals[i].useportal = 0;
	}
	
	for(size_t i = 0; i < portals->rooms.size(); i++) {
		ARX_PORTALS_Frustrum_ClearIndexCount(i);
	}

	RoomDraw.resize(portals->rooms.size());

	for(size_t i = 0; i < RoomDraw.size(); i++) {
		RoomDraw[i].count = 0;
		RoomDraw[i].frustrum.nb_frustrums = 0;
	}

	RoomDrawList.clear();

	vPolyWater.clear();
	vPolyLava.clear();

	if(pDynamicVertexBuffer) {
		pDynamicVertexBuffer->vb->setData(NULL, 0, 0, DiscardBuffer);
		dynamicVertices.reset();
	}
}

bool IsSphereInFrustrum(const Vec3f & point, const EERIE_FRUSTRUM & frustrum, float radius) {
	
	float dists[4];
	dists[0] = distanceToPoint(frustrum.plane[0], point);
	dists[1] = distanceToPoint(frustrum.plane[1], point);
	dists[2] = distanceToPoint(frustrum.plane[2], point);
	dists[3] = distanceToPoint(frustrum.plane[3], point);
	
	return (dists[0] + radius > 0 && dists[1] + radius > 0 && dists[2] + radius > 0 && dists[3] + radius > 0);
}

static bool FrustrumsClipPoly(const EERIE_FRUSTRUM_DATA & frustrums,
                              const EERIEPOLY & ep) {
	
	for(long i = 0; i < frustrums.nb_frustrums; i++) {
		if(IsSphereInFrustrum(ep.center, frustrums.frustrums[i], ep.v[0].w))
			return false;
	}
	
	return true;
}

static Plane CreatePlane(const Vec3f & orgn, const Vec3f & pt1, const Vec3f & pt2) {
	
	Plane plane;
	
	Vec3f A = pt1 - orgn;
	Vec3f B = pt2 - orgn;
	
	plane.a = A.y * B.z - A.z * B.y;
	plane.b = A.z * B.x - A.x * B.z;
	plane.c = A.x * B.y - A.y * B.x;
	
	float epnlen = std::sqrt(plane.a * plane.a + plane.b * plane.b + plane.c * plane.c);
	epnlen = 1.f / epnlen;
	
	plane.a *= epnlen;
	plane.b *= epnlen;
	plane.c *= epnlen;
	plane.d = -(orgn.x * plane.a + orgn.y * plane.b + orgn.z * plane.c);
	
	return plane;
}

static EERIE_FRUSTRUM CreateFrustrum(const Vec3f & pos, const PortalPoly & ep, bool cull) {
	
	EERIE_FRUSTRUM frustrum;
	
	if(cull) {
		frustrum.plane[0] = CreatePlane(pos, ep.p[0], ep.p[1]);
		frustrum.plane[1] = CreatePlane(pos, ep.p[3], ep.p[2]);
		frustrum.plane[2] = CreatePlane(pos, ep.p[1], ep.p[3]);
		frustrum.plane[3] = CreatePlane(pos, ep.p[2], ep.p[0]);
	} else {
		frustrum.plane[0] = CreatePlane(pos, ep.p[1], ep.p[0]);
		frustrum.plane[1] = CreatePlane(pos, ep.p[2], ep.p[3]);
		frustrum.plane[2] = CreatePlane(pos, ep.p[3], ep.p[1]);
		frustrum.plane[3] = CreatePlane(pos, ep.p[0], ep.p[2]);
	}
	
	return frustrum;
}

static EERIE_FRUSTRUM CreateScreenFrustrum() {
	
	EERIE_FRUSTRUM frustrum;
	
	glm::mat4x4 worldToClip = g_preparedCamera.m_viewToClip * g_preparedCamera.m_worldToView;
	
	{
		Plane & plane = frustrum.plane[0];
		plane.a = worldToClip[0][3] - worldToClip[0][0];
		plane.b = worldToClip[1][3] - worldToClip[1][0];
		plane.c = worldToClip[2][3] - worldToClip[2][0];
		plane.d = worldToClip[3][3] - worldToClip[3][0];
		normalizePlane(plane);
	}
	
	{
		Plane & plane = frustrum.plane[1];
		plane.a = worldToClip[0][3] + worldToClip[0][0];
		plane.b = worldToClip[1][3] + worldToClip[1][0];
		plane.c = worldToClip[2][3] + worldToClip[2][0];
		plane.d = worldToClip[3][3] + worldToClip[3][0];
		normalizePlane(plane);
	}
	
	{
		Plane & plane = frustrum.plane[2];
		plane.a = worldToClip[0][3] - worldToClip[0][1];
		plane.b = worldToClip[1][3] - worldToClip[1][1];
		plane.c = worldToClip[2][3] - worldToClip[2][1];
		plane.d = worldToClip[3][3] - worldToClip[3][1];
		normalizePlane(plane);
	}
	
	{
		Plane & plane = frustrum.plane[3];
		plane.a = worldToClip[0][3] + worldToClip[0][1];
		plane.b = worldToClip[1][3] + worldToClip[1][1];
		plane.c = worldToClip[2][3] + worldToClip[2][1];
		plane.d = worldToClip[3][3] + worldToClip[3][1];
		normalizePlane(plane);
	}
	
	{
		Plane & plane = efpPlaneNear;
		plane.a = worldToClip[0][3] + worldToClip[0][2];
		plane.b = worldToClip[1][3] + worldToClip[1][2];
		plane.c = worldToClip[2][3] + worldToClip[2][2];
		plane.d = worldToClip[3][3] + worldToClip[3][2];
		normalizePlane(plane);
	}
	
	return frustrum;
}

void RoomDrawRelease() {
	RoomDrawList.clear();
	RoomDraw.clear();
}

static void RoomFrustrumAdd(size_t num, const EERIE_FRUSTRUM & fr) {
	if(RoomDraw[num].frustrum.nb_frustrums < MAX_FRUSTRUMS - 1) {
		RoomDraw[num].frustrum.frustrums[RoomDraw[num].frustrum.nb_frustrums] = fr;
		RoomDraw[num].frustrum.nb_frustrums++;
	}
}

static void RenderWaterBatch() {
	
	if(!dynamicVertices.nbindices) {
		return;
	}
	
	GRenderer->GetTextureStage(1)->setColorOp(TextureStage::OpModulate4X);
	GRenderer->GetTextureStage(1)->setAlphaOp(TextureStage::OpDisable);
	
	GRenderer->GetTextureStage(2)->setColorOp(TextureStage::OpModulate);
	GRenderer->GetTextureStage(2)->setAlphaOp(TextureStage::OpDisable);
	
	dynamicVertices.draw(Renderer::TriangleList);
	
	GRenderer->GetTextureStage(1)->setColorOp(TextureStage::OpDisable);
	GRenderer->GetTextureStage(2)->setColorOp(TextureStage::OpDisable);
	
}

static Vec2f FluidTextureDisplacement(const Vec3f & p, float time,
                                      float divVar1, float divVar2, float divVar3,
                                      float divVar4, float addVar1,
                                      float addVar2, Vec2f sign) {
	
	float u = (p.x + addVar1) / divVar1 + sign.x * glm::sin((p.x + addVar2) / divVar2 + time / divVar3) / divVar4;
	float v = (p.z + addVar1) / divVar1 + sign.y * glm::cos((p.z + addVar2) / divVar2 + time / divVar3) / divVar4;
	
	return Vec2f(u, v);
}

static Vec2f CalculateWaterDisplacement(EERIEPOLY * ep, float time, int vertIndex, int step) {
	
	const Vec3f & p = ep->v[vertIndex].p;
	
	switch(step) {
	case 0: {
		return FluidTextureDisplacement(p, time, 1000, 200, 1000, 32, 0.f, 0.f, Vec2f(1.f, 1.f));
	}
	case 1: {
		return FluidTextureDisplacement(p, time, 1000, 200, 1000, 28, 30.f, 30, Vec2f(1.f, -1.f));
	}
	case 2: {
		return FluidTextureDisplacement(p, time, 1000, 200, 1000, 40, 60.f, 60, Vec2f(-1.f, -1.f));
	}
	default:
		return Vec2f(0.f);
	}
}

static Vec2f CalculateLavaDisplacement(EERIEPOLY * ep, float time, int vertIndex, int step) {
	
	const Vec3f & p = ep->v[vertIndex].p;
	
	switch(step) {
	case 0: {
		return FluidTextureDisplacement(p, time, 1000, 200, 2000, 20, 0.f, 0.f, Vec2f(1.f, 1.f));
	}
	case 1: {
		return FluidTextureDisplacement(p, time, 1000, 100, 2000, 10, 0.f, 0.f, Vec2f(1.f, 1.f));
	}
	case 2: {
		return FluidTextureDisplacement(p, time, 600, 160, 2000, 11, 0.f, 0.f, Vec2f(1.f, 1.f));
	}
	default:
		return Vec2f(0.f);
	}
}

const int FTVU_STEP_COUNT = 3; // For fTv and fTu calculations

static void RenderWater() {
	
	ARX_PROFILE_FUNC();
	
	if(vPolyWater.empty()) {
		return;
	}
	
	size_t iNbIndice = 0;
	int iNb = vPolyWater.size();
	
	dynamicVertices.lock(iNb * 4);
	
	UseRenderState state(render3D().depthWrite(false).cull(CullCW).depthOffset(8).blend(BlendDstColor, BlendOne));
	
	GRenderer->SetTexture(0, enviro);
	GRenderer->SetTexture(1, enviro);
	GRenderer->SetTexture(2, enviro);
	
	unsigned short * indices = &dynamicVertices.indices[0];
	
	float time = toMsf(g_gameTime.now());

	while(iNb--) {
		EERIEPOLY * ep = vPolyWater[iNb];
		
		unsigned short iNbVertex = (ep->type & POLY_QUAD) ? 4 : 3;
		SMY_VERTEX3 * pVertex = dynamicVertices.append(iNbVertex);
		
		if(!pVertex) {
			dynamicVertices.unlock();
			RenderWaterBatch();
			dynamicVertices.reset();
			dynamicVertices.lock((iNb + 1) * 4);
			iNbIndice = 0;
			indices = &dynamicVertices.indices[0];
			pVertex = dynamicVertices.append(iNbVertex);
		}
		
		for(int j = 0; j < iNbVertex; ++j) {
			pVertex->p = ep->v[j].p;
			pVertex->color = Color::gray(0.314f).toRGBA();
			
			for(int i = 0; i < FTVU_STEP_COUNT; ++i) {
				Vec2f uv = CalculateWaterDisplacement(ep, time, j, i);
				if(ep->type & POLY_FALL) {
					uv.y += time * (1.f / 4000);
				}
				pVertex->uv[i] = uv;
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

static void RenderLavaBatch() {
	
	if(!dynamicVertices.nbindices) {
		return;
	}
	
	RenderState baseState = render3D().depthWrite(false).cull(CullCW).depthOffset(8);
	
	GRenderer->GetTextureStage(0)->setColorOp(TextureStage::OpModulate2X);
	
	GRenderer->GetTextureStage(1)->setColorOp(TextureStage::OpModulate4X);
	GRenderer->GetTextureStage(1)->setAlphaOp(TextureStage::OpDisable);
	
	GRenderer->GetTextureStage(2)->setColorOp(TextureStage::OpModulate);
	GRenderer->GetTextureStage(2)->setAlphaOp(TextureStage::OpDisable);
	
	{
		UseRenderState state(baseState.blend(BlendDstColor, BlendOne));
		dynamicVertices.draw(Renderer::TriangleList);
	}
	
	GRenderer->GetTextureStage(0)->setColorOp(TextureStage::OpModulate);
	
	{
		UseRenderState state(baseState.blend(BlendZero, BlendInvSrcColor));
		dynamicVertices.draw(Renderer::TriangleList);
	}
	
	GRenderer->GetTextureStage(1)->setColorOp(TextureStage::OpDisable);
	GRenderer->GetTextureStage(2)->setColorOp(TextureStage::OpDisable);
	
}

static void RenderLava() {
	
	ARX_PROFILE_FUNC();
	
	if(vPolyLava.empty()) {
		return;
	}
	
	size_t iNbIndice = 0;
	int iNb = vPolyLava.size();
	
	dynamicVertices.lock(iNb * 4);
	
	GRenderer->SetTexture(0, enviro);
	GRenderer->SetTexture(1, enviro);
	GRenderer->SetTexture(2, enviro);
	
	unsigned short * indices = &dynamicVertices.indices[0];
	
	float time = toMsf(g_gameTime.now());

	while(iNb--) {
		EERIEPOLY * ep = vPolyLava[iNb];
		
		unsigned short iNbVertex = (ep->type & POLY_QUAD) ? 4 : 3;
		SMY_VERTEX3 * pVertex = dynamicVertices.append(iNbVertex);
		
		if(!pVertex) {
			dynamicVertices.unlock();
			RenderLavaBatch();
			dynamicVertices.reset();
			dynamicVertices.lock((iNb + 1) * 4);
			iNbIndice = 0;
			indices = &dynamicVertices.indices[0];
			pVertex = dynamicVertices.append(iNbVertex);
		}
		
		for(int j = 0; j < iNbVertex; ++j) {
			pVertex->p = ep->v[j].p;
			pVertex->color = Color::gray(0.4f).toRGBA();
			for(int i = 0; i < FTVU_STEP_COUNT; ++i) {
				Vec2f uv = CalculateLavaDisplacement(ep, time, j, i);
				pVertex->uv[i] = uv;
			}
			pVertex++;
			if(j == 2) {
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

static void ARX_PORTALS_Frustrum_RenderRoomTCullSoft(size_t room_num,
                                                     const EERIE_FRUSTRUM_DATA & frustrums,
                                                     GameInstant now,
                                                     const Vec3f & camPos
) {
	ARX_PROFILE_FUNC();
	
	if(!RoomDraw[room_num].count)
		return;

	EERIE_ROOM_DATA & room = portals->rooms[room_num];

	if(!room.pVertexBuffer) {
		// No need to spam this for every frame as there will already be an
		// earlier warning
		LogDebug("no vertex data for room " << room_num);
		return;
	}
	
	SMY_VERTEX * pMyVertex = room.pVertexBuffer->lock(NoOverwrite);
	
	unsigned short * pIndices = &room.indexBuffer[0];
	
	BOOST_FOREACH(const EP_DATA & epd, room.epdata) {
		
		BackgroundTileData * feg = &ACTIVEBKG->m_tileData[epd.tile.x][epd.tile.y];
		
		if(!ACTIVEBKG->isTileActive(epd.tile)) {
			// TODO copy-paste background tiles
			int tilex = epd.tile.x;
			int tilez = epd.tile.y;
			int radius = 1;
			
			int minx = std::max(tilex - radius, 0);
			int maxx = std::min(tilex + radius, ACTIVEBKG->m_size.x - 1);
			int minz = std::max(tilez - radius, 0);
			int maxz = std::min(tilez + radius, ACTIVEBKG->m_size.y - 1);
			for(int z = minz; z <= maxz; z++)
			for(int x = minx; x <= maxx; x++) {
				if(!ACTIVEBKG->isTileActive(Vec2s(x, z))) {
					ACTIVEBKG->setTileActive(Vec2s(x, z));
					ComputeTileLights(x, z);
				}
			}
		}
		
		EERIEPOLY * ep = &feg->polydata[epd.idx];
		if(!ep->tex) {
			continue;
		}
		
		if(ep->type & (POLY_IGNORE | POLY_NODRAW | POLY_HIDE)) {
			continue;
		}
		
		if(FrustrumsClipPoly(frustrums, *ep)) {
			continue;
		}

		if(ep->v[0].w < -distanceToPoint(efpPlaneNear, ep->center)) {
			continue;
		}

		Vec3f nrm = ep->v[2].p - camPos;
		int to = (ep->type & POLY_QUAD) ? 4 : 3;

		if(!(ep->type & POLY_DOUBLESIDED) && glm::dot(ep->norm , nrm) > 0.f) {
			if(to == 3 || glm::dot(ep->norm2 , nrm) > 0.f) {
				continue;
			}
		}

		BatchBucket transparencyType;

		if(ep->type & POLY_TRANS) {
			if(ep->transval >= 2.f) {
				transparencyType = BatchBucket_Multiplicative;
			} else if(ep->transval >= 1.f) {
				transparencyType = BatchBucket_Additive;
			} else if(ep->transval > 0.f) {
				transparencyType = BatchBucket_Blended;
			} else {
				transparencyType = BatchBucket_Subtractive;
			}
		} else {
			transparencyType = BatchBucket_Opaque;
		}

		SMY_ARXMAT & roomMat = ep->tex->m_roomBatches[room_num];

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

		if(!player.m_improve) { // Normal View...
			if(ep->type & POLY_GLOW) {
				pMyVertexCurr[ep->uslInd[0]].color = Color::white.toRGBA();
				pMyVertexCurr[ep->uslInd[1]].color = Color::white.toRGBA();
				pMyVertexCurr[ep->uslInd[2]].color = Color::white.toRGBA();

				if(to == 4) {
					pMyVertexCurr[ep->uslInd[3]].color = Color::white.toRGBA();
				}
			} else {
				if(!(ep->type & POLY_TRANS)) {
					ApplyTileLights(ep, epd.tile);
					pMyVertexCurr[ep->uslInd[0]].color = ep->color[0];
					pMyVertexCurr[ep->uslInd[1]].color = ep->color[1];
					pMyVertexCurr[ep->uslInd[2]].color = ep->color[2];
					if(to & 4) {
						pMyVertexCurr[ep->uslInd[3]].color = ep->color[3];
					}
				}

			}

		} else { // Improve Vision Activated
			if(!(ep->type & POLY_TRANS)) {

				ApplyTileLights(ep, epd.tile);

				bool valid = true;
				for(int k = 0; k < to; k++) {
					
					float lr = Color4f::fromRGBA(ep->color[k]).r;
					
					Vec4f p = worldToClipSpace(ep->v[1].p);
					if(p.w <= 0.f || p.z <= 0.f) {
						valid = false;
						break;
					}
					
					float dd = 1.f / p.w;
					dd = glm::clamp(dd, 0.f, 1.f);
					Vec3f & norm = ep->nrml[k];
					float fb = ((1.f - dd) * 6.f + glm::abs(norm.x) + glm::abs(norm.y)) * 0.125f;
					float fr = ((0.6f - dd) * 6.f + glm::abs(norm.z) + glm::abs(norm.y)) * 0.125f;
					if(fr < 0.f) {
						fr = 0.f;
					} else {
						fr = std::max(lr, fr);
					}
					
					ep->color[k] = Color3f(fr, 0.12f, fb).toRGB();
				}
				if(!valid) {
					continue;
				}

				pMyVertexCurr[ep->uslInd[0]].color = ep->color[0];
				pMyVertexCurr[ep->uslInd[1]].color = ep->color[1];
				pMyVertexCurr[ep->uslInd[2]].color = ep->color[2];

				if(to == 4) {
					pMyVertexCurr[ep->uslInd[3]].color = ep->color[3];
				}
			}
		}
		
		if(ep->type & POLY_LAVA) {
			float uvScroll = toMsf(now) * (1.f / 12000);
			ManageLava_VertexBuffer(ep, to, uvScroll, pMyVertexCurr);
			vPolyLava.push_back(ep);
		} else if(ep->type & POLY_WATER) {
			float uvScroll = toMsf(now) * 0.001f;
			ManageWater_VertexBuffer(ep, to, uvScroll, pMyVertexCurr);
			vPolyWater.push_back(ep);
		}
		
	}

	room.pVertexBuffer->unlock();
}


static void BackgroundRenderOpaque(size_t room_num) {
	
	ARX_PROFILE_FUNC();
	
	EERIE_ROOM_DATA & room = portals->rooms[room_num];
	
	std::vector<TextureContainer *>::const_iterator itr;
	for(itr = room.ppTextureContainer.begin(); itr != room.ppTextureContainer.end(); ++itr) {
		
		TextureContainer * pTexCurr = *itr;
		const SMY_ARXMAT & roomMat = pTexCurr->m_roomBatches[room_num];
		
		RenderState baseState = render3D();
		
		GRenderer->SetTexture(0, pTexCurr);
		baseState.setAlphaCutout(pTexCurr->m_pTexture && pTexCurr->m_pTexture->hasAlpha());
		
		UseRenderState state(baseState);
		
		if(roomMat.count[BatchBucket_Opaque]) {
			if (pTexCurr->userflags & POLY_METAL)
				GRenderer->GetTextureStage(0)->setColorOp(TextureStage::OpModulate2X);
			else
				GRenderer->GetTextureStage(0)->setColorOp(TextureStage::OpModulate);
			
			room.pVertexBuffer->drawIndexed(
				Renderer::TriangleList,
				roomMat.uslNbVertex,
				roomMat.uslStartVertex,
				&room.indexBuffer[roomMat.offset[BatchBucket_Opaque]],
				roomMat.count[BatchBucket_Opaque]);
			
			EERIEDrawnPolys += roomMat.count[BatchBucket_Opaque];
		}
	}
	
	GRenderer->GetTextureStage(0)->setColorOp(TextureStage::OpModulate);
	
}

//-----------------------------------------------------------------------------

static const BatchBucket transRenderOrder[] = {
	BatchBucket_Blended,
	BatchBucket_Multiplicative,
	BatchBucket_Additive,
	BatchBucket_Subtractive
};


static void BackgroundRenderTransparent(size_t room_num) {
	
	ARX_PROFILE_FUNC();
	
	EERIE_ROOM_DATA & room = portals->rooms[room_num];
	
	std::vector<TextureContainer *>::const_iterator itr;
	for(itr = room.ppTextureContainer.begin(); itr != room.ppTextureContainer.end(); ++itr) {
		
		RenderState baseState = render3D().depthWrite(false).depthOffset(2);
		
		TextureContainer * pTexCurr = *itr;
		GRenderer->SetTexture(0, pTexCurr);
		baseState.setAlphaCutout(pTexCurr->m_pTexture && pTexCurr->m_pTexture->hasAlpha());
		
		SMY_ARXMAT & roomMat = pTexCurr->m_roomBatches[room_num];

		for(size_t i = 0; i < ARRAY_SIZE(transRenderOrder); i++) {
			BatchBucket transType = transRenderOrder[i];

			if(!roomMat.count[transType])
				continue;

			RenderState desiredState = baseState;
			switch(transType) {
			case BatchBucket_Opaque: {
				// This should currently not happen
				arx_assert(false);
				continue;
			}
			case BatchBucket_Blended: {
				desiredState.setBlend(BlendSrcColor, BlendDstColor);
				break;
			}
			case BatchBucket_Multiplicative: {
				desiredState.setBlend(BlendOne, BlendOne);
				break;
			}
			case BatchBucket_Additive: {
				desiredState.setBlend(BlendOne, BlendOne);
				break;
			}
			case BatchBucket_Subtractive: {
				desiredState.setDepthOffset(8);
				desiredState.setBlend(BlendZero, BlendInvSrcColor);
				break;
			}
			}
			
			UseRenderState state(desiredState);
			room.pVertexBuffer->drawIndexed(
				Renderer::TriangleList,
				roomMat.uslNbVertex,
				roomMat.uslStartVertex,
				&room.indexBuffer[roomMat.offset[transType]],
				roomMat.count[transType]);

			EERIEDrawnPolys += roomMat.count[transType];
		}
	}
}

static void ARX_PORTALS_Frustrum_ComputeRoom(size_t roomIndex,
                                             const EERIE_FRUSTRUM & frustrum,
                                             const Vec3f & camPos, float camDepth
) {
	arx_assert(roomIndex < portals->rooms.size());
	
	if(RoomDraw[roomIndex].count == 0) {
		RoomDrawList.push_back(roomIndex);
	}
	
	RoomFrustrumAdd(roomIndex, frustrum);
	RoomDraw[roomIndex].count++;
	
	float fClippZFar = camDepth * fZFogEnd * 1.1f;
	
	// Now Checks For room Portals !!!
	BOOST_FOREACH(long portal, portals->rooms[roomIndex].portals) {
		EERIE_PORTALS * po = &portals->portals[portal];
		
		if(po->useportal) {
			continue;
		}
		
		PortalPoly & epp = po->poly;
		
		unsigned char ucVisibilityNear = 0;
		unsigned char ucVisibilityFar = 0;
		
		for(size_t i = 0; i < ARRAY_SIZE(epp.p); i++) {
			float fDist0 = distanceToPoint(efpPlaneNear, epp.p[i]);
			if(fDist0 < 0.f) {
				ucVisibilityNear++;
			}
			if(fDist0 > fClippZFar) {
				ucVisibilityFar++;
			}
		}
		
		if((ucVisibilityFar & 4) || (ucVisibilityNear & 4)) {
			po->useportal = 2;
			continue;
		}
		
		Vec3f pos = epp.center - camPos;
		float fRes = glm::dot(pos, epp.norm);
		
		if(!IsSphereInFrustrum(epp.center, frustrum, epp.rhw)) {
			continue;
		}
		
		bool Cull = !(fRes < 0.f);
		
		EERIE_FRUSTRUM fd = CreateFrustrum(camPos, epp, Cull);

		size_t roomToCompute = 0;
		bool computeRoom = false;

		if(po->room_1 == roomIndex && !Cull) {
			roomToCompute = po->room_2;
			computeRoom = true;
		}else if(po->room_2 == roomIndex && Cull) {
			roomToCompute = po->room_1;
			computeRoom = true;
		}
		
		if(computeRoom) {
			po->useportal = 1;
			ARX_PORTALS_Frustrum_ComputeRoom(roomToCompute, fd, camPos, camDepth);
		}
		
	}
	
}

void ARX_SCENE_Update() {
	
	if(!portals) {
		return;
	}
	
	ARX_PROFILE_FUNC();
	
	GameInstant now = g_gameTime.now();
	
	WATEREFFECT = (toMsi(now) % long(2 * glm::pi<float>() / 0.0005f)) * 0.0005f;
	
	const Vec3f camPos = g_camera->m_pos;
	const float camDepth = g_camera->cdepth;

	TreatBackgroundDynlights();
	PrecalcDynamicLighting(camPos, camDepth);
	
	ACTIVEBKG->resetActiveTiles();
	
	ARX_PORTALS_InitDrawnRooms();
	
	EERIE_FRUSTRUM screenFrustrum = CreateScreenFrustrum();
	
	if(!USE_PLAYERCOLLISIONS) {
		for(size_t i = 0; i < portals->rooms.size(); i++) {
			RoomDraw[i].count = 1;
			RoomDrawList.push_back(i);
			RoomFrustrumAdd(i, screenFrustrum);
		}
	} else {
		long room_num = ARX_PORTALS_GetRoomNumForPosition(camPos, 1);
		if(room_num > -1) {
			size_t roomIndex = static_cast<size_t>(room_num);
			ARX_PORTALS_Frustrum_ComputeRoom(roomIndex, screenFrustrum, camPos, camDepth);
		}
	}
	
	for(size_t i = 0; i < RoomDrawList.size(); i++) {
		ARX_PORTALS_Frustrum_RenderRoomTCullSoft(RoomDrawList[i], RoomDraw[RoomDrawList[i]].frustrum, now, camPos);
	}
	
	ARX_THROWN_OBJECT_Manage(g_gameTime.lastFrameDuration());
	
	UpdateInter();
}

void ARX_SCENE_Render() {
	
	ARX_PROFILE_FUNC();
	
	if(uw_mode)
		GRenderer->GetTextureStage(0)->setMipMapLODBias(10.f);
	
	for(size_t i = 0; i < RoomDrawList.size(); i++) {
		BackgroundRenderOpaque(RoomDrawList[i]);
	}
	
	if(!player.m_improve) {
		ARXDRAW_DrawInterShadows();
	}
	
	ARX_THROWN_OBJECT_Render();
	
	GRenderer->GetTextureStage(0)->setMipMapLODBias(-0.6f);
	
	RenderInter();
	
	GRenderer->GetTextureStage(0)->setMipMapLODBias(-0.3f);
	
	// To render Dragged objs
	if(DRAGINTER) {
		ARX_INTERFACE_RenderCursor(false, true);
	}
	
	PopAllTriangleListOpaque();
	
	// *Now* draw the player
	if(entities.player()->animlayer[0].cur_anim) {
		float invisibility = std::min(0.9f, entities.player()->invisibility);
		AnimatedEntityRender(entities.player(), invisibility);
		if(!EXTERNALVIEW) {
			// In first person mode, always render the player over other objects
			// in order to avoid clipping the player and weapon with walls.
			PopAllTriangleListOpaque(render3D().depthTest(false), /*clear=*/false);
		}
		PopAllTriangleListOpaque();
	}
	
	ARXDRAW_DrawEyeBall();
	
	PolyBoomDraw();
	
	PopAllTriangleListTransparency();
	
	GRenderer->SetFogColor(Color::none);
	
	for(size_t i = 0; i < RoomDrawList.size(); i++) {
		BackgroundRenderTransparent(RoomDrawList[i]);
	}
	
	RenderWater();
	RenderLava();
	
	GRenderer->SetFogColor(g_fogColor);
	GRenderer->GetTextureStage(0)->setColorOp(TextureStage::OpModulate);
	
	Halo_Render();
	
}

