/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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

#include "scene/Interactive.h"

#include <cstdlib>
#include <iomanip>
#include <algorithm>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "ai/Anchors.h"
#include "ai/Paths.h"

#include "animation/Animation.h"
#include "animation/AnimationRender.h"

#include "core/Application.h"
#include "core/GameTime.h"
#include "core/Config.h"
#include "core/Core.h"

#include "game/Camera.h"
#include "game/Damage.h"
#include "game/EntityManager.h"
#include "game/Equipment.h"
#include "game/Inventory.h"
#include "game/Item.h"
#include "game/Levels.h"
#include "game/NPC.h"
#include "game/Player.h"

#include "gui/Cursor.h"
#include "gui/Speech.h"
#include "gui/Interface.h"
#include "gui/book/Book.h"

#include "graphics/Draw.h"
#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/data/MeshManipulation.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/MagicFlare.h"
#include "graphics/texture/TextureStage.h"

#include "io/fs/FilePath.h"
#include "io/fs/Filesystem.h"
#include "io/fs/SystemPaths.h"
#include "io/resource/ResourcePath.h"
#include "io/resource/PakReader.h"
#include "io/log/Logger.h"

#include "math/Random.h"

#include "physics/Collisions.h"
#include "physics/CollisionShapes.h"
#include "physics/Physics.h"

#include "platform/Thread.h"
#include "platform/profiler/Profiler.h"

#include "scene/ChangeLevel.h"
#include "scene/GameSound.h"
#include "scene/Scene.h"
#include "scene/LinkedObject.h"
#include "scene/LoadLevel.h"
#include "scene/Light.h"
#include "scene/Object.h"

#include "script/ScriptEvent.h"


long HERO_SHOW_1ST = 1;

static bool IsCollidingInter(Entity * io, const Vec3f & pos);
static Entity * AddCamera(const res::path & classPath, EntityInstance instance = -1);
static Entity * AddMarker(const res::path & classPath, EntityInstance instance = -1);

float STARTED_ANGLE = 0;
void Set_DragInter(Entity * io)
{
	if(io != DRAGINTER)
		STARTED_ANGLE = player.angle.getYaw();

	DRAGINTER = io;

	if(io && io->obj && io->obj->pbox) {
		io->obj->pbox->active = 0;
	}
}

// Checks if an IO index number is valid
bool ValidIONum(EntityHandle num) {
	
	return !(num.handleData() < 0 || num.handleData() >= long(entities.size()) || !entities[num]);
}

bool ValidIOAddress(const Entity * io) {
	
	if(!io)
		return false;
	
	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * e = entities[handle];
		
		if(e == io) {
			return true;
		}
	}
	
	return false;
}

long ARX_INTERACTIVE_GetPrice(Entity * io, Entity * shop) {
	
	if(!io || !(io->ioflags & IO_ITEM))
		return 0;
	
	float shop_multiply = shop ? shop->shop_multiply : 1.f;
	float durability_ratio = io->durability / io->max_durability;
	
	return long(io->_itemdata->price * shop_multiply * durability_ratio);
}

long ARX_INTERACTIVE_GetSellValue(Entity * item, Entity * shop, long count) {
	
	float price = float(ARX_INTERACTIVE_GetPrice(item, shop) / 3 * count);
	
	return long(price + price * player.m_skillFull.intuition * 0.005f);
}

static void ARX_INTERACTIVE_ForceIOLeaveZone(Entity * io) {
	
	ARX_PATH * op = io->inzone;
	if(!op || op->controled.empty()) {
		return;
	}
	
	EntityHandle t = entities.getById(op->controled);
	if(t != EntityHandle()) {
		ScriptParameters parameters;
		parameters.push_back(io->idString());
		parameters.push_back(op->name);
		SendIOScriptEvent(NULL, entities[t], SM_CONTROLLEDZONE_LEAVE, parameters);
	}
	
}

void ARX_INTERACTIVE_DestroyDynamicInfo(Entity * io)
{
	if(!io)
		return;

	EntityHandle n = io->index();

	ARX_INTERACTIVE_ForceIOLeaveZone(io);

	for(size_t i = 0; i < MAX_EQUIPED; i++) {
		if(player.equiped[i] == n && ValidIONum(player.equiped[i])) {
			ARX_EQUIPMENT_UnEquip(entities.player(), entities[player.equiped[i]], 1);
			player.equiped[i] = EntityHandle();
		}
	}
	
	ARX_SPEECH_ReleaseIOSpeech(io);
	
	ARX_SCRIPT_EventStackClearForIo(io);
	
	if(ValidIONum(n)) {
		spells.endByCaster(n);
	}

	if(io->flarecount) {
		MagicFlareReleaseEntity(io);
	}
	
	if(io->ioflags & IO_NPC) {
		// to check again later...
		long count = 50;
		while((io->_npcdata->pathfind.pathwait == 1) && count--) {
			Thread::sleep(PlatformDurationMs(1));
		}
		delete[] io->_npcdata->pathfind.list;
		io->_npcdata->pathfind = IO_PATHFIND();
	}
	
	lightHandleDestroy(io->dynlight);

	IO_UnlinkAllLinkedObjects(io);
}


void ARX_INTERACTIVE_Attach(EntityHandle n_source, EntityHandle n_target,
                            const std::string & ap_source, const std::string & ap_target) {
	
	Entity * source = entities.get(n_source);
	Entity * target = entities.get(n_target);
	
	if(!source || !target) {
		return;
	}
	
	source->show = SHOW_FLAG_LINKED;
	EERIE_LINKEDOBJ_UnLinkObjectFromObject(target->obj, source->obj);
	EERIE_LINKEDOBJ_LinkObjectToObject(target->obj,
	        source->obj, ap_target, ap_source, source);
}

void ARX_INTERACTIVE_Detach(EntityHandle n_source, EntityHandle n_target)
{
	Entity * source = entities.get(n_source);
	Entity * target = entities.get(n_target);
	
	if(!source || !target)
		return;

	source->show = SHOW_FLAG_IN_SCENE;
	EERIE_LINKEDOBJ_UnLinkObjectFromObject(target->obj, source->obj);
}

void ARX_INTERACTIVE_Show_Hide_1st(Entity * io, long state)
{
	ARX_PROFILE_FUNC();
	
	if(!io || HERO_SHOW_1ST == state)
		return;

	HERO_SHOW_1ST = state;
	ObjSelection grp = EERIE_OBJECT_GetSelection(io->obj, "1st");

	if(grp != ObjSelection()) {
		for(size_t nn = 0; nn < io->obj->facelist.size(); nn++) {
			EERIE_FACE * ef = &io->obj->facelist[nn];

			for(long jj = 0; jj < 3; jj++) {
				if(IsInSelection(io->obj, ef->vid[jj], grp)) {
					if(state)
						ef->facetype |= POLY_HIDE;
					else
						ef->facetype &= ~POLY_HIDE;

					break;
				}
			}
		}
	}

	ARX_INTERACTIVE_HideGore(entities.player(), 1);
}

void ARX_INTERACTIVE_RemoveGoreOnIO(Entity * io)
{
	if (!io || !io->obj || io->obj->texturecontainer.empty())
		return;

	long gorenum = -1;

	for(size_t nn = 0; nn < io->obj->texturecontainer.size(); nn++) {
		if(io->obj->texturecontainer[nn] && io->obj->texturecontainer[nn]->m_texName.string().find("gore") != std::string::npos) {
			gorenum = nn;
			break;
		}
	}

	if(gorenum > -1) {
		for(size_t nn = 0; nn < io->obj->facelist.size(); nn++) {
			if(io->obj->facelist[nn].texid == gorenum) {
				io->obj->facelist[nn].facetype |= POLY_HIDE;
				io->obj->facelist[nn].texid = -1;
			}
		}
	}
}

// flag & 1 == no unhide non-gore
// TODO very simmilar to ARX_INTERACTIVE_RemoveGoreOnIO
void ARX_INTERACTIVE_HideGore(Entity * io, long flag)
{
	if (!io || !io->obj || io->obj->texturecontainer.empty())
		return;

	if(io == entities.player() && !flag)
		return;

	long gorenum = -1;

	for(size_t nn = 0; nn < io->obj->texturecontainer.size(); nn++) {
		if(io->obj->texturecontainer[nn] && io->obj->texturecontainer[nn]->m_texName.string().find("gore") != std::string::npos) {
			gorenum = nn;
			break;
		}
	}

	if(gorenum > -1) {
		for(size_t nn = 0; nn < io->obj->facelist.size(); nn++) {
			EERIE_FACE & face = io->obj->facelist[nn];
			// Hide gore faces...
			if(face.texid == gorenum)
				face.facetype |= POLY_HIDE;
			else if(!flag)
				face.facetype &= ~POLY_HIDE;
		}
	}
}

bool ForceNPC_Above_Ground(Entity * io) {
	
	if(io && (io->ioflags & IO_NPC) && !(io->ioflags & IO_PHYSICAL_OFF)) {
		io->physics.cyl.origin = io->pos;
		AttemptValidCylinderPos(io->physics.cyl, io, CFLAG_NO_INTERCOL);
		if(glm::abs(io->pos.y - io->physics.cyl.origin.y) < 45.f) {
			io->pos.y = io->physics.cyl.origin.y;
			return true;
		}
	}
	return false;
}

// Unlinks all linked objects from all IOs
void UnlinkAllLinkedObjects() {
	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * e = entities[handle];
		
		if(e) {
			EERIE_LINKEDOBJ_ReleaseData(e->obj);
		}
	}
}

void IO_UnlinkAllLinkedObjects(Entity * io) {
	
	if(!io || !io->obj) {
		return;
	}
	
	for(size_t k = 0; k < io->obj->linked.size(); k++) {
		
		Entity * linked = io->obj->linked[k].io;
		if(!ValidIOAddress(linked)) {
			continue;
		}
		
		linked->angle = Anglef(Random::getf(340.f, 380.f), Random::getf(0.f, 360.f), 0.f);
		linked->soundtime = 0;
		linked->soundcount = 0;
		linked->gameFlags |= GFLAG_NO_PHYS_IO_COL;
		linked->show = SHOW_FLAG_IN_SCENE;
		linked->no_collide = io->index();
		
		Vec3f pos = actionPointPosition(io->obj, io->obj->linked[k].lidx);
		
		Vec3f vector = angleToVectorXZ(linked->angle.getYaw()) * 0.5f;
		
		vector.y = std::sin(glm::radians(linked->angle.getPitch()));
		
		EERIE_PHYSICS_BOX_Launch(linked->obj, pos, linked->angle, vector);
		
	}
	
	EERIE_LINKEDOBJ_ReleaseData(io->obj);
}

// First is always the player
std::vector<TREATZONE_IO> treatio;

void TREATZONE_Clear() {
	treatio.clear();
}

void TREATZONE_Release() {
	treatio.clear();
}

void TREATZONE_RemoveIO(Entity * io) {
	for(size_t i = 0; i < treatio.size(); i++) {
		if(treatio[i].io == io) {
			treatio[i].io = NULL;
			treatio[i].ioflags = 0;
			treatio[i].show = SHOW_FLAG_NOT_DRAWN;
		}
	}
}

void TREATZONE_AddIO(Entity * io, bool justCollide) {
	
	for(size_t i = 0; i < treatio.size(); i++) {
		if(treatio[i].io == io) {
			return;
		}
	}
	
	TREATZONE_IO entry;
	entry.io = io;
	entry.ioflags = io->ioflags;
	if(justCollide) {
		entry.ioflags |= IO_JUST_COLLIDE;
	}
	entry.show = io->show;
	
	treatio.push_back(entry);
	
}

void CheckSetAnimOutOfTreatZone(Entity * io, AnimLayer & layer) {
	
	arx_assert(io);
	
	if(layer.cur_anim && !(io->gameFlags & GFLAG_ISINTREATZONE)
	   && fartherThan(io->pos, g_camera->m_pos, 2500.f)) {
		layer.ctime = layer.currentAltAnim()->anim_time - AnimationDurationMs(1);
	}
	
}

void PrepareIOTreatZone(long flag) {
	
	ARX_PROFILE_FUNC();
	
	static long status = -1;
	static Vec3f lastpos(0.f);
	
	const Vec3f cameraPos = g_camera->m_pos;
	
	if(flag || status == -1) {
		status = 0;
		lastpos = cameraPos;
	} else if(status == 3) {
		status = 0;
	}

	if(fartherThan(cameraPos, lastpos, 100.f)) {
		status = 0;
		lastpos = cameraPos;
	}

	if(status++)
		return;

	TREATZONE_Clear();
	long Cam_Room = ARX_PORTALS_GetRoomNumForPosition(cameraPos, 1);
	long PlayerRoom = ARX_PORTALS_GetRoomNumForPosition(player.pos, 1);
	TREATZONE_AddIO(entities.player());

	short sGlobalPlayerRoom = checked_range_cast<short>(PlayerRoom);

	for(size_t i = 0; i < MAX_EQUIPED; i++) {
		Entity * toequip = entities.get(player.equiped[i]);
		if(toequip) {
			toequip->room = sGlobalPlayerRoom;
			toequip->requestRoomUpdate = false;
		}
	}

	if(DRAGINTER)
		TREATZONE_AddIO(DRAGINTER);
	
	float TREATZONE_LIMIT = 3200;
	if(!g_roomDistance.empty()) {
		TREATZONE_LIMIT += 600;
		if(CURRENTLEVEL == 4) {
			TREATZONE_LIMIT += 1200;
		}
		if(g_camera->cdepth > 3000) {
			TREATZONE_LIMIT += 500;
		}
		if(g_camera->cdepth > 4000) {
			TREATZONE_LIMIT += 500;
		}
		if(g_camera->cdepth > 6000) {
			TREATZONE_LIMIT += 500;
		}
	}
	
	for(size_t i = 1; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * io = entities[handle];
		
		if(io && (io->show == SHOW_FLAG_IN_SCENE || io->show == SHOW_FLAG_TELEPORTING
		          || io->show == SHOW_FLAG_ON_PLAYER || io->show == SHOW_FLAG_HIDDEN)) {
			
			bool treat;
			if (io->ioflags & IO_CAMERA) {
				treat = false;
			} else if(io->ioflags & IO_MARKER) {
				treat = false;
			} else if((io->ioflags & IO_NPC) && (io->_npcdata->pathfind.flags & PATHFIND_ALWAYS)) {
				treat = true;
			} else {
				
				float dists;
				if(Cam_Room >= 0) {
					if(io->show == SHOW_FLAG_TELEPORTING) {
						Vec3f pos = GetItemWorldPosition(io);
						dists = arx::distance2(cameraPos, pos);
					} else {
						if(io->requestRoomUpdate) {
							UpdateIORoom(io);
						}
						dists = square(SP_GetRoomDist(io->pos, cameraPos, io->room, Cam_Room));
					}
				} else {
					if(io->show == SHOW_FLAG_TELEPORTING) {
						Vec3f pos = GetItemWorldPosition(io);
						dists = arx::distance2(cameraPos, pos);
					} else {
						dists = arx::distance2(io->pos, cameraPos);
					}
				}
				
				treat = (dists < square(TREATZONE_LIMIT));
			}

			if(!treat) {
				if(io == DRAGINTER)
					treat = true;
			}
			
			if(io->gameFlags & GFLAG_ISINTREATZONE) {
				io->gameFlags |= GFLAG_WASINTREATZONE;
			} else {
				io->gameFlags &= ~GFLAG_WASINTREATZONE;
			}
			
			if(treat) {
				io->gameFlags |= GFLAG_ISINTREATZONE;
				TREATZONE_AddIO(io);
				if((io->ioflags & IO_NPC) && io->_npcdata->weapon) {
					Entity * iooo = io->_npcdata->weapon;
					iooo->room = io->room;
					iooo->requestRoomUpdate = io->requestRoomUpdate;
				}
			} else {
				io->gameFlags &= ~GFLAG_ISINTREATZONE;
			}
			
			if((io->gameFlags & GFLAG_ISINTREATZONE) && (!(io->gameFlags & GFLAG_WASINTREATZONE))) {
				// Coming back - doesn't really matter right now
			} else if(!(io->gameFlags & GFLAG_ISINTREATZONE) && (io->gameFlags & GFLAG_WASINTREATZONE)) {
				// Going away
				io->gameFlags |= GFLAG_ISINTREATZONE;
				if(SendIOScriptEvent(NULL, io, SM_TREATOUT) != REFUSE) {
					if(io->ioflags & IO_NPC)
						io->_npcdata->pathfind.flags &= ~PATHFIND_ALWAYS;
					io->gameFlags &= ~GFLAG_ISINTREATZONE;
				}
			}
			
		}
	}
	
	size_t M_TREAT = treatio.size();
	
	for(size_t i = 1; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * io = entities[handle];
		
		if(io && !(io->gameFlags & GFLAG_ISINTREATZONE)
		   && (io->show == SHOW_FLAG_IN_SCENE || io->show == SHOW_FLAG_TELEPORTING
		       || io->show == SHOW_FLAG_ON_PLAYER || io->show == SHOW_FLAG_HIDDEN)) {
			
			if(io->ioflags & (IO_CAMERA | IO_ITEM | IO_MARKER))
				continue;

			bool toadd = false;

			for(size_t ii = 1; ii < M_TREAT; ii++) {
				Entity * ioo = treatio[ii].io;

				if(ioo) {
					if(closerThan(io->pos, ioo->pos, 300.f)) {
						toadd = true;
						break;
					}
				}
			}

			if(toadd) {
				TREATZONE_AddIO(io, true);
			}
		}
	}
	
}

/*!
 * \brief Removes an IO loaded by a script command
 */
void CleanScriptLoadedIO() {
	for(size_t i = 1; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * io = entities[handle];
		
		if(io) {
			if(io->scriptload) {
				delete io;
			} else {
				// TODO why not jus leave it as is?
				io->show = SHOW_FLAG_IN_SCENE;
			}
		}
	}
}

/*!
 * \brief Restores an IO to its initial status (Game start Status)
 */
void RestoreInitialIOStatus() {
	arx_assert(entities.player());
	
	ARX_NPC_Behaviour_ResetAll();
	
	entities.player()->spellcast_data.castingspell = SPELL_NONE;
	
	for(size_t i = 1; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * e = entities[handle];
		
		RestoreInitialIOStatusOfIO(e);
	}
}

bool ARX_INTERACTIVE_USEMESH(Entity * io, const res::path & temp) {
	
	if(!io || temp.empty()) {
		return false;
	}
	
	if(io->ioflags & IO_NPC) {
		io->usemesh = "graph/obj3d/interactive/npc" / temp;
	} else if(io->ioflags & IO_FIX) {
		io->usemesh = "graph/obj3d/interactive/fix_inter" / temp;
	} else if(io->ioflags & IO_ITEM) {
		io->usemesh = "graph/obj3d/interactive/items" / temp;
	} else {
		io->usemesh.clear();
	}
	
	if(io->usemesh.empty()) {
		return false;
	}
	
	delete io->obj, io->obj = NULL;
	
	bool pbox = (!(io->ioflags & IO_FIX) && !(io->ioflags & IO_NPC));
	io->obj = loadObject(io->usemesh, pbox);
	
	EERIE_COLLISION_Cylinder_Create(io);
	return true;
}

void ARX_INTERACTIVE_MEMO_TWEAK(Entity * io, TweakType type, const res::path & param1, const res::path & param2) {
	
	io->tweaks.resize(io->tweaks.size() + 1);
	
	io->tweaks.back().type = type;
	io->tweaks.back().param1 = param1;
	io->tweaks.back().param2 = param2;
}

void ARX_INTERACTIVE_APPLY_TWEAK_INFO(Entity * io) {
	
	for(std::vector<TWEAK_INFO>::const_iterator i = io->tweaks.begin(); i != io->tweaks.end(); ++i) {
		switch(i->type) {
			case TWEAK_REMOVE: EERIE_MESH_TWEAK_Do(io, TWEAK_REMOVE, res::path()); break;
			case TWEAK_TYPE_SKIN: EERIE_MESH_TWEAK_Skin(io->obj, i->param1, i->param2); break;
			case TWEAK_TYPE_ICON: ARX_INTERACTIVE_TWEAK_Icon(io, i->param1); break;
			case TWEAK_TYPE_MESH: ARX_INTERACTIVE_USEMESH(io, i->param1); break;
			default: EERIE_MESH_TWEAK_Do(io, i->type, i->param1);
		}
	}
}

static void ARX_INTERACTIVE_ClearIODynData(Entity * io) {
	
	if(!io)
		return;
	
	lightHandleDestroy(io->dynlight);
	
	delete io->symboldraw;
	io->symboldraw = NULL;
	
	io->spellcast_data.castingspell = SPELL_NONE;
}

static void ARX_INTERACTIVE_ClearIODynData_II(Entity * io) {
	
	if(!io)
		return;
	
	ARX_INTERACTIVE_ClearIODynData(io);
	
	io->shop_category.clear();
	io->inventory_skin.clear();
	
	io->tweaks.clear();
	io->groups.clear();
	ARX_INTERACTIVE_HideGore(io);
	ARX_SCRIPT_Timer_Clear_For_IO(io);
	
	io->stepmaterial.clear();
	io->armormaterial.clear();
	io->weaponmaterial.clear();
	io->strikespeech.clear();
	
	io->animBlend.m_active = false;
	
	for(size_t j = 0; j < MAX_ANIMS; j++) {
		EERIE_ANIMMANAGER_ReleaseHandle(io->anims[j]);
		io->anims[j] = NULL;
	}
	
	spells.removeTarget(io);
	ARX_EQUIPMENT_ReleaseAll(io);
	
	if(io->ioflags & IO_NPC) {
		delete[] io->_npcdata->pathfind.list;
		io->_npcdata->pathfind = IO_PATHFIND();
		io->_npcdata->pathfind.truetarget = EntityHandle();
		io->_npcdata->pathfind.listnb = -1;
		ARX_NPC_Behaviour_Reset(io);
	}
	
	delete io->tweakerinfo;
	io->tweakerinfo = NULL;
	
	if(io->inventory != NULL) {
		INVENTORY_DATA * id = io->inventory;
		
		for(long nj = 0; nj < id->m_size.y; nj++) {
			for(long ni = 0; ni < id->m_size.x; ni++) {
				if(id->slot[ni][nj].io != NULL) {
					id->slot[ni][nj].io->destroy();
					id->slot[ni][nj].io = NULL;
				}
			}
		}
		
		if(TSecondaryInventory && TSecondaryInventory->io == io) {
			TSecondaryInventory = NULL;
		}
		
		delete io->inventory;
		io->inventory = NULL;
	}
	
	io->inventory = NULL;
	io->gameFlags |= GFLAG_INTERACTIVITY;
	
	if(io->tweaky) {
		delete io->obj;
		io->obj = io->tweaky;
		io->tweaky = NULL;
	}
}

void ARX_INTERACTIVE_ClearAllDynData() {
	ARX_NPC_Behaviour_ResetAll();
	for(size_t i = 1; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * e = entities[handle];
		
		ARX_INTERACTIVE_ClearIODynData(e);
	}
}

static void RestoreIOInitPos(Entity * io) {
	if(!io)
		return;

	{
		ARX_INTERACTIVE_Teleport(io, io->initpos);
		io->pos = io->lastpos = io->initpos;
		io->move = Vec3f(0.f);
		io->lastmove = Vec3f(0.f);
		io->angle = io->initangle;
	}
}

void ARX_HALO_SetToNative(Entity * io) {
	io->halo.color = io->halo_native.color;
	io->halo.radius = io->halo_native.radius;
	io->halo.flags = io->halo_native.flags;
}

void RestoreInitialIOStatusOfIO(Entity * io)
{
	if(!io)
		return;

	{
		ARX_INTERACTIVE_ClearIODynData_II(io);

		io->shop_multiply = 1.f;

		ARX_INTERACTIVE_HideGore(io, 1);

		io->halo_native.color = Color3f(0.2f, 0.5f, 1.f);
		io->halo_native.radius = 45.f;
		io->halo_native.flags = 0;

		ARX_HALO_SetToNative(io);

		io->forcedmove = Vec3f(0.f);
		io->ioflags &= ~IO_NO_COLLISIONS;
		io->ioflags &= ~IO_INVERTED;
		io->lastspeechflag = 2;
	
		io->no_collide = EntityHandle();

		MagicFlareReleaseEntity(io);

		io->flarecount = 0;
		io->inzone = NULL;
		io->speed_modif = 0.f;
		io->basespeed = 1.f;
		io->sfx_flag = 0;
		io->max_durability = io->durability = 100;
		io->gameFlags &= ~GFLAG_INVISIBILITY;
		io->gameFlags &= ~GFLAG_MEGAHIDE;
		io->gameFlags &= ~GFLAG_NOGORE;
		io->gameFlags &= ~GFLAG_ISINTREATZONE;
		io->gameFlags &= ~GFLAG_PLATFORM;
		io->gameFlags &= ~GFLAG_ELEVATOR;
		io->gameFlags &= ~GFLAG_HIDEWEAPON;
		io->gameFlags &= ~GFLAG_NOCOMPUTATION;
		io->gameFlags &= ~GFLAG_INTERACTIVITYHIDE;
		io->gameFlags &= ~GFLAG_DOOR;
		io->gameFlags &= ~GFLAG_GOREEXPLODE;
		io->invisibility = 0.f;
		io->rubber = BASE_RUBBER;
		io->scale = 1.f;
		io->move = Vec3f(0.f);
		io->type_flags = 0;
		io->m_sound = audio::SourcedSample();
		io->soundtime = 0;
		io->soundcount = 0;
		io->material = MATERIAL_STONE;
		io->collide_door_time = 0;
		io->ouch_time = 0;
		io->dmg_sum = 0;
		io->ignition = 0.f;
		io->ignit_light = LightHandle();
		io->ignit_sound = audio::SourcedSample();

		if(io->obj && io->obj->pbox)
			io->obj->pbox->active = 0;

		io->room = -1;
		io->requestRoomUpdate = true;
		RestoreIOInitPos(io);
		ARX_INTERACTIVE_Teleport(io, io->initpos);
		io->animBlend.lastanimtime = GameInstantMs(1);
		io->secretvalue = -1;
		
		io->poisonous = 0;
		io->poisonous_count = 0;

		for(size_t count = 0; count < MAX_ANIM_LAYERS; count++) {
			io->animlayer[count] = AnimLayer();
		}

		if(io->obj && io->obj->pbox) {
			io->obj->pbox->storedtiming = 0;
		}
		
		io->physics.cyl.origin = io->pos;
		io->physics.cyl.radius = io->original_radius;
		io->physics.cyl.height = io->original_height;
		io->fall = 0;
		io->show = SHOW_FLAG_IN_SCENE;
		io->targetinfo = EntityHandle(TARGET_NONE);
		io->spellcast_data.castingspell = SPELL_NONE;
		io->summoner = EntityHandle();
		io->spark_n_blood = 0;

		if(io->ioflags & IO_NPC) {
			io->_npcdata->climb_count = 0;
			io->_npcdata->vvpos = -99999.f;
			io->_npcdata->SPLAT_DAMAGES = 0;
			io->_npcdata->speakpitch = 1.f;
			io->_npcdata->behavior = BEHAVIOUR_NONE;
			io->_npcdata->cut = 0;
			io->_npcdata->cuts = 0;
			io->_npcdata->poisonned = 0.f;
			io->_npcdata->blood_color = Color::red;
			io->_npcdata->stare_factor = 1.f;

			io->_npcdata->weapon = NULL;
			io->_npcdata->weaponinhand = 0;
			io->_npcdata->weapontype = 0;
			io->_npcdata->weaponinhand = 0;
			io->_npcdata->fightdecision = 0;
			io->_npcdata->walk_start_time = 0;

			io->_npcdata->reachedtarget = 0;
			io->_npcdata->lifePool.max = 20.f;
			io->_npcdata->lifePool.current = io->_npcdata->lifePool.max;
			io->_npcdata->manaPool.max = 10.f;
			io->_npcdata->manaPool.current = io->_npcdata->manaPool.max;
			io->_npcdata->critical = 5.f;
			io->infracolor = Color3f(1.f, 0.f, 0.2f);
			io->_npcdata->detect = 0;
			io->_npcdata->movemode = WALKMODE;
			io->_npcdata->reach = 20.f;
			io->_npcdata->armor_class = 0;
			io->_npcdata->absorb = 0;
			io->_npcdata->damages = 20;
			io->_npcdata->tohit = 50;
			io->_npcdata->aimtime = 0;
			io->_npcdata->aiming_start = 0;
			io->_npcdata->npcflags = 0;
			io->_npcdata->backstab_skill = 0;
			io->_npcdata->fDetect = -1;
		}
		
		if(io->ioflags & IO_ITEM) {
			io->collision = COLLIDE_WITH_PLAYER;
			io->_itemdata->count = 1;
			io->_itemdata->maxcount = 1;
			io->_itemdata->food_value = 0;
			io->_itemdata->playerstacksize = 1;
			io->_itemdata->stealvalue = -1;
			io->_itemdata->LightValue = -1;
		} else {
			io->collision = 0;
		}
		
		if(io->ioflags & IO_FIX) {
			io->_fixdata->trapvalue = -1;
		}
		
	}
}

void ARX_INTERACTIVE_TWEAK_Icon(Entity * io, const res::path & s1) {
	
	if(!io || s1.empty())
		return;
	
	res::path icontochange = io->classPath().parent() / s1;
	
	TextureContainer * tc = TextureContainer::LoadUI(icontochange,
	                                                 TextureContainer::Level);
	if(!tc) {
		tc = TextureContainer::LoadUI("graph/interface/misc/default[icon]");
	}
	
	if(tc) {
		io->m_inventorySize = inventorySizeFromTextureSize(tc->size());
		io->m_icon = tc;
	}
}

// Be careful with this func...
Entity * CloneIOItem(Entity * src) {
	
	Entity * dest = AddItem(src->classPath());
	if(!dest) {
		return NULL;
	}
	
	SendInitScriptEvent(dest);
	dest->m_icon = src->m_icon;
	dest->m_inventorySize = src->m_inventorySize;
	delete dest->obj;
	dest->obj = Eerie_Copy(src->obj);
	CloneLocalVars(dest, src);
	dest->_itemdata->price = src->_itemdata->price;
	dest->_itemdata->maxcount = src->_itemdata->maxcount;
	dest->_itemdata->count = src->_itemdata->count;
	dest->_itemdata->food_value = src->_itemdata->food_value;
	dest->_itemdata->stealvalue = src->_itemdata->stealvalue;
	dest->_itemdata->playerstacksize = src->_itemdata->playerstacksize;
	dest->_itemdata->LightValue = src->_itemdata->LightValue;
	
	if(src->_itemdata->equipitem) {
		dest->_itemdata->equipitem = new IO_EQUIPITEM;
		*dest->_itemdata->equipitem = *src->_itemdata->equipitem;
	}
	
	dest->locname = src->locname;
	
	if(dest->obj->pbox == NULL && src->obj->pbox != NULL) {
		dest->obj->pbox = new PHYSICS_BOX_DATA();
		*dest->obj->pbox = *src->obj->pbox;
		
		dest->obj->pbox->vert = src->obj->pbox->vert;
	}
	
	return dest;
}

bool ARX_INTERACTIVE_ConvertToValidPosForIO(Entity * io, Vec3f * target) {
	
	Cylinder phys;
	if (io && io != entities.player()) {
		phys.height = io->original_height * io->scale;
		phys.radius = io->original_radius * io->scale;
	} else {
		phys.height = -200;
		phys.radius = 50;
	}
	
	phys.origin = *target;
	float count = 0;
	
	while(count < 600) {
		Vec3f mod = angleToVectorXZ(count) * count * (1.f / 3);
		
		phys.origin.x = target->x + mod.x;
		phys.origin.z = target->z + mod.z;
		float anything = CheckAnythingInCylinder(phys, io, CFLAG_JUST_TEST);

		if(glm::abs(anything) < 150.f) {
			EERIEPOLY * ep = CheckInPoly(phys.origin + Vec3f(0.f, -20.f, 0.f));
			EERIEPOLY * ep2 = CheckTopPoly(phys.origin + Vec3f(0.f, anything, 0.f));

			if(ep && ep2 && glm::abs((phys.origin.y + anything) - ep->center.y) < 20.f) {
				target->x = phys.origin.x;
				target->y = phys.origin.y + anything;
				target->z = phys.origin.z;
				return true;
			}
		}

		count += 5.f;
	}

	return false;
}

void ARX_INTERACTIVE_TeleportBehindTarget(Entity * io) {
	
	if(!io || scriptTimerExists(io, "_r_a_t_")) {
		return;
	}
	
	SCR_TIMER & timer = createScriptTimer(io, "_r_a_t_");
	timer.interval = GameDurationMs(Random::get(3000, 6000));
	timer.start = g_gameTime.now();
	timer.count = 1;
	
	io->show = SHOW_FLAG_TELEPORTING;
	AddRandomSmoke(*io, 10);
	ARX_PARTICLES_Add_Smoke(io->pos, 3, 20);
	io->requestRoomUpdate = true;
	io->room = -1;
	ARX_PARTICLES_Add_Smoke(io->pos + Vec3f(0.f, io->physics.cyl.height * 0.5f, 0.f), 3, 20);
	MakeCoolFx(io->pos);
	io->gameFlags |= GFLAG_INVISIBILITY;
	
}

void ResetVVPos(Entity * io)
{
	if(io && (io->ioflags & IO_NPC))
		io->_npcdata->vvpos = io->pos.y;
}

void ComputeVVPos(Entity * io) {
	if(io->ioflags & IO_NPC) {
		float vvp = io->_npcdata->vvpos;

		if(vvp == -99999.f || vvp == io->pos.y) {
			io->_npcdata->vvpos = io->pos.y;
			return;
		}

		float diff = io->pos.y - vvp;
		float fdiff = glm::abs(diff);
		float eediff = fdiff;

		if(fdiff > 120.f) {
			fdiff = 120.f;
		} else {
			float mul = ((fdiff * (1.0f / 120)) * 0.9f + 0.6f);
			
			float val;
			if(io == entities.player()) {
				val = toMs(g_platformTime.lastFrameDuration());
			} else {
				val = toMsf(g_gameTime.lastFrameDuration());
			}
			val *= (1.0f / 4) * mul;
			
			if(eediff < 15.f) {
				if(eediff < 10.f) {
					val *= (1.0f / 10);
				} else {
					float ratio = (eediff - 10.f) * (1.0f / 5);
					val = val * ratio + val * (1.f - ratio);
				}
			}
			fdiff -= val;
		}
		
		if(fdiff > eediff) {
			fdiff = eediff;
		}
		
		if(fdiff < 0.f) {
			fdiff = 0.f;
		}
		
		if(diff < 0.f) {
			io->_npcdata->vvpos = io->pos.y + fdiff;
		} else {
			io->_npcdata->vvpos = io->pos.y - fdiff;
		}
	}
}

void ARX_INTERACTIVE_Teleport(Entity * io, const Vec3f & target, bool flag) {
	
	if(!io)
		return;
	
	arx_assert(isallfinite(target));
	
	io->gameFlags &= ~GFLAG_NOCOMPUTATION;
	io->requestRoomUpdate = true;
	io->room = -1;
	
	if(io == entities.player()) {
		g_moveto = player.pos = target + player.baseOffset();
	}
	
	// In case it is being dragged... (except for drag teleport update)
	if(!flag && io == DRAGINTER) {
		Set_DragInter(NULL);
	}
	
	if(io->ioflags & IO_NPC) {
		io->_npcdata->vvpos = io->pos.y;
	}
	
	Vec3f translate = target - io->pos;
	io->lastpos = io->physics.cyl.origin = io->pos = target;
	
	if(io->obj) {
		if(io->obj->pbox) {
			if(io->obj->pbox->active) {
				for(size_t i = 0; i < io->obj->pbox->vert.size(); i++) {
					io->obj->pbox->vert[i].pos += translate;
				}
				io->obj->pbox->active = 0;
			}
		}
		for(size_t i = 0; i < io->obj->vertexlist.size(); i++) {
			io->obj->vertexWorldPositions[i].v += translate;
		}
	}
	
	ResetVVPos(io);
}

Entity * AddInteractive(const res::path & classPath, EntityInstance instance, AddInteractiveFlags flags) {
	
	const std::string & ficc = classPath.string();
	
	Entity * io = NULL;
	if(boost::contains(ficc, "items")) {
		io = AddItem(classPath, instance, flags);
	} else if(boost::contains(ficc, "npc")) {
		io = AddNPC(classPath, instance, flags);
	} else if(boost::contains(ficc, "fix")) {
		io = AddFix(classPath, instance, flags);
	} else if(boost::contains(ficc, "camera")) {
		io = AddCamera(classPath, instance);
	} else if(boost::contains(ficc, "marker")) {
		io = AddMarker(classPath, instance);
	}
	
	return io;
}

/*!
 * \brief Links an object designed by path "temp" to the primary attach of interactive object "io"
 */
void SetWeapon_On(Entity * io) {
	
	if(!io || !(io->ioflags & IO_NPC))
		return;
	
	Entity * ioo = io->_npcdata->weapon;
	
	if(ioo && ioo->obj) {
		EERIE_LINKEDOBJ_UnLinkObjectFromObject(io->obj, ioo->obj);
		EERIE_LINKEDOBJ_LinkObjectToObject(io->obj, ioo->obj, "primary_attach", "primary_attach", ioo);
	}
}

void SetWeapon_Back(Entity * io) {
	
	if(!io || !(io->ioflags & IO_NPC))
		return;
	
	Entity * weapon = io->_npcdata->weapon;
	
	if(weapon && weapon->obj) {
		EERIE_LINKEDOBJ_UnLinkObjectFromObject(io->obj, weapon->obj);

		if(io->gameFlags & GFLAG_HIDEWEAPON)
			return;

		ActionPoint ni = io->obj->fastaccess.weapon_attach;

		if(ni != ActionPoint()) {
			EERIE_LINKEDOBJ_LinkObjectToObject(io->obj, weapon->obj, "weapon_attach", "primary_attach", weapon);
		} else {
			ni = io->obj->fastaccess.secondary_attach;

			if(ni != ActionPoint())
				EERIE_LINKEDOBJ_LinkObjectToObject(io->obj, weapon->obj, "secondary_attach", "primary_attach", weapon);
		}
	}
}

void Prepare_SetWeapon(Entity * io, const res::path & temp) {
	
	arx_assert(io && (io->ioflags & IO_NPC));
	
	if(io->_npcdata->weapon) {
		Entity * ioo = io->_npcdata->weapon;
		EERIE_LINKEDOBJ_UnLinkObjectFromObject(io->obj, ioo->obj);
		io->_npcdata->weapon = NULL;
		delete ioo;
	}
	
	res::path file = "graph/obj3d/interactive/items/weapons" / temp / temp;
	Entity * ioo = io->_npcdata->weapon = AddItem(file);
	if(ioo) {
		
		SendInitScriptEvent(ioo);
		io->_npcdata->weapontype = ioo->type_flags;
		ioo->show = SHOW_FLAG_LINKED;
		ioo->scriptload = 2;
		
		SetWeapon_Back(io);
	}
}

/*!
 * \brief Links an Interactive Object to another interactive object using an attach point
 */
void LinkObjToMe(Entity * io, Entity * io2, const std::string & attach) {
	
	if(!io || !io2)
		return;
	
	RemoveFromAllInventories(io2);
	io2->show = SHOW_FLAG_LINKED;
	EERIE_LINKEDOBJ_LinkObjectToObject(io->obj, io2->obj, attach, attach, io2);
}

/*!
 * \brief Creates a Temporary IO Ident
 */
static EntityInstance getFreeEntityInstance(const res::path & classPath) {
	
	std::string className = classPath.filename();
	res::path classDir = classPath.parent();
	
	for(EntityInstance instance = 1; ; instance++) {
		
		std::string idString = EntityId(className, instance).string();
		
		// Check if the candidate instance number is used in the current scene
		if(entities.getById(idString) != EntityHandle()) {
			continue;
		}
		
		// Check if the candidate instance number is used in any visited area
		if(currentSavedGameHasEntity(idString)) {
			continue;
		}
		
		// Check if the candidate instance number is reserved for any scene
		if(g_resources->getDirectory(classDir / idString)) {
			continue;
		}
		
		return instance;
	}
}

Entity * AddFix(const res::path & classPath, EntityInstance instance, AddInteractiveFlags flags) {
	
	res::path object = classPath + ".teo";
	res::path script = classPath + ".asl";
	
	if(!g_resources->getFile(("game" / classPath) + ".ftl")
	   && !g_resources->getFile(object) && !g_resources->getFile(script)) {
		return NULL;
	}
	
	if(instance == -1) {
		instance = getFreeEntityInstance(classPath);
	}
	arx_assert(instance > 0);
	
	Entity * io = new Entity(classPath, instance);
	
	io->_fixdata = new IO_FIXDATA;
	io->ioflags = IO_FIX;
	io->_fixdata->trapvalue = -1;
	
	loadScript(io->script, g_resources->getFile(script));
	
	if(!(flags & NO_ON_LOAD)) {
		SendIOScriptEvent(NULL, io, SM_LOAD);
	}
	
	io->spellcast_data.castingspell = SPELL_NONE;
	
	io->pos = player.pos;
	io->pos += angleToVectorXZ(player.angle.getYaw()) * 140.f;
	
	io->lastpos = io->initpos = io->pos;
	io->lastpos.x = io->initpos.x = glm::abs(io->initpos.x / 20) * 20.f;
	io->lastpos.z = io->initpos.z = glm::abs(io->initpos.z / 20) * 20.f;
	
	float tempo;
	EERIEPOLY * ep = CheckInPoly(io->pos + Vec3f(0.f, player.baseHeight(), 0.f));
	if(ep && GetTruePolyY(ep, io->pos, &tempo)) {
		io->lastpos.y = io->initpos.y = io->pos.y = tempo;
	}
	
	ep = CheckInPoly(io->pos);
	if(ep) {
		io->pos.y = std::min(ep->v[0].p.y, ep->v[1].p.y);
		io->lastpos.y = io->initpos.y = io->pos.y = std::min(io->pos.y, ep->v[2].p.y);
	}
	
	if(!io->obj && !(flags & NO_MESH)) {
		io->obj = loadObject(object, false);
	}
	
	io->infracolor = Color3f(0.6f, 0.f, 1.f);
	
	TextureContainer * tc = TextureContainer::LoadUI("graph/interface/misc/default[icon]");
	
	if(tc) {
		io->m_inventorySize = inventorySizeFromTextureSize(tc->size());
		io->m_icon = tc;
	}
	
	io->collision = COLLIDE_WITH_PLAYER;
	
	return io;
}

static Entity * AddCamera(const res::path & classPath, EntityInstance instance) {
	
	res::path object = classPath + ".teo";
	res::path script = classPath + ".asl";
	
	if(!g_resources->getFile(("game" / classPath) + ".ftl")
	   && !g_resources->getFile(object) && !g_resources->getFile(script)) {
		return NULL;
	}
	
	if(instance == -1) {
		instance = getFreeEntityInstance(classPath);
	}
	arx_assert(instance > 0);
	
	Entity * io = new Entity(classPath, instance);
	
	loadScript(io->script, g_resources->getFile(script));
	
	io->pos = player.pos;
	io->pos += angleToVectorXZ(player.angle.getYaw()) * 140.f;
	
	io->lastpos = io->initpos = io->pos;
	io->lastpos.x = io->initpos.x = glm::abs(io->initpos.x / 20) * 20.f;
	io->lastpos.z = io->initpos.z = glm::abs(io->initpos.z / 20) * 20.f;
	
	float tempo;
	EERIEPOLY * ep;
	ep = CheckInPoly(io->pos + Vec3f(0.f, player.baseHeight(), 0.f), &tempo);
	if(ep) {
		io->lastpos.y = io->initpos.y = io->pos.y = tempo;
	}
	
	ep = CheckInPoly(io->pos);
	if(ep) {
		io->pos.y = std::min(ep->v[0].p.y, ep->v[1].p.y);
		io->lastpos.y = io->initpos.y = io->pos.y = std::min(io->pos.y, ep->v[2].p.y);
	}
	
	io->lastpos.y = io->initpos.y = io->pos.y += player.baseHeight();
	
	io->obj = cameraobj;
	
	io->_camdata = new IO_CAMDATA();
	io->_camdata->cam = g_playerCamera;
	io->_camdata->cam.focal = 350.f;
	io->ioflags = IO_CAMERA;
	io->collision = 0;
	
	return io;
}

static Entity * AddMarker(const res::path & classPath, EntityInstance instance) {
	
	res::path object = classPath + ".teo";
	res::path script = classPath + ".asl";
	
	if(!g_resources->getFile(("game" / classPath) + ".ftl")
	   && !g_resources->getFile(object) && !g_resources->getFile(script)) {
		return NULL;
	}
	
	if(instance == -1) {
		instance = getFreeEntityInstance(classPath);
	}
	arx_assert(instance > 0);
	
	Entity * io = new Entity(classPath, instance);
	
	loadScript(io->script, g_resources->getFile(script));
	
	io->pos = player.pos;
	io->pos += angleToVectorXZ(player.angle.getYaw()) * 140.f;
	
	io->lastpos = io->initpos = io->pos;
	io->lastpos.x = io->initpos.x = glm::abs(io->initpos.x / 20) * 20.f;
	io->lastpos.z = io->initpos.z = glm::abs(io->initpos.z / 20) * 20.f;
	
	float tempo;
	EERIEPOLY * ep;
	ep = CheckInPoly(io->pos + Vec3f(0.f, player.baseHeight(), 0.f));
	if(ep && GetTruePolyY(ep, io->pos, &tempo)) {
		io->lastpos.y = io->initpos.y = io->pos.y = tempo;
	}
	
	ep = CheckInPoly(io->pos);
	if(ep) {
		io->pos.y = std::min(ep->v[0].p.y, ep->v[1].p.y);
		io->lastpos.y = io->initpos.y = io->pos.y = std::min(io->pos.y, ep->v[2].p.y);
	}
	
	io->lastpos.y = io->initpos.y = io->pos.y += player.baseHeight();
	
	io->obj = markerobj;
	io->ioflags = IO_MARKER;
	io->collision = 0;
	
	return io;
}

IO_NPCDATA::IO_NPCDATA()
	: lifePool(20.f)
	, manaPool(0.f)
	, reachedtime(0)
	, reachedtarget(0)
	, weapon(NULL)
	, detect(0)
	, movemode(WALKMODE)
	, armor_class(0.f)
	, absorb(0.f)
	, damages(0.f)
	, tohit(0.f)
	, aimtime(0)
	, critical(0.f)
	, reach(0.f)
	, backstab_skill(0.f)
	, behavior(0)
	, behavior_param(0.f)
	, xpvalue(0)
	, cut(0)
	, moveproblem(0.f)
	, weapontype(0)
	, weaponinhand(0)
	, fightdecision(0)
	, look_around_inc(0.f)
	, speakpitch(1.f)
	, lastmouth(0.f)
	, ltemp(0)
	, poisonned(0.f)
	, resist_poison(0)
	, resist_magic(0)
	, resist_fire(0)
	, walk_start_time(0)
	, aiming_start(0)
	, npcflags(0)
	, ex_rotate(0)
	, blood_color(Color::red)
	, SPLAT_DAMAGES(0)
	, SPLAT_TOT_NB(0)
	, last_splat_pos(0.f)
	, vvpos(0.f)
	, climb_count(0.f)
	, stare_factor(0.f)
	, fDetect(0.f)
	, cuts(0)
{ }

IO_NPCDATA::~IO_NPCDATA() {
	delete ex_rotate;
	delete[] pathfind.list;
}

Entity * AddNPC(const res::path & classPath, EntityInstance instance, AddInteractiveFlags flags) {
	
	res::path object = classPath + ".teo";
	res::path script = classPath + ".asl";
	
	if(!g_resources->getFile(("game" / classPath) + ".ftl")
	   && !g_resources->getFile(object) && !g_resources->getFile(script)) {
		return NULL;
	}
	
	if(instance == -1) {
		instance = getFreeEntityInstance(classPath);
	}
	arx_assert(instance > 0);
	
	Entity * io = new Entity(classPath, instance);
	
	io->forcedmove = Vec3f(0.f);
	
	io->_npcdata = new IO_NPCDATA;
	io->ioflags = IO_NPC;
	
	loadScript(io->script, g_resources->getFile(script));
	
	io->spellcast_data.castingspell = SPELL_NONE;
	io->_npcdata->manaPool.current = io->_npcdata->manaPool.max = 10.f;
	io->_npcdata->critical = 5.f;
	io->_npcdata->reach = 20.f;
	io->_npcdata->stare_factor = 1.f;
	
	if(!(flags & NO_ON_LOAD)) {
		SendIOScriptEvent(NULL, io, SM_LOAD);
	}
	
	io->pos = player.pos;
	io->pos += angleToVectorXZ(player.angle.getYaw()) * 140.f;
	
	io->lastpos = io->initpos = io->pos;
	io->lastpos.x = io->initpos.x = glm::abs(io->initpos.x / 20) * 20.f;
	io->lastpos.z = io->initpos.z = glm::abs(io->initpos.z / 20) * 20.f;
	
	float tempo;
	EERIEPOLY * ep = CheckInPoly(io->pos + Vec3f(0.f, player.baseHeight(), 0.f));
	if(ep && GetTruePolyY(ep, io->pos, &tempo)) {
		io->lastpos.y = io->initpos.y = io->pos.y = tempo;
	}
	
	ep = CheckInPoly(io->pos);
	if(ep) {
		io->pos.y = std::min(ep->v[0].p.y, ep->v[1].p.y);
		io->lastpos.y = io->initpos.y = io->pos.y = std::min(io->pos.y, ep->v[2].p.y);
	}
	
	if(!io->obj && !(flags & NO_MESH)) {
		io->obj = loadObject(object, false);
	}
	
	io->_npcdata->pathfind.listnb = -1;
	io->_npcdata->behavior = BEHAVIOUR_NONE;
	io->_npcdata->pathfind.truetarget = EntityHandle();
	
	if(!(flags & NO_MESH) && (flags & IO_IMMEDIATELOAD)) {
		EERIE_COLLISION_Cylinder_Create(io);
	}
	
	io->infracolor = Color3f(1.f, 0.f, 0.2f);
	io->collision = COLLIDE_WITH_PLAYER;
	io->m_icon = NULL;
	
	ARX_INTERACTIVE_HideGore(io);
	return io;
}

Entity * AddItem(const res::path & classPath_, EntityInstance instance, AddInteractiveFlags flags) {
	
	EntityFlags type = IO_ITEM;

	res::path classPath = classPath_;
	
	if(boost::starts_with(classPath.filename(), "gold_coin")) {
		classPath.up() /= "gold_coin";
		type = IO_ITEM | IO_GOLD;
	}

	if(boost::contains(classPath.string(), "movable")) {
		type = IO_ITEM | IO_MOVABLE;
	}
	
	res::path object = classPath + ".teo";
	res::path script = classPath + ".asl";
	res::path icon = classPath + "[icon]";
	
	if(!g_resources->getFile(("game" / classPath) + ".ftl")
	   && !g_resources->getFile(object) && !g_resources->getFile(script)) {
		return NULL;
	}
	
	if(!g_resources->getFile(res::path(icon).set_ext("bmp"))) {
		return NULL;
	}
	
	if(instance == -1) {
		instance = getFreeEntityInstance(classPath);
	}
	arx_assert(instance > 0);
	
	Entity * io = new Entity(classPath, instance);
	
	io->ioflags = type;
	io->_itemdata = new IO_ITEMDATA();
	io->_itemdata->count = 1;
	io->_itemdata->maxcount = 1;
	io->_itemdata->food_value = 0;
	io->_itemdata->LightValue = -1;

	if(io->ioflags & IO_GOLD)
		io->_itemdata->price = 1;
	else
		io->_itemdata->price = 10;

	io->_itemdata->playerstacksize = 1;

	loadScript(io->script, g_resources->getFile(script));
	
	if(!(flags & NO_ON_LOAD)) {
		SendIOScriptEvent(NULL, io, SM_LOAD);
	}
	
	io->spellcast_data.castingspell = SPELL_NONE;
	
	io->pos = player.pos;
	io->pos += angleToVectorXZ(player.angle.getYaw()) * 140.f;
	
	io->lastpos.x = io->initpos.x = std::floor(io->pos.x / 20.f) * 20.f;
	io->lastpos.z = io->initpos.z = std::floor(io->pos.z / 20.f) * 20.f;

	EERIEPOLY * ep;
	ep = CheckInPoly(io->pos + Vec3f(0.f, -60.f, 0.f));
	
	if(ep) {
		float tempo;
		if(GetTruePolyY(ep, io->pos, &tempo)) {
			io->lastpos.y = io->initpos.y = io->pos.y = tempo;
		}
	}
	
	ep = CheckInPoly(io->pos);

	if(ep) {
		io->pos.y = std::min(ep->v[0].p.y, ep->v[1].p.y);
		io->lastpos.y = io->initpos.y = io->pos.y = std::min(io->pos.y, ep->v[2].p.y);
	}

	if(io->ioflags & IO_GOLD) {
		io->obj = GoldCoinsObj[0];
	}
	
	if(!io->obj && !(flags & NO_MESH)) {
		io->obj = loadObject(object);
	}
	
	TextureContainer * tc;
	if(io->ioflags & IO_MOVABLE) {
		tc = cursorMovable;
	} else if(io->ioflags & IO_GOLD) {
		tc = GoldCoinsTC[0];
	} else {
		tc = TextureContainer::LoadUI(icon, TextureContainer::Level);
	}

	if(!tc) {
		tc = TextureContainer::LoadUI("graph/interface/misc/default[icon]");
	}
	
	if(tc) {
		io->m_inventorySize = inventorySizeFromTextureSize(tc->size());
		io->m_icon = tc;
	}

	io->infracolor = Color3f(0.2f, 0.2f, 1.f);
	io->collision = 0;

	return io;
}

/*!
 * \brief Returns nearest interactive object found at position x, y
 */
Entity * GetFirstInterAtPos(const Vec2s & pos, long flag, Vec3f * _pRef, Entity ** _pTable, size_t * _pnNbInTable)
{
	float _fdist = 9999999999.f;
	float fdistBB = 9999999999.f;
	float fMaxDist = flag ? 9999999999.f : 350;
	Entity * foundBB = NULL;
	Entity * foundPixel = NULL;
	bool bPlayerEquiped = false;

	if(player.m_telekinesis) {
		fMaxDist = 850;
	}

	size_t nStart = 1;
	size_t nEnd = entities.size();

	if(flag == 3 && _pTable && _pnNbInTable) {
		nStart = 0;
		nEnd = *_pnNbInTable;
	}

	for(size_t i = nStart; i < nEnd; i++) {
		const EntityHandle handle = EntityHandle(i);
		
		bool bPass = true;

		Entity * io;
		
		if(flag == 3 && _pTable && _pnNbInTable) {
			io = _pTable[i];
		} else {
			io = entities[handle];
		}

		// Is Object Valid ??
		if(!io)
			continue;

		if((io->ioflags & IO_CAMERA) || (io->ioflags & IO_MARKER))
			continue;

		if(!flag && !(io->gameFlags & GFLAG_INTERACTIVITY))
			continue;

		// Is Object in TreatZone ??
		bPlayerEquiped = IsEquipedByPlayer(io);

		if( !((bPlayerEquiped  && (player.Interface & INTER_PLAYERBOOK)) || (io->gameFlags & GFLAG_ISINTREATZONE)) )
			continue;

		// Is Object Displayed on screen ???
		if(!(io->show == SHOW_FLAG_IN_SCENE
		     || (bPlayerEquiped && flag)
		     || (bPlayerEquiped && (player.Interface & INTER_PLAYERBOOK)
		         && (g_playerBook.currentPage() == BOOKMODE_STATS)))) {
			continue;
		}

		if(flag == 2 && _pTable && _pnNbInTable && ((*_pnNbInTable) < 256)) {
			_pTable[ *_pnNbInTable ] = io;
			(*_pnNbInTable)++;
			continue;
		}

		if(pos.x < io->bbox2D.min.x ||
		   pos.x > io->bbox2D.max.x ||
		   pos.y < io->bbox2D.min.y ||
		   pos.y > io->bbox2D.max.y)
		{
			continue;
		}
		
		if(flag && _pRef) {
			float flDistanceToRef = arx::distance2(g_camera->m_pos, *_pRef);
			float flDistanceToIO = arx::distance2(g_camera->m_pos, io->pos);
			bPass = bPlayerEquiped || (flDistanceToIO < flDistanceToRef);
		}
		
		float fp = fdist(io->pos, player.pos);

		if((!flag && fp <= fMaxDist) && (!foundBB || fp < fdistBB)) {
			fdistBB = fp;
			foundBB = io;
		}

		if((io->ioflags & (IO_CAMERA | IO_MARKER | IO_GOLD)) || (bPlayerEquiped && !flag)) {
			if(bPlayerEquiped)
				fp = 0.f;
			else
				fp = fdist(io->pos, player.pos);

			if(fp < fdistBB || !foundBB) {
				fdistBB = fp;
				foundBB = io;
				foundPixel = io;
			}
		} else {
			for(size_t j = 0; j < io->obj->facelist.size(); j++) {
				float n = PtIn2DPolyProj(io->obj->vertexClipPositions, &io->obj->facelist[j] , pos.x, pos.y);

				if(n > 0.f) {
					if(bPlayerEquiped)
						fp = 0.f;
					else
						fp = fdist(io->pos, player.pos);

					if((bPass && fp <= fMaxDist) && (fp < _fdist || !foundPixel)) {

						_fdist = fp;
						foundPixel = io;
						break;
					}
				}
			}
		}
	}

	if(foundPixel)
		return foundPixel;

	return foundBB;
}

bool IsEquipedByPlayer(const Entity * io)
{
	if(!io)
		return false;

	if((io->ioflags & IO_ICONIC) && (io->show == SHOW_FLAG_ON_PLAYER))
		return true;

	EntityHandle num = io->index();

	for(size_t i = 0; i < MAX_EQUIPED; i++) {
		if(ValidIONum(player.equiped[i]) && player.equiped[i] == num)
			return true;
	}

	return false;
}

extern long LOOKING_FOR_SPELL_TARGET;
Entity * InterClick(const Vec2s & pos) {
	
	if(InInventoryPos(pos)) {
		return NULL;
	}

	float dist_Threshold;

	if (LOOKING_FOR_SPELL_TARGET)
		dist_Threshold = 550.f;
	else
		dist_Threshold = 360.f;

	Entity * io = GetFirstInterAtPos(pos);

	if(io) {
		if(io->ioflags & IO_NPC) {
			if(closerThan(player.pos, io->pos, dist_Threshold)) {
				return io;
			}
		} else if(player.m_telekinesis) {
			return io;
		} else if(IsEquipedByPlayer(io) || closerThan(player.pos, io->pos, dist_Threshold)) {
			return io;
		}
	}

	return NULL;
}

// Need To upgrade to a more precise collision.
EntityHandle IsCollidingAnyInter(const Vec3f & pos, const Vec3f & size) {
	
	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * io = entities[handle];

		if(   io
		   && !(io->ioflags & IO_NO_COLLISIONS)
		   && (io->collision)
		   && (io->gameFlags & GFLAG_ISINTREATZONE)
		   && ((io->ioflags & IO_NPC) || (io->ioflags & IO_FIX))
		   && (io->show == SHOW_FLAG_IN_SCENE)
		   && !((io->ioflags & IO_NPC) && io->_npcdata->lifePool.current <= 0.f)
		) {
			Vec3f tempPos = pos;
			
			if(IsCollidingInter(io, tempPos))
				return handle;

			tempPos.y += size.y;

			if(IsCollidingInter(io, tempPos))
				return handle;
		}
	}

	return EntityHandle();
}

static bool IsCollidingInter(Entity * io, const Vec3f & pos) {
	
	if(!io || !io->obj)
		return false;

	if(closerThan(pos, io->pos, 190.f)) {
		
		std::vector<EERIE_VERTEX> & vlist = io->obj->vertexWorldPositions;

		if(io->obj->grouplist.size() > 4) {
			for(size_t i = 0; i < io->obj->grouplist.size(); i++) {
				long idx = io->obj->grouplist[i].origin;

				if(!fartherThan(pos, vlist[idx].v, 50.f)) {
					return true;
				}
			}
		} else {
			size_t nbv = io->obj->vertexWorldPositions.size();
			for(size_t i = 0; i < nbv; i++) {
				if(i != io->obj->origin)
					if(!fartherThan(pos, vlist[i].v, 30.f)) {
						return true;
					}
			}
		}
	}

	return false;
}

void SetYlsideDeath(Entity * io) {
	io->sfx_flag = SFX_TYPE_YLSIDE_DEATH;
	io->sfx_time = g_gameTime.now();
}

void UpdateCameras() {
	
	ARX_PROFILE_FUNC();
	
	for(size_t i = 1; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * io = entities[handle];
		
		if(!io)
			continue;

		// interpolate & send events
		if(io->usepath) {
			ARX_USE_PATH * aup = io->usepath;
			GameDuration elapsed = g_gameTime.now() - aup->_curtime;

			if(aup->aupflags & ARX_USEPATH_FORWARD) {
				if(aup->aupflags & ARX_USEPATH_FLAG_FINISHED) {
				} else {
					aup->_curtime += elapsed;
				}
			}

			if(aup->aupflags & ARX_USEPATH_BACKWARD) {
				aup->_starttime += elapsed * 2;
				aup->_curtime += elapsed;

				if(aup->_starttime >= aup->_curtime)
					aup->_curtime = aup->_starttime + GameDurationMs(1);
			}

			if(aup->aupflags & ARX_USEPATH_PAUSE) {
				aup->_starttime += elapsed;
				aup->_curtime += elapsed;
			}

			long last = ARX_PATHS_Interpolate(aup, &io->pos);

			if(aup->lastWP != last) {
				if(last == -2) {
					std::string waypoint = boost::lexical_cast<std::string>(aup->path->pathways.size() - 1);
					SendIOScriptEvent(NULL, io, SM_WAYPOINT, waypoint);
					SendIOScriptEvent(NULL, io, ScriptEventName("waypoint" + waypoint));
					SendIOScriptEvent(NULL, io, SM_PATHEND);
					aup->lastWP = last;
				} else {
					last--;
					long _from = aup->lastWP;
					long _to = last;

					if(_from > _to)
						_from = -1;

					if(_from < 0)
						_from = -1;

					long ii = _from + 1;

					std::string waypoint = boost::lexical_cast<std::string>(ii);
					SendIOScriptEvent(NULL, io, SM_WAYPOINT, waypoint);
					SendIOScriptEvent(NULL, io, ScriptEventName("waypoint" + waypoint));
					if(size_t(ii) == aup->path->pathways.size()) {
						SendIOScriptEvent(NULL, io, SM_PATHEND);
					}
					
					aup->lastWP = last + 1;
				}
			}

			if(io->damager_damages > 0 && io->show == SHOW_FLAG_IN_SCENE) {
				for(size_t i2 = 0; i2 < entities.size(); i2++) {
					const EntityHandle handle2 = EntityHandle(i2);
					Entity * io2 = entities[handle2];

					if(io2
					   && handle2 != handle
					   && io2->show == SHOW_FLAG_IN_SCENE
					   && (io2->ioflags & IO_NPC)
					   && closerThan(io->pos, io2->pos, 600.f)
					) {
						bool Touched = false;

						for(size_t ri = 0; ri < io->obj->vertexlist.size(); ri += 3) {
							for(size_t rii = 0; rii < io2->obj->vertexlist.size(); rii += 3) {
								if(closerThan(io->obj->vertexWorldPositions[ri].v, io2->obj->vertexWorldPositions[rii].v, 20.f)) {
									Touched = true;
									ri = io->obj->vertexlist.size();
									break;
								}
							}
						}

						if(Touched)
							ARX_DAMAGES_DealDamages(handle2, io->damager_damages, handle, io->damager_type, &io2->pos);
					}
				}
			}
		}

		if(io->ioflags & IO_CAMERA) {
			arx_assert(isallfinite(io->pos));
			
			io->_camdata->cam.m_pos = io->pos;

			if(io->targetinfo != EntityHandle(TARGET_NONE)) {
				
				// Follows target
				GetTargetPos(io, static_cast<unsigned long>(io->_camdata->smoothing));
				io->target += io->_camdata->translatetarget;
				
				Vec3f target = io->target;
				if(io->_camdata->lastinfovalid && io->_camdata->smoothing != 0.f) {

					float vv = io->_camdata->smoothing;

					if(vv > 8000)
						vv = 8000;

					vv = (8000 - vv) * (1.0f / 4000);

					float f1 = g_gameTime.lastFrameDuration() / GameDurationMs(1000) * vv;

					if(f1 > 1.f)
						f1 = 1.f;

					float f2 = 1.f - f1;
					target = io->target * f2 + io->_camdata->lasttarget * f1;
				}
				
				io->_camdata->cam.lookAt(target);
				io->_camdata->lasttarget = target;
				io->angle.setPitch(0.f);
				io->angle.setYaw(io->_camdata->cam.angle.getYaw() + 90.f);
				io->angle.setRoll(0.f);
				
			} else {
				// no target...
				io->target = io->pos;
				io->target += angleToVectorXZ(io->angle.getYaw() + 90) * 20.f;
				
				io->_camdata->cam.lookAt(io->target);
				io->_camdata->cam.angle.setPitch(MAKEANGLE(-io->_camdata->cam.angle.getPitch()));
				io->_camdata->cam.angle.setYaw(MAKEANGLE(io->_camdata->cam.angle.getYaw() + 180.f));
				io->_camdata->lasttarget = io->target;
			}

			io->_camdata->lastinfovalid = true;
		}
	}
}

void UpdateIOInvisibility(Entity * io)
{
	if(io && io->invisibility <= 1.f) {
		if((io->gameFlags & GFLAG_INVISIBILITY) && io->invisibility < 1.f) {
			io->invisibility += g_gameTime.lastFrameDuration() / GameDurationMs(1000);
		} else if(!(io->gameFlags & GFLAG_INVISIBILITY) && io->invisibility != 0.f) {
			io->invisibility -= g_gameTime.lastFrameDuration() / GameDurationMs(1000);
		}
		
		io->invisibility = glm::clamp(io->invisibility, 0.f, 1.f);
	}
}

void UpdateInter() {

	for(size_t i = 1; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * io = entities[handle];

		if(   !io
		   || io == DRAGINTER
		   || !(io->gameFlags & GFLAG_ISINTREATZONE)
		   || io->show != SHOW_FLAG_IN_SCENE
		   || (io->ioflags & IO_CAMERA)
		   || (io->ioflags & IO_MARKER)
		) {
			continue;
		}
		
		UpdateIOInvisibility(io);

		Anglef temp = io->angle;

		if(io->ioflags & IO_NPC) {
			temp.setYaw(MAKEANGLE(180.f - temp.getYaw()));
		} else {
			temp.setYaw(MAKEANGLE(270.f - temp.getYaw()));
		}

		if(io->animlayer[0].cur_anim) {

			io->bbox2D.min.x = 9999;
			io->bbox2D.max.x = -1;

			AnimationDuration diff;
			if(io->animlayer[0].flags & EA_PAUSED)
				diff = 0;
			else
				diff = toAnimationDuration(g_gameTime.lastFrameDuration());

			Vec3f pos = io->pos;

			if(io->ioflags & IO_NPC) {
				ComputeVVPos(io);
				pos.y = io->_npcdata->vvpos;
			}

			EERIEDrawAnimQuatUpdate(io->obj, io->animlayer, temp, pos, diff, io, true);
		}
	}
}


/*!
 * \brief Render entities
 */
void RenderInter() {
	
	ARX_PROFILE_FUNC();
	
	UseTextureState textureState(TextureStage::FilterLinear, TextureStage::WrapClamp);
	
	for(size_t i = 1; i < entities.size(); i++) { // Player isn't rendered here...
		const EntityHandle handle = EntityHandle(i);
		Entity * io = entities[handle];

		if(   !io
		   || !(io->gameFlags & GFLAG_ISINTREATZONE)
		   || io->show != SHOW_FLAG_IN_SCENE
		   || (io->ioflags & IO_CAMERA)
		   || (io->ioflags & IO_MARKER)
		) {
			continue;
		}
		
		float invisibility = Cedric_GetInvisibility(io);
		
		if(io->animlayer[0].cur_anim) {
			
			Vec3f pos = io->pos;
			if(io->ioflags & IO_NPC) {
				pos.y = io->_npcdata->vvpos;
			}
			
			EERIEDrawAnimQuatRender(io->obj, pos, io, invisibility);
			
		} else {
			
			io->bbox2D.min.x = 9999;
			io->bbox2D.max.x = -1;
			
			if(io->obj) {
				UpdateGoldObject(io);
			}
			
			if(!(io->ioflags & IO_NPC) && io->obj) {
				Anglef angle = io->angle;
				angle.setYaw(MAKEANGLE(270.f - angle.getYaw()));
				glm::quat rotation = glm::quat_cast(toRotationMatrix(angle));
				TransformInfo t(io->pos, rotation, io->scale);
				DrawEERIEInter(io->obj, t, io, false, invisibility);
			}
			
		}
		
	}
	
}

static std::vector<Entity *> toDestroy;

bool ARX_INTERACTIVE_DestroyIOdelayed(Entity * entity) {
	
	if((entity->ioflags & IO_ITEM) && entity->_itemdata->count > 1) {
		entity->_itemdata->count--;
		return false;
	}
	
	LogDebug("will destroy entity " << entity->idString());
	if(std::find(toDestroy.begin(), toDestroy.end(), entity) == toDestroy.end()) {
		toDestroy.push_back(entity);
	}
	
	return true;
}

void ARX_INTERACTIVE_DestroyIOdelayedRemove(Entity * entity) {
	Entity * null = NULL;
	// Remove the entity from the list but don't invalidate iterators
	std::replace(toDestroy.begin(), toDestroy.end(), entity, null);
}

void ARX_INTERACTIVE_DestroyIOdelayedExecute() {
	if(!toDestroy.empty()) {
		LogDebug("executing delayed entity destruction");
	}
	for(std::vector<Entity *>::iterator it = toDestroy.begin(); it != toDestroy.end(); ++it) {
		if(*it) {
			(*it)->destroy();
		}
	}
	toDestroy.clear();
}

bool IsSameObject(Entity * io, Entity * ioo) {
	
	if(!io || !ioo || io->classPath() != ioo->classPath() || (io->ioflags & IO_UNIQUE)
	   || io->durability != ioo->durability || io->max_durability != ioo->max_durability) {
		return false;
	}
	
	if((io->ioflags & IO_ITEM) && (ioo->ioflags & IO_ITEM)
	   && !io->over_script.valid && !ioo->over_script.valid) {
		if(io->locname == ioo->locname) {
			return true;
		}
	}
	
	return false;
}

static bool intersect(const std::set<std::string> & set1, const std::set<std::string> & set2) {
	
	if(set1.empty() || set2.empty()) {
		return false;
	}
	
	typedef std::set<std::string>::const_iterator itr;
	
	itr it1 = set1.begin();
	itr it2 = set2.begin();
	
	if(*it1 > *set2.rbegin() || *it2 > *set1.rbegin()) {
		return false;
	}
	
	while(it1 != set1.end() && it2 != set2.end()) {
		if(*it1 == *it2) {
			return true;
		} else if(*it1 < *it2) {
			++it1;
		} else {
			++it2;
		}
	}
	
	return false;
}

bool HaveCommonGroup(Entity * io, Entity * ioo) {
	return io && ioo && intersect(io->groups, ioo->groups);
}

float ARX_INTERACTIVE_GetArmorClass(Entity * io) {

	if(!io || !(io->ioflags & IO_NPC))
		return -1;

	float ac = io->_npcdata->armor_class;

	SpellBase * spell;
	
	spell = spells.getSpellOnTarget(io->index(), SPELL_LOWER_ARMOR);
	if(spell) {
		ac += spell->m_level;
	}
	
	spell = spells.getSpellOnTarget(io->index(), SPELL_LOWER_ARMOR);
	if(spell) {
		ac -= spell->m_level;
	}
	
	if(ac < 0)
		ac = 0;

	return ac;
}

void ARX_INTERACTIVE_ActivatePhysics(EntityHandle t)
{
	Entity * io = entities.get(t);
	if(io) {
		
		if(io == DRAGINTER || (io->show != SHOW_FLAG_IN_SCENE))
			return;

		float yy;
		EERIEPOLY * ep = CheckInPoly(io->pos, &yy);

		if(ep && (yy - io->pos.y < 10.f))
			return;

		io->obj->pbox->active = 1;
		io->obj->pbox->stopcount = 0;
		Vec3f fallvector = Vec3f(0.0f, 0.000001f, 0.f);
		io->show = SHOW_FLAG_IN_SCENE;
		io->soundtime = 0;
		io->soundcount = 0;
		EERIE_PHYSICS_BOX_Launch(io->obj, io->pos, io->angle, fallvector);
	}
}

std::string GetMaterialString(const res::path & texture) {
	
	const std::string & origin = texture.string();
	
	// need to be precomputed !!!
	if(boost::contains(origin, "stone")) return "stone";
	else if(boost::contains(origin, "marble")) return "stone";
	else if(boost::contains(origin, "rock")) return "stone";
	else if(boost::contains(origin, "wood")) return "wood";
	else if(boost::contains(origin, "wet")) return "wet";
	else if(boost::contains(origin, "mud")) return "wet";
	else if(boost::contains(origin, "blood")) return "wet";
	else if(boost::contains(origin, "bone")) return "wet";
	else if(boost::contains(origin, "flesh")) return "wet";
	else if(boost::contains(origin, "shit")) return "wet";
	else if(boost::contains(origin, "soil")) return "gravel";
	else if(boost::contains(origin, "gravel")) return "gravel";
	else if(boost::contains(origin, "earth")) return "gravel";
	else if(boost::contains(origin, "dust")) return "gravel";
	else if(boost::contains(origin, "sand")) return "gravel";
	else if(boost::contains(origin, "straw")) return "gravel";
	else if(boost::contains(origin, "metal")) return "metal";
	else if(boost::contains(origin, "iron")) return "metal";
	else if(boost::contains(origin, "glass")) return "metal";
	else if(boost::contains(origin, "rust")) return "metal";
	else if(boost::contains(origin, "ice")) return "ice";
	else if(boost::contains(origin, "fabric")) return "carpet";
	else if(boost::contains(origin, "moss")) return "carpet";
	else return "unknown";
}

void UpdateGoldObject(Entity * io) {
	
	if(io->ioflags & IO_GOLD) {
		
		long num = 0;
		if(io->_itemdata->price <= 3) {
			num = io->_itemdata->price - 1;
		} else if(io->_itemdata->price <= 8) {
			num = 3;
		} else if(io->_itemdata->price <= 20) {
			num = 4;
		} else if(io->_itemdata->price <= 50) {
			num = 5;
		} else {
			num = 6;
		}
		
		io->obj = GoldCoinsObj[num];
		io->m_icon = GoldCoinsTC[num];
		
	}
	
}
