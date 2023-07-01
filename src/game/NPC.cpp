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
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include "game/NPC.h"

#include <stddef.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <limits>
#include <vector>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/strided.hpp>

#include "animation/Animation.h"

#include "ai/Anchors.h"
#include "ai/Paths.h"
#include "ai/PathFinderManager.h"

#include "core/GameTime.h"
#include "core/Core.h"
#include "core/Config.h"

#include "game/Camera.h"
#include "game/Damage.h"
#include "game/EntityManager.h"
#include "game/Equipment.h"
#include "game/Inventory.h"
#include "game/Item.h"
#include "game/Player.h"
#include "game/Spells.h"

#include "gui/Cursor.h"
#include "gui/Dragging.h"
#include "gui/Interface.h"
#include "gui/Speech.h"
#include "gui/hud/SecondaryInventory.h"

#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Color.h"
#include "graphics/DrawLine.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/Vertex.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/data/Mesh.h"
#include "graphics/data/MeshManipulation.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/MagicFlare.h"

#include "io/resource/ResourcePath.h"

#include "math/Angle.h"
#include "math/Random.h"
#include "math/Vector.h"

#include "physics/CollisionShapes.h"
#include "physics/Collisions.h"
#include "physics/Physics.h"

#include "platform/Platform.h"
#include "platform/profiler/Profiler.h"

#include "scene/Object.h"
#include "scene/Interactive.h"
#include "scene/Scene.h"
#include "scene/GameSound.h"
#include "scene/Light.h"

#include "script/Script.h"

#include "util/Cast.h"


void CheckNPCEx(Entity & io);

static const float ARX_NPC_ON_HEAR_MAX_DISTANCE_STEP = 600.f;
static const float ARX_NPC_ON_HEAR_MAX_DISTANCE_ITEM = 800.f;

void StareAtTarget(Entity * io);

static const float RUN_WALK_RADIUS = 450.f;

static bool isCurrentAnimation(Entity * entity, size_t layer, AnimationNumber anim) {
	ANIM_HANDLE * animation = entity->anims[anim];
	return animation != nullptr && entity->animlayer[layer].cur_anim == animation;
}



static void changeAnimation(Entity * entity, size_t layer, AnimationNumber anim,
                          AnimUseType flags = 0, bool startAtBeginning = false) {
	changeAnimation(entity, layer, entity->anims[anim], flags, startAtBeginning);
}

static void changeAnimation(Entity * entity, AnimationNumber anim,
                          AnimUseType flags = 0, bool startAtBeginning = false) {
	changeAnimation(entity, entity->anims[anim], flags, startAtBeginning);
}

static void setAnimation(Entity * entity, AnimationNumber anim,
                         AnimUseType flags = 0, bool startAtBeginning = false) {
	setAnimation(entity, entity->anims[anim], flags, startAtBeginning);
}

static void CheckHit(Entity * source, float ratioaim) {
	
	if(!source) {
		return;
	}
	
	Vec3f from(0.f, 0.f, -90.f);
	Vec3f to = VRotateY(from, MAKEANGLE(180.f - source->angle.getYaw()));
	Vec3f ppos = source->pos + Vec3f(0.f, -80.f, 0.f);
	Vec3f pos = ppos + to;
	
	float dmg;
	if(source->ioflags & IO_NPC) {
		dmg = source->_npcdata->damages;
	} else {
		dmg = 40.f;
	}
	
	Entity * target = entities.get(source->targetinfo);
	if(!target) {
		return;
	}
	
	if(target->ioflags & (IO_MARKER | IO_CAMERA)) {
		return;
	}
	
	if(!(target->gameFlags & GFLAG_ISINTREATZONE)
	   || target->show != SHOW_FLAG_IN_SCENE
	   || !target->obj
	   || target->pos.y <= (source->pos.y + source->physics.cyl.height)
	   || source->pos.y <= (target->pos.y + target->physics.cyl.height)) {
		return;
	}
	
	float dist_limit = source->_npcdata->reach + source->physics.cyl.radius;
	long count = 0;
	float mindist = std::numeric_limits<float>::max();
	
	for(const EERIE_VERTEX & vertex : target->obj->vertexWorldPositions | boost::adaptors::strided(2)) {
		float dist = fdist(pos, vertex.v);
		if(dist <= dist_limit && glm::abs(pos.y - vertex.v.y) < 60.f) {
			count++;
			if(dist < mindist) {
				mindist = dist;
			}
		}
	}
	
	float ratio = float(count) / (float(target->obj->vertexlist.size()) * 0.5f);
	
	if(target->ioflags & IO_NPC) {
		if(mindist <= dist_limit) {
			ARX_EQUIPMENT_ComputeDamages(source, target, ratioaim);
		}
	} else if(target->ioflags & IO_FIX) {
		if(mindist <= 120.f) {
			DamageType type = getDamageTypeFromWeaponMaterial(getWeaponMaterial(*source));
			damageProp(*target, dmg * ratio, source, nullptr, type);
		}
	}
	
}

void ARX_NPC_Kill_Spell_Launch(Entity * io) {
	
	if(!io) {
		return;
	}
	
	if(io->flarecount) {
		MagicFlareReleaseEntity(io);
	}
	
	io->spellcast_data.castingspell = SPELL_NONE;
}

//! Releases Pathfinder info from an NPC
static void ARX_NPC_ReleasePathFindInfo(Entity * io) {
	
	if(!io || !(io->ioflags & IO_NPC)) {
		return;
	}
	
	// Releases data & resets vars
	delete[] io->_npcdata->pathfind.list;
	io->_npcdata->pathfind.list = nullptr;
	io->_npcdata->pathfind.listnb = -1;
	io->_npcdata->pathfind.listpos = 0;
	io->_npcdata->pathfind.pathwait = 0;
}

//! Creates an extra rotations structure for a NPC
static void ARX_NPC_CreateExRotateData(Entity * io) {
	
	if(!io || !(io->ioflags & IO_NPC) || io->_npcdata->ex_rotate) {
		return;
	}
	
	io->_npcdata->ex_rotate = new EERIE_EXTRA_ROTATE();
	io->head_rot = 0;
	
	io->_npcdata->ex_rotate->group_number[0] = EERIE_OBJECT_GetGroup(io->obj, "head");
	io->_npcdata->ex_rotate->group_number[1] = EERIE_OBJECT_GetGroup(io->obj, "neck");
	io->_npcdata->ex_rotate->group_number[2] = EERIE_OBJECT_GetGroup(io->obj, "chest");
	io->_npcdata->ex_rotate->group_number[3] = EERIE_OBJECT_GetGroup(io->obj, "belt");
	
	for(Anglef & rotation : io->_npcdata->ex_rotate->group_rotate) {
		rotation = Anglef();
	}
	
	io->_npcdata->look_around_inc = 0.f;
	
}

//! Resurects an NPC
void ARX_NPC_Revive(Entity * io, bool init) {
	
	if(g_secondaryInventoryHud.isOpen(io)) {
		g_secondaryInventoryHud.close();
	}
	
	io->mainevent = SM_MAIN;
	
	if(io->ioflags & IO_NPC) {
		io->ioflags &= ~IO_NO_COLLISIONS;
		io->_npcdata->lifePool.current = io->_npcdata->lifePool.max;
		ARX_SCRIPT_ResetObject(io, true);
		io->_npcdata->lifePool.current = io->_npcdata->lifePool.max;
	}

	if(init) {
		io->requestRoomUpdate = true;
		io->pos = io->initpos;
	}
	
	ARX_INTERACTIVE_HideGore(io, true);
	
	if(io->ioflags & IO_NPC) {
		io->_npcdata->cuts = 0;
	}
	
}

//! Sets a new behaviour for NPC
void ARX_NPC_Behaviour_Change(Entity * io, Behaviour behavior, long behavior_param) {
	
	if(!io || !(io->ioflags & IO_NPC)) {
		return;
	}
	
	if((io->_npcdata->behavior & BEHAVIOUR_FIGHT) && !(behavior & BEHAVIOUR_FIGHT)) {
		stopAnimation(io, 1);
	}
	
	if((behavior & BEHAVIOUR_NONE) || (behavior == 0)) {
		changeAnimation(io, 0, ANIM_DEFAULT);
		io->animlayer[0].flags &= ~EA_LOOP;
		stopAnimation(io, 1);
		io->animlayer[1].flags &= ~EA_LOOP;
		stopAnimation(io, 2);
		io->animlayer[2].flags &= ~EA_LOOP;
	}
	
	if(behavior & BEHAVIOUR_FRIENDLY) {
		changeAnimation(io, ANIM_DEFAULT, 0, true);
	}
	
	io->_npcdata->behavior = behavior;
	io->_npcdata->behavior_param = float(behavior_param);
}

//! Resets all behaviour data from a NPC
void resetNpcBehavior(Entity & npc) {
	
	arx_assert(npc.ioflags & IO_NPC);
	
	npc._npcdata->behavior = BEHAVIOUR_NONE;
	
	for(IO_BEHAVIOR_DATA & behavior : npc._npcdata->stacked) {
		behavior.exist = 0;
	}
	
}

//! Reset all Behaviours from all NPCs
void resetAllNpcBehaviors() {
	for(Entity & npc : entities(IO_NPC)) {
		resetNpcBehavior(npc);
	}
}

//! Stacks an NPC behaviour
void ARX_NPC_Behaviour_Stack(Entity * io) {
	
	if(!io || !(io->ioflags & IO_NPC)) {
		return;
	}
	
	for(IO_BEHAVIOR_DATA & behavior : io->_npcdata->stacked) {
		
		if(behavior.exist == 0) {
			behavior.behavior = io->_npcdata->behavior;
			behavior.behavior_param = io->_npcdata->behavior_param;
			if(io->_npcdata->pathfind.listnb > 0) {
				behavior.target = io->_npcdata->pathfind.truetarget;
			} else {
				behavior.target = io->targetinfo;
			}
			behavior.movemode = io->_npcdata->movemode;
			behavior.exist = 1;
			return;
		}
		
	}
	
}

//! Unstacks One stacked behaviour from an NPC
void ARX_NPC_Behaviour_UnStack(Entity * io) {
	
	if(!io || !(io->ioflags & IO_NPC)) {
		return;
	}
	
	for(IO_BEHAVIOR_DATA & behavior : boost::adaptors::reverse(io->_npcdata->stacked)) {
		
		if(!behavior.exist) {
			continue;
		}
		
		AcquireLastAnim(io);
		io->_npcdata->behavior = behavior.behavior;
		io->_npcdata->behavior_param = behavior.behavior_param;
		io->targetinfo = behavior.target;
		io->_npcdata->movemode = behavior.movemode;
		behavior.exist = 0;
		ARX_NPC_LaunchPathfind(io, behavior.target);
		
		if(io->_npcdata->behavior & BEHAVIOUR_NONE) {
			for(size_t l = 0; l < MAX_ANIM_LAYERS; l++) {
				io->animlayer[l] = behavior.animlayer[l];
			}
		}
		
		return;
	}
	
}

//! Checks for any direct shortcut between NPC and future anchors...
static long ARX_NPC_GetNextAttainableNodeIncrement(Entity * io) {
	
	arx_assert(io);
	if(!(io->ioflags & IO_NPC) || (io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND)) {
		return 0;
	}
	
	float dists = arx::distance2(io->pos, g_camera->m_pos);
	if(dists > square(g_camera->cdepth) * square(1.0f / 2)) {
		return 0;
	}
	
	long MAX_TEST;
	if(dists < square(g_camera->cdepth) * square(1.0f / 4)) {
		MAX_TEST = 6;
	} else {
		MAX_TEST = 4;
	}
	
	for(long l_try = MAX_TEST; l_try > 1; l_try--) {
		
		if(io->_npcdata->pathfind.listpos + l_try > io->_npcdata->pathfind.listnb - 1) {
			continue;
		}
		
		size_t total = 0;
		for(long aa = l_try; aa > 1; aa--) {
			long v = io->_npcdata->pathfind.list[io->_npcdata->pathfind.listpos + aa];
			total += g_anchors[v].linked.size();
			if(aa == l_try) {
				total += g_anchors[v].linked.size();
			}
		}
		
		float average = float(total) / float(l_try + 1);
		if(average <= 3.5f) {
			continue;
		}
		
		io->physics.startpos = io->pos;
		long pos = io->_npcdata->pathfind.list[io->_npcdata->pathfind.listpos + l_try];
		io->physics.targetpos = g_anchors[pos].pos;

		if(glm::abs(io->physics.startpos.y - io->physics.targetpos.y) > 60.f) {
			continue;
		}

		io->physics.targetpos.y += 60.f; // FAKE Gravity !
		IO_PHYSICS phys = io->physics;
		phys.cyl = getEntityCylinder(*io);

		// Now we try the physical move for real
		if(   io->physics.startpos == io->physics.targetpos
		   || ARX_COLLISION_Move_Cylinder(&phys, io, 40, CFLAG_JUST_TEST | CFLAG_NPC)
		) {
			if(closerThan(phys.cyl.origin, g_anchors[pos].pos, 30.f)) {
				return l_try;
			}
		}
	}

	return 0;
}

//! Checks for nearest VALID anchor for a cylinder from a position
static long AnchorData_GetNearest(const Vec3f & pos, const Cylinder & cyl, long except = -1) {
	long returnvalue = -1;
	float distmax = std::numeric_limits<float>::max();

	for(size_t i = 0; i < g_anchors.size(); i++) {
		
		if(except != -1 && i == size_t(except)) {
			continue;
		}
		
		if(!g_anchors[i].linked.empty()) {
			float d = arx::distance2(g_anchors[i].pos, pos);
			if(d < distmax && g_anchors[i].height <= cyl.height
			   && g_anchors[i].radius >= cyl.radius && !g_anchors[i].blocked) {
				returnvalue = long(i);
				distmax = d;
			}
		}
		
	}
	
	return returnvalue;
}

static long AnchorData_GetNearest_2(float beta, const Vec3f & pos, const Cylinder & cyl) {
	
	float d = glm::radians(beta);
	Vec3f vect(-std::sin(d), 0, std::cos(d));
	vect = glm::normalize(vect);

	Vec3f posi = pos;
	posi.x += vect.x * 50.f;
	// XXX should this really be vect.x ? copy-paste error ?
	posi.z += vect.x * 50.f;
	
	return AnchorData_GetNearest(posi, cyl);
}

static bool ARX_NPC_LaunchPathfind_Cleanup(Entity * io) {
	
	io->_npcdata->pathfind.pathwait = 0;
	
	if(io->_npcdata->pathfind.list) {
		ARX_NPC_ReleasePathFindInfo(io);
	}

	io->_npcdata->pathfind.listnb = -2;

	if(io->_npcdata->pathfind.flags & PATHFIND_ALWAYS) {
		return false; // TODO was BEHAVIOUR_NONE
	}
	
	SendIOScriptEvent(nullptr, io, SM_PATHFINDER_FAILURE);
	
	return false;
}

static bool ARX_NPC_LaunchPathfind_End(Entity * io, EntityHandle target, const Vec3f & pos2) {
	
	io->targetinfo = target;
	io->_npcdata->pathfind.truetarget = target;
	
	long from;
	if((io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND) || (io->_npcdata->behavior & BEHAVIOUR_FLEE)) {
		from = AnchorData_GetNearest(io->pos, io->physics.cyl);
	} else {
		from = AnchorData_GetNearest_2(io->angle.getYaw(), io->pos, io->physics.cyl);
	}
	
	long to;
	if(io->_npcdata->behavior & BEHAVIOUR_FLEE) {
		to = AnchorData_GetNearest(pos2, io->physics.cyl, from);
	} else if(io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND) {
		to = from;
	} else {
		to = AnchorData_GetNearest(pos2, io->physics.cyl);
	}
	
	if(from != -1 && to != -1) {
		
		if(from == to && !(io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND)) {
			return true;
		}
		
		if((io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND) ||
		   closerThan(io->pos, g_anchors[from].pos, 200.f)) {
			
			if(!(io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND) &&
			   !(io->_npcdata->behavior & BEHAVIOUR_FLEE)) {
				if(closerThan(g_anchors[from].pos, g_anchors[to].pos, 200.f)) {
					return false;
				}
				if(fartherThan(pos2, g_anchors[to].pos, 200.f)) {
					return ARX_NPC_LaunchPathfind_Cleanup(io);
				}
			}
			
			if(io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND) {
				io->_npcdata->pathfind.truetarget = EntityHandle(TARGET_NONE);
			}
			
			io->_npcdata->pathfind.listnb = -1;
			io->_npcdata->pathfind.listpos = 0;
			io->_npcdata->pathfind.pathwait = 1;
			
			delete[] io->_npcdata->pathfind.list;
			io->_npcdata->pathfind.list = nullptr;
			
			PATHFINDER_REQUEST tpr;
			tpr.from = from;
			tpr.to = to;
			tpr.entity = io;
			if(EERIE_PATHFINDER_Add_To_Queue(tpr)) {
				return true;
			}
			
		}
	}

	return ARX_NPC_LaunchPathfind_Cleanup(io);
}

bool ARX_NPC_LaunchPathfind(Entity * io, EntityHandle target)
{
	if(!io || !(io->ioflags & IO_NPC)) {
		return false;
	}

	io->physics.cyl.origin = io->pos;
	EntityHandle old_target = io->targetinfo;

	if(!(io->ioflags & IO_PHYSICAL_OFF)) {
		if(!ForceNPC_Above_Ground(io)) {
			io->_npcdata->pathfind.pathwait = 0;
			return false;
		}
	}

	if(io->_npcdata->behavior & BEHAVIOUR_FRIENDLY) {
		io->targetinfo = target;
		io->_npcdata->pathfind.pathwait = 0;
		return false;
	}
	
	if(io->_npcdata->pathfind.listnb > 0) {
		io->_npcdata->pathfind.listnb = -1;
		io->_npcdata->pathfind.listpos = 0;
		io->_npcdata->pathfind.pathwait = 0;
		io->_npcdata->pathfind.truetarget = EntityHandle(TARGET_NONE);
		delete[] io->_npcdata->pathfind.list;
		io->_npcdata->pathfind.list = nullptr;
	}
	
	if(io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND) {
		return ARX_NPC_LaunchPathfind_End(io, target, io->pos + Vec3f(1000.f, 0.f, 1000.f));
	}
	
	if(target.handleData() < 0 || (io->_npcdata->behavior & BEHAVIOUR_GO_HOME)) {
		target = io->index();

		if(target == EntityHandle()) {
			return ARX_NPC_LaunchPathfind_Cleanup(io);
		}

		io->_npcdata->pathfind.truetarget = target;
		
		Vec3f pos2 = (io->_npcdata->behavior & BEHAVIOUR_GO_HOME) ? io->initpos : io->pos;
		
		return ARX_NPC_LaunchPathfind_End(io, target, pos2);
	}

	if(entities.get(target) == io) {
		io->_npcdata->pathfind.pathwait = 0;
		return false; // cannot pathfind self...
	}
	
	if(old_target != target) {
		io->_npcdata->reachedtarget = 0;
	}
	
	Vec3f pos2 = io->pos;
	if(io->_npcdata->behavior & BEHAVIOUR_GO_HOME) {
		pos2 = io->initpos;
	} else if(Entity * entity = entities.get(target)) {
		pos2 = entity->pos;
	}
	
	io->_npcdata->pathfind.truetarget = target;
	
	if(closerThan(io->pos, g_camera->m_pos, g_camera->cdepth * 0.5f)
	   && glm::abs(io->pos.y - pos2.y) < 50.f
	   && closerThan(io->pos, pos2, 520)
	   && (io->_npcdata->behavior & BEHAVIOUR_MOVE_TO)
	   && !(io->_npcdata->behavior & BEHAVIOUR_SNEAK)
	   && !(io->_npcdata->behavior & BEHAVIOUR_FLEE)) {
		
		// COLLISION Management START *********************************************************************
		io->physics.startpos = io->pos;
		io->physics.targetpos = pos2;
		IO_PHYSICS phys = io->physics;
		phys.cyl = getEntityCylinder(*io);

		// Now we try the physical move for real
		if(   io->physics.startpos == io->physics.targetpos
		   || ARX_COLLISION_Move_Cylinder(&phys, io, 40, CFLAG_JUST_TEST | CFLAG_NPC | CFLAG_NO_HEIGHT_MOD)
		) {
			if(closerThan(phys.cyl.origin, pos2, 100.f)) {
				io->_npcdata->pathfind.pathwait = 0;
				return false;
			}
		}
	}
	
	return ARX_NPC_LaunchPathfind_End(io, target, pos2);
}

bool ARX_NPC_SetStat(Entity & io, std::string_view statname, float value) {
	
	arx_assert(io.ioflags & IO_NPC);
	
	if(statname == "armor_class") {
		io._npcdata->armor_class = value < 0 ? 0 : value;
	} else if(statname == "backstab_skill" || statname == "backstabskill") {
		io._npcdata->backstab_skill = value < 0 ? 0 : value;
	} else if(statname == "backstab") {
		if(value == 0) {
			io._npcdata->npcflags &= ~NPCFLAG_BACKSTAB;
		} else {
			io._npcdata->npcflags |= NPCFLAG_BACKSTAB;
		}
	} else if(statname == "reach") {
		io._npcdata->reach = value < 0 ? 0 : value;
	} else if(statname == "critical") {
		io._npcdata->critical = value < 0 ? 0 : value;
	} else if(statname == "absorb") {
		io._npcdata->absorb = value < 0 ? 0 : value;
	} else if(statname == "damages") {
		io._npcdata->damages = value < 0 ? 0 : value;
	} else if(statname == "tohit") {
		io._npcdata->tohit = value < 0 ? 0 : value;
	} else if(statname == "aimtime") {
		io._npcdata->aimtime = std::chrono::duration<float, std::milli>(std::max(value, 0.f));
	} else if(statname == "life") {
		io._npcdata->lifePool.max = io._npcdata->lifePool.current = value < 0 ? 0.0000001f : value;
	} else if(statname == "mana") {
		io._npcdata->manaPool.max = io._npcdata->manaPool.current = value < 0 ? 0 : value;
	} else if(statname == "resistfire") {
		io._npcdata->resist_fire = static_cast<unsigned char>(glm::clamp(value, 0.f, 100.f));
	} else if(statname == "resistpoison") {
		io._npcdata->resist_poison = static_cast<unsigned char>(glm::clamp(value, 0.f, 100.f));
	} else if(statname == "resistmagic") {
		io._npcdata->resist_magic = static_cast<unsigned char>(glm::clamp(value, 0.f, 100.f));
	} else {
		return false;
	}
	
	return true;
}

/*!
 * \brief Sets a New MoveMode for a NPC
 */
void ARX_NPC_ChangeMoveMode(Entity * io, MoveMode MOVEMODE) {
	
	if(!io || !(io->ioflags & IO_NPC)) {
		return;
	}
	
	switch(MOVEMODE) {
		case RUNMODE: {
			if(isCurrentAnimation(io, 0, ANIM_WALK) || isCurrentAnimation(io, 0, ANIM_WALK_SNEAK)) {
				changeAnimation(io, ANIM_RUN, 0, true);
			}
			break;
		}
		case WALKMODE: {
			if(isCurrentAnimation(io, 0, ANIM_RUN) || isCurrentAnimation(io, 0, ANIM_WALK_SNEAK)) {
				changeAnimation(io, ANIM_WALK);
			}
			break;
		}
		case NOMOVEMODE: {
			if(isCurrentAnimation(io, 0, ANIM_WALK) || isCurrentAnimation(io, 0, ANIM_RUN)
			   || isCurrentAnimation(io, 0, ANIM_WALK_SNEAK)) {
				changeAnimation(io, ANIM_WAIT, 0, true);
			}
			break;
		}
		case SNEAKMODE: {
			if(isCurrentAnimation(io, 0, ANIM_WALK) || isCurrentAnimation(io, 0, ANIM_RUN)) {
				changeAnimation(io, ANIM_WALK_SNEAK);
			}
			break;
		}
	}
	
	io->_npcdata->movemode = MOVEMODE;
}

/*!
 * \brief Diminishes life of a Poisoned NPC
 */
static void ARX_NPC_ManagePoison(Entity & io) {
	
	float cp = io._npcdata->poisonned * g_framedelay * 0.00025f;
	float faster = 10.f - io._npcdata->poisonned;
	if(faster < 0.f) {
		faster = 0.f;
	}
	
	if(Random::getf(0.f, 100.f) > io._npcdata->resist_poison + faster) {
		float dmg = cp * (1.f / 3);
		if(io._npcdata->lifePool.current > 0 && io._npcdata->lifePool.current - dmg <= 0.f) {
			long xp = io._npcdata->xpvalue;
			damageNpc(io, dmg, nullptr, nullptr, DAMAGE_TYPE_POISON, nullptr);
			ARX_PLAYER_Modify_XP(xp);
		} else {
			io._npcdata->lifePool.current -= dmg;
		}
		io._npcdata->poisonned -= cp * 0.1f;
	} else {
		io._npcdata->poisonned -= cp;
	}
	
	if(io._npcdata->poisonned < 0.1f) {
		io._npcdata->poisonned = 0.f;
	}
	
}

/*!
 * \brief Checks if the bottom of an IO is underwater.
 *
 * Plays Water sounds
 * Decrease/stops Ignition of this IO if necessary
 */
static void CheckUnderWaterIO(Entity & io) {
	
	Vec3f ppos = io.pos;
	EERIEPOLY * ep = EEIsUnderWater(ppos);
	
	if(io.ioflags & IO_UNDERWATER) {
		if(!ep) {
			io.ioflags &= ~IO_UNDERWATER;
			ARX_SOUND_PlaySFX(g_snd.PLOUF, &ppos);
			ARX_PARTICLES_SpawnWaterSplash(ppos);
		}
	} else if(ep) {
		io.ioflags |= IO_UNDERWATER;
		ARX_SOUND_PlaySFX(g_snd.PLOUF, &ppos);
		ARX_PARTICLES_SpawnWaterSplash(ppos);
		
		if(io.ignition > 0.f) {
			ARX_SOUND_PlaySFX(g_snd.TORCH_END, &ppos);
			
			lightHandleDestroy(io.ignit_light);
			
			ARX_SOUND_Stop(io.ignit_sound);
			io.ignit_sound = audio::SourcedSample();
			
			io.ignition = 0;
		}
	}
}

static void ManageNPCMovement(Entity * io);

void ARX_PHYSICS_Apply() {
	
	ARX_PROFILE_FUNC();
	
	static size_t CURRENT_DETECT = 0;

	CURRENT_DETECT++;
	if(CURRENT_DETECT > treatio.size()) {
		CURRENT_DETECT = 1;
	}
	
	// We don't manage Player(0) this way
	for(size_t i = 1; i < treatio.size(); i++) {
		ARX_PROFILE(IO);
		
		if(treatio[i].show != SHOW_FLAG_IN_SCENE) {
			continue;
		}
		
		if(treatio[i].ioflags & (IO_FIX | IO_JUST_COLLIDE)) {
			continue;
		}
		
		Entity * io = treatio[i].io;
		if(!io) {
			continue;
		}
		
		if((io->ioflags & IO_NPC) && io->_npcdata->poisonned > 0.f) {
			ARX_NPC_ManagePoison(*io);
		}
		
		if(   (io->ioflags & IO_ITEM)
		   && (io->gameFlags & GFLAG_GOREEXPLODE)
		   && g_gameTime.now() - io->animBlend.lastanimtime > 300ms
		   && io->obj
		   && !io->obj->vertexlist.empty()
		) {
			long cnt = (io->obj->vertexlist.size() << 12) + 1;
			
			cnt = glm::clamp(cnt, 2l, 10l);
			
			for(long nn = 0; nn < cnt; nn++) {
				std::vector<EERIE_VERTEX>::iterator it = Random::getIterator(io->obj->vertexlist);
				
				ARX_PARTICLES_Spawn_Splat(it->v, 20.f, Color::red);
				ARX_PARTICLES_Spawn_Blood(it->v, 20.f, io->index());
			}
			
			io->destroyOne();
			continue;
		}
		
		EERIEPOLY * ep = CheckInPoly(io->pos);
		if(ep && (ep->type & POLY_LAVA) && glm::abs(ep->center.y - io->pos.y) < 40) {
			ARX_PARTICLES_Spawn_Lava_Burn(io->pos, io);
			if(io->ioflags & IO_NPC) {
				const float LAVA_DAMAGE = 10.f;
				float dmg = LAVA_DAMAGE * g_framedelay * 0.01f;
				damageNpc(*io, dmg, nullptr, nullptr, DAMAGE_TYPE_FIRE | DAMAGE_TYPE_FIELD, nullptr);
			}
		}
		
		CheckUnderWaterIO(*io);
		
		if(io->obj && io->obj->pbox) {
			io->gameFlags &= ~GFLAG_NOCOMPUTATION;
			
			PHYSICS_BOX_DATA & pbox = *io->obj->pbox;
			
			if(pbox.active == 1) {
				ARX_PHYSICS_BOX_ApplyModel(pbox, g_framedelay, io->rubber, *io);
				
				if(io->soundcount > 12) {
					io->soundtime = 0;
					io->soundcount = 0;
					for(PhysicsParticle & vert : pbox.vert) {
						vert.velocity = Vec3f(0.f);
					}
					pbox.active = 2;
					pbox.stopcount = 0;
				}
				
				io->requestRoomUpdate = true;
				
				Vec3f offset = pbox.vert[0].initpos;
				offset = VRotateY(offset, MAKEANGLE(270.f - io->angle.getYaw()));
				offset = VRotateX(offset, -io->angle.getPitch());
				offset = VRotateZ(offset, io->angle.getRoll());
				io->pos = pbox.vert[0].pos - offset;
				
				arx_assert(isallfinite(io->pos));
				
				continue;
			}
		}
		
		if(IsDeadNPC(*io)) {
			continue;
		}
		
		if(io->ioflags & IO_PHYSICAL_OFF) {
			if(io->ioflags & IO_NPC) {
				AnimLayer & layer0 = io->animlayer[0];
				
				if(!layer0.cur_anim || (layer0.flags & EA_ANIMEND)) {
					ANIM_Set(layer0, io->anims[ANIM_WAIT]);
					layer0.altidx_cur = 0;
				}
				
				GetTargetPos(io);
				
				if(!g_gameTime.isPaused() && !(layer0.flags & EA_FORCEPLAY)) {
					if(io->_npcdata->behavior & BEHAVIOUR_STARE_AT) {
						StareAtTarget(io);
					} else {
						FaceTarget2(io);
					}
				}
			}
			continue;
		}
		
		if(io->ioflags & IO_NPC) {
			
			if(io->_npcdata->climb_count != 0.f && g_framedelay > 0) {
				io->_npcdata->climb_count -= MAX_ALLOWED_CLIMBS_PER_SECOND * g_framedelay * 0.001f;
				if(io->_npcdata->climb_count < 0) {
					io->_npcdata->climb_count = 0.f;
				}
			}
			
			if(io->_npcdata->pathfind.pathwait) { // Waiting For Pathfinder Answer
				if(io->_npcdata->pathfind.listnb == 0) { // Not Found
					SendIOScriptEvent(nullptr, io, SM_PATHFINDER_FAILURE);
					io->_npcdata->pathfind.pathwait = 0;
					if(io->_npcdata->pathfind.list) {
						ARX_NPC_ReleasePathFindInfo(io);
					}
					io->_npcdata->pathfind.listnb = -2;
				} else if(io->_npcdata->pathfind.listnb > 0) { // Found
					SendIOScriptEvent(nullptr, io, SM_PATHFINDER_SUCCESS);
					io->_npcdata->pathfind.pathwait = 0;
					io->_npcdata->pathfind.listpos += ARX_NPC_GetNextAttainableNodeIncrement(io);
					if(io->_npcdata->pathfind.listpos >= io->_npcdata->pathfind.listnb) {
						io->_npcdata->pathfind.listpos = 0;
					}
				}
			}
			
			ManageNPCMovement(io);
			CheckNPC(*io);
			
			if(CURRENT_DETECT == i) {
				CheckNPCEx(*io);
			}
		}
	}
}

void FaceTarget2(Entity * io) {
	
	if(io->ioflags & IO_NPC) {
		if(io->_npcdata->lifePool.current <= 0.f) {
			return;
		}
		if(io->_npcdata->behavior & BEHAVIOUR_NONE) {
			return;
		}
		if(io->_npcdata->pathfind.listnb <= 0 && (io->_npcdata->behavior & BEHAVIOUR_FLEE)) {
			return;
		}
	}
	
	GetTargetPos(io);
	Vec3f tv = io->pos;
	
	if(!fartherThan(getXZ(tv), getXZ(io->target), 5.f)) {
		return;
	}
	
	float tangle = MAKEANGLE(180.f + glm::degrees(getAngle(io->target.x, io->target.z, tv.x, tv.z)));
	float cangle = io->angle.getYaw();
	
	float tt = (cangle - tangle);
	if(tt == 0) {
		return;
	}
	
	float rot = 0.33f * g_framedelay;
	
	if(glm::abs(tt) < rot) {
		rot = glm::abs(tt);
	}
	
	rot = -rot;
	
	if((tt > 0.f && tt < 180.f) || (tt < -180.f)) {
		rot = -rot;
	}
	
	if(rot != 0) {
		Vec3f temp = io->move;
		io->move = VRotateY(temp, rot);
		temp = io->lastmove;
		io->lastmove = VRotateY(temp, rot);
	}
	
	// Needed angle to turn toward target
	io->angle.setYaw(MAKEANGLE(io->angle.getYaw() - rot)); // -tt
}

void StareAtTarget(Entity * io) {
	
	if(io->_npcdata->ex_rotate == nullptr) {
		ARX_NPC_CreateExRotateData(io);
	}
	
	if(io->ioflags & IO_NPC) {
		if(io->_npcdata->lifePool.current <= 0.f) {
			return;
		}
		
		if(io->_npcdata->pathfind.listnb <= 0 && (io->_npcdata->behavior & BEHAVIOUR_FLEE)) {
			return;
		}
	}
	
	if(io->_npcdata->behavior & BEHAVIOUR_NONE) {
		return;
	}
	
	GetTargetPos(io);
	Vec3f tv = io->pos;
	
	if(glm::distance(tv, io->target) <= 20.f) {
		return; // To fix "stupid" rotation near target
	}
	
	if(io->target.x - tv.x == 0 && io->target.z - tv.z == 0) {
		return;
	}
	
	float rot = 0.27f * g_framedelay;
	float alpha = MAKEANGLE(io->angle.getYaw());
	float beta = -io->head_rot;
	float pouet = MAKEANGLE(180.f + glm::degrees(getAngle(io->target.x, io->target.z, tv.x, tv.z)));
	float A = MAKEANGLE((MAKEANGLE(alpha + beta) - pouet));
	float B = MAKEANGLE(alpha - pouet);
	
	if(A == 0.f) {
		rot = 0.f;
	}
	
	if(B < 180 && B > 90) {
		if(rot > A) {
			rot = A;
		}
	} else if(B > 180 && B < 270) {
		if(rot > 360 - A) {
			rot = -(360 - A);
		} else {
			rot = -rot;
		}
	} else if(A < 180) {
		if(rot > A) {
			rot = A;
		}
	} else {
		if(rot > 360 - A) {
			rot = -(360 - A);
		} else {
			rot = -rot;
		}
	}
	rot *= 0.5f;
	
	// Needed angle to turn toward target
	float HEAD_ANGLE_THRESHOLD;
	if(io->ioflags & IO_NPC) {
		HEAD_ANGLE_THRESHOLD = 45.f * io->_npcdata->stare_factor;
	} else {
		HEAD_ANGLE_THRESHOLD = 45.f;
	}
	
	io->head_rot += rot;
	
	io->head_rot = glm::clamp(io->head_rot, -120.f, 120.f);
	
	float groupRotation[2];
	
	groupRotation[0] = io->head_rot * 1.5f;
	groupRotation[1] = io->head_rot * 0.5f;
	
	groupRotation[0] = glm::clamp(groupRotation[0], -HEAD_ANGLE_THRESHOLD, HEAD_ANGLE_THRESHOLD);
	groupRotation[1] = glm::clamp(groupRotation[1], -HEAD_ANGLE_THRESHOLD, HEAD_ANGLE_THRESHOLD);
	
	io->_npcdata->ex_rotate->group_rotate[0].setYaw(groupRotation[0]);
	io->_npcdata->ex_rotate->group_rotate[1].setYaw(groupRotation[1]);
	
}

static float GetTRUETargetDist(Entity * io) {
	
	arx_assert(io->ioflags & IO_NPC);
	
	Entity * target = entities.get(io->_npcdata->pathfind.truetarget);
	if(!target) {
		return 99999999.f;
	}
	
	if(io->_npcdata->behavior & BEHAVIOUR_GO_HOME) {
		return glm::distance(io->pos, io->initpos);
	}
	
	return glm::distance(io->pos, target->pos);
}

//! Checks If a NPC is dead
bool IsDeadNPC(const Entity & io) {
	
	if(io.ioflags & IO_NPC) {
		return (io._npcdata->lifePool.current <= 0 || io.mainevent == SM_DEAD);
	}
	
	return false;
}

//! Checks if Player is currently striking.
static bool IsPlayerStriking() {
	
	arx_assert(entities.player());
	Entity * io = entities.player();
	
	const AnimLayer & layer1 = io->animlayer[1];
	WeaponType weapontype = ARX_EQUIPMENT_GetPlayerWeaponType();
	
	switch(weapontype) {
		case WEAPON_BARE: {
			for(long j = 0; j < 4; j++) {
				if(player.m_strikeAimRatio > 300 && layer1.cur_anim == io->anims[ANIM_BARE_STRIKE_LEFT_CYCLE + j * 3]) {
					return true;
				}
				if(layer1.cur_anim == io->anims[ANIM_BARE_STRIKE_LEFT + j * 3]) {
					return true;
				}
			}
			break;
		}
		case WEAPON_DAGGER: {
			for(long j = 0; j < 4; j++) {
				if(player.m_strikeAimRatio > 300 && layer1.cur_anim == io->anims[ANIM_DAGGER_STRIKE_LEFT_CYCLE + j * 3]) {
					return true;
				}
				if(layer1.cur_anim == io->anims[ANIM_DAGGER_STRIKE_LEFT + j * 3]) {
					return true;
				}
			}
			break;
		}
		case WEAPON_1H: {
			for(long j = 0; j < 4; j++) {
				if(player.m_strikeAimRatio > 300 && layer1.cur_anim == io->anims[ANIM_1H_STRIKE_LEFT_CYCLE + j * 3]) {
					return true;
				}
				if(layer1.cur_anim == io->anims[ANIM_1H_STRIKE_LEFT + j * 3]) {
					return true;
				}
			}
			break;
		}
		case WEAPON_2H: {
			for(long j = 0; j < 4; j++) {
				if(player.m_strikeAimRatio > 300 && layer1.cur_anim == io->anims[ANIM_2H_STRIKE_LEFT_CYCLE + j * 3]) {
					return true;
				}
				if(layer1.cur_anim == io->anims[ANIM_2H_STRIKE_LEFT + j * 3]) {
					return true;
				}
			}
			break;
		}
		case WEAPON_BOW: break; // Bows cannot be used as melee weapons
	}
	
	return false;
}

static void ARX_NPC_Manage_NON_Fight(Entity * io) {
	
	AnimLayer & layer1 = io->animlayer[1];
	
	if((layer1.flags & EA_ANIMEND) && (layer1.cur_anim != nullptr)) {
		if(!(layer1.flags & EA_FORCEPLAY)) {
			AcquireLastAnim(io);
			FinishAnim(io, layer1.cur_anim);
			layer1.cur_anim = nullptr;
		}
	}
}

//! NPC IS in fight mode and close to target...
static void ARX_NPC_Manage_Fight(Entity * io) {
	
	if(!(io->ioflags & IO_NPC)) {
		return;
	}
	
	Entity * ioo = io->_npcdata->weapon;
	
	if(ioo) {
		io->_npcdata->weapontype = ioo->type_flags;
	} else {
		io->_npcdata->weapontype = 0;
	}
	
	if(io->_npcdata->weapontype != 0 && io->_npcdata->weaponinhand != 1) {
		return;
	}
	
	const AnimLayer & layer1 = io->animlayer[1];
	
	if(io->_npcdata->weapontype == 0) {
		if((layer1.cur_anim != io->anims[ANIM_BARE_WAIT]
		    && layer1.cur_anim != io->anims[ANIM_BARE_READY]
		    && layer1.cur_anim != io->anims[ANIM_BARE_STRIKE_LEFT_START]
		    && layer1.cur_anim != io->anims[ANIM_BARE_STRIKE_RIGHT_START]
		    && layer1.cur_anim != io->anims[ANIM_BARE_STRIKE_TOP_START]
		    && layer1.cur_anim != io->anims[ANIM_BARE_STRIKE_BOTTOM_START]
		    && layer1.cur_anim != io->anims[ANIM_BARE_STRIKE_LEFT_CYCLE]
		    && layer1.cur_anim != io->anims[ANIM_BARE_STRIKE_RIGHT_CYCLE]
		    && layer1.cur_anim != io->anims[ANIM_BARE_STRIKE_TOP_CYCLE]
		    && layer1.cur_anim != io->anims[ANIM_BARE_STRIKE_BOTTOM_CYCLE]
		    && layer1.cur_anim != io->anims[ANIM_BARE_STRIKE_LEFT]
		    && layer1.cur_anim != io->anims[ANIM_BARE_STRIKE_RIGHT]
		    && layer1.cur_anim != io->anims[ANIM_BARE_STRIKE_TOP]
		    && layer1.cur_anim != io->anims[ANIM_BARE_STRIKE_BOTTOM]
		    && layer1.cur_anim != io->anims[ANIM_CAST_CYCLE]
		    && layer1.cur_anim != io->anims[ANIM_CAST]
		    && layer1.cur_anim != io->anims[ANIM_CAST_END]
		    && layer1.cur_anim != io->anims[ANIM_CAST_START])
		   || layer1.cur_anim == nullptr) {
			changeAnimation(io, 1, ANIM_BARE_WAIT, EA_LOOP);
		}
	} else if(io->_npcdata->weapontype & OBJECT_TYPE_DAGGER) {
		if((layer1.cur_anim != io->anims[ANIM_DAGGER_WAIT]
		    && layer1.cur_anim != io->anims[ANIM_DAGGER_READY_PART_1]
		    && layer1.cur_anim != io->anims[ANIM_DAGGER_READY_PART_2]
		    && layer1.cur_anim != io->anims[ANIM_DAGGER_STRIKE_LEFT_START]
		    && layer1.cur_anim != io->anims[ANIM_DAGGER_STRIKE_RIGHT_START]
		    && layer1.cur_anim != io->anims[ANIM_DAGGER_STRIKE_TOP_START]
		    && layer1.cur_anim != io->anims[ANIM_DAGGER_STRIKE_BOTTOM_START]
		    && layer1.cur_anim != io->anims[ANIM_DAGGER_STRIKE_LEFT_CYCLE]
		    && layer1.cur_anim != io->anims[ANIM_DAGGER_STRIKE_RIGHT_CYCLE]
		    && layer1.cur_anim != io->anims[ANIM_DAGGER_STRIKE_TOP_CYCLE]
		    && layer1.cur_anim != io->anims[ANIM_DAGGER_STRIKE_BOTTOM_CYCLE]
		    && layer1.cur_anim != io->anims[ANIM_DAGGER_STRIKE_LEFT]
		    && layer1.cur_anim != io->anims[ANIM_DAGGER_STRIKE_RIGHT]
		    && layer1.cur_anim != io->anims[ANIM_DAGGER_STRIKE_TOP]
		    && layer1.cur_anim != io->anims[ANIM_DAGGER_STRIKE_BOTTOM]
		    && layer1.cur_anim != io->anims[ANIM_CAST_CYCLE]
		    && layer1.cur_anim != io->anims[ANIM_CAST]
		    && layer1.cur_anim != io->anims[ANIM_CAST_END]
		    && layer1.cur_anim != io->anims[ANIM_CAST_START])
		   || layer1.cur_anim == nullptr) {
			changeAnimation(io, 1, ANIM_DAGGER_WAIT, EA_LOOP);
		}
	} else if(io->_npcdata->weapontype & OBJECT_TYPE_1H) {
		if((layer1.cur_anim != io->anims[ANIM_1H_WAIT]
		    && layer1.cur_anim != io->anims[ANIM_1H_READY_PART_1]
		    && layer1.cur_anim != io->anims[ANIM_1H_READY_PART_2]
		    && layer1.cur_anim != io->anims[ANIM_1H_STRIKE_LEFT_START]
		    && layer1.cur_anim != io->anims[ANIM_1H_STRIKE_RIGHT_START]
		    && layer1.cur_anim != io->anims[ANIM_1H_STRIKE_TOP_START]
		    && layer1.cur_anim != io->anims[ANIM_1H_STRIKE_BOTTOM_START]
		    && layer1.cur_anim != io->anims[ANIM_1H_STRIKE_LEFT_CYCLE]
		    && layer1.cur_anim != io->anims[ANIM_1H_STRIKE_RIGHT_CYCLE]
		    && layer1.cur_anim != io->anims[ANIM_1H_STRIKE_TOP_CYCLE]
		    && layer1.cur_anim != io->anims[ANIM_1H_STRIKE_BOTTOM_CYCLE]
		    && layer1.cur_anim != io->anims[ANIM_1H_STRIKE_LEFT]
		    && layer1.cur_anim != io->anims[ANIM_1H_STRIKE_RIGHT]
		    && layer1.cur_anim != io->anims[ANIM_1H_STRIKE_TOP]
		    && layer1.cur_anim != io->anims[ANIM_1H_STRIKE_BOTTOM]
		    && layer1.cur_anim != io->anims[ANIM_CAST_CYCLE]
		    && layer1.cur_anim != io->anims[ANIM_CAST]
		    && layer1.cur_anim != io->anims[ANIM_CAST_END]
		    && layer1.cur_anim != io->anims[ANIM_CAST_START])
		   || layer1.cur_anim == nullptr) {
			changeAnimation(io, 1, ANIM_1H_WAIT, EA_LOOP);
		}
	} else if(io->_npcdata->weapontype & OBJECT_TYPE_2H) {
		if((layer1.cur_anim != io->anims[ANIM_2H_WAIT]
		    && layer1.cur_anim != io->anims[ANIM_2H_READY_PART_1]
		    && layer1.cur_anim != io->anims[ANIM_2H_READY_PART_2]
		    && layer1.cur_anim != io->anims[ANIM_2H_STRIKE_LEFT_START]
		    && layer1.cur_anim != io->anims[ANIM_2H_STRIKE_RIGHT_START]
		    && layer1.cur_anim != io->anims[ANIM_2H_STRIKE_TOP_START]
		    && layer1.cur_anim != io->anims[ANIM_2H_STRIKE_BOTTOM_START]
		    && layer1.cur_anim != io->anims[ANIM_2H_STRIKE_LEFT_CYCLE]
		    && layer1.cur_anim != io->anims[ANIM_2H_STRIKE_RIGHT_CYCLE]
		    && layer1.cur_anim != io->anims[ANIM_2H_STRIKE_TOP_CYCLE]
		    && layer1.cur_anim != io->anims[ANIM_2H_STRIKE_BOTTOM_CYCLE]
		    && layer1.cur_anim != io->anims[ANIM_2H_STRIKE_LEFT]
		    && layer1.cur_anim != io->anims[ANIM_2H_STRIKE_RIGHT]
		    && layer1.cur_anim != io->anims[ANIM_2H_STRIKE_TOP]
		    && layer1.cur_anim != io->anims[ANIM_2H_STRIKE_BOTTOM]
		    && layer1.cur_anim != io->anims[ANIM_CAST_CYCLE]
		    && layer1.cur_anim != io->anims[ANIM_CAST]
		    && layer1.cur_anim != io->anims[ANIM_CAST_END]
		    && layer1.cur_anim != io->anims[ANIM_CAST_START])
		   || layer1.cur_anim == nullptr) {
			changeAnimation(io, 1, ANIM_2H_WAIT, EA_LOOP);
		}
	}
}

static void ARX_NPC_Manage_Anims_End(Entity * io) {
	
	AnimLayer & layer0 = io->animlayer[0];
	if((layer0.flags & EA_ANIMEND) && layer0.cur_anim) {
		
		if(layer0.flags & EA_FORCEPLAY) {
			bool startAtBeginning = (io->_npcdata->behavior & BEHAVIOUR_FRIENDLY) != 0;
			changeAnimation(io, ANIM_DEFAULT, 0, startAtBeginning);
		}
		
		// some specific code for combat animation end management
		if(isCurrentAnimation(io, 0, ANIM_FIGHT_STRAFE_LEFT)
		   || isCurrentAnimation(io, 0, ANIM_FIGHT_STRAFE_RIGHT)
		   || isCurrentAnimation(io, 0, ANIM_FIGHT_WALK_BACKWARD)) {
			changeAnimation(io, ANIM_FIGHT_WAIT, EA_LOOP);
		}
		
	}
}

static bool TryIOAnimMove(Entity * io, long animnum) {
	
	if(!io || !io->anims[animnum]) {
		return false;
	}
	
	Vec3f trans = GetAnimTotalTranslate(io->anims[animnum], 0);
	Vec3f trans2 = VRotateY(trans, MAKEANGLE(180.f - io->angle.getYaw()));
	
	IO_PHYSICS phys = io->physics;
	phys.cyl = getEntityCylinder(*io);
	
	phys.startpos = io->pos;
	phys.targetpos = io->pos + trans2;
	bool res = ARX_COLLISION_Move_Cylinder(&phys, io, 30, CFLAG_JUST_TEST | CFLAG_NPC);
	
	return (res && glm::abs(phys.cyl.origin.y - io->pos.y) < 20.f);
}

static void TryAndCheckAnim(Entity * io, long animnum, long layerIndex) {
	
	if(!io) {
		return;
	}
	
	const AnimLayer & layer = io->animlayer[layerIndex];
	if(layer.cur_anim != io->anims[animnum] && layer.cur_anim) {
		if(TryIOAnimMove(io, animnum)) {
			changeAnimation(io, layerIndex, AnimationNumber(animnum));
		}
	}
}

// Define Time of Strike Damage
static const float STRIKE_MUL = 0.25f;
static const float STRIKE_MUL2 = 0.8f;
static const float STRIKE_DISTANCE = 220.f;

//! Main animations management
static void ARX_NPC_Manage_Anims(Entity * io, float TOLERANCE) {
	
	const AnimLayer & layer0 = io->animlayer[0];
	AnimLayer & layer1 = io->animlayer[1];
	
	float tdist = std::numeric_limits<float>::max();
	if(Entity * target = entities.get(io->_npcdata->pathfind.truetarget); target && io->_npcdata->pathfind.listnb) {
		tdist = arx::distance2(io->pos, target->pos);
	} else if(Entity * fallback = entities.get(io->targetinfo)) {
		tdist = arx::distance2(io->pos, fallback->pos);
	}
	
	if(ValidIOAddress(io->_npcdata->weapon)) {
		io->_npcdata->weapontype = io->_npcdata->weapon->type_flags;
	} else {
		io->_npcdata->weapontype = 0;
	}
	
	
	// Manage combat movement
	
	if((io->_npcdata->behavior & BEHAVIOUR_FIGHT) && tdist <= square(TOLERANCE + 10)
	   && (tdist <= square(TOLERANCE - 20) || Random::getf() > 0.97f)
	   && isCurrentAnimation(io, 0, ANIM_FIGHT_WAIT)) {
		// Evade during combat
		
		float r = (tdist < square(TOLERANCE - 20)) ? 0.f : Random::getf();
		if(r < 0.1f) {
			TryAndCheckAnim(io, ANIM_FIGHT_WALK_BACKWARD, 0);
		} else if(r < 0.55f) {
			TryAndCheckAnim(io, ANIM_FIGHT_STRAFE_LEFT, 0);
		} else {
			TryAndCheckAnim(io, ANIM_FIGHT_STRAFE_RIGHT, 0);
		}
		
	} else if(((io->_npcdata->behavior & (BEHAVIOUR_MAGIC | BEHAVIOUR_DISTANT))
	           || io->spellcast_data.castingspell != SPELL_NONE)
	          && Random::getf() > 0.85f && isCurrentAnimation(io, 0, ANIM_FIGHT_WAIT) ) {
		// Evade while (not) casting
		
		AcquireLastAnim(io);
		FinishAnim(io, layer0.cur_anim);
		float r = (tdist < square(340)) ? 0.f : Random::getf();
		if(r < 0.33f) {
			TryAndCheckAnim(io, ANIM_FIGHT_WALK_BACKWARD, 0);
		} else if(r < 0.66f) {
			TryAndCheckAnim(io, ANIM_FIGHT_STRAFE_LEFT, 0);
		} else {
			TryAndCheckAnim(io, ANIM_FIGHT_STRAFE_RIGHT, 0);
		}
		
	}
	
	if(IsPlayerStriking() && isCurrentAnimation(io, 0, ANIM_FIGHT_WAIT)) {
		AcquireLastAnim(io);
		FinishAnim(io, layer0.cur_anim);
		float r = Random::getf();
		if(r < 0.2f) {
			TryAndCheckAnim(io, ANIM_FIGHT_WALK_BACKWARD, 0);
		} else if(r < 0.6f) {
			TryAndCheckAnim(io, ANIM_FIGHT_STRAFE_LEFT, 0);
		} else {
			TryAndCheckAnim(io, ANIM_FIGHT_STRAFE_RIGHT, 0);
		}
	}
	
	
	// Manage casting animation
	
	if(layer1.flags & EA_ANIMEND) {
		if(isCurrentAnimation(io, 1, ANIM_CAST)) {
			changeAnimation(io, 1, ANIM_CAST_END);
		} else if(isCurrentAnimation(io, 1, ANIM_CAST_END)) {
			stopAnimation(io, 1);
		}
	}
	
	if(io->spellcast_data.castingspell == SPELL_NONE
	   && isCurrentAnimation(io, 1, ANIM_CAST_START)) {
		changeAnimation(io, 1, ANIM_CAST);
	}
	
	if(io->spellcast_data.castingspell != SPELL_NONE) {
		return;
	}
	
	if(isCurrentAnimation(io, 1, ANIM_CAST_CYCLE)
	   || isCurrentAnimation(io, 1, ANIM_CAST)
	   || isCurrentAnimation(io, 1, ANIM_CAST_END)
	   || isCurrentAnimation(io, 1, ANIM_CAST_START)) {
		return;
	}
	
	
	// Manage fighting animations
	
	if(io->_npcdata->weapontype == 0) {
		// Hand to hand combat
		
		if(io->_npcdata->weaponinhand == -1) {
			io->_npcdata->weaponinhand = 1;
		}
		
		if(isCurrentAnimation(io, 1, ANIM_BARE_WAIT) && (io->_npcdata->behavior & BEHAVIOUR_FIGHT)
			 && tdist < square(STRIKE_DISTANCE)) {
			size_t j = Random::getu(0, 3); // Choose a random attack move
			changeAnimation(io, 1, AnimationNumber(ANIM_BARE_STRIKE_LEFT_START + j * 3));
		}
		
		for(size_t j = 0; j < 4; j++) {
			
			AnimationNumber start = AnimationNumber(ANIM_BARE_STRIKE_LEFT_START + j * 3);
			AnimationNumber cycle = AnimationNumber(ANIM_BARE_STRIKE_LEFT_CYCLE + j * 3);
			AnimationNumber strike = AnimationNumber(ANIM_BARE_STRIKE_LEFT + j * 3);
			
			if(isCurrentAnimation(io, 1, start) && (layer1.flags & EA_ANIMEND)) {
				
				io->ioflags &= ~IO_HIT;
				changeAnimation(io, 1, cycle, EA_LOOP);
				io->_npcdata->aiming_start = g_gameTime.now();
				
			} else if(isCurrentAnimation(io, 1, cycle)) {
				
				GameDuration elapsed = g_gameTime.now() - io->_npcdata->aiming_start;
				GameDuration aimtime = io->_npcdata->aimtime;
				if((elapsed > aimtime || (elapsed * 2 > aimtime && Random::getf() > 0.9f))
				    && tdist < square(STRIKE_DISTANCE)) {
					changeAnimation(io, 1, strike);
					SendIOScriptEvent(nullptr, io, SM_STRIKE, "bare");
				}
				
			} else if(isCurrentAnimation(io, 1, strike)) {
				
				if(layer1.flags & EA_ANIMEND) {
					io->ioflags &= ~IO_HIT;
					changeAnimation(io, 1, ANIM_BARE_WAIT);
					if((io->ioflags & IO_NPC) && !io->_npcdata->reachedtarget) {
						layer1.cur_anim = nullptr;
					}
				} else if(!(io->ioflags & IO_HIT)) {
					AnimationDuration ctime = layer1.ctime;
					AnimationDuration animtime = layer1.cur_anim->anims[0]->anim_time;
					if(ctime > animtime * STRIKE_MUL && ctime <= animtime * STRIKE_MUL2) {
						CheckHit(io, 1.f);
						io->ioflags |= IO_HIT;
					}
				}
				
			}
			
		}
		
	} else if(io->_npcdata->weapontype & (OBJECT_TYPE_1H | OBJECT_TYPE_2H | OBJECT_TYPE_DAGGER)) {
		// Melee combat
		
		long wtype = 0;
		if(io->_npcdata->weapontype & OBJECT_TYPE_1H) {
			wtype = 0;
		} else if(io->_npcdata->weapontype & OBJECT_TYPE_2H) {
			wtype = ANIM_2H_READY_PART_1 - ANIM_1H_READY_PART_1;
		} else if(io->_npcdata->weapontype & OBJECT_TYPE_DAGGER) {
			wtype = ANIM_DAGGER_READY_PART_1 - ANIM_1H_READY_PART_1;
		}
		
		AnimationNumber ready = AnimationNumber(ANIM_1H_WAIT + wtype);
		
		if(io->_npcdata->weaponinhand == 2) {
			// Unequipping weapon
			
			AnimationNumber part1 = AnimationNumber(ANIM_1H_UNREADY_PART_1 + wtype);
			AnimationNumber part2 = AnimationNumber(ANIM_1H_UNREADY_PART_2 + wtype);
			if(isCurrentAnimation(io, 1, part1) && (layer1.flags & EA_ANIMEND)) {
				SetWeapon_Back(io);
				changeAnimation(io, 1, part2);
			} else if(isCurrentAnimation(io, 1, part2) && (layer1.flags & EA_ANIMEND)) {
				stopAnimation(io, 1);
				io->_npcdata->weaponinhand = 0;
			} else if(!isCurrentAnimation(io, 1, part1) && isCurrentAnimation(io, 1, part2)) {
				changeAnimation(io, 1, part1);
			}
			
		} else if(io->_npcdata->weaponinhand == -1) {
			// Equipping weapon
			
			AnimationNumber part1 = AnimationNumber(ANIM_1H_READY_PART_1 + wtype);
			AnimationNumber part2 = AnimationNumber(ANIM_1H_READY_PART_2 + wtype);
			if(isCurrentAnimation(io, 1, part1) && (layer1.flags & EA_ANIMEND)) {
				SetWeapon_On(io);
				changeAnimation(io, 1, part2);
			} else if(isCurrentAnimation(io, 1, part2) && (layer1.flags & EA_ANIMEND)) {
				if(io->_npcdata->behavior & BEHAVIOUR_FIGHT) {
					changeAnimation(io, 1, ready, EA_LOOP);
				} else {
					stopAnimation(io, 1);
				}
				io->_npcdata->weaponinhand = 1;
			} else if(!layer1.cur_anim || (layer1.flags & EA_ANIMEND)) {
				changeAnimation(io, 1, part1);
			}
			
		} else if(io->_npcdata->weaponinhand > 0) {
			// Weapon in hand... ready to strike
			
			if(isCurrentAnimation(io, 1, ready) && (io->_npcdata->behavior & BEHAVIOUR_FIGHT)
				 && tdist < square(STRIKE_DISTANCE)) {
				size_t j = Random::getu(0, 3); // Choose a random attack move
				changeAnimation(io, 1, AnimationNumber(ANIM_1H_STRIKE_LEFT_START + j * 3 + wtype));
			}
			
			for(size_t j = 0; j < 4; j++) {
				
				AnimationNumber start = AnimationNumber(ANIM_1H_STRIKE_LEFT_START + j * 3 + wtype);
				AnimationNumber cycle = AnimationNumber(ANIM_1H_STRIKE_LEFT_CYCLE + j * 3 + wtype);
				AnimationNumber strike = AnimationNumber(ANIM_1H_STRIKE_LEFT + j * 3 + wtype);
				
				if(isCurrentAnimation(io, 1, start) && (layer1.flags & EA_ANIMEND)) {
					
					changeAnimation(io, 1, cycle, EA_LOOP);
					io->_npcdata->aiming_start = g_gameTime.now();
					
				} else if(isCurrentAnimation(io, 1, cycle)) {
					
					GameDuration elapsed = g_gameTime.now() - io->_npcdata->aiming_start;
					GameDuration aimtime = io->_npcdata->aimtime;
					if((elapsed > aimtime || (elapsed * 2 > aimtime && Random::getf() > 0.9f))
					   && tdist < square(STRIKE_DISTANCE)) {
						changeAnimation(io, 1, strike);
						if(io->_npcdata->weapontype & OBJECT_TYPE_1H) {
							SendIOScriptEvent(nullptr, io, SM_STRIKE, "1h");
						}
						if(io->_npcdata->weapontype & OBJECT_TYPE_2H) {
							SendIOScriptEvent(nullptr, io, SM_STRIKE, "2h");
						}
						if(io->_npcdata->weapontype & OBJECT_TYPE_DAGGER) {
							SendIOScriptEvent(nullptr, io, SM_STRIKE, "dagger");
						}
					}
					
				} else if(isCurrentAnimation(io, 1, strike)) {
					
					if(layer1.flags & EA_ANIMEND) {
						io->ioflags &= ~IO_HIT;
						changeAnimation(io, 1, ready);
					} else {
						AnimationDuration ctime = layer1.ctime;
						AnimationDuration animtime = layer1.cur_anim->anims[0]->anim_time;
						
						if(   ctime > animtime * STRIKE_MUL
						   && ctime <= animtime * STRIKE_MUL2
						) {
							if(!(io->ioflags & IO_HIT)) {
								if(ARX_EQUIPMENT_Strike_Check(io, io->_npcdata->weapon, 1, 0, io->targetinfo)) {
									io->ioflags |= IO_HIT;
								}
							} else {
								ARX_EQUIPMENT_Strike_Check(io, io->_npcdata->weapon, 1, 1, io->targetinfo);
							}
						}
					}
				}
			}
		}
	} else if(io->_npcdata->weapontype & OBJECT_TYPE_BOW) {
		// TODO ranged combat
	}
}

float getEntityHeight(const Entity & entity) {
	
	if(entity == *entities.player()) {
		return entity.physics.cyl.height;
	}
	
	return glm::clamp(entity.original_height * entity.scale, -165.f, -45.f);
}

float getEntityRadius(const Entity & entity) {
	
	if(entity == *entities.player()) {
		return player.baseRadius();
	}
	
	return glm::clamp(entity.original_radius * entity.scale, 25.f, 60.f);
}

Cylinder getEntityCylinder(const Entity & entity) {
	return Cylinder(entity.pos, getEntityRadius(entity), getEntityHeight(entity));
}


static void ManageNPCMovement_check_target_reached(Entity * io);
static void ManageNPCMovement_REFACTOR_end(Entity * io, float TOLERANCE2);

//! Computes distance tolerance between NPC and its target
static float ComputeTolerance(const Entity * io, EntityHandle targ) {
	
	float TOLERANCE = 30.f;
	
	if(Entity * target = entities.get(targ)) {
		
		float self_dist, targ_dist;
		
		// Compute min target close-dist
		if(target->ioflags & IO_NO_COLLISIONS) {
			targ_dist = 0.f;
		} else {
			targ_dist = std::max(target->physics.cyl.radius, getEntityRadius(*target));
		}
		
		// Compute min self close-dist
		if(io->ioflags & IO_NO_COLLISIONS) {
			self_dist = 0.f;
		} else {
			self_dist = std::max(io->physics.cyl.radius, getEntityRadius(*io));
		}
		
		// Base tolerance = radius added
		TOLERANCE = std::max(targ_dist + self_dist + 5.f, 0.f);
		
		if(target->ioflags & IO_FIX) {
			TOLERANCE += 100.f;
		}
		
		// If target is a NPC add another tolerance
		// TODO this also matches items, props and cameras
		if(target->_npcdata) {
			TOLERANCE += 20.f;
		}
		
		// If target is the player improve again tolerance
		if(io->targetinfo == EntityHandle_Player) {
			TOLERANCE += 10.f;
		}
		
		if(io->_npcdata->behavior & BEHAVIOUR_FIGHT) {
			TOLERANCE += io->_npcdata->reach * 0.7f;
		}
		
		// If distant of magic behavior Maximize tolerance
		if((io->_npcdata->behavior & (BEHAVIOUR_MAGIC | BEHAVIOUR_DISTANT)) || (io->spellcast_data.castingspell != SPELL_NONE)) {
			TOLERANCE += 300.f;
		}
		
		// If target is a marker set to a minimal tolerance
		if(target->ioflags & IO_MARKER) {
			TOLERANCE = 21.f + io->_npcdata->moveproblem * 0.1f;
		}
		
	}
	
	// Tolerance is modified by current moveproblem status
	TOLERANCE += io->_npcdata->moveproblem * 0.1f;
	
	return TOLERANCE;
}

static void ManageNPCMovement_End(Entity * io) {
	
	AnimLayer & layer0 = io->animlayer[0];
	const auto & alist = io->anims;
	
	if(io->_npcdata->behavior & BEHAVIOUR_NONE) {
		ARX_NPC_Manage_Anims(io, 0);
		if(!layer0.cur_anim || (layer0.flags & EA_ANIMEND)) {
			changeAnimation(io, ANIM_WAIT);
			layer0.flags &= ~EA_LOOP;
		}
		return;
	}
	
	// First retrieves current position of target...
	GetTargetPos(io);
	
	ARX_NPC_Manage_Anims_End(io);
	
	if((io->_npcdata->behavior & BEHAVIOUR_LOOK_AROUND)
	   && io->_npcdata->pathfind.listnb <= 0
	   && !io->_npcdata->pathfind.pathwait
	) {
		ARX_NPC_LaunchPathfind(io, io->targetinfo);
	}
	
	if((io->_npcdata->behavior & (BEHAVIOUR_FRIENDLY | BEHAVIOUR_NONE))
	   && (layer0.cur_anim == alist[ANIM_WALK]
	       || layer0.cur_anim == alist[ANIM_WALK_SNEAK]
	       || layer0.cur_anim == alist[ANIM_FIGHT_WALK_FORWARD]
	       || layer0.cur_anim == alist[ANIM_RUN]
	       || layer0.cur_anim == alist[ANIM_FIGHT_STRAFE_LEFT]
	       || layer0.cur_anim == alist[ANIM_FIGHT_STRAFE_RIGHT]
	       || layer0.cur_anim == alist[ANIM_FIGHT_WALK_BACKWARD])) {
		changeAnimation(io, ANIM_WAIT, 0, true);
	}
	
	// look around if finished fleeing or being looking around !
	if((io->_npcdata->behavior & BEHAVIOUR_LOOK_AROUND) && fartherThan(io->pos, io->target, 150.f)) {
		
		if(!io->_npcdata->ex_rotate) {
			ARX_NPC_CreateExRotateData(io);
		} else { // already created
			EERIE_EXTRA_ROTATE * extraRotation = io->_npcdata->ex_rotate;
			
			if((layer0.cur_anim == alist[ANIM_WAIT] || layer0.cur_anim == alist[ANIM_WALK]
			    || layer0.cur_anim == alist[ANIM_WALK_SNEAK] || layer0.cur_anim == alist[ANIM_FIGHT_WALK_FORWARD]
			    || layer0.cur_anim == alist[ANIM_RUN])
			   || layer0.cur_anim != nullptr /* TODO check this */) {
				
				io->_npcdata->look_around_inc = 0.f;
				
				for(Anglef & rotation : extraRotation->group_rotate) {
					rotation.setYaw(rotation.getYaw() - rotation.getYaw() * (1.0f / 3));
					if(glm::abs(rotation.getYaw()) < 0.01f) {
						rotation.setYaw(0.f);
					}
				}
				
			} else {
				
				if(io->_npcdata->look_around_inc == 0.f) {
					io->_npcdata->look_around_inc = Random::getf(-0.04f, 0.04f);
				}
				
				for(size_t n = 0; n < extraRotation->group_rotate.size(); n++) {
					Anglef & rotation = extraRotation->group_rotate[n];
					float t = 1.5f - float(n) * 0.2f;
					rotation.setYaw(rotation.getYaw() + io->_npcdata->look_around_inc * g_framedelay * t);
				}
				
				if(extraRotation->group_rotate[0].getYaw() > 30) {
					io->_npcdata->look_around_inc = -io->_npcdata->look_around_inc;
				}
				
				if(extraRotation->group_rotate[0].getYaw() < -30) {
					io->_npcdata->look_around_inc = -io->_npcdata->look_around_inc;
				}
				
			}
			
		}
		
	} else if(!(io->_npcdata->behavior & BEHAVIOUR_STARE_AT) && io->_npcdata->ex_rotate) {
		
		io->_npcdata->look_around_inc = 0.f;
		for(Anglef & rotation : io->_npcdata->ex_rotate->group_rotate) {
			rotation.setYaw(rotation.getYaw() - rotation.getYaw() * (1.0f / 3));
			if(glm::abs(rotation.getYaw()) < 0.01f) {
				rotation.setYaw(0.f);
			}
		}
		
	}
	
	layer0.flags &= ~EA_STATICANIM;
	
	if(layer0.cur_anim && (layer0.cur_anim == alist[ANIM_HIT1] || layer0.cur_anim == alist[ANIM_HIT_SHORT])) {
		if(layer0.flags & EA_ANIMEND) {
			bool startAtBeginning = (io->_npcdata->behavior & BEHAVIOUR_FRIENDLY) != 0;
			if(io->_npcdata->behavior & BEHAVIOUR_FIGHT) {
				changeAnimation(io, ANIM_FIGHT_WAIT, 0, startAtBeginning);
			} else {
				changeAnimation(io, ANIM_WAIT, 0, startAtBeginning);
			}
		} else {
			return;
		}
	}
	
	// GetTargetPos MUST be called before FaceTarget2
	if(io->_npcdata->pathfind.listnb > 0
	   && (io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND)
	   && (io->_npcdata->pathfind.listpos < io->_npcdata->pathfind.listnb - 2)
	   && (io->_npcdata->pathfind.list[io->_npcdata->pathfind.listpos]
	       == io->_npcdata->pathfind.list[io->_npcdata->pathfind.listpos + 1])) {
		if(layer0.cur_anim != io->anims[ANIM_DEFAULT]) {
			bool startAtBeginning = (io->_npcdata->behavior & BEHAVIOUR_FRIENDLY) != 0;
			changeAnimation(io, ANIM_DEFAULT, 0, startAtBeginning);
		} else if(layer0.flags & EA_ANIMEND) {
			layer0.flags &= ~EA_FORCEPLAY;
			ManageNPCMovement_check_target_reached(io);
			ManageNPCMovement_REFACTOR_end(io, 0.f);
		}
		return;
	}
	
	// XS : Moved to top of func
	float _dist = glm::distance(getXZ(io->pos), getXZ(io->target));
	float dis = _dist;
	
	if(io->_npcdata->pathfind.listnb > 0) {
		dis = GetTRUETargetDist(io);
	}
	
	if(io->_npcdata->behavior & BEHAVIOUR_FLEE) {
		dis = 9999999;
	}
	
	// Force to flee/wander again
	if(!io->_npcdata->pathfind.pathwait && io->_npcdata->pathfind.listnb <= 0) {
		if(io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND) {
			ARX_NPC_LaunchPathfind(io, io->targetinfo);
		} else if(dis > STRIKE_DISTANCE && (io->_npcdata->behavior & BEHAVIOUR_MOVE_TO)
		          && !(io->_npcdata->behavior & (BEHAVIOUR_FIGHT | BEHAVIOUR_MAGIC | BEHAVIOUR_SNEAK))) {
			ARX_NPC_LaunchPathfind(io, io->targetinfo);
		}
	}
	
	if(io->_npcdata->pathfind.listnb <= 0 && (io->_npcdata->behavior & BEHAVIOUR_FLEE)) {
		bool startAtBeginning = (io->_npcdata->behavior & BEHAVIOUR_FRIENDLY) != 0;
		setAnimation(io, ANIM_DEFAULT, 0, startAtBeginning);
	} else { // Force object to walk if using pathfinder !!!
		
		if(io->_npcdata->behavior & (BEHAVIOUR_FRIENDLY | BEHAVIOUR_NONE)) {
		} else if(!io->_npcdata->reachedtarget) {
			// Walking/running towards target
			
			if((io->targetinfo != EntityHandle(TARGET_NONE)
			     || (io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND)
			     || (io->_npcdata->behavior & BEHAVIOUR_FLEE)
			     || (io->_npcdata->behavior & BEHAVIOUR_GO_HOME))
				&& io->_npcdata->pathfind.listnb > 0
				&& layer0.cur_anim != alist[ANIM_WALK]
				&& layer0.cur_anim != alist[ANIM_FIGHT_WALK_FORWARD]
				&& layer0.cur_anim != alist[ANIM_RUN]
				&& layer0.cur_anim != alist[ANIM_WALK_SNEAK]
				&& !(layer0.flags & EA_FORCEPLAY)
			) {
				
				bool startAtBeginning = false;
				AnimationNumber anim = ANIM_FIGHT_WALK_FORWARD;
				if(dis > RUN_WALK_RADIUS || !(io->_npcdata->behavior & BEHAVIOUR_FIGHT)) {
					switch(io->_npcdata->movemode) {
						case SNEAKMODE: anim = ANIM_WALK_SNEAK; break;
						case WALKMODE:  anim = ANIM_WALK;       break;
						case RUNMODE: {
							if(dis > RUN_WALK_RADIUS && alist[ANIM_RUN]) {
								anim = ANIM_RUN;
							} else {
								anim = ANIM_WALK;
							}
							break;
						}
						case NOMOVEMODE: {
							anim = ANIM_WAIT;
							startAtBeginning = (io->_npcdata->behavior & BEHAVIOUR_FRIENDLY) != 0;
							break;
						}
					}
				}
				changeAnimation(io, anim, EA_LOOP, startAtBeginning);
				io->_npcdata->walk_start_time = 0;
			}
			
		} else if((io->_npcdata->behavior & (BEHAVIOUR_FIGHT | BEHAVIOUR_MAGIC | BEHAVIOUR_DISTANT))
		          && io->anims[ANIM_FIGHT_WAIT]) {
			// Reached target while fighting
			
			if(layer0.cur_anim != alist[ANIM_FIGHT_STRAFE_LEFT]
			   && layer0.cur_anim != alist[ANIM_FIGHT_STRAFE_RIGHT]
			   && layer0.cur_anim != alist[ANIM_FIGHT_WALK_BACKWARD]
			   && layer0.cur_anim != alist[ANIM_FIGHT_WALK_FORWARD]) {
				setAnimation(io, ANIM_FIGHT_WAIT, EA_LOOP);
			}
			
		} else {
			// Stop it and put it in Wait anim after finishing his walk anim...
			
			bool startAtBeginning = (io->_npcdata->behavior & BEHAVIOUR_FRIENDLY) != 0;
			
			if(layer0.cur_anim == alist[ANIM_FIGHT_WALK_FORWARD]) {
				layer0.flags &= ~EA_LOOP;
				changeAnimation(io, ANIM_DEFAULT, EA_LOOP, startAtBeginning);
			} else if(layer0.cur_anim == alist[ANIM_WALK] || layer0.cur_anim == alist[ANIM_RUN]
			          || layer0.cur_anim == alist[ANIM_WALK_SNEAK]) {
				layer0.flags &= ~EA_LOOP;
				if(io->_npcdata->reachedtime + 500ms < g_gameTime.now()) {
					changeAnimation(io, ANIM_DEFAULT, EA_LOOP, startAtBeginning);
				}
			}
			
			if(!(layer0.flags & EA_FORCEPLAY) && layer0.cur_anim != alist[ANIM_DEFAULT]
			   && layer0.cur_anim != alist[ANIM_FIGHT_WAIT] && (layer0.flags & EA_ANIMEND)
			   && !(layer0.flags & EA_LOOP)) {
				changeAnimation(io, ANIM_DEFAULT, 0, startAtBeginning);
			}
			
		}
	}
	
	// Force Run when far from target and using RUNMODE
	if(dis > RUN_WALK_RADIUS && io->_npcdata->movemode == RUNMODE
	   && layer0.cur_anim == alist[ANIM_FIGHT_WALK_FORWARD] && alist[ANIM_RUN]) {
		changeAnimation(io, ANIM_RUN);
	}
	
	// Reset WAIT Animation if reached end !
	if(isCurrentAnimation(io, 0, ANIM_DEFAULT) && (layer0.flags & EA_ANIMEND)) {
		bool startAtBeginning = (io->_npcdata->behavior & BEHAVIOUR_FRIENDLY) != 0;
		changeAnimation(io, ANIM_DEFAULT, 0, startAtBeginning);
	}
	
	// Can only change direction during some specific animations
	long CHANGE = 0;
	if((layer0.cur_anim == alist[ANIM_DEFAULT] && layer0.altidx_cur == 0)
	   || layer0.cur_anim == alist[ANIM_FIGHT_WAIT]
	   || layer0.cur_anim == alist[ANIM_FIGHT_WALK_FORWARD]
	   || layer0.cur_anim == alist[ANIM_WALK]
	   || layer0.cur_anim == alist[ANIM_WALK_SNEAK]
	   || layer0.cur_anim == alist[ANIM_RUN]
	   || layer0.cur_anim == alist[ANIM_FIGHT_STRAFE_LEFT]
	   || layer0.cur_anim == alist[ANIM_FIGHT_STRAFE_RIGHT]
	   || layer0.cur_anim == alist[ANIM_FIGHT_WALK_BACKWARD]
	   || layer0.cur_anim == nullptr
	) {
		CHANGE = 1;
	}
	
	// Tries to face/stare at target
	if(!g_gameTime.isPaused() && CHANGE && !(layer0.flags & EA_FORCEPLAY)) {
		if(io->_npcdata->behavior & BEHAVIOUR_STARE_AT) {
			StareAtTarget(io);
		} else {
			FaceTarget2(io);
		}
	}
	
	
	float TOLERANCE = 0.f;
	float TOLERANCE2 = 0.f;
	
	// Choose tolerance value depending on target...
	if(io->_npcdata->pathfind.listnb > 0
	   && io->_npcdata->pathfind.listpos < io->_npcdata->pathfind.listnb
	) {
		// TODO is this cast to EntityHandle correct ?
		TOLERANCE  = ComputeTolerance(io, EntityHandle(io->_npcdata->pathfind.list[io->_npcdata->pathfind.listpos]));
		TOLERANCE2 = ComputeTolerance(io, io->_npcdata->pathfind.truetarget);
	} else {
		TOLERANCE  = ComputeTolerance(io, io->targetinfo);
		TOLERANCE2 = TOLERANCE;
	}
	
	// COLLISION Management START *********************************************************************
	// Try physics from last valid pos to current desired pos...
	// For this frame we want to try a move from startpos (valid pos)
	// to targetpos (potentially invalid pos)
	io->physics.startpos = io->physics.cyl.origin = io->pos;
	
	Vec3f ForcedMove(0.f);
	if(io->forcedmove != Vec3f(0.f)) {
		float dd = std::min(1.f, g_framedelay * (1.0f / 6) / glm::length(io->forcedmove));
		ForcedMove = io->forcedmove * dd;
	}
	
	// Sets Target position to desired position...
	io->physics.targetpos.x = io->pos.x + io->move.x + ForcedMove.x;
	io->physics.targetpos.z = io->pos.z + io->move.z + ForcedMove.z;
	
	IO_PHYSICS phys;
	phys = io->physics;
	phys.cyl = getEntityCylinder(*io);
	
	CollisionFlags levitate = 0;
	
	if(spells.getSpellOnTarget(io->index(), SPELL_LEVITATE)) {
		levitate = CFLAG_LEVITATE;
		io->physics.targetpos.y = io->pos.y + io->move.y + ForcedMove.y;
	} else { // Gravity 'simulation'
		phys.cyl.origin.y += 10.f;
		float anything = CheckAnythingInCylinder(phys.cyl, io, CFLAG_JUST_TEST | CFLAG_NPC);
		
		if(anything >= 0) {
			io->physics.targetpos.y = io->pos.y + g_framedelay * 1.5f + ForcedMove.y;
		} else {
			io->physics.targetpos.y = io->pos.y + ForcedMove.y;
		}
		
		phys.cyl.origin.y -= 10.f;
	}
	
	phys = io->physics;
	phys.cyl = getEntityCylinder(*io);
	
	io->forcedmove -= ForcedMove;
	
	DIRECT_PATH = true;
	
	// Now we try the physical move for real
	if(io->physics.startpos == io->physics.targetpos
	   || ARX_COLLISION_Move_Cylinder(&phys, io, 40, levitate | CFLAG_NPC)
	) {
		// Successfull move now validate it
		if(!DIRECT_PATH) {
			io->_npcdata->moveproblem += 1;
		} else {
			io->_npcdata->moveproblem = 0;
		}
	} else {
		// Object was unable to move to target... Stop it
		io->_npcdata->moveproblem += 3;
	}
	
	io->requestRoomUpdate = true;
	io->pos = phys.cyl.origin;
	io->physics.cyl = getEntityCylinder(*io);
	
	// Compute distance 2D to target.
	_dist = glm::distance(getXZ(io->pos), getXZ(io->target));
	dis = _dist;
	
	if(io->_npcdata->pathfind.listnb > 0) {
		dis = GetTRUETargetDist(io);
	}
	
	if(io->_npcdata->behavior & BEHAVIOUR_FLEE) {
		dis = 9999999;
	}
	
	// Tries to solve Moveproblems... sort of...
	if(io->_npcdata->moveproblem > 11) {
		if(_dist > TOLERANCE && !io->_npcdata->pathfind.pathwait) {
			EntityHandle targ;
			
			if(io->_npcdata->pathfind.listnb > 0) {
				targ = io->_npcdata->pathfind.truetarget;
			} else {
				targ = io->targetinfo;
			}
			
			ARX_NPC_LaunchPathfind(io, targ);
		}
		
		io->_npcdata->moveproblem = 0;
	}
	
	// Checks if pathfind final target is still locked on true target
	if(!io->_npcdata->pathfind.pathwait
	   && !(io->_npcdata->pathfind.flags & PATHFIND_ONCE)
	   && !(io->_npcdata->pathfind.flags & PATHFIND_NO_UPDATE)
	   && io->_npcdata->pathfind.listnb > 0
	   && io->_npcdata->pathfind.listpos < io->_npcdata->pathfind.listnb
	   && (io->_npcdata->behavior & BEHAVIOUR_MOVE_TO)
	   && !(io->_npcdata->behavior & BEHAVIOUR_FLEE)
	) {
		if(Entity * target = entities.get(io->_npcdata->pathfind.truetarget)) {
			long t = AnchorData_GetNearest(target->pos, io->physics.cyl);
			if(t != -1 && t != io->_npcdata->pathfind.list[io->_npcdata->pathfind.listnb - 1]) {
				long anchor = io->_npcdata->pathfind.list[io->_npcdata->pathfind.listnb - 1];
				float d = glm::distance(g_anchors[t].pos, g_anchors[anchor].pos);
				if(d > 200.f) {
					ARX_NPC_LaunchPathfind(io, io->_npcdata->pathfind.truetarget);
				}
			}
		}
	}
	
	// We are still too far from our target...
	if(io->_npcdata->pathfind.pathwait == 0) {
		if(_dist > TOLERANCE && dis > TOLERANCE2) {
			
			if(io->_npcdata->reachedtarget) {
				SendIOScriptEvent(entities.get(io->targetinfo), io, SM_LOSTTARGET);
				io->_npcdata->reachedtarget = 0;
			}
			
			// If not blocked and not Flee-Pathfinding
			if(io->_npcdata->pathfind.listnb > 0 ||
			   !(io->_npcdata->behavior & BEHAVIOUR_FLEE) ||
			   (io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND) ||
			   (io->_npcdata->behavior & BEHAVIOUR_GO_HOME)) {
				
				ANIM_HANDLE * desiredanim = nullptr;
				
				if(dis <= RUN_WALK_RADIUS
				   && (io->_npcdata->behavior & BEHAVIOUR_FIGHT)
				   && layer0.cur_anim != alist[ANIM_RUN]
				) {
					io->_npcdata->walk_start_time += g_gameTime.lastFrameDuration();
					if(io->_npcdata->walk_start_time > 600ms) {
						desiredanim = alist[ANIM_FIGHT_WALK_FORWARD];
						io->_npcdata->walk_start_time = 0;
					}
				} else {
					switch(io->_npcdata->movemode) {
						case SNEAKMODE:
							desiredanim = alist[ANIM_WALK_SNEAK];
							break;
						case WALKMODE:
							desiredanim = alist[ANIM_WALK];
							break;
						case RUNMODE:
							if(dis <= RUN_WALK_RADIUS) {
								desiredanim = alist[ANIM_WALK];
							} else {
								desiredanim = alist[ANIM_RUN];
							}
							
							break;
						case NOMOVEMODE:
							desiredanim = alist[ANIM_DEFAULT];
							break;
					}
				}
				
				if(io->targetinfo == EntityHandle(TARGET_NONE)) {
					desiredanim = alist[ANIM_DEFAULT];
				}
				
				if(desiredanim && !(layer0.flags & EA_FORCEPLAY)
				   && (layer0.cur_anim == alist[ANIM_DEFAULT]
				       || layer0.cur_anim == alist[ANIM_FIGHT_WAIT])) {
					bool startAtBeginning = false;
					if(desiredanim == alist[ANIM_DEFAULT]) {
						startAtBeginning = (io->_npcdata->behavior & BEHAVIOUR_FRIENDLY) != 0;
					}
					setAnimation(io, desiredanim, EA_LOOP, startAtBeginning);
				}
				
			}
		} else { // Near target
			if(dis <= TOLERANCE2) {
				io->_npcdata->pathfind.listpos = 0;
				io->_npcdata->pathfind.listnb = -1;
				io->_npcdata->pathfind.pathwait = 0;
				
				delete[] io->_npcdata->pathfind.list;
				io->_npcdata->pathfind.list = nullptr;
				
				if(layer0.cur_anim == alist[ANIM_FIGHT_WALK_FORWARD]) {
					layer0.flags &= ~EA_LOOP;
					bool startAtBeginning = (io->_npcdata->behavior & BEHAVIOUR_FRIENDLY) != 0;
					changeAnimation(io, ANIM_DEFAULT, EA_LOOP, startAtBeginning);
				}
			}
			
			if(io->_npcdata->pathfind.listnb > 0) {
				ManageNPCMovement_check_target_reached(io);
			} else if(!io->_npcdata->reachedtarget) {
				
				io->_npcdata->reachedtarget = 1;
				io->_npcdata->reachedtime = g_gameTime.now();
				
				if(io->animlayer[1].flags & EA_ANIMEND) {
					io->animlayer[1].cur_anim = nullptr;
				}
				
				if(io->targetinfo != io->index()) {
					SendIOScriptEvent(entities.get(io->targetinfo), io, SM_REACHEDTARGET);
				}
				
			}
		}
	}
	
	if(dis < 280.f) {
		if((io->_npcdata->behavior & BEHAVIOUR_FIGHT) && !(io->_npcdata->behavior & BEHAVIOUR_FLEE)) {
			ARX_NPC_Manage_Fight(io);
		} else {
			ARX_NPC_Manage_NON_Fight(io);
		}
	}
	
	ManageNPCMovement_REFACTOR_end(io, TOLERANCE2);
	
}

static void ManageNPCMovement(Entity * io) {
	
	ARX_PROFILE_FUNC();
	
	// Ignores invalid or dead IO
	if(!io || !(io->ioflags & IO_NPC)) {
		return;
	}
	
	// Specific USEPATH management
	ARX_USE_PATH * aup = io->usepath;
	
	if(aup && (aup->aupflags & ARX_USEPATH_WORM_SPECIFIC)) {
		io->requestRoomUpdate = true;
		Vec3f tv;
		
		if(aup->_curtime - aup->_starttime > 500ms) {
			aup->_curtime -= 500ms;
			ARX_PATHS_Interpolate(aup, &tv);
			aup->_curtime += 500ms;
			io->angle.setYaw(MAKEANGLE(glm::degrees(getAngle(tv.x, tv.z, io->pos.x, io->pos.z))));
		} else {
			aup->_curtime += 500ms;
			ARX_PATHS_Interpolate(aup, &tv);
			aup->_curtime -= 500ms;
			io->angle.setYaw(MAKEANGLE(180.f + glm::degrees(getAngle(tv.x, tv.z, io->pos.x, io->pos.z))));
		}
		return;
	}
	
	// Frozen ?
	if(io->ioflags & IO_FREEZESCRIPT) {
		return;
	}
	
	// Dead ?
	if(IsDeadNPC(*io)) {
		io->ioflags |= IO_NO_COLLISIONS;
		return;
	}
	
	AnimLayer & layer0 = io->animlayer[0];
	
	// Using USER animation ?
	if(layer0.cur_anim
	   && (layer0.flags & EA_FORCEPLAY)
	   && layer0.cur_anim != io->anims[ANIM_DIE]
	   && layer0.cur_anim != io->anims[ANIM_HIT1]
	   && layer0.cur_anim != io->anims[ANIM_HIT_SHORT]
	   && !(layer0.flags & EA_ANIMEND)
	) {
		io->requestRoomUpdate = true;
		io->lastpos = (io->pos += io->move);
		return;
	}
	
	if(io->_npcdata->pathfind.listnb > 0 && !io->_npcdata->pathfind.list) {
		io->_npcdata->pathfind.listnb = 0;
	}
	
	// Waiting for pathfinder or pathfinder failure ---> wait anim
	if(io->_npcdata->pathfind.pathwait || io->_npcdata->pathfind.listnb == -2) {
		
		if(io->_npcdata->pathfind.listnb == -2) {
			if(!io->_npcdata->pathfind.pathwait) {
				if(io->_npcdata->pathfind.flags & PATHFIND_NO_UPDATE) {
					io->_npcdata->reachedtarget = 1;
					io->_npcdata->reachedtime = g_gameTime.now();
					if(io->targetinfo != io->index()) {
						SendIOScriptEvent(nullptr, io, SM_REACHEDTARGET);
					}
				} else if(layer0.cur_anim == io->anims[ANIM_WAIT] && (layer0.flags & EA_ANIMEND)) {
					io->_npcdata->pathfind.listnb = -1;
					io->_npcdata->pathfind.pathwait = 0;
					ARX_NPC_LaunchPathfind(io, io->targetinfo);
					ManageNPCMovement_End(io);
					return;
				}
			}
		}
		
		if(!(io->_npcdata->behavior & BEHAVIOUR_FIGHT)) {
			if(layer0.cur_anim == io->anims[ANIM_WALK]
			   || layer0.cur_anim == io->anims[ANIM_RUN]
			   || layer0.cur_anim == io->anims[ANIM_WALK_SNEAK]
			) {
				return changeAnimation(io, ANIM_WAIT, 0, true);
			} else if(layer0.cur_anim == io->anims[ANIM_WAIT]) {
				if(layer0.flags & EA_ANIMEND) {
					// TODO why no AcquireLastAnim(io) like everywhere else?
					FinishAnim(io, layer0.cur_anim);
					ANIM_Set(layer0, io->anims[ANIM_WAIT]);
					layer0.altidx_cur = 0;
				}
				return;
			}
		}
		
	}
	
	ManageNPCMovement_End(io);
}

static void ManageNPCMovement_check_target_reached(Entity * io) {
	
	long lMax = std::max(ARX_NPC_GetNextAttainableNodeIncrement(io), 1l);
	
	io->_npcdata->pathfind.listpos = util::to<unsigned short>(io->_npcdata->pathfind.listpos + lMax);
	
	if(io->_npcdata->pathfind.listpos >= io->_npcdata->pathfind.listnb) // || (dis<=120.f))
	{
		io->_npcdata->pathfind.listpos = 0;
		io->_npcdata->pathfind.listnb = -1;
		io->_npcdata->pathfind.pathwait = 0;
		
		delete[] io->_npcdata->pathfind.list;
		io->_npcdata->pathfind.list = nullptr;
		
		if((io->_npcdata->behavior & BEHAVIOUR_FLEE) && !io->_npcdata->pathfind.pathwait) {
			SendIOScriptEvent(nullptr, io, "flee_end");
		}
		
		if((io->_npcdata->pathfind.flags & PATHFIND_NO_UPDATE) && io->_npcdata->pathfind.pathwait == 0) {
			if(!io->_npcdata->reachedtarget) {
				EntityHandle num = io->index();
				io->_npcdata->reachedtarget = 1;
				io->_npcdata->reachedtime = g_gameTime.now();
				if(io->targetinfo != num) {
					SendIOScriptEvent(nullptr, io, SM_REACHEDTARGET, "fake");
					io->targetinfo = num;
				}
			}
		} else {
			io->targetinfo = io->_npcdata->pathfind.truetarget;
			GetTargetPos(io);
			
			if(glm::abs(io->pos.y - io->target.y) > 200.f) {
				io->_npcdata->pathfind.listnb = -2;
			}
		}
		
	}
}


static void ManageNPCMovement_REFACTOR_end(Entity * io, float TOLERANCE2) {
	
	ARX_NPC_Manage_Anims(io, TOLERANCE2);
	
	AnimLayer & layer0 = io->animlayer[0];
	
	// Puts at least WAIT anim on NPC if he has no main animation...
	if(layer0.cur_anim == nullptr) {
		if(io->_npcdata->behavior & (BEHAVIOUR_FIGHT | BEHAVIOUR_MAGIC | BEHAVIOUR_DISTANT)) {
			// TODO why no AcquireLastAnim() like everywhere else?
			FinishAnim(io, layer0.cur_anim);
			ANIM_Set(layer0, io->anims[ANIM_FIGHT_WAIT]);
			layer0.flags |= EA_LOOP;
		} else {
			// TODO why no AcquireLastAnim() like everywhere else?
			FinishAnim(io, layer0.cur_anim);
			ANIM_Set(layer0, io->anims[ANIM_WAIT]);
			if(io->_npcdata->behavior & BEHAVIOUR_FRIENDLY) {
				layer0.altidx_cur = 0;
			}
		}
	}
	
	// Now update lastpos values for next call use...
	arx_assert(isallfinite(io->pos));
	io->lastpos = io->pos;
}

static float AngularDifference(float a1, float a2) {
	
	a1 = MAKEANGLE(a1);
	a2 = MAKEANGLE(a2);
	
	if(a1 == a2) {
		return 0;
	}
	
	float ret;
	
	if(a1 < a2) {
		ret = a2;
		a2 = a1;
		a1 = ret;
	}
	
	ret = a1 - a2;
	ret = std::min(ret, (a2 + 360) - a1);
	return ret;
	
}

/*!
 * \brief ARX_NPC_GetFirstNPCInSight
 * \return the "first" NPC in sight for another NPC (ioo)
 */
Entity * getFirstNpcInSight(const Entity & source) {
	
	// Basic Clipping to avoid performance loss
	if(fartherThan(g_camera->m_pos, source.pos, 2500)) {
		return nullptr;
	}
	
	Entity * closestNpc = nullptr;
	float closestNpcDistance2 = std::numeric_limits<float>::max();
	
	for(Entity & npc : entities.inScene(IO_NPC)) {
		
		if(IsDeadNPC(npc) || npc == source) {
			continue;
		}
		
		float dist_io = arx::distance2(npc.pos, source.pos);
		if(dist_io > closestNpcDistance2 || dist_io > square(1800)) {
			continue;
		}
		
		if(dist_io < square(130)) {
			if(closestNpcDistance2 > dist_io) {
				closestNpc = &npc;
				closestNpcDistance2 = dist_io;
			}
			continue;
		}
		
		float ab = MAKEANGLE(source.angle.getYaw());
		
		Vec3f orgn = source.pos + Vec3f(0.f, -90.f, 0.f);
		if(VertexId sourceHead = source.obj->fastaccess.head_group_origin) {
			orgn = source.obj->vertexWorldPositions[sourceHead].v;
		} else if(source == *entities.player()) {
			orgn.y = player.pos.y + 90.f;
		}
		
		Vec3f dest = npc.pos + Vec3f(0.f, -90.f, 0.f);
		if(VertexId npcHead = npc.obj->fastaccess.head_group_origin) {
			dest = npc.obj->vertexWorldPositions[npcHead].v;
		} else if(npc == *entities.player()) {
			dest.y = player.pos.y + 90.f;
		}
		
		float aa = getAngle(orgn.x, orgn.z, dest.x, dest.z);
		aa = MAKEANGLE(glm::degrees(aa));
		if(glm::abs(AngularDifference(aa, ab)) < 110.f) {
			if(dist_io < square(200)) {
				if(closestNpcDistance2 > dist_io) {
					closestNpc = &npc;
					closestNpcDistance2 = dist_io;
				}
			} else if(CURRENT_PLAYER_COLOR > GetPlayerStealth())  {
				Vec3f ppos;
				if(IO_Visible(orgn, dest, &ppos)) {
					if(closestNpcDistance2 > dist_io) {
						closestNpc = &npc;
						closestNpcDistance2 = dist_io;
					}
				} else if(closerThan(ppos, dest, 25.f)) {
					if(closestNpcDistance2 > dist_io) {
						closestNpc = &npc;
						closestNpcDistance2 = dist_io;
					}
				}
			}
			
		}
		
	}
	
	return closestNpc;
}

//! Checks if a NPC is dead to prevent further Layers Animation
void CheckNPC(Entity & io) {
	
	if(io.show == SHOW_FLAG_IN_SCENE && IsDeadNPC(io)) {
		io.animlayer[1].cur_anim = nullptr;
		io.animlayer[2].cur_anim = nullptr;
		io.animlayer[3].cur_anim = nullptr;
	}
	
}

/*!
 * \brief Checks an NPC Visibility Field (Player Detect)
 * Sends appropriate Detectplayer/Undetectplayer events to the IO
 *
 * \remarks Uses Invisibility/Confuse/Torch infos.
 */
void CheckNPCEx(Entity & io) {
	
	ARX_PROFILE_FUNC();
	
	// Distance Between Player and IO
	float ds = arx::distance2(io.pos, player.basePosition());
	
	// Start as not visible
	long Visible = 0;
	
	// Check visibility only if player is visible, not too far and not dead
	if(entities.player()->invisibility <= 0.f && ds < square(2000.f) && player.lifePool.current > 0.f) {
		
		// checks for near contact +/- 15 cm --> force visibility
		if(io.requestRoomUpdate) {
			UpdateIORoom(&io);
		}
		
		RoomHandle playerRoom = ARX_PORTALS_GetRoomNumForPosition(player.pos, RoomPositionForCamera);
		
		float fdist = SP_GetRoomDist(io.pos, player.pos, io.room, playerRoom);
		
		// Use Portal Room Distance for Extra Visibility Clipping.
		if(playerRoom && io.room && fdist > 2000.f) {
			// nothing to do
		} else if(ds < square(getEntityRadius(io) + getEntityRadius(*entities.player()) + 15.f)
		          && glm::abs(player.pos.y - io.pos.y) < 200.f) {
			Visible = 1;
		} else { // Make full visibility test
			
			// Retreives Head group position for "eye" pos.
			Vec3f orgn = io.pos - Vec3f(0.f, io.obj->fastaccess.head_group_origin ? 120.f : 90.f, 0.f);
			Vec3f dest = player.pos + Vec3f(0.f, 90.f, 0.f);
			
			// Check for Field of vision angle
			float aa = getAngle(orgn.x, orgn.z, dest.x, dest.z);
			aa = MAKEANGLE(glm::degrees(aa));
			float ab = MAKEANGLE(io.angle.getYaw());
			if(glm::abs(AngularDifference(aa, ab)) < 110.f) {
				
				// Check for Darkness/Stealth
				if(CURRENT_PLAYER_COLOR > GetPlayerStealth() || player.torch
				   || ds < square(200.f)) {
					Vec3f ppos;
					// Check for Geometrical Visibility
					if(IO_Visible(orgn, dest, &ppos) || closerThan(ppos, dest, 25.f)) {
						Visible = 1;
					}
				}
			}
		}
		
		if(Visible && !io._npcdata->detect) {
			// if visible but was NOT visible, sends an Detectplayer Event
			SendIOScriptEvent(nullptr, &io, SM_DETECTPLAYER);
			io._npcdata->detect = 1;
		}
		
	}
	
	// if not visible but was visible, sends an Undetectplayer Event
	if(!Visible && io._npcdata->detect) {
		SendIOScriptEvent(nullptr, &io, SM_UNDETECTPLAYER);
		io._npcdata->detect = 0;
	}
	
}

void ARX_NPC_NeedStepSound(Entity * io, const Vec3f & pos, const float volume, const float power) {
	
	std::string_view step_material = "foot_bare";
	std::string_view floor_material = "earth";
	
	if(EEIsUnderWater(pos)) {
		floor_material = "water";
	} else {
		EERIEPOLY * ep = CheckInPoly(pos + Vec3f(0.f, -100.f, 0.f));
		if(ep && ep->tex && !ep->tex->m_texName.empty()) {
			floor_material = GetMaterialString(ep->tex->m_texName);
		}
	}
	
	if(io && !io->stepmaterial.empty()) {
		step_material = io->stepmaterial;
	}
	
	if(io == entities.player()) {
		if(Entity * leggings = entities.get(player.equiped[EQUIP_SLOT_LEGGINGS])) {
			if(!leggings->stepmaterial.empty()) {
				step_material = leggings->stepmaterial;
			}
		}
	}
	
	ARX_SOUND_PlayCollision(step_material, floor_material, volume, power, pos, io);
}

/*!
 * \brief Sends ON HEAR events to NPCs for audible sounds
 * \note factor > 1.0F harder to hear, < 0.0F easier to hear
 */
void spawnAudibleSound(const Vec3f & pos, Entity & source, const float factor, const float presence) {
	
	float max_distance;
	if(source == *entities.player()) {
		max_distance = ARX_NPC_ON_HEAR_MAX_DISTANCE_STEP;
	} else if(source.ioflags & IO_ITEM) {
		max_distance = ARX_NPC_ON_HEAR_MAX_DISTANCE_ITEM;
	} else {
		return;
	}
	max_distance *= presence;
	max_distance /= factor;
	
	RoomHandle sourceRoom = ARX_PORTALS_GetRoomNumForPosition(pos, RoomPositionForCamera);
	
	for(Entity & npc : entities(IO_NPC)) {
		
		if((npc.gameFlags & GFLAG_ISINTREATZONE)
		   && (npc != source)
		   && (npc.show == SHOW_FLAG_IN_SCENE || npc.show == SHOW_FLAG_HIDDEN)
		   && (npc._npcdata->lifePool.current > 0.f) ) {
			
			float distance = fdist(pos, npc.pos);
			if(distance < max_distance) {
				
				if(npc.requestRoomUpdate) {
					UpdateIORoom(&npc);
				}
				
				if(sourceRoom && npc.room) {
					float fdist = SP_GetRoomDist(pos, npc.pos, sourceRoom, npc.room);
					if(fdist < max_distance * 1.5f) {
						distance = fdist;
					}
				}
				
				SendIOScriptEvent(&source, &npc, SM_HEAR, long(distance));
				
			}
			
		}
		
	}
	
}

void ManageIgnition(Entity & io) {
	
	if(player.torch == &io) {
		lightHandleDestroy(io.ignit_light);
		
		ARX_SOUND_Stop(io.ignit_sound);
		io.ignit_sound = audio::SourcedSample();
		
		return;
	}
	
	bool addParticles = (&io != g_draggedEntity || g_dragStatus == EntityDragStatus_OnGround);
	
	// Torch Management
	Entity * plw = entities.get(player.equiped[EQUIP_SLOT_WEAPON]);
	
	if((io.ioflags & IO_FIERY) && (!(io.type_flags & OBJECT_TYPE_BOW))
	   && (io.show == SHOW_FLAG_IN_SCENE || &io == plw)) {
		
		io.ignition = 25.f;
		
		if(addParticles && io.obj && !io.obj->facelist.empty()) {
			createObjFireParticles(io.obj, 4, 1, 1ms);
		}
		
	} else if(io.obj && io.obj->fastaccess.fire && io.ignition > 0.f) {
		
		io.ignition = 25.f;
		io.durability -= g_framedelay * 0.0001f;
		
		if(io.durability <= 0.f) {
			ARX_SOUND_PlaySFX(g_snd.TORCH_END, &io.pos);
			ARX_INTERACTIVE_DestroyIOdelayed(&io);
			return;
		}
		
		if(addParticles) {
			createFireParticles(io.obj->vertexWorldPositions[io.obj->fastaccess.fire].v, 2, 2ms);
		}
		
	} else {
		
		io.ignition -= g_framedelay * 0.01f;
		
		if(addParticles && io.obj && !io.obj->facelist.empty()) {
			float p = io.ignition * g_framedelay * 0.001f * float(io.obj->facelist.size()) * 0.001f * 2.f;
			int positions = std::min(int(std::ceil(p)), 10);
			createObjFireParticles(io.obj, positions, 6, 180ms);
		}
		
	}
	
	ManageIgnition_2(io);
}


void ManageIgnition_2(Entity & io) {
	
	if(io.ignition > 0.f) {
		
		if(io.ignition > 100.f) {
			io.ignition = 100.f;
		}
		
		Vec3f position = io.pos;
		if(io.obj) {
			if(&io == g_draggedEntity && io.show == SHOW_FLAG_ON_PLAYER) {
				position = player.pos;
			} else if(io.obj->fastaccess.fire) {
				position = io.obj->vertexWorldPositions[io.obj->fastaccess.fire].v;
			}
		}
		
		EERIE_LIGHT * light = dynLightCreate(io.ignit_light);
		if(light) {
			light->intensity = std::max(io.ignition * 0.1f, 1.f);
			light->fallstart = std::max(io.ignition * 10.f, 100.f);
			light->fallend   = std::max(io.ignition * 25.f, 240.f);
			float v = glm::clamp(io.ignition * 0.1f, 0.5f, 1.f);
			light->rgb = (Color3f(1.f, 0.8f, 0.6f) - randomColor3f() * Color3f(0.2f, 0.2f, 0.2f)) * v;
			light->pos = position + Vec3f(0.f, -30.f, 0.f);
			light->ex_flaresize = 40.f;
			if(io.show == SHOW_FLAG_IN_SCENE || io.show == SHOW_FLAG_TELEPORTING) {
				light->extras |= EXTRAS_FLARE;
			} else {
				light->extras &= ~EXTRAS_FLARE;
			}
		}
		
		if(io.ignit_sound == audio::SourcedSample()) {
			io.ignit_sound = ARX_SOUND_PlaySFX_loop(g_snd.FIREPLACE_LOOP, &position, Random::getf(0.95f, 1.05f));
		} else {
			ARX_SOUND_RefreshPosition(io.ignit_sound, position);
		}
		
		if(Random::getf() > 0.9f) {
			Sphere sphere(position, io.ignition);
			igniteLights(sphere);
			igniteEntities(sphere);
		}
		
	} else {
		lightHandleDestroy(io.ignit_light);
		
		ARX_SOUND_Stop(io.ignit_sound);
		io.ignit_sound = audio::SourcedSample();
	}
}

void GetTargetPos(Entity * io, unsigned long smoothing) {
	
	if(!io) {
		return;
	}
	
	if(io->ioflags & IO_NPC) {
		IO_NPCDATA * npcData = io->_npcdata;
		arx_assert(npcData);
		
		if(npcData->behavior & BEHAVIOUR_NONE) {
			io->target = io->pos;
			return;
		}
		
		if(npcData->behavior & BEHAVIOUR_GO_HOME) {
			if(npcData->pathfind.listpos < npcData->pathfind.listnb) {
				long pos = npcData->pathfind.list[npcData->pathfind.listpos];
				io->target = g_anchors[pos].pos;
			} else {
				io->target = io->initpos;
			}
			return;
		}
		
		if(npcData->pathfind.listnb != -1 && npcData->pathfind.list
		   && !(npcData->behavior & BEHAVIOUR_FRIENDLY)) { // Targeting Anchors !
			if(npcData->pathfind.listpos < npcData->pathfind.listnb) {
				long pos = npcData->pathfind.list[npcData->pathfind.listpos];
				io->target = g_anchors[pos].pos;
			} else if(Entity * target = entities.get(npcData->pathfind.truetarget)) {
				io->target = target->pos;
			}
			return;
		}
	}
	
	if(io->targetinfo == EntityHandle(TARGET_PATH)) {
		
		if(!io->usepath) {
			io->target = io->pos;
			return;
		}
		
		ARX_USE_PATH * aup = io->usepath;
		aup->_curtime += std::chrono::milliseconds(s64(smoothing) + 100);
		
		Vec3f tp;
		long wp = ARX_PATHS_Interpolate(aup, &tp);
		if(wp >= 0) {
			io->target = tp;
		} else if(io->ioflags & IO_CAMERA) {
			io->_camdata->lastinfovalid = false;
		}
		
		return;
	}
	
	if(io->targetinfo == EntityHandle_Player || io->targetinfo == EntityHandle()) {
		io->target = player.pos + Vec3f(0.f, player.size.y, 0.f);
	} else if(Entity * target = entities.get(io->targetinfo)) {
		io->target = GetItemWorldPosition(target);
	} else {
		io->target = io->pos;
	}
	
}

bool isEnemy(const Entity * entity) {
	return (entity->ioflags & IO_NPC)
	       && !(entity->_npcdata->behavior & BEHAVIOUR_FRIENDLY)
	       && (entity->_npcdata->behavior & BEHAVIOUR_FIGHT);
}
