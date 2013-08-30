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

#include "graphics/DrawDebug.h"

#include "core/Core.h"
#include "graphics/Math.h"
#include "graphics/Draw.h"
#include "gui/Interface.h"

#include "ai/Paths.h"
#include "scene/Light.h"


extern void EE_RT(Vec3f * in, Vec3f * out);
extern void EE_P(Vec3f * in, TexturedVertex * out);

void DrawLineSphere(const EERIE_SPHERE & sphere, Color color) {

	if(sphere.radius <= 0)
		return;

	static const size_t sections = 64;

	int rings = sphere.radius / 10;
	rings = std::max(rings, 7);

	std::vector<TexturedVertex> vertices;

	bool skip = false;

	for(float i = 1; i < rings - 1; i++) {
		float a = i * (PI / (rings - 1));
		for(int j = 0; j <= sections; j++) {
			float b = j * ((2 * PI) / sections);

			Vec3f pos;
			pos.x = cos(b) * sin(a);
			pos.y = sin(b) * sin(a);
			pos.z = cos(a);

			pos *= sphere.radius;
			pos += sphere.origin;

			Vec3f temp;
			TexturedVertex out;
			EE_RT(&pos, &temp);
			EE_P(&temp, &out);

			if(skip) {
				skip = false;
				out.color = 0x00000000;
				vertices.push_back(out);
			}

			out.color = color.toBGRA();
			vertices.push_back(out);

			if(j == sections) {
				skip = true;
				out.color = 0x00000000;
				vertices.push_back(out);
			}
		}
	}

	GRenderer->ResetTexture(0);
	EERIEDRAWPRIM(Renderer::LineStrip, &vertices[0], vertices.size());
}

extern bool MouseInRect(const float x0, const float y0, const float x1, const float y1);
extern TextureContainer * lightsource_tc;

void EERIEDrawLight(EERIE_LIGHT * el) {

	TexturedVertex in;
	TexturedVertex center;
	//GRenderer->SetCulling(Renderer::CullNone);
	//GRenderer->SetRenderState(Renderer::DepthTest, true);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);


	if(!el || !el->treat)
		return;

	in.p = el->pos;
	EE_RTP(&in, &center);

	if(MouseInRect(center.p.x - 20, center.p.y - 20, center.p.x + 20, center.p.y + 20)) {
		GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
		EERIE_SPHERE fallstart;
		fallstart.origin = el->pos;
		fallstart.radius = el->fallstart;
		DrawLineSphere(fallstart, Color(Color3<u8>::green, 200));

		EERIE_SPHERE fallend;
		fallend.origin = el->pos;
		fallend.radius = el->fallend;
		DrawLineSphere(fallend, Color(Color3<u8>::red, 200));
	}

	GRenderer->SetBlendFunc(Renderer::BlendSrcAlpha, Renderer::BlendSrcAlpha);
	EERIEDrawSprite(&in, 11.f, lightsource_tc, el->rgb.to<u8>(), 2.f);
}

void ARXDRAW_DrawAllLights(long x0,long z0,long x1,long z1) {
	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		EERIE_LIGHT *light = GLight[i];

		if(light) {
			long tx = light->pos.x * ACTIVEBKG->Xmul;
			long tz = light->pos.z * ACTIVEBKG->Zmul;
			light->mins.x = 9999999999.f;

			if(tx >= x0 && tx <= x1 && tz >= z0 && tz <= z1)  {
				light->treat = 1;
				EERIEDrawLight(light);
			}
		}
	}
}

void DebugPortalsRender() {
	GRenderer->SetRenderState(Renderer::Fog, false);
	GRenderer->SetRenderState(Renderer::DepthTest, false);

	for(size_t i = 0; i < portals->portals.size(); i++) {
		EERIE_PORTALS & po = portals->portals[i];

		Color color = Color::red;
		if(po.useportal == 1) {
			color = Color::green;
		}

		EERIEPOLY & epp = po.poly;

		EERIEDraw3DLine(epp.v[0].p, epp.v[1].p, color);
		EERIEDraw3DLine(epp.v[1].p, epp.v[3].p, color);
		EERIEDraw3DLine(epp.v[2].p, epp.v[3].p, color);
		EERIEDraw3DLine(epp.v[0].p, epp.v[2].p, color);
	}

	GRenderer->SetRenderState(Renderer::DepthTest, true);
	GRenderer->SetRenderState(Renderer::Fog, true);
}

void DebugPathsRender() {
	GRenderer->SetRenderState(Renderer::Fog, false);
	GRenderer->SetRenderState(Renderer::DepthTest, false);

	for(long i = 0; i < nbARXpaths; i++) {
		ARX_PATH * path = ARXpaths[i];

		if(!path)
			continue;

		std::vector<Vec3f> points;

		for(long i = 0; i < path->nb_pathways; i++) {
			ARX_PATHWAY node = path->pathways[i];

			points.push_back(path->pos + node.rpos);
		}

		if(points.size() > 0) {
			points.push_back(points[0]);
		}

		for(int i=0; i+1<points.size(); i++) {
			EERIEDraw3DLine(points[i], points[i+1], Color::red);
		}

		if(path->height > 0) {
			Vec3f offset(0.f, -path->height, 0.f);

			for(int i=0; i+1<points.size(); i++) {
				EERIEDraw3DLine(points[i] + offset, points[i+1] + offset, Color::red);
			}

			for(int i=0; i<points.size(); i++) {
				EERIEDraw3DLine(points[i], points[i] + offset, Color::red);
			}
		}
	}
}

void DrawDebugRender() {

	if(EDITION == EDITION_NONE)
		return;

	if(EDITION == EDITION_LIGHTS) {
		//TODO copy-paste
		long l = ACTIVECAM->cdepth * 0.42f;
		long clip3D = (l / (long)BKG_SIZX) + 1;
		long lcval = clip3D + 4;

		long camXsnap = ACTIVECAM->orgTrans.pos.x * ACTIVEBKG->Xmul;
		long camZsnap = ACTIVECAM->orgTrans.pos.z * ACTIVEBKG->Zmul;
		camXsnap = clamp(camXsnap, 0, ACTIVEBKG->Xsize - 1L);
		camZsnap = clamp(camZsnap, 0, ACTIVEBKG->Zsize - 1L);

		long x0 = std::max(camXsnap - lcval, 0L);
		long x1 = std::min(camXsnap + lcval, ACTIVEBKG->Xsize - 1L);
		long z0 = std::max(camZsnap - lcval, 0L);
		long z1 = std::min(camZsnap + lcval, ACTIVEBKG->Zsize - 1L);

		ARXDRAW_DrawAllLights(x0,z0,x1,z1);
	}

	if(EDITION == EDITION_Portals) {
		DebugPortalsRender();
	}

	if(EDITION == EDITION_Paths) {
		DebugPathsRender();
	}
}
