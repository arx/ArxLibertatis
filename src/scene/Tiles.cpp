/*
 * Copyright 2018-2022 Arx Libertatis Team (see the AUTHORS file)
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

#include "scene/Tiles.h"

#include <algorithm>

#include "graphics/data/Mesh.h"


TileData * g_tiles = nullptr;

bool TileData::isNearActiveTile(Vec3f pos, float extend) noexcept {
	
	for(auto neighbor : g_tiles->tilesAround(pos, extend)) {
		if(neighbor.active()) {
			return true;
		}
	}
	
	return false;
}

void TileData::clear() {
	
	for(auto tile : tiles()) {
		tile.data() = BackgroundTileData();
	}
	
}

static bool PointInBBox(const Vec3f & point, const Rectf & bb) {
	return (point.x <= bb.right && point.x >= bb.left && point.z <= bb.bottom && point.z >= bb.top);
}

void TileData::computeIntersectingPolygons() {
	
	for(auto tile : tiles()) {
		
		std::vector<EERIEPOLY *> & polygons = tile.data().polyin;
		polygons.clear();
		
		Rectf bb = tile.bounds();
		bb.left -= 10.f;
		bb.top -= 10.f;
		bb.right += 10.f;
		bb.bottom += 10.f;
		Vec2f bbcenter = tile.center();
		
		for(auto neighbour : tilesAround(tile, 2)) {
			for(EERIEPOLY & ep2 : neighbour.polygons()) {
				
				if(fartherThan(bbcenter, getXZ(ep2.center), 120.f)) {
					continue;
				}
				
				bool insert = PointInBBox(ep2.center, bb);
				size_t nbvert = (ep2.type & POLY_QUAD) ? 4 : 3;
				for(size_t k = 0; !insert && k < nbvert; k++) {
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

size_t TileData::countVertices() {
	
	size_t count = 0;
	
	for(auto tile : tiles()) {
		for(const EERIEPOLY & ep : tile.polygons()) {
			count += (ep.type & POLY_QUAD) ? 4 : 3;
		}
	}
	
	return count;
}
