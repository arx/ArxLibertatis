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

#include "graphics/Draw.h"

#include "core/Core.h"
#include "core/Application.h"
#include "core/GameTime.h"

#include "graphics/VertexBuffer.h"
#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/data/Mesh.h"

using std::min;
using std::max;


extern TextureContainer * enviro;

CircularVertexBuffer<TexturedVertex> * pDynamicVertexBuffer_TLVERTEX;

void EERIEDRAWPRIM(Renderer::Primitive primitive, const TexturedVertex * vertices, size_t count, bool nocount) {
	
	if(!nocount) {
		EERIEDrawnPolys++;
	}
	
	pDynamicVertexBuffer_TLVERTEX->draw(primitive, vertices, count);
}

static const float BASICFOCAL = 350.f;

void SetTextureDrawPrim(TextureContainer* tex, TexturedVertex* v, Renderer::Primitive prim) {
	GRenderer->SetTexture(0, tex);
	EERIEDRAWPRIM(prim, v, 4);
}

bool EERIECreateSprite(TexturedQuad& sprite, const Vec3f & in, float siz, Color color, float Zpos, float rot = 0) {

	TexturedVertex out;
	EE_RTP(in, &out);
	out.rhw *= 3000.f;

	if ((out.p.z>0.f) && (out.p.z<1000.f)
		&& (out.p.x>-1000) && (out.p.x<2500.f)
		&& (out.p.y>-500) && (out.p.y<1800.f))
	{
		float use_focal=BASICFOCAL*g_sizeRatio.x;
		float t;

		if(siz < 0) {
			t = -siz;
		} else {
			t = siz * ((out.rhw-1.f)*use_focal*0.001f);

			if(t <= 0.f)
				t = 0.00000001f;
		}
		
		if(Zpos <= 1.f) {
			out.p.z = Zpos;
			out.rhw = 1.f - out.p.z;
		} else {
			out.rhw *= (1.f/3000.f);
		}		
		
		ColorBGRA col = color.toBGRA();
		
		sprite.v[0] = TexturedVertex(Vec3f(), out.rhw, col, Vec2f_ZERO);
		sprite.v[1] = TexturedVertex(Vec3f(), out.rhw, col, Vec2f_X_AXIS);
		sprite.v[2] = TexturedVertex(Vec3f(), out.rhw, col, Vec2f(1.f, 1.f));
		sprite.v[3] = TexturedVertex(Vec3f(), out.rhw, col, Vec2f_Y_AXIS);
		
		if(rot == 0) {
			Vec3f maxs = out.p + t;
			Vec3f mins = out.p - t;

			sprite.v[0].p = Vec3f(mins.x, mins.y, out.p.z);
			sprite.v[1].p = Vec3f(maxs.x, mins.y, out.p.z);
			sprite.v[2].p = Vec3f(maxs.x, maxs.y, out.p.z);
			sprite.v[3].p = Vec3f(mins.x, maxs.y, out.p.z);
		} else {
			for(long i=0;i<4;i++) {
				float tt = radians(MAKEANGLE(rot+90.f*i+45+90));
				sprite.v[i].p.x = std::sin(tt) * t + out.p.x;
				sprite.v[i].p.y = std::cos(tt) * t + out.p.y;
				sprite.v[i].p.z = out.p.z;
			}
		}

		return true;
	}

	return false;
}

void EERIEAddSprite(const RenderMaterial & mat, const Vec3f & in, float siz, Color color, float Zpos, float rot) {
	TexturedQuad s;

	if(EERIECreateSprite(s, in, siz, color, Zpos, rot)) {
		RenderBatcher::getInstance().add(mat, s);
	}
}

void EERIEDrawSprite(const Vec3f & in, float siz, TextureContainer * tex, Color color, float Zpos) {
	
	TexturedQuad s;

	if(EERIECreateSprite(s, in, siz, color, Zpos)) {
		SetTextureDrawPrim(tex, s.v, Renderer::TriangleFan);
	}
}

//! Match pixel and texel origins.
void MatchPixTex(float& x, float& y) {
	x -= .5f, y -= .5f;
}

void CreateBitmap(TexturedQuad& s, Rectf rect, float z, TextureContainer * tex, Color color, bool isRhw) {
	
	rect.move(-.5f, -.5f);
	
	Vec2f uv = (tex) ? tex->uv : Vec2f_ZERO;
	ColorBGRA col = color.toBGRA();
	float val = 1.f;

	if(isRhw) {
		val -= z;
	}
	
	s.v[0] = TexturedVertex(Vec3f(rect.topLeft(), z), val, col, Vec2f(0.f, 0.f));
	s.v[1] = TexturedVertex(Vec3f(rect.topRight(), z), val, col, Vec2f(uv.x, 0.f));
	s.v[2] = TexturedVertex(Vec3f(rect.bottomRight(), z), val, col, Vec2f(uv.x, uv.y));
	s.v[3] = TexturedVertex(Vec3f(rect.bottomLeft(), z), val, col, Vec2f(0.f, uv.y));
}

void DrawBitmap(const Rectf & rect, float z, TextureContainer * tex, Color color, bool isRhw) {
	TexturedQuad s;
	CreateBitmap(s, rect, z, tex, color, isRhw);
	
	GRenderer->SetTexture(0, tex);
	if(isRhw) {
		if(tex && tex->hasColorKey()) {
			GRenderer->SetAlphaFunc(Renderer::CmpGreater, .5f);
			EERIEDRAWPRIM(Renderer::TriangleFan, s.v, 4);
			GRenderer->SetAlphaFunc(Renderer::CmpNotEqual, 0.f);
			return;
		}
	}
	EERIEDRAWPRIM(Renderer::TriangleFan, s.v, 4);
}

void EERIEAddBitmap(const RenderMaterial & mat, float x, float y, float sx, float sy, float z, TextureContainer * tex, Color color) {
	TexturedQuad s;
	CreateBitmap(s, Rectf(Vec2f(x, y), sx, sy), z, tex, color, false);
	RenderBatcher::getInstance().add(mat, s);
}

void EERIEDrawBitmap(const Rectf & rect, float z, TextureContainer * tex, Color color) {
	DrawBitmap(rect, z, tex, color, false);
}

void EERIEDrawBitmap2(const Rectf & rect, float z, TextureContainer * tex, Color color) {
	DrawBitmap(rect, z, tex, color, true);
}

void EERIEDrawBitmap_uv(Rectf rect, float z, TextureContainer * tex,
                        Color color, float u0, float v0, float u1, float v1) {
	
	rect.move(-.5f, -.5f);
	
	Vec2f uv = (tex) ? tex->uv : Vec2f_ONE;
	u0 *= uv.x, u1 *= uv.x, v0 *= uv.y, v1 *= uv.y;

	ColorBGRA col = color.toBGRA();
	TexturedVertex v[4];
	v[0] = TexturedVertex(Vec3f(rect.topLeft(),     z), 1.f, col, Vec2f(u0, v0));
	v[1] = TexturedVertex(Vec3f(rect.topRight(),    z), 1.f, col, Vec2f(u1, v0));
	v[2] = TexturedVertex(Vec3f(rect.bottomRight(), z), 1.f, col, Vec2f(u1, v1));
	v[3] = TexturedVertex(Vec3f(rect.bottomLeft(),  z), 1.f, col, Vec2f(u0, v1));
	SetTextureDrawPrim(tex, v, Renderer::TriangleFan);
}

void EERIEDrawBitmapUVs(float x, float y, float sx, float sy, float z, TextureContainer * tex,
                        Color color, float u0, float v0, float u1, float v1, float u2, float v2,
	                      float u3, float v3) {
	
	MatchPixTex(x, y);
	
	ColorBGRA col = color.toBGRA();
	TexturedVertex v[4];
	v[0] = TexturedVertex(Vec3f(x,      y,      z), 1.f, col, Vec2f(u0, v0));
	v[1] = TexturedVertex(Vec3f(x + sx, y,      z), 1.f, col, Vec2f(u1, v1));
	v[2] = TexturedVertex(Vec3f(x,      y + sy, z), 1.f, col, Vec2f(u2, v2));
	v[3] = TexturedVertex(Vec3f(x + sx, y + sy, z), 1.f, col, Vec2f(u3, v3));
	SetTextureDrawPrim(tex, v, Renderer::TriangleStrip);
}

void EERIEDrawBitmap2DecalY(float x, float y, float sx, float sy, float z, TextureContainer * tex,
                            Color color, float _fDeltaY) {
	
	MatchPixTex(x, y);	
	Vec2f uv = (tex) ? tex->uv : Vec2f_ZERO;
	float sv = uv.y * _fDeltaY;	
	ColorBGRA col = color.toBGRA();
	TexturedVertex v[4];
	float fDy = _fDeltaY * sy;	

	Vec2f uv1(0.f, sv);
	Vec2f uv2(uv.x, sv);
	Vec2f uv3(uv.x, uv.y);
	Vec2f uv4(0.f, uv.y);

	if(sx < 0) {
		sx *= -1;
		uv1.x = uv.x;
		uv2.x = 0.f;
		uv3.x = 0.f;
		uv4.x = uv.x;
	}	

	v[0] = TexturedVertex(Vec3f(x,      y + fDy, z), 1.f, col, uv1);
	v[1] = TexturedVertex(Vec3f(x + sx, y + fDy, z), 1.f, col, uv2);
	v[2] = TexturedVertex(Vec3f(x + sx, y + sy,  z), 1.f, col, uv3);
	v[3] = TexturedVertex(Vec3f(x,      y + sy,  z), 1.f, col, uv4);
	SetTextureDrawPrim(tex, v, Renderer::TriangleFan);
}
