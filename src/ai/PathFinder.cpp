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

#include "ai/PathFinder.h"

#include <limits>
#include <algorithm>

#include <glm/gtx/norm.hpp>

#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/data/Mesh.h"
#include "math/Random.h"
#include "math/Vector.h"
#include "platform/Platform.h"
#include "physics/Anchors.h"
#include "scene/Light.h"

static const float MIN_RADIUS = 110.0f;

#define frnd() (1.0f - 2 * rnd())

const float PathFinder::HEURISTIC_MIN = 0.0f;
const float PathFinder::HEURISTIC_MAX = 0.5f;

const float PathFinder::HEURISTIC_DEFAULT = 0.5f;
const float PathFinder::RADIUS_DEFAULT = 0.0f;
const float PathFinder::HEIGHT_DEFAULT = 0.0f;

class PathFinder::Node {
	
	NodeId id;
	const Node * parent;
	
	float cost;
	float distance;
	
public:
	
	Node(long _id, const Node * _parent, float _distance, float _remaining)
		: id(_id), parent(_parent), cost(_distance + _remaining), distance(_distance) { }
	
	inline NodeId getId() const {
		return id;
	}
	
	inline const Node * getParent() const {
		return parent;
	}
	
	inline float getCost() const {
		return cost;
	}
	
	inline float getDistance() const {
		return distance;
	}
	
	inline void newParent(const Node * _parent, float _distance) {
		parent = _parent;
		cost = cost - distance + _distance;
		distance = _distance;
	}
	
};

class PathFinder::OpenNodeList {
	
	typedef std::vector<Node*> NodeList;
	NodeList nodes;
	
public:
	
	~OpenNodeList() {
		for(NodeList::iterator i = nodes.begin(); i != nodes.end(); ++i) {
			delete *i;
		}
	}
	
	/*!
	 * If a node with the same ID exists, update it.
	 * Otherwise add a new node.
	 * Assumes that remaining never changes for the same node id.
	 */
	inline void add(NodeId id, const Node * parent, float distance, float remaining) {
		
		// Check if node is already in open list.
		for(NodeList::iterator i = nodes.begin(); i != nodes.end(); ++i) {
			if((*i)->getId() == id) {
				if((*i)->getDistance() > distance) {
					(*i)->newParent(parent, distance);
				}
				return;
			}
		}
		
		nodes.push_back(new Node(id, parent, distance, remaining));
	}
	
	Node * extractBestNode() {
		// TODO use a better datastructure
		
		if(nodes.empty()) {
			return NULL;
		}
		
		NodeList::iterator best = nodes.begin();
		float cost = std::numeric_limits<float>::max();
		for(NodeList::iterator i = nodes.begin(); i != nodes.end(); ++i) {
			if((*i)->getCost() < cost) {
				cost = (*i)->getCost();
				best = i;
			}
		}
		
		Node * node = *best;
		nodes.erase(best);
		
		return node;
	}
	
};

class PathFinder::ClosedNodeList {
	
	typedef std::vector<Node*> NodeList;
	NodeList nodes;
	
public:
	
	~ClosedNodeList() {
		for(NodeList::iterator i = nodes.begin(); i != nodes.end(); ++i) {
			delete *i;
		}
	}
	
	void add(Node * node) {
		nodes.push_back(node);
	}
	
	bool contains(NodeId id) const {
		// TODO better datastructure: set of closed node ids
		for(NodeList::const_iterator i = nodes.begin(); i != nodes.end(); ++i) {
			if((*i)->getId() == id) {
				return true;
			}
		}
		return false;
	}
	
};

PathFinder::PathFinder(size_t map_size, const ANCHOR_DATA * map_data,
                       size_t slight_count, const EERIE_LIGHT * const * slight_list)
	: radius(RADIUS_DEFAULT), height(HEIGHT_DEFAULT), heuristic(HEURISTIC_DEFAULT),
	  map_s(map_size), map_d(map_data), slight_c(slight_count), slight_l(slight_list) { }

void PathFinder::setHeuristic(float _heuristic) {
	if(_heuristic >= HEURISTIC_MAX) {
		heuristic = HEURISTIC_MAX;
	} else if(_heuristic <= HEURISTIC_MIN) {
		heuristic = HEURISTIC_MIN;
	} else {
		heuristic = _heuristic;
	}
}

void PathFinder::setCylinder(float _radius, float _height) {
	radius = _radius;
	height = _height;
}

bool PathFinder::move(NodeId from, NodeId to, Result & rlist, bool stealth) const {
	
	if(from == to) {
		rlist.push_back(to);
		return true;
	}
	
	// Create start node and put it on open list
	Node * node = new Node(from, NULL, 0.0f, 0.0f);
	if(!node) {
		return false;
	}
	
	// A* main loop
	OpenNodeList open;
	ClosedNodeList close;
	do {
		
		// Put node onto close list as we have now examined this node.
		close.add(node);
		
		NodeId nid = node->getId();
		
		// If it's the goal node then we're done.
		if(nid == to) {
			buildPath(*node, rlist);
			return true;
		}
		
		// Otherwise, generate child from current node.
		for(short i = 0; i < map_d[nid].nblinked; i++) {
			
			NodeId cid = map_d[nid].linked[i];
			
			if((map_d[cid].flags & ANCHOR_FLAG_BLOCKED) || map_d[cid].height > height
			   || map_d[cid].radius < radius) {
				continue;
			}
			
			if(close.contains(cid)) {
				continue;
			}
			
			// Cost to reach this node.
			float distance = fdist(map_d[cid].pos, map_d[nid].pos);
			if(stealth) {
				distance += getIlluminationCost(map_d[cid].pos);
			}
			distance *= heuristic;
			distance += node->getDistance();
			
			// Estimated cost to get from this node to the destination.
			float remaining = (1.0f - heuristic) * fdist(map_d[cid].pos, map_d[to].pos);
			
			open.add(cid, node, distance, remaining);
		}
	
		node = open.extractBestNode();
	} while(node);
	
	// No path found!
	return false;
}

bool PathFinder::flee(NodeId from, const Vec3f & danger, float safeDist, Result & rlist,
                      bool stealth) const {
	
	static const float FLEE_DISTANCE_COST = 130.0F;
	
	if(!closerThan(map_d[from].pos, danger, safeDist)) {
		rlist.push_back(from);
		return true;
	}
	
	// Create start node and put it on open list
	Node * node = new Node(from, NULL, 0.0f, 0.0f);
	if(!node) {
		return false;
	}
	
	// A* main loop
	OpenNodeList open;
	ClosedNodeList close;
	do {
		
		// Put node onto close list as we have now examined this node.
		close.add(node);
		
		// If it's the goal node then we're done.
		if(node->getCost() == node->getDistance()) {
			buildPath(*node, rlist);
			return true;
		}
		
		NodeId nid = node->getId();
		
		// Otherwise, generate child from current node.
		for(short i(0); i < map_d[nid].nblinked; i++) {
			
			long cid = map_d[nid].linked[i];
			
			if((map_d[cid].flags & ANCHOR_FLAG_BLOCKED) || map_d[cid].height > height
			   || map_d[cid].radius < radius) {
				continue;
			}
			
			if(close.contains(cid)) {
				continue;
			}
			
			// Cost to reach this node.
			float distance = node->getDistance() + fdist(map_d[cid].pos, map_d[nid].pos);
			if(stealth) {
				distance += getIlluminationCost(map_d[cid].pos);
			}
			
			// Estimated cost to get from this node to the destination.
			float remaining = std::max(0.0f, safeDist - fdist(map_d[cid].pos, danger));
			remaining *= FLEE_DISTANCE_COST;
			
			open.add(cid, node, distance, remaining);
		}
		
		node = open.extractBestNode();
	} while(node);
	
	// No path found!
	return false;
}

bool PathFinder::wanderAround(NodeId from, float rad, Result & rlist, bool stealth) const {
	
	if(!map_d[from].nblinked) {
		return false;
	}
	
	if(rad <= MIN_RADIUS) {
		rlist.push_back(from);
		return true;
	}
	
	size_t s = rlist.size();
	
	NodeId last = from;
	
	unsigned int step_c = Random::get(4, 9);
	for(unsigned int i = 0; i < step_c; i++) {
		
		NodeId next = from;
		
		// Select the next node.
		unsigned int nb = Random::get(0, rad / 50);
		for(unsigned int j = 0; j < nb && map_d[next].nblinked; j++) {
			for(int notfinished = 0; notfinished < 4; notfinished++) {
				
				size_t r = Random::get(0, map_d[next].nblinked - 1);
				arx_assert(r < (size_t)map_d[next].nblinked);
				
				arx_assert(map_d[next].linked[r] >= 0);
				
				NodeId nid = map_d[next].linked[r];
				if((!(map_d[nid].flags & ANCHOR_FLAG_BLOCKED)) && (map_d[nid].nblinked)
				   && (map_d[nid].height <= height) && (map_d[nid].radius >= radius)) {
					next = nid;
					break;
				}
			}
		}
		
		if(!move(last, next, rlist, stealth)) {
			// Try again
			// TODO can cause infinite loop?
			i--;
		}
		
		last = next;
	}
	
	// Close wander around path (return to start position).
	if(rlist.size() == s || !move(last, from, rlist, stealth)) {
		return false;
	}
	
	return true;
}

PathFinder::NodeId PathFinder::getNearestNode(const Vec3f & pos) const {
	
	NodeId best = 0;
	float distance = std::numeric_limits<float>::max();
	
	for(size_t i = 0; i < map_s; i++) {
		float dist = glm::distance2(map_d[i].pos, pos);
		if(dist < distance && map_d[i].nblinked) {
			best = i;
			distance = dist;
		}
	}
	
	return best;
}

bool PathFinder::lookFor(NodeId from, const Vec3f & pos, float radius, Result & rlist,
                         bool stealth) const {
	
	if(radius <= MIN_RADIUS) {
		rlist.push_back(from);
		return true;
	}
	
	size_t s = rlist.size();
	
	NodeId to = getNearestNode(pos);
	
	NodeId last = from;
	
	unsigned long step_c = Random::get(4, 9);
	for(unsigned long i = 0; i < step_c; i++) {
		
		Vec3f pos = map_d[to].pos + randomVec(-1.f, 1.f) * radius;
		
		NodeId next = getNearestNode(pos);
		
		if(!move(last, next, rlist, stealth)) {
			// TODO can cause infinite loop?
			i--;
			last = next;
			continue;
		}
		
		last = next;
	}
	
	if(rlist.size() == s) {
		return false;
	}
	
	return true;
}

void PathFinder::buildPath(const Node & node, Result & rlist) {
	
	const Node * next = &node;
	
	size_t s = rlist.size();
	
	while(next) {
		rlist.push_back(next->getId());
		next = next->getParent();
	}
	
	std::reverse(rlist.begin() + s, rlist.end());
}

float PathFinder::getIlluminationCost(const Vec3f & pos) const {
	
	static const float STEALTH_LIGHT_COST = 300.0F;
	
	float cost = 0.0f;
	
	for(size_t i = 0; i < slight_c; i++) {
		
		if(!slight_l[i] || !slight_l[i]->exist || !slight_l[i]->status) {
			continue;
		}
		
		const EERIE_LIGHT & light = *slight_l[i];
		
		float dist = fdist(light.pos, pos);
		
		if(dist <= light.fallend) {
			
			float l_cost = STEALTH_LIGHT_COST;
			
			l_cost *= light.intensity * (light.rgb.r + light.rgb.g + light.rgb.b) * (1.0f / 3);
			
			if(dist > light.fallstart) {
				l_cost *= ((dist - light.fallstart) / (light.fallend - light.fallstart));
			}
			
			cost += l_cost;
		}
	}
	
	return cost;
}
