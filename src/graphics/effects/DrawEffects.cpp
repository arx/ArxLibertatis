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
// Initial Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include "graphics/effects/DrawEffects.h"

#include "animation/AnimationRender.h"

#include "core/Application.h"
#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"

#include "game/Entity.h"
#include "game/Player.h"
#include "game/Spells.h"

#include "graphics/Draw.h"
#include "graphics/Math.h"
#include "graphics/data/Mesh.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/texture/TextureStage.h"
#include "platform/profiler/Profiler.h"

#include "scene/Light.h"
#include "scene/Interactive.h"


extern TextureContainer * Boom;

std::vector<POLYBOOM> polyboom(MAX_POLYBOOM);
std::vector<TexturedVertex> g_shadowBatch;

extern Color ulBKGColor;

static void AddToShadowBatch(TexturedVertex * _pVertex1, TexturedVertex * _pVertex2,
                             TexturedVertex * _pVertex3) {
	
	TexturedVertex pPointAdd[3];
	EE_P(_pVertex1->p, pPointAdd[0]);
	EE_P(_pVertex2->p, pPointAdd[1]);
	EE_P(_pVertex3->p, pPointAdd[2]);
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
	
	GRenderer->SetFogColor(Color::none);
	GRenderer->SetDepthBias(1);
	
	for(long i=0; i<TREATZONE_CUR; i++) {
		if(treatio[i].show != 1 || !treatio[i].io)
			continue;
		
		Entity *io = treatio[i].io;
		
		if(   !io->obj
		   || (io->ioflags & IO_JUST_COLLIDE)
		   || (io->ioflags & IO_NOSHADOW)
		   || (io->ioflags & IO_GOLD)
		   || !(io->show == SHOW_FLAG_IN_SCENE)
		) {
			continue;
		}
		
		
		EERIE_BKG_INFO * bkgData = getFastBackgroundData(io->pos.x, io->pos.z);
		if(bkgData && !bkgData->treat) { //TODO is that correct ?
			continue;
		}
		
		TexturedVertex ltv[4];
		ltv[0] = TexturedVertex(Vec3f(0, 0, 0.001f), 1.f, ColorRGBA(0), Vec2f(0.3f, 0.3f));
		ltv[1] = TexturedVertex(Vec3f(0, 0, 0.001f), 1.f, ColorRGBA(0), Vec2f(0.7f, 0.3f));
		ltv[2] = TexturedVertex(Vec3f(0, 0, 0.001f), 1.f, ColorRGBA(0), Vec2f(0.7f, 0.7f));
		ltv[3] = TexturedVertex(Vec3f(0, 0, 0.001f), 1.f, ColorRGBA(0), Vec2f(0.3f, 0.7f));
		
		if(io->obj->grouplist.size() <= 1) {
			for(size_t k = 0; k < io->obj->vertexlist.size(); k += 9) {
				EERIEPOLY *ep = CheckInPoly(io->obj->vertexlist3[k].v);
				
				if(!ep)
					continue;
				
				Vec3f in;
				in.y = ep->min.y - 3.f;
				float r = 0.5f - ((float)glm::abs(io->obj->vertexlist3[k].v.y - in.y)) * (1.f/500);
				r -= io->invisibility;
				r *= io->scale;
				
				if(r <= 0.f)
					continue;
				
				float s1 = 16.f * io->scale;
				float s2 = s1 * (1.f/2);
				in.x = io->obj->vertexlist3[k].v.x - s2;
				in.z = io->obj->vertexlist3[k].v.z - s2;
				
				r *= 255.f;
				long lv = r;
				ltv[0].color = ltv[1].color = ltv[2].color = ltv[3].color = Color(lv, lv, lv, 255).toRGBA();
				
				ltv[0].p = EE_RT(in);
				in.x += s1;
				ltv[1].p = EE_RT(in);
				in.z += s1;
				ltv[2].p = EE_RT(in);
				in.x -= s1;
				ltv[3].p = EE_RT(in);
				
				if(ltv[0].p.z > 0.f && ltv[1].p.z > 0.f && ltv[2].p.z > 0.f) {
					AddToShadowBatch(&ltv[0], &ltv[1], &ltv[2]);
					AddToShadowBatch(&ltv[0], &ltv[2], &ltv[3]);
				}
			}
		} else {
			for(size_t k = 0; k < io->obj->grouplist.size(); k++) {
				long origin = io->obj->grouplist[k].origin;
				EERIEPOLY *ep = CheckInPoly(io->obj->vertexlist3[origin].v);
				
				if(!ep)
					continue;
				
				Vec3f in;
				in.y = ep->min.y - 3.f;
				float r = 0.8f - ((float)glm::abs(io->obj->vertexlist3[origin].v.y - in.y)) * (1.f/500);
				r *= io->obj->grouplist[k].siz;
				r -= io->invisibility;
				
				if(r <= 0.f)
					continue;
				
				float s1 = io->obj->grouplist[k].siz * 44.f;
				float s2 = s1 * (1.f/2);
				in.x = io->obj->vertexlist3[origin].v.x - s2;
				in.z = io->obj->vertexlist3[origin].v.z - s2;
				
				r *= 255.f;
				long lv = r;
				ltv[0].color = ltv[1].color = ltv[2].color = ltv[3].color = Color(lv, lv, lv, 255).toRGBA();
				
				ltv[0].p = EE_RT(in);
				in.x += s1;
				ltv[1].p = EE_RT(in);
				in.z += s1;
				ltv[2].p = EE_RT(in);
				in.x -= s1;
				ltv[3].p = EE_RT(in);
				
				AddToShadowBatch(&ltv[0], &ltv[1], &ltv[2]);
				AddToShadowBatch(&ltv[0], &ltv[2], &ltv[3]);
			}
		}
	}
	
	if(g_shadowBatch.size() > 0)
	{
		GRenderer->SetRenderState(Renderer::DepthWrite, false);
		GRenderer->SetBlendFunc(BlendZero, BlendInvSrcColor);
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		GRenderer->SetTexture(0, Boom);
		
		EERIEDRAWPRIM(Renderer::TriangleList, &g_shadowBatch[0], g_shadowBatch.size());
		
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		GRenderer->SetRenderState(Renderer::DepthWrite, true);
		GRenderer->SetDepthBias(0);
		GRenderer->SetFogColor(ulBKGColor);
	}
}

// This used to add a bias when the "forceZbias" config option was activated, but it
// was off by default and we removed it.
static void IncrementPolyWithNormalOutput(EERIEPOLY * _pPoly, TexturedVertex * _pOut) {
	
	_pOut[0].p = _pPoly->v[0].p;
	_pOut[1].p = _pPoly->v[1].p;
	_pOut[2].p = _pPoly->v[2].p;
	
	if(_pPoly->type&POLY_QUAD) {
		_pOut[3].p = _pPoly->v[3].p;
	}
}

void ARXDRAW_DrawPolyBoom() {
	
	ARX_PROFILE_FUNC();
	
	TexturedVertex ltv[4];

	GRenderer->SetFogColor(Color::none); // TODO: not handled by RenderMaterial
	unsigned long now = arxtime.now_ul();
	
	for(size_t i = 0; i < polyboom.size(); i++) {
		
		POLYBOOM & pb = polyboom[i];
		
		if(pb.type & 128) {
			if(pb.timecreation - g_framedelay > 0) {
				float fCalc = pb.timecreation - g_framedelay;
				pb.timecreation = checked_range_cast<unsigned long>(fCalc);
			}

			if(pb.timecreation - g_framedelay > 0) {
				float fCalc =  pb.timecreation - g_framedelay;
				pb.timecreation = checked_range_cast<unsigned long>(fCalc);
			}
		}

		float t = (float)pb.timecreation + (float)pb.tolive - (float)now;

		if(t <= 0) {
			std::swap(polyboom[i], polyboom.back());
			polyboom.pop_back();
			i--;
		}
	}
	
	RenderMaterial mat;
	mat.setDepthTest(true);
	mat.setDepthBias(8);
	mat.setLayer(RenderMaterial::Decal);
	mat.setWrapMode(TextureStage::WrapClamp);

	std::vector<POLYBOOM>::iterator itr = polyboom.begin();
	while (itr != polyboom.end()) {

		POLYBOOM & pb = *itr;
		
		float t = (float)pb.timecreation + (float)pb.tolive - (float)now;
		
		long typp = pb.type;
		typp &= ~128;
		
		switch(typp) {
			
		case 0: { // Scorch mark
			
			float tt = t / (float)pb.tolive * 0.8f;
			
			IncrementPolyWithNormalOutput(pb.ep,ltv);
			
			for(long k = 0; k < pb.nbvert; k++) {
				ltv[k].p = EE_RT(ltv[k].p);
				ltv[k].uv.x=pb.u[k];
				ltv[k].uv.y=pb.v[k];
				ltv[k].color = (player.m_improve ? (Color3f::red * (tt*.5f)) : Color3f::gray(tt)).toRGB();
			}
			
			if(player.m_improve) {
				mat.setBlendType(RenderMaterial::Additive);
			} else {
				mat.setBlendType(RenderMaterial::Subtractive);
			}
			mat.setTexture(Boom);
			
			drawTriangle(mat, &ltv[0]);
			if(pb.nbvert & 4) {
				drawTriangle(mat, &ltv[1]);
			}
			
			break;
		}
		
		case 1: { // Blood
			
			float div = 1.f / (float)pb.tolive;
			float tt = t * div;
			float tr = std::max(1.f, tt * 2 - 0.5f);
			ColorRGBA col = (pb.rgb * tt).toRGB(glm::clamp(tt * 1.5f, 0.f, 1.f) * 255);
			
			IncrementPolyWithNormalOutput(pb.ep, ltv);
			
			for(long k = 0; k < pb.nbvert; k++) {
				ltv[k].p = EE_RT(ltv[k].p);
				ltv[k].uv.x=(pb.u[k]-0.5f)*(tr)+0.5f;
				ltv[k].uv.y=(pb.v[k]-0.5f)*(tr)+0.5f;
				ltv[k].color = col;
			}
			
			mat.setBlendType(RenderMaterial::Subtractive2);
			mat.setTexture(pb.tc);
			
			drawTriangle(mat, &ltv[0]);
			if(pb.nbvert & 4) {
				drawTriangle(mat, &ltv[1]);
			}
			
			break;
		}
		
		case 2: { // Water
			
			float div = 1.f / (float)pb.tolive;
			float tt = t * div;
			float tr = std::max(1.f, tt * 2 - 0.5f);
			float ttt = tt * 0.5f;
			ColorRGBA col = (pb.rgb * ttt).toRGB();
			
			IncrementPolyWithNormalOutput(pb.ep,ltv);
			
			for(long k = 0; k < pb.nbvert; k++) {
				ltv[k].p = EE_RT(ltv[k].p);
				ltv[k].uv.x=(pb.u[k]-0.5f)*(tr)+0.5f;
				ltv[k].uv.y=(pb.v[k]-0.5f)*(tr)+0.5f;
				ltv[k].color=col;
			}

			if (	(ltv[0].uv.x<0.f)
				&&	(ltv[1].uv.x<0.f)
				&&	(ltv[2].uv.x<0.f)
				&&	(ltv[3].uv.x<0.f) )
				break;

			if (	(ltv[0].uv.y<0.f)
				&&	(ltv[1].uv.y<0.f)
				&&	(ltv[2].uv.y<0.f)
				&&	(ltv[3].uv.y<0.f) )
				break;

			if (	(ltv[0].uv.x>1.f)
				&&	(ltv[1].uv.x>1.f)
				&&	(ltv[2].uv.x>1.f)
				&&	(ltv[3].uv.x>1.f) )
				break;

			if (	(ltv[0].uv.y>1.f)
				&&	(ltv[1].uv.y>1.f)
				&&	(ltv[2].uv.y>1.f)
				&&	(ltv[3].uv.y>1.f) )
				break;
			
			mat.setBlendType(RenderMaterial::Screen);
			mat.setTexture(pb.tc);
			
			drawTriangle(mat, &ltv[0]);
			if(pb.nbvert & 4) {
				drawTriangle(mat, &ltv[1]);
			}
			
			break;
		}
		}
		
		++itr;
	}
	
	GRenderer->SetFogColor(ulBKGColor);
}
