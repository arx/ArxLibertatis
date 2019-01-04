/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include "ai/Anchors.h"

#include <cstdio>

#include <boost/foreach.hpp>

#include "ai/PathFinderManager.h"

#include "game/Entity.h"

#include "graphics/data/Mesh.h"

void AnchorData_ClearAll(BackgroundData * eb) {
	
	EERIE_PATHFINDER_Clear();
	
	eb->m_anchors.clear();
}

void ANCHOR_BLOCK_Clear() {
	
	BackgroundData * eb = ACTIVEBKG;
	if(!eb) {
		return;
	}
	
	BOOST_FOREACH(ANCHOR_DATA & ad, eb->m_anchors) {
		ad.blocked = false;
	}
	
}

void ANCHOR_BLOCK_By_IO(Entity * io, bool blocked) {
	
	BackgroundData * eb = ACTIVEBKG;
	
	BOOST_FOREACH(ANCHOR_DATA & ad, eb->m_anchors) {
		
		if(fartherThan(ad.pos, io->pos, 600.f))
			continue;

		if(closerThan(Vec2f(io->pos.x, io->pos.z), Vec2f(ad.pos.x, ad.pos.z), 440.f)) {
			
			EERIEPOLY ep;
			ep.type = 0;

			for(size_t ii = 0; ii < io->obj->facelist.size(); ii++) {
				float cx = 0;
				float cz = 0;

				for(long kk = 0; kk < 3; kk++) {
					ep.v[kk].p = io->obj->vertexlist[io->obj->facelist[ii].vid[kk]].v + io->pos;

					cx += ep.v[kk].p.x;
					cz += ep.v[kk].p.z;
				}

				cx *= 1.f / 3;
				cz *= 1.f / 3;

				for(int kk = 0; kk < 3; kk++) {
					ep.v[kk].p.x = (ep.v[kk].p.x - cx) * 3.5f + cx;
					ep.v[kk].p.z = (ep.v[kk].p.z - cz) * 3.5f + cz;
				}

				if(PointIn2DPolyXZ(&ep, ad.pos.x, ad.pos.z)) {
					ad.blocked = blocked;
				}
				
			}
			
		}
		
	}
	
}
