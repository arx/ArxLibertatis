/*
 * Copyright 2011-2022 Arx Libertatis Team (see the AUTHORS file)
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

#include <cmath>
#include <cstdio>
#include <array>
#include <memory>
#include <utility>
#include <vector>

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

#include "gui/Cursor.h"
#include "gui/Dragging.h"
#include "gui/Interface.h"
#include "gui/hud/PlayerInventory.h"
#include "gui/hud/SecondaryInventory.h"

#include "graphics/Draw.h"
#include "graphics/DrawLine.h"
#include "graphics/GlobalFog.h"
#include "graphics/Math.h"
#include "graphics/Raycast.h"
#include "graphics/VertexBuffer.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/BlobShadow.h"
#include "graphics/effects/Decal.h"
#include "graphics/effects/Halo.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/texture/Texture.h"
#include "graphics/texture/TextureStage.h"

#include "input/Input.h"

#include "io/log/Logger.h"

#include "scene/Interactive.h"
#include "scene/Light.h"
#include "scene/Rooms.h"
#include "scene/Tiles.h"

#include "physics/Projectile.h"

#include "platform/profiler/Profiler.h"

#include "util/Range.h"


extern bool EXTERNALVIEW; // *sigh*

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
		: vertices(nullptr)
		, start(0)
		, nbindices(0)
		, offset(0)
	{ }
	
	void lock(size_t count) {
		
		arx_assert(!vertices);
		
		if(indices.empty()) {
			indices.resize(4 * pDynamicVertexBuffer->m_buffer->capacity());
			start = 0;
		}
		
		BufferFlags flags = (pDynamicVertexBuffer->pos == 0) ? DiscardBuffer : NoOverwrite | DiscardRange;
		
		vertices =  pDynamicVertexBuffer->m_buffer->lock(flags, pDynamicVertexBuffer->pos, count);
		offset = 0;
	}
	
	SMY_VERTEX3 * append(size_t nbvertices) {
		
		arx_assert(vertices);
		
		if(pDynamicVertexBuffer->pos + nbvertices > pDynamicVertexBuffer->m_buffer->capacity()) {
			return nullptr;
		}
		
		SMY_VERTEX3 * pos = vertices + offset;
		
		pDynamicVertexBuffer->pos += nbvertices, offset += nbvertices;
		
		return pos;
	}
	
	void unlock() {
		arx_assert(vertices);
		pDynamicVertexBuffer->m_buffer->unlock(), vertices = nullptr;
	}
	
	void draw(Renderer::Primitive primitive) {
		arx_assert(!vertices);
		pDynamicVertexBuffer->m_buffer->drawIndexed(primitive, pDynamicVertexBuffer->pos - start, start,
		                                      indices.data(), nbindices);
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

Plane efpPlaneNear;
static EERIE_FRUSTRUM g_screenFrustum;

static std::vector<EERIEPOLY *> vPolyWater;
static std::vector<EERIEPOLY *> vPolyLava;

Vec2f getWaterFxUvOffset(float watereffect, const Vec3f & odtv) {
	return Vec2f(std::sin(watereffect + odtv.x), std::cos(watereffect + odtv.z));
}

static void ApplyLavaGlowToVertex(const Vec3f & odtv, ColorRGBA & color, float power) {
	power = 1.f - std::sin(WATEREFFECT + odtv.x + odtv.z) * 0.05f * power;
	color = (Color4f::fromRGBA(color) * power).toRGBA();
}

static void ManageWater_VertexBuffer(EERIEPOLY * ep, float uvScroll, SMY_VERTEX * vertices) {
	
	size_t count = (ep->type & POLY_QUAD) ? 4 : 3;
	for(size_t i = 0; i < count; i++) {
		Vec2f uv = ep->v[i].uv;
		uv += getWaterFxUvOffset(WATEREFFECT, ep->v[i].p) * (0.35f * 0.05f);
		if(ep->type & POLY_FALL) {
			uv.y -= uvScroll;
		}
		vertices[ep->uslInd[i]].uv = uv;
	}
	
}

static void ManageLava_VertexBuffer(EERIEPOLY * ep, float uvScroll, SMY_VERTEX * vertices) {
	
	size_t count = (ep->type & POLY_QUAD) ? 4 : 3;
	for(size_t i = 0; i < count; i++) {
		Vec2f uv = ep->v[i].uv;
		uv += getWaterFxUvOffset(WATEREFFECT, ep->v[i].p) * (0.35f * 0.05f);
		ApplyLavaGlowToVertex(ep->v[i].p, ep->color[i], 0.6f);
		if(ep->type & POLY_FALL) {
			uv.y -= uvScroll;
		}
		vertices[ep->uslInd[i]].uv = uv;
	}
	
}

bool IsSphereInFrustrum(const Vec3f & point, const EERIE_FRUSTRUM & frustrum, float radius = 0.f);

static bool FrustrumsClipSphere(const RoomData::Frustums & frustums, const Sphere & sphere) {
	
	float dists = distanceToPoint(efpPlaneNear, sphere.origin);
	
	if(dists + sphere.radius > 0) {
		for(const EERIE_FRUSTRUM & frustum : frustums) {
			if(IsSphereInFrustrum(sphere.origin, frustum, sphere.radius))
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
	
	if(!IsSphereInFrustrum(sphere.origin, g_screenFrustum, sphere.radius)) {
		return false;
	}
	
	if(RoomHandle room = ARX_PORTALS_GetRoomNumForPosition(sphere.origin)) {
		arx_assert(g_rooms);
		if(FrustrumsClipSphere(g_rooms->frustums[room], sphere)) {
			return false;
		}
	}
	
	return true;
}

static bool IsBBoxInFrustrum(const EERIE_3D_BBOX & bbox, const EERIE_FRUSTRUM & frustum) {
	
	const std::array<Vec3f, 8> corners = {
		bbox.min,
		Vec3f(bbox.max.x, bbox.min.y, bbox.min.z),
		Vec3f(bbox.min.x, bbox.max.y, bbox.min.z),
		Vec3f(bbox.max.x, bbox.max.y, bbox.min.z),
		Vec3f(bbox.min.x, bbox.min.y, bbox.max.z),
		Vec3f(bbox.max.x, bbox.min.y, bbox.max.z),
		Vec3f(bbox.min.x, bbox.max.y, bbox.max.z),
		bbox.max
	};
	
	for(const Plane & plane : frustum.plane) {
		bool intersect = false;
		for(Vec3f corner : corners) {
			if(distanceToPoint(plane, corner) >= 0.f) {
				intersect = true;
				break;
			}
		}
		if(!intersect) {
			return false;
		}
	}
	
	return true;
}

static bool FrustrumsClipBBox3D(const RoomData::Frustums & frustums, const EERIE_3D_BBOX & bbox) {
	
	for(const EERIE_FRUSTRUM & frustum : frustums) {
		if(IsBBoxInFrustrum(bbox, frustum)) {
			return false;
		}
	}
	
	return true;
}

// USAGE/FUNCTION
//   io can be nullptr if io is valid io->bbox3D contains 3D world-bbox
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
	
	Sphere sphere;
	if(io) {
		sphere.origin = (io->bbox3D.min + io->bbox3D.max) / 2.f;
		sphere.radius = glm::distance(sphere.origin, io->bbox3D.min) + 10.f;
		if(!IsSphereInFrustrum(sphere.origin, g_screenFrustum, sphere.radius) ||
		   !IsBBoxInFrustrum(io->bbox3D, g_screenFrustum)) {
			io->bbox2D.min = Vec2f(-1.f, -1.f);
			io->bbox2D.max = Vec2f(-1.f, -1.f);
			return true;
		}
	}
	
	if(!g_rooms) {
		return false;
	}
	
	Vec3f posi = position + Vec3f(0, -60, 0); // -20 ?
	
	RoomHandle room;
	if(io) {
		if(io->requestRoomUpdate) {
			UpdateIORoom(io);
		}
		room = io->room;
	} else {
		room = ARX_PORTALS_GetRoomNumForPosition(posi);
	}
	if(!room) {
		posi.y = position.y - 120;
		room = ARX_PORTALS_GetRoomNumForPosition(posi);
	}
	if(!room) {
		return false;
	}
	
	arx_assume(size_t(room) < g_rooms->frustums.size());
	
	const RoomData::Frustums & frustums = g_rooms->frustums[room];
	if(frustums.empty()) {
		if(io) {
			io->bbox2D.min = Vec2f(-1.f, -1.f);
			io->bbox2D.max = Vec2f(-1.f, -1.f);
		}
		return true;
	}
	
	if(io) {
		// TODO also use the portals from intermediate rooms for clipping
		if(FrustrumsClipSphere(frustums, sphere) || FrustrumsClipBBox3D(frustums, io->bbox3D)) {
			io->bbox2D.min = Vec2f(-1.f, -1.f);
			io->bbox2D.max = Vec2f(-1.f, -1.f);
			return true;
		}
	}
	
	return false;
}

static bool isInCameraFrustum(Vec3f pos, float radius = 0.f) {
	
	float dist = distanceToPoint(efpPlaneNear, pos);
	if(dist + radius < 0.f) {
		return false;
	}
	if(dist - radius - 10.f > g_camera->cdepth * fZFogEnd) {
		return false;
	}
	
	return IsSphereInFrustrum(pos, g_screenFrustum, radius);
}

static bool isPointVisible(Vec3f pos, const Entity * owner = nullptr) {
	
	arx_assert(g_camera);
	
	if(!isInCameraFrustum(pos)) {
		return false;
	}
	
	PolyType transparent = POLY_WATER | POLY_TRANS;
	RaycastFlags flags = 0;
	if(g_camera == &g_playerCamera) {
		flags |= RaycastIgnorePlayer;
	}
	
	RaycastResult sceneHit = raycastScene(g_camera->m_pos, pos, transparent, flags | RaycastAnyHit);
	if(sceneHit && fartherThan(sceneHit.pos, pos, 1.f)) {
		return false;
	}
	
	EntityRaycastResult hit = raycastEntities(g_camera->m_pos, pos, transparent, flags);
	if(hit && hit.entity != owner) {
		return false;
	}
	
	return true;
}

static bool isOccludedByPortals(Entity & entity, float dist2, RoomHandle currentRoom, RoomHandle cameraRoom) {
	
	arx_assume(currentRoom && size_t(currentRoom) < g_rooms->rooms.size());
	
	Sphere sphere;
	sphere.origin = (entity.bbox3D.min + entity.bbox3D.max) / 2.f;
	sphere.radius = glm::distance(sphere.origin, entity.bbox3D.min);
	
	if(FrustrumsClipSphere(g_rooms->frustums[currentRoom], sphere) ||
	   FrustrumsClipBBox3D(g_rooms->frustums[currentRoom], entity.bbox3D)) {
		return true;
	}
	
	const Room & room = g_rooms->rooms[currentRoom];
	for(PortalHandle portalIndex : room.portals) {
		
		arx_assume(portalIndex && size_t(portalIndex) < g_rooms->portals.size());
		const RoomPortal & portal = g_rooms->portals[portalIndex];
		if(portal.useportal != 1) {
			continue;
		}
		
		RoomHandle nextRoom = (portal.room0 == currentRoom) ? portal.room1 : portal.room0;
		if(nextRoom == cameraRoom) {
			return false;
		}
		
		float nextDist2 = arx::distance2(portal.bounds.origin, g_camera->m_pos);
		if(nextDist2 < dist2 && !isOccludedByPortals(entity, nextDist2, nextRoom, cameraRoom)) {
			return false;
		}
		
	}
	
	return true;
}

static bool isPointInViewCenter(Vec3f pos) {
	Vec3f cameraDirection = angleToVector(g_camera->angle);
	Vec3f direction = glm::normalize(pos - g_camera->m_pos);
	return (glm::dot(direction, cameraDirection) > glm::cos(glm::radians(70.f / 2.f)));
}

EntityVisibility getEntityVisibility(Entity & entity, bool cullingOnly) {
	
	switch(entity.show) {
		case SHOW_FLAG_IN_SCENE:     break;
		case SHOW_FLAG_LINKED:       break;
		case SHOW_FLAG_IN_INVENTORY: break;
		case SHOW_FLAG_HIDDEN:       return EntityInactive;
		case SHOW_FLAG_TELEPORTING:  return EntityVisibilityUnknown;
		case SHOW_FLAG_MEGAHIDE:     return EntityInactive;
		case SHOW_FLAG_ON_PLAYER:    break;
	}
	
	// Special handling for items in inventories
	if(InventoryPos slot = locateInInventories(&entity)) {
		if(g_playerInventoryHud.isSlotVisible(slot) || g_secondaryInventoryHud.isSlotVisible(slot)) {
			return EntityInFocus;
		} else {
			return EntityInactive;
		}
	}
	if((player.Interface & INTER_PLAYERBOOK) && IsEquipedByPlayer(&entity)) {
		return EntityInFocus;
	}
	if(&entity == g_draggedEntity) {
		return EntityInFocus;
	}
	if(entity.ioflags & IO_ICONIC) {
		return EntityInactive;
	}
	
	arx_assert(g_camera);
	arx_assert(g_tiles);
	
	// Point entities do not have a bounding box so need special treatment
	// On the other hand, a perfect occlusion test works with a single raycast.
	if(!entity.obj || (entity.ioflags & (IO_CAMERA | IO_MARKER))) {
		if(!g_tiles->isInActiveTile(entity.pos)) {
			return EntityInactive;
		}
		if(!isInCameraFrustum(entity.pos)) {
			return EntityNotInView;
		}
		if(!isPointVisible(entity.pos, &entity)) {
			return EntityFullyOccluded;
		}
		return isPointInViewCenter(entity.pos) ? EntityInFocus : EntityVisible;
	}
	
	// Linked entities do not have a valid position and never have the GFLAG_ISINTREATZONE game flag
	Entity * parent = &entity;
	if(entity.show == SHOW_FLAG_LINKED || entity.show == SHOW_FLAG_ON_PLAYER) {
		for(Entity & other : entities.inScene(~(IO_CAMERA | IO_MARKER))) {
			if(other != entity && other.obj) {
				for(const EERIE_LINKED & link : other.obj->linked) {
					if(link.io == &entity) {
						if(&other == entities.player() && !EXTERNALVIEW &&
						   link.lidx == entities.player()->obj->fastaccess.weapon_attach) {
							return EntityInactive;
						}
						parent = &other;
						break;
					}
				}
			}
			if(parent != &entity) {
				break;
			}
		}
	}
	
	// Coarse culling based on inactive rooms / tiles
	if(!(parent->gameFlags & GFLAG_ISINTREATZONE)) {
		return EntityInactive;
	}
	if(!g_tiles->isNearActiveTile(parent->pos)) {
		return EntityInactive;
	}
	
	// Frustum culling and portal occlusion
	Vec3f center = (entity.bbox3D.min + entity.bbox3D.max) / 2.f;
	if(g_camera != &g_playerCamera || &entity != entities.player()) {
		if(!isInCameraFrustum(center, glm::distance(center, entity.bbox3D.min))) {
			return EntityNotInView;
		}
		if(!IsBBoxInFrustrum(entity.bbox3D, g_screenFrustum)) {
			return EntityNotInView;
		}
		if(g_rooms && USE_PLAYERCOLLISIONS) {
			if(RoomHandle room = ARX_PORTALS_GetRoomNumForPosition(g_camera->m_pos, RoomPositionForCamera)) {
				arx_assume(size_t(room) < g_rooms->frustums.size());
				RoomHandle room2 = entity.room;
				if(!room2) {
					room2 = ARX_PORTALS_GetRoomNumForPosition(parent->pos - Vec3f(0.f, 120.f, 0.f));
				}
				if(room2 && room2 != room) {
					arx_assume(size_t(room2) < g_rooms->frustums.size());
					if(isOccludedByPortals(entity, arx::distance2(parent->pos, g_camera->m_pos), room2, room)) {
						return EntityFullyOccluded;
					}
				}
			}
		}
	}
	
	if(cullingOnly) {
		return EntityVisibilityUnknown;
	}
	
	// For full visibility use the head (or center if there is no head)
	Vec3f target = center;
	if(entity.obj->fastaccess.head_group_origin) {
		target = entity.obj->vertexWorldPositions[entity.obj->fastaccess.head_group_origin].v;
	}
	if(isPointVisible(target, &entity)) {
		return isPointInViewCenter(target) ? EntityInFocus : EntityVisible;
	}
	
	// If the head/center is not visible test all other groups
	for(const VertexGroup & group : entity.obj->grouplist) {
		if(group.origin != entity.obj->fastaccess.head_group_origin &&
		   isPointVisible(entity.obj->vertexWorldPositions[group.origin].v, &entity)) {
			if(!entity.obj->fastaccess.head_group_origin &&
			   isPointInViewCenter(entity.obj->vertexWorldPositions[group.origin].v)) {
				return EntityInFocus;
			}
			return EntityVisible;
		}
	}
	
	// For objects without 2 or fewer groups (i.e. non-NPCs), also test extreme vertices
	if(entity.obj->grouplist.size() <= 2) {
		Vec3f minX(std::numeric_limits<float>::max());
		Vec3f maxX(-std::numeric_limits<float>::max());
		Vec3f minY(std::numeric_limits<float>::max());
		Vec3f maxY(-std::numeric_limits<float>::max());
		Vec3f minZ(std::numeric_limits<float>::max());
		Vec3f maxZ(-std::numeric_limits<float>::max());
		for(const EERIE_VERTEX & vertex : entity.obj->vertexWorldPositions) {
			if(vertex.v.x < minX.x) {
				minX = vertex.v;
			}
			if(vertex.v.x > maxX.x) {
				maxX = vertex.v;
			}
			if(vertex.v.y < minY.y) {
				minY = vertex.v;
			}
			if(vertex.v.y > maxY.y) {
				maxY = vertex.v;
			}
			if(vertex.v.z < minZ.z) {
				minZ = vertex.v;
			}
			if(vertex.v.z > maxZ.z) {
				maxZ = vertex.v;
			}
		}
		if(isPointVisible(minY, &entity)) {
			return isPointInViewCenter(center) ? EntityInFocus : EntityVisible;
		}
		if(isPointVisible(maxY, &entity)) {
			return isPointInViewCenter(center) ? EntityInFocus : EntityVisible;
		}
		if(isPointVisible(minX, &entity)) {
			return isPointInViewCenter(center) ? EntityInFocus : EntityVisible;
		}
		if(isPointVisible(maxX, &entity)) {
			return isPointInViewCenter(center) ? EntityInFocus : EntityVisible;
		}
		if(isPointVisible(minZ, &entity)) {
			return isPointInViewCenter(center) ? EntityInFocus : EntityVisible;
		}
		if(isPointVisible(maxZ, &entity)) {
			return isPointInViewCenter(center) ? EntityInFocus : EntityVisible;
		}
	}
	
	// Not culled but we also found not visible vertex, give up
	return EntityVisibilityUnknown;
}

static EERIEPOLY * ARX_PORTALS_GetRoomNumForPosition2(const Vec3f & pos, bool xzOffset) {
	
	EERIEPOLY * ep = CheckInPoly(pos);
	if(ep && ep->room) {
		return ep;
	}
	
	// Security... ?
	ep = GetMinPoly(pos);
	if(ep && ep->room) {
		return ep;
	}
	
	if(xzOffset) {
		
		float off = 20.f;
		
		ep = CheckInPoly(pos + Vec3f(-off, -off, 0.f));
		if(ep && ep->room) {
			return ep;
		}
		
		ep = CheckInPoly(pos + Vec3f(-off, -20, -off));
		if(ep && ep->room) {
			return ep;
		}
		
		ep = CheckInPoly(pos + Vec3f(-off, -20, off));
		if(ep && ep->room) {
			return ep;
		}
		
		ep = CheckInPoly(pos + Vec3f(off, -20, 0.f));
		if(ep && ep->room) {
			return ep;
		}
		
		ep = CheckInPoly(pos + Vec3f(off, -20, off));
		if(ep && ep->room) {
			return ep;
		}
		
		ep = CheckInPoly(pos + Vec3f(off, -20, -off));
		if(ep && ep->room) {
			return ep;
		}
		
	}
	
	return nullptr;
}

static EERIEPOLY * ARX_PORTALS_GetRoomNumForCamera(const Vec3f & pos, const Vec3f & direction) {
	
	{
		EERIEPOLY * ep = CheckInPoly(pos);
		if(ep && ep->room) {
			return ep;
		}
	}
	
	{
		EERIEPOLY * ep = GetMinPoly(pos);
		if(ep && ep->room) {
			return ep;
		}
	}
	
	float dist = 0.f;
	while(dist <= 20.f) {
		
		Vec3f tmpPos = pos;
		tmpPos += direction * dist;
		
		EERIEPOLY * ep = CheckInPoly(tmpPos);
		
		if(ep && ep->room) {
			return ep;
		}
		
		dist += 5.f;
	}
	
	return nullptr;
}

// flag==1 for player
RoomHandle ARX_PORTALS_GetRoomNumForPosition(const Vec3f & pos, RoomPositionMode mode) {
	
	ARX_PROFILE_FUNC();
	
	RoomHandle result;
	float height = 0.f;
	
	if(mode == RoomPositionForCamera) {
		Vec3f direction = angleToVectorXZ_180offset(g_camera->angle.getYaw());
		EERIEPOLY * face = ARX_PORTALS_GetRoomNumForCamera(g_camera->m_pos, direction);
		if(face) {
			result = face->room;
			height = face->center.y;
		}
	} else {
		EERIEPOLY * face = ARX_PORTALS_GetRoomNumForPosition2(pos, mode == RoomPositionXZOffset);
		if(face) {
			result = face->room;
			height = face->center.y;
		}
	}
	
	if(result) {
		
		arx_assert(size_t(result) < g_rooms->rooms.size());
		
		RoomHandle nearest;
		float nearest_dist = 99999.f;
		
		for(const Room & room : g_rooms->rooms) {
			for(PortalHandle portalIndex : room.portals) {
				arx_assume(portalIndex && size_t(portalIndex) < g_rooms->portals.size());
				const RoomPortal & portal = g_rooms->portals[portalIndex];
				if(PointIn2DPolyXZ(portal, pos.x, pos.z)) {
					float yy;
					if(GetTruePolyY(portal, pos, &yy)) {
						if(height > yy) {
							if(yy >= pos.y && yy - pos.y < nearest_dist) {
								if(portal.plane.normal.y > 0) {
									nearest = portal.room1;
								} else {
									nearest = portal.room0;
								}
								nearest_dist = yy - pos.y;
							}
						}
					}
				}
			}
		}
		
		if(nearest) {
			arx_assert(size_t(nearest) < g_rooms->rooms.size());
			result = nearest;
		}
		
	}
	
	return result;
}

static void ARX_PORTALS_Frustrum_ClearIndexCount(RoomHandle room) {
	
	for(TextureContainer & material : util::dereference(g_rooms->rooms[room].ppTextureContainer)) {
		SMY_ARXMAT & roomMat = material.m_roomBatches[room];
		roomMat.indexCounts[BatchBucket_Opaque] = 0;
		roomMat.indexCounts[BatchBucket_Blended] = 0;
		roomMat.indexCounts[BatchBucket_Multiplicative] = 0;
		roomMat.indexCounts[BatchBucket_Additive] = 0;
		roomMat.indexCounts[BatchBucket_Subtractive] = 0;
	}
	
}

static void ARX_PORTALS_InitDrawnRooms() {
	
	ARX_PROFILE_FUNC();
	
	arx_assert(g_rooms);
	
	for(RoomPortal & portal : g_rooms->portals) {
		portal.useportal = 0;
	}
	
	for(RoomHandle room : g_rooms->rooms.handles()) {
		ARX_PORTALS_Frustrum_ClearIndexCount(room);
	}
	
	g_rooms->frustums.resize(g_rooms->rooms.size());
	for(RoomData::Frustums & frustums : g_rooms->frustums) {
		frustums.clear();
	}
	
	g_rooms->visibleRooms.clear();
	
	vPolyWater.clear();
	vPolyLava.clear();
	
	if(pDynamicVertexBuffer) {
		pDynamicVertexBuffer->m_buffer->setData(nullptr, 0, 0, DiscardBuffer);
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

static bool FrustrumsClipPoly(const RoomData::Frustums & frustums, const EERIEPOLY & ep) {
	
	for(const EERIE_FRUSTRUM & frustum : frustums) {
		if(IsSphereInFrustrum(ep.center, frustum, ep.v[0].w)) {
			return false;
		}
	}
	
	return true;
}

static EERIE_FRUSTRUM createFrustum(const Vec3f & pos, const RoomPortal & portal, bool cull) {
	
	EERIE_FRUSTRUM frustrum;
	
	if(cull) {
		frustrum.plane[0] = createNormalizedPlane(pos, portal.p[0], portal.p[1]);
		frustrum.plane[1] = createNormalizedPlane(pos, portal.p[3], portal.p[2]);
		frustrum.plane[2] = createNormalizedPlane(pos, portal.p[1], portal.p[3]);
		frustrum.plane[3] = createNormalizedPlane(pos, portal.p[2], portal.p[0]);
	} else {
		frustrum.plane[0] = createNormalizedPlane(pos, portal.p[1], portal.p[0]);
		frustrum.plane[1] = createNormalizedPlane(pos, portal.p[2], portal.p[3]);
		frustrum.plane[2] = createNormalizedPlane(pos, portal.p[3], portal.p[1]);
		frustrum.plane[3] = createNormalizedPlane(pos, portal.p[0], portal.p[2]);
	}
	
	return frustrum;
}

static void CreateScreenFrustrum() {
	
	glm::mat4x4 worldToClip = g_preparedCamera.m_viewToClip * g_preparedCamera.m_worldToView;
	
	{
		Plane & plane = g_screenFrustum.plane[0];
		plane.normal.x = worldToClip[0][3] - worldToClip[0][0];
		plane.normal.y = worldToClip[1][3] - worldToClip[1][0];
		plane.normal.z = worldToClip[2][3] - worldToClip[2][0];
		plane.offset   = worldToClip[3][3] - worldToClip[3][0];
		normalizePlane(plane);
	}
	
	{
		Plane & plane = g_screenFrustum.plane[1];
		plane.normal.x = worldToClip[0][3] + worldToClip[0][0];
		plane.normal.y = worldToClip[1][3] + worldToClip[1][0];
		plane.normal.z = worldToClip[2][3] + worldToClip[2][0];
		plane.offset   = worldToClip[3][3] + worldToClip[3][0];
		normalizePlane(plane);
	}
	
	{
		Plane & plane = g_screenFrustum.plane[2];
		plane.normal.x = worldToClip[0][3] - worldToClip[0][1];
		plane.normal.y = worldToClip[1][3] - worldToClip[1][1];
		plane.normal.z = worldToClip[2][3] - worldToClip[2][1];
		plane.offset   = worldToClip[3][3] - worldToClip[3][1];
		normalizePlane(plane);
	}
	
	{
		Plane & plane = g_screenFrustum.plane[3];
		plane.normal.x = worldToClip[0][3] + worldToClip[0][1];
		plane.normal.y = worldToClip[1][3] + worldToClip[1][1];
		plane.normal.z = worldToClip[2][3] + worldToClip[2][1];
		plane.offset   = worldToClip[3][3] + worldToClip[3][1];
		normalizePlane(plane);
	}
	
	{
		Plane & plane = efpPlaneNear;
		plane.normal.x = worldToClip[0][3] + worldToClip[0][2];
		plane.normal.y = worldToClip[1][3] + worldToClip[1][2];
		plane.normal.z = worldToClip[2][3] + worldToClip[2][2];
		plane.offset   = worldToClip[3][3] + worldToClip[3][2];
		normalizePlane(plane);
	}
	
}

void RoomDrawRelease() {
	if(g_rooms) {
		g_rooms->visibleRooms.clear();
		g_rooms->frustums.clear();
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

static Vec2f FluidTextureDisplacement(const Vec3f & p, GameInstant time,
                                      float divVar1, float divVar2, GameDuration duration,
                                      float divVar4, float addVar1,
                                      float addVar2, Vec2f sign) {
	
	float offset = timeWaveSaw(time, duration) * 2.f * glm::pi<float>();
	
	float u = (p.x + addVar1) / divVar1 + sign.x * glm::sin((p.x + addVar2) / divVar2 + offset) / divVar4;
	float v = (p.z + addVar1) / divVar1 + sign.y * glm::cos((p.z + addVar2) / divVar2 + offset) / divVar4;
	
	return Vec2f(u, v);
}

static Vec2f CalculateWaterDisplacement(EERIEPOLY * ep, GameInstant time, int vertIndex, int step) {
	
	const Vec3f & p = ep->v[vertIndex].p;
	GameDuration duration = 2s * glm::pi<float>();
	
	switch(step) {
	case 0: {
		return FluidTextureDisplacement(p, time, 1000, 200, duration, 32, 0.f, 0.f, Vec2f(1.f, 1.f));
	}
	case 1: {
		return FluidTextureDisplacement(p, time, 1000, 200, duration, 28, 30.f, 30, Vec2f(1.f, -1.f));
	}
	case 2: {
		return FluidTextureDisplacement(p, time, 1000, 200, duration, 40, 60.f, 60, Vec2f(-1.f, -1.f));
	}
	default:
		return Vec2f(0.f);
	}
}

static Vec2f CalculateLavaDisplacement(EERIEPOLY * ep, GameInstant time, int vertIndex, int step) {
	
	const Vec3f & p = ep->v[vertIndex].p;
	GameDuration duration = 4s * glm::pi<float>();
	
	switch(step) {
	case 0: {
		return FluidTextureDisplacement(p, time, 1000, 200, duration, 20, 0.f, 0.f, Vec2f(1.f, 1.f));
	}
	case 1: {
		return FluidTextureDisplacement(p, time, 1000, 100, duration, 10, 0.f, 0.f, Vec2f(1.f, 1.f));
	}
	case 2: {
		return FluidTextureDisplacement(p, time, 600, 160, duration, 11, 0.f, 0.f, Vec2f(1.f, 1.f));
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
	
	UseRenderState state(render3D().depthWrite(false).cull().depthOffset(8).blend(BlendDstColor, BlendOne));
	
	GRenderer->SetTexture(0, enviro);
	GRenderer->SetTexture(1, enviro);
	GRenderer->SetTexture(2, enviro);
	
	unsigned short * indices = dynamicVertices.indices.data();
	
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
			indices = dynamicVertices.indices.data();
			pVertex = dynamicVertices.append(iNbVertex);
		}
		
		for(int j = 0; j < iNbVertex; ++j) {
			pVertex->p = ep->v[j].p;
			pVertex->color = Color::gray(0.314f).toRGBA();
			
			for(int i = 0; i < FTVU_STEP_COUNT; ++i) {
				Vec2f uv = CalculateWaterDisplacement(ep, g_gameTime.now(), j, i);
				if(ep->type & POLY_FALL) {
					uv.y += timeWaveSaw(g_gameTime.now(), 4s);
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
	
	RenderState baseState = render3D().depthWrite(false).cull().depthOffset(8);
	
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
	
	unsigned short * indices = dynamicVertices.indices.data();
	
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
			indices = dynamicVertices.indices.data();
			pVertex = dynamicVertices.append(iNbVertex);
		}
		
		for(int j = 0; j < iNbVertex; ++j) {
			pVertex->p = ep->v[j].p;
			pVertex->color = Color::gray(0.4f).toRGBA();
			for(int i = 0; i < FTVU_STEP_COUNT; ++i) {
				Vec2f uv = CalculateLavaDisplacement(ep, g_gameTime.now(), j, i);
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

static void ARX_PORTALS_Frustrum_RenderRoomTCullSoft(RoomHandle roomIndex, const Vec3f & camPos) {
	
	ARX_PROFILE_FUNC();
	
	const RoomData::Frustums & frustums = g_rooms->frustums[roomIndex];
	if(frustums.empty()) {
		return;
	}
	
	Room & room = g_rooms->rooms[roomIndex];
	if(!room.pVertexBuffer) {
		// No need to spam this for every frame as there will already be an
		// earlier warning
		LogDebug("no vertex data for room " << roomIndex);
		return;
	}
	
	SMY_VERTEX * pMyVertex = room.pVertexBuffer->lock(NoOverwrite);
	
	for(const EP_DATA & epd : room.epdata) {
		
		auto tile = g_tiles->get(epd.tile);
		if(!tile.active()) {
			for(auto neighbour : g_tiles->tilesAround(tile, 1)) {
				if(!neighbour.active()) {
					neighbour.setActive();
					ComputeTileLights(neighbour);
				}
			}
		}
		
		EERIEPOLY * ep = &tile.polygons()[epd.idx];
		if(!ep->tex) {
			continue;
		}
		
		if(ep->type & (POLY_IGNORE | POLY_NODRAW | POLY_HIDE)) {
			continue;
		}
		
		if(FrustrumsClipPoly(frustums, *ep)) {
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
		
		SMY_ARXMAT & roomMat = ep->tex->m_roomBatches[roomIndex];
		
		u32 & indexCount = roomMat.indexCounts[transparencyType];
		unsigned short * indices = room.indexBuffer.data() + roomMat.indexOffsets[transparencyType] + indexCount;
		
		*indices++ = ep->uslInd[0];
		*indices++ = ep->uslInd[1];
		*indices++ = ep->uslInd[2];
		indexCount += 3;
		
		if(to == 4) {
			*indices++ = ep->uslInd[3];
			*indices++ = ep->uslInd[2];
			*indices++ = ep->uslInd[1];
			indexCount += 3;
		}
		
		SMY_VERTEX * vertices = &pMyVertex[roomMat.vertexOffset];
		
		if(!player.m_improve) { // Normal View...
			
			if(ep->type & POLY_GLOW) {
				vertices[ep->uslInd[0]].color = Color::white.toRGBA();
				vertices[ep->uslInd[1]].color = Color::white.toRGBA();
				vertices[ep->uslInd[2]].color = Color::white.toRGBA();
				if(to == 4) {
					vertices[ep->uslInd[3]].color = Color::white.toRGBA();
				}
			} else  if(!(ep->type & POLY_TRANS)) {
				ApplyTileLights(ep, epd.tile);
				vertices[ep->uslInd[0]].color = ep->color[0];
				vertices[ep->uslInd[1]].color = ep->color[1];
				vertices[ep->uslInd[2]].color = ep->color[2];
				if(to & 4) {
					vertices[ep->uslInd[3]].color = ep->color[3];
				}
			}
			
		} else if(!(ep->type & POLY_TRANS)) { // Improve Vision Activated
			
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
			vertices[ep->uslInd[0]].color = ep->color[0];
			vertices[ep->uslInd[1]].color = ep->color[1];
			vertices[ep->uslInd[2]].color = ep->color[2];
			if(to == 4) {
				vertices[ep->uslInd[3]].color = ep->color[3];
			}
			
		}
		
		if(ep->type & POLY_LAVA) {
			float uvScroll = timeWaveSaw(g_gameTime.now(), 12s);
			ManageLava_VertexBuffer(ep, uvScroll, vertices);
			vPolyLava.push_back(ep);
		} else if(ep->type & POLY_WATER) {
			float uvScroll = timeWaveSaw(g_gameTime.now(), 1s);
			ManageWater_VertexBuffer(ep, uvScroll, vertices);
			vPolyWater.push_back(ep);
		}
		
	}
	
	room.pVertexBuffer->unlock();
	
}

static void BackgroundRenderOpaque(RoomHandle roomIndex) {
	
	ARX_PROFILE_FUNC();
	
	Room & room = g_rooms->rooms[roomIndex];
	
	for(TextureContainer & material : util::dereference(room.ppTextureContainer)) {
		
		const SMY_ARXMAT & roomMat = material.m_roomBatches[roomIndex];
		if(!roomMat.indexCounts[BatchBucket_Opaque]) {
			continue;
		}
		
		GRenderer->SetTexture(0, &material);
		
		RenderState baseState = render3D();
		baseState.setAlphaCutout(material.m_pTexture && material.m_pTexture->hasAlpha());
		UseRenderState state(baseState);
		
		if(material.userflags & POLY_METAL) {
			GRenderer->GetTextureStage(0)->setColorOp(TextureStage::OpModulate2X);
		} else {
			GRenderer->GetTextureStage(0)->setColorOp(TextureStage::OpModulate);
		}
		
		room.pVertexBuffer->drawIndexed(Renderer::TriangleList, roomMat.vertexCount, roomMat.vertexOffset,
		                                room.indexBuffer.data() + roomMat.indexOffsets[BatchBucket_Opaque],
		                                roomMat.indexCounts[BatchBucket_Opaque]);
		
		EERIEDrawnPolys += roomMat.indexCounts[BatchBucket_Opaque];
		
	}
	
	GRenderer->GetTextureStage(0)->setColorOp(TextureStage::OpModulate);
	
}

static constexpr std::array<BatchBucket, 4> transRenderOrder = {
	BatchBucket_Blended,
	BatchBucket_Multiplicative,
	BatchBucket_Additive,
	BatchBucket_Subtractive
};

static void BackgroundRenderTransparent(RoomHandle roomIndex) {
	
	ARX_PROFILE_FUNC();
	
	Room & room = g_rooms->rooms[roomIndex];
	
	for(TextureContainer & material : util::dereference(room.ppTextureContainer)) {
		
		SMY_ARXMAT & roomMat = material.m_roomBatches[roomIndex];
		bool empty = std::all_of(transRenderOrder.begin(), transRenderOrder.end(), [&](BatchBucket transType) {
			return !roomMat.indexCounts[transType];
		});
		if(empty) {
			continue;
		}
		
		GRenderer->SetTexture(0, &material);
		
		RenderState baseState = render3D().depthWrite(false).depthOffset(2);
		baseState.setAlphaCutout(material.m_pTexture && material.m_pTexture->hasAlpha());
		
		for(BatchBucket transType : transRenderOrder) {
			
			if(!roomMat.indexCounts[transType]) {
				continue;
			}
			
			RenderState desiredState = baseState;
			switch(transType) {
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
				case BatchBucket_Opaque: {
					// This should currently not happen
					arx_unreachable();
				}
			}
			
			UseRenderState state(desiredState);
			room.pVertexBuffer->drawIndexed(Renderer::TriangleList, roomMat.vertexCount, roomMat.vertexOffset,
			                                room.indexBuffer.data() + roomMat.indexOffsets[transType],
			                                roomMat.indexCounts[transType]);
			
			EERIEDrawnPolys += roomMat.indexCounts[transType];
			
		}
		
	}
	
}

static void ARX_PORTALS_Frustrum_ComputeRoom(RoomHandle roomIndex,
                                             const EERIE_FRUSTRUM & frustrum,
                                             const Vec3f & camPos, float camDepth) {
	
	arx_assume(roomIndex && size_t(roomIndex) < g_rooms->rooms.size());
	
	if(g_rooms->frustums[roomIndex].empty()) {
		g_rooms->visibleRooms.push_back(roomIndex);
	}
	
	g_rooms->frustums[roomIndex].push_back(frustrum);
	
	float fClippZFar = camDepth * fZFogEnd * 1.1f;
	
	// Now Checks For room Portals !!!
	for(PortalHandle portalIndex : g_rooms->rooms[roomIndex].portals) {
		
		arx_assume(portalIndex && size_t(portalIndex) < g_rooms->portals.size());
		RoomPortal & portal = g_rooms->portals[portalIndex];
		if(portal.useportal) {
			continue;
		}
		
		unsigned char ucVisibilityNear = 0;
		unsigned char ucVisibilityFar = 0;
		for(Vec3f corner : portal.p) {
			float fDist0 = distanceToPoint(efpPlaneNear, corner);
			if(fDist0 < 0.f) {
				ucVisibilityNear++;
			}
			if(fDist0 > fClippZFar) {
				ucVisibilityFar++;
			}
		}
		if(ucVisibilityFar ==  std::size(portal.p) || ucVisibilityNear == std::size(portal.p)) {
			portal.useportal = 2;
			continue;
		}
		
		Vec3f pos = portal.bounds.origin - camPos;
		float fRes = glm::dot(pos, portal.plane.normal);
		
		if(!IsSphereInFrustrum(portal.bounds.origin, frustrum, portal.bounds.radius)) {
			continue;
		}
		// TODO Ideally we would also check portal frustums from intermediate rooms
		// like in isOccludedByPortals() but those may be incomplete at this point.
		if(!IsSphereInFrustrum(portal.bounds.origin, g_screenFrustum, portal.bounds.radius)) {
			continue;
		}
		
		bool Cull = !(fRes < 0.f);
		
		EERIE_FRUSTRUM fd = createFrustum(camPos, portal, Cull);
		
		RoomHandle roomToCompute;
		if(portal.room0 == roomIndex && !Cull) {
			roomToCompute = portal.room1;
		} else if(portal.room1 == roomIndex && Cull) {
			roomToCompute = portal.room0;
		}
		
		if(roomToCompute) {
			portal.useportal = 1;
			ARX_PORTALS_Frustrum_ComputeRoom(roomToCompute, fd, camPos, camDepth);
		}
		
	}
	
}

void ARX_SCENE_Update() {
	
	CreateScreenFrustrum();
	
	if(!g_rooms) {
		return;
	}
	
	ARX_PROFILE_FUNC();
	
	WATEREFFECT = timeWaveSaw(g_gameTime.now(), 12566370us) * 2.f * glm::pi<float>();
	
	const Vec3f camPos = g_camera->m_pos;
	const float camDepth = g_camera->cdepth;

	TreatBackgroundDynlights();
	PrecalcDynamicLighting(camPos, camDepth);
	
	g_tiles->resetActiveTiles();
	
	ARX_PORTALS_InitDrawnRooms();
	
	if(!USE_PLAYERCOLLISIONS) {
		for(RoomHandle room : g_rooms->rooms.handles()) {
			g_rooms->visibleRooms.push_back(room);
			g_rooms->frustums[room].push_back(g_screenFrustum);
		}
	} else if(RoomHandle room = ARX_PORTALS_GetRoomNumForPosition(camPos, RoomPositionForCamera)) {
		ARX_PORTALS_Frustrum_ComputeRoom(room, g_screenFrustum, camPos, camDepth);
	}
	
	for(RoomHandle room : g_rooms->visibleRooms) {
		ARX_PORTALS_Frustrum_RenderRoomTCullSoft(room, camPos);
	}
	
	ARX_THROWN_OBJECT_Manage(g_gameTime.lastFrameDuration());
	
	updateDraggedEntity();
	
	UpdateInter();
}

void ARX_SCENE_Render() {
	
	ARX_PROFILE_FUNC();
	
	if(uw_mode) {
		GRenderer->GetTextureStage(0)->setMipMapLODBias(10.f);
	}
	
	if(g_rooms) {
		for(RoomHandle room : g_rooms->visibleRooms) {
			BackgroundRenderOpaque(room);
		}
	}
	
	if(!player.m_improve) {
		ARXDRAW_DrawInterShadows();
	}
	
	ARX_THROWN_OBJECT_Render();
	
	GRenderer->GetTextureStage(0)->setMipMapLODBias(-0.6f);
	
	RenderInter();
	
	GRenderer->GetTextureStage(0)->setMipMapLODBias(-0.3f);
	
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
	
	GRenderer->SetFogColor(Color());
	
	if(g_rooms) {
		for(RoomHandle room : g_rooms->visibleRooms) {
			BackgroundRenderTransparent(room);
		}
	}
	
	RenderWater();
	RenderLava();
	
	GRenderer->SetFogColor(g_fogColor);
	GRenderer->GetTextureStage(0)->setColorOp(TextureStage::OpModulate);
	
	Halo_Render();
	
}

