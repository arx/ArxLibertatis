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

#include "scene/LinkedObject.h"

#include <cstdlib>
#include <cstring>

#include "scene/Object.h"

/*!
 * \brief Releases Data for linked objects
 * \param obj
 */
void EERIE_LINKEDOBJ_ReleaseData(EERIE_3DOBJ * obj) {
	
	if(!obj)
		return;
	
	obj->linked.clear();
}

/*!
 * \brief Init Data for linked objects
 * \param obj
 */
void EERIE_LINKEDOBJ_InitData(EERIE_3DOBJ * obj)
{
	if(!obj)
		return;

	obj->linked.clear();
}

/*!
 * \brief Add New Data field for a linked object to an object
 * \param obj
 * \return
 */
static long EERIE_LINKEDOBJ_Create(EERIE_3DOBJ * obj)
{
	if(!obj)
		return -1;
	
	EERIE_LINKED link;
	link.lgroup = -1;
	link.lidx = -1;
	link.obj = NULL;
	link.io = NULL;
	obj->linked.push_back(link);

	return (obj->linked.size() - 1);
}

/*!
 * \brief Removes a linked object Data field from an object
 * \param obj
 * \param num
 */
static void EERIE_LINKEDOBJ_Remove(EERIE_3DOBJ * obj, long num)
{
	if(!obj || num < 0 || num >= obj->linked.size())
		return;

	obj->linked.erase(obj->linked.begin() + num);
}


void EERIE_LINKEDOBJ_UnLinkObjectFromObject(EERIE_3DOBJ * obj, EERIE_3DOBJ * tounlink) {
	
	if(!obj || !tounlink)
		return;
	
	for(size_t k = 0; k < obj->linked.size(); k++) {
		if(obj->linked[k].lgroup != -1 && obj->linked[k].obj == tounlink) {
			for(size_t i = 0; i < tounlink->vertexlist.size(); i++) {
				tounlink->vertexlist[i].vert.p = tounlink->vertexlist[i].v;
			}
			EERIE_LINKEDOBJ_Remove(obj, k);
			return;
		}
	}
}

bool EERIE_LINKEDOBJ_LinkObjectToObject(EERIE_3DOBJ * obj, EERIE_3DOBJ * tolink, const std::string& actiontext, const std::string& actiontext2, Entity * io)
{
	long ni = GetActionPointIdx(obj, actiontext);
	if(ni < 0)
		return false; 

	long n = EERIE_LINKEDOBJ_Create(obj);
	if(n == -1)
		return false;

	long group = GetActionPointGroup(obj, ni);
	if(group < 0)
		return false;

	long ni2 = GetActionPointIdx(tolink, actiontext2);
	if(ni2 < 0)
		return false;

	obj->linked[n].lidx2 = ni2;
	obj->linked[n].lidx = ni;
	obj->linked[n].lgroup = group;
	obj->linked[n].obj = tolink;
	obj->linked[n].io = io;

	return true;
}
