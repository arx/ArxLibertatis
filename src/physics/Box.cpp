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
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// EERIEPhysicsBox
///////////////////////////////////////////////////////////////////////////////
//
// Description:
//	Provides funcs for 3D Object Physics (Box Based)
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
///////////////////////////////////////////////////////////////////////////////

#include "physics/Box.h"

#include "ai/Paths.h"
#include "core/Core.h"
#include "game/NPC.h"
#include "graphics/Math.h"
#include "physics/Collisions.h"
#include "scene/Interactive.h"

using std::min;
using std::max;

//-----------------------------------------------------------------------------
float VELOCITY_THRESHOLD = 850.f;
#define FULLTESTS 0

long CUR_COLLISION_MATERIAL = 0;

//*************************************************************************************
// Used to launch an object into the physical world...
//*************************************************************************************
void EERIE_PHYSICS_BOX_Launch(EERIE_3DOBJ * obj, Vec3f * pos, Vec3f * vect, long flag, Anglef * angle)
{
	if ((!obj) || !(obj->pbox)) return;

	obj->pbox->storedtiming = 0;

	for (size_t i = 0; i < obj->vertexlist.size(); i++)
	{
		obj->vertexlist[i].vert.sx = obj->vertexlist[i].v.x;
		obj->vertexlist[i].vert.sy = obj->vertexlist[i].v.y;
		obj->vertexlist[i].vert.sz = obj->vertexlist[i].v.z;
	}

	float surface = 0.f;

	for (size_t i = 0; i < obj->facelist.size(); i++)
	{

		TexturedVertex * ev[3];
		ev[0] = (TexturedVertex *)&obj->vertexlist[obj->facelist[i].vid[0]].v;
		ev[1] = (TexturedVertex *)&obj->vertexlist[obj->facelist[i].vid[1]].v;
		ev[2] = (TexturedVertex *)&obj->vertexlist[obj->facelist[i].vid[2]].v;
		surface += TRUEDistance3D((ev[0]->sx + ev[1]->sx) * ( 1.0f / 2 ),
		                          (ev[0]->sy + ev[1]->sy) * ( 1.0f / 2 ),
		                          (ev[0]->sz + ev[1]->sz) * ( 1.0f / 2 ),
		                          ev[2]->sx, ev[2]->sy, ev[2]->sz)
		           * TRUEDistance3D(ev[0]->sx, ev[0]->sy, ev[0]->sz,
		                            ev[1]->sx, ev[1]->sy, ev[1]->sz) * ( 1.0f / 2 );
	}

	float ratio = surface * ( 1.0f / 10000 );

	if (ratio > 0.8f) ratio = 0.8f;
	else if (ratio < 0.f) ratio = 0.f;

	ratio = 1.f - (ratio * ( 1.0f / 4 ));

	for (int i = 0; i < obj->pbox->nb_physvert; i++)
	{
		PHYSVERT * pv = &obj->pbox->vert[i];
		pv->pos.x = pv->initpos.x + pos->x;
		pv->pos.y = pv->initpos.y + pos->y;
		pv->pos.z = pv->initpos.z + pos->z;

		pv->inertia.x = 0.f;
		pv->inertia.y = 0.f;
		pv->inertia.z = 0.f;


		pv->force.x = 0.f;
		pv->force.y = 0.f;
		pv->force.z = 0.f;

		pv->velocity.x = vect->x * 250.f * ratio;
		pv->velocity.y = vect->y * 250.f * ratio; 
		pv->velocity.z = vect->z * 250.f * ratio;

		pv->mass = 0.4f + ratio * 0.1f; 

		if (flag)
		{
			Vector_RotateY(&pv->pos, &pv->initpos, angle->b);
			pv->pos.x += pos->x;
			pv->pos.y += pos->y;
			pv->pos.z += pos->z;
		}

	}

	obj->pbox->active = 1;
	obj->pbox->stopcount = 0;
}

//*************************************************************************************
//*************************************************************************************
bool IsValidPos3(Vec3f * pos)
{
	long px, pz;
	px = pos->x * ACTIVEBKG->Xmul;

	if (px >= ACTIVEBKG->Xsize)
	{
		return false;
	}

	if (px < 0)
	{
		return false;
	}

	pz = pos->z * ACTIVEBKG->Zmul;

	if (pz >= ACTIVEBKG->Zsize)
	{
		return false;
	}

	if (pz < 0)
	{
		return false;
	}

	EERIE_BKG_INFO * eg;

	eg = &ACTIVEBKG->Backg[px+pz*ACTIVEBKG->Xsize];

	if (eg->nbpolyin <= 0)
		return false;

	if (pos->y > eg->tile_maxy)
		return false;

	return true;
}

//*************************************************************************************
// Apply spring force on two physical vertexes
//*************************************************************************************
void ApplySpring(EERIE_3DOBJ * obj, long k, long l, float PHYSICS_constant, float PHYSICS_Damp)
{
	Vec3f deltaP, deltaV, springforce;
	PHYSVERT * pv_k = &obj->pbox->vert[k];
	PHYSVERT * pv_l = &obj->pbox->vert[l];
	float Dterm, Hterm;

	float restlength = dist(obj->pbox->vert[k].initpos, obj->pbox->vert[l].initpos);
	//Computes Spring Magnitude
	deltaP.x = pv_k->pos.x - pv_l->pos.x;		// Vector distance
	deltaP.y = pv_k->pos.y - pv_l->pos.y;		// Vector distance
	deltaP.z = pv_k->pos.z - pv_l->pos.z;		// Vector distance
	float dist = (float)TRUEsqrt(deltaP.x * deltaP.x + deltaP.y * deltaP.y + deltaP.z * deltaP.z); // Magnitude of delta
	float divdist = 1.f / dist;
	Hterm = (dist - restlength) * PHYSICS_constant;

	deltaV.x = pv_k->velocity.x - pv_l->velocity.x;
	deltaV.y = pv_k->velocity.y - pv_l->velocity.y;
	deltaV.z = pv_k->velocity.z - pv_l->velocity.z;		// Delta Velocity Vector
	Dterm = dot(deltaV, deltaP) * PHYSICS_Damp * divdist; // Damping Term
	Dterm = (-(Hterm + Dterm));
	divdist *= Dterm;
	springforce.x = deltaP.x * divdist;	// Normalize Distance Vector
	springforce.y = deltaP.y * divdist;	// & Calc Force
	springforce.z = deltaP.z * divdist;

	pv_k->force.x += springforce.x;	// + force on particle 1
	pv_k->force.y += springforce.y;
	pv_k->force.z += springforce.z;

	pv_l->force.x -= springforce.x;	// - force on particle 2
	pv_l->force.y -= springforce.y;
	pv_l->force.z -= springforce.z;
}

//*************************************************************************************
// Computes the forces applied to the physical vertices of an object
//*************************************************************************************

void EERIE_PHYSICS_BOX_ComputeForces(EERIE_3DOBJ * obj)
{
	Vec3f PHYSICS_Gravity;
	PHYSICS_Gravity.x = 0.f;
	PHYSICS_Gravity.y = -20.f;
	PHYSICS_Gravity.z = 0.f;


	float PHYSICS_Damping = 0.6f;
	float lastmass = 1.f;
	float div = 1.f;

	for (long k = 0; k < obj->pbox->nb_physvert; k++)
	{
		PHYSVERT * pv = &obj->pbox->vert[k];
		// Reset Force
		pv->force.x = pv->inertia.x;
		pv->force.y = pv->inertia.y;
		pv->force.z = pv->inertia.z;

		// Apply Gravity
		if (pv->mass > 0.f)
		{
			//need to be precomputed...
			if (lastmass != pv->mass)
			{
				div = 1.f / pv->mass;
				lastmass = pv->mass;
			}

			pv->force.x += (PHYSICS_Gravity.x * div);
			pv->force.y -= (PHYSICS_Gravity.y * div);
			pv->force.z += (PHYSICS_Gravity.z * div);
		}

		// Apply Damping
		pv->force.x += (-PHYSICS_Damping * pv->velocity.x);
		pv->force.y += (-PHYSICS_Damping * pv->velocity.y);
		pv->force.z += (-PHYSICS_Damping * pv->velocity.z);
	}

	for (int k = 0; k < obj->pbox->nb_physvert; k++)
	{
		// Now Resolves Spring System
		for (long l = 0; l < obj->pbox->nb_physvert; l++)
		{
			if (l != k) ApplySpring(obj, l, k, 18.f, 0.4f);
		}
	}
}

#define MAKE_COLL_TEST if (Triangles_Intersect(&t1,&t2)) return true;

long PHYS_COLLIDER = -1;

//*************************************************************************************
// Checks is a triangle of a physical object is colliding a triangle
//*************************************************************************************
bool IsObjectVertexCollidingTriangle(EERIE_3DOBJ * obj, Vec3f * verts, long k, long * validd)
{
	EERIE_TRI t1, t2;
	bool ret = false;
	std::copy(verts, verts + 2, t2.v);

	PHYSVERT * vert = obj->pbox->vert;

	Vec3f center = (verts[0] + verts[1] + verts[2]) * ( 1.0f / 3 );
	float rad = fdist(center, verts[0]);

	if (k == -1)
	{
		long nn = 0;

		for (; nn < obj->pbox->nb_physvert; nn++)
		{
			if (distSqr(center, vert[nn].pos) <= max(square(60.0f), square(rad + 25)))
			{
				nn = 1000;
			}
		}

		if (nn < 1000)
			return false;
	}
	else
	{
		if (distSqr(center, vert[k].pos) > square(rad + 25))
			return false;
	}

	//TOP
	if ((k == -1) || (k == 1) || (k == 2) || (k == 3))
	{
		t1.v[0] = vert[1].pos;
		t1.v[1] = vert[2].pos;
		t1.v[2] = vert[3].pos;
		PHYS_COLLIDER = 1;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[1] = 0;
				validd[2] = 0;
				validd[3] = 0;
				ret = true;
			}
			else return true;
		}//MAKE_COLL_TEST
	}

#if FULLTESTS

	if ((k == -1) || (k == 1) || (k == 4) || (k == 3))
	{
		t1.v[0] = vert[3].pos;
		t1.v[1] = vert[4].pos;
		t1.v[2] = vert[1].pos;
		PHYS_COLLIDER = 1;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[1] = 0;
				validd[3] = 0;
				validd[4] = 0;
				ret = true;
			}
			else return true;
		}//MAKE_COLL_TEST
	}

#endif

	//BOTTOM
	if ((k == -1) || (k == 9) || (k == 10) || (k == 12))
	{
		t1.v[0] = vert[10].pos;
		t1.v[1] = vert[9].pos;
		t1.v[2] = vert[11].pos;
		PHYS_COLLIDER = 9;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[0] = 0;
				validd[10] = 0;
				validd[12] = 0;
				ret = true;
			}
			else return true;
		}//MAKE_COLL_TEST
	}

#if FULLTESTS

	if ((k == -1) || (k == 10) || (k == 11) || (k == 12))
	{
		t1.v[0] = vert[9].pos;
		t1.v[1] = vert[12].pos;
		t1.v[2] = vert[11].pos;
		PHYS_COLLIDER = 10;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[10] = 0;
				validd[11] = 0;
				validd[12] = 0;
				ret = true;
			}
			else return true;
		}//MAKE_COLL_TEST
	}

#endif

	//UP/FRONT
	if ((k == -1) || (k == 1) || (k == 4) || (k == 5))
	{
		t1.v[0] = vert[1].pos;
		t1.v[1] = vert[4].pos;
		t1.v[2] = vert[5].pos;
		PHYS_COLLIDER = 4;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[1] = 0;
				validd[4] = 0;
				validd[5] = 0;
				ret = true;
			}
			else return true;
		}//MAKE_COLL_TEST
	}

#if FULLTESTS

	if ((k == -1) || (k == 4) || (k == 5) || (k == 8))
	{
		t1.v[0] = vert[4].pos;
		t1.v[1] = vert[8].pos;
		t1.v[2] = vert[5].pos;
		PHYS_COLLIDER = 5;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[4] = 0;
				validd[5] = 0;
				validd[8] = 0;
				ret = true;
			}
			else return true;
		}//MAKE_COLL_TEST
	}

#endif

	//DOWN/FRONT
	if ((k == -1) || (k == 5) || (k == 8) || (k == 9))
	{
		t1.v[0] = vert[5].pos;
		t1.v[1] = vert[8].pos;
		t1.v[2] = vert[9].pos;
		PHYS_COLLIDER = 8;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[5] = 0;
				validd[8] = 0;
				validd[9] = 0;
				ret = true;
			}
			else return true;
		}//MAKE_COLL_TEST
	}

#if FULLTESTS

	if ((k == -1) || (k == 8) || (k == 12) || (k == 9))
	{
		t1.v[0] = vert[8].pos;
		t1.v[1] = vert[12].pos;
		t1.v[2] = vert[9].pos;
		PHYS_COLLIDER = 12;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[8] = 0;
				validd[12] = 0;
				validd[9] = 0;
				ret = true;
			}
			else return true;
		}//MAKE_COLL_TEST
	}

#endif

	//UP/BACK
	if ((k == -1) || (k == 3) || (k == 2) || (k == 7))
	{
		t1.v[0] = vert[3].pos;
		t1.v[1] = vert[2].pos;
		t1.v[2] = vert[7].pos;
		PHYS_COLLIDER = 3;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[3] = 0;
				validd[2] = 0;
				validd[7] = 0;
				ret = true;
			}
			else return true;
		}//MAKE_COLL_TEST
	}

#if FULLTESTS

	if ((k == -1) || (k == 2) || (k == 6) || (k == 7))
	{
		t1.v[0] = vert[2].pos;
		t1.v[1] = vert[6].pos;
		t1.v[2] = vert[7].pos;
		PHYS_COLLIDER = 2;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[2] = 0;
				validd[6] = 0;
				validd[7] = 0;
				ret = true;
			}
			else return true;
		}//MAKE_COLL_TEST
	}

#endif

	//DOWN/BACK
	if ((k == -1) || (k == 7) || (k == 6) || (k == 11))
	{
		t1.v[0] = vert[7].pos;
		t1.v[1] = vert[6].pos;
		t1.v[2] = vert[11].pos;
		PHYS_COLLIDER = 7;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[6] = 0;
				validd[7] = 0;
				validd[11] = 0;
				ret = true;
			}
			else return true;
		}//MAKE_COLL_TEST
	}

#if FULLTESTS

	if ((k == -1) || (k == 6) || (k == 10) || (k == 11))
	{
		t1.v[0] = vert[6].pos;
		t1.v[1] = vert[10].pos;
		t1.v[2] = vert[11].pos;
		PHYS_COLLIDER = 6;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[6] = 0;
				validd[10] = 0;
				validd[11] = 0;
				ret = true;
			}
			else return true;
		}//MAKE_COLL_TEST
	}

#endif

	//UP/LEFT
	if ((k == -1) || (k == 1) || (k == 2) || (k == 6))
	{
		t1.v[0] = vert[6].pos;
		t1.v[1] = vert[2].pos;
		t1.v[2] = vert[1].pos;
		PHYS_COLLIDER = 2;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[1] = 0;
				validd[2] = 0;
				validd[6] = 0;
				ret = true;
			}
			else return true;
		}//MAKE_COLL_TEST
	}

#if FULLTESTS

	if ((k == -1) || (k == 1) || (k == 5) || (k == 6))
	{
		t1.v[0] = vert[1].pos;
		t1.v[1] = vert[5].pos;
		t1.v[2] = vert[6].pos;
		PHYS_COLLIDER = 5;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[1] = 0;
				validd[5] = 0;
				validd[6] = 0;
				ret = true;
			}
			else return true;
		}//MAKE_COLL_TEST
	}

#endif

	//DOWN/LEFT
	if ((k == -1) || (k == 10) || (k == 6) || (k == 5))
	{
		t1.v[0] = vert[10].pos;
		t1.v[1] = vert[6].pos;
		t1.v[2] = vert[5].pos;
		PHYS_COLLIDER = 6;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[10] = 0;
				validd[6] = 0;
				validd[5] = 0;
				ret = true;
			}
			else return true;
		}//MAKE_COLL_TEST
	}

#if FULLTESTS

	if ((k == -1) || (k == 5) || (k == 9) || (k == 10))
	{
		t1.v[0] = vert[5].pos;
		t1.v[1] = vert[9].pos;
		t1.v[2] = vert[10].pos;
		PHYS_COLLIDER = 5;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[5] = 0;
				validd[9] = 0;
				validd[10] = 0;
				ret = true;
			}
			else return true;
		}//MAKE_COLL_TEST
	}

#endif

	//UP/RIGHT
	if ((k == -1) || (k == 4) || (k == 3) || (k == 7))
	{
		t1.v[0] = vert[4].pos;
		t1.v[1] = vert[3].pos;
		t1.v[2] = vert[7].pos;
		PHYS_COLLIDER = 4;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[3] = 0;
				validd[4] = 0;
				validd[7] = 0;
				ret = true;
			}
			else return true;
		}//MAKE_COLL_TEST
	}

#if FULLTESTS

	if ((k == -1) || (k == 7) || (k == 8) || (k == 4))
	{
		t1.v[0] = vert[7].pos;
		t1.v[1] = vert[8].pos;
		t1.v[2] = vert[4].pos;
		PHYS_COLLIDER = 7;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[4] = 0;
				validd[7] = 0;
				validd[8] = 0;
				ret = true;
			}
			else return true;
		}//MAKE_COLL_TEST
	}

#endif

	//DOWN/RIGHT
	if ((k == -1) || (k == 8) || (k == 7) || (k == 11))
	{
		t1.v[0] = vert[8].pos;
		t1.v[1] = vert[7].pos;
		t1.v[2] = vert[11].pos;
		PHYS_COLLIDER = 8;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[7] = 0;
				validd[8] = 0;
				validd[11] = 0;
				ret = true;
			}
			else return true;
		}//MAKE_COLL_TEST
	}

#if FULLTESTS

	if ((k == -1) || (k == 11) || (k == 12) || (k == 8))
	{
		t1.v[0] = vert[11].pos;
		t1.v[1] = vert[12].pos;
		t1.v[2] = vert[8].pos;
		PHYS_COLLIDER = 11;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[8] = 0;
				validd[11] = 0;
				validd[12] = 0;
				ret = true;
			}
			else return true;
		}//MAKE_COLL_TEST
	}

#endif
	return ret;
}

static void copy(Vec3f & dest, const TexturedVertex & src) {
	dest.x = src.sx;
	dest.y = src.sy;
	dest.z = src.sz;
}

//*************************************************************************************
//*************************************************************************************
bool IsObjectVertexCollidingPoly(EERIE_3DOBJ * obj, EERIEPOLY * ep, long k, long * validd)
{
	Vec3f pol[3];
	copy(pol[0], ep->v[0]);
	copy(pol[1], ep->v[1]);
	copy(pol[2], ep->v[2]);
	float mul = 1.3f;
	pol[0] = (pol[0] - ep->center) * mul + ep->center;

	if (ep->type & POLY_QUAD)
	{
		if (IsObjectVertexCollidingTriangle(obj, pol, k, validd)) return true;

		copy(pol[0], ep->v[2]);
		copy(pol[1], ep->v[3]);
		copy(pol[2], ep->v[0]);

		if (IsObjectVertexCollidingTriangle(obj, pol, k, validd)) return true;

		return false;
	}

	if (IsObjectVertexCollidingTriangle(obj, pol, k, validd)) return true;

	return false;
}

 
EERIEPOLY * LAST_COLLISION_POLY = NULL;

//*************************************************************************************
//*************************************************************************************
bool IsObjectVertexInValidPosition(EERIE_3DOBJ * obj, long kk, long flags, long source)
{
	EERIEPOLY * back_ep = CheckInPolyPrecis(obj->pbox->vert[kk].pos.x,
	                                        obj->pbox->vert[kk].pos.y,
	                                        obj->pbox->vert[kk].pos.z);

	if (!back_ep)
	{
		Vec3f posi = obj->pbox->vert[kk].pos;
		posi.y -= 30.f;

		CUR_COLLISION_MATERIAL = MATERIAL_STONE;
		return false;
	}

	if (!(flags & 1))
	{
		EERIE_SPHERE sphere;
		Vec3f * pos = &obj->pbox->vert[kk].pos;
		sphere.origin.x = pos->x;
		sphere.origin.y = pos->y;
		sphere.origin.z = pos->z;
		sphere.radius = 8.f;

		if (ARX_INTERACTIVE_CheckCollision(obj, kk, source))
		{
			CUR_COLLISION_MATERIAL = 11;
			return false;
		}
	}

	return true;
}

// Debug function used to show the physical box of an object
void EERIE_PHYSICS_BOX_Show(EERIE_3DOBJ * obj) {
	
	for (long k = 0; k < obj->pbox->nb_physvert; k++) {
		if(obj->pbox->active == 2) {
			DebugSphere(obj->pbox->vert[k].pos.x, obj->pbox->vert[k].pos.y,  obj->pbox->vert[k].pos.z,
			            0.6f, 40, Color::green);
		} else if(k == 0 || k == 14 || k == 13) {
			DebugSphere(obj->pbox->vert[k].pos.x, obj->pbox->vert[k].pos.y, obj->pbox->vert[k].pos.z,
			            0.6f, 40, Color::yellow);
		} else if ((k > 0) && (k < 5)) {
			DebugSphere(obj->pbox->vert[k].pos.x, obj->pbox->vert[k].pos.y, obj->pbox->vert[k].pos.z,
			            0.6f, 40, Color::green);
		} else if ((k > 4) && (k < 9)) {
			DebugSphere(obj->pbox->vert[k].pos.x, obj->pbox->vert[k].pos.y, obj->pbox->vert[k].pos.z,
			            0.6f, 40, Color::blue);
		} else {
			DebugSphere(obj->pbox->vert[k].pos.x, obj->pbox->vert[k].pos.y, obj->pbox->vert[k].pos.z,
			            0.6f, 40, Color::red);
		}
	}
}

//-----------------------------------------------------------------------------
// Releases physic box data from an object
void EERIE_PHYSICS_BOX_Release(EERIE_3DOBJ * obj)
{
	if (obj == NULL) return;

	if (obj->pbox == NULL) return;

	if (obj->pbox->vert != NULL)
	{
		free(obj->pbox->vert);
		obj->pbox->vert = NULL;
	}

	free(obj->pbox);
	obj->pbox = NULL;
}

// Creation of the physics box... quite cabalistic and extensive func...
// Need to put a (really) smarter algorithm in there...
void EERIE_PHYSICS_BOX_Create(EERIE_3DOBJ * obj)
{
	if (!obj) return;

	EERIE_PHYSICS_BOX_Release(obj);

	if (obj->vertexlist.empty()) return;

	obj->pbox =	(PHYSICS_BOX_DATA *)
	            malloc(sizeof(PHYSICS_BOX_DATA));
	memset(obj->pbox, 0, sizeof(PHYSICS_BOX_DATA));
	obj->pbox->nb_physvert = 15;
	obj->pbox->stopcount = 0;
	obj->pbox->vert =	(PHYSVERT *)
	                    malloc(sizeof(PHYSVERT) * obj->pbox->nb_physvert);
	memset(obj->pbox->vert, 0, sizeof(PHYSVERT)*obj->pbox->nb_physvert);

	Vec3f cubmin, cubmax;
	cubmin.x = std::numeric_limits<float>::max();
	cubmin.y = std::numeric_limits<float>::max();
	cubmin.z = std::numeric_limits<float>::max();

	cubmax.x = -std::numeric_limits<float>::max();
	cubmax.y = -std::numeric_limits<float>::max();
	cubmax.z = -std::numeric_limits<float>::max();

	for (size_t k = 0; k < obj->vertexlist.size(); k++)
	{
		if (k == (size_t)obj->origin) continue;

		cubmin.x = min(cubmin.x, obj->vertexlist[k].v.x);
		cubmin.y = min(cubmin.y, obj->vertexlist[k].v.y);
		cubmin.z = min(cubmin.z, obj->vertexlist[k].v.z);

		cubmax.x = max(cubmax.x, obj->vertexlist[k].v.x);
		cubmax.y = max(cubmax.y, obj->vertexlist[k].v.y);
		cubmax.z = max(cubmax.z, obj->vertexlist[k].v.z);
	}

	obj->pbox->vert[0].pos.x = cubmin.x + (cubmax.x - cubmin.x) * ( 1.0f / 2 );
	obj->pbox->vert[0].pos.y = cubmin.y + (cubmax.y - cubmin.y) * ( 1.0f / 2 );
	obj->pbox->vert[0].pos.z = cubmin.z + (cubmax.z - cubmin.z) * ( 1.0f / 2 );

	obj->pbox->vert[13].pos.x = obj->pbox->vert[0].pos.x;
	obj->pbox->vert[13].pos.y = cubmin.y;
	obj->pbox->vert[13].pos.z = obj->pbox->vert[0].pos.z;

	obj->pbox->vert[14].pos.x = obj->pbox->vert[0].pos.x;
	obj->pbox->vert[14].pos.y = cubmax.y;
	obj->pbox->vert[14].pos.z = obj->pbox->vert[0].pos.z;

	for (int k = 1; k < obj->pbox->nb_physvert - 2; k++)
	{
		obj->pbox->vert[k].pos.x = obj->pbox->vert[0].pos.x;
		obj->pbox->vert[k].pos.z = obj->pbox->vert[0].pos.z;

		if (k < 5)		obj->pbox->vert[k].pos.y = cubmin.y;
		else if (k < 9)	obj->pbox->vert[k].pos.y = obj->pbox->vert[0].pos.y;
		else			obj->pbox->vert[k].pos.y = cubmax.y;
	}

	float diff = cubmax.y - cubmin.y;

	if (diff < 12.f) 
	{
		cubmax.y += 8.f; 
		cubmin.y -= 8.f; 

		for (int k = 1; k < obj->pbox->nb_physvert - 2; k++)
		{
			obj->pbox->vert[k].pos.x = obj->pbox->vert[0].pos.x;
			obj->pbox->vert[k].pos.z = obj->pbox->vert[0].pos.z;

			if (k < 5)		obj->pbox->vert[k].pos.y = cubmin.y;
			else if (k < 9)	obj->pbox->vert[k].pos.y = obj->pbox->vert[0].pos.y;
			else			obj->pbox->vert[k].pos.y = cubmax.y;
		}

		obj->pbox->vert[14].pos.y = cubmax.y;
		obj->pbox->vert[13].pos.y = cubmin.y;
		float RATI = diff * ( 1.0f / 8 );

		for (size_t k = 0; k < obj->vertexlist.size(); k++)
		{
			if (k == (size_t)obj->origin) continue;

			Vec3f curr = obj->vertexlist[k].v;
			long SEC = 1;
			obj->pbox->vert[SEC].pos.x = min(obj->pbox->vert[SEC].pos.x, curr.x);
			obj->pbox->vert[SEC].pos.z = min(obj->pbox->vert[SEC].pos.z, curr.z);

			obj->pbox->vert[SEC+1].pos.x = min(obj->pbox->vert[SEC+1].pos.x, curr.x);
			obj->pbox->vert[SEC+1].pos.z = max(obj->pbox->vert[SEC+1].pos.z, curr.z);

			obj->pbox->vert[SEC+2].pos.x = max(obj->pbox->vert[SEC+2].pos.x, curr.x);
			obj->pbox->vert[SEC+2].pos.z = max(obj->pbox->vert[SEC+2].pos.z, curr.z);

			obj->pbox->vert[SEC+3].pos.x = max(obj->pbox->vert[SEC+3].pos.x, curr.x);
			obj->pbox->vert[SEC+3].pos.z = min(obj->pbox->vert[SEC+3].pos.z, curr.z);

			SEC = 5;
			obj->pbox->vert[SEC].pos.x = min(obj->pbox->vert[SEC].pos.x, curr.x - RATI);
			obj->pbox->vert[SEC].pos.z = min(obj->pbox->vert[SEC].pos.z, curr.z - RATI);

			obj->pbox->vert[SEC+1].pos.x = min(obj->pbox->vert[SEC+1].pos.x, curr.x - RATI);
			obj->pbox->vert[SEC+1].pos.z = max(obj->pbox->vert[SEC+1].pos.z, curr.z + RATI);

			obj->pbox->vert[SEC+2].pos.x = max(obj->pbox->vert[SEC+2].pos.x, curr.x + RATI);
			obj->pbox->vert[SEC+2].pos.z = max(obj->pbox->vert[SEC+2].pos.z, curr.z + RATI);

			obj->pbox->vert[SEC+3].pos.x = max(obj->pbox->vert[SEC+3].pos.x, curr.x + RATI);
			obj->pbox->vert[SEC+3].pos.z = min(obj->pbox->vert[SEC+3].pos.z, curr.z - RATI);


			SEC = 9;
			obj->pbox->vert[SEC].pos.x = min(obj->pbox->vert[SEC].pos.x, curr.x);
			obj->pbox->vert[SEC].pos.z = min(obj->pbox->vert[SEC].pos.z, curr.z);

			obj->pbox->vert[SEC+1].pos.x = min(obj->pbox->vert[SEC+1].pos.x, curr.x);
			obj->pbox->vert[SEC+1].pos.z = max(obj->pbox->vert[SEC+1].pos.z, curr.z);

			obj->pbox->vert[SEC+2].pos.x = max(obj->pbox->vert[SEC+2].pos.x, curr.x);
			obj->pbox->vert[SEC+2].pos.z = max(obj->pbox->vert[SEC+2].pos.z, curr.z);

			obj->pbox->vert[SEC+3].pos.x = max(obj->pbox->vert[SEC+3].pos.x, curr.x);
			obj->pbox->vert[SEC+3].pos.z = min(obj->pbox->vert[SEC+3].pos.z, curr.z);
		}
	}
	else
	{
		float cut = (cubmax.y - cubmin.y) * ( 1.0f / 3 );
		float ysec2 = cubmin.y + cut * 2.f;
		float ysec1 = cubmin.y + cut;

		for (size_t k = 0; k < obj->vertexlist.size(); k++)
		{
			if (k == (size_t)obj->origin) continue;

			Vec3f curr = obj->vertexlist[k].v;
			long SEC;

			if (curr.y < ysec1)
			{
				SEC = 1;
			}
			else if (curr.y < ysec2)
			{
				SEC = 5;
			}
			else
			{
				SEC = 9;
			}

			obj->pbox->vert[SEC].pos.x = min(obj->pbox->vert[SEC].pos.x, curr.x);
			obj->pbox->vert[SEC].pos.z = min(obj->pbox->vert[SEC].pos.z, curr.z);

			obj->pbox->vert[SEC+1].pos.x = min(obj->pbox->vert[SEC+1].pos.x, curr.x);
			obj->pbox->vert[SEC+1].pos.z = max(obj->pbox->vert[SEC+1].pos.z, curr.z);

			obj->pbox->vert[SEC+2].pos.x = max(obj->pbox->vert[SEC+2].pos.x, curr.x);
			obj->pbox->vert[SEC+2].pos.z = max(obj->pbox->vert[SEC+2].pos.z, curr.z);

			obj->pbox->vert[SEC+3].pos.x = max(obj->pbox->vert[SEC+3].pos.x, curr.x);
			obj->pbox->vert[SEC+3].pos.z = min(obj->pbox->vert[SEC+3].pos.z, curr.z);
		}
	}

	for (int k = 0; k < 4; k++)
	{
		if (EEfabs(obj->pbox->vert[5+k].pos.x - obj->pbox->vert[0].pos.x) < 2.f)
			obj->pbox->vert[5+k].pos.x = (obj->pbox->vert[1+k].pos.x + obj->pbox->vert[9+k].pos.x) * ( 1.0f / 2 );

		if (EEfabs(obj->pbox->vert[5+k].pos.z - obj->pbox->vert[0].pos.z) < 2.f)
			obj->pbox->vert[5+k].pos.z = (obj->pbox->vert[1+k].pos.z + obj->pbox->vert[9+k].pos.z) * ( 1.0f / 2 );
	}

	obj->pbox->radius = 0.f;

	for(int k = 0; k < obj->pbox->nb_physvert; k++) {
		float distt = dist(obj->pbox->vert[k].pos, obj->pbox->vert[0].pos);

		if (distt > 20.f)
		{
			obj->pbox->vert[k].pos.x = (obj->pbox->vert[k].pos.x
			                            - obj->pbox->vert[0].pos.x) * 0.5f +
			                           obj->pbox->vert[0].pos.x;
			obj->pbox->vert[k].pos.z = (obj->pbox->vert[k].pos.z
			                            - obj->pbox->vert[0].pos.z) * 0.5f +
			                           obj->pbox->vert[0].pos.z;
		}

		obj->pbox->vert[k].initpos = obj->pbox->vert[k].pos;

		if(k != 0) {
			float d = dist(obj->pbox->vert[0].pos, obj->pbox->vert[k].pos);
			obj->pbox->radius = max(obj->pbox->radius, d);
		}
	}
}
