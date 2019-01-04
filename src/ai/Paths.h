/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_AI_PATHS_H
#define ARX_AI_PATHS_H

#include <stddef.h>
#include <string>
#include <vector>

#include "core/TimeTypes.h"
#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Color.h"
#include "io/resource/ResourcePath.h"
#include "math/Types.h"
#include "math/Vector.h"
#include "util/Flags.h"

class Entity;

enum PathwayType {
	PATHWAY_STANDARD = 0,
	PATHWAY_BEZIER = 1,
	PATHWAY_BEZIER_CONTROLPOINT = 2
};

struct ARX_PATHWAY {
	
	Vec3f rpos; // Relative position
	PathwayType flag;
	GameDuration _time;
	
	ARX_PATHWAY()
		: rpos(0.f)
		, flag(PATHWAY_STANDARD)
		, _time(0)
	{ }
	
};

// ARX_PATH@flags values
enum PathFlag {
	PATH_LOOP     = 1 << 0,
	PATH_AMBIANCE = 1 << 1,
	PATH_RGB      = 1 << 2,
	PATH_FARCLIP  = 1 << 3,
	PATH_REVERB   = 1 << 4
};
DECLARE_FLAGS(PathFlag, PathFlags)
DECLARE_FLAGS_OPERATORS(PathFlags)

// TODO this struct is used both for paths followed by NPCs and for zones
struct ARX_PATH {
	
	ARX_PATH(const std::string & _name, const Vec3f & _pos);
	
	std::string name;
	PathFlags flags;
	Vec3f initpos;
	Vec3f pos;
	std::vector<ARX_PATHWAY> pathways;
	
	long height; // 0 NOT A ZONE
	
	//! name of IO to be notified of other IOs interacting with the path
	std::string controled; // TODO why store the name and not a pointer?
	
	res::path ambiance;
	
	Color3f rgb;
	float farclip;
	float reverb; // TODO unused
	float amb_max_vol;
	Vec3f bbmin;
	Vec3f bbmax;
	
	Vec3f interpolateCurve(size_t i, float step) const;
	
};

enum UsePathFlag {
	ARX_USEPATH_FLAG_FINISHED    = 1 << 0,
	ARX_USEPATH_WORM_SPECIFIC    = 1 << 1,
	ARX_USEPATH_FOLLOW_DIRECTION = 1 << 2,
	ARX_USEPATH_FORWARD          = 1 << 3,
	ARX_USEPATH_BACKWARD         = 1 << 4,
	ARX_USEPATH_PAUSE            = 1 << 5,
	ARX_USEPATH_FLAG_ADDSTARTPOS = 1 << 6
};
DECLARE_FLAGS(UsePathFlag, UsePathFlags)
DECLARE_FLAGS_OPERATORS(UsePathFlags)

struct ARX_USE_PATH {
	
	ARX_PATH * path;
	GameInstant _starttime;
	GameInstant _curtime;
	UsePathFlags aupflags;
	Vec3f initpos;
	long lastWP;
	
	ARX_USE_PATH()
		: path(NULL)
		, _starttime(0)
		, _curtime(0)
		, aupflags(0)
		, initpos(0.f)
		, lastWP(0)
	{ }
	
};

enum PathMod {
	ARX_PATH_MOD_POSITION  = 1 << 0,
	ARX_PATH_MOD_FLAGS     = 1 << 1,
	ARX_PATH_MOD_TIME      = 1 << 2,
	ARX_PATH_MOD_TRANSLATE = 1 << 3,
	ARX_PATH_HIERARCHY     = 1 << 4
};
DECLARE_FLAGS(PathMod, PathMods)
DECLARE_FLAGS_OPERATORS(PathMods)

extern std::vector<ARX_PATH *> g_paths;

void ARX_PATH_UpdateAllZoneInOutInside();
long ARX_PATH_IsPosInZone(ARX_PATH * ap, Vec3f pos);
void ARX_PATH_ClearAllUsePath();
void ARX_PATH_ReleaseAllPath();
ARX_PATH * ARX_PATH_GetAddressByName(const std::string & name);
void ARX_PATH_ClearAllControled();
void ARX_PATH_ComputeAllBoundingBoxes();

void ARX_PATHS_Delete(ARX_PATH * ap);
long ARX_PATHS_Interpolate(ARX_USE_PATH * aup, Vec3f * pos);

#endif // ARX_AI_PATHS_H
