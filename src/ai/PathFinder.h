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

#ifndef ARX_AI_PATHFINDER_H
#define ARX_AI_PATHFINDER_H

#include <stddef.h>
#include <vector>

#include "math/Types.h"

struct ANCHOR_DATA;
struct EERIE_LIGHT;


class PathFinder {
	
public:
	
	static const float HEURISTIC_MIN;
	static const float HEURISTIC_MAX;
	
	static const float HEURISTIC_DEFAULT;
	static const float RADIUS_DEFAULT;
	static const float HEIGHT_DEFAULT;
	
	/*!
	 * Create a PathFinder instance for the provided data.
	 * The pathfinder instance does not copy the provided data and will not clean it up
	 * The light data is only used when the stealth parameter is set to true.
	 */
	PathFinder(size_t graphSize, const ANCHOR_DATA * graph,
	           size_t lightCount, const EERIE_LIGHT * const * lights);
	
	typedef unsigned long NodeId;
	typedef std::vector<NodeId> Result;
	
	/*!
	 * Set a heuristic for selecting the best next node.
	 * For 0.0f only the distance to the target will be considered.
	 * For 0.5f the distance to target is weigted equaly to the already traversed distance + light costs.
	 * This is not used for flee().
	 * The default value is HEURISTIC_DEFAULT.
	 */
	void setHeuristic(float heuristic);
	
	/*!
	 * Set a cylinder to constrain the search space.
	 * The default values are RADIUS_DEFAULT and HEIGHT_DEFAULT.
	 */
	void setCylinder(float radius, float height);
	
	/*!
	 * Find a path between two nodes.
	 * \param from The index of the start node into the provided map_data.
	 * \param to The index of the destination node into the provided map_data.
	 * \param rlist A list to append the path to.
	 * \param stealth True if the path should avoid light sources.
	 * \return true if a path was found.
	 */
	bool move(NodeId from, NodeId to, Result & rlist, bool stealth = false) const;
	
	/*!
	 * Find a path away from a position.
	 * \param from The index of the start node into the provided map_data.
	 * \param danger The position to get away from.
	 * \param safeDistance How far to get away from danger.
	 * \param rlist A list to append the path to.
	 * \param stealth True if the path should avoid light sources.
	 * \return true if a path was found.
	 */
	bool flee(NodeId from, const Vec3f & danger, float safeDistance, Result & rlist, bool stealth = false) const;
	
	/*!
	 * Wander around and then return to the start node.
	 * \param from The index of the start node into the provided map_data.
	 * \param aroundRadius How far to wander.
	 * \param rlist A list to append the path to.
	 * \param stealth True if the path should avoid light sources.
	 * \return true if a path was found.
	 */
	bool wanderAround(NodeId from, float aroundRadius, Result & rlist, bool stealth = false) const;
	
	/*!
	 * Walk to and then to random offsets around the given position
	 * \param from The index of the start node into the provided map_data.
	 * \param pos The position to walk to.
	 * \param radius How far to walk around the given position.
	 * \param rlist A list to append the path to.
	 * \param stealth True if the path should avoid light sources.
	 * \return true if a path was found.
	 */
	bool lookFor(NodeId from, const Vec3f & pos, float radius, Result & rlist, bool stealth = false) const;
	
private:
	
	class Node;
	class OpenNodeList;
	class ClosedNodeList;
	
	static void buildPath(const Node & node, Result & rlist);
	float getIlluminationCost(const Vec3f & pos) const;
	NodeId getNearestNode(const Vec3f & pos) const;
	
	float m_radius;
	float m_height;
	float m_heuristic;
	
	size_t map_s; // Map size
	const ANCHOR_DATA * map_d; // Map data
	size_t slight_c; // Light count
	const EERIE_LIGHT * const * slight_l; // Light data
	
};

#endif // ARX_AI_PATHFINDER_H
