/*
 * Copyright 2016-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include <vector>

#include "game/Entity.h"
#include "graphics/Draw.h"
#include "graphics/GlobalFog.h"
#include "graphics/Renderer.h"
#include "platform/profiler/Profiler.h"
#include "scene/Interactive.h"

extern TextureContainer * Boom;

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

void ARXDRAW_DrawInterShadows() {
	
	ARX_PROFILE_FUNC();
	
	g_shadowBatch.clear();
	
	for(size_t i = 0; i < treatio.size(); i++) {
		
		if(treatio[i].show != SHOW_FLAG_IN_SCENE || !treatio[i].io) {
			continue;
		}
		
		Entity * io = treatio[i].io;
		if(!io->obj || (io->ioflags & IO_JUST_COLLIDE) || (io->ioflags & IO_NOSHADOW)
		   || (io->ioflags & IO_GOLD) || io->show != SHOW_FLAG_IN_SCENE) {
			continue;
		}
		
		if(!ACTIVEBKG->isInActiveTile(io->pos)) {
			continue;
		}
		
		TexturedVertexUntransformed ltv[4];
		ltv[0] = TexturedVertexUntransformed(Vec3f(0, 0, 0.001f), ColorRGBA(0), Vec2f(0.3f, 0.3f));
		ltv[1] = TexturedVertexUntransformed(Vec3f(0, 0, 0.001f), ColorRGBA(0), Vec2f(0.7f, 0.3f));
		ltv[2] = TexturedVertexUntransformed(Vec3f(0, 0, 0.001f), ColorRGBA(0), Vec2f(0.7f, 0.7f));
		ltv[3] = TexturedVertexUntransformed(Vec3f(0, 0, 0.001f), ColorRGBA(0), Vec2f(0.3f, 0.7f));
		
		if(io->obj->grouplist.size() <= 1) {
			for(size_t k = 0; k < io->obj->vertexlist.size(); k += 9) {
				
				EERIEPOLY * ep = CheckInPoly(io->obj->vertexWorldPositions[k].v);
				if(!ep) {
					continue;
				}
				
				Vec3f in;
				in.y = ep->min.y - 3.f;
				float r = 0.5f - glm::abs(io->obj->vertexWorldPositions[k].v.y - in.y) * 0.002f;
				r -= io->invisibility;
				r *= io->scale;
				if(r <= 0.f) {
					continue;
				}
				
				float s1 = 16.f * io->scale;
				float s2 = s1 * 0.5f;
				in.x = io->obj->vertexWorldPositions[k].v.x - s2;
				in.z = io->obj->vertexWorldPositions[k].v.z - s2;
				
				ColorRGBA rgba = Color::gray(r).toRGB();
				ltv[0].color = ltv[1].color = ltv[2].color = ltv[3].color = rgba;
				
				ltv[0].p = in;
				ltv[1].p = in + Vec3f(s1, 0, 0);
				ltv[2].p = in + Vec3f(s1, 0, s1);
				ltv[3].p = in + Vec3f(0, 0, s1);
				
				if(ltv[0].p.z > 0.f && ltv[1].p.z > 0.f && ltv[2].p.z > 0.f) {
					AddToShadowBatch(&ltv[0], &ltv[2], &ltv[1]);
					AddToShadowBatch(&ltv[0], &ltv[3], &ltv[2]);
				}
				
			}
		} else {
			for(size_t k = 0; k < io->obj->grouplist.size(); k++) {
				
				size_t origin = io->obj->grouplist[k].origin;
				Vec3f pos = io->obj->vertexWorldPositions[origin].v;
				
				EERIEPOLY * ep = CheckInPoly(pos);
				if(!ep) {
					continue;
				}
				
				Vec3f in;
				in.y = ep->min.y - 3.f;
				float r = 0.8f - glm::abs(pos.y - in.y) * 0.002f;
				r *= io->obj->grouplist[k].siz;
				r -= io->invisibility;
				if(r <= 0.f) {
					continue;
				}
				
				float s1 = io->obj->grouplist[k].siz * 44.f;
				float s2 = s1 * 0.5f;
				in.x = pos.x - s2;
				in.z = pos.z - s2;
				
				ColorRGBA rgba = Color::gray(r).toRGB();
				ltv[0].color = ltv[1].color = ltv[2].color = ltv[3].color = rgba;
				
				ltv[0].p = in;
				ltv[1].p = in + Vec3f(s1, 0, 0);
				ltv[2].p = in + Vec3f(s1, 0, s1);
				ltv[3].p = in + Vec3f(0, 0, s1);
				
				AddToShadowBatch(&ltv[0], &ltv[2], &ltv[1]);
				AddToShadowBatch(&ltv[0], &ltv[3], &ltv[2]);
				
			}
		}
	}
	
	if(!g_shadowBatch.empty()) {
		GRenderer->SetFogColor(Color::none);
		UseRenderState state(render3D().depthWrite(false).blend(BlendZero, BlendInvSrcColor).depthOffset(1));
		GRenderer->SetTexture(0, Boom);
		EERIEDRAWPRIM(Renderer::TriangleList, &g_shadowBatch[0], g_shadowBatch.size());
		GRenderer->SetFogColor(g_fogColor);
	}
	
}
