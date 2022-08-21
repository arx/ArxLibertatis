/*
 * Copyright 2011-2022 Arx Libertatis Team (see the AUTHORS file)
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
#include <limits>
#include <sstream>
#include <utility>

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
#include "scene/Tiles.h"

#include "util/Range.h"

MiniMap g_miniMap; // TODO: remove this

static constexpr Vec2f g_mapMod(float(MAX_BKGX) / MINIMAP_MAX_X, float(MAX_BKGZ) / MINIMAP_MAX_Z);
static constexpr Vec2f g_worldToMapScale = 0.01f / g_mapMod / Vec2f(MINIMAP_MAX_X, MINIMAP_MAX_Z);

void MiniMap::getData(size_t showLevel) {
	
	if(m_levels[showLevel].m_texContainer == nullptr) {
		
		res::path levelMap = "graph/levels/level" + std::to_string(showLevel) + "/map";
		m_levels[showLevel].m_texContainer = TextureContainer::Load(levelMap, TextureContainer::NoColorKey);
		
		if(m_levels[showLevel].m_texContainer) { // 4 pix/meter
			m_levels[showLevel].m_size = Vec2f(m_levels[showLevel].m_texContainer->m_size);
		}
		
	}
	
}

void MiniMap::validatePos() {
	
	int showLevel = ARX_LEVELS_GetRealNum(m_currentLevel);
	
	if((showLevel >= 0) && (showLevel < int(MAX_MINIMAP_LEVELS))) {
		
		if(m_levels[showLevel].m_texContainer == nullptr) {
			getData(showLevel);
		}
		
		if(m_levels[m_currentLevel].m_texContainer == nullptr) {
			getData(m_currentLevel);
		}
		
		if(m_levels[showLevel].m_texContainer) {
			revealPlayerPos(ARX_LEVELS_GetRealNum(m_currentLevel));
		}
	}
}

void MiniMap::validatePlayerPos(int currentLevel, bool blockPlayerControls, ARX_INTERFACE_BOOK_MODE bookMode) {
	
	if(m_currentLevel != currentLevel) {
		
		m_currentLevel = currentLevel;
		
		if(m_currentLevel >= 0 && size_t(m_currentLevel) < m_miniOffset.size()) {
			float minX = std::numeric_limits<float>::max();
			float maxZ = std::numeric_limits<float>::min();
			for(auto tile : m_activeBkg->tiles()) {
				for(const EERIEPOLY & ep : tile.polygons()) {
					minX = std::min(minX, ep.min.x);
					maxZ = std::max(maxZ, ep.max.z);
				}
			}
			m_worldToMapOffset = Vec2f(-minX, maxZ) * g_worldToMapScale +
			                     m_miniOffset[m_currentLevel] * (1.f / 250.f + Vec2f(1.f, -2.f) * g_worldToMapScale);
		} else {
			m_worldToMapOffset = Vec2f(0.f);
		}
		
	}
	
	if(!blockPlayerControls) {
		
		float req;
		
		if((m_player->Interface & INTER_PLAYERBOOK) && (!(m_player->Interface & INTER_COMBATMODE)) && (bookMode == BOOKMODE_MINIMAP)) {
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

void MiniMap::loadOffsets(PakReader * pakRes) {
	
	std::string iniMiniOffsets = "graph/levels/mini_offsets.ini";
	
	PakFile * file = pakRes->getFile(iniMiniOffsets.c_str());
	if(!file) {
		LogError << "Missing " << iniMiniOffsets;
		return;
	}
	
	std::istringstream iss(file->read());
	
	std::string dummy;
	
	for(int i = 0; i < 29; i++) { // Why 29?
		iss >> dummy >> m_miniOffset[i].x >> m_miniOffset[i].y;
		if(iss.fail()) {
			LogError << "Error parsing line " << i << " of mini_offsets.ini";
		}
		iss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	
	m_miniOffset[0] = Vec2f(0, -0.5);
	m_miniOffset[1] = Vec2f(0, 0);
	m_miniOffset[14] = Vec2f(130, 0);
	m_miniOffset[15] = Vec2f(31, -3.5);
	
}

void MiniMap::reveal() {
	
	for(MiniMapData & level : m_levels) {
		for(Vec2i tile : util::grid(Vec2i(0), Vec2i(MINIMAP_MAX_X, MINIMAP_MAX_Z))) {
			level.m_revealed[tile.x][tile.y] = 255;
		}
	}
	
}

void MiniMap::firstInit(ARXCHARACTER * pl, PakReader * pakRes, EntityManager * entityMng) {
	
	m_pTexDetect = nullptr;
	m_mapMarkerTexCont = nullptr;
	
	m_player = pl;
	m_playerLastPosX = -999999.f;
	m_playerLastPosZ = -999999.f;
	
	m_currentLevel = 0;
	m_entities = entityMng;
	m_activeBkg = nullptr;
	
	m_mapVertices.reserve(MINIMAP_MAX_X * MINIMAP_MAX_Z);
	
	resetLevels();
	
	m_miniOffset.fill(Vec2f(0.f));
	
	loadOffsets(pakRes);
}

void MiniMap::resetLevels() {
	
	for(MiniMapData & level : m_levels) {
		level.m_texContainer = nullptr;
		level.m_size = Vec2f(0.f);
		// Sets the whole array to 0
		memset(level.m_revealed, 0, sizeof(level.m_revealed));
	}
	
}

void MiniMap::reset() {
	
	purgeTexContainer();
	resetLevels();
}

void MiniMap::purgeTexContainer() {
	
	for(MiniMapData & level : m_levels) {
		delete level.m_texContainer;
		level.m_texContainer = nullptr;
	}
	
}

void MiniMap::showPlayerMiniMap(size_t showLevel) {
	
	UseRenderState state(render2D());
	
	ARX_PROFILE_FUNC();
	
	float scale = minSizeRatio();
	const float miniMapZoom = 300.f * scale; // zoom of the minimap
	const Vec2f minimapSizeOrig = Vec2f(200.0f, 160.0f);
	const Vec2f minimapSizeScaled = minimapSizeOrig * scale;
	const Rect miniMapRect(Vec2i(g_size.right - s32(minimapSizeScaled.x), s32(-10 * scale)),
	                       s32(minimapSizeScaled.x),
	                       s32(minimapSizeScaled.y));
	const float playerSize = 4.f * scale; // red arrow size
	
	// First Load Minimap TC & DATA if needed
	if(m_levels[showLevel].m_texContainer == nullptr) {
		getData(showLevel);
	}
	
	if(m_levels[showLevel].m_texContainer) {
		
		Vec2f start(0.f);
		Vec2f playerPos(0.f);
		if(showLevel == size_t(ARX_LEVELS_GetRealNum(m_currentLevel))) {
			start = Vec2f(miniMapRect.center()) - worldToMapPos(m_player->pos, miniMapZoom);
			playerPos = Vec2f(miniMapRect.center());
		}
		
		// Draw the background
		drawBackground(showLevel, miniMapRect, start, miniMapZoom, 20.f, true, 0.5f);
		
		// Draw the player (red arrow)
		if(showLevel == size_t(ARX_LEVELS_GetRealNum(m_currentLevel))) {
			drawPlayer(playerSize, playerPos, true);
			drawDetectedEntities(start, miniMapZoom);
		}
		
	}
}

void MiniMap::showBookMiniMap(size_t showLevel, Rect rect, float scale) {
	
	UseRenderState state(render2D());
	
	// First Load Minimap TC & DATA if needed
	if(m_levels[showLevel].m_texContainer == nullptr) {
		getData(showLevel);
	}
	
	if(m_levels[showLevel].m_texContainer) {
		
		float zoom = 900.f * scale;
		
		Vec2f start(0.f);
		Vec2f playerPos(0.f);
		if(showLevel == size_t(ARX_LEVELS_GetRealNum(m_currentLevel))) {
			start = Vec2f(rect.center()) - worldToMapPos(m_player->pos, zoom);
			playerPos = Vec2f(rect.center());
		}
		
		drawBackground(showLevel, rect, start, zoom, 20.f * scale);
		
		if(showLevel == size_t(ARX_LEVELS_GetRealNum(m_currentLevel))) {
			drawPlayer(6.f * scale, playerPos, false);
			drawDetectedEntities(start, zoom);
		}
		
	}
}

void MiniMap::showBookEntireMap(size_t showLevel, Rect rect, float scale) {
	
	UseRenderState state(render2D());
	
	// First Load Minimap TC & DATA if needed
	if(m_levels[showLevel].m_texContainer == nullptr) {
		getData(showLevel);
	}
	
	if(!m_levels[showLevel].m_texContainer) {
		return;
	}
	
	float zoom = 250.f * scale;
	
	Vec2f start(rect.topLeft());
	
	Vec2f playerPos(0.f, 0.f);
	
	if(showLevel == size_t(ARX_LEVELS_GetRealNum(m_currentLevel))) {
		playerPos = worldToMapPos(m_player->pos, zoom);
		playerPos += start;
	}
	
	drawBackground(showLevel, rect, start, zoom);
	
	if(showLevel == size_t(ARX_LEVELS_GetRealNum(m_currentLevel))) {
		drawPlayer(3.f * scale, playerPos, false);
		drawDetectedEntities(start, zoom);
	}
	
	std::array<TexturedVertex, 4> verts;
	for(TexturedVertex & vertex : verts) {
		vertex.color = Color::white.toRGB();
		vertex.w = 1;
		vertex.p.z = 0.00001f;
	}
	
	Vec2f casePos(zoom / MINIMAP_MAX_X, zoom / MINIMAP_MAX_Z);
	
	for(MapMarkerData & marker : m_mapMarkers) {
		
		if(marker.m_lvl != showLevel + 1) {
			continue;
		}
		
		Vec2f pos = marker.m_pos * 8.f * m_activeBkg->m_mul * casePos + start;
		float size = 5.f * scale;
		verts[0].color = Color::red.toRGB();
		verts[1].color = Color::red.toRGB();
		verts[2].color = Color::red.toRGB();
		verts[3].color = Color::red.toRGB();
		verts[0].p.x = (pos.x - size);
		verts[0].p.y = (pos.y - size);
		verts[1].p.x = (pos.x + size);
		verts[1].p.y = (pos.y - size);
		verts[2].p.x = (pos.x + size);
		verts[2].p.y = (pos.y + size);
		verts[3].p.x = (pos.x - size);
		verts[3].p.y = (pos.y + size);
		verts[0].uv = Vec2f(0.f);
		verts[1].uv = Vec2f(1.f, 0.f);
		verts[2].uv = Vec2f(1.f);
		verts[3].uv = Vec2f(0.f, 1.f);
		
		const Rect mouseTestRect(
			s32(verts[0].p.x),
			s32(verts[0].p.y),
			s32(verts[2].p.x),
			s32(verts[2].p.y)
		);
		
		if(mouseTestRect.contains(Vec2i(DANAEMouse))) {
			if(!marker.m_text.empty()) {
				
				Rect bRect(Vec2i(rect.left, rect.bottom), s32(205 * scale), s32(63 * scale));
				
				long lLengthDraw = ARX_UNICODE_ForceFormattingInRect(hFontInGameNote, marker.m_text, bRect);
				
				ARX_UNICODE_DrawTextInRect(hFontInGameNote,
				                           Vec2f(bRect.topLeft()),
				                           float(bRect.right),
				                           marker.m_text.substr(0, lLengthDraw),
				                           Color::none);
				
			}
		}
		
		if(m_mapMarkerTexCont == nullptr) {
			m_mapMarkerTexCont = TextureContainer::Load("graph/interface/icons/mapmarker");
		}
		
		GRenderer->SetTexture(0, m_mapMarkerTexCont);
		
		EERIEDRAWPRIM(Renderer::TriangleFan, verts.data(), 4);
	}
}

void MiniMap::revealPlayerPos(size_t showLevel) {
	
	float zoom = 250.f;
	float maxDistance = 6.0f;
	Vec2f start(140.f, 120.f);
	Vec2f cas(zoom / MINIMAP_MAX_X, zoom / MINIMAP_MAX_Z);
	
	Vec2f playerPos = worldToMapPos(m_player->pos, zoom);
	Vec2i playerCell = Vec2i(playerPos.x / cas.x, playerPos.y / cas.y);
	
	if(   playerCell.x < 0
	   || playerCell.x >= s32(MINIMAP_MAX_X)
	   || playerCell.y < 0
	   || playerCell.y >= s32(MINIMAP_MAX_Z)
	   ) {
		return;
	}
	
	playerPos += start;
	
	Vec2i startCell = playerCell - Vec2i(s32(glm::ceil(maxDistance / cas.x)));
	Vec2i endCell = playerCell + Vec2i(s32(glm::ceil(maxDistance / cas.y)));

	Vec2i maxCell = Vec2i(MINIMAP_MAX_X - 1, MINIMAP_MAX_Z - 1);
	
	startCell = glm::clamp(startCell, Vec2i(0), maxCell);
	endCell = glm::clamp(endCell, Vec2i(0), maxCell);
	
	for(Vec2i tile : util::grid(startCell, endCell + Vec2i(1))) {
		
		Vec2f pos = start + Vec2f(tile) * cas;
		
		float d = fdist(Vec2f(pos.x + cas.x * 0.5f, pos.y), playerPos);
		if(d >= maxDistance) {
			continue;
		}
		
		float revealPercent = (maxDistance - d) * (1.f / maxDistance);
		revealPercent = std::clamp(revealPercent * 2.0f, 0.0f, 1.0f);
		
		int r = int(revealPercent * 255.f);
		
		int ucLevel = std::max(r, int(m_levels[showLevel].m_revealed[tile.x][tile.y]));
		m_levels[showLevel].m_revealed[tile.x][tile.y] = checked_range_cast<unsigned char>(ucLevel);
		
	}
	
}

Vec2f MiniMap::worldToMapPos(Vec3f pos, float zoom) {
	return (Vec2f(pos.x, -pos.z) * g_worldToMapScale + m_worldToMapOffset) * zoom;
}

void MiniMap::drawBackground(size_t showLevel, Rect boundaries, Vec2f start, float zoom, float fadeBorder, bool invColor, float alpha) {
	
	m_mapVertices.clear();
	
	Vec2f cas(zoom / MINIMAP_MAX_X, zoom / MINIMAP_MAX_Z);
	
	GRenderer->SetTexture(0, m_levels[showLevel].m_texContainer);
	
	float div = (1.0f / 25);
	TextureContainer * tc = m_levels[showLevel].m_texContainer;
	Vec2f d(1.f / float(tc->m_pTexture->getStoredSize().x), 1.f / float(tc->m_pTexture->getStoredSize().y));
	
	Vec2f v2 = 4.f * d * g_mapMod;
	
	float fadeDiv = 0.f;
	Rect fadeBounds = boundaries;
	if(fadeBorder > 0.f) {
		fadeDiv = 1.f / fadeBorder;
		fadeBounds.left += s32(fadeBorder);
		fadeBounds.right -= s32(fadeBorder);
		fadeBounds.top += s32(fadeBorder);
		fadeBounds.bottom -= s32(fadeBorder);
	}
	
	RenderState desiredState = render2D();
	if(invColor) {
		desiredState.setBlend(BlendOne, BlendInvSrcColor);
	} else {
		desiredState.setBlend(BlendZero, BlendInvSrcColor);
	}
	UseRenderState state(desiredState);
	UseTextureState textureState(TextureStage::FilterLinear, TextureStage::WrapClamp);
	
	for(Vec2i tile : util::grid(Vec2i(-2), Vec2i(MINIMAP_MAX_X, MINIMAP_MAX_X) + Vec2i(2))) {
		
		Vec2f v3 = Vec2f(tile) * g_backgroundTileSize * g_mapMod;
		
		Vec2f v4 = v3 * div * d;
		
		Vec2f pos = start + Vec2f(tile) * cas;
		
		if((pos.x < boundaries.left)
		   || (pos.x > boundaries.right)
		   || (pos.y < boundaries.top)
		   || (pos.y > boundaries.bottom)) {
			continue; // out of bounds
		}
		
		std::array<TexturedVertex, 4> verts;
		
		verts[3].p.x = verts[0].p.x = pos.x;
		verts[1].p.y = verts[0].p.y = pos.y;
		verts[2].p.x = verts[1].p.x = pos.x + cas.x;
		verts[3].p.y = verts[2].p.y = pos.y + cas.y;
		
		verts[3].uv.x = verts[0].uv.x = v4.x;
		verts[1].uv.y = verts[0].uv.y = v4.y;
		verts[2].uv.x = verts[1].uv.x = v4.x + v2.x;
		verts[3].uv.y = verts[2].uv.y = v4.y + v2.y;
		
		float v;
		float oo = 0.f;
		
		for(size_t vert = 0; vert < 4; vert++) {
			verts[vert].color = Color::white.toRGB();
			verts[vert].w = 1;
			verts[vert].p.z = 0.00001f;

			// Array offset according to "vert"
			int iOffset = 0;
			int jOffset = 0;
			
			if(vert == 1 || vert == 2)
				iOffset = 1;
			if(vert == 2 || vert == 3)
				jOffset = 1;
			
			if(tile.x + iOffset < 0 || tile.x + iOffset >= int(MINIMAP_MAX_X)
			   || tile.y + jOffset < 0 || tile.y + jOffset >= int(MINIMAP_MAX_Z)) {
				v = 0;
			} else {
				int minx = std::min(int(tile.x) + iOffset, int(MINIMAP_MAX_X) - iOffset);
				int minz = std::min(int(tile.y) + jOffset, int(MINIMAP_MAX_Z) - jOffset);
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
			m_mapVertices.push_back(verts[0]);
			m_mapVertices.push_back(verts[1]);
			m_mapVertices.push_back(verts[2]);
			m_mapVertices.push_back(verts[0]);
			m_mapVertices.push_back(verts[2]);
			m_mapVertices.push_back(verts[3]);
		}
		
	}
	
	if(!m_mapVertices.empty()) {
		EERIEDRAWPRIM(Renderer::TriangleList, m_mapVertices.data(), m_mapVertices.size());
	}
	
}

void MiniMap::drawPlayer(float playerSize, Vec2f playerPos, bool alphaBlending) {
	
	GRenderer->SetAntialiasing(true);
	
	std::array<TexturedVertex, 4> verts;
	for(TexturedVertex & vertex : verts) {
		vertex.color = Color::red.toRGB();
		vertex.w = 1;
		vertex.p.z = 0.00001f;
	}
	
	Vec2f r1(0.f, -playerSize * 1.8f);
	Vec2f r2(-playerSize * 0.5f, playerSize);
	Vec2f r3(playerSize * 0.5f, playerSize);
	
	float angle = glm::radians(m_player->angle.getYaw());
	float ca = std::cos(angle);
	float sa = std::sin(angle);
	
	verts[0].p.x = (playerPos.x + r2.x * ca + r2.y * sa);
	verts[0].p.y = (playerPos.y + r2.y * ca - r2.x * sa);
	verts[1].p.x = (playerPos.x + r1.x * ca + r1.y * sa);
	verts[1].p.y = (playerPos.y + r1.y * ca - r1.x * sa);
	verts[2].p.x = (playerPos.x + r3.x * ca + r3.y * sa);
	verts[2].p.y = (playerPos.y + r3.y * ca - r3.x * sa);
	
	GRenderer->ResetTexture(0);
	
	UseRenderState state(alphaBlending ? render2D().blend(BlendOne, BlendInvSrcColor) : render2D());
	
	EERIEDRAWPRIM(Renderer::TriangleFan, verts.data());
	
	GRenderer->SetAntialiasing(false);
	
}

void MiniMap::drawDetectedEntities(Vec2f start, float zoom) {
	
	if(!m_pTexDetect) {
		m_pTexDetect = TextureContainer::Load("graph/particles/flare");
	}
	
	UseRenderState state(render2D().blendAdditive());
	
	const EntityManager & ents = *m_entities;
	for(Entity & npc : entities.inScene(IO_NPC)) {
		
		if(npc == *entities.player()) {
			continue; // only NPCs can be detected
		}
		
		if(npc._npcdata->lifePool.current < 0.f) {
			continue; // don't show dead NPCs
		}
		
		if((npc.gameFlags & GFLAG_MEGAHIDE)) {
			continue; // don't show hidden NPCs
		}
		
		if(npc._npcdata->fDetect < 0) {
			continue; // don't show undetectable NPCs
		}
		
		if(player.m_skillFull.etheralLink < npc._npcdata->fDetect) {
			continue; // the player doesn't have enough skill to detect this NPC
		}
		
		Vec2f fp = start + worldToMapPos(npc.pos + Vec3f(-100.f, 0.f, 200.f), zoom);
		
		float d = fdist(Vec2f(m_player->pos.x, m_player->pos.z), Vec2f(npc.pos.x, npc.pos.z));
		if(d > 800 || glm::abs(ents.player()->pos.y - npc.pos.y) > 250.f) {
			continue; // the NPC is too far away to be detected
		}
		
		float col = 1.f;
		
		if(d > 600.f) {
			col = 1.f - (d - 600.f) * (1.0f / 200);
		}
		
		Rectf rect(
			fp,
			5.f * zoom / 250.f,
			5.f * zoom / 250.f
		);
		EERIEDrawBitmap(rect, 0, m_pTexDetect, Color::red * col);
	}
	
}

void MiniMap::clearMarkerTexCont() {
	m_mapMarkerTexCont = nullptr;
}

void MiniMap::load(const SavedMiniMap * saved, size_t size) {
	std::copy(saved, saved + size, m_levels.begin());
}

void MiniMap::save(SavedMiniMap * toSave, size_t size) {
	std::copy(m_levels.begin(), m_levels.begin() + size, toSave);
}

void MiniMap::mapMarkerInit(size_t reserveSize) {
	m_mapMarkers.clear();
	
	if(reserveSize > 0)
		m_mapMarkers.reserve(reserveSize);
}

int MiniMap::mapMarkerGetID(std::string_view name) {
	
	for(size_t i = 0; i < m_mapMarkers.size(); i++) {
		if(m_mapMarkers[i].m_name == name) {
			return i;
		}
	}
	
	return -1;
}

void MiniMap::mapMarkerAdd(const Vec2f & pos, size_t lvl, std::string && name) {
	
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
	newMMD.m_name = std::move(name);
	newMMD.m_text = getLocalised(newMMD.m_name);
	m_mapMarkers.push_back(newMMD);
}

void MiniMap::mapMarkerRemove(std::string_view name) {
	
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

void MiniMap::setActiveBackground(TileData * activeBkg) {
	m_activeBkg = activeBkg;
}
