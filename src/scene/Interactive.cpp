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

#include "scene/Interactive.h"

#include <cstdlib>
#include <iomanip>
#include <algorithm>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>

#include <boost/algorithm/string/predicate.hpp>

#include "ai/Paths.h"

#include "animation/Animation.h"

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
using std::string;
using std::vector;

extern long FRAME_COUNT;

extern Entity * CAMERACONTROLLER;
extern TextureContainer * Movable;
extern long EXTERNALVIEW;

ARX_NODES nodes;
static float TREATZONE_LIMIT = 1800.f;
 
long HERO_SHOW_1ST = 1;
#ifdef BUILD_EDITOR
long NbIOSelected = 0;
#endif
long INTER_DRAW = 0;

static bool IsCollidingInter(Entity * io, Vec3f * pos);
static Entity * AddCamera(const res::path & classPath,
                          EntityInstance instance = -1);
static Entity * AddMarker(const res::path & classPath,
                          EntityInstance instance = -1);

float STARTED_ANGLE = 0;
void Set_DragInter(Entity * io)
{
	if (io != DRAGINTER)
		STARTED_ANGLE = player.angle.b;

	DRAGINTER = io;

	if (io)
	{
		if ((io->obj) && (io->obj->pbox))
		{
			io->obj->pbox->active = 0;
		}
	}
}

// Checks if an IO index number is valid
long ValidIONum(long num) {
	
	if(num < 0 || num >= long(entities.size()) || !entities[num]) {
		return 0;
	}
	
	return 1;
}

long ValidIOAddress(const Entity * io) {
	
	if(!io) {
		return 0;
	}
	
	for(size_t i = 0; i < entities.size(); i++) {
		if(entities[i] == io) {
			return 1;
		}
	}
	
	return 0;
}

static float ARX_INTERACTIVE_fGetPrice(Entity * io, Entity * shop) {
	
	if ((!io)
	        ||	(!(io->ioflags & IO_ITEM)))
		return 0;

	float durability_ratio = io->durability / io->max_durability;
	float shop_multiply = 1.f;

	if (shop)
		shop_multiply = shop->shop_multiply;

	return io->_itemdata->price * shop_multiply * durability_ratio;

}
long ARX_INTERACTIVE_GetPrice(Entity * io, Entity * shop) {
	return ARX_INTERACTIVE_fGetPrice(io, shop);
}

static void ARX_INTERACTIVE_ForceIOLeaveZone(Entity * io, long flags) {
	
	ARX_PATH * op = io->inzone;

	if (op)
	{
		std::string temp = op->name;

		if (flags & 1) // no need when being destroyed !
			SendIOScriptEvent(io, SM_LEAVEZONE, temp);

		if(!op->controled.empty())
		{
			long t = entities.getById(op->controled);

			if (t >= 0)
			{
				std::string str = io->long_name() + ' ' + temp;
				SendIOScriptEvent( entities[t], SM_CONTROLLEDZONE_LEAVE, str ); 
			}
		}
	}
}

void ARX_INTERACTIVE_DestroyDynamicInfo(Entity * io)
{
	if (!io) return;

	long n = io->index();

	short sN = checked_range_cast<short>(n);

	ARX_INTERACTIVE_ForceIOLeaveZone(io, 0);

	for (long i = 0; i < MAX_EQUIPED; i++)
	{
		if ((player.equiped[i])
		        &&	(player.equiped[i] == n)
		        &&	ValidIONum(player.equiped[i]))
		{
			ARX_EQUIPMENT_UnEquip(entities.player(), entities[player.equiped[i]], 1);
			player.equiped[i] = 0;
		}
	}
	
	ARX_SPEECH_ReleaseIOSpeech(io);
	
	ARX_SCRIPT_EventStackClearForIo(io);
	
	if (ValidIONum(n))
	{
		for (size_t i = 0; i < MAX_SPELLS; i++)
		{
			if ((spells[i].exist) && (spells[i].caster == n))
			{
				spells[i].tolive = 0;
			}
		}
	}

	if (io->flarecount)
	{
		for (long i = 0; i < MAX_FLARES; i++)
		{
			if ((flare[i].exist)
			        &&	(flare[i].io == io))
				flare[i].io = NULL;
		}
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
	
	if (ValidDynLight(io->dynlight))
	{
		DynLight[io->dynlight].exist = 0;
	}

	io->dynlight = -1;

	if (io->obj)
	{
		EERIE_3DOBJ * eobj = io->obj;

		for (long k = 0; k < eobj->nblinked; k++)
		{
			if ((eobj->linked[k].lgroup != -1) && eobj->linked[k].obj)
			{
				Entity * ioo = (Entity *)eobj->linked[k].io;

				if ((ioo) && ValidIOAddress(ioo))
				{
					long ll = eobj->linked[k].lidx;
					Vec3f pos, vector;
					pos = io->obj->vertexlist3[ll].v;
					ioo->angle = Anglef(rnd() * 40.f + 340.f, rnd() * 360.f, 0.f);
					vector.x = -(float)EEsin(radians(ioo->angle.b)) * ( 1.0f / 2 );
					vector.y = EEsin(radians(ioo->angle.a));
					vector.z = (float)EEcos(radians(ioo->angle.b)) * ( 1.0f / 2 );
					ioo->soundtime = 0;
					ioo->soundcount = 0;
					EERIE_PHYSICS_BOX_Launch_NOCOL(ioo, ioo->obj, &pos, &vector, 2, &ioo->angle);
					ioo->show = SHOW_FLAG_IN_SCENE;
					ioo->no_collide = sN;
					EERIE_LINKEDOBJ_UnLinkObjectFromObject(io->obj, ioo->obj);
				}
			}
		}
	}
}


bool ARX_INTERACTIVE_Attach(long n_source, long n_target, const std::string& ap_source, const std::string& ap_target)
{
	if (!ValidIONum(n_source) || !ValidIONum(n_target))
		return false;

	entities[n_source]->show = SHOW_FLAG_LINKED;
	EERIE_LINKEDOBJ_UnLinkObjectFromObject(entities[n_target]->obj, entities[n_source]->obj);
	return EERIE_LINKEDOBJ_LinkObjectToObject(entities[n_target]->obj,
	        entities[n_source]->obj, ap_target, ap_source, entities[n_source]);
}
void ARX_INTERACTIVE_Detach(long n_source, long n_target)
{
	if (!ValidIONum(n_source)
	        ||	!ValidIONum(n_target))
		return;

	entities[n_source]->show = SHOW_FLAG_IN_SCENE;
	EERIE_LINKEDOBJ_UnLinkObjectFromObject(entities[n_target]->obj, entities[n_source]->obj);
}

void ARX_INTERACTIVE_Show_Hide_1st(Entity * io, long state)
{
	if ((!io)
	        ||	(HERO_SHOW_1ST == state))
		return;

	HERO_SHOW_1ST = state;
	long grp = EERIE_OBJECT_GetSelection(io->obj, "1st");

	if (grp != -1)
	{
		for (size_t nn = 0; nn < io->obj->facelist.size(); nn++)
		{
			EERIE_FACE * ef = &io->obj->facelist[nn];

			for (long jj = 0; jj < 3; jj++)
			{
				if (IsInSelection(io->obj, ef->vid[jj], grp) != -1)
				{
					if (state)
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

	for (size_t nn = 0; nn < io->obj->texturecontainer.size(); nn++)
	{
		if (io->obj->texturecontainer[nn] && ( io->obj->texturecontainer[nn]->m_texName.string().find("gore") != std::string::npos ) )
		{
			gorenum = nn;
			break;
		}
	}

	if (gorenum > -1)
	{
		for (size_t nn = 0; nn < io->obj->facelist.size(); nn++)
		{
			if (io->obj->facelist[nn].texid == gorenum)
			{
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

	if ((io == entities.player()) && (!flag & 1))
		return;

	long gorenum = -1;

	for (size_t nn = 0; nn < io->obj->texturecontainer.size(); nn++)
	{
		if (io->obj->texturecontainer[nn] && io->obj->texturecontainer[nn]->m_texName.string().find("gore") != string::npos)
		{
			gorenum = nn;
			break;
		}
	}

	if (gorenum > -1)
	{
		for (size_t nn = 0; nn < io->obj->facelist.size(); nn++)
		{
			//Hide Gore Polys...
			if (io->obj->facelist[nn].texid == gorenum)
				io->obj->facelist[nn].facetype |= POLY_HIDE;
			else if (!flag & 1)
				io->obj->facelist[nn].facetype &= ~POLY_HIDE;
		}
	}
}


bool ForceNPC_Above_Ground(Entity * io) {
	
	if(io && (io->ioflags & IO_NPC) && !(io->ioflags & IO_PHYSICAL_OFF)) {
		io->physics.cyl.origin = io->pos;
		AttemptValidCylinderPos(&io->physics.cyl, io, CFLAG_NO_INTERCOL);
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
		if(entities[i]) {
			EERIE_LINKEDOBJ_ReleaseData(entities[i]->obj);
		}
	}
}

void IO_UnlinkAllLinkedObjects(Entity * io) {
	if(io && io->obj) {
		for(long k = 0; k < io->obj->nblinked; k++) {
			if(io->obj->linked[k].io) {
				Entity * ioo = io->obj->linked[k].io;
				if(ValidIOAddress(ioo)) {
					IO_Drop_Item(io, ioo);
				}
			}
		}
		EERIE_LINKEDOBJ_ReleaseData(io->obj);
	}
}

// First is always the player
TREATZONE_IO * treatio = NULL;
long TREATZONE_CUR = 0;
static long TREATZONE_MAX = 0;

void TREATZONE_Clear() {
	TREATZONE_CUR = 0;
}

void TREATZONE_Release() {
	free(treatio), treatio = NULL;
	TREATZONE_MAX = 0;
	TREATZONE_CUR = 0;
}

void TREATZONE_RemoveIO(Entity * io)
{
	if (treatio)
	{
		for (long i = 0; i < TREATZONE_CUR; i++)
		{
			if (treatio[i].io == io)
			{
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
	if (TREATZONE_MAX == TREATZONE_CUR)
	{
		TREATZONE_MAX++;
		treatio = (TREATZONE_IO *)realloc(treatio, sizeof(TREATZONE_IO) * TREATZONE_MAX);
	}

	for (long i = 0; i < TREATZONE_CUR; i++)
	{
		if (treatio[i].io == io)
			return;
	}

	treatio[TREATZONE_CUR].io = io;
	treatio[TREATZONE_CUR].ioflags = io->ioflags;

	if (flag & 1) treatio[TREATZONE_CUR].ioflags |= IO_JUST_COLLIDE;

	treatio[TREATZONE_CUR].show = io->show;
	treatio[TREATZONE_CUR].num = io->index();
	TREATZONE_CUR++;
}

void CheckSetAnimOutOfTreatZone(Entity * io, long num)
{
	if ((io)
	        &&	(io->animlayer[num].cur_anim)
	        &&	!(io->gameFlags & GFLAG_ISINTREATZONE)
			&&	distSqr(io->pos, ACTIVECAM->orgTrans.pos) > square(2500.f))
	{

		io->animlayer[num].ctime =
		    checked_range_cast<long>(io->animlayer[num].cur_anim->anims[io->animlayer[num].altidx_cur]->anim_time - 1.f);

	}
}

long GLOBAL_Player_Room = -1;
void PrepareIOTreatZone(long flag)
{
	static long status = -1;
	static Vec3f lastpos;

	if ((flag)
	        ||	(status == -1))
	{
		status = 0;
		lastpos = ACTIVECAM->orgTrans.pos;
	}
	else if (status == 3) status = 0;

	if (distSqr(ACTIVECAM->orgTrans.pos, lastpos) > square(100.f))
	{
		status = 0;
		lastpos = ACTIVECAM->orgTrans.pos;
	}

	if (status++) return;

	TREATZONE_Clear();
	long Cam_Room = ARX_PORTALS_GetRoomNumForPosition(&ACTIVECAM->orgTrans.pos, 1);
	GLOBAL_Player_Room = ARX_PORTALS_GetRoomNumForPosition(&player.pos, 1);
	TREATZONE_AddIO(entities.player());

	short sGlobalPlayerRoom = checked_range_cast<short>(GLOBAL_Player_Room);

	for (long i = 0; i < MAX_EQUIPED; i++)
	{
		if ((player.equiped[i] != 0)
		        &&	ValidIONum(player.equiped[i]))
		{
			Entity * toequip = entities[player.equiped[i]];

			if (toequip)
			{
				toequip->room = sGlobalPlayerRoom;
				toequip->room_flags = 0;
			}
		}
	}

	if (DRAGINTER) TREATZONE_AddIO(DRAGINTER);

	TREATZONE_LIMIT = 3200; 

	if (RoomDistance)
	{
		TREATZONE_LIMIT += 600; 

		if (CURRENTLEVEL == 4)
			TREATZONE_LIMIT += 1200;

		if (ACTIVECAM->cdepth > 3000)
			TREATZONE_LIMIT += 500;

		if (ACTIVECAM->cdepth > 4000)
			TREATZONE_LIMIT += 500;

		if (ACTIVECAM->cdepth > 6000)
			TREATZONE_LIMIT += 500;
	}
	
	char treat;
	for(size_t i = 1; i < entities.size(); i++) {
		Entity * io = entities[i];

		if ((io)
		        &&	((io->show == SHOW_FLAG_IN_SCENE)
		             ||	(io->show == SHOW_FLAG_TELEPORTING)
		             ||	(io->show == SHOW_FLAG_ON_PLAYER)
		             ||	(io->show == SHOW_FLAG_HIDDEN)))   
		{
			if ((io->ioflags & IO_CAMERA) && (!EDITMODE))
				treat = 0;
			else if ((io->ioflags & IO_MARKER) && (!EDITMODE))
				treat = 0;
			else if ((io->ioflags & IO_NPC) 
			         && (io->_npcdata->pathfind.flags & PATHFIND_ALWAYS))
			{
				treat = 1;
			}
			else
			{
				float dists;

				if (Cam_Room >= 0)
				{
					if (io->show == SHOW_FLAG_TELEPORTING)
					{
						Vec3f pos;
						GetItemWorldPosition(io, &pos);
						dists = distSqr(ACTIVECAM->orgTrans.pos, pos);
					}
					else
					{
						if (io->room_flags & 1)
							UpdateIORoom(io);

						dists = square(SP_GetRoomDist(&io->pos, &ACTIVECAM->orgTrans.pos, io->room, Cam_Room));
					}
				}
				else
				{
					if (io->show == SHOW_FLAG_TELEPORTING)
					{
						Vec3f pos;
						GetItemWorldPosition(io, &pos);
						dists = distSqr(ACTIVECAM->orgTrans.pos, pos); //&io->pos,&pos);
					}
					else
						dists = distSqr(io->pos, ACTIVECAM->orgTrans.pos);
				}
		
				if (dists < square(TREATZONE_LIMIT)) treat = 1;
				else treat = 0;
				
			}

			if (!treat)
			{
				if (io == CAMERACONTROLLER)
					treat = 1;

				if (io == DRAGINTER)
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

				if (SendIOScriptEvent(io, SM_TREATOUT) != REFUSE)
				{
					if (io->ioflags & IO_NPC)
						io->_npcdata->pathfind.flags &= ~PATHFIND_ALWAYS;

					io->gameFlags &= ~GFLAG_ISINTREATZONE;
				}
			}
		}
	}

	long M_TREAT = TREATZONE_CUR;

	for(size_t i = 1; i < entities.size(); i++) {
		Entity * io = entities[i];

		if ((io != NULL)
		        &&	!(io->gameFlags & GFLAG_ISINTREATZONE)
		        && ((io->show == SHOW_FLAG_IN_SCENE)
		            ||	(io->show == SHOW_FLAG_TELEPORTING)
		            ||	(io->show == SHOW_FLAG_ON_PLAYER)
		            ||	(io->show == SHOW_FLAG_HIDDEN)))   // show 5 = ininventory; 15 = destroyed
		{
			if ((io->ioflags & IO_CAMERA)
			        ||	(io->ioflags & IO_ITEM)
			        ||	(io->ioflags & IO_MARKER))
				continue;

			long toadd = 0;

			for (long ii = 1; ii < M_TREAT; ii++)
			{
				Entity * ioo = treatio[ii].io;

				if (ioo)
				{
					if (distSqr(io->pos, ioo->pos) < square(300.f))
					{
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

//*************************************************************************************
//*************************************************************************************
long GetNumNodeByName(char * name)
{
	for (long i = 0; i < nodes.nbmax; i++)
	{
		if ((nodes.nodes[i].exist)
		        &&	(!strcmp(name, nodes.nodes[i].name)))
			return i;
	}

	return -1;
}

void RestoreNodeNumbers() {
	for(long i = 0; i < nodes.nbmax; i++) {
		for(size_t j = 0; j < MAX_LINKS; j++) {
			if(nodes.nodes[i].lnames[j][0] != 0) {
				nodes.nodes[i].link[j] = GetNumNodeByName(nodes.nodes[i].lnames[j]);
			}
		}
	}
}

void ClearNode(long i, long first = 0) {
	
	nodes.nodes[i].exist = 0;
	nodes.nodes[i].selected = 0;

	for (size_t j = 0; j < MAX_LINKS; j++)
	{
		if ((nodes.nodes[i].link[j] != -1) && (!first))
		{
			long k = nodes.nodes[i].link[j];

			for (size_t l = 0; l < MAX_LINKS; l++)
				if (nodes.nodes[k].link[l] == i)
					nodes.nodes[k].link[l] = -1;
		}

		nodes.nodes[i].lnames[j][0] = 0;
		nodes.nodes[i].link[j] = -1;
	}

	strcpy(nodes.nodes[i].name, "");
	nodes.nodes[i].pos = Vec3f::ZERO;
}

void ClearNodes()
{
	static long first = 1;

	for (long i = 0; i < nodes.nbmax; i++)
	{
		ClearNode(i, first);
	}

	first = 0;
}
//*************************************************************************************
//*************************************************************************************
void SelectNode(long i)
{
	if ((i >= nodes.nbmax)
	        ||	(i < 0))
		return;

	if (nodes.nodes[i].exist)  nodes.nodes[i].selected = 1;
}

void UnselectAllNodes() {
	for(long i = 0; i < nodes.nbmax; i++) {
		if(nodes.nodes[i].exist) {
			nodes.nodes[i].selected = 0;
		}
	}
}

void TranslateSelectedNodes(Vec3f * trans) {
	for(long i = 0; i < nodes.nbmax; i++) {
		if(nodes.nodes[i].exist && nodes.nodes[i].selected) {
			nodes.nodes[i].pos += *trans;
		}
	}
}

bool IsLinkedNode(long i, long j)
{
	if ((!nodes.nodes[i].exist)
	        ||	(!nodes.nodes[j].exist))
		return false;

	for (size_t k = 0; k < MAX_LINKS; k++)
	{
		if (nodes.nodes[i].link[k] == j) return true;
	}

	return false;
}

long CountNodes() {
	long count = 0;
	for(long i = 0; i < nodes.nbmax; i++) {
		if(nodes.nodes[i].exist) {
			count++;
		}
	}
	return count;
}

void AddLink(long i, long j)
{
	if ((!nodes.nodes[i].exist)
	        ||	(!nodes.nodes[j].exist))
		return;

	for (size_t k = 0; k < MAX_LINKS; k++)
	{
		if (nodes.nodes[i].link[k] == -1)
		{
			nodes.nodes[i].link[k] = j;
			return;
		}
	}
}
//*************************************************************************************
//*************************************************************************************
void RemoveLink(long i, long j)
{
	if ((!nodes.nodes[i].exist)
	        ||	(!nodes.nodes[j].exist))
		return;

	for (size_t k = 0; k < MAX_LINKS; k++)
	{
		if (nodes.nodes[i].link[k] == j)
		{
			nodes.nodes[i].link[k] = -1;
			return;
		}
	}
}
//*************************************************************************************
//*************************************************************************************
void LinkNodeToNode(long i, long j)
{
	if ((!IsLinkedNode(i, j))
	        ||	(!IsLinkedNode(j, i)))
		return;

	AddLink(i, j);
	AddLink(j, i);
}
//*************************************************************************************
//*************************************************************************************
void UnLinkNodeFromNode(long i, long j)
{
	if ((!IsLinkedNode(i, j))
	        ||	(!IsLinkedNode(j, i)))
		return;

	RemoveLink(i, j);
	RemoveLink(j, i);
}
//*************************************************************************************
//*************************************************************************************
void ClearSelectedNodes()
{
	for (long i = 0; i < nodes.nbmax; i++)
	{
		if ((nodes.nodes[i].exist)
		        &&	(nodes.nodes[i].selected))
		{
			ClearNode(i, 0);
		}
	}
}

//*************************************************************************************
//*************************************************************************************
bool ExistNodeName(char * name)
{
	for (long i = 0; i < nodes.nbmax; i++)
	{
		if ((nodes.nodes[i].exist)
		        &&	(!strcmp(name, nodes.nodes[i].name)))
			return true;
	}

	return false;
}
//*************************************************************************************
//*************************************************************************************
void MakeNodeName(long i)
{
	char name[64];
	long o;
	//float f;
	sprintf(name, "node_%08ld", i);

	while (ExistNodeName(name))
	{
		//f=rnd()*99999999.f;
		//o=(long)f;
		o = rnd() * 99999999.f;
		sprintf(name, "node_%08ld", o);
	}

	strcpy(nodes.nodes[i].name, name);
}

//*************************************************************************************
//*************************************************************************************
void InitNodes(long nb)
{
	if (nb < 1) nb = 1;

	nodes.init = 1;
	nodes.nbmax = nb;
	nodes.nodes = (ARX_NODE *)malloc(sizeof(ARX_NODE) * nodes.nbmax);
	memset(nodes.nodes, 0, sizeof(ARX_NODE)*nodes.nbmax);
	ClearNodes();
}
//*************************************************************************************
//*************************************************************************************
long GetFreeNode()
{
	for (long i = 0; i < nodes.nbmax; i++)
	{
		if (!nodes.nodes[i].exist) return i;
	}

	return -1;
}

void ReleaseNode() {
	free(nodes.nodes), nodes.nodes = NULL;
	nodes.nbmax = 0;
}

// Removes an IO loaded by a script command
void CleanScriptLoadedIO() {
	for(size_t i = 1; i < entities.size(); i++) {
		Entity * io = entities[i];
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

// Restores an IO to its initial status (Game start Status)
void RestoreInitialIOStatus() {
	ARX_INTERACTIVE_HideGore(entities.player());
	ARX_NPC_Behaviour_ResetAll();
	if(entities.player()) {
		entities.player()->spellcast_data.castingspell = SPELL_NONE;
	}
	for(size_t i = 1; i < entities.size(); i++) {
		RestoreInitialIOStatusOfIO(entities[i]);
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
	
	if(!io) {
		return;
	}
	
	if(ValidDynLight(io->dynlight)) {
		DynLight[io->dynlight].exist = 0;
	}
	io->dynlight = -1;
	
	if(ValidDynLight(io->halo.dynlight)) {
		DynLight[io->halo.dynlight].exist = 0;
	}
	io->halo.dynlight = -1;
	
	free(io->symboldraw), io->symboldraw = NULL;
	
	io->spellcast_data.castingspell = SPELL_NONE;
}

void ARX_INTERACTIVE_ClearIODynData_II(Entity * io)
{
	if (io)
	{
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
		
		free(io->lastanimvertex), io->lastanimvertex = NULL, io->nb_lastanimvertex = 0;
		
		for (long j = 0; j < MAX_ANIMS; j++)
		{
			EERIE_ANIMMANAGER_ReleaseHandle(io->anims[j]);
			io->anims[j] = NULL;
		}

		ARX_SPELLS_RemoveAllSpellsOn(io);
		ARX_EQUIPMENT_ReleaseAll(io);
		
		if(io->ioflags & IO_NPC) {
			free(io->_npcdata->pathfind.list), io->_npcdata->pathfind.list = NULL;
			memset(&io->_npcdata->pathfind, 0, sizeof(IO_PATHFIND));
			io->_npcdata->pathfind.truetarget = -1;
			io->_npcdata->pathfind.listnb = -1;
			ARX_NPC_Behaviour_Reset(io);
		}
		
		delete io->tweakerinfo, io->tweakerinfo = NULL;
		
		if (io->inventory != NULL)
		{
			INVENTORY_DATA * id = io->inventory;

			for (long nj = 0; nj < id->sizey; nj++) {
				for (long ni = 0; ni < id->sizex; ni++) {
					if (id->slot[ni][nj].io != NULL) {
						id->slot[ni][nj].io->destroy();
						id->slot[ni][nj].io = NULL;
					}
				}
			}

			if ((TSecondaryInventory) && (TSecondaryInventory->io == io))
			{
				TSecondaryInventory = NULL;
			}

			free(io->inventory);
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
}

void ARX_INTERACTIVE_ClearAllDynData() {
	ARX_INTERACTIVE_HideGore(entities.player());
	ARX_NPC_Behaviour_ResetAll();
	for(size_t i = 1; i < entities.size(); i++) {
		ARX_INTERACTIVE_ClearIODynData(entities[i]);
	}
}

static void RestoreIOInitPos(Entity * io) {
	if(io) {
		ARX_INTERACTIVE_Teleport(io, &io->initpos, 0);
		io->pos = io->lastpos = io->initpos;
		io->move = Vec3f::ZERO;
		io->lastmove = Vec3f::ZERO;
		io->angle = io->initangle;
	}
}

void RestoreAllIOInitPos() {
	for(size_t i = 1; i < entities.size(); i++) {
		RestoreIOInitPos(entities[i]);
	}
}

void ARX_HALO_SetToNative(Entity * io) {
	io->halo.color = io->halo_native.color;
	io->halo.radius = io->halo_native.radius;
	io->halo.flags = io->halo_native.flags;
}

void RestoreInitialIOStatusOfIO(Entity * io)
{
	if (io)
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
		io->halo.dynlight = -1;

		io->forcedmove = Vec3f::ZERO;
		io->ioflags &= ~IO_NO_COLLISIONS;
		io->ioflags &= ~IO_INVERTED;
		io->lastspeechflag = 2;
	
		//HALO_NEGATIVE;
		io->no_collide = -1;

		for (long i = 0; i < MAX_FLARES; i++)
		{
			if ((flare[i].exist)  && (flare[i].io == io))
			{
				flare[i].io = NULL;
			}
		}

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
		io->move = Vec3f::ZERO;
		io->type_flags = 0;
		io->sound = -1;
		io->soundtime = 0;
		io->soundcount = 0;
		io->material = MATERIAL_STONE;
		io->collide_door_time = 0;
		io->ouch_time = 0;
		io->dmg_sum = 0;
		io->ignition = 0.f;
		io->ignit_light = -1;
		io->ignit_sound = audio::INVALID_ID;

		if ((io->obj) && (io->obj->pbox)) io->obj->pbox->active = 0;

		io->room = -1;
		io->room_flags = 1;
		RestoreIOInitPos(io);
		ARX_INTERACTIVE_Teleport(io, &io->initpos, 0);
		io->lastanimtime = 1;
		io->secretvalue = -1;

		if (io->damagedata >= 0) damages[io->damagedata].exist = 0;

		io->damagedata = -1;
		io->poisonous = 0;
		io->poisonous_count = 0;

		for (long count = 0; count < MAX_ANIM_LAYERS; count++)
		{
			memset(&io->animlayer[count], 0, sizeof(ANIM_USE));
			io->animlayer[count].cur_anim = NULL;
			io->animlayer[count].next_anim = NULL;
		}

		if ((io->obj) && (io->obj->pbox))
		{
			io->obj->pbox->storedtiming = 0;
		}
		
		io->physics.cyl.origin = io->pos;
		io->physics.cyl.radius = io->original_radius;
		io->physics.cyl.height = io->original_height;
		io->fall = 0;
		io->show = SHOW_FLAG_IN_SCENE;
		io->targetinfo = TARGET_NONE;
		io->spellcast_data.castingspell = SPELL_NONE;
		io->summoner = -1;
		io->spark_n_blood = 0;

		if (io->ioflags & IO_NPC)
		{
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
			io->_npcdata->maxlife = 20.f;
			io->_npcdata->life = io->_npcdata->maxlife;
			io->_npcdata->maxmana = 10.f;
			io->_npcdata->mana = io->_npcdata->mana;
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

		if (io->ioflags & IO_ITEM)
		{
			io->collision = COLLIDE_WITH_PLAYER;
			io->_itemdata->count = 1;
			io->_itemdata->maxcount = 1;
			io->_itemdata->food_value = 0;
			io->_itemdata->playerstacksize = 1;
			io->_itemdata->stealvalue = -1;
			io->_itemdata->LightValue = -1;
		}
		else io->collision = 0;

		if (io->ioflags & IO_FIX)
		{
			memset(io->_fixdata, 0, sizeof(IO_FIXDATA));
			io->_fixdata->trapvalue = -1;
		}
	}
}

void ARX_INTERACTIVE_TWEAK_Icon(Entity * io, const res::path & s1) {
	
	if(!io || s1.empty()) {
		return;
	}
	
	res::path icontochange = io->classPath().parent() / s1;
	
	TextureContainer * tc = TextureContainer::LoadUI(icontochange,
	                                                 TextureContainer::Level);
	if(!tc) {
		tc = TextureContainer::LoadUI("graph/interface/misc/default[icon]");
	}
	
	if(tc) {
		
		unsigned long w = tc->m_dwWidth >> 5;
		unsigned long h = tc->m_dwHeight >> 5; 

		if ((w << 5) != tc->m_dwWidth) io->sizex = (char)(w + 1);
		else io->sizex = (char)(w);

		if ((h << 5) != tc->m_dwHeight) io->sizey = (char)(h + 1);
		else io->sizey = (char)(h);

		if (io->sizex < 1) io->sizex = 1;
		else if (io->sizex > 3) io->sizex = 3;

		if (io->sizey < 1) io->sizey = 1;
		else if (io->sizey > 3) io->sizey = 3;

		io->inv = tc;
	}
}

// Count IO number ignoring ScriptLoaded IOs
long GetNumberInterWithOutScriptLoad() {
	long count = 0;
	for(size_t i = 1; i < entities.size(); i++) {
		if(entities[i] != NULL && !entities[i]->scriptload) {
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
		dest->obj->pbox = (PHYSICS_BOX_DATA *)malloc(sizeof(PHYSICS_BOX_DATA));
		memcpy(dest->obj->pbox, src->obj->pbox, sizeof(PHYSICS_BOX_DATA));
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
	long count = 0;
	float modx, modz;
	
	while(count < 600) {
		
		modx = -EEsin(count) * (float)count * ( 1.0f / 3 );
		modz = EEcos(count) * (float)count * ( 1.0f / 3 );
		phys.origin.x = target->x + modx;
		phys.origin.z = target->z + modz;
		float anything = CheckAnythingInCylinder(&phys, io, CFLAG_JUST_TEST);

		if (EEfabs(anything) < 150.f)
		{
			EERIEPOLY * ep = CheckInPoly(phys.origin.x, phys.origin.y + anything - 20.f, phys.origin.z);
			EERIEPOLY * ep2 = CheckTopPoly(phys.origin.x, phys.origin.y + anything, phys.origin.z);

			if (ep && ep2 && (EEfabs((phys.origin.y + anything) - ep->center.y) < 20.f))
			{
				target->x = phys.origin.x;
				target->y = phys.origin.y + anything;
				target->z = phys.origin.z;
				return true;
			}
		}

		count += 5;
	}

	return false;
}
void ARX_INTERACTIVE_TeleportBehindTarget(Entity * io)
{
	if (!io) return;

	if (ARX_SCRIPT_GetSystemIOScript(io, "_r_a_t_") < 0)
	{
		long num = ARX_SCRIPT_Timer_GetFree();

		if (num != -1)
		{
			long t = io->index();
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
			MakeCoolFx(&io->pos);
			io->gameFlags |= GFLAG_INVISIBILITY;
			FRAME_COUNT = 0;
		}
	}
}
void ResetVVPos(Entity * io)
{
	if ((io) && (io->ioflags & IO_NPC))
		io->_npcdata->vvpos = io->pos.y;
}
void ComputeVVPos(Entity * io)
{
	if (io->ioflags & IO_NPC)
	{
		float vvp = io->_npcdata->vvpos;

		if ((vvp == -99999.f) || (vvp == io->pos.y))
		{
			io->_npcdata->vvpos = io->pos.y;
			return;
		}

		float diff = io->pos.y - vvp;
		float fdiff = EEfabs(diff);
		float eediff = fdiff;

		if (fdiff > 120.f) 
		{
			fdiff = 120.f;
		}
		else
		{
			float mul = ((fdiff * ( 1.0f / 120 )) * 0.9f + 0.6f);

			if ((eediff < 15.f))
			{
				float val = (float)FrameDiff * ( 1.0f / 4 ) * mul;

				if (eediff < 10.f)
					val *= ( 1.0f / 10 );
				else
				{
					float ratio = (eediff - 10.f) * ( 1.0f / 5 );
					val = val * ratio + val * (1.f - ratio); 
				}

				fdiff -= val;
			}
			else
			{
				fdiff -= (float)FrameDiff * ( 1.0f / 4 ) * mul;
			}
		}

		if (fdiff > eediff)
			fdiff = eediff;

		if (fdiff < 0.f) fdiff = 0.f;

		if (diff < 0.f) io->_npcdata->vvpos = io->pos.y + fdiff;
		else io->_npcdata->vvpos = io->pos.y - fdiff;
	}
}

void ARX_INTERACTIVE_Teleport(Entity * io, Vec3f * target, long flags) {
	
	if(!io) {
		return;
	}
	
	FRAME_COUNT = -1;
	io->gameFlags &= ~GFLAG_NOCOMPUTATION;
	io->room_flags |= 1;
	io->room = -1;
	
	if(io == entities.player()) {
		moveto = player.pos = *target + player.baseOffset();
	}
	
	// In case it is being dragged... (except for drag teleport update)
	if((!flags & 1) && io == DRAGINTER) { // TODO probably wrong
		Set_DragInter(NULL);
	}
	
	if(io->ioflags & IO_NPC) {
		io->_npcdata->vvpos = io->pos.y;
	}
	
	Vec3f translate = *target - io->pos;
	io->lastpos = io->physics.cyl.origin = io->pos = *target;
	
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

Entity * AddInteractive(const res::path & classPath, EntityInstance instance,
                        AddInteractiveFlags flags) {
	
	const string & ficc = classPath.string();
	
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
//***********************************************************************************
// SetWeapon:
// Links an object designed by path "temp" to the primary attach of interactive object
// "io".
//***********************************************************************************

void SetWeapon_On(Entity * io) {
	
	if(!io || !(io->ioflags & IO_NPC)) {
		return;
	}
	
	Entity * ioo = io->_npcdata->weapon;
	
	if(ioo && ioo->obj) {
		EERIE_LINKEDOBJ_UnLinkObjectFromObject(io->obj, ioo->obj);
		EERIE_LINKEDOBJ_LinkObjectToObject(io->obj, ioo->obj, "primary_attach", "primary_attach", ioo);
	}
}

void SetWeapon_Back(Entity * io) {
	
	if(!io || !(io->ioflags & IO_NPC)) {
		return;
	}
	
	Entity * ioo = io->_npcdata->weapon;
	
	if(ioo && ioo->obj) {
		
		EERIE_LINKEDOBJ_UnLinkObjectFromObject(io->obj, ioo->obj);

		if (io->gameFlags & GFLAG_HIDEWEAPON) return;

		long ni = io->obj->fastaccess.weapon_attach;

		if (ni >= 0)
			EERIE_LINKEDOBJ_LinkObjectToObject(io->obj, ioo->obj, "weapon_attach", "primary_attach", ioo);
		else
		{
			ni = io->obj->fastaccess.secondary_attach;

			if (ni >= 0)
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

// Links an Interactive Object to another interactive object using an attach point
void LinkObjToMe(Entity * io, Entity * io2, const std::string & attach) {
	
	if(!io || !io2) {
		return;
	}
	
	RemoveFromAllInventories(io2);
	io2->show = SHOW_FLAG_LINKED;
	EERIE_LINKEDOBJ_LinkObjectToObject(io->obj, io2->obj, attach, attach, io2);
}

// Creates a Temporary IO Ident
static void MakeTemporaryIOIdent(Entity * io) {
	
	if(!io) {
		return;
	}
	
	// TODO keep the current game open all the time (or even in memory)
	ARX_Changelevel_CurGame_Open();
	
	std::string className = io->short_name();
	res::path classDir = io->classPath().parent();
	
	for(long t = 1; ; t++) {
		
		// Check if the candidate instance number is used in the current scene
		bool used = false;
		// TODO replace this loop by an (className, instance) index
		for(size_t i = 0; i < entities.size(); i++) {
			if(entities[i] && entities[i]->ident == t && io != entities[i]) {
				if(entities[i]->short_name() == className) {
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
		if(ARX_Changelevel_CurGame_Seek(ss.str())) {
			continue;
		}
		
		io->ident = t;
		
		ARX_Changelevel_CurGame_Close();
		
		return;
	}
}

Entity * AddFix(const res::path & classPath, EntityInstance instance,
                AddInteractiveFlags flags) {
	
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
	io->pos.x -= EEsin(radians(player.angle.b)) * 140.f;
	io->pos.z += EEcos(radians(player.angle.b)) * 140.f;
	io->lastpos = io->initpos = io->pos;
	io->lastpos.x = io->initpos.x = EEfabs(io->initpos.x / 20) * 20.f;
	io->lastpos.z = io->initpos.z = EEfabs(io->initpos.z / 20) * 20.f;
	
	float tempo;
	EERIEPOLY * ep = CheckInPoly(io->pos.x, io->pos.y + player.baseHeight(), io->pos.z);
	if(ep && GetTruePolyY(ep, &io->pos, &tempo)) {
		io->lastpos.y = io->initpos.y = io->pos.y = tempo;
	}
	
	ep = CheckInPoly(io->pos.x, player.pos.y, io->pos.z);
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

static Entity * AddCamera(const res::path & classPath,
                          EntityInstance instance) {
	
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
	io->pos.x -= EEsin(radians(player.angle.b)) * 140.f;
	io->pos.z += EEcos(radians(player.angle.b)) * 140.f;
	io->lastpos = io->initpos = io->pos;
	io->lastpos.x = io->initpos.x = EEfabs(io->initpos.x / 20) * 20.f;
	io->lastpos.z = io->initpos.z = EEfabs(io->initpos.z / 20) * 20.f;
	
	float tempo;
	EERIEPOLY * ep;
	ep = CheckInPoly(io->pos.x, io->pos.y + player.baseHeight(), io->pos.z, &tempo);
	if(ep) {
		io->lastpos.y = io->initpos.y = io->pos.y = tempo;
	}
	
	ep = CheckInPoly(io->pos.x, player.pos.y, io->pos.z);
	if(ep) {
		io->pos.y = min(ep->v[0].p.y, ep->v[1].p.y);
		io->lastpos.y = io->initpos.y = io->pos.y = min(io->pos.y, ep->v[2].p.y);
	}
	
	io->lastpos.y = io->initpos.y = io->pos.y += player.baseHeight();
	
	io->obj = cameraobj;
	
	io->_camdata = (IO_CAMDATA *)malloc(sizeof(IO_CAMDATA));
	memcpy(&io->_camdata->cam, &subj, sizeof(EERIE_CAMERA));
	io->_camdata->cam.focal = 350.f;
	io->ioflags = IO_CAMERA;
	io->collision = 0;
	
	return io;
}

static Entity * AddMarker(const res::path & classPath,
                          EntityInstance instance) {
	
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
	io->pos.x -= EEsin(radians(player.angle.b)) * 140.f;
	io->pos.z += EEcos(radians(player.angle.b)) * 140.f;
	io->lastpos = io->initpos = io->pos;
	io->lastpos.x = io->initpos.x = EEfabs(io->initpos.x / 20) * 20.f;
	io->lastpos.z = io->initpos.z = EEfabs(io->initpos.z / 20) * 20.f;
	
	float tempo;
	EERIEPOLY * ep;
	ep = CheckInPoly(io->pos.x, io->pos.y + player.baseHeight(), io->pos.z);
	if(ep && GetTruePolyY(ep, &io->pos, &tempo)) {
		io->lastpos.y = io->initpos.y = io->pos.y = tempo;
	}
	
	ep = CheckInPoly(io->pos.x, player.pos.y, io->pos.z);
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

void ShowIOPath(Entity * io) {
	
	for(long i = 0; i < ACTIVEBKG->nbanchors; i++) {
		ANCHOR_DATA * ad = &ACTIVEBKG->anchors[i];
		ad->flags &= ~ANCHOR_FLAG_GREEN_DRAW;
	}
	
	if(io && (io->ioflags & IO_NPC)) {
		for(long j = 0; j < io->_npcdata->pathfind.listnb; j++) {
			ANCHOR_DATA * ad = &ACTIVEBKG->anchors[io->_npcdata->pathfind.list[j]];
			ad->flags |= ANCHOR_FLAG_GREEN_DRAW;
		}
	}
}

#ifdef BUILD_EDITOR

//*************************************************************************************
// Delete All Selected IOs
//*************************************************************************************
void ARX_INTERACTIVE_DeleteByIndex(long i, DeleteByIndexFlags flag) {
	
	if(!ValidIONum(i) || i == 0) {
		return;
	}
	
	//Must KILL dir...
	if(!(flag & FLAG_DONTKILLDIR) && entities[i]->scriptload == 0 && entities[i]->ident > 0) {
		
		fs::path dir = fs::paths.user / entities[i]->full_name().string();
		
		if(fs::is_directory(dir) && !fs::remove_all(dir)) {
			LogError << "Could not remove directory " << dir;
		}
	}
	
	delete entities[i];
}

// Snaps to ground all selected IOs
void GroundSnapSelectedIO() {
	for(size_t i = 1; i < entities.size(); i++) {
		if(entities[i] != NULL && false /* is selected */) {
			Entity * io = entities[i];
			float ay;
			EERIEPOLY * ep = CheckInPoly(io->pos.x, io->pos.y + player.baseHeight(), io->pos.z);
			if(ep && GetTruePolyY(ep, &io->pos, &ay)) {
				io->initpos.y = io->pos.y = ay;
			}
		}
	}
}

#endif // BUILD_EDITOR

IO_NPCDATA::IO_NPCDATA() {
	
	life = maxlife = 20.f;
	mana = maxmana = 0.f;
	
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
	last_splat_pos = Vec3f::ZERO;
	vvpos = 0.f;
	
	climb_count = 0.f;
	stare_factor = 0.f;
	fDetect = 0.f;
	cuts = 0;
	unused = 0;
	
}

IO_NPCDATA::~IO_NPCDATA() {
	free(ex_rotate);
	free(pathfind.list);
}

Entity * AddNPC(const res::path & classPath, EntityInstance instance,
                AddInteractiveFlags flags) {
	
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
	
	io->forcedmove = Vec3f::ZERO;
	
	io->_npcdata = new IO_NPCDATA;
	io->ioflags = IO_NPC;
	
	GetIOScript(io, script);
	
	io->spellcast_data.castingspell = SPELL_NONE;
	io->_npcdata->mana = io->_npcdata->maxmana = 10.f;
	io->_npcdata->critical = 5.f;
	io->_npcdata->reach = 20.f;
	io->_npcdata->stare_factor = 1.f;
	
	if(!(flags & NO_ON_LOAD)) {
		SendIOScriptEvent(io, SM_LOAD);
	}
	
	io->pos = player.pos;
	io->pos.x -= EEsin(radians(player.angle.b)) * 140.f;
	io->pos.z += EEcos(radians(player.angle.b)) * 140.f;
	io->lastpos = io->initpos = io->pos;
	io->lastpos.x = io->initpos.x = EEfabs(io->initpos.x / 20) * 20.f;
	io->lastpos.z = io->initpos.z = EEfabs(io->initpos.z / 20) * 20.f;
	
	float tempo;
	EERIEPOLY * ep = CheckInPoly(io->pos.x, io->pos.y + player.baseHeight(), io->pos.z);
	if(ep && GetTruePolyY(ep, &io->pos, &tempo)) {
		io->lastpos.y = io->initpos.y = io->pos.y = tempo; 
	}
	
	ep = CheckInPoly(io->pos.x, player.pos.y, io->pos.z);
	if(ep) {
		io->pos.y = min(ep->v[0].p.y, ep->v[1].p.y);
		io->lastpos.y = io->initpos.y = io->pos.y = min(io->pos.y, ep->v[2].p.y);
	}
	
	if(!io->obj && !(flags & NO_MESH)) {
		io->obj = loadObject(object, false);
	}
	
	io->_npcdata->pathfind.listnb = -1;
	io->_npcdata->behavior = BEHAVIOUR_NONE;
	io->_npcdata->pathfind.truetarget = -1;
	
	if(!(flags & NO_MESH) && (flags & IO_IMMEDIATELOAD)) {
		EERIE_COLLISION_Cylinder_Create(io);
	}
	
	io->infracolor = Color3f(1.f, 0.f, 0.2f);
	io->collision = COLLIDE_WITH_PLAYER;
	io->inv = NULL;
	
	ARX_INTERACTIVE_HideGore(io);
	return io;
}

//*************************************************************************************
// Reload Scripts (Class & Local) for an IO
//*************************************************************************************
void ReloadScript(Entity * io) {
	
	ARX_SCRIPT_Timer_Clear_For_IO(io);
	ReleaseScript(&io->over_script);
	ReleaseScript(&io->script);

	loadScript(io->script, resources->getFile(io->classPath() + ".asl"));
	loadScript(io->over_script, resources->getFile((io->full_name() / io->short_name()) + ".asl"));

	long num = io->index();

	if (ValidIONum(num))
	{
		if (entities[num] && entities[num]->script.data)
			ScriptEvent::send(&entities[num]->script, SM_INIT, "", entities[num], "");

		if (entities[num] && entities[num]->over_script.data)
			ScriptEvent::send(&entities[num]->over_script, SM_INIT, "", entities[num], "");

		if (entities[num] && entities[num]->script.data)
			ScriptEvent::send(&entities[num]->script, SM_INITEND, "", entities[num], "");

		if (entities[num] && entities[num]->over_script.data)
			ScriptEvent::send(&entities[num]->over_script, SM_INITEND, "", entities[num], "");
	}
}

// Reloads All Scripts for all IOs
void ReloadAllScripts() {
	ARX_SCRIPT_Timer_ClearAll();
	for(size_t i = 0; i < entities.size(); i++) {
		if(entities[i]) {
			ReloadScript(entities[i]);
		}
	}
}

#ifdef BUILD_EDIT_LOADSAVE

// Creates an unique identifier for an IO
void MakeIOIdent(Entity * io) {
	
	if(!io) {
		return;
	}
	
	long t = 1;
	
	while(io->ident == 0) {
		
		fs::path temp = fs::paths.user / io->full_name().string();
		
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

extern EERIE_3DOBJ	* arrowobj;

Entity * AddItem(const res::path & classPath_, EntityInstance instance,
                 AddInteractiveFlags flags) {
	
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

	if (io->ioflags & IO_GOLD)
	{
		io->_itemdata->price = 1;
	}
	else
	{
		io->_itemdata->price = 10;
	}

	io->_itemdata->playerstacksize = 1;

	GetIOScript(io, script);
	
	if(!(flags & NO_ON_LOAD)) {
		SendIOScriptEvent(io, SM_LOAD);
	}
	
	io->spellcast_data.castingspell = SPELL_NONE;
	io->lastpos.x = io->initpos.x = io->pos.x = player.pos.x - (float)EEsin(radians(player.angle.b)) * 140.f;
	io->lastpos.y = io->initpos.y = io->pos.y = player.pos.y;
	io->lastpos.z = io->initpos.z = io->pos.z = player.pos.z + (float)EEcos(radians(player.angle.b)) * 140.f;
	io->lastpos.x = io->initpos.x = (float)((long)(io->initpos.x / 20)) * 20.f;
	io->lastpos.z = io->initpos.z = (float)((long)(io->initpos.z / 20)) * 20.f;

	EERIEPOLY * ep;
	ep = CheckInPoly(io->pos.x, io->pos.y - 60.f, io->pos.z);

	if (ep)
	{
		float tempo;

		if (GetTruePolyY(ep, &io->pos, &tempo))
			io->lastpos.y = io->initpos.y = io->pos.y = tempo; 
	}

	ep = CheckInPoly(io->pos.x, player.pos.y, io->pos.z);

	if (ep)
	{
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
	
	if (tc)
	{
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

		if (io->sizex < 1)
			io->sizex = 1;
		else if (io->sizex > 3)
			io->sizex = 3;

		if (io->sizey < 1)
			io->sizey = 1;
		else if (io->sizey > 3)
			io->sizey = 3;

		io->inv = tc;
	}

	io->infracolor.r = 0.2f;
	io->infracolor.g = 0.2f;
	io->infracolor.b = 1.f;
	io->collision = 0;

	return io;
}

//*************************************************************************************
// Returns nearest interactive object found at position x,y
//*************************************************************************************
Entity * GetFirstInterAtPos(Vec2s * pos, long flag, Vec3f * _pRef, Entity ** _pTable, int * _pnNbInTable)
{
	float n;
 
	float _fdist = 9999999999.f;
	float fdistBB = 9999999999.f;
	float fMaxDist = flag ? 9999999999.f : 350;
	Entity * foundBB = NULL;
	Entity * foundPixel = NULL;
	bool bPlayerEquiped = false;

	if (Project.telekinesis)
	{
		fMaxDist = 850;
	}

	int nStart = 1;
	int nEnd = entities.size();

	if ((flag == 3) && _pTable && _pnNbInTable)
	{
		nStart = 0;
		nEnd = *_pnNbInTable;
	}

	for (long i = nStart; i < nEnd; i++)
	{
		bool bPass = true;

		Entity * io;
		
		if ((flag == 3) && _pTable && _pnNbInTable) {
			io = _pTable[i];
		} else {
			io = entities[i];
		}

		// Is Object Valid ??
		if (io == NULL) continue;

		if (((io->ioflags & IO_CAMERA) || (io->ioflags & IO_MARKER))
		        && (EDITMODE != 1)) continue;

		if ((!(io->gameFlags & GFLAG_INTERACTIVITY)) && (!EDITMODE)) continue;

		// Is Object in TreatZone ??
		bPlayerEquiped = IsEquipedByPlayer(io);
		if ((bPlayerEquiped  && (player.Interface & INTER_MAP))
		    || (io->gameFlags & GFLAG_ISINTREATZONE))

			// Is Object Displayed on screen ???
			if ((io->show == SHOW_FLAG_IN_SCENE) || (bPlayerEquiped && flag) || (bPlayerEquiped  && (player.Interface & INTER_MAP) && (Book_Mode == BOOKMODE_STATS))) //((io->show==9) && (player.Interface & INTER_MAP)) )
			{
				if ((flag == 2) && _pTable && _pnNbInTable && ((*_pnNbInTable) < 256))
				{
					_pTable[ *_pnNbInTable ] = io;
					(*_pnNbInTable)++;
					continue;
				}

				if ((pos->x >= io->bbox1.x) && (pos->x <= io->bbox2.x)
				        && (pos->y >= io->bbox1.y) && (pos->y <= io->bbox2.y))
				{
					if (flag && _pRef)
					{
						float flDistanceToRef = distSqr(ACTIVECAM->orgTrans.pos, *_pRef);
						float flDistanceToIO = distSqr(ACTIVECAM->orgTrans.pos, io->pos);
						bPass = bPlayerEquiped || (flDistanceToIO < flDistanceToRef);
					}

					float fp = fdist(io->pos, player.pos);

					if ((!flag && (fp <= fMaxDist)) && ((foundBB == NULL) || (fp < fdistBB)))
					{
						fdistBB = fp;
						foundBB = io;
					}

					if ((io->ioflags & (IO_CAMERA | IO_MARKER | IO_GOLD)) || (bPlayerEquiped && !flag))
					{
						if (bPlayerEquiped)
							fp = 0.f;
						else
							fp = fdist(io->pos, player.pos);

						if ((fp < fdistBB) || (foundBB == NULL))
						{
							fdistBB = fp;
							foundBB = io;
							foundPixel = io;
						}

						goto suite;
					}

					for(size_t j = 0; j < io->obj->facelist.size(); j++) {
						
						if(io->animlayer[0].cur_anim != NULL) {
							n = CEDRIC_PtIn2DPolyProjV2(io->obj, &io->obj->facelist[j] , pos->x, pos->y);
						} else {
							n = PtIn2DPolyProj(io->obj, &io->obj->facelist[j] , pos->x, pos->y);
						}

						if (n > 0.f) 
						{
							if (bPlayerEquiped)
								fp = 0.f;
							else
								fp = fdist(io->pos, player.pos);

							if ((bPass && (fp <= fMaxDist)) && ((fp < _fdist) || (foundPixel == NULL)))
							{
								{
									_fdist = fp;
									foundPixel = io;
									goto suite;
								}
							}
						}
					}

				suite:
					;
				}
			}
	}

	if (foundPixel) return foundPixel;

	return foundBB;
}
bool IsEquipedByPlayer(const Entity * io)
{
	if (!io)
		return false;

	if ((io->ioflags & IO_ICONIC) && (io->show == SHOW_FLAG_ON_PLAYER))
		return true;

	long num = io->index();

	for (long i = 0; i < MAX_EQUIPED; i++)
	{
		if ((player.equiped[i] != 0) && (player.equiped[i] == num)) return true;
	}

	return false;
}

extern long LOOKING_FOR_SPELL_TARGET;
Entity * InterClick(Vec2s * pos) {
	
	if (IsFlyingOverInventory(pos))
	{
		return NULL;
	}

	float dist_Threshold;

	if (LOOKING_FOR_SPELL_TARGET)
		dist_Threshold = 550.f;
	else
		dist_Threshold = 360.f;

	Entity * io = GetFirstInterAtPos(pos);

	if(io != NULL)
	{
		if(io->ioflags & IO_NPC) {
			if(closerThan(player.pos, io->pos, dist_Threshold)) {
				return io;
			}
		}
		else if ((Project.telekinesis) || (EDITMODE))
		{
			return io;
		}
		else if (IsEquipedByPlayer(io)
		         || closerThan(player.pos, io->pos, dist_Threshold))
		{
			return io;
		}
	}

	return NULL;
}

// Need To upgrade to a more precise collision.
long IsCollidingAnyInter(float x, float y, float z, Vec3f * size) {
	
	Vec3f pos;
	for(size_t i = 0; i < entities.size(); i++) {
		Entity * io = entities[i];

		if ((io)
		        && (!(io->ioflags & IO_NO_COLLISIONS))
		        && (io->collision)
		        && (io->gameFlags & GFLAG_ISINTREATZONE)
		        && ((io->ioflags & IO_NPC) || (io->ioflags & IO_FIX))
		        && (io->show == SHOW_FLAG_IN_SCENE)
		   )
		{
			if (io->ioflags & IO_NPC)
			{
				if (io->_npcdata->life <= 0.f)
					goto suitet;
			}

			pos.x = x;
			pos.y = y;
			pos.z = z;

			if (IsCollidingInter(io, &pos)) return i;

			pos.y += size->y;

			if (IsCollidingInter(io, &pos)) return i;

		suitet:
			;
		}
	}

	return -1;
}

//*************************************************************************************
// To upgrade to a more precise collision.
//*************************************************************************************
static bool IsCollidingInter(Entity * io, Vec3f * pos) {
	
	if ((!io)
	        ||	(!io->obj))
		return false;

	if(closerThan(*pos, io->pos, 190.f)) {
		
		vector<EERIE_VERTEX> & vlist = io->obj->vertexlist3;

		if (io->obj->nbgroups > 4)
		{
			for (long i = 0; i < io->obj->nbgroups; i++)
			{
				long idx = io->obj->grouplist[i].origin;

				if(!fartherThan(*pos, vlist[idx].v, 50.f)) {
					return true;
				}
			}
		}
		else
		{
			long nbv = io->obj->vertexlist3.size();
			for (long i = 0; i < nbv; i++)
			{
				if (i != io->obj->origin)
					if(!fartherThan(*pos, vlist[i].v, 30.f)) {
						return true;
					}
			}
		}
	}

	return false;
}

void SetYlsideDeath(Entity * io)
{
	io->sfx_flag = SFX_TYPE_YLSIDE_DEATH;
	io->sfx_time = (unsigned long)(arxtime); 	
}
bool ARX_INTERACTIVE_CheckCollision(EERIE_3DOBJ * obj, long kk, long source)
{
	bool col = false;
	long avoid = -1;
	Entity * io_source = NULL;

	if (ValidIONum(source))
	{
		io_source = entities[source];
		avoid = io_source->no_collide;

	}

	for(size_t i = 1; i < entities.size(); i++) {
		Entity * io = entities[i];

		if (
		    (io)
				&& long(i) != avoid
		    && (!(io->ioflags & (IO_CAMERA | IO_MARKER | IO_ITEM)))
		    && (io->show == SHOW_FLAG_IN_SCENE)
		    && !(io->ioflags & IO_NO_COLLISIONS)
		    && (io->obj)
		    && (io->obj != obj)
		    && (!io->usepath)
		)
		{
			if (distSqr(io->pos, obj->pbox->vert[0].pos) < square(450.f) && (In3DBBoxTolerance(&obj->pbox->vert[kk].pos, &io->bbox3D, obj->pbox->radius)))
			{
				if ((io->ioflags & IO_NPC) && (io->_npcdata->life > 0.f))
				{
					if (PointInCylinder(&io->physics.cyl, &obj->pbox->vert[kk].pos))
					{
						return true;
					}
				}
				else if (io->ioflags & IO_FIX)
				{
					long step;
					long nbv;
					nbv = io->obj->vertexlist.size();

					if (nbv < 300) step = 1;
					else if (nbv < 600) step = 2;
					else if (nbv < 1200) step = 4;
					else step = 6;

					vector<EERIE_VERTEX> & vlist = io->obj->vertexlist3;

					EERIE_SPHERE sp;
					sp.radius = 22.f; 

					for (long ii = 1; ii < nbv; ii += step)
					{
						if (ii != io->obj->origin)
						{
							sp.origin = vlist[ii].v;

							if(sp.contains(obj->pbox->vert[kk].pos)) {
								if ((io_source) && (io->gameFlags & GFLAG_DOOR))
								{
									if (float(arxtime) > io->collide_door_time + 500)
									{
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
	}

	return col;
}
bool ARX_INTERACTIVE_CheckFULLCollision(EERIE_3DOBJ * obj, long source)
{
	bool col = false;
	long i;
	long avoid = -1;
	Entity * io_source = NULL;
	Entity * io = NULL;

	if (ValidIONum(source))
	{
		io_source = entities[source];
		avoid = io_source->no_collide;

	}

	for (i = 0; i < TREATZONE_CUR; i++)
	{
		if	((treatio[i].show != SHOW_FLAG_IN_SCENE)
		        ||	((treatio[i].ioflags & IO_NO_COLLISIONS))
		        || (!treatio[i].io)) continue;

		io = treatio[i].io;

		if ((io == io_source) || (!io->obj) || (io == entities.player())) 
			continue;

		if (treatio[i].num == avoid) continue;

		if ((io->ioflags  & (IO_CAMERA | IO_MARKER | IO_ITEM))
		        ||	(io->usepath))
			continue;

		if ((io->ioflags & IO_NPC)
		        &&	(io_source)
		        &&	(io_source->ioflags & IO_NO_NPC_COLLIDE))
			continue;

		
		if (distSqr(io->pos, obj->pbox->vert[0].pos) < square(600.f) && (In3DBBoxTolerance(&obj->pbox->vert[0].pos, &io->bbox3D, obj->pbox->radius)))
		{
			if ((io->ioflags & IO_NPC) && (io->_npcdata->life > 0.f))
			{
				for (long kk = 0; kk < obj->pbox->nb_physvert; kk++)
					if (PointInCylinder(&io->physics.cyl, &obj->pbox->vert[kk].pos))
					{
						return true;
					}
			}
			else if (io->ioflags & IO_FIX)
			{
				long step;
				long nbv;
				nbv = io->obj->vertexlist.size();
				EERIE_SPHERE sp;
				sp.radius = 28.f;

				if (nbv < 500)
				{
					step = 1;
					sp.radius = 36.f; 
				}
				else if (nbv < 900) step = 2; 
				else if (nbv < 1500) step = 4; 
				else step = 6;

				vector<EERIE_VERTEX> & vlist = io->obj->vertexlist3;


				if (io->gameFlags & GFLAG_PLATFORM)
				{
					for (long kk = 0; kk < obj->pbox->nb_physvert; kk++)
					{
						EERIE_SPHERE sphere;
						sphere.origin = obj->pbox->vert[kk].pos;
						sphere.radius = 30.f;
						float miny, maxy;
						miny = io->bbox3D.min.y;
						maxy = io->bbox3D.max.y;

						if ((maxy <= sphere.origin.y + sphere.radius)
						        ||	(miny >= sphere.origin.y))
							if (In3DBBoxTolerance(&sphere.origin, &io->bbox3D, sphere.radius))
							{
								// TODO why ignore the z components?
								if(closerThan(Vec2f(io->pos.x, io->pos.z), Vec2f(sphere.origin.x, sphere.origin.z), 440.f + sphere.radius)) {
									
									EERIEPOLY ep;
									ep.type = 0;

									for (size_t ii = 0; ii < io->obj->facelist.size(); ii++)
									{
										float cx = 0;
										float cz = 0;

										for (long idx = 0 ; idx < 3 ; idx++)
										{
											cx			+=	ep.v[idx].p.x	=	io->obj->vertexlist3[ io->obj->facelist[ii].vid[idx] ].v.x;
											ep.v[idx].p.y	=	io->obj->vertexlist3[ io->obj->facelist[ii].vid[idx] ].v.y;
											cz			+=	ep.v[idx].p.z	=	io->obj->vertexlist3[ io->obj->facelist[ii].vid[idx] ].v.z;
										}

										cx *= ( 1.0f / 3 );
										cz *= ( 1.0f / 3 );

										for (kk = 0; kk < 3; kk++)
										{
											ep.v[kk].p.x = (ep.v[kk].p.x - cx) * 3.5f + cx;
											ep.v[kk].p.z = (ep.v[kk].p.z - cz) * 3.5f + cz;
										}

										if (PointIn2DPolyXZ(&ep, sphere.origin.x, sphere.origin.z))
										{
											return true;
										}
									}
								}
							}
					}
				}


				for (long ii = 1; ii < nbv; ii += step)
				{
					if (ii != io->obj->origin)
					{

						if (0)
						{
							for (long jii = 1; jii < nbv; jii += step)
							{
								sp.origin = (vlist[ii].v + vlist[jii].v) * ( 1.0f / 2 );

								for (long kk = 0; kk < obj->pbox->nb_physvert; kk++)
									if(sp.contains(obj->pbox->vert[kk].pos)) {
										if ((io_source) && (io->gameFlags & GFLAG_DOOR))
										{
											if (float(arxtime) > io->collide_door_time + 500)
											{
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

						sp.origin = vlist[ii].v;

						for (long kk = 0; kk < obj->pbox->nb_physvert; kk++)
							if (sp.contains(obj->pbox->vert[kk].pos))
							{
								if ((io_source) && (io->gameFlags & GFLAG_DOOR))
								{
									if (float(arxtime) > io->collide_door_time + 500)
									{
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
		Entity * io = entities[i];
		
		if(io) {
			
			// interpolate & send events
			if(io->usepath) {

				ARX_USE_PATH * aup = io->usepath;
				float diff = float(arxtime) - aup->_curtime;

				if (aup->aupflags & ARX_USEPATH_FORWARD)
				{
					if (aup->aupflags & ARX_USEPATH_FLAG_FINISHED)
					{
					}
					else aup->_curtime += diff;
				}

				if (aup->aupflags & ARX_USEPATH_BACKWARD)
				{
					aup->_starttime += diff * 2;
					aup->_curtime += diff;

					if (aup->_starttime >= aup->_curtime)
						aup->_curtime = aup->_starttime + 1;
				}

				if (aup->aupflags & ARX_USEPATH_PAUSE)
				{
					aup->_starttime += diff;
					aup->_curtime += diff;
				}

				long last = ARX_PATHS_Interpolate(aup, &io->pos);

				if (aup->lastWP != last)
				{
					if (last == -2)
					{
						char str[16];
						sprintf(str, "%ld", aup->path->nb_pathways - 1);
						EVENT_SENDER = NULL;
						SendIOScriptEvent(io, SM_WAYPOINT, str);
						sprintf(str, "waypoint%ld", aup->path->nb_pathways - 1);
						SendIOScriptEvent(io, SM_NULL, "", str);
						SendIOScriptEvent(io, SM_PATHEND);
						aup->lastWP = last;
					}
					else
					{
						last--;
						long _from = aup->lastWP;
						long _to = last;

						if (_from > _to) _from = -1;

						if (_from < 0) _from = -1;

						long ii = _from + 1;
						
						char str[16];
						sprintf(str, "%ld", ii);
						EVENT_SENDER = NULL;
						SendIOScriptEvent(io, SM_WAYPOINT, str);
						sprintf(str, "waypoint%ld", ii);
						SendIOScriptEvent(io, SM_NULL, "", str);

						if (ii == aup->path->nb_pathways)
						{
							SendIOScriptEvent(io, SM_PATHEND);
						}
						
						aup->lastWP = last + 1;
					}
				}

				if ((io->damager_damages > 0)
				        &&	(io->show == SHOW_FLAG_IN_SCENE))
				{
					for(size_t ii = 0; ii < entities.size(); ii++) {
						Entity * ioo = entities[ii];

						if ((ioo)
						        &&	(ii != i)
						        &&	(ioo->show == SHOW_FLAG_IN_SCENE)
						        &&	(ioo->ioflags & IO_NPC)
						        &&	closerThan(io->pos, ioo->pos, 600.f))
						{
							bool Touched = false;

							for (size_t ri = 0; ri < io->obj->vertexlist.size(); ri += 3)
							{
								for (size_t rii = 0; rii < ioo->obj->vertexlist.size(); rii += 3)
								{
									if(closerThan(io->obj->vertexlist3[ri].v, ioo->obj->vertexlist3[rii].v, 20.f)) {
										Touched = true;
										ri = io->obj->vertexlist.size();
										break;
									}
								}
							}

							if (Touched)
								ARX_DAMAGES_DealDamages(ii, io->damager_damages, i, io->damager_type, &ioo->pos);
						}
					}
				}
			}
			
			if(io->ioflags & IO_CAMERA) {
				
				entities[i]->_camdata->cam.orgTrans.pos = io->pos;

				if (io->targetinfo != TARGET_NONE) // Follows target
				{
					GetTargetPos(io, (unsigned long)entities[i]->_camdata->cam.smoothing);
					io->target += io->_camdata->cam.translatetarget;

					if ((io->_camdata->cam.lastinfovalid) && (io->_camdata->cam.smoothing != 0.f))
					{
						Vec3f smoothtarget;
 
						float vv = (float)io->_camdata->cam.smoothing;

						if (vv > 8000) vv = 8000;

						vv = (8000 - vv) * ( 1.0f / 4000 );

						float f1 = framedelay * ( 1.0f / 1000 ) * vv; 

						if (f1 > 1.f) f1 = 1.f;

						float f2 = 1.f - f1;
						smoothtarget = io->target * f2 + io->_camdata->cam.lasttarget * f1;

						SetTargetCamera(&io->_camdata->cam, smoothtarget.x, smoothtarget.y, smoothtarget.z);
						io->_camdata->cam.lasttarget = smoothtarget;
						io->_camdata->cam.lastinfovalid = true;
						io->_camdata->cam.lastpos = io->_camdata->cam.orgTrans.pos;
					}
					else
					{
						if ((io->target.x == io->_camdata->cam.orgTrans.pos.x)
								&&	(io->target.y == io->_camdata->cam.orgTrans.pos.y)
								&&	(io->target.z == io->_camdata->cam.orgTrans.pos.z))
						{
						}
						else SetTargetCamera(&io->_camdata->cam, io->target.x, io->target.y, io->target.z);

						io->_camdata->cam.lasttarget = io->target;
						io->_camdata->cam.lastinfovalid = true;
						io->_camdata->cam.lastpos = io->_camdata->cam.orgTrans.pos;
					}

					io->_camdata->cam.angle.b -= 180.f;
					io->_camdata->cam.angle.a = -io->_camdata->cam.angle.a;
					io->angle.a = 0.f; 
					io->angle.b = io->_camdata->cam.angle.b + 90.f;
					io->angle.g = 0.f; 
				}	
				else // no target...
				{
					float tr = radians(MAKEANGLE(io->angle.b + 90));
					io->target.x = io->pos.x - (float)EEsin(tr) * 20.f;
					io->target.y = io->pos.y; 
					io->target.z = io->pos.z + (float)EEcos(tr) * 20.f;
					SetTargetCamera(&io->_camdata->cam, io->target.x, io->target.y, io->target.z);
					io->_camdata->cam.lasttarget = io->target;
					io->_camdata->cam.lastinfovalid = true;
					io->_camdata->cam.lastpos = io->_camdata->cam.orgTrans.pos;
				}
			}
		}
	}
}

void ARX_INTERACTIVE_UnfreezeAll() {
	for(size_t i = 0; i < entities.size(); i++) {
		if(entities[i]) {
			entities[i]->ioflags &= ~IO_FREEZESCRIPT;
		}
	}
}

void UpdateIOInvisibility(Entity * io)
{
	if (io && (io->invisibility <= 1.f))
	{
		if ((io->gameFlags & GFLAG_INVISIBILITY) && (io->invisibility < 1.f))
		{
			io->invisibility += framedelay * ( 1.0f / 1000 );

			if (io->invisibility > 1.f) io->invisibility = 1.f;
		}
		else if ((!(io->gameFlags & GFLAG_INVISIBILITY)) && (io->invisibility != 0.f))
		{
			io->invisibility -= framedelay * ( 1.0f / 1000 );

			if (io->invisibility < 0.f) io->invisibility = 0.f;
		}
	}
}
extern Entity * DESTROYED_DURING_RENDERING;

//*************************************************************************************
// Renders Interactive objects.
// Will render objects between distance "from" (included)
// to distance "to" (not included)
// from camera position.
//*************************************************************************************
void RenderInter(float from, float to) {

	GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapClamp);
	float val = -0.6f;
	GRenderer->GetTextureStage(0)->SetMipMapLODBias(val);
	Anglef temp;
	EERIEMATRIX mat;
	INTER_DRAW = 0;
	float dist;
	long diff;

	if (entities.player() && (entities.player()->ignition > 0.f))
	{
		ManageIgnition(entities.player());
	}

	for(size_t i = 1; i < entities.size(); i++) { // Player isn't rendered here...
		
		Entity * io = entities[i];

		if ((io)
		        &&	(io != DRAGINTER)
		        &&	(io->gameFlags & GFLAG_ISINTREATZONE))
		{
			if ((i == 0) && ((player.Interface & INTER_MAP) && (!(player.Interface & INTER_COMBATMODE)))
			        && (Book_Mode == BOOKMODE_STATS)) continue;
			
			if(io->show != SHOW_FLAG_IN_SCENE) {
				continue;
			}
			
			if(!EDITMODE && ((io->ioflags & IO_CAMERA) || (io->ioflags & IO_MARKER))) {
				continue;
			}
			
			if ((io->obj) &&
			        (io->obj->pbox) &&
			        (io->obj->pbox->active))  
			{
				dist = fdist(ACTIVECAM->orgTrans.pos, io->obj->pbox->vert[0].pos);
			}
			else dist = fdist(ACTIVECAM->orgTrans.pos, io->pos);

			if ((io) && (io->ioflags & IO_NPC) && (io->_npcdata->pathfind.flags  & PATHFIND_ALWAYS))
			{
			}
			else if ((dist < from) || (dist >= to)) continue;

			UpdateIOInvisibility(io);

			io->bbox1.x = 9999;
			io->bbox2.x = -1;

#ifdef BUILD_EDITOR
			if((io->obj) && (io->obj->pbox) && DEBUGNPCMOVE)
				EERIE_PHYSICS_BOX_Show(io->obj);
#endif

			if (
			    (io->obj)
			    &&	(io->obj->pbox)
			    &&	(io->obj->pbox->active))
			{
				Vec3f tmp = io->obj->pbox->vert[14].pos - io->obj->pbox->vert[13].pos;
				Vec3f up = io->obj->pbox->vert[2].pos - io->obj->pbox->vert[1].pos;
				up += io->obj->pbox->vert[3].pos - io->obj->pbox->vert[4].pos;
				up += io->obj->pbox->vert[10].pos - io->obj->pbox->vert[9].pos;
				up += io->obj->pbox->vert[11].pos - io->obj->pbox->vert[12].pos;
				up *= 0.25f;
				
				MatrixSetByVectors(&mat, &up, &tmp);
				mat._14 = mat._24 = mat._34 = 0.f;
				mat._41 = mat._42 = mat._43 = mat._44 = 0.f;


			}
				
			if(io->animlayer[0].cur_anim) {
				
				temp.a = io->angle.a;
				if(io->ioflags & IO_NPC) {
					temp.b = MAKEANGLE(180.f - io->angle.b);
				} else {
					temp.b = MAKEANGLE(270.f - io->angle.b);
				}
				temp.g = io->angle.g;

				if (io->animlayer[0].flags & EA_PAUSED)
					diff = 0;
				else diff = static_cast<long>(FrameDiff);

				if ((io == FlyingOverIO)
				        &&	(!(io->ioflags & IO_NPC))
				        &&	io->obj)
					io->obj->drawflags |= DRAWFLAG_HIGHLIGHT;

				Vec3f pos = io->pos;

				if (io->ioflags & IO_NPC)
				{
					ComputeVVPos(io);
					pos.y = io->_npcdata->vvpos;
				}

				bool render = (EDITMODE || !ARX_SCENE_PORTAL_Basic_ClipIO(io));

				EERIEDrawAnimQuat(io->obj, &io->animlayer[0], &temp, &pos, diff, io, render);

				if (DESTROYED_DURING_RENDERING)
					continue;

				if (io->obj)
					io->obj->drawflags &= ~DRAWFLAG_HIGHLIGHT;
			}
			else 
			{
				if ((!EDITMODE) && (ARX_SCENE_PORTAL_Basic_ClipIO(io))) continue;
				
				temp.a = io->angle.a;
				if(io->ioflags & IO_NPC) {
					temp.b = MAKEANGLE(180.f - io->angle.b);
				} else {
					temp.b = MAKEANGLE(270.f - io->angle.b);
				}
				temp.g = io->angle.g;
				
				if ((io->ioflags & IO_GOLD) && io->obj)
				{
					if (io->_itemdata->price <= 3)
					{
						io->obj = GoldCoinsObj[io->_itemdata->price-1];
						io->inv = GoldCoinsTC[io->_itemdata->price-1];
					}
					else if (io->_itemdata->price <= 8)
					{
						io->obj = GoldCoinsObj[3];
						io->inv = GoldCoinsTC[3];
					}
					else if (io->_itemdata->price <= 20)
					{
						io->obj = GoldCoinsObj[4];
						io->inv = GoldCoinsTC[4];
					}
					else if (io->_itemdata->price <= 50)
					{
						io->obj = GoldCoinsObj[5];
						io->inv = GoldCoinsTC[5];
					}
					else
					{
						io->obj = GoldCoinsObj[6];
						io->inv = GoldCoinsTC[6];
					}
				}

				if ((!(io->ioflags & IO_NPC))
				        ||	(EDITMODE))
				{
					if (io->obj)
					{
						if ((io == FlyingOverIO)
						        &&	(!(io->ioflags & IO_NPC)))
						{
							io->obj->drawflags |= DRAWFLAG_HIGHLIGHT;
						}

						if ((io->obj->pbox)
						        &&	(io->obj->pbox->active))
						{
							DrawEERIEInter(io->obj, NULL, &io->pos, io, &mat);
						}
						else
						{
							DrawEERIEInter(io->obj, &temp, &io->pos, io);
						}

						if (DESTROYED_DURING_RENDERING)
							continue;

						io->obj->drawflags &= ~DRAWFLAG_HIGHLIGHT;

					}
				}
			}

			if ((io->ignition > 0.f) || (io->ioflags & IO_FIERY))
				ManageIgnition(io);

			if(EDITMODE) {
				Color color = Color::blue;
				if(io->bbox1.x != io->bbox2.x && io->bbox1.x < DANAESIZX) {
					EERIEDraw2DLine(io->bbox1.x, io->bbox1.y, io->bbox2.x, io->bbox1.y, 0.01f, color);
					EERIEDraw2DLine(io->bbox2.x, io->bbox1.y, io->bbox2.x, io->bbox2.y, 0.01f, color);
					EERIEDraw2DLine(io->bbox2.x, io->bbox2.y, io->bbox1.x, io->bbox2.y, 0.01f, color);
					EERIEDraw2DLine(io->bbox1.x, io->bbox2.y, io->bbox1.x, io->bbox1.y, 0.01f, color);
				}
			}
		}
	}


	GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);
	val = -0.3f;
	GRenderer->GetTextureStage(0)->SetMipMapLODBias(val);
}

void ARX_INTERACTIVE_DestroyIO(Entity * ioo)
{
	if (ioo)
	{
		if (ioo->show == SHOW_FLAG_DESTROYED)
			return;

		ARX_INTERACTIVE_ForceIOLeaveZone(ioo, 0);

		if (DRAGINTER == ioo)
			Set_DragInter(NULL);

		if (FlyingOverIO == ioo)
			FlyingOverIO = NULL;

		if (COMBINE == ioo)
			COMBINE = NULL;

		if ((ioo->ioflags & IO_ITEM) && (ioo->_itemdata->count > 1))
		{
			ioo->_itemdata->count--;
		}
		else
		{
			// Kill all spells
			long numm = ioo->index();

			if (ValidIONum(numm))
				ARX_SPELLS_FizzleAllSpellsFromCaster(numm);

			// Need To Kill timers
			ARX_SCRIPT_Timer_Clear_By_IO(ioo);
			ioo->show = SHOW_FLAG_DESTROYED;
			ioo->gameFlags &= ~GFLAG_ISINTREATZONE;

			if (!FAST_RELEASE)
				RemoveFromAllInventories(ioo);

			if (ioo->obj)
			{
				EERIE_3DOBJ * eobj = ioo->obj;

				while (eobj->nblinked)
				{
					long k = 0;

					if ((eobj->linked[k].lgroup != -1) && eobj->linked[k].obj)
					{
						Entity * iooo = (Entity *)eobj->linked[k].io;

						if ((iooo) && ValidIOAddress(iooo))
						{
							EERIE_LINKEDOBJ_UnLinkObjectFromObject(ioo->obj, iooo->obj);
							ARX_INTERACTIVE_DestroyIO(iooo);
						}
					}
				}
			}

			ARX_INTERACTIVE_DestroyDynamicInfo(ioo);

			if(ioo->scriptload) {
				delete ioo;
			}
		}
	}
}

//*************************************************************************************
//
//*************************************************************************************
bool IsSameObject(Entity * io, Entity * ioo)
{
	if ((io == NULL)
	        ||	(ioo == NULL)
	        ||	io->classPath() != ioo->classPath()
	        ||	(io->ioflags & IO_UNIQUE)
	        ||	(io->durability != ioo->durability)
	        ||	(io->max_durability != ioo->max_durability))
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
			it1++;
		} else {
			it2++;
		}
	}
	
	return false;
}

bool HaveCommonGroup(Entity * io, Entity * ioo) {
	return io && ioo && intersect(io->groups, ioo->groups);
}

float ARX_INTERACTIVE_GetArmorClass(Entity * io)
{
	if (!io) return -1;

	if (!(io->ioflags & IO_NPC)) return -1;

	float ac = io->_npcdata->armor_class;

	for (long i = 0; i < io->nb_spells_on; i++)
	{
		long n = io->spells_on[i];

		if (spells[n].exist)
		{
			switch (spells[n].type)
			{
				case SPELL_ARMOR:
					ac += spells[n].caster_level;
					break;
				case SPELL_LOWER_ARMOR:
					ac -= spells[n].caster_level;
					break;
				default: break;
			}
		}
	}

	if (ac < 0) ac = 0;

	return ac;
}

void ARX_INTERACTIVE_ActivatePhysics(long t)
{
	if (ValidIONum(t))
	{
		Entity * io = entities[t];

		if ((io == DRAGINTER)
		        ||	(io->show != SHOW_FLAG_IN_SCENE))
			return;

		float yy;
		EERIEPOLY * ep = CheckInPoly(io->pos.x, io->pos.y, io->pos.z, &yy);

		if ((ep) && (yy - io->pos.y < 10.f))
			return;

		io->obj->pbox->active = 1;
		io->obj->pbox->stopcount = 0;
		Vec3f pos = io->pos;
		io->velocity = Vec3f::ZERO;
		io->stopped = 1;
		Vec3f fallvector = Vec3f(0.0f, 0.000001f, 0.f);
		io->show = SHOW_FLAG_IN_SCENE;
		io->soundtime = 0;
		io->soundcount = 0;
		EERIE_PHYSICS_BOX_Launch(io->obj, &pos, &fallvector);
	}
}

//-------------------------------------------------------------------------
string GetMaterialString(const res::path & texture) {
	
	const string & origin = texture.string();
	
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
