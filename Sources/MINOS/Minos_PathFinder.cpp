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
#include "Minos_PathFinder.h"
#include <Float.h>
#include <time.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

const Float MIN_RADIUS(110.F);

 
 
Float fac3(300.0F);

Float fac5(130.0F);

static const unsigned long SEED = 43;
static const unsigned long MODULO = 2147483647;
static const unsigned long FACTOR = 16807;
static const unsigned long SHIFT = 91;

static unsigned long __current(SEED);

static unsigned long Random()
{
	return __current = (__current * FACTOR + SHIFT) % MODULO;
}

ULong InitSeed()
{
	__current = (ULong)time(NULL);
	return Random();
}

#define frnd() (1.0F - 2 * rnd())

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Constructor and destructor                                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
// Default constructor                                                       //
PathFinder::PathFinder(const ULong & map_size, _ANCHOR_DATA * map_data,
                       const ULong & slight_count, EERIE_LIGHT ** slight_list,
                       const ULong & dlight_count, EERIE_LIGHT ** dlight_list) :
	heuristic(MINOS_DEFAULT_HEURISTIC),
	map_s(map_size), map_d(map_data),
	slight_c(slight_count), slight_l(slight_list),
	dlight_c(dlight_count), dlight_l(dlight_list),
	height(MINOS_DEFAULT_RADIUS), radius(MINOS_DEFAULT_HEIGHT)
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
void PathFinder::SetHeuristic(const Float & _heuristic)
{
	heuristic = _heuristic >= MINOS_HEURISTIC_MAX ? 0.5F : _heuristic < 0.0F ? MINOS_HEURISTIC_MIN : _heuristic;
}

void PathFinder::SetCylinder(const Float & _radius, const Float & _height)
{
	radius = _radius;
	height = _height;
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Methods                                                                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
UBool PathFinder::Move(const ULong & flags, const ULong & f, const ULong & t, SLong * rstep, UWord ** rlist)
{

	MINOSNode * node, *child;
	long _from, _to;

	//Init open and close lists
	Clean();

	if (!rlist || !rstep)	return UFALSE;

	if (f == t)
	{
		*rlist = (UWord *)malloc(sizeof(UWord));
		** rlist = (UWord)t;
		*rstep = 1;
		return UTRUE;
	}

	_from = f, _to = t;

	//Create start node and put it on open list
	if (!(node = CreateNode(_from, NULL)))
	{
		Clean(); // Cyril
		*rstep = 0;
		return UFALSE;
	}

	node->g_cost = 0;
	node->f_cost = Distance(map_d[_from].pos, map_d[_to].pos);

	if (flags & MINOS_STEALTH) AddEnlightmentCost(node);

	if (open.Append(node))
	{
		free(node);
		Clean(); // Cyril
		*rstep = 0;
		return UFALSE;
	}

	//A* main loop
	while (node = GetBestNode())
	{
		//If it's the goal node then we've done
		if (node->data == _to)
		{
			if (close.Append(node))
			{
				free(node);
				Clean(); 
				*rstep = 0;
				return UFALSE;
			}

			if (BuildPath(rlist, rstep))
			{
				Clean(); 
				*rstep = 0;
				return UFALSE;
			}

			Clean(); // Cyril
			return UTRUE;
		}

		//Otherwise, generate child from current node
		long _pipo(node->data);

		for (SWord i(0); i < map_d[_pipo].nblinked; i++)
		{
			//Create new child
			child = CreateNode(map_d[_pipo].linked[i], node);

			if (!child)
			{
				free(node);
				Clean(); // Cyril
				*rstep = 0;
				return UFALSE;
			}

			//Cost to reach this node
			if ((map_d[child->data].flags & ANCHOR_FLAG_BLOCKED) || map_d[child->data].height > height || map_d[child->data].radius < radius)
				free(child);
			else
			{
				child->g_cost = node->g_cost + Distance(map_d[child->data].pos, map_d[node->data].pos);

				if (flags & MINOS_STEALTH) AddEnlightmentCost(child);

				if (Check(child))
				{
					if (open.Append(child))
					{
						free(node);
						free(child);
						*rstep = 0;
						return UFALSE;
					}

					//Get total cost for this node
					child->f_cost = heuristic * child->g_cost + (1.0F - heuristic) * Distance(map_d[child->data].pos, map_d[_to].pos);
				}
				else free(child);
			}
		}

		//Put node onto close list as we have now examined this node
		if (close.Append(node))
		{
			free(node);
			Clean(); // Cyril
			*rstep = 0;
			return UFALSE;
		}
	}

	//No path found!!!
	Clean(); 
	*rstep = 0;
	return UFALSE;
}

UBool PathFinder::Flee(const ULong & flags, const ULong & f, const EERIE_3D & danger, const Float & safe_dist, SLong * rstep, UWord ** rlist)
{
	MINOSNode * node, *child;
	long _from;

	//Init open and close lists
	Clean();

	if (!rlist || !rstep)
		return UFALSE;

	if (Distance(map_d[f].pos, danger) >= safe_dist)
	{
		*rlist = (UWord *)malloc(sizeof(UWord));
		** rlist = (UWord)f;
		*rstep = 1;
		return UTRUE;
	}

	_from = f;

	//Create start node and put it on open list
	if (!(node = CreateNode(_from, NULL)))
	{
		Clean(); 
		*rstep = 0;
		return UFALSE;
	}

	node->g_cost = 0;

	if (flags & MINOS_STEALTH)
		AddEnlightmentCost(node);

	node->f_cost = safe_dist - Distance(map_d[_from].pos, danger);

	if (node->f_cost < 0.0F)
		node->f_cost = 0.0F;

	node->f_cost += node->g_cost;

	if (open.Append(node))
	{
		free(node);
		Clean(); 
		*rstep = 0;
		return UFALSE;
	}

	//A* main loop
	while (node = GetBestNode())
	{
		//If it's the goal node then we've done
		if (Distance(map_d[node->data].pos, danger) >= safe_dist)
		{
			if (close.Append(node))
			{
				free(node);
				*rstep = 0;
				return UFALSE;
			}

			//BuildPath(rlist, rstep);
			if (BuildPath(rlist, rstep))
			{
				Clean(); 
				*rstep = 0;
				return UFALSE;
			}

			Clean(); 
			return UTRUE;
		}

		//Otherwise, generate child from current node
		long _pipo = node->data;

		for (SWord i(0); i < map_d[_pipo].nblinked; i++)
		{
			child = CreateNode(map_d[_pipo].linked[i], node);

			if (!child)
			{
				free(node);
				Clean(); 
				*rstep = 0;
				return UFALSE;
			}

			//Cost to reach this node
			if ((map_d[child->data].flags & ANCHOR_FLAG_BLOCKED) || map_d[child->data].height > height || map_d[child->data].radius < radius)
				free(child);
			else
			{
				child->g_cost = node->g_cost + Distance(map_d[child->data].pos, map_d[node->data].pos);

				if (flags & MINOS_STEALTH)
					AddEnlightmentCost(child);

				if (Check(child))
				{
					Float dist;

					if (open.Append(child))
					{
						free(node);
						free(child);
						*rstep = 0;
						return UFALSE;
					}

					//Get total cost for this node
					child->f_cost = child->g_cost;
					dist = Distance(map_d[child->data].pos, danger);

					if ((dist = safe_dist - dist) > 0.0F)
						child->f_cost += fac5 * dist;
				}
				else free(child);
			}
		}

		//Put node onto close list as we have now examined this node
		if (close.Append(node))
		{
			free(node);
			Clean(); 
			*rstep = 0;
			return UFALSE;
		}
	}

	Clean(); 
	*rstep = 0;
	
	//No path found!!!
	return UFALSE;
}

UBool PathFinder::WanderAround(const ULong & flags, const ULong & f, const Float & rad, SLong * rstep, UWord ** rlist)
{
	
	Void * ptr;
	ULong step_c, last, next;
	SLong temp_c(0), path_c(0);
	UWord * temp_d = NULL, *path_d = NULL;

	Clean(); 
	//Check if params are valid
	if (!rlist || !rstep) return UFALSE;

	if (!map_d[f].nblinked)
	{
		*rstep = 0;
		return UFALSE;
	}

	if (rad <= MIN_RADIUS)
	{
		*rlist = (UWord *)malloc(sizeof(UWord));
		** rlist = (UWord)f;
		*rstep = 1;
		return UTRUE;
	}

	last = f;

	step_c = Random() % 5 + 5;

	for (ULong i(0); i < step_c; i++)
	{
		ULong nb = ULong(rad * rnd() * DIV50);
		long _current = f;

		while (nb)
		{
			if ((map_d[_current].nblinked)
			   )
			{

				long notfinished = 4;

				while (notfinished--)
				{
					ULong r = ULong(rnd() * (Float)map_d[_current].nblinked);

					if (r >= (ULong)map_d[_current].nblinked)
						r = ULong(map_d[_current].nblinked - 1);

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
			if (!(ptr = realloc(path_d, sizeof(UWord) * (path_c + temp_c))))
			{
				free(temp_d);
				free(path_d);
				Clean(); 
				*rstep = 0;
				return UFALSE;
			}

			//Add temp path to wander around path
			path_d = (UWord *)ptr;
			memcpy(&path_d[path_c], temp_d, sizeof(UWord) * temp_c);
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
		return UFALSE;
	}

	if (!(ptr = realloc(path_d, sizeof(UWord) * (path_c + temp_c))))
	{
		free(temp_d);
		free(path_d);
		Clean(); 
		*rstep = 0;
		return UFALSE;
	}

	//Add temp path to wander around path
	path_d = (UWord *)ptr;
	memcpy(&path_d[path_c], temp_d, sizeof(UWord) * temp_c);
	path_c += temp_c;

	free(temp_d);

	*rlist = path_d;
	*rstep = path_c;
	Clean(); 
	return UTRUE;
}

ULong PathFinder::GetNearestNode(const EERIE_3D & pos) const
{
	ULong best(0);
	Float dist, b_dist(FLT_MAX);

	for (ULong i(0); i < map_s; i++)
	{
		dist = Distance(map_d[i].pos, pos);

		if (dist < b_dist && map_d[i].nblinked) best = i, b_dist = dist;
	}

	return best;
}

UBool PathFinder::LookFor(const ULong & flags, const ULong & f, const EERIE_3D & pos, const Float & radius, SLong * rstep, UWord ** rlist)
{
	Void * ptr;
	ULong step_c, to, last, next;
	SLong temp_c(0), path_c(0);
	UWord * temp_d = NULL, *path_d = NULL;

	Clean(); 
	//Check if params are valid
	if (!rlist || !rstep)
	{
		Clean();
		*rstep = 0;
		return UFALSE;
	}

	if (radius <= MIN_RADIUS)
	{
		*rlist = (UWord *)malloc(sizeof(UWord));
		** rlist = (UWord)f;
		*rstep = 1;
		Clean();
		return UTRUE;
	}

	to = GetNearestNode(pos);

	last = f;

	step_c = Random() % 5 + 5;

	for (ULong i(0); i < step_c; i++)
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
				return UFALSE;
			}

			if (!(ptr = realloc(path_d, sizeof(UWord) * (path_c + temp_c - 1))))
			{
				if (temp_d) 
				{
					free(temp_d);
					temp_d = NULL;
				}

				Clean(); 
				*rstep = 0;
				return UFALSE;
			}

			//Add temp path to wander around path
			path_d = (UWord *)ptr;
			memcpy(&path_d[path_c], temp_d, sizeof(UWord) *(temp_c - 1));
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
		return UFALSE;
	}

	*rlist = path_d;
	*rstep = path_c;
	Clean(); // Cyril
	return UTRUE;
}

Void PathFinder::Clean()
{
	ULong i;

	for (i = 0; i < close.Count(); i++) free(close[i]);

	close.Free();

	for (i = 0; i < open.Count(); i++) free(open[i]);

	open.Free();
}

// Return best node (lower cost) from open list or NULL if list is empty
MINOSNode * PathFinder::GetBestNode()
{
	MINOSNode * node;
	ULong best(0);
	Float cost(FLT_MAX);

	if (!open.Count()) return NULL;

	for (ULong i(0); i < open.Count(); i++)
		if (open[i]->f_cost < cost) cost = open[i]->f_cost, best = i;

	node = open[best];
	open.Remove(best);

	return node;
}

UBool PathFinder::Check(MINOSNode * node)
{
	ULong i;

	//Check if node is already in close list
	for (i = 0; i < close.Count(); i++)
		if (close[i]->data == node->data) return UFALSE;

	//Check if node is already in open list
	for (i = 0; i < open.Count(); i++)
		if (open[i]->data == node->data)
		{
			if (open[i]->g_cost < node->g_cost) return UFALSE;

			free(open[i]), open.Remove(i);
		}

	return UTRUE;
}

SBool PathFinder::BuildPath(UWord ** rlist, SLong * rstep)
{
	Void * ptr;
	MINOSNode * next;
	UWord path_c(0);
	UWord * path_d = NULL;

	next = close[close.Count() - 1];

	while (next)
	{
		if (!(ptr = realloc(path_d, (path_c + 1) << 1))) return SFALSE;

		path_d = (UWord *)ptr;
		path_d[path_c++] = (UWord)next->data;
		next = next->parent;
	}

	if (!rlist || !(*rlist = (UWord *)malloc(sizeof(UWord) * path_c)))
	{
		free(path_d);
		return SFALSE;
	}

	for (ULong i(0); i < path_c; i++)(*rlist)[i] = path_d[path_c - i - 1];

	free(path_d);

	if (rstep) *rstep = path_c;

	return STRUE;
}

MINOSNode * PathFinder::CreateNode(long data, MINOSNode * parent)
{
	MINOSNode * node;

	node = (MINOSNode *)malloc(sizeof(MINOSNode));

	if (!node) return NULL;

	node->data = data;
	node->parent = parent;

	return node;
}

Void PathFinder::AddEnlightmentCost(MINOSNode * node)
{
	for (ULong i(0); i < slight_c; i++)
	{
		if (!slight_l[i] || !slight_l[i]->exist || !slight_l[i]->status) continue;

		Float dist = Distance(slight_l[i]->pos, map_d[node->data].pos);

		if (slight_l[i]->fallend >= dist)
		{
			Float l_cost(fac3);

			l_cost *= slight_l[i]->intensity * (slight_l[i]->rgb.r + slight_l[i]->rgb.g + slight_l[i]->rgb.b) * DIV3;

			if (slight_l[i]->fallstart >= dist)
				node->g_cost += l_cost;
			else
				node->g_cost += l_cost * ((dist - slight_l[i]->fallstart) / (slight_l[i]->fallend - slight_l[i]->fallstart));
		}
	}
}

inline Float PathFinder::Distance(const EERIE_3D & from, const EERIE_3D & to) const
{
	Float x, y, z;

	x = from.x - to.x;
	y = from.y - to.y;
	z = from.z - to.z;

	return Float(EEsqrt(x * x + y * y + z * z));
}
