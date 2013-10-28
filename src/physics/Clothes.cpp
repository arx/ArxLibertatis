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

#include "physics/Clothes.h"

#include <cstring>

#include <glm/gtx/norm.hpp>

#include "graphics/data/MeshManipulation.h"
#include "graphics/Math.h"

using std::vector;

#define MOLLESS_USEGRAVITY 1
#define MOLLESS_USEDAMPING 1
#define MOLLESS_DEFAULT_DAMPING		0.1f
#define SPHEREMUL 48.f;
#define MOLLESS_COLLISION_NONE			0
#define MOLLESS_COLLISION_PENETRATING	1
#define MOLLESS_COLLISION_COLLIDING		2

void MOLLESS_Clear(EERIE_3DOBJ * obj, long flag) {
	
	if(obj && obj->cdata) {
		for(long i = 0; i < obj->cdata->nb_cvert; i++) {
			CLOTHESVERTEX * cv = &obj->cdata->cvert[i];
			cv->velocity = Vec3f_ZERO;
			cv->force = Vec3f_ZERO;
			if(!(flag & 1)) {
				cv->coll = -1;
				cv->t_pos = cv->pos = obj->vertexlist[cv->idx].v;
			} else {
				cv->t_pos = cv->pos;
			}
		}
	}
}

void AddSpring(EERIE_3DOBJ * obj, short vert1, short vert2, float constant, float damping, long type) {
	
	if(vert1 == -1 || vert2 == -1 || vert1 == vert2) {
		return;
	}
	
	for(vector<EERIE_SPRINGS>::const_iterator i = obj->cdata->springs.begin(); i < obj->cdata->springs.end(); ++i) {
		if((i->startidx == vert1 && i->endidx == vert2) || (i->startidx == vert2 && i->endidx == vert1)) {
			return;
		}
	}
	
	EERIE_SPRINGS newSpring;
	newSpring.startidx = vert1;
	newSpring.endidx = vert2;
	newSpring.restlength = glm::distance(obj->cdata->cvert[vert1].pos, obj->cdata->cvert[vert2].pos);
	newSpring.constant = constant;
	newSpring.damping = damping;
	newSpring.type = type;
	
	obj->cdata->springs.push_back(newSpring);
}

short GetIDXVert(EERIE_3DOBJ * obj, short num) {

	for(short i = 0; i < obj->cdata->nb_cvert; i++) {
		if(obj->cdata->cvert[i].idx == num)
			return i;
	}

	return -1;
}

//*************************************************************************************
// Creates Clothes Data Structure for an object.
//*************************************************************************************
void EERIEOBJECT_AddClothesData(EERIE_3DOBJ * obj) {

	long sel = -1;
	long selmounocol = -1;

	for(size_t i = 0; i < obj->selections.size(); i++) { // TODO iterator
		if(obj->selections[i].name == "mou") {
			sel = i;
			break;
		}
	}
	
	for(size_t i = 0; i < obj->selections.size(); i++) { // TODO iterator
		if(obj->selections[i].name == "mounocol") {
			selmounocol = i;
			break;
		}
	}

	if(sel == -1)
		return;

	if(obj->selections[sel].selected.size() > 0) {
		obj->cdata = new CLOTHES_DATA();

		obj->cdata->nb_cvert = (short)obj->selections[sel].selected.size();
		obj->cdata->cvert = new CLOTHESVERTEX[obj->cdata->nb_cvert]; 
		memset(obj->cdata->cvert, 0, sizeof(CLOTHESVERTEX)*obj->cdata->nb_cvert);

		obj->cdata->backup = new CLOTHESVERTEX[obj->cdata->nb_cvert]; 
		memset(obj->cdata->backup, 0, sizeof(CLOTHESVERTEX)*obj->cdata->nb_cvert);
	}


	// There is a Mollesse (TM) (C) Selection
	if(obj->selections[sel].selected.size() > 0) {
		for(int i = 0; i < obj->cdata->nb_cvert; i++) {
			obj->cdata->cvert[i].idx = (short)obj->selections[sel].selected[i];
			obj->cdata->cvert[i].pos = obj->vertexlist[obj->cdata->cvert[i].idx].v;
			obj->cdata->cvert[i].t_pos = obj->vertexlist[obj->cdata->cvert[i].idx].v;
			obj->cdata->cvert[i].mass = 0.5f; 

			if(selmounocol != -1 && IsInSelection(obj, obj->selections[sel].selected[i], selmounocol) >= 0) {
				obj->cdata->cvert[i].flags = CLOTHES_FLAG_NORMAL | CLOTHES_FLAG_NOCOL;
			} else {
				obj->cdata->cvert[i].flags = CLOTHES_FLAG_NORMAL;
			}

			obj->cdata->cvert[i].coll = -1;
		}

		for(int i = 0; i < obj->cdata->nb_cvert; i++) {
			for(long j = 0; j < obj->ndata[obj->cdata->cvert[i].idx].nb_Nvertex; j++) {
				short vert = obj->ndata[obj->cdata->cvert[i].idx].Nvertex[j];

				if(IsInSelection(obj, vert, sel) >= 0) {
					AddSpring(obj, (short)i, (short)GetIDXVert(obj, vert), 11.f, 0.3f, 0); 
				} else {
					obj->cdata->cvert[i].flags |= CLOTHES_FLAG_FIX;
					obj->cdata->cvert[i].coll = -2;
					obj->cdata->cvert[i].mass = 0.f;
				}
			}
		}

		// Adds more springs (shear)
		for(int i = 0; i < obj->cdata->nb_cvert; i++) {
			for(long j = 0; j < obj->ndata[obj->cdata->cvert[i].idx].nb_Nvertex; j++) {
				short vert = obj->ndata[obj->cdata->cvert[i].idx].Nvertex[j];

				if(vert == obj->cdata->cvert[i].idx)
					continue; // Cannot add a spring between 1 node :p

				if(IsInSelection(obj, vert, sel) >= 0) {
					float distance = glm::distance2(obj->vertexlist[obj->cdata->cvert[i].idx].v,
					                         obj->vertexlist[vert].v) * square(1.2f);

					// We springed it in the previous part of code
					for(long k = 0; k < obj->ndata[vert].nb_Nvertex; k++) {
						short ver = obj->ndata[vert].Nvertex[k];

						if(IsInSelection(obj, ver, sel) >= 0) { // This time we have one !
							if(ver == obj->cdata->cvert[i].idx)
								continue;

							float distance2 = glm::distance2(obj->vertexlist[obj->cdata->cvert[i].idx].v,
							                          obj->vertexlist[ver].v);

							if(distance2 < distance) {
								AddSpring(obj, (short)i, (short)GetIDXVert(obj, ver), 4.2f, 0.7f, 1); 
							}
						}
					}
				}
			}
		}

		// Adds more springs (bend)
		for(int i = 0; i < obj->cdata->nb_cvert; i++) {
			for(long j = 0; j < obj->ndata[obj->cdata->cvert[i].idx].nb_Nvertex; j++) {
				short vert = obj->ndata[obj->cdata->cvert[i].idx].Nvertex[j];

				if(vert == obj->cdata->cvert[i].idx)
					continue; // Cannot add a spring between 1 node :p

				if(IsInSelection(obj, vert, sel) >= 0) {
					// We springed it in the previous part of code
					for(long k = 0; k < obj->ndata[vert].nb_Nvertex; k++) {
						short ver = obj->ndata[vert].Nvertex[k];

						if(IsInSelection(obj, ver, sel) >= 0) { // This time we have one !
							float distance = glm::distance2(obj->vertexlist[obj->cdata->cvert[i].idx].v,
							                         obj->vertexlist[ver].v) * square(1.2f);

							for(long k2 = 0; k2 < obj->ndata[ver].nb_Nvertex; k2++) {
								short ve = obj->ndata[ver].Nvertex[k];

								if(ve == vert)
									continue;

								if(IsInSelection(obj, ve, sel) >= 0) { // This time we have one !

									if(obj->cdata->cvert[(short)GetIDXVert(obj, ve)].flags & CLOTHES_FLAG_FIX)
										continue;

									float distance2 = glm::distance2(obj->vertexlist[obj->cdata->cvert[i].idx].v,
									                          obj->vertexlist[ve].v);

									if(distance2 > distance && distance2 < distance * square(2.f)) {
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

void KillClothesData(EERIE_3DOBJ * obj) {
	
	if(!obj || !obj->cdata) {
		return;
	}
	
	delete[] obj->cdata->cvert, obj->cdata->cvert = NULL;
	delete[] obj->cdata->backup, obj->cdata->backup = NULL;
	delete obj->cdata, obj->cdata = NULL;
}
