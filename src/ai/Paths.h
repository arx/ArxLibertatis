/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Color.h"
#include "io/resource/ResourcePath.h"
#include "math/Types.h"
#include "math/Vector.h"
#include "platform/Flags.h"

#include "graphics/effects/Trail.h"

struct EERIE_CAMERA;
struct EERIE_3DOBJ;
class Entity;
class Trail;

enum PathwayType {
	PATHWAY_STANDARD = 0,
	PATHWAY_BEZIER = 1,
	PATHWAY_BEZIER_CONTROLPOINT = 2
};

struct ARX_PATHWAY {
	Vec3f rpos; //relative pos
	PathwayType flag;
	float _time;
};

// ARX_PATH@flags values
enum PathFlag {
	PATH_LOOP     = (1<<0),
	PATH_AMBIANCE = (1<<1),
	PATH_RGB      = (1<<2),
	PATH_FARCLIP  = (1<<3),
	PATH_REVERB   = (1<<4)
};
DECLARE_FLAGS(PathFlag, PathFlags)
DECLARE_FLAGS_OPERATORS(PathFlags)

struct ARX_PATH {
	
	ARX_PATH(const std::string & name, const Vec3f & pos);
	
	std::string name;
	PathFlags flags;
	Vec3f initpos;
	Vec3f pos;
	long nb_pathways;
	ARX_PATHWAY * pathways;
	
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
	
};

enum UsePathFlag {
	ARX_USEPATH_FLAG_FINISHED    = (1<<0),
	ARX_USEPATH_WORM_SPECIFIC    = (1<<1),
	ARX_USEPATH_FOLLOW_DIRECTION = (1<<2),
	ARX_USEPATH_FORWARD          = (1<<3),
	ARX_USEPATH_BACKWARD         = (1<<4),
	ARX_USEPATH_PAUSE            = (1<<5),
	ARX_USEPATH_FLAG_ADDSTARTPOS = (1<<6)
};
DECLARE_FLAGS(UsePathFlag, UsePathFlags)
DECLARE_FLAGS_OPERATORS(UsePathFlags)

struct ARX_USE_PATH {
	ARX_PATH * path;
	float _starttime;
	float _curtime;
	UsePathFlags aupflags;
	Vec3f initpos;
	long lastWP;
};

struct MASTER_CAMERA_STRUCT {
	long exist; // 2== want to change to want_vars...
	Entity * io;
	Entity * want_io;
};

enum PathMod {
	ARX_PATH_MOD_POSITION  = (1<<0),
	ARX_PATH_MOD_FLAGS     = (1<<1),
	ARX_PATH_MOD_TIME      = (1<<2),
	ARX_PATH_MOD_TRANSLATE = (1<<3),
	ARX_PATH_HIERARCHY     = (1<<4)
};
DECLARE_FLAGS(PathMod, PathMods)
DECLARE_FLAGS_OPERATORS(PathMods)

extern MASTER_CAMERA_STRUCT MasterCamera;
extern ARX_PATH ** ARXpaths;
#ifdef BUILD_EDITOR
extern ARX_PATH * ARX_PATHS_FlyingOverAP;
extern ARX_PATH * ARX_PATHS_SelectedAP;
extern long	ARX_PATHS_SelectedNum;
extern long	ARX_PATHS_FlyingOverNum;
#endif
extern long	nbARXpaths;

void ARX_PATH_UpdateAllZoneInOutInside();
long ARX_PATH_IsPosInZone(ARX_PATH * ap, float x, float y, float z);
void ARX_PATH_ClearAllUsePath();
void ARX_PATH_ReleaseAllPath();
ARX_PATH * ARX_PATH_GetAddressByName(const std::string & name);
void ARX_PATH_ClearAllControled();
void ARX_PATH_ComputeAllBoundingBoxes();

ARX_PATH * ARX_PATHS_ExistName(const std::string & name);
void ARX_PATHS_Delete(ARX_PATH * ap);
long ARX_PATHS_Interpolate(ARX_USE_PATH * aup, Vec3f * pos);

enum ThrownObjectFlag {
	ATO_EXIST      = (1<<0),
	ATO_MOVING     = (1<<1),
	ATO_UNDERWATER = (1<<2),
	ATO_FIERY      = (1<<3)
};
DECLARE_FLAGS(ThrownObjectFlag, ThrownObjectFlags)
DECLARE_FLAGS_OPERATORS(ThrownObjectFlags)

struct ARX_THROWN_OBJECT {
	ThrownObjectFlags flags;
	Vec3f vector;
	EERIE_QUAT quat;
	Vec3f initial_position;
	float velocity;
	Vec3f position;
	float damages;
	EERIE_3DOBJ * obj;
	long source;
	unsigned long creation_time;
	float poisonous;
	Trail * pRuban;
};

const size_t MAX_THROWN_OBJECTS = 100;

extern ARX_THROWN_OBJECT Thrown[MAX_THROWN_OBJECTS];



long ARX_THROWN_OBJECT_GetFree();
long ARX_THROWN_OBJECT_Throw(long source, Vec3f * position, Vec3f * vect, EERIE_QUAT * quat, float velocity, float damages, float poisonous);
void ARX_THROWN_OBJECT_KillAll();
void ARX_THROWN_OBJECT_Manage(unsigned long time_offset);
void ARX_THROWN_OBJECT_Render();

long ARX_PHYSICS_BOX_ApplyModel(EERIE_3DOBJ * obj, float framediff, float rubber, long source);

#endif // ARX_AI_PATHS_H
