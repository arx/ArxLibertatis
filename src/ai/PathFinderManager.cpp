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

#include "ai/PathFinderManager.h"

#include <cstring>
#include <cstdlib>
#include <algorithm>

#include "ai/PathFinder.h"
#include "game/Entity.h"
#include "game/NPC.h"
#include "graphics/Math.h"
#include "platform/Thread.h"
#include "platform/Lock.h"
#include "physics/Anchors.h"
#include "scene/Light.h"

using std::memcpy;


static const float PATHFINDER_HEURISTIC_MIN = 0.2f;
static const float PATHFINDER_HEURISTIC_MAX = PathFinder::HEURISTIC_MAX;
static const float PATHFINDER_HEURISTIC_RANGE = PATHFINDER_HEURISTIC_MAX
                                                - PATHFINDER_HEURISTIC_MIN;
static const float PATHFINDER_DISTANCE_MAX = 5000.0f;

// Pathfinder Definitions
static unsigned long PATHFINDER_UPDATE_INTERVAL = 10;

PATHFINDER_REQUEST pr;
long PATHFINDER_WORKING = 0;

class PathFinderThread : public StoppableThread {
	
	void run();
	
};

static PathFinderThread * pathfinder = NULL;
static Lock * mutex = NULL;

struct PATHFINDER_QUEUE_ELEMENT {
	PATHFINDER_REQUEST req;
	PATHFINDER_QUEUE_ELEMENT * next;
	long valid;
};

static PATHFINDER_QUEUE_ELEMENT * pathfinder_queue_start = NULL;

// An Io can request Pathfinding only once so we insure that it's always the case.
// A new pathfinder request from the same IO will overwrite the precedent.
static PATHFINDER_QUEUE_ELEMENT * PATHFINDER_Find_ioid(Entity * io) {
	
	if (!pathfinder_queue_start) return NULL;

	PATHFINDER_QUEUE_ELEMENT * cur = pathfinder_queue_start;

	while(cur) {
		if (cur->req.ioid == io) return cur->valid ? cur : NULL;
		
		cur = cur->next;
	}

	return NULL;
}

// Adds a Pathfinder Search Element to the pathfinder queue.
bool EERIE_PATHFINDER_Add_To_Queue(PATHFINDER_REQUEST * req) {
	
	if(!pathfinder) {
		return false;
	}
	
	Autolock lock(mutex);

	PATHFINDER_QUEUE_ELEMENT * cur = pathfinder_queue_start;

	PATHFINDER_QUEUE_ELEMENT * temp;

	// If this NPC is already requesting a Pathfinding then either
	// try to Override it or add it to queue if it is currently being
	// processed.
	temp = PATHFINDER_Find_ioid(req->ioid);

	if(temp && temp->valid && temp != pathfinder_queue_start) {
		temp->valid = 0;
		memcpy(&temp->req, req, sizeof(PATHFINDER_REQUEST));
		temp->valid = 1;
		return true;
	}

	// Create a New element for the queue
	temp = (PATHFINDER_QUEUE_ELEMENT *)malloc(sizeof(PATHFINDER_QUEUE_ELEMENT));

	if(!temp) {
		return false;
	}

	// Fill this New element with new request
	memcpy(&temp->req, req, sizeof(PATHFINDER_REQUEST));
	temp->valid = 1;

	// No queue start ? then this element becomes the queue start
	if(!cur) {
		temp->next = NULL;
		pathfinder_queue_start = temp;
		
	} else if((req->ioid->_npcdata->behavior & (BEHAVIOUR_MOVE_TO | BEHAVIOUR_FLEE
	                                            | BEHAVIOUR_LOOK_FOR)) && cur->next) {
		// priority: insert as second element of queue
		temp->next = cur->next;
		cur->next = temp;
		
	} else {
		
		// add to end of queue
		temp->next = NULL;
		
		if(!pathfinder_queue_start) {
			pathfinder_queue_start = temp;
			return true;
		} else {
			while(cur->next) {
				cur = cur->next;
			}
			cur->next = temp;
		}
	}
	
	return true;
}

long EERIE_PATHFINDER_Get_Queued_Number() {
	if(!mutex)
		return 0;

	Autolock lock(mutex);
	
	PATHFINDER_QUEUE_ELEMENT * cur = pathfinder_queue_start;

	long count = 0;

	while (cur) cur = cur->next, count++;

	return count;
}

static void EERIE_PATHFINDER_Clear_Private() {
	
	PATHFINDER_QUEUE_ELEMENT * cur = pathfinder_queue_start;
	PATHFINDER_QUEUE_ELEMENT * next;
	
	while(cur) {
		next = cur->next;
		free(cur);
		cur = next;
	}
	
	pathfinder_queue_start = NULL;
	
}


void EERIE_PATHFINDER_Clear() {
	
	if(!pathfinder) {
		return;
	}
	
	Autolock lock(mutex);
	
	EERIE_PATHFINDER_Clear_Private();
	
}

// Retrieves & Removes next Pathfind request from queue
static bool EERIE_PATHFINDER_Get_Next_Request(PATHFINDER_REQUEST * request) {
	
	if(!request || !pathfinder_queue_start || !pathfinder_queue_start->valid) {
		return false;
	}
	
	PATHFINDER_QUEUE_ELEMENT * cur = pathfinder_queue_start;

	if ((cur->req.ioid)
	        && (cur->req.ioid->ioflags & IO_NPC)
	        && (cur->req.ioid->_npcdata->behavior == BEHAVIOUR_NONE)) {
		pathfinder_queue_start = cur->next;
		free(cur);
		return false;
	}

	memcpy(request, &cur->req, sizeof(PATHFINDER_REQUEST));
	pathfinder_queue_start = cur->next;
	free(cur);

	return true;
}

// Pathfinder Thread
void PathFinderThread::run() {
	
	EERIE_BACKGROUND * eb = ACTIVEBKG;
	PathFinder pathfinder(eb->nbanchors, eb->anchors,
	                      MAX_LIGHTS, (EERIE_LIGHT **)GLight);

	while(!isStopRequested()) {
		
		mutex->lock();

		PATHFINDER_WORKING = 1;

		if (EERIE_PATHFINDER_Get_Next_Request(&pr) && pr.isvalid)
		{

			PATHFINDER_REQUEST curpr;
			memcpy(&curpr, &pr, sizeof(PATHFINDER_REQUEST));
			PATHFINDER_WORKING = 2;

			if (curpr.ioid && curpr.ioid->_npcdata)
			{
				float heuristic(PATHFINDER_HEURISTIC_MAX);

				pathfinder.setCylinder(curpr.ioid->physics.cyl.radius, curpr.ioid->physics.cyl.height);

				bool stealth = (curpr.ioid->_npcdata->behavior & (BEHAVIOUR_SNEAK | BEHAVIOUR_HIDE))
				                == (BEHAVIOUR_SNEAK | BEHAVIOUR_HIDE);

				
				PathFinder::Result result;
				
				if ((curpr.ioid->_npcdata->behavior & BEHAVIOUR_MOVE_TO)
				        || (curpr.ioid->_npcdata->behavior & BEHAVIOUR_GO_HOME))
				{
					float distance = fdist(ACTIVEBKG->anchors[curpr.from].pos, ACTIVEBKG->anchors[curpr.to].pos);

					if (distance < PATHFINDER_DISTANCE_MAX)
						heuristic = PATHFINDER_HEURISTIC_MIN
						            + PATHFINDER_HEURISTIC_RANGE * (distance / PATHFINDER_DISTANCE_MAX);

					pathfinder.setHeuristic(heuristic);
					pathfinder.move(curpr.from, curpr.to, result, stealth);
				}
				else if (curpr.ioid->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND)
				{
					if (curpr.ioid->_npcdata->behavior_param < PATHFINDER_DISTANCE_MAX)
						heuristic = PATHFINDER_HEURISTIC_MIN
						            + PATHFINDER_HEURISTIC_RANGE
						              * (curpr.ioid->_npcdata->behavior_param / PATHFINDER_DISTANCE_MAX);

					pathfinder.setHeuristic(heuristic);
					pathfinder.wanderAround(curpr.from, curpr.ioid->_npcdata->behavior_param, result, stealth);
				}
				else if (curpr.ioid->_npcdata->behavior & (BEHAVIOUR_FLEE | BEHAVIOUR_HIDE))
				{
					if (curpr.ioid->_npcdata->behavior_param < PATHFINDER_DISTANCE_MAX)
						heuristic = PATHFINDER_HEURISTIC_MIN
						            + PATHFINDER_HEURISTIC_RANGE
						              * (curpr.ioid->_npcdata->behavior_param / PATHFINDER_DISTANCE_MAX);

					pathfinder.setHeuristic(heuristic);
					float safedist = curpr.ioid->_npcdata->behavior_param
					                 + fdist(curpr.ioid->target, curpr.ioid->pos);

					pathfinder.flee(curpr.from, curpr.ioid->target, safedist, result, stealth);
				}
				else if (curpr.ioid->_npcdata->behavior & BEHAVIOUR_LOOK_FOR)
				{
					float distance = fdist(curpr.ioid->pos, curpr.ioid->target);

					if (distance < PATHFINDER_DISTANCE_MAX)
						heuristic = PATHFINDER_HEURISTIC_MIN
						            + PATHFINDER_HEURISTIC_RANGE * (distance / PATHFINDER_DISTANCE_MAX);

					pathfinder.setHeuristic(heuristic);
					pathfinder.lookFor(curpr.from, curpr.ioid->target,
					                   curpr.ioid->_npcdata->behavior_param, result, stealth);
				}
				
				if(!result.empty()) {
					unsigned short * list = (unsigned short*)malloc(result.size() * sizeof(unsigned short));
					std::copy(result.begin(), result.end(), list);
					*(curpr.returnlist) = list;
				}
				*(curpr.returnnumber) = result.size();
				
			}
		}

		PATHFINDER_WORKING = 0;

		mutex->unlock();
		sleep(PATHFINDER_UPDATE_INTERVAL);
	}

	// fix leaks memory but freeze characters
	// pathfinder.Clean();

	PATHFINDER_WORKING = 0;
	
}

void EERIE_PATHFINDER_Release() {
	
	if(!pathfinder) {
		return;
	}
	
	mutex->lock();
	
	EERIE_PATHFINDER_Clear_Private();
	
	pathfinder->stop();
	
	delete pathfinder, pathfinder = NULL;
	
	mutex->unlock(), delete mutex, mutex = NULL;
}

void EERIE_PATHFINDER_Create() {
	
	if(pathfinder) {
		EERIE_PATHFINDER_Release();
	}
	
	if(!mutex) {
		mutex = new Lock();
	}
	
	pathfinder = new PathFinderThread();
	pathfinder->setThreadName("Pathfinder");
	pathfinder->start();
}
