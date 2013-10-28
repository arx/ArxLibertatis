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
#include "game/Spells.h"

#include "graphics/Draw.h"
#include "graphics/Math.h"
#include "graphics/data/Mesh.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/texture/TextureStage.h"

#include "scene/Light.h"
#include "scene/Interactive.h"

// Some external defs needing to be cleaned...
extern Rect g_size;

extern Vec3f SPRmins;
extern Vec3f SPRmaxs;

extern TextureContainer * Boom;

std::vector<POLYBOOM> polyboom(MAX_POLYBOOM);
std::vector<TexturedVertex> g_shadowBatch;

extern Color ulBKGColor;

void EE_RT2(TexturedVertex*,TexturedVertex*);
void EE_P(Vec3f * in, TexturedVertex * out);

void AddToShadowBatch(TexturedVertex * _pVertex1, TexturedVertex * _pVertex2, TexturedVertex * _pVertex3) {

	TexturedVertex pPointAdd[3];
	EE_P(&_pVertex1->p, &pPointAdd[0]);
	EE_P(&_pVertex2->p, &pPointAdd[1]);
	EE_P(&_pVertex3->p, &pPointAdd[2]);
	pPointAdd[0].color = _pVertex1->color;
	pPointAdd[0].specular = _pVertex1->specular;
	pPointAdd[0].uv = _pVertex1->uv;
	pPointAdd[1].color = _pVertex2->color;
	pPointAdd[1].specular = _pVertex2->specular;
	pPointAdd[1].uv = _pVertex2->uv;
	pPointAdd[2].color = _pVertex3->color;
	pPointAdd[2].specular = _pVertex3->specular;
	pPointAdd[2].uv = _pVertex3->uv;

	g_shadowBatch.push_back(pPointAdd[0]);
	g_shadowBatch.push_back(pPointAdd[1]);
	g_shadowBatch.push_back(pPointAdd[2]);
}

void ARXDRAW_DrawInterShadows()
{	
	g_shadowBatch.resize(0);

	GRenderer->SetFogColor(Color::none);
	SetZBias(1);

	for(long i=0; i<TREATZONE_CUR; i++) {
		if(treatio[i].show != 1 || !treatio[i].io)
			continue;

		Entity *io = treatio[i].io;

		if(!io->obj || (io->ioflags & IO_JUST_COLLIDE))
			continue;


		FAST_BKG_DATA * bkgData = getFastBackgroundData(io->pos.x, io->pos.z);
		if(bkgData && !bkgData->treat) { //TODO is that correct ?
			continue;
		}

		if(!(io->ioflags & IO_NOSHADOW) && io->show==SHOW_FLAG_IN_SCENE && !(io->ioflags & IO_GOLD)) {
			TexturedVertex in;

			TexturedVertex ltv[4];
			ltv[0] = TexturedVertex(Vec3f(0, 0, 0.001f), 1.f, 0, 1, Vec2f(0.3f, 0.3f));
			ltv[1] = TexturedVertex(Vec3f(0, 0, 0.001f), 1.f, 0, 1, Vec2f(0.7f, 0.3f));
			ltv[2] = TexturedVertex(Vec3f(0, 0, 0.001f), 1.f, 0, 1, Vec2f(0.7f, 0.7f));
			ltv[3] = TexturedVertex(Vec3f(0, 0, 0.001f), 1.f, 0, 1, Vec2f(0.3f, 0.7f));

			if(io->obj->nbgroups <= 1) {
				for(size_t k=0; k < io->obj->vertexlist.size(); k += 9) {
					EERIEPOLY *ep = EECheckInPoly(&io->obj->vertexlist3[k].v);

					if(ep) {
						in.p.y=ep->min.y-3.f;
						float r=0.5f-((float)EEfabs(io->obj->vertexlist3[k].v.y-in.p.y))*( 1.0f / 500 );
						r-=io->invisibility;
						r*=io->scale;

						if(r<=0.f)
							continue;

						float s1=16.f*io->scale;
						float s2=s1*( 1.0f / 2 );
						in.p.x=io->obj->vertexlist3[k].v.x-s2;
						in.p.z=io->obj->vertexlist3[k].v.z-s2;

						r*=255.f;
						long lv = r;
						ltv[0].color=ltv[1].color=ltv[2].color=ltv[3].color=0xFF000000 | lv<<16 | lv<<8 | lv;

						EE_RT2(&in,&ltv[0]);
						in.p.x+=s1;
						EE_RT2(&in,&ltv[1]);
						in.p.z+=s1;
						EE_RT2(&in,&ltv[2]);
						in.p.x-=s1;
						EE_RT2(&in,&ltv[3]);

						if(ltv[0].p.z > 0.f && ltv[1].p.z > 0.f && ltv[2].p.z > 0.f) {
							AddToShadowBatch(&ltv[0], &ltv[1], &ltv[2]);
							AddToShadowBatch(&ltv[0], &ltv[2], &ltv[3]);
						}
					}
				}
			} else {
				for(long k = 0; k < io->obj->nbgroups; k++) {
					long origin=io->obj->grouplist[k].origin;
					EERIEPOLY *ep = EECheckInPoly(&io->obj->vertexlist3[origin].v);

					if(ep) {
						in.p.y=ep->min.y-3.f;
						float r=0.8f-((float)EEfabs(io->obj->vertexlist3[origin].v.y-in.p.y))*( 1.0f / 500 );
						r*=io->obj->grouplist[k].siz;
						r-=io->invisibility;

						if(r<=0.f)
							continue;

						float s1=io->obj->grouplist[k].siz*44.f;
						float s2=s1*( 1.0f / 2 );
						in.p.x=io->obj->vertexlist3[origin].v.x-s2;
						in.p.z=io->obj->vertexlist3[origin].v.z-s2;

						r*=255.f;
						long lv = r;
						ltv[0].color=ltv[1].color=ltv[2].color=ltv[3].color=0xFF000000 | lv<<16 | lv<<8 | lv;
												
						EE_RT2(&in,&ltv[0]);
						in.p.x+=s1;
						EE_RT2(&in,&ltv[1]);
						in.p.z+=s1;
						EE_RT2(&in,&ltv[2]);
						in.p.x-=s1;
						EE_RT2(&in,&ltv[3]);

						AddToShadowBatch(&ltv[0], &ltv[1], &ltv[2]);
						AddToShadowBatch(&ltv[0], &ltv[2], &ltv[3]);
					}
				}
			}
		}

	}

	if(g_shadowBatch.size() > 0)
	{
		GRenderer->SetRenderState(Renderer::DepthWrite, false);
		GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		GRenderer->SetTexture(0, Boom);

		EERIEDRAWPRIM(Renderer::TriangleList, &g_shadowBatch[0], g_shadowBatch.size());

		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		GRenderer->SetRenderState(Renderer::DepthWrite, true);
		SetZBias(0);
		GRenderer->SetFogColor(ulBKGColor);
	}	
}

extern Entity * CAMERACONTROLLER;


// This used to add a bias when the "forceZbias" config option was activated, but it
// was off by default and we removed it.
void IncrementPolyWithNormalOutput(EERIEPOLY *_pPoly,TexturedVertex *_pOut) {
	
	_pOut[0].p = _pPoly->v[0].p;
	_pOut[1].p = _pPoly->v[1].p;
	_pOut[2].p = _pPoly->v[2].p;
	
	if(_pPoly->type&POLY_QUAD) {
		_pOut[3].p = _pPoly->v[3].p;
	}
}

extern float framedelay;
void ARXDRAW_DrawPolyBoom()
{
	TexturedVertex ltv[4];

	SetZBias(8);
	GRenderer->SetFogColor(Color::none);
	unsigned long tim = (unsigned long)(arxtime);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	std::vector<POLYBOOM>::iterator pb = polyboom.begin();
	while (pb != polyboom.end()) {

		if(pb->type & 128) {
			if(pb->timecreation - framedelay > 0) {
				float fCalc = pb->timecreation - framedelay;
				pb->timecreation = checked_range_cast<unsigned long>(fCalc);
			}

			if(pb->timecreation - framedelay > 0) {
				float fCalc =  pb->timecreation - framedelay;
				pb->timecreation = checked_range_cast<unsigned long>(fCalc);
			}
		}

		float t = (float)pb->timecreation + (float)pb->tolive - (float)tim;

		if(t <= 0) {
			pb = polyboom.erase(pb);
			continue;
		}

		long typp = pb->type;
		typp &= ~128;

		switch(typp) {
		case 0:
		{
			float tt = t / (float)pb->tolive * 0.8f;

			IncrementPolyWithNormalOutput(pb->ep,ltv);
			EE_RT2(&ltv[0],&ltv[0]);
			EE_RT2(&ltv[1],&ltv[1]);
			EE_RT2(&ltv[2],&ltv[2]);

			for(long k = 0; k < pb->nbvert; k++) {
				ltv[k].uv.x=pb->u[k];
				ltv[k].uv.y=pb->v[k];
				ltv[k].color = (Project.improve ? (Color3f::red * (tt*.5f)) : Color3f::gray(tt)).toBGR();
				ltv[k].specular = Color::black.toBGR();
			}

			if(Project.improve) {
				GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
			} else {
				GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
			}

			GRenderer->SetTexture(0, Boom);
			ARX_DrawPrimitive(&ltv[0], &ltv[1], &ltv[2]);

			if(pb->nbvert & 4) {
				EE_RT2(&ltv[3],&ltv[3]);
				ARX_DrawPrimitive(&ltv[1], &ltv[2], &ltv[3]);
			}
		}
		break;
		case 1:	// Blood
		{
			float div=1.f/(float)pb->tolive;
			float tt=(float)t*div;
			float tr = tt * 2 - 0.5f;

			if(tr < 1.f)
				tr = 1.f;

			ColorBGRA col = (pb->rgb * tt).toBGR();

			for(long k = 0; k < pb->nbvert; k++) {
				ltv[k].uv.x=(pb->u[k]-0.5f)*(tr)+0.5f;
				ltv[k].uv.y=(pb->v[k]-0.5f)*(tr)+0.5f;
				ltv[k].color=col;
				ltv[k].specular=0xFF000000;
			}

			IncrementPolyWithNormalOutput(pb->ep,ltv);
			EE_RT2(&ltv[0],&ltv[0]);
			EE_RT2(&ltv[1],&ltv[1]);
			EE_RT2(&ltv[2],&ltv[2]);

			if(pb->nbvert & 4) {
				EE_RT2(&ltv[3],&ltv[3]);
			}

			GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapClamp);
			GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
			GRenderer->SetTexture(0, pb->tc);

			ARX_DrawPrimitive(&ltv[0], &ltv[1], &ltv[2]);

			if(pb->nbvert & 4) {
				ARX_DrawPrimitive(&ltv[1], &ltv[2], &ltv[3]);
			}

			ltv[0].color = ltv[1].color = ltv[2].color = ltv[3].color = Color::gray(tt).toBGR();

			GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);

			ARX_DrawPrimitive(&ltv[0], &ltv[1], &ltv[2]);

			if(pb->nbvert & 4) {
				ARX_DrawPrimitive(&ltv[1], &ltv[2], &ltv[3]);
			}

			GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
		}
		break;
		case 2: // WATER
		{
			float div=1.f/(float)pb->tolive;
			float tt=(float)t*div;
			float tr = (tt * 2 - 0.5f);

			if (tr<1.f) tr=1.f;

			float ttt=tt*0.5f;
			ColorBGRA col = (pb->rgb * ttt).toBGR();

			for(long k = 0; k < pb->nbvert; k++) {
				ltv[k].uv.x=(pb->u[k]-0.5f)*(tr)+0.5f;
				ltv[k].uv.y=(pb->v[k]-0.5f)*(tr)+0.5f;
				ltv[k].color=col;
				ltv[k].specular=0xFF000000;
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

			IncrementPolyWithNormalOutput(pb->ep,ltv);
			EE_RT2(&ltv[0],&ltv[0]);
			EE_RT2(&ltv[1],&ltv[1]);
			EE_RT2(&ltv[2],&ltv[2]);

			GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapClamp);
			GRenderer->SetBlendFunc(Renderer::BlendInvDstColor, Renderer::BlendOne);
			GRenderer->SetTexture(0, pb->tc);

			ARX_DrawPrimitive(&ltv[0], &ltv[1], &ltv[2]);

			if(pb->nbvert & 4) {
				EE_RT2(&ltv[3],&ltv[3]);
				ARX_DrawPrimitive(&ltv[1], &ltv[2], &ltv[3]);
			}

			GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
		}
		break;
		}

		++ pb;
	}

	SetZBias(0);
	GRenderer->SetFogColor(ulBKGColor);
}
