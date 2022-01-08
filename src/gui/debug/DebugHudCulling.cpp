/*
 * Copyright 2022 Arx Libertatis Team (see the AUTHORS file)
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

#include "gui/debug/DebugHudCulling.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>

#include "cinematic/CinematicController.h"

#include "core/Core.h"

#include "game/Entity.h"
#include "game/EntityManager.h"

#include "graphics/Draw.h"
#include "graphics/DrawLine.h"
#include "graphics/GlobalFog.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "graphics/font/Font.h"

#include "gui/Menu.h"

#include "scene/Background.h"
#include "scene/Scene.h"

#include "gui/Text.h"

void debugHud_Culling() {
	
	if(ARXmenu.mode() != Mode_InGame || isInCinematic()) {
		return;
	}
	
	arx_assert(g_tiles);
	
	UseRenderState state(RenderState().blend());
	
	hFontDebug->draw(Vec2i(10, 10), "Entity visibility", Color::white);
	
	// Scale the region of used tiles to fit the screen
	Vec2s min(std::numeric_limits<s16>::max());
	Vec2s max(std::numeric_limits<s16>::min());
	for(auto tile : g_tiles->tiles()) {
		if(!tile.intersectingPolygons().empty()) {
			min = glm::min(min, Vec2s(tile));
			max = glm::max(max, Vec2s(tile));
		}
	}
	if(min == Vec2s(std::numeric_limits<s16>::max()) || max == Vec2s(std::numeric_limits<s16>::min())) {
		min = Vec2s(0);
		max = Vec2s(MAX_BKGX, MAX_BKGZ);
	}
	Vec2s size = max - min + Vec2s(3);
	int bottomMargin = hFontDebug->getLineHeight();
	float scale = std::floor(std::min(float(g_size.width()) / float(size.x),
	                                  (float(g_size.height() - bottomMargin)) / float(size.y)));
	Vec2f offset(std::floor(g_size.center().x - scale * size.x / 2.f - (min.x - 1) * scale),
	             std::floor(g_size.center().y - scale * size.y / 2.f + (max.y + 2) * scale - float(bottomMargin)));
	
	// Draw active and/or non-empty tiles
	for(auto tile : g_tiles->tiles()) {
		if(!tile.active() && tile.intersectingPolygons().empty()) {
			continue;
		}
		Vec2f pos = offset + Vec2f(tile.x, -tile.y) * scale;
		if(!tile.intersectingPolygons().empty()) {
			drawLineRectangle(Rectf(pos, scale, -scale), 0.01f, Color::black);
		}
		if(tile.active() || !tile.intersectingPolygons().empty()) {
			Color color = tile.active() ? Color::gray(0.25f, 0.75f) : Color(Color::black, 127);
			EERIEDrawBitmap(Rectf(pos + Vec2f(1.f, 0.f), scale - 1.f, 1.f - scale), 0.01f, nullptr, color);
		}
	}
	
	glm::mat3x2 worldToDebug(0.f);
	worldToDebug[0][0] = g_tiles->m_mul.x * scale;
	worldToDebug[2][1] = -g_tiles->m_mul.y * scale;
	
	// Draw all portals
	if(portals) {
		for(size_t i = 0; i < portals->portals.size(); i++) {
			EERIE_PORTALS & po = portals->portals[i];
			Color color = Color::red;
			if(po.useportal == 1) {
				color = Color::green;
			}
			PortalPoly & epp = po.poly;
			Vec2f pos[4];
			for(int j = 0; j < 4; j++) {
				pos[j] = offset + worldToDebug * epp.p[j];
			}
			drawLine(pos[0], pos[1], 0.01f, color);
			drawLine(pos[1], pos[3], 0.01f, color);
			drawLine(pos[3], pos[2], 0.01f, color);
			drawLine(pos[2], pos[0], 0.01f, color);
		}
	}
	
	// Draw the camera frustum (minus near plane)
	{
		Vec3f frustum[5] = {
			screenToWorldSpace(Vec2f(g_size.topLeft()), g_camera->cdepth * fZFogEnd),
			screenToWorldSpace(Vec2f(g_size.topRight()), g_camera->cdepth * fZFogEnd),
			screenToWorldSpace(Vec2f(g_size.bottomRight()), g_camera->cdepth * fZFogEnd),
			screenToWorldSpace(Vec2f(g_size.bottomLeft()), g_camera->cdepth * fZFogEnd),
			g_camera->m_pos
		};
		Vec2f p[5];
		for(size_t i = 0; i < 5; i++) {
			p[i] = offset + worldToDebug * frustum[i];
		}
		drawLine(p[0], p[1], 0.01f, Color::blue);
		drawLine(p[1], p[2], 0.01f, Color::blue);
		drawLine(p[2], p[3], 0.01f, Color::blue);
		drawLine(p[3], p[0], 0.01f, Color::blue);
		drawLine(p[0], p[4], 0.01f, Color::blue);
		drawLine(p[1], p[4], 0.01f, Color::blue);
		drawLine(p[2], p[4], 0.01f, Color::blue);
		drawLine(p[3], p[4], 0.01f, Color::blue);
	}
	
	const Color colors[] = {
		Color::gray(0.75f),
		Color::red,
		Color::magenta,
		Color::yellow,
		Color::green,
		Color::cyan
	};
	size_t sums[] = { 0, 0, 0, 0, 0, 0 };
	std::string_view labels[] = {
		"Inactive",
		"View",
		"Occluded",
		"Unknown",
		"Visible",
		"Focus"
	};
	
	// Draw entity bounding boxes or positions colored based on visibility
	for(Entity & entity : entities) {
		
		switch(entity.show) {
			case SHOW_FLAG_NOT_DRAWN:    continue;
			case SHOW_FLAG_IN_SCENE:     break;
			case SHOW_FLAG_LINKED:       break;
			case SHOW_FLAG_IN_INVENTORY: continue;
			case SHOW_FLAG_HIDDEN:       continue;
			case SHOW_FLAG_TELEPORTING:  continue;
			case SHOW_FLAG_KILLED:       continue;
			case SHOW_FLAG_MEGAHIDE:     continue;
			case SHOW_FLAG_ON_PLAYER:    break;
			case SHOW_FLAG_DESTROYED:    continue;
		}
		
		if(!entity.obj) {
			continue;
		}
		
		size_t vis = size_t(getEntityVisibility(entity)) - size_t(EntityInactive);
		arx_assert(vis < std::size(colors) && vis < std::size(sums));
		sums[vis]++;
		
		Color color = colors[vis];
		if(&entity == g_cameraEntity) {
			color = Color::white;
		}
		
		if(!vis || !entity.obj || (entity.ioflags & (IO_CAMERA | IO_MARKER))) {
			// Inactive entities do not have a valid bounding box.
			// Further, linked entities, do not have a valid position.
			Vec2f pos = glm::floor(offset + worldToDebug * entity.pos);
			if(entity.show == SHOW_FLAG_LINKED || entity.show == SHOW_FLAG_ON_PLAYER) {
				for(Entity & other : entities.inScene(~(IO_CAMERA | IO_MARKER))) {
					if(other != entity) {
						bool found = false;
						for(const EERIE_LINKED & link : other.obj->linked) {
							if(link.io == &entity) {
								found = true;
								pos = glm::floor(offset + worldToDebug * other.pos);
								break;
							}
						}
						if(found) {
							break;
						}
					}
				}
			}
			float d = glm::floor(0.15f * scale);
			drawLine(pos + Vec2f(-d, -d), pos + Vec2f(d, d + 1), 0.01f, color);
			drawLine(pos + Vec2f(-d, d), pos + Vec2f(d, -d - 1), 0.01f, color);
		} else {
			Vec2f omin = offset + worldToDebug * entity.bbox3D.min;
			Vec2f omax = offset + worldToDebug * entity.bbox3D.max;
			drawLineRectangle(Rectf(omin, omax), 0.01f, color);
		}
		
	}
	
	// Draw statistics
	int advance = hFontDebug->getMaxAdvance() * 18;
	Vec2i textpos = Vec2i(g_size.bottomCenter()) - Vec2i(advance * std::size(labels) / 2, bottomMargin * 2);
	for(size_t i = 0; i < std::size(labels); i++) {
		arx_assert(i < std::size(colors) && i < std::size(sums));
		std::ostringstream oss;
		oss << labels[i] << ": " << sums[i];
		hFontDebug->draw(textpos + Vec2i(1), oss.str(), Color::black);
		hFontDebug->draw(textpos, oss.str(), colors[i]);
		textpos.x += advance;
	}
	
}
