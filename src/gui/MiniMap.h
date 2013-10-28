/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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
#include "io/resource/PakReader.h"
#include "game/EntityManager.h"
#include "game/Player.h"
#include "graphics/data/Mesh.h"
#include "graphics/VertexBuffer.h"

class TextureContainer;
struct SavedMiniMap;

#define MINIMAP_MAX_X 50
#define MINIMAP_MAX_Z 50
#define MAX_MINIMAP_LEVELS 32

class MiniMap {
	
public:
	
	//! MiniMap data
	struct MiniMapData {
		
		TextureContainer* m_texContainer;
		
		//! Start of scene pos x
		float m_offsetX;
		float m_offsetY;
		
		//! Multiply x by ratioX to obtain real-world pos
		float m_ratioX;
		float m_ratioY;
		
		//! Bitmap width/height
		float m_width;
		float m_height;
		
		unsigned char m_revealed[MINIMAP_MAX_X][MINIMAP_MAX_Z];
		
	};
	
	//! Map markers
	struct MapMarkerData {
		float m_x;
		float m_y;
		int m_lvl;
		std::string m_name;
		std::string m_text;
	};
	
	void mapMarkerRemove(const std::string &name);
	void mapMarkerAdd(float x, float y, int lvl, const std::string &name);
	void mapMarkerInit(size_t reserveSize = 0);
	size_t mapMarkerCount();
	MapMarkerData mapMarkerGet(size_t id);
	
	void firstInit(ARXCHARACTER *pl, PakReader *pakRes, EntityManager *entityMng); // This should be a constructor
	void reset();
	void purgeTexContainer();
	
	/*! 
	* Calls revealPlayerPos if the player moved, also sets m_currentLevel and m_playerPos
	*
	* @param int currentLevel
	* @param long blockPlayerControls
	* @param ARX_INTERFACE_BOOK_MODE bookMode
	*/
	void validatePlayerPos(int currentLevel, long blockPlayerControls, ARX_INTERFACE_BOOK_MODE bookMode);
	
	/*! 
	* Shows the top right minimap
	*
	* @param int showLevel
	*/
	void showPlayerMiniMap(int showLevel);
	
	/*! 
	* Shows the zoomed-in minimap in the book
	*
	* @param int showLevel
	*/
	void showBookMiniMap(int showLevel);
	
	/*!
	* Shows the entire map in the book
	*
	* @param int showLevel
	*/
	void showBookEntireMap(int showLevel);
	
	//! Reveals entirely all levels
	void reveal();

	void clearMarkerTexCont();
	
	void load(const SavedMiniMap *saved, size_t size);
	void save(SavedMiniMap *toSave, size_t size);
	
	void setActiveBackground(EERIE_BACKGROUND *activeBkg);
	
private:
	
	int m_currentLevel;
	EntityManager *m_entities;
	EERIE_BACKGROUND *m_activeBkg;
	
	float m_miniOffsetX[MAX_MINIMAP_LEVELS];
	float m_miniOffsetY[MAX_MINIMAP_LEVELS];
	float m_mapMaxY[MAX_MINIMAP_LEVELS];
	
	TextureContainer *m_pTexDetect;
	TextureContainer *m_mapMarkerTexCont;
	
	ARXCHARACTER *m_player;
	float m_playerLastPosX;
	float m_playerLastPosZ;
	
	/*const */float m_modX; // used everywhere, calculate it once
	/*const */float m_modZ; // should and will be const
	
	std::vector<MapMarkerData> m_mapMarkers;
	MiniMapData m_levels[MAX_MINIMAP_LEVELS];
	
	void getData(int showLevel);
	void resetLevels();
	void loadOffsets(PakReader *pakRes);
	void validatePos();
	
	/*!
	* Reveals the direct surroundings of the player
	*
	* @param int showLevel
	*/
	void revealPlayerPos(int showLevel);
	
	/*!
	* Gets the id from the MapMarker's name. Returns -1 when not found.
	*
	* @param std::string name
	* @return MapMarker's id (int).
	*/
	int mapMarkerGetID(const std::string &name);
	
	Vec2f computePlayerPos(float zoom, int showLevel);
	void drawBackground(int showLevel, Rect boundaries, float startX, float startY, float zoom, float fadeBorder = 0.f, float decalX = 0.f, float decalY = 0.f, bool invColor = false, float alpha = 1.f);
	void drawPlayer(float playerSize, float playerX, float playerY, bool alphaBlending = false);
	void drawDetectedEntities(int showLevel, float startX, float startY, float zoom);
	
    std::vector<TexturedVertex> m_mapVertices;
};

extern MiniMap g_miniMap;

#endif // ARX_GUI_MINIMAP_H
