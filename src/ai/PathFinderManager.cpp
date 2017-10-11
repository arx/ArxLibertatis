/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
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
#include <list>

#include <boost/foreach.hpp>

#include "ai/Anchors.h"

#include "ai/PathFinder.h"
#include "game/Entity.h"
#include "game/NPC.h"
#include "graphics/Math.h"
#include "platform/Thread.h"
#include "platform/Lock.h"
#include "platform/profiler/Profiler.h"
#include "scene/Light.h"

static const float PATHFINDER_HEURISTIC_MIN = 0.2f;
static const float PATHFINDER_HEURISTIC_MAX = PathFinder::HEURISTIC_MAX;
static const float PATHFINDER_HEURISTIC_RANGE = PATHFINDER_HEURISTIC_MAX
                                                - PATHFINDER_HEURISTIC_MIN;
static const float PATHFINDER_DISTANCE_MAX = 5000.0f;

// Pathfinder Definitions
static const unsigned long PATHFINDER_UPDATE_INTERVAL = 10;

class PathFinderThread : public StoppableThread {
	
	std::list<PATHFINDER_REQUEST> m_queue;
	volatile bool m_busy;
	
	void run();
	
	bool getNextRequest(PATHFINDER_REQUEST & request);
	
public:
	
	PathFinderThread() : m_busy(false) { }
	
	Lock m_mutex;
	
	void queueRequest(const PATHFINDER_REQUEST & request);
	size_t queueSize() { return m_queue.size(); }
	void clearQueue() { m_queue.clear(); }
	
	bool isBusy() {
		return m_busy;
	}
	
};

static PathFinderThread * g_pathFinderThread = NULL;

void PathFinderThread::queueRequest(const PATHFINDER_REQUEST & request) {
	
	Autolock lock(&m_mutex);

	// If this NPC is already requesting a Pathfinding then either
	// try to Override it or add it to queue if it is currently being
	// processed.
	BOOST_FOREACH(PATHFINDER_REQUEST & oldRequest, m_queue) {
		if(oldRequest.ioid == request.ioid) {
			oldRequest = request;
			return;
		}
	}
	
	if(!m_queue.empty()
	   && (request.ioid->_npcdata->behavior & (BEHAVIOUR_MOVE_TO | BEHAVIOUR_FLEE | BEHAVIOUR_LOOK_FOR))) {
		// priority: insert as second element of queue
		m_queue.insert(++m_queue.begin(), request);
	} else {
		m_queue.push_back(request);
	}
	
}

// Adds a Pathfinder Search Element to the pathfinder queue.
bool EERIE_PATHFINDER_Add_To_Queue(const PATHFINDER_REQUEST & request) {
	
	if(!g_pathFinderThread) {
		return false;
	}
	
	g_pathFinderThread->queueRequest(request);
	
	return true;
}

long EERIE_PATHFINDER_Get_Queued_Number() {
	
	if(!g_pathFinderThread) {
		return 0;
	}
	
	Autolock lock(&g_pathFinderThread->m_mutex);
	
	return g_pathFinderThread->queueSize();
}

bool EERIE_PATHFINDER_Is_Busy() {
	
	if(!g_pathFinderThread) {
		return false;
	}
	
	return g_pathFinderThread->isBusy();
}

void EERIE_PATHFINDER_Clear() {
	
	if(!g_pathFinderThread) {
		return;
	}
	
	Autolock lock(&g_pathFinderThread->m_mutex);
	
	g_pathFinderThread->clearQueue();
}

// Retrieves & Removes next Pathfind request from queue
bool PathFinderThread::getNextRequest(PATHFINDER_REQUEST & request) {
	
	if(m_queue.empty()) {
		return false;
	}
	
	request = m_queue.front();
	
	m_queue.pop_front();
	
	// TODO potentially unsafe Entity access
	if(request.ioid && (request.ioid->ioflags & IO_NPC) && (request.ioid->_npcdata->behavior == BEHAVIOUR_NONE)) {
		return false;
	}
	
	return true;
}

// Pathfinder Thread
void PathFinderThread::run() {
	
	BackgroundData * eb = ACTIVEBKG;
	PathFinder pathfinder(eb->nbanchors, eb->anchors, g_staticLightsMax, (EERIE_LIGHT **)g_staticLights);

	for(; !isStopRequested(); sleep(PATHFINDER_UPDATE_INTERVAL)) {
		
		Autolock lock(&m_mutex);
		
		m_busy = true;

		PATHFINDER_REQUEST curpr;
		if(getNextRequest(curpr)) {
			
			ARX_PROFILE_FUNC();
			
			if(curpr.ioid && curpr.ioid->_npcdata) {
				float heuristic(PATHFINDER_HEURISTIC_MAX);

				pathfinder.setCylinder(curpr.ioid->physics.cyl.radius, curpr.ioid->physics.cyl.height);

				bool stealth = (curpr.ioid->_npcdata->behavior & (BEHAVIOUR_SNEAK | BEHAVIOUR_HIDE))
				                == (BEHAVIOUR_SNEAK | BEHAVIOUR_HIDE);

				
				PathFinder::Result result;
				
				if(   (curpr.ioid->_npcdata->behavior & BEHAVIOUR_MOVE_TO)
				   || (curpr.ioid->_npcdata->behavior & BEHAVIOUR_GO_HOME)
				) {
					float distance = fdist(ACTIVEBKG->anchors[curpr.from].pos, ACTIVEBKG->anchors[curpr.to].pos);

					if(distance < PATHFINDER_DISTANCE_MAX)
						heuristic = PATHFINDER_HEURISTIC_MIN + PATHFINDER_HEURISTIC_RANGE * (distance / PATHFINDER_DISTANCE_MAX);

					pathfinder.setHeuristic(heuristic);
					pathfinder.move(curpr.from, curpr.to, result, stealth);
				} else if(curpr.ioid->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND) {
					if(curpr.ioid->_npcdata->behavior_param < PATHFINDER_DISTANCE_MAX)
						heuristic = PATHFINDER_HEURISTIC_MIN + PATHFINDER_HEURISTIC_RANGE * (curpr.ioid->_npcdata->behavior_param / PATHFINDER_DISTANCE_MAX);

					pathfinder.setHeuristic(heuristic);
					pathfinder.wanderAround(curpr.from, curpr.ioid->_npcdata->behavior_param, result, stealth);
				} else if(curpr.ioid->_npcdata->behavior & (BEHAVIOUR_FLEE | BEHAVIOUR_HIDE)) {
					if(curpr.ioid->_npcdata->behavior_param < PATHFINDER_DISTANCE_MAX)
						heuristic = PATHFINDER_HEURISTIC_MIN
						            + PATHFINDER_HEURISTIC_RANGE
						              * (curpr.ioid->_npcdata->behavior_param / PATHFINDER_DISTANCE_MAX);

					pathfinder.setHeuristic(heuristic);
					float safedist = curpr.ioid->_npcdata->behavior_param
					                 + fdist(curpr.ioid->target, curpr.ioid->pos);

					pathfinder.flee(curpr.from, curpr.ioid->target, safedist, result, stealth);
				} else if(curpr.ioid->_npcdata->behavior & BEHAVIOUR_LOOK_FOR) {
					float distance = fdist(curpr.ioid->pos, curpr.ioid->target);

					if(distance < PATHFINDER_DISTANCE_MAX)
						heuristic = PATHFINDER_HEURISTIC_MIN + PATHFINDER_HEURISTIC_RANGE * (distance / PATHFINDER_DISTANCE_MAX);

					pathfinder.setHeuristic(heuristic);
					pathfinder.lookFor(curpr.from, curpr.ioid->target,
					                   curpr.ioid->_npcdata->behavior_param, result, stealth);
				}
				
				if(!result.empty()) {
					long * list = (long*)malloc(result.size() * sizeof(long));
					std::copy(result.begin(), result.end(), list);
					*(curpr.returnlist) = list;
				}
				*(curpr.returnnumber) = result.size();
				
			}
		}

		m_busy = false;
		
	}

	// fix leaks memory but freeze characters
	// pathfinder.Clean();
	
}

void EERIE_PATHFINDER_Release() {
	
	if(!g_pathFinderThread) {
		return;
	}
	
	EERIE_PATHFINDER_Clear();
	
	g_pathFinderThread->stop();
	
	delete g_pathFinderThread, g_pathFinderThread = NULL;
}

void EERIE_PATHFINDER_Create() {
	
	if(g_pathFinderThread) {
		EERIE_PATHFINDER_Release();
	}
	
	g_pathFinderThread = new PathFinderThread();
	g_pathFinderThread->setThreadName("Pathfinder");
	g_pathFinderThread->start();
}
