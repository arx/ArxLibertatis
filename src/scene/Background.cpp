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

#include <algorithm>

#include <boost/foreach.hpp>

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
	
	for(short z = 0; z < eb->m_size.y; z++)
	for(short x = 0; x < eb->m_size.x; x++) {
		eb->m_tileData[x][z] = BackgroundTileData();
	}
}

static void ReleaseBKG_INFO(BackgroundTileData * eg) {
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
	
	FreeRoomDistance();
}

static void EERIEPOLY_Add_PolyIn(BackgroundTileData * eg, EERIEPOLY * ep) {
	if(std::find(eg->polyin.begin(), eg->polyin.end(), ep) == eg->polyin.end()) {
		eg->polyin.push_back(ep);
	}
}

static bool PointInBBox(const Vec3f & point, const Rectf & bb) {
	return (point.x <= bb.right && point.x >= bb.left && point.z <= bb.bottom && point.z >= bb.top);
}

void EERIEPOLY_Compute_PolyIn() {
	
	for(long z = 0; z < ACTIVEBKG->m_size.y; z++)
	for(long x = 0; x < ACTIVEBKG->m_size.x; x++) {
		BackgroundTileData * eg = &ACTIVEBKG->m_tileData[x][z];
		
		eg->polyin.clear();
		
		long minx = std::max(x - 2, 0l);
		long minz = std::max(z - 2, 0l);
		long maxx = std::min(x + 2, ACTIVEBKG->m_size.x - 1l);
		long maxz = std::min(z + 2, ACTIVEBKG->m_size.y - 1l);
		
		Vec2f bbmin = Vec2f(x * g_backgroundTileSize.x - 10, z * g_backgroundTileSize.y - 10);
		Vec2f bbmax = Vec2f(bbmin.x + g_backgroundTileSize.x + 20, bbmin.y + g_backgroundTileSize.y + 20);
		
		Rectf bb = Rectf(bbmin, bbmax);
		
		Vec2f bbcenter = bb.center();
		
		for(long z2 = minz; z2 < maxz; z2++)
		for(long x2 = minx; x2 < maxx; x2++) {
			BackgroundTileData & eg2 = ACTIVEBKG->m_tileData[x2][z2];
			BOOST_FOREACH(EERIEPOLY & ep2, eg2.polydata) {
				
				if(fartherThan(bbcenter, Vec2f(ep2.center.x, ep2.center.z), 120.f)) {
					continue;
				}
				
				long nbvert = (ep2.type & POLY_QUAD) ? 4 : 3;
				
				if(PointInBBox(ep2.center, bb)) {
					EERIEPOLY_Add_PolyIn(eg, &ep2);
				} else {
					for(long k = 0; k < nbvert; k++) {
						if(PointInBBox(ep2.v[k].p, bb)) {
							EERIEPOLY_Add_PolyIn(eg, &ep2);
							break;
						} else if(PointInBBox((ep2.v[k].p + ep2.center) * 0.5f, bb)) {
							EERIEPOLY_Add_PolyIn(eg, &ep2);
							break;
						}
					}
				}
				
			}
		}
		
		eg->maxy = -std::numeric_limits<float>::infinity();
		BOOST_FOREACH(EERIEPOLY * ep, eg->polyin) {
			eg->maxy = std::max(eg->maxy, ep->max.y);
		}
		
	}
}

long CountBkgVertex() {
	
	long count = 0;
	
	for(long z = 0; z < ACTIVEBKG->m_size.y; z++) {
		for(long x = 0; x < ACTIVEBKG->m_size.x; x++) {
			const BackgroundTileData & eg = ACTIVEBKG->m_tileData[x][z];
			BOOST_FOREACH(const EERIEPOLY & ep, eg.polydata) {
				count += (ep.type & POLY_QUAD) ? 4 : 3;
			}
		}
	}
	
	return count;
}


BackgroundTileData * getFastBackgroundData(float x, float z) {
	
	long px = long(x * ACTIVEBKG->m_mul.x);
	long pz = long(z * ACTIVEBKG->m_mul.y);
	
	if(px < 0 || px >= ACTIVEBKG->m_size.x || pz < 0 || pz >= ACTIVEBKG->m_size.y)
		return NULL;
	
	return &ACTIVEBKG->m_tileData[px][pz];
}
