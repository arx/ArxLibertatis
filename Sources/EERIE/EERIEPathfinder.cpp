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
///////////////////////////////////////////////////////////////////////////////
// EERIEPathfinder
///////////////////////////////////////////////////////////////////////////////
//
// Description:
//	Interface betweed EERIE & MINOS
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
///////////////////////////////////////////////////////////////////////////////
#include "EERIEPathfinder.h"
#include "EERIELight.h"

#include "HERMESMain.h"
#include "MINOS_PathFinder.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

static const float PATHFINDER_HEURISTIC_MIN(0.2F);
static const float PATHFINDER_HEURISTIC_MAX(MINOS_HEURISTIC_MAX);
static const float PATHFINDER_HEURISTIC_RANGE(PATHFINDER_HEURISTIC_MAX - PATHFINDER_HEURISTIC_MIN);
static const float PATHFINDER_DISTANCE_MAX(5000.0F);

// Pathfinder Definitions
static HANDLE PATHFINDER(NULL);
static HANDLE PATHFINDER_MUTEX(NULL);
static unsigned long PATHFINDER_UPDATE_INTERVAL(10);
static unsigned long PATHFINDER_MUTEX_WAIT(5000);
static unsigned long PATHFINDER_RELEASE_WAIT(5000);
static bool bExitPathfinderThread(false);

PATHFINDER_REQUEST pr;
long PATHFINDER_WORKING(0);

typedef struct _PATHFINDER_QUEUE_ELEMENT
{
	PATHFINDER_REQUEST req;
	_PATHFINDER_QUEUE_ELEMENT * next;
	long valid;
} PATHFINDER_QUEUE_ELEMENT;

PATHFINDER_QUEUE_ELEMENT * pathfinder_queue_start = NULL;

// An Io can request Pathfinding only once so we insure that it's always the case.
// A new pathfinder request from the same IO will overwrite the precedent.
static PATHFINDER_QUEUE_ELEMENT * PATHFINDER_Find_ioid(INTERACTIVE_OBJ * io)
{
	if (!pathfinder_queue_start) return NULL;

	PATHFINDER_QUEUE_ELEMENT * cur = pathfinder_queue_start;

	while (cur)
	{
		if (cur->req.ioid == io) return cur->valid ? cur : NULL;

		cur = cur->next;
	}

	return NULL;
}
INTERACTIVE_OBJ * CURPATHFINDIO = NULL;

// Adds a Pathfinder Search Element to the pathfinder queue.
bool EERIE_PATHFINDER_Add_To_Queue(PATHFINDER_REQUEST * req)
{
	if (!PATHFINDER) return false;

	if (WaitForSingleObject(PATHFINDER_MUTEX, PATHFINDER_MUTEX_WAIT) == WAIT_TIMEOUT)
		return false;

	PATHFINDER_QUEUE_ELEMENT * cur = pathfinder_queue_start;

	PATHFINDER_QUEUE_ELEMENT * temp;

	// If this NPC is already requesting a Pathfinding then either
	// try to Override it or add it to queue if it is currently being
	// processed.
	temp = PATHFINDER_Find_ioid(req->ioid);

	if (temp && temp->valid && temp != pathfinder_queue_start)
	{
		temp->valid = 0;
		memcpy(&temp->req, req, sizeof(PATHFINDER_REQUEST));
		temp->valid = 1;

		ReleaseMutex(PATHFINDER_MUTEX);
		return true;
	}

	// Create a New element for the queue
	temp = (PATHFINDER_QUEUE_ELEMENT *)malloc(sizeof(PATHFINDER_QUEUE_ELEMENT));

	if (!temp)
	{
		ReleaseMutex(PATHFINDER_MUTEX);
		return false;
	}

	// Fill this New element with new request
	memcpy(&temp->req, req, sizeof(PATHFINDER_REQUEST));
	temp->valid = 1;

	// No queue start ? then this element becomes the queue start
	if (!cur)
	{
		temp->next = NULL;
		pathfinder_queue_start = temp;
	}
	else if (req->ioid->_npcdata->behavior & (BEHAVIOUR_MOVE_TO | BEHAVIOUR_FLEE | BEHAVIOUR_LOOK_FOR) && cur->next)
	{
		// priority: insert as second element of queue
		temp->next = cur->next;
		cur->next = temp;
	}
	else
	{
		// add to end of queue
		temp->next = NULL;

		if (!pathfinder_queue_start)
		{
			pathfinder_queue_start = temp;
			ReleaseMutex(PATHFINDER_MUTEX);
			return true;
		}
		else					
		{
			while (cur->next)		
			{
			
				cur = cur->next;		
			}					

			cur->next = temp;		
		}							
	}

	ReleaseMutex(PATHFINDER_MUTEX);
	return true;
}

long EERIE_PATHFINDER_Get_Queued_Number()
{
	if (WaitForSingleObject(PATHFINDER_MUTEX, PATHFINDER_MUTEX_WAIT) == WAIT_TIMEOUT)
		return -1;

	PATHFINDER_QUEUE_ELEMENT * cur = pathfinder_queue_start;

	long count = 0;

	while (cur) cur = cur->next, count++;

	ReleaseMutex(PATHFINDER_MUTEX);

	return count;
}

void EERIE_PATHFINDER_Clear(long flag)
{
	if (!flag)
	{
		if (PATHFINDER_MUTEX)
			if (WaitForSingleObject(PATHFINDER_MUTEX, PATHFINDER_MUTEX_WAIT) == WAIT_TIMEOUT)
				return;
	}

	CURPATHFINDIO = NULL;

	PATHFINDER_QUEUE_ELEMENT * cur = pathfinder_queue_start;
	PATHFINDER_QUEUE_ELEMENT * next;

	while (cur)
	{
		next = cur->next;
		free(cur);
		cur = next;
	}

	pathfinder_queue_start = NULL;

	if (!flag && PATHFINDER_MUTEX) ReleaseMutex(PATHFINDER_MUTEX);
}

// Retrieves & Removes next Pathfind request from queue
static BOOL EERIE_PATHFINDER_Get_Next_Request(PATHFINDER_REQUEST * request)
{
	if (!request || !pathfinder_queue_start || !pathfinder_queue_start->valid)
		return FALSE;

	PATHFINDER_QUEUE_ELEMENT * cur = pathfinder_queue_start;

	if ((cur->req.ioid)
	        &&	(cur->req.ioid->ioflags & IO_NPC)
	        &&	(cur->req.ioid->_npcdata->behavior == BEHAVIOUR_NONE))
	{
		pathfinder_queue_start = cur->next;
		free(cur);
		return FALSE;
	}

	memcpy(request, &cur->req, sizeof(PATHFINDER_REQUEST));
	pathfinder_queue_start = cur->next;
	free(cur);

	return TRUE;
}
LARGE_INTEGER Pstart_chrono, Pend_chrono;
unsigned long BENCH_PATHFINDER = 0;

// Pathfinder Thread
LPTHREAD_START_ROUTINE PATHFINDER_Proc(char *)
{
	EERIE_BACKGROUND * eb = ACTIVEBKG;
	PathFinder pathfinder(eb->nbanchors, eb->anchors,
	                      MAX_LIGHTS, (EERIE_LIGHT **)GLight,
	                      MAX_DYNLIGHTS, (EERIE_LIGHT **)PDL);

	bExitPathfinderThread = false;

	while (!bExitPathfinderThread)
	{
		QueryPerformanceCounter(&Pstart_chrono);

		if (WaitForSingleObject(PATHFINDER_MUTEX, PATHFINDER_MUTEX_WAIT) == WAIT_TIMEOUT)
			continue;

		PATHFINDER_WORKING = 1;

		if (EERIE_PATHFINDER_Get_Next_Request(&pr) && pr.isvalid)
		{

			PATHFINDER_REQUEST curpr;
			memcpy(&curpr, &pr, sizeof(PATHFINDER_REQUEST));
			CURPATHFINDIO = curpr.ioid;
			PATHFINDER_WORKING = 2;

			if (CURPATHFINDIO->ident == 43)
				CURPATHFINDIO->ident = 43;

			if (curpr.ioid && curpr.ioid->_npcdata)
			{
				unsigned long flags(MINOS_REGULAR);
				unsigned char found(0);
				float heuristic(PATHFINDER_HEURISTIC_MAX);

				pathfinder.SetCylinder(curpr.ioid->physics.cyl.radius, curpr.ioid->physics.cyl.height);

				if (curpr.ioid->_npcdata->behavior & BEHAVIOUR_FIGHT)
					flags |= MINOS_TACTIC;

				if (curpr.ioid->_npcdata->behavior & (BEHAVIOUR_SNEAK | BEHAVIOUR_HIDE))
					flags |= MINOS_STEALTH;

				if ((curpr.ioid->_npcdata->behavior & BEHAVIOUR_MOVE_TO)
				        || (curpr.ioid->_npcdata->behavior & BEHAVIOUR_GO_HOME))
				{
					float distance(EEDistance3D(&ACTIVEBKG->anchors[curpr.from].pos, &ACTIVEBKG->anchors[curpr.to].pos));

					if (distance < PATHFINDER_DISTANCE_MAX)
						heuristic = PATHFINDER_HEURISTIC_MIN + PATHFINDER_HEURISTIC_RANGE * (distance / PATHFINDER_DISTANCE_MAX);

					pathfinder.SetHeuristic(heuristic);
					found = pathfinder.Move(flags,
					                        curpr.from, curpr.to,
					                        curpr.returnnumber, curpr.returnlist);
				}
				else if (curpr.ioid->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND)
				{
					if (curpr.ioid->_npcdata->behavior_param < PATHFINDER_DISTANCE_MAX)
						heuristic = PATHFINDER_HEURISTIC_MIN + PATHFINDER_HEURISTIC_RANGE * (curpr.ioid->_npcdata->behavior_param / PATHFINDER_DISTANCE_MAX);

					pathfinder.SetHeuristic(heuristic);
					found = pathfinder.WanderAround(flags,
					                                curpr.from, curpr.ioid->_npcdata->behavior_param,
					                                curpr.returnnumber, curpr.returnlist);
				}
				else if (curpr.ioid->_npcdata->behavior & (BEHAVIOUR_FLEE | BEHAVIOUR_HIDE))
				{
					if (curpr.ioid->_npcdata->behavior_param < PATHFINDER_DISTANCE_MAX)
						heuristic = PATHFINDER_HEURISTIC_MIN + PATHFINDER_HEURISTIC_RANGE * (curpr.ioid->_npcdata->behavior_param / PATHFINDER_DISTANCE_MAX);

					pathfinder.SetHeuristic(heuristic);
					float safedist = curpr.ioid->_npcdata->behavior_param + EEDistance3D(&curpr.ioid->target, &curpr.ioid->pos);

					found = pathfinder.Flee(flags,
					                        curpr.from,
					                        curpr.ioid->target,
					                        safedist, 
					                        curpr.returnnumber,
					                        curpr.returnlist);
				}
				else if (curpr.ioid->_npcdata->behavior & BEHAVIOUR_LOOK_FOR)
				{
					float distance(EEDistance3D(&curpr.ioid->pos, &curpr.ioid->target));

					if (distance < PATHFINDER_DISTANCE_MAX)
						heuristic = PATHFINDER_HEURISTIC_MIN + PATHFINDER_HEURISTIC_RANGE * (distance / PATHFINDER_DISTANCE_MAX);

					pathfinder.SetHeuristic(heuristic);
					found = pathfinder.LookFor(flags, curpr.from,
					                           curpr.ioid->target, curpr.ioid->_npcdata->behavior_param,
					                           curpr.returnnumber, curpr.returnlist);
				}
			}
		}

		CURPATHFINDIO = NULL;

		PATHFINDER_WORKING = 0;

		ReleaseMutex(PATHFINDER_MUTEX);
		QueryPerformanceCounter(&Pend_chrono);
		BENCH_PATHFINDER += (unsigned long)(Pend_chrono.QuadPart - Pstart_chrono.QuadPart);
		Sleep(PATHFINDER_UPDATE_INTERVAL);
	}

	//fix leaks memory but freeze characters
	//	pathfinder.Clean();

	PATHFINDER_WORKING = 0;

	ExitThread(0);

	return 0;
}

void EERIE_PATHFINDER_Release()
{
	if (!PATHFINDER) return;

	CURPATHFINDIO = NULL;

	while (WaitForSingleObject(PATHFINDER_MUTEX, PATHFINDER_RELEASE_WAIT) == WAIT_TIMEOUT)
		Sleep(1);

	EERIE_PATHFINDER_Clear(1);

	bExitPathfinderThread = true;

	while (WaitForSingleObject(PATHFINDER, PATHFINDER_RELEASE_WAIT) == WAIT_TIMEOUT)
		Sleep(1);

	CloseHandle(PATHFINDER), PATHFINDER = NULL;
	ReleaseMutex(PATHFINDER_MUTEX), CloseHandle(PATHFINDER_MUTEX), PATHFINDER_MUTEX = NULL;
}

void EERIE_PATHFINDER_Create(EERIE_BACKGROUND * eb)
{
	if (PATHFINDER) EERIE_PATHFINDER_Release();

	if (PATHFINDER_MUTEX)
	{
		ReleaseMutex(PATHFINDER_MUTEX);
		CloseHandle(PATHFINDER_MUTEX);
		PATHFINDER_MUTEX = NULL;
	}

	PATHFINDER_MUTEX = CreateMutex(NULL, FALSE, NULL);
	CURPATHFINDIO = NULL;
	DWORD id;
	PATHFINDER = (HANDLE)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PATHFINDER_Proc, NULL, 0, (LPDWORD)&id);

	if (PATHFINDER) SetThreadPriority(PATHFINDER, THREAD_PRIORITY_NORMAL); 
}

