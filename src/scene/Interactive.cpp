/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#include <boost/algorithm/string/predicate.hpp>

#include <glm/gtx/norm.hpp>

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

#include "gui/Speech.h"
#include "gui/Interface.h"

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

#include "physics/Anchors.h"
#include "physics/Collisions.h"
#include "physics/CollisionShapes.h"
#include "physics/Box.h"
#include "physics/Clothes.h"

#include "platform/Thread.h"

#include "scene/ChangeLevel.h"
#include "scene/GameSound.h"
#include "scene/Scene.h"
#include "scene/LinkedObject.h"
#include "scene/LoadLevel.h"
#include "scene/Light.h"
#include "scene/Object.h"

#include "script/ScriptEvent.h"

using std::min;
using std::vector;

extern Entity * CAMERACONTROLLER;
extern TextureContainer * Movable;

long HERO_SHOW_1ST = 1;

static bool IsCollidingInter(Entity * io, const Vec3f & pos);
static Entity * AddCamera(const res::path & classPath, EntityInstance instance = -1);
static Entity * AddMarker(const res::path & classPath, EntityInstance instance = -1);

float STARTED_ANGLE = 0;
void Set_DragInter(Entity * io)
{
	if(io != DRAGINTER)
		STARTED_ANGLE = player.angle.getPitch();

	DRAGINTER = io;

	if(io && io->obj && io->obj->pbox) {
		io->obj->pbox->active = 0;
	}
}

// Checks if an IO index number is valid
bool ValidIONum(EntityHandle num) {
	
	return !(num < 0 || num >= long(entities.size()) || !entities[num]);
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

static float ARX_INTERACTIVE_fGetPrice(Entity * io, Entity * shop) {
	
	if(!io || !(io->ioflags & IO_ITEM))
		return 0;

	float durability_ratio = io->durability / io->max_durability;
	float shop_multiply = 1.f;

	if(shop)
		shop_multiply = shop->shop_multiply;

	return io->_itemdata->price * shop_multiply * durability_ratio;

}

long ARX_INTERACTIVE_GetPrice(Entity * io, Entity * shop) {
	return ARX_INTERACTIVE_fGetPrice(io, shop);
}

static void ARX_INTERACTIVE_ForceIOLeaveZone(Entity * io, long flags) {
	
	ARX_PATH * op = io->inzone;

	if(op) {
		std::string temp = op->name;

		if(flags & 1) // no need when being destroyed !
			SendIOScriptEvent(io, SM_LEAVEZONE, temp);

		if(!op->controled.empty()) {
			EntityHandle t = entities.getById(op->controled);

			if(t != InvalidEntityHandle) {
				std::string str = io->idString() + ' ' + temp;
				SendIOScriptEvent( entities[t], SM_CONTROLLEDZONE_LEAVE, str ); 
			}
		}
	}
}

void ARX_INTERACTIVE_DestroyDynamicInfo(Entity * io)
{
	if(!io)
		return;

	EntityHandle n = io->index();

	ARX_INTERACTIVE_ForceIOLeaveZone(io, 0);

	for(long i = 0; i < MAX_EQUIPED; i++) {
		if(player.equiped[i] && player.equiped[i] == n && ValidIONum(player.equiped[i])) {
			ARX_EQUIPMENT_UnEquip(entities.player(), entities[player.equiped[i]], 1);
			player.equiped[i] = EntityHandle(0); // TODO inband signaling
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
			Thread::sleep(1);
		}
		free(io->_npcdata->pathfind.list);
		memset(&io->_npcdata->pathfind, 0, sizeof(IO_PATHFIND));
	}
	
	lightHandleDestroy(io->dynlight);

	IO_UnlinkAllLinkedObjects(io);
}


bool ARX_INTERACTIVE_Attach(EntityHandle n_source, EntityHandle n_target, const std::string& ap_source, const std::string& ap_target)
{
	if(!ValidIONum(n_source) || !ValidIONum(n_target))
		return false;

	entities[n_source]->show = SHOW_FLAG_LINKED;
	EERIE_LINKEDOBJ_UnLinkObjectFromObject(entities[n_target]->obj, entities[n_source]->obj);
	return EERIE_LINKEDOBJ_LinkObjectToObject(entities[n_target]->obj,
	        entities[n_source]->obj, ap_target, ap_source, entities[n_source]);
}

void ARX_INTERACTIVE_Detach(EntityHandle n_source, EntityHandle n_target)
{
	if(!ValidIONum(n_source) || !ValidIONum(n_target))
		return;

	entities[n_source]->show = SHOW_FLAG_IN_SCENE;
	EERIE_LINKEDOBJ_UnLinkObjectFromObject(entities[n_target]->obj, entities[n_source]->obj);
}

void ARX_INTERACTIVE_Show_Hide_1st(Entity * io, long state)
{
	if(!io || HERO_SHOW_1ST == state)
		return;

	HERO_SHOW_1ST = state;
	long grp = EERIE_OBJECT_GetSelection(io->obj, "1st");

	if(grp != -1) {
		for(size_t nn = 0; nn < io->obj->facelist.size(); nn++) {
			EERIE_FACE * ef = &io->obj->facelist[nn];

			for(long jj = 0; jj < 3; jj++) {
				if(IsInSelection(io->obj, ef->vid[jj], grp) != -1) {
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
			//Hide Gore Polys...
			if(io->obj->facelist[nn].texid == gorenum)
				io->obj->facelist[nn].facetype |= POLY_HIDE;
			else if(!flag)
				io->obj->facelist[nn].facetype &= ~POLY_HIDE;
		}
	}
}

bool ForceNPC_Above_Ground(Entity * io) {
	
	if(io && (io->ioflags & IO_NPC) && !(io->ioflags & IO_PHYSICAL_OFF)) {
		io->physics.cyl.origin = io->pos;
		AttemptValidCylinderPos(io->physics.cyl, io, CFLAG_NO_INTERCOL);
		if(EEfabs(io->pos.y - io->physics.cyl.origin.y) < 45.f) {
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
		
		linked->angle = Anglef(rnd() * 40.f + 340.f, rnd() * 360.f, 0.f);
		linked->soundtime = 0;
		linked->soundcount = 0;
		linked->gameFlags |= GFLAG_NO_PHYS_IO_COL;
		linked->show = SHOW_FLAG_IN_SCENE;
		linked->no_collide = io->index();
		
		Vec3f pos = io->obj->vertexlist3[io->obj->linked[k].lidx].v;
		Vec3f vector;
		vector.x = -std::sin(radians(linked->angle.getPitch())) * 0.5f;
		vector.y =  std::sin(radians(linked->angle.getYaw()));
		vector.z =  std::cos(radians(linked->angle.getPitch())) * 0.5f;
		EERIE_PHYSICS_BOX_Launch(linked->obj, pos, linked->angle, vector);
		
	}
	
	EERIE_LINKEDOBJ_ReleaseData(io->obj);
}

// First is always the player
TREATZONE_IO * treatio = NULL;
long TREATZONE_CUR = 0;
static long TREATZONE_MAX = 0;

void TREATZONE_Clear() {
	TREATZONE_CUR = 0;
}

void TREATZONE_Release() {
	free(treatio);
	treatio = NULL;
	TREATZONE_MAX = 0;
	TREATZONE_CUR = 0;
}

void TREATZONE_RemoveIO(Entity * io)
{
	if(treatio) {
		for(long i = 0; i < TREATZONE_CUR; i++) {
			if(treatio[i].io == io) {
				treatio[i].io = NULL;
				treatio[i].ioflags = 0;
				treatio[i].show = 0;
			}
		}
	}
}

// flag & 1 IO_JUST_COLLIDE
void TREATZONE_AddIO(Entity * io, long flag)
{
	if(TREATZONE_MAX == TREATZONE_CUR) {
		TREATZONE_MAX++;
		treatio = (TREATZONE_IO *)realloc(treatio, sizeof(TREATZONE_IO) * TREATZONE_MAX);
	}

	for(long i = 0; i < TREATZONE_CUR; i++) {
		if(treatio[i].io == io)
			return;
	}

	treatio[TREATZONE_CUR].io = io;
	treatio[TREATZONE_CUR].ioflags = io->ioflags;

	if(flag & 1)
		treatio[TREATZONE_CUR].ioflags |= IO_JUST_COLLIDE;

	treatio[TREATZONE_CUR].show = io->show;
	treatio[TREATZONE_CUR].num = io->index();
	TREATZONE_CUR++;
}

void CheckSetAnimOutOfTreatZone(Entity * io, long num)
{
	arx_assert(io);

	if( io->animlayer[num].cur_anim &&
		!(io->gameFlags & GFLAG_ISINTREATZONE) &&
		fartherThan(io->pos, ACTIVECAM->orgTrans.pos, 2500.f))
	{

		io->animlayer[num].ctime = io->animlayer[num].cur_anim->anims[io->animlayer[num].altidx_cur]->anim_time - 1;
	}
}

void PrepareIOTreatZone(long flag)
{
	static long status = -1;
	static Vec3f lastpos;

	if(flag || status == -1) {
		status = 0;
		lastpos = ACTIVECAM->orgTrans.pos;
	} else if(status == 3) {
		status = 0;
	}

	if(fartherThan(ACTIVECAM->orgTrans.pos, lastpos, 100.f)) {
		status = 0;
		lastpos = ACTIVECAM->orgTrans.pos;
	}

	if(status++)
		return;

	TREATZONE_Clear();
	long Cam_Room = ARX_PORTALS_GetRoomNumForPosition(ACTIVECAM->orgTrans.pos, 1);
	long PlayerRoom = ARX_PORTALS_GetRoomNumForPosition(player.pos, 1);
	TREATZONE_AddIO(entities.player());

	short sGlobalPlayerRoom = checked_range_cast<short>(PlayerRoom);

	for(long i = 0; i < MAX_EQUIPED; i++) {
		if(player.equiped[i] != PlayerEntityHandle && ValidIONum(player.equiped[i])) {
			Entity *toequip = entities[player.equiped[i]];

			if(toequip) {
				toequip->room = sGlobalPlayerRoom;
				toequip->room_flags = 0;
			}
		}
	}

	if(DRAGINTER)
		TREATZONE_AddIO(DRAGINTER);

	float TREATZONE_LIMIT = 3200;

	if(RoomDistance) {
		TREATZONE_LIMIT += 600; 

		if(CURRENTLEVEL == 4)
			TREATZONE_LIMIT += 1200;

		if(ACTIVECAM->cdepth > 3000)
			TREATZONE_LIMIT += 500;

		if(ACTIVECAM->cdepth > 4000)
			TREATZONE_LIMIT += 500;

		if(ACTIVECAM->cdepth > 6000)
			TREATZONE_LIMIT += 500;
	}
	
	for(size_t i = 1; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * io = entities[handle];

		if ((io)
		        &&	((io->show == SHOW_FLAG_IN_SCENE)
		             ||	(io->show == SHOW_FLAG_TELEPORTING)
		             ||	(io->show == SHOW_FLAG_ON_PLAYER)
		             ||	(io->show == SHOW_FLAG_HIDDEN)))   
		{
			char treat;

			if (io->ioflags & IO_CAMERA) {
				treat = 0;
			} else if (io->ioflags & IO_MARKER) {
				treat = 0;
			} else if ((io->ioflags & IO_NPC) && (io->_npcdata->pathfind.flags & PATHFIND_ALWAYS)) {
				treat = 1;
			} else {
				float dists;

				if(Cam_Room >= 0) {
					if(io->show == SHOW_FLAG_TELEPORTING) {
						Vec3f pos;
						GetItemWorldPosition(io, &pos);
						dists = glm::distance2(ACTIVECAM->orgTrans.pos, pos);
					} else {
						if(io->room_flags & 1)
							UpdateIORoom(io);

						dists = square(SP_GetRoomDist(io->pos, ACTIVECAM->orgTrans.pos, io->room, Cam_Room));
					}
				} else {
					if(io->show == SHOW_FLAG_TELEPORTING) {
						Vec3f pos;
						GetItemWorldPosition(io, &pos);
						dists = glm::distance2(ACTIVECAM->orgTrans.pos, pos); //&io->pos,&pos);
					}
					else
						dists = glm::distance2(io->pos, ACTIVECAM->orgTrans.pos);
				}
		
				if(dists < square(TREATZONE_LIMIT))
					treat = 1;
				else
					treat = 0;
			}

			if(!treat) {
				if(io == CAMERACONTROLLER)
					treat = 1;

				if(io == DRAGINTER)
					treat = 1;
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
					iooo->room_flags = io->room_flags;
				}
			} else {
				io->gameFlags &= ~GFLAG_ISINTREATZONE;
			}
			
			EVENT_SENDER = NULL;

			if ((io->gameFlags & GFLAG_ISINTREATZONE)
			        && (!(io->gameFlags & GFLAG_WASINTREATZONE)))
			{
				//coming back; doesn't really matter right now
				//	SendIOScriptEvent(entities[i],SM_TREATIN);

			}
			else if ((!(io->gameFlags & GFLAG_ISINTREATZONE))
			         &&	(io->gameFlags & GFLAG_WASINTREATZONE))
			{
				//going away;
				io->gameFlags |= GFLAG_ISINTREATZONE;

				if(SendIOScriptEvent(io, SM_TREATOUT) != REFUSE) {
					if(io->ioflags & IO_NPC)
						io->_npcdata->pathfind.flags &= ~PATHFIND_ALWAYS;

					io->gameFlags &= ~GFLAG_ISINTREATZONE;
				}
			}
		}
	}

	long M_TREAT = TREATZONE_CUR;

	for(size_t i = 1; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * io = entities[handle];

		if(io && !(io->gameFlags & GFLAG_ISINTREATZONE)
		        && ((io->show == SHOW_FLAG_IN_SCENE)
		            ||	(io->show == SHOW_FLAG_TELEPORTING)
		            ||	(io->show == SHOW_FLAG_ON_PLAYER)
		            ||	(io->show == SHOW_FLAG_HIDDEN)))   // show 5 = ininventory; 15 = destroyed
		{
			if(io->ioflags & (IO_CAMERA | IO_ITEM | IO_MARKER))
				continue;

			long toadd = 0;

			for(long ii = 1; ii < M_TREAT; ii++) {
				Entity * ioo = treatio[ii].io;

				if(ioo) {
					if(closerThan(io->pos, ioo->pos, 300.f)) {
						toadd = 1;
						break;
					}
				}
			}

			if(toadd) {
				TREATZONE_AddIO(io, 1);
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
	ARX_INTERACTIVE_HideGore(entities.player());
	ARX_NPC_Behaviour_ResetAll();
	if(entities.player()) {
		entities.player()->spellcast_data.castingspell = SPELL_NONE;
	}
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
	} else if (io->ioflags & IO_ITEM) {
		io->usemesh = "graph/obj3d/interactive/items" / temp;
	} else {
		io->usemesh.clear();
	}
	
	if(io->usemesh.empty() ) {
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

void ARX_INTERACTIVE_ClearIODynData(Entity * io) {
	
	if(!io)
		return;
	
	lightHandleDestroy(io->dynlight);
	lightHandleDestroy(io->halo.dynlight);
	
	free(io->symboldraw);
	io->symboldraw = NULL;
	
	io->spellcast_data.castingspell = SPELL_NONE;
}

void ARX_INTERACTIVE_ClearIODynData_II(Entity * io)
{
	if(!io)
		return;
	
	ARX_INTERACTIVE_ClearIODynData(io);
	
	io->shop_category.clear();
	io->inventory_skin.clear();
	
	io->tweaks.clear();
	io->groups.clear();
	ARX_INTERACTIVE_HideGore(io);
	MOLLESS_Clear(io->obj);
	ARX_SCRIPT_Timer_Clear_For_IO(io);
	
	io->stepmaterial.clear();
	io->armormaterial.clear();
	io->weaponmaterial.clear();
	io->strikespeech.clear();
	
	io->animBlend.nb_lastanimvertex = 0;
	
	for(long j = 0; j < MAX_ANIMS; j++) {
		EERIE_ANIMMANAGER_ReleaseHandle(io->anims[j]);
		io->anims[j] = NULL;
	}
	
	spells.removeTarget(io);
	ARX_EQUIPMENT_ReleaseAll(io);
	
	if(io->ioflags & IO_NPC) {
		free(io->_npcdata->pathfind.list);
		io->_npcdata->pathfind.list = NULL;
		memset(&io->_npcdata->pathfind, 0, sizeof(IO_PATHFIND));
		io->_npcdata->pathfind.truetarget = InvalidEntityHandle;
		io->_npcdata->pathfind.listnb = -1;
		ARX_NPC_Behaviour_Reset(io);
	}
	
	delete io->tweakerinfo;
	io->tweakerinfo = NULL;
	
	if(io->inventory != NULL) {
		INVENTORY_DATA * id = io->inventory;
		
		for(long nj = 0; nj < id->sizey; nj++) {
			for(long ni = 0; ni < id->sizex; ni++) {
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
	ARX_INTERACTIVE_HideGore(entities.player());
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
		io->move = Vec3f_ZERO;
		io->lastmove = Vec3f_ZERO;
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

		io->halo_native.color.r = 0.2f;
		io->halo_native.color.g = 0.5f;
		io->halo_native.color.b = 1.f;
		io->halo_native.radius = 45.f;
		io->halo_native.flags = 0;

		ARX_HALO_SetToNative(io);
		io->halo.dynlight = InvalidLightHandle;

		io->forcedmove = Vec3f_ZERO;
		io->ioflags &= ~IO_NO_COLLISIONS;
		io->ioflags &= ~IO_INVERTED;
		io->lastspeechflag = 2;
	
		io->no_collide = InvalidEntityHandle;

		MagicFlareReleaseEntity(io);

		io->flarecount = 0;
		io->inzone = NULL;
		io->speed_modif = 0.f;
		io->basespeed = 1.f;
		io->frameloss = 0.f;
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
		io->move = Vec3f_ZERO;
		io->type_flags = 0;
		io->sound = -1;
		io->soundtime = 0;
		io->soundcount = 0;
		io->material = MATERIAL_STONE;
		io->collide_door_time = 0;
		io->ouch_time = 0;
		io->dmg_sum = 0;
		io->ignition = 0.f;
		io->ignit_light = InvalidLightHandle;
		io->ignit_sound = audio::INVALID_ID;

		if(io->obj && io->obj->pbox)
			io->obj->pbox->active = 0;

		io->room = -1;
		io->room_flags = 1;
		RestoreIOInitPos(io);
		ARX_INTERACTIVE_Teleport(io, io->initpos);
		io->animBlend.lastanimtime = 1;
		io->secretvalue = -1;
		
		io->poisonous = 0;
		io->poisonous_count = 0;

		for(long count = 0; count < MAX_ANIM_LAYERS; count++) {
			memset(&io->animlayer[count], 0, sizeof(ANIM_USE));
			io->animlayer[count].cur_anim = NULL;
			io->animlayer[count].next_anim = NULL;
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
		io->summoner = -1;
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
			io->_npcdata->collid_state = 0;
			io->_npcdata->collid_time = 0;
			io->_npcdata->strike_time = 0;
			io->_npcdata->walk_start_time = 0;

			io->_npcdata->reachedtarget = 0;
			io->_npcdata->lifePool.max = 20.f;
			io->_npcdata->lifePool.current = io->_npcdata->lifePool.max;
			io->_npcdata->manaPool.max = 10.f;
			io->_npcdata->manaPool.current = io->_npcdata->manaPool.max;
			io->_npcdata->critical = 5.f;
			io->infracolor.r = 1.f;
			io->infracolor.g = 0.f;
			io->infracolor.b = 0.2f;
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
		}
		else io->collision = 0;

		if(io->ioflags & IO_FIX) {
			memset(io->_fixdata, 0, sizeof(IO_FIXDATA));
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
		unsigned long w = tc->m_dwWidth >> 5;
		unsigned long h = tc->m_dwHeight >> 5; 

		if ((w << 5) != tc->m_dwWidth)
			io->sizex = (char)(w + 1);
		else
			io->sizex = (char)(w);

		if ((h << 5) != tc->m_dwHeight)
			io->sizey = (char)(h + 1);
		else
			io->sizey = (char)(h);

		io->sizex = clamp(io->sizex, 1, 3);
		io->sizey = clamp(io->sizey, 1, 3);

		io->inv = tc;
	}
}

/*!
 * \brief Count IO number ignoring ScriptLoaded IOs
 * \return
 */
long GetNumberInterWithOutScriptLoad() {
	long count = 0;
	for(size_t i = 1; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * e = entities[handle];
		
		if(e != NULL && !e->scriptload) {
			count++;
		}
	}
	return count;
}

// Be careful with this func...
Entity * CloneIOItem(Entity * src) {
	
	Entity * dest = AddItem(src->classPath());
	if(!dest) {
		return NULL;
	}
	
	SendInitScriptEvent(dest);
	dest->inv = src->inv;
	dest->sizex = src->sizex;
	dest->sizey = src->sizey;
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
		dest->_itemdata->equipitem = (IO_EQUIPITEM *)malloc(sizeof(IO_EQUIPITEM));
		memcpy(dest->_itemdata->equipitem, src->_itemdata->equipitem,
		       sizeof(IO_EQUIPITEM));
	}
	
	dest->locname = src->locname;
	
	if(dest->obj->pbox == NULL && src->obj->pbox != NULL) {
		dest->obj->pbox = new PHYSICS_BOX_DATA();
		*dest->obj->pbox = *src->obj->pbox;
		
		dest->obj->pbox->vert = (PHYSVERT *)malloc(sizeof(PHYSVERT)
		                                           * src->obj->pbox->nb_physvert);
		memcpy(dest->obj->pbox->vert, src->obj->pbox->vert,
		       sizeof(PHYSVERT) * src->obj->pbox->nb_physvert);
	}
	
	return dest;
}

bool ARX_INTERACTIVE_ConvertToValidPosForIO(Entity * io, Vec3f * target) {
	
	EERIE_CYLINDER phys;
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
		float modx = -std::sin(count) * count * (1.f / 3);
		float modz =  std::cos(count) * count * (1.f / 3);
		phys.origin.x = target->x + modx;
		phys.origin.z = target->z + modz;
		float anything = CheckAnythingInCylinder(phys, io, CFLAG_JUST_TEST);

		if(EEfabs(anything) < 150.f) {
			EERIEPOLY * ep = CheckInPoly(phys.origin + Vec3f(0.f, -20.f, 0.f));
			EERIEPOLY * ep2 = CheckTopPoly(phys.origin + Vec3f(0.f, anything, 0.f));

			if(ep && ep2 && EEfabs((phys.origin.y + anything) - ep->center.y) < 20.f) {
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

void ARX_INTERACTIVE_TeleportBehindTarget(Entity * io)
{
	if(!io)
		return;

	if(ARX_SCRIPT_GetSystemIOScript(io, "_r_a_t_") < 0) {
		long num = ARX_SCRIPT_Timer_GetFree();

		if(num != -1) {
			EntityHandle t = io->index();
			ActiveTimers++;
			scr_timer[num].es = NULL;
			scr_timer[num].exist = 1;
			scr_timer[num].io = io;
			scr_timer[num].msecs = Random::get(3000, 6000);
			scr_timer[num].name = "_r_a_t_";
			scr_timer[num].pos = -1; 
			scr_timer[num].tim = (unsigned long)(arxtime);
			scr_timer[num].times = 1;
			entities[t]->show = SHOW_FLAG_TELEPORTING;
			AddRandomSmoke(io, 10);
			ARX_PARTICLES_Add_Smoke(&io->pos, 3, 20);
			Vec3f pos;
			pos.x = entities[t]->pos.x;
			pos.y = entities[t]->pos.y + entities[t]->physics.cyl.height * ( 1.0f / 2 );
			pos.z = entities[t]->pos.z;
			io->room_flags |= 1;
			io->room = -1;
			ARX_PARTICLES_Add_Smoke(&pos, 3, 20);
			MakeCoolFx(io->pos);
			io->gameFlags |= GFLAG_INVISIBILITY;
		}
	}
}

void ResetVVPos(Entity * io)
{
	if(io && (io->ioflags & IO_NPC))
		io->_npcdata->vvpos = io->pos.y;
}

void ComputeVVPos(Entity * io)
{
	if(io->ioflags & IO_NPC) {
		float vvp = io->_npcdata->vvpos;

		if(vvp == -99999.f || vvp == io->pos.y) {
			io->_npcdata->vvpos = io->pos.y;
			return;
		}

		float diff = io->pos.y - vvp;
		float fdiff = EEfabs(diff);
		float eediff = fdiff;

		if(fdiff > 120.f) {
			fdiff = 120.f;
		} else {
			float mul = ((fdiff * ( 1.0f / 120 )) * 0.9f + 0.6f);

			if(eediff < 15.f) {
				float val = (float)framedelay * ( 1.0f / 4 ) * mul;

				if(eediff < 10.f) {
					val *= ( 1.0f / 10 );
				} else {
					float ratio = (eediff - 10.f) * ( 1.0f / 5 );
					val = val * ratio + val * (1.f - ratio); 
				}

				fdiff -= val;
			} else {
				fdiff -= (float)framedelay * ( 1.0f / 4 ) * mul;
			}
		}

		if(fdiff > eediff)
			fdiff = eediff;

		if(fdiff < 0.f)
			fdiff = 0.f;

		if(diff < 0.f)
			io->_npcdata->vvpos = io->pos.y + fdiff;
		else
			io->_npcdata->vvpos = io->pos.y - fdiff;
	}
}

void ARX_INTERACTIVE_Teleport(Entity * io, const Vec3f & target, bool flag) {
	
	if(!io)
		return;
	
	io->gameFlags &= ~GFLAG_NOCOMPUTATION;
	io->room_flags |= 1;
	io->room = -1;
	
	if(io == entities.player()) {
		moveto = player.pos = target + player.baseOffset();
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
				for(long i = 0; i < io->obj->pbox->nb_physvert; i++) {
					io->obj->pbox->vert[i].pos += translate;
				}
				io->obj->pbox->active = 0;
			}
		}
		for(size_t i = 0; i < io->obj->vertexlist.size(); i++) {
			io->obj->vertexlist3[i].v += translate;
		}
	}
	
	MOLLESS_Clear(io->obj, 1);
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
	} else if (boost::contains(ficc, "marker")) {
		io = AddMarker(classPath, instance);
	}
	
	return io;
}

/*!
 * \brief Links an object designed by path "temp" to the primary attach of interactive object "io"
 * \param io
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
	
	Entity * ioo = io->_npcdata->weapon;
	
	if(ioo && ioo->obj) {
		EERIE_LINKEDOBJ_UnLinkObjectFromObject(io->obj, ioo->obj);

		if(io->gameFlags & GFLAG_HIDEWEAPON)
			return;

		long ni = io->obj->fastaccess.weapon_attach;

		if(ni >= 0) {
			EERIE_LINKEDOBJ_LinkObjectToObject(io->obj, ioo->obj, "weapon_attach", "primary_attach", ioo);
		} else {
			ni = io->obj->fastaccess.secondary_attach;

			if(ni >= 0)
				EERIE_LINKEDOBJ_LinkObjectToObject(io->obj, ioo->obj, "secondary_attach", "primary_attach", ioo);
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
		
		SendIOScriptEvent(ioo, SM_INIT);
		SendIOScriptEvent(ioo, SM_INITEND);
		io->_npcdata->weapontype = ioo->type_flags;
		ioo->show = SHOW_FLAG_LINKED;
		ioo->scriptload = 2;
		
		SetWeapon_Back(io);
	}
}

static void GetIOScript(Entity * io, const res::path & script) {
	loadScript(io->script, resources->getFile(script));
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
 * \param io
 */
static void MakeTemporaryIOIdent(Entity * io) {
	
	if(!io)
		return;
	
	std::string className = io->className();
	res::path classDir = io->classPath().parent();
	
	for(long t = 1; ; t++) {
		
		// Check if the candidate instance number is used in the current scene
		bool used = false;
		// TODO replace this loop by an (className, instance) index
		for(size_t i = 0; i < entities.size(); i++) {
			const EntityHandle handle = EntityHandle(i);
			Entity * e = entities[handle];
			
			if(e && e->ident == t && io != e) {
				if(e->className() == className) {
					used = true;
					break;
				}
			}
		}
		if(used) {
			continue;
		}
		
		std::stringstream ss;
		ss << className << '_' << std::setw(4) << std::setfill('0') << t;
		
		// Check if the candidate instance number is reserved for any scene
		if(resources->getDirectory(classDir / ss.str())) {
			continue;
		}
		
		// Check if the candidate instance number is used in any visited area
		if(currentSavedGameHasEntity(ss.str())) {
			continue;
		}
		
		io->ident = t;
		
		return;
	}
}

Entity * AddFix(const res::path & classPath, EntityInstance instance, AddInteractiveFlags flags) {
	
	res::path object = classPath + ".teo";
	res::path script = classPath + ".asl";
	
	if(!resources->getFile(("game" / classPath) + ".ftl")
	   && !resources->getFile(object) && !resources->getFile(script)) {
		return NULL;
	}
	
	Entity * io = new Entity(classPath);
	
	if(instance == -1) {
		MakeTemporaryIOIdent(io);
	} else {
		arx_assert(instance > 0);
		io->ident = instance;
	}
	
	io->_fixdata = (IO_FIXDATA *)malloc(sizeof(IO_FIXDATA));
	memset(io->_fixdata, 0, sizeof(IO_FIXDATA));
	io->ioflags = IO_FIX;
	io->_fixdata->trapvalue = -1;
	
	GetIOScript(io, script);
	
	if(!(flags & NO_ON_LOAD)) {
		SendIOScriptEvent(io, SM_LOAD);
	}
	
	io->spellcast_data.castingspell = SPELL_NONE;
	io->pos = player.pos;
	io->pos.x -= std::sin(radians(player.angle.getPitch())) * 140.f;
	io->pos.z += std::cos(radians(player.angle.getPitch())) * 140.f;
	io->lastpos = io->initpos = io->pos;
	io->lastpos.x = io->initpos.x = EEfabs(io->initpos.x / 20) * 20.f;
	io->lastpos.z = io->initpos.z = EEfabs(io->initpos.z / 20) * 20.f;
	
	float tempo;
	EERIEPOLY * ep = CheckInPoly(io->pos + Vec3f(0.f, player.baseHeight(), 0.f));
	if(ep && GetTruePolyY(ep, io->pos, &tempo)) {
		io->lastpos.y = io->initpos.y = io->pos.y = tempo;
	}
	
	ep = CheckInPoly(io->pos);
	if(ep) {
		io->pos.y = min(ep->v[0].p.y, ep->v[1].p.y);
		io->lastpos.y = io->initpos.y = io->pos.y = min(io->pos.y, ep->v[2].p.y);
	}
	
	if(!io->obj && !(flags & NO_MESH)) {
		io->obj = loadObject(object, false);
	}
	
	io->infracolor = Color3f(0.6f, 0.f, 1.f);
	
	TextureContainer * tc = TextureContainer::LoadUI("graph/interface/misc/default[icon]"); 
	if(tc) {
		unsigned long w = tc->m_dwWidth >> 5;
		unsigned long h = tc->m_dwHeight >> 5;
		io->sizex = char(clamp(((w << 5) != tc->m_dwWidth) ? (w + 1) : w, 1ul, 3ul));
		io->sizey = char(clamp(((h << 5) != tc->m_dwHeight) ? (h + 1) : h, 1ul, 3ul));
		io->inv = tc;
	}
	
	io->collision = COLLIDE_WITH_PLAYER;
	
	return io;
}

static Entity * AddCamera(const res::path & classPath, EntityInstance instance) {
	
	res::path object = classPath + ".teo";
	res::path script = classPath + ".asl";
	
	if(!resources->getFile(("game" / classPath) + ".ftl")
	   && !resources->getFile(object) && !resources->getFile(script)) {
		return NULL;
	}
	
	Entity * io = new Entity(classPath);
	
	if(instance == -1) {
		MakeTemporaryIOIdent(io);
	} else {
		arx_assert(instance > 0);
		io->ident = instance;
	}
	
	GetIOScript(io, script);
	
	io->pos = player.pos;
	io->pos.x -= std::sin(radians(player.angle.getPitch())) * 140.f;
	io->pos.z += std::cos(radians(player.angle.getPitch())) * 140.f;
	io->lastpos = io->initpos = io->pos;
	io->lastpos.x = io->initpos.x = EEfabs(io->initpos.x / 20) * 20.f;
	io->lastpos.z = io->initpos.z = EEfabs(io->initpos.z / 20) * 20.f;
	
	float tempo;
	EERIEPOLY * ep;
	ep = CheckInPoly(io->pos + Vec3f(0.f, player.baseHeight(), 0.f), &tempo);
	if(ep) {
		io->lastpos.y = io->initpos.y = io->pos.y = tempo;
	}
	
	ep = CheckInPoly(io->pos);
	if(ep) {
		io->pos.y = min(ep->v[0].p.y, ep->v[1].p.y);
		io->lastpos.y = io->initpos.y = io->pos.y = min(io->pos.y, ep->v[2].p.y);
	}
	
	io->lastpos.y = io->initpos.y = io->pos.y += player.baseHeight();
	
	io->obj = cameraobj;
	
	io->_camdata = new IO_CAMDATA();
	io->_camdata->cam = subj;
	io->_camdata->cam.focal = 350.f;
	io->ioflags = IO_CAMERA;
	io->collision = 0;
	
	return io;
}

static Entity * AddMarker(const res::path & classPath, EntityInstance instance) {
	
	res::path object = classPath + ".teo";
	res::path script = classPath + ".asl";
	
	if(!resources->getFile(("game" / classPath) + ".ftl")
	   && !resources->getFile(object) && !resources->getFile(script)) {
		return NULL;
	}
	
	Entity * io = new Entity(classPath);
	
	if(instance == -1) {
		MakeTemporaryIOIdent(io);
	} else {
		arx_assert(instance > 0);
		io->ident = instance;
	}
	
	GetIOScript(io, script);
	
	io->pos = player.pos;
	io->pos.x -= std::sin(radians(player.angle.getPitch())) * 140.f;
	io->pos.z += std::cos(radians(player.angle.getPitch())) * 140.f;
	io->lastpos = io->initpos = io->pos;
	io->lastpos.x = io->initpos.x = EEfabs(io->initpos.x / 20) * 20.f;
	io->lastpos.z = io->initpos.z = EEfabs(io->initpos.z / 20) * 20.f;
	
	float tempo;
	EERIEPOLY * ep;
	ep = CheckInPoly(io->pos + Vec3f(0.f, player.baseHeight(), 0.f));
	if(ep && GetTruePolyY(ep, io->pos, &tempo)) {
		io->lastpos.y = io->initpos.y = io->pos.y = tempo;
	}
	
	ep = CheckInPoly(io->pos);
	if(ep) {
		io->pos.y = min(ep->v[0].p.y, ep->v[1].p.y);
		io->lastpos.y = io->initpos.y = io->pos.y = min(io->pos.y, ep->v[2].p.y);
	}
	
	io->lastpos.y = io->initpos.y = io->pos.y += player.baseHeight();
	
	io->obj = markerobj;
	io->ioflags = IO_MARKER;
	io->collision = 0;
	
	return io;
}

IO_NPCDATA::IO_NPCDATA() {
	
	lifePool.current = lifePool.max = 20.f;
	manaPool.current = manaPool.max = 0.f;
	
	reachedtime = 0ul;
	reachedtarget = 0l;
	weapon = NULL;
	detect = 0;
	movemode = WALKMODE;
	armor_class = 0.f;
	absorb = 0.f;
	damages = 0.f;
	tohit = 0.f;
	aimtime = 0.f;
	critical = 0.f;
	reach = 0.f;
	backstab_skill = 0.f;
	
	behavior = 0;
	behavior_param = 0.f;
	tactics = 0l;
	xpvalue = 0l;
	cut = 0l;
	
	moveproblem = 0.f;
	weapontype = 0;
	weaponinhand = 0l;
	fightdecision = 0l;
	
	look_around_inc = 0.f;
	collid_time = 0ul;
	collid_state = 0l;
	speakpitch = 1.f;
	lastmouth = 0.f;
	ltemp = 0l;
	
	memset(stacked, 0, sizeof(IO_BEHAVIOR_DATA) * MAX_STACKED_BEHAVIOR); // TODO use constructor
	
	poisonned = 0.f;
	resist_poison = 0;
	resist_magic = 0;
	resist_fire = 0;
	
	strike_time = 0;
	walk_start_time = 0;
	aiming_start = 0l;
	npcflags = 0l;
	memset(&pathfind, 0, sizeof(IO_PATHFIND)); // TODO use constructor
	ex_rotate = 0;
	blood_color = Color::red;
	
	SPLAT_DAMAGES = 0;
	SPLAT_TOT_NB = 0;
	last_splat_pos = Vec3f_ZERO;
	vvpos = 0.f;
	
	climb_count = 0.f;
	stare_factor = 0.f;
	fDetect = 0.f;
	cuts = 0;
	unused = 0;
	
}

IO_NPCDATA::~IO_NPCDATA() {
	delete ex_rotate;
	free(pathfind.list);
}

Entity * AddNPC(const res::path & classPath, EntityInstance instance, AddInteractiveFlags flags) {
	
	res::path object = classPath + ".teo";
	res::path script = classPath + ".asl";
	
	if(!resources->getFile(("game" / classPath) + ".ftl")
	   && !resources->getFile(object) && !resources->getFile(script)) {
		return NULL;
	}
	
	Entity * io = new Entity(classPath);
	
	if(instance == -1) {
		MakeTemporaryIOIdent(io);
	} else {
		arx_assert(instance > 0);
		io->ident = instance;
	}
	
	io->forcedmove = Vec3f_ZERO;
	
	io->_npcdata = new IO_NPCDATA;
	io->ioflags = IO_NPC;
	
	GetIOScript(io, script);
	
	io->spellcast_data.castingspell = SPELL_NONE;
	io->_npcdata->manaPool.current = io->_npcdata->manaPool.max = 10.f;
	io->_npcdata->critical = 5.f;
	io->_npcdata->reach = 20.f;
	io->_npcdata->stare_factor = 1.f;
	
	if(!(flags & NO_ON_LOAD)) {
		SendIOScriptEvent(io, SM_LOAD);
	}
	
	io->pos = player.pos;
	io->pos.x -= std::sin(radians(player.angle.getPitch())) * 140.f;
	io->pos.z += std::cos(radians(player.angle.getPitch())) * 140.f;
	io->lastpos = io->initpos = io->pos;
	io->lastpos.x = io->initpos.x = EEfabs(io->initpos.x / 20) * 20.f;
	io->lastpos.z = io->initpos.z = EEfabs(io->initpos.z / 20) * 20.f;
	
	float tempo;
	EERIEPOLY * ep = CheckInPoly(io->pos + Vec3f(0.f, player.baseHeight(), 0.f));
	if(ep && GetTruePolyY(ep, io->pos, &tempo)) {
		io->lastpos.y = io->initpos.y = io->pos.y = tempo; 
	}
	
	ep = CheckInPoly(io->pos);
	if(ep) {
		io->pos.y = min(ep->v[0].p.y, ep->v[1].p.y);
		io->lastpos.y = io->initpos.y = io->pos.y = min(io->pos.y, ep->v[2].p.y);
	}
	
	if(!io->obj && !(flags & NO_MESH)) {
		io->obj = loadObject(object, false);
	}
	
	io->_npcdata->pathfind.listnb = -1;
	io->_npcdata->behavior = BEHAVIOUR_NONE;
	io->_npcdata->pathfind.truetarget = InvalidEntityHandle;
	
	if(!(flags & NO_MESH) && (flags & IO_IMMEDIATELOAD)) {
		EERIE_COLLISION_Cylinder_Create(io);
	}
	
	io->infracolor = Color3f(1.f, 0.f, 0.2f);
	io->collision = COLLIDE_WITH_PLAYER;
	io->inv = NULL;
	
	ARX_INTERACTIVE_HideGore(io);
	return io;
}

#if BUILD_EDIT_LOADSAVE

/*!
 * \brief Creates an unique identifier for an IO
 * \param io
 */
void MakeIOIdent(Entity * io) {
	if(!io)
		return;
	
	long t = 1;
	
	while(io->ident == 0) {	
		fs::path temp = fs::paths.user / io->instancePath().string();
		
		if(!fs::is_directory(temp)) {
			io->ident = t;
			
			if(fs::create_directories(temp)) {
				LogDirCreation(temp);
				WriteIOInfo(io, temp);
			} else {
				LogError << "Could not create a unique identifier " << temp;
			}
		}

		t++;
	}
}

#endif // BUILD_EDIT_LOADSAVE

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
	
	if(!resources->getFile(("game" / classPath) + ".ftl")
	   && !resources->getFile(object) && !resources->getFile(script)) {
		return NULL;
	}
	
	if(!resources->getFile(res::path(icon).set_ext("bmp"))) {
		return NULL;
	}
	
	Entity * io = new Entity(classPath);
	
	if(instance == -1) {
		MakeTemporaryIOIdent(io);
	} else {
		arx_assert(instance > 0);
		io->ident = instance;
	}
	
	io->ioflags = type;
	io->_itemdata = (IO_ITEMDATA *)malloc(sizeof(IO_ITEMDATA));
	memset(io->_itemdata, 0, sizeof(IO_ITEMDATA));
	io->_itemdata->count = 1;
	io->_itemdata->maxcount = 1;
	io->_itemdata->food_value = 0;
	io->_itemdata->LightValue = -1;

	if(io->ioflags & IO_GOLD)
		io->_itemdata->price = 1;
	else
		io->_itemdata->price = 10;

	io->_itemdata->playerstacksize = 1;

	GetIOScript(io, script);
	
	if(!(flags & NO_ON_LOAD)) {
		SendIOScriptEvent(io, SM_LOAD);
	}
	
	io->spellcast_data.castingspell = SPELL_NONE;
	io->lastpos.x = io->initpos.x = io->pos.x = player.pos.x - std::sin(radians(player.angle.getPitch())) * 140.f;
	io->lastpos.y = io->initpos.y = io->pos.y = player.pos.y;
	io->lastpos.z = io->initpos.z = io->pos.z = player.pos.z + std::cos(radians(player.angle.getPitch())) * 140.f;
	io->lastpos.x = io->initpos.x = (float)((long)(io->initpos.x / 20)) * 20.f;
	io->lastpos.z = io->initpos.z = (float)((long)(io->initpos.z / 20)) * 20.f;

	EERIEPOLY * ep;
	ep = CheckInPoly(io->pos + Vec3f(0.f, -60.f, 0.f));

	if(ep) {
		float tempo;

		if(GetTruePolyY(ep, io->pos, &tempo))
			io->lastpos.y = io->initpos.y = io->pos.y = tempo; 
	}

	ep = CheckInPoly(io->pos);

	if(ep) {
		io->pos.y = min(ep->v[0].p.y, ep->v[1].p.y);
		io->lastpos.y = io->initpos.y = io->pos.y = min(io->pos.y, ep->v[2].p.y);
	}

	if(io->ioflags & IO_GOLD) {
		io->obj = GoldCoinsObj[0];
	}
	
	if(!io->obj && !(flags & NO_MESH)) {
		io->obj = loadObject(object);
	}
	
	TextureContainer * tc;
	if (io->ioflags & IO_MOVABLE) {
		tc = Movable;
	} else if (io->ioflags & IO_GOLD) {
		tc = GoldCoinsTC[0];
	} else {
		tc = TextureContainer::LoadUI(icon, TextureContainer::Level);
	}

	if(!tc) {
		tc = TextureContainer::LoadUI("graph/interface/misc/default[icon]");
	}
	
	if(tc) {
		unsigned long w = tc->m_dwWidth >> 5;
		unsigned long h = tc->m_dwHeight >> 5;

		if ((w << 5) != tc->m_dwWidth)
			io->sizex = (char)(w + 1);
		else
			io->sizex = (char)(w);

		if ((h << 5) != tc->m_dwHeight)
			io->sizey = (char)(h + 1);
		else
			io->sizey = (char)(h);

		io->sizex = clamp(io->sizex, 1, 3);
		io->sizey = clamp(io->sizey, 1, 3);

		io->inv = tc;
	}

	io->infracolor.r = 0.2f;
	io->infracolor.g = 0.2f;
	io->infracolor.b = 1.f;
	io->collision = 0;

	return io;
}

/*!
 * \brief Returns nearest interactive object found at position x, y
 * \param pos
 * \param flag
 * \param _pRef
 * \param _pTable
 * \param _pnNbInTable
 * \return
 */
Entity * GetFirstInterAtPos(const Vec2s & pos, long flag, Vec3f * _pRef, Entity ** _pTable, int * _pnNbInTable)
{
	float n;
 
	float _fdist = 9999999999.f;
	float fdistBB = 9999999999.f;
	float fMaxDist = flag ? 9999999999.f : 350;
	Entity * foundBB = NULL;
	Entity * foundPixel = NULL;
	bool bPlayerEquiped = false;

	if(Project.telekinesis) {
		fMaxDist = 850;
	}

	int nStart = 1;
	int nEnd = entities.size();

	if(flag == 3 && _pTable && _pnNbInTable) {
		nStart = 0;
		nEnd = *_pnNbInTable;
	}

	for(long i = nStart; i < nEnd; i++) {
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

		if(!(io->gameFlags & GFLAG_INTERACTIVITY))
			continue;

		// Is Object in TreatZone ??
		bPlayerEquiped = IsEquipedByPlayer(io);

		if( !((bPlayerEquiped  && (player.Interface & INTER_MAP)) || (io->gameFlags & GFLAG_ISINTREATZONE)) )
			continue;

		// Is Object Displayed on screen ???
		if( !((io->show == SHOW_FLAG_IN_SCENE) ||
			  (bPlayerEquiped && flag) ||
			  (bPlayerEquiped && (player.Interface & INTER_MAP) && (Book_Mode == BOOKMODE_STATS))) )
			//((io->show==9) && (player.Interface & INTER_MAP)) )
		{
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
			float flDistanceToRef = glm::distance2(ACTIVECAM->orgTrans.pos, *_pRef);
			float flDistanceToIO = glm::distance2(ACTIVECAM->orgTrans.pos, io->pos);
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

				if(io->animlayer[0].cur_anim != NULL) {
					n = CEDRIC_PtIn2DPolyProjV2(io->obj, &io->obj->facelist[j] , pos.x, pos.y);
				} else {
					n = PtIn2DPolyProj(io->obj, &io->obj->facelist[j] , pos.x, pos.y);
				}

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

	for(long i = 0; i < MAX_EQUIPED; i++) {
		if(player.equiped[i] != PlayerEntityHandle && player.equiped[i] == num)
			return true;
	}

	return false;
}

extern long LOOKING_FOR_SPELL_TARGET;
Entity * InterClick(const Vec2s & pos) {
	
	if(IsFlyingOverInventory(pos)) {
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
		} else if(Project.telekinesis) {
			return io;
		} else if(IsEquipedByPlayer(io) || closerThan(player.pos, io->pos, dist_Threshold)) {
			return io;
		}
	}

	return NULL;
}

// Need To upgrade to a more precise collision.
long IsCollidingAnyInter(const Vec3f & pos, Vec3f * size) {
	
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
				return i;

			tempPos.y += size->y;

			if(IsCollidingInter(io, tempPos))
				return i;
		}
	}

	return -1;
}

//*************************************************************************************
// To upgrade to a more precise collision.
//*************************************************************************************
static bool IsCollidingInter(Entity * io, const Vec3f & pos) {
	
	if(!io || !io->obj)
		return false;

	if(closerThan(pos, io->pos, 190.f)) {
		
		vector<EERIE_VERTEX> & vlist = io->obj->vertexlist3;

		if(io->obj->grouplist.size() > 4) {
			for(size_t i = 0; i < io->obj->grouplist.size(); i++) {
				long idx = io->obj->grouplist[i].origin;

				if(!fartherThan(pos, vlist[idx].v, 50.f)) {
					return true;
				}
			}
		} else {
			long nbv = io->obj->vertexlist3.size();
			for(long i = 0; i < nbv; i++) {
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
	io->sfx_time = (unsigned long)(arxtime);
}

bool ARX_INTERACTIVE_CheckFULLCollision(EERIE_3DOBJ * obj, EntityHandle source)
{
	bool col = false;
	EntityHandle avoid = InvalidEntityHandle;
	Entity * io_source = NULL;

	if(ValidIONum(source)) {
		io_source = entities[source];
		avoid = io_source->no_collide;
	}

	for(long i = 0; i < TREATZONE_CUR; i++) {

		if(treatio[i].show != SHOW_FLAG_IN_SCENE || (treatio[i].ioflags & IO_NO_COLLISIONS))
			continue;

		Entity * io = treatio[i].io;

		if(!io
		   || io == io_source
		   || !io->obj
		   || io == entities.player()
		   || treatio[i].num == avoid
		   || (io->ioflags & (IO_CAMERA | IO_MARKER | IO_ITEM))
		   || io->usepath
		   || ((io->ioflags & IO_NPC) && io_source && (io_source->ioflags & IO_NO_NPC_COLLIDE))
		   || !closerThan(io->pos, obj->pbox->vert[0].pos, 600.f)
		   || !In3DBBoxTolerance(obj->pbox->vert[0].pos, io->bbox3D, obj->pbox->radius)
		) {
			continue;
		}

		if((io->ioflags & IO_NPC) && io->_npcdata->lifePool.current > 0.f) {
			for(long kk = 0; kk < obj->pbox->nb_physvert; kk++)
				if(PointInCylinder(io->physics.cyl, &obj->pbox->vert[kk].pos))
					return true;
		} else if(io->ioflags & IO_FIX) {
			long step;
			long nbv = io->obj->vertexlist.size();
			Sphere sp;
			sp.radius = 28.f;

			if(nbv < 500) {
				step = 1;
				sp.radius = 36.f;
			}
			else if(nbv < 900)
				step = 2;
			else if(nbv < 1500)
				step = 4;
			else
				step = 6;

			vector<EERIE_VERTEX> & vlist = io->obj->vertexlist3;

			if(io->gameFlags & GFLAG_PLATFORM) {
				for(long kk = 0; kk < obj->pbox->nb_physvert; kk++) {
					Sphere sphere;
					sphere.origin = obj->pbox->vert[kk].pos;
					sphere.radius = 30.f;
					float miny, maxy;
					miny = io->bbox3D.min.y;
					maxy = io->bbox3D.max.y;

					if(maxy <= sphere.origin.y + sphere.radius || miny >= sphere.origin.y) {
						if(In3DBBoxTolerance(sphere.origin, io->bbox3D, sphere.radius)) {
							// TODO why ignore the z components?
							if(closerThan(Vec2f(io->pos.x, io->pos.z), Vec2f(sphere.origin.x, sphere.origin.z), 440.f + sphere.radius)) {

								EERIEPOLY ep;
								ep.type = 0;

								for(size_t ii = 0; ii < io->obj->facelist.size(); ii++) {
									float cx = 0;
									float cz = 0;

									for(long idx = 0 ; idx < 3 ; idx++) {
										ep.v[idx].p = io->obj->vertexlist3[io->obj->facelist[ii].vid[idx]].v;

										cx += ep.v[idx].p.x;
										cz += ep.v[idx].p.z;
									}

									cx *= ( 1.0f / 3 );
									cz *= ( 1.0f / 3 );

									for(kk = 0; kk < 3; kk++) {
										ep.v[kk].p.x = (ep.v[kk].p.x - cx) * 3.5f + cx;
										ep.v[kk].p.z = (ep.v[kk].p.z - cz) * 3.5f + cz;
									}

									if(PointIn2DPolyXZ(&ep, sphere.origin.x, sphere.origin.z))
										return true;
								}
							}
						}
					}
				}
			}


			for(long ii = 1; ii < nbv; ii += step) {
				if(ii != io->obj->origin) {
					sp.origin = vlist[ii].v;

					for(long kk = 0; kk < obj->pbox->nb_physvert; kk++) {
						if(sp.contains(obj->pbox->vert[kk].pos)) {
							if(io_source && (io->gameFlags & GFLAG_DOOR)) {
								if(float(arxtime) > io->collide_door_time + 500) {
									EVENT_SENDER = io_source;
									io->collide_door_time = (unsigned long)(arxtime);
									SendIOScriptEvent(io, SM_COLLIDE_DOOR);
									EVENT_SENDER = io;
									io->collide_door_time = (unsigned long)(arxtime);
									SendIOScriptEvent(io_source, SM_COLLIDE_DOOR);
								}
							}
							return true;
						}
					}
				}
			}
		}
	}

	return col;
}

void UpdateCameras() {
	
	arxtime.get_updated();
	
	for(size_t i = 1; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * io = entities[handle];
		
		if(!io)
			continue;

		// interpolate & send events
		if(io->usepath) {
			ARX_USE_PATH * aup = io->usepath;
			float diff = float(arxtime) - aup->_curtime;

			if(aup->aupflags & ARX_USEPATH_FORWARD) {
				if(aup->aupflags & ARX_USEPATH_FLAG_FINISHED) {
				} else {
					aup->_curtime += diff;
				}
			}

			if(aup->aupflags & ARX_USEPATH_BACKWARD) {
				aup->_starttime += diff * 2;
				aup->_curtime += diff;

				if(aup->_starttime >= aup->_curtime)
					aup->_curtime = aup->_starttime + 1;
			}

			if(aup->aupflags & ARX_USEPATH_PAUSE) {
				aup->_starttime += diff;
				aup->_curtime += diff;
			}

			long last = ARX_PATHS_Interpolate(aup, &io->pos);

			if(aup->lastWP != last) {
				if(last == -2) {
					char str[16];
					sprintf(str, "%ld", aup->path->nb_pathways - 1);
					EVENT_SENDER = NULL;
					SendIOScriptEvent(io, SM_WAYPOINT, str);
					sprintf(str, "waypoint%ld", aup->path->nb_pathways - 1);
					SendIOScriptEvent(io, SM_NULL, "", str);
					SendIOScriptEvent(io, SM_PATHEND);
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

					char str[16];
					sprintf(str, "%ld", ii);
					EVENT_SENDER = NULL;
					SendIOScriptEvent(io, SM_WAYPOINT, str);
					sprintf(str, "waypoint%ld", ii);
					SendIOScriptEvent(io, SM_NULL, "", str);

					if(ii == aup->path->nb_pathways) {
						SendIOScriptEvent(io, SM_PATHEND);
					}

					aup->lastWP = last + 1;
				}
			}

			if(io->damager_damages > 0 && io->show == SHOW_FLAG_IN_SCENE) {
				for(size_t ii = 0; ii < entities.size(); ii++) {
					const EntityHandle handle = EntityHandle(ii);
					Entity * ioo = entities[handle];

					if(ioo
					   && ii != i
					   && ioo->show == SHOW_FLAG_IN_SCENE
					   && (ioo->ioflags & IO_NPC)
					   && closerThan(io->pos, ioo->pos, 600.f)
					) {
						bool Touched = false;

						for(size_t ri = 0; ri < io->obj->vertexlist.size(); ri += 3) {
							for(size_t rii = 0; rii < ioo->obj->vertexlist.size(); rii += 3) {
								if(closerThan(io->obj->vertexlist3[ri].v, ioo->obj->vertexlist3[rii].v, 20.f)) {
									Touched = true;
									ri = io->obj->vertexlist.size();
									break;
								}
							}
						}

						if(Touched)
							ARX_DAMAGES_DealDamages(EntityHandle(ii), io->damager_damages, EntityHandle(i), io->damager_type, &ioo->pos);
					}
				}
			}
		}

		if(io->ioflags & IO_CAMERA) {

			io->_camdata->cam.orgTrans.pos = io->pos;

			if(io->targetinfo != EntityHandle(TARGET_NONE)) {
				// Follows target
				GetTargetPos(io, (unsigned long)io->_camdata->cam.smoothing);
				io->target += io->_camdata->cam.translatetarget;

				if(io->_camdata->cam.lastinfovalid && io->_camdata->cam.smoothing != 0.f) {

					float vv = io->_camdata->cam.smoothing;

					if(vv > 8000)
						vv = 8000;

					vv = (8000 - vv) * ( 1.0f / 4000 );

					float f1 = framedelay * ( 1.0f / 1000 ) * vv;

					if(f1 > 1.f)
						f1 = 1.f;

					float f2 = 1.f - f1;
					Vec3f smoothtarget = io->target * f2 + io->_camdata->cam.lasttarget * f1;

					io->_camdata->cam.setTargetCamera(smoothtarget);
					io->_camdata->cam.lasttarget = smoothtarget;
				} else {
					io->_camdata->cam.setTargetCamera(io->target);
					io->_camdata->cam.lasttarget = io->target;
				}

				io->_camdata->cam.angle.setPitch(io->_camdata->cam.angle.getPitch() - 180.f);
				io->_camdata->cam.angle.setYaw(-io->_camdata->cam.angle.getYaw());
				io->angle.setYaw(0.f);
				io->angle.setPitch(io->_camdata->cam.angle.getPitch() + 90.f);
				io->angle.setRoll(0.f);
			} else {
				// no target...
				float tr = radians(MAKEANGLE(io->angle.getPitch() + 90));
				io->target.x = io->pos.x - std::sin(tr) * 20.f;
				io->target.y = io->pos.y;
				io->target.z = io->pos.z + std::cos(tr) * 20.f;
				io->_camdata->cam.setTargetCamera(io->target);
				io->_camdata->cam.lasttarget = io->target;
			}

			io->_camdata->cam.lastinfovalid = true;
			io->_camdata->cam.lastpos = io->_camdata->cam.orgTrans.pos;
		}
	}
}

void UpdateIOInvisibility(Entity * io)
{
	if(io && io->invisibility <= 1.f) {
		if((io->gameFlags & GFLAG_INVISIBILITY) && io->invisibility < 1.f) {
			io->invisibility += framedelay * ( 1.0f / 1000 );
		} else if(!(io->gameFlags & GFLAG_INVISIBILITY) && io->invisibility != 0.f) {
			io->invisibility -= framedelay * ( 1.0f / 1000 );
		}
		
		io->invisibility = clamp(io->invisibility, 0.f, 1.f);
	}
}

glm::mat4x4 convertToMatrixForDrawEERIEInter(const PHYSICS_BOX_DATA &box) {
	Vec3f tmp = box.vert[14].pos - box.vert[13].pos;
	Vec3f up  = box.vert[2].pos  - box.vert[1].pos;
	up += box.vert[3].pos  - box.vert[4].pos;
	up += box.vert[10].pos - box.vert[9].pos;
	up += box.vert[11].pos - box.vert[12].pos;
	up *= 0.25f;

	glm::mat4x4 mat;
	MatrixSetByVectors(mat, up, tmp);
	mat[0][3] = mat[1][3] = mat[2][3] = 0.f;
	mat[3][0] = mat[3][1] = mat[3][2] = mat[3][3] = 0.f;

	return mat;
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
			temp.setPitch(MAKEANGLE(180.f - temp.getPitch()));
		} else {
			temp.setPitch(MAKEANGLE(270.f - temp.getPitch()));
		}

		if(io->animlayer[0].cur_anim) {

			io->bbox2D.min.x = 9999;
			io->bbox2D.max.x = -1;

			long diff;
			if(io->animlayer[0].flags & EA_PAUSED)
				diff = 0;
			else
				diff = static_cast<long>(framedelay);

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

	for(size_t i = 1; i < entities.size(); i++) { // Player isn't rendered here...
		const EntityHandle handle = EntityHandle(i);
		Entity * io = entities[handle];

		if(!io || io == DRAGINTER || !(io->gameFlags & GFLAG_ISINTREATZONE))
			continue;

		if(io->show != SHOW_FLAG_IN_SCENE) {
			continue;
		}

		if((io->ioflags & IO_CAMERA) || (io->ioflags & IO_MARKER)) {
			continue;
		}

		Anglef temp = io->angle;

		if(io->ioflags & IO_NPC) {
			temp.setPitch(MAKEANGLE(180.f - temp.getPitch()));
		} else {
			temp.setPitch(MAKEANGLE(270.f - temp.getPitch()));
		}

		if(io->animlayer[0].cur_anim) {

			Vec3f pos = io->pos;

			if(io->ioflags & IO_NPC) {
				pos.y = io->_npcdata->vvpos;
			}

			float invisibility = Cedric_GetInvisibility(io);

			EERIEDrawAnimQuatRender(io->obj, pos, io, invisibility);
		} else {
			io->bbox2D.min.x = 9999;
			io->bbox2D.max.x = -1;

			if(io->obj) {
				UpdateGoldObject(io);
			}

			if(!(io->ioflags & IO_NPC)) {
				if(io->obj) {

					if(io->obj->pbox && io->obj->pbox->active) {
						glm::mat4x4 mat = convertToMatrixForDrawEERIEInter(*io->obj->pbox);
						glm::quat rotation = glm::toQuat(mat);
						
						arx_assert(io->obj->point0 == Vec3f_ZERO);

						TransformInfo t(io->pos, rotation, io->scale, io->obj->pbox->vert[0].initpos);

						float invisibility = Cedric_GetInvisibility(io);

						DrawEERIEInter(io->obj, t, io, false, invisibility);
					} else {
						glm::quat rotation = glm::toQuat(toRotationMatrix(temp));
						
						TransformInfo t(io->pos, rotation, io->scale);

						float invisibility = Cedric_GetInvisibility(io);

						DrawEERIEInter(io->obj, t, io, false, invisibility);
					}
				}
			}
		}
	}
}

static std::vector<Entity *> toDestroy;

void ARX_INTERACTIVE_DestroyIOdelayed(Entity * entity) {
	LogDebug("will destroy entity " << entity->idString());
	if(std::find(toDestroy.begin(), toDestroy.end(), entity) == toDestroy.end()) {
		toDestroy.push_back(entity);
	}
}

void ARX_INTERACTIVE_DestroyIOdelayedExecute() {
	if(!toDestroy.empty()) {
		LogDebug("executing delayed entity destruction");
	}
	for(std::vector<Entity *>::iterator it = toDestroy.begin(); it != toDestroy.end(); ++it) {
		if(*it) {
			(*it)->destroyOne();
		}
	}
	toDestroy.clear();
}

bool IsSameObject(Entity * io, Entity * ioo)
{
	if(!io || !ioo
			|| io->classPath() != ioo->classPath()
			|| (io->ioflags & IO_UNIQUE)
			|| io->durability != ioo->durability
			|| io->max_durability != ioo->max_durability)
		return false;

	if	((io->ioflags & IO_ITEM)
	        &&	(ioo->ioflags & IO_ITEM)
	        &&	(io->over_script.data == NULL)
	        &&	(ioo->over_script.data == NULL)) {
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
	if(ValidIONum(t)) {
		Entity * io = entities[t];

		if(io == DRAGINTER || (io->show != SHOW_FLAG_IN_SCENE))
			return;

		float yy;
		EERIEPOLY * ep = CheckInPoly(io->pos, &yy);

		if(ep && (yy - io->pos.y < 10.f))
			return;

		io->obj->pbox->active = 1;
		io->obj->pbox->stopcount = 0;
		io->velocity = Vec3f_ZERO;
		io->stopped = 1;
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
	else if(boost::contains(origin, "earth")) return "earth";
	else if(boost::contains(origin, "ice")) return "ice";
	else if(boost::contains(origin, "fabric")) return "carpet";
	else if(boost::contains(origin, "moss")) return "carpet";
	else return "unknown";
}

void UpdateGoldObject(Entity * io) {
	if(io->ioflags & IO_GOLD) {
		long num=0;

		if (io->_itemdata->price<=3)
			num=io->_itemdata->price-1;
		else if (io->_itemdata->price<=8)
			num=3;
		else if (io->_itemdata->price<=20)
			num=4;
		else if (io->_itemdata->price<=50)
			num=5;
		else
			num=6;

		io->obj=GoldCoinsObj[num];
		io->inv=GoldCoinsTC[num];
	}
}
