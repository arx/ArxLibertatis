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
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#ifndef ARX_GUI_MINIMAP_H
#define ARX_GUI_MINIMAP_H

#include <string>
#include <vector>

class TextureContainer;

#define MINIMAP_MAX_X 50
#define MINIMAP_MAX_Z 50
#define MAX_MINIMAP_DATA 32

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
    
    MiniMapData m_data[MAX_MINIMAP_DATA];
    
    //! Map markers
    struct MapMarkerData {

        float m_x;
        float m_y;
        int m_lvl;
        std::string m_name;
        std::string m_text;
    };
    
    std::vector<MapMarkerData> m_mapMarkers;
    
    void mapMarkerRemove(const std::string &name);
    void mapMarkerAdd(float x, float y, int lvl, const std::string &name);
    void mapMarkerInit();
    
    void firstInit(); // This should be a constructor
    void reset();
    void purgeTexContainer();
    void validatePlayerPos();
    
    void show(int showLevel, int flag, int fl2 = 0);
    void reveal();
    
    void clearMarkerTexCont();
    
private:

    float m_miniOffsetX[MAX_MINIMAP_DATA];
    float m_miniOffsetY[MAX_MINIMAP_DATA];
    float m_mapMaxY[MAX_MINIMAP_DATA];
    
    TextureContainer *m_pTexDetect;
    TextureContainer *m_mapMarkerTexCont;
    
    float m_playerLastPosX;
    float m_playerLastPosZ;
    
    void getData(int showLevel);
    void resetData();
    void loadOffsets();
    void validatePos();
    
    /*! 
    * Gets the id from the MapMarker's name. Returns -1 when not found.
    *
    * @param std::string name
    * @return MapMarker's id (int).
    */
    int mapMarkerGetID(const std::string &name);
};

extern MiniMap g_miniMap;

#endif // ARX_GUI_MINIMAP_H
