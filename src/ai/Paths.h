/*
 * Copyright 2011-2021 Arx Libertatis Team (see the AUTHORS file)
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
#include <string_view>
#include <vector>

#include "core/TimeTypes.h"
#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Color.h"
#include "io/resource/ResourcePath.h"
#include "math/Types.h"
#include "math/Vector.h"
#include "util/Flags.h"

class Entity;

// Zone@flags values
enum ZoneFlag {
	PATH_AMBIANCE = 1 << 1,
	PATH_RGB      = 1 << 2,
	PATH_FARCLIP  = 1 << 3
};
DECLARE_FLAGS(ZoneFlag, ZoneFlags)
DECLARE_FLAGS_OPERATORS(ZoneFlags)

struct Zone {
	
	Zone(std::string && _name, const Vec3f & _pos) noexcept;
	
	std::string name;
	ZoneFlags flags;
	Vec3f pos;
	std::vector<Vec3f> pathways;
	
	long height = 0;
	
	//! name of IO to be notified of other IOs interacting with the path
	std::string controled; // TODO why store the name and not a pointer?
	
	res::path ambiance;
	
	Color3f rgb = Color3f::black;
	float farclip = 0.f;
	float amb_max_vol = 0.f;
	Vec3f bbmin = Vec3f(0.f);
	Vec3f bbmax = Vec3f(0.f);
	
};

enum PathwayType {
	PATHWAY_STANDARD = 0,
	PATHWAY_BEZIER = 1,
	PATHWAY_BEZIER_CONTROLPOINT = 2
};

struct ARX_PATHWAY {
	
	Vec3f rpos = Vec3f(0.f); // Relative position
	PathwayType flag = PATHWAY_STANDARD;
	GameDuration _time;
	
	constexpr ARX_PATHWAY() noexcept { }
	
};

struct Path {
	
	Path(std::string && _name, const Vec3f & _pos) noexcept;
	
	std::string name;
	Vec3f pos;
	std::vector<ARX_PATHWAY> pathways;
	
	[[nodiscard]] Vec3f interpolateCurve(size_t i, float step) const noexcept;
	
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
	
	const Path * path = nullptr;
	GameInstant _starttime;
	GameInstant _curtime;
	UsePathFlags aupflags;
	long lastWP = 0;
	
};

extern std::vector<Zone> g_zones;
extern std::vector<Path> g_paths;

void ARX_PATH_UpdateAllZoneInOutInside();
long ARX_PATH_IsPosInZone(const Zone * ap, Vec3f pos);
void ARX_PATH_ClearAllUsePath();
void ARX_PATH_ReleaseAllPath();
Zone * getZoneByName(std::string_view name);
const Path * getPathByName(std::string_view name);
void ARX_PATH_ClearAllControled();
void ARX_PATH_ComputeAllBoundingBoxes();

long ARX_PATHS_Interpolate(ARX_USE_PATH * aup, Vec3f * pos);

#endif // ARX_AI_PATHS_H
