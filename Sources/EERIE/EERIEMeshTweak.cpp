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
// EERIEMeshTweak
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		DynMeshTweaking
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>

#include "EERIEMeshTweak.h"
#include "EERIEApp.h"
#include "EERIEObject.h"
#include "EERIEMath.h"

#include "HERMESMain.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

void EERIE_MESH_ReleaseTransPolys(EERIE_3DOBJ * obj)
{
	if (!obj) return;

	for (long i = 0; i < INTERTRANSPOLYSPOS; i++)
	{
		for (long ii = 0; ii < obj->nbmaps; ii++)
		{
			if (InterTransTC[i] == obj->texturecontainer[ii])
			{
				for (long jj = 0; jj < obj->nbfaces; jj++)
				{
					if (InterTransFace[jj] == &obj->facelist[jj])
						InterTransFace[jj] = NULL;
				}
			}
		}
	}
}
 
void EERIE_MESH_TWEAK_Skin(EERIE_3DOBJ * obj, char * s1, char * s2)
{
	if (obj == NULL) return;

	if (s1 == NULL) return;

	if (s2 == NULL) return;

	char skintochange[512];
	char skinname[512];

	sprintf(skintochange, "Graph\\Obj3D\\Textures\\%s.bmp", s1);
	MakeUpcase(skintochange);
	sprintf(skinname, "%sGraph\\Obj3D\\Textures\\%s.bmp", Project.workingdir, s2);
	TextureContainer * tex = D3DTextr_CreateTextureFromFile(skinname, Project.workingdir);

	if (obj->originaltextures == NULL)
	{
		obj->originaltextures = (char *)malloc(256 * obj->nbmaps); 
		memset(obj->originaltextures, 0, 256 * obj->nbmaps);

		for (long i = 0; i < obj->nbmaps; i++)
		{
			if (obj->texturecontainer[i])
				strcpy(obj->originaltextures + 256 * i, obj->texturecontainer[i]->m_texName);
		}
	}

	if ((tex != NULL) && (obj->originaltextures != NULL))
	{
		for (long i = 0; i < obj->nbmaps; i++)
		{
			if ((strstr(obj->originaltextures + 256 * i, skintochange)))
			{
				strcpy(skintochange, obj->texturecontainer[i]->m_texName);
				break;
			}
		}

		tex->Restore(GDevice);
		TextureContainer * tex2 = FindTexture(skintochange);

		if (tex2)
			for (long i = 0; i < obj->nbmaps; i++)
			{
				if (obj->texturecontainer[i] == tex2)  obj->texturecontainer[i] = tex;
			}
	}
}

//**************************************************************************************************************
//**************************************************************************************************************
long IsInSelection(EERIE_3DOBJ * obj, long vert, long tw)
{
	if (obj == NULL) return -1;

	if (tw < 0) return -1;

	if (vert < 0) return -1;

	for (long i = 0; i < obj->selections[tw].nb_selected; i++)
	{
		if (obj->selections[tw].selected[i] == vert)
			return i;
	}

	return -1;
}
long GetEquivalentVertex(EERIE_3DOBJ * obj, EERIE_VERTEX * vert)
{
	for (long i = 0; i < obj->nbvertex; i++)
	{
		if ((obj->vertexlist[i].v.x == vert->v.x)
		        &&	(obj->vertexlist[i].v.y == vert->v.y)
		        &&	(obj->vertexlist[i].v.z == vert->v.z))
			return i;
	}

	return -1;
}

//*************************************************************************************
//*************************************************************************************
long ObjectAddVertex(EERIE_3DOBJ * obj, EERIE_VERTEX * vert)
{
	for (long i = 0; i < obj->nbvertex; i++)
	{
		if ((obj->vertexlist[i].v.x == vert->v.x)  
		        &&	(obj->vertexlist[i].v.y == vert->v.y) 
		        &&	(obj->vertexlist[i].v.z == vert->v.z)) 
			return i;
	}

	if (obj->nbvertex == 0)
	{
		obj->vertexlist = (EERIE_VERTEX *)malloc(sizeof(EERIE_VERTEX));
	}
	else obj->vertexlist = (EERIE_VERTEX *)realloc(obj->vertexlist, sizeof(EERIE_VERTEX) * (obj->nbvertex + 1));

	memcpy(&obj->vertexlist[obj->nbvertex], vert, sizeof(EERIE_VERTEX));
	obj->nbvertex++;
	return (obj->nbvertex - 1);
}
//*************************************************************************************
//*************************************************************************************
long GetActionPoint(EERIE_3DOBJ * obj, char * name)
{
	if (!obj) return -1;

	for (long n = 0; n < obj->nbaction; n++)
	{
		if (!stricmp(obj->actionlist[n].name, name))
			return obj->actionlist[n].idx;
	}

	return -1;
}
//*************************************************************************************
//*************************************************************************************
long ObjectAddFace(EERIE_3DOBJ * obj, EERIE_FACE * face, EERIE_3DOBJ * srcobj)
{
	long i;

	for (i = 0; i < obj->nbfaces; i++) // Check Already existing faces
	{
		if ((obj->vertexlist[obj->facelist[i].vid[0]].v.x == srcobj->vertexlist[face->vid[0]].v.x)
		        &&	(obj->vertexlist[obj->facelist[i].vid[1]].v.x == srcobj->vertexlist[face->vid[1]].v.x)
		        &&	(obj->vertexlist[obj->facelist[i].vid[2]].v.x == srcobj->vertexlist[face->vid[2]].v.x)
		        &&	(obj->vertexlist[obj->facelist[i].vid[0]].v.y == srcobj->vertexlist[face->vid[0]].v.y)
		        &&	(obj->vertexlist[obj->facelist[i].vid[1]].v.y == srcobj->vertexlist[face->vid[1]].v.y)
		        &&	(obj->vertexlist[obj->facelist[i].vid[2]].v.y == srcobj->vertexlist[face->vid[2]].v.y)
		        &&	(obj->vertexlist[obj->facelist[i].vid[0]].v.z == srcobj->vertexlist[face->vid[0]].v.z)
		        &&	(obj->vertexlist[obj->facelist[i].vid[1]].v.z == srcobj->vertexlist[face->vid[1]].v.z)
		        &&	(obj->vertexlist[obj->facelist[i].vid[2]].v.z == srcobj->vertexlist[face->vid[2]].v.z)
		   )

			return -1;
	}

	long f0, f1, f2;
	f0 = ObjectAddVertex(obj, &srcobj->vertexlist[face->vid[0]]);
	f1 = ObjectAddVertex(obj, &srcobj->vertexlist[face->vid[1]]);
	f2 = ObjectAddVertex(obj, &srcobj->vertexlist[face->vid[2]]);

	if ((f1 == -1) || (f2 == -1) || (f0 == -1)) return -1;

	if (obj->nbfaces == 0)
	{
		obj->facelist = (EERIE_FACE *)malloc(sizeof(EERIE_FACE));
	}
	else
	{
		obj->facelist = (EERIE_FACE *)realloc(obj->facelist, sizeof(EERIE_FACE) * (obj->nbfaces + 1));
	}

	memcpy(&obj->facelist[obj->nbfaces], face, sizeof(EERIE_FACE));

	obj->facelist[obj->nbfaces].vid[0] = (unsigned short)f0;
	obj->facelist[obj->nbfaces].vid[1] = (unsigned short)f1;
	obj->facelist[obj->nbfaces].vid[2] = (unsigned short)f2;
	obj->facelist[obj->nbfaces].texid = 0; 

	for (i = 0; i < obj->nbmaps; i++)
	{
		if (obj->texturecontainer[i] == srcobj->texturecontainer[face->texid])
		{
			obj->facelist[obj->nbfaces].texid = (short)i;
			break;
		}
	}

	obj->nbfaces++;
	return (obj->nbfaces - 1);
}
//*************************************************************************************
//*************************************************************************************
long ObjectAddAction(EERIE_3DOBJ * obj, char * name, long act,
                     long sfx, EERIE_VERTEX * vert) 
{
	long newvert = ObjectAddVertex(obj, vert);

	if (newvert < 0) return -1;

	if (obj->nbaction == 0)
	{
		obj->actionlist = (EERIE_ACTIONLIST *)malloc(sizeof(EERIE_ACTIONLIST)); 
	}
	else
	{
		for (long i = 0; i < obj->nbaction; i++)
		{
			if (!strcmp(obj->actionlist[i].name, name))
				return i;
		}

		obj->actionlist = (EERIE_ACTIONLIST *)realloc(obj->actionlist, sizeof(EERIE_ACTIONLIST) * (obj->nbaction + 1));
	}

	strcpy(obj->actionlist[obj->nbaction].name, name);
	obj->actionlist[obj->nbaction].act = act;
	obj->actionlist[obj->nbaction].sfx = sfx;
	obj->actionlist[obj->nbaction].idx = newvert;
	obj->nbaction++;
	return (obj->nbaction - 1);
}
//*************************************************************************************
//*************************************************************************************
long ObjectAddMap(EERIE_3DOBJ * obj, TextureContainer * tc)
{
	if (tc == NULL) return -1;

	for (long i = 0; i < obj->nbmaps; i++)
	{
		if (obj->texturecontainer[i] == tc)
			return i;
	}

	if (obj->nbmaps == 0)
		obj->texturecontainer = (TextureContainer **)malloc(sizeof(TextureContainer *)); 
	else
		obj->texturecontainer = (TextureContainer **)realloc(obj->texturecontainer, sizeof(TextureContainer *) * (obj->nbmaps + 1));

	obj->texturecontainer[obj->nbmaps] = tc;
	obj->nbmaps++;
	return (obj->nbmaps - 1);
}
//*************************************************************************************
//*************************************************************************************
void AddVertexToGroup(EERIE_3DOBJ * obj, long group, EERIE_VERTEX * vert)
{

	for (long i = 0; i < obj->nbvertex; i++)
	{
		if ((obj->vertexlist[i].v.x == vert->v.x)
		        &&	(obj->vertexlist[i].v.y == vert->v.y)
		        &&	(obj->vertexlist[i].v.z == vert->v.z))
		{
			AddVertexIdxToGroup(obj, group, i);
		}
	}
}

void AddVertexIdxToGroupWithoutInheritance(EERIE_3DOBJ * obj, long group, long val)
{
	if (obj->grouplist[group].nb_index == 0)
	{
		if (obj->grouplist[group].indexes) free(obj->grouplist[group].indexes);

		obj->grouplist[group].indexes = (long *)malloc(sizeof(long)); 
		obj->grouplist[group].indexes[obj->grouplist[group].nb_index] = val;
		obj->grouplist[group].nb_index++;
		return;
	}

	for (long i = 0; i < obj->grouplist[group].nb_index; i++)
	{
		if (obj->grouplist[group].indexes[i] == val) return;
	}

	obj->grouplist[group].indexes = (long *)realloc(obj->grouplist[group].indexes, sizeof(long) * (obj->grouplist[group].nb_index + 1));
	obj->grouplist[group].indexes[obj->grouplist[group].nb_index] = val;
	obj->grouplist[group].nb_index++;

}
void AddVertexIdxToGroup(EERIE_3DOBJ * obj, long group, long val)
{
	AddVertexIdxToGroupWithoutInheritance(obj, group, val);
}

void ObjectAddSelection(EERIE_3DOBJ * obj, long numsel, long vidx)
{
	for (long i = 0; i < obj->selections[numsel].nb_selected; i++)
	{
		if (obj->selections[numsel].selected[i] == vidx) return;
	}

	if (obj->selections[numsel].nb_selected == 0)
	{
		obj->selections[numsel].selected = (long *)malloc(sizeof(long)); 
	}
	else
	{
		obj->selections[numsel].selected = (long *)realloc(obj->selections[numsel].selected, sizeof(long) * (obj->selections[numsel].nb_selected + 1));
	}

	obj->selections[numsel].selected[obj->selections[numsel].nb_selected] = vidx;
	obj->selections[numsel].nb_selected++;
}

//*************************************************************************************
//*************************************************************************************
EERIE_3DOBJ * CreateIntermediaryMesh(EERIE_3DOBJ * obj1, EERIE_3DOBJ * obj2, long tw)
{
	long i;
	long tw1 = -1;
	long tw2 = -1;
	long iw1 = -1;
	long iw2 = -1;
	long jw1 = -1;
	long jw2 = -1;
	long sel_head1 = -1;
	long sel_head2 = -1;
	long sel_torso1 = -1;
	long sel_torso2 = -1;
	long sel_legs1 = -1;
	long sel_legs2 = -1;

	// First we retreive selection groups indexes
	for (i = 0; i < obj1->nbselections; i++)
	{
		if (!stricmp(obj1->selections[i].name, "head")) sel_head1 = i;
		else if (!stricmp(obj1->selections[i].name, "chest")) sel_torso1 = i;
		else if (!stricmp(obj1->selections[i].name, "leggings")) sel_legs1 = i;
	}

	for (i = 0; i < obj2->nbselections; i++)
	{
		if (!stricmp(obj2->selections[i].name, "head")) sel_head2 = i;
		else if (!stricmp(obj2->selections[i].name, "chest")) sel_torso2 = i;
		else if (!stricmp(obj2->selections[i].name, "leggings")) sel_legs2 = i;
	}

	if (sel_head1 == -1) return NULL;

	if (sel_head2 == -1) return NULL;

	if (sel_torso1 == -1) return NULL;

	if (sel_torso2 == -1) return NULL;

	if (sel_legs1 == -1) return NULL;

	if (sel_legs2 == -1) return NULL;

	if (tw == TWEAK_HEAD)
	{
		tw1 = sel_head1;
		tw2 = sel_head2;
		iw1 = sel_torso1;
		iw2 = sel_torso2;
		jw1 = sel_legs1;
		jw2 = sel_legs2;
	}

	if (tw == TWEAK_TORSO)
	{
		tw1 = sel_torso1;
		tw2 = sel_torso2;
		iw1 = sel_head1;
		iw2 = sel_head2;
		jw1 = sel_legs1;
		jw2 = sel_legs2;
	}

	if (tw == TWEAK_LEGS)
	{
		tw1 = sel_legs1;
		tw2 = sel_legs2;
		iw1 = sel_torso1;
		iw2 = sel_torso2;
		jw1 = sel_head1;
		jw2 = sel_head2;
	}

	if ((tw1 == -1) || (tw2 == -1)) return NULL;

	// Now Retreives Tweak Action Points
	{
		long idx_head1, idx_head2;
		long idx_torso1, idx_torso2;
		long idx_legs1, idx_legs2;

		idx_head1 = GetActionPoint(obj1, "head2chest");

		if (idx_head1 < 0) return NULL;

		idx_head2 = GetActionPoint(obj2, "head2chest");

		if (idx_head2 < 0) return NULL;

		idx_torso1 = GetActionPoint(obj1, "chest2leggings");

		if (idx_torso1 < 0) return NULL;

		idx_torso2 = GetActionPoint(obj2, "chest2leggings");

		if (idx_torso2 < 0) return NULL;

		idx_legs1 = obj1->origin;
		idx_legs2 = obj2->origin;

	}

	// copy vertexes
	EERIE_VERTEX * obj1vertexlist2 = NULL;
	EERIE_VERTEX * obj2vertexlist2 = NULL;
	obj1vertexlist2 = (EERIE_VERTEX *)malloc(sizeof(EERIE_VERTEX) * obj1->nbvertex);
	obj2vertexlist2 = (EERIE_VERTEX *)malloc(sizeof(EERIE_VERTEX) * obj2->nbvertex);
	memcpy(obj1vertexlist2, obj1->vertexlist, sizeof(EERIE_VERTEX)*obj1->nbvertex);
	memcpy(obj2vertexlist2, obj2->vertexlist, sizeof(EERIE_VERTEX)*obj2->nbvertex);

	// Work will contain the Tweaked object
	EERIE_3DOBJ * work = NULL;
	work = (EERIE_3DOBJ *)malloc(sizeof(EERIE_3DOBJ));
	memset(work, 0, sizeof(EERIE_3DOBJ));
	memcpy(&work->pos, &obj1->pos, sizeof(EERIE_3D));
	memcpy(&work->angle, &obj1->angle, sizeof(EERIE_3D));

	// ident will be the same as original object obj1
	work->ident = obj1->ident;

	// We reset all data to create a fresh object
	memcpy(&work->cub, &obj1->cub, sizeof(CUB3D));
	memcpy(&work->quat, &obj1->quat, sizeof(EERIE_QUAT));

	// Linked objects are linked to this object.
	if (obj1->nblinked > obj2->nblinked)
	{
		work->linked = (EERIE_LINKED *)malloc(obj1->nblinked * sizeof(EERIE_LINKED));
		memcpy(work->linked, obj1->linked, obj1->nblinked * sizeof(EERIE_LINKED));
		work->nblinked = obj1->nblinked;
	}
	else if (obj2->nblinked > 0)
	{
		work->linked = (EERIE_LINKED *)malloc(obj2->nblinked * sizeof(EERIE_LINKED));
		memcpy(work->linked, obj2->linked, obj2->nblinked * sizeof(EERIE_LINKED));
		work->nblinked = obj2->nblinked;
	}
	else
	{
		work->linked = NULL;
		work->nblinked = 0;
	}

	// Is the origin of object in obj1 or obj2 ? Retreives it for work object
	if (IsInSelection(obj1, obj1->origin, tw1) != -1)
	{
		work->point0.x = obj2->point0.x;
		work->point0.y = obj2->point0.y;
		work->point0.z = obj2->point0.z;
		work->origin = ObjectAddVertex(work, &obj2vertexlist2[obj2->origin]); 
	}
	else
	{
		work->point0.x = obj1->point0.x;
		work->point0.y = obj1->point0.y;
		work->point0.z = obj1->point0.z;
		work->origin = ObjectAddVertex(work, &obj1vertexlist2[obj1->origin]); 
	}

	// Recreate Action Points included in work object.for Obj1
	for (i = 0; i < obj1->nbaction; i++)
	{
		if ((IsInSelection(obj1, obj1->actionlist[i].idx, iw1) != -1)
		        ||	(IsInSelection(obj1, obj1->actionlist[i].idx, jw1) != -1)
		        || (!stricmp(obj1->actionlist[i].name, "head2chest"))
		        || (!stricmp(obj1->actionlist[i].name, "chest2leggings"))
		   )
		{
			ObjectAddAction(work, obj1->actionlist[i].name, obj1->actionlist[i].act,
			                obj1->actionlist[i].sfx, &obj1vertexlist2[obj1->actionlist[i].idx]);
		}
	}

	// Do the same for Obj2
	for (i = 0; i < obj2->nbaction; i++)
	{
		if ((IsInSelection(obj2, obj2->actionlist[i].idx, tw2) != -1)
		        || (!stricmp(obj1->actionlist[i].name, "head2chest"))
		        || (!stricmp(obj1->actionlist[i].name, "chest2leggings"))
		   )
		{
			ObjectAddAction(work, obj2->actionlist[i].name, obj2->actionlist[i].act,
			                obj2->actionlist[i].sfx, &obj2vertexlist2[obj2->actionlist[i].idx]);
		}
	}

	// Recreate Vertex using Obj1 Vertexes
	for (i = 0; i < obj1->nbvertex; i++)
	{
		if ((IsInSelection(obj1, i, iw1) != -1)
		        ||	(IsInSelection(obj1, i, jw1) != -1))
		{
			ObjectAddVertex(work, &obj1vertexlist2[i]);
		}
	}

	// The same for Obj2
	for (i = 0; i < obj2->nbvertex; i++)
	{
		if (IsInSelection(obj2, i, tw2) != -1)
		{
			ObjectAddVertex(work, &obj2vertexlist2[i]);
		}
	}


	// Look in Faces for forgotten Vertexes... AND
	// Re-Create TextureContainers Infos
	// We look for texturecontainers included in the future tweaked object
	TextureContainer * tc = NULL;

	for (i = 0; i < obj1->nbfaces; i++)
	{
		if (((IsInSelection(obj1, obj1->facelist[i].vid[0], iw1) != -1)
		        ||	(IsInSelection(obj1, obj1->facelist[i].vid[0], jw1) != -1))
		        &&	((IsInSelection(obj1, obj1->facelist[i].vid[1], iw1) != -1)
		             ||	(IsInSelection(obj1, obj1->facelist[i].vid[1], jw1) != -1))
		        &&	((IsInSelection(obj1, obj1->facelist[i].vid[2], iw1) != -1)
		             ||	(IsInSelection(obj1, obj1->facelist[i].vid[2], jw1) != -1))
		   )
		{

			if (obj1->facelist[i].texid != -1)
				if (tc != obj1->texturecontainer[obj1->facelist[i].texid])
				{
					tc = obj1->texturecontainer[obj1->facelist[i].texid];
					ObjectAddMap(work, tc);
				}

			ObjectAddFace(work, &obj1->facelist[i], obj1);
		}
	}

	for (i = 0; i < obj2->nbfaces; i++)
	{
		if ((IsInSelection(obj2, obj2->facelist[i].vid[0], tw2) != -1)
		        || (IsInSelection(obj2, obj2->facelist[i].vid[1], tw2) != -1)
		        || (IsInSelection(obj2, obj2->facelist[i].vid[2], tw2) != -1))
		{

			if (obj2->facelist[i].texid != -1)
				if (tc != obj2->texturecontainer[obj2->facelist[i].texid])
				{
					tc = obj2->texturecontainer[obj2->facelist[i].texid];
					ObjectAddMap(work, tc);
				}

			ObjectAddFace(work, &obj2->facelist[i], obj2);
		}
	}

	// Recreate Groups
	work->nbgroups = __max(obj1->nbgroups, obj2->nbgroups);
	work->grouplist = (EERIE_GROUPLIST *)malloc(sizeof(EERIE_GROUPLIST) * work->nbgroups); 
	memset(work->grouplist, 0, sizeof(EERIE_GROUPLIST)*work->nbgroups);

	for (long k = 0; k < obj1->nbgroups; k++)
	{
		strcpy(work->grouplist[k].name, obj1->grouplist[k].name);
		long v = GetEquivalentVertex(work, &obj1vertexlist2[obj1->grouplist[k].origin]);

		if (v >= 0)
		{
			work->grouplist[k].siz = obj1->grouplist[k].siz;

			if ((IsInSelection(obj1, obj1->grouplist[k].origin, iw1) != -1)
			        || (IsInSelection(obj1, obj1->grouplist[k].origin, jw1) != -1))
				work->grouplist[k].origin = v;
		}
	}

	for (int k = 0; k < obj2->nbgroups; k++)
	{
		if (k >= obj1->nbgroups)
		{
			strcpy(work->grouplist[k].name, obj2->grouplist[k].name);

		}

		long v = GetEquivalentVertex(work, &obj2vertexlist2[obj2->grouplist[k].origin]);

		if (v >= 0)
		{
			work->grouplist[k].siz = obj2->grouplist[k].siz;

			if (IsInSelection(obj2, obj2->grouplist[k].origin, tw2) != -1)
				work->grouplist[k].origin = v;
		}
	}

	// Recreate Selection Groups (only the 3 selections needed to reiterate MeshTweaking !)
	work->nbselections = 3;
	work->selections = (EERIE_SELECTIONS *)malloc(sizeof(EERIE_SELECTIONS) * work->nbselections); 
	memset(work->selections, 0, sizeof(EERIE_SELECTIONS)*work->nbselections);
	strcpy(work->selections[0].name, "head");
	strcpy(work->selections[1].name, "chest");
	strcpy(work->selections[2].name, "leggings");

	// Re-Creating sel_head
	if (tw == TWEAK_HEAD)
	{
		for (long l = 0; l < obj2->selections[sel_head2].nb_selected; l++)
		{
			EERIE_VERTEX temp;
			temp.v.x = obj2vertexlist2[obj2->selections[sel_head2].selected[l]].v.x;
			temp.v.y = obj2vertexlist2[obj2->selections[sel_head2].selected[l]].v.y;
			temp.v.z = obj2vertexlist2[obj2->selections[sel_head2].selected[l]].v.z;
			long t = GetEquivalentVertex(work, &temp);

			if (t != -1)
			{
				ObjectAddSelection(work, 0, t);
			}
		}
	}
	else for (long l = 0; l < obj1->selections[sel_head1].nb_selected; l++)
		{
			EERIE_VERTEX temp;
			temp.v.x = obj1vertexlist2[obj1->selections[sel_head1].selected[l]].v.x;
			temp.v.y = obj1vertexlist2[obj1->selections[sel_head1].selected[l]].v.y;
			temp.v.z = obj1vertexlist2[obj1->selections[sel_head1].selected[l]].v.z;
			long t = GetEquivalentVertex(work, &temp);

			if (t != -1)
			{
				ObjectAddSelection(work, 0, t);
			}
		}

	// Re-Create sel_torso
	if (tw == TWEAK_TORSO)
	{
		for (long l = 0; l < obj2->selections[sel_torso2].nb_selected; l++)
		{
			EERIE_VERTEX temp;
			temp.v.x = obj2vertexlist2[obj2->selections[sel_torso2].selected[l]].v.x;
			temp.v.y = obj2vertexlist2[obj2->selections[sel_torso2].selected[l]].v.y;
			temp.v.z = obj2vertexlist2[obj2->selections[sel_torso2].selected[l]].v.z;
			long t = GetEquivalentVertex(work, &temp);

			if (t != -1)
			{
				ObjectAddSelection(work, 1, t);
			}
		}
	}
	else for (long l = 0; l < obj1->selections[sel_torso1].nb_selected; l++)
		{
			EERIE_VERTEX temp;
			temp.v.x = obj1vertexlist2[obj1->selections[sel_torso1].selected[l]].v.x;
			temp.v.y = obj1vertexlist2[obj1->selections[sel_torso1].selected[l]].v.y;
			temp.v.z = obj1vertexlist2[obj1->selections[sel_torso1].selected[l]].v.z;
			long t = GetEquivalentVertex(work, &temp);

			if (t != -1)
			{
				ObjectAddSelection(work, 1, t);
			}
		}

	// Re-Create sel_legs
	if (tw == TWEAK_LEGS)
	{
		for (long l = 0; l < obj2->selections[sel_legs2].nb_selected; l++)
		{
			EERIE_VERTEX temp;
			temp.v.x = obj2vertexlist2[obj2->selections[sel_legs2].selected[l]].v.x;
			temp.v.y = obj2vertexlist2[obj2->selections[sel_legs2].selected[l]].v.y;
			temp.v.z = obj2vertexlist2[obj2->selections[sel_legs2].selected[l]].v.z;
			long t = GetEquivalentVertex(work, &temp);

			if (t != -1)
			{
				ObjectAddSelection(work, 2, t);
			}
		}
	}
	else for (long l = 0; l < obj1->selections[sel_legs1].nb_selected; l++)
		{
			EERIE_VERTEX temp;
			temp.v.x = obj1vertexlist2[obj1->selections[sel_legs1].selected[l]].v.x;
			temp.v.y = obj1vertexlist2[obj1->selections[sel_legs1].selected[l]].v.y;
			temp.v.z = obj1vertexlist2[obj1->selections[sel_legs1].selected[l]].v.z;
			long t = GetEquivalentVertex(work, &temp);

			if (t != -1)
			{
				ObjectAddSelection(work, 2, t);
			}
		}

	//Now recreates other selections...
	for (i = 0; i < obj1->nbselections; i++)
	{
		if (EERIE_OBJECT_GetSelection(work, obj1->selections[i].name) == -1)
		{
			long num = work->nbselections;
			work->nbselections++;
			work->selections = (EERIE_SELECTIONS *)realloc(work->selections, sizeof(EERIE_SELECTIONS) * work->nbselections);
			memset(&work->selections[num], 0, sizeof(EERIE_SELECTIONS));
			strcpy(work->selections[num].name, obj1->selections[i].name);

			for (long l = 0; l < obj1->selections[i].nb_selected; l++)
			{
				EERIE_VERTEX temp;
				temp.v.x = obj1vertexlist2[obj1->selections[i].selected[l]].v.x;
				temp.v.y = obj1vertexlist2[obj1->selections[i].selected[l]].v.y;
				temp.v.z = obj1vertexlist2[obj1->selections[i].selected[l]].v.z;
				long t = GetEquivalentVertex(work, &temp);

				if (t != -1)
				{
					ObjectAddSelection(work, num, t);
				}
			}

			long ii = EERIE_OBJECT_GetSelection(obj2, obj1->selections[i].name);

			if (ii != -1)
				for (long l = 0; l < obj2->selections[ii].nb_selected; l++)
				{
					EERIE_VERTEX temp;
					temp.v.x = obj2vertexlist2[obj2->selections[ii].selected[l]].v.x;
					temp.v.y = obj2vertexlist2[obj2->selections[ii].selected[l]].v.y;
					temp.v.z = obj2vertexlist2[obj2->selections[ii].selected[l]].v.z;
					long t = GetEquivalentVertex(work, &temp);

					if (t != -1)
					{
						ObjectAddSelection(work, num, t);
					}
				}
		}
	}

	for (i = 0; i < obj2->nbselections; i++)
	{
		if (EERIE_OBJECT_GetSelection(work, obj2->selections[i].name) == -1)
		{
			long num = work->nbselections;
			work->nbselections++;
			work->selections = (EERIE_SELECTIONS *)realloc(work->selections, sizeof(EERIE_SELECTIONS) * work->nbselections);
			memset(&work->selections[num], 0, sizeof(EERIE_SELECTIONS));
			strcpy(work->selections[num].name, obj2->selections[i].name);

			for (long l = 0; l < obj2->selections[i].nb_selected; l++)
			{
				EERIE_VERTEX temp;
				temp.v.x = obj2vertexlist2[obj2->selections[i].selected[l]].v.x;
				temp.v.y = obj2vertexlist2[obj2->selections[i].selected[l]].v.y;
				temp.v.z = obj2vertexlist2[obj2->selections[i].selected[l]].v.z;
				long t = GetEquivalentVertex(work, &temp);

				if (t != -1)
				{
					ObjectAddSelection(work, num, t);
				}
			}
		}
	}

	// Recreate Animation-groups vertex
	for (i = 0; i < obj1->nbgroups; i++)
	{
		for (long j = 0; j < obj1->grouplist[i].nb_index; j++)
		{
			AddVertexToGroup(work, i, &obj1vertexlist2[obj1->grouplist[i].indexes[j]]);
		}
	}

	for (i = 0; i < obj2->nbgroups; i++)
	{
		for (long j = 0; j < obj2->grouplist[i].nb_index; j++)
		{
			AddVertexToGroup(work, i, &obj2vertexlist2[obj2->grouplist[i].indexes[j]]);
		}
	}

	// Look for Vertices Without Group...
	for (i = 0; i < work->nbmaps; i++)
	{
		work->texturecontainer[i]->Restore(GDevice);
	}

	if (work->nbvertex)
	{
		work->vertexlist3 = (EERIE_VERTEX *)malloc(sizeof(EERIE_VERTEX) * work->nbvertex);
		memcpy(work->vertexlist3, work->vertexlist, sizeof(EERIE_VERTEX)*work->nbvertex);
	}

	if (obj1vertexlist2)
		free(obj1vertexlist2);

	if (obj2vertexlist2)
		free(obj2vertexlist2);

	return work;
}
long ALLOW_MESH_TWEAKING = 1;

//*************************************************************************************
//*************************************************************************************

void EERIE_MESH_TWEAK_Do(INTERACTIVE_OBJ * io, long tw, char * _path)
{
	if (!ALLOW_MESH_TWEAKING) return;

	char file2[256];
	char filet[256];
	char path[256];
	File_Standardize(_path, path);

	strcpy(filet, Project.workingdir);
	strcat(filet, "GAME\\");
	strcat(filet, path + strlen(Project.workingdir));

	SetExt(filet, ".FTL");
	File_Standardize(filet, file2);

	if ((!PAK_FileExist(file2)) && (!PAK_FileExist(path))) return;

	if (tw == TWEAK_ERROR) return;

	if (io == NULL) return;

	if (io->obj == NULL) return;

	EERIE_MESH_ReleaseTransPolys(io->obj);

	if ((path == NULL) && (tw == TWEAK_REMOVE))
	{
		if (io->tweaky)
		{
			ReleaseEERIE3DObj(io->obj);
			io->obj = io->tweaky;
			EERIE_Object_Precompute_Fast_Access(io->obj);
			io->tweaky = NULL;
		}

		return;
	}

	EERIE_3DOBJ * tobj = NULL;
	EERIE_3DOBJ * result = NULL;
	EERIE_3DOBJ * result2 = NULL;

	if ((PAK_FileExist(file2)) || (PAK_FileExist(path)))
	{
		char tex1[256];
		sprintf(tex1, "%sGraph\\Obj3D\\Textures\\", Project.workingdir);

		if (io->ioflags & IO_NPC)
			tobj = TheoToEerie_Fast(tex1, path, TTE_NPC);
		else
			tobj = TheoToEerie_Fast(tex1, path, 0);

		if (!tobj) return;

		switch (tw)
		{
			case TWEAK_ALL:

				if (io->tweaky == NULL) io->tweaky = io->obj;
				else ReleaseEERIE3DObj(io->obj);

				long i;
				TextureContainer * tc;
				tc = NULL;

				for (i = 0; i < tobj->nbfaces; i++)
				{
					if ((tobj->facelist[i].texid >= 0)
					        &&	(tobj->texturecontainer[tobj->facelist[i].texid]))
					{
						tc = tobj->texturecontainer[tobj->facelist[i].texid];

						if (!tc->m_pddsSurface)
							tc->Restore(GDevice);
					}
				}

				io->obj = tobj;
				return;
				break;
			case TWEAK_UPPER:
				result2 = CreateIntermediaryMesh(io->obj, tobj, TWEAK_HEAD);
				result = CreateIntermediaryMesh(result2, tobj, TWEAK_TORSO);
				ReleaseEERIE3DObj(result2);
				break;
			case TWEAK_LOWER:
				result2 = CreateIntermediaryMesh(io->obj, tobj, TWEAK_TORSO);
				result = CreateIntermediaryMesh(result2, tobj, TWEAK_LEGS);
				ReleaseEERIE3DObj(result2);
				break;
			case TWEAK_UP_LO:
				result = CreateIntermediaryMesh(tobj, io->obj, TWEAK_TORSO);
				break;
			default:
				result = CreateIntermediaryMesh(io->obj, tobj, tw);
				break;
		}

		if (result == NULL)
		{
			ReleaseEERIE3DObj(tobj);
			return;
		}

		result->pdata = NULL;
		result->cdata = NULL;

		if (io->tweaky == NULL) io->tweaky = io->obj;
		else if (io->tweaky != io->obj)
			ReleaseEERIE3DObj(io->obj);

		io->obj = result;
		EERIE_Object_Precompute_Fast_Access(io->obj);
	}

	EERIE_CreateCedricData(io->obj);

	if (io)
	{
		io->lastanimtime = 0;
		io->nb_lastanimvertex = 0;
	}

	ReleaseEERIE3DObj(tobj);
}
