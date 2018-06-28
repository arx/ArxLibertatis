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
#ifndef ARX_SCENE_BACKGROUND_H
#define ARX_SCENE_BACKGROUND_H

#include <bitset>

#include "ai/Anchors.h"
#include "graphics/GraphicsTypes.h"

struct BackgroundTileData {
	
	std::vector<EERIEPOLY> polydata;
	std::vector<EERIEPOLY *> polyin;
	float maxy;
	
	BackgroundTileData()
		: maxy(0.f)
	{ }
	
};

static const short MAX_BKGX = 160;
static const short MAX_BKGZ = 160;
static const short BKG_SIZX = 100;
static const short BKG_SIZZ = 100;

static const Vec2f g_backgroundTileSize = Vec2f(BKG_SIZX, BKG_SIZZ);

struct BackgroundData {
	
private:
	
	std::bitset<MAX_BKGX * MAX_BKGZ> m_activeTiles;
	
public:
	
	long exist;
	Vec2s m_size;
	Vec2f m_mul;
	BackgroundTileData m_tileData[MAX_BKGX][MAX_BKGZ];
	std::vector<ANCHOR_DATA> m_anchors;
	
	bool isTileActive(Vec2s tile) {
		return m_activeTiles.test(size_t(tile.x) * size_t(MAX_BKGZ) + size_t(tile.y));
	}
	
	void setTileActive(Vec2s tile) {
		m_activeTiles.set(size_t(tile.x) * size_t(MAX_BKGZ) + size_t(tile.y));
	}
	
	void resetActiveTiles() {
		m_activeTiles.reset();
	}
	
	bool isInActiveTile(const Vec3f & pos) {
		Vec2s tile(s16(pos.x * m_mul.x), s16(pos.z * m_mul.y));
		return tile.x >= 0 && tile.x < m_size.x && tile.y >= 0 && tile.y < m_size.y && isTileActive(tile);
	}
	
	BackgroundData()
		: exist(false)
		, m_size(0, 0)
		, m_mul(0, 0)
	{ }
	
};

void InitBkg(BackgroundData * eb);
void ClearBackground(BackgroundData * eb);
void EERIEPOLY_Compute_PolyIn();
long CountBkgVertex();

BackgroundTileData * getFastBackgroundData(float x, float z);

#endif // ARX_SCENE_BACKGROUND_H
