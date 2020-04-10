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

#include "ai/Paths.h"

#include <cstdlib>
#include <cstring>
#include <algorithm>

#include <boost/foreach.hpp>

#include "core/GameTime.h"
#include "core/Core.h"

#include "game/Player.h"
#include "game/EntityManager.h"
#include "game/Inventory.h"

#include "graphics/GlobalFog.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"

#include "math/Random.h"

#include "platform/Platform.h"
#include "platform/profiler/Profiler.h"

#include "scene/GameSound.h"

#include "script/Script.h"

std::vector<Zone> g_zones;
std::vector<Path> g_paths;

static void ARX_PATH_ComputeBB(Zone * ap) {
	
	ap->bbmin = Vec3f(9999999999.f);
	ap->bbmax = Vec3f(-9999999999.f);
	
	BOOST_FOREACH(const Vec3f & rpos, ap->pathways) {
		ap->bbmin.x = std::min(ap->bbmin.x, ap->pos.x + rpos.x);
		ap->bbmax.x = std::max(ap->bbmax.x, ap->pos.x + rpos.x);
		ap->bbmin.z = std::min(ap->bbmin.z, ap->pos.z + rpos.z);
		ap->bbmax.z = std::max(ap->bbmax.z, ap->pos.z + rpos.z);
	}
	
	if(ap->height > 0) {
		ap->bbmin.y = ap->pos.y - float(ap->height);
		ap->bbmax.y = ap->pos.y;
	} else {
		ap->bbmin.y = -99999999.f;
		ap->bbmax.y = 99999999.f;
	}
	
}

void ARX_PATH_ComputeAllBoundingBoxes() {
	BOOST_FOREACH(Zone & zone, g_zones) {
		ARX_PATH_ComputeBB(&zone);
	}
}

long ARX_PATH_IsPosInZone(const Zone * ap, Vec3f pos) {
	
	if(pos.x < ap->bbmin.x || pos.x > ap->bbmax.x || pos.z < ap->bbmin.z || pos.z > ap->bbmax.z
	   || pos.y < ap->bbmin.y || pos.y > ap->bbmax.y) {
		return 0;
	}
	
	int c = 0;
	
	pos.x -= ap->pos.x;
	pos.z -= ap->pos.z;
	
	for(size_t i = 0, j = ap->pathways.size() - 1; i < ap->pathways.size(); j = i++) {
		const Vec3f & pi = ap->pathways[i];
		const Vec3f & pj = ap->pathways[j];
		if(((pi.z <= pos.z && pos.z < pj.z) || (pj.z <= pos.z && pos.z < pi.z))
		   && (pos.x < (pj.x - pi.x) * (pos.z - pi.z) / (pj.z - pi.z) + pi.x)) {
			c = !c;
		}
	}
	
	return c;
}

static Zone * ARX_PATH_CheckInZone(const Vec3f & pos) {
	
	BOOST_FOREACH(Zone & zone, g_zones) {
		if(ARX_PATH_IsPosInZone(&zone, pos)) {
			return &zone;
		}
	}
	
	return NULL;
}

static Zone * ARX_PATH_CheckInZone(Entity * io) {
	arx_assert(io);
	return ARX_PATH_CheckInZone(GetItemWorldPosition(io));
}

static Zone * ARX_PATH_CheckPlayerInZone() {
	return ARX_PATH_CheckInZone(player.pos + Vec3f(0.f, 160.f, 0.f));
}

static void EntityEnteringCurrentZone(Entity * io, Zone * current) {
	
	io->inzone_show = io->show;
	
	SendIOScriptEvent(NULL, io, SM_ENTERZONE, current->name);
	
	if(!current->controled.empty()) {
		EntityHandle t = entities.getById(current->controled);
		if(t != EntityHandle()) {
			ScriptParameters parameters;
			parameters.push_back(io->idString());
			parameters.push_back(current->name);
			SendIOScriptEvent(NULL, entities[t], SM_CONTROLLEDZONE_ENTER, parameters);
		}
	}
	
}

static void EntityLeavingLastZone(Entity * io, Zone * last) {
	
	SendIOScriptEvent(NULL, io, SM_LEAVEZONE, last->name);
	
	if(!last->controled.empty()) {
		EntityHandle t = entities.getById(last->controled);
		if(t != EntityHandle()) {
			ScriptParameters parameters;
			parameters.push_back(io->idString());
			parameters.push_back(last->name);
			SendIOScriptEvent(NULL, entities[t], SM_CONTROLLEDZONE_LEAVE, parameters);
		}
	}
	
}

void ARX_PATH_UpdateAllZoneInOutInside() {
	
	ARX_PROFILE_FUNC();
	
	arx_assert(entities.player());
	
	static size_t count = 1;
	
	long f = glm::clamp(static_cast<long>(g_framedelay), 10l, 50l);
	
	if(count >= entities.size()) {
		count = 1;
	}

	if(entities.size() > 1) {
		for(long tt = 0; tt < f; tt++) {
			const EntityHandle i = EntityHandle(count);
			Entity * io = entities[i];
			
			if(   count < entities.size()
			   && io
			   && io->ioflags & (IO_NPC | IO_ITEM)
			   && io->show != SHOW_FLAG_MEGAHIDE
			) {
				arx_assert(io->show != SHOW_FLAG_DESTROYED);
				Zone * current = ARX_PATH_CheckInZone(io);
				Zone * last = io->inzone;
				
				if(current != last) {
					// Changed zones
					if(last) {
						EntityLeavingLastZone(io, last);
					}
					if(current) {
						EntityEnteringCurrentZone(io, current);
					}
				} else if(current && io->show != io->inzone_show) {
					// Stayed in the same zone but show flag changed
					EntityEnteringCurrentZone(io, current);
				}
				
				io->inzone = current;
			}
			
			count++;
			
			if(count >= entities.size())
				count = 1;
		}
	}

	// player check*************************************************
	{
		Zone * current = ARX_PATH_CheckPlayerInZone();
		Zone * last = entities.player()->inzone;

		if(current != last) {
			
			if(last && !current) {
				
				// TODO why is this not sent when changing directly between zones
				
				SendIOScriptEvent(NULL, entities.player(), SM_LEAVEZONE, last->name);
				CHANGE_LEVEL_ICON = NoChangeLevel;
				
			}
			
			if(!last && current) {
				
				// TODO why is this not sent when changing directly between zones
				
				SendIOScriptEvent(NULL, entities.player(), SM_ENTERZONE, current->name);
				
				if(!current->ambiance.empty()) {
					ARX_SOUND_PlayZoneAmbiance(current->ambiance, ARX_SOUND_PLAY_LOOPED, current->amb_max_vol * 0.01f);
				}
				
				if(current->flags & PATH_FARCLIP) {
					g_desiredFogParameters.flags |= GMOD_ZCLIP;
					g_desiredFogParameters.zclip = current->farclip;
				}
				
				if(current->flags & PATH_RGB) {
					g_desiredFogParameters.flags |= GMOD_DCOLOR;
					g_desiredFogParameters.depthcolor = current->rgb;
				}
				
			}
			
			if(last && !last->controled.empty()) {
				EntityHandle t = entities.getById(last->controled);
				if(t != EntityHandle()) {
					ScriptParameters parameters;
					parameters.push_back("player");
					parameters.push_back(last->name);
					SendIOScriptEvent(NULL, entities[t], SM_CONTROLLEDZONE_LEAVE, parameters);
				}
			}
			
			if(current && !current->controled.empty()) {
				EntityHandle t = entities.getById(current->controled);
				if(t != EntityHandle()) {
					ScriptParameters parameters;
					parameters.push_back("player");
					parameters.push_back(current->name);
					SendIOScriptEvent(NULL, entities[t], SM_CONTROLLEDZONE_ENTER, parameters);
				}
			}
			
		}
		
		entities.player()->inzone = current;
	}
	
}

Zone::Zone(const std::string & _name, const Vec3f & _pos)
	: name(_name)
	, flags(0)
	, pos(_pos)
	, height(0)
	, rgb(Color3f::black)
	, farclip(0.f)
	, amb_max_vol(0.f)
	, bbmin(0.f)
	, bbmax(0.f)
{ }

Path::Path(const std::string & _name, const Vec3f & _pos)
	: name(_name)
	, pos(_pos)
{ }

void ARX_PATH_ClearAllUsePath() {
	BOOST_FOREACH(Entity * e, entities) {
		if(e && e->usepath) {
			delete e->usepath;
			e->usepath = NULL;
		}
	}
}

void ARX_PATH_ClearAllControled() {
	BOOST_FOREACH(Zone & zone, g_zones) {
		zone.controled.clear();
	}
}

Zone * getZoneByName(const std::string & name) {
	
	if(name.empty()) {
		return NULL;
	}
	
	BOOST_FOREACH(Zone & zone, g_zones) {
		if(zone.name == name) {
			return &zone;
		}
	}
	
	return NULL;
}

const Path * getPathByName(const std::string & name) {
	
	if(name.empty()) {
		return NULL;
	}
	
	BOOST_FOREACH(const Path & path, g_paths) {
		if(path.name == name) {
			return &path;
		}
	}
	
	return NULL;
}

void ARX_PATH_ReleaseAllPath() {
	
	ARX_PATH_ClearAllUsePath();
	
	g_zones.clear();
	g_paths.clear();
	
}

Vec3f Path::interpolateCurve(size_t i, float step) const {
	Vec3f p0 = pathways[i + 0].rpos, p1 = pathways[i + 1].rpos, p2 = pathways[i + 2].rpos;
	return pos + p0 * (1 - step) + p1 * (step - square(step)) + p2 * square(step);
}

long ARX_PATHS_Interpolate(ARX_USE_PATH * aup, Vec3f * pos) {
	
	const Path * ap = aup->path;
	
	// compute Delta Time
	GameDuration tim = aup->_curtime - aup->_starttime;
	if(tim < 0) {
		return -1;
	}
	
	// set pos to startpos
	*pos = Vec3f(0.f);
	
	if(tim == 0) {
		return 0;
	}
	
	// we start at reference waypoint 0  (time & rpos = 0 for this waypoint).
	size_t targetwaypoint = 1;
	aup->aupflags &= ~ARX_USEPATH_FLAG_FINISHED;

	if(ap->pathways.empty()) {
		return -1;
	}
	
	// While we have time left, iterate
	while(targetwaypoint < ap->pathways.size()) {
		
		bool bezier = (ap->pathways[targetwaypoint - 1].flag == PATHWAY_BEZIER
		               && targetwaypoint + 1 < ap->pathways.size());
		if(bezier) {
			targetwaypoint++;
		}
		
		GameDuration delta = tim - ap->pathways[targetwaypoint]._time;
		if(delta >= 0) {
			tim = delta;
			targetwaypoint++;
			continue;
		}
		
		float rel = tim / ap->pathways[targetwaypoint]._time;
		
		if(bezier) {
			*pos = ap->interpolateCurve(targetwaypoint - 2, rel);
		} else {
			*pos = ap->pos + glm::mix(ap->pathways[targetwaypoint - 1].rpos, ap->pathways[targetwaypoint].rpos, rel);
		}
		
		return targetwaypoint - 1;
	}
	
	*pos = ap->pos + ap->pathways[ap->pathways.size() - 1].rpos;
	aup->aupflags |= ARX_USEPATH_FLAG_FINISHED;
	return -2;
}
