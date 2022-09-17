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
#include "graphics/effects/PolyBoom.h"
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
	
	if(!IsSphereInFrustrum(sphere.origin, g_screenFrustum, sphere.radius)) {
		return false;
	}
	
	long room_num = ARX_PORTALS_GetRoomNumForPosition(sphere.origin);
	if(room_num >= 0) {
		arx_assert(g_rooms);
		if(FrustrumsClipSphere(g_rooms->visibility[room_num].frustrum, sphere)) {
			return false;
		}
	}
	
	return true;
}

static bool IsBBoxInFrustrum(const EERIE_3D_BBOX & bbox, const EERIE_FRUSTRUM & frustum) {
	
	const Vec3f corners[] = {
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

static bool FrustrumsClipBBox3D(const EERIE_FRUSTRUM_DATA & frustrums,
                                const EERIE_3D_BBOX & bbox) {
	
	for(long i = 0; i < frustrums.nb_frustrums; i++) {
		if(IsBBoxInFrustrum(bbox, frustrums.frustrums[i])) {
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
	
	if(g_rooms) {
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

		if(room_num >= 0 && size_t(room_num) < g_rooms->visibility.size()) {
			if(g_rooms->visibility[room_num].count == 0) {
				if(io) {
					io->bbox2D.min = Vec2f(-1.f, -1.f);
					io->bbox2D.max = Vec2f(-1.f, -1.f);
				}
				return true;
			}
			
			if(io) {
				const EERIE_FRUSTRUM_DATA & frustrums = g_rooms->visibility[room_num].frustrum;
				// TODO also use the portals from intermediate rooms for clipping
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

static bool isOccludedByPortals(Entity & entity, float dist2, size_t currentRoom, size_t cameraRoom) {
	
	Sphere sphere;
	sphere.origin = (entity.bbox3D.min + entity.bbox3D.max) / 2.f;
	sphere.radius = glm::distance(sphere.origin, entity.bbox3D.min);
	
	const EERIE_FRUSTRUM_DATA & frustrums = g_rooms->visibility[currentRoom].frustrum;
	if(FrustrumsClipSphere(frustrums, sphere) || FrustrumsClipBBox3D(frustrums, entity.bbox3D)) {
		return true;
	}
	
	const Room & room = g_rooms->rooms[currentRoom];
	for(long i : room.portals) {
		
		if(i < 0 || size_t(i) >= g_rooms->portals.size()) {
			continue;
		}
		const RoomPortal & portal = g_rooms->portals[i];
		if(portal.useportal != 1) {
			continue;
		}
		
		u32 nextRoom = (portal.room0 == currentRoom) ? portal.room1 : portal.room0;
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
		case SHOW_FLAG_NOT_DRAWN:    return EntityInactive;
		case SHOW_FLAG_IN_SCENE:     break;
		case SHOW_FLAG_LINKED:       break;
		case SHOW_FLAG_IN_INVENTORY: break;
		case SHOW_FLAG_HIDDEN:       return EntityInactive;
		case SHOW_FLAG_TELEPORTING:  return EntityVisibilityUnknown;
		case SHOW_FLAG_KILLED:       return EntityInactive;
		case SHOW_FLAG_MEGAHIDE:     return EntityInactive;
		case SHOW_FLAG_ON_PLAYER:    break;
		case SHOW_FLAG_DESTROYED:    return EntityInactive;
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
			long room = ARX_PORTALS_GetRoomNumForPosition(g_camera->m_pos, 1);
			if(room >= 0 && size_t(room) < g_rooms->visibility.size()) {
				long room2 = entity.room;
				if(room2 == -1) {
					room2 = ARX_PORTALS_GetRoomNumForPosition(parent->pos - Vec3f(0.f, 120.f, 0.f));
				}
				if(room2 >= 0 && size_t(room2) < g_rooms->visibility.size() && room2 != room) {
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
	if(entity.obj->fastaccess.head_group_origin != ObjVertHandle()) {
		target = entity.obj->vertexWorldPositions[entity.obj->fastaccess.head_group_origin.handleData()].v;
	}
	if(isPointVisible(target, &entity)) {
		return isPointInViewCenter(target) ? EntityInFocus : EntityVisible;
	}
	
	// If the head/center is not visible test all other groups
	for(const VertexGroup & group : entity.obj->grouplist) {
		if(ObjVertHandle(group.origin) != entity.obj->fastaccess.head_group_origin &&
		   isPointVisible(entity.obj->vertexWorldPositions[group.origin].v, &entity)) {
			if(entity.obj->fastaccess.head_group_origin == ObjVertHandle() &&
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
	
	return nullptr;
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
	
	return nullptr;
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
		
		for(const Room & room : g_rooms->rooms) {
			for(long i : room.portals) {
				const RoomPortal & portal = g_rooms->portals[i];
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
		
		if(nearest > -1) {
			num = nearest;
		}
		
	}
	
	return num;
}

static void ARX_PORTALS_Frustrum_ClearIndexCount(size_t room_num) {
	
	Room & room = g_rooms->rooms[room_num];
	
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
	
	arx_assert(g_rooms);
	
	for(RoomPortal & portal : g_rooms->portals) {
		portal.useportal = 0;
	}
	
	for(size_t i = 0; i < g_rooms->rooms.size(); i++) {
		ARX_PORTALS_Frustrum_ClearIndexCount(i);
	}
	
	g_rooms->visibility.resize(g_rooms->rooms.size());
	for(PORTAL_ROOM_DRAW & room : g_rooms->visibility) {
		room.count = 0;
		room.frustrum.nb_frustrums = 0;
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

static bool FrustrumsClipPoly(const EERIE_FRUSTRUM_DATA & frustrums,
                              const EERIEPOLY & ep) {
	
	for(long i = 0; i < frustrums.nb_frustrums; i++) {
		if(IsSphereInFrustrum(ep.center, frustrums.frustrums[i], ep.v[0].w))
			return false;
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
		g_rooms->visibility.clear();
	}
}

static void RoomFrustrumAdd(size_t num, const EERIE_FRUSTRUM & fr) {
	if(g_rooms->visibility[num].frustrum.nb_frustrums < MAX_FRUSTRUMS - 1) {
		g_rooms->visibility[num].frustrum.frustrums[g_rooms->visibility[num].frustrum.nb_frustrums] = fr;
		g_rooms->visibility[num].frustrum.nb_frustrums++;
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

static void ARX_PORTALS_Frustrum_RenderRoomTCullSoft(size_t room_num,
                                                     const EERIE_FRUSTRUM_DATA & frustrums,
                                                     GameInstant now,
                                                     const Vec3f & camPos
) {
	ARX_PROFILE_FUNC();
	
	if(!g_rooms->visibility[room_num].count) {
		return;
	}
	
	Room & room = g_rooms->rooms[room_num];
	if(!room.pVertexBuffer) {
		// No need to spam this for every frame as there will already be an
		// earlier warning
		LogDebug("no vertex data for room " << room_num);
		return;
	}
	
	SMY_VERTEX * pMyVertex = room.pVertexBuffer->lock(NoOverwrite);
	
	unsigned short * pIndices = room.indexBuffer.data();
	
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
			float uvScroll = timeWaveSaw(now, 12s);
			ManageLava_VertexBuffer(ep, to, uvScroll, pMyVertexCurr);
			vPolyLava.push_back(ep);
		} else if(ep->type & POLY_WATER) {
			float uvScroll = timeWaveSaw(now, 1s);
			ManageWater_VertexBuffer(ep, to, uvScroll, pMyVertexCurr);
			vPolyWater.push_back(ep);
		}
		
	}

	room.pVertexBuffer->unlock();
}


static void BackgroundRenderOpaque(size_t room_num) {
	
	ARX_PROFILE_FUNC();
	
	Room & room = g_rooms->rooms[room_num];
	
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

static constexpr std::array<BatchBucket, 4> transRenderOrder = {
	BatchBucket_Blended,
	BatchBucket_Multiplicative,
	BatchBucket_Additive,
	BatchBucket_Subtractive
};


static void BackgroundRenderTransparent(size_t room_num) {
	
	ARX_PROFILE_FUNC();
	
	Room & room = g_rooms->rooms[room_num];
	
	for(TextureContainer * pTexCurr : room.ppTextureContainer) {
		
		RenderState baseState = render3D().depthWrite(false).depthOffset(2);
		
		GRenderer->SetTexture(0, pTexCurr);
		baseState.setAlphaCutout(pTexCurr->m_pTexture && pTexCurr->m_pTexture->hasAlpha());
		
		SMY_ARXMAT & roomMat = pTexCurr->m_roomBatches[room_num];
		
		for(BatchBucket transType : transRenderOrder) {
			
			if(!roomMat.count[transType]) {
				continue;
			}
			
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
	arx_assert(roomIndex < g_rooms->rooms.size());
	
	if(g_rooms->visibility[roomIndex].count == 0) {
		g_rooms->visibleRooms.push_back(roomIndex);
	}
	
	RoomFrustrumAdd(roomIndex, frustrum);
	g_rooms->visibility[roomIndex].count++;
	
	float fClippZFar = camDepth * fZFogEnd * 1.1f;
	
	// Now Checks For room Portals !!!
	for(long i : g_rooms->rooms[roomIndex].portals) {
		RoomPortal & portal = g_rooms->portals[i];
		
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

		size_t roomToCompute = 0;
		bool computeRoom = false;

		if(portal.room0 == roomIndex && !Cull) {
			roomToCompute = portal.room1;
			computeRoom = true;
		}else if(portal.room1 == roomIndex && Cull) {
			roomToCompute = portal.room0;
			computeRoom = true;
		}
		
		if(computeRoom) {
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
	
	GameInstant now = g_gameTime.now();
	
	WATEREFFECT = (toMsi(now) % long(2 * glm::pi<float>() / 0.0005f)) * 0.0005f;
	
	const Vec3f camPos = g_camera->m_pos;
	const float camDepth = g_camera->cdepth;

	TreatBackgroundDynlights();
	PrecalcDynamicLighting(camPos, camDepth);
	
	g_tiles->resetActiveTiles();
	
	ARX_PORTALS_InitDrawnRooms();
	
	if(!USE_PLAYERCOLLISIONS) {
		for(size_t i = 0; i < g_rooms->rooms.size(); i++) {
			g_rooms->visibility[i].count = 1;
			g_rooms->visibleRooms.push_back(i);
			RoomFrustrumAdd(i, g_screenFrustum);
		}
	} else {
		long room_num = ARX_PORTALS_GetRoomNumForPosition(camPos, 1);
		if(room_num > -1) {
			size_t roomIndex = static_cast<size_t>(room_num);
			ARX_PORTALS_Frustrum_ComputeRoom(roomIndex, g_screenFrustum, camPos, camDepth);
		}
	}
	
	for(size_t room : g_rooms->visibleRooms) {
		ARX_PORTALS_Frustrum_RenderRoomTCullSoft(room, g_rooms->visibility[room].frustrum, now, camPos);
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
		for(size_t room : g_rooms->visibleRooms) {
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
	
	GRenderer->SetFogColor(Color::none);
	
	if(g_rooms) {
		for(size_t room : g_rooms->visibleRooms) {
			BackgroundRenderTransparent(room);
		}
	}
	
	RenderWater();
	RenderLava();
	
	GRenderer->SetFogColor(g_fogColor);
	GRenderer->GetTextureStage(0)->setColorOp(TextureStage::OpModulate);
	
	Halo_Render();
	
}

