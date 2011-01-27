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
#include "EERIEPhysicsBox.h"
#include "EERIEMath.h"

#include "arx_interactive.h"
#include "arx_npc.h"
#include "arx_Collisions.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

//-----------------------------------------------------------------------------
float VELOCITY_THRESHOLD = 850.f;
#define FULLTESTS FALSE

//-----------------------------------------------------------------------------
extern long DEBUGNPCMOVE;

//-----------------------------------------------------------------------------
long CUR_COLLISION_MATERIAL = 0;

//*************************************************************************************
// Used to launch an object into the physical world...
//*************************************************************************************
void EERIE_PHYSICS_BOX_Launch(EERIE_3DOBJ * obj, EERIE_3D * pos, EERIE_3D * vect, long flag, EERIE_3D * angle)
{
	if ((!obj) || !(obj->pbox)) return;

	obj->pbox->storedtiming = 0;

	for (long i = 0; i < obj->nbvertex; i++)
	{
		obj->vertexlist[i].vert.sx = obj->vertexlist[i].v.x;
		obj->vertexlist[i].vert.sy = obj->vertexlist[i].v.y;
		obj->vertexlist[i].vert.sz = obj->vertexlist[i].v.z;
	}

	float surface = 0.f;

	for (int i = 0; i < obj->nbfaces; i++)
	{

		D3DTLVERTEX * ev[3];
		ev[0] = (D3DTLVERTEX *)&obj->vertexlist[obj->facelist[i].vid[0]].v;
		ev[1] = (D3DTLVERTEX *)&obj->vertexlist[obj->facelist[i].vid[1]].v;
		ev[2] = (D3DTLVERTEX *)&obj->vertexlist[obj->facelist[i].vid[2]].v;
		surface += TRUEDistance3D((ev[0]->sx + ev[1]->sx) * DIV2,
		                          (ev[0]->sy + ev[1]->sy) * DIV2,
		                          (ev[0]->sz + ev[1]->sz) * DIV2,
		                          ev[2]->sx, ev[2]->sy, ev[2]->sz)
		           * TRUEDistance3D(ev[0]->sx, ev[0]->sy, ev[0]->sz,
		                            ev[1]->sx, ev[1]->sy, ev[1]->sz) * DIV2;
	}

	float ratio = surface * DIV10000;

	if (ratio > 0.8f) ratio = 0.8f;
	else if (ratio < 0.f) ratio = 0.f;

	ratio = 1.f - (ratio * DIV4);

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
bool IsValidPos3(EERIE_3D * pos)
{
	long px, pz;
	F2L(pos->x * ACTIVEBKG->Xmul, &px);

	if (px >= ACTIVEBKG->Xsize)
	{
		return false;
	}

	if (px < 0)
	{
		return false;
	}

	F2L(pos->z * ACTIVEBKG->Zmul, &pz);

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
	EERIE_3D deltaP, deltaV, springforce;
	PHYSVERT * pv_k = &obj->pbox->vert[k];
	PHYSVERT * pv_l = &obj->pbox->vert[l];
	float Dterm, Hterm;

	float restlength = TRUEEEDistance3D(&obj->pbox->vert[k].initpos, &obj->pbox->vert[l].initpos);
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
	Dterm = (Vector_DotProduct(&deltaV, &deltaP) * PHYSICS_Damp) * divdist; // Damping Term
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
	EERIE_3D PHYSICS_Gravity;
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

#define MAKE_COLL_TEST if (Triangles_Intersect(&t1,&t2)) return TRUE;

long PHYS_COLLIDER = -1;

//*************************************************************************************
// Checks is a triangle of a physical object is colliding a triangle
//*************************************************************************************
BOOL IsObjectVertexCollidingTriangle(EERIE_3DOBJ * obj, EERIE_3D * verts, long k, long * validd)
{
	EERIE_TRI t1, t2;
	BOOL ret = FALSE;
	memcpy(&t2, verts, sizeof(EERIE_3D) * 3);

	PHYSVERT * vert = obj->pbox->vert;

	EERIE_3D center;
	center.x = (verts[0].x + verts[1].x + verts[2].x) * DIV3;
	center.y = (verts[0].y + verts[1].y + verts[2].y) * DIV3;
	center.z = (verts[0].z + verts[1].z + verts[2].z) * DIV3;
	float rad = EEDistance3D(&center, &verts[0]);

	if (k == -1)
	{
		long nn = 0;

		for (; nn < obj->pbox->nb_physvert; nn++)
		{
			if (EEDistance3D(&center, &vert[nn].pos) <= __max(60, rad + 25))
			{
				nn = 1000;
			}
		}

		if (nn < 1000)
			return FALSE;
	}
	else
	{
		if (EEDistance3D(&center, &vert[k].pos) > rad + 25)
			return FALSE;
	}

	//TOP
	if ((k == -1) || (k == 1) || (k == 2) || (k == 3))
	{
		Vector_Copy(&t1.v[0], &vert[1].pos);
		Vector_Copy(&t1.v[1], &vert[2].pos);
		Vector_Copy(&t1.v[2], &vert[3].pos);
		PHYS_COLLIDER = 1;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[1] = 0;
				validd[2] = 0;
				validd[3] = 0;
				ret = TRUE;
			}
			else return TRUE;
		}//MAKE_COLL_TEST
	}

#if FULLTESTS==TRUE

	if ((k == -1) || (k == 1) || (k == 4) || (k == 3))
	{
		Vector_Copy(&t1.v[0], &vert[3].pos);
		Vector_Copy(&t1.v[1], &vert[4].pos);
		Vector_Copy(&t1.v[2], &vert[1].pos);
		PHYS_COLLIDER = 1;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[1] = 0;
				validd[3] = 0;
				validd[4] = 0;
				ret = TRUE;
			}
			else return TRUE;
		}//MAKE_COLL_TEST
	}

#endif

	//BOTTOM
	if ((k == -1) || (k == 9) || (k == 10) || (k == 12))
	{
		Vector_Copy(&t1.v[0], &vert[10].pos);
		Vector_Copy(&t1.v[1], &vert[9].pos);
		Vector_Copy(&t1.v[2], &vert[11].pos);
		PHYS_COLLIDER = 9;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[0] = 0;
				validd[10] = 0;
				validd[12] = 0;
				ret = TRUE;
			}
			else return TRUE;
		}//MAKE_COLL_TEST
	}

#if FULLTESTS==TRUE

	if ((k == -1) || (k == 10) || (k == 11) || (k == 12))
	{
		Vector_Copy(&t1.v[0], &vert[9].pos);
		Vector_Copy(&t1.v[1], &vert[12].pos);
		Vector_Copy(&t1.v[2], &vert[11].pos);
		PHYS_COLLIDER = 10;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[10] = 0;
				validd[11] = 0;
				validd[12] = 0;
				ret = TRUE;
			}
			else return TRUE;
		}//MAKE_COLL_TEST
	}

#endif

	//UP/FRONT
	if ((k == -1) || (k == 1) || (k == 4) || (k == 5))
	{
		Vector_Copy(&t1.v[0], &vert[1].pos);
		Vector_Copy(&t1.v[1], &vert[4].pos);
		Vector_Copy(&t1.v[2], &vert[5].pos);
		PHYS_COLLIDER = 4;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[1] = 0;
				validd[4] = 0;
				validd[5] = 0;
				ret = TRUE;
			}
			else return TRUE;
		}//MAKE_COLL_TEST
	}

#if FULLTESTS==TRUE

	if ((k == -1) || (k == 4) || (k == 5) || (k == 8))
	{
		Vector_Copy(&t1.v[0], &vert[4].pos);
		Vector_Copy(&t1.v[1], &vert[8].pos);
		Vector_Copy(&t1.v[2], &vert[5].pos);
		PHYS_COLLIDER = 5;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[4] = 0;
				validd[5] = 0;
				validd[8] = 0;
				ret = TRUE;
			}
			else return TRUE;
		}//MAKE_COLL_TEST
	}

#endif

	//DOWN/FRONT
	if ((k == -1) || (k == 5) || (k == 8) || (k == 9))
	{
		Vector_Copy(&t1.v[0], &vert[5].pos);
		Vector_Copy(&t1.v[1], &vert[8].pos);
		Vector_Copy(&t1.v[2], &vert[9].pos);
		PHYS_COLLIDER = 8;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[5] = 0;
				validd[8] = 0;
				validd[9] = 0;
				ret = TRUE;
			}
			else return TRUE;
		}//MAKE_COLL_TEST
	}

#if FULLTESTS==TRUE

	if ((k == -1) || (k == 8) || (k == 12) || (k == 9))
	{
		Vector_Copy(&t1.v[0], &vert[8].pos);
		Vector_Copy(&t1.v[1], &vert[12].pos);
		Vector_Copy(&t1.v[2], &vert[9].pos);
		PHYS_COLLIDER = 12;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[8] = 0;
				validd[12] = 0;
				validd[9] = 0;
				ret = TRUE;
			}
			else return TRUE;
		}//MAKE_COLL_TEST
	}

#endif

	//UP/BACK
	if ((k == -1) || (k == 3) || (k == 2) || (k == 7))
	{
		Vector_Copy(&t1.v[0], &vert[3].pos);
		Vector_Copy(&t1.v[1], &vert[2].pos);
		Vector_Copy(&t1.v[2], &vert[7].pos);
		PHYS_COLLIDER = 3;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[3] = 0;
				validd[2] = 0;
				validd[7] = 0;
				ret = TRUE;
			}
			else return TRUE;
		}//MAKE_COLL_TEST
	}

#if FULLTESTS==TRUE

	if ((k == -1) || (k == 2) || (k == 6) || (k == 7))
	{
		Vector_Copy(&t1.v[0], &vert[2].pos);
		Vector_Copy(&t1.v[1], &vert[6].pos);
		Vector_Copy(&t1.v[2], &vert[7].pos);
		PHYS_COLLIDER = 2;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[2] = 0;
				validd[6] = 0;
				validd[7] = 0;
				ret = TRUE;
			}
			else return TRUE;
		}//MAKE_COLL_TEST
	}

#endif

	//DOWN/BACK
	if ((k == -1) || (k == 7) || (k == 6) || (k == 11))
	{
		Vector_Copy(&t1.v[0], &vert[7].pos);
		Vector_Copy(&t1.v[1], &vert[6].pos);
		Vector_Copy(&t1.v[2], &vert[11].pos);
		PHYS_COLLIDER = 7;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[6] = 0;
				validd[7] = 0;
				validd[11] = 0;
				ret = TRUE;
			}
			else return TRUE;
		}//MAKE_COLL_TEST
	}

#if FULLTESTS==TRUE

	if ((k == -1) || (k == 6) || (k == 10) || (k == 11))
	{
		Vector_Copy(&t1.v[0], &vert[6].pos);
		Vector_Copy(&t1.v[1], &vert[10].pos);
		Vector_Copy(&t1.v[2], &vert[11].pos);
		PHYS_COLLIDER = 6;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[6] = 0;
				validd[10] = 0;
				validd[11] = 0;
				ret = TRUE;
			}
			else return TRUE;
		}//MAKE_COLL_TEST
	}

#endif

	//UP/LEFT
	if ((k == -1) || (k == 1) || (k == 2) || (k == 6))
	{
		Vector_Copy(&t1.v[0], &vert[6].pos);
		Vector_Copy(&t1.v[1], &vert[2].pos);
		Vector_Copy(&t1.v[2], &vert[1].pos);
		PHYS_COLLIDER = 2;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[1] = 0;
				validd[2] = 0;
				validd[6] = 0;
				ret = TRUE;
			}
			else return TRUE;
		}//MAKE_COLL_TEST
	}

#if FULLTESTS==TRUE

	if ((k == -1) || (k == 1) || (k == 5) || (k == 6))
	{
		Vector_Copy(&t1.v[0], &vert[1].pos);
		Vector_Copy(&t1.v[1], &vert[5].pos);
		Vector_Copy(&t1.v[2], &vert[6].pos);
		PHYS_COLLIDER = 5;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[1] = 0;
				validd[5] = 0;
				validd[6] = 0;
				ret = TRUE;
			}
			else return TRUE;
		}//MAKE_COLL_TEST
	}

#endif

	//DOWN/LEFT
	if ((k == -1) || (k == 10) || (k == 6) || (k == 5))
	{
		Vector_Copy(&t1.v[0], &vert[10].pos);
		Vector_Copy(&t1.v[1], &vert[6].pos);
		Vector_Copy(&t1.v[2], &vert[5].pos);
		PHYS_COLLIDER = 6;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[10] = 0;
				validd[6] = 0;
				validd[5] = 0;
				ret = TRUE;
			}
			else return TRUE;
		}//MAKE_COLL_TEST
	}

#if FULLTESTS==TRUE

	if ((k == -1) || (k == 5) || (k == 9) || (k == 10))
	{
		Vector_Copy(&t1.v[0], &vert[5].pos);
		Vector_Copy(&t1.v[1], &vert[9].pos);
		Vector_Copy(&t1.v[2], &vert[10].pos);
		PHYS_COLLIDER = 5;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[5] = 0;
				validd[9] = 0;
				validd[10] = 0;
				ret = TRUE;
			}
			else return TRUE;
		}//MAKE_COLL_TEST
	}

#endif

	//UP/RIGHT
	if ((k == -1) || (k == 4) || (k == 3) || (k == 7))
	{
		Vector_Copy(&t1.v[0], &vert[4].pos);
		Vector_Copy(&t1.v[1], &vert[3].pos);
		Vector_Copy(&t1.v[2], &vert[7].pos);
		PHYS_COLLIDER = 4;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[3] = 0;
				validd[4] = 0;
				validd[7] = 0;
				ret = TRUE;
			}
			else return TRUE;
		}//MAKE_COLL_TEST
	}

#if FULLTESTS==TRUE

	if ((k == -1) || (k == 7) || (k == 8) || (k == 4))
	{
		Vector_Copy(&t1.v[0], &vert[7].pos);
		Vector_Copy(&t1.v[1], &vert[8].pos);
		Vector_Copy(&t1.v[2], &vert[4].pos);
		PHYS_COLLIDER = 7;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[4] = 0;
				validd[7] = 0;
				validd[8] = 0;
				ret = TRUE;
			}
			else return TRUE;
		}//MAKE_COLL_TEST
	}

#endif

	//DOWN/RIGHT
	if ((k == -1) || (k == 8) || (k == 7) || (k == 11))
	{
		Vector_Copy(&t1.v[0], &vert[8].pos);
		Vector_Copy(&t1.v[1], &vert[7].pos);
		Vector_Copy(&t1.v[2], &vert[11].pos);
		PHYS_COLLIDER = 8;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[7] = 0;
				validd[8] = 0;
				validd[11] = 0;
				ret = TRUE;
			}
			else return TRUE;
		}//MAKE_COLL_TEST
	}

#if FULLTESTS==TRUE

	if ((k == -1) || (k == 11) || (k == 12) || (k == 8))
	{
		Vector_Copy(&t1.v[0], &vert[11].pos);
		Vector_Copy(&t1.v[1], &vert[12].pos);
		Vector_Copy(&t1.v[2], &vert[8].pos);
		PHYS_COLLIDER = 11;

		if (Triangles_Intersect(&t1, &t2))
		{
			if (validd)
			{
				validd[8] = 0;
				validd[11] = 0;
				validd[12] = 0;
				ret = TRUE;
			}
			else return TRUE;
		}//MAKE_COLL_TEST
	}

#endif
	return ret;
}

//*************************************************************************************
//*************************************************************************************
BOOL IsObjectVertexCollidingPoly(EERIE_3DOBJ * obj, EERIEPOLY * ep, long k, long * validd)
{
	EERIE_3D pol[3];
	Vector_Copy(&pol[0], (EERIE_3D *)&ep->v[0]);
	Vector_Copy(&pol[1], (EERIE_3D *)&ep->v[1]);
	Vector_Copy(&pol[2], (EERIE_3D *)&ep->v[2]);
	float mul = 1.3f;
	pol[0].x = (pol[0].x - ep->center.x) * mul + ep->center.x;
	pol[0].y = (pol[0].y - ep->center.y) * mul + ep->center.y;
	pol[0].z = (pol[0].z - ep->center.z) * mul + ep->center.z;

	if (ep->type & POLY_QUAD)
	{
		if (IsObjectVertexCollidingTriangle(obj, (EERIE_3D *)&pol, k, validd)) return TRUE;

		Vector_Copy(&pol[0], (EERIE_3D *)&ep->v[2]);
		Vector_Copy(&pol[1], (EERIE_3D *)&ep->v[3]);
		Vector_Copy(&pol[2], (EERIE_3D *)&ep->v[0]);

		if (IsObjectVertexCollidingTriangle(obj, (EERIE_3D *)&pol, k, validd)) return TRUE;

		return FALSE;
	}

	if (IsObjectVertexCollidingTriangle(obj, (EERIE_3D *)&pol, k, validd)) return TRUE;

	return FALSE;
}

 
EERIEPOLY * LAST_COLLISION_POLY = NULL;
BOOL IsFULLObjectVertexInValidPosition(EERIE_3DOBJ * obj, long flags, long source, long * validd)
{
	BOOL ret = TRUE;
	long px, pz;
	float x = obj->pbox->vert[0].pos.x;
	F2L(x * ACTIVEBKG->Xmul, &px);
	float z = obj->pbox->vert[0].pos.z;
	F2L(z * ACTIVEBKG->Zmul, &pz);
	long ix, iz, ax, az;
	long n;
	F2L(obj->pbox->radius * DIV100, &n);
	n = __min(2, n + 1);
	ix = __max(px - n, 0);
	ax = __min(px + n, ACTIVEBKG->Xsize - 1);
	iz = __max(pz - n, 0);
	az = __min(pz + n, ACTIVEBKG->Zsize - 1);
	LAST_COLLISION_POLY = NULL;
	EERIEPOLY * ep;
	EERIE_BKG_INFO * eg;


	for (pz = iz; pz <= az; pz++)
		for (px = ix; px <= ax; px++)
		{
			eg = &ACTIVEBKG->Backg[px+pz*ACTIVEBKG->Xsize];

			for (long k = 0; k < eg->nbpoly; k++)
			{
				ep = &eg->polydata[k];

				if ( (ep->area > 190.f)
				    &&	(!(ep->type & (POLY_WATER)))
				    &&	(!(ep->type & (POLY_TRANS)))
				    &&	(!(ep->type & (POLY_NOCOL)))
				)
				{
					if ((EEDistance3D(&ep->center, &obj->pbox->vert[0].pos) > obj->pbox->radius + 75.f)
					        &&	(EEDistance3D((EERIE_3D *)&ep->v[0], &obj->pbox->vert[0].pos) > obj->pbox->radius + 55.f)
					        &&	(EEDistance3D((EERIE_3D *)&ep->v[1], &obj->pbox->vert[0].pos) > obj->pbox->radius + 55.f)
					        &&	(EEDistance3D((EERIE_3D *)&ep->v[2], &obj->pbox->vert[0].pos) > obj->pbox->radius + 55.f))
						continue;

					if (IsObjectVertexCollidingPoly(obj, ep, -1, NULL)) 
					{
						LAST_COLLISION_POLY = ep;

						if (ep->type & POLY_METAL) CUR_COLLISION_MATERIAL = MATERIAL_METAL;
						else if (ep->type & POLY_WOOD) CUR_COLLISION_MATERIAL = MATERIAL_WOOD;
						else if (ep->type & POLY_STONE) CUR_COLLISION_MATERIAL = MATERIAL_STONE;
						else if (ep->type & POLY_GRAVEL) CUR_COLLISION_MATERIAL = MATERIAL_GRAVEL;
						else if (ep->type & POLY_WATER) CUR_COLLISION_MATERIAL = MATERIAL_WATER;
						else if (ep->type & POLY_EARTH) CUR_COLLISION_MATERIAL = MATERIAL_EARTH;
						else CUR_COLLISION_MATERIAL = MATERIAL_STONE;

						ret = FALSE;
						return FALSE;
					}
				}
			}
		}

	return ret;
}
 
//*************************************************************************************
//*************************************************************************************
BOOL IsObjectVertexInValidPosition(EERIE_3DOBJ * obj, long kk, long flags, long source)
{
	EERIEPOLY * back_ep = CheckInPolyPrecis(obj->pbox->vert[kk].pos.x,
	                                        obj->pbox->vert[kk].pos.y,
	                                        obj->pbox->vert[kk].pos.z);

	if (!back_ep)
	{
		EERIE_3D posi;
		Vector_Copy(&posi, &obj->pbox->vert[kk].pos);
		posi.y -= 30.f;

		CUR_COLLISION_MATERIAL = MATERIAL_STONE;
		return FALSE;
	}

	if (!(flags & 1))
	{
		EERIE_SPHERE sphere;
		EERIE_3D * pos = &obj->pbox->vert[kk].pos;
		sphere.origin.x = pos->x;
		sphere.origin.y = pos->y;
		sphere.origin.z = pos->z;
		sphere.radius = 8.f;

		if (ARX_INTERACTIVE_CheckCollision(obj, kk, source))
		{
			CUR_COLLISION_MATERIAL = 11;
			return FALSE;
		}
	}

	return TRUE;
}
 
extern BOOL ARX_EERIE_PHYSICS_BOX_Compute(EERIE_3DOBJ * obj, float framediff, float rubber, long flags, long source);
extern long ARX_PHYSICS_BOX_ApplyModel(EERIE_3DOBJ * obj, float framediff, float rubber, long flags, long source);

//*************************************************************************************
//*************************************************************************************

long EERIE_PHYSICS_BOX_ApplyModel(EERIE_3DOBJ * obj, float framediff, float rubber, long flags, long source)
{
	return ARX_PHYSICS_BOX_ApplyModel(obj, framediff, rubber, flags, source);
	long ret = 0;

	if (obj->pbox->active == 2) return ret;

	if (framediff == 0.f) return ret;

	PHYSVERT * pv;

	// Memorizes initpos
	for (long k = 0; k < obj->pbox->nb_physvert; k++)
	{
		pv = &obj->pbox->vert[k];
		pv->temp.x = pv->pos.x;
		pv->temp.y = pv->pos.y;
		pv->temp.z = pv->pos.z;
	}

	float timing = framediff * rubber * DIV50;

	while (timing > 0.f)
	{
		EERIE_PHYSICS_BOX_ComputeForces(obj);

		if (!ARX_EERIE_PHYSICS_BOX_Compute(obj, __min(0.08f, timing), rubber, flags, source))
			ret = 1;

		timing -= 0.08f;
	}


	if (obj->pbox->stopcount < 16) return ret;

	obj->pbox->active = 2;
	obj->pbox->stopcount = 0;

	if (ValidIONum(source))
	{
		inter.iobj[source]->soundcount = 0;
		inter.iobj[source]->ioflags &= ~IO_NO_NPC_COLLIDE;
	}

	return ret;
}

// Debug function used to show the physical box of an object
void EERIE_PHYSICS_BOX_Show(EERIE_3DOBJ * obj, EERIE_3D * pos)
{
	if (DEBUGNPCMOVE)
	{

		for (long k = 0; k < obj->pbox->nb_physvert; k++)
		{
			if (obj->pbox->active == 2)
			{
				DebugSphere(obj->pbox->vert[k].pos.x,
				            obj->pbox->vert[k].pos.y,
				            obj->pbox->vert[k].pos.z,
				            0.6f, 40, 0xFF00FF00);
			}
			else if ((k == 0) || (k == 14) || (k == 13))
				DebugSphere(obj->pbox->vert[k].pos.x,
				            obj->pbox->vert[k].pos.y,
				            obj->pbox->vert[k].pos.z,
				            0.6f, 40, 0xFFFFFF00);
			else if ((k > 0) && (k < 5))
				DebugSphere(obj->pbox->vert[k].pos.x,
				            obj->pbox->vert[k].pos.y,
				            obj->pbox->vert[k].pos.z,
				            0.6f, 40, 0xFF00FF00);
			else if ((k > 4) && (k < 9))
				DebugSphere(obj->pbox->vert[k].pos.x,
				            obj->pbox->vert[k].pos.y,
				            obj->pbox->vert[k].pos.z,
				            0.6f, 40, 0xFF0000FF);
			else
				DebugSphere(obj->pbox->vert[k].pos.x,
				            obj->pbox->vert[k].pos.y,
				            obj->pbox->vert[k].pos.z,
				            0.6f, 40, 0xFFFF0000);
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

	if (obj->nbvertex == 0) return;

	obj->pbox =	(PHYSICS_BOX_DATA *)
	            malloc(sizeof(PHYSICS_BOX_DATA));
	memset(obj->pbox, 0, sizeof(PHYSICS_BOX_DATA));
	obj->pbox->nb_physvert = 15;
	obj->pbox->stopcount = 0;
	obj->pbox->vert =	(PHYSVERT *)
	                    malloc(sizeof(PHYSVERT) * obj->pbox->nb_physvert);
	memset(obj->pbox->vert, 0, sizeof(PHYSVERT)*obj->pbox->nb_physvert);

	EERIE_3D cubmin, cubmax;
	cubmin.x = FLT_MAX;
	cubmin.y = FLT_MAX;
	cubmin.z = FLT_MAX;

	cubmax.x = -FLT_MAX;
	cubmax.y = -FLT_MAX;
	cubmax.z = -FLT_MAX;

	for (long k = 0; k < obj->nbvertex; k++)
	{
		if (k == obj->origin) continue;

		cubmin.x = __min(cubmin.x, obj->vertexlist[k].v.x);
		cubmin.y = __min(cubmin.y, obj->vertexlist[k].v.y);
		cubmin.z = __min(cubmin.z, obj->vertexlist[k].v.z);

		cubmax.x = __max(cubmax.x, obj->vertexlist[k].v.x);
		cubmax.y = __max(cubmax.y, obj->vertexlist[k].v.y);
		cubmax.z = __max(cubmax.z, obj->vertexlist[k].v.z);
	}

	obj->pbox->vert[0].pos.x = cubmin.x + (cubmax.x - cubmin.x) * DIV2;
	obj->pbox->vert[0].pos.y = cubmin.y + (cubmax.y - cubmin.y) * DIV2;
	obj->pbox->vert[0].pos.z = cubmin.z + (cubmax.z - cubmin.z) * DIV2;

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
		float RATI = diff * DIV8;

		for (int k = 0; k < obj->nbvertex; k++)
		{
			if (k == obj->origin) continue;

			EERIE_3D curr;
			memcpy(&curr, &obj->vertexlist[k].v, sizeof(EERIE_3D));
			long SEC = 1;
			obj->pbox->vert[SEC].pos.x = __min(obj->pbox->vert[SEC].pos.x, curr.x);
			obj->pbox->vert[SEC].pos.z = __min(obj->pbox->vert[SEC].pos.z, curr.z);

			obj->pbox->vert[SEC+1].pos.x = __min(obj->pbox->vert[SEC+1].pos.x, curr.x);
			obj->pbox->vert[SEC+1].pos.z = __max(obj->pbox->vert[SEC+1].pos.z, curr.z);

			obj->pbox->vert[SEC+2].pos.x = __max(obj->pbox->vert[SEC+2].pos.x, curr.x);
			obj->pbox->vert[SEC+2].pos.z = __max(obj->pbox->vert[SEC+2].pos.z, curr.z);

			obj->pbox->vert[SEC+3].pos.x = __max(obj->pbox->vert[SEC+3].pos.x, curr.x);
			obj->pbox->vert[SEC+3].pos.z = __min(obj->pbox->vert[SEC+3].pos.z, curr.z);

			SEC = 5;
			obj->pbox->vert[SEC].pos.x = __min(obj->pbox->vert[SEC].pos.x, curr.x - RATI);
			obj->pbox->vert[SEC].pos.z = __min(obj->pbox->vert[SEC].pos.z, curr.z - RATI);

			obj->pbox->vert[SEC+1].pos.x = __min(obj->pbox->vert[SEC+1].pos.x, curr.x - RATI);
			obj->pbox->vert[SEC+1].pos.z = __max(obj->pbox->vert[SEC+1].pos.z, curr.z + RATI);

			obj->pbox->vert[SEC+2].pos.x = __max(obj->pbox->vert[SEC+2].pos.x, curr.x + RATI);
			obj->pbox->vert[SEC+2].pos.z = __max(obj->pbox->vert[SEC+2].pos.z, curr.z + RATI);

			obj->pbox->vert[SEC+3].pos.x = __max(obj->pbox->vert[SEC+3].pos.x, curr.x + RATI);
			obj->pbox->vert[SEC+3].pos.z = __min(obj->pbox->vert[SEC+3].pos.z, curr.z - RATI);


			SEC = 9;
			obj->pbox->vert[SEC].pos.x = __min(obj->pbox->vert[SEC].pos.x, curr.x);
			obj->pbox->vert[SEC].pos.z = __min(obj->pbox->vert[SEC].pos.z, curr.z);

			obj->pbox->vert[SEC+1].pos.x = __min(obj->pbox->vert[SEC+1].pos.x, curr.x);
			obj->pbox->vert[SEC+1].pos.z = __max(obj->pbox->vert[SEC+1].pos.z, curr.z);

			obj->pbox->vert[SEC+2].pos.x = __max(obj->pbox->vert[SEC+2].pos.x, curr.x);
			obj->pbox->vert[SEC+2].pos.z = __max(obj->pbox->vert[SEC+2].pos.z, curr.z);

			obj->pbox->vert[SEC+3].pos.x = __max(obj->pbox->vert[SEC+3].pos.x, curr.x);
			obj->pbox->vert[SEC+3].pos.z = __min(obj->pbox->vert[SEC+3].pos.z, curr.z);
		}
	}
	else
	{
		float cut = (cubmax.y - cubmin.y) * DIV3;
		float ysec2 = cubmin.y + cut * 2.f;
		float ysec1 = cubmin.y + cut;

		for (int k = 0; k < obj->nbvertex; k++)
		{
			if (k == obj->origin) continue;

			EERIE_3D curr;
			memcpy(&curr, &obj->vertexlist[k].v, sizeof(EERIE_3D));
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

			obj->pbox->vert[SEC].pos.x = __min(obj->pbox->vert[SEC].pos.x, curr.x);
			obj->pbox->vert[SEC].pos.z = __min(obj->pbox->vert[SEC].pos.z, curr.z);

			obj->pbox->vert[SEC+1].pos.x = __min(obj->pbox->vert[SEC+1].pos.x, curr.x);
			obj->pbox->vert[SEC+1].pos.z = __max(obj->pbox->vert[SEC+1].pos.z, curr.z);

			obj->pbox->vert[SEC+2].pos.x = __max(obj->pbox->vert[SEC+2].pos.x, curr.x);
			obj->pbox->vert[SEC+2].pos.z = __max(obj->pbox->vert[SEC+2].pos.z, curr.z);

			obj->pbox->vert[SEC+3].pos.x = __max(obj->pbox->vert[SEC+3].pos.x, curr.x);
			obj->pbox->vert[SEC+3].pos.z = __min(obj->pbox->vert[SEC+3].pos.z, curr.z);
		}
	}

	for (int k = 0; k < 4; k++)
	{
		if (EEfabs(obj->pbox->vert[5+k].pos.x - obj->pbox->vert[0].pos.x) < 2.f)
			obj->pbox->vert[5+k].pos.x = (obj->pbox->vert[1+k].pos.x + obj->pbox->vert[9+k].pos.x) * DIV2;

		if (EEfabs(obj->pbox->vert[5+k].pos.z - obj->pbox->vert[0].pos.z) < 2.f)
			obj->pbox->vert[5+k].pos.z = (obj->pbox->vert[1+k].pos.z + obj->pbox->vert[9+k].pos.z) * DIV2;
	}

	obj->pbox->radius = 0.f;

	for (int k = 0; k < obj->pbox->nb_physvert; k++)
	{
		float distt = TRUEEEDistance3D(&obj->pbox->vert[k].pos, &obj->pbox->vert[0].pos);

		if (distt > 20.f)
		{
			obj->pbox->vert[k].pos.x = (obj->pbox->vert[k].pos.x
			                            - obj->pbox->vert[0].pos.x) * 0.5f +
			                           obj->pbox->vert[0].pos.x;
			obj->pbox->vert[k].pos.z = (obj->pbox->vert[k].pos.z
			                            - obj->pbox->vert[0].pos.z) * 0.5f +
			                           obj->pbox->vert[0].pos.z;
		}

		memcpy(&obj->pbox->vert[k].initpos, &obj->pbox->vert[k].pos, sizeof(EERIE_3D));

		if (k != 0)
		{
			float dist = TRUEEEDistance3D(&obj->pbox->vert[0].pos, &obj->pbox->vert[k].pos);
			obj->pbox->radius = __max(obj->pbox->radius, dist);
		}
	}
}
