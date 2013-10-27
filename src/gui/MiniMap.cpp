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

#include "gui/MiniMap.h"

#include <cstdio>

#include "core/Core.h"
#include "core/Localisation.h"

#include "game/EntityManager.h"
#include "game/Levels.h"
#include "game/NPC.h"
#include "game/Player.h"

#include "gui/Text.h"
#include "gui/Interface.h"

#include "graphics/Draw.h"
#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/texture/TextureStage.h"
#include "graphics/texture/Texture.h"

#include "io/log/Logger.h"

#include "scene/Interactive.h"
#include "scene/SaveFormat.h"

MiniMap g_miniMap; // TODO: remove this

void MiniMap::getData(int showLevel) {
	
	if(m_levels[showLevel].m_texContainer == NULL) {
		
		char name[256];
		char levelMap[256];
		GetLevelNameByNum(showLevel, name);
		
		sprintf(levelMap, "graph/levels/level%s/map", name);
		m_levels[showLevel].m_texContainer = TextureContainer::Load(levelMap);
		
		if(m_levels[showLevel].m_texContainer) { // 4 pix/meter
			
			m_levels[showLevel].m_height = static_cast<float>(m_levels[showLevel].m_texContainer->m_dwHeight); 
			m_levels[showLevel].m_width = static_cast<float>(m_levels[showLevel].m_texContainer->m_dwWidth);
			
			float minX = std::numeric_limits<float>::max();
			float maxX = std::numeric_limits<float>::min();
			float minY = std::numeric_limits<float>::max();
			float maxY = std::numeric_limits<float>::min();
			
			EERIEPOLY *ep = NULL;
			EERIE_BKG_INFO *eg = NULL;
			
			for(int j = 0; j < m_activeBkg->Zsize; j++) {
				
				for(int i = 0; i < m_activeBkg->Xsize; i++) {
					eg = &m_activeBkg->Backg[i+j*m_activeBkg->Xsize];
					for(int k = 0; k < eg->nbpoly; k++) {
						ep = &eg->polydata[k];
						if(ep) {
							minX = min(minX, ep->min.x);
							maxX = max(maxX, ep->max.x);
							minY = min(minY, ep->min.z);
							maxY = max(maxY, ep->max.z);
						}
					}
				}
				
				m_mapMaxY[showLevel] = maxY;
				m_levels[showLevel].m_ratioX = minX;
				m_levels[showLevel].m_ratioY = minY;
				
				for(int l = 0; l < MAX_MINIMAP_LEVELS; l++) {
					m_levels[l].m_offsetX = 0;
					m_levels[l].m_offsetY = 0;
				}
			}
		}
	}
}

void MiniMap::validatePos() {
	
	int showLevel = ARX_LEVELS_GetRealNum(m_currentLevel); 
	
	if((showLevel >= 0) && (showLevel < MAX_MINIMAP_LEVELS)) {
		
		if(m_levels[showLevel].m_texContainer == NULL) {
			getData(showLevel);
		}
		
		if(m_levels[m_currentLevel].m_texContainer == NULL) {
			getData(m_currentLevel);
		}
		
		if(m_levels[showLevel].m_texContainer) {
			revealPlayerPos(ARX_LEVELS_GetRealNum(m_currentLevel));
		}
	}
}

void MiniMap::validatePlayerPos(int currentLevel, long blockPlayerControls, ARX_INTERFACE_BOOK_MODE bookMode) {
	
	m_currentLevel = currentLevel;
	
	if(!blockPlayerControls) {
		
		float req;
		
		if((m_player->Interface & INTER_MAP) && (!(m_player->Interface & INTER_COMBATMODE)) && (bookMode == BOOKMODE_MINIMAP)) {
			req = 20.f;
		} else {
			req = 80.f;
		}
		
		if(fartherThan(Vec2f(m_playerLastPosX, m_playerLastPosZ), Vec2f(m_player->pos.x, m_player->pos.z), req)) {
			m_playerLastPosX = m_player->pos.x;
			m_playerLastPosZ = m_player->pos.z;
			validatePos();
		}
	}
}

void MiniMap::loadOffsets(PakReader *pakRes) {
	
	std::string iniMiniOffsets = "graph/levels/mini_offsets.ini";
	
	PakFile *file = pakRes->getFile(iniMiniOffsets.c_str());
	
	if(!file) {
		LogError << "Missing " << iniMiniOffsets;
		return;
	}
	
	size_t fileSize = file->size();
	char *dat = new char[fileSize + 2];
	dat[fileSize + 1] = '\0';
	
	file->read(dat);
	
	if(dat) {
		
		size_t pos = 0;
		
		for(int i = 0; i < 29; i++) { // Why 29?
			
			char t[512];
			int nRead = sscanf(dat + pos, "%s %f %f", t, &m_miniOffsetX[i], &m_miniOffsetY[i]);
			
			if(nRead != 3) {
				LogError << "Error parsing line " << i << " of mini_offsets.ini: read " << nRead;
			}
			
			while((pos < fileSize) && (dat[pos] != '\n')) {
				pos++;
			}
			
			pos++;
			
			if(pos >= fileSize) {
				break;
			}
		}
		
		delete[] dat;
	}
	
	m_miniOffsetX[0] = 0;
	m_miniOffsetY[0] = -0.5;
	m_miniOffsetX[1] = 0;
	m_miniOffsetY[1] = 0;
	m_miniOffsetX[14] = 130;
	m_miniOffsetY[14] = 0;
	m_miniOffsetX[15] = 31;
	m_miniOffsetY[15] = -3.5;
}

void MiniMap::reveal() {
	
	for(int d = 0; d < MAX_MINIMAP_LEVELS; d++) {
		for(int j = 0; j < MINIMAP_MAX_Z; j++) {
			for(int i = 0; i < MINIMAP_MAX_X; i++) {
				m_levels[d].m_revealed[i][j] = 255;
			}
		}
	}
}

void MiniMap::firstInit(ARXCHARACTER *pl, PakReader *pakRes, EntityManager *entityMng) {
	
	m_pTexDetect = NULL;
	m_mapMarkerTexCont = NULL;
	
	m_player = pl;
	m_playerLastPosX = -999999.f;
	m_playerLastPosZ = -999999.f;
	
	m_modX = (float)MAX_BKGX / (float)MINIMAP_MAX_X;
	m_modZ = (float)MAX_BKGZ / (float)MINIMAP_MAX_Z;
	
	m_currentLevel = 0;
	m_entities = entityMng;
	m_activeBkg = NULL;

    m_mapVertices.reserve(MINIMAP_MAX_X * MINIMAP_MAX_Z);

	resetLevels();
	
	for(int i = 0; i < MAX_MINIMAP_LEVELS; i++) {
		m_miniOffsetX[i] = 0;
		m_miniOffsetY[i] = 0;
	}
	
	loadOffsets(pakRes);
}

void MiniMap::resetLevels() {
	
	for(int i = 0; i < MAX_MINIMAP_LEVELS; i++) {
		m_levels[i].m_texContainer = NULL;
		m_levels[i].m_offsetX = 0.f;
		m_levels[i].m_offsetY = 0.f;
		m_levels[i].m_ratioX = 0.f;
		m_levels[i].m_ratioY = 0.f;
		m_levels[i].m_width = 0.f;
		m_levels[i].m_height = 0.f;
		memset(m_levels[i].m_revealed, 0, sizeof(m_levels[i].m_revealed[0][0] * MINIMAP_MAX_X * MINIMAP_MAX_Z)); // Sets the whole array to 0
	}
}

void MiniMap::reset() {
	
	purgeTexContainer();
	resetLevels();
}

void MiniMap::purgeTexContainer() {
	
	for(int i = 0; i < MAX_MINIMAP_LEVELS; i++) {
		delete m_levels[i].m_texContainer;
		m_levels[i].m_texContainer = NULL;
	}
}

void MiniMap::showPlayerMiniMap(int showLevel) {
	
	const float miniMapZoom = 300.f; // zoom of the minimap
	const Rect miniMapRect(390, 135, 590, 295); // minimap rect on a 640*480 screen
	const float playerSize = 4.f; // red arrow size
	
	const float decalY = -150;
	const float decalX = +40;
	
	// First Load Minimap TC & DATA if needed
	if(m_levels[showLevel].m_texContainer == NULL) {
		getData(showLevel);
	}
	
	if(m_levels[showLevel].m_texContainer) {
		
		GRenderer->SetRenderState(Renderer::DepthTest, false);
		
		float startX = 0.f;
		float startY = 0.f;
		
		Vec2f playerPos(0.f, 0.f);
		
		if(showLevel == ARX_LEVELS_GetRealNum(m_currentLevel)) {
			playerPos = computePlayerPos(miniMapZoom, showLevel);
			startX = 490.f - playerPos.x;
			startY = 220.f - playerPos.y;
			playerPos.x += startX;
			playerPos.y += startY;
		}
		
		// Draw the background
		drawBackground(showLevel, Rect(390, 135, 590, 295), startX, startY, miniMapZoom, 20.f, decalX, decalY, true, 0.5f);
		
		GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
		
		// Draw the player (red arrow)
		if(showLevel == ARX_LEVELS_GetRealNum(m_currentLevel)) {
			drawPlayer(playerSize, playerPos.x + decalX, playerPos.y + decalY, true);
			drawDetectedEntities(showLevel, startX + decalX, startY + decalY, miniMapZoom);
		}
		
	}
}

void MiniMap::showBookMiniMap(int showLevel) {
	
	// First Load Minimap TC & DATA if needed
	if(m_levels[showLevel].m_texContainer == NULL) {
		getData(showLevel);
	}
	
	if(m_levels[showLevel].m_texContainer) {
		
		GRenderer->SetRenderState(Renderer::DepthTest, false);
		
		float zoom = 900.f;
		float startX = 0.f;
		float startY = 0.f;
		
		Vec2f playerPos(0.f, 0.f);
		
		if(showLevel == ARX_LEVELS_GetRealNum(m_currentLevel)) {
			playerPos = computePlayerPos(zoom, showLevel);
			startX = 490.f - playerPos.x;
			startY = 220.f - playerPos.y;
			playerPos.x += startX;
			playerPos.y += startY;
		}
		
		drawBackground(showLevel, Rect(360, 85, 555, 355), startX, startY, zoom, 20.f);
		
		GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
		
		if(showLevel == ARX_LEVELS_GetRealNum(m_currentLevel)) {
			drawPlayer(6.f, playerPos.x, playerPos.y);
			drawDetectedEntities(showLevel, startX, startY, zoom);
		}
		
	}
}

void MiniMap::showBookEntireMap(int showLevel) {
	
	// First Load Minimap TC & DATA if needed
	if(m_levels[showLevel].m_texContainer == NULL) {
		getData(showLevel);
	}
	
	if(!m_levels[showLevel].m_texContainer) {
		return;
	}
	
	GRenderer->SetRenderState(Renderer::DepthTest, false);
	
	float zoom = 250.f;
	float startX = 140.f;
	float startY = 120.f;
	
	Vec2f playerPos(0.f, 0.f);
	
	if(showLevel == ARX_LEVELS_GetRealNum(m_currentLevel)) {
		playerPos = computePlayerPos(zoom, showLevel);
		playerPos.x += startX;
		playerPos.y += startY;
	}
	
	drawBackground(showLevel, Rect(0, 0, 345, 290), startX, startY, zoom);
	
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
	
	if(showLevel == ARX_LEVELS_GetRealNum(m_currentLevel)) {
		drawPlayer(3.f, playerPos.x, playerPos.y);
		drawDetectedEntities(showLevel, startX, startY, zoom);
	}
	
	TexturedVertex verts[4];
	for(int k = 0; k < 4; k++) {
		verts[k].color = 0xFFFFFFFF;
		verts[k].rhw = 1;
		verts[k].p.z = 0.00001f;
	}
	
	float caseX = zoom / ((float)MINIMAP_MAX_X);
	float caseY = zoom / ((float)MINIMAP_MAX_Z);
	float ratio = 1.f;
	
	for(size_t i = 0; i < m_mapMarkers.size(); i++) {
		
		if(m_mapMarkers[i].m_lvl != showLevel + 1) {
			continue;
		}
		
		float pos_x = m_mapMarkers[i].m_x * 8 * ratio * m_activeBkg->Xmul * caseX + startX;
		float pos_y = m_mapMarkers[i].m_y * 8 * ratio * m_activeBkg->Zmul * caseY + startY;
		float size = 5.f * ratio;
		verts[0].color = 0xFFFF0000;
		verts[1].color = 0xFFFF0000;
		verts[2].color = 0xFFFF0000;
		verts[3].color = 0xFFFF0000;
		verts[0].p.x = (pos_x - size) * Xratio;
		verts[0].p.y = (pos_y - size) * Yratio;
		verts[1].p.x = (pos_x + size) * Xratio;
		verts[1].p.y = (pos_y - size) * Yratio;
		verts[2].p.x = (pos_x + size) * Xratio;
		verts[2].p.y = (pos_y + size) * Yratio;
		verts[3].p.x = (pos_x - size) * Xratio;
		verts[3].p.y = (pos_y + size) * Yratio;
		verts[0].uv = Vec2f_ZERO;
		verts[1].uv = Vec2f_X_AXIS;
		verts[2].uv = Vec2f_ONE;
		verts[3].uv = Vec2f_Y_AXIS;
		
		if(MouseInRect(verts[0].p.x, verts[0].p.y, verts[2].p.x, verts[2].p.y)) {
			if(!m_mapMarkers[i].m_text.empty()) {
				
				Rect bRect(140, 290, 140 + 205, 358);
				
				Rect::Num left = checked_range_cast<Rect::Num>((bRect.left) * Xratio);
				Rect::Num right = checked_range_cast<Rect::Num>((bRect.right) * Xratio);
				Rect::Num top = checked_range_cast<Rect::Num>((bRect.top) * Yratio);
				Rect::Num bottom = checked_range_cast<Rect::Num>((bRect.bottom) * Yratio);
				Rect rRect = Rect(left, top, right, bottom);
				
				long lLengthDraw = ARX_UNICODE_ForceFormattingInRect(hFontInGameNote, m_mapMarkers[i].m_text, rRect);
				
				DrawBookTextInRect(hFontInGameNote, float(bRect.left), float(bRect.top), float(bRect.right), m_mapMarkers[i].m_text.substr(0, lLengthDraw), Color::none);
			}
		}
		
		if(m_mapMarkerTexCont == NULL) {
			m_mapMarkerTexCont = TextureContainer::Load("graph/interface/icons/mapmarker");
		}
		
		GRenderer->SetTexture(0, m_mapMarkerTexCont);
		
		EERIEDRAWPRIM(Renderer::TriangleFan, verts, 4);
	}
}

void MiniMap::revealPlayerPos(int showLevel) {
	
	float zoom = 250.f;
	float startX = 140.f;
	float startY = 120.f;
	float caseX = zoom / ((float)MINIMAP_MAX_X);
	float caseY = zoom / ((float)MINIMAP_MAX_Z);
	
	Vec2f playerPos = computePlayerPos(zoom, showLevel);
	playerPos.x += startX;
	playerPos.y += startY;
	
	// TODO this is inefficient - we don't really need to iterate over the whole minimap!
	// only the area around the player will be modified
	for(int j = 0; j < MINIMAP_MAX_Z; j++) {
		for(int i = 0; i < MINIMAP_MAX_X; i++) {
			
			float posx = startX + i * caseX;
			float posy = startY + j * caseY;
			
			float d = fdist(Vec2f(posx + caseX * 0.5f, posy), playerPos);
			if(d > 6.f) {
				continue;
			}
			
			float vv = (6 - d) * (1.f / 6);
			
			if(vv >= 0.5f) {
				vv = 1.f;
			} else if(vv > 0.f) {
				vv = vv * 2.f;
			} else {
				vv = 0.f;
			}
			
			int r = vv * 255.f;
			
			int ucLevel =  max(r, (int)m_levels[showLevel].m_revealed[i][j]);
			m_levels[showLevel].m_revealed[i][j] = checked_range_cast<unsigned char>(ucLevel);
		}
	}
}

Vec2f MiniMap::computePlayerPos(float zoom, int showLevel) {
	
	float caseX = zoom / ((float)MINIMAP_MAX_X);
	float caseY = zoom / ((float)MINIMAP_MAX_Z);
	float ratio = zoom / 250.f;
	
	Vec2f pos(0.f, 0.f);
	
	float ofx = m_miniOffsetX[m_currentLevel];
	float ofx2 = m_levels[showLevel].m_ratioX;
	float ofy = m_miniOffsetY[m_currentLevel];
	float ofy2 = m_levels[showLevel].m_ratioY;
	
	pos.x = ((m_player->pos.x + ofx - ofx2) * ( 1.0f / 100 ) * caseX
	+ m_miniOffsetX[m_currentLevel] * ratio * m_modX) / m_modX;
	pos.y = ((m_mapMaxY[showLevel] - ofy - ofy2) * ( 1.0f / 100 ) * caseY
	- (m_player->pos.z + ofy - ofy2) * ( 1.0f / 100 ) * caseY + m_miniOffsetY[m_currentLevel] * ratio * m_modZ) / m_modZ;
	
	return pos;
}

void MiniMap::drawBackground(int showLevel, Rect boundaries, float startX, float startY, float zoom, float fadeBorder, float decalX, float decalY, bool invColor, float alpha) {
	
    m_mapVertices.resize(0);

	float caseX = zoom / ((float)MINIMAP_MAX_X);
	float caseY = zoom / ((float)MINIMAP_MAX_Z);
	
	GRenderer->SetTexture(0, m_levels[showLevel].m_texContainer);
	
	float div = (1.0f / 25);
	TextureContainer * tc = m_levels[showLevel].m_texContainer;
	float dw = 1.f / tc->m_pTexture->getStoredSize().x; 
	float dh = 1.f / tc->m_pTexture->getStoredSize().y;
	
	float vx2 = 4.f * dw * m_modX;
	float vy2 = 4.f * dh * m_modZ;
	
	float fadeDiv = 0.f;
	Rect fadeBounds(0, 0, 0, 0);
	
	if(fadeBorder > 0.f) {
		fadeDiv = 1.f/fadeBorder;
		fadeBounds.left = checked_range_cast<Rect::Num>((boundaries.left + fadeBorder) * Xratio);
		fadeBounds.right = checked_range_cast<Rect::Num>((boundaries.right - fadeBorder) * Xratio);
		fadeBounds.top = checked_range_cast<Rect::Num>((boundaries.top + fadeBorder) * Yratio);
		fadeBounds.bottom = checked_range_cast<Rect::Num>((boundaries.bottom - fadeBorder) * Yratio);
	}
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	if(invColor) {
		GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendInvSrcColor);
	} else {
		GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
	}
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapClamp);

	for(int j = -2; j < MINIMAP_MAX_Z + 2; j++) {
		for(int i = -2; i < MINIMAP_MAX_X + 2; i++) {
			
			float vx, vy, vxx, vyy;
			vxx = ((float)i * (float)m_activeBkg->Xdiv * m_modX);
			vyy = ((float)j * (float)m_activeBkg->Zdiv * m_modZ);
			vx = (vxx * div) * dw;
			vy = (vyy * div) * dh;
			
			float posx = (startX + i * caseX) * Xratio;
			float posy = (startY + j * caseY) * Yratio;
			
			if((posx < boundaries.left * Xratio)
			   || (posx > boundaries.right * Xratio)
			   || (posy < boundaries.top * Yratio)
			   || (posy > boundaries.bottom * Yratio)) {
				continue; // out of bounds
			}

			TexturedVertex verts[4];
			
			verts[3].p.x = verts[0].p.x = (posx);
			verts[1].p.y = verts[0].p.y = (posy);
			verts[2].p.x = verts[1].p.x = posx + (caseX * Xratio);
			verts[3].p.y = verts[2].p.y = posy + (caseY * Yratio);
			
			verts[3].uv.x = verts[0].uv.x = vx;
			verts[1].uv.y = verts[0].uv.y = vy;
			verts[2].uv.x = verts[1].uv.x = vx + vx2;
			verts[3].uv.y = verts[2].uv.y = vy + vy2;
			
			float v;
			float oo = 0.f;
			
			for(int vert = 0; vert < 4; vert++) {
				verts[vert].color = 0xFFFFFFFF;
				verts[vert].rhw = 1;
				verts[vert].p.z = 0.00001f;

				// Array offset according to "vert"
				int iOffset = 0;
				int jOffset = 0;
				
				if(vert == 1 || vert == 2)
					iOffset = 1;
				if(vert == 2 || vert == 3)
					jOffset = 1;
				
				if((i + iOffset < 0) || (i + iOffset >= MINIMAP_MAX_X) || (j + jOffset < 0) || (j + jOffset >= MINIMAP_MAX_Z)) {
					v = 0;
				} else {
					v = ((float)m_levels[showLevel].m_revealed[min(i+iOffset, MINIMAP_MAX_X-iOffset)][min(j+jOffset, MINIMAP_MAX_Z-jOffset)]) * (1.0f / 255);
				}
				
				if(fadeBorder > 0.f) {
					
					float _px = verts[vert].p.x - fadeBounds.left;
					
					if(_px < 0.f) {
						v = 0.f;
					} else if(_px < fadeBorder) {
						v *= _px * fadeDiv;
					}
					
					_px = fadeBounds.right - verts[vert].p.x;
					
					if(_px < 0.f) {
						v = 0.f;
					} else if(_px < fadeBorder) {
						v *= _px * fadeDiv;
					}
					
					_px = verts[vert].p.y - fadeBounds.top;
					
					if(_px < 0.f) {
						v = 0.f;
					} else if(_px < fadeBorder) {
						v *= _px * fadeDiv;
					}
					
					_px = fadeBounds.bottom - verts[vert].p.y;
					
					if(_px < 0.f) {
						v = 0.f;
					} else if(_px < fadeBorder) {
						v *= _px * fadeDiv;
					}
				}
				
				verts[vert].color = Color::gray(v * alpha).toBGR();
				
				oo += v;
			}
			
			if(oo > 0.f) {
				
				verts[0].p.x += decalX * Xratio;
				verts[0].p.y += decalY * Yratio;
				verts[1].p.x += decalX * Xratio;
				verts[1].p.y += decalY * Yratio;
				verts[2].p.x += decalX * Xratio;
				verts[2].p.y += decalY * Yratio;
				verts[3].p.x += decalX * Xratio;
				verts[3].p.y += decalY * Yratio;
				
				m_mapVertices.push_back(verts[0]);
				m_mapVertices.push_back(verts[1]);
				m_mapVertices.push_back(verts[2]);

				m_mapVertices.push_back(verts[0]);
				m_mapVertices.push_back(verts[2]);
				m_mapVertices.push_back(verts[3]);
			}
		}
	}

	if(!m_mapVertices.empty()) {
		EERIEDRAWPRIM(Renderer::TriangleList, &m_mapVertices[0], m_mapVertices.size());
	}
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}

void MiniMap::drawPlayer(float playerSize, float playerX, float playerY, bool alphaBlending) {
	
	TexturedVertex verts[4];
	
	for(int k = 0; k < 4; k++) {
		verts[k].color = 0xFFFF0000; // red
		verts[k].rhw = 1;
		verts[k].p.z = 0.00001f;
	}
	
	float rx = 0.f;
	float ry = -playerSize * 1.8f;
	float rx2 = -playerSize * (1.0f / 2);
	float ry2 = playerSize;
	float rx3 = playerSize * (1.0f / 2);
	float ry3 = playerSize;
	
	float angle = radians(m_player->angle.getPitch());
	float ca = EEcos(angle);
	float sa = EEsin(angle);
	
	verts[0].p.x = (playerX + rx2 * ca + ry2 * sa) * Xratio;
	verts[0].p.y = (playerY + ry2 * ca - rx2 * sa) * Yratio;
	verts[1].p.x = (playerX + rx * ca + ry * sa) * Xratio;
	verts[1].p.y = (playerY + ry * ca - rx * sa) * Yratio;
	verts[2].p.x = (playerX + rx3 * ca + ry3 * sa) * Xratio;
	verts[2].p.y = (playerY + ry3 * ca - rx3 * sa) * Yratio;
	
	GRenderer->ResetTexture(0);
	GRenderer->SetRenderState(Renderer::AlphaBlending, alphaBlending);
	if(alphaBlending) {
		GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendInvSrcColor);
	}
	
	EERIEDRAWPRIM(Renderer::TriangleFan, verts);
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}

void MiniMap::drawDetectedEntities(int showLevel, float startX, float startY, float zoom) {
	
	float caseX = zoom / ((float)MINIMAP_MAX_X);
	float caseY = zoom / ((float)MINIMAP_MAX_Z);
	float ratio = zoom / 250.f;
	
	if(!m_pTexDetect) {
		m_pTexDetect = TextureContainer::Load("graph/particles/flare");
	}
	
	// Computes playerpos
	float ofx = m_miniOffsetX[m_currentLevel];
	float ofx2 = m_levels[showLevel].m_ratioX;
	float ofy = m_miniOffsetY[m_currentLevel];
	float ofy2 = m_levels[showLevel].m_ratioY;
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	
	const EntityManager &ents = *m_entities; // for convenience
	for(size_t lnpc = 1; lnpc < ents.size(); lnpc++) {
		Entity * npc = ents[lnpc];
		
		if(!npc || !(npc->ioflags & IO_NPC)) {
			continue; // only NPCs can be detected
		}
		
		if(npc->_npcdata->life < 0.f) {
			continue; // don't show dead NPCs
		}
		
		if((npc->gameFlags & GFLAG_MEGAHIDE) || npc->show != SHOW_FLAG_IN_SCENE) {
			continue; // don't show hidden NPCs
		}
		
		if(npc->_npcdata->fDetect < 0) {
			continue; // don't show undetectable NPCs
		}
		
		if(player.Full_Skill_Etheral_Link < npc->_npcdata->fDetect) {
			continue; // the player doesn't have enough skill to detect this NPC
		}
		
		float fpx = startX + ((npc->pos.x - 100 + ofx - ofx2) * ( 1.0f / 100 ) * caseX
		+ m_miniOffsetX[m_currentLevel] * ratio * m_modX) / m_modX; 
		float fpy = startY + ((m_mapMaxY[showLevel] - ofy - ofy2) * ( 1.0f / 100 ) * caseY
		- (npc->pos.z + 200 + ofy - ofy2) * ( 1.0f / 100 ) * caseY + m_miniOffsetY[m_currentLevel] * ratio * m_modZ) / m_modZ;
		
		float d = fdist(Vec2f(m_player->pos.x, m_player->pos.z), Vec2f(npc->pos.x, npc->pos.z));
		if(d > 800 || fabs(ents.player()->pos.y - npc->pos.y) > 250.f) {
			continue; // the NPC is too far away to be detected
		}
		
		float col = 1.f;
		
		if(d > 600.f) {
			col = 1.f - (d - 600.f) * ( 1.0f / 200 );
		}
		
		fpx *= Xratio;
		fpy *= Yratio;
		EERIEDrawBitmap(fpx, fpy, 5.f * ratio, 5.f * ratio, 0, m_pTexDetect,
		Color3f(col, 0, 0).to<u8>());
	}
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}

void MiniMap::clearMarkerTexCont() {
	m_mapMarkerTexCont = NULL;
}

void MiniMap::load(const SavedMiniMap *saved, size_t size) {
	std::copy(saved, saved + size, m_levels);
}

void MiniMap::save(SavedMiniMap *toSave, size_t size) {
	std::copy(m_levels, m_levels + size, toSave);
}

void MiniMap::mapMarkerInit(size_t reserveSize) {
	m_mapMarkers.clear();
	
	if(reserveSize > 0)
		m_mapMarkers.reserve(reserveSize);
}

int MiniMap::mapMarkerGetID(const std::string &name) {
	
	for(size_t i = 0; i < m_mapMarkers.size(); i++) {
		if(m_mapMarkers[i].m_name == name) {
			return i;
		}
	}
	
	return -1;
}

void MiniMap::mapMarkerAdd(float x, float y, int lvl, const std::string &name) {
	
	int num = mapMarkerGetID(name);
	
	if(num >= 0) {
		// Already exists, update it
		m_mapMarkers[num].m_lvl = lvl;
		m_mapMarkers[num].m_x = x;
		m_mapMarkers[num].m_y = y;
		return;
	}
	
	// Else, create one
	MapMarkerData newMMD;
	newMMD.m_lvl = lvl;
	newMMD.m_x = x;
	newMMD.m_y = y;
	newMMD.m_name = name;
	newMMD.m_text = getLocalised(name);
	m_mapMarkers.push_back(newMMD);
}

void MiniMap::mapMarkerRemove(const std::string &name) {
	
	int num = mapMarkerGetID(name);
	
	if(num < 0) {
		return; // Doesn't exist
	}
	
	m_mapMarkers.erase(m_mapMarkers.begin() + num);
}

size_t MiniMap::mapMarkerCount() {
	return m_mapMarkers.size();
}

MiniMap::MapMarkerData MiniMap::mapMarkerGet(size_t id) {
	
	assert(id < m_mapMarkers.size());
	return m_mapMarkers[id];
}

void MiniMap::setActiveBackground(EERIE_BACKGROUND *activeBkg) {
	m_activeBkg = activeBkg;
}
