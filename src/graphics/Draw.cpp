/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/Draw.h"

#include "core/Core.h"
#include "core/Application.h"
#include "core/GameTime.h"

#include "graphics/VertexBuffer.h"
#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/data/Mesh.h"

CircularVertexBuffer<TexturedVertex> * pDynamicVertexBuffer_TLVERTEX;

void EERIEDRAWPRIM(Renderer::Primitive primitive, const TexturedVertex * vertices, size_t count, bool nocount) {
	
	if(!nocount) {
		EERIEDrawnPolys++;
	}
	
	pDynamicVertexBuffer_TLVERTEX->draw(primitive, vertices, count);
}

static const float BASICFOCAL = 350.f;

static void SetTextureDrawPrim(TextureContainer * tex, const TexturedVertex * v,
                               Renderer::Primitive prim) {
	GRenderer->SetTexture(0, tex);
	EERIEDRAWPRIM(prim, v, 4);
}

static bool EERIECreateSprite(TexturedQuad & sprite, const Vec3f & in, float siz,
                              Color color, float Zpos, float rot = 0) {
	
	Vec4f out = worldToClipSpace(in);
	if(out.w <= 0.f) {
		return false;
	}
	Vec3f p = Vec3f(out) / out.w;
	
	if(   p.z > 0.f
	   && p.z < 1000.f
	   && p.x > -1000.f
	   && p.x < 2500.f
	   && p.y > -500.f
	   && p.y < 1800.f
	) {
		float t;

		if(siz < 0) {
			t = -siz * g_sizeRatio.y;
		} else {
			t = siz * (1.f / out.w * 3000.f - 1.f) * BASICFOCAL * g_sizeRatio.y * 0.001f;

			if(t <= 0.f)
				t = 0.00000001f;
		}
		
		if(Zpos <= 1.f) {
			p.z = Zpos;
			out.w = 1.f / (1.f - Zpos);
		}
		
		ColorRGBA col = color.toRGBA();
		
		sprite.v[0] = TexturedVertex(Vec3f(0.f), out.w, col, Vec2f(0.f));
		sprite.v[1] = TexturedVertex(Vec3f(0.f), out.w, col, Vec2f(1.f, 0.f));
		sprite.v[2] = TexturedVertex(Vec3f(0.f), out.w, col, Vec2f(1.f, 1.f));
		sprite.v[3] = TexturedVertex(Vec3f(0.f), out.w, col, Vec2f(0.f, 1.f));
		
		if(rot == 0) {
			Vec3f maxs = p + t;
			Vec3f mins = p - t;

			sprite.v[0].p = Vec3f(mins.x, mins.y, p.z) * out.w;
			sprite.v[1].p = Vec3f(maxs.x, mins.y, p.z) * out.w;
			sprite.v[2].p = Vec3f(maxs.x, maxs.y, p.z) * out.w;
			sprite.v[3].p = Vec3f(mins.x, maxs.y, p.z) * out.w;
		} else {
			for(long i = 0; i < 4; i++) {
				float tt = glm::radians(MAKEANGLE(rot + 90.f * i + 135.f));
				sprite.v[i].p.x = std::sin(tt) * t + p.x;
				sprite.v[i].p.y = std::cos(tt) * t + p.y;
				sprite.v[i].p.z = p.z;
				sprite.v[i].p *= out.w;
			}
		}

		return true;
	}

	return false;
}

void EERIEAddSprite(const RenderMaterial & mat, const Vec3f & in, float siz, Color color, float Zpos, float rot) {
	TexturedQuad s;

	if(EERIECreateSprite(s, in, siz, color, Zpos, rot)) {
		g_renderBatcher.add(mat, s);
	}
}

void EERIEDrawSprite(const Vec3f & in, float siz, TextureContainer * tex, Color color, float Zpos) {
	
	TexturedQuad s;

	if(EERIECreateSprite(s, in, siz, color, Zpos)) {
		SetTextureDrawPrim(tex, s.v, Renderer::TriangleFan);
	}
}

static void CreateBitmap(TexturedQuad & s, Rectf rect, float z, TextureContainer * tex,
                         Color color) {
	
	rect.move(-.5f, -.5f);
	
	Vec2f uv = (tex) ? tex->uv : Vec2f(0.f);
	ColorRGBA col = color.toRGBA();
	
	s.v[0] = TexturedVertex(Vec3f(rect.topLeft(), z), 1.f, col, Vec2f(0.f, 0.f));
	s.v[1] = TexturedVertex(Vec3f(rect.topRight(), z), 1.f, col, Vec2f(uv.x, 0.f));
	s.v[2] = TexturedVertex(Vec3f(rect.bottomRight(), z), 1.f, col, Vec2f(uv.x, uv.y));
	s.v[3] = TexturedVertex(Vec3f(rect.bottomLeft(), z), 1.f, col, Vec2f(0.f, uv.y));
}

void EERIEAddBitmap(const RenderMaterial & mat, const Vec3f & p, float sx, float sy, TextureContainer * tex, Color color) {
	TexturedQuad s;
	CreateBitmap(s, Rectf(Vec2f(p.x, p.y), sx, sy), p.z, tex, color);
	g_renderBatcher.add(mat, s);
}

void EERIEDrawBitmap(const Rectf & rect, float z, TextureContainer * tex, Color color) {
	TexturedQuad s;
	CreateBitmap(s, rect, z, tex, color);
	GRenderer->SetTexture(0, tex);
	EERIEDRAWPRIM(Renderer::TriangleFan, s.v, 4);
}

void EERIEDrawBitmap_uv(Rectf rect, float z, TextureContainer * tex,
                        Color color, float u0, float v0, float u1, float v1) {
	
	rect.move(-.5f, -.5f);
	
	Vec2f uv = (tex) ? tex->uv : Vec2f(1.f);
	u0 *= uv.x, u1 *= uv.x, v0 *= uv.y, v1 *= uv.y;

	ColorRGBA col = color.toRGBA();
	TexturedVertex v[4];
	v[0] = TexturedVertex(Vec3f(rect.topLeft(),     z), 1.f, col, Vec2f(u0, v0));
	v[1] = TexturedVertex(Vec3f(rect.topRight(),    z), 1.f, col, Vec2f(u1, v0));
	v[2] = TexturedVertex(Vec3f(rect.bottomRight(), z), 1.f, col, Vec2f(u1, v1));
	v[3] = TexturedVertex(Vec3f(rect.bottomLeft(),  z), 1.f, col, Vec2f(u0, v1));
	SetTextureDrawPrim(tex, v, Renderer::TriangleFan);
}

void EERIEDrawBitmapUVs(Rectf rect, float z, TextureContainer * tex,
                        Color color, Vec2f uv0, Vec2f uv1, Vec2f uv2, Vec2f uv3) {
	
	rect.move(-.5f, -.5f);
	
	ColorRGBA col = color.toRGBA();
	TexturedVertex v[4];
	v[0] = TexturedVertex(Vec3f(rect.topLeft(),     z), 1.f, col, uv0);
	v[1] = TexturedVertex(Vec3f(rect.topRight(),    z), 1.f, col, uv1);
	v[2] = TexturedVertex(Vec3f(rect.bottomLeft(),  z), 1.f, col, uv2);
	v[3] = TexturedVertex(Vec3f(rect.bottomRight(), z), 1.f, col, uv3);
	SetTextureDrawPrim(tex, v, Renderer::TriangleStrip);
}

void EERIEDrawBitmap2DecalY(Rectf rect, float z, TextureContainer * tex, Color color, float _fDeltaY) {
	
	rect.move(-.5f, -.5f);
	
	rect.top = rect.top + _fDeltaY * rect.height();
	
	Vec2f uv = (tex) ? tex->uv : Vec2f(0.f);
	float sv = uv.y * _fDeltaY;
	ColorRGBA col = color.toRGBA();
	
	Vec2f uv1(0.f, sv);
	Vec2f uv2(uv.x, sv);
	Vec2f uv3(uv.x, uv.y);
	Vec2f uv4(0.f, uv.y);
	
	TexturedVertex v[4];
	v[0] = TexturedVertex(Vec3f(rect.topLeft(),     z), 1.f, col, uv1);
	v[1] = TexturedVertex(Vec3f(rect.topRight(),    z), 1.f, col, uv2);
	v[2] = TexturedVertex(Vec3f(rect.bottomRight(), z), 1.f, col, uv3);
	v[3] = TexturedVertex(Vec3f(rect.bottomLeft(),  z), 1.f, col, uv4);
	SetTextureDrawPrim(tex, v, Renderer::TriangleFan);
}
