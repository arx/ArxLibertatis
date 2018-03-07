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

#include "ai/Anchors.h"
#include "graphics/GraphicsTypes.h"

struct BackgroundTileData {
	
	bool treat;
	short nbpoly;
	short nbpolyin;
	EERIEPOLY * polydata;
	EERIEPOLY ** polyin;
	float maxy;
	
	BackgroundTileData()
		: treat(false)
		, nbpoly(0)
		, nbpolyin(0)
		, polydata(NULL)
		, polyin(NULL)
		, maxy(0.f)
	{ }
	
};

static const short MAX_BKGX = 160;
static const short MAX_BKGZ = 160;
static const short BKG_SIZX = 100;
static const short BKG_SIZZ = 100;

struct BackgroundData {
	
	long exist;
	Vec2s m_size;
	Vec2s m_tileSize;
	Vec2f m_mul;
	BackgroundTileData m_tileData[MAX_BKGX][MAX_BKGZ];
	std::vector<ANCHOR_DATA> m_anchors;
	
	BackgroundData()
		: exist(false)
		, m_size(0, 0)
		, m_tileSize(0, 0)
		, m_mul(0, 0)
	{ }
	
};

void InitBkg(BackgroundData * eb, short sx, short sz);
void ClearBackground(BackgroundData * eb);

#endif // ARX_SCENE_BACKGROUND_H
