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
// EERIEClothes.cpp
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		Adds Clothes Data to a Mesh
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "EERIEClothes.h"
#include "EERIEMeshTweak.h"
#include "EERIEDraw.h"
#include "EERIEMath.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#define MOLLESS_USEGRAVITY 1
#define MOLLESS_USEDAMPING 1
#define MOLLESS_DEFAULT_DAMPING		0.1f
#define SPHEREMUL 48.f;
#define MOLLESS_COLLISION_NONE			0
#define MOLLESS_COLLISION_PENETRATING	1
#define MOLLESS_COLLISION_COLLIDING		2

long MOLLESS_Nb_Interpolations = 1;
extern EERIE_3D SP_EC_POS;
 
 
long DEBUG_MOLLESS = 0;
long MOLLESS_ComputeCollisionsII(LPDIRECT3DDEVICE7 pd3dDevice, EERIE_3DOBJ * obj, float deltaTime);

 

extern long USE_CEDRIC_ANIM;


void MOLLESS_Clear(EERIE_3DOBJ * obj, long flag)
{
	if ((obj) && (obj->cdata))
	{
		for (long i = 0; i < obj->cdata->nb_cvert; i++)
		{
			CLOTHESVERTEX * cv = &obj->cdata->cvert[i];
			cv->velocity.x = 0.f;
			cv->velocity.y = 0.f;
			cv->velocity.z = 0.f;
			cv->force.x = 0.f;
			cv->force.y = 0.f;
			cv->force.z = 0.f;

			if (!(flag & 1))
			{
				cv->coll = -1;
				cv->pos.x = cv->t_pos.x = obj->vertexlist[cv->idx].v.x;
				cv->pos.y = cv->t_pos.y = obj->vertexlist[cv->idx].v.y;
				cv->pos.z = cv->t_pos.z = obj->vertexlist[cv->idx].v.z;
			}
			else
			{
				cv->t_pos.x = cv->pos.x;
				cv->t_pos.y = cv->pos.y;
				cv->t_pos.z = cv->pos.z;
			}
		}
	}
}

void AddSpring(EERIE_3DOBJ * obj, short vert1, short vert2, float constant, float damping, long type)
{
	if (vert1 == -1) return;

	if (vert2 == -1) return;

	if (vert1 == vert2) return;

	for (long i = 0; i < obj->cdata->nb_springs; i++)
	{
		if (((obj->cdata->springs[i].startidx == vert1) && (obj->cdata->springs[i].endidx == vert2))
		        || ((obj->cdata->springs[i].startidx == vert2) && (obj->cdata->springs[i].endidx == vert1)))
			return;
	}

	if (obj->cdata->springs == NULL)
	{
		obj->cdata->springs = (EERIE_SPRINGS *)malloc(sizeof(EERIE_SPRINGS)); 
		obj->cdata->nb_springs = 1;
	}
	else
	{
		obj->cdata->nb_springs++;
		obj->cdata->springs = (EERIE_SPRINGS *)realloc(obj->cdata->springs, sizeof(EERIE_SPRINGS) * obj->cdata->nb_springs);
	}

	obj->cdata->springs[obj->cdata->nb_springs-1].startidx = vert1;
	obj->cdata->springs[obj->cdata->nb_springs-1].endidx = vert2;
	obj->cdata->springs[obj->cdata->nb_springs-1].restlength =
	    TRUEDistance3D(obj->cdata->cvert[vert1].pos.x, obj->cdata->cvert[vert1].pos.y, obj->cdata->cvert[vert1].pos.z,
	                   obj->cdata->cvert[vert2].pos.x, obj->cdata->cvert[vert2].pos.y, obj->cdata->cvert[vert2].pos.z);
	obj->cdata->springs[obj->cdata->nb_springs-1].restlength;
	obj->cdata->springs[obj->cdata->nb_springs-1].constant = constant;
	obj->cdata->springs[obj->cdata->nb_springs-1].damping = damping;
	obj->cdata->springs[obj->cdata->nb_springs-1].type = type;
}

short GetIDXVert(EERIE_3DOBJ * obj, short num)
{
	for (short i = 0; i < obj->cdata->nb_cvert; i++)
	{
		if (obj->cdata->cvert[i].idx == num) return i;
	}

	return -1;
}

//*************************************************************************************
// Creates Clothes Data Structure for an object.
//*************************************************************************************
void EERIEOBJECT_AddClothesData(EERIE_3DOBJ * obj)
{
	long sel = -1;
	long selmounocol = -1;

	for (long i = 0; i < obj->nbselections; i++)
	{
		if (!stricmp(obj->selections[i].name, "mou"))
		{
			sel = i;
			break;
		}
	}

	for (int i = 0; i < obj->nbselections; i++)
	{
		if (!stricmp(obj->selections[i].name, "mounocol"))
		{
			selmounocol = i;
			break;
		}
	}

	if (sel == -1) return;

	if (obj->selections[sel].nb_selected > 0)
	{
		obj->cdata = (CLOTHES_DATA *)malloc(sizeof(CLOTHES_DATA)); 
		memset(obj->cdata, 0, sizeof(CLOTHES_DATA));

		obj->cdata->nb_cvert = (short)obj->selections[sel].nb_selected;
		obj->cdata->cvert = (CLOTHESVERTEX *)malloc(sizeof(CLOTHESVERTEX) * obj->cdata->nb_cvert); 
		memset(obj->cdata->cvert, 0, sizeof(CLOTHESVERTEX)*obj->cdata->nb_cvert);

		obj->cdata->backup = (CLOTHESVERTEX *)malloc(sizeof(CLOTHESVERTEX) * obj->cdata->nb_cvert); 
		memset(obj->cdata->backup, 0, sizeof(CLOTHESVERTEX)*obj->cdata->nb_cvert);
	}


	// There is a Mollesse (TM) (C) Selection
	if (obj->selections[sel].nb_selected > 0)
	{
		for (int i = 0; i < obj->cdata->nb_cvert; i++)
		{
			obj->cdata->cvert[i].idx = (short)obj->selections[sel].selected[i];
			obj->cdata->cvert[i].pos.x = obj->cdata->cvert[i].t_pos.x = obj->vertexlist[obj->cdata->cvert[i].idx].v.x;
			obj->cdata->cvert[i].pos.y = obj->cdata->cvert[i].t_pos.y = obj->vertexlist[obj->cdata->cvert[i].idx].v.y;
			obj->cdata->cvert[i].pos.z = obj->cdata->cvert[i].t_pos.z = obj->vertexlist[obj->cdata->cvert[i].idx].v.z;
			obj->cdata->cvert[i].mass = 0.5f; 

			if ((selmounocol != -1) && (IsInSelection(obj, obj->selections[sel].selected[i], selmounocol) >= 0))
			{

				obj->cdata->cvert[i].flags = CLOTHES_FLAG_NORMAL | CLOTHES_FLAG_NOCOL;
			}
			else
				obj->cdata->cvert[i].flags = CLOTHES_FLAG_NORMAL;

			obj->cdata->cvert[i].coll = -1;
		}

		for (int i = 0; i < obj->cdata->nb_cvert; i++)
		{

			for (long j = 0; j < obj->ndata[obj->cdata->cvert[i].idx].nb_Nvertex; j++)
			{
				short vert = obj->ndata[obj->cdata->cvert[i].idx].Nvertex[j];

				if (IsInSelection(obj, vert, sel) >= 0)
				{
					AddSpring(obj, (short)i, (short)GetIDXVert(obj, vert), 11.f, 0.3f, 0); 
				}
				else
				{
					obj->cdata->cvert[i].flags |= CLOTHES_FLAG_FIX;
					obj->cdata->cvert[i].coll = -2;
					obj->cdata->cvert[i].mass = 0.f;
				}
			}
		}

		// Adds more springs (shear)
		for (int i = 0; i < obj->cdata->nb_cvert; i++)
		{
			for (long j = 0; j < obj->ndata[obj->cdata->cvert[i].idx].nb_Nvertex; j++)
			{
				short vert = obj->ndata[obj->cdata->cvert[i].idx].Nvertex[j];

				if (vert == obj->cdata->cvert[i].idx) continue; // Cannot add a spring between 1 node :p

				if (IsInSelection(obj, vert, sel) >= 0)
				{
					float distance = EEDistance3D(&obj->vertexlist[obj->cdata->cvert[i].idx].v,
					                              &obj->vertexlist[vert].v) * 1.2f;

					// We springed it in the previous part of code
					for (long k = 0; k < obj->ndata[vert].nb_Nvertex; k++)
					{
						short ver = obj->ndata[vert].Nvertex[k];

						if (IsInSelection(obj, ver, sel) >= 0) // This time we have one !
						{
							if (ver == obj->cdata->cvert[i].idx) continue;
							float distance2 = EEDistance3D(&obj->vertexlist[obj->cdata->cvert[i].idx].v,
							                               &obj->vertexlist[ver].v);

							if (distance2 < distance)
							{
								AddSpring(obj, (short)i, (short)GetIDXVert(obj, ver), 4.2f, 0.7f, 1); 
							}
						}
					}
				}
			}
		}

		// Adds more springs (bend)
		for (int i = 0; i < obj->cdata->nb_cvert; i++)
		{
			for (long j = 0; j < obj->ndata[obj->cdata->cvert[i].idx].nb_Nvertex; j++)
			{
				short vert = obj->ndata[obj->cdata->cvert[i].idx].Nvertex[j];

				if (vert == obj->cdata->cvert[i].idx) continue; // Cannot add a spring between 1 node :p

				if (IsInSelection(obj, vert, sel) >= 0)
				{
					// We springed it in the previous part of code
					for (long k = 0; k < obj->ndata[vert].nb_Nvertex; k++)
					{
						short ver = obj->ndata[vert].Nvertex[k];

						if (IsInSelection(obj, ver, sel) >= 0) // This time we have one !
						{
							float distance = EEDistance3D(&obj->vertexlist[obj->cdata->cvert[i].idx].v,
							                              &obj->vertexlist[ver].v) * 1.2f;

							for (long k2 = 0; k2 < obj->ndata[ver].nb_Nvertex; k2++)
							{
								short ve = obj->ndata[ver].Nvertex[k];

								if (ve == vert) continue;

								if (IsInSelection(obj, ve, sel) >= 0) // This time we have one !
								{
									if (obj->cdata->cvert[(short)GetIDXVert(obj, ve)].flags & CLOTHES_FLAG_FIX) continue;

									float distance2 = EEDistance3D(&obj->vertexlist[obj->cdata->cvert[i].idx].v,
									                               &obj->vertexlist[ve].v);

									if ((distance2 > distance) && (distance2 < distance * 2.f)) 
									{
										AddSpring(obj, (short)i, (short)GetIDXVert(obj, ve), 2.2f, 0.9f, 2);
									}

								}
							}
						}
					}
				}
			}
		}
		
	}
}

void KillClothesData(EERIE_3DOBJ * obj)
{
	if (obj == NULL) return;

	if (obj->cdata == NULL) return;

	if (obj->cdata->cvert) free(obj->cdata->cvert);

	obj->cdata->cvert = NULL;

	if (obj->cdata->backup) free(obj->cdata->backup);

	obj->cdata->backup = NULL;

	if (obj->cdata->springs) free(obj->cdata->springs);

	obj->cdata->springs = NULL;

	free(obj->cdata);
	obj->cdata = NULL;
}


 