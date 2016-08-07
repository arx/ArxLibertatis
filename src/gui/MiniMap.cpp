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

#include "platform/profiler/Profiler.h"

#include "scene/Interactive.h"
#include "scene/SaveFormat.h"

MiniMap g_miniMap; // TODO: remove this

void MiniMap::getData(int showLevel) {
	
	if(m_levels[showLevel].m_texContainer == NULL) {
		
		char levelMap[256];
		const char * name = GetLevelNameByNum(showLevel);
		
		sprintf(levelMap, "graph/levels/level%s/map", name);
		m_levels[showLevel].m_texContainer = TextureContainer::Load(levelMap, TextureContainer::NoColorKey);
		
		if(m_levels[showLevel].m_texContainer) { // 4 pix/meter
			
			m_levels[showLevel].m_size = Vec2f(m_levels[showLevel].m_texContainer->m_size);
			
			float minX = std::numeric_limits<float>::max();
			float maxX = std::numeric_limits<float>::min();
			float minY = std::numeric_limits<float>::max();
			float maxY = std::numeric_limits<float>::min();
			
			for(int z = 0; z < m_activeBkg->Zsize; z++) {
				
				for(int x = 0; x < m_activeBkg->Xsize; x++) {
					const EERIE_BKG_INFO & eg = m_activeBkg->fastdata[x][z];
					for(int k = 0; k < eg.nbpoly; k++) {
						const EERIEPOLY & ep = eg.polydata[k];
						
						minX = std::min(minX, ep.min.x);
						maxX = std::max(maxX, ep.max.x);
						minY = std::min(minY, ep.min.z);
						maxY = std::max(maxY, ep.max.z);
					}
				}
				
				m_mapMaxY[showLevel] = maxY;
				m_levels[showLevel].m_ratio.x = minX;
				m_levels[showLevel].m_ratio.y = minY;
				
				for(size_t l = 0; l < MAX_MINIMAP_LEVELS; l++) {
					m_levels[l].m_offset = Vec2f_ZERO;
				}
			}
		}
	}
}

void MiniMap::validatePos() {
	
	int showLevel = ARX_LEVELS_GetRealNum(m_currentLevel); 
	
	if((showLevel >= 0) && (showLevel < int(MAX_MINIMAP_LEVELS))) {
		
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
	char * dat = new char[fileSize + 1];
	dat[fileSize] = '\0';
	
	file->read(dat);
	
	size_t pos = 0;
	
	for(int i = 0; i < 29; i++) { // Why 29?
		
		char t[512];
		int nRead = sscanf(dat + pos, "%s %f %f", t, &m_miniOffset[i].x, &m_miniOffset[i].y);
		
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
	
	m_miniOffset[0] = Vec2f(0, -0.5);
	m_miniOffset[1] = Vec2f(0, 0);
	m_miniOffset[14] = Vec2f(130, 0);
	m_miniOffset[15] = Vec2f(31, -3.5);
}

void MiniMap::reveal() {
	
	for(size_t l = 0; l < MAX_MINIMAP_LEVELS; l++) {
		for(size_t z = 0; z < MINIMAP_MAX_Z; z++) {
			for(size_t x = 0; x < MINIMAP_MAX_X; x++) {
				m_levels[l].m_revealed[x][z] = 255;
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
	
	m_mod.x = float(MAX_BKGX) / MINIMAP_MAX_X;
	m_mod.y = float(MAX_BKGZ) / MINIMAP_MAX_Z;
	
	m_currentLevel = 0;
	m_entities = entityMng;
	m_activeBkg = NULL;

    m_mapVertices.reserve(MINIMAP_MAX_X * MINIMAP_MAX_Z);

	resetLevels();
	
	for(size_t i = 0; i < MAX_MINIMAP_LEVELS; i++) {
		m_miniOffset[i] = Vec2f_ZERO;
	}
	
	loadOffsets(pakRes);
}

void MiniMap::resetLevels() {
	
	for(size_t i = 0; i < MAX_MINIMAP_LEVELS; i++) {
		m_levels[i].m_texContainer = NULL;
		m_levels[i].m_offset = Vec2f_ZERO;
		m_levels[i].m_ratio = Vec2f_ZERO;
		m_levels[i].m_size = Vec2f_ZERO;
		// Sets the whole array to 0
		memset(m_levels[i].m_revealed, 0, sizeof(m_levels[i].m_revealed));
	}
}

void MiniMap::reset() {
	
	purgeTexContainer();
	resetLevels();
}

void MiniMap::purgeTexContainer() {
	
	for(size_t i = 0; i < MAX_MINIMAP_LEVELS; i++) {
		delete m_levels[i].m_texContainer;
		m_levels[i].m_texContainer = NULL;
	}
}

void MiniMap::showPlayerMiniMap(int showLevel) {
	
	ARX_PROFILE_FUNC();
	
	const float miniMapZoom = 300.f; // zoom of the minimap
	const Rect miniMapRect(390, 135, 590, 295); // minimap rect on a 640*480 screen
	const float playerSize = 4.f; // red arrow size
	
	static const Vec2f decal = Vec2f(40.f, -150.f);
	
	// First Load Minimap TC & DATA if needed
	if(m_levels[showLevel].m_texContainer == NULL) {
		getData(showLevel);
	}
	
	if(m_levels[showLevel].m_texContainer) {
		
		GRenderer->SetRenderState(Renderer::DepthTest, false);
		
		Vec2f start = Vec2f_ZERO;
		
		Vec2f playerPos(0.f, 0.f);
		
		if(showLevel == ARX_LEVELS_GetRealNum(m_currentLevel)) {
			playerPos = computePlayerPos(miniMapZoom, showLevel);
			start = Vec2f(490.f, 220.f) - playerPos;
			playerPos += start;
		}
		
		// Draw the background
		drawBackground(showLevel, Rect(390, 135, 590, 295), start, miniMapZoom, 20.f, decal, true, 0.5f);
		
		GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
		
		// Draw the player (red arrow)
		if(showLevel == ARX_LEVELS_GetRealNum(m_currentLevel)) {
			drawPlayer(playerSize, playerPos + decal, true);
			drawDetectedEntities(showLevel, start + decal, miniMapZoom);
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
		
		Vec2f start = Vec2f_ZERO;
		Vec2f playerPos(0.f, 0.f);
		
		if(showLevel == ARX_LEVELS_GetRealNum(m_currentLevel)) {
			playerPos = computePlayerPos(zoom, showLevel);
			start = Vec2f(490.f, 220.f) - playerPos;
			playerPos += start;
		}
		
		drawBackground(showLevel, Rect(360, 85, 555, 355), start, zoom, 20.f);
		
		GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
		
		if(showLevel == ARX_LEVELS_GetRealNum(m_currentLevel)) {
			drawPlayer(6.f, playerPos, false);
			drawDetectedEntities(showLevel, start, zoom);
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
	
	Vec2f start(140.f, 120.f);
	
	Vec2f playerPos(0.f, 0.f);
	
	if(showLevel == ARX_LEVELS_GetRealNum(m_currentLevel)) {
		playerPos = computePlayerPos(zoom, showLevel);
		playerPos += start;
	}
	
	drawBackground(showLevel, Rect(0, 0, 345, 290), start, zoom);
	
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
	
	if(showLevel == ARX_LEVELS_GetRealNum(m_currentLevel)) {
		drawPlayer(3.f, playerPos, false);
		drawDetectedEntities(showLevel, start, zoom);
	}
	
	TexturedVertex verts[4];
	for(int k = 0; k < 4; k++) {
		verts[k].color = Color(255, 255, 255, 255).toRGBA();
		verts[k].rhw = 1;
		verts[k].p.z = 0.00001f;
	}
	
	Vec2f casePos(zoom / MINIMAP_MAX_X, zoom / MINIMAP_MAX_Z);
	float ratio = 1.f;
	
	for(size_t i = 0; i < m_mapMarkers.size(); i++) {
		
		if(m_mapMarkers[i].m_lvl != showLevel + 1) {
			continue;
		}
		
		Vec2f pos;
		pos.x = m_mapMarkers[i].m_pos.x * 8 * ratio * m_activeBkg->Xmul * casePos.x + start.x;
		pos.y = m_mapMarkers[i].m_pos.y * 8 * ratio * m_activeBkg->Zmul * casePos.y + start.y;
		
		float size = 5.f * ratio;
		verts[0].color = Color(255, 0, 0, 255).toRGBA();
		verts[1].color = Color(255, 0, 0, 255).toRGBA();
		verts[2].color = Color(255, 0, 0, 255).toRGBA();
		verts[3].color = Color(255, 0, 0, 255).toRGBA();
		verts[0].p.x = (pos.x - size) * g_sizeRatio.x;
		verts[0].p.y = (pos.y - size) * g_sizeRatio.y;
		verts[1].p.x = (pos.x + size) * g_sizeRatio.x;
		verts[1].p.y = (pos.y - size) * g_sizeRatio.y;
		verts[2].p.x = (pos.x + size) * g_sizeRatio.x;
		verts[2].p.y = (pos.y + size) * g_sizeRatio.y;
		verts[3].p.x = (pos.x - size) * g_sizeRatio.x;
		verts[3].p.y = (pos.y + size) * g_sizeRatio.y;
		verts[0].uv = Vec2f_ZERO;
		verts[1].uv = Vec2f_X_AXIS;
		verts[2].uv = Vec2f_ONE;
		verts[3].uv = Vec2f_Y_AXIS;
		
		const Rect mouseTestRect(
			verts[0].p.x,
			verts[0].p.y,
			verts[2].p.x,
			verts[2].p.y
		);
		
		if(mouseTestRect.contains(Vec2i(DANAEMouse))) {
			if(!m_mapMarkers[i].m_text.empty()) {
				
				Rect bRect(140, 295, 140 + 205, 358);
				
				Rect::Num left = checked_range_cast<Rect::Num>((bRect.left) * g_sizeRatio.x);
				Rect::Num right = checked_range_cast<Rect::Num>((bRect.right) * g_sizeRatio.x);
				Rect::Num top = checked_range_cast<Rect::Num>((bRect.top) * g_sizeRatio.y);
				Rect::Num bottom = checked_range_cast<Rect::Num>((bRect.bottom) * g_sizeRatio.y);
				Rect rRect = Rect(left, top, right, bottom);
				
				long lLengthDraw = ARX_UNICODE_ForceFormattingInRect(hFontInGameNote, m_mapMarkers[i].m_text, rRect);
				
				
				ARX_UNICODE_DrawTextInRect(hFontInGameNote,
				                           (BOOKDEC + Vec2f(bRect.topLeft())) * g_sizeRatio,
				                           (BOOKDEC.x + float(bRect.right)) * g_sizeRatio.x,
				                           m_mapMarkers[i].m_text.substr(0, lLengthDraw),
				                           Color::none);
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
	Vec2f start = Vec2f(140.f, 120.f);
	Vec2f cas;
	cas.x = zoom / MINIMAP_MAX_X;
	cas.y = zoom / MINIMAP_MAX_Z;
	
	Vec2f playerPos = computePlayerPos(zoom, showLevel);
	playerPos += start;
	
	// TODO this is inefficient - we don't really need to iterate over the whole minimap!
	// only the area around the player will be modified
	for(size_t z = 0; z < MINIMAP_MAX_Z; z++) {
	for(size_t x = 0; x < MINIMAP_MAX_X; x++) {
		
		Vec2f pos;
		pos.x = start.x + x * cas.x;
		pos.y = start.y + z * cas.y;
		
		float d = fdist(Vec2f(pos.x + cas.x * 0.5f, pos.y), playerPos);
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
		
		int ucLevel = std::max(r, (int)m_levels[showLevel].m_revealed[x][z]);
		m_levels[showLevel].m_revealed[x][z] = checked_range_cast<unsigned char>(ucLevel);
	}
	}
}

Vec2f MiniMap::computePlayerPos(float zoom, int showLevel) {
	
	Vec2f cas;
	cas.x = zoom / MINIMAP_MAX_X;
	cas.y = zoom / MINIMAP_MAX_Z;
	
	float ratio = zoom / 250.f;
	
	Vec2f pos(0.f, 0.f);
	
	const Vec2f of = m_miniOffset[m_currentLevel];
	const Vec2f of2 = m_levels[showLevel].m_ratio;
	
	pos.x = ((m_player->pos.x + of.x - of2.x) * ( 1.0f / 100 ) * cas.x
	+ of.x * ratio * m_mod.x) / m_mod.x;
	pos.y = ((m_mapMaxY[showLevel] - of.y - of2.y) * ( 1.0f / 100 ) * cas.y
	- (m_player->pos.z + of.y - of2.y) * ( 1.0f / 100 ) * cas.y + of.y * ratio * m_mod.y) / m_mod.y;
	
	return pos;
}

void MiniMap::drawBackground(int showLevel, Rect boundaries, Vec2f start, float zoom, float fadeBorder, Vec2f decal, bool invColor, float alpha) {
	
	m_mapVertices.clear();
	
	Vec2f cas;
	cas.x = zoom / MINIMAP_MAX_X;
	cas.y = zoom / MINIMAP_MAX_Z;
	
	GRenderer->SetTexture(0, m_levels[showLevel].m_texContainer);
	
	float div = (1.0f / 25);
	TextureContainer * tc = m_levels[showLevel].m_texContainer;
	Vec2f d;
	d.x = 1.f / tc->m_pTexture->getStoredSize().x;
	d.y = 1.f / tc->m_pTexture->getStoredSize().y;
	
	Vec2f v2;
	v2.x = 4.f * d.x * m_mod.x;
	v2.y = 4.f * d.y * m_mod.y;
	
	float fadeDiv = 0.f;
	Rect fadeBounds(0, 0, 0, 0);
	
	if(fadeBorder > 0.f) {
		fadeDiv = 1.f/fadeBorder;
		fadeBounds.left = checked_range_cast<Rect::Num>((boundaries.left + fadeBorder) * g_sizeRatio.x);
		fadeBounds.right = checked_range_cast<Rect::Num>((boundaries.right - fadeBorder) * g_sizeRatio.x);
		fadeBounds.top = checked_range_cast<Rect::Num>((boundaries.top + fadeBorder) * g_sizeRatio.y);
		fadeBounds.bottom = checked_range_cast<Rect::Num>((boundaries.bottom - fadeBorder) * g_sizeRatio.y);
	}
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	if(invColor) {
		GRenderer->SetBlendFunc(BlendOne, BlendInvSrcColor);
	} else {
		GRenderer->SetBlendFunc(BlendZero, BlendInvSrcColor);
	}
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapClamp);
	GRenderer->GetTextureStage(0)->setMinFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterLinear);

	for(int z = -2; z < int(MINIMAP_MAX_Z) + 2; z++) {
	for(int x = -2; x < int(MINIMAP_MAX_X) + 2; x++) {
		
		Vec2f v3;
		v3.x = float(x) * float(m_activeBkg->Xdiv) * m_mod.x;
		v3.y = float(z) * float(m_activeBkg->Zdiv) * m_mod.y;
		
		Vec2f v4;
		v4.x = (v3.x * div) * d.x;
		v4.y = (v3.y * div) * d.y;
		
		Vec2f pos;
		pos.x = (start.x + x * cas.x) * g_sizeRatio.x;
		pos.y = (start.y + z * cas.y) * g_sizeRatio.y;
		
		if((pos.x < boundaries.left * g_sizeRatio.x)
		   || (pos.x > boundaries.right * g_sizeRatio.x)
		   || (pos.y < boundaries.top * g_sizeRatio.y)
		   || (pos.y > boundaries.bottom * g_sizeRatio.y)) {
			continue; // out of bounds
		}
		
		TexturedVertex verts[4];
		
		verts[3].p.x = verts[0].p.x = pos.x;
		verts[1].p.y = verts[0].p.y = pos.y;
		verts[2].p.x = verts[1].p.x = pos.x + (cas.x * g_sizeRatio.x);
		verts[3].p.y = verts[2].p.y = pos.y + (cas.y * g_sizeRatio.y);
		
		verts[3].uv.x = verts[0].uv.x = v4.x;
		verts[1].uv.y = verts[0].uv.y = v4.y;
		verts[2].uv.x = verts[1].uv.x = v4.x + v2.x;
		verts[3].uv.y = verts[2].uv.y = v4.y + v2.y;
		
		float v;
		float oo = 0.f;
		
		for(int vert = 0; vert < 4; vert++) {
			verts[vert].color = Color(255, 255, 255, 255).toRGBA();
			verts[vert].rhw = 1;
			verts[vert].p.z = 0.00001f;

			// Array offset according to "vert"
			int iOffset = 0;
			int jOffset = 0;
			
			if(vert == 1 || vert == 2)
				iOffset = 1;
			if(vert == 2 || vert == 3)
				jOffset = 1;
			
			if((x + iOffset < 0) || (x + iOffset >= int(MINIMAP_MAX_X)) || (z + jOffset < 0) || (z + jOffset >= int(MINIMAP_MAX_Z))) {
				v = 0;
			} else {
				int minx = std::min(x + iOffset, int(MINIMAP_MAX_X) - iOffset);
				int minz = std::min(z + jOffset, int(MINIMAP_MAX_Z) - jOffset);
				v = float(m_levels[showLevel].m_revealed[minx][minz]) * (1.0f / 255);
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
			
			verts[vert].color = Color::gray(v * alpha).toRGB();
			
			oo += v;
		}
		
		if(oo > 0.f) {
			
			verts[0].p.x += decal.x * g_sizeRatio.x;
			verts[0].p.y += decal.y * g_sizeRatio.y;
			verts[1].p.x += decal.x * g_sizeRatio.x;
			verts[1].p.y += decal.y * g_sizeRatio.y;
			verts[2].p.x += decal.x * g_sizeRatio.x;
			verts[2].p.y += decal.y * g_sizeRatio.y;
			verts[3].p.x += decal.x * g_sizeRatio.x;
			verts[3].p.y += decal.y * g_sizeRatio.y;
			
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

void MiniMap::drawPlayer(float playerSize, Vec2f playerPos, bool alphaBlending) {
	
	TexturedVertex verts[4];
	
	for(int k = 0; k < 4; k++) {
		verts[k].color = Color(255, 0, 0, 255).toRGBA();
		verts[k].rhw = 1;
		verts[k].p.z = 0.00001f;
	}
	
	Vec2f r;
	r.x = 0.f;
	r.y = -playerSize * 1.8f;
	Vec2f r2;
	r2.x = -playerSize * (1.0f / 2);
	r2.y = playerSize;
	Vec2f r3;
	r3.x = playerSize * (1.0f / 2);
	r3.y = playerSize;
	
	float angle = glm::radians(m_player->angle.getYaw());
	float ca = std::cos(angle);
	float sa = std::sin(angle);
	
	verts[0].p.x = (playerPos.x + r2.x * ca + r2.y * sa) * g_sizeRatio.x;
	verts[0].p.y = (playerPos.y + r2.y * ca - r2.x * sa) * g_sizeRatio.y;
	verts[1].p.x = (playerPos.x + r.x * ca + r.y * sa) * g_sizeRatio.x;
	verts[1].p.y = (playerPos.y + r.y * ca - r.x * sa) * g_sizeRatio.y;
	verts[2].p.x = (playerPos.x + r3.x * ca + r3.y * sa) * g_sizeRatio.x;
	verts[2].p.y = (playerPos.y + r3.y * ca - r3.x * sa) * g_sizeRatio.y;
	
	GRenderer->ResetTexture(0);
	GRenderer->SetRenderState(Renderer::AlphaBlending, alphaBlending);
	if(alphaBlending) {
		GRenderer->SetBlendFunc(BlendOne, BlendInvSrcColor);
	}
	
	EERIEDRAWPRIM(Renderer::TriangleFan, verts);
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}

void MiniMap::drawDetectedEntities(int showLevel, Vec2f start, float zoom) {
	
	Vec2f cas;
	cas.x = zoom / MINIMAP_MAX_X;
	cas.y = zoom / MINIMAP_MAX_Z;
	
	float ratio = zoom / 250.f;
	
	if(!m_pTexDetect) {
		m_pTexDetect = TextureContainer::Load("graph/particles/flare");
	}
	
	// Computes playerpos
	const Vec2f of = m_miniOffset[m_currentLevel];
	const Vec2f of2 = m_levels[showLevel].m_ratio;
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(BlendOne, BlendOne);
	
	const EntityManager &ents = *m_entities; // for convenience
	for(size_t lnpc = 1; lnpc < ents.size(); lnpc++) {
		const EntityHandle handle = EntityHandle(lnpc);
		Entity * npc = ents[handle];
		
		if(!npc || !(npc->ioflags & IO_NPC)) {
			continue; // only NPCs can be detected
		}
		
		if(npc->_npcdata->lifePool.current < 0.f) {
			continue; // don't show dead NPCs
		}
		
		if((npc->gameFlags & GFLAG_MEGAHIDE) || npc->show != SHOW_FLAG_IN_SCENE) {
			continue; // don't show hidden NPCs
		}
		
		if(npc->_npcdata->fDetect < 0) {
			continue; // don't show undetectable NPCs
		}
		
		if(player.m_skillFull.etheralLink < npc->_npcdata->fDetect) {
			continue; // the player doesn't have enough skill to detect this NPC
		}
		
		Vec2f fp;
		
		fp.x = start.x + ((npc->pos.x - 100 + of.x - of2.x) * ( 1.0f / 100 ) * cas.x
		+ of.x * ratio * m_mod.x) / m_mod.x;
		fp.y = start.y + ((m_mapMaxY[showLevel] - of.y - of2.y) * ( 1.0f / 100 ) * cas.y
		- (npc->pos.z + 200 + of.y - of2.y) * ( 1.0f / 100 ) * cas.y + of.y * ratio * m_mod.y) / m_mod.y;
		
		float d = fdist(Vec2f(m_player->pos.x, m_player->pos.z), Vec2f(npc->pos.x, npc->pos.z));
		if(d > 800 || glm::abs(ents.player()->pos.y - npc->pos.y) > 250.f) {
			continue; // the NPC is too far away to be detected
		}
		
		float col = 1.f;
		
		if(d > 600.f) {
			col = 1.f - (d - 600.f) * ( 1.0f / 200 );
		}
		
		fp *= g_sizeRatio;
		
		Rectf rect(
			fp,
			5.f * ratio,
			5.f * ratio
		);
		EERIEDrawBitmap(rect, 0, m_pTexDetect, Color3f(col, 0, 0).to<u8>());
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

void MiniMap::mapMarkerAdd(const Vec2f & pos, int lvl, const std::string &name) {
	
	int num = mapMarkerGetID(name);
	
	if(num >= 0) {
		// Already exists, update it
		m_mapMarkers[num].m_lvl = lvl;
		m_mapMarkers[num].m_pos.x = pos.x;
		m_mapMarkers[num].m_pos.y = pos.y;
		return;
	}
	
	// Else, create one
	MapMarkerData newMMD;
	newMMD.m_lvl = lvl;
	newMMD.m_pos.x = pos.x;
	newMMD.m_pos.y = pos.y;
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
