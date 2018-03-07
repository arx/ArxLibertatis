/*
 * Copyright 2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "scene/Background.h"

#include "graphics/data/Mesh.h"


void InitBkg(BackgroundData * eb, short sx, short sz) {
	
	arx_assert(eb);
	
	if(eb->exist) {
		EERIE_PORTAL_Release();
		ClearBackground(eb);
	}
	
	eb->exist = 1;
	eb->m_anchors.clear();
	eb->m_size.x = sx;
	eb->m_size.y = sz;
	
	eb->m_tileSize = Vec2s(BKG_SIZX, BKG_SIZZ);
	
	eb->m_mul.x = 1.f / (float)eb->m_tileSize.x;
	eb->m_mul.y = 1.f / (float)eb->m_tileSize.y;
	
	for(short z = 0; z < eb->m_size.y; z++)
	for(short x = 0; x < eb->m_size.x; x++) {
		eb->m_tileData[x][z] = BackgroundTileData();
	}
}

static void ReleaseBKG_INFO(BackgroundTileData * eg) {
	free(eg->polydata);
	eg->polydata = NULL;
	free(eg->polyin);
	eg->polyin = NULL;
	eg->nbpolyin = 0;
	*eg = BackgroundTileData();
}

void ClearBackground(BackgroundData * eb) {
	
	if(!eb)
		return;
	
	AnchorData_ClearAll(eb);
	
	for(long z = 0; z < eb->m_size.y; z++)
	for(long x = 0; x < eb->m_size.x; x++) {
		ReleaseBKG_INFO(&eb->m_tileData[x][z]);
	}
	
	free(RoomDistance);
	RoomDistance = NULL;
	NbRoomDistance = 0;
}
