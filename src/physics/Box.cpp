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
// Code: Cyril Meynier
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved

#include "physics/Box.h"

#include "graphics/Math.h"

using std::min;
using std::max;

#define FULLTESTS 0

long CUR_COLLISION_MATERIAL = 0;

// Used to launch an object into the physical world...
void EERIE_PHYSICS_BOX_Launch(EERIE_3DOBJ * obj, const Vec3f & pos, const Anglef & angle, const Vec3f & vect)
{
	if ((!obj) || !(obj->pbox)) return;
	
	obj->pbox->storedtiming = 0;
	
	for(size_t i = 0; i < obj->vertexlist.size(); i++) {
		obj->vertexlist[i].vert.p = obj->vertexlist[i].v;
	}
	
	float surface = 0.f;
	
	for(size_t i = 0; i < obj->facelist.size(); i++) {
		const Vec3f & p0 = obj->vertexlist[obj->facelist[i].vid[0]].v;
		const Vec3f & p1 = obj->vertexlist[obj->facelist[i].vid[1]].v;
		const Vec3f & p2 = obj->vertexlist[obj->facelist[i].vid[2]].v;
		surface += glm::distance((p0 + p1) * .5f, p2) * glm::distance(p0, p1) * .5f;
	}

	float ratio = surface * ( 1.0f / 10000 );

	ratio = clamp(ratio, 0.f, 0.8f);

	ratio = 1.f - (ratio * ( 1.0f / 4 ));
	
	for(int i = 0; i < obj->pbox->nb_physvert; i++) {
		PHYSVERT * pv = &obj->pbox->vert[i];
		pv->pos = pv->initpos;
		VRotateY(&pv->pos, angle.getPitch());
		VRotateX(&pv->pos, angle.getYaw());
		VRotateZ(&pv->pos, angle.getRoll());
		pv->pos += pos;

		pv->inertia = Vec3f_ZERO;
		pv->force = Vec3f_ZERO;
		pv->velocity = vect * (250.f * ratio);
		pv->mass = 0.4f + ratio * 0.1f;
	}
	
	obj->pbox->active = 1;
	obj->pbox->stopcount = 0;
}

bool IsValidPos3(Vec3f * pos)
{
	long px = pos->x * ACTIVEBKG->Xmul;
	long pz = pos->z * ACTIVEBKG->Zmul;

	if(px < 0 || px >= ACTIVEBKG->Xsize || pz < 0 || pz >= ACTIVEBKG->Zsize)
		return false;

	EERIE_BKG_INFO *eg = &ACTIVEBKG->Backg[px + pz * ACTIVEBKG->Xsize];

	if(eg->nbpolyin <= 0 || pos->y > eg->tile_maxy)
		return false;

	return true;
}

// Checks is a triangle of a physical object is colliding a triangle
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
			if(!fartherThan(center, vert[nn].pos, max(60.0f, rad + 25))) {
				nn = 1000;
			}
		}

		if (nn < 1000)
			return false;
	}
	else
	{
		if(fartherThan(center, vert[k].pos, rad + 25)) {
			return false;
		}
	}

	//TOP
	if ((k == -1) || (k == 1) || (k == 2) || (k == 3))
	{
		t1.v[0] = vert[1].pos;
		t1.v[1] = vert[2].pos;
		t1.v[2] = vert[3].pos;

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
		}
	}

#if FULLTESTS

	if ((k == -1) || (k == 1) || (k == 4) || (k == 3))
	{
		t1.v[0] = vert[3].pos;
		t1.v[1] = vert[4].pos;
		t1.v[2] = vert[1].pos;

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
		}
	}

#endif

	//BOTTOM
	if ((k == -1) || (k == 9) || (k == 10) || (k == 12))
	{
		t1.v[0] = vert[10].pos;
		t1.v[1] = vert[9].pos;
		t1.v[2] = vert[11].pos;

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
		}
	}

#if FULLTESTS

	if ((k == -1) || (k == 10) || (k == 11) || (k == 12))
	{
		t1.v[0] = vert[9].pos;
		t1.v[1] = vert[12].pos;
		t1.v[2] = vert[11].pos;

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
		}
	}

#endif

	//UP/FRONT
	if ((k == -1) || (k == 1) || (k == 4) || (k == 5))
	{
		t1.v[0] = vert[1].pos;
		t1.v[1] = vert[4].pos;
		t1.v[2] = vert[5].pos;

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
		}
	}

#if FULLTESTS

	if ((k == -1) || (k == 4) || (k == 5) || (k == 8))
	{
		t1.v[0] = vert[4].pos;
		t1.v[1] = vert[8].pos;
		t1.v[2] = vert[5].pos;

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
		}
	}

#endif

	//DOWN/FRONT
	if ((k == -1) || (k == 5) || (k == 8) || (k == 9))
	{
		t1.v[0] = vert[5].pos;
		t1.v[1] = vert[8].pos;
		t1.v[2] = vert[9].pos;

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
		}
	}

#if FULLTESTS

	if ((k == -1) || (k == 8) || (k == 12) || (k == 9))
	{
		t1.v[0] = vert[8].pos;
		t1.v[1] = vert[12].pos;
		t1.v[2] = vert[9].pos;

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
		}
	}

#endif

	//UP/BACK
	if ((k == -1) || (k == 3) || (k == 2) || (k == 7))
	{
		t1.v[0] = vert[3].pos;
		t1.v[1] = vert[2].pos;
		t1.v[2] = vert[7].pos;

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
		}
	}

#if FULLTESTS

	if ((k == -1) || (k == 2) || (k == 6) || (k == 7))
	{
		t1.v[0] = vert[2].pos;
		t1.v[1] = vert[6].pos;
		t1.v[2] = vert[7].pos;

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
		}
	}

#endif

	//DOWN/BACK
	if ((k == -1) || (k == 7) || (k == 6) || (k == 11))
	{
		t1.v[0] = vert[7].pos;
		t1.v[1] = vert[6].pos;
		t1.v[2] = vert[11].pos;

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
		}
	}

#if FULLTESTS

	if ((k == -1) || (k == 6) || (k == 10) || (k == 11))
	{
		t1.v[0] = vert[6].pos;
		t1.v[1] = vert[10].pos;
		t1.v[2] = vert[11].pos;

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
		}
	}

#endif

	//UP/LEFT
	if ((k == -1) || (k == 1) || (k == 2) || (k == 6))
	{
		t1.v[0] = vert[6].pos;
		t1.v[1] = vert[2].pos;
		t1.v[2] = vert[1].pos;

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
		}
	}

#if FULLTESTS

	if ((k == -1) || (k == 1) || (k == 5) || (k == 6))
	{
		t1.v[0] = vert[1].pos;
		t1.v[1] = vert[5].pos;
		t1.v[2] = vert[6].pos;

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
		}
	}

#endif

	//DOWN/LEFT
	if ((k == -1) || (k == 10) || (k == 6) || (k == 5))
	{
		t1.v[0] = vert[10].pos;
		t1.v[1] = vert[6].pos;
		t1.v[2] = vert[5].pos;

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
		}
	}

#if FULLTESTS

	if ((k == -1) || (k == 5) || (k == 9) || (k == 10))
	{
		t1.v[0] = vert[5].pos;
		t1.v[1] = vert[9].pos;
		t1.v[2] = vert[10].pos;

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
		}
	}

#endif

	//UP/RIGHT
	if ((k == -1) || (k == 4) || (k == 3) || (k == 7))
	{
		t1.v[0] = vert[4].pos;
		t1.v[1] = vert[3].pos;
		t1.v[2] = vert[7].pos;

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
		}
	}

#if FULLTESTS

	if ((k == -1) || (k == 7) || (k == 8) || (k == 4))
	{
		t1.v[0] = vert[7].pos;
		t1.v[1] = vert[8].pos;
		t1.v[2] = vert[4].pos;

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
		}
	}

#endif

	//DOWN/RIGHT
	if ((k == -1) || (k == 8) || (k == 7) || (k == 11))
	{
		t1.v[0] = vert[8].pos;
		t1.v[1] = vert[7].pos;
		t1.v[2] = vert[11].pos;

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
		}
	}

#if FULLTESTS

	if ((k == -1) || (k == 11) || (k == 12) || (k == 8))
	{
		t1.v[0] = vert[11].pos;
		t1.v[1] = vert[12].pos;
		t1.v[2] = vert[8].pos;

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
		}
	}

#endif
	return ret;
}

// Releases physic box data from an object
void EERIE_PHYSICS_BOX_Release(EERIE_3DOBJ * obj) {
	
	if(!obj || !obj->pbox) {
		return;
	}
	
	free(obj->pbox->vert);
	obj->pbox->vert = NULL;

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

	obj->pbox =	(PHYSICS_BOX_DATA *) malloc(sizeof(PHYSICS_BOX_DATA));
	memset(obj->pbox, 0, sizeof(PHYSICS_BOX_DATA));

	obj->pbox->nb_physvert = 15;
	obj->pbox->stopcount = 0;

	obj->pbox->vert = (PHYSVERT *) malloc(sizeof(PHYSVERT) * obj->pbox->nb_physvert);
	memset(obj->pbox->vert, 0, sizeof(PHYSVERT)*obj->pbox->nb_physvert);
	
	Vec3f cubmin = Vec3f(std::numeric_limits<float>::max());
	Vec3f cubmax = Vec3f(-std::numeric_limits<float>::max());
	
	for(size_t k = 0; k < obj->vertexlist.size(); k++) {
		if(long(k) != obj->origin) {
			cubmin = glm::min(cubmin, obj->vertexlist[k].v);
			cubmax = glm::max(cubmax, obj->vertexlist[k].v);
		}
	}
	
	obj->pbox->vert[0].pos = cubmin + (cubmax - cubmin) * .5f;
	obj->pbox->vert[13].pos = obj->pbox->vert[0].pos;
	obj->pbox->vert[13].pos.y = cubmin.y;
	obj->pbox->vert[14].pos = obj->pbox->vert[0].pos;
	obj->pbox->vert[14].pos.y = cubmax.y;
	
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
			obj->pbox->vert[5+k].pos.x = (obj->pbox->vert[1+k].pos.x + obj->pbox->vert[9+k].pos.x) * .5f;

		if (EEfabs(obj->pbox->vert[5+k].pos.z - obj->pbox->vert[0].pos.z) < 2.f)
			obj->pbox->vert[5+k].pos.z = (obj->pbox->vert[1+k].pos.z + obj->pbox->vert[9+k].pos.z) * .5f;
	}

	obj->pbox->radius = 0.f;

	for(int k = 0; k < obj->pbox->nb_physvert; k++) {
		float distt = glm::distance(obj->pbox->vert[k].pos, obj->pbox->vert[0].pos);

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
			float d = glm::distance(obj->pbox->vert[0].pos, obj->pbox->vert[k].pos);
			obj->pbox->radius = max(obj->pbox->radius, d);
		}
	}
}
