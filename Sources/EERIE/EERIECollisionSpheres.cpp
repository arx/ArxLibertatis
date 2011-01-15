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
//////////////////////////////////////////////////////////////////////////////////////
// EERIECollisionSpheres
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		CollisionSpheres creation/handling
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
#include <math.h>

#include "EERIECollisionSpheres.h"
#include "EERIEMath.h"
#include "EERIEMeshTweak.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

void EERIE_COLLISION_Cylinder_Create(INTERACTIVE_OBJ * io)
{
	if (io == NULL) return;

	EERIE_3DOBJ * obj = io->obj;

	if (!obj) return;

	if (obj->vertexlist == NULL)
	{
		io->physics.cyl.height = 0.f;
		return;
	}

	io->physics.cyl.origin.x = obj->vertexlist[obj->origin].v.x;
	io->physics.cyl.origin.y = obj->vertexlist[obj->origin].v.y;
	io->physics.cyl.origin.z = obj->vertexlist[obj->origin].v.z;
	float dist = 0.f;
	float height = 0.f;

	for (long i = 0; i < obj->nbvertex; i++)
	{
		if ((i != obj->origin) && (EEfabs(io->physics.cyl.origin.y - obj->vertexlist[i].v.y) < 20.f))
		{
			dist = __max(dist, TRUEDistance3D(io->physics.cyl.origin.x, io->physics.cyl.origin.y, io->physics.cyl.origin.z,
			                                  obj->vertexlist[i].v.x, obj->vertexlist[i].v.y, obj->vertexlist[i].v.z));
		}

		height = __max(height, io->physics.cyl.origin.y - obj->vertexlist[i].v.y);
	}

	if ((dist == 0.f) || (height == 0.f))
	{
		io->physics.cyl.height = 0.f;
		return;
	}

	io->original_radius = dist * 1.2f;
	io->original_height = -height;
	io->physics.cyl.origin.x = io->pos.x;
	io->physics.cyl.origin.y = io->pos.y;
	io->physics.cyl.origin.z = io->pos.z;

	if (io->original_height > -40)
	{
		float v = (-io->original_height) * DIV40;
		io->original_radius *= (0.5f + v * 0.5f);
	}

	if (io->original_height > -40) io->original_height = -40;

	if (io->original_height < -165) io->original_height = -165;

	if (io->original_radius > 40.f) io->original_radius = 40.f;

	io->physics.cyl.radius = io->original_radius * io->scale;
	io->physics.cyl.height = io->original_height * io->scale;
}

void EERIE_COLLISION_SPHERES_Release(EERIE_3DOBJ * obj)
{
	if (obj == NULL) return;

	if (obj->sdata == NULL) return;

	obj->sdata->nb_spheres = 0;

	if (obj->sdata->spheres)
	{
		free(obj->sdata->spheres);
		obj->sdata->spheres = NULL;
	}

	free(obj->sdata);
	obj->sdata = NULL;
}

void AddCollisionSphere(EERIE_3DOBJ * obj, long idx, float radius, long flags)
{
	if (radius < 1.f) return;

	for (long i = 0; i < obj->sdata->nb_spheres; i++)
	{
		COLLISION_SPHERE * cs = &obj->sdata->spheres[i];

		if (cs->idx == i)
		{
			if (radius < cs->radius) return;

			cs->radius = radius;
			return;
		}
	}

	if (obj->sdata->nb_spheres == 0)
		obj->sdata->spheres = (COLLISION_SPHERE *)
		                      malloc(sizeof(COLLISION_SPHERE));

	else obj->sdata->spheres = (COLLISION_SPHERE *)
		                           realloc(obj->sdata->spheres, sizeof(COLLISION_SPHERE) * (obj->sdata->nb_spheres + 1));

	memset(&obj->sdata->spheres[obj->sdata->nb_spheres], 0, sizeof(COLLISION_SPHERE));
	obj->sdata->spheres[obj->sdata->nb_spheres].idx = (short)idx;
	obj->sdata->spheres[obj->sdata->nb_spheres].radius = radius;
	obj->sdata->spheres[obj->sdata->nb_spheres].flags = (short)flags;
	obj->sdata->nb_spheres++;
}
long GetFirstChildGroup(EERIE_3DOBJ * obj, long group)
{
	if (obj->nbgroups < group + 2) return -1;

	for (long k = group + 1; k < obj->nbgroups; k++)
	{
		for (long i = 0; i < obj->grouplist[group].nb_index; i++)
		{
			if (obj->grouplist[group].indexes[i] == obj->grouplist[k].origin)
			{
				return k;
			}
		}
	}

	return -1;
}
 
BOOL IsExclusiveGroupMember(EERIE_3DOBJ * obj, long idx, long group)
{

	long exclusive = 1;

	for (long i = group + 1; i < obj->nbgroups; i++)
	{
		for (long j = 0; j < obj->grouplist[i].nb_index; j++)
		{
			if (idx == obj->grouplist[i].indexes[j])
			{
				exclusive = 0;
			}
		}
	}

	if (exclusive) return TRUE;
	else return FALSE;
}

float GetSphereRadiusForGroup(EERIE_3DOBJ * obj, EERIE_3D * center, EERIE_3D * dirvect, long group, float maxi)
{
	float curradius = 0.f;
	float max = 0.f;
	float div = 0.f;
	long sel = -1;

	for (long i = 0; i < obj->nbselections; i++)
	{
		if (!stricmp(obj->selections[i].name, "mou"))
		{
			sel = i;
			break;
		}
	}

	for (int i = 0; i < obj->grouplist[group].nb_index; i++)
	{
		if (!IsExclusiveGroupMember(obj, obj->grouplist[group].indexes[i], group)) continue;

		if ((sel > -1) && (IsInSelection(obj, obj->grouplist[group].indexes[i], sel) >= 0)) continue;

		EERIE_3D target;
		target.x = obj->vertexlist[obj->grouplist[group].indexes[i]].v.x;
		target.y = obj->vertexlist[obj->grouplist[group].indexes[i]].v.y;
		target.z = obj->vertexlist[obj->grouplist[group].indexes[i]].v.z;
		float distance = Distance3D(center->x, center->y, center->z,
		                            target.x, target.y, target.z);

		if (distance < 2.f) continue;

		if (distance < max) continue;

		EERIE_3D targvect;
		targvect.x = target.x - center->x;
		targvect.y = target.y - center->y;
		targvect.z = target.z - center->z;

		float divdist = 1.f / distance;
		targvect.x *= divdist;
		targvect.y *= divdist;
		targvect.z *= divdist;
		float val = Vector_DotProduct(dirvect, &targvect);

		if (EEfabs(val) < 1.2f)
		{
			if (distance > maxi) distance = maxi;

			curradius += distance;
			div += 1.f;
			max = __max(max, distance);
		}
	}

	if (div > 0.f)
	{
		curradius /= div;
	}

	return (curradius);
}

long AddVertexToVertexList(EERIE_3DOBJ * obj, EERIE_3D * center, long group)
{
	if (obj->vertexlist == NULL) return -1;

	for (long i = 0; i < obj->nbvertex; i++)
	{
		if ((center->x == obj->vertexlist[i].v.x)
		        &&	(center->y == obj->vertexlist[i].v.y)
		        &&	(center->z == obj->vertexlist[i].v.z))
		{
			if (IsVertexIdxInGroup(obj, i, group) == FALSE)
			{
				if ((obj->vertexlist[i].norm.x == 50.f)
				        &&	(obj->vertexlist[i].norm.y == 50.f)
				        &&	(obj->vertexlist[i].norm.z == 50.f)
				   )
					AddVertexIdxToGroup(obj, group, i);
			}

			return i;
		}
	}

	// Must create a new one...
	obj->vertexlist = (EERIE_VERTEX *)
	                  realloc(obj->vertexlist, sizeof(EERIE_VERTEX) * (obj->nbvertex + 1));

	obj->vertexlist3 = (EERIE_VERTEX *)
	                   realloc(obj->vertexlist3, sizeof(EERIE_VERTEX) * (obj->nbvertex + 1));

	memset(&obj->vertexlist[obj->nbvertex], 0, sizeof(EERIE_VERTEX));

	if (obj->pdata)
	{
		obj->pdata = (PROGRESSIVE_DATA *)
		             realloc(obj->pdata, sizeof(PROGRESSIVE_DATA) * (obj->nbvertex + 1));

		memset(&obj->pdata[obj->nbvertex], 0, sizeof(PROGRESSIVE_DATA));
		obj->pdata[obj->nbvertex].collapse_candidate = -1;
		obj->pdata[obj->nbvertex].collapse_cost = 1000000;
	}

	obj->vertexlist[obj->nbvertex].v.x = center->x;
	obj->vertexlist[obj->nbvertex].v.y = center->y;
	obj->vertexlist[obj->nbvertex].v.z = center->z;
	obj->vertexlist[obj->nbvertex].norm.x = 50.f;
	obj->vertexlist[obj->nbvertex].norm.y = 50.f;
	obj->vertexlist[obj->nbvertex].norm.z = 50.f;

	memcpy(&obj->vertexlist3[obj->nbvertex], &obj->vertexlist[obj->nbvertex], sizeof(EERIE_VERTEX));
	long num = obj->nbvertex;
	obj->nbvertex++;
	AddVertexIdxToGroup(obj, group, num);

	return num;
}
void EERIE_COLLISION_SPHERES_Create(EERIE_3DOBJ * obj)
{
	if (obj == NULL) return;

	EERIE_COLLISION_SPHERES_Release(obj);
	obj->sdata = (COLLISION_SPHERES_DATA *)
	             malloc(sizeof(COLLISION_SPHERES_DATA));

	memset(obj->sdata, 0, sizeof(COLLISION_SPHERES_DATA));

	for (long k = 1; k < obj->nbgroups; k++)
	{
		long workon;
		workon = GetFirstChildGroup(obj, k);

		if (workon != -1)
		{
			EERIE_3D center; // Group origin pos
			center.x = obj->vertexlist[obj->grouplist[k].origin].v.x;
			center.y = obj->vertexlist[obj->grouplist[k].origin].v.y;
			center.z = obj->vertexlist[obj->grouplist[k].origin].v.z;
			EERIE_3D dest;  // Destination Group origin pos
			dest.x = obj->vertexlist[obj->grouplist[workon].origin].v.x;
			dest.y = obj->vertexlist[obj->grouplist[workon].origin].v.y;
			dest.z = obj->vertexlist[obj->grouplist[workon].origin].v.z;
			EERIE_3D dirvect; // Direction Vector from Origin Group to Destination Origin Group
			dirvect.x = dest.x - center.x;
			dirvect.y = dest.y - center.y;
			dirvect.z = dest.z - center.z;
			// Distance between those 2 pos
			float dista = Distance3D(dest.x, dest.y, dest.z, center.x, center.y, center.z);
			float tot = dista;
			float divdist = 1.f / dista;
			// Vector Normalization
			dirvect.x *= divdist;
			dirvect.y *= divdist;
			dirvect.z *= divdist;
	
			while (dista >= 0.f) // Iterate along the whole distance
			{
				// Compute required radius for this group and that pos
				float val = GetSphereRadiusForGroup(obj, &center, &dest, k, tot * 1.4f);

				if (val > 0.2f)
				{
					long idx = AddVertexToVertexList(obj, &center, k);

					if (idx > -1)
						AddCollisionSphere(obj, idx, val, 0);

					val *= DIV2;
				}
				else val = 0.15f;
			
				float inc = val;
				dista -= inc;
			
				center.x += dirvect.x * inc;
				center.y += dirvect.y * inc;
				center.z += dirvect.z * inc;
			}
		}
		else AddCollisionSphere(obj, obj->grouplist[k].origin, obj->grouplist[k].siz * 18.f, 0);
	

	}

	if (obj->sdata->nb_spheres == 0) EERIE_COLLISION_SPHERES_Release(obj);
}
