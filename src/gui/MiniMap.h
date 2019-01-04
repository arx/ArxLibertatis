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

#ifndef ARX_GUI_MINIMAP_H
#define ARX_GUI_MINIMAP_H

#include <string>
#include <vector>
#include "math/Types.h"
#include "gui/Interface.h"
#include "gui/Text.h"
#include "gui/book/Book.h"
#include "io/resource/PakReader.h"
#include "game/EntityManager.h"
#include "game/Player.h"
#include "graphics/data/Mesh.h"
#include "graphics/VertexBuffer.h"

class TextureContainer;
struct SavedMiniMap;

static const size_t MINIMAP_MAX_X = 50;
static const size_t MINIMAP_MAX_Z = 50;
static const size_t MAX_MINIMAP_LEVELS = 32;

class MiniMap {
	
public:
	
	struct MiniMapData {
		
		TextureContainer * m_texContainer;
		
		//! Start of scene pos x
		Vec2f m_offset;
		
		//! Multiply x by ratioX to obtain real-world pos
		Vec2f m_ratio;
		
		//! Bitmap width/height
		Vec2f m_size;
		
		unsigned char m_revealed[MINIMAP_MAX_X][MINIMAP_MAX_Z];
		
		MiniMapData()
			: m_texContainer(NULL)
			, m_offset(0.f)
			, m_ratio(0.f)
			, m_size(0.f)
		{ }
		
	};
	
	struct MapMarkerData {
		
		Vec2f m_pos;
		int m_lvl;
		std::string m_name;
		std::string m_text;
		
		MapMarkerData()
			: m_pos(0.f)
			, m_lvl(0)
		{ }
		
	};
	
	MiniMap()
		: m_currentLevel(0)
		, m_entities(NULL)
		, m_activeBkg(NULL)
		, m_pTexDetect(NULL)
		, m_mapMarkerTexCont(NULL)
		, m_player(NULL)
		, m_playerLastPosX(0.f)
		, m_playerLastPosZ(0.f)
		, m_mod(0.f)
	{ }
	
	void mapMarkerRemove(const std::string & name);
	void mapMarkerAdd(const Vec2f & pos, int lvl, const std::string & name);
	void mapMarkerInit(size_t reserveSize = 0);
	size_t mapMarkerCount();
	MapMarkerData mapMarkerGet(size_t id);
	
	void firstInit(ARXCHARACTER * pl, PakReader * pakRes, EntityManager * entityMng); // This should be a constructor
	void reset();
	void purgeTexContainer();
	
	//! Calls revealPlayerPos if the player moved, also sets m_currentLevel and m_playerPos
	void validatePlayerPos(int currentLevel, bool blockPlayerControls, ARX_INTERFACE_BOOK_MODE bookMode);
	
	//! Shows the top right minimap
	void showPlayerMiniMap(int showLevel);
	
	//! Shows the zoomed-in minimap in the book
	void showBookMiniMap(int showLevel, Rect rect, float scale);
	
	//! Shows the entire map in the book
	void showBookEntireMap(int showLevel, Rect rect, float scale);
	
	//! Reveals entirely all levels
	void reveal();

	void clearMarkerTexCont();
	
	void load(const SavedMiniMap * saved, size_t size);
	void save(SavedMiniMap * toSave, size_t size);
	
	void setActiveBackground(BackgroundData * activeBkg);
	
private:
	
	int m_currentLevel;
	EntityManager * m_entities;
	BackgroundData * m_activeBkg;
	
	Vec2f m_miniOffset[MAX_MINIMAP_LEVELS];
	float m_mapMaxY[MAX_MINIMAP_LEVELS];
	
	TextureContainer * m_pTexDetect;
	TextureContainer * m_mapMarkerTexCont;
	
	ARXCHARACTER * m_player;
	float m_playerLastPosX;
	float m_playerLastPosZ;
	
	// should be const
	Vec2f m_mod;
	
	std::vector<MapMarkerData> m_mapMarkers;
	MiniMapData m_levels[MAX_MINIMAP_LEVELS];
	
	void getData(int showLevel);
	void resetLevels();
	void loadOffsets(PakReader * pakRes);
	void validatePos();
	
	/*!
	* Reveals the direct surroundings of the player
	*/
	void revealPlayerPos(int showLevel);
	
	/*!
	* Gets the id from the MapMarker's name. Returns -1 when not found.
	*
	* \return MapMarker's id (int).
	*/
	int mapMarkerGetID(const std::string & name);
	
	Vec2f computePlayerPos(float zoom, int showLevel);
	void drawBackground(int showLevel, Rect boundaries, Vec2f start, float zoom, float fadeBorder = 0.f, bool invColor = false, float alpha = 1.f);
	void drawPlayer(float playerSize, Vec2f playerPos, bool alphaBlending);
	void drawDetectedEntities(int showLevel, Vec2f start, float zoom);
	
	std::vector<TexturedVertex> m_mapVertices;
	
};

extern MiniMap g_miniMap;

#endif // ARX_GUI_MINIMAP_H
