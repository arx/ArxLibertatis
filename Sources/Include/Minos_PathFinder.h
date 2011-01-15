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
#ifndef __MINOS_PATHFINDER_H__
#define __MINOS_PATHFINDER_H__

#define __MINOS_PATHFINDER_VERSION__ "0000"

#include <malloc.h>
#include <string.h>
#include "Minos_Common.h"
#include "Minos_List.h"
#include "EERIEMath.h"
#include "EERIEPoly.h"

using namespace MINOS;

// Flags                                                                     //
typedef enum MINOSFlags
{
	MINOS_REGULAR = 0x0000,
	MINOS_STEALTH = 0x0001,
	MINOS_TACTIC  = 0x0002
};

// Internal MINOSNode structure                                                   //
typedef struct _MINOSNode
{
	long data;
	Float g_cost;
	Float f_cost;
	_MINOSNode * parent;
} MINOSNode;

// Constant and default values                                               //
const Float MINOS_HEURISTIC_MIN(0.0F);
const Float MINOS_HEURISTIC_MAX(0.5F);

const Float MINOS_DEFAULT_HEURISTIC(MINOS_HEURISTIC_MAX);
const Float MINOS_DEFAULT_RADIUS(0.0F);
const Float MINOS_DEFAULT_HEIGHT(0.0F);

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Class PathFinder                                                          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
class PathFinder
{
	public:
		// Constructor and destructor                                                //
		PathFinder(const ULong & map_size, _ANCHOR_DATA * map_data,
		           const ULong & light_count, EERIE_LIGHT ** light_list,
		           const ULong & dynlight_count, EERIE_LIGHT ** dynlight_list);
		~PathFinder();
		// Setup                                                                     //
		void SetHeuristic(const Float & heuristic);
		void SetCylinder(const Float & radius, const Float & height);
		// Status                                                                    //
		void GetHeuristic(Float & heuristic);
		void GetCylinder(Float & radius, Float & height);
		// Path creation                                                             //
		UBool Move(const ULong & flags, const ULong & from, const ULong & to, SLong * rstep, UWord ** rlist);
		UBool Flee(const ULong & flags, const ULong & from, const EERIE_3D & danger, const Float & safe_distance, SLong * rstep, UWord ** rlist);
		UBool WanderAround(const ULong & flags, const ULong & from, const Float & around_radius, SLong * rstep, UWord ** rlist);
		UBool LookFor(const ULong & flags, const ULong & from, const EERIE_3D & pos, const Float & radius, SLong * rstep, UWord ** rlist);
		Void Clean();
	private:
		// Implementation                                                            //
		MINOSNode * CreateNode(long data, MINOSNode * parent);
		MINOSNode * GetBestNode();
		UBool Check(MINOSNode *);
		SBool BuildPath(UWord ** rlist, SLong * rnumber);
		Void AddEnlightmentCost(MINOSNode * MINOSNode);
		inline Float Distance(const EERIE_3D & from, const EERIE_3D & to) const;
		ULong GetNearestNode(const EERIE_3D & pos) const;
		// Data                                                                      //
		Float radius;
		Float height;
		Float heuristic;
		ULong map_s;                        //Map size
		_ANCHOR_DATA * map_d;               //Map data
		ULong slight_c, dlight_c;           //Static and dynamic lights count
		EERIE_LIGHT ** slight_l, **dlight_l; //Static and dynamic lights data
		List<MINOSNode *> open, close;
};

#endif//__MINOS_PATHFINDER_H__
