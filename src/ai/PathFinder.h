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
#ifndef ARX_AI_PATHFINDER_H
#define ARX_AI_PATHFINDER_H

#include <vector>

class _ANCHOR_DATA;
class EERIE_LIGHT;
class EERIE_3D;

enum PathFinderFlags {
	MINOS_REGULAR = 0x0000,
	MINOS_STEALTH = 0x0001,
	MINOS_TACTIC  = 0x0002
};


const float MINOS_HEURISTIC_MIN(0.0F);
const float MINOS_HEURISTIC_MAX(0.5F);

const float MINOS_DEFAULT_HEURISTIC(MINOS_HEURISTIC_MAX);
const float MINOS_DEFAULT_RADIUS(0.0F);
const float MINOS_DEFAULT_HEIGHT(0.0F);

class PathFinder {
	
public:
	
	PathFinder(unsigned long map_size, _ANCHOR_DATA * map_data,
	           unsigned long light_count, EERIE_LIGHT ** light_list,
	           unsigned long dynlight_count, EERIE_LIGHT ** dynlight_list);
	~PathFinder();
	
	// Setup                                                                     //
	void SetHeuristic(float heuristic);
	void SetCylinder(float radius, float height);
	
	// Path creation                                                             //
	bool Move(unsigned long flags, unsigned long from, unsigned long to, long * rstep, unsigned short ** rlist);
	bool Flee(unsigned long flags, unsigned long from, const EERIE_3D & danger, float safe_distance, long * rstep, unsigned short ** rlist);
	bool WanderAround(unsigned long flags, unsigned long from, float around_radius, long * rstep, unsigned short ** rlist);
	bool LookFor(unsigned long flags, unsigned long from, const EERIE_3D & pos, float radius, long * rstep, unsigned short ** rlist);
	void Clean();
	
private:
	
	class Node;
	
	Node * GetBestNode();
	bool Check(Node *);
	bool BuildPath(unsigned short ** rlist, long * rnumber);
	void AddEnlightmentCost(Node * node);
	inline float Distance(const EERIE_3D & from, const EERIE_3D & to) const;
	unsigned long GetNearestNode(const EERIE_3D & pos) const;
	
	float radius;
	float height;
	float heuristic;
	unsigned long map_s; // Map size
	_ANCHOR_DATA * map_d; // Map data
	unsigned long slight_c, dlight_c; // Static and dynamic lights count
	EERIE_LIGHT ** slight_l, **dlight_l; // Static and dynamic lights data
	
	typedef std::vector<Node *> nodelist;
	nodelist open;
	nodelist close;
	
};

#endif // ARX_AI_PATHFINDER_H
