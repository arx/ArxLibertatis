/*
 * Copyright 2018-2021 Arx Libertatis Team (see the AUTHORS file)
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

#include <algorithm>

#include "graphics/data/Mesh.h"


void InitBkg(BackgroundData * eb) {
	
	arx_assert(eb);
	
	if(eb->exist) {
		EERIE_PORTAL_Release();
		ClearBackground(eb);
	}
	
	eb->exist = 1;
	eb->m_anchors.clear();
	eb->m_size.x = MAX_BKGX;
	eb->m_size.y = MAX_BKGZ;
	
	eb->m_mul.x = 1.f / g_backgroundTileSize.x;
	eb->m_mul.y = 1.f / g_backgroundTileSize.y;
	
	for(auto tile : eb->tiles()) {
		tile.data() = BackgroundTileData();
	}
	
}

void ClearBackground(BackgroundData * eb) {
	
	if(!eb)
		return;
	
	AnchorData_ClearAll(eb);
	
	for(auto tile : eb->tiles()) {
		tile.data() = BackgroundTileData();
	}
	
	FreeRoomDistance();
	
}

static bool PointInBBox(const Vec3f & point, const Rectf & bb) {
	return (point.x <= bb.right && point.x >= bb.left && point.z <= bb.bottom && point.z >= bb.top);
}

void EERIEPOLY_Compute_PolyIn() {
	
	for(auto tile : ACTIVEBKG->tiles()) {
		
		std::vector<EERIEPOLY *> & polygons = tile.data().polyin;
		polygons.clear();
		
		Vec2f bbmin = Vec2f(tile.x * g_backgroundTileSize.x - 10, tile.y * g_backgroundTileSize.y - 10);
		Vec2f bbmax = Vec2f(bbmin.x + g_backgroundTileSize.x + 20, bbmin.y + g_backgroundTileSize.y + 20);
		Rectf bb = Rectf(bbmin, bbmax);
		Vec2f bbcenter = bb.center();
		
		for(auto neighbour : ACTIVEBKG->tilesAround(tile, 2)) {
			for(EERIEPOLY & ep2 : neighbour.polygons()) {
				
				if(fartherThan(bbcenter, Vec2f(ep2.center.x, ep2.center.z), 120.f)) {
					continue;
				}
				
				bool insert = PointInBBox(ep2.center, bb);
				long nbvert = (ep2.type & POLY_QUAD) ? 4 : 3;
				for(long k = 0; !insert && k < nbvert; k++) {
					insert = PointInBBox(ep2.v[k].p, bb) || PointInBBox((ep2.v[k].p + ep2.center) * 0.5f, bb);
				}
				
				if(insert && std::find(polygons.begin(), polygons.end(), &ep2) == polygons.end()) {
					polygons.push_back(&ep2);
				}
				
			}
		}
		
		tile.maxY() = -std::numeric_limits<float>::infinity();
		for(EERIEPOLY * ep : polygons) {
			tile.maxY() = std::max(tile.maxY(), ep->max.y);
		}
		
	}
	
}

long CountBkgVertex() {
	
	long count = 0;
	
	for(auto tile : ACTIVEBKG->tiles()) {
		for(const EERIEPOLY & ep : tile.polygons()) {
			count += (ep.type & POLY_QUAD) ? 4 : 3;
		}
	}
	
	return count;
}


BackgroundTileData * getFastBackgroundData(float x, float z) {
	Vec2s tile = ACTIVEBKG->getTile(Vec3f(x, 0.f, z));
	return ACTIVEBKG->isTileValid(tile) ? &ACTIVEBKG->m_tileData[tile.x][tile.y] : nullptr;
}
