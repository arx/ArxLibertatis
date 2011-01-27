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
// EERIELinkedObj
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		Manages Objects linked to other objects
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
/////////////////////////////////////////////////////////////////////////////////////

#include "EERIELinkedObj.h"
#include "EERIEObject.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

//*************************************************************************************
//*************************************************************************************
// Releases Data for linked objects
void EERIE_LINKEDOBJ_ReleaseData(EERIE_3DOBJ * obj)
{
	if (obj == NULL) return;

	if (obj->linked) free(obj->linked);

	obj->linked = NULL;
	obj->nblinked = 0;
}

//*************************************************************************************
//*************************************************************************************
// Init Data for linked objects
void EERIE_LINKEDOBJ_InitData(EERIE_3DOBJ * obj)
{
	if (obj == NULL) return;

	obj->nblinked = 0;
	obj->linked = NULL;
}

//*************************************************************************************
//*************************************************************************************
// Add New Data field for a linked object to an object
long EERIE_LINKEDOBJ_Create(EERIE_3DOBJ * obj)
{
	if (obj == NULL) return -1;

	obj->linked = (EERIE_LINKED *)realloc(obj->linked, sizeof(EERIE_LINKED) * (obj->nblinked + 1));
	obj->linked[obj->nblinked].lgroup = -1;
	obj->linked[obj->nblinked].lidx = -1;
	obj->linked[obj->nblinked].obj = NULL;
	obj->linked[obj->nblinked].io = NULL;
	obj->nblinked++;

	if (obj->nblinked > 1)
		obj->nblinked = obj->nblinked;

	return (obj->nblinked - 1);
}

//*************************************************************************************
//*************************************************************************************
// Removes a linked object Data field from an object
void EERIE_LINKEDOBJ_Remove(EERIE_3DOBJ * obj, long num)
{
	if (obj == NULL) return;

	if (obj->linked == NULL) return;

	if (num < 0) return;

	if (num >= obj->nblinked) return;

	if (obj->nblinked == 1)
	{
		free(obj->linked);
		obj->linked = NULL;
		obj->nblinked = 0;
		return;
	}

	memcpy(&obj->linked[num], &obj->linked[num+1], sizeof(EERIE_LINKED)*(obj->nblinked - num - 1));
	obj->linked = (EERIE_LINKED *)realloc(obj->linked, sizeof(EERIE_LINKED) * (obj->nblinked - 1));
	obj->nblinked--;
}


//*************************************************************************************
//*************************************************************************************
void EERIE_LINKEDOBJ_UnLinkObjectFromObject(EERIE_3DOBJ * obj, EERIE_3DOBJ * tounlink)
{
	if (obj && tounlink)
		for (long k = 0; k < obj->nblinked; k++)
		{
			if (obj->linked[k].lgroup != -1)
			{
				if (obj->linked[k].obj == tounlink)
				{
					for (long i = 0; i < tounlink->nbvertex; i++)
					{
						tounlink->vertexlist[i].vert.sx = tounlink->vertexlist[i].v.x;
						tounlink->vertexlist[i].vert.sy = tounlink->vertexlist[i].v.y;
						tounlink->vertexlist[i].vert.sz = tounlink->vertexlist[i].v.z;
					}

					EERIE_LINKEDOBJ_Remove(obj, k);
					return;
				}
			}
		}
}
//*************************************************************************************
//*************************************************************************************
BOOL EERIE_LINKEDOBJ_LinkObjectToObject(EERIE_3DOBJ * obj, EERIE_3DOBJ * tolink, char * actiontext, char * actiontext2, INTERACTIVE_OBJ * io)
{
	long group = -1;

	long ni = GetActionPointIdx(obj, actiontext);

	if (ni < 0)
	{
		return FALSE; 
	}

	long n = EERIE_LINKEDOBJ_Create(obj);

	if (n == -1) return FALSE;

	group = GetActionPointGroup(obj, ni);

	if (group < 0) return FALSE; 

	long ni2 = GetActionPointIdx(tolink, actiontext2);

	if (ni2 < 0) return FALSE; 

	obj->linked[n].lidx2 = ni2;
	obj->linked[n].lidx = ni;
	obj->linked[n].lgroup = group;
	obj->linked[n].obj = tolink;
	obj->linked[n].io = io;
	return TRUE;

}