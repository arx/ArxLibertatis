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

#include "game/Entity.h"
#include "graphics/Draw.h"
#include "graphics/GlobalFog.h"
#include "graphics/Renderer.h"
#include "graphics/particle/ParticleTextures.h"
#include "platform/profiler/Profiler.h"
#include "scene/Interactive.h"
#include "scene/Tiles.h"

static std::vector<TexturedVertex> g_shadowBatch;

static void AddToShadowBatch(TexturedVertexUntransformed * _pVertex1, TexturedVertexUntransformed * _pVertex2,
                             TexturedVertexUntransformed * _pVertex3) {
	
	TexturedVertex pPointAdd[3];
	worldToClipSpace(_pVertex1->p, pPointAdd[0]);
	worldToClipSpace(_pVertex2->p, pPointAdd[1]);
	worldToClipSpace(_pVertex3->p, pPointAdd[2]);
	pPointAdd[0].color = _pVertex1->color;
	pPointAdd[0].uv = _pVertex1->uv;
	pPointAdd[1].color = _pVertex2->color;
	pPointAdd[1].uv = _pVertex2->uv;
	pPointAdd[2].color = _pVertex3->color;
	pPointAdd[2].uv = _pVertex3->uv;

	g_shadowBatch.push_back(pPointAdd[0]);
	g_shadowBatch.push_back(pPointAdd[1]);
	g_shadowBatch.push_back(pPointAdd[2]);
}

static void addShadowBlob(const Entity & entity, size_t vertex, float scale, bool isGroup) {
	
	Vec3f pos = entity.obj->vertexWorldPositions[vertex].v;
	
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
	
	ColorRGBA color = Color::gray(strength).toRGB();
	
	std::array<TexturedVertexUntransformed, 4> ltv = { {
		{ in,                        color, Vec2f(0.3f, 0.3f) },
		{ in + Vec3f(size, 0, 0),    color, Vec2f(0.7f, 0.3f) },
		{ in + Vec3f(size, 0, size), color, Vec2f(0.7f, 0.7f) },
		{ in + Vec3f(0,    0, size), color, Vec2f(0.3f, 0.7f) }
	} };
	
	if(isGroup || (ltv[0].p.z > 0.f && ltv[1].p.z > 0.f && ltv[2].p.z > 0.f)) {
		AddToShadowBatch(&ltv[0], &ltv[2], &ltv[1]);
		AddToShadowBatch(&ltv[0], &ltv[3], &ltv[2]);
	}
	
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
		
		if(entity.obj->grouplist.size() <= 1) {
			for(size_t k = 0; k < entity.obj->vertexlist.size(); k += 9) {
				addShadowBlob(entity, k, entity.scale, false);
			}
		} else {
			for(const VertexGroup & group : entity.obj->grouplist) {
				addShadowBlob(entity, group.origin, group.m_blobShadowSize, true);
			}
		}
		
	}
	
	if(!g_shadowBatch.empty()) {
		GRenderer->SetFogColor(Color::none);
		UseRenderState state(render3D().depthWrite(false).blend(BlendZero, BlendInvSrcColor).depthOffset(1));
		GRenderer->SetTexture(0, g_particleTextures.boom);
		EERIEDRAWPRIM(Renderer::TriangleList, g_shadowBatch.data(), g_shadowBatch.size());
		GRenderer->SetFogColor(g_fogColor);
	}
	
}
