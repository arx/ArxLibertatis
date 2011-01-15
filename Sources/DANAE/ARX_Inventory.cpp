/*
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
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// ARX_Inventory
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Inventories Management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------------
#include "HERMESPerf.h"

#include "EERIEApp.h"
#include "EERIELight.h"
#include "EERIELinkedObj.h"
#include "EERIEMath.h"
#include "EERIEPhysicsBox.h"
#include "EERIEPoly.H"
#include "EERIETexture.H"

#include "ARX_INPUT.h"
#include "ARX_Interactive.h"
#include "ARX_Interface.h"
#include "ARX_Equipment.h"
#include "ARX_Menu2.h"
#include "ARX_Paths.h"
#include "ARX_Player.h"
#include "ARX_Script.h"
#include "ARX_Sound.h"

#include <vector>
#include <algorithm>
using namespace std;

#include <stdio.h>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

//------------------------------------------------------------------------------------
extern E_ARX_STATE_MOUSE eMouseState;
extern float InventoryX;
extern float InventoryDir;
extern long PLAYER_INTERFACE_HIDE_COUNT;
extern long	DRAGGING;

extern CMenuConfig * pMenuConfig;
extern long TRUE_PLAYER_MOUSELOOK_ON;

void ARX_INTERFACE_Combat_Mode(long i);
void ARX_INVENTORY_ReOrder();
void ARX_INVENTORY_IdentifyIO(INTERACTIVE_OBJ * _pIO);

//------------------------------------------------------------------------------------
//CInventory Inventory;
INVENTORY_SLOT		inventory[3][INVENTORY_X][INVENTORY_Y];
INVENTORY_DATA *	SecondaryInventory	= NULL;
INTERACTIVE_OBJ *	DRAGINTER			= NULL;
INTERACTIVE_OBJ *	ioSteal				= NULL;
long InventoryY			= 100;
long HERO_OR_SECONDARY	= 0;
short sActiveInventory	= 0;

// 1 player 2 secondary
short sInventory = -1;
short sInventoryX = -1;
short sInventoryY = -1;

//*************************************************************************************
// void ARX_INVENTORY_Declare_InventoryIn(INTERACTIVE_OBJ * io)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Declares an IO as entering into player Inventory
//   Sends appropriate INVENTORYIN Event to player AND concerned io.
//*************************************************************************************
void ARX_INVENTORY_Declare_InventoryIn(INTERACTIVE_OBJ * io)
{
	if (io)
	{
		if (io->ignition > 0)
		{
			if (ValidDynLight(io->ignit_light))
				DynLight[io->ignit_light].exist = 0;

			io->ignit_light = -1;

			if (io->ignit_sound != ARX_SOUND_INVALID_RESOURCE)
			{
				ARX_SOUND_Stop(io->ignit_sound);
				io->ignit_sound = ARX_SOUND_INVALID_RESOURCE;
			}

			io->ignition = 0;
		}

		EVENT_SENDER = io;
		SendIOScriptEvent(inter.iobj[0], SM_INVENTORYIN, "", NULL);
		EVENT_SENDER = inter.iobj[0];
		SendIOScriptEvent(io, SM_INVENTORYIN, "", NULL);
		EVENT_SENDER = NULL;
	}
}
//*************************************************************************************
// void ARX_INVENTORY_Declare_InventoryOut(INTERACTIVE_OBJ * io)
//-------------------------------------------------------------------------------------
// NOT USED Right Now
//*************************************************************************************
void ARX_INVENTORY_Declare_InventoryOut(INTERACTIVE_OBJ * io)
{
}
//*************************************************************************************
// void ARX_INVENTORY_Declare_Inventory_2_Out(INTERACTIVE_OBJ * io)
//-------------------------------------------------------------------------------------
// NOT USED Right Now
//*************************************************************************************
void ARX_INVENTORY_Declare_Inventory_2_Out(INTERACTIVE_OBJ * io)
{

}

//*************************************************************************************
// void CleanInventory()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Cleans Player inventory
//*************************************************************************************
void CleanInventory()
{
	for (long iNbBag = 0; iNbBag < 3; iNbBag++)
		for (long j = 0; j < INVENTORY_Y; j++)
			for (long i = 0; i < INVENTORY_X; i++)
			{
				inventory[iNbBag][i][j].io	 = NULL;
				inventory[iNbBag][i][j].show = 1;
			}

	sActiveInventory = 0;
}

extern EERIE_S2D DANAEMouse;
extern long DANAESIZX;
extern long DANAESIZY;
extern long DANAECENTERX;
extern long DANAECENTERY;

//*************************************************************************************
// INTERACTIVE_OBJ * GetInventoryObj()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//
//*************************************************************************************
INTERACTIVE_OBJ * GetInventoryObj(EERIE_S2D * pos)
{
	long tx, ty;


	float fCenterX	= DANAECENTERX - INTERFACE_RATIO(320) + INTERFACE_RATIO(35);
	float fSizY		= DANAESIZY - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY);
	ARX_CHECK_INT(fCenterX);
	ARX_CHECK_INT(fSizY);

	int iPosX = ARX_CLEAN_WARN_CAST_INT(fCenterX);
	int iPosY = ARX_CLEAN_WARN_CAST_INT(fSizY);


	if (player.Interface & INTER_INVENTORY)
	{
		tx = pos->x - iPosX; //-4
		ty = pos->y - iPosY; //-2

		if ((tx >= 0) && (ty >= 0))
		{

			ARX_CHECK_LONG(tx / INTERFACE_RATIO(32.f));
			ARX_CHECK_LONG(ty / INTERFACE_RATIO(32.f));
			tx = ARX_CLEAN_WARN_CAST_LONG(tx / INTERFACE_RATIO(32));
			ty = ARX_CLEAN_WARN_CAST_LONG(ty / INTERFACE_RATIO(32));


			if ((tx >= 0) && (tx < INVENTORY_X) && (ty >= 0) && (ty < INVENTORY_Y))
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
		ARX_CHECK_INT(fBag);

		int iY = ARX_CLEAN_WARN_CAST_INT(fBag);


		for (int i = 0; i < player.bag; i++)
		{
			tx = pos->x - iPosX;
			ty = pos->y - iPosY - iY;


			ARX_CHECK_LONG(tx / INTERFACE_RATIO(32));
			ARX_CHECK_LONG(ty / INTERFACE_RATIO(32));
			tx = ARX_CLEAN_WARN_CAST_LONG(tx / INTERFACE_RATIO(32));
			ty = ARX_CLEAN_WARN_CAST_LONG(ty / INTERFACE_RATIO(32));


			if ((tx >= 0) && (tx < INVENTORY_X) && (ty >= 0) && (ty < INVENTORY_Y))
			{
				if ((inventory[i][tx][ty].io)
				        &&	(inventory[i][tx][ty].io->GameFlags & GFLAG_INTERACTIVITY))
				{
					HERO_OR_SECONDARY = 1;
					return (inventory[i][tx][ty].io);
				}

				return NULL;
			}


			float fRatio	= INTERFACE_RATIO(121);
			ARX_CHECK_INT(iY + fRatio);

			iY	+= ARX_CLEAN_WARN_CAST_INT(fRatio);

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
INTERACTIVE_OBJ * GetInventoryObj_INVENTORYUSE(EERIE_S2D * pos)
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
//   flag=1 don't apply physics
//*************************************************************************************
void PutInFrontOfPlayer(INTERACTIVE_OBJ * io, long flag)
{
	if (io == NULL) return;

	float t = DEG2RAD(player.angle.b);
	io->pos.x = player.pos.x - (float)EEsin(t) * 80.f;
	io->pos.z = player.pos.z + (float)EEcos(t) * 80.f;
	io->pos.y = player.pos.y + 20.f; 
	io->velocity.y = 0.3f;
	io->velocity.x = 0; 
	io->velocity.z = 0; 
	io->angle.a = 0.f;
	io->angle.b = 0; 
	io->angle.g = 0.f;
	io->stopped = 0;
	io->show = SHOW_FLAG_IN_SCENE;
	EERIE_3D pos, vector;
	vector.x = 0.f; 
	vector.y = 100.f;
	vector.z = 0.f;
	pos.x = io->pos.x;
	pos.y = io->pos.y;
	pos.z = io->pos.z;

	if ((flag) && (io) && (io->obj) && (io->obj->pbox))
	{
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

	float t = DEG2RAD(io_src->angle.b);
	io->velocity.y = 0.3f;
	io->velocity.x = -(float)EEsin(t) * 50.f;
	io->velocity.z = (float)EEcos(t) * 50.f;
	io->angle.a = 0.f;
	io->angle.b = 0; 
	io->angle.g = 0.f;
	io->stopped = 0;
	io->show = SHOW_FLAG_IN_SCENE;
	EERIE_3D pos, vector;
	vector.x = 0.f; 
	vector.y = 100.f;
	vector.z = 0.f;
	pos.x = io->pos.x;
	pos.y = io->pos.y;
	pos.z = io->pos.z;

	if ((io) && (io->obj) && (io->obj->pbox))
	{
		io->soundtime = 0;
		io->soundcount = 0;
		EERIE_PHYSICS_BOX_Launch_NOCOL(io, io->obj, &pos, &vector);
	}
}

//*************************************************************************************
// 
//*************************************************************************************
void ForcePlayerInventoryObjectLevel(long level)
{
	for (long iNbBag = 0; iNbBag < 3; iNbBag++)
		for (long j = 0; j < INVENTORY_Y; j++)
			for (long i = 0; i < INVENTORY_X; i++)
			{
				if (inventory[iNbBag][i][j].io != NULL)
					inventory[iNbBag][i][j].io->level = (short)level;
			}
}

//-----------------------------------------------------------------------------

typedef struct _ATRIMAXSIZE: public greater<INTERACTIVE_OBJ *>
{

	bool operator()(const INTERACTIVE_OBJ * x, const INTERACTIVE_OBJ * y) const;


} ATRIMAXSIZE;

bool ATRIMAXSIZE::operator()(const INTERACTIVE_OBJ * x, const INTERACTIVE_OBJ * y) const

{
	int iSize0 = x->sizex * x->sizey * x->sizey;
	int iSize1 = y->sizex * y->sizey * y->sizey;

	if (iSize0 > iSize1)
	{
		return true;
	}
	else
	{
		if (iSize0 == iSize1)
		{
			int iRes = strcmp(x->locname, y->locname);	 

			if (!iRes)
			{
				return false;
			}

			return iRes < 0 ? true : false;
		}
	}

	return false;
}

bool FastInsert(INTERACTIVE_OBJ * _pIO, long _uiNumBag)
{
	//on essaye de stacker
	bool bFullStack = false;

	for (long i = 0; i < INVENTORY_X; i++)
	{
		for (long j = 0; j < INVENTORY_Y; j++)
		{
			INTERACTIVE_OBJ * ioo = inventory[_uiNumBag][i][j].io;

			if ((ioo) &&
			        (ioo->_itemdata->playerstacksize > 1) &&
			        IsSameObject(_pIO, ioo))
			{
				if (ioo->_itemdata->count < ioo->_itemdata->playerstacksize)
				{
					ioo->_itemdata->count += _pIO->_itemdata->count;

					if (ioo->_itemdata->count > ioo->_itemdata->playerstacksize)
					{
						_pIO->_itemdata->count = ioo->_itemdata->count - ioo->_itemdata->playerstacksize;
						ioo->_itemdata->count = ioo->_itemdata->playerstacksize;
						bFullStack = true;
						break;
					}
					else
					{
						_pIO->_itemdata->count = 0;
					}

					if (!_pIO->_itemdata->count)
					{
						if (_pIO->scriptload)
						{
							for (long ii = 0; ii < inter.nbmax; ii++)
							{
								if (inter.iobj[ii] == _pIO)
								{
									ReleaseInter(inter.iobj[ii]);

									if (DRAGINTER == _pIO)
									{
										Set_DragInter(NULL);
									}

									_pIO = inter.iobj[ii] = NULL;
								}
							}
						}
						else
						{
							_pIO->show = SHOW_FLAG_KILLED;
						}
					}

					return true;
				}
			}
		}

		if (bFullStack)
		{
			break;
		}
	}

	for (int i = 0; i < INVENTORY_X; i++)
	{
		for (long j = 0; j < INVENTORY_Y; j++)
		{
			if (inventory[_uiNumBag][i][j].io == NULL)
			{
				int iSizeX = _pIO->sizex;
				int iSizeY = _pIO->sizey;
				bool bFound = true;

				if (((i + iSizeX) > INVENTORY_X) ||
				        ((j + iSizeY) > INVENTORY_Y))
				{
					bFound = false;
				}

				if (bFound)
				{
					for (long jj = j; jj < (j + iSizeY); jj++)
					{
						for (long ii = i; ii < (i + iSizeX); ii++)
						{
							if (inventory[_uiNumBag][ii][jj].io)
							{
								bFound = false;
							}

							if (!bFound)
							{
								break;
							}
						}

						if (!bFound)
						{
							break;
						}
					}
				}

				if (bFound)
				{
					for (long jj = j; jj < (j + iSizeY); jj++)
					{
						for (long ii = i; ii < (i + iSizeX); ii++)
						{
							inventory[_uiNumBag][ii][jj].io = _pIO;
							inventory[_uiNumBag][ii][jj].show = (ii == i) && (jj == j) ? 1 : 0;
						}
					}

					return true;
				}
			}
		}
	}

	return false;
}

void OptmizeInventory(unsigned int _uiNumBag)
{

	ARX_CHECK_NOT_NEG(player.bag);

	if (_uiNumBag < ARX_CAST_USHORT(player.bag))
	{

		vector<INTERACTIVE_OBJ *> vIO;

		for (long j = 0 ; j < INVENTORY_Y ; j++)
		{
			for (long i = 0 ; i < INVENTORY_X ; i++)
			{
				if (inventory[_uiNumBag][i][j].io)
				{
					vIO.push_back(inventory[_uiNumBag][i][j].io);
					int iSizeX = inventory[_uiNumBag][i][j].io->sizex;
					int iSizeY = inventory[_uiNumBag][i][j].io->sizey;

					for (long jj = j ; jj < (j + iSizeY) ; jj++)
					{
						for (long ii = i ; ii < (i + iSizeX) ; ii++)
						{
							inventory[_uiNumBag][ii][jj].io = NULL;
						}
					}
				}
			}
		}

		std::sort(vIO.begin(), vIO.end(), ATRIMAXSIZE());

		vector<INTERACTIVE_OBJ *>::iterator it;

		for (it = vIO.begin(); it != vIO.end(); it++)
		{
			FastInsert(*it, _uiNumBag);
		}

		vIO.clear();
	}
}

//*************************************************************************************
// BOOL CanBePutInInventory(INTERACTIVE_OBJ * io)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   tries to put an object in player inventory
//*************************************************************************************
BOOL CanBePutInInventory(INTERACTIVE_OBJ * io)
{
	if (io == NULL) return FALSE;

	if (io->ioflags & IO_MOVABLE) return FALSE;

	if (io->ioflags & IO_GOLD)
	{
		ARX_PLAYER_AddGold(io->_itemdata->price);

		if (io->scriptload)
		{
			RemoveFromAllInventories(io);
			long i = GetInterNum(io);
			ReleaseInter(io);
			inter.iobj[i] = NULL;
		}
		else
		{
			io->show = SHOW_FLAG_KILLED;
			io->GameFlags &= ~GFLAG_ISINTREATZONE;
		}

		return TRUE;
	}

	long sx, sy;
	long i, j, k, l;

	sx = io->sizex;
	sy = io->sizey;

	// on essaie de le remettre à son ancienne place --------------------------
	if (sInventory == 1 &&
	        (sInventoryX >= 0) &&
	        (sInventoryX <= INVENTORY_X - sx) &&
	        (sInventoryY >= 0) &&
	        (sInventoryY <= INVENTORY_Y - sy))
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
						return TRUE;
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
						return TRUE;
					}
				}
			}
		}
	}


	if (player.bag)
		for (int iNbBag = 0; iNbBag < player.bag; iNbBag++)
			for (i = 0; i <= INVENTORY_X - sx; i++)
				for (j = 0; j <= INVENTORY_Y - sy; j++)			
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

								ARX_INVENTORY_Declare_InventoryIn(ioo);
								return TRUE;
							}
						}
					}
				}


	if (player.bag)
		for (int iNbBag = 0; iNbBag < player.bag; iNbBag++)
			for (i = 0; i <= INVENTORY_X - sx; i++)
				for (j = 0; j <= INVENTORY_Y - sy; j++)
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
							return TRUE;
						}
					}
				}

	return FALSE;
}

//*************************************************************************************
// BOOL CanBePutInSecondaryInventory(INVENTORY_DATA * id,INTERACTIVE_OBJ * io,long * xx,long * yy)
//------------------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Tries to put an object in secondary inventory
//*************************************************************************************
BOOL CanBePutInSecondaryInventory(INVENTORY_DATA * id, INTERACTIVE_OBJ * io, long * xx, long * yy)
{
	if (!id) return FALSE;

	if (!io) return FALSE;

	if (io->ioflags & IO_MOVABLE) return FALSE;

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
					return TRUE;
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
				return TRUE;
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

						return TRUE;
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
					return TRUE;
				}
			}
		}

	*xx = -1;
	*yy = -1;

	return FALSE;
}

//*************************************************************************************
// BOOL PutInInventory()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Try to put DRAGINTER object in an inventory
//*************************************************************************************
BOOL PutInInventory()
{
	// Check Validity
	if ((!DRAGINTER)
	        ||	(DRAGINTER->ioflags & IO_MOVABLE))
		return FALSE;

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

		return FALSE;
	}

	// First Look for Identical Item...
	if ((SecondaryInventory != NULL) && (InSecondaryInventoryPos(&DANAEMouse)))
	{
		INTERACTIVE_OBJ * io =	(INTERACTIVE_OBJ *)SecondaryInventory->io;

		float	fcos	=	ARX_INTERACTIVE_GetPrice(DRAGINTER, io) / 3.0f; //>>1;
		ARX_CHECK_LONG(fcos);
		long	cos		=	ARX_CLEAN_WARN_CAST_LONG(fcos);
		cos				*=	DRAGINTER->_itemdata->count;
		fcos	=	cos + cos * ((float)player.Full_Skill_Intuition) * 0.005f;
		ARX_CHECK_LONG(fcos);
		cos				=	ARX_CLEAN_WARN_CAST_LONG(fcos);


		if (io->ioflags & IO_SHOP)
		{
			if ((io->shop_category) && (!IsIOGroup(DRAGINTER, io->shop_category)))
				return FALSE;

			if (cos <= 0) return FALSE;
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
							return TRUE;
						}
					}
				}
		}



		tx = DANAEMouse.x + ARX_CLEAN_WARN_CAST_SHORT(InventoryX) - SHORT_INTERFACE_RATIO(2);
		ty = DANAEMouse.y - SHORT_INTERFACE_RATIO(13);
		tx = tx / SHORT_INTERFACE_RATIO(32);
		ty = ty / SHORT_INTERFACE_RATIO(32);


		if ((tx <= SecondaryInventory->sizex - sx) && (ty <= SecondaryInventory->sizey - sy))
		{

			float	fcos	=	ARX_INTERACTIVE_GetPrice(DRAGINTER, io) / 3.0f; //>>1;
			ARX_CHECK_LONG(fcos);
			long cos		=	ARX_CLEAN_WARN_CAST_LONG(fcos);
			cos *= DRAGINTER->_itemdata->count;
			fcos	=	cos + cos * ((float)player.Full_Skill_Intuition) * 0.005f;
			ARX_CHECK_LONG(fcos);
			cos				=	ARX_CLEAN_WARN_CAST_LONG(fcos);


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
							else return FALSE;
						}

						ARX_SOUND_PlayInterface(SND_INVSTD);
						Set_DragInter(NULL);
						return TRUE;
					}
				}

			if (DRAGINTER->ioflags & IO_GOLD)
			{
				ARX_PLAYER_AddGold(DRAGINTER->_itemdata->price);
				ARX_SOUND_PlayInterface(SND_GOLD);

				if (DRAGINTER->scriptload)
				{
					RemoveFromAllInventories(DRAGINTER);
	
					ReleaseInter(DRAGINTER);
				}
				else
				{
					DRAGINTER->show = SHOW_FLAG_KILLED;
					DRAGINTER->GameFlags &= ~GFLAG_ISINTREATZONE;
				}

				Set_DragInter(NULL);
				return TRUE;
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
			return TRUE;
		}
	}

	if (!(player.Interface & INTER_INVENTORY) && !(player.Interface & INTER_INVENTORYALL))
		return FALSE;

	if (InventoryY != 0) return FALSE;

	if (!InPlayerInventoryPos(&DANAEMouse)) return FALSE;

	int iBag = 0;


	float fCenterX	= DANAECENTERX - INTERFACE_RATIO(320) + INTERFACE_RATIO(35);
	float fSizY		= DANAESIZY - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY);
	ARX_CHECK_SHORT(fCenterX);
	ARX_CHECK_SHORT(fSizY);

	short iPosX = ARX_CLEAN_WARN_CAST_SHORT(fCenterX);
	short iPosY = ARX_CLEAN_WARN_CAST_SHORT(fSizY);


	if (player.Interface & INTER_INVENTORY)
	{

		tx = DANAEMouse.x - iPosX;
		ty = DANAEMouse.y - iPosY;
		tx = tx / SHORT_INTERFACE_RATIO(32); 
		ty = ty / SHORT_INTERFACE_RATIO(32); 


		if ((tx >= 0) && (tx <= 16 - sx) && (ty >= 0) && (ty <= 3 - sy))
			iBag = sActiveInventory;
		else return FALSE;
	}
	else
	{
		bool bOk = false;


		float fBag	= (player.bag - 1) * INTERFACE_RATIO(-121);
		ARX_CHECK_SHORT(fBag);

		short iY = ARX_CLEAN_WARN_CAST_SHORT(fBag);



		//We must enter the for-loop to initialyze tx/ty
		ARX_CHECK(0 < player.bag);


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
			ARX_CHECK_SHORT(iY + fRatio);

			iY	+= ARX_CLEAN_WARN_CAST_SHORT(fRatio);


		}

		if (!bOk)
			return false;
	}

	if (DRAGINTER->ioflags & IO_GOLD)
	{
		ARX_PLAYER_AddGold(DRAGINTER->_itemdata->price);
		ARX_SOUND_PlayInterface(SND_GOLD);

		if (DRAGINTER->scriptload)
		{
			RemoveFromAllInventories(DRAGINTER);
 
			ReleaseInter(DRAGINTER);
		}
		else
		{
			DRAGINTER->show = SHOW_FLAG_KILLED;
			DRAGINTER->GameFlags &= ~GFLAG_ISINTREATZONE;
		}

		Set_DragInter(NULL);
		return TRUE;
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
					return TRUE;
				}

				if (0)
					if (CanBePutInInventory(DRAGINTER))
					{
						if (DRAGINTER)
							DRAGINTER->show = SHOW_FLAG_IN_INVENTORY;

						ARX_SOUND_PlayInterface(SND_INVSTD);
						Set_DragInter(NULL);
						return TRUE;
					}

				return FALSE;
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
	return TRUE;
}
//*************************************************************************************
// BOOL InSecondaryInventoryPos(EERIE_S2D * pos)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Returns TRUE if xx,yy is a position in secondary inventory
//*************************************************************************************
BOOL InSecondaryInventoryPos(EERIE_S2D * pos)
{
	if (SecondaryInventory != NULL)
	{
		short tx, ty;


		ARX_CHECK_SHORT(InventoryX);
		tx = pos->x + ARX_CLEAN_WARN_CAST_SHORT(InventoryX) - SHORT_INTERFACE_RATIO(2);
		ty = pos->y - SHORT_INTERFACE_RATIO(13);
		tx = tx / SHORT_INTERFACE_RATIO(32);
		ty = ty / SHORT_INTERFACE_RATIO(32);


		if ((tx < 0) || (tx >= SecondaryInventory->sizex)) return FALSE;

		if ((ty < 0) || (ty >= SecondaryInventory->sizey)) return FALSE;

		return TRUE;
	}

	return FALSE;
}

//*************************************************************************************
// BOOL InPlayerInventoryPos(EERIE_S2D * pos)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Returns TRUE if xx,yy is a position in player inventory
//*************************************************************************************
BOOL InPlayerInventoryPos(EERIE_S2D * pos)
{
	if (PLAYER_INTERFACE_HIDE_COUNT) return FALSE;


	float fCenterX	= DANAECENTERX - INTERFACE_RATIO(320) + INTERFACE_RATIO(35);
	float fSizY		= DANAESIZY - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY);
	ARX_CHECK_SHORT(fCenterX);
	ARX_CHECK_SHORT(fSizY);

	short iPosX = ARX_CLEAN_WARN_CAST_SHORT(fCenterX);
	short iPosY = ARX_CLEAN_WARN_CAST_SHORT(fSizY);


	short tx, ty;

	if (player.Interface & INTER_INVENTORY)
	{
		tx = pos->x - iPosX;
		ty = pos->y - iPosY;//-2;

		if ((tx >= 0) && (ty >= 0))
		{

			tx = tx / SHORT_INTERFACE_RATIO(32);
			ty = ty / SHORT_INTERFACE_RATIO(32);


			if ((tx >= 0) && (tx <= INVENTORY_X) && (ty >= 0) && (ty < INVENTORY_Y))
				return TRUE;
			else
				return FALSE;
		}
	}
	
	else if (player.Interface & INTER_INVENTORYALL)
	{

		float fBag	= (player.bag - 1) * INTERFACE_RATIO(-121);
		ARX_CHECK_SHORT(fBag);

		short iY = ARX_CLEAN_WARN_CAST_SHORT(fBag);


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


				if ((tx >= 0) && (tx <= INVENTORY_X) && (ty >= 0) && (ty < INVENTORY_Y))
					return TRUE;
			}


			float fRatio	= INTERFACE_RATIO(121);
			ARX_CHECK_SHORT(iY + fRatio);

			iY	+= ARX_CLEAN_WARN_CAST_SHORT(fRatio);


		}
	}

	return FALSE;
}
//*************************************************************************************
// BOOL InInventoryPos(EERIE_S2D * pos)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Returns TRUE if "pos" is a position in player inventory or in SECONDARY inventory
//*************************************************************************************
BOOL InInventoryPos(EERIE_S2D * pos)
{
	if (InSecondaryInventoryPos(pos))
		return TRUE;

	return (InPlayerInventoryPos(pos));
}

//*************************************************************************************
// BOOL IsFlyingOverInventory(EERIE_S2D * pos)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   returns TRUE if cursor is flying over any inventory
//*************************************************************************************
BOOL IsFlyingOverInventory(EERIE_S2D * pos)
{
	short tx, ty;

	//	if(eMouseState==MOUSE_IN_WORLD) return false;

	if (SecondaryInventory != NULL)
	{

		ARX_CHECK_SHORT(InventoryX);
		tx = pos->x + ARX_CLEAN_WARN_CAST_SHORT(InventoryX) - SHORT_INTERFACE_RATIO(2);
		ty = pos->y - SHORT_INTERFACE_RATIO(13);
		tx = tx / SHORT_INTERFACE_RATIO(32);
		ty = ty / SHORT_INTERFACE_RATIO(32);


		if ((tx >= 0) && (tx <= SecondaryInventory->sizex) && (ty >= 0) && (ty <= SecondaryInventory->sizey))
			return TRUE;
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
INTERACTIVE_OBJ * GetFromInventory(EERIE_S2D * pos)
{
	short tx, ty;
	INTERACTIVE_OBJ * io;
	HERO_OR_SECONDARY = 0;

	if (!IsFlyingOverInventory(pos))
		return NULL;

	if (SecondaryInventory != NULL)
	{


		ARX_CHECK_SHORT(InventoryX);
		tx = pos->x + ARX_CLEAN_WARN_CAST_SHORT(InventoryX) - SHORT_INTERFACE_RATIO(2);
		ty = pos->y - SHORT_INTERFACE_RATIO(13);

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

				io = SecondaryInventory->slot[tx][ty].io;

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
// BOOL GetItemWorldPosition( INTERACTIVE_OBJ * io,EERIE_3D * pos)
//-------------------------------------------------------------------------------------
// FUNCTION:
//   Gets real world position for an IO (can be used for non items)
//   (even in an inventory or being dragged)
// RESULT:
//   Put the position in "pos". returns TRUE if position was found
//   or FALSE if object is invalid, or position not defined.
//*************************************************************************************
BOOL GetItemWorldPosition(INTERACTIVE_OBJ * io, EERIE_3D * pos)
{
	// Valid IO ?
	if (!io) return FALSE;

	// Is this object being Dragged by player ?
	if (DRAGINTER == io)
	{
		// Set position to approximate center of player.
		pos->x = player.pos.x;
		pos->y = player.pos.y + 80.f; 
		pos->z = player.pos.z;
		return TRUE;
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
			return TRUE;
		}

		// Is it in any player inventory ?
		for (long iNbBag = 0; iNbBag < player.bag; iNbBag++)
			for (long j = 0; j < INVENTORY_Y; j++)
				for (long i = 0; i < INVENTORY_X; i++)
				{
					if (inventory[iNbBag][i][j].io == io)
					{
						pos->x = player.pos.x;
						pos->y = player.pos.y + 80.f; 
						pos->z = player.pos.z;
						return TRUE;
					}
				}

		// Is it in any other IO inventory ?
		for (long i = 0; i < inter.nbmax; i++)
		{
			INTERACTIVE_OBJ * ioo = inter.iobj[i];

			if (ioo && ioo->inventory)
			{
				INVENTORY_DATA * id = (INVENTORY_DATA *)ioo->inventory;

				for (long j = 0; j < id->sizey; j++)
					for (long k = 0; k < id->sizex; k++)
					{
						if (id->slot[k][j].io == io)
						{
							Vector_Copy(pos, &ioo->pos);
							return TRUE;
						}
					}
			}
		}
	}

	// Default position.
	Vector_Copy(pos, &io->pos);
	return TRUE;
}

//*************************************************************************************
// BOOL GetItemWorldPositionSound( INTERACTIVE_OBJ * io,EERIE_3D * pos)
//-------------------------------------------------------------------------------------
// FUNCTION:
//   Gets real world position for an IO to spawn a sound
//*************************************************************************************
BOOL GetItemWorldPositionSound(INTERACTIVE_OBJ * io, EERIE_3D * pos)
{
	if (!io) return FALSE;

	long i, j, k;
	INVENTORY_DATA * id;

	if (DRAGINTER == io)
	{
		ARX_PLAYER_FrontPos(pos);
		return TRUE;
	}

	if (io->show != SHOW_FLAG_IN_SCENE)
	{
		if (IsEquipedByPlayer(io))
		{
			// in player inventory
			ARX_PLAYER_FrontPos(pos);
			return TRUE;
		}

		if (player.bag)
			for (int iNbBag = 0; iNbBag < player.bag; iNbBag++)
				for (j = 0; j < INVENTORY_Y; j++)
					for (i = 0; i < INVENTORY_X; i++)
					{
						if (inventory[iNbBag][i][j].io == io)
						{
							// in player inventory
							ARX_PLAYER_FrontPos(pos);
							return TRUE;
						}
					}

		for (i = 0; i < inter.nbmax; i++)
		{
			INTERACTIVE_OBJ * ioo = inter.iobj[i];

			if (ioo && ioo->inventory)
			{
				id = (INVENTORY_DATA *)ioo->inventory;

				for (j = 0; j < id->sizey; j++)
					for (k = 0; k < id->sizex; k++)
					{
						if (id->slot[k][j].io == io)
						{
							pos->x = ioo->pos.x;
							pos->y = ioo->pos.y;
							pos->z = ioo->pos.z;
							return TRUE;
						}
					}
			}
		}
	}

	pos->x = io->pos.x;
	pos->y = io->pos.y;
	pos->z = io->pos.z;
	return TRUE;
}

//*************************************************************************************
// void RemoveFromAllInventories(INTERACTIVE_OBJ * io)
//-------------------------------------------------------------------------------------
// FUNCTION:
//   Seeks an IO in all Inventories to remove it
//*************************************************************************************
void RemoveFromAllInventories(INTERACTIVE_OBJ * io)
{
	if (!io) return;

	long iNbBag, i, j, k;
	INVENTORY_DATA * id;
	long removed_count = 0;

	// Seek IO in Player Inventory/ies
	for (iNbBag = 0; iNbBag < player.bag; iNbBag++)
		for (j = 0; j < INVENTORY_Y; j++)
			for (i = 0; i < INVENTORY_X; i++)
			{
				if (inventory[iNbBag][i][j].io == io)
				{
					inventory[iNbBag][i][j].io = NULL;
					inventory[iNbBag][i][j].show = 1;
					removed_count++;
				}
			}

	// Seek IO in Other IO's Inventories
	for (i = 0; i < inter.nbmax; i++)
	{
		if (inter.iobj[i] != NULL)
		{
			if (inter.iobj[i]->inventory != NULL)
			{
				id = (INVENTORY_DATA *)inter.iobj[i]->inventory;

				for (j = 0; j < id->sizey; j++)
					for (k = 0; k < id->sizex; k++)
					{
						if (id->slot[k][j].io == io)
						{
							id->slot[k][j].io = NULL;
							id->slot[k][j].show = 1;
							removed_count++;
						}
					}
			}
		}
	}
}
//*************************************************************************************
// Seeks an IO in all Inventories to replace it by another IO
//*************************************************************************************
void CheckForInventoryReplaceMe(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * old)
{
	if (player.bag)
		for (int iNbBag = 0; iNbBag < player.bag; iNbBag++)
			for (long j = 0; j < INVENTORY_Y; j++)
				for (long i = 0; i < INVENTORY_X; i++)
				{
					if (inventory[iNbBag][i][j].io == old)
					{
						if (CanBePutInInventory(io)) return;

						PutInFrontOfPlayer(io, 1); 
						return;
					}
				}

	for (long i = 0; i < inter.nbmax; i++)
	{
		if (inter.iobj[i] != NULL)
		{
			if (inter.iobj[i]->inventory != NULL)
			{
				INVENTORY_DATA * id = (INVENTORY_DATA *)inter.iobj[i]->inventory;

				for (long j = 0; j < id->sizey; j++)
					for (long k = 0; k < id->sizex; k++)
					{
						if (id->slot[k][j].io == old)
						{
							long xx, yy;

							if (CanBePutInSecondaryInventory(id, io, &xx, &yy)) return;

							PutInFrontOfPlayer(io, 1); 
							return;
						}
					}
			}
		}
	}
}
void ReplaceInAllInventories(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * ioo)
{
	if ((io == NULL) || (ioo == NULL)) return;

	long i, j, k;
	INVENTORY_DATA * id;

	long ion = GetInterNum(io);
	long ioon = GetInterNum(ioo);

	if (player.bag)
		for (int iNbBag = 0; iNbBag < player.bag; iNbBag++)
			for (j = 0; j < INVENTORY_Y; j++)
				for (i = 0; i < INVENTORY_X; i++)
				{
					if (inventory[iNbBag][i][j].io == inter.iobj[ion])
					{
						inventory[iNbBag][i][j].io = inter.iobj[ioon];
					}
				}

	for (i = 0; i < inter.nbmax; i++)
	{
		if ((inter.iobj[i] != NULL)
		        &&	(inter.iobj[i] != inter.iobj[ion]))
		{
			if (inter.iobj[i]->inventory != NULL)
			{
				id = (INVENTORY_DATA *)inter.iobj[i]->inventory;

				for (j = 0; j < id->sizey; j++)
					for (k = 0; k < id->sizex; k++)
					{
						if (id->slot[k][j].io == inter.iobj[ion])
						{
							id->slot[k][j].io = inter.iobj[ioon];
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
// returns TRUE if an object was taken FALSE elseway
//*************************************************************************************
BOOL TakeFromInventory(EERIE_S2D * pos)
{
	long i, j;
	INTERACTIVE_OBJ * io = GetFromInventory(pos);
	INTERACTIVE_OBJ * ioo;

	if (io == NULL) return FALSE;

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
						ARX_CHECK_LONG(fcos);
						cos			= ARX_CLEAN_WARN_CAST_LONG(fcos);


						if (player.gold < cos)
						{
							return FALSE;
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
							ARX_INVENTORY_Declare_Inventory_2_Out(ioo);
							return TRUE;
						}
					}
				}
			}
			else if ((io->ioflags & IO_ITEM) &&
			         (io->_itemdata->count > 1))
			{
				if (!ARX_IMPULSE_Pressed(CONTROLS_CUST_STEALTHMODE))
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
					ARX_CHECK_SHORT(fCalcX);
					ARX_CHECK_SHORT(fCalcY);

					sInventoryX = ARX_CLEAN_WARN_CAST_SHORT(fCalcX);
					sInventoryY = ARX_CLEAN_WARN_CAST_SHORT(fCalcY);

					//ARX_INVENTORY_Object_Out(SecondaryInventory->io, ioo);

					ARX_INVENTORY_Declare_Inventory_2_Out(ioo);
					ARX_INVENTORY_IdentifyIO(ioo);
					return TRUE;
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
					ARX_CHECK_SHORT(fCalcX);
					ARX_CHECK_SHORT(fCalcY);

					sInventoryX = ARX_CLEAN_WARN_CAST_SHORT(fCalcX);
					sInventoryY = ARX_CLEAN_WARN_CAST_SHORT(fCalcY);

				}
			}
	}


	float fCenterX	= DANAECENTERX - INTERFACE_RATIO(320) + INTERFACE_RATIO(35);
	float fSizY		= DANAESIZY - INTERFACE_RATIO(101) + INTERFACE_RATIO_LONG(InventoryY);
	ARX_CHECK_INT(fCenterX);
	ARX_CHECK_INT(fSizY);

	int iPosX = ARX_CLEAN_WARN_CAST_INT(fCenterX);
	int iPosY = ARX_CLEAN_WARN_CAST_INT(fSizY);


	long inplayer = 0;

	if (InPlayerInventoryPos(pos))
	{
		inplayer = 1;
		{
			if (!ARX_IMPULSE_Pressed(CONTROLS_CUST_STEALTHMODE))
				if ((io->ioflags & IO_ITEM) && (io->_itemdata->count > 1)) // Multi-obj
				{
					if (io->_itemdata->count - 1 > 0)
					{
						ioo = AddItem(GDevice, io->filename);
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
						ARX_CHECK_SHORT(fX);
						ARX_CHECK_SHORT(fY);

						sInventoryX = ARX_CLEAN_WARN_CAST_SHORT(fX);
						sInventoryY = ARX_CLEAN_WARN_CAST_SHORT(fY);


						SendInitScriptEvent(ioo);
						ARX_INVENTORY_Declare_InventoryOut(ioo);
						ARX_INVENTORY_IdentifyIO(ioo);
						return TRUE;
					}
				}
		}
	}

	if (player.bag)
		for (int iNbBag = 0; iNbBag < player.bag; iNbBag++)
			for (j = 0; j < INVENTORY_Y; j++)
				for (i = 0; i < INVENTORY_X; i++)
				{
					if (inventory[iNbBag][i][j].io == io)
					{
						inventory[iNbBag][i][j].io = NULL;
						inventory[iNbBag][i][j].show = 1;
						sInventory = 1;


						float fX = (pos->x - iPosX) / INTERFACE_RATIO(32);
						float fY = (pos->y - iPosY) / INTERFACE_RATIO(32);
						ARX_CHECK_SHORT(fX);
						ARX_CHECK_SHORT(fY);

						sInventoryX = ARX_CLEAN_WARN_CAST_SHORT(fX);
						sInventoryY = ARX_CLEAN_WARN_CAST_SHORT(fY);

					}
				}

	Set_DragInter(io);

	ARX_INVENTORY_Declare_InventoryOut(io);
	RemoveFromAllInventories(io);
	ARX_INVENTORY_IdentifyIO(io);
	return TRUE;
}

//-----------------------------------------------------------------------------
BOOL IsInPlayerInventory(INTERACTIVE_OBJ * io)
{
	for (long iNbBag = 0; iNbBag < player.bag; iNbBag ++)
		for (long j = 0; j < INVENTORY_Y; j++)
			for (long i = 0; i < INVENTORY_X; i++)
			{
				if (inventory[iNbBag][i][j].io == io)
				{
					return TRUE;
				}
			}

	return FALSE;
}

//-----------------------------------------------------------------------------
BOOL IsInSecondaryInventory(INTERACTIVE_OBJ * io)
{
	if (SecondaryInventory)
	{
		for (long j = 0; j < SecondaryInventory->sizey; j++)
			for (long i = 0; i < SecondaryInventory->sizex; i++)
			{
				if (SecondaryInventory->slot[i][j].io == io)
				{
					return TRUE;
				}
			}
	}

	return FALSE;
}

//-----------------------------------------------------------------------------
void SendInventoryObjectCommand(char * _lpszText, long _lCommand)
{
	if (player.bag)
		for (int iNbBag = 0; iNbBag < player.bag; iNbBag++)
			for (long j = 0; j < INVENTORY_Y; j++)
				for (long i = 0; i < INVENTORY_X; i++)
				{
					if ((inventory[iNbBag][i][j].io)
					        &&	(inventory[iNbBag][i][j].io->obj))
						for (long lTex = 0; lTex < inventory[iNbBag][i][j].io->obj->nbmaps; lTex++)
						{
							if (inventory[iNbBag][i][j].io->obj->texturecontainer)
							{
								if (inventory[iNbBag][i][j].io->obj->texturecontainer[lTex])
								{
									if (strcmp(inventory[iNbBag][i][j].io->obj->texturecontainer[lTex]->m_texName, _lpszText) == 0)
									{
										if (inventory[iNbBag][i][j].io->GameFlags & GFLAG_INTERACTIVITY)
											SendIOScriptEvent(inventory[iNbBag][i][j].io, _lCommand, "");

										return;
									}
								}
							}
						}
				}
}

//-----------------------------------------------------------------------------
INTERACTIVE_OBJ * ARX_INVENTORY_GetTorchLowestDurability()
{
	INTERACTIVE_OBJ * io = NULL;

	if (player.bag)
		for (int iNbBag = 0; iNbBag < player.bag; iNbBag++)
			for (long j = 0; j < INVENTORY_Y; j++)
				for (long i = 0; i < INVENTORY_X; i++)
				{
					if (inventory[iNbBag][i][j].io)
					{
						if (strcmp(inventory[iNbBag][i][j].io->locname, "[description_torch]") == 0)
						{
							if (!io)
							{
								io = inventory[iNbBag][i][j].io;
							}
							else
							{
								if (inventory[iNbBag][i][j].io->durability < io->durability)
								{
									io = inventory[iNbBag][i][j].io;
								}
							}
						}
					}
				}

	return io;
}

//-----------------------------------------------------------------------------
void ARX_INVENTORY_IdentifyIO(INTERACTIVE_OBJ * _pIO)
{
	if (_pIO)
	{
		if ((_pIO) && (_pIO->ioflags & IO_ITEM) && _pIO->_itemdata->equipitem)
		{
			if (player.Full_Skill_Object_Knowledge + player.Full_Attribute_Mind
			        >= _pIO->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Identify_Value].value)
			{
				SendIOScriptEvent(_pIO, SM_IDENTIFY, "");
			}
		}
	}
}

//-----------------------------------------------------------------------------
void ARX_INVENTORY_IdentifyAll()
{
	INTERACTIVE_OBJ * io = NULL;

	if (player.bag)
		for (int iNbBag = 0; iNbBag < player.bag; iNbBag++)
			for (long j = 0; j < INVENTORY_Y; j++)
				for (long i = 0; i < INVENTORY_X; i++)
				{
					io = inventory[iNbBag][i][j].io;

					if ((io) && (io->ioflags & IO_ITEM) && io->_itemdata->equipitem)
					{
						if (player.Full_Skill_Object_Knowledge + player.Full_Attribute_Mind
						        >= io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Identify_Value].value)
						{
							SendIOScriptEvent(io, SM_IDENTIFY, "");
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
			SendIOScriptEvent((INTERACTIVE_OBJ *)SecondaryInventory->io, SM_INVENTORY2_CLOSE, "");

		InventoryDir = -1;
		TSecondaryInventory = SecondaryInventory;
		SecondaryInventory = NULL;
		EERIEMouseButton &= ~4;

		if (DRAGGING) DRAGGING = 0;
	}
	else
	{
		if (TSecondaryInventory
		        && TSecondaryInventory->io) SendIOScriptEvent((INTERACTIVE_OBJ *)TSecondaryInventory->io, SM_INVENTORY2_CLOSE, "");

		InventoryDir = 1;
		TSecondaryInventory = SecondaryInventory = (INVENTORY_DATA *)_io->inventory;

		if (SecondaryInventory && SecondaryInventory->io != NULL)
		{
			if (SendIOScriptEvent((INTERACTIVE_OBJ *)SecondaryInventory->io, SM_INVENTORY2_OPEN, "") == REFUSE)
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

		if (pMenuConfig->bAutoReadyWeapon == false)
		{
			TRUE_PLAYER_MOUSELOOK_ON &= ~1;
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

		ARX_CHECK_SHORT(TSecondaryInventory->sizey);
		ARX_CHECK_SHORT(TSecondaryInventory->sizex);


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

					if (CanBePutInInventory(io))
					{
						bSound = true;
					}
					else
					{
						sInventory = 2;

						sInventoryX = ARX_CLEAN_WARN_CAST_SHORT(i);
						sInventoryY = ARX_CLEAN_WARN_CAST_SHORT(j);

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

		ARX_CHECK_SHORT(TSecondaryInventory->sizey);
		ARX_CHECK_SHORT(TSecondaryInventory->sizex);


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

						sInventoryX = ARX_CLEAN_WARN_CAST_SHORT(i);
						sInventoryY = ARX_CLEAN_WARN_CAST_SHORT(j);

						sx = i;
						sy = j;
						CanBePutInSecondaryInventory(TSecondaryInventory, io, &sx, &sy);
					}
				}
			}
	}
}
