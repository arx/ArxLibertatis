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
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include "game/Inventory.h"

#include <algorithm>
#include <set>
#include <vector>

#include <boost/foreach.hpp>

#include "ai/Paths.h"

#include "core/Application.h"
#include "core/Config.h"

#include "game/Player.h"

#include "gui/Interface.h"

#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/data/Mesh.h"
#include "graphics/data/TextureContainer.h"

#include "input/Input.h"

#include "io/log/Logger.h"

#include "math/Angle.h"
#include "math/Vector2.h"
#include "math/Vector3.h"

#include "physics/Box.h"

#include "platform/Platform.h"

#include "scene/Light.h"
#include "scene/Interactive.h"
#include "scene/GameSound.h"

#include "script/Script.h"

using std::vector;
using std::string;

//------------------------------------------------------------------------------------
extern E_ARX_STATE_MOUSE eMouseState;
extern float InventoryX;
extern float InventoryDir;
extern long PLAYER_INTERFACE_HIDE_COUNT;
extern long	DRAGGING;

extern bool TRUE_PLAYER_MOUSELOOK_ON;

void ARX_INTERFACE_Combat_Mode(long i);
void ARX_INVENTORY_ReOrder();
void ARX_INVENTORY_IdentifyIO(INTERACTIVE_OBJ * _pIO);

//------------------------------------------------------------------------------------
//CInventory Inventory;
INVENTORY_SLOT inventory[3][INVENTORY_X][INVENTORY_Y];
INVENTORY_DATA * SecondaryInventory = NULL;
INTERACTIVE_OBJ * DRAGINTER = NULL;
INTERACTIVE_OBJ * ioSteal = NULL;
long InventoryY = 100;
long HERO_OR_SECONDARY = 0;
short sActiveInventory = 0;

// 1 player 2 secondary
short sInventory = -1;
short sInventoryX = -1;
short sInventoryY = -1;

/*!
 * Declares an IO as entering into player Inventory
 * Sends appropriate INVENTORYIN Event to player AND concerned io.
 */
static void ARX_INVENTORY_Declare_InventoryIn(INTERACTIVE_OBJ * io) {
	
	if(!io) {
		return;
	}
	
	io->show = SHOW_FLAG_IN_INVENTORY;
	
	if(io->ignition > 0) {
		
		if(ValidDynLight(io->ignit_light)) {
			DynLight[io->ignit_light].exist = 0;
		}
		io->ignit_light = -1;
		
		if (io->ignit_sound != audio::INVALID_ID) {
			ARX_SOUND_Stop(io->ignit_sound);
			io->ignit_sound = audio::INVALID_ID;
		}
		
		io->ignition = 0;
	}
	
	EVENT_SENDER = io;
	SendIOScriptEvent(inter.iobj[0], SM_INVENTORYIN);
	EVENT_SENDER = inter.iobj[0];
	SendIOScriptEvent(io, SM_INVENTORYIN);
	EVENT_SENDER = NULL;
}

//*************************************************************************************
// void CleanInventory()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Cleans Player inventory
//*************************************************************************************
void CleanInventory() {
	
	for(long iNbBag = 0; iNbBag < 3; iNbBag++) {
		for(size_t j = 0; j < INVENTORY_Y; j++) {
			for(size_t i = 0; i < INVENTORY_X; i++) {
				inventory[iNbBag][i][j].io	 = NULL;
				inventory[iNbBag][i][j].show = 1;
			}
		}
	}
	
	sActiveInventory = 0;
}

extern Vec2s DANAEMouse;
extern long DANAESIZX;
extern long DANAESIZY;
extern long DANAECENTERX;
extern long DANAECENTERY;

static INTERACTIVE_OBJ * GetInventoryObj(Vec2s * pos) {
	
	long tx, ty;


	float fCenterX	= DANAECENTERX - INTERFACE_RATIO(320) + INTERFACE_RATIO(35);
	float fSizY		= DANAESIZY - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY);

	int iPosX = checked_range_cast<int>(fCenterX);
	int iPosY = checked_range_cast<int>(fSizY);


	if (player.Interface & INTER_INVENTORY)
	{
		tx = pos->x - iPosX; //-4
		ty = pos->y - iPosY; //-2

		if ((tx >= 0) && (ty >= 0))
		{

			tx = checked_range_cast<long>(tx / INTERFACE_RATIO(32));
			ty = checked_range_cast<long>(ty / INTERFACE_RATIO(32));


			if ((tx >= 0) && ((size_t)tx < INVENTORY_X) && (ty >= 0) && ((size_t)ty < INVENTORY_Y))
			{
				if ((inventory[sActiveInventory][tx][ty].io)
				        &&	(inventory[sActiveInventory][tx][ty].io->GameFlags & GFLAG_INTERACTIVITY))
				{
					HERO_OR_SECONDARY = 1;
					return (inventory[sActiveInventory][tx][ty].io);
				}
			}

			return NULL;
		}
	}
	else if (player.Interface & INTER_INVENTORYALL)
	{

		float fBag	= (player.bag - 1) * INTERFACE_RATIO(-121);

		int iY = checked_range_cast<int>(fBag);


		for (int i = 0; i < player.bag; i++)
		{
			tx = pos->x - iPosX;
			ty = pos->y - iPosY - iY;

			tx = checked_range_cast<long>(tx / INTERFACE_RATIO(32));
			ty = checked_range_cast<long>(ty / INTERFACE_RATIO(32));

			if ((tx >= 0) && ((size_t)tx < INVENTORY_X) && (ty >= 0) && ((size_t)ty < INVENTORY_Y))
			{
				if ((inventory[i][tx][ty].io)
					&&	(inventory[i][tx][ty].io->GameFlags & GFLAG_INTERACTIVITY))
				{
					HERO_OR_SECONDARY = 1;
					return (inventory[i][tx][ty].io);
				}

				return NULL;
			}

			iY += checked_range_cast<int>(INTERFACE_RATIO(121));
		}
	}

	return NULL;
}


//*************************************************************************************
// INTERACTIVE_OBJ * GetInventoryObj_INVENTORYUSE(EERIE_S2D * pos)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//
//*************************************************************************************
INTERACTIVE_OBJ * GetInventoryObj_INVENTORYUSE(Vec2s * pos)
{
	INTERACTIVE_OBJ * io = GetFromInventory(pos);

	if (io != NULL)
	{
		if (HERO_OR_SECONDARY == 2)
		{
			if (SecondaryInventory != NULL)
			{
				INTERACTIVE_OBJ * temp = (INTERACTIVE_OBJ *)SecondaryInventory->io;

				if (temp->ioflags & IO_SHOP) return NULL;
			}
		}

		return io;
	}

	if (InInventoryPos(pos)) return NULL;

	if ((io = InterClick(pos)) != NULL)
	{
		return io;
	}

	return NULL;

}

//*************************************************************************************
// void PutInFrontOfPlayer(INTERACTIVE_OBJ * io,long flag)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Puts an IO in front of the player
//*************************************************************************************
void PutInFrontOfPlayer(INTERACTIVE_OBJ * io)
{
	if (io == NULL) return;

	float t = radians(player.angle.b);
	io->pos.x = player.pos.x - (float)EEsin(t) * 80.f;
	io->pos.y = player.pos.y + 20.f; 
	io->pos.z = player.pos.z + (float)EEcos(t) * 80.f;
	io->velocity.y = 0.3f;
	io->velocity.x = 0; 
	io->velocity.z = 0; 
	io->angle.a = 0.f;
	io->angle.b = 0; 
	io->angle.g = 0.f;
	io->stopped = 0;
	io->show = SHOW_FLAG_IN_SCENE;

	if (io->obj && io->obj->pbox)
	{
		Vec3f pos, vector;
		vector.x = 0.f; 
		vector.y = 100.f;
		vector.z = 0.f;
		pos.x = io->pos.x;
		pos.y = io->pos.y;
		pos.z = io->pos.z;
		io->soundtime = 0;
		io->soundcount = 0;
		EERIE_PHYSICS_BOX_Launch(io->obj, &pos, &vector);
	}
}

//*************************************************************************************
// void IO_Drop_Item(INTERACTIVE_OBJ * io_src,INTERACTIVE_OBJ * io)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   forces "io_scr" IO to drop "io" item with physics
//*************************************************************************************
void IO_Drop_Item(INTERACTIVE_OBJ * io_src, INTERACTIVE_OBJ * io)
{
	// Validity Check
	if ((!io) || (!io_src)) return;

	float t = radians(io_src->angle.b);
	io->velocity.x = -(float)EEsin(t) * 50.f;
	io->velocity.y = 0.3f;
	io->velocity.z = (float)EEcos(t) * 50.f;
	io->angle.a = 0.f;
	io->angle.b = 0; 
	io->angle.g = 0.f;
	io->stopped = 0;
	io->show = SHOW_FLAG_IN_SCENE;

	if (io->obj && io->obj->pbox)
	{
		Vec3f pos, vector;
		vector.x = 0.f; 
		vector.y = 100.f;
		vector.z = 0.f;
		pos.x = io->pos.x;
		pos.y = io->pos.y;
		pos.z = io->pos.z;
		io->soundtime = 0;
		io->soundcount = 0;
		EERIE_PHYSICS_BOX_Launch_NOCOL(io, io->obj, &pos, &vector);
	}
}

void ForcePlayerInventoryObjectLevel(long level) {
	for(long iNbBag = 0; iNbBag < 3; iNbBag++) {
		for(size_t j = 0; j < INVENTORY_Y; j++) {
			for(size_t i = 0; i < INVENTORY_X; i++) {
				if(inventory[iNbBag][i][j].io != NULL) {
					inventory[iNbBag][i][j].io->level = (short)level;
				}
			}
		}
	}
}

PlayerInventory playerInventory;

bool PlayerInventory::insertIntoNewSlotAt(INTERACTIVE_OBJ * item, const Pos & pos) {
	
	if(pos.bag >= size_t(player.bag)) {
		return false;
	}
	
	if(pos.x + item->sizex > INVENTORY_X || pos.y + item->sizey > INVENTORY_Y) {
		return false;
	}
	
	arx_assert(item != NULL && (item->ioflags & IO_ITEM));
	
	// Check if the whole area required by this item is empty
	for(size_t j = pos.y; j < pos.y + item->sizey; j++) {
		for(size_t i = pos.x; i < pos.x + item->sizex; i++) {
			if(inventory[pos.bag][i][j].io != NULL) {
				return false;
			}
		}
	}
	
	LogDebug(" - (" << pos.bag << ',' << pos.x << ',' << pos.y << ") := "
	         << item->long_name() << " [" << item->_itemdata->count << '/'
	         << item->_itemdata->playerstacksize << "]: "
	         << int(item->sizex) << 'x' << int(item->sizey));
	
	// Insert the item at the found position
	for(size_t j = pos.y; j < (pos.y + item->sizey); j++) {
		for (size_t i = pos.x; i < (pos.x + item->sizex); i++) {
			inventory[pos.bag][i][j].io = item;
			inventory[pos.bag][i][j].show = 0;
		}
	}
	inventory[pos.bag][pos.x][pos.y].show = 1;
	
	return true;
}

PlayerInventory::Pos PlayerInventory::insertIntoNewSlot(INTERACTIVE_OBJ * item) {
	
	arx_assert(item != NULL && (item->ioflags & IO_ITEM));
	
	for(size_t bag = 0; bag < size_t(player.bag); bag++) {
		for(size_t i = 0; i < INVENTORY_X + 1 - item->sizex; i++) {
			for(size_t j = 0; j < INVENTORY_Y + 1 - item->sizey; j++) {
				
				// Ignore already used inventory slots
				if(inventory[bag][i][j].io != NULL) {
					continue;
				}
				
				Pos pos(bag, i, j);
				if(insertIntoNewSlotAt(item, pos)) {
					return pos;
				}
			}
		}
	}
	
	return Pos();
}

bool PlayerInventory::insertIntoStackAt(INTERACTIVE_OBJ * item, const Pos & pos) {
	
	if(pos.bag >= size_t(player.bag)) {
		return false;
	}
	
	if(pos.x + item->sizex > INVENTORY_X || pos.y + item->sizey > INVENTORY_Y) {
		return false;
	}
	
	arx_assert(item != NULL && (item->ioflags & IO_ITEM));
	
	INTERACTIVE_OBJ * io = inventory[pos.bag][pos.x][pos.y].io;
	
	// Ignore empty slots and different or non-stackeable items
	if(!io || io->_itemdata->playerstacksize <= 1 || !IsSameObject(item, io)) {
		return false;
	}
	
	// Ignore full stacks
	if(io->_itemdata->count >= io->_itemdata->playerstacksize) {
		return false;
	}
	
	// Get the number of items to add to the stack
	short int remainingSpace = io->_itemdata->playerstacksize - io->_itemdata->count;
	short int count = std::min(item->_itemdata->count, remainingSpace);
	
	LogDebug(" - (" << pos.bag << ',' << pos.x << ',' << pos.y << ") "
	         << io->long_name()
	         << " [" << io->_itemdata->count << '/'
	         << io->_itemdata->playerstacksize << "] += "
	         << item->long_name() << " x" << count << '/' << item->_itemdata->count);
	
	io->_itemdata->count += count, item->_itemdata->count -= count;
	
	if(item->_itemdata->count != 0) {
		// We inserted some of the items into the stack, but there was not enough
		// space for all of them.
		return false;
	}
	
	// Delete or hide the old item
	if(item->scriptload) {
		delete item;
	} else {
		item->show = SHOW_FLAG_KILLED;
	}
	return true;
}


PlayerInventory::Pos PlayerInventory::insertIntoStack(INTERACTIVE_OBJ * item) {
	
	arx_assert(item != NULL && (item->ioflags & IO_ITEM));
	
	// Try to add the items to an existing stack
	for(size_t bag = 0; bag < size_t(player.bag); bag++) {
		for(size_t i = 0; i < INVENTORY_X + 1 - item->sizex; i++) {
			for(size_t j = 0; j < INVENTORY_Y + 1 - item->sizey; j++) {
				Pos pos(bag, i, j);
				if(insertIntoStackAt(item, pos)) {
					return pos;
				}
			}
		}
	}
	
	return Pos();
}

PlayerInventory::Pos PlayerInventory::insertImpl(INTERACTIVE_OBJ * item) {
	arx_assert(item != NULL && (item->ioflags & IO_ITEM));
	if(Pos pos = insertIntoStack(item)) {
		return pos;
	}
	return insertIntoNewSlot(item);
}

PlayerInventory::Pos PlayerInventory::insertImpl(INTERACTIVE_OBJ * item, const Pos & pos) {
	arx_assert(item != NULL && (item->ioflags & IO_ITEM));
	if(insertIntoStackAt(item, pos)) {
		return pos;
	}
	if(Pos newPos = insertIntoStack(item)) {
		return newPos;
	}
	if(insertIntoNewSlotAt(item, pos)) {
		return pos;
	}
	return insertIntoNewSlot(item);
}

PlayerInventory::Pos PlayerInventory::locate(const INTERACTIVE_OBJ * item) {
	for(size_t bag = 0; bag < size_t(player.bag); bag++) {
		for(size_t i = 0; i < INVENTORY_X; i++) {
			for(size_t j = 0; j < INVENTORY_Y; j++) {
				if(inventory[bag][i][j].io == item) {
					return Pos(bag, i, j);
				}
			}
		}
	}
	return Pos();
}

void PlayerInventory::removeAt(const INTERACTIVE_OBJ * item, const Pos & pos) {
	
	arx_assert(item != NULL && (item->ioflags & IO_ITEM));
	arx_assert(pos.x + item->sizex <= INVENTORY_X);
	arx_assert(pos.y + item->sizey <= INVENTORY_Y);
	arx_assert(inventory[pos.bag][pos.x][pos.y].io == item);
	
	LogDebug(" - (" << pos.bag << ',' << pos.x << ',' << pos.y << ") remove "
	         << item->long_name() << " [" << item->_itemdata->count << '/'
	         << item->_itemdata->playerstacksize << "]: "
	         << int(item->sizex) << 'x' << int(item->sizey));
	
	for(size_t jj = pos.y; jj < (pos.y + item->sizey); jj++) {
		for(size_t ii = pos.x; ii < (pos.x + item->sizex); ii++) {
			inventory[pos.bag][ii][jj].io = NULL;
		}
	}
}

PlayerInventory::Pos PlayerInventory::remove(const INTERACTIVE_OBJ * item) {
	Pos pos = locate(item);
	if(pos) {
		removeAt(item, pos);
	}
	return pos;
}

bool PlayerInventory::insert(INTERACTIVE_OBJ * item) {
	if(!item) {
		return false;
	}
	if(item->ioflags & IO_GOLD) {
		ARX_PLAYER_AddGold(item);
		return true;
	}
	if(item->ioflags & IO_ITEM) {
		if(Pos pos = insertImpl(item)) {
			ARX_INVENTORY_Declare_InventoryIn(get(pos));
			return true;
		}
	}
	return false;
}

bool PlayerInventory::insert(INTERACTIVE_OBJ * item, const Pos & pos) {
	if(!item) {
		return false;
	}
	if(item->ioflags & IO_GOLD) {
		ARX_PLAYER_AddGold(item);
		return true;
	}
	if(item->ioflags & IO_ITEM) {
		if(Pos newPos = insertImpl(item, pos)) {
			ARX_INVENTORY_Declare_InventoryIn(get(newPos));
			return true;
		}
	}
	return false;
}

bool giveToPlayer(INTERACTIVE_OBJ * item) {
	if(playerInventory.insert(item)) {
		return true;
	} else {
		PutInFrontOfPlayer(item);
		return false;
	}
}

bool giveToPlayer(INTERACTIVE_OBJ * item, const PlayerInventory::Pos & pos) {
	if(playerInventory.insert(item, pos)) {
		return true;
	} else {
		PutInFrontOfPlayer(item);
		return false;
	}
}

namespace {
//! Compare items by their size and name
struct ItemSizeComaparator {
	bool operator()(const INTERACTIVE_OBJ * a, const INTERACTIVE_OBJ * b) const {
		int sizea = a->sizex * a->sizey * a->sizey;
		int sizeb = b->sizex * b->sizey * b->sizey;
		if(sizea != sizeb) {
			return (sizea > sizeb);
		}
		int locname = a->locname.compare(b->locname);
		if(locname != 0) {
			return (locname < 0);
		}
		int name = a->long_name().compare(b->long_name());
		if(name != 0) {
			return (name < 0);
		}
		return (a < b);
	}
};
} // anonymous namespace

void PlayerInventory::optimize() {
	
	LogDebug("collecting items");
	
	// Collect all inventory items
	vector<INTERACTIVE_OBJ *> items;
	for(size_t bag = 0; bag < size_t(player.bag); bag++) {
		for(size_t j = 0 ; j < INVENTORY_Y; j++) {
			for(size_t i = 0 ; i < INVENTORY_X; i++) {
				INTERACTIVE_OBJ * io = inventory[bag][i][j].io;
				if(io && inventory[bag][i][j].show) {
					items.push_back(io);
					removeAt(io, Pos(bag, i, j));
				}
				inventory[bag][i][j].io = NULL;
				inventory[bag][i][j].show = 0;
			}
		}
	}
	
	// Sort the items by their size and name
	std::sort(items.begin(), items.end(), ItemSizeComaparator());
	
	LogDebug("sorting");
#ifdef _DEBUG
	BOOST_FOREACH(const INTERACTIVE_OBJ * item, items) {
		LogDebug(" - " << item->long_name() << ": "
		         << int(item->sizex) << 'x' << int(item->sizey));
	}
#endif
	
	LogDebug("putting back items");
	
	// Now put the items back into the inventory
	BOOST_FOREACH(INTERACTIVE_OBJ * item, items) {
		if(!insertImpl(item)) {
			// TODO: oops - there was no space for the item
			// ideally this should not happen, but the current sorting algorithm does not
			// guarantee that the resulting order is at least as good as the existing one
			LogDebug("could not insert " << item->long_name() << " after sorting inventory");
			PutInFrontOfPlayer(item);
		}
	}
}

//*************************************************************************************
// bool CanBePutInInventory(INTERACTIVE_OBJ * io)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   tries to put an object in player inventory
//*************************************************************************************
// TODO replace remaining uses of this with playerInventory.insert()
bool CanBePutInInventory(INTERACTIVE_OBJ * io)
{
	if (io == NULL) return false;

	if (io->ioflags & IO_MOVABLE) return false;

	if(io->ioflags & IO_GOLD) {
		ARX_PLAYER_AddGold(io);
		return true;
	}

	long sx, sy;
	long i, j, k, l;

	sx = io->sizex;
	sy = io->sizey;

	// on essaie de le remettre à son ancienne place --------------------------
	if (sInventory == 1 &&
	        (sInventoryX >= 0) &&
	        ((size_t)sInventoryX <= INVENTORY_X - sx) &&
	        (sInventoryY >= 0) &&
	        ((size_t)sInventoryY <= INVENTORY_Y - sy))
	{
		j = sInventoryY;
		i = sInventoryX;

		// first try to stack -------------------------------------------------
		if (player.bag)
		{
			for (int iNbBag = 0; iNbBag < player.bag; iNbBag++)
			{

				INTERACTIVE_OBJ * ioo = inventory[iNbBag][i][j].io;

				if ((ioo)
				        &&	(ioo->_itemdata->playerstacksize > 1)
				        &&	(IsSameObject(io, ioo)))
				{
					if (ioo->_itemdata->count < ioo->_itemdata->playerstacksize)
					{
						ioo->_itemdata->count += io->_itemdata->count;

						if (ioo->_itemdata->count > ioo->_itemdata->playerstacksize)
						{
							io->_itemdata->count = ioo->_itemdata->count - ioo->_itemdata->playerstacksize;
							ioo->_itemdata->count = ioo->_itemdata->playerstacksize;
						}
						else io->_itemdata->count = 0;

						if (!io->_itemdata->count)
						{
							if (io->scriptload)
							{
								for (long ii = 0; ii < inter.nbmax; ii++)
								{
									if (inter.iobj[ii] == io)
									{
										ReleaseInter(inter.iobj[ii]);

										if (DRAGINTER == io)
											Set_DragInter(NULL);

										io = inter.iobj[ii] = NULL;
									}
								}
							}
							else
							{
								io->show = SHOW_FLAG_KILLED;
							}
						}

						ARX_INVENTORY_Declare_InventoryIn(ioo);
						sInventory = -1;
						return true;
					}
				}
			}
		}


		if (player.bag)
		{
			for (int iNbBag = 0; iNbBag < player.bag; iNbBag++)
			{
				if (inventory[iNbBag][i][j].io == NULL)
				{
					bool valid = true;

					if ((sx == 0) || (sy == 0)) valid = false;

					for (k = j; k < j + sy; k++)
						for (l = i; l < i + sx; l++)
						{
							if (inventory[iNbBag][l][k].io != NULL) valid = false;
						}

					if (valid)
					{
						for (k = j; k < j + sy; k++)
							for (l = i; l < i + sx; l++)
							{
								inventory[iNbBag][l][k].io = io;
								inventory[iNbBag][l][k].show = 0;
							}

						inventory[iNbBag][i][j].show = 1;
						ARX_INVENTORY_Declare_InventoryIn(io);
						sInventory = -1;
						return true;
					}
				}
			}
		}
	}


	if(player.bag) {
		for (int iNbBag = 0; iNbBag < player.bag; iNbBag++) {
			for(size_t i = 0; i <= INVENTORY_X - sx; i++) {
				for(size_t j = 0; j <= INVENTORY_Y - sy; j++) {
					
					INTERACTIVE_OBJ * ioo = inventory[iNbBag][i][j].io;

					if ((ioo)
					        &&	(ioo->_itemdata->playerstacksize > 1)
					        &&	(IsSameObject(io, ioo)))
					{
						if (ioo->_itemdata->count < ioo->_itemdata->playerstacksize)
						{

							ioo->_itemdata->count += io->_itemdata->count;


							if (ioo->_itemdata->count > ioo->_itemdata->playerstacksize)
							{
								io->_itemdata->count = ioo->_itemdata->count - ioo->_itemdata->playerstacksize;
								ioo->_itemdata->count = ioo->_itemdata->playerstacksize;
							}
							else io->_itemdata->count = 0;

							if (!io->_itemdata->count) 
							{
								if (io->scriptload)
								{
									for (long ii = 0; ii < inter.nbmax; ii++)
									{
										if (inter.iobj[ii] == io)
										{
											ReleaseInter(inter.iobj[ii]);

											if (DRAGINTER == io)
												Set_DragInter(NULL);

											io = inter.iobj[ii] = NULL;
										}
									}
								}
								else
								{
									io->show = SHOW_FLAG_KILLED;
								}

								ARX_INVENTORY_Declare_InventoryIn(ioo);
								return true;
							}
						}
					}
				}
			}
		}
	}
	
	if(player.bag) {
		for(int iNbBag = 0; iNbBag < player.bag; iNbBag++) {
			for(size_t i = 0; i <= INVENTORY_X - sx; i++) {
				for(size_t j = 0; j <= INVENTORY_Y - sy; j++) {
					
					if (inventory[iNbBag][i][j].io == NULL)
					{
						bool valid = true;

						if ((sx == 0) || (sy == 0)) valid = false;

						for (size_t k = j; k < j + sy; k++)
							for (size_t l = i; l < i + sx; l++)
							{
								if (inventory[iNbBag][l][k].io != NULL) valid = false;
							}

						if (valid)
						{
							for (size_t k = j; k < j + sy; k++)
								for (size_t l = i; l < i + sx; l++)
								{
									inventory[iNbBag][l][k].io = io;
									inventory[iNbBag][l][k].show = 0;
								}

							inventory[iNbBag][i][j].show = 1;
							ARX_INVENTORY_Declare_InventoryIn(io);
							return true;
						}
					}
				}
			}
		}
	}
	
	return false;
}

//*************************************************************************************
// bool CanBePutInSecondaryInventory(INVENTORY_DATA * id,INTERACTIVE_OBJ * io,long * xx,long * yy)
//------------------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Tries to put an object in secondary inventory
//*************************************************************************************
bool CanBePutInSecondaryInventory(INVENTORY_DATA * id, INTERACTIVE_OBJ * io, long * xx, long * yy)
{
	if (!id) return false;

	if (!io) return false;

	if (io->ioflags & IO_MOVABLE) return false;

	long sx, sy;
	long i, j, k, l;

	*xx = -1;
	*yy = -1;

	sx = io->sizex;
	sy = io->sizey;

	// on essaie de le remettre à son ancienne place
	if (sInventory == 2 &&
	        (sInventoryX >= 0) &&
	        (sInventoryX <= id->sizex - sx) &&
	        (sInventoryY >= 0) &&
	        (sInventoryY <= id->sizey - sy))
	{
		j = sInventoryY;
		i = sInventoryX;
		// first try to stack

		
		INTERACTIVE_OBJ * ioo = id->slot[i][j].io;

		if (ioo)
			if ((ioo->_itemdata->playerstacksize > 1) &&
			        (IsSameObject(io, ioo)))
			{
				if ((ioo->_itemdata->count < ioo->_itemdata->playerstacksize)
				        && (ioo->durability == io->durability))
				{
					if (io->ioflags & IO_GOLD)
					{
						ioo->_itemdata->price += io->_itemdata->price;
					}
					else
					{
						ioo->_itemdata->count += io->_itemdata->count;
						ioo->scale = 1.f;
					}

					if (io->scriptload)
					{
						for (long ii = 0; ii < inter.nbmax; ii++)
						{
							if (inter.iobj[ii] == io)
							{
								ReleaseInter(inter.iobj[ii]);
							}
						}
					}
					else
					{
						io->show = SHOW_FLAG_KILLED;
					}

					sInventory = -1;
					return true;
				}
			}
		

		ioo = id->slot[i][j].io;

		if (!ioo)
		{
			long valid = 1;

			if ((sx == 0) || (sy == 0)) valid = 0;

			for (k = j; k < j + sy; k++)
				for (l = i; l < i + sx; l++)
				{
					if (id->slot[l][k].io != NULL)
					{
						valid = 0;
						break;
					}
				}

			if (valid)
			{
				for (k = j; k < j + sy; k++)
					for (l = i; l < i + sx; l++)
					{
						id->slot[l][k].io = io;
						id->slot[l][k].show = 0;
					}

				id->slot[i][j].show = 1;
				*xx = i;
				*yy = j;
				sInventory = -1;
				return true;
			}
		}
	}

	for (j = 0; j <= id->sizey - sy; j++)
		for (i = 0; i <= id->sizex - sx; i++)
		{
			INTERACTIVE_OBJ * ioo = id->slot[i][j].io;

			if (ioo)
				if ((ioo->_itemdata->playerstacksize > 1) &&
				        (IsSameObject(io, ioo)))
				{
					if ((ioo->_itemdata->count < ioo->_itemdata->playerstacksize)
					        && (ioo->durability == io->durability))
					{
						if (io->ioflags & IO_GOLD)
						{
							ioo->_itemdata->price += io->_itemdata->price;
						}
						else
						{
							ioo->_itemdata->count += io->_itemdata->count;
							ioo->scale = 1.f;
						}

						if (io->scriptload)
						{
							for (long ii = 0; ii < inter.nbmax; ii++)
							{
								if (inter.iobj[ii] == io)
								{
									ReleaseInter(inter.iobj[ii]);
								}
							}
						}
						else
						{
							io->show = SHOW_FLAG_KILLED;
						}

						return true;
					}
				}
		}

	for (j = 0; j <= id->sizey - sy; j++)
		for (i = 0; i <= id->sizex - sx; i++)
		{
			INTERACTIVE_OBJ * ioo = id->slot[i][j].io;

			if (!ioo)
			{
				long valid = 1;

				if ((sx == 0) || (sy == 0)) valid = 0;

				for (k = j; k < j + sy; k++)
					for (l = i; l < i + sx; l++)
					{
						if (id->slot[l][k].io != NULL)
						{
							valid = 0;
							break;
						}
					}

				if (valid)
				{
					for (k = j; k < j + sy; k++)
						for (l = i; l < i + sx; l++)
						{
							id->slot[l][k].io = io;
							id->slot[l][k].show = 0;
						}

					id->slot[i][j].show = 1;
					*xx = i;
					*yy = j;
					return true;
				}
			}
		}

	*xx = -1;
	*yy = -1;

	return false;
}

//*************************************************************************************
// bool PutInInventory()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Try to put DRAGINTER object in an inventory
//*************************************************************************************
bool PutInInventory()
{
	// Check Validity
	if ((!DRAGINTER)
	        ||	(DRAGINTER->ioflags & IO_MOVABLE))
		return false;

	short tx, ty;
	long sx, sy;
	long i, j;
 

	tx = ty = 0;

	sx = DRAGINTER->sizex;
	sy = DRAGINTER->sizey;

	// Check for backpack Icon
	if (MouseInRect((float)DANAESIZX - 35, (float)DANAESIZY - 113, (float)DANAESIZX - 35 + 32, (float)DANAESIZY - 113 + 32))
	{
		if (CanBePutInInventory(DRAGINTER))
		{
			if (DRAGINTER)
				DRAGINTER->show = SHOW_FLAG_IN_INVENTORY;

			ARX_SOUND_PlayInterface(SND_INVSTD);
			Set_DragInter(NULL);
		}

		return false;
	}

	// First Look for Identical Item...
	if ((SecondaryInventory != NULL) && (InSecondaryInventoryPos(&DANAEMouse)))
	{
		INTERACTIVE_OBJ * io =	(INTERACTIVE_OBJ *)SecondaryInventory->io;

		float fcos = ARX_INTERACTIVE_GetPrice(DRAGINTER, io) / 3.0f; //>>1;
		long cos = checked_range_cast<long>(fcos);
		cos *= DRAGINTER->_itemdata->count;
		fcos = cos + cos * ((float)player.Full_Skill_Intuition) * 0.005f;
		cos = checked_range_cast<long>(fcos);


		if (io->ioflags & IO_SHOP)
		{
			if(!io->shop_category.empty() && DRAGINTER->groups.find(io->shop_category) == DRAGINTER->groups.end())
				return false;

			if (cos <= 0) return false;
		}

		INTERACTIVE_OBJ * ioo;

		if (io->ioflags & IO_SHOP) // SHOP
		{

			// Check shop group
			for (j = 0; j < SecondaryInventory->sizey; j++)
				for (i = 0; i < SecondaryInventory->sizex; i++)
				{
					ioo = (INTERACTIVE_OBJ *)SecondaryInventory->slot[i][j].io;

					if (ioo)
					{
						if (IsSameObject(DRAGINTER, ioo))
						{
							ioo->_itemdata->count += DRAGINTER->_itemdata->count;
							ioo->scale = 1.f;

							if (DRAGINTER->scriptload)
							{
								for (long ii = 0; ii < inter.nbmax; ii++)
								{
									if (inter.iobj[ii] == DRAGINTER)
									{
										ReleaseInter(inter.iobj[ii]);
										inter.iobj[ii] = NULL;
										Set_DragInter(NULL);
									}
								}
							}
							else
							{
								DRAGINTER->show = SHOW_FLAG_KILLED;
								Set_DragInter(NULL);
							}

							ARX_PLAYER_AddGold(cos);
							ARX_SOUND_PlayInterface(SND_GOLD);
							ARX_SOUND_PlayInterface(SND_INVSTD);
							return true;
						}
					}
				}
		}



		tx = DANAEMouse.x + static_cast<short>(InventoryX) - SHORT_INTERFACE_RATIO(2);
		ty = DANAEMouse.y - SHORT_INTERFACE_RATIO(13);
		tx = tx / SHORT_INTERFACE_RATIO(32);
		ty = ty / SHORT_INTERFACE_RATIO(32);


		if ((tx <= SecondaryInventory->sizex - sx) && (ty <= SecondaryInventory->sizey - sy))
		{

			float fcos = ARX_INTERACTIVE_GetPrice(DRAGINTER, io) / 3.0f;
			long cos = checked_range_cast<long>(fcos);
			cos *= DRAGINTER->_itemdata->count;
			fcos = cos + cos * ((float)player.Full_Skill_Intuition) * 0.005f;
			cos = checked_range_cast<long>(fcos);

			for (j = 0; j < sy; j++)
				for (i = 0; i < sx; i++)
				{
					if (SecondaryInventory->slot[tx+i][ty+j].io != NULL)  
					{
						long xx, yy;
						DRAGINTER->show = SHOW_FLAG_IN_INVENTORY;

						//Superposition d'objets
						INTERACTIVE_OBJ * ioo = SecondaryInventory->slot[tx+i][ty+j].io;

						if ((ioo->_itemdata->playerstacksize > 1) &&
						        (IsSameObject(DRAGINTER, ioo)) &&
						        (ioo->_itemdata->count < ioo->_itemdata->playerstacksize))
						{
							ioo->_itemdata->count += DRAGINTER->_itemdata->count;

							if (ioo->_itemdata->count > ioo->_itemdata->playerstacksize)
							{
								DRAGINTER->_itemdata->count = ioo->_itemdata->count - ioo->_itemdata->playerstacksize;
								ioo->_itemdata->count = ioo->_itemdata->playerstacksize;
							}
							else DRAGINTER->_itemdata->count = 0;
						}

						if (DRAGINTER->_itemdata->count)
						{
							if (CanBePutInSecondaryInventory(SecondaryInventory, DRAGINTER, &xx, &yy))
							{
								if (io->ioflags & IO_SHOP) // SHOP
								{
									ARX_PLAYER_AddGold(cos);
									ARX_SOUND_PlayInterface(SND_GOLD);
								}
							}
							else return false;
						}

						ARX_SOUND_PlayInterface(SND_INVSTD);
						Set_DragInter(NULL);
						return true;
					}
				}

			if(DRAGINTER->ioflags & IO_GOLD) {
				ARX_PLAYER_AddGold(DRAGINTER);
				Set_DragInter(NULL);
				return true;
			}

			for (j = 0; j < sy; j++)
				for (i = 0; i < sx; i++)
				{
					SecondaryInventory->slot[tx+i][ty+j].io = DRAGINTER;
					SecondaryInventory->slot[tx+i][ty+j].show = 0;
				}

			if (io->ioflags & IO_SHOP) // SHOP
			{
				player.gold += cos;
				ARX_SOUND_PlayInterface(SND_GOLD);
			}

			SecondaryInventory->slot[tx][ty].show = 1;
			DRAGINTER->show = SHOW_FLAG_IN_INVENTORY;
			ARX_SOUND_PlayInterface(SND_INVSTD);
			Set_DragInter(NULL);
			return true;
		}
	}

	if (!(player.Interface & INTER_INVENTORY) && !(player.Interface & INTER_INVENTORYALL))
		return false;

	if (InventoryY != 0) return false;

	if (!InPlayerInventoryPos(&DANAEMouse)) return false;

	int iBag = 0;


	float fCenterX	= DANAECENTERX - INTERFACE_RATIO(320) + INTERFACE_RATIO(35);
	float fSizY		= DANAESIZY - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY);

	short iPosX = checked_range_cast<short>(fCenterX);
	short iPosY = checked_range_cast<short>(fSizY);


	if (player.Interface & INTER_INVENTORY)
	{

		tx = DANAEMouse.x - iPosX;
		ty = DANAEMouse.y - iPosY;
		tx = tx / SHORT_INTERFACE_RATIO(32); 
		ty = ty / SHORT_INTERFACE_RATIO(32); 


		if ((tx >= 0) && (tx <= 16 - sx) && (ty >= 0) && (ty <= 3 - sy))
			iBag = sActiveInventory;
		else return false;
	}
	else
	{
		bool bOk = false;


		float fBag	= (player.bag - 1) * INTERFACE_RATIO(-121);

		short iY = checked_range_cast<short>(fBag);



		//We must enter the for-loop to initialyze tx/ty
		arx_assert(0 < player.bag);


		for (int i = 0; i < player.bag; i++)
		{
			tx = DANAEMouse.x - iPosX;
			ty = DANAEMouse.y - iPosY - iY; 

			if ((tx >= 0) && (ty >= 0))
			{

				tx = tx / SHORT_INTERFACE_RATIO(32); 
				ty = ty / SHORT_INTERFACE_RATIO(32); 

				if ((tx >= 0) && (tx <= 16 - sx) && (ty >= 0) && (ty <= 3 - sy))
				{
					bOk = true;
					iBag = i;
					break;
				}
			}


			float fRatio	= INTERFACE_RATIO(121);

			iY += checked_range_cast<short>(fRatio);

		}

		if (!bOk)
			return false;
	}

	if(DRAGINTER->ioflags & IO_GOLD) {
		ARX_PLAYER_AddGold(DRAGINTER);
		Set_DragInter(NULL);
		return true;
	}

	for (j = 0; j < sy; j++)
		for (i = 0; i < sx; i++)
		{
			INTERACTIVE_OBJ * ioo = inventory[iBag][tx+i][ty+j].io;

			if (ioo != NULL)
			{
				ARX_INVENTORY_IdentifyIO(ioo);

				if ((ioo->_itemdata->playerstacksize > 1) &&
				        (IsSameObject(DRAGINTER, ioo)) &&
				        (ioo->_itemdata->count < ioo->_itemdata->playerstacksize))
				{
					ioo->_itemdata->count += DRAGINTER->_itemdata->count;

					if (ioo->_itemdata->count > ioo->_itemdata->playerstacksize)
					{
						DRAGINTER->_itemdata->count = ioo->_itemdata->count - ioo->_itemdata->playerstacksize;
						ioo->_itemdata->count = ioo->_itemdata->playerstacksize;
					}
					else DRAGINTER->_itemdata->count = 0;

					ioo->scale = 1.f;
					ARX_INVENTORY_Declare_InventoryIn(DRAGINTER);

					if (!DRAGINTER->_itemdata->count)
					{
						if (DRAGINTER->scriptload)
						{
							for (long ii = 0; ii < inter.nbmax; ii++)
							{
								if (inter.iobj[ii] == DRAGINTER)
								{
									ReleaseInter(inter.iobj[ii]);
									inter.iobj[ii] = NULL;
									Set_DragInter(NULL);
								}
							}
						}
						else
						{
							DRAGINTER->show = SHOW_FLAG_KILLED;
							Set_DragInter(NULL);
						}
					}

					ARX_SOUND_PlayInterface(SND_INVSTD);
					return true;
				}

				return false;
			}
		}

	for (j = 0; j < sy; j++)
		for (i = 0; i < sx; i++)
		{
			inventory[iBag][tx+i][ty+j].io = DRAGINTER;
			inventory[iBag][tx+i][ty+j].show = 0;
		}

	inventory[iBag][tx][ty].show = 1;

	ARX_INVENTORY_Declare_InventoryIn(DRAGINTER);
	ARX_SOUND_PlayInterface(SND_INVSTD);
	DRAGINTER->show = SHOW_FLAG_IN_INVENTORY;
	Set_DragInter(NULL);
	return true;
}
//*************************************************************************************
// bool InSecondaryInventoryPos(EERIE_S2D * pos)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Returns true if xx,yy is a position in secondary inventory
//*************************************************************************************
bool InSecondaryInventoryPos(Vec2s * pos)
{
	if (SecondaryInventory != NULL)
	{
		short tx, ty;

		tx = pos->x + checked_range_cast<short>(InventoryX) - SHORT_INTERFACE_RATIO(2);
		ty = pos->y - SHORT_INTERFACE_RATIO(13);
		tx = tx / SHORT_INTERFACE_RATIO(32);
		ty = ty / SHORT_INTERFACE_RATIO(32);


		if ((tx < 0) || (tx >= SecondaryInventory->sizex)) return false;

		if ((ty < 0) || (ty >= SecondaryInventory->sizey)) return false;

		return true;
	}

	return false;
}

//*************************************************************************************
// bool InPlayerInventoryPos(EERIE_S2D * pos)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Returns true if xx,yy is a position in player inventory
//*************************************************************************************
bool InPlayerInventoryPos(Vec2s * pos)
{
	if (PLAYER_INTERFACE_HIDE_COUNT) return false;


	float fCenterX	= DANAECENTERX - INTERFACE_RATIO(320) + INTERFACE_RATIO(35);
	float fSizY		= DANAESIZY - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY);

	short iPosX = checked_range_cast<short>(fCenterX);
	short iPosY = checked_range_cast<short>(fSizY);

	short tx, ty;

	if (player.Interface & INTER_INVENTORY)
	{
		tx = pos->x - iPosX;
		ty = pos->y - iPosY;//-2;

		if ((tx >= 0) && (ty >= 0))
		{
			tx = tx / SHORT_INTERFACE_RATIO(32);
			ty = ty / SHORT_INTERFACE_RATIO(32);

			if ((tx >= 0) && ((size_t)tx <= INVENTORY_X) && (ty >= 0) && ((size_t)ty < INVENTORY_Y))
				return true;
			else
				return false;
		}
	}
	
	else if (player.Interface & INTER_INVENTORYALL)
	{
		float fBag	= (player.bag - 1) * INTERFACE_RATIO(-121);

		short iY = checked_range_cast<short>(fBag);

		if ((
		            (pos->x >= iPosX) &&
		            (pos->x <= iPosX + INVENTORY_X * INTERFACE_RATIO(32)) &&
		            (pos->y >= iPosY + iY) &&
		            (pos->y <= DANAESIZY)))
			return true;

		for (int i = 0; i < player.bag; i++)
		{
			tx = pos->x - iPosX;
			ty = pos->y - iPosY - iY;

			if ((tx >= 0) && (ty >= 0))
			{
				tx = tx / SHORT_INTERFACE_RATIO(32);
				ty = ty / SHORT_INTERFACE_RATIO(32);

				if ((tx >= 0) && ((size_t)tx <= INVENTORY_X) && (ty >= 0) && ((size_t)ty < INVENTORY_Y))
					return true;
			}

			float fRatio	= INTERFACE_RATIO(121);

			iY = checked_range_cast<short>(iY + fRatio);
		}
	}

	return false;
}
//*************************************************************************************
// bool InInventoryPos(EERIE_S2D * pos)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Returns true if "pos" is a position in player inventory or in SECONDARY inventory
//*************************************************************************************
bool InInventoryPos(Vec2s * pos)
{
	if (InSecondaryInventoryPos(pos))
		return true;

	return (InPlayerInventoryPos(pos));
}

//*************************************************************************************
// bool IsFlyingOverInventory(EERIE_S2D * pos)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   returns true if cursor is flying over any inventory
//*************************************************************************************
bool IsFlyingOverInventory(Vec2s * pos)
{
	//	if(eMouseState==MOUSE_IN_WORLD) return false;

	if (SecondaryInventory != NULL)
	{

		short tx = pos->x + checked_range_cast<short>(InventoryX) - SHORT_INTERFACE_RATIO(2);
		short ty = pos->y - SHORT_INTERFACE_RATIO(13);
		tx /= SHORT_INTERFACE_RATIO(32);
		ty /= SHORT_INTERFACE_RATIO(32);


		if ((tx >= 0) && (tx <= SecondaryInventory->sizex) && (ty >= 0) && (ty <= SecondaryInventory->sizey))
			return true;
	}

	return InPlayerInventoryPos(pos);
}

//*************************************************************************************
// INTERACTIVE_OBJ * GetFromInventory(EERIE_S2D * pos)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Returns IO under position xx,yy in any INVENTORY or NULL if no IO
//   was found
//*************************************************************************************
INTERACTIVE_OBJ * GetFromInventory(Vec2s * pos)
{
	HERO_OR_SECONDARY = 0;

	if (!IsFlyingOverInventory(pos))
		return NULL;

	if (SecondaryInventory != NULL)
	{
		short tx = pos->x + checked_range_cast<short>(InventoryX) - SHORT_INTERFACE_RATIO(2);
		short ty = pos->y - SHORT_INTERFACE_RATIO(13);

		if ((tx >= 0) && (ty >= 0))
		{
			tx = tx / SHORT_INTERFACE_RATIO(32); 
			ty = ty / SHORT_INTERFACE_RATIO(32); 

			if ((tx >= 0) && (tx <= SecondaryInventory->sizex)
			        && (ty >= 0) && (ty <= SecondaryInventory->sizey))
			{
				if (SecondaryInventory->slot[tx][ty].io == NULL)
					return NULL;

				if (((player.Interface & INTER_STEAL) && (!ARX_PLAYER_CanStealItem(SecondaryInventory->slot[tx][ty].io))))
					return NULL;

				INTERACTIVE_OBJ * io = SecondaryInventory->slot[tx][ty].io;

				if (!(io->GameFlags & GFLAG_INTERACTIVITY))
					return NULL;

				HERO_OR_SECONDARY = 2;
				return io;
			}
		}
	}

	return GetInventoryObj(pos);
}

//*************************************************************************************
// bool GetItemWorldPosition( INTERACTIVE_OBJ * io,EERIE_3D * pos)
//-------------------------------------------------------------------------------------
// FUNCTION:
//   Gets real world position for an IO (can be used for non items)
//   (even in an inventory or being dragged)
// RESULT:
//   Put the position in "pos". returns true if position was found
//   or false if object is invalid, or position not defined.
//*************************************************************************************
bool GetItemWorldPosition(INTERACTIVE_OBJ * io, Vec3f * pos)
{
	// Valid IO ?
	if (!io) return false;

	// Is this object being Dragged by player ?
	if (DRAGINTER == io)
	{
		// Set position to approximate center of player.
		pos->x = player.pos.x;
		pos->y = player.pos.y + 80.f; 
		pos->z = player.pos.z;
		return true;
	}

	// Not in scene ?
	if (io->show != SHOW_FLAG_IN_SCENE)
	{
		// Is it equiped ?
		if (IsEquipedByPlayer(io))
		{
			// in player inventory
			pos->x = player.pos.x;
			pos->y = player.pos.y + 80.f; 
			pos->z = player.pos.z;
			return true;
		}

		// Is it in any player inventory ?
		for(long iNbBag = 0; iNbBag < player.bag; iNbBag++) {
			for(size_t j = 0; j < INVENTORY_Y; j++) {
				for(size_t i = 0; i < INVENTORY_X; i++) {
					if(inventory[iNbBag][i][j].io == io) {
						pos->x = player.pos.x;
						pos->y = player.pos.y + 80.f; 
						pos->z = player.pos.z;
						return true;
					}
				}
			}
		}

		// Is it in any other IO inventory ?
		for (long i = 0; i < inter.nbmax; i++)
		{
			INTERACTIVE_OBJ * ioo = inter.iobj[i];

			if (ioo && ioo->inventory)
			{
				INVENTORY_DATA * id = ioo->inventory;

				for (long j = 0; j < id->sizey; j++)
					for (long k = 0; k < id->sizex; k++)
					{
						if (id->slot[k][j].io == io)
						{
							*pos = ioo->pos;
							return true;
						}
					}
			}
		}
	}

	// Default position.
	*pos = io->pos;
	return true;
}

//*************************************************************************************
// bool GetItemWorldPositionSound( INTERACTIVE_OBJ * io,EERIE_3D * pos)
//-------------------------------------------------------------------------------------
// FUNCTION:
//   Gets real world position for an IO to spawn a sound
//*************************************************************************************
bool GetItemWorldPositionSound(const INTERACTIVE_OBJ * io, Vec3f * pos)
{
	if (!io) return false;
	
	if (DRAGINTER == io) {
		ARX_PLAYER_FrontPos(pos);
		return true;
	}
	
	if (io->show != SHOW_FLAG_IN_SCENE) {
		if (IsEquipedByPlayer(io)) {
			// in player inventory
			ARX_PLAYER_FrontPos(pos);
			return true;
		}
		
		if(player.bag) {
			for(int iNbBag = 0; iNbBag < player.bag; iNbBag++) {
				for(size_t j = 0; j < INVENTORY_Y; j++) {
					for(size_t i = 0; i < INVENTORY_X; i++) {
						if(inventory[iNbBag][i][j].io == io) {
							// in player inventory
							ARX_PLAYER_FrontPos(pos);
							return true;
						}
					}
				}
			}
		}
		
		for(long i = 0; i < inter.nbmax; i++) {
			
			INTERACTIVE_OBJ * ioo = inter.iobj[i];
			
			if (ioo && ioo->inventory)
			{
				INVENTORY_DATA * id = ioo->inventory;

				for (long j = 0; j < id->sizey; j++)
					for (long k = 0; k < id->sizex; k++)
					{
						if (id->slot[k][j].io == io)
						{
							pos->x = ioo->pos.x;
							pos->y = ioo->pos.y;
							pos->z = ioo->pos.z;
							return true;
						}
					}
			}
		}
	}

	pos->x = io->pos.x;
	pos->y = io->pos.y;
	pos->z = io->pos.z;
	return true;
}

//*************************************************************************************
// void RemoveFromAllInventories(INTERACTIVE_OBJ * io)
//-------------------------------------------------------------------------------------
// FUNCTION:
//   Seeks an IO in all Inventories to remove it
//*************************************************************************************
void RemoveFromAllInventories(const INTERACTIVE_OBJ * io) {
	
	if(!io) {
		return;
	}
	
	// Seek IO in Player Inventory/ies
	playerInventory.remove(io);
	
	// Seek IO in Other IO's Inventories
	for(long i = 0; i < inter.nbmax; i++) {
		if(inter.iobj[i] != NULL) {
			if(inter.iobj[i]->inventory != NULL) {
				INVENTORY_DATA * id = inter.iobj[i]->inventory;
				
				for(long j = 0; j < id->sizey; j++) {
					for(long k = 0; k < id->sizex; k++) {
						if(id->slot[k][j].io == io) {
							id->slot[k][j].io = NULL;
							id->slot[k][j].show = 1;
						}
					}
				}
			}
		}
	}
}

//*************************************************************************************
// Seeks an IO in all Inventories to replace it by another IO
//*************************************************************************************
void CheckForInventoryReplaceMe(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * old) {
	
	for(long i = 0; i < inter.nbmax; i++) {
		if(inter.iobj[i] != NULL) {
			if(inter.iobj[i]->inventory != NULL) {
				INVENTORY_DATA * id = inter.iobj[i]->inventory;
				
				for(long j = 0; j < id->sizey; j++) {
					for(long k = 0; k < id->sizex; k++) {
						if (id->slot[k][j].io == old) {
							long xx, yy;
							if(CanBePutInSecondaryInventory(id, io, &xx, &yy)) {
								return;
							}
							PutInFrontOfPlayer(io); 
							return;
						}
					}
				}
			}
		}
	}
}

void ReplaceInAllInventories(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * ioo) {
	
	if(!io || !ioo) {
		return;
	}
	
	long ion = GetInterNum(io);
	long ioon = GetInterNum(ioo);
	
	for(long i = 0; i < inter.nbmax; i++) {
		if(inter.iobj[i] && inter.iobj[i] != inter.iobj[ion]) {
			if(inter.iobj[i]->inventory != NULL) {
				INVENTORY_DATA * id = inter.iobj[i]->inventory;
				
				for(long j = 0; j < id->sizey; j++) {
					for(long k = 0; k < id->sizex; k++) {
						if (id->slot[k][j].io == inter.iobj[ion]) {
							id->slot[k][j].io = inter.iobj[ioon];
						}
					}
				}
			}
		}
	}
}

//*************************************************************************************
// Takes an object from an inventory (be it player's or secondary inventory)
// at screen position "xx,yy"
// Puts that object in player's "hand" (cursor)
// returns true if an object was taken false elseway
//*************************************************************************************
bool TakeFromInventory(Vec2s * pos)
{
	long i, j;
	INTERACTIVE_OBJ * io = GetFromInventory(pos);
	INTERACTIVE_OBJ * ioo;

	if (io == NULL) return false;

	if (SecondaryInventory != NULL)
	{
		if (InSecondaryInventoryPos(pos))
		{
			ioo = (INTERACTIVE_OBJ *)SecondaryInventory->io;

			if (ioo->ioflags & IO_SHOP)   // SHOP !
			{
				{
					if (io->ioflags & IO_ITEM) // Just in case...
					{
						long cos = ARX_INTERACTIVE_GetPrice(io, ioo);

						float fcos	= cos - cos * ((float)player.Full_Skill_Intuition) * 0.005f;
						cos = checked_range_cast<long>(fcos);

						if (player.gold < cos)
						{
							return false;
						}

						ARX_SOUND_PlayInterface(SND_GOLD);
						player.gold -= cos;

						if (io->_itemdata->count > 1) // Multi-obj
						{
							ioo = CloneIOItem(io);
							MakeTemporaryIOIdent(ioo);
							ioo->show = SHOW_FLAG_NOT_DRAWN;
							ioo->scriptload = 1;
							ioo->_itemdata->count = 1;
							io->_itemdata->count--;
							ARX_SOUND_PlayInterface(SND_INVSTD);
							Set_DragInter(ioo);
							return true;
						}
					}
				}
			}
			else if ((io->ioflags & IO_ITEM) &&
			         (io->_itemdata->count > 1))
			{
				if (!GInput->actionPressed(CONTROLS_CUST_STEALTHMODE))
				{
					ioo = CloneIOItem(io);
					MakeTemporaryIOIdent(ioo);
					ioo->show = SHOW_FLAG_NOT_DRAWN;
					ioo->scriptload = 1;
					ioo->_itemdata->count = 1;
					io->_itemdata->count--;
					ARX_SOUND_PlayInterface(SND_INVSTD);
					Set_DragInter(ioo);
					sInventory = 2;


					float fCalcX = (pos->x + InventoryX - INTERFACE_RATIO(2)) / INTERFACE_RATIO(32);
					float fCalcY = (pos->y - INTERFACE_RATIO(13)) / INTERFACE_RATIO(32);

					sInventoryX = checked_range_cast<short>(fCalcX);
					sInventoryY = checked_range_cast<short>(fCalcY);

					//ARX_INVENTORY_Object_Out(SecondaryInventory->io, ioo);

					ARX_INVENTORY_IdentifyIO(ioo);
					return true;
				}
			}

		}

		for (j = 0; j < SecondaryInventory->sizey; j++)
			for (i = 0; i < SecondaryInventory->sizex; i++)
			{
				if (SecondaryInventory->slot[i][j].io == io)
				{
					SecondaryInventory->slot[i][j].io = NULL;
					SecondaryInventory->slot[i][j].show = 1;
					sInventory = 2;

					float fCalcX = (pos->x + InventoryX - INTERFACE_RATIO(2)) / INTERFACE_RATIO(32);
					float fCalcY = (pos->y - INTERFACE_RATIO(13)) / INTERFACE_RATIO(32);

					sInventoryX = checked_range_cast<short>(fCalcX);
					sInventoryY = checked_range_cast<short>(fCalcY);

				}
			}
	}


	float fCenterX	= DANAECENTERX - INTERFACE_RATIO(320) + INTERFACE_RATIO(35);
	float fSizY		= DANAESIZY - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY);

	int iPosX = checked_range_cast<int>(fCenterX);
	int iPosY = checked_range_cast<int>(fSizY);


	if (InPlayerInventoryPos(pos))
	{
		{
			if (!GInput->actionPressed(CONTROLS_CUST_STEALTHMODE))
				if ((io->ioflags & IO_ITEM) && (io->_itemdata->count > 1)) // Multi-obj
				{
					if (io->_itemdata->count - 1 > 0)
					{
						ioo = AddItem(io->filename);
						MakeTemporaryIOIdent(ioo);
						ioo->show = SHOW_FLAG_NOT_DRAWN;
						ioo->_itemdata->count = 1;
						io->_itemdata->count--;
						ioo->scriptload = 1;
						long ioon = GetInterNum(ioo);
						ARX_SOUND_PlayInterface(SND_INVSTD);
						Set_DragInter(inter.iobj[ioon]);
						RemoveFromAllInventories(ioo);
						sInventory = 1;


						float fX = (pos->x - iPosX) / INTERFACE_RATIO(32);
						float fY = (pos->y - iPosY) / INTERFACE_RATIO(32);

						sInventoryX = checked_range_cast<short>(fX);
						sInventoryY = checked_range_cast<short>(fY);

						SendInitScriptEvent(ioo);
						ARX_INVENTORY_IdentifyIO(ioo);
						return true;
					}
				}
		}
	}
	
	if(player.bag) {
		for(int iNbBag = 0; iNbBag < player.bag; iNbBag++) {
			for(size_t j = 0; j < INVENTORY_Y; j++) {
				for(size_t i = 0; i < INVENTORY_X; i++) {
					if (inventory[iNbBag][i][j].io == io) {
						
						inventory[iNbBag][i][j].io = NULL;
						inventory[iNbBag][i][j].show = 1;
						sInventory = 1;
						
						float fX = (pos->x - iPosX) / INTERFACE_RATIO(32);
						float fY = (pos->y - iPosY) / INTERFACE_RATIO(32);
						
						sInventoryX = checked_range_cast<short>(fX);
						sInventoryY = checked_range_cast<short>(fY);
					}
				}
			}
		}
	}
	
	Set_DragInter(io);
	
	RemoveFromAllInventories(io);
	ARX_INVENTORY_IdentifyIO(io);
	return true;
}

bool IsInPlayerInventory(INTERACTIVE_OBJ * io) {
	
	for(long iNbBag = 0; iNbBag < player.bag; iNbBag ++) {
		for(size_t j = 0; j < INVENTORY_Y; j++) {
			for(size_t i = 0; i < INVENTORY_X; i++) {
				if(inventory[iNbBag][i][j].io == io) {
					return true;
				}
			}
		}
	}
	
	return false;
}

bool IsInSecondaryInventory(INTERACTIVE_OBJ * io) {
	
	if(SecondaryInventory) {
		for(long j = 0; j < SecondaryInventory->sizey; j++) {
			for(long i = 0; i < SecondaryInventory->sizex; i++) {
				if(SecondaryInventory->slot[i][j].io == io) {
					return true;
				}
			}
		}
	}
	
	return false;
}

void SendInventoryObjectCommand(const string & _lpszText, ScriptMessage _lCommand) {
	
	if(player.bag) {
		for(int iNbBag = 0; iNbBag < player.bag; iNbBag++) {
			for(size_t j = 0; j < INVENTORY_Y; j++) {
				for(size_t i = 0; i < INVENTORY_X; i++) {
					
					if(inventory[iNbBag][i][j].io && inventory[iNbBag][i][j].io->obj) {
						INTERACTIVE_OBJ * item = inventory[iNbBag][i][j].io;
						for(size_t lTex = 0; lTex < item->obj->texturecontainer.size(); lTex++) {
							if(!item->obj->texturecontainer.empty()) {
								if(item->obj->texturecontainer[lTex]) {
									if(item->obj->texturecontainer[lTex]->m_texName == _lpszText) {
										if(item->GameFlags & GFLAG_INTERACTIVITY) {
											SendIOScriptEvent(item, _lCommand);
										}
										return;
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

INTERACTIVE_OBJ * ARX_INVENTORY_GetTorchLowestDurability() {
	
	INTERACTIVE_OBJ * io = NULL;
	
	if(player.bag) {
		for(int iNbBag = 0; iNbBag < player.bag; iNbBag++) {
			for(size_t j = 0; j < INVENTORY_Y; j++) {
				for(size_t i = 0; i < INVENTORY_X; i++) {
					if(inventory[iNbBag][i][j].io) {
						if(inventory[iNbBag][i][j].io->locname == "description_torch") {
							if(!io) {
								io = inventory[iNbBag][i][j].io;
							} else {
								if(inventory[iNbBag][i][j].io->durability < io->durability) {
									io = inventory[iNbBag][i][j].io;
								}
							}
						}
					}
				}
			}
		}
	}
	
	return io;
}

void ARX_INVENTORY_IdentifyIO(INTERACTIVE_OBJ * _pIO) {
	if(_pIO && (_pIO->ioflags & IO_ITEM) && _pIO->_itemdata->equipitem) {
		if(player.Full_Skill_Object_Knowledge + player.Full_Attribute_Mind
		   >= _pIO->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Identify_Value].value) {
			SendIOScriptEvent(_pIO, SM_IDENTIFY);
		}
	}
}

void ARX_INVENTORY_IdentifyAll() {
	
	if(player.bag) {
		for(int iNbBag = 0; iNbBag < player.bag; iNbBag++) {
			for(size_t j = 0; j < INVENTORY_Y; j++) {
				for(size_t i = 0; i < INVENTORY_X; i++) {
					INTERACTIVE_OBJ * io = inventory[iNbBag][i][j].io;
					if(io && (io->ioflags & IO_ITEM) && io->_itemdata->equipitem) {
						if(player.Full_Skill_Object_Knowledge + player.Full_Attribute_Mind
						   >= io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Identify_Value].value) {
							SendIOScriptEvent(io, SM_IDENTIFY);
						}
					}
				}
			}
		}
	}
}

extern bool bInventoryClosing;

//-----------------------------------------------------------------------------
void ARX_INVENTORY_OpenClose(INTERACTIVE_OBJ * _io)
{
	if ((_io && (SecondaryInventory == _io->inventory)) || (_io == NULL)) // CLOSING
	{
		if (SecondaryInventory && (SecondaryInventory->io != NULL))
			SendIOScriptEvent(SecondaryInventory->io, SM_INVENTORY2_CLOSE);

		InventoryDir = -1;
		TSecondaryInventory = SecondaryInventory;
		SecondaryInventory = NULL;
		EERIEMouseButton &= ~4;

		if (DRAGGING) DRAGGING = 0;
	}
	else
	{
		if (TSecondaryInventory
		        && TSecondaryInventory->io) SendIOScriptEvent(TSecondaryInventory->io, SM_INVENTORY2_CLOSE);

		InventoryDir = 1;
		TSecondaryInventory = SecondaryInventory = _io->inventory;

		if (SecondaryInventory && SecondaryInventory->io != NULL)
		{
			if (SendIOScriptEvent(SecondaryInventory->io, SM_INVENTORY2_OPEN) == REFUSE)
			{
				InventoryDir = -1;
				TSecondaryInventory = SecondaryInventory = NULL;
				return;
			}
		}

		if (player.Interface & INTER_COMBATMODE)
		{
			ARX_INTERFACE_Combat_Mode(0);
		}

		if(!config.input.autoReadyWeapon) {
			TRUE_PLAYER_MOUSELOOK_ON = false;
		}

		if (SecondaryInventory && SecondaryInventory->io
		        && (SecondaryInventory->io->ioflags & IO_SHOP))
			ARX_INVENTORY_ReOrder();

		EERIEMouseButton &= ~4;

		if (DRAGGING) DRAGGING = 0;
	}

	if (player.Interface & INTER_INVENTORYALL)
	{
		ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
		bInventoryClosing = true;
	}
}

//-----------------------------------------------------------------------------
void ARX_INVENTORY_TakeAllFromSecondaryInventory()
{
	bool bSound = false;

	if (TSecondaryInventory)
	{

		(void)checked_range_cast<short>(TSecondaryInventory->sizey);
		(void)checked_range_cast<short>(TSecondaryInventory->sizex);


		for (long j = 0; j < TSecondaryInventory->sizey; j++)
			for (long i = 0; i < TSecondaryInventory->sizex; i++)
			{
				if (TSecondaryInventory->slot[i][j].io && TSecondaryInventory->slot[i][j].show)
				{
					long sx = TSecondaryInventory->slot[i][j].io->sizex;
					long sy = TSecondaryInventory->slot[i][j].io->sizey;
					INTERACTIVE_OBJ * io = TSecondaryInventory->slot[i][j].io;

					if (!(io->ioflags & IO_GOLD))
						RemoveFromAllInventories(io);

					if(playerInventory.insert(io)) {
						bSound = true;
					} else {
						sInventory = 2;

						sInventoryX = static_cast<short>(i);
						sInventoryY = static_cast<short>(j);

						sx = i;
						sy = j;
						CanBePutInSecondaryInventory(TSecondaryInventory, io, &sx, &sy);
					}
				}
			}
	}

	if (bSound)
		ARX_SOUND_PlayInterface(SND_INVSTD);
	else
		ARX_SOUND_PlayInterface(SND_INVSTD, 0.1f);
}

//-----------------------------------------------------------------------------
void ARX_INVENTORY_ReOrder()
{
	if (TSecondaryInventory)
	{

		(void)checked_range_cast<short>(TSecondaryInventory->sizey);
		(void)checked_range_cast<short>(TSecondaryInventory->sizex);


		for (long j = 0; j < TSecondaryInventory->sizey; j++)
			for (long i = 0; i < TSecondaryInventory->sizex; i++)
			{
				if (TSecondaryInventory->slot[i][j].io && TSecondaryInventory->slot[i][j].show)
				{
					long sx = TSecondaryInventory->slot[i][j].io->sizex;
					long sy = TSecondaryInventory->slot[i][j].io->sizey;
					INTERACTIVE_OBJ * io = TSecondaryInventory->slot[i][j].io;

					RemoveFromAllInventories(io);
					long x, y;
					sInventory = 2;
					sInventoryX = 0;
					sInventoryY = 0;

					if (CanBePutInSecondaryInventory(TSecondaryInventory, io, &x, &y))
					{
					}
					else
					{
						sInventory = 2;

						sInventoryX = static_cast<short>(i);
						sInventoryY = static_cast<short>(j);

						sx = i;
						sy = j;
						CanBePutInSecondaryInventory(TSecondaryInventory, io, &sx, &sy);
					}
				}
			}
	}
}
