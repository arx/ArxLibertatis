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

#include "graphics/data/Progressive.h"

#include <stddef.h>
#include <cstring>
#include <cstdlib>
#include <vector>

#include "graphics/GraphicsTypes.h"

using std::memset;

/*!
 * \brief Adds a neighboring vertex to a vertex
 */
void AddNeighBoringVertex(EERIE_3DOBJ * obj, long i, long b) {
	
	if(i == b)
		return;

	if(obj->ndata[i].Nvertex == NULL) {
		obj->ndata[i].Nvertex = (short *)malloc(sizeof(short)); 
		obj->ndata[i].Nvertex[0] = (short)b;
		obj->ndata[i].nb_Nvertex = 1;
		return;
	}

	//check for existence
	for(long k = 0; k < obj->ndata[i].nb_Nvertex; k++)
		if(obj->ndata[i].Nvertex[k] == b)
			return;

	obj->ndata[i].Nvertex = (short *)realloc(obj->ndata[i].Nvertex, sizeof(short) * (obj->ndata[i].nb_Nvertex + 1));
	obj->ndata[i].Nvertex[obj->ndata[i].nb_Nvertex] = (short)b;
	obj->ndata[i].nb_Nvertex++;
}

/*!
 * \brief Adds a neighboring face to a vertex
 */
void AddNeighBoringFace(EERIE_3DOBJ * obj, long i, long b) {

	if(obj->ndata[i].Nfaces == NULL) {
		obj->ndata[i].Nfaces = (short *)malloc(sizeof(short));
		obj->ndata[i].Nfaces[0] = (short)b;
		obj->ndata[i].nb_Nfaces = 1;
		return;
	}

	//check for existence
	for(long k = 0; k < obj->ndata[i].nb_Nfaces; k++)
		if(obj->ndata[i].Nfaces[k] == b)
			return;

	obj->ndata[i].Nfaces = (short *)realloc(obj->ndata[i].Nfaces, sizeof(short) * (obj->ndata[i].nb_Nfaces + 1));
	obj->ndata[i].Nfaces[obj->ndata[i].nb_Nfaces] = (short)b;
	obj->ndata[i].nb_Nfaces++;
}

/*!
 * \brief Computes collapse cost for an edge
 *
 * search all neighboring edges for "least-folding-cost" edge
 *
 * Computes collapse cost for an edge
 * Conditions for collapsability:
 *		.Distance (The shorter the better)
 *		.Surface of the Collapsed sides (The lesser the better)
 *		.Angle Between Faces on each side of Collapsed Edge (The lesser angular diff, The better)
 *		.UV/Texture (Requires same texture map, and contiguous UVs (Quite Arbitrary)
 */
void CreateNeighbours(EERIE_3DOBJ * obj) {

	if(obj->ndata == NULL) {
		obj->ndata = (NEIGHBOURS_DATA *)malloc(sizeof(NEIGHBOURS_DATA) * obj->vertexlist.size());
	} else {
		memset(obj->ndata, 0, sizeof(NEIGHBOURS_DATA)*obj->vertexlist.size());
	}

	for(size_t i = 0; i < obj->vertexlist.size(); i++) {
		obj->ndata[i].Nvertex = NULL;
		obj->ndata[i].Nfaces = NULL;
		obj->ndata[i].nb_Nfaces = 0;
		obj->ndata[i].nb_Nvertex = 0;

		for(size_t j = 0; j < obj->facelist.size(); j++) {
			if(obj->facelist[j].vid[0] == i
			   || obj->facelist[j].vid[1] == i
			   || obj->facelist[j].vid[2] == i
			) {
				AddNeighBoringVertex(obj, i, obj->facelist[j].vid[0]);
				AddNeighBoringVertex(obj, i, obj->facelist[j].vid[1]);
				AddNeighBoringVertex(obj, i, obj->facelist[j].vid[2]);
				AddNeighBoringFace(obj, i, j);
			}
		}
	}
}

void KillNeighbours(EERIE_3DOBJ * obj) {
	
	if(!obj->ndata) {
		return;
	}
	
	for(size_t i = 0; i < obj->vertexlist.size(); i++) {
		free(obj->ndata[i].Nfaces);
		obj->ndata[i].Nfaces = NULL;

		free(obj->ndata[i].Nvertex);
		obj->ndata[i].Nvertex = NULL;
	}
	
	free(obj->ndata);
	obj->ndata = NULL;
}

void KillProgressiveData(EERIE_3DOBJ * obj) {
	free(obj->pdata);
	obj->pdata = NULL;
}
