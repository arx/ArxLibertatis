/*
 * Copyright 2013-2020 Arx Libertatis Team (see the AUTHORS file)
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

#include "gui/Dragging.h"

#include "core/Core.h"
#include "game/Camera.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/Inventory.h"
#include "game/Player.h"
#include "graphics/Math.h"
#include "gui/Hud.h"
#include "gui/Interface.h"
#include "gui/book/Book.h"
#include "gui/hud/PlayerInventory.h"
#include "gui/hud/SecondaryInventory.h"
#include "input/Input.h"
#include "math/Angle.h"
#include "math/GtxFunctions.h"
#include "math/Vector.h"
#include "physics/Collisions.h"
#include "physics/Physics.h"
#include "platform/Platform.h"
#include "platform/profiler/Profiler.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"

EntityDragStatus g_dragStatus = EntityDragStatus_Invalid;
Entity * g_draggedEntity = NULL;
InventoryPos g_draggedItemPreviousPosition;
Vec2f g_draggedIconOffset;
static Vec2f g_draggedObjectOffset;
static float g_dragStartAngle = 0;
static Camera * g_dragStartCamera = NULL;

void setDraggedEntity(Entity * entity) {
	
	if(entity != g_draggedEntity) {
		g_dragStartCamera = g_camera;
		g_dragStartAngle = g_camera->angle.getYaw();
	}
	
	if(entity) {
		g_draggedItemPreviousPosition = removeFromInventories(entity);
		if(entity->obj && entity->show == SHOW_FLAG_IN_SCENE && (entity->gameFlags & GFLAG_ISINTREATZONE)
		   && !(entity->gameFlags & (GFLAG_INVISIBILITY | GFLAG_MEGAHIDE))) {
			EERIE_3D_BBOX bbox;
			for(size_t i = 0; i < entity->obj->vertexlist.size(); i++) {
				bbox.add(entity->obj->vertexlist[i].v);
			}
			Vec3f center = bbox.min + (bbox.max - bbox.min) * Vec3f(0.5f);
			Anglef angle = entity->angle;
			angle.setYaw(270.f - angle.getYaw());
			center = toQuaternion(angle) * center;
			center.y = 0.f;
			Vec4f p = worldToClipSpace(entity->pos + center);
			if(p.w > 0.f) {
				Vec2f pos = Vec2f(p) / p.w;
				if(g_size.contains(Rect::Vec2(pos))) {
					g_draggedIconOffset = Vec2f(0.f);
					g_draggedObjectOffset = pos - Vec2f(DANAEMouse);
				}
			}
		}
		entity->show = g_draggedItemPreviousPosition ? SHOW_FLAG_ON_PLAYER : SHOW_FLAG_IN_SCENE;
	} else {
		g_draggedItemPreviousPosition = InventoryPos();
		g_draggedIconOffset = Vec2f(0.f);
		g_draggedObjectOffset = Vec2f(0.f);
	}
	
	g_draggedEntity = entity;
	
	if(entity && entity->obj && entity->obj->pbox) {
		entity->obj->pbox->active = 0;
	}
	
}

struct EntityDragResult {
	
	Vec3f offset;
	float height;
	
	bool foundSpot;
	Vec3f pos;
	float offsetY;
	
	bool foundCollision;
	
};

static EntityDragResult findSpotForDraggedEntity(Vec3f origin, Vec3f dir, Entity * entity, Sphere limit) {
	
	EntityDragResult result;
	result.foundSpot = false;
	result.foundCollision = false;
	
	EERIE_3D_BBOX bbox;
	for(size_t i = 0; i < entity->obj->vertexlist.size(); i++) {
		bbox.add(entity->obj->vertexlist[i].v);
	}
	
	result.offset = bbox.min + (bbox.max - bbox.min) * Vec3f(0.5f);
	result.height = std::max(bbox.max.y - bbox.min.y, 30.f);
	
	float maxdist = 0.f;
	for(size_t i = 0; i < entity->obj->vertexlist.size(); i++) {
		const EERIE_VERTEX & vert = entity->obj->vertexlist[i];
		float dist = glm::distance(Vec2f(result.offset.x, result.offset.z), Vec2f(vert.v.x, vert.v.z)) - 4.f;
		maxdist = std::max(maxdist, dist);
	}
	
	if(entity->obj->pbox) {
		Vec2f tmpVert(entity->obj->pbox->vert[0].initpos.x, entity->obj->pbox->vert[0].initpos.z);
		for(size_t i = 1; i < entity->obj->pbox->vert.size(); i++) {
			const PhysicsParticle & physVert = entity->obj->pbox->vert[i];
			float dist = glm::distance(tmpVert, Vec2f(physVert.initpos.x, physVert.initpos.z)) + 14.f;
			maxdist = std::max(maxdist, dist);
		}
	}
	
	Cylinder cyl(origin + Vec3f(0.f, bbox.max.y, 0.f), glm::clamp(maxdist, 20.f, 150.f), -result.height);
	
	float inc = 10.f;
	cyl.origin += dir * inc;
	
	if(!limit.contains(cyl.origin)) {
		float t;
		if(!arx::intersectRaySphere(cyl.origin, dir, limit.origin, limit.radius, t)) {
			t = glm::dot(dir, player.pos - cyl.origin);
		}
		if(t >= 0.f) {
			cyl.origin += dir * (t + 0.1f);
		}
	}
	
	while(true) {
		
		float offsetY = CheckAnythingInCylinder(cyl, entity,
		                                        CFLAG_JUST_TEST | CFLAG_COLLIDE_NOCOL | CFLAG_NO_NPC_COLLIDE);
		
		if(offsetY < 0.f) {
			
			result.foundCollision = true;
			
			if(inc / 2.f < 0.1f) {
				break;
			}
			
			// Decrease step distance by half and step back
			inc /= 2.f;
			cyl.origin -= dir * inc;
			
		} else {
			
			result.foundSpot = true;
			result.pos = cyl.origin;
			result.offsetY = offsetY;
			
			if(!limit.contains(result.pos)) {
				break;
			}
			
			// Step forward
			cyl.origin += dir * inc;
			
		}
		
	}
	
	if(result.foundCollision && !limit.contains(result.pos)) {
		result.foundCollision = false;
	}
	
	Anglef angle = entity->angle;
	angle.setYaw(270.f - angle.getYaw());
	result.offset = toQuaternion(angle) * result.offset;
	
	return result;
}

void updateDraggedEntity() {
	
	Entity * entity = g_draggedEntity;
	
	if(!entity || BLOCK_PLAYER_CONTROLS || !PLAYER_INTERFACE_SHOW) {
		return;
	}
	
	ARX_PROFILE_FUNC();
	
	arx_assert(!locateInInventories(entity));
	
	bool drop = eeMouseUp1();
	
	Vec2f mouse = Vec2f(DANAEMouse) + g_draggedIconOffset;
	
	g_dragStatus = EntityDragStatus_OverHud;
	entity->show = SHOW_FLAG_ON_PLAYER;
	
	if(g_secondaryInventoryHud.containsPos(Vec2s(mouse))) {
		if(drop) {
			g_secondaryInventoryHud.dropEntity();
		}
		return;
	} else if(g_playerInventoryHud.containsPos(Vec2s(mouse))) {
		if(drop) {
			g_playerInventoryHud.dropEntity();
		}
		return;
	} else if(ARX_INTERFACE_MouseInBook()) {
		if(drop && g_playerBook.currentPage() == BOOKMODE_STATS) {
			SendIOScriptEvent(entities.player(), entity, SM_INVENTORYUSE);
			COMBINE = NULL;
		}
		return;
	} else if(drop && (entity->ioflags & IO_GOLD)) {
		ARX_PLAYER_AddGold(entity);
		return;
	}
	
	if(g_camera == g_dragStartCamera && g_camera->angle.getYaw() != g_dragStartAngle) {
		float deltaYaw = g_camera->angle.getYaw() - g_dragStartAngle;
		Anglef angle = entity->angle;
		angle.setYaw(270.f - angle.getYaw());
		angle = toAngle(glm::quat(glm::vec3(0.f, glm::radians(-deltaYaw), 0.f)) * toQuaternion(angle));
		angle.setYaw(270.f - angle.getYaw());
		entity->angle = angle;
	}
	g_dragStartCamera = g_camera;
	g_dragStartAngle = g_camera->angle.getYaw();
	
	Vec3f origin = g_camera->m_pos;
	Vec3f dest = screenToWorldSpace(mouse + g_draggedObjectOffset, 1000.f);
	
	if(g_camera == &g_playerCamera) {
		// Use stable camera position so the dragged entity does not move around with the player head animation
		dest = dest - origin + g_playerCameraStablePos;
		origin = g_playerCameraStablePos;
	}
	
	Vec3f dir = glm::normalize(dest - origin);
	
	// Only allow dropping entities near the player
	Sphere limit(player.pos, 300.f);
	
	EntityDragResult result = findSpotForDraggedEntity(origin, dir, entity, limit);
	if(!result.foundSpot) {
		// No space to drop the entity
		g_dragStatus = EntityDragStatus_Invalid;
		return;
	}
	
	Vec3f pos = result.pos;
	
	// Snap entities to the ground up to a threshold
	float threshold = std::min(result.height, 12.0f);
	if(result.offsetY <= threshold) {
		pos += Vec3f(0.f, result.offsetY, 0.f);
	} else {
		pos += Vec3f(0.f, threshold - std::min(result.offsetY - threshold, threshold), 0.f);
	}
	
	ARX_INTERACTIVE_Teleport(entity, pos - Vec3f(result.offset.x, 0.f, result.offset.z), true);
	
	if(!result.foundCollision || pos.y < player.pos.y) {
		// Throw item if there is no collision withthin the maximum distance
		g_dragStatus = EntityDragStatus_Throw;
	} else if(glm::abs(result.offsetY) > result.height) {
		entity->show = SHOW_FLAG_IN_SCENE;
		g_dragStatus = EntityDragStatus_Drop;
	} else {
		entity->show = SHOW_FLAG_IN_SCENE;
		g_dragStatus = EntityDragStatus_OnGround;
	}
	
	if(!drop) {
		return;
	}
	
	ARX_PLAYER_Remove_Invisibility();
	entity->soundtime = 0;
	entity->soundcount = 0;
	entity->show = SHOW_FLAG_IN_SCENE;
	entity->obj->pbox->active = 0;
	entity->gameFlags &= ~GFLAG_NOCOMPUTATION;
	setDraggedEntity(NULL);
	
	if(g_dragStatus == EntityDragStatus_Throw) {
		
		Vec3f start = player.pos + Vec3f(0.f, 80.f, 0.f) - Vec3f(result.offset.x, 0.f, result.offset.z);
		Vec3f direction = glm::normalize(entity->pos - start);
		entity->pos = start;
		EERIE_PHYSICS_BOX_Launch(entity->obj, entity->pos, entity->angle, direction);
		ARX_SOUND_PlaySFX(g_snd.WHOOSH, &entity->pos);
		
	} else if(glm::abs(result.offsetY) > threshold) {
		
		EERIE_PHYSICS_BOX_Launch(entity->obj, entity->pos, entity->angle, Vec3f(0.f, 0.1f, 0.f));
		ARX_SOUND_PlaySFX(g_snd.WHOOSH, &entity->pos);
		
	} else {
		
		ARX_SOUND_PlayInterface(g_snd.INVSTD);
		
	}
	
}
