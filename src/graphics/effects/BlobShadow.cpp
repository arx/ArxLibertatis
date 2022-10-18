/*
 * Copyright 2016-2022 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/effects/BlobShadow.h"

#include <array>
#include <vector>

#include <boost/range/adaptor/strided.hpp>

#include "game/Entity.h"
#include "graphics/Draw.h"
#include "graphics/GlobalFog.h"
#include "graphics/Renderer.h"
#include "graphics/particle/ParticleTextures.h"
#include "platform/profiler/Profiler.h"
#include "scene/Interactive.h"
#include "scene/Tiles.h"

static std::vector<TexturedVertex> g_shadowBatch;

static void addShadowBlob(const Entity & entity, Vec3f pos, float scale, bool isGroup) {
	
	EERIEPOLY * ep = CheckInPoly(pos);
	if(!ep) {
		return;
	}
	
	Vec3f in;
	in.y = ep->min.y - 3.f;
	
	float strength = (isGroup ? 0.8f : 0.5f) - glm::abs(pos.y - in.y) * 0.002f;
	strength = isGroup ? (strength * scale - entity.invisibility) : (strength - entity.invisibility) * scale;
	if(strength <= 0.f) {
		return;
	}
	
	float size = (isGroup ? 44.f :  16.f) * scale;
	in.x = pos.x - size * 0.5f;
	in.z = pos.z - size * 0.5f;
	std::array<Vec3f, 4> p = { in, in + Vec3f(size, 0, 0), in + Vec3f(size, 0, size), in + Vec3f(0, 0, size) };
	if(!isGroup && (p[0].z <= 0.f || p[1].z <= 0.f || p[2].z <= 0.f)) {
		return;
	}
	
	ColorRGBA color = Color::gray(strength).toRGB();
	std::array<TexturedVertex, 4> vertices = { {
		{ worldToClipSpace(p[0]), color, Vec2f(0.3f, 0.3f) },
		{ worldToClipSpace(p[1]), color, Vec2f(0.7f, 0.3f) },
		{ worldToClipSpace(p[2]), color, Vec2f(0.7f, 0.7f) },
		{ worldToClipSpace(p[3]), color, Vec2f(0.3f, 0.7f) }
	} };
	
	g_shadowBatch.push_back(vertices[0]);
	g_shadowBatch.push_back(vertices[2]);
	g_shadowBatch.push_back(vertices[1]);
	
	g_shadowBatch.push_back(vertices[0]);
	g_shadowBatch.push_back(vertices[3]);
	g_shadowBatch.push_back(vertices[2]);
	
}

void ARXDRAW_DrawInterShadows() {
	
	ARX_PROFILE_FUNC();
	
	g_shadowBatch.clear();
	
	for(const auto & entry : treatio) {
		
		if(entry.show != SHOW_FLAG_IN_SCENE || !entry.io) {
			continue;
		}
		
		const Entity & entity = *entry.io;
		if(!entity.obj || (entity.ioflags & IO_JUST_COLLIDE) || (entity.ioflags & IO_NOSHADOW)
		   || (entity.ioflags & IO_GOLD) || entity.show != SHOW_FLAG_IN_SCENE) {
			continue;
		}
		
		if(!g_tiles->isInActiveTile(entity.pos)) {
			continue;
		}
		
		if(entity.obj->grouplist.size() > 1) {
			for(const VertexGroup & group : entity.obj->grouplist) {
				addShadowBlob(entity, entity.obj->vertexWorldPositions[group.origin].v, group.m_blobShadowSize, true);
			}
		} else {
			for(const EERIE_VERTEX & vertex : entity.obj->vertexWorldPositions | boost::adaptors::strided(9)) {
				addShadowBlob(entity, vertex.v, entity.scale, false);
			}
		}
		
	}
	
	if(!g_shadowBatch.empty()) {
		GRenderer->SetFogColor(Color());
		UseRenderState state(render3D().depthWrite(false).blend(BlendZero, BlendInvSrcColor).depthOffset(1));
		GRenderer->SetTexture(0, g_particleTextures.boom);
		EERIEDRAWPRIM(Renderer::TriangleList, g_shadowBatch.data(), g_shadowBatch.size());
		GRenderer->SetFogColor(g_fogColor);
	}
	
}
