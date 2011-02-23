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

#include "ai/PathFinder.h"

#include <ctime>

#include "graphics/Math.h"
#include "graphics/data/Mesh.h"

#include "scene/Interactive.h"

struct PathFinder::Node {
	
	long data;
	float g_cost;
	float f_cost;
	Node * parent;
	
	Node(long _data, Node * _parent) : data(_data), parent(_parent) { }
	
};

const float MIN_RADIUS(110.F);

float fac3(300.0F);
float fac5(130.0F);

static const unsigned long SEED = 43;
static const unsigned long MODULO = 2147483647;
static const unsigned long FACTOR = 16807;
static const unsigned long SHIFT = 91;

static unsigned long __current(SEED);

static unsigned long Random()
{
	return __current = (__current * FACTOR + SHIFT) % MODULO;
}

unsigned long InitSeed()
{
	__current = (unsigned long)time(NULL);
	return Random();
}

#define frnd() (1.0F - 2 * rnd())

PathFinder::PathFinder(unsigned long map_size, _ANCHOR_DATA * map_data,
                       unsigned long slight_count, EERIE_LIGHT ** slight_list,
                       unsigned long dlight_count, EERIE_LIGHT ** dlight_list) :
	radius(MINOS_DEFAULT_RADIUS),
	height(MINOS_DEFAULT_HEIGHT),
	heuristic(MINOS_DEFAULT_HEURISTIC),
	map_s(map_size),
	map_d(map_data),
	slight_c(slight_count),
	dlight_c(dlight_count),
	slight_l(slight_list),
	dlight_l(dlight_list)
{
	InitSeed();
}

// Destructor                                                                //
PathFinder::~PathFinder()
{
	Clean();
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Setup                                                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
void PathFinder::SetHeuristic(float _heuristic)
{
	heuristic = _heuristic >= MINOS_HEURISTIC_MAX ? 0.5F : _heuristic < 0.0F ? MINOS_HEURISTIC_MIN : _heuristic;
}

void PathFinder::SetCylinder(float _radius, float _height)
{
	radius = _radius;
	height = _height;
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Methods                                                                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
bool PathFinder::Move(unsigned long flags, unsigned long f, unsigned long t, long * rstep, unsigned short ** rlist)
{

	Node * node, *child;
	long _from, _to;

	//Init open and close lists
	Clean();

	if (!rlist || !rstep)	return false;

	if (f == t)
	{
		*rlist = (unsigned short *)malloc(sizeof(unsigned short));
		** rlist = (unsigned short)t;
		*rstep = 1;
		return true;
	}

	_from = f, _to = t;

	//Create start node and put it on open list
	if (!(node = new Node(_from, NULL)))
	{
		Clean(); // Cyril
		*rstep = 0;
		return false;
	}

	node->g_cost = 0;
	node->f_cost = Distance(map_d[_from].pos, map_d[_to].pos);

	if (flags & MINOS_STEALTH) AddEnlightmentCost(node);

	open.push_back(node);
	// TODO on error {
	//	free(node);
	//	Clean(); // Cyril
	//	*rstep = 0;
	//	return false;
	//}

	//A* main loop
	while ((node = GetBestNode()))
	{
		//If it's the goal node then we've done
		if (node->data == _to)
		{
			close.push_back(node);
			// TODO on error {
			//	free(node);
			//	Clean(); 
			//	*rstep = 0;
			//	return false;
			//}

			if (BuildPath(rlist, rstep))
			{
				Clean(); 
				*rstep = 0;
				return false;
			}

			Clean(); // Cyril
			return true;
		}

		//Otherwise, generate child from current node
		long _pipo(node->data);

		for (short i(0); i < map_d[_pipo].nblinked; i++)
		{
			//Create new child
			child = new Node(map_d[_pipo].linked[i], node);

			if (!child)
			{
				delete node;
				Clean(); // Cyril
				*rstep = 0;
				return false;
			}

			//Cost to reach this node
			if ((map_d[child->data].flags & ANCHOR_FLAG_BLOCKED) || map_d[child->data].height > height || map_d[child->data].radius < radius)
				delete child;
			else
			{
				child->g_cost = node->g_cost + Distance(map_d[child->data].pos, map_d[node->data].pos);

				if (flags & MINOS_STEALTH) AddEnlightmentCost(child);

				if (Check(child))
				{
					open.push_back(child);
					// TODO on error {
					//	free(node);
					//	free(child);
					//	*rstep = 0;
					//	return false;
					//}

					//Get total cost for this node
					child->f_cost = heuristic * child->g_cost + (1.0F - heuristic) * Distance(map_d[child->data].pos, map_d[_to].pos);
				}
				else {
					delete child;
				}
			}
		}

		//Put node onto close list as we have now examined this node
		close.push_back(node);
		// TODO on error {
		//	free(node);
		//	Clean(); // Cyril
		//	*rstep = 0;
		//	return false;
		//}
	}

	//No path found!!!
	Clean(); 
	*rstep = 0;
	return false;
}

bool PathFinder::Flee(unsigned long flags, unsigned long f, const EERIE_3D & danger, float safe_dist, long * rstep, unsigned short ** rlist)
{
	Node * node, *child;
	long _from;

	//Init open and close lists
	Clean();

	if (!rlist || !rstep)
		return false;

	if (Distance(map_d[f].pos, danger) >= safe_dist)
	{
		*rlist = (unsigned short *)malloc(sizeof(unsigned short));
		** rlist = (unsigned short)f;
		*rstep = 1;
		return true;
	}

	_from = f;

	//Create start node and put it on open list
	if (!(node = new Node(_from, NULL)))
	{
		Clean(); 
		*rstep = 0;
		return false;
	}

	node->g_cost = 0;

	if (flags & MINOS_STEALTH)
		AddEnlightmentCost(node);

	node->f_cost = safe_dist - Distance(map_d[_from].pos, danger);

	if (node->f_cost < 0.0F)
		node->f_cost = 0.0F;

	node->f_cost += node->g_cost;

	open.push_back(node);
	// TODO on error {
	//	free(node);
	//	Clean(); 
	//	*rstep = 0;
	//	return false;
	//}

	//A* main loop
	while ((node = GetBestNode()))
	{
		//If it's the goal node then we've done
		if (Distance(map_d[node->data].pos, danger) >= safe_dist)
		{
			close.push_back(node);
			// TODO on error {
			//	free(node);
			//	*rstep = 0;
			//	return false;
			//}

			if (!BuildPath(rlist, rstep))
			{
				Clean(); 
				*rstep = 0;
				return false;
			}

			Clean(); 
			return true;
		}

		//Otherwise, generate child from current node
		long _pipo = node->data;

		for (short i(0); i < map_d[_pipo].nblinked; i++)
		{
			child = new Node(map_d[_pipo].linked[i], node);

			if (!child)
			{
				delete node;
				Clean(); 
				*rstep = 0;
				return false;
			}

			//Cost to reach this node
			if ((map_d[child->data].flags & ANCHOR_FLAG_BLOCKED) || map_d[child->data].height > height || map_d[child->data].radius < radius)
				delete child;
			else
			{
				child->g_cost = node->g_cost + Distance(map_d[child->data].pos, map_d[node->data].pos);

				if (flags & MINOS_STEALTH)
					AddEnlightmentCost(child);

				if (Check(child))
				{
					float dist;

					open.push_back(child);
					// TODO on error {
					//	free(node);
					//	free(child);
					//	*rstep = 0;
					//	return false;
					//}

					//Get total cost for this node
					child->f_cost = child->g_cost;
					dist = Distance(map_d[child->data].pos, danger);

					if ((dist = safe_dist - dist) > 0.0F)
						child->f_cost += fac5 * dist;
				}
				else {
					delete child;
				}
			}
		}

		//Put node onto close list as we have now examined this node
		close.push_back(node);
		// TODO on error {
		//	free(node);
		//	Clean(); 
		//	*rstep = 0;
		//	return false;
		//}
	}

	Clean(); 
	*rstep = 0;
	
	//No path found!!!
	return false;
}

bool PathFinder::WanderAround(unsigned long flags, unsigned long f, float rad, long * rstep, unsigned short ** rlist)
{
	
	void * ptr;
	unsigned long step_c, last, next;
	long temp_c(0), path_c(0);
	unsigned short * temp_d = NULL, *path_d = NULL;

	Clean(); 
	//Check if params are valid
	if (!rlist || !rstep) return false;

	if (!map_d[f].nblinked)
	{
		*rstep = 0;
		return false;
	}

	if (rad <= MIN_RADIUS)
	{
		*rlist = (unsigned short *)malloc(sizeof(unsigned short));
		** rlist = (unsigned short)f;
		*rstep = 1;
		return true;
	}

	last = f;

	step_c = Random() % 5 + 5;

	for (unsigned long i(0); i < step_c; i++)
	{
		unsigned long nb = (unsigned long)(rad * rnd() * ( 1.0f / 50 ));
		long _current = f;

		while (nb)
		{
			if ((map_d[_current].nblinked)
			   )
			{

				long notfinished = 4;

				while (notfinished--)
				{
					unsigned long r = (unsigned long)(rnd() * (float)map_d[_current].nblinked);

					if (r >= (unsigned long)map_d[_current].nblinked)
						r = (unsigned long)(map_d[_current].nblinked - 1);

					if ((!(map_d[map_d[_current].linked[r]].flags & ANCHOR_FLAG_BLOCKED))
					        &&	(map_d[map_d[_current].linked[r]].nblinked)
					        &&	(map_d[map_d[_current].linked[r]].height <= height)
					        &&	(map_d[map_d[_current].linked[r]].radius >= radius))
					{
						_current = map_d[_current].linked[r];
						notfinished = 0;
					}
				}
			}

			nb--;
		}

		if (_current < 0) continue;

		next = nb = _current;

		if (Move(flags, last, next, &temp_c, &temp_d) && temp_c)
		{
			if (!(ptr = realloc(path_d, sizeof(unsigned short) * (path_c + temp_c))))
			{
				free(temp_d);
				free(path_d);
				Clean(); 
				*rstep = 0;
				return false;
			}

			//Add temp path to wander around path
			path_d = (unsigned short *)ptr;
			memcpy(&path_d[path_c], temp_d, sizeof(unsigned short) * temp_c);
			path_c += temp_c;

			//Free temp path
			free(temp_d), temp_d = NULL, temp_c = 0;
		}
		else i--;

		last = next;
	}

	//Close wander around path (return to start position)
	if (!path_c || !Move(flags, last, f, &temp_c, &temp_d))
	{
		*rstep = 0;
		return false;
	}

	if (!(ptr = realloc(path_d, sizeof(unsigned short) * (path_c + temp_c))))
	{
		free(temp_d);
		free(path_d);
		Clean(); 
		*rstep = 0;
		return false;
	}

	//Add temp path to wander around path
	path_d = (unsigned short *)ptr;
	memcpy(&path_d[path_c], temp_d, sizeof(unsigned short) * temp_c);
	path_c += temp_c;

	free(temp_d);

	*rlist = path_d;
	*rstep = path_c;
	Clean(); 
	return true;
}

unsigned long PathFinder::GetNearestNode(const EERIE_3D & pos) const
{
	unsigned long best(0);
	float dist, b_dist(FLT_MAX);

	for (unsigned long i(0); i < map_s; i++)
	{
		dist = Distance(map_d[i].pos, pos);

		if (dist < b_dist && map_d[i].nblinked) best = i, b_dist = dist;
	}

	return best;
}

bool PathFinder::LookFor(unsigned long flags, unsigned long f, const EERIE_3D & pos, float radius, long * rstep, unsigned short ** rlist)
{
	void * ptr;
	unsigned long step_c, to, last, next;
	long temp_c(0), path_c(0);
	unsigned short * temp_d = NULL, *path_d = NULL;

	Clean(); 
	//Check if params are valid
	if (!rlist || !rstep)
	{
		Clean();
		*rstep = 0;
		return false;
	}

	if (radius <= MIN_RADIUS)
	{
		*rlist = (unsigned short *)malloc(sizeof(unsigned short));
		** rlist = (unsigned short)f;
		*rstep = 1;
		Clean();
		return true;
	}

	to = GetNearestNode(pos);

	last = f;

	step_c = Random() % 5 + 5;

	for (unsigned long i(0); i < step_c; i++)
	{
		EERIE_3D pos;

		pos.x = map_d[to].pos.x + radius * frnd();
		pos.y = map_d[to].pos.y + radius * frnd();
		pos.z = map_d[to].pos.z + radius * frnd();
		next = GetNearestNode(pos);

		if (Move(flags, last, next, &temp_c, &temp_d) && temp_c)
		{
			if ((path_c + temp_c - 1) <= 0)
			{
				if (temp_d) 
				{
					free(temp_d);
					temp_d = NULL;
				}

				if (path_d)
				{
					free(path_d);
					path_d = NULL;
				}

				Clean(); 
				*rstep = 0;
				return false;
			}

			if (!(ptr = realloc(path_d, sizeof(unsigned short) * (path_c + temp_c - 1))))
			{
				if (temp_d) 
				{
					free(temp_d);
					temp_d = NULL;
				}

				Clean(); 
				*rstep = 0;
				return false;
			}

			//Add temp path to wander around path
			path_d = (unsigned short *)ptr;
			memcpy(&path_d[path_c], temp_d, sizeof(unsigned short) *(temp_c - 1));
			path_c += temp_c - 1;

			//Free temp path
			free(temp_d), temp_d = NULL, temp_c = 0;
		}
		else i--;

		last = next;
	}

	//Close wander around path (return to start position)
	if (!path_c)
	{
		Clean(); // Cyril
		*rstep = 0;
		return false;
	}

	*rlist = path_d;
	*rstep = path_c;
	Clean(); // Cyril
	return true;
}

void PathFinder::Clean() {
	
	unsigned long i;
	
	for (i = 0; i < close.size(); i++) {
		delete close[i];
	}
	close.clear();
	
	for(i = 0; i < open.size(); i++) {
		delete open[i];
	}
	open.clear();
}

// Return best node (lower cost) from open list or NULL if list is empty
PathFinder::Node * PathFinder::GetBestNode()
{
	Node * node;
	nodelist::iterator best = open.begin();
	float cost(FLT_MAX);

	if (!open.size()) return NULL;

	for(nodelist::iterator i = open.begin(); i != open.end(); i++) {
		if((*i)->f_cost < cost) {
			cost = (*i)->f_cost;
			best = i;
		}
	}
	
	node = *best;
	open.erase(best);
	
	return node;
}

bool PathFinder::Check(Node * node) {
	
	// TODO use set/map instead of vector?
	
	//Check if node is already in close list
	for(nodelist::const_iterator i = close.begin(); i != close.end(); ++i) {
		if((*i)->data == node->data) return false;
	}
	
	//Check if node is already in open list
	for(nodelist::iterator i = open.begin(); i != open.end();) {
		if((*i)->data == node->data) {
			if((*i)->g_cost < node->g_cost) {
				return false;
			}
			
			delete *i;
			i = open.erase(i);
		} else {
			++i;
		}
	}

	return true;
}

bool PathFinder::BuildPath(unsigned short ** rlist, long * rstep) {
	
	void * ptr;
	Node * next;
	unsigned short path_c(0);
	unsigned short * path_d = NULL;

	next = close[close.size() - 1];

	while (next)
	{
		if (!(ptr = realloc(path_d, (path_c + 1) << 1))) return false;

		path_d = (unsigned short *)ptr;
		path_d[path_c++] = (unsigned short)next->data;
		next = next->parent;
	}

	if (!rlist || !(*rlist = (unsigned short *)malloc(sizeof(unsigned short) * path_c)))
	{
		free(path_d);
		return false;
	}

	for (unsigned long i(0); i < path_c; i++)(*rlist)[i] = path_d[path_c - i - 1];

	free(path_d);

	if (rstep) *rstep = path_c;

	return true;
}



void PathFinder::AddEnlightmentCost(Node * node)
{
	for (unsigned long i(0); i < slight_c; i++)
	{
		if (!slight_l[i] || !slight_l[i]->exist || !slight_l[i]->status) continue;

		float dist = Distance(slight_l[i]->pos, map_d[node->data].pos);

		if (slight_l[i]->fallend >= dist)
		{
			float l_cost(fac3);

			l_cost *= slight_l[i]->intensity * (slight_l[i]->rgb.r + slight_l[i]->rgb.g + slight_l[i]->rgb.b) * ( 1.0f / 3 );

			if (slight_l[i]->fallstart >= dist)
				node->g_cost += l_cost;
			else
				node->g_cost += l_cost * ((dist - slight_l[i]->fallstart) / (slight_l[i]->fallend - slight_l[i]->fallstart));
		}
	}
}

inline float PathFinder::Distance(const EERIE_3D & from, const EERIE_3D & to) const
{
	float x, y, z;

	x = from.x - to.x;
	y = from.y - to.y;
	z = from.z - to.z;

	return EEsqrt(x * x + y * y + z * z);
}
