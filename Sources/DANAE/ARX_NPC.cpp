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
// ARX_NPC
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX NPC Management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>

#include <ARX_NPC.h>
#include <HERMESMain.h>

#include <EERIEDraw.h>
#include <EERIEMath.h>
#include <EERIEObject.h>
#include <EERIEPhysicsBox.h>
#include <EERIEPathfinder.h>
#include <EERIEAnchors.h>

#include "ARX_Script.h"
#include "ARX_Collisions.h"
#include "ARX_Damages.h"
#include "ARX_Equipment.h"
#include "ARX_interface.h"
#include "ARX_interactive.h"
#include "ARX_Speech.h"
#include "ARX_Paths.h"
#include "ARX_Paths.h"
#include "ARX_Particles.h"
#include "ARX_Scene.h"
#include "ARX_Sound.h"
#include "ARX_Spells.h"
#include "ARX_Time.h"

#include "EERIECollisionSpheres.h"
#include "EERIEMeshTweak.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

void CheckNPCEx(INTERACTIVE_OBJ * io);

static const float ARX_NPC_ON_HEAR_MAX_DISTANCE_STEP(600.0F);
static const float ARX_NPC_ON_HEAR_MAX_DISTANCE_ITEM(800.0F);

extern long LastSelectedIONum;

extern long APPLY_PUSH;
void StareAtTarget(INTERACTIVE_OBJ * io);
#define RUN_WALK_RADIUS 450

void ARX_NPC_Kill_Spell_Launch(INTERACTIVE_OBJ * io)
{
	if (io)
	{
		if (io->flarecount)
		{
			for (long i = 0; i < MAX_FLARES; i++)
			{
				if ((flare[i].exist)  && (flare[i].io == io))
					flare[i].io = NULL;
			}
		}

		io->spellcast_data.castingspell = -1;
	}
}
//***********************************************************************************************
// Releases Pathfinder info from an NPC
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/15)
//***********************************************************************************************
void ARX_NPC_ReleasePathFindInfo(INTERACTIVE_OBJ * io)
{
	// Checks for valid IO/NPC
	if ((!io)
	        ||	(!(io->ioflags & IO_NPC)))
		return;

	// Releases data & resets vars
	if (io->_npcdata->pathfind.list)
		free(io->_npcdata->pathfind.list);

	io->_npcdata->pathfind.list = NULL;
	io->_npcdata->pathfind.listnb = -1;
	io->_npcdata->pathfind.listpos = 0;
	io->_npcdata->pathfind.pathwait = 0;
}

//***********************************************************************************************
// Creates an extra rotations structure for a NPC
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/15)
//***********************************************************************************************
void ARX_NPC_CreateExRotateData(INTERACTIVE_OBJ * io)
{
	if ((!io)
	        ||	(!(io->ioflags & IO_NPC))
	        ||	(io->_npcdata->ex_rotate))
		return;

	io->_npcdata->ex_rotate = (EERIE_EXTRA_ROTATE *)malloc(sizeof(EERIE_EXTRA_ROTATE));
	io->head_rot = 0;

	if (io->_npcdata->ex_rotate)
	{
		io->_npcdata->ex_rotate->group_number[0] = (short)EERIE_OBJECT_GetGroup(io->obj, "head");
		io->_npcdata->ex_rotate->group_number[1] = (short)EERIE_OBJECT_GetGroup(io->obj, "neck");
		io->_npcdata->ex_rotate->group_number[2] = (short)EERIE_OBJECT_GetGroup(io->obj, "chest");
		io->_npcdata->ex_rotate->group_number[3] = (short)EERIE_OBJECT_GetGroup(io->obj, "belt");

		for (long n = 0; n < MAX_EXTRA_ROTATE; n++)
		{
			io->_npcdata->ex_rotate->group_rotate[n].a = 0;
			io->_npcdata->ex_rotate->group_rotate[n].b = 0;
			io->_npcdata->ex_rotate->group_rotate[n].g = 0;
		}

		io->_npcdata->ex_rotate->flags = 0;
	}

	io->_npcdata->look_around_inc = 0.f;
}
//***********************************************************************************************
// Resurects an NPC
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/15)
//***********************************************************************************************
void ARX_NPC_Revive(INTERACTIVE_OBJ * io, long flags)
{
	if ((TSecondaryInventory) && (TSecondaryInventory->io == io))
	{
		TSecondaryInventory = NULL;
	}

	ARX_SCRIPT_SetMainEvent(io, "MAIN");

	if (io->ioflags & IO_NPC)
	{
		io->ioflags &= ~IO_NO_COLLISIONS;
		io->_npcdata->life = io->_npcdata->maxlife;
		ARX_SCRIPT_ResetObject(io, 1);
		io->_npcdata->life = io->_npcdata->maxlife;
	}

	if (flags & 1)
	{
		io->room_flags |= 1;
		io->pos.x = io->initpos.x;
		io->pos.y = io->initpos.y;
		io->pos.z = io->initpos.z;
	}

	long goretex = -1;

	for (long i = 0; i < io->obj->nbmaps; i++)
	{
		if (io->obj->texturecontainer
		        &&	io->obj->texturecontainer[i]
		        &&	(IsIn(io->obj->texturecontainer[i]->m_strName, "GORE")))
		{
			goretex = i;
			break;
		}
	}

	for (long ll = 0; ll < io->obj->nbfaces; ll++)
	{
		if (io->obj->facelist[ll].texid != goretex)
		{
			io->obj->facelist[ll].facetype &= ~POLY_HIDE;
		}
		else
		{
			io->obj->facelist[ll].facetype |= POLY_HIDE;
		}
	}

	if (io->ioflags & IO_NPC)
		io->_npcdata->cuts = 0;
}
//***********************************************************************************************
// Sets a new behaviour for NPC
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/15)
//***********************************************************************************************
void ARX_NPC_Behaviour_Change(INTERACTIVE_OBJ * io, long behavior, long behavior_param)
{
	if ((!io)
	        ||	(!(io->ioflags & IO_NPC)))
		return;

	if ((io->_npcdata->behavior & BEHAVIOUR_FIGHT) && !(behavior & BEHAVIOUR_FIGHT))
	{
		ANIM_USE * ause1 = &io->animlayer[1];
		AcquireLastAnim(io);
		FinishAnim(io, ause1->cur_anim);
		ause1->cur_anim = NULL;
	}

	if ((behavior & BEHAVIOUR_NONE) || (behavior == 0))
	{
		ANIM_USE * ause0 = &io->animlayer[0];
		AcquireLastAnim(io);
		FinishAnim(io, ause0->cur_anim);
		ause0->cur_anim = NULL;
		ANIM_Set(ause0, io->anims[ANIM_DEFAULT]);
		ause0->flags &= ~EA_LOOP;

		ANIM_USE * ause1 = &io->animlayer[1];
		AcquireLastAnim(io);
		FinishAnim(io, ause1->cur_anim);
		ause1->cur_anim = NULL;
		ause1->flags &= ~EA_LOOP;

		ANIM_USE * ause2 = &io->animlayer[2];
		AcquireLastAnim(io);
		FinishAnim(io, ause2->cur_anim);
		ause2->cur_anim = NULL;
		ause2->flags &= ~EA_LOOP;


	}

	if (behavior & BEHAVIOUR_FRIENDLY)
	{
		ANIM_USE * ause0 = &io->animlayer[0];
		AcquireLastAnim(io);
		FinishAnim(io, ause0->cur_anim);
		ANIM_Set(ause0, io->anims[ANIM_DEFAULT]);
		ause0->altidx_cur = 0;
	}

	io->_npcdata->behavior = behavior;
	io->_npcdata->behavior_param = (float)behavior_param;
}
//***********************************************************************************************
// Resets all behaviour data from a NPC
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/15)
//***********************************************************************************************
void ARX_NPC_Behaviour_Reset(INTERACTIVE_OBJ * io)
{
	if ((!io)
	        ||	(!(io->ioflags & IO_NPC)))
		return;

	io->_npcdata->behavior = BEHAVIOUR_NONE;

	for (long i = 0; i < MAX_STACKED_BEHAVIOR; i++)
		io->_npcdata->stacked[i].exist = 0;
}
//***********************************************************************************************
// Reset all Behaviours from all NPCs
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/15)
//***********************************************************************************************
void ARX_NPC_Behaviour_ResetAll()
{
	for (long i = 0; i < inter.nbmax; i++)
	{
		if (inter.iobj[i])
			ARX_NPC_Behaviour_Reset(inter.iobj[i]);
	}
}
//***********************************************************************************************
// Stacks an NPC behaviour
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/15)
//***********************************************************************************************
void ARX_NPC_Behaviour_Stack(INTERACTIVE_OBJ * io)
{
	if ((!io)
	        ||	(!(io->ioflags & IO_NPC)))
		return;

	for (long i = 0; i < MAX_STACKED_BEHAVIOR; i++)
	{
		IO_BEHAVIOR_DATA * bd = &io->_npcdata->stacked[i];

		if (bd->exist == 0)
		{
			bd->behavior = io->_npcdata->behavior;
			bd->behavior_param = io->_npcdata->behavior_param;
			bd->tactics = io->_npcdata->tactics;

			if (io->_npcdata->pathfind.listnb > 0)
				bd->target = io->_npcdata->pathfind.truetarget;
			else
				bd->target = io->targetinfo;

			bd->movemode = io->_npcdata->movemode;
			bd->exist = 1;
			return;
		}
	}
}

//***********************************************************************************************
// Unstacks One stacked behaviour from an NPC
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/15)
//***********************************************************************************************
void ARX_NPC_Behaviour_UnStack(INTERACTIVE_OBJ * io)
{
	if ((!io)
	        ||	(!(io->ioflags & IO_NPC)))
		return;

	for (long i = MAX_STACKED_BEHAVIOR - 1; i >= 0; i--)
	{
		IO_BEHAVIOR_DATA * bd = &io->_npcdata->stacked[i];

		if (bd->exist)
		{
			AcquireLastAnim(io);
			io->_npcdata->behavior = bd->behavior;
			io->_npcdata->behavior_param = bd->behavior_param;
			io->_npcdata->tactics = bd->tactics;
			io->targetinfo = bd->target;
			io->_npcdata->movemode = bd->movemode;
			bd->exist = 0;
			ARX_NPC_LaunchPathfind(io, bd->target);

			if (io->_npcdata->behavior & BEHAVIOUR_NONE)
			{
				memcpy(io->animlayer, bd->animlayer, sizeof(ANIM_USE)*MAX_ANIM_LAYERS);
			}

			return;
		}
	}
}
extern void GetIOCyl(INTERACTIVE_OBJ * io, EERIE_CYLINDER * cyl);
//***********************************************************************************************
// long ARX_NPC_GetNextAttainableNodeIncrement(INTERACTIVE_OBJ * io)
// Description:
//			Checks for any direct shortcut between NPC and future anchors...
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/15)
//***********************************************************************************************
long ARX_NPC_GetNextAttainableNodeIncrement(INTERACTIVE_OBJ * io)
{
	if ((!io)
	        ||	(!(io->ioflags & IO_NPC))
	        ||	(io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND))
		return 0;

	float dist = EEDistance3D(&io->pos, &ACTIVECAM->pos);

	if (dist > ACTIVECAM->cdepth * DIV2)
		return 0;

	long MAX_TEST;

	if (dist < ACTIVECAM->cdepth * DIV4)
		MAX_TEST = 6; //4;
	else
		MAX_TEST = 4; //3;

	for (long l_try = MAX_TEST; l_try > 1; l_try--)
	{
		if (io->_npcdata->pathfind.listpos + l_try > io->_npcdata->pathfind.listnb - 1) continue;

		float tot = 0.f;

		for (long aa = l_try; aa > 1; aa--)
		{
			long v = io->_npcdata->pathfind.list[io->_npcdata->pathfind.listpos+aa];
			tot += ACTIVEBKG->anchors[v].nblinked;

			if (aa == l_try)
				tot += ACTIVEBKG->anchors[v].nblinked;
		}

		tot /= (float)(l_try + 1);

		if (tot <= 3.5f) continue;

		io->physics.startpos.x = io->pos.x;
		io->physics.startpos.y = io->pos.y;
		io->physics.startpos.z = io->pos.z;
		long pos = io->_npcdata->pathfind.list[io->_npcdata->pathfind.listpos+l_try];
		io->physics.targetpos.x = ACTIVEBKG->anchors[pos].pos.x;
		io->physics.targetpos.y = ACTIVEBKG->anchors[pos].pos.y;
		io->physics.targetpos.z = ACTIVEBKG->anchors[pos].pos.z;

		if (EEfabs(io->physics.startpos.y - io->physics.targetpos.y) > 60.f) continue;

		io->physics.targetpos.y += 60.f; // FAKE Gravity !
		IO_PHYSICS phys;
		memcpy(&phys, &io->physics, sizeof(IO_PHYSICS));
		GetIOCyl(io, &phys.cyl);

		// Now we try the physical move for real
		if	(Vector_Compare(&io->physics.startpos, &io->physics.targetpos) 
		        || ((ARX_COLLISION_Move_Cylinder(&phys, io, 40, CFLAG_JUST_TEST | CFLAG_NPC))))
		{
			float dist = EEDistance3D(&phys.cyl.origin, &ACTIVEBKG->anchors[pos].pos); 

			if (dist < 30)
			{
				return l_try;
			}
		}
	}

	return 0;
}
//*****************************************************************************
// Checks for nearest VALID anchor for a cylinder from a position
//*****************************************************************************
long AnchorData_GetNearest(EERIE_3D * pos, EERIE_CYLINDER * cyl)
{
	long returnvalue = -1;
	float distmax = 99999999.f;
	EERIE_BACKGROUND * eb = ACTIVEBKG;

	for (long i = 0; i < eb->nbanchors; i++)
	{
		if (eb->anchors[i].nblinked)
		{
			float d = EEDistance3D(&eb->anchors[i].pos, pos);

			if ((d < distmax) && (eb->anchors[i].height <= cyl->height)
			        && (eb->anchors[i].radius >= cyl->radius)
			        && (!(eb->anchors[i].flags & ANCHOR_FLAG_BLOCKED)))
			{
				returnvalue = i;
				distmax = d;
			}
		}
	}

	return returnvalue;
}
long AnchorData_GetNearest_2(float beta, EERIE_3D * pos, EERIE_CYLINDER * cyl)
{
	EERIE_3D vect;
	float d = DEG2RAD(beta);
	vect.x = -EEsin(d);
	vect.y = 0;
	vect.z = EEcos(d);
	TRUEVector_Normalize(&vect);

	EERIE_3D posi;
	posi.x = pos->x + vect.x * 50.f;
	posi.y = pos->y;
	posi.z = pos->z + vect.x * 50.f;
	return AnchorData_GetNearest(&posi, cyl);
}

long AnchorData_GetNearest_Except(EERIE_3D * pos, EERIE_CYLINDER * cyl, long except)
{
	long returnvalue = -1;
	float distmax = 99999999.f;
	EERIE_BACKGROUND * eb = ACTIVEBKG;

	for (long i = 0; i < eb->nbanchors; i++)
	{
		if (i == except) continue;

		if (eb->anchors[i].nblinked)
		{
			float d = EEDistance3D(&eb->anchors[i].pos, pos);

			if ((d < distmax) && (eb->anchors[i].height <= cyl->height)
			        && (eb->anchors[i].radius >= cyl->radius)
			        && (!(eb->anchors[i].flags & ANCHOR_FLAG_BLOCKED)))
			{
				returnvalue = i;
				distmax = d;
			}
		}
	}

	return returnvalue;
}

BOOL ARX_NPC_LaunchPathfind(INTERACTIVE_OBJ * io, long target)
{
	// Check Validity
	if ((!io) || (!(io->ioflags & IO_NPC)))
		return FALSE;

	long MUST_SELECT_Start_Anchor = -1;
	io->physics.cyl.origin.x = io->pos.x;
	io->physics.cyl.origin.y = io->pos.y;
	io->physics.cyl.origin.z = io->pos.z;
	long old_target = io->targetinfo;

	if	((!(io->ioflags & IO_PHYSICAL_OFF)))
	{
		if (!ForceNPC_Above_Ground(io))
		{
			io->_npcdata->pathfind.pathwait = 0;
			return FALSE;
		}
	}

	if (io->_npcdata->behavior & BEHAVIOUR_FRIENDLY)
	{
		io->targetinfo = target;
		io->_npcdata->pathfind.pathwait = 0;
		return FALSE;
	}

	EERIE_3D pos1, pos2;

	if (io->_npcdata->pathfind.listnb > 0)
	{
		io->_npcdata->pathfind.listnb = -1;
		io->_npcdata->pathfind.listpos = 0;
		io->_npcdata->pathfind.pathwait = 0;
		io->_npcdata->pathfind.truetarget = TARGET_NONE;

		if (io->_npcdata->pathfind.list) free(io->_npcdata->pathfind.list);

		io->_npcdata->pathfind.list = NULL;
	}

	if (io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND)
	{
		pos1.x = io->pos.x;
		pos1.y = io->pos.y;
		pos1.z = io->pos.z;
		pos2.x = io->pos.x + 1000.f;
		pos2.y = io->pos.y;
		pos2.z = io->pos.z + 1000.f;
		goto wander;
	}

	if ((target < 0) || (io->_npcdata->behavior & BEHAVIOUR_GO_HOME)) 
	{
		target = GetInterNum(io);

		if (target == -1)
			goto failure;

		io->_npcdata->pathfind.truetarget = target;
		pos1.x = io->pos.x;
		pos1.y = io->pos.y;
		pos1.z = io->pos.z;

		if (io->_npcdata->behavior & BEHAVIOUR_GO_HOME)
		{
			pos2.x = io->initpos.x;
			pos2.y = io->initpos.y;
			pos2.z = io->initpos.z;
		}
		else
		{
			pos2.x = pos1.x;
			pos2.y = pos1.y;
			pos2.z = pos1.z;
		}

		goto suite;
	}

	if ((ValidIONum(target))
	        &&	(inter.iobj[target] == io))
	{
		io->_npcdata->pathfind.pathwait = 0;
		return FALSE; // cannot pathfind self...
	}

	if (old_target != target)
		io->_npcdata->reachedtarget = 0;

	EVENT_SENDER = NULL;



	pos1.x = io->pos.x;
	pos1.y = io->pos.y;
	pos1.z = io->pos.z;


	if (io->_npcdata->behavior & BEHAVIOUR_GO_HOME)
	{
		pos2.x = io->initpos.x;
		pos2.y = io->initpos.y;
		pos2.z = io->initpos.z;
	}
	else
	{
		if (ValidIONum(target))
		{
			INTERACTIVE_OBJ * io2;
			io2 = inter.iobj[target];
			pos2.x = io2->pos.x;
			pos2.y = io2->pos.y;
			pos2.z = io2->pos.z;
		}
		else
		{
			pos2.x = io->pos.x;
			pos2.y = io->pos.y;
			pos2.z = io->pos.z;
		}
	}

	io->_npcdata->pathfind.truetarget = target;
	float dist;
	dist = EEDistance3D(&pos1, &pos2);

	if ((EEDistance3D(&pos1, &ACTIVECAM->pos) < ACTIVECAM->cdepth * DIV2)
	        &&	(EEfabs(pos1.y - pos2.y) < 50.f)
	        && (dist < 520) && (io->_npcdata->behavior & BEHAVIOUR_MOVE_TO)
	        && (!(io->_npcdata->behavior & BEHAVIOUR_SNEAK))
	        && (!(io->_npcdata->behavior & BEHAVIOUR_FLEE))
	   )
	{
		// COLLISION Management START *********************************************************************
		io->physics.startpos.x = pos1.x;
		io->physics.startpos.y = pos1.y;
		io->physics.startpos.z = pos1.z;
		io->physics.targetpos.x = pos2.x;
		io->physics.targetpos.y = pos2.y;
		io->physics.targetpos.z = pos2.z;
		IO_PHYSICS phys;
		memcpy(&phys, &io->physics, sizeof(IO_PHYSICS));
		GetIOCyl(io, &phys.cyl);

		// Now we try the physical move for real
		if	(Vector_Compare(&io->physics.startpos, &io->physics.targetpos) // optim
		        || ((ARX_COLLISION_Move_Cylinder(&phys, io, 40, CFLAG_JUST_TEST | CFLAG_NPC | CFLAG_NO_HEIGHT_MOD)) ))
		{

			dist = TRUEEEDistance3D(&phys.cyl.origin, &pos2);

			if (dist < 100)
			{
				io->_npcdata->pathfind.pathwait = 0;
				return FALSE;
			}
		}
	}

suite:
	;
wander:
	;
	io->targetinfo = target; 
	io->_npcdata->pathfind.truetarget = target;

	long from;

	if (MUST_SELECT_Start_Anchor == -1)
	{
		if ((io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND)
		        ||	(io->_npcdata->behavior & BEHAVIOUR_FLEE))
			from = AnchorData_GetNearest(&pos1, &io->physics.cyl);
		else
			from = AnchorData_GetNearest_2(io->angle.b, &pos1, &io->physics.cyl);
	}
	else from = MUST_SELECT_Start_Anchor;

	long to;

	if (io->_npcdata->behavior & BEHAVIOUR_FLEE)
		to = AnchorData_GetNearest_Except(&pos2, &io->physics.cyl, from);
	else if (io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND)
		to = from;
	else to = AnchorData_GetNearest(&pos2, &io->physics.cyl); 

	if ((from != -1) && (to != -1))
	{
		if ((from == to) && !(io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND))
			return TRUE;

		float dist1 = EEDistance3D(&pos1, &ACTIVEBKG->anchors[from].pos);

		if ((io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND) ||
		        (dist1 < 200.f))
		{
			if (!(io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND)
			        && !(io->_npcdata->behavior & BEHAVIOUR_FLEE))
			{
				float dist2 = EEDistance3D(&pos2, &ACTIVEBKG->anchors[to].pos);
				float dist3 = EEDistance3D(&ACTIVEBKG->anchors[from].pos, &ACTIVEBKG->anchors[to].pos);

				if (dist3 < 200.f) return FALSE;

				if ((dist2 > 200.f)) 
					goto failure;
			}

			if (io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND)
			{
				io->_npcdata->pathfind.truetarget = TARGET_NONE;
			}

			io->_npcdata->pathfind.listnb = -1;
			io->_npcdata->pathfind.listpos = 0;
			io->_npcdata->pathfind.pathwait = 1;

			if (io->_npcdata->pathfind.list) free(io->_npcdata->pathfind.list);

			io->_npcdata->pathfind.list = NULL;

			PATHFINDER_REQUEST tpr;
			tpr.from = from;
			tpr.to = to;
			tpr.returnlist = &io->_npcdata->pathfind.list;
			tpr.returnnumber = &io->_npcdata->pathfind.listnb;
			tpr.ioid = io;
			tpr.isvalid = TRUE;

			if (EERIE_PATHFINDER_Add_To_Queue(&tpr))
				return TRUE;
		}
	}

failure:
	;
	io->_npcdata->pathfind.pathwait = 0;

	if (io->_npcdata->pathfind.list) ARX_NPC_ReleasePathFindInfo(io);

	io->_npcdata->pathfind.listnb = -2;

	if (io->_npcdata->pathfind.flags & BEHAVIOUR_NONE) return FALSE;

	SendIOScriptEvent(io, SM_PATHFINDER_FAILURE, "", NULL);
	return FALSE;
}

//***********************************************************************************************
//***********************************************************************************************
void ARX_NPC_SetStat(INTERACTIVE_OBJ * io, char * statname, float value)
{
	if ((!io)
	        ||	(!(io->ioflags & IO_NPC)))
		return;

	if (!stricmp(statname, "ARMOR_CLASS"))
	{
		if (value < 0) value = 0;

		io->_npcdata->armor_class = value;
	}
	else if (!stricmp(statname, "BACKSTAB_SKILL"))
	{
		if (value < 0) value = 0;

		io->_npcdata->backstab_skill = value;
	}
	else if (!stricmp(statname, "BACKSTAB"))
	{
		if (value == 0) io->_npcdata->npcflags &= ~NPCFLAG_BACKSTAB;
		else io->_npcdata->npcflags |= NPCFLAG_BACKSTAB;
	}
	else if (!stricmp(statname, "REACH"))
	{
		if (value < 0) value = 0;

		io->_npcdata->reach = value;
	}
	else if (!stricmp(statname, "CRITICAL"))
	{
		if (value < 0) value = 0;

		io->_npcdata->critical = value;
	}
	else if (!stricmp(statname, "ABSORB"))
	{
		if (value < 0) value = 0;

		io->_npcdata->absorb = value;
	}
	else if (!stricmp(statname, "DAMAGES"))
	{
		if (value < 0) value = 0;

		io->_npcdata->damages = value;
	}
	else if (!stricmp(statname, "TOHIT"))
	{
		if (value < 0) value = 0;

		io->_npcdata->tohit = value;
	}
	else if (!stricmp(statname, "AIMTIME"))
	{
		if (value < 0) value = 0;

		io->_npcdata->aimtime = value;
	}
	else if (!stricmp(statname, "LIFE"))
	{
		if (value < 0) value = 0.0000001f;

		io->_npcdata->maxlife = io->_npcdata->life = value;
	}
	else if (!stricmp(statname, "MANA"))
	{
		if (value < 0) value = 0;

		io->_npcdata->maxmana = io->_npcdata->mana = value;
	}
	else if (!stricmp(statname, "RESISTFIRE"))
	{
		if (value < 0) value = 0;
		else if (value > 100) value = 100;

		io->_npcdata->resist_fire = (unsigned char)value;
	}
	else if (!stricmp(statname, "RESISTPOISON"))
	{
		if (value < 0) value = 0;
		else if (value > 100) value = 100;

		io->_npcdata->resist_poison = (unsigned char)value;
	}
	else if (!stricmp(statname, "RESISTMAGIC"))
	{
		if (value < 0) value = 0;
		else if (value > 100) value = 100;

		io->_npcdata->resist_magic = (unsigned char)value;
	}
}

extern long CUR_COLLISION_MATERIAL;
INTERACTIVE_OBJ * PHYSICS_CURIO = NULL;
//***********************************************************************************************
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/15)
//***********************************************************************************************
void ARX_TEMPORARY_TrySound(float volume)
{
	if (PHYSICS_CURIO)
	{
		if (PHYSICS_CURIO->ioflags & IO_BODY_CHUNK)
			return;

		unsigned long at = ARXTimeUL();

		if (at > PHYSICS_CURIO->soundtime)
		{
			long material;

			PHYSICS_CURIO->soundcount++;

			if (PHYSICS_CURIO->soundcount < 5)
			{

				if ( EEIsUnderWaterFast( &PHYSICS_CURIO->pos ) ) material = MATERIAL_WATER; //ARX: jycorbel (2010-08-20) - rendering issues with bGATI8500: optimize time to render;
				else if (PHYSICS_CURIO->material) material = PHYSICS_CURIO->material;
				else material = MATERIAL_STONE;

				if (volume > 1.f)
					volume = 1.f;

				PHYSICS_CURIO->soundtime = at + (ARX_SOUND_PlayCollision(material, CUR_COLLISION_MATERIAL, volume, 1.f, &PHYSICS_CURIO->pos, PHYSICS_CURIO) >> 4) + 50;
			}
		}
	}
}
//***********************************************************************************************
// Sets a New MoveMode for a NPC
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/15)
//***********************************************************************************************
void ARX_NPC_ChangeMoveMode(INTERACTIVE_OBJ * io, long MOVEMODE)
{
	if ((!io)
	        ||	(!(io->ioflags & IO_NPC)))
		return;

	ANIM_USE * ause0 = &io->animlayer[0];
	ANIM_HANDLE ** alist = io->anims;

	switch (MOVEMODE)
	{
		case RUNMODE:

			if (((ause0->cur_anim == alist[ANIM_WALK]) && (alist[ANIM_WALK]))
			        || ((ause0->cur_anim == alist[ANIM_WALK_SNEAK]) && (alist[ANIM_WALK_SNEAK])))
			{
				AcquireLastAnim(io);
				FinishAnim(io, ause0->cur_anim);
				ANIM_Set(ause0, alist[ANIM_RUN]);
				ause0->altidx_cur = 0;
			}

			break;
		case WALKMODE:

			if (((ause0->cur_anim == alist[ANIM_RUN]) && (alist[ANIM_RUN]))
			        || ((ause0->cur_anim == alist[ANIM_WALK_SNEAK]) && (alist[ANIM_WALK_SNEAK])))
			{
				AcquireLastAnim(io);
				FinishAnim(io, ause0->cur_anim);
				ANIM_Set(ause0, alist[ANIM_WALK]);
			}

			break;
		case NOMOVEMODE:

			if (((ause0->cur_anim == alist[ANIM_WALK]) && (alist[ANIM_WALK]))
			        || ((ause0->cur_anim == alist[ANIM_RUN]) && (alist[ANIM_RUN]))
			        || ((ause0->cur_anim == alist[ANIM_WALK_SNEAK]) && (alist[ANIM_WALK_SNEAK])))
			{
				AcquireLastAnim(io);
				FinishAnim(io, ause0->cur_anim);
				ANIM_Set(ause0, alist[ANIM_WAIT]);
				ause0->altidx_cur = 0;
			}

			break;
		case SNEAKMODE:

			if (((ause0->cur_anim == alist[ANIM_WALK]) && (alist[ANIM_WALK]))
			        || ((ause0->cur_anim == alist[ANIM_RUN]) && (alist[ANIM_RUN])))
			{
				AcquireLastAnim(io);
				FinishAnim(io, ause0->cur_anim);
				ANIM_Set(ause0, alist[ANIM_WALK_SNEAK]);
			}

			break;
	}

	io->_npcdata->movemode = MOVEMODE;
}
//***********************************************************************************************
// Diminishes life of a Poisoned NPC
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/15)
//***********************************************************************************************
void ARX_NPC_ManagePoison(INTERACTIVE_OBJ * io)
{
	float cp = io->_npcdata->poisonned;
	cp *= DIV2 * _framedelay * DIV1000 * DIV2;
	float faster = 10.f - io->_npcdata->poisonned;

	if (faster < 0.f) faster = 0.f;

	if (rnd() * 100.f > io->_npcdata->resist_poison + faster)
	{
		float dmg = cp * DIV3;

		if ((io->_npcdata->life > 0) && (io->_npcdata->life - dmg <= 0.f))
		{
			long xp = io->_npcdata->xpvalue;
			ARX_DAMAGES_DamageNPC(io, dmg, -1, 0, NULL);
			ARX_PLAYER_Modify_XP(xp);
		}
		else io->_npcdata->life -= dmg;

		io->_npcdata->poisonned -= cp * DIV10;
	}
	else io->_npcdata->poisonned -= cp;

	if (io->_npcdata->poisonned < 0.1f) io->_npcdata->poisonned = 0.f;
}
//*************************************************************************************
//*************************************************************************************
 
extern void ARX_PARTICLES_Spawn_Splat(EERIE_3D * pos, float dmgs, D3DCOLOR col, long vert, INTERACTIVE_OBJ * io, long flags);
extern void ARX_PARTICLES_Spawn_Blood3(EERIE_3D * pos, float dmgs, D3DCOLOR col, long vert, INTERACTIVE_OBJ * io, long flags);
extern float MAX_ALLOWED_PER_SECOND;
long REACTIVATION_COUNT = 0;
void ARX_PHYSICS_Apply()
{

	static long CURRENT_DETECT = 0;
	REACTIVATION_COUNT++;

	if (REACTIVATION_COUNT > 12) REACTIVATION_COUNT = 0;

	CURRENT_DETECT++;

	if (CURRENT_DETECT > TREATZONE_CUR) CURRENT_DETECT = 1;

	long needkill = 0;

	for (long i = 1; i < TREATZONE_CUR; i++) // We don't manage Player(0) this way
	{
		if (treatio[i].show != 1) continue;

		if (treatio[i].ioflags & (IO_FIX | IO_JUST_COLLIDE))
			continue;

		INTERACTIVE_OBJ * io = treatio[i].io;

		if (!io) continue;

		if ((io->ioflags & IO_NPC) && (io->_npcdata->poisonned > 0.f)) ARX_NPC_ManagePoison(io);

		if ((io->ioflags & IO_ITEM)
		        &&	(io->show != SHOW_FLAG_DESTROYED)
		        &&	((io->GameFlags & GFLAG_GOREEXPLODE)
		             &&	(ARXTime - io->lastanimtime > 300))
		        &&	((io->obj)
		             &&	(io->obj->nbvertex))
		   )
		{
			long idx;
			long cnt = (io->obj->nbvertex << 12) + 1;

			if (cnt < 2) cnt = 2;

			if (cnt > 10) cnt = 10;

			for (long nn = 0; nn < cnt; nn++)
			{
				F2L(rnd()*(float)io->obj->nbvertex, &idx);

				if (idx >= io->obj->nbvertex) idx = io->obj->nbvertex - 1;

				EERIE_3D vector;
				vector.x = io->obj->vertexlist3[idx].v.x - io->obj->vertexlist3[0].v.x + (rnd() - rnd()) * 3.f;
				vector.y = io->obj->vertexlist3[idx].v.y - io->obj->vertexlist3[0].v.y + (rnd() - rnd()) * 3.f;
				vector.z = io->obj->vertexlist3[idx].v.z - io->obj->vertexlist3[0].v.z + (rnd() - rnd()) * 3.f;
				TRUEVector_Normalize(&vector);

				ARX_PARTICLES_Spawn_Splat(&io->obj->vertexlist3[idx].v, 20, 0xFFFF0000, idx, io, 1);
				ARX_PARTICLES_Spawn_Blood(&io->obj->vertexlist3[idx].v, &vector, 20, GetInterNum(io));
				needkill = 1;
			}

			ARX_INTERACTIVE_DestroyIO(io);
			continue;
		}

		EERIEPOLY * ep = EECheckInPoly(&io->pos);

		if ((ep)
		        &&	(ep->type & POLY_LAVA)
		        &&	(EEfabs(ep->center.y - io->pos.y) < 40))
		{
			ARX_PARTICLES_Spawn_Lava_Burn(&io->pos, 0.6f, io);

			if (io->ioflags & IO_NPC)
			{
#define LAVA_DAMAGE 10.f
				ARX_DAMAGES_DamageNPC(io, LAVA_DAMAGE * FrameDiff * DIV100, -1, 0, NULL);
			}
		}

		CheckUnderWaterIO(io);

		if (io->obj->pbox)
		{
			io->GameFlags &= ~GFLAG_NOCOMPUTATION;

			if ((io->obj->pbox->active == 1))
			{
				PHYSICS_CURIO = io;
				long flags = 0;

				if (io->GameFlags & GFLAG_NO_PHYS_IO_COL) flags = 1;

				if (EERIE_PHYSICS_BOX_ApplyModel(io->obj, (float)FrameDiff, io->rubber, flags, treatio[i].num))
				{
					if (io->damagedata >= 0)
					{
						damages[io->damagedata].active = 1;
						ARX_DAMAGES_UpdateDamage(io->damagedata, (float)ARXTime);
						damages[io->damagedata].exist = 0;
						io->damagedata = -1;
					}
				}

				if (io->soundcount > 12)
				{
					io->soundtime = 0;
						io->soundcount = 0;

						for (long k = 0; k < io->obj->pbox->nb_physvert; k++)
						{
							PHYSVERT * pv = &io->obj->pbox->vert[k];
							pv->velocity.x = 0.f;
							pv->velocity.y = 0.f;
							pv->velocity.z = 0.f;
						}

						io->obj->pbox->active = 2;
						io->obj->pbox->stopcount = 0;
				}

				io->room_flags |= 1;
				io->pos.x = io->obj->pbox->vert[0].pos.x;
				io->pos.y = io->obj->pbox->vert[0].pos.y; 
				io->pos.z = io->obj->pbox->vert[0].pos.z; 

				continue;
			}
		}

		if (IsDeadNPC(io)) continue;

		if (io->ioflags & IO_PHYSICAL_OFF)
		{
			if (io->ioflags & IO_NPC)
			{
				ANIM_USE * ause0 = &io->animlayer[0];

				if ((ause0->cur_anim == 0) || (ause0->flags & EA_ANIMEND))
				{
					ANIM_Set(ause0, io->anims[ANIM_WAIT]);
					ause0->altidx_cur = 0;
				}

				GetTargetPos(io);

				if ((!ARXPausedTimer) && (!(ause0->flags & EA_FORCEPLAY)))
				{
					if (io->_npcdata->behavior & BEHAVIOUR_STARE_AT)
						StareAtTarget(io);
					else	FaceTarget2(io);
				}
			}

			continue;
		}


		if ((io->ioflags & IO_NPC)
		        &&	(!EDITMODE))
		{
			if ((io->_npcdata->climb_count != 0.f) && (FrameDiff > 0))
			{
				io->_npcdata->climb_count -= MAX_ALLOWED_PER_SECOND * (float)FrameDiff * DIV1000;

				if (io->_npcdata->climb_count < 0)
					io->_npcdata->climb_count = 0.f;
			}

			if (io->_npcdata->pathfind.pathwait) // Waiting For Pathfinder Answer
			{
				if ((ValidIONum(LastSelectedIONum)) &&
				        (io == inter.iobj[LastSelectedIONum])) ShowIOPath(io);

				if (io->_npcdata->pathfind.listnb == 0) // Not Found
				{
					SendIOScriptEvent(io, SM_PATHFINDER_FAILURE, "", NULL);
					io->_npcdata->pathfind.pathwait = 0;

					if (io->_npcdata->pathfind.list)
						ARX_NPC_ReleasePathFindInfo(io);

					io->_npcdata->pathfind.listnb = -2;
				}
				else if (io->_npcdata->pathfind.listnb > 0) // Found
				{
					SendIOScriptEvent(io, SM_PATHFINDER_SUCCESS, "", NULL);
					io->_npcdata->pathfind.pathwait = 0;
					io->_npcdata->pathfind.listpos += (unsigned short)ARX_NPC_GetNextAttainableNodeIncrement(io);

					if (io->_npcdata->pathfind.listpos >= io->_npcdata->pathfind.listnb)
						io->_npcdata->pathfind.listpos = 0;
				}
			}

			ManageNPCMovement(io);
			CheckNPC(io);

			if (CURRENT_DETECT == i) CheckNPCEx(io);
		}
	}
}
//*************************************************************************************
//*************************************************************************************
void FaceTarget2(INTERACTIVE_OBJ * io)
{
	EERIE_3D tv;

	if (!io->show) return;

	if (io->ioflags & IO_NPC)
	{
		if (io->_npcdata->life <= 0.f) return;

		if (io->_npcdata->behavior & BEHAVIOUR_NONE) return;

		if ((io->_npcdata->pathfind.listnb <= 0)
		        && (io->_npcdata->behavior & BEHAVIOUR_FLEE)) return;

	}

	GetTargetPos(io);
	tv.x = io->pos.x;
	tv.y = io->pos.y;
	tv.z = io->pos.z;

	if (Distance2D(tv.x, tv.z, io->target.x, io->target.z) <= 5.f) return;

	float cangle, tangle;
	tangle = MAKEANGLE(180.f + RAD2DEG(GetAngle(io->target.x, io->target.z, tv.x, tv.z)));
	cangle = io->angle.b;

	float tt = (cangle - tangle);

	if (tt == 0) return;

	float rot = 0.33f * _framedelay; 

	if (EEfabs(tt) < rot) rot = (float)EEfabs(tt);

	rot = -rot;

	if ((tt > 0.f) && (tt < 180.f)) rot = -rot;
	else if ((tt < -180.f)) rot = -rot;

	if (rot != 0)
	{
		EERIE_3D temp;
		temp.x = io->move.x;
		temp.y = io->move.y;
		temp.z = io->move.z;
		Vector_RotateY(&io->move, &temp, rot);
		temp.x = io->lastmove.x;
		temp.y = io->lastmove.y;
		temp.z = io->lastmove.z;
		Vector_RotateY(&io->lastmove, &temp, rot);
	}

	// Needed angle to turn toward target
	io->angle.b = MAKEANGLE(io->angle.b - rot); // -tt
}

//***********************************************************************************************
//***********************************************************************************************
void StareAtTarget(INTERACTIVE_OBJ * io)
{
	if (io->_npcdata->ex_rotate == NULL)
	{
		ARX_NPC_CreateExRotateData(io);
	}

	EERIE_3D tv;

	if (!io->show) return;

	if (io->ioflags & IO_NPC)
	{
		if (io->_npcdata->life <= 0.f) return;

		if ((io->_npcdata->pathfind.listnb <= 0) && (io->_npcdata->behavior & BEHAVIOUR_FLEE)) return;
	}

	if (io->_npcdata->behavior & BEHAVIOUR_NONE) return;

	GetTargetPos(io);
	tv.x = io->pos.x;
	tv.y = io->pos.y;
	tv.z = io->pos.z;

	if (TRUEEEDistance3D(&tv, &io->target) <= 20.f) return; // To fix "stupid" rotation near target

	if (((io->target.x - tv.x) == 0) && ((io->target.z - tv.z) == 0)) return;

	float rot = 0.27f * _framedelay;
	float alpha = MAKEANGLE(io->angle.b);
	float beta = -io->head_rot; 
	float pouet = MAKEANGLE(180.f + RAD2DEG(GetAngle(io->target.x, io->target.z, tv.x, tv.z)));
	float A = MAKEANGLE((MAKEANGLE(alpha + beta) - pouet));
	float B = MAKEANGLE(alpha - pouet);

	if (A == 0.f) rot = 0.f;

	if ((B < 180) && (B > 90))
	{
		rot = rot;

		if (rot > A) rot = A;
	}
	else if ((B > 180) && (B < 270)) 
	{
		if (rot > 360 - A) rot = -(360 - A);
		else rot = -rot;
	}
	else if (A < 180)
	{
		rot = rot;

		if (rot > A) rot = A;
	}
	else
	{
		if (rot > 360 - A) rot = -(360 - A);
		else rot = -rot;
	}

	// Needed angle to turn toward target
	rot *= DIV2;
	float HEAD_ANGLE_THRESHOLD;

	if ((io) && (io->ioflags & IO_NPC))
		HEAD_ANGLE_THRESHOLD = 45.f * io->_npcdata->stare_factor;
	else HEAD_ANGLE_THRESHOLD = 45.f;

	io->head_rot += rot;

	if (io->head_rot > 120.f) io->head_rot = 120.f;

	if (io->head_rot < -120.f) io->head_rot = -120.f;

	io->_npcdata->ex_rotate->group_rotate[0].b = io->head_rot * 1.5f;

	if (io->_npcdata->ex_rotate->group_rotate[0].b > HEAD_ANGLE_THRESHOLD) io->_npcdata->ex_rotate->group_rotate[0].b = HEAD_ANGLE_THRESHOLD;

	if (io->_npcdata->ex_rotate->group_rotate[0].b < -HEAD_ANGLE_THRESHOLD) io->_npcdata->ex_rotate->group_rotate[0].b = -HEAD_ANGLE_THRESHOLD;

	io->_npcdata->ex_rotate->group_rotate[1].b = io->head_rot * DIV2;

	if (io->_npcdata->ex_rotate->group_rotate[1].b > HEAD_ANGLE_THRESHOLD) io->_npcdata->ex_rotate->group_rotate[1].b = HEAD_ANGLE_THRESHOLD;

	if (io->_npcdata->ex_rotate->group_rotate[1].b < -HEAD_ANGLE_THRESHOLD) io->_npcdata->ex_rotate->group_rotate[1].b = -HEAD_ANGLE_THRESHOLD;

	//MAKEANGLE(io->angle.b-rot); // -tt
	return;
}


//***********************************************************************************************
//***********************************************************************************************
float GetTRUETargetDist(INTERACTIVE_OBJ * io)
{
	long t;

	if (io->ioflags & IO_NPC)
		t = io->_npcdata->pathfind.truetarget;
	else t = io->targetinfo;

	if ((t >= 0) && (t < inter.nbmax) && (inter.iobj[t] != NULL))
	{
		if (io->_npcdata->behavior & BEHAVIOUR_GO_HOME)
			return TRUEEEDistance3D(&io->pos, &io->initpos);

		return TRUEEEDistance3D(&io->pos, &inter.iobj[t]->pos);
	}

	return 99999999.f;
}

extern TextureContainer * sphere_particle;
extern INTERACTIVE_OBJ * EVENT_SENDER;

//***********************************************************************************************
// Checks If a NPC is dead
//***********************************************************************************************
BOOL IsDeadNPC(INTERACTIVE_OBJ * io)
{
	if (!io) return FALSE;

	if (!(io->ioflags & IO_NPC)) return FALSE;

	if (io->_npcdata->life <= 0) return TRUE;

	if ((io->mainevent) && !strcmp(io->mainevent, "DEAD")) return TRUE;

	return FALSE;
}
//***********************************************************************************************
//***********************************************************************************************
long IsInGroup(EERIE_3DOBJ * obj, long vert, long tw)
{
	if (obj == NULL) return -1;

	if (tw < 0) return -1;

	if (tw > obj->nbgroups) return -1;

	if (vert < 0) return -1;

	for (long i = 0; i < obj->grouplist[tw].nb_index; i++)
	{
		if (obj->grouplist[tw].indexes[i] == vert)
			return i;
	}

	return -1;
}

//***********************************************************************************************
//***********************************************************************************************
long IsNearSelection(EERIE_3DOBJ * obj, long vert, long tw)
{
	if (obj == NULL) return -1;

	if (tw < 0) return -1;

	if (vert < 0) return -1;

	for (long i = 0; i < obj->selections[tw].nb_selected; i++)
	{
		float d = TRUEEEDistance3D(&obj->vertexlist[obj->selections[tw].selected[i]].v,
		                           &obj->vertexlist[vert].v);

		if (d < 8.f)
			return i;
	}

	return -1;
}
//***********************************************************************************************
// Spawns a body part from NPC
//***********************************************************************************************
void ARX_NPC_SpawnMember(INTERACTIVE_OBJ * ioo, long num)
{
	if (!ioo) return;

	EERIE_3DOBJ * from = ioo->obj;

	if ((!from)
	        ||	(num < 0)
	        ||	(num > from->nbselections))
		return;

	EERIE_3DOBJ * nouvo;
	nouvo = (EERIE_3DOBJ *)malloc(sizeof(EERIE_3DOBJ)); 

	if (!nouvo)
		return;

	memset(nouvo, 0, sizeof(EERIE_3DOBJ));
	nouvo->nbvertex = from->selections[num].nb_selected;

	long gore = -1;

	for (long k = 0; k < from->nbmaps; k++)
	{
		if (from->texturecontainer
		        && from->texturecontainer[k]
		        && (IsIn(from->texturecontainer[k]->m_strName, "GORE")))
		{
			gore = k;
			break;
		}
	}

	for (int k = 0; k < from->nbfaces; k++)
	{
		if (from->facelist[k].texid == gore)
		{
			if	((IsNearSelection(from, from->facelist[k].vid[0], num) >= 0)
			        ||	(IsNearSelection(from, from->facelist[k].vid[1], num) >= 0)
			        ||	(IsNearSelection(from, from->facelist[k].vid[2], num) >= 0))
				nouvo->nbvertex += 3;
		}
	}

	nouvo->vertexlist = (EERIE_VERTEX *)malloc(sizeof(EERIE_VERTEX) * nouvo->nbvertex);

	if (!nouvo->vertexlist)
	{
		free(nouvo);
		return;
	}

	nouvo->vertexlist3 = (EERIE_VERTEX *)malloc(sizeof(EERIE_VERTEX) * nouvo->nbvertex);

	if (!nouvo->vertexlist3)
	{
		free(nouvo->vertexlist);
		free(nouvo);
		return;
	}

	long	inpos	= 0;
	long *	equival = (long *)malloc(sizeof(long) * from->nbvertex);

	if (!equival)
	{
		free(nouvo->vertexlist3);
		free(nouvo->vertexlist);
		free(nouvo);
	}

	for (int k = 0 ; k < from->nbvertex ; k++)
		equival[k] = -1;

	ARX_CHECK(0 < from->selections[num].nb_selected);

	for (int k = 0 ; k < from->selections[num].nb_selected ; k++)
	{
		inpos						=	from->selections[num].selected[k];
		equival[from->selections[num].selected[k]]	=	k;
		memcpy(&nouvo->vertexlist[k], &from->vertexlist[from->selections[num].selected[k]], sizeof(EERIE_VERTEX));
		nouvo->vertexlist[k].v.x	=	nouvo->vertexlist[k].vert.sx	=	from->vertexlist3[from->selections[num].selected[k]].v.x - ioo->pos.x;
		nouvo->vertexlist[k].v.y	=	nouvo->vertexlist[k].vert.sy	=	from->vertexlist3[from->selections[num].selected[k]].v.y - ioo->pos.y;
		nouvo->vertexlist[k].v.z	=	nouvo->vertexlist[k].vert.sz	=	from->vertexlist3[from->selections[num].selected[k]].v.z - ioo->pos.z;

		nouvo->vertexlist[k].vert.color	=	from->vertexlist[k].vert.color;
		nouvo->vertexlist[k].vert.tu	=	from->vertexlist[k].vert.tu;
		nouvo->vertexlist[k].vert.tv	=	from->vertexlist[k].vert.tv;

		memcpy(&nouvo->vertexlist3[k], &nouvo->vertexlist[k], sizeof(EERIE_VERTEX));
	}

	long count = from->selections[num].nb_selected;

	for (int k = 0; k < from->nbfaces; k++)
	{
		if (from->facelist[k].texid == gore)
		{
			if ((IsNearSelection(from, from->facelist[k].vid[0], num) >= 0)
			        ||	(IsNearSelection(from, from->facelist[k].vid[1], num) >= 0)
			        ||	(IsNearSelection(from, from->facelist[k].vid[2], num) >= 0))
			{
				for (long j = 0; j < 3; j++)
				{
					equival[from->facelist[k].vid[j]] = count;

					if (count < nouvo->nbvertex)
					{
						memcpy(&nouvo->vertexlist[count], &from->vertexlist[from->facelist[k].vid[j]], sizeof(EERIE_VERTEX));
						nouvo->vertexlist[count].v.x = nouvo->vertexlist[count].vert.sx = from->vertexlist3[from->facelist[k].vid[j]].v.x - ioo->pos.x;
						nouvo->vertexlist[count].v.y = nouvo->vertexlist[count].vert.sy = from->vertexlist3[from->facelist[k].vid[j]].v.y - ioo->pos.y;
						nouvo->vertexlist[count].v.z = nouvo->vertexlist[count].vert.sz = from->vertexlist3[from->facelist[k].vid[j]].v.z - ioo->pos.z;
						memcpy(&nouvo->vertexlist3[count], &nouvo->vertexlist[count], sizeof(EERIE_VERTEX));
					}
					else
						equival[from->facelist[k].vid[j]] = -1;

					count++;
				}
			}
		}
	}

	float min = nouvo->vertexlist[0].vert.sy;
	long nummm = 0;

	for (int k = 1; k < nouvo->nbvertex; k++)
	{
		if (nouvo->vertexlist[k].vert.sy > min)
		{
			min = nouvo->vertexlist[k].vert.sy;
			nummm = k;
		}
	}

	nouvo->origin = nummm;

	nouvo->point0.x = nouvo->vertexlist[nouvo->origin].v.x;
	nouvo->point0.y = nouvo->vertexlist[nouvo->origin].v.y;
	nouvo->point0.z = nouvo->vertexlist[nouvo->origin].v.z;

	for (int k = 0; k < nouvo->nbvertex; k++)
	{
		nouvo->vertexlist[k].vert.sx = nouvo->vertexlist[k].v.x -= nouvo->point0.x;
		nouvo->vertexlist[k].vert.sy = nouvo->vertexlist[k].v.y -= nouvo->point0.y;
		nouvo->vertexlist[k].vert.sz = nouvo->vertexlist[k].v.z -= nouvo->point0.z;
		nouvo->vertexlist[k].vert.color = 0xFFFFFFFF;
	}

	nouvo->point0.x = 0;
	nouvo->point0.y = 0;
	nouvo->point0.z = 0;

	nouvo->pbox = NULL;
	nouvo->pdata = NULL;

	nouvo->cdata = NULL;
	nouvo->sdata = NULL;

	nouvo->ndata = NULL;

	nouvo->nbfaces = 0;

	for (int k = 0; k < from->nbfaces; k++)
	{
		if ((equival[from->facelist[k].vid[0]] != -1)
		        &&	(equival[from->facelist[k].vid[1]] != -1)
		        &&	(equival[from->facelist[k].vid[2]] != -1))
			nouvo->nbfaces++;
	}

	if (nouvo->nbfaces)
	{
		nouvo->facelist = (EERIE_FACE *)malloc(sizeof(EERIE_FACE) * nouvo->nbfaces);
		long pos = 0;

		for (long k = 0; k < from->nbfaces; k++)
		{
			if ((equival[from->facelist[k].vid[0]] != -1)
			        &&	(equival[from->facelist[k].vid[1]] != -1)
			        &&	(equival[from->facelist[k].vid[2]] != -1))
			{
				memcpy(&nouvo->facelist[pos], &from->facelist[k], sizeof(EERIE_FACE));
				nouvo->facelist[pos].vid[0] = (unsigned short)equival[from->facelist[k].vid[0]];
				nouvo->facelist[pos].vid[1] = (unsigned short)equival[from->facelist[k].vid[1]];
				nouvo->facelist[pos].vid[2] = (unsigned short)equival[from->facelist[k].vid[2]];
				pos++;
			}
		}

		long gore = -1;

		for (int k = 0; k < from->nbmaps; k++)
		{
			if (from->texturecontainer
			        && from->texturecontainer[k]
			        && (IsIn(from->texturecontainer[k]->m_strName, "GORE")))
			{
				gore = k;
				break;
			}
		}

		for (int k = 0; k < nouvo->nbfaces; k++)
		{
			nouvo->facelist[k].facetype &= ~POLY_HIDE;

			if (nouvo->facelist[k].texid == gore)
				nouvo->facelist[k].facetype |= POLY_DOUBLESIDED;
		}


	}

	free(equival);
	nouvo->nbmaps = from->nbmaps;

	if (from->nbmaps)
	{

		nouvo->texturecontainer = (TextureContainer **)malloc(sizeof(TextureContainer *) * nouvo->nbmaps); 

		if (!nouvo->texturecontainer)
		{
			free(equival);
			free(nouvo->vertexlist3);
			free(nouvo->vertexlist);
			free(nouvo);
		}

		memcpy(nouvo->texturecontainer, from->texturecontainer, sizeof(TextureContainer *)*nouvo->nbmaps);
	}

	nouvo->linked			=	NULL;
	nouvo->nblinked			=	0;
	nouvo->originaltextures	=	NULL;

	INTERACTIVE_OBJ * io	=	CreateFreeInter();

	if (io == NULL)
	{
		ReleaseEERIE3DObj(nouvo);
		return;
	}

	io->_itemdata	=	(IO_ITEMDATA *)malloc(sizeof(IO_ITEMDATA));

	memset(io->_itemdata, 0, sizeof(IO_ITEMDATA));

	io->ioflags		=	IO_ITEM;
	io->script.size	=	0;
	io->script.data	=	NULL;
	io->GameFlags	|=	GFLAG_NO_PHYS_IO_COL;
	
	strcpy(io->filename, "NoName");

	EERIE_COLLISION_Cylinder_Create(io);
	EERIE_PHYSICS_BOX_Create(nouvo);

	if (nouvo->pbox == NULL)
	{
		ReleaseEERIE3DObj(nouvo);
		return;
	}

	io->infracolor.r	=	0.f;
	io->infracolor.g	=	0.f;
	io->infracolor.b	=	0.8f;
	io->collision		=	1;
	io->inv				=	NULL;
	io->scriptload		=	1;
	io->obj				=	nouvo;
	io->lastpos.x		=	io->initpos.x	=	io->pos.x	=	ioo->obj->vertexlist3[inpos].v.x;
	io->lastpos.y		=	io->initpos.y	=	io->pos.y	=	ioo->obj->vertexlist3[inpos].v.y;
	io->lastpos.z		=	io->initpos.z	=	io->pos.z	=	ioo->obj->vertexlist3[inpos].v.z;
	io->angle.a			=	ioo->angle.a;
	io->angle.b			=	ioo->angle.b;
	io->angle.g			=	ioo->angle.g;

	io->GameFlags		=	ioo->GameFlags;
	memcpy(&io->halo, &ioo->halo, sizeof(IO_HALO));
	ioo->halo.dynlight	=	-1;
	io->ioflags			|=	IO_MOVABLE;

	io->angle.a					=	rnd() * 40.f + 340.f;
	io->angle.b					=	rnd() * 360.f;
	io->angle.g					=	0;
	io->obj->pbox->active		=	1;
	io->obj->pbox->stopcount	=	0;

	EERIE_3D pos, vector;

	io->velocity.x				=	0.f;
	io->velocity.y				=	0.f;
	io->velocity.z				=	0.f;
	io->stopped					=	1;

	vector.x					=	-(float)EEsin(DEG2RAD(io->angle.b));
	vector.y					=	EEsin(DEG2RAD(io->angle.a)) * 2.f; 
	vector.z					=	(float)EEcos(DEG2RAD(io->angle.b));
	Vector_Normalize(&vector);
	pos.x						=	io->pos.x;
	pos.y						=	io->pos.y;
	pos.z						=	io->pos.z;
	io->rubber					=	0.6f;


	long long_no_collide = GetInterNum(ioo);
	ARX_CHECK_SHORT(long_no_collide);
	io->no_collide = ARX_CLEAN_WARN_CAST_SHORT(long_no_collide);



	io->GameFlags |= GFLAG_GOREEXPLODE;
	io->lastanimtime = ARXTimeUL();//treat warning C4244 conversion from 'float' to 'unsigned long'
	io->soundtime = 0;
	io->soundcount = 0;
	EERIE_PHYSICS_BOX_Launch(io->obj, &pos, &vector, 3, &io->angle);
	return;

}
extern long GORE_MODE;

#define	FLAG_CUT_HEAD	(1)
#define	FLAG_CUT_TORSO	(1<<1)
#define	FLAG_CUT_LARM	(1<<2)
#define	FLAG_CUT_RARM	(1<<3)
#define	FLAG_CUT_LLEG	(1<<4)
#define	FLAG_CUT_RLEG	(1<<5)

short GetCutFlag(char * str)
{
	if (!stricmp(str, "CUT_HEAD"))
		return FLAG_CUT_HEAD;

	if (!stricmp(str, "CUT_TORSO"))
		return FLAG_CUT_TORSO;

	if (!stricmp(str, "CUT_LARM"))
		return FLAG_CUT_LARM;

	if (!stricmp(str, "CUT_RARM"))
		return FLAG_CUT_HEAD;

	if (!stricmp(str, "CUT_LLEG"))
		return FLAG_CUT_LLEG;

	if (!stricmp(str, "CUT_RLEG"))
		return FLAG_CUT_RLEG;

	return 0;
}

long GetCutSelection(INTERACTIVE_OBJ * io, short flag)
{
	if ((!io) || (!(io->ioflags & IO_NPC)) || flag == 0)
		return -1;

	char tx[64];
	tx[0] = 0;

	if (flag == FLAG_CUT_HEAD)
		strcpy(tx, "CUT_HEAD");
	else if (flag == FLAG_CUT_TORSO)
		strcpy(tx, "CUT_TORSO");
	else if (flag == FLAG_CUT_LARM)
		strcpy(tx, "CUT_LARM");

	if (flag == FLAG_CUT_RARM)
		strcpy(tx, "CUT_RARM");

	if (flag == FLAG_CUT_LLEG)
		strcpy(tx, "CUT_LLEG");

	if (flag == FLAG_CUT_RLEG)
		strcpy(tx, "CUT_RLEG");

	if (tx[0])
	{
		for (long i = 0; i < io->obj->nbselections; i++)
		{
			if ((io->obj->selections[i].nb_selected > 0)
			        &&	(!stricmp(io->obj->selections[i].name, tx)))
				return i;
		}
	}

	return -1;
}

void ReComputeCutFlags(INTERACTIVE_OBJ * io)
{
	if ((!io) || (!(io->ioflags & IO_NPC)))
		return;

	if (io->_npcdata->cuts & FLAG_CUT_TORSO)
	{
		io->_npcdata->cuts &= ~FLAG_CUT_HEAD;
		io->_npcdata->cuts &= ~FLAG_CUT_LARM;
		io->_npcdata->cuts &= ~FLAG_CUT_RARM;
	}
}
bool IsAlreadyCut(INTERACTIVE_OBJ * io, short fl)
{
	if (io->_npcdata->cuts & fl)
		return true;

	if (io->_npcdata->cuts & FLAG_CUT_TORSO)
	{
		if (fl == FLAG_CUT_HEAD)
			return true;

		if (fl == FLAG_CUT_LARM)
			return true;

		if (fl == FLAG_CUT_RARM)
			return true;
	}

	return false;
}
long ARX_NPC_ApplyCuts(INTERACTIVE_OBJ * io)
{
	if ((!io) || (!(io->ioflags & IO_NPC)))
		return 0 ;

	if (!GORE_MODE)
		return 0;

	if (io->_npcdata->cuts == 0)
		return 0;	// No cuts

	ReComputeCutFlags(io);
	long goretex = -1;

	for (long i = 0; i < io->obj->nbmaps; i++)
	{
		if (io->obj->texturecontainer
		        &&	io->obj->texturecontainer[i]
		        &&	(IsIn(io->obj->texturecontainer[i]->m_strName, "GORE")))
		{
			goretex = i;
			break;
		}
	}

	long hid = 0;

	for (long nn = 0; nn < io->obj->nbfaces; nn++)
	{
		io->obj->facelist[nn].facetype &= ~POLY_HIDE;
	}

	for (long jj = 0; jj < 6; jj++)
	{
		short flg = 1 << jj;
		long numsel = GetCutSelection(io, flg);

		if ((io->_npcdata->cuts & flg) && (numsel >= 0))
		{
			for (long ll = 0; ll < io->obj->nbfaces; ll++)
			{
				if	((IsInSelection(io->obj, io->obj->facelist[ll].vid[0], numsel) != -1)
				        ||	(IsInSelection(io->obj, io->obj->facelist[ll].vid[1], numsel) != -1)
				        ||	(IsInSelection(io->obj, io->obj->facelist[ll].vid[2], numsel) != -1)
				   )
				{
					if (!(io->obj->facelist[ll].facetype & POLY_HIDE))
					{
						if (io->obj->facelist[ll].texid != goretex)
							hid = 1;
					}

					io->obj->facelist[ll].facetype |= POLY_HIDE;
				}
			}

			io->_npcdata->cut = 1;
		}
	}

	return hid;
}
//***********************************************************************************************
// Attempt to cut something on NPC
//***********************************************************************************************
void ARX_NPC_TryToCutSomething(INTERACTIVE_OBJ * target, EERIE_3D * pos)
{
	//return;
	if (!target) return;

	if (!(target->ioflags & IO_NPC)) return;

	if	(!GORE_MODE)
		return; 

	if	(target->GameFlags & GFLAG_NOGORE)
		return;

	float mindist = 9999.f;
	long numsel = -1;
	long goretex = -1;

	for (long i = 0; i < target->obj->nbmaps; i++)
	{
		if (target->obj->texturecontainer
		        &&	target->obj->texturecontainer[i]
		        &&	(IsIn(target->obj->texturecontainer[i]->m_strName, "GORE")))
		{
			goretex = i;
			break;
		}
	}

	for (int i = 0; i < target->obj->nbselections; i++)
	{
		if ((target->obj->selections[i].nb_selected > 0)
		        &&	(IsIn(target->obj->selections[i].name, "CUT_")))
		{
			short fll = GetCutFlag(target->obj->selections[i].name);

			if (IsAlreadyCut(target, fll))
				continue;

			long out = 0;

			for (long ll = 0; ll < target->obj->nbfaces; ll++)
			{
				if (target->obj->facelist[ll].texid != goretex)
				{
					if	((IsInSelection(target->obj, target->obj->facelist[ll].vid[0], i) != -1)
					        ||	(IsInSelection(target->obj, target->obj->facelist[ll].vid[1], i) != -1)
					        ||	(IsInSelection(target->obj, target->obj->facelist[ll].vid[2], i) != -1)
					   )
					{
						if (target->obj->facelist[ll].facetype & POLY_HIDE)
						{
							out++;
						}
					}
				}
			}

			if (out < 3)
			{
				float dist = EEDistance3D(pos, &target->obj->vertexlist3[target->obj->selections[i].selected[0]].v);

				if (dist < mindist)
				{
					mindist = dist;
					numsel = i;
				}
			}
		}
	}

	if (numsel == -1) return; // Nothing to cut...

	long hid = 0;

	if (mindist < 60) // can only cut a close part...
	{
		short fl = GetCutFlag(target->obj->selections[numsel].name);

		if ((fl)
		        &&	(!(target->_npcdata->cuts & fl)))
		{
			target->_npcdata->cuts |= fl;
			hid = ARX_NPC_ApplyCuts(target);
		}
	}

	if (hid)
	{
		ARX_SOUND_PlayCinematic("Flesh_Critical.wav");
		ARX_NPC_SpawnMember(target, numsel);
	}
}


extern float STRIKE_AIMTIME;
//***********************************************************************************************
// IsPlayerStriking()
// Checks if Player is currently striking.
//***********************************************************************************************
BOOL IsPlayerStriking()
{
	INTERACTIVE_OBJ * io = inter.iobj[0];

	if (!io) return FALSE;

	ANIM_USE * useanim = &io->animlayer[1];
	long weapontype = ARX_EQUIPMENT_GetPlayerWeaponType();
	long j;

	switch (weapontype)
	{
		case WEAPON_BARE:

			for (j = 0; j < 4; j++)
			{
				if ((STRIKE_AIMTIME > 300)
				        && (useanim->cur_anim == io->anims[ANIM_BARE_STRIKE_LEFT_CYCLE+j*3])) return TRUE;

				if (useanim->cur_anim == io->anims[ANIM_BARE_STRIKE_LEFT+j*3]) return TRUE;
			}

			break;
		case WEAPON_DAGGER:

			for (j = 0; j < 4; j++)
			{
				if ((STRIKE_AIMTIME > 300)
				        && (useanim->cur_anim == io->anims[ANIM_DAGGER_STRIKE_LEFT_CYCLE+j*3])) return TRUE;

				if (useanim->cur_anim == io->anims[ANIM_DAGGER_STRIKE_LEFT+j*3]) return TRUE;
			}

			break;
		case WEAPON_1H:

			for (j = 0; j < 4; j++)
			{
				if ((STRIKE_AIMTIME > 300)
				        && (useanim->cur_anim == io->anims[ANIM_1H_STRIKE_LEFT_CYCLE+j*3])) return TRUE;

				if (useanim->cur_anim == io->anims[ANIM_1H_STRIKE_LEFT+j*3]) return TRUE;
			}

			break;
		case WEAPON_2H:

			for (j = 0; j < 4; j++)
			{
				if ((STRIKE_AIMTIME > 300)
				        && (useanim->cur_anim == io->anims[ANIM_2H_STRIKE_LEFT_CYCLE+j*3])) return TRUE;

				if (useanim->cur_anim == io->anims[ANIM_2H_STRIKE_LEFT+j*3]) return TRUE;
			}

			break;
	}

	return FALSE;
}
//***********************************************************************************************
//***********************************************************************************************
void ARX_NPC_Manage_NON_Fight(INTERACTIVE_OBJ * io)
{
	ANIM_USE * ause1 = &io->animlayer[1];

	if ((ause1->flags & EA_ANIMEND) && (ause1->cur_anim != NULL))
	{
		if (!(ause1->flags & EA_FORCEPLAY))
		{
			AcquireLastAnim(io);
			FinishAnim(io, ause1->cur_anim);
			ause1->cur_anim = NULL;
		}
	}
}
//***********************************************************************************************
//***********************************************************************************************
void Strike_StartTickCount(INTERACTIVE_OBJ * io)
{
	io->_npcdata->strike_time = 0;
}
//***********************************************************************************************
//***********************************************************************************************
// NPC IS in fight mode and close to target...
void ARX_NPC_Manage_Fight(INTERACTIVE_OBJ * io)
{
	if (!(io->ioflags & IO_NPC)) return;

	INTERACTIVE_OBJ * ioo = (INTERACTIVE_OBJ *)io->_npcdata->weapon;

	if (ioo)
		io->_npcdata->weapontype = ioo->type_flags;
	else io->_npcdata->weapontype = 0;

	if ((io->_npcdata->weapontype != 0) && (io->_npcdata->weaponinhand != 1))
		return;

	ANIM_USE * ause = &io->animlayer[1];
	{
		// BARE HANDS fight !!! *******************************
		if (io->_npcdata->weapontype == 0)
		{
			if (((ause->cur_anim != io->anims[ANIM_BARE_WAIT])
			        &&	(ause->cur_anim != io->anims[ANIM_BARE_READY])
			        &&	(ause->cur_anim != io->anims[ANIM_BARE_STRIKE_LEFT_START ])
			        &&	(ause->cur_anim != io->anims[ANIM_BARE_STRIKE_LEFT_START+3 ])
			        &&	(ause->cur_anim != io->anims[ANIM_BARE_STRIKE_LEFT_START+6 ])
			        &&	(ause->cur_anim != io->anims[ANIM_BARE_STRIKE_LEFT_START+9 ])
			        &&	(ause->cur_anim != io->anims[ANIM_BARE_STRIKE_LEFT_CYCLE])
			        &&	(ause->cur_anim != io->anims[ANIM_BARE_STRIKE_LEFT_CYCLE+3])
			        &&	(ause->cur_anim != io->anims[ANIM_BARE_STRIKE_LEFT_CYCLE+6])
			        &&	(ause->cur_anim != io->anims[ANIM_BARE_STRIKE_LEFT_CYCLE+9])
			        &&	(ause->cur_anim != io->anims[ANIM_BARE_STRIKE_LEFT ])
			        &&	(ause->cur_anim != io->anims[ANIM_BARE_STRIKE_LEFT+3 ])
			        &&	(ause->cur_anim != io->anims[ANIM_BARE_STRIKE_LEFT+6 ])
			        &&	(ause->cur_anim != io->anims[ANIM_BARE_STRIKE_LEFT+9 ])
			        && (ause->cur_anim != io->anims[ANIM_CAST_CYCLE])
			        && (ause->cur_anim != io->anims[ANIM_CAST])
			        && (ause->cur_anim != io->anims[ANIM_CAST_END])
			        && (ause->cur_anim != io->anims[ANIM_CAST_START]))
			        || (ause->cur_anim == NULL))
			{
				AcquireLastAnim(io);
				FinishAnim(io, ause->cur_anim);
				ANIM_Set(ause, io->anims[ANIM_BARE_WAIT]);
				Strike_StartTickCount(io);
				ause->flags |= EA_LOOP;
			}
		}
		// DAGGER fight !!! ***********************************
		else if (io->_npcdata->weapontype & OBJECT_TYPE_DAGGER)
		{
			if (((ause->cur_anim != io->anims[ANIM_DAGGER_WAIT])
			        &&	(ause->cur_anim != io->anims[ANIM_DAGGER_READY_PART_1])
			        &&	(ause->cur_anim != io->anims[ANIM_DAGGER_READY_PART_2])
			        &&	(ause->cur_anim != io->anims[ANIM_DAGGER_STRIKE_LEFT_START ])
			        &&	(ause->cur_anim != io->anims[ANIM_DAGGER_STRIKE_LEFT_START+3 ])
			        &&	(ause->cur_anim != io->anims[ANIM_DAGGER_STRIKE_LEFT_START+6 ])
			        &&	(ause->cur_anim != io->anims[ANIM_DAGGER_STRIKE_LEFT_START+9 ])
			        &&	(ause->cur_anim != io->anims[ANIM_DAGGER_STRIKE_LEFT_CYCLE])
			        &&	(ause->cur_anim != io->anims[ANIM_DAGGER_STRIKE_LEFT_CYCLE+3])
			        &&	(ause->cur_anim != io->anims[ANIM_DAGGER_STRIKE_LEFT_CYCLE+6])
			        &&	(ause->cur_anim != io->anims[ANIM_DAGGER_STRIKE_LEFT_CYCLE+9])
			        &&	(ause->cur_anim != io->anims[ANIM_DAGGER_STRIKE_LEFT ])
			        &&	(ause->cur_anim != io->anims[ANIM_DAGGER_STRIKE_LEFT+3 ])
			        &&	(ause->cur_anim != io->anims[ANIM_DAGGER_STRIKE_LEFT+6 ])
			        &&	(ause->cur_anim != io->anims[ANIM_DAGGER_STRIKE_LEFT+9 ])
			        && (ause->cur_anim != io->anims[ANIM_CAST_CYCLE])
			        && (ause->cur_anim != io->anims[ANIM_CAST])
			        && (ause->cur_anim != io->anims[ANIM_CAST_END])
			        && (ause->cur_anim != io->anims[ANIM_CAST_START]))
			        || (ause->cur_anim == NULL))
			{
				AcquireLastAnim(io);
				FinishAnim(io, ause->cur_anim);
				ANIM_Set(ause, io->anims[ANIM_DAGGER_WAIT]);
				Strike_StartTickCount(io);
				ause->flags |= EA_LOOP;
			}
		}
		// 1H fight !!! ***************************************
		else if (io->_npcdata->weapontype & OBJECT_TYPE_1H)
		{
			if (((ause->cur_anim != io->anims[ANIM_1H_WAIT])
			        &&	(ause->cur_anim != io->anims[ANIM_1H_READY_PART_1])
			        &&	(ause->cur_anim != io->anims[ANIM_1H_READY_PART_2])
			        &&	(ause->cur_anim != io->anims[ANIM_1H_STRIKE_LEFT_START ])
			        &&	(ause->cur_anim != io->anims[ANIM_1H_STRIKE_LEFT_START+3 ])
			        &&	(ause->cur_anim != io->anims[ANIM_1H_STRIKE_LEFT_START+6 ])
			        &&	(ause->cur_anim != io->anims[ANIM_1H_STRIKE_LEFT_START+9 ])
			        &&	(ause->cur_anim != io->anims[ANIM_1H_STRIKE_LEFT_CYCLE])
			        &&	(ause->cur_anim != io->anims[ANIM_1H_STRIKE_LEFT_CYCLE+3])
			        &&	(ause->cur_anim != io->anims[ANIM_1H_STRIKE_LEFT_CYCLE+6])
			        &&	(ause->cur_anim != io->anims[ANIM_1H_STRIKE_LEFT_CYCLE+9])
			        &&	(ause->cur_anim != io->anims[ANIM_1H_STRIKE_LEFT ])
			        &&	(ause->cur_anim != io->anims[ANIM_1H_STRIKE_LEFT+3 ])
			        &&	(ause->cur_anim != io->anims[ANIM_1H_STRIKE_LEFT+6 ])
			        &&	(ause->cur_anim != io->anims[ANIM_1H_STRIKE_LEFT+9 ])
			        && (ause->cur_anim != io->anims[ANIM_CAST_CYCLE])
			        && (ause->cur_anim != io->anims[ANIM_CAST])
			        && (ause->cur_anim != io->anims[ANIM_CAST_END])
			        && (ause->cur_anim != io->anims[ANIM_CAST_START]))
			        || (ause->cur_anim == NULL))
			{
				AcquireLastAnim(io);
				FinishAnim(io, ause->cur_anim);
				ANIM_Set(ause, io->anims[ANIM_1H_WAIT]);
				Strike_StartTickCount(io);
				ause->flags |= EA_LOOP;
			}
		}
		// 2H fight !!! ***************************************
		else if (io->_npcdata->weapontype & OBJECT_TYPE_2H)
		{
			if (((ause->cur_anim != io->anims[ANIM_2H_WAIT])
			        &&	(ause->cur_anim != io->anims[ANIM_2H_READY_PART_1])
			        &&	(ause->cur_anim != io->anims[ANIM_2H_READY_PART_2])
			        &&	(ause->cur_anim != io->anims[ANIM_2H_STRIKE_LEFT_START ])
			        &&	(ause->cur_anim != io->anims[ANIM_2H_STRIKE_LEFT_START+3 ])
			        &&	(ause->cur_anim != io->anims[ANIM_2H_STRIKE_LEFT_START+6 ])
			        &&	(ause->cur_anim != io->anims[ANIM_2H_STRIKE_LEFT_START+9 ])
			        &&	(ause->cur_anim != io->anims[ANIM_2H_STRIKE_LEFT_CYCLE])
			        &&	(ause->cur_anim != io->anims[ANIM_2H_STRIKE_LEFT_CYCLE+3])
			        &&	(ause->cur_anim != io->anims[ANIM_2H_STRIKE_LEFT_CYCLE+6])
			        &&	(ause->cur_anim != io->anims[ANIM_2H_STRIKE_LEFT_CYCLE+9])
			        &&	(ause->cur_anim != io->anims[ANIM_2H_STRIKE_LEFT ])
			        &&	(ause->cur_anim != io->anims[ANIM_2H_STRIKE_LEFT+3 ])
			        &&	(ause->cur_anim != io->anims[ANIM_2H_STRIKE_LEFT+6 ])
			        &&	(ause->cur_anim != io->anims[ANIM_2H_STRIKE_LEFT+9 ])
			        && (ause->cur_anim != io->anims[ANIM_CAST_CYCLE])
			        && (ause->cur_anim != io->anims[ANIM_CAST])
			        && (ause->cur_anim != io->anims[ANIM_CAST_END])
			        && (ause->cur_anim != io->anims[ANIM_CAST_START]))
			        || (ause->cur_anim == NULL))
			{
				AcquireLastAnim(io);
				FinishAnim(io, ause->cur_anim);
				ANIM_Set(ause, io->anims[ANIM_2H_WAIT]);
				ause->flags |= EA_LOOP;
			}
		}
		// BOW fight !!! **************************************
		else if (io->_npcdata->weapontype & OBJECT_TYPE_BOW)
		{
			////////////// later...
		}
	}
}
//***********************************************************************************************
//***********************************************************************************************
void ARX_NPC_Manage_Anims_End(INTERACTIVE_OBJ * io)
{
	ANIM_USE * ause = &io->animlayer[0];

	if ((ause->flags & EA_ANIMEND) && (ause->cur_anim != NULL))
	{
		if (ause->flags & EA_FORCEPLAY)
		{
			AcquireLastAnim(io);
			FinishAnim(io, ause->cur_anim);
			ANIM_Set(ause, io->anims[ANIM_DEFAULT]);

			if (io->_npcdata->behavior & BEHAVIOUR_FRIENDLY) ause->altidx_cur = 0;
		}

		// some specific code for combat animation end management
		if (ause->cur_anim == io->anims[ANIM_FIGHT_STRAFE_LEFT])
		{
			AcquireLastAnim(io);
			FinishAnim(io, ause->cur_anim);
			ANIM_Set(ause, io->anims[ANIM_FIGHT_WAIT]);
			ause->flags |= EA_LOOP;
		}

		if (ause->cur_anim == io->anims[ANIM_FIGHT_STRAFE_RIGHT])
		{
			AcquireLastAnim(io);
			FinishAnim(io, ause->cur_anim);
			ANIM_Set(ause, io->anims[ANIM_FIGHT_WAIT]);
			ause->flags |= EA_LOOP;
		}

		if (ause->cur_anim == io->anims[ANIM_FIGHT_WALK_BACKWARD])
		{
			AcquireLastAnim(io);
			FinishAnim(io, ause->cur_anim);
			ANIM_Set(ause, io->anims[ANIM_FIGHT_WAIT]);
			ause->flags |= EA_LOOP;
		}
	}
}
bool TryIOAnimMove(INTERACTIVE_OBJ * io, long animnum)
{
	if ((!io) || (!io->anims[animnum])) return false;

	EERIE_3D trans, trans2;
	GetAnimTotalTranslate(io->anims[animnum], 0, &trans);
	float temp = DEG2RAD(MAKEANGLE(180.f - io->angle.b));
	_YRotatePoint(&trans, &trans2, (float)EEcos(temp), (float)EEsin(temp));
	IO_PHYSICS phys;
	memcpy(&phys, &io->physics, sizeof(IO_PHYSICS));
	GetIOCyl(io, &phys.cyl);

	phys.startpos.x = io->pos.x;
	phys.startpos.y = io->pos.y;
	phys.startpos.z = io->pos.z;
	phys.targetpos.x = io->pos.x + trans2.x;
	phys.targetpos.y = io->pos.y + trans2.y;
	phys.targetpos.z = io->pos.z + trans2.z;
	BOOL res = ARX_COLLISION_Move_Cylinder(&phys, io, 30, CFLAG_JUST_TEST | CFLAG_NPC);

	if (res && (EEfabs(phys.cyl.origin.y - io->pos.y) < 20.f))
		return true;

	return false;
}
void TryAndCheckAnim(INTERACTIVE_OBJ * io, long animnum, long layer)
{
	if (!io) return;

	ANIM_USE * ause = &io->animlayer[layer];

	if ((ause->cur_anim != io->anims[animnum])
	        && (ause->cur_anim != NULL))
	{
		if (TryIOAnimMove(io, animnum))
		{
			AcquireLastAnim(io);
			FinishAnim(io, ause->cur_anim);
			ANIM_Set(ause, io->anims[animnum]);
		}
	}
}
//Define Time of Strike Damage
#define STRIKE_MUL 0.25f 
#define STRIKE_MUL2 0.8f 
#define STRIKE_DISTANCE 220
// Main animations management
//***********************************************************************************************
//***********************************************************************************************
void ARX_NPC_Manage_Anims(INTERACTIVE_OBJ * io, float TOLERANCE)
{
	io->_npcdata->strike_time += (short)FrameDiff;
	ANIM_USE * ause = &io->animlayer[0];
	ANIM_USE * ause1 = &io->animlayer[1];
	float tdist = 999999.f;

	if (ValidIONum(io->targetinfo))
	{
		tdist = EEDistance3D(&io->pos, &inter.iobj[io->targetinfo]->pos);
	}

	if ((io->_npcdata->pathfind.listnb)
	        &&	(ValidIONum(io->_npcdata->pathfind.truetarget)))
	{
		tdist = EEDistance3D(&io->pos, &inter.iobj[io->_npcdata->pathfind.truetarget]->pos);
	}




	INTERACTIVE_OBJ * ioo = (INTERACTIVE_OBJ *)io->_npcdata->weapon;

	if (ValidIOAddress(ioo))
		io->_npcdata->weapontype = ioo->type_flags;
	else
		io->_npcdata->weapontype = 0;

	if ((io->_npcdata->behavior & BEHAVIOUR_FIGHT)
	        &&	(tdist <= TOLERANCE + 10)
	        &&	((tdist <= TOLERANCE - 20) || (rnd() > 0.97f)))
	{
		{
			if ((ause->cur_anim == io->anims[ANIM_FIGHT_WAIT])
			        && (ause->cur_anim != NULL))
			{
				float r = rnd();

				if (tdist < TOLERANCE - 20) r = 0;

				if (r < 0.1f)
					TryAndCheckAnim(io, ANIM_FIGHT_WALK_BACKWARD, 0);
				else if (r < 0.55f)
					TryAndCheckAnim(io, ANIM_FIGHT_STRAFE_LEFT, 0);
				else
					TryAndCheckAnim(io, ANIM_FIGHT_STRAFE_RIGHT, 0);

			}
		}
	}
	//Decides fight moves
	else if ((io->_npcdata->behavior & (BEHAVIOUR_MAGIC | BEHAVIOUR_DISTANT))
	         ||	(io->spellcast_data.castingspell >= 0))
	{
		if (rnd() > 0.85f)
		{
			if ((ause->cur_anim == io->anims[ANIM_FIGHT_WAIT])
			        && (ause->cur_anim != NULL))
			{
				AcquireLastAnim(io);
				FinishAnim(io, ause->cur_anim);
				float r = rnd();

				if (tdist < 340) r = 0;

				if (r < 0.33f)
					TryAndCheckAnim(io, ANIM_FIGHT_WALK_BACKWARD, 0);
				else if (r < 0.66f)
					TryAndCheckAnim(io, ANIM_FIGHT_STRAFE_LEFT, 0);
				else
					TryAndCheckAnim(io, ANIM_FIGHT_STRAFE_RIGHT, 0);
			}
		}
	}


	if (IsPlayerStriking())
	{
		if ((ause->cur_anim == io->anims[ANIM_FIGHT_WAIT])
		        && (ause->cur_anim != NULL))
		{
			AcquireLastAnim(io);
			FinishAnim(io, ause->cur_anim);
			float r = rnd();

			if (r < 0.2f)
				TryAndCheckAnim(io, ANIM_FIGHT_WALK_BACKWARD, 0);
			else if (r < 0.6f)
				TryAndCheckAnim(io, ANIM_FIGHT_STRAFE_LEFT, 0);
			else
				TryAndCheckAnim(io, ANIM_FIGHT_STRAFE_RIGHT, 0);
		}
	}
	long j;






	// MAGICAL FIGHT
	if ((ause1->cur_anim == io->anims[ANIM_CAST])
	        &&	(io->anims[ANIM_CAST])
	        &&	(ause1->flags & EA_ANIMEND))
	{
		AcquireLastAnim(io);
		FinishAnim(io, ause1->cur_anim);
		ANIM_Set(ause1, io->anims[ANIM_CAST_END]);
	}
	else if ((ause1->cur_anim == io->anims[ANIM_CAST_END])
	         &&	(io->anims[ANIM_CAST_END])
	         &&	(ause1->flags & EA_ANIMEND))
	{
		AcquireLastAnim(io);
		FinishAnim(io, ause1->cur_anim);
		ause1->cur_anim = NULL;
	}

	if ((io->spellcast_data.castingspell < 0)
	        &&	(ause1->cur_anim == io->anims[ANIM_CAST_START])
	        &&	(io->anims[ANIM_CAST_START]))
	{
		AcquireLastAnim(io);
		FinishAnim(io, ause1->cur_anim);
		ANIM_Set(ause1, io->anims[ANIM_CAST]);
	}

	if ((ause1->cur_anim == io->anims[ANIM_CAST_START])
	        &&	(io->anims[ANIM_CAST_START]))
	{
		return;
	}

	if ((ause1->cur_anim == io->anims[ANIM_CAST_CYCLE])
	        &&	(io->anims[ANIM_CAST_CYCLE]))
	{
		return;
	}

	if (io->spellcast_data.castingspell >= 0) return;

	if (ause1->cur_anim)
	{
		if ((ause1->cur_anim == io->anims[ANIM_CAST_CYCLE])
		        ||	(ause1->cur_anim == io->anims[ANIM_CAST])
		        ||	(ause1->cur_anim == io->anims[ANIM_CAST_END])
		        ||	(ause1->cur_anim == io->anims[ANIM_CAST_START]))
			return;
	}

	// BARE HANDS fight !!! *******************************
	if (io->_npcdata->weapontype == 0)
	{
		if (io->_npcdata->weaponinhand == -1)
		{
			io->_npcdata->weaponinhand = 1;
		}

		if ((ause1->cur_anim == io->anims[ANIM_BARE_WAIT])
		        &&	(ause1->cur_anim))
		{
			if ((io->_npcdata->behavior & BEHAVIOUR_FIGHT)
			        &&	(tdist < STRIKE_DISTANCE)
			        &&	(io->_npcdata->strike_time > 0))
			{
				AcquireLastAnim(io);
				F2L(rnd() * 4, &j);

				if (j < 0) j = 0;

				if (j > 3) j = 3;

				FinishAnim(io, ause1->cur_anim);
				ANIM_Set(ause1, io->anims[ANIM_BARE_STRIKE_LEFT_START+j*3]);
			}
		}

		for (j = 0; j < 4; j++)
		{
			if ((ause1->cur_anim == io->anims[ANIM_BARE_STRIKE_LEFT_START+j*3])
			        &&	(ause1->cur_anim)
			        &&	(ause1->flags & EA_ANIMEND))
			{
				io->ioflags &= ~IO_HIT;
				AcquireLastAnim(io);
				FinishAnim(io, ause1->cur_anim);
				ANIM_Set(ause1, io->anims[ANIM_BARE_STRIKE_LEFT_CYCLE+j*3]);
				io->_npcdata->aiming_start	=	lARXTime;
				ause1->flags |= EA_LOOP;
			}
			else if ((ause1->cur_anim == io->anims[ANIM_BARE_STRIKE_LEFT_CYCLE+j*3])
			         &&	(ause1->cur_anim))
			{
				if (((ARXTime > io->_npcdata->aiming_start + io->_npcdata->aimtime) || ((ARXTime > io->_npcdata->aiming_start + io->_npcdata->aimtime * DIV2) && (rnd() > 0.9f)))
				        &&	(tdist < STRIKE_DISTANCE))
				{
					AcquireLastAnim(io);
					FinishAnim(io, ause1->cur_anim);
					ANIM_Set(ause1, io->anims[ANIM_BARE_STRIKE_LEFT+j*3]);
					SendIOScriptEvent(io, SM_STRIKE, "BARE");
				}
			}
			else if ((ause1->cur_anim == io->anims[ANIM_BARE_STRIKE_LEFT+j*3])
			         &&	(ause1->cur_anim))
			{

				if (ause1->flags & EA_ANIMEND)
				{
					AcquireLastAnim(io);
					FinishAnim(io, ause1->cur_anim);
					ANIM_Set(ause1, io->anims[ANIM_BARE_WAIT]);
					Strike_StartTickCount(io);
					io->ioflags &= ~IO_HIT;

					if ((io->ioflags & IO_NPC) && (!io->_npcdata->reachedtarget))
						ause1->cur_anim = NULL;
				}
				else if (!(io->ioflags & IO_HIT))
				{
					if ((ause1->ctime > ause1->cur_anim->anims[0]->anim_time * STRIKE_MUL)
					        &&	(ause1->ctime <= ause1->cur_anim->anims[0]->anim_time * STRIKE_MUL2))
					{
						CheckHit(io, 1.f);
						io->ioflags |= IO_HIT;
					}
				}
			}
		}

	}
	
	// 1H fight !!! ***************************************
	else if (io->_npcdata->weapontype & (OBJECT_TYPE_1H | OBJECT_TYPE_2H | OBJECT_TYPE_DAGGER))
	{
		long ANIMBase = 0;

		if (io->_npcdata->weapontype & OBJECT_TYPE_1H)
			ANIMBase = 0;
		else if (io->_npcdata->weapontype & OBJECT_TYPE_2H)
			ANIMBase = ANIM_2H_READY_PART_1 - ANIM_1H_READY_PART_1;
		else if (io->_npcdata->weapontype & OBJECT_TYPE_DAGGER)
			ANIMBase = ANIM_DAGGER_READY_PART_1 - ANIM_1H_READY_PART_1;

		// desire to remove weapon
		if (io->_npcdata->weaponinhand == 2)
		{
			if ((ause1->cur_anim == io->anims[ANIM_1H_UNREADY_PART_1+ANIMBase])
			        &&	(ause1->cur_anim)
			        &&	(ause1->flags & EA_ANIMEND))
			{
				if (io->_npcdata->weaponname[0] != 0)
					SetWeapon_Back(io);

				AcquireLastAnim(io);
				FinishAnim(io, ause1->cur_anim);
				ANIM_Set(ause1, io->anims[ANIM_1H_UNREADY_PART_2+ANIMBase]);
			}
			else if ((ause1->cur_anim == io->anims[ANIM_1H_UNREADY_PART_2+ANIMBase])
			         &&	(ause1->cur_anim)
			         &&	(ause1->flags & EA_ANIMEND))
			{
				AcquireLastAnim(io);
				FinishAnim(io, ause1->cur_anim);
				ause1->cur_anim = NULL;
				io->_npcdata->weaponinhand = 0;
			}
			else if ((ause1->cur_anim != io->anims[ANIM_1H_UNREADY_PART_1+ANIMBase])
			         &&	(ause1->cur_anim != io->anims[ANIM_1H_UNREADY_PART_2+ANIMBase]))
			{
				AcquireLastAnim(io);
				FinishAnim(io, ause1->cur_anim);
				ANIM_Set(ause1, io->anims[ANIM_1H_UNREADY_PART_1+ANIMBase]);
			}
		}
		// Desire to have weapon in hand...
		else if (io->_npcdata->weaponinhand == -1)
		{
			if ((ause1->cur_anim == io->anims[ANIM_1H_READY_PART_1+ANIMBase])
			        &&	(ause1->cur_anim)
			        &&	(ause1->flags & EA_ANIMEND))
			{
				if (io->_npcdata->weaponname[0] != 0)
					SetWeapon_On(io);

				AcquireLastAnim(io);
				FinishAnim(io, ause1->cur_anim);
				ANIM_Set(ause1, io->anims[ANIM_1H_READY_PART_2+ANIMBase]);

			}
			else if ((ause1->cur_anim == io->anims[ANIM_1H_READY_PART_2+ANIMBase])
			         &&	(ause1->cur_anim)
			         &&	(ause1->flags & EA_ANIMEND))
			{
				AcquireLastAnim(io);
				FinishAnim(io, ause1->cur_anim);

				if (io->_npcdata->behavior & BEHAVIOUR_FIGHT)
				{
					ANIM_Set(ause1, io->anims[ANIM_1H_WAIT+ANIMBase]);
					Strike_StartTickCount(io);
					ause1->flags |= EA_LOOP;
				}
				else ause1->cur_anim = NULL;

				io->_npcdata->weaponinhand = 1;
			}
			else if ((!ause1->cur_anim)
			         ||	((ause1->cur_anim) && (ause1->flags & EA_ANIMEND)))
			{
				AcquireLastAnim(io);
				FinishAnim(io, ause1->cur_anim);
				ANIM_Set(ause1, io->anims[ANIM_1H_READY_PART_1+ANIMBase]);
			}
		}
		// Weapon in hand... ready to strike
		else if (io->_npcdata->weaponinhand > 0)
		{
			if ((ause1->cur_anim == io->anims[ANIM_1H_WAIT+ANIMBase])
			        &&	(ause1->cur_anim))
			{
				if ((io->_npcdata->behavior & BEHAVIOUR_FIGHT)
				        &&	(tdist < STRIKE_DISTANCE)
				        &&	(io->_npcdata->strike_time > 0))
				{
					AcquireLastAnim(io);
					FinishAnim(io, ause1->cur_anim);
					F2L(rnd() * 4, &j);

					if (j < 0) j = 0;

					if (j > 3) j = 3;

					ANIM_Set(ause1, io->anims[ANIM_1H_STRIKE_LEFT_START+j*3+ANIMBase]);
				}
			}

			for (j = 0; j < 4; j++)
			{
				if ((ause1->cur_anim == io->anims[ANIM_1H_STRIKE_LEFT_START+j*3+ANIMBase])
				        &&	(ause1->cur_anim)
				        &&	(ause1->flags & EA_ANIMEND))
				{
					AcquireLastAnim(io);
					FinishAnim(io, ause1->cur_anim);
					ANIM_Set(ause1, io->anims[ANIM_1H_STRIKE_LEFT_CYCLE+j*3+ANIMBase]);
					io->_npcdata->aiming_start	=	lARXTime;
					ause1->flags |= EA_LOOP;
				}
				else if ((ause1->cur_anim == io->anims[ANIM_1H_STRIKE_LEFT_CYCLE+j*3+ANIMBase])
				         &&	(ause1->cur_anim))
				{
					if (((ARXTime > io->_npcdata->aiming_start + io->_npcdata->aimtime) || ((ARXTime > io->_npcdata->aiming_start + io->_npcdata->aimtime * DIV2) && (rnd() > 0.9f)))
					        &&	(tdist < STRIKE_DISTANCE))
					{
						AcquireLastAnim(io);
						FinishAnim(io, ause1->cur_anim);
						ANIM_Set(ause1, io->anims[ANIM_1H_STRIKE_LEFT+j*3+ANIMBase]);

						if (io->_npcdata->weapontype & OBJECT_TYPE_1H)
							SendIOScriptEvent(io, SM_STRIKE, "1H");

						if (io->_npcdata->weapontype & OBJECT_TYPE_2H)
							SendIOScriptEvent(io, SM_STRIKE, "2H");

						if (io->_npcdata->weapontype & OBJECT_TYPE_DAGGER)
							SendIOScriptEvent(io, SM_STRIKE, "DAGGER");
					}
				}
				else if ((ause1->cur_anim == io->anims[ANIM_1H_STRIKE_LEFT+j*3+ANIMBase])
				         &&	(ause1->cur_anim))
				{
					if (ause1->flags & EA_ANIMEND)
					{
						AcquireLastAnim(io);
						FinishAnim(io, ause1->cur_anim);
						ANIM_Set(ause1, io->anims[ANIM_1H_WAIT+ANIMBase]);
						Strike_StartTickCount(io);
						io->ioflags &= ~IO_HIT;
					}
					else 
					{
						if ((ause1->ctime > ause1->cur_anim->anims[0]->anim_time * STRIKE_MUL)
						        &&	(ause1->ctime <= ause1->cur_anim->anims[0]->anim_time * STRIKE_MUL2))
						{
							if (!(io->ioflags & IO_HIT))
							{
								if (ARX_EQUIPMENT_Strike_Check(io, (INTERACTIVE_OBJ *)io->_npcdata->weapon, 1, 0, io->targetinfo))
									io->ioflags |= IO_HIT;
							}
							else
								ARX_EQUIPMENT_Strike_Check(io, (INTERACTIVE_OBJ *)io->_npcdata->weapon, 1, 1, io->targetinfo);

						}
					}
				}
			}
		}
	}
	// BOW fight !!! **************************************
	else if (io->_npcdata->weapontype & OBJECT_TYPE_BOW)
	{
		////////////// later...
	}



}
float GetIOHeight(INTERACTIVE_OBJ * io)
{
	if (io == inter.iobj[0])
	{
		return io->physics.cyl.height; 
	}

	float v = (io->original_height * io->scale);

	if (v < -165.f) return -165.f;

	return __min(v, -45.f);
}
float GetIORadius(INTERACTIVE_OBJ * io)
{
	if (io == inter.iobj[0])
		return PLAYER_BASE_RADIUS;

	float v = __max(io->original_radius * io->scale, 25.f);

	if (v > 60.f) return 60.f;

	return v;
}
void GetIOCyl(INTERACTIVE_OBJ * io, EERIE_CYLINDER * cyl)
{
	cyl->height = GetIOHeight(io);
	cyl->radius = GetIORadius(io);

	if (io == inter.iobj[0])
	{
		cyl->origin.x = inter.iobj[0]->pos.x;
		cyl->origin.y = inter.iobj[0]->pos.y; 
		cyl->origin.z = inter.iobj[0]->pos.z; 
	}
	else
		Vector_Copy(&cyl->origin, &io->pos);
}

//***********************************************************************************************
// Computes distance tolerance between NPC and its target
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/15)
//***********************************************************************************************
void ComputeTolerance(INTERACTIVE_OBJ * io, long targ, float * dst)
{
	float TOLERANCE = 30.f;

	if ((targ >= 0)
	        &&	(targ < inter.nbmax)
	        &&	(inter.iobj[targ]))
	{
		float self_dist, targ_dist;

		// Compute min target close-dist
		if (inter.iobj[targ]->ioflags & IO_NO_COLLISIONS)
			targ_dist = 0.f;
		else targ_dist = __max(inter.iobj[targ]->physics.cyl.radius, GetIORadius(inter.iobj[targ])); //inter.iobj[targ]->physics.cyl.radius;

		// Compute min self close-dist
		if (io->ioflags & IO_NO_COLLISIONS)
			self_dist = 0.f;
		else self_dist = __max(io->physics.cyl.radius, GetIORadius(io)); //io->physics.cyl.radius;

		// Base tolerance = radius added
		TOLERANCE = targ_dist + self_dist + 5.f;

		if (TOLERANCE < 0.f)
		{
			TOLERANCE = 0.f;
		}

		if (inter.iobj[targ]->ioflags & IO_FIX)
			TOLERANCE += 100.f;

		// If target is a NPC add another tolerance
		if (inter.iobj[targ]->_npcdata)
		{
			TOLERANCE += 20.f;
		}

		// If target is the player improve again tolerance
		if (io->targetinfo == 0) // PLAYER TARGET
		{
			TOLERANCE += 10.f;
		}

		if (io->_npcdata->behavior & BEHAVIOUR_FIGHT)
		{
			TOLERANCE += io->_npcdata->reach * 0.7f;
		}

		// If distant of magic behavior Maximize tolerance
		if ((io->_npcdata->behavior & (BEHAVIOUR_MAGIC | BEHAVIOUR_DISTANT)) || (io->spellcast_data.castingspell >= 0))
			TOLERANCE += 300.f;

		// if target is a marker set to a minimal tolerance
		if (inter.iobj[targ]->ioflags & IO_MARKER)
			TOLERANCE = 21.f + (float)io->_npcdata->moveproblem * DIV10;
	}

	// Tolerance is modified by current moveproblem status
	TOLERANCE += (float)io->_npcdata->moveproblem * DIV10;
	// Now fill our return value with TOLERANCE
	*dst = TOLERANCE;
}
extern long FRAME_COUNT;
//now APOS is computed in Anim but used here and mustn't be used elsewhere...
//***********************************************************************************************
//***********************************************************************************************

void ManageNPCMovement(INTERACTIVE_OBJ * io)
{
	float dis = FLT_MAX;
	IO_PHYSICS phys;
	float TOLERANCE = 0.f;
	float TOLERANCE2 = 0.f;

	// Ignores invalid or dead IO
	if	((!io)
	        ||	(!io->show)
	        ||	(!(io->ioflags & IO_NPC)))
		return;

	//	AnchorData_GetNearest_2(io->angle.b,&io->pos,&io->physics.cyl);
	// Specific USEPATH management
	ARX_USE_PATH * aup = (ARX_USE_PATH *)io->usepath;

	if ((aup)
	        &&	(aup->aupflags & ARX_USEPATH_WORM_SPECIFIC))
	{
		io->room_flags |= 1;
		EERIE_3D tv;

		if (aup->_curtime - aup->_starttime > 500)
		{
			aup->_curtime -= 500;
			ARX_PATHS_Interpolate(aup, &tv);
			aup->_curtime += 500;
			io->angle.b = MAKEANGLE(RAD2DEG(GetAngle(tv.x, tv.z, io->pos.x, io->pos.z)));
		}
		else
		{
			aup->_curtime += 500;
			ARX_PATHS_Interpolate(aup, &tv);
			aup->_curtime -= 500;
			io->angle.b = MAKEANGLE(180.f + RAD2DEG(GetAngle(tv.x, tv.z, io->pos.x, io->pos.z)));
		}

		return;
	}

	// Frozen ?
	if (io->ioflags & IO_FREEZESCRIPT)
		return;

	// Dead ?
	if (IsDeadNPC(io))
	{
		io->ioflags |= IO_NO_COLLISIONS;
		io->animlayer[0].next_anim = NULL;
		io->animlayer[1].next_anim = NULL;
		io->animlayer[2].next_anim = NULL;
		io->animlayer[3].next_anim = NULL;
		return;
	}

	ANIM_USE * ause0 = &io->animlayer[0];
	ANIM_HANDLE ** alist = io->anims;

	// Using USER animation ?
	if ((ause0->cur_anim)
	        &&	(ause0->flags & EA_FORCEPLAY)
	        &&	(ause0->cur_anim != alist[ANIM_DIE])
	        &&	(ause0->cur_anim != alist[ANIM_HIT1])
	        &&	(ause0->cur_anim != alist[ANIM_HIT_SHORT])
	        &&	!(ause0->flags & EA_ANIMEND))
	{
		io->room_flags |= 1;
		io->lastpos.x = io->pos.x += io->move.x;
		io->lastpos.y = io->pos.y += io->move.y;
		io->lastpos.z = io->pos.z += io->move.z;

		return;
	}

	if ((io->_npcdata->pathfind.listnb > 0)
	        &&	(!io->_npcdata->pathfind.list))
		io->_npcdata->pathfind.listnb = 0;


	// waiting for pathfinder ? or pathfinder failure ? ---> Wait Anim
	if ((io->_npcdata->pathfind.pathwait)		// waiting for pathfinder
	        ||	(io->_npcdata->pathfind.listnb == -2))	// pathfinder failure
	{
		if (io->_npcdata->pathfind.listnb == -2)
		{
			if (!io->_npcdata->pathfind.pathwait)
			{
				if (io->_npcdata->pathfind.flags & PATHFIND_NO_UPDATE)
				{
					io->_npcdata->reachedtarget = 1;
					io->_npcdata->reachedtime = ARXTimeUL();//treat warning C4244 conversion from 'float' to 'unsigned long'

					if (io->targetinfo != GetInterNum(io))
						SendIOScriptEvent(io, SM_REACHEDTARGET, "");
				}
				else if ((ause0->cur_anim == alist[ANIM_WAIT]) && (ause0->flags & EA_ANIMEND))
				{
					io->_npcdata->pathfind.listnb = -1;
					io->_npcdata->pathfind.pathwait = 0;
					ARX_NPC_LaunchPathfind(io, io->targetinfo);
					goto afterthat;
				}
			}
		}

		if (!(io->_npcdata->behavior & BEHAVIOUR_FIGHT))
		{
			if ((ause0->cur_anim == alist[ANIM_WALK])
			        ||	(ause0->cur_anim == alist[ANIM_RUN])
			        || (ause0->cur_anim == alist[ANIM_WALK_SNEAK]))
			{
				AcquireLastAnim(io);
				FinishAnim(io, ause0->cur_anim);
				ANIM_Set(ause0, alist[ANIM_WAIT]);
				ause0->altidx_cur = 0;
				return;
			}
			else if (ause0->cur_anim == alist[ANIM_WAIT])
			{
				if (ause0->flags & EA_ANIMEND)
				{
					FinishAnim(io, ause0->cur_anim);
					ANIM_Set(ause0, alist[ANIM_WAIT]);
					ause0->altidx_cur = 0;
				}

				return;
			}
		}

	afterthat:
		;
	}

	if ((io->_npcdata->behavior  & BEHAVIOUR_NONE))
	{
		ARX_NPC_Manage_Anims(io, 0);

		if ((ause0->cur_anim == NULL)
		        ||	((ause0->cur_anim) && (ause0->flags & EA_ANIMEND)))
		{
			AcquireLastAnim(io);
			FinishAnim(io, ause0->cur_anim);
			ANIM_Set(ause0, alist[ANIM_WAIT]);
			ause0->flags &= ~EA_LOOP;
		}

		return;
	}

	// First retrieves current position of target...
	GetTargetPos(io);

	ARX_NPC_Manage_Anims_End(io);

	if ((io->_npcdata->behavior & BEHAVIOUR_LOOK_AROUND)
	        && (io->_npcdata->pathfind.listnb <= 0)
	        &&  !(io->_npcdata->pathfind.pathwait))
	{
		ARX_NPC_LaunchPathfind(io, io->targetinfo);
	}

	if ((io->_npcdata->behavior & (BEHAVIOUR_FRIENDLY | BEHAVIOUR_NONE))
	        && ((ause0->cur_anim == alist[ANIM_WALK])
	            || (ause0->cur_anim == alist[ANIM_WALK_SNEAK])
	            ||	(ause0->cur_anim == alist[ANIM_FIGHT_WALK_FORWARD])
	            ||	(ause0->cur_anim == alist[ANIM_RUN])
	            ||	(ause0->cur_anim == alist[ANIM_RUN])
	            ||	(ause0->cur_anim == alist[ANIM_FIGHT_STRAFE_LEFT])
	            ||	(ause0->cur_anim == alist[ANIM_FIGHT_STRAFE_RIGHT])
	            ||	(ause0->cur_anim == alist[ANIM_FIGHT_WALK_BACKWARD])
	           ))
	{
		AcquireLastAnim(io);
		FinishAnim(io, ause0->cur_anim);
		ANIM_Set(ause0, alist[ANIM_WAIT]);
		ause0->altidx_cur = 0;
	}


	// look around if finished fleeing or being looking around !
	if ((io->_npcdata->behavior & BEHAVIOUR_LOOK_AROUND)
	        &&	(EEDistance3D(&io->pos, &io->target) > 150.f))
	{
		if (!io->_npcdata->ex_rotate)
		{
			ARX_NPC_CreateExRotateData(io);
		}
		else // already created
		{
			if	(((ause0->cur_anim == alist[ANIM_WAIT])
			        ||	(ause0->cur_anim == alist[ANIM_WALK])
			        || (ause0->cur_anim == alist[ANIM_WALK_SNEAK])
			        ||	(ause0->cur_anim == alist[ANIM_FIGHT_WALK_FORWARD])
			        ||	(ause0->cur_anim == alist[ANIM_RUN]))
			        ||	(ause0->cur_anim != NULL))
			{
				io->_npcdata->look_around_inc = 0.f;

				for (long n = 0; n < 4; n++)
				{
					io->_npcdata->ex_rotate->group_rotate[n].b -= io->_npcdata->ex_rotate->group_rotate[n].b * DIV3;

					if (fabs(io->_npcdata->ex_rotate->group_rotate[n].b) < 0.01f)
						io->_npcdata->ex_rotate->group_rotate[n].b = 0.f;
				}
			}
			else
			{
				if (io->_npcdata->look_around_inc == 0.f)
				{
					io->_npcdata->look_around_inc = (rnd() - 0.5f) * 0.08f;

				}

				for (long n = 0; n < 4; n++)
				{
					float t = 1.5f - (float)n * DIV5;
					io->_npcdata->ex_rotate->group_rotate[n].b += io->_npcdata->look_around_inc * _framedelay * t;
				}

				if (io->_npcdata->ex_rotate->group_rotate[0].b > 30)
					io->_npcdata->look_around_inc = -io->_npcdata->look_around_inc;

				if (io->_npcdata->ex_rotate->group_rotate[0].b < -30)
					io->_npcdata->look_around_inc = -io->_npcdata->look_around_inc;
			}
		}

	}
	else
	{
		if ((!(io->_npcdata->behavior & BEHAVIOUR_STARE_AT))
		        &&	(io->_npcdata->ex_rotate != NULL))
		{
			io->_npcdata->look_around_inc = 0.f;

			for (long n = 0; n < 4; n++)
			{
				io->_npcdata->ex_rotate->group_rotate[n].b -= io->_npcdata->ex_rotate->group_rotate[n].b * DIV3;

				if (fabs(io->_npcdata->ex_rotate->group_rotate[n].b) < 0.01f)
					io->_npcdata->ex_rotate->group_rotate[n].b = 0.f;
			}
		}
	}

	ause0->flags &= ~EA_STATICANIM;

	if ((ause0->cur_anim)
	        &&	((ause0->cur_anim == alist[ANIM_HIT1]) || (ause0->cur_anim == alist[ANIM_HIT_SHORT])))
	{
		if (ause0->flags & EA_ANIMEND)
		{
			AcquireLastAnim(io);
			FinishAnim(io, ause0->cur_anim);

			if (io->_npcdata->behavior & BEHAVIOUR_FIGHT)
				ANIM_Set(ause0, alist[ANIM_FIGHT_WAIT]);
			else
				ANIM_Set(ause0, alist[ANIM_WAIT]);

			if (io->_npcdata->behavior & BEHAVIOUR_FRIENDLY)
				ause0->altidx_cur = 0;
		}
		else
			return;
	}

	float dist = FLT_MAX;
	long CHANGE = 0;

	// GetTargetPos MUST be called before FaceTarget2
	if ((io->_npcdata->pathfind.listnb > 0)
	        &&	(io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND)
	        &&	(io->_npcdata->pathfind.listpos < io->_npcdata->pathfind.listnb - 2)
	        &&	(io->_npcdata->pathfind.list[io->_npcdata->pathfind.listpos] == io->_npcdata->pathfind.list[io->_npcdata->pathfind.listpos+1]))
	{
		if (ause0->cur_anim != io->anims[ANIM_DEFAULT])
		{
			AcquireLastAnim(io);
			FinishAnim(io, ause0->cur_anim);
			ANIM_Set(ause0, alist[ANIM_DEFAULT]);

			if (io->_npcdata->behavior & BEHAVIOUR_FRIENDLY) ause0->altidx_cur = 0;
		}
		else if (ause0->flags & EA_ANIMEND)
		{
			ause0->flags &= ~EA_FORCEPLAY;
			goto argh;
		}

		return;
	}

	// XS : Moved to top of func
	dist = TRUEDistance2D(io->pos.x, io->pos.z, io->target.x, io->target.z);
	dis = dist;

	if (io->_npcdata->pathfind.listnb > 0)
		dis = GetTRUETargetDist(io);

	if (io->_npcdata->behavior & BEHAVIOUR_FLEE)
		dis = 9999999;

	// Force to flee/wander again
	if ((!io->_npcdata->pathfind.pathwait)
	        &&	(io->_npcdata->pathfind.listnb <= 0))
	{
		if (io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND)
		{
			ARX_NPC_LaunchPathfind(io, io->targetinfo); 
		}
		else if ((dis > 220)
		         &&	(io->_npcdata->behavior & BEHAVIOUR_MOVE_TO)
		         &&	(!(io->_npcdata->behavior & (BEHAVIOUR_FIGHT | BEHAVIOUR_MAGIC | BEHAVIOUR_SNEAK))))
		{
			ARX_NPC_LaunchPathfind(io, io->targetinfo);
		}
	}

	if ((io->_npcdata->pathfind.listnb <= 0)
	        &&	(io->_npcdata->behavior & BEHAVIOUR_FLEE))
	{
		if (ause0->cur_anim != alist[ANIM_DEFAULT])
		{
			AcquireLastAnim(io);
			FinishAnim(io, ause0->cur_anim);
			ANIM_Set(ause0, alist[ANIM_DEFAULT]);

			if (io->_npcdata->behavior & BEHAVIOUR_FRIENDLY) ause0->altidx_cur = 0;
		}
	}
	else 	// Force object to walk if using pathfinder !!!
	{
		if (io->_npcdata->behavior & (BEHAVIOUR_FRIENDLY | BEHAVIOUR_NONE))
		{
		}
		else if (!io->_npcdata->reachedtarget)
		{
			if (
			    ((io->targetinfo != -2) 
			     || (io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND)
			     || (io->_npcdata->behavior & BEHAVIOUR_FLEE)
			     || (io->_npcdata->behavior & BEHAVIOUR_GO_HOME))
			    && (io->_npcdata->pathfind.listnb > 0)
			    && (ause0->cur_anim != alist[ANIM_WALK])
			    && (ause0->cur_anim != alist[ANIM_FIGHT_WALK_FORWARD])
			    && (ause0->cur_anim != alist[ANIM_RUN])
			    && (ause0->cur_anim != alist[ANIM_WALK_SNEAK])
			    && (!(ause0->flags & EA_FORCEPLAY))
			)
			{
				AcquireLastAnim(io);
				FinishAnim(io, ause0->cur_anim);

				if ((dis <= RUN_WALK_RADIUS) && (io->_npcdata->behavior & BEHAVIOUR_FIGHT))
				{
					ANIM_Set(ause0, alist[ANIM_FIGHT_WALK_FORWARD]);
				}
				else
					switch (io->_npcdata->movemode)
					{
						case SNEAKMODE:
							ANIM_Set(ause0, alist[ANIM_WALK_SNEAK]);
							break;
						case WALKMODE:
							ANIM_Set(ause0, alist[ANIM_WALK]);
							break;
						case RUNMODE:

							if (dis <= RUN_WALK_RADIUS)
							{
								ANIM_Set(ause0, alist[ANIM_WALK]);
							}
							else
							{
								if (alist[ANIM_RUN])
									ANIM_Set(ause0, alist[ANIM_RUN]);
								else ANIM_Set(ause0, alist[ANIM_WALK]);
							}

							break;
						case NOMOVEMODE:
							ANIM_Set(ause0, alist[ANIM_WAIT]);

							if (io->_npcdata->behavior & BEHAVIOUR_FRIENDLY) ause0->altidx_cur = 0;

							break;
					}

				ause0->flags |= EA_LOOP;
				io->_npcdata->walk_start_time = 0;
			
			}
		}
		else // our object has reached its target...
		{
			// Put it in Fighting stance if using Fight Behavior
			if ((io->_npcdata->behavior & (BEHAVIOUR_FIGHT | BEHAVIOUR_MAGIC | BEHAVIOUR_DISTANT))
			        && io->anims[ANIM_FIGHT_WAIT])
			{
				if ((ause0->cur_anim != alist[ANIM_FIGHT_WAIT])
				        &&	(ause0->cur_anim != alist[ANIM_FIGHT_STRAFE_LEFT])
				        &&	(ause0->cur_anim != alist[ANIM_FIGHT_STRAFE_RIGHT])
				        &&	(ause0->cur_anim != alist[ANIM_FIGHT_WALK_BACKWARD])
				        &&	(ause0->cur_anim != alist[ANIM_FIGHT_WALK_FORWARD])
				   )

				AcquireLastAnim(io);
				FinishAnim(io, ause0->cur_anim);
				ANIM_Set(ause0, alist[ANIM_FIGHT_WAIT]);
				ause0->flags |= EA_LOOP;
				
			}
			// Stop it and put it in Wait anim after finishing his walk anim...
			else
			{
				if	(ause0->cur_anim == alist[ANIM_FIGHT_WALK_FORWARD])
				{
					ause0->flags &= ~EA_LOOP;
					AcquireLastAnim(io);
					FinishAnim(io, ause0->cur_anim);
					ANIM_Set(ause0, alist[ANIM_DEFAULT]);

					if (io->_npcdata->behavior & BEHAVIOUR_FRIENDLY) ause0->altidx_cur = 0;

					ause0->flags |= EA_LOOP;
				}
				else if ((ause0->cur_anim == alist[ANIM_WALK])
				         || (ause0->cur_anim == alist[ANIM_RUN])
				         || (ause0->cur_anim == alist[ANIM_WALK_SNEAK]))
				{
					ause0->flags &= ~EA_LOOP;

					if (io->_npcdata->reachedtime + 500 < ARXTime)
					{
						AcquireLastAnim(io);
						FinishAnim(io, ause0->cur_anim);
						ANIM_Set(ause0, alist[ANIM_DEFAULT]);

						if (io->_npcdata->behavior & BEHAVIOUR_FRIENDLY) ause0->altidx_cur = 0;

						ause0->flags |= EA_LOOP;
					}
				}

				if (!(ause0->flags & EA_FORCEPLAY)
				        &&	(ause0->cur_anim != alist[ANIM_DEFAULT])
				        &&	(ause0->cur_anim != alist[ANIM_FIGHT_WAIT])
				        && (ause0->flags & EA_ANIMEND)
				        &&	!(ause0->flags & EA_LOOP))
				{
					AcquireLastAnim(io);
					FinishAnim(io, ause0->cur_anim);
					ANIM_Set(ause0, alist[ANIM_DEFAULT]);

					if (io->_npcdata->behavior & BEHAVIOUR_FRIENDLY) ause0->altidx_cur = 0;
				}
			}
		}
	}

	// Force Run when far from target and using RUNMODE
	if ((dis > RUN_WALK_RADIUS) && (io->_npcdata->movemode == RUNMODE)
	        && (ause0->cur_anim == alist[ANIM_FIGHT_WALK_FORWARD])
	        && alist[ANIM_RUN])
	{
		AcquireLastAnim(io);
		FinishAnim(io, ause0->cur_anim);
		ANIM_Set(ause0, alist[ANIM_RUN]);
	}

	// Reset WAIT Animation if reached end !
	if ((ause0->cur_anim == alist[ANIM_DEFAULT])
	        && (ause0->cur_anim != NULL)
	        && (ause0->flags & EA_ANIMEND))
	{
		AcquireLastAnim(io);
		FinishAnim(io, ause0->cur_anim);
		ANIM_Set(ause0, alist[ANIM_DEFAULT]);

		if (io->_npcdata->behavior & BEHAVIOUR_FRIENDLY) ause0->altidx_cur = 0;
	}

	// Can only change direction during some specific animations
	if (((ause0->cur_anim == alist[ANIM_DEFAULT]) && (ause0->altidx_cur == 0))
	        || (ause0->cur_anim == alist[ANIM_FIGHT_WAIT])
	        || (ause0->cur_anim == alist[ANIM_FIGHT_WALK_FORWARD])
	        || (ause0->cur_anim == alist[ANIM_WALK])
	        || (ause0->cur_anim == alist[ANIM_WALK_SNEAK])
	        || (ause0->cur_anim == alist[ANIM_RUN])
	        || (ause0->cur_anim == alist[ANIM_FIGHT_STRAFE_LEFT])
	        || (ause0->cur_anim == alist[ANIM_FIGHT_STRAFE_RIGHT])
	        || (ause0->cur_anim == alist[ANIM_FIGHT_WALK_BACKWARD])
	        || (ause0->cur_anim == NULL)
	   )	CHANGE = 1;

	// Tries to face/stare at target
	if ((!ARXPausedTimer) && (CHANGE)
	    && (!(ause0->flags & EA_FORCEPLAY)))
	{
		if (io->_npcdata->behavior & BEHAVIOUR_STARE_AT)
			StareAtTarget(io);
		else	FaceTarget2(io);
	}

	// Choose tolerance value depending on target...
	if ((io->_npcdata->pathfind.listnb > 0)
	        && (io->_npcdata->pathfind.listpos < io->_npcdata->pathfind.listnb))
	{
		ComputeTolerance(io, io->_npcdata->pathfind.list[io->_npcdata->pathfind.listpos], &TOLERANCE);
		ComputeTolerance(io, io->_npcdata->pathfind.truetarget, &TOLERANCE2);
	}
	else
	{
		ComputeTolerance(io, io->targetinfo, &TOLERANCE);
		TOLERANCE2 = TOLERANCE;
	}

	// COLLISION Management START *********************************************************************
	//	EERIE_CYLINDER cyl;
	// Try physics from last valid pos to current desired pos...
	// For this frame we want to try a move from startpos (valid pos)
	// to targetpos (potentially invalid pos)
	io->physics.startpos.x = io->physics.cyl.origin.x = io->pos.x;
	io->physics.startpos.y = io->physics.cyl.origin.y = io->pos.y;
	io->physics.startpos.z = io->physics.cyl.origin.z = io->pos.z;


	EERIE_3D ForcedMove;

	if ((io->forcedmove.x == 0.f) && (io->forcedmove.y == 0.f) && (io->forcedmove.z == 0.f))
		Vector_Init(&ForcedMove);
	else
	{
		EERIE_3D vect;
		Vector_Copy(&vect, &io->forcedmove);
		float d = TRUEVector_Magnitude(&vect);
		TRUEVector_Normalize(&vect);
		float dd = __min(d, (float)FrameDiff * DIV6);
		ForcedMove.x = vect.x * dd;
		ForcedMove.y = vect.y * dd;
		ForcedMove.z = vect.z * dd;
	}

	// Sets Target position to desired position...
	io->physics.targetpos.x = io->pos.x + io->move.x + ForcedMove.x;
	io->physics.targetpos.z = io->pos.z + io->move.z + ForcedMove.z;
	// IO_PHYSICS phys;	// XS : Moved to func beginning
	memcpy(&phys, &io->physics, sizeof(IO_PHYSICS));
	GetIOCyl(io, &phys.cyl);

	long levitate;
	levitate = 0;

	if (ARX_SPELLS_GetSpellOn(io, SPELL_LEVITATE) >= 0)
	{
		levitate = 1;
		io->physics.targetpos.y = io->pos.y + io->move.y + ForcedMove.y;
	}
	else  // Gravity 'simulation'
	{
		phys.cyl.origin.y += 10.f;
		float anything = CheckAnythingInCylinder(&phys.cyl, io, CFLAG_JUST_TEST | CFLAG_NPC);

		if (anything >= 0) 
		{
			io->physics.targetpos.y = io->pos.y + (float)FrameDiff * 1.5f + ForcedMove.y;
		}
		else io->physics.targetpos.y = io->pos.y + ForcedMove.y;

		phys.cyl.origin.y -= 10.f;
	}

	memcpy(&phys, &io->physics, sizeof(IO_PHYSICS));
	GetIOCyl(io, &phys.cyl);

	io->forcedmove.x -= ForcedMove.x;
	io->forcedmove.y -= ForcedMove.y;
	io->forcedmove.z -= ForcedMove.z;

	// Some visual debug stuff
	if (DEBUGNPCMOVE)
	{
		EERIE_CYLINDER cyll;
		cyll.height = GetIOHeight(io); 
		cyll.radius = GetIORadius(io); 
		cyll.origin.x = phys.startpos.x;
		cyll.origin.y = phys.startpos.y;
		cyll.origin.z = phys.startpos.z;
		EERIEDraw3DCylinder(GDevice, &cyll, 0xFF00FF00);

		if (!(AttemptValidCylinderPos(&cyll, io, levitate | CFLAG_NPC)))
		{
			cyll.height = -40.f;
			EERIEDraw3DCylinder(GDevice, &cyll, 0xFF0000FF);
			cyll.height = GetIOHeight(io); 
		}

		cyll.origin.x = io->physics.targetpos.x;
		cyll.origin.y = io->physics.targetpos.y;
		cyll.origin.z = io->physics.targetpos.z;
		EERIEDraw3DCylinder(GDevice, &cyll, 0xFFFF0000);

		if (!(AttemptValidCylinderPos(&cyll, io, levitate | CFLAG_NPC)))
		{
			cyll.height = GetIOHeight(io); 
		}
	}

	APPLY_PUSH = 0; 
	DIRECT_PATH = TRUE;

	// Now we try the physical move for real
	if	(Vector_Compare(&io->physics.startpos, &io->physics.targetpos) // optim
	        ||	(ARX_COLLISION_Move_Cylinder(&phys, io, 40, levitate | CFLAG_NPC)))
	{
		// Successfull move now validate it
		if (!DIRECT_PATH) io->_npcdata->moveproblem += 1;
		else io->_npcdata->moveproblem = 0;

		io->_npcdata->collid_state = 0;

	}
	else // Object was unable to move to target... Stop it
	{
		io->_npcdata->moveproblem += 3;
	}

	io->room_flags |= 1;
	io->physics.cyl.origin.x = io->pos.x = phys.cyl.origin.x;
	io->physics.cyl.origin.y = io->pos.y = phys.cyl.origin.y;
	io->physics.cyl.origin.z = io->pos.z = phys.cyl.origin.z;
	io->physics.cyl.radius = GetIORadius(io);
	io->physics.cyl.height = GetIOHeight(io);


	APPLY_PUSH = 0;

	// Compute distance 2D to target.
	dist = TRUEDistance2D(io->pos.x, io->pos.z, io->target.x, io->target.z);
	dis = dist;

	if (io->_npcdata->pathfind.listnb > 0)
		dis = GetTRUETargetDist(io);

	if (io->_npcdata->behavior & BEHAVIOUR_FLEE)
		dis = 9999999;

	// Tries to solve Moveproblems... sort of...
	if (io->_npcdata->moveproblem > 11) 
	{
		if ((dist > TOLERANCE) && (!io->_npcdata->pathfind.pathwait))
		{
			long targ;

			if (io->_npcdata->pathfind.listnb > 0)
				targ = io->_npcdata->pathfind.truetarget;
			else
				targ = io->targetinfo;

			ARX_NPC_LaunchPathfind(io, targ);
		}

		io->_npcdata->moveproblem = 0;
	}

	// Checks if pathfind final target is still locked on true target
	if ((FRAME_COUNT <= 0)
	        &&	(!io->_npcdata->pathfind.pathwait)
	        && !(io->_npcdata->pathfind.flags & PATHFIND_ONCE)
	        && !(io->_npcdata->pathfind.flags & PATHFIND_NO_UPDATE)
	        &&	(io->_npcdata->pathfind.listnb > 0)
	        &&	(io->_npcdata->pathfind.listpos < io->_npcdata->pathfind.listnb)
	        &&	(io->_npcdata->behavior & BEHAVIOUR_MOVE_TO)
	        && !(io->_npcdata->behavior & BEHAVIOUR_FLEE)
	   )
	{
		if (ValidIONum(io->_npcdata->pathfind.truetarget))
		{
			EERIE_3D * p = &inter.iobj[io->_npcdata->pathfind.truetarget]->pos;
			long t = AnchorData_GetNearest(p, &io->physics.cyl); 

			if ((t != -1) && (t != io->_npcdata->pathfind.list[io->_npcdata->pathfind.listnb-1]))
			{
				float dist = TRUEEEDistance3D(&ACTIVEBKG->anchors[t].pos, &ACTIVEBKG->anchors[io->_npcdata->pathfind.list[io->_npcdata->pathfind.listnb-1]].pos);

				if (dist > 200.f)
					ARX_NPC_LaunchPathfind(io, io->_npcdata->pathfind.truetarget);
			}
		}
	}


	// We are still too far from our target...
	if (io->_npcdata->pathfind.pathwait == 0)
	{
		if ((dist > TOLERANCE) && (dis > TOLERANCE2))
		{
			if ((io->_npcdata->reachedtarget))
			{
				if (ValidIONum(io->targetinfo))
					EVENT_SENDER = inter.iobj[io->targetinfo];
				else
					EVENT_SENDER = NULL;

				SendIOScriptEvent(io, SM_LOSTTARGET, "");
				io->_npcdata->reachedtarget = 0;
			}

			// if not blocked & not Flee-Pathfinding
			if ( !((io->_npcdata->pathfind.listnb <= 0) && (io->_npcdata->behavior & BEHAVIOUR_FLEE))
			    || (io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND)
			    || (io->_npcdata->behavior & BEHAVIOUR_GO_HOME))
			{
				ANIM_HANDLE * desiredanim = NULL;

				//long desiredloop=1;
				if ((dis <= RUN_WALK_RADIUS) && (io->_npcdata->behavior & BEHAVIOUR_FIGHT)
				        && (ause0->cur_anim != alist[ANIM_RUN]))
				{


					float fCalc = io->_npcdata->walk_start_time + FrameDiff ;
					ARX_CHECK_SHORT(fCalc);

					io->_npcdata->walk_start_time = ARX_CLEAN_WARN_CAST_SHORT(fCalc);


					if (io->_npcdata->walk_start_time > 600)
					{
						desiredanim = alist[ANIM_FIGHT_WALK_FORWARD];
						io->_npcdata->walk_start_time = 0;
					}
				}
				else switch (io->_npcdata->movemode)
					{
						case SNEAKMODE:
							desiredanim = alist[ANIM_WALK_SNEAK];
							break;
						case WALKMODE:
							desiredanim = alist[ANIM_WALK];
							break;
						case RUNMODE:

							if (dis <= RUN_WALK_RADIUS)
							{
								desiredanim = alist[ANIM_WALK];
							}
							else
								desiredanim = alist[ANIM_RUN];

							break;
						case NOMOVEMODE:
							desiredanim = alist[ANIM_DEFAULT];
							break;
					}

				if (io->targetinfo == -2) desiredanim = alist[ANIM_DEFAULT];

				if ((desiredanim)
				        && (desiredanim != ause0->cur_anim)
				        && (!(ause0->flags & EA_FORCEPLAY))
				        &&
				        ((ause0->cur_anim == alist[ANIM_DEFAULT])
				         || (ause0->cur_anim == alist[ANIM_FIGHT_WAIT]))
				   )
				{
					AcquireLastAnim(io);
					FinishAnim(io, ause0->cur_anim);
					ANIM_Set(ause0, desiredanim);

					if ((desiredanim == alist[ANIM_DEFAULT])
					        && (io->_npcdata->behavior & BEHAVIOUR_FRIENDLY)) ause0->altidx_cur = 0;

					ause0->flags |= EA_LOOP;
				}
			}
		}
		// Near target
		else
		{
			if (dis <= TOLERANCE2)
			{
				io->_npcdata->pathfind.listpos = 0;
				io->_npcdata->pathfind.listnb = -1;
				io->_npcdata->pathfind.pathwait = 0;

				if (io->_npcdata->pathfind.list) MemFree(io->_npcdata->pathfind.list);

				io->_npcdata->pathfind.list = NULL;

				if	(ause0->cur_anim == alist[ANIM_FIGHT_WALK_FORWARD])
				{
					ause0->flags &= ~EA_LOOP;
					AcquireLastAnim(io);
					FinishAnim(io, ause0->cur_anim);
					ANIM_Set(ause0, alist[ANIM_DEFAULT]);

					if (io->_npcdata->behavior & BEHAVIOUR_FRIENDLY) ause0->altidx_cur = 0;

					ause0->flags |= EA_LOOP;
				}
			}

			if (io->_npcdata->pathfind.listnb > 0)
			{
			argh:;


				long lMax = __max(ARX_NPC_GetNextAttainableNodeIncrement(io), 1L);
				ARX_CHECK_USHORT(io->_npcdata->pathfind.listpos + lMax);

				io->_npcdata->pathfind.listpos	+= ARX_CLEAN_WARN_CAST_USHORT(lMax);


				if ((io->_npcdata->pathfind.listpos >= io->_npcdata->pathfind.listnb)) // || (dis<=120.f))
				{
					io->_npcdata->pathfind.listpos = 0;
					io->_npcdata->pathfind.listnb = -1;
					io->_npcdata->pathfind.pathwait = 0;

					if (io->_npcdata->pathfind.list) MemFree(io->_npcdata->pathfind.list);

					io->_npcdata->pathfind.list = NULL;
					EVENT_SENDER = NULL;

					if ((io->_npcdata->behavior & BEHAVIOUR_FLEE)
					        && (!io->_npcdata->pathfind.pathwait))
						SendIOScriptEvent(io, 0, "", "FLEE_END");

					if ((io->_npcdata->pathfind.flags & PATHFIND_NO_UPDATE) &&
					        (io->_npcdata->pathfind.pathwait == 0))
					{
						if (!io->_npcdata->reachedtarget)
						{
							long num = GetInterNum(io);
							io->_npcdata->reachedtarget = 1;
							io->_npcdata->reachedtime = ARXTimeUL();

							if (io->targetinfo != num)
							{
								SendIOScriptEvent(io, SM_REACHEDTARGET, "FAKE");
								io->targetinfo = num;
							}
						}

					}
					else
					{
						io->targetinfo = io->_npcdata->pathfind.truetarget;
						GetTargetPos(io);

						if (fabs(io->pos.y - io->target.y) > 200.f)
						{
							io->_npcdata->pathfind.listnb = -2;
						}
					}
				}
			}
			else if (!io->_npcdata->reachedtarget) 
			{
				if (ValidIONum(io->targetinfo))
					EVENT_SENDER = inter.iobj[io->targetinfo];
				else
					EVENT_SENDER = NULL;

				io->_npcdata->reachedtarget = 1;
				io->_npcdata->reachedtime = ARXTimeUL();//treat warning C4244 conversion from 'float' to 'unsigned long'

				if (io->animlayer[1].flags & EA_ANIMEND)
					io->animlayer[1].cur_anim = NULL;

				if (io->targetinfo != GetInterNum(io))
					SendIOScriptEvent(io, SM_REACHEDTARGET, "");
			}
		}
	}

	if (dis < 280.f)
	{
		if ((io->_npcdata->behavior & BEHAVIOUR_FIGHT)
		        && !(io->_npcdata->behavior & BEHAVIOUR_FLEE))
		{
			ARX_NPC_Manage_Fight(io);
		}
		else
		{
			ARX_NPC_Manage_NON_Fight(io);
		}
	}

	ARX_NPC_Manage_Anims(io, TOLERANCE2);

	// Puts at least WAIT anim on NPC if he has no main animation...
	if (ause0->cur_anim == NULL)
	{
		if (io->_npcdata->behavior & (BEHAVIOUR_FIGHT | BEHAVIOUR_MAGIC | BEHAVIOUR_DISTANT))
		{
			FinishAnim(io, ause0->cur_anim);
			ANIM_Set(ause0, alist[ANIM_FIGHT_WAIT]);
			ause0->flags |= EA_LOOP;
		}
		else
		{
			FinishAnim(io, ause0->cur_anim);
			ANIM_Set(ause0, alist[ANIM_WAIT]);

			if (io->_npcdata->behavior & BEHAVIOUR_FRIENDLY) ause0->altidx_cur = 0;
		}
	}

	// Now update lastpos values for next call use...
	io->lastpos.x = io->pos.x;
	io->lastpos.y = io->pos.y;
	io->lastpos.z = io->pos.z;
}

float AngularDifference(float a1, float a2)
{
	a1 = MAKEANGLE(a1);
	a2 = MAKEANGLE(a2);

	if (a1 == a2) return 0;

	float ret;

	if (a1 < a2)
	{
		ret = a2;
		a2 = a1;
		a1 = ret;
	}

	ret = a1 - a2;
	ret = __min(ret, (a2 + 360) - a1);
	return ret;

}
extern float CURRENT_PLAYER_COLOR;
//***********************************************************************************************
// INTERACTIVE_OBJ * ARX_NPC_GetFirstNPCInSight(INTERACTIVE_OBJ * ioo)
//-----------------------------------------------------------------------------------------------
// FUNCTION:
//   returns the "first" NPC in sight for another NPC (ioo)
//***********************************************************************************************
INTERACTIVE_OBJ * ARX_NPC_GetFirstNPCInSight(INTERACTIVE_OBJ * ioo)
{
	if (!ioo) return NULL;

	// Basic Clipping to avoid performance loss
	float dist_cam = EEDistance3D(&ACTIVECAM->pos, &ioo->pos);

	if (dist_cam > 2500) return NULL;

	INTERACTIVE_OBJ * found_io = NULL;
	float found_dist = 9999999999999.f;

	for (long i = 0; i < inter.nbmax; i++)
	{
		INTERACTIVE_OBJ * io = inter.iobj[i];

		if ((!io)
		        ||	(IsDeadNPC(io))
		        ||	(io == ioo)
		        ||	(!(io->ioflags & IO_NPC))
		        ||	(io->show != SHOW_FLAG_IN_SCENE))
			continue;

		float dist_io = EEDistance3D(&io->pos, &ioo->pos);

		if ((dist_io > found_dist)
		        ||	(dist_io > 1800))
			continue; // too far

		if (dist_io < 130)
		{
			if (found_dist > dist_io)
			{
				found_io = io;
				found_dist = dist_io;
			}

			continue;
		}

		EERIE_3D orgn, dest;

		float ab = MAKEANGLE(ioo->angle.b);

		long grp = ioo->obj->fastaccess.head_group_origin;

		if (grp < 0)
		{
			orgn.x = ioo->pos.x;
			orgn.y = ioo->pos.y - 90.f;
			orgn.z = ioo->pos.z;

			if (ioo == inter.iobj[0])	orgn.y = player.pos.y + 90.f;
		}
		else
			GetVertexPos(ioo, ioo->obj->fastaccess.head_group_origin, &orgn);

		grp = io->obj->fastaccess.head_group_origin;

		if (grp < 0)
		{
			dest.x = io->pos.x;
			dest.y = io->pos.y - 90.f;
			dest.z = io->pos.z;

			if (io == inter.iobj[0])	dest.y = player.pos.y + 90.f;
		}
		else
			GetVertexPos(io, io->obj->fastaccess.head_group_origin, &dest);


		float aa = GetAngle(orgn.x, orgn.z, dest.x, dest.z);
		aa = MAKEANGLE(RAD2DEG(aa));

		if (EEfabs(AngularDifference(aa, ab)) < 110.f)
		{
			if (dist_io < 200)
			{
				if (found_dist > dist_io)
				{
					found_io = io;
					found_dist = dist_io;
				}

				continue;
			}

			
			float grnd_color = CURRENT_PLAYER_COLOR - GetPlayerStealth(); 

			if (grnd_color > 0) 
			{
				EERIE_3D ppos;
				EERIEPOLY * epp = NULL;

				if (IO_Visible(&orgn, &dest, epp, &ppos))
				{
					if (found_dist > dist_io)
					{
						found_io = io;
						found_dist = dist_io;
					}

					continue;
				}
				else if (EEDistance3D(&ppos, &dest) < 25.f)
				{
					if (found_dist > dist_io)
					{
						found_io = io;
						found_dist = dist_io;
					}

					continue;
				}
			}
		}
	}

	return found_io;
}
extern float CURRENT_PLAYER_COLOR;
 
//***********************************************************************************************
// void CheckNPC(INTERACTIVE_OBJ * io)
//-----------------------------------------------------------------------------------------------
// FUNCTION
//   Checks if a NPC is dead to prevent further Layers Animation
//***********************************************************************************************
void CheckNPC(INTERACTIVE_OBJ * io)
{
	if ((!io)
	        ||	(io->show != SHOW_FLAG_IN_SCENE))
		return;

	if (IsDeadNPC(io))
	{
		io->animlayer[1].cur_anim = NULL;
		io->animlayer[2].cur_anim = NULL;
		io->animlayer[3].cur_anim = NULL;

		io->animlayer[0].next_anim = NULL;
		io->animlayer[1].next_anim = NULL;
		io->animlayer[2].next_anim = NULL;
		io->animlayer[3].next_anim = NULL;
	}
}
//***********************************************************************************************
// void CheckUnderWaterIO(INTERACTIVE_OBJ * io)
//-----------------------------------------------------------------------------------------------
// FUNCTION:
//   Checks if the bottom of an IO is underwater.
// RESULT:
//   Plays Water sounds
//   Decrease/stops Ignition of this IO if necessary
//-----------------------------------------------------------------------------------------------
// WARNINGS:
// io must be valid (no check !)
//***********************************************************************************************
void CheckUnderWaterIO(INTERACTIVE_OBJ * io)
{
	EERIE_3D ppos;
	ppos.x = io->pos.x;
	ppos.y = io->pos.y;
	ppos.z = io->pos.z;
	EERIEPOLY * ep = EEIsUnderWater(&ppos);

	if (io->ioflags & IO_UNDERWATER)
	{
		if (!ep)
		{
			io->ioflags &= ~IO_UNDERWATER;
			ARX_SOUND_PlaySFX(SND_PLOUF, &ppos);
			ARX_PARTICLES_SpawnWaterSplash(&ppos);
		}
	}
	else if (ep)
	{
		io->ioflags |= IO_UNDERWATER;
		ARX_SOUND_PlaySFX(SND_PLOUF, &ppos);
		ARX_PARTICLES_SpawnWaterSplash(&ppos);

		if (io->ignition > 0.f)
		{
			ARX_SOUND_PlaySFX(SND_TORCH_END, &ppos);

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
	}
}
extern long GLOBAL_Player_Room;
//***********************************************************************************************
// void CheckNPCEx(INTERACTIVE_OBJ * io)
// ----------------------------------------------------------------------------------------------
// FUNCTION:
//   Checks an NPC Visibility Field (Player Detect)
// NECESSARY:
//   Uses Invisibility/Confuse/Torch infos.
// RESULT:
//   Sends appropriate Detectplayer/Undetectplayer events to the IO
//-----------------------------------------------------------------------------------------------
// WARNINGS:
//   io and io->obj must be valid (no check !)
//***********************************************************************************************
void CheckNPCEx(INTERACTIVE_OBJ * io)
{
	// Distance Between Player and IO
	float dist = Distance3D(io->pos.x, io->pos.y, io->pos.z, player.pos.x, player.pos.y - PLAYER_BASE_HEIGHT, player.pos.z);

	// Start as not visible
	long Visible = 0;

	// Check visibility only if player is visible, not too far and not dead
	if ((!(inter.iobj[0]->invisibility > 0.f)) && (dist < 2000.f) && (player.life > 0))
	{
		// checks for near contact +/- 15 cm --> force visibility
		if (io->room_flags & 1)
			UpdateIORoom(io);

		if (GLOBAL_Player_Room == -1)
			GLOBAL_Player_Room = ARX_PORTALS_GetRoomNumForPosition(&player.pos, 1);

		float fdist = SP_GetRoomDist(&io->pos, &player.pos, io->room, GLOBAL_Player_Room);

		// Use Portal Room Distance for Extra Visibility Clipping.
		if ((GLOBAL_Player_Room > -1) && (io->room > -1) && (fdist > 2000.f))
		{
		}
		else if ((dist < GetIORadius(io) + GetIORadius(inter.iobj[0]) + 15.f)
		         && (EEfabs(player.pos.y - io->pos.y) < 200.f))
		{
			Visible = 1;
		}
		else // Make full visibility test
		{
			EERIE_3D orgn, dest;
			// Retreives Head group position for "eye" pos.
			long grp = io->obj->fastaccess.head_group_origin; 

			if (grp < 0)
			{
				orgn.x = io->pos.x;
				orgn.y = io->pos.y - 90.f;
				orgn.z = io->pos.z;
			}
			else
			{
				orgn.x = io->pos.x;
				orgn.y = io->pos.y - 120.f;
				orgn.z = io->pos.z;
			}

			dest.x = player.pos.x;
			dest.y = player.pos.y + 90.f;
			dest.z = player.pos.z;

			// Check for Field of vision angle
			float aa = GetAngle(orgn.x, orgn.z, dest.x, dest.z);
			aa = MAKEANGLE(RAD2DEG(aa));
			float ab = MAKEANGLE(io->angle.b);

			if (EEfabs(AngularDifference(aa, ab)) < 110.f)
			{
				// Check for Darkness/Stealth
				if ((CURRENT_PLAYER_COLOR > GetPlayerStealth())
				        ||	SHOW_TORCH
				        ||	(dist < 200))
				{
					EERIEPOLY * epp = NULL;
					EERIE_3D ppos;

					// Check for Geometrical Visibility
					if ((IO_Visible(&orgn, &dest, epp, &ppos))
					        ||	(EEDistance3D(&ppos, &dest) < 25.f))
						Visible = 1;
				}
			}
		}

		if (Visible)
		{
			if (!io->_npcdata->detect)
			{
				// if visible but was NOT visible, sends an Detectplayer Event
				EVENT_SENDER = NULL;
				SendIOScriptEvent(io, SM_DETECTPLAYER, "");
				io->_npcdata->detect = 1;
			}
		}
	}

	// if not visible but was visible, sends an Undetectplayer Event
	if ((!Visible) && (io->_npcdata->detect))
	{
		EVENT_SENDER = NULL;
		SendIOScriptEvent(io, SM_UNDETECTPLAYER, "");
		io->_npcdata->detect = 0;
	}
}

//-------------------------------------------------------------------------
void GetMaterialString(char * origin, char * dest)
{
	// need to be precomputed !!!
	if (IsIn(origin, "STONE"))	strcpy(dest, "STONE");
	else if (IsIn(origin, "MARBLE"))	strcpy(dest, "STONE");
	else if (IsIn(origin, "ROCK"))	strcpy(dest, "STONE");
	else if (IsIn(origin, "WOOD"))	strcpy(dest, "WOOD");
	else if (IsIn(origin, "WET"))	strcpy(dest, "WET");
	else if (IsIn(origin, "MUD"))	strcpy(dest, "WET");
	else if (IsIn(origin, "BLOOD"))	strcpy(dest, "WET");
	else if (IsIn(origin, "BONE"))	strcpy(dest, "WET");
	else if (IsIn(origin, "FLESH"))	strcpy(dest, "WET");
	else if (IsIn(origin, "SHIT"))	strcpy(dest, "WET");
	else if (IsIn(origin, "SOIL"))	strcpy(dest, "GRAVEL");
	else if (IsIn(origin, "GRAVEL"))	strcpy(dest, "GRAVEL");
	else if (IsIn(origin, "EARTH"))	strcpy(dest, "GRAVEL");
	else if (IsIn(origin, "DUST"))	strcpy(dest, "GRAVEL");
	else if (IsIn(origin, "SAND"))	strcpy(dest, "GRAVEL");
	else if (IsIn(origin, "STRAW"))	strcpy(dest, "GRAVEL");
	else if (IsIn(origin, "METAL"))	strcpy(dest, "METAL");
	else if (IsIn(origin, "IRON"))	strcpy(dest, "METAL");
	else if (IsIn(origin, "GLASS"))	strcpy(dest, "METAL");
	else if (IsIn(origin, "RUST"))	strcpy(dest, "METAL");
	else if (IsIn(origin, "EARTH"))	strcpy(dest, "EARTH");
	else if (IsIn(origin, "ICE"))	strcpy(dest, "ICE");
	else if (IsIn(origin, "FABRIC")) strcpy(dest, "CARPET");
	else if (IsIn(origin, "MOSS"))	strcpy(dest, "CARPET");
	else strcpy(dest, "UNKNOWN");
}
//-------------------------------------------------------------------------
void ARX_NPC_NeedStepSound(INTERACTIVE_OBJ * io, EERIE_3D * pos, const float volume, const float power)
{
	char step_material[64] = "Foot_bare";
	char floor_material[64] = "EARTH";

	if (IsUnderWater(pos->x, pos->y, pos->z))
		strcpy(floor_material, "WATER");
	else
	{
		EERIEPOLY * ep;
		ep = CheckInPoly(pos->x, pos->y - 100.0F, pos->z);

		if (ep &&  ep->tex && ep->tex->m_texName)
			GetMaterialString(ep->tex->m_texName, floor_material);
	}

	if (io && io->stepmaterial)
		strcpy(step_material, io->stepmaterial);

	if ((io == inter.iobj[0]) && (player.equiped[EQUIP_SLOT_LEGGINGS] > 0))
	{
		if (ValidIONum(player.equiped[EQUIP_SLOT_LEGGINGS]))
		{
			INTERACTIVE_OBJ * ioo = inter.iobj[player.equiped[EQUIP_SLOT_LEGGINGS]];

			if (ioo->stepmaterial)
				strcpy(step_material, ioo->stepmaterial);
		}
	}

	ARX_SOUND_PlayCollision(step_material, floor_material, volume, power, pos, io);
}
//-------------------------------------------------------------------------
//***********************************************************************************************
// ARX_NPC_SpawnAudibleSound
// Sends ON HEAR events to NPCs for audible sounds
// factor > 1.0F harder to hear, < 0.0F easier to hear
//***********************************************************************************************
void ARX_NPC_SpawnAudibleSound(EERIE_3D * pos, INTERACTIVE_OBJ * source, const float factor, const float presence)
{
	float max_distance;

	if (source == inter.iobj[0])
		max_distance = ARX_NPC_ON_HEAR_MAX_DISTANCE_STEP;
	else if (source && source->ioflags & IO_ITEM)
		max_distance = ARX_NPC_ON_HEAR_MAX_DISTANCE_ITEM;
	else return;

	max_distance *= presence;
	max_distance /= factor;

	EVENT_SENDER = source;


	long Source_Room = ARX_PORTALS_GetRoomNumForPosition(pos, 1);


	for (long i = 0; i < inter.nbmax; i++)
		if ((inter.iobj[i])
		        &&	(inter.iobj[i]->ioflags & IO_NPC)
		        &&	(inter.iobj[i]->GameFlags & GFLAG_ISINTREATZONE)
		        &&	(inter.iobj[i] != source)
		        &&	((inter.iobj[i]->show == SHOW_FLAG_IN_SCENE)
		             ||	(inter.iobj[i]->show == SHOW_FLAG_HIDDEN))
		        &&	(inter.iobj[i]->_npcdata->life > 0.f)
		   )
		{
			float distance(EEDistance3D(pos, &inter.iobj[i]->pos));

			if (distance < max_distance)
			{
				if (inter.iobj[i]->room_flags & 1)
					UpdateIORoom(inter.iobj[i]);

				float fdist;

				if ((Source_Room > -1) && (inter.iobj[i]->room > -1))
				{
					fdist = SP_GetRoomDist(pos, &inter.iobj[i]->pos, Source_Room, inter.iobj[i]->room);

					if (fdist < max_distance * 1.5f)
					{
						long ldistance;
						char temp[64];

						F2L(fdist, &ldistance);
						sprintf(temp, "%d", ldistance);

						SendIOScriptEvent(inter.iobj[i], SM_HEAR, temp);
					}
				}
				else
				{
					long ldistance;
					char temp[64];

					F2L(distance, &ldistance);
					sprintf(temp, "%d", ldistance);

					SendIOScriptEvent(inter.iobj[i], SM_HEAR, temp);
				}
			}
		}
}
extern INTERACTIVE_OBJ * CURRENT_TORCH;
//-------------------------------------------------------------------------
void ManageIgnition(INTERACTIVE_OBJ * io)
{
	if (!io) return;

	if (CURRENT_TORCH == io)
	{
		if (ValidDynLight(io->ignit_light))
			DynLight[io->ignit_light].exist = 0;

		io->ignit_light = -1;

		if (io->ignit_sound != ARX_SOUND_INVALID_RESOURCE)
		{
			ARX_SOUND_Stop(io->ignit_sound);
			io->ignit_sound = ARX_SOUND_INVALID_RESOURCE;
		}

		return;
	}

	// Torch Management
	INTERACTIVE_OBJ * plw = NULL;

	if ((player.equiped[EQUIP_SLOT_WEAPON] != 0)
	        &&	(ValidIONum(player.equiped[EQUIP_SLOT_WEAPON])))
		plw = inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]];

	if ((io->ioflags & IO_FIERY)
	        &&	(!(io->type_flags & OBJECT_TYPE_BOW))
	        && ((io->show == SHOW_FLAG_IN_SCENE) || (io == plw)))
	{
		float p = io->ignition = 25.f;

		while (p > 0.f)
		{
			p -= 6.f;

			if ((io) && (io->obj) && (io->obj->nbfaces))
			{
				EERIE_3D	pos;
				long		notok	=	10;
				long		num		=	0;

				while (notok-- > 0)
				{
					F2L((float)(rnd() *(float)io->obj->nbfaces), &num);

					if ((num >= 0) && (num < io->obj->nbfaces))
					{
						if (io->obj->facelist[num].facetype & POLY_HIDE) continue;

						notok = -1;
					}
				}

				if (notok < 0)
				{
					Vector_Copy(&pos, &io->obj->vertexlist3[io->obj->facelist[num].vid[0]].v);

					for (long nn = 0 ; nn < 1 ; nn++)
					{
						long j = ARX_PARTICLES_GetFree();

						if ((j != -1) && (!ARXPausedTimer) && (rnd() < 0.4f))
						{
							ParticleCount++;
							PARTICLE_DEF * pd	=	&particle[j];
							pd->exist			=	TRUE;
							pd->zdec			=	0;
							Vector_Copy(&pd->ov, &pos);
							pd->move.x			=	(2.f - 4.f * rnd());
							pd->move.y			=	(2.f - 22.f * rnd());
							pd->move.z			=	(2.f - 4.f * rnd());
							pd->siz				=	7.f;
							pd->tolive			=	500 + (unsigned long)(rnd() * 1000.f);
							pd->special			=	FIRE_TO_SMOKE | ROTATING | MODULATE_ROTATION;
							pd->tc				=	fire2;//tc;
							pd->fparam			=	0.1f - rnd() * 0.2f;
							pd->scale.x			=	-8.f;
							pd->scale.y			=	-8.f;
							pd->scale.z			=	-8.f;
							pd->timcreation		=	lARXTime;
							pd->r				=	0.71f;
							pd->g				=	0.43f;
							pd->b				=	0.29f;
							//pd->delay=nn*180;
						}
					}

				}
			}
		}
	}
	else if (io->obj && (io->obj->fastaccess.fire >= 0) && (io->ignition > 0.f))
	{
		io->ignition = 25.f;
		io->durability -= FrameDiff * DIV10000;

		if (io->durability <= 0.F)
		{
			if (ValidDynLight(io->ignit_light))
				DynLight[io->ignit_light].exist = 0;

			io->ignit_light = -1;

			if (io->ignit_sound != ARX_SOUND_INVALID_RESOURCE)
			{
				ARX_SOUND_Stop(io->ignit_sound);
				io->ignit_sound = ARX_SOUND_INVALID_RESOURCE;
			}

			ARX_SPEECH_ReleaseIOSpeech(io);
			// Need To Kill timers
			ARX_SCRIPT_Timer_Clear_By_IO(io);
			io->show = SHOW_FLAG_KILLED;
			io->GameFlags &= ~GFLAG_ISINTREATZONE;
			RemoveFromAllInventories(io);
			ARX_INTERACTIVE_DestroyDynamicInfo(io);
			ARX_SOUND_PlaySFX(SND_TORCH_END, &io->pos);

			if (io == DRAGINTER)
				Set_DragInter(NULL);

			ARX_INTERACTIVE_DestroyIO(io);
			return;
		}

		EERIE_3D pos;
		Vector_Copy(&pos, &io->obj->vertexlist3[io->obj->fastaccess.fire].v);

		for (long nn = 0; nn < 2; nn++)
		{
			long j = ARX_PARTICLES_GetFree();

			if ((j != -1) && (!ARXPausedTimer) && (rnd() < 0.4f))
			{
				ParticleCount++;
				PARTICLE_DEF * pd	=	&particle[j];
				pd->exist		=	TRUE;
				pd->zdec		=	0;
				Vector_Copy(&pd->ov, &pos);
				pd->move.x		=	(2.f - 4.f * rnd());
				pd->move.y		=	(2.f - 22.f * rnd());
				pd->move.z		=	(2.f - 4.f * rnd());
				pd->siz			=	7.f;
				pd->tolive		=	500 + (unsigned long)(rnd() * 1000.f);
				pd->special		=	FIRE_TO_SMOKE | ROTATING | MODULATE_ROTATION;
				pd->tc			=	fire2;
				pd->fparam		=	0.1f - rnd() * 0.2f;
				pd->scale.x		=	-8.f;
				pd->scale.y		=	-8.f;
				pd->scale.z		=	-8.f;
				pd->timcreation	=	lARXTime;
				pd->r			=	0.71f;
				pd->g			=	0.43f;
				pd->b			=	0.29f;
				pd->delay		=	nn * 2;
			}
		}
	}
	else
	{
		io->ignition -= _framedelay * DIV100;

		if ((!io) || (!io->obj)) return;

		float p = io->ignition * _framedelay * DIV1000 * io->obj->nbfaces * DIV1000;

		if (p > 5.f)
			p = 5.f;

		while (p > 0.f)
		{
			p -= 0.5f;

			if ((io) && (io->obj) && (io->obj->nbfaces))
			{
				EERIE_3D	pos;
				long		notok	=	10;
				long		num		=	0;

				while (notok-- > 0)
				{
					F2L((float)(rnd() *(float)io->obj->nbfaces), &num);

					if ((num >= 0) && (num < io->obj->nbfaces))
					{
						if (io->obj->facelist[num].facetype & POLY_HIDE) continue;

						notok = -1;
					}
				}

				if (notok < 0)
				{
					Vector_Copy(&pos, &io->obj->vertexlist3[io->obj->facelist[num].vid[0]].v);

					for (long nn = 0 ; nn < 6 ; nn++)
					{
						long j = ARX_PARTICLES_GetFree();

						if ((j != -1) && (!ARXPausedTimer) && (rnd() < 0.4f))
						{
							ParticleCount++;
							PARTICLE_DEF * pd	=	&particle[j];
							pd->exist			=	TRUE;
							pd->zdec			=	0;

							Vector_Copy(&pd->ov, &pos);

							pd->move.x			=	(2.f - 4.f * rnd());
							pd->move.y			=	(2.f - 22.f * rnd());
							pd->move.z			=	(2.f - 4.f * rnd());
							pd->siz				=	7.f;
							pd->tolive			=	500 + (unsigned long)(rnd() * 1000.f);
							pd->special			=	FIRE_TO_SMOKE | ROTATING | MODULATE_ROTATION;
							pd->tc				=	fire2;//tc;
							pd->fparam			=	0.1f - rnd() * 0.2f;
							pd->scale.x			=	-8.f;
							pd->scale.y			=	-8.f;
							pd->scale.z			=	-8.f;
							pd->timcreation		=	lARXTime;
							pd->r				=	0.71f;
							pd->g				=	0.43f;
							pd->b				=	0.29f;
							pd->delay			=	nn * 180;
						}
					}

				}
			}
		}
	}

	ManageIgnition_2(io);
}
//-------------------------------------------------------------------------
void ManageIgnition_2(INTERACTIVE_OBJ * io)
{
	if (!io) return;

	if (io->ignition > 0.f)
	{
		if (io->ignition > 100.f)
			io->ignition = 100.f;

		EERIE_3D position;

		if (io->obj && (io->obj->fastaccess.fire >= 0))
		{
			if (io == DRAGINTER)
				Vector_Copy(&position, &player.pos);
			else
			{
				Vector_Copy(&position, &io->obj->vertexlist3[io->obj->fastaccess.fire].v);
			}
		}
		else
		{
			Vector_Copy(&position, &io->pos);
		}

		if (io->ignit_light == -1)
			io->ignit_light = GetFreeDynLight();

		if (io->ignit_light != -1)
		{
			long id = io->ignit_light;
			DynLight[id].exist = 1;

			DynLight[id].intensity = __max(io->ignition * DIV10, 1.f);
			DynLight[id].fallstart = __max(io->ignition * 10.f, 100.f);
			DynLight[id].fallend   = __max(io->ignition * 25.f, 240.f);
			float v = __max((io->ignition * DIV10), 0.5f);
			v = __min(v, 1.f);
			DynLight[id].rgb.r = (1.f - rnd() * 0.2f) * v;
			DynLight[id].rgb.g = (0.8f - rnd() * 0.2f) * v;
			DynLight[id].rgb.b = (0.6f - rnd() * 0.2f) * v;
			DynLight[id].pos.x = position.x;
			DynLight[id].pos.y = position.y - 30.f;
			DynLight[id].pos.z = position.z;
			DynLight[id].ex_flaresize = 40.f; //16.f;
			DynLight[id].extras |= EXTRAS_FLARE;
		}

		if (io->ignit_sound == ARX_SOUND_INVALID_RESOURCE)
		{
			io->ignit_sound = SND_FIREPLACE;
			ARX_SOUND_PlaySFX(io->ignit_sound, &position, 0.95F + 0.1F * rnd(), ARX_SOUND_PLAY_LOOPED);
		}
		else ARX_SOUND_RefreshPosition(io->ignit_sound, &position);

		if (rnd() > 0.9f) CheckForIgnition(&position, io->ignition, 1);
	}
	else
	{
		if (ValidDynLight(io->ignit_light))
			DynLight[io->ignit_light].exist = 0;

		io->ignit_light = -1;

		if (io->ignit_sound != ARX_SOUND_INVALID_RESOURCE)
		{
			ARX_SOUND_Stop(io->ignit_sound);
			io->ignit_sound = ARX_SOUND_INVALID_RESOURCE;
		}
	}
}
//-------------------------------------------------------------------------
