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
// ARX_Interactive
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Interactive Objects Management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
#define DIRECTINPUT_VERSION 0x0700
#include <dinput.h>

#include "ARX_Interactive.h"
#include "ARX_PATHS.h"
#include "ARX_FTL.h"
#include "ARX_Equipment.h"
#include "ARX_Sound.h"
#include "ARX_Spells.h"
#include "ARX_Levels.h"
#include "ARX_NPC.h"
#include "ARX_Collisions.h"
#include "ARX_Changelevel.h"
#include "ARX_Particles.h"
#include "ARX_Damages.h"
#include "ARX_Speech.h"
#include "ARX_Script.h"
#include "ARX_time.h"
#include "ARX_scene.h"
#include "ARX_menu2.h"
#include "danaedlg.h"

#include "HERMESMain.h"

#include "EERIEAnim.h"
#include "EERIELight.h"
#include "EERIEObject.H"
#include "EERIEPOLY.H"
#include "EERIELinkedObj.h"
#include "EERIECollisionSpheres.h"
#include "EERIEPhysicsBox.h"
#include "EERIEProgressive.h"
#include "EERIEClothes.h"
#include "EERIEDRAW.h"
#include "EERIEMeshTweak.h"

#include <stdlib.h>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

extern EERIE_CAMERA TCAM[];
extern long FRAME_COUNT;

#define BASE_RUBBER 1.5f

extern INTERACTIVE_OBJ * CDP_EditIO;
extern INTERACTIVE_OBJ * FlyingOverIO;
extern INTERACTIVE_OBJ * CAMERACONTROLLER;
extern TextureContainer * Movable;
extern long NODIRCREATION;
extern long SPECIAL_DRAW_INTER_SHADOW;
extern long LOOK_AT_TARGET;
extern long EXTERNALVIEW;
extern long CURRENTLEVEL;
extern long EDITMODE;
extern long CYRIL_VERSION;
extern long FINAL_RELEASE;
extern long FOR_EXTERNAL_PEOPLE;
extern long NEED_TEST_TEXT;

ARX_NODES nodes;
INTERACTIVE_OBJ * CURRENTINTER = NULL;
INTERACTIVE_OBJECTS inter;
float TREATZONE_LIMIT = 1800.f;
 
long HERO_SHOW_1ST = 1;
long TreatAllIO = 0;
long INTREATZONECOUNT = 0;
long NbIOSelected = 0;
long LastSelectedIONum = -1;
long INTERNMB = -1;
long LASTINTERCLICKNB = -1;
long FORCE_IO_INDEX = -1;
long INTER_DRAW = 0;
long INTER_COMPUTE = 0;
long ForceIODraw = 0;

float STARTED_ANGLE = 0;
void Set_DragInter(INTERACTIVE_OBJ * io)
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

//***********************************************************************************************
// ValidIONum
// Checks if an IO index number is valid
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/16)
//***********************************************************************************************
long ValidIONum(long num)
{
	if ((num <	 0)
	        ||	(num >= inter.nbmax)
	        ||	(!inter.iobj)
	        ||	(!inter.iobj[num]))
	{
		return 0;
	}

	return 1;
}
long ValidIOAddress(INTERACTIVE_OBJ * io)
{
	if (!io) return 0;

	for (long i = 0; i < inter.nbmax; i++)
	{
		if (inter.iobj[i] == io) return 1;
	}

	return 0;
}
float ARX_INTERACTIVE_fGetPrice(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * shop)
{
	if ((!io)
	        ||	(!(io->ioflags & IO_ITEM)))
		return 0;

	float durability_ratio = io->durability / io->max_durability;
	float shop_multiply = 1.f;

	if (shop)
		shop_multiply = shop->shop_multiply;

	return io->_itemdata->price * shop_multiply * durability_ratio;

}
long ARX_INTERACTIVE_GetPrice(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * shop)
{
	long lresult;
	F2L(ARX_INTERACTIVE_fGetPrice(io, shop), &lresult);
	return lresult;

}
void ARX_INTERACTIVE_ForceIOLeaveZone(INTERACTIVE_OBJ * io, long flags)
{
	ARX_PATH * op = (ARX_PATH *)io->inzone;

	if (op)
	{
		char temp[256];
		strcpy(temp, op->name);
		MakeUpcase(temp);

		if (flags & 1) // no need when being destroyed !
			SendIOScriptEvent(io, SM_LEAVEZONE, temp, NULL);

		if (op->controled[0] != 0)
		{
			long t = GetTargetByNameTarget(op->controled);

			if (t >= 0)
			{
				char texx[128];
				char tex2[128];
				strcpy(texx, GetName(io->filename));
				sprintf(tex2, "%s_%04d %s", texx, io->ident, temp);
				SendIOScriptEvent(inter.iobj[t], SM_CONTROLLEDZONE_LEAVE, tex2, NULL); 
			}
		}
	}
}
extern long FAST_RELEASE;
void ARX_INTERACTIVE_DestroyDynamicInfo(INTERACTIVE_OBJ * io)
{
	if (!io) return;

	long n = GetInterNum(io);

	ARX_CHECK_SHORT(n);
	short sN = ARX_CLEAN_WARN_CAST_SHORT(n);


	ARX_INTERACTIVE_ForceIOLeaveZone(io, 0);

	for (long i = 0; i < MAX_EQUIPED; i++)
	{
		if ((player.equiped[i])
		        &&	(player.equiped[i] == n)
		        &&	ValidIONum(player.equiped[i]))
		{
			ARX_EQUIPMENT_UnEquip(inter.iobj[0], inter.iobj[player.equiped[i]], 1);
			player.equiped[i] = 0;
		}
	}


	ARX_SPEECH_ReleaseIOSpeech(io);

	if (!FAST_RELEASE)
		EERIE_MESH_ReleaseTransPolys(io->obj);

	ARX_SCRIPT_EventStackClearForIo(io);

	if (ValidIONum(n))
	{
		for (long i = 0; i < MAX_SPELLS; i++)
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

	if (io->ioflags & IO_NPC)
	{
		// to check again later...
		long count = 50;

		while ((io->_npcdata->pathfind.pathwait == 1) && count--)
		{
			Sleep(1);
		}

		if (io->_npcdata->pathfind.list) free(io->_npcdata->pathfind.list);

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
				INTERACTIVE_OBJ * ioo = (INTERACTIVE_OBJ *)eobj->linked[k].io;

				if ((ioo) && ValidIOAddress(ioo))
				{
					long ll = eobj->linked[k].lidx;
					EERIE_3D pos, vector;
					pos.x = io->obj->vertexlist3[ll].v.x;
					pos.y = io->obj->vertexlist3[ll].v.y;
					pos.z = io->obj->vertexlist3[ll].v.z;
					ioo->angle.a = rnd() * 40.f + 340.f;
					ioo->angle.b = rnd() * 360.f;
					ioo->angle.g = 0;
					vector.x = -(float)EEsin(DEG2RAD(ioo->angle.b)) * DIV2;
					vector.y = EEsin(DEG2RAD(ioo->angle.a));
					vector.z = (float)EEcos(DEG2RAD(ioo->angle.b)) * DIV2;
					ioo->soundtime = 0;
					ioo->soundcount = 0;
					EERIE_PHYSICS_BOX_Launch_NOCOL(ioo, ioo->obj, &pos, &vector, 2, &ioo->angle);
					ioo->show = 1;
					ioo->no_collide = sN;
					EERIE_LINKEDOBJ_UnLinkObjectFromObject(io->obj, ioo->obj);
				}
			}
		}
	}
}


BOOL ARX_INTERACTIVE_Attach(long n_source, long n_target, char * ap_source, char * ap_target)
{
	if (!ValidIONum(n_source)
	        ||	!ValidIONum(n_target))
		return FALSE;

	inter.iobj[n_source]->show = SHOW_FLAG_LINKED;
	EERIE_LINKEDOBJ_UnLinkObjectFromObject(inter.iobj[n_target]->obj, inter.iobj[n_source]->obj);
	return EERIE_LINKEDOBJ_LinkObjectToObject(inter.iobj[n_target]->obj,
	        inter.iobj[n_source]->obj, ap_target, ap_source, inter.iobj[n_source]);
}
void ARX_INTERACTIVE_Detach(long n_source, long n_target)
{
	if (!ValidIONum(n_source)
	        ||	!ValidIONum(n_target))
		return;

	inter.iobj[n_source]->show = SHOW_FLAG_IN_SCENE;
	EERIE_LINKEDOBJ_UnLinkObjectFromObject(inter.iobj[n_target]->obj, inter.iobj[n_source]->obj);
}

void ARX_INTERACTIVE_Show_Hide_1st(INTERACTIVE_OBJ * io, long state)
{
	if ((!io)
	        ||	(HERO_SHOW_1ST == state))
		return;

	HERO_SHOW_1ST = state;
	long grp = EERIE_OBJECT_GetSelection(io->obj, "1st");

	if (grp != -1)
	{
		for (long nn = 0; nn < io->obj->nbfaces; nn++)
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

	ARX_INTERACTIVE_HideGore(inter.iobj[0], 1);
}


void ARX_INTERACTIVE_RemoveGoreOnIO(INTERACTIVE_OBJ * io)
{
	if ((!io)
	        ||	(!io->obj)
	        ||	(!io->obj->texturecontainer))
		return;

	long gorenum = -1;

	for (long nn = 0; nn < io->obj->nbmaps; nn++)
	{
		if (io->obj->texturecontainer[nn]
		        &&	TextureContainer_Exist(io->obj->texturecontainer[nn])
		        &&	strstr(io->obj->texturecontainer[nn]->m_strName, "GORE"))
		{
			gorenum = nn;
			break;
		}
	}

	if (gorenum > -1)
		for (long nn = 0; nn < io->obj->nbfaces; nn++)
		{
			if (io->obj->facelist[nn].texid == gorenum)
			{
				io->obj->facelist[nn].facetype |= POLY_HIDE;
				io->obj->facelist[nn].texid = -1;
			}
		}
}


// flag & 1 == no unhide non-gore
void ARX_INTERACTIVE_HideGore(INTERACTIVE_OBJ * io, long flag)
{
	if ((!io)
	        ||	(!io->obj)
	        ||	(!io->obj->texturecontainer))
		return;

	if ((io == inter.iobj[0]) && (!flag & 1))
		return;

	long gorenum = -1;

	for (long nn = 0; nn < io->obj->nbmaps; nn++)
	{
		if (io->obj->texturecontainer[nn]
		        &&	TextureContainer_Exist(io->obj->texturecontainer[nn])
		        &&	strstr(io->obj->texturecontainer[nn]->m_strName, "GORE"))
		{
			gorenum = nn;
			break;
		}
	}

	if (gorenum > -1)
		for (long nn = 0; nn < io->obj->nbfaces; nn++)
		{
			//Hide Gore Polys...
			if (io->obj->facelist[nn].texid == gorenum)
				io->obj->facelist[nn].facetype |= POLY_HIDE;
			else if (!flag & 1)
				io->obj->facelist[nn].facetype &= ~POLY_HIDE;
		}
}
extern long GORE_MODE;
 
EERIE_3DOBJ * GetExistingEerie(char * file)
{
	for (long i = 1; i < inter.nbmax; i++)
	{
		if ((inter.iobj[i] != NULL)
		        &&	(!inter.iobj[i]->tweaky)
		        &&	(inter.iobj[i]->obj))
		{
			if	((!inter.iobj[i]->obj->originaltextures)
			        &&	(!strcmp(file, inter.iobj[i]->obj->file)))
			{
				return inter.iobj[i]->obj;
			}
		}
	}

	return NULL;
}
BOOL ForceNPC_Above_Ground(INTERACTIVE_OBJ * io)
{
	if (io
	        &&	(io->ioflags & IO_NPC)
	        && !(io->ioflags & IO_PHYSICAL_OFF))
	{
		io->physics.cyl.origin.x = io->pos.x;
		io->physics.cyl.origin.y = io->pos.y;
		io->physics.cyl.origin.z = io->pos.z;
 
		AttemptValidCylinderPos(&io->physics.cyl, io, CFLAG_NO_INTERCOL);
		{
			if (EEfabs(io->pos.y - io->physics.cyl.origin.y) < 45.f)
			{
				io->pos.y = io->physics.cyl.origin.y;
				return TRUE;
			}
			else return FALSE;
		}
	}

	return FALSE;
}

EERIE_3DOBJ * TheoToEerie_Fast(char * texpath, char * ficc, long flag, LPDIRECT3DDEVICE7 pd3dDevice)
{
	char fic[256];
	char pfic[256];
	File_Standardize(ficc, fic);
	File_Standardize(fic + strlen(Project.workingdir) - 1, pfic);
	EERIE_3DOBJ * ret = NULL;

	if ((ret = ARX_FTL_Load(pfic, fic, NULL)) != NULL)
	{

		if (!(flag & TTE_NO_PHYSICS_BOX))
			EERIE_PHYSICS_BOX_Create(ret);

		return ret;
	}

	if (ret = GetExistingEerie(fic))
	{
		ret = Eerie_Copy(ret);

		if (!ret) goto alternateway;
	}
	else
	{
	alternateway:
		;
		long FileSize = 0;
		unsigned char * adr;

		if (adr = (unsigned char *)PAK_FileLoadMalloc(fic, &FileSize))
		{
			ret = TheoToEerie(adr, FileSize, texpath, fic, flag, pd3dDevice, flag | TTE_NO_RESTORE); //SLOWLOAD));

			if (!ret)
			{
				free(adr);
				return NULL;
			}

			EERIE_OBJECT_CenterObjectCoordinates(ret);
			free(adr);
		}
		else return NULL;
	}

	if (FASTLOADS)
	{
		if ((ret)
		        &&	(ret->pdata))
		{
			free(ret->pdata);
			ret->pdata = NULL;
		}

		return ret;
	}

	if ((ret)
	        &&	(!(flag & TTE_NO_PDATA)))
	{
		CreateNeighbours(ret);
		EERIEOBJECT_AddProgressiveData(ret);
		EERIEOBJECT_AddClothesData(ret);
		KillNeighbours(ret);

		if (ret->cdata)
			EERIE_COLLISION_SPHERES_Create(ret); // Must be out of the Neighbours zone

		if (!(flag & TTE_NO_PHYSICS_BOX))
			EERIE_PHYSICS_BOX_Create(ret);

		ARX_FTL_Save(pfic, fic, ret);
	}

	return ret;
}

//*************************************************************************************
// Unlinks all linked objects from all IOs
//*************************************************************************************
void UnlinkAllLinkedObjects()
{
	for (long i = 0; i < inter.nbmax; i++)
	{
		if (inter.iobj[i])
		{
			EERIE_LINKEDOBJ_ReleaseData(inter.iobj[i]->obj);
		}
	}
}

void IO_UnlinkAllLinkedObjects(INTERACTIVE_OBJ * io)
{
	if (io && io->obj)
	{
		for (long k = 0; k < io->obj->nblinked; k++)
		{
			if (io->obj->linked[k].io)
			{
				INTERACTIVE_OBJ * ioo = (INTERACTIVE_OBJ *)io->obj->linked[k].io;

				if (ValidIOAddress(ioo))
					IO_Drop_Item(io, ioo);

			}
		}

		EERIE_LINKEDOBJ_ReleaseData(io->obj);
	}
}

// First is always the player
TREATZONE_IO * treatio = NULL;
long TREATZONE_CUR = 0;
long TREATZONE_MAX = 0;

void TREATZONE_Clear()
{
	TREATZONE_CUR = 0;
}

void TREATZONE_Release()
{
	if (treatio) free(treatio);

	treatio = NULL;
	TREATZONE_MAX = 0;
	TREATZONE_CUR = 0;
}

void TREATZONE_RemoveIO(INTERACTIVE_OBJ * io)
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
void TREATZONE_AddIO(INTERACTIVE_OBJ * io, long num, long flag)
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
	treatio[TREATZONE_CUR].num = num;
	TREATZONE_CUR++;
}

void CheckSetAnimOutOfTreatZone(INTERACTIVE_OBJ * io, long num)
{
	if ((io)
	        &&	(io->animlayer[num].cur_anim)
	        &&	!(io->GameFlags & GFLAG_ISINTREATZONE)
	        &&	(EEDistance3D(&io->pos, &ACTIVECAM->pos) > 2500.f))
	{

		ARX_CHECK_LONG(io->animlayer[num].cur_anim->anims[ io->animlayer[num].altidx_cur ]->anim_time - 1);
		io->animlayer[num].ctime =
		    ARX_CLEAN_WARN_CAST_LONG(io->animlayer[num].cur_anim->anims[ io->animlayer[num].altidx_cur ]->anim_time - 1);

	}
}

long GLOBAL_Player_Room = -1;
extern float fZFogEnd;
void PrepareIOTreatZone(long flag)
{
	static long status = -1;
	static EERIE_3D lastpos;

	if ((flag)
	        ||	(status == -1))
	{
		status = 0;
		lastpos.x = ACTIVECAM->pos.x;
		lastpos.y = ACTIVECAM->pos.y;
		lastpos.z = ACTIVECAM->pos.z;
	}
	else if (status == 3) status = 0;

	if (EEDistance3D(&ACTIVECAM->pos, &lastpos) > 100.f)
	{
		status = 0;
		lastpos.x = ACTIVECAM->pos.x;
		lastpos.y = ACTIVECAM->pos.y;
		lastpos.z = ACTIVECAM->pos.z;
	}

	if (status++) return;

	TREATZONE_Clear();
	long Cam_Room = ARX_PORTALS_GetRoomNumForPosition(&ACTIVECAM->pos, 1);
	GLOBAL_Player_Room = ARX_PORTALS_GetRoomNumForPosition(&player.pos, 1);
	TREATZONE_AddIO(inter.iobj[0], 0, 0);


	ARX_CHECK_SHORT(GLOBAL_Player_Room);
	short sGlobalPlayerRoom = ARX_CLEAN_WARN_CAST_SHORT(GLOBAL_Player_Room);


	for (long i = 0; i < MAX_EQUIPED; i++)
	{
		if ((player.equiped[i] != 0)
		        &&	ValidIONum(player.equiped[i]))
		{
			INTERACTIVE_OBJ * toequip = inter.iobj[player.equiped[i]];

			if (toequip)
			{
				toequip->room = sGlobalPlayerRoom;
				toequip->room_flags = 0;
			}
		}
	}

	if (DRAGINTER) TREATZONE_AddIO(DRAGINTER, GetInterNum(DRAGINTER), 0);

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

	INTREATZONECOUNT = 0;

	if (TreatAllIO)
	{
		for (long ii = 1; ii < inter.nbmax; ii++)
		{
			INTERACTIVE_OBJ * io = inter.iobj[ii];

			if (io)
			{
				io->GameFlags |= GFLAG_ISINTREATZONE;
				TREATZONE_AddIO(io, ii, 0);
			}
		}

		return; 
	}

	char treat;

	for (int i = 1; i < inter.nbmax; i++)
	{
		INTERACTIVE_OBJ * io = inter.iobj[i];

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
				float dist;

				if (Cam_Room >= 0)
				{
					if (io->show == SHOW_FLAG_TELEPORTING)
					{
						EERIE_3D pos;
						GetItemWorldPosition(io, &pos);
						dist = EEDistance3D(&ACTIVECAM->pos, &pos);
					}
					else
					{
						if (io->room_flags & 1)
							UpdateIORoom(io);

						dist = SP_GetRoomDist(&io->pos, &ACTIVECAM->pos, io->room, Cam_Room);
					}
				}
				else
				{
					if (io->show == SHOW_FLAG_TELEPORTING)
					{
						EERIE_3D pos;
						GetItemWorldPosition(io, &pos);
						dist = EEDistance3D(&ACTIVECAM->pos, &pos); //&io->pos,&pos);
					}
					else
						dist = EEDistance3D(&io->pos, &ACTIVECAM->pos);
				}
		
				if (dist < TREATZONE_LIMIT) treat = 1;
				else treat = 0;
				
			}

			if (!treat)
			{
				if (io == CAMERACONTROLLER)
					treat = 1;

				if (io == DRAGINTER)
					treat = 1;
			}

			if (io->GameFlags & GFLAG_ISINTREATZONE)
				io->GameFlags |= GFLAG_WASINTREATZONE;
			else
				io->GameFlags &= ~GFLAG_WASINTREATZONE;

			if (treat)
			{
				INTREATZONECOUNT++;
				io->GameFlags |= GFLAG_ISINTREATZONE;
				TREATZONE_AddIO(io, i, 0);

				if ((io->ioflags & IO_NPC) && (io->_npcdata->weapon))
				{
					INTERACTIVE_OBJ * iooo = (INTERACTIVE_OBJ *)io->_npcdata->weapon;
					iooo->room = io->room;
					iooo->room_flags = io->room_flags;
				}
			}
			else	io->GameFlags &= ~GFLAG_ISINTREATZONE;

			EVENT_SENDER = NULL;

			if ((io->GameFlags & GFLAG_ISINTREATZONE)
			        && (!(io->GameFlags & GFLAG_WASINTREATZONE)))
			{
				//coming back; doesn't really matter right now
				//	SendIOScriptEvent(inter.iobj[i],SM_TREATIN,"");

			}
			else if ((!(io->GameFlags & GFLAG_ISINTREATZONE))
			         &&	(io->GameFlags & GFLAG_WASINTREATZONE))
			{
				//going away;
				io->GameFlags |= GFLAG_ISINTREATZONE;

				if (SendIOScriptEvent(io, SM_TREATOUT, "") != REFUSE)
				{
					if (io->ioflags & IO_NPC)
						io->_npcdata->pathfind.flags &= ~PATHFIND_ALWAYS;

					io->GameFlags &= ~GFLAG_ISINTREATZONE;
				}
			}
		}
	}

	long M_TREAT = TREATZONE_CUR;

	for (int i = 1; i < inter.nbmax; i++)
	{
		INTERACTIVE_OBJ * io = inter.iobj[i];

		if ((io != NULL)
		        &&	!(io->GameFlags & GFLAG_ISINTREATZONE)
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
				INTERACTIVE_OBJ * ioo = treatio[ii].io;

				if (ioo)
				{
					if (EEDistance3D(&io->pos, &ioo->pos) < 300.f)
					{
						toadd = 1;
						break;
					}
				}
			}

			if (toadd)
				TREATZONE_AddIO(io, i, 1);
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
//*************************************************************************************
//*************************************************************************************
void RestoreNodeNumbers()
{
	for (long i = 0; i < nodes.nbmax; i++)
		for (long j = 0; j < MAX_LINKS; j++)
		{
			if (nodes.nodes[i].lnames[j][0] != 0)
			{
				nodes.nodes[i].link[j] = GetNumNodeByName(nodes.nodes[i].lnames[j]);
			}
		}
}
//*************************************************************************************
//*************************************************************************************
void ClearNode(long i, long first = 0)
{
	nodes.nodes[i].exist = 0;
	nodes.nodes[i].selected = 0;

	for (long j = 0; j < MAX_LINKS; j++)
	{
		if ((nodes.nodes[i].link[j] != -1) && (!first))
		{
			long k = nodes.nodes[i].link[j];

			for (long l = 0; l < MAX_LINKS; l++)
				if (nodes.nodes[k].link[l] == i)
					nodes.nodes[k].link[l] = -1;
		}

		nodes.nodes[i].lnames[j][0] = 0;
		nodes.nodes[i].link[j] = -1;
	}

	strcpy(nodes.nodes[i].name, "");
	nodes.nodes[i].pos.z = nodes.nodes[i].pos.y = nodes.nodes[i].pos.x = 0.f;
}
//*************************************************************************************
//*************************************************************************************
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

//*************************************************************************************
//*************************************************************************************
void UnselectAllNodes()
{
	for (long i = 0; i < nodes.nbmax; i++)
	{
		if (nodes.nodes[i].exist)  nodes.nodes[i].selected = 0;
	}
}
//*************************************************************************************
//*************************************************************************************
void TranslateSelectedNodes(EERIE_3D * trans)
{
	for (long i = 0; i < nodes.nbmax; i++)
	{
		if ((nodes.nodes[i].exist)
		        &&	(nodes.nodes[i].selected))
		{
			nodes.nodes[i].pos.x += trans->x;
			nodes.nodes[i].pos.y += trans->y;
			nodes.nodes[i].pos.z += trans->z;
		}
	}
}
//*************************************************************************************
//*************************************************************************************
BOOL IsLinkedNode(long i, long j)
{
	if ((!nodes.nodes[i].exist)
	        ||	(!nodes.nodes[j].exist))
		return FALSE;

	for (long k = 0; k < MAX_LINKS; k++)
	{
		if (nodes.nodes[i].link[k] == j) return TRUE;
	}

	return FALSE;
}
//*************************************************************************************
//*************************************************************************************
long CountNodes()
{
	register long count = 0;

	for (long i = 0; i < nodes.nbmax; i++)
	{
		if (nodes.nodes[i].exist)
		{
			count++;
		}
	}

	return count;
}
//*************************************************************************************
//*************************************************************************************
void AddLink(long i, long j)
{
	if ((!nodes.nodes[i].exist)
	        ||	(!nodes.nodes[j].exist))
		return;

	for (long k = 0; k < MAX_LINKS; k++)
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

	for (long k = 0; k < MAX_LINKS; k++)
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
BOOL ExistNodeName(char * name)
{
	for (long i = 0; i < nodes.nbmax; i++)
	{
		if ((nodes.nodes[i].exist)
		        &&	(!strcmp(name, nodes.nodes[i].name)))
			return TRUE;
	}

	return FALSE;
}
//*************************************************************************************
//*************************************************************************************
void MakeNodeName(long i)
{
	char name[64];
	long o;
	//float f;
	sprintf(name, "NODE_%08d", i);

	while (ExistNodeName(name))
	{
		//f=rnd()*99999999.f;
		//o=(long)f;
		F2L(rnd() * 99999999.f, &o);
		sprintf(name, "NODE_%08d", o);
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
	nodes.nodes = (ARX_NODE *)malloc(sizeof(ARX_NODE) * nodes.nbmax); //"NODES Structure"
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
//*************************************************************************************
//*************************************************************************************
 
//-----------------------------------------------------------------------------
void ReleaseNode()
{
	if (nodes.nodes)
	{
		free((void *)nodes.nodes);
		nodes.nbmax = 0;
		nodes.nodes = NULL;
	}
}
//*************************************************************************************
// Initialises Interactive Objects Main Structure (pointer list)
//*************************************************************************************
void InitInter(long nb)
{
	if (nb < 10) nb = 10;

	inter.nbmax = nb;

	if (inter.init)
	{
		free(inter.iobj);
		inter.iobj = NULL;
	}

	inter.init = 1;
	inter.iobj = (INTERACTIVE_OBJ **)malloc(sizeof(INTERACTIVE_OBJ *) * inter.nbmax); //,"Interactive Objects Structure");
	memset(inter.iobj, 0, sizeof(INTERACTIVE_OBJ *)*inter.nbmax);
}

//*************************************************************************************
//	Removes an IO loaded by a script command
//*************************************************************************************
void CleanScriptLoadedIO()
{
	for (long i = 1; i < inter.nbmax; i++)
	{
		if (inter.iobj[i] != NULL)
		{
			INTERACTIVE_OBJ * io = inter.iobj[i];

			if (io->scriptload)
			{
				RemoveFromAllInventories(io);
				ReleaseInter(inter.iobj[i]);
				inter.iobj[i] = NULL;
			}
			else inter.iobj[i]->show = SHOW_FLAG_IN_SCENE;
		}
	}
}

//*************************************************************************************
// Restores an IO to its initial status (Game start Status)
//*************************************************************************************
void RestoreInitialIOStatus()
{
	long i = 0;
	ARX_INTERACTIVE_HideGore(inter.iobj[0]);
	ARX_NPC_Behaviour_ResetAll();

	if (inter.iobj[0]) inter.iobj[0]->spellcast_data.castingspell = -1;

	for (i = 1; i < inter.nbmax; i++)
	{
		RestoreInitialIOStatusOfIO(inter.iobj[i]);
	}
}
void ARX_INTERACTIVE_USEMESH(INTERACTIVE_OBJ * io, char * temp)
{
	if ((!io)
	        ||	(!temp))
		return;

	char tex[256];
	char tex1[256];
	char tex2[256];

	if (io->ioflags & IO_NPC)	sprintf(tex2, "%sGraph\\Obj3D\\Interactive\\NPC\\%s", Project.workingdir, temp);
	else if (io->ioflags & IO_FIX)	sprintf(tex2, "%sGraph\\Obj3D\\Interactive\\FIX_INTER\\%s", Project.workingdir, temp);
	else if (io->ioflags & IO_ITEM)	sprintf(tex2, "%sGraph\\Obj3D\\Interactive\\Items\\%s", Project.workingdir, temp);
	else tex2[0] = 0;

	File_Standardize(tex2, tex);

	if (tex[0] != 0)
	{
		if (io->usemesh == NULL)
			io->usemesh = (char *)malloc(256);
		else if (!stricmp(io->usemesh, tex)) return; //already tweaked with this mesh !

		strcpy(io->usemesh, tex);

		if (io->obj != NULL)
		{
			ReleaseEERIE3DObj(io->obj);
			io->obj = NULL;
		}

		char tex2[256];
		sprintf(tex2, "%sGraph\\Obj3D\\Textures\\", Project.workingdir);
		File_Standardize(tex2, tex1);

		if (io->ioflags & IO_FIX)
			io->obj = TheoToEerie_Fast(tex1, tex, TTE_NO_NDATA | TTE_NO_PHYSICS_BOX, GDevice);
		else if (io->ioflags & IO_NPC)
			io->obj = TheoToEerie_Fast(tex1, tex, TTE_NO_PHYSICS_BOX | TTE_NPC, GDevice);
		else
			io->obj = TheoToEerie_Fast(tex1, tex, 0, GDevice);

		EERIE_COLLISION_Cylinder_Create(io);
	}
}
void ARX_INTERACTIVE_MEMO_TWEAK_CLEAR(INTERACTIVE_OBJ * io)
{
	if (io->Tweaks)
		free(io->Tweaks);

	io->Tweaks = NULL;
	io->Tweak_nb = 0;
}
void ARX_INTERACTIVE_MEMO_TWEAK(INTERACTIVE_OBJ * io, long type, char * param1, char * param2)
{
	io->Tweaks = (TWEAK_INFO *)realloc(io->Tweaks, sizeof(TWEAK_INFO) * (io->Tweak_nb + 1));
	memset(&io->Tweaks[io->Tweak_nb], 0, sizeof(TWEAK_INFO));
	io->Tweaks[io->Tweak_nb].type = type;

	if (param1) strcpy(io->Tweaks[io->Tweak_nb].param1, param1);

	if (param2) strcpy(io->Tweaks[io->Tweak_nb].param2, param2);

	io->Tweak_nb++;
}
void ARX_INTERACTIVE_APPLY_TWEAK_INFO(INTERACTIVE_OBJ * io)
{
	for (long ii = 0; ii < io->Tweak_nb; ii++)
	{
		if (io->Tweaks[ii].type == TWEAK_REMOVE)
			EERIE_MESH_TWEAK_Do(io, TWEAK_REMOVE, NULL);
		else if (io->Tweaks[ii].type == TWEAK_TYPE_SKIN)
			EERIE_MESH_TWEAK_Skin(io->obj, io->Tweaks[ii].param1, io->Tweaks[ii].param2);
		else if (io->Tweaks[ii].type == TWEAK_TYPE_ICON)
			ARX_INTERACTIVE_TWEAK_Icon(io, io->Tweaks[ii].param1);
		else if (io->Tweaks[ii].type == TWEAK_TYPE_MESH)
			ARX_INTERACTIVE_USEMESH(io, io->Tweaks[ii].param1);
		else
		{
			char temp[256];
			sprintf(temp, "%s%s", Project.workingdir, io->Tweaks[ii].param1);
			EERIE_MESH_TWEAK_Do(io, io->Tweaks[ii].type, temp);
		}
	}
}
void ARX_INTERACTIVE_ClearIODynData(INTERACTIVE_OBJ * io)
{
	if (io)
	{
		if (ValidDynLight(io->dynlight))
			DynLight[io->dynlight].exist = 0;

		io->dynlight = -1;

		if (ValidDynLight(io->halo.dynlight))
			DynLight[io->halo.dynlight].exist = 0;

		io->halo.dynlight = -1;

		if (io->symboldraw)
			free(io->symboldraw);

		io->symboldraw = NULL;
		io->spellcast_data.castingspell = -1;
	}
}
void ARX_INTERACTIVE_ClearIODynData_II(INTERACTIVE_OBJ * io)
{
	if (io)
	{
		//		ARX_SCRIPT_Timer_Clear_For_IO(io);
		if (ValidDynLight(io->dynlight))
			DynLight[io->dynlight].exist = 0;

		io->dynlight = -1;

		if (ValidDynLight(io->halo.dynlight))
			DynLight[io->halo.dynlight].exist = 0;

		io->halo.dynlight = -1;

		if (io->symboldraw)
			free(io->symboldraw);

		io->symboldraw = NULL;
		io->spellcast_data.castingspell = -1;

		if (io->shop_category)
			free(io->shop_category);

		io->shop_category = NULL;

		if (io->inventory_skin)
			free(io->inventory_skin);

		io->inventory_skin = NULL;
		ARX_INTERACTIVE_MEMO_TWEAK_CLEAR(io);
		ARX_IOGROUP_Release(io);
		ARX_INTERACTIVE_HideGore(io);
		MOLLESS_Clear(io->obj);
		ARX_SCRIPT_Timer_Clear_For_IO(io);

		if (io->stepmaterial)
			free(io->stepmaterial);

		io->stepmaterial = NULL;

		if (io->armormaterial)
			free(io->armormaterial);

		io->armormaterial = NULL;

		if (io->weaponmaterial)
			free(io->weaponmaterial);

		io->weaponmaterial = NULL;

		if (io->strikespeech)
			free(io->strikespeech);

		io->strikespeech = NULL;

		if (io->lastanimvertex)
			free(io->lastanimvertex);

		io->lastanimvertex = NULL;
		io->nb_lastanimvertex = 0;

		for (long j = 0; j < MAX_ANIMS; j++)
		{
			EERIE_ANIMMANAGER_ReleaseHandle(io->anims[j]);
			io->anims[j] = NULL;
		}

		ARX_SPELLS_RemoveAllSpellsOn(io);
		ARX_EQUIPMENT_ReleaseAll(io);

		if (io->ioflags & IO_NPC)
		{
			if (io->_npcdata->pathfind.list)
				free(io->_npcdata->pathfind.list);

			io->_npcdata->pathfind.list = NULL;
			memset(&io->_npcdata->pathfind, 0, sizeof(IO_PATHFIND));
			io->_npcdata->pathfind.truetarget = -1;
			io->_npcdata->pathfind.listnb = -1;
			ARX_NPC_Behaviour_Reset(io);
		}

		if (io->tweakerinfo != NULL)
		{
			free(io->tweakerinfo);
			io->tweakerinfo = NULL;
		}

		if (io->inventory != NULL)
		{
			INVENTORY_DATA * id = (INVENTORY_DATA *)io->inventory;

			for (long nj = 0; nj < id->sizey; nj++)
				for (long ni = 0; ni < id->sizex; ni++)
				{
					if (id->slot[ni][nj].io != NULL)
					{
						long tmp = GetInterNum(id->slot[ni][nj].io);

						if (ValidIONum(tmp))
						{
							if (inter.iobj[tmp]->scriptload)
							{
								RemoveFromAllInventories(inter.iobj[tmp]);
								ReleaseInter(inter.iobj[tmp]);
								inter.iobj[tmp] = NULL;
							}
							else inter.iobj[tmp]->show = SHOW_FLAG_KILLED;
						}
						id->slot[ni][nj].io = NULL;
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
		io->GameFlags |= GFLAG_INTERACTIVITY;

		if (io->tweaky)
		{
			ReleaseEERIE3DObj(io->obj);
			io->obj = io->tweaky;
			io->tweaky = NULL;
		}
	}
}
void ARX_INTERACTIVE_ClearAllDynData()
{
	long i = 0;
	ARX_INTERACTIVE_HideGore(inter.iobj[0]);
	ARX_NPC_Behaviour_ResetAll();

	for (i = 1; i < inter.nbmax; i++)
	{
		ARX_INTERACTIVE_ClearIODynData(inter.iobj[i]);
	}
}
void RestoreIOInitPos(INTERACTIVE_OBJ * io)
{
	if (io)
	{
		ARX_INTERACTIVE_Teleport(io, &io->initpos, 0);
		io->pos.x = io->lastpos.x = io->initpos.x;
		io->pos.y = io->lastpos.y = io->initpos.y;
		io->pos.z = io->lastpos.z = io->initpos.z;
		io->move.x = io->move.y = io->move.z = 0.f;
		io->lastmove.x = io->lastmove.y = io->lastmove.z = 0.f;
		io->angle.a = io->initangle.a;
		io->angle.b = io->initangle.b;
		io->angle.g = io->initangle.g;
	}
}
void RestoreAllIOInitPos()
{
	for (long i = 1; i < inter.nbmax; i++)
	{
		RestoreIOInitPos(inter.iobj[i]);
	}
}
void ARX_HALO_SetToNative(INTERACTIVE_OBJ * io)
{
	io->halo.color.r = io->halo_native.color.r;
	io->halo.color.g = io->halo_native.color.g;
	io->halo.color.b = io->halo_native.color.b;
	io->halo.radius = io->halo_native.radius;
	io->halo.flags = io->halo_native.flags;
}

void RestoreInitialIOStatusOfIO(INTERACTIVE_OBJ * io)
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

		io->level = io->truelevel;
		Vector_Init(&io->forcedmove);
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
		io->GameFlags &= ~GFLAG_INVISIBILITY;
		io->GameFlags &= ~GFLAG_MEGAHIDE;
		io->GameFlags &= ~GFLAG_NOGORE;
		io->GameFlags &= ~GFLAG_ISINTREATZONE;
		io->GameFlags &= ~GFLAG_PLATFORM;
		io->GameFlags &= ~GFLAG_ELEVATOR;
		io->GameFlags &= ~GFLAG_HIDEWEAPON;
		io->GameFlags &= ~GFLAG_NOCOMPUTATION;
		io->GameFlags &= ~GFLAG_INTERACTIVITYHIDE;
		io->GameFlags &= ~GFLAG_DOOR;
		io->GameFlags &= ~GFLAG_GOREEXPLODE;
		io->invisibility = 0.f;
		io->rubber = BASE_RUBBER;
		io->scale = 1.f;
		Vector_Init(&io->move);
		io->type_flags = 0;
		io->sound = -1;
		io->soundtime = 0;
		io->soundcount = 0;
		io->material = 11;
		io->collide_door_time = 0;
		io->ouch_time = 0;
		io->dmg_sum = 0;
		io->ignition = 0.f;
		io->ignit_light = -1;
		io->ignit_sound = ARX_SOUND_INVALID_RESOURCE;

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

		io->physics.cyl.origin.x = io->pos.x;
		io->physics.cyl.origin.y = io->pos.y;
		io->physics.cyl.origin.z = io->pos.z;
		io->physics.cyl.radius = io->original_radius;
		io->physics.cyl.height = io->original_height;

		io->fall = 0;
		io->show = SHOW_FLAG_IN_SCENE;
		io->targetinfo = TARGET_NONE;
		io->spellcast_data.castingspell = -1;
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
			io->_npcdata->blood_color = 0xFFFF0000;
			io->_npcdata->stare_factor = 1.f;

			io->_npcdata->weapon = NULL;
			io->_npcdata->weaponname[0] = 0;
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
			io->_npcdata->movemode = 0;
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
			io->collision = 1;
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

void ARX_INTERACTIVE_TWEAK_Icon(INTERACTIVE_OBJ * io, char * s1)
{
	if ((!io)
	        ||	(!s1))
		return;

	char icontochange[HERMES_PATH_SIZE];

	sprintf(icontochange, io->filename);
	RemoveName(icontochange);
	strcat(icontochange, s1);
	SetExt(icontochange, ".bmp");

	D3DTextr_CreateTextureFromFile(icontochange, Project.workingdir, 0, D3DTEXTR_NO_REFINEMENT, EERIETEXTUREFLAG_LOADSCENE_RELEASE);

	if (GDevice) D3DTextr_Restore(icontochange, GDevice);

	TextureContainer * tc;
	tc = D3DTextr_GetSurfaceContainer(icontochange);

	if (tc == NULL)
	{
		MakeDir(icontochange, "Graph\\Interface\\misc\\Default[Icon].bmp");
		D3DTextr_CreateTextureFromFile(icontochange, Project.workingdir, 0, D3DTEXTR_NO_REFINEMENT);

		if (GDevice) D3DTextr_Restore(icontochange, GDevice);

		tc = D3DTextr_GetSurfaceContainer(icontochange);
	}

	if (tc != NULL)
	{
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

//*************************************************************************************
// Count IO number ignoring ScriptLoaded IOs
//*************************************************************************************
long GetNumberInterWithOutScriptLoadForLevel(long level)
{
	register long count = 0;

	for (long i = 1; i < inter.nbmax; i++)
	{
		if ((inter.iobj[i] != NULL) && (!inter.iobj[i]->scriptload)
		        && (inter.iobj[i]->truelevel == level)) count++;
	}

	return count;
}
//*************************************************************************************
// Clears All Inter From Memory
//*************************************************************************************
void FreeAllInter()
{
	for (long i = 1; i < inter.nbmax; i++) //ignoring Player.
	{
		if (inter.iobj[i] != NULL)
		{
			ReleaseInter(inter.iobj[i]);
			inter.iobj[i] = NULL;
		}
	}

	inter.nbmax = 1;
	inter.iobj = (INTERACTIVE_OBJ **)realloc(inter.iobj, sizeof(INTERACTIVE_OBJ *) * inter.nbmax);
}

//*************************************************************************************
// Creates a new free Interactive Object
//*************************************************************************************
INTERACTIVE_OBJ * CreateFreeInter(long num)
{
	long i;
	long tocreate = -1;

	if (num == 0) //used to create player
	{
		tocreate = 0;

		if (inter.iobj[0] != NULL) return NULL;

		goto create;
	}

	if (FORCE_IO_INDEX != -1)
		tocreate = FORCE_IO_INDEX;
	else for (i = 1; i < inter.nbmax; i++) // ignoring player
		{			
			if (!inter.iobj[i])
			{
				tocreate = i;
				break;
			}
		}

	if (tocreate == -1)
	{
		inter.nbmax++;
		inter.iobj = (INTERACTIVE_OBJ **)realloc(inter.iobj, sizeof(INTERACTIVE_OBJ *) * inter.nbmax);
		tocreate = inter.nbmax - 1;
		inter.iobj[tocreate] = NULL;
	}

	if (tocreate != -1)
	{
	create:
		;
		i = tocreate;
		INTERACTIVE_OBJ * io;
		//todo free

	retry_allocation:
		;
		inter.iobj[i] = (INTERACTIVE_OBJ *)malloc(sizeof(INTERACTIVE_OBJ)); 

		if (!inter.iobj[i])
		{
			if (HERMES_Memory_Emergency_Out(sizeof(INTERACTIVE_OBJ), "Interactive_Object_Alloc"))
				goto retry_allocation;
		}

		memset(inter.iobj[i], 0, sizeof(INTERACTIVE_OBJ));
		io = inter.iobj[i];
		io->room_flags = 1;
		io->room = -1;
		io->no_collide = -1;
		io->shop_multiply = 1.f;
		io->Tweaks = NULL;
		io->Tweak_nb = 0;
		io->ignition = 0.f;
		io->ignit_light = -1;
		io->ignit_sound = ARX_SOUND_INVALID_RESOURCE;

		if (CURRENTLEVEL == -1) CURRENTLEVEL = GetLevelNumByName(LastLoadedScene);

		io->truelevel = io->level = (short)CURRENTLEVEL;
		io->usemesh = NULL;
		io->lastspeechflag = 2;
		io->damagedata = -1;
		io->flarecount = 0;
		io->rubber = BASE_RUBBER;

		io->halo_native.color.r = 0.2f;
		io->halo_native.color.g = 0.5f;
		io->halo_native.color.b = 1.f;
		io->halo_native.radius = 45.f;
		io->halo_native.flags = 0;
		io->halo_native.dynlight = -1;
		ARX_HALO_SetToNative(io);
		io->halo.dynlight = -1;

		io->symboldraw = NULL;
		io->dynlight = -1;
		io->sfx_flag = 0;

		io->max_durability = io->durability = 100.f;
		io->speed_modif = 0.f;
		io->basespeed = 1.f;
		io->frameloss = 0.f;

		io->GameFlags |= GFLAG_NEEDINIT;
		io->poisonous = 0;
		io->poisonous_count = 0;
		io->secretvalue = -1;

		// Already set to 0 by memset
		io->infracolor.b = 1.f;
		io->show = SHOW_FLAG_IN_SCENE;
		io->sound = -1;
		io->sizex = 1;
		io->sizey = 1;

		io->GameFlags |= GFLAG_INTERACTIVITY;
		io->scale = 1.f;
		io->stopped = 1;

		io->changeanim = -1;
		io->bbox1.x = -1;
		io->bbox1.y = -1;
		io->bbox2.x = -1;
		io->bbox2.y = -1;

		ARX_SCRIPT_SetMainEvent(io, "MAIN");
		io->targetinfo = TARGET_NONE;

		io->weight = 1.f;
		memset(io->animlayer, 0, sizeof(ANIM_USE)*MAX_ANIM_LAYERS);
		FORCE_IO_INDEX = -1;
		return (io);

	}

	FORCE_IO_INDEX = -1;
	return NULL;
}
// Be careful with this func...
INTERACTIVE_OBJ * CloneIOItem(INTERACTIVE_OBJ * src)
{
	INTERACTIVE_OBJ * dest;
	dest = AddItem(GDevice, src->filename);

	if (!dest) return NULL;

	SendInitScriptEvent(dest);
	dest->inv = src->inv;
	dest->sizex = src->sizex;
	dest->sizey = src->sizey;
	ReleaseEERIE3DObj(dest->obj);
	dest->obj = Eerie_Copy(src->obj);
	CloneLocalVars(dest, src);
	dest->_itemdata->price = src->_itemdata->price;
	dest->_itemdata->maxcount = src->_itemdata->maxcount;
	dest->_itemdata->count = src->_itemdata->count;
	dest->_itemdata->food_value = src->_itemdata->food_value;
	dest->_itemdata->stealvalue = src->_itemdata->stealvalue;
	dest->_itemdata->playerstacksize = src->_itemdata->playerstacksize;
	dest->_itemdata->LightValue = src->_itemdata->LightValue;

	if (src->_itemdata->equipitem)
	{
		dest->_itemdata->equipitem = (IO_EQUIPITEM *)malloc(sizeof(IO_EQUIPITEM));
		memcpy(dest->_itemdata->equipitem, src->_itemdata->equipitem, sizeof(IO_EQUIPITEM));
	}

	strcpy(dest->locname, src->locname);

	if ((dest->obj->pbox == NULL) && (src->obj->pbox))
	{
		dest->obj->pbox = (PHYSICS_BOX_DATA *)malloc(sizeof(PHYSICS_BOX_DATA));
		memcpy(dest->obj->pbox, src->obj->pbox, sizeof(PHYSICS_BOX_DATA));
		dest->obj->pbox->vert = (PHYSVERT *)malloc(sizeof(PHYSVERT) * src->obj->pbox->nb_physvert);
		memcpy(dest->obj->pbox->vert, src->obj->pbox->vert, sizeof(PHYSVERT)*src->obj->pbox->nb_physvert);
	}

	return dest;
}
bool ARX_INTERACTIVE_ConvertToValidPosForIO(INTERACTIVE_OBJ * io, EERIE_3D * target)
{
	EERIE_CYLINDER phys;

	if (io && (io != inter.iobj[0]))
	{
		phys.height = io->original_height * io->scale;
		phys.radius = io->original_radius * io->scale;
	}
	else
	{
		phys.height = -200;
		phys.radius = 50;
	}

	phys.origin.x = target->x;
	phys.origin.y = target->y;
	phys.origin.z = target->z;
	long count = 0;
	float modx, modz;

	while (count < 600)
	{
		modx = -EEsin(count) * (float)count * DIV3;
		modz = EEcos(count) * (float)count * DIV3;
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
void ARX_INTERACTIVE_TeleportBehindTarget(INTERACTIVE_OBJ * io)
{
	if (!io) return;

	if (ARX_SCRIPT_GetSystemIOScript(io, "_R_A_T_") < 0)
	{
		long num = ARX_SCRIPT_Timer_GetFree();

		if (num != -1)
		{
			long t = GetInterNum(io);
			ActiveTimers++;
			scr_timer[num].es = NULL;
			scr_timer[num].exist = 1;
			scr_timer[num].io = io;
			F2L(rnd() * 3000 + 3000, &scr_timer[num].msecs);
			scr_timer[num].namelength = 8;
			scr_timer[num].name = (char *)malloc(8);
			strcpy(scr_timer[num].name, "_R_A_T_");
			scr_timer[num].pos = -1; 
			scr_timer[num].tim = ARXTimeUL();
			scr_timer[num].times = 1;
			inter.iobj[t]->show = SHOW_FLAG_TELEPORTING;
			AddRandomSmoke(io, 10);
			ARX_PARTICLES_Add_Smoke(&io->pos, 3, 20);
			EERIE_3D pos;
			pos.x = inter.iobj[t]->pos.x;
			pos.y = inter.iobj[t]->pos.y + inter.iobj[t]->physics.cyl.height * DIV2;
			pos.z = inter.iobj[t]->pos.z;
			io->room_flags |= 1;
			io->room = -1;
			ARX_PARTICLES_Add_Smoke(&pos, 3, 20);
			MakeCoolFx(&io->pos);
			io->GameFlags |= GFLAG_INVISIBILITY;
			FRAME_COUNT = 0;
		}
	}
}
void ResetVVPos(INTERACTIVE_OBJ * io)
{
	if ((io) && (io->ioflags & IO_NPC))
		io->_npcdata->vvpos = io->pos.y;
}
void ComputeVVPos(INTERACTIVE_OBJ * io)
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
		float mul = 1.f;

		if (fdiff > 120.f) 
		{
			fdiff = 120.f;
		}
		else
		{
			mul = ((fdiff * DIV120) * 0.9f + 0.6f);

			if ((eediff < 15.f))
			{
				float val = (float)FrameDiff * DIV4 * mul;

				if (eediff < 10.f)
					val *= DIV10;
				else
				{
					float ratio = (eediff - 10.f) * DIV5;
					val = val * ratio + val * (1.f - ratio); 
				}

				fdiff -= val;
			}
			else
			{
				fdiff -= (float)FrameDiff * DIV4 * mul;
			}
		}

		if (fdiff > eediff)
			fdiff = eediff;

		if (fdiff < 0.f) fdiff = 0.f;

		if (diff < 0.f) io->_npcdata->vvpos = io->pos.y + fdiff;
		else io->_npcdata->vvpos = io->pos.y - fdiff;
	}
}
long FLAG_ALLOW_CLOTHES = 1;
void ARX_INTERACTIVE_Teleport(INTERACTIVE_OBJ * io, EERIE_3D * target, long flags)
{

	if (!io)
		return;

	FRAME_COUNT = -1;
	EERIE_3D translate;
	io->GameFlags &= ~GFLAG_NOCOMPUTATION;
	io->room_flags |= 1;
	io->room = -1;

	if (io == inter.iobj[0])
	{
		moveto.x = player.pos.x = target->x;
		moveto.y = player.pos.y = target->y + PLAYER_BASE_HEIGHT;
		moveto.z = player.pos.z = target->z;
	}

	translate.x = target->x - io->pos.x;
	translate.y = target->y - io->pos.y;
	translate.z = target->z - io->pos.z;

	// In case it is being dragged... (except for drag teleport update)
	if (!flags & 1)
	{
		if (io == DRAGINTER)
			Set_DragInter(NULL);
	}

	io->pos.x += translate.x;
	io->pos.y += translate.y;

	if (io->ioflags & IO_NPC)
		io->_npcdata->vvpos = io->pos.y;

	io->pos.z += translate.z;
	io->lastpos.x = io->physics.cyl.origin.x = io->pos.x;
	io->lastpos.y = io->physics.cyl.origin.y = io->pos.y;
	io->lastpos.z = io->physics.cyl.origin.z = io->pos.z;

	if (io->obj)
	{
		if (io->obj->pbox)
		{
			if (io->obj->pbox->active)
			{
				for (long i = 0; i < io->obj->pbox->nb_physvert; i++)
				{
					io->obj->pbox->vert[i].pos.x += translate.x;
					io->obj->pbox->vert[i].pos.y += translate.y;
					io->obj->pbox->vert[i].pos.z += translate.z;
				}

				io->obj->pbox->active = 0;
			}
		}

		for (long i = 0; i < io->obj->nbvertex; i++)
		{
			io->obj->vertexlist3[i].v.x += translate.x;
			io->obj->vertexlist3[i].v.y += translate.y;
			io->obj->vertexlist3[i].v.z += translate.z;
		}
	}

	MOLLESS_Clear(io->obj, 1);
	ResetVVPos(io);
}
//*************************************************************************************
// Finds IO number by name
//*************************************************************************************
long GetTargetByNameTarget(char * name)
{
	char temp[256];

	if (name == NULL) return -1;

	if (!stricmp(name, "self"))		return -2;

	if (!stricmp(name, "none"))		return -1;

	if (!stricmp(name, "me"))		return -2;

	if (!stricmp(name, "player"))	return 0;     ///player is now an io with index 0

	for (long i = 0 ; i < inter.nbmax ; i++)
	{
		if ((inter.iobj[i] != NULL)
		        &&	(inter.iobj[i]->ident > -1))
		{
			sprintf(temp, "%s_%04d", GetName(inter.iobj[i]->filename), inter.iobj[i]->ident);

			if (!stricmp(name, temp)) return i;
		}
	}

	return -1;
}

extern long TOTAL_BODY_CHUNKS_COUNT;
//*************************************************************************************
// Releases An Interactive Object from memory
//*************************************************************************************
void ReleaseInter(INTERACTIVE_OBJ * io)
{

	if (!io) return;

	if (!FAST_RELEASE)
		TREATZONE_RemoveIO(io);

	if (io->ioflags & IO_BODY_CHUNK)
	{
		TOTAL_BODY_CHUNKS_COUNT--;

		if (TOTAL_BODY_CHUNKS_COUNT < 0) TOTAL_BODY_CHUNKS_COUNT = 0;
	}

	if (io->ignit_light > -1)
	{
		DynLight[io->ignit_light].exist = 0;
		io->ignit_light = -1;
	}

	if (io->ignit_sound != ARX_SOUND_INVALID_RESOURCE)
	{
		ARX_SOUND_Stop(io->ignit_sound);
		io->ignit_sound = ARX_SOUND_INVALID_RESOURCE;
	}

	if (io == FlyingOverIO)
		FlyingOverIO = NULL;

	if ((MasterCamera.exist & 1) && (MasterCamera.io == io))
		MasterCamera.exist = 0;

	if ((MasterCamera.exist & 2) && (MasterCamera.want_io == io))
		MasterCamera.exist = 0;

	InterTreeViewItemRemove(io);
	ARX_INTERACTIVE_DestroyDynamicInfo(io);
	IO_UnlinkAllLinkedObjects(io);


	// Releases "ToBeDrawn" Transparent Polys linked to this object !
	ARX_INTERACTIVE_MEMO_TWEAK_CLEAR(io);
	ARX_SCRIPT_Timer_Clear_For_IO(io);

	if ((io->obj) && (!(io->ioflags & IO_CAMERA)) && (!(io->ioflags & IO_MARKER)) && (!(io->ioflags & IO_GOLD)))
	{
		ReleaseEERIE3DObj(io->obj);
		io->obj = NULL;
	}

	ARX_SPELLS_RemoveAllSpellsOn(io);

	if (io->tweakerinfo != NULL)
	{
		free(io->tweakerinfo);
		io->tweakerinfo = NULL;
	}

	if (io->tweaky)
	{
		ReleaseEERIE3DObj(io->tweaky);
		io->tweaky = NULL;
	}

	for (long iNbBag = 0; iNbBag < 3; iNbBag++)
		for (long j = 0; j < INVENTORY_Y; j++)
			for (long i = 0; i < INVENTORY_X; i++)
			{
				if (inventory[iNbBag][i][j].io == io)
					inventory[iNbBag][i][j].io = NULL;
			}

	ReleaseScript(&io->script);
	ReleaseScript(&io->over_script);

	for (long n = 0; n < MAX_ANIMS; n++)
	{
		if (io->anims[n] != NULL)
		{
			EERIE_ANIMMANAGER_ReleaseHandle(io->anims[n]);
			io->anims[n] = NULL;
		}
	}

	if (io->shop_category)
		free(io->shop_category);

	if (io->inventory_skin)
		free(io->inventory_skin);

	if (io->damagedata >= 0)
		damages[io->damagedata].exist = 0;

	ARX_IOGROUP_Release(io);

	if (io->usemesh)
		free(io->usemesh);

	if (ValidDynLight(io->dynlight))
		DynLight[io->dynlight].exist = 0;

	io->dynlight = -1;

	if (ValidDynLight(io->halo.dynlight))
		DynLight[io->halo.dynlight].exist = 0;

	io->halo.dynlight = -1;

	if (io->lastanimvertex)
		free(io->lastanimvertex);

	if (io->usepath)
		free(io->usepath);

	if (io->symboldraw != NULL)
	{
		free(io->symboldraw);
		io->symboldraw = NULL;
	}

	if (io->ioflags & IO_NPC)
	{
		if (io->_npcdata->ex_rotate != NULL) free(io->_npcdata->ex_rotate);

		if (io->_npcdata->pathfind.list) free(io->_npcdata->pathfind.list);

		free(io->_npcdata);
	}

	if (io->ioflags & IO_ITEM)
	{
		if (io->_itemdata->equipitem) free(io->_itemdata->equipitem);

		io->_itemdata->equipitem = NULL;
	}

	if (io->ioflags & IO_FIX)
		free(io->_fixdata);

	if (io->ioflags & IO_ITEM)
		free(io->_itemdata);

	if (io->ioflags & IO_CAMERA)
	{
		if (io->_camdata)
		{
			void * ptr = (void *)ACTIVECAM;

			if (ptr == io->_camdata)
				ACTIVECAM = &subj;

			free(io->_camdata);
		}

		io->_camdata = NULL;
	}

	if ((TSecondaryInventory) && (TSecondaryInventory->io == io))
	{
		TSecondaryInventory = NULL;
	}

	if (io->inventory != NULL) free(io->inventory);

	if (io->weaponmaterial)
		free(io->weaponmaterial);

	if (io->strikespeech)
		free(io->strikespeech);

	if (io->stepmaterial)
		free(io->stepmaterial);

	if (io->armormaterial)
		free(io->armormaterial);

	long ion = GetInterNum(io);

	if (ion > -1)
		inter.iobj[ion] = NULL;

	free(io);//
	io = NULL;
}

//***********************************************************************************
// AddInteractive:
// Adds an Interactive Object to the Scene
// Calls appropriate func depending on object Type (ITEM, NPC or FIX)
// Creates an IO Ident for added object if necessary
// flags can be IO_IMMEDIATELOAD (1) to FORCE loading
//***********************************************************************************
INTERACTIVE_OBJ * AddInteractive(LPDIRECT3DDEVICE7 pd3dDevice, char * file, long id, long flags)
{
	INTERACTIVE_OBJ * io = NULL;
	char ficc[HERMES_PATH_SIZE];
	strcpy(ficc, file);
	MakeUpcase(ficc);

	if (IsIn(ficc, "ITEMS"))
		io = AddItem(pd3dDevice, file, flags);
	else if (IsIn(ficc, "NPC"))
		io = AddNPC(pd3dDevice, file, flags);
	else if (IsIn(ficc, "FIX"))
		io = AddFix(pd3dDevice, file, flags);
	else if (IsIn(ficc, "CAMERA"))
		io = AddCamera(pd3dDevice, file);
	else if (IsIn(ficc, "MARKER"))
		io = AddMarker(pd3dDevice, file);

	if (io)
	{
		if ((id == 0)
		        &&	!(flags & NO_IDENT))
			MakeIOIdent(io);
		else if (id == -1)
			MakeTemporaryIOIdent(io);
		else
			io->ident = id;
	}

	return io;
}
//***********************************************************************************
// SetWeapon:
// Links an object designed by path "temp" to the primary attach of interactive object
// "io".
//***********************************************************************************

void SetWeapon_On(INTERACTIVE_OBJ * io)
{
	if ((!io)
	        &&	!(io->ioflags & IO_NPC))
		return;

	INTERACTIVE_OBJ * ioo = (INTERACTIVE_OBJ *)io->_npcdata->weapon;

	if ((ioo)
	        &&	(ioo->obj))
	{
		EERIE_LINKEDOBJ_UnLinkObjectFromObject(io->obj, ioo->obj);
		EERIE_LINKEDOBJ_LinkObjectToObject(io->obj, ioo->obj, "PRIMARY_ATTACH", "PRIMARY_ATTACH", ioo);
	}
}
void SetWeapon_Back(INTERACTIVE_OBJ * io)
{
	if (!io
	        ||	!(io->ioflags & IO_NPC))
		return;

	INTERACTIVE_OBJ * ioo = (INTERACTIVE_OBJ *)io->_npcdata->weapon;

	if ((ioo)
	        &&	(ioo->obj))
	{
		EERIE_LINKEDOBJ_UnLinkObjectFromObject(io->obj, ioo->obj);

		if (io->GameFlags & GFLAG_HIDEWEAPON) return;

		long ni = io->obj->fastaccess.weapon_attach;

		if (ni >= 0)
			EERIE_LINKEDOBJ_LinkObjectToObject(io->obj, ioo->obj, "WEAPON_ATTACH", "PRIMARY_ATTACH", ioo);
		else
		{
			ni = io->obj->fastaccess.secondary_attach;

			if (ni >= 0)
				EERIE_LINKEDOBJ_LinkObjectToObject(io->obj, ioo->obj, "SECONDARY_ATTACH", "PRIMARY_ATTACH", ioo);
		}
	}
}

void Prepare_SetWeapon(INTERACTIVE_OBJ * io, char * temp)
{
	if (!io
	        ||	!(io->ioflags & IO_NPC))
		return;

	if (io->_npcdata->weapon)
	{
		INTERACTIVE_OBJ * ioo = (INTERACTIVE_OBJ *)io->_npcdata->weapon;
		EERIE_LINKEDOBJ_UnLinkObjectFromObject(io->obj, ioo->obj);
		io->_npcdata->weapon = NULL;
		ReleaseInter(ioo);
	}

	char tex[256];
	char tex1[256];
	char tx[256];
	MakeDir(tex1, "Graph\\Obj3D\\Interactive\\Items\\Weapons\\");
	sprintf(tx, "%s\\%s\\%s.teo", tex1, temp, temp);
	File_Standardize(tx, tex);
	
	io->_npcdata->weapon = AddItem(GDevice, tex, IO_IMMEDIATELOAD);

	INTERACTIVE_OBJ * ioo = (INTERACTIVE_OBJ *)io->_npcdata->weapon;

	if (ioo)
	{
		MakeTemporaryIOIdent(ioo);
		SendIOScriptEvent(ioo, SM_INIT, "", NULL);
		SendIOScriptEvent(ioo, SM_INITEND, "", NULL);
		io->_npcdata->weapontype = ioo->type_flags;
		ioo->show = SHOW_FLAG_LINKED;
		ioo->scriptload = 2;

		SetWeapon_Back(io);
	}
}

void GetIOScript(INTERACTIVE_OBJ * io, char * texscript)
{
	if (PAK_FileExist(texscript))
	{
		long FileSize = 0;
		io->script.data = (char *)PAK_FileLoadMallocZero(texscript, &FileSize);

		if (io->script.data)
		{
			io->script.size = FileSize;
			InitScript(&io->script);
		}
	}
	else
	{
		io->script.size = 0;
		io->script.data = NULL;
	}
}

//***********************************************************************************
// Links an Interactive Object to another interactive object using an attach point
//***********************************************************************************
void LinkObjToMe(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * io2, char * attach)
{
	if ((!io)
	        ||	(!io2))
		return;

	RemoveFromAllInventories(io2);
	io2->show = SHOW_FLAG_LINKED;
	EERIE_LINKEDOBJ_LinkObjectToObject(io->obj, io2->obj, attach, attach, io2);
}
//***********************************************************************************
// AddFix
// Adds a FIX INTERACTIVE OBJECT to the Scene
//***********************************************************************************
INTERACTIVE_OBJ * AddFix(LPDIRECT3DDEVICE7 pd3dDevice, char * file, long flags)
{
	char tex1[HERMES_PATH_SIZE];
	char tex5[HERMES_PATH_SIZE];
	char texscript[HERMES_PATH_SIZE];

	strcpy(texscript, file);
	SetExt(texscript, "asl");
	sprintf(tex1, file);
	SetExt(tex1, "teo");

	char file2[256];
	sprintf(file2, "%sGAME\\%s", Project.workingdir, file + strlen(Project.workingdir));
	SetExt(file2, ".FTL");

	if (!PAK_FileExist(file2)
	        &&	!PAK_FileExist(file))
	{
		return NULL;
	}

	char texx[HERMES_PATH_SIZE];
	sprintf(texx, "AddFix - %s", file);
	SendConsole(texx, 2, 0, (HWND)g_pD3DApp->m_hWnd);

	INTERACTIVE_OBJ * io = CreateFreeInter();

	if (!io)
		return NULL;


	io->_fixdata = (IO_FIXDATA *)malloc(sizeof(IO_FIXDATA)); //"IO FIXdata"
	memset(io->_fixdata, 0, sizeof(IO_FIXDATA));
	io->ioflags = IO_FIX;
	io->_fixdata->trapvalue = -1;

	GetIOScript(io, texscript);

	if (!(flags & NO_ON_LOAD))
		SendIOScriptEvent(io, SM_LOAD, "", NULL);

	io->spellcast_data.castingspell = -1;
	io->lastpos.x = io->initpos.x = io->pos.x = player.pos.x - (float)EEsin(DEG2RAD(player.angle.b)) * 140.f;
	io->lastpos.y = io->initpos.y = io->pos.y = player.pos.y;
	io->lastpos.z = io->initpos.z = io->pos.z = player.pos.z + (float)EEcos(DEG2RAD(player.angle.b)) * 140.f;
	io->lastpos.x = io->initpos.x = (float)((long)(io->initpos.x / 20)) * 20.f;
	io->lastpos.z = io->initpos.z = (float)((long)(io->initpos.z / 20)) * 20.f;

	EERIEPOLY * ep;
	ep = CheckInPoly(io->pos.x, io->pos.y + PLAYER_BASE_HEIGHT, io->pos.z);

	if (ep)
	{
		float tempo;

		if (GetTruePolyY(ep, &io->pos, &tempo))
			io->lastpos.y = io->initpos.y = io->pos.y = tempo;
	}

	ep = CheckInPoly(io->pos.x, player.pos.y, io->pos.z);

	if (ep)
	{
		io->pos.y = __min(ep->v[0].sy, ep->v[1].sy);
		io->lastpos.y = io->initpos.y = io->pos.y = __min(io->pos.y, ep->v[2].sy);
	}

	strcpy(io->filename, tex1);

	if (!io->obj)
	{
		if (flags & NO_MESH)
		{
			io->obj = NULL;

		}
		else
		{
			strcpy(tex5, tex1);
			sprintf(tex1, "%sGraph\\Obj3D\\Textures\\", Project.workingdir);
			io->obj = TheoToEerie_Fast(tex1, tex5, TTE_NO_PHYSICS_BOX, GDevice);
		}
	}

	io->infracolor.r = 0.6f;
	io->infracolor.g = 0.f;
	io->infracolor.b = 1.f;
	TextureContainer * tc;
	tc = MakeTCFromFile_NoRefinement("Graph\\Interface\\misc\\Default[Icon].bmp"); 

	if (tc)
	{
		if (!tc->m_pddsSurface) tc->Restore(pd3dDevice);

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

	io->collision = 1;

	if (CheckScriptSyntax_Loading(io) != TRUE) io->ioflags |= IO_FREEZESCRIPT;

	return io;
}

//***********************************************************************************
// AddCamera
// Adds a CAMERA INTERACTIVE OBJECT to the Scene
//***********************************************************************************
INTERACTIVE_OBJ * AddCamera(LPDIRECT3DDEVICE7 pd3dDevice, char * file)
{

	char tex1[HERMES_PATH_SIZE];
	char texscript[HERMES_PATH_SIZE];

	strcpy(texscript, file);
	SetExt(texscript, "asl");
	sprintf(tex1, file);
	SetExt(tex1, "teo");

	char file2[256];
	sprintf(file2, "%sGAME\\%s", Project.workingdir, file + strlen(Project.workingdir));
	SetExt(file2, ".FTL");

	if (!PAK_FileExist(file2)
	        &&	!PAK_FileExist(file))
	{
		return NULL;
	}

	char texx[HERMES_PATH_SIZE];
	sprintf(texx, "AddCamera - %s", file);
	SendConsole(texx, 2, 0, (HWND)g_pD3DApp->m_hWnd);

	INTERACTIVE_OBJ * io = CreateFreeInter();
	EERIEPOLY * ep;

	if (!io) return NULL;

	GetIOScript(io, texscript);

	io->lastpos.x = io->initpos.x = io->pos.x = player.pos.x - (float)EEsin(DEG2RAD(player.angle.b)) * 140.f;
	io->lastpos.y = io->initpos.y = io->pos.y = player.pos.y;
	io->lastpos.z = io->initpos.z = io->pos.z = player.pos.z + (float)EEcos(DEG2RAD(player.angle.b)) * 140.f;
	io->lastpos.x = io->initpos.x = (float)((long)(io->initpos.x / 20)) * 20.f;
	io->lastpos.z = io->initpos.z = (float)((long)(io->initpos.z / 20)) * 20.f;
	float tempo;
	ep = CheckInPoly(io->pos.x, io->pos.y + PLAYER_BASE_HEIGHT, io->pos.z, &tempo);

	if (ep)
	{
		io->lastpos.y = io->initpos.y = io->pos.y = tempo;
	}

	ep = CheckInPoly(io->pos.x, player.pos.y, io->pos.z);

	if (ep)
	{
		io->pos.y = __min(ep->v[0].sy, ep->v[1].sy);
		io->lastpos.y = io->initpos.y = io->pos.y = __min(io->pos.y, ep->v[2].sy);
	}

	io->lastpos.y = io->initpos.y = io->pos.y += PLAYER_BASE_HEIGHT;

	strcpy(io->filename, tex1);
 
	io->obj = cameraobj;

	io->_camdata = (IO_CAMDATA *)malloc(sizeof(IO_CAMDATA));
	memcpy(&io->_camdata->cam, &subj, sizeof(EERIE_CAMERA));
	io->_camdata->cam.focal = 350.f;
	io->ioflags = IO_CAMERA;
	io->collision = 0;

	if (CheckScriptSyntax_Loading(io) != TRUE) io->ioflags |= IO_FREEZESCRIPT;

	return io;
}
//***********************************************************************************
// AddMarker
// Adds a MARKER INTERACTIVE OBJECT to the Scene
//***********************************************************************************
INTERACTIVE_OBJ * AddMarker(LPDIRECT3DDEVICE7 pd3dDevice, char * file)
{
	char tex1[HERMES_PATH_SIZE];
	char texscript[HERMES_PATH_SIZE];

	strcpy(texscript, file);
	SetExt(texscript, "asl");
	sprintf(tex1, file);
	SetExt(tex1, "teo");

	char file2[256];
	sprintf(file2, "%sGAME\\%s", Project.workingdir, file + strlen(Project.workingdir));
	SetExt(file2, ".FTL");

	if (!PAK_FileExist(file2)
	        &&	!PAK_FileExist(file))
	{
		return NULL;
	}

	char texx[HERMES_PATH_SIZE];
	sprintf(texx, "AddMarker - %s", file);
	SendConsole(texx, 2, 0, (HWND)g_pD3DApp->m_hWnd);

	INTERACTIVE_OBJ * io = CreateFreeInter();
	EERIEPOLY * ep;

	if (!io) return NULL;

	GetIOScript(io, texscript);
	
	io->lastpos.x = io->initpos.x = io->pos.x = player.pos.x - (float)EEsin(DEG2RAD(player.angle.b)) * 140.f;
	io->lastpos.y = io->initpos.y = io->pos.y = player.pos.y;
	io->lastpos.z = io->initpos.z = io->pos.z = player.pos.z + (float)EEcos(DEG2RAD(player.angle.b)) * 140.f;
	io->lastpos.x = io->initpos.x = (float)((long)(io->initpos.x / 20)) * 20.f;
	io->lastpos.z = io->initpos.z = (float)((long)(io->initpos.z / 20)) * 20.f;
	ep = CheckInPoly(io->pos.x, io->pos.y + PLAYER_BASE_HEIGHT, io->pos.z);

	if (ep)
	{
		float tempo;

		if (GetTruePolyY(ep, &io->pos, &tempo))
			io->lastpos.y = io->initpos.y = io->pos.y = tempo;
	}

	ep = CheckInPoly(io->pos.x, player.pos.y, io->pos.z);

	if (ep)
	{
		io->pos.y = __min(ep->v[0].sy, ep->v[1].sy);
		io->lastpos.y = io->initpos.y = io->pos.y = __min(io->pos.y, ep->v[2].sy);
	}

	io->lastpos.y = io->initpos.y = io->pos.y += PLAYER_BASE_HEIGHT;

	strcpy(io->filename, tex1);
 
	io->obj = markerobj;
	io->ioflags = IO_MARKER;
	io->collision = 0;

	if (CheckScriptSyntax_Loading(io) != TRUE) io->ioflags |= IO_FREEZESCRIPT;

	return io;
}
void ShowIOPath(INTERACTIVE_OBJ * io)
{
	for (long i = 0; i < ACTIVEBKG->nbanchors; i++)
	{
		_ANCHOR_DATA * ad = &ACTIVEBKG->anchors[i];
		ad->flags &= ~1;
	}

	if ((io) && (io->ioflags & IO_NPC))
		for (long j = 0; j < io->_npcdata->pathfind.listnb; j++)
		{
			_ANCHOR_DATA * ad = &ACTIVEBKG->anchors[io->_npcdata->pathfind.list[j]];
			ad->flags |= 1;
		}
}

//*************************************************************************************
// Unselect an IO
//*************************************************************************************
void UnSelectIO(INTERACTIVE_OBJ * io)
{
	if ((io)
	        && (io->EditorFlags & EFLAG_SELECTED))
	{
		io->EditorFlags &= ~EFLAG_SELECTED;
		NbIOSelected--;
		LastSelectedIONum = -1;
	}
}
//*************************************************************************************
// Select an IO
//*************************************************************************************
void SelectIO(INTERACTIVE_OBJ * io)
{
	for (long i = 0; i < inter.nbmax; i++)
	{
		if ((inter.iobj[i] != NULL)
		        &&	(inter.iobj[i]->EditorFlags & EFLAG_SELECTED))
		{
			UnSelectIO(inter.iobj[i]);
		}
	}

	if ((io)
	        &&	(!(io->EditorFlags & EFLAG_SELECTED)))
	{
		io->EditorFlags |= EFLAG_SELECTED;
		NbIOSelected++;
		EERIE_3D curpos;
		GetItemWorldPosition(io, &curpos);
		LastSelectedIONum = GetInterNum(io);
		ShowIOPath(io);
	}
}
//*************************************************************************************
// Translate all selected IOs
//*************************************************************************************
void TranslateSelectedIO(EERIE_3D * op)
{
	for (long i = 1; i < inter.nbmax; i++)
	{
		if ((inter.iobj[i])
		        &&	(inter.iobj[i]->EditorFlags & EFLAG_SELECTED))
		{
			inter.iobj[i]->initpos.x = inter.iobj[i]->pos.x = inter.iobj[i]->initpos.x + op->x;
			inter.iobj[i]->initpos.y = inter.iobj[i]->pos.y = inter.iobj[i]->initpos.y + op->y;
			inter.iobj[i]->initpos.z = inter.iobj[i]->pos.z = inter.iobj[i]->initpos.z + op->z;
		}
	}
}
//*************************************************************************************
// Reset all selected IOs rotations
//*************************************************************************************
void ResetSelectedIORot()
{
	for (long i = 1; i < inter.nbmax; i++)
	{
		if ((inter.iobj[i])
		        &&	(inter.iobj[i]->EditorFlags & EFLAG_SELECTED))
		{
			inter.iobj[i]->initangle.a = inter.iobj[i]->angle.a = 0.f;
			inter.iobj[i]->initangle.b = inter.iobj[i]->angle.b = 0.f;
			inter.iobj[i]->initangle.g = inter.iobj[i]->angle.g = 0.f;
		}
	}
}

//*************************************************************************************
// Rotate all selected IOs
//*************************************************************************************
void RotateSelectedIO(EERIE_3D * op)
{
	for (long i = 1; i < inter.nbmax; i++)
	{
		if ((inter.iobj[i])
		        &&	(inter.iobj[i]->EditorFlags & EFLAG_SELECTED))
		{
			inter.iobj[i]->initangle.a = inter.iobj[i]->angle.a = inter.iobj[i]->initangle.a + op->a;
			inter.iobj[i]->initangle.b = inter.iobj[i]->angle.b = inter.iobj[i]->initangle.b + op->b;
			inter.iobj[i]->initangle.g = inter.iobj[i]->angle.g = inter.iobj[i]->initangle.g + op->g;
		}
	}
}
//*************************************************************************************
// Delete All Selected IOs
//*************************************************************************************
void ARX_INTERACTIVE_DeleteByIndex(long i, long flag)
{
	if ((i < 1)
	        ||	(i >= inter.nbmax))
		return;

	char temp[HERMES_PATH_SIZE];
	char temp2[HERMES_PATH_SIZE];
	char temp3[HERMES_PATH_SIZE];

	if (inter.iobj[i] != NULL)
	{
		//Must "KILL" dir...
		if (inter.iobj[i]->scriptload == 0)
		{
			if (inter.iobj[i]->ident > 0)
			{
				sprintf(temp, inter.iobj[i]->filename);
				strcpy(temp2, GetName(temp));
				RemoveName(temp);
				sprintf(temp, "%s%s_%04d.", temp, temp2, inter.iobj[i]->ident);

				if (DirectoryExist(temp))
				{
					long _delete = 0;
					sprintf(temp3, "Really remove Directory & Directory Contents ?\n\n%s", temp);

					if (flag & FLAG_NOCONFIRM)
					{
						_delete = 1;
					}
					else if (OKBox(temp3, "WARNING"))
					{
						_delete = 1;
					}

					if (flag & FLAG_DONTKILLDIR) _delete = 0;

					if (_delete)
					{
						strcat(temp, "\\");
						KillAllDirectory(temp);
					}
				}
			}
		}

		ReleaseInter(inter.iobj[i]);
		inter.iobj[i] = NULL;
	}
}
void DeleteSelectedIO()
{
	for (long i = 1; i < inter.nbmax; i++)
	{
		if ((inter.iobj[i] != NULL)
		        &&	(inter.iobj[i]->EditorFlags & EFLAG_SELECTED))
		{
			UnSelectIO(inter.iobj[i]);
			ARX_INTERACTIVE_DeleteByIndex(i, 0);
		}
	}
}

//*************************************************************************************
// Snaps to ground all selected IOs
//*************************************************************************************
void GroundSnapSelectedIO()
{
	for (long i = 1; i < inter.nbmax; i++)
	{
		if ((inter.iobj[i] != NULL)
		        &&	(inter.iobj[i]->EditorFlags & EFLAG_SELECTED))
		{
			INTERACTIVE_OBJ * io = inter.iobj[i];
			EERIE_3D ppos;
			ppos.x = io->pos.x;
			ppos.y = io->pos.y;
			ppos.z = io->pos.z;
			EERIEPOLY * ep = CheckInPoly(ppos.x, ppos.y + PLAYER_BASE_HEIGHT, ppos.z);

			if (ep)
			{
				float ay;

				if (GetTruePolyY(ep, &io->pos, &ay))
					io->initpos.y = io->pos.y = ay;
			}
		}
	}
}
//***********************************************************************************
// AddNPC
// Adds a NPC INTERACTIVE OBJECT to the Scene
//***********************************************************************************
INTERACTIVE_OBJ * AddNPC(LPDIRECT3DDEVICE7 pd3dDevice, char * file, long flags)
{

	char tex1[HERMES_PATH_SIZE];
	char tex5[HERMES_PATH_SIZE];
	char texscript[HERMES_PATH_SIZE];

	// creates script filename
	strcpy(texscript, file);
	SetExt(texscript, "asl");

	// creates teo filename
	sprintf(tex1, file);
	SetExt(tex1, "teo");

	char file2[256];
	sprintf(file2, "%sGAME\\%s", Project.workingdir, file + strlen(Project.workingdir));
	SetExt(file2, ".FTL");

	if ((!PAK_FileExist(file2))
	        &&	(!PAK_FileExist(file))
	   )
	{
		return NULL;
	}

	char texx[HERMES_PATH_SIZE];
	sprintf(texx, "AddNPC - %s", file);
	SendConsole(texx, 2, 0, (HWND)g_pD3DApp->m_hWnd);

	INTERACTIVE_OBJ * io = CreateFreeInter();
	EERIEPOLY * ep;

	if (io == NULL) return NULL;

	io->forcedmove.x = 0.f;
	io->forcedmove.y = 0.f;
	io->forcedmove.z = 0.f;
	io->_npcdata = (IO_NPCDATA *)malloc(sizeof(IO_NPCDATA)); //"NPC DATA"
	memset(io->_npcdata, 0, sizeof(IO_NPCDATA));
	io->ioflags = IO_NPC;

	GetIOScript(io, texscript);
	
	io->spellcast_data.castingspell = -1;
	io->_npcdata->life = io->_npcdata->maxlife = 20.f;
	io->_npcdata->mana = io->_npcdata->maxmana = 10.f;
	io->_npcdata->poisonned = 0.f;
	io->_npcdata->critical = 5.f;
	io->_npcdata->strike_time = 0;
	io->_npcdata->walk_start_time = 0;
	io->_npcdata->reach = 20.f;
	io->_npcdata->aimtime = 0.f;
	io->_npcdata->aiming_start = 0;
	io->_npcdata->npcflags = 0;
	io->_npcdata->backstab_skill = 0;
	io->_npcdata->blood_color = 0xFFFF0000;
	io->_npcdata->stare_factor = 1.f;

	if (!(flags & NO_ON_LOAD))
		SendIOScriptEvent(io, SM_LOAD, "", NULL);

	io->lastpos.x = io->initpos.x = io->pos.x = player.pos.x - (float)EEsin(DEG2RAD(player.angle.b)) * 140.f;
	io->lastpos.y = io->initpos.y = io->pos.y = player.pos.y;
	io->lastpos.z = io->initpos.z = io->pos.z = player.pos.z + (float)EEcos(DEG2RAD(player.angle.b)) * 140.f;
	io->lastpos.x = io->initpos.x = (float)((long)(io->initpos.x / 20)) * 20.f;
	io->lastpos.z = io->initpos.z = (float)((long)(io->initpos.z / 20)) * 20.f;
	ep = CheckInPoly(io->pos.x, io->pos.y + PLAYER_BASE_HEIGHT, io->pos.z);

	if (ep)
	{
		float tempo;

		if (GetTruePolyY(ep, &io->pos, &tempo))
			io->lastpos.y = io->initpos.y = io->pos.y = tempo; 
	}

	ep = CheckInPoly(io->pos.x, player.pos.y, io->pos.z);

	if (ep)
	{
		io->pos.y = __min(ep->v[0].sy, ep->v[1].sy);
		io->lastpos.y = io->initpos.y = io->pos.y = __min(io->pos.y, ep->v[2].sy);
	}

	
	strcpy(io->filename, tex1);

	if (!io->obj)
	{
		if (flags & NO_MESH)
		{
			io->obj = NULL;

		}
		else
		{
			strcpy(tex5, tex1);
			sprintf(tex1, "%sGraph\\Obj3D\\Textures\\", Project.workingdir);
			io->obj = TheoToEerie_Fast(tex1, tex5, TTE_NO_PHYSICS_BOX | TTE_NPC, GDevice);
		}
	}

	io->_npcdata->speakpitch = 1.f;
	io->_npcdata->pathfind.listnb = -1;
	io->_npcdata->behavior = BEHAVIOUR_NONE;
	io->_npcdata->pathfind.truetarget = -1;

	if ((!(flags & NO_MESH))
	        &&	(flags & IO_IMMEDIATELOAD))
		EERIE_COLLISION_Cylinder_Create(io);

	io->infracolor.r = 1.f;
	io->infracolor.g = 0.f;
	io->infracolor.b = 0.2f;
	io->collision = 1;
	io->inv = NULL;

	if (CheckScriptSyntax_Loading(io) != TRUE) io->ioflags |= IO_FREEZESCRIPT;

	ARX_INTERACTIVE_HideGore(io);
	return io;
}

//*************************************************************************************
// Reload Scripts (Class & Local) for an IO
//*************************************************************************************
void ReloadScript(INTERACTIVE_OBJ * io)
{
	char texscript[HERMES_PATH_SIZE];
	char tmp2[HERMES_PATH_SIZE];
	strcpy(texscript, io->filename);
	SetExt(texscript, "asl");
	ARX_SCRIPT_Timer_Clear_For_IO(io);
	ReleaseScript(&io->over_script);
	ReleaseScript(&io->script);

	if (PAK_FileExist(texscript))
	{
		long FileSize = 0;
		io->script.data = (char *)PAK_FileLoadMallocZero(texscript, &FileSize);

		if (io->script.data != NULL)
		{
			io->script.size = FileSize;
			InitScript(&io->script);
		}
	}
	else
	{
		io->script.size = 0;
		io->script.data = NULL;
	}

	sprintf(texscript, io->filename);
	strcpy(tmp2, GetName(texscript));
	RemoveName(texscript);
	sprintf(texscript, "%s%s_%04d\\%s.asl", texscript, tmp2, io->ident, tmp2);

	if (PAK_FileExist(texscript))
	{
		long FileSize = 0;
		io->over_script.data = (char *)PAK_FileLoadMallocZero(texscript, &FileSize);

		if (io->over_script.data != NULL)
		{
			io->over_script.size = FileSize;
			InitScript(&io->over_script);
			io->over_script.master = (void *)&io->script;
		}
	}
	else
	{
		io->over_script.size = 0;
		io->over_script.data = NULL;
	}

	long num = GetInterNum(io);

	if (ValidIONum(num))
	{
		if (inter.iobj[num]
		        &&	inter.iobj[num]->script.data)
		{
			SendScriptEvent(&inter.iobj[num]->script, SM_INIT, "", inter.iobj[num], NULL);
		}

		if (inter.iobj[num]
		        &&	inter.iobj[num]->over_script.data)
		{
			SendScriptEvent(&inter.iobj[num]->over_script, SM_INIT, "", inter.iobj[num], NULL);
		}

		if (inter.iobj[num]
		        &&	inter.iobj[num]->script.data)
		{
			SendScriptEvent(&inter.iobj[num]->script, SM_INITEND, "", inter.iobj[num], NULL);
		}

		if (inter.iobj[num]
		        &&	inter.iobj[num]->over_script.data)
		{
			SendScriptEvent(&inter.iobj[num]->over_script, SM_INITEND, "", inter.iobj[num], NULL);
		}
	}
}

//*************************************************************************************
// Reloads All Scripts for all IOs
//*************************************************************************************
void ReloadAllScripts()
{
	ARX_SCRIPT_Timer_ClearAll();

	for (long i = 0; i < inter.nbmax; i++)
	{
		if (inter.iobj[i])
			ReloadScript(inter.iobj[i]);
	}
}
BOOL ExistTemporaryIdent(INTERACTIVE_OBJ * io, long t);
//*************************************************************************************
// Creates an unique identifier for an IO
//*************************************************************************************
void MakeIOIdent(INTERACTIVE_OBJ * io)
{
	char temp[HERMES_PATH_SIZE];
	char temp2[HERMES_PATH_SIZE];
	long t = 1;

	if ((NODIRCREATION)
	        ||	!io)
		return;

	while (io->ident == 0)
	{
		sprintf(temp, io->filename);
		strcpy(temp2, GetName(temp));
		RemoveName(temp);
		sprintf(temp, "%s%s_%04d.", temp, temp2, t);

		if (!DirectoryExist(temp))
		{
			io->ident = t;
			CreateDirectory(temp, NULL);
			LogDirCreation(temp);
			WriteIOInfo(io, temp);
		}

		t++;
	}
}
//*************************************************************************************
// Tells if an ident corresponds to a temporary IO
// NEED TO OPEN "if (LAST_CHINSTANCE!=-1) ARX_Changelevel_CurGame_Open();"
// And close after seek session
//*************************************************************************************
BOOL ExistTemporaryIdent(INTERACTIVE_OBJ * io, long t)
{
	if (!io)
		return FALSE;

	char name1[256];
	char ident[256];;
	strcpy(name1, GetName(io->filename));
	sprintf(ident, "%s_%04d", name1, t);

	for (long i = 0; i < inter.nbmax; i++)
	{
		if (inter.iobj[i])
		{
			if ((inter.iobj[i]->ident == t)
			        &&	(io != inter.iobj[i]))
			{
				char name2[256];
				strcpy(name2, GetName(inter.iobj[i]->filename));

				if (!stricmp(name1, name2))
				{
					return TRUE;
				}
			}
		}
	}

	char file2[256];
	strcpy(file2, io->filename);
	RemoveName(file2);
	sprintf(file2, "%s%s", file2, ident);

	if (PAK_DirectoryExist(file2))
		return TRUE;

	if (LAST_CHINSTANCE != -1)
	{
		ARX_CHANGELEVEL_MakePath();

		if (ARX_Changelevel_CurGame_Seek(ident))
			return TRUE;
	}

	return FALSE;
}
//*************************************************************************************
// Creates a Temporary IO Ident
//*************************************************************************************
void MakeTemporaryIOIdent(INTERACTIVE_OBJ * io)
{
	long t = 1;

	if (!io) return;

	if (LAST_CHINSTANCE != -1) ARX_Changelevel_CurGame_Open();

	while (1)
	{
		if (!ExistTemporaryIdent(io, t))
		{
			io->ident = t;

			if (LAST_CHINSTANCE != -1) ARX_Changelevel_CurGame_Close();

			return;
			//	}
		}

		t++;
	}
}
extern EERIE_3DOBJ	* arrowobj;
extern long SP_DBG;
//***********************************************************************************
// AddItem
// Adds an ITEM INTERACTIVE OBJECT to the Scene
//***********************************************************************************
INTERACTIVE_OBJ * AddItem(LPDIRECT3DDEVICE7 pd3dDevice, char * fil, long flags)
{
	char tex1[HERMES_PATH_SIZE];
	char tex2[HERMES_PATH_SIZE];
	char tex5[HERMES_PATH_SIZE];
	char texscript[HERMES_PATH_SIZE];
	char file[256];
	long type = IO_ITEM;
	MakeUpcase(fil);

	if (!specialstrcmp(GetName(fil), "GOLD_COIN"))
	{
		strcpy(file, fil);
		RemoveName(file);
		strcat(file, "GOLD_COIN.asl");
		type = IO_ITEM | IO_GOLD;
	}
	else strcpy(file, fil);

	if (IsIn(fil, "MOVABLE"))
	{
		type = IO_ITEM | IO_MOVABLE;
	}

	strcpy(texscript, file);
	SetExt(texscript, "asl");
	sprintf(tex1, file);
	SetExt(tex1, "teo");

	strcpy(tex2, file);
	SetExt(tex2, "bmp");
	AddToName(tex2, "[Icon]");

	char file2[256];
	sprintf(file2, "%sGAME\\%s", Project.workingdir, file + strlen(Project.workingdir));
	SetExt(file2, ".FTL");

	if (!PAK_FileExist(file2)
	        &&	!PAK_FileExist(file))
	{
		return NULL;
	}

	if (!PAK_FileExist(tex2)) 	return NULL;

	INTERACTIVE_OBJ * io = CreateFreeInter();

	EERIEPOLY * ep;

	if (io == NULL) return NULL;

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

	GetIOScript(io, texscript);

	if (!(flags & NO_ON_LOAD))
		SendIOScriptEvent(io, SM_LOAD, "", NULL);

	io->spellcast_data.castingspell = -1;
	io->lastpos.x = io->initpos.x = io->pos.x = player.pos.x - (float)EEsin(DEG2RAD(player.angle.b)) * 140.f;
	io->lastpos.y = io->initpos.y = io->pos.y = player.pos.y;
	io->lastpos.z = io->initpos.z = io->pos.z = player.pos.z + (float)EEcos(DEG2RAD(player.angle.b)) * 140.f;
	io->lastpos.x = io->initpos.x = (float)((long)(io->initpos.x / 20)) * 20.f;
	io->lastpos.z = io->initpos.z = (float)((long)(io->initpos.z / 20)) * 20.f;

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
		io->pos.y = __min(ep->v[0].sy, ep->v[1].sy);
		io->lastpos.y = io->initpos.y = io->pos.y = __min(io->pos.y, ep->v[2].sy);
	}


	strcpy(io->filename, tex1);

	if (io->ioflags & IO_GOLD)
		io->obj = GoldCoinsObj[0];

	if (!io->obj)
	{
		if (flags & NO_MESH)
		{
			io->obj = NULL;
		}
		else
		{
 
			strcpy(tex5, tex1);
			sprintf(tex1, "%sGraph\\Obj3D\\Textures\\", Project.workingdir);

			io->obj = TheoToEerie_Fast(tex1, tex5, 0, GDevice);
		}
	}

	TextureContainer * tc;

	if (io->ioflags & IO_MOVABLE)
		tc = Movable;
	else if (io->ioflags & IO_GOLD)
		tc = GoldCoinsTC[0];
	else
	{
		D3DTextr_CreateTextureFromFile(tex2, Project.workingdir, 0, D3DTEXTR_NO_REFINEMENT , EERIETEXTUREFLAG_LOADSCENE_RELEASE);
		tc = D3DTextr_GetSurfaceContainer(tex2);
	}

	if (tc == NULL)
	{
		MakeDir(tex2, "Graph\\Interface\\misc\\Default[Icon].bmp");
		D3DTextr_CreateTextureFromFile(tex2, Project.workingdir, 0, D3DTEXTR_NO_REFINEMENT);
		D3DTextr_Restore(tex2, pd3dDevice);
		tc = D3DTextr_GetSurfaceContainer(tex2);
	}

	if (tc)
	{
		if (!tc->m_pddsSurface)
			tc->Restore(pd3dDevice);

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

	if (CheckScriptSyntax_Loading(io) != TRUE) io->ioflags |= IO_FREEZESCRIPT;

	return io;
}

extern float LAST_FZPOS;
extern float LAST_FZSCREEN;
extern long USE_CEDRIC_ANIM;
INTERACTIVE_OBJ * GetFirstInterAtPos(EERIE_S2D * pos, long flag = 0, EERIE_3D * _pRef = NULL, INTERACTIVE_OBJ ** _pTable = NULL, int * _pnNbInTable = NULL);

//*************************************************************************************
// Returns nearest interactive object found at position x,y
//*************************************************************************************
INTERACTIVE_OBJ * GetFirstInterAtPos(EERIE_S2D * pos, long flag, EERIE_3D * _pRef, INTERACTIVE_OBJ ** _pTable, int * _pnNbInTable)
{
	float n;
 
	float fdist = 9999999999.f;
	float fdistBB = 9999999999.f;
	float fMaxDist = flag ? 9999999999.f : 350;
	INTERACTIVE_OBJ * foundBB = NULL;
	INTERACTIVE_OBJ * foundPixel = NULL;
	INTERNMB = -1;
	bool bPlayerEquiped = false;

	if (Project.telekinesis)
	{
		fMaxDist = 850;
	}

	int nStart = 1;
	int nEnd = inter.nbmax;
	INTERACTIVE_OBJ ** pTableIO = inter.iobj;

	if ((flag == 3) && _pTable && _pnNbInTable)
	{
		nStart = 0;
		nEnd = *_pnNbInTable;
		pTableIO = _pTable;
	}

	for (long i = nStart; i < nEnd; i++)
	{
		bool bPass = true;

		INTERACTIVE_OBJ * io = pTableIO[i];

		// Is Object Valid ??
		if (io == NULL) continue;

		if (((io->ioflags & IO_CAMERA) || (io->ioflags & IO_MARKER))
		        && (EDITMODE != 1)) continue;

		if ((!(io->GameFlags & GFLAG_INTERACTIVITY)) && (!EDITMODE)) continue;

		// Is Object in TreatZone ??
		if (
		    ((bPlayerEquiped = IsEquipedByPlayer(io))  && (player.Interface & INTER_MAP))
		    || (io->GameFlags & GFLAG_ISINTREATZONE))

			// Is Object Displayed on screen ???
			if ((io->show == SHOW_FLAG_IN_SCENE) || (bPlayerEquiped && flag) || (bPlayerEquiped  && (player.Interface & INTER_MAP) && (Book_Mode == 0))) //((io->show==9) && (player.Interface & INTER_MAP)) )
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
						float flDistanceToRef = EESquaredDistance3D(&ACTIVECAM->pos, _pRef);
						float flDistanceToIO = EESquaredDistance3D(&ACTIVECAM->pos, &io->pos);
						bPass = bPlayerEquiped || (flDistanceToIO < flDistanceToRef);
					}

					float fp = EEDistance3D(&io->pos, &player.pos);

					if ((!flag && (fp <= fMaxDist)) && ((foundBB == NULL) || (fp < fdistBB)))
					{
						fdistBB = fp;
						foundBB = io;
						INTERNMB = i;
					}

					if ((io->ioflags & (IO_CAMERA | IO_MARKER | IO_GOLD)) || (bPlayerEquiped && !flag))
					{
						if (bPlayerEquiped)
							fp = 0.f;
						else
							fp = EEDistance3D(&io->pos, &player.pos);

						if ((fp < fdistBB) || (foundBB == NULL))
						{
							fdistBB = fp;
							foundBB = io;
							foundPixel = io;
							INTERNMB = i;
						}

						goto suite;
					}

					long j;

					for (j = 0; j < io->obj->nbfaces; j++)
					{
						if (io->animlayer[0].cur_anim != NULL)
						{
							if (USE_CEDRIC_ANIM)
								n = CEDRIC_PtIn2DPolyProjV2(io->obj, &io->obj->facelist[j] , pos->x, pos->y);
							else
								n = -1; 
						}
						else
							n = PtIn2DPolyProj(io->obj, &io->obj->facelist[j] , pos->x, pos->y);

						if (n > 0.f) 
						{
							if (bPlayerEquiped)
								fp = 0.f;
							else
								fp = EEDistance3D(&io->pos, &player.pos);

							if ((bPass && (fp <= fMaxDist)) && ((fp < fdist) || (foundPixel == NULL)))
							{
								{
									fdist = fp;
									foundPixel = io;
									INTERNMB = i;
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
bool IsEquipedByPlayer(INTERACTIVE_OBJ * io)
{
	if (!io)
		return FALSE;

	if ((io->ioflags & IO_ICONIC) && (io->show == SHOW_FLAG_ON_PLAYER))
		return true;

	long num = GetInterNum(io);

	for (long i = 0; i < MAX_EQUIPED; i++)
	{
		if ((player.equiped[i] != 0) && (player.equiped[i] == num)) return true;
	}

	return FALSE;
}

extern long LOOKING_FOR_SPELL_TARGET;
INTERACTIVE_OBJ * InterClick(EERIE_S2D * pos, long flag)
{
	LASTINTERCLICKNB = -1;

	if (IsFlyingOverInventory(pos))
	{
		return NULL;
	}

	float dist_Threshold;

	if (LOOKING_FOR_SPELL_TARGET)
		dist_Threshold = 550.f;
	else
		dist_Threshold = 360.f;

	INTERACTIVE_OBJ * io = GetFirstInterAtPos(pos);

	if (io != NULL)
	{
		if (io->ioflags & IO_NPC)
		{
			if (Distance3D(player.pos.x, player.pos.y, player.pos.z, io->pos.x,
			               io->pos.y, io->pos.z) < dist_Threshold)
			{
				LASTINTERCLICKNB = INTERNMB;
				return io;
			}
		}
		else if ((Project.telekinesis) || (EDITMODE))
		{
			LASTINTERCLICKNB = INTERNMB;
			return io;
		}
		else if (IsEquipedByPlayer(io)
		         || (Distance3D(player.pos.x, player.pos.y, player.pos.z, io->pos.x,
		                        io->pos.y, io->pos.z) < dist_Threshold))
		{
			LASTINTERCLICKNB = INTERNMB;
			return io;
		}
	}

	return NULL;
}

//*************************************************************************************
// Need To upgrade to a more precise collision.
//*************************************************************************************
long IsCollidingAnyInter(float x, float y, float z, EERIE_3D * size)
{
	EERIE_3D pos;

	for (long i = 0; i < inter.nbmax; i++)
	{
		INTERACTIVE_OBJ * io = inter.iobj[i];

		if ((io)
		        && (!(io->ioflags & IO_NO_COLLISIONS))
		        && (io->collision)
		        && (io->GameFlags & GFLAG_ISINTREATZONE)
		        && (io != CURRENTINTER)
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
BOOL IsCollidingInter(INTERACTIVE_OBJ * io, EERIE_3D * pos)
{
	long nbv;
	long idx;
	EERIE_VERTEX * vlist;

	if ((!io)
	        ||	(!io->obj))
		return FALSE;

	if (Distance3D(pos->x, pos->y, pos->z, io->pos.x, io->pos.y, io->pos.z) < 190.f)
	{
		vlist = io->obj->vertexlist3;
		nbv = io->obj->nbvertex;

		if (io->obj->nbgroups > 4)
		{
			for (long i = 0; i < io->obj->nbgroups; i++)
			{
				idx = io->obj->grouplist[i].origin;

				if (Distance3D(pos->x, pos->y, pos->z, vlist[idx].v.x, vlist[idx].v.y, vlist[idx].v.z) <= 50.f)
					return TRUE;
			}
		}
		else
		{
			for (long i = 0; i < nbv; i++)
			{
				if (i != io->obj->origin)
					if (Distance3D(pos->x, pos->y, pos->z, vlist[i].v.x, vlist[i].v.y, vlist[i].v.z) <= 30.f)
						return TRUE;
			}
		}
	}

	return FALSE;
}

void SetYlsideDeath(INTERACTIVE_OBJ * io)
{
	io->sfx_flag = SFX_TYPE_YLSIDE_DEATH;
	io->sfx_time = ARXTimeUL(); 	
}
BOOL ARX_INTERACTIVE_CheckCollision(EERIE_3DOBJ * obj, long kk, long source)
{
	BOOL col = FALSE;
	float dist;
	long i, ret;
	long avoid = -1;
	INTERACTIVE_OBJ * io_source = NULL;

	if (ValidIONum(source))
	{
		io_source = inter.iobj[source];
		avoid = io_source->no_collide;

	}

	for (i = 1; i < inter.nbmax; i++)
	{
		INTERACTIVE_OBJ * io = inter.iobj[i];

		if (
		    (io)
		    && (i != avoid)
		    && (!(io->ioflags & (IO_CAMERA | IO_MARKER | IO_ITEM)))
		    && (io->show == SHOW_FLAG_IN_SCENE)
		    && !(io->ioflags & IO_NO_COLLISIONS)
		    && (io->obj)
		    && (io->obj != obj)
		    && (!io->usepath)
		)
		{
			dist = EEDistance3D(&io->pos, &obj->pbox->vert[0].pos);

			if ((dist < 450.f) && (In3DBBoxTolerance(&obj->pbox->vert[kk].pos, &io->bbox3D, obj->pbox->radius)))
			{

				ret = -1;

				if ((io->ioflags & IO_NPC) && (io->_npcdata->life > 0.f))
				{
					if (PointInCylinder(&io->physics.cyl, &obj->pbox->vert[kk].pos))
					{
						return TRUE;
					}
				}
				else if (io->ioflags & IO_FIX)
				{
					long step;
					long nbv;
					nbv = io->obj->nbvertex;

					if (nbv < 300) step = 1;
					else if (nbv < 600) step = 2;
					else if (nbv < 1200) step = 4;
					else step = 6;

					EERIE_VERTEX * vlist = io->obj->vertexlist3;

					EERIE_SPHERE sp;
					sp.radius = 22.f; 

					for (long ii = 1; ii < nbv; ii += step)
					{
						if (ii != io->obj->origin)
						{
							sp.origin.x = vlist[ii].v.x;
							sp.origin.y = vlist[ii].v.y;
							sp.origin.z = vlist[ii].v.z;

							if (EEDistance3D(&obj->pbox->vert[kk].pos, &sp.origin) < sp.radius)
							{
								if ((io_source) && (io->GameFlags & GFLAG_DOOR))
								{
									if (ARXTime > io->collide_door_time + 500)
									{
										EVENT_SENDER = io_source;
										io->collide_door_time = ARXTimeUL(); 	
										SendIOScriptEvent(io, SM_COLLIDE_DOOR, "", NULL);
										EVENT_SENDER = io;
										io->collide_door_time = ARXTimeUL(); 	
										SendIOScriptEvent(io_source, SM_COLLIDE_DOOR, "", NULL);
									}
								}

								return TRUE;
								col = TRUE;
								ret = 1;
							}
						}
					}
				}
			}
		}
	}

	return col;
}
BOOL ARX_INTERACTIVE_CheckFULLCollision(EERIE_3DOBJ * obj, long source)
{
	BOOL col = FALSE;
	float dist;
	long i, ret;
	long avoid = -1;
	INTERACTIVE_OBJ * io_source = NULL;
	INTERACTIVE_OBJ * io = NULL;

	if (ValidIONum(source))
	{
		io_source = inter.iobj[source];
		avoid = io_source->no_collide;

	}

	for (i = 0; i < TREATZONE_CUR; i++)
	{
		if	((treatio[i].show != SHOW_FLAG_IN_SCENE)
		        ||	((treatio[i].ioflags & IO_NO_COLLISIONS))
		        || (!treatio[i].io)) continue;

		io = treatio[i].io;

		if ((io == io_source) || (!io->obj) || (io == inter.iobj[0])) 
			continue;

		if (treatio[i].num == avoid) continue;

		if ((io->ioflags  & (IO_CAMERA | IO_MARKER | IO_ITEM))
		        ||	(io->usepath))
			continue;

		if ((io->ioflags & IO_NPC)
		        &&	(io_source)
		        &&	(io_source->ioflags & IO_NO_NPC_COLLIDE))
			continue;

		
		dist = EEDistance3D(&io->pos, &obj->pbox->vert[0].pos);

		if ((dist < 600.f) && (In3DBBoxTolerance(&obj->pbox->vert[0].pos, &io->bbox3D, obj->pbox->radius)))
		{
			ret = -1;

			if ((io->ioflags & IO_NPC) && (io->_npcdata->life > 0.f))
			{
				for (long kk = 0; kk < obj->pbox->nb_physvert; kk++)
					if (PointInCylinder(&io->physics.cyl, &obj->pbox->vert[kk].pos))
					{
						return TRUE;
					}
			}
			else if (io->ioflags & IO_FIX)
			{
				long step;
				long nbv;
				nbv = io->obj->nbvertex;
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

				EERIE_VERTEX * vlist = io->obj->vertexlist3;


				if (io->GameFlags & GFLAG_PLATFORM)
				{
					for (long kk = 0; kk < obj->pbox->nb_physvert; kk++)
					{
						EERIE_SPHERE sphere;
						sphere.origin.x = obj->pbox->vert[kk].pos.x;
						sphere.origin.y = obj->pbox->vert[kk].pos.y;
						sphere.origin.z = obj->pbox->vert[kk].pos.z;
						sphere.radius = 30.f;
						float miny, maxy;
						miny = io->bbox3D.min.y;
						maxy = io->bbox3D.max.y;

						if ((maxy <= sphere.origin.y + sphere.radius)
						        ||	(miny >= sphere.origin.y))
							if (In3DBBoxTolerance(&sphere.origin, &io->bbox3D, sphere.radius))
							{
								if (Distance2D(io->pos.x, io->pos.z, sphere.origin.x, sphere.origin.z) < 440.f + sphere.radius)
								{
									EERIEPOLY ep;
									ep.type = 0;

									for (long ii = 0; ii < io->obj->nbfaces; ii++)
									{
										float cx = 0;
										float cz = 0;

										for (long idx = 0 ; idx < 3 ; idx++)
										{
											cx			+=	ep.v[idx].sx	=	io->obj->vertexlist3[ io->obj->facelist[ii].vid[idx] ].v.x;
											ep.v[idx].sy	=	io->obj->vertexlist3[ io->obj->facelist[ii].vid[idx] ].v.y;
											cz			+=	ep.v[idx].sz	=	io->obj->vertexlist3[ io->obj->facelist[ii].vid[idx] ].v.z;
										}

										cx *= DIV3;
										cz *= DIV3;

										for (kk = 0; kk < 3; kk++)
										{
											ep.v[kk].sx = (ep.v[kk].sx - cx) * 3.5f + cx;
											ep.v[kk].sz = (ep.v[kk].sz - cz) * 3.5f + cz;
										}

										if (PointIn2DPolyXZ(&ep, sphere.origin.x, sphere.origin.z))
										{
											return TRUE;
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
								sp.origin.x = (vlist[ii].v.x + vlist[jii].v.x) * DIV2;
								sp.origin.y = (vlist[ii].v.y + vlist[jii].v.y) * DIV2;
								sp.origin.z = (vlist[ii].v.z + vlist[jii].v.z) * DIV2;

								for (long kk = 0; kk < obj->pbox->nb_physvert; kk++)
									if (EEDistance3D(&obj->pbox->vert[kk].pos, &sp.origin) < sp.radius)
									{
										if ((io_source) && (io->GameFlags & GFLAG_DOOR))
										{
											if (ARXTime > io->collide_door_time + 500)
											{
												EVENT_SENDER = io_source;
												io->collide_door_time = ARXTimeUL(); 
												SendIOScriptEvent(io, SM_COLLIDE_DOOR, "", NULL);
												EVENT_SENDER = io;
												io->collide_door_time = ARXTimeUL(); 	
												SendIOScriptEvent(io_source, SM_COLLIDE_DOOR, "", NULL);
											}
										}

										return TRUE;
									}
							}
						}

						sp.origin.x = vlist[ii].v.x;
						sp.origin.y = vlist[ii].v.y;
						sp.origin.z = vlist[ii].v.z;

						for (long kk = 0; kk < obj->pbox->nb_physvert; kk++)
							if (EEDistance3D(&obj->pbox->vert[kk].pos, &sp.origin) < sp.radius)
							{
								if ((io_source) && (io->GameFlags & GFLAG_DOOR))
								{
									if (ARXTime > io->collide_door_time + 500)
									{
										EVENT_SENDER = io_source;
										io->collide_door_time = ARXTimeUL(); 	
										SendIOScriptEvent(io, SM_COLLIDE_DOOR, "", NULL);
										EVENT_SENDER = io;
										io->collide_door_time = ARXTimeUL(); 	
										SendIOScriptEvent(io_source, SM_COLLIDE_DOOR, "", NULL);
									}
								}
								return TRUE;
							}
					}
				}
			}
		}
		
	}

	return col;
}

void UpdateCameras()
{
	ARX_TIME_Get();

	for (long i = 1; i < inter.nbmax; i++)
	{
		INTERACTIVE_OBJ * io = inter.iobj[i];

		if (io)
		{
			if (io->usepath) // interpolate & send events
			{

				ARX_USE_PATH * aup = (ARX_USE_PATH *)io->usepath;
				float diff = ARXTime - aup->_curtime;

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
						sprintf(str, "%d", aup->path->nb_pathways - 1);
						EVENT_SENDER = NULL;
						SendIOScriptEvent(io, SM_WAYPOINT, str, NULL);
						sprintf(str, "WAYPOINT%d", aup->path->nb_pathways - 1);
						SendIOScriptEvent(io, 0, "", str);
						SendIOScriptEvent(io, SM_PATHEND, "", NULL);
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
						sprintf(str, "%d", ii);
						EVENT_SENDER = NULL;
						SendIOScriptEvent(io, SM_WAYPOINT, str, NULL);
						sprintf(str, "WAYPOINT%d", ii);
						SendIOScriptEvent(io, 0, "", str);

						if (ii == aup->path->nb_pathways)
						{
							SendIOScriptEvent(io, SM_PATHEND, "", NULL);
						}
						
						aup->lastWP = last + 1;
					}
				}

				if ((io->damager_damages > 0)
				        &&	(io->show == SHOW_FLAG_IN_SCENE))
				{
					for (long ii = 0; ii < inter.nbmax; ii++)
					{
						INTERACTIVE_OBJ * ioo = inter.iobj[ii];

						if ((ioo)
						        &&	(ii != i)
						        &&	(ioo->show == SHOW_FLAG_IN_SCENE)
						        &&	(ioo->ioflags & IO_NPC)
						        &&	(EEDistance3D(&io->pos, &ioo->pos) < 600.f))
						{
							bool Touched = false;

							for (long ri = 0; ri < io->obj->nbvertex; ri += 3)
							{
								for (long rii = 0; rii < ioo->obj->nbvertex; rii += 3)
								{
									if (EEDistance3D(&io->obj->vertexlist3[ri].v,
									                 &ioo->obj->vertexlist3[rii].v) < 20.f)
									{
										Touched = true;
										ri = 999999;
										rii = 999999;
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

			if (io->ioflags & IO_CAMERA)
			{
				inter.iobj[i]->_camdata->cam.pos.x = io->pos.x;
				inter.iobj[i]->_camdata->cam.pos.y = io->pos.y;
				inter.iobj[i]->_camdata->cam.pos.z = io->pos.z;

				if (io->targetinfo != TARGET_NONE) // Follows target
				{
					GetTargetPos(io, (unsigned long)inter.iobj[i]->_camdata->cam.smoothing);
					io->target.x += io->_camdata->cam.translatetarget.x;
					io->target.y += io->_camdata->cam.translatetarget.y;
					io->target.z += io->_camdata->cam.translatetarget.z;

					if ((io->_camdata->cam.lastinfovalid) && (io->_camdata->cam.smoothing != 0.f))
					{
						EERIE_3D smoothtarget;
 
						float vv = (float)io->_camdata->cam.smoothing;

						if (vv > 8000) vv = 8000;

						vv = (8000 - vv) * DIV4000;
						float vll = _framedelay * DIV1000 * vv;
						EERIE_3D oldvector;
						EERIE_3D newvector;
						oldvector.x = io->_camdata->cam.lasttarget.x - io->_camdata->cam.lastpos.x;
						oldvector.y = io->_camdata->cam.lasttarget.y - io->_camdata->cam.lastpos.y;
						oldvector.z = io->_camdata->cam.lasttarget.z - io->_camdata->cam.lastpos.z;

						newvector.x = io->target.x - io->_camdata->cam.pos.x;
						newvector.y = io->target.y - io->_camdata->cam.pos.y;
						newvector.z = io->target.z - io->_camdata->cam.pos.z;

						EEDistance3D(&oldvector, &newvector);
						float f1 = vll; 

						if (f1 > 1.f) f1 = 1.f;

						float f2 = 1.f - f1;
						smoothtarget.x = io->target.x * f2 + io->_camdata->cam.lasttarget.x * f1;
						smoothtarget.y = io->target.y * f2 + io->_camdata->cam.lasttarget.y * f1;
						smoothtarget.z = io->target.z * f2 + io->_camdata->cam.lasttarget.z * f1;

						SetTargetCamera(&io->_camdata->cam, smoothtarget.x, smoothtarget.y, smoothtarget.z);
						io->_camdata->cam.lasttarget.x = smoothtarget.x;
						io->_camdata->cam.lasttarget.y = smoothtarget.y;
						io->_camdata->cam.lasttarget.z = smoothtarget.z;
						io->_camdata->cam.lastinfovalid = TRUE;
						io->_camdata->cam.lastpos.x = io->_camdata->cam.pos.x;
						io->_camdata->cam.lastpos.y = io->_camdata->cam.pos.y;
						io->_camdata->cam.lastpos.z = io->_camdata->cam.pos.z;
					}
					else
					{
						if ((io->target.x == io->_camdata->cam.pos.x)
						        &&	(io->target.y == io->_camdata->cam.pos.y)
						        &&	(io->target.z == io->_camdata->cam.pos.z))
						{
						}
						else SetTargetCamera(&io->_camdata->cam, io->target.x, io->target.y, io->target.z);

						io->_camdata->cam.lasttarget.x = io->target.x;
						io->_camdata->cam.lasttarget.y = io->target.y;
						io->_camdata->cam.lasttarget.z = io->target.z;
						io->_camdata->cam.lastinfovalid = TRUE;
						io->_camdata->cam.lastpos.x = io->_camdata->cam.pos.x;
						io->_camdata->cam.lastpos.y = io->_camdata->cam.pos.y;
						io->_camdata->cam.lastpos.z = io->_camdata->cam.pos.z;
					}

					io->_camdata->cam.angle.b -= 180.f;
					io->_camdata->cam.angle.a = -io->_camdata->cam.angle.a;
					io->angle.a = 0.f; 
					io->angle.b = io->_camdata->cam.angle.b + 90.f;
					io->angle.g = 0.f; 
				}	
				else // no target...
				{
					float tr = DEG2RAD(MAKEANGLE(io->angle.b + 90));
					io->target.x = io->pos.x - (float)EEsin(tr) * 20.f;
					io->target.y = io->pos.y; 
					io->target.z = io->pos.z + (float)EEcos(tr) * 20.f;
					SetTargetCamera(&io->_camdata->cam, io->target.x, io->target.y, io->target.z);
					io->_camdata->cam.lasttarget.x = io->target.x;
					io->_camdata->cam.lasttarget.y = io->target.y;
					io->_camdata->cam.lasttarget.z = io->target.z;
					io->_camdata->cam.lastinfovalid = TRUE;
					io->_camdata->cam.lastpos.x = io->_camdata->cam.pos.x;
					io->_camdata->cam.lastpos.y = io->_camdata->cam.pos.y;
					io->_camdata->cam.lastpos.z = io->_camdata->cam.pos.z;
				}
			}
		}
	}
}
void ARX_INTERACTIVE_UnfreezeAll()
{
	if (inter.iobj)
	{
		for (long i = 0; i < inter.nbmax; i++)
		{
			if (inter.iobj[i] != NULL)
				inter.iobj[i]->ioflags &= ~IO_FREEZESCRIPT;
		}
	}
}
void UpdateIOInvisibility(INTERACTIVE_OBJ * io)
{
	if (io && (io->invisibility <= 1.f))
	{
		if ((io->GameFlags & GFLAG_INVISIBILITY) && (io->invisibility < 1.f))
		{
			io->invisibility += _framedelay * DIV1000;

			if (io->invisibility > 1.f) io->invisibility = 1.f;
		}
		else if ((!(io->GameFlags & GFLAG_INVISIBILITY)) && (io->invisibility != 0.f))
		{
			io->invisibility -= _framedelay * DIV1000;

			if (io->invisibility < 0.f) io->invisibility = 0.f;
		}
	}
}
extern INTERACTIVE_OBJ * DESTROYED_DURING_RENDERING;
//*************************************************************************************
// Renders Interactive objects.
// Will render objects between distance "from" (included)
// to distance "to" (not included)
// from camera position.
//*************************************************************************************

extern CDirectInput * pGetInfoDirectInput;
extern TextureContainer TexMetal;
extern long FINAL_COMMERCIAL_DEMO;
bool bRenderInterList = true; //false;
void RenderInter(LPDIRECT3DDEVICE7 pd3dDevice, float from, float to, long flags)
{

	SETTEXTUREWRAPMODE(pd3dDevice, D3DTADDRESS_CLAMP);
	float val = -0.6f;
	pd3dDevice->SetTextureStageState(0, D3DTSS_MIPMAPLODBIAS, *((LPDWORD)(&val)));
	EERIE_3D temp;
	EERIEMATRIX mat;
	INTER_DRAW = 0;
	INTER_COMPUTE = 0;
	float dist;
	long diff;

	if (inter.iobj[0] && (inter.iobj[0]->ignition > 0.f))
	{
		ManageIgnition(inter.iobj[0]);
	}

	for (long i = 1; i < inter.nbmax; i++) // Player isn't rendered here...
	{
		if (i == 379)
			i = i;

		INTERACTIVE_OBJ * io = inter.iobj[i];

		if ((io)
		        &&	(io != DRAGINTER)
		        &&	(io->GameFlags & GFLAG_ISINTREATZONE))
		{
			if ((i == 0) && ((player.Interface & INTER_MAP) && (!(player.Interface & INTER_COMBATMODE)))
			        && (Book_Mode == 0)) continue;

			if (io->show != SHOW_FLAG_IN_SCENE) continue;

			if (!ForceIODraw)
			{
				if ((Project.hide & HIDE_NPC) && (io->ioflags & IO_NPC)) continue;

				if ((Project.hide & HIDE_ITEMS) && (io->ioflags & IO_ITEM)) continue;

				if ((Project.hide & HIDE_FIXINTER) && (io->ioflags & IO_FIX)) continue;
			}

			if ((Project.hide & HIDE_CAMERAS) && (io->ioflags & IO_CAMERA)) continue;

			if (!EDITMODE)
			{
				if (io->ioflags & IO_CAMERA) continue;

				if (io->ioflags & IO_MARKER) continue;
			}

			if ((io->obj) &&
			        (io->obj->pbox) &&
			        (io->obj->pbox->active))  
			{
				dist = EEDistance3D(&ACTIVECAM->pos, &io->obj->pbox->vert[0].pos);
			}
			else dist = EEDistance3D(&ACTIVECAM->pos, &io->pos);

			if ((0) && (inter.iobj[i]->stepmaterial))
			{
				free(inter.iobj[i]->stepmaterial);
				inter.iobj[i]->stepmaterial = NULL;
			}

			if ((io) && (io->ioflags & IO_NPC) && (io->_npcdata->pathfind.flags  & PATHFIND_ALWAYS))
			{
			}
			else if ((dist < from) || (dist >= to)) continue;

			UpdateIOInvisibility(io);

			io->bbox1.x = 9999;
			io->bbox2.x = -1;

			if ((io->obj)
			        &&	(io->obj->pbox))
				EERIE_PHYSICS_BOX_Show(io->obj, &io->pos);

			if (
			    (io->obj)
			    &&	(io->obj->pbox)
			    &&	(io->obj->pbox->active))
			{
				EERIE_3D tmp;
				tmp.x = (io->obj->pbox->vert[14].pos.x - io->obj->pbox->vert[13].pos.x);
				tmp.y = (io->obj->pbox->vert[14].pos.y - io->obj->pbox->vert[13].pos.y);
				tmp.z = (io->obj->pbox->vert[14].pos.z - io->obj->pbox->vert[13].pos.z);
				EERIE_3D up;

				up.x = io->obj->pbox->vert[2].pos.x - io->obj->pbox->vert[1].pos.x;
				up.y = io->obj->pbox->vert[2].pos.y - io->obj->pbox->vert[1].pos.y;
				up.z = io->obj->pbox->vert[2].pos.z - io->obj->pbox->vert[1].pos.z;

				up.x += io->obj->pbox->vert[3].pos.x - io->obj->pbox->vert[4].pos.x;
				up.y += io->obj->pbox->vert[3].pos.y - io->obj->pbox->vert[4].pos.y;
				up.z += io->obj->pbox->vert[3].pos.z - io->obj->pbox->vert[4].pos.z;

				up.x += io->obj->pbox->vert[10].pos.x - io->obj->pbox->vert[9].pos.x;
				up.y += io->obj->pbox->vert[10].pos.y - io->obj->pbox->vert[9].pos.y;
				up.z += io->obj->pbox->vert[10].pos.z - io->obj->pbox->vert[9].pos.z;

				up.x += io->obj->pbox->vert[11].pos.x - io->obj->pbox->vert[12].pos.x;
				up.y += io->obj->pbox->vert[11].pos.y - io->obj->pbox->vert[12].pos.y;
				up.z += io->obj->pbox->vert[11].pos.z - io->obj->pbox->vert[12].pos.z;

				up.x *= DIV4;
				up.y *= DIV4;
				up.z *= DIV4;
				MatrixSetByVectors(&mat, &up, &tmp);
				mat._14 = mat._24 = mat._34 = 0.f;
				mat._41 = mat._42 = mat._43 = mat._44 = 0.f;


			}
				
			if (io->animlayer[0].cur_anim)
			{
				if (ForceIODraw && (dist > 2200.f)) continue;


				temp.a = io->angle.a;

				if (io->ioflags & IO_NPC)
					temp.b = MAKEANGLE(180.f - io->angle.b);
				else 
					temp.b = MAKEANGLE(270.f - io->angle.b);

				temp.g = io->angle.g;

				if (io->animlayer[0].flags & EA_PAUSED)
					diff = 0;
				else diff = ARX_CLEAN_WARN_CAST_LONG(FrameDiff);

				if ((io == FlyingOverIO)
				        &&	(!(io->ioflags & IO_NPC))
				        &&	io->obj)
					io->obj->drawflags |= DRAWFLAG_HIGHLIGHT;

				if (io->targetinfo >= 0)
				{
					if ((io->ioflags & IO_NPC)
					        && !(io->_npcdata->behavior & BEHAVIOUR_STARE_AT)
					        && !(io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND)
					        && !(io->_npcdata->behavior & BEHAVIOUR_LOOK_FOR) && (io->_npcdata->behavior & BEHAVIOUR_FIGHT))
						LOOK_AT_TARGET = 1;
				}

				EERIE_3D pos;
				Vector_Copy(&pos, &io->pos);

				if (io->ioflags & IO_NPC)
				{
					ComputeVVPos(io);
					pos.y = io->_npcdata->vvpos;
				}

				long flgs;

				if (!(EDITMODE) &&
				        (ARX_SCENE_PORTAL_Basic_ClipIO(io)))
					flgs = 4;
				else flgs = 0;

				EERIEDrawAnimQuat(pd3dDevice,	io->obj,
				                  &io->animlayer[0],
				                  &temp, &pos, diff, io, flgs);
				LOOK_AT_TARGET = 0;

				if (DESTROYED_DURING_RENDERING)
					continue;

				if (io->obj)
					io->obj->drawflags &= ~DRAWFLAG_HIGHLIGHT;
			}
			else 
			{
				if ((!EDITMODE) && (ARX_SCENE_PORTAL_Basic_ClipIO(io))) continue;

				if (ForceIODraw && (dist > ACTIVECAM->cdepth * fZFogEnd)) continue;

				temp.a = io->angle.a;

				if (io->ioflags & IO_NPC)
					temp.b = MAKEANGLE(180.f - io->angle.b);
				else	temp.b = MAKEANGLE(270.f - io->angle.b);

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
							DrawEERIEInterMatrix(pd3dDevice, io->obj, &mat, &io->pos, io, NULL);
						}
						else
						{
							DrawEERIEInter(pd3dDevice, io->obj, &temp, &io->pos, io);
						}

						if (DESTROYED_DURING_RENDERING)
							continue;

						io->obj->drawflags &= ~DRAWFLAG_HIGHLIGHT;

					}
				}
			}

			if ((io->ignition > 0.f) || (io->ioflags & IO_FIERY))
				ManageIgnition(io);

			if ((NEED_TEST_TEXT || EDITMODE) && (!FOR_EXTERNAL_PEOPLE))
			{
				D3DCOLOR color;

				if ((EDITMODE && (io->EditorFlags & EFLAG_SELECTED))) 
					color = 0xFFFFFF00;
				else
					color = 0xFF0000FF;

				if ((io->bbox1.x != io->bbox2.x) && (io->bbox1.x < DANAESIZX))
				{
					EERIEDraw2DLine(pd3dDevice, io->bbox1.x, io->bbox1.y, io->bbox2.x, io->bbox1.y, 0.01f, color);
					EERIEDraw2DLine(pd3dDevice, io->bbox2.x, io->bbox1.y, io->bbox2.x, io->bbox2.y, 0.01f, color);
					EERIEDraw2DLine(pd3dDevice, io->bbox2.x, io->bbox2.y, io->bbox1.x, io->bbox2.y, 0.01f, color);
					EERIEDraw2DLine(pd3dDevice, io->bbox1.x, io->bbox2.y, io->bbox1.x, io->bbox1.y, 0.01f, color);
				}
			}
		}
	}


	SETTEXTUREWRAPMODE(pd3dDevice, D3DTADDRESS_WRAP);
	val = -0.3f;
	pd3dDevice->SetTextureStageState(0, D3DTSS_MIPMAPLODBIAS, *((LPDWORD)(&val)));
}

void ARX_INTERACTIVE_DestroyIO(INTERACTIVE_OBJ * ioo)
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
			long numm = GetInterNum(ioo);

			if (ValidIONum(numm))
				ARX_SPELLS_FizzleAllSpellsFromCaster(numm);

			// Need To Kill timers
			ARX_SCRIPT_Timer_Clear_By_IO(ioo);
			ioo->show = SHOW_FLAG_DESTROYED;
			ioo->GameFlags &= ~GFLAG_ISINTREATZONE;

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
						INTERACTIVE_OBJ * iooo = (INTERACTIVE_OBJ *)eobj->linked[k].io;

						if ((iooo) && ValidIOAddress(iooo))
						{
							EERIE_LINKEDOBJ_UnLinkObjectFromObject(ioo->obj, iooo->obj);
							ARX_INTERACTIVE_DestroyIO(iooo);
						}
					}
				}
			}

			ARX_INTERACTIVE_DestroyDynamicInfo(ioo);

			if (ioo->scriptload)
			{
				long num = GetInterNum(ioo);
				ReleaseInter(ioo);

				if (ValidIONum(num))
					inter.iobj[num] = NULL;
			}
		}
	}
}

//*************************************************************************************
//
//*************************************************************************************
BOOL IsSameObject(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * ioo)
{
	if ((io == NULL)
	        ||	(ioo == NULL)
	        ||	(stricmp(io->filename, ioo->filename))
	        ||	(io->ioflags & IO_UNIQUE)
	        ||	(io->durability != ioo->durability)
	        ||	(io->max_durability != ioo->max_durability))
		return FALSE;

	if	((io->ioflags & IO_ITEM)
	        &&	(ioo->ioflags & IO_ITEM)
	        &&	(io->over_script.data == NULL)
	        &&	(ioo->over_script.data == NULL))
	{
		if ((io->locname) && (ioo->locname))
		{
			if (strcmp(io->locname, ioo->locname) == 0)
				return TRUE;
		}
		else
			return TRUE;
	}

	return FALSE;
}
BOOL HaveCommonGroup(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * ioo)
{
	if ((!io) || (!ioo)) return FALSE;

	for (long i = 0; i < io->nb_iogroups; i++)
	{
		for (long k = 0; k < ioo->nb_iogroups; k++)
		{
			if (!stricmp(io->iogroups[i].name, ioo->iogroups[k].name))
				return TRUE;
		}
	}

	return FALSE;
}

float ARX_INTERACTIVE_GetArmorClass(INTERACTIVE_OBJ * io)
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
		INTERACTIVE_OBJ * io = inter.iobj[t];

		if ((io == DRAGINTER)
		        ||	(io->show != SHOW_FLAG_IN_SCENE))
			return;

		float yy;
		EERIEPOLY * ep = CheckInPoly(io->pos.x, io->pos.y, io->pos.z, &yy);

		if ((ep) && (yy - io->pos.y < 10.f))
			return;

		io->obj->pbox->active = 1;
		io->obj->pbox->stopcount = 0;
		EERIE_3D pos;
		pos.x = io->pos.x;
		pos.z = io->pos.z;
		pos.y = io->pos.y;
		io->velocity.x = 0.f;
		io->velocity.y = 0.f;
		io->velocity.z = 0.f;

		io->stopped = 1;
		EERIE_3D fallvector;
		fallvector.x = 0.f;
		fallvector.z = 0.f;
		fallvector.y = 0.000001f;
		io->show = SHOW_FLAG_IN_SCENE;
		io->soundtime = 0;
		io->soundcount = 0;
		EERIE_PHYSICS_BOX_Launch(io->obj, &pos, &fallvector);

	}
}
