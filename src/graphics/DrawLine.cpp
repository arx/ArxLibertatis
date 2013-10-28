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

#include "graphics/DrawLine.h"

#include <cstring>

#include <glm/gtx/norm.hpp>

#include "math/Angle.h"

#include "graphics/Math.h"
#include "graphics/Vertex.h"


extern void EE_RT(const Vec3f & in, Vec3f & out);
extern void EE_P(Vec3f * in, TexturedVertex * out);
extern void EE_RTP(TexturedVertex * in, TexturedVertex * out);

void EERIEDrawCircle(float x0, float y0, float r, Color col, float z) {

	float lx = x0;
	float ly = y0 + r;
	GRenderer->ResetTexture(0);

	for(long i = 0; i < 361; i += 10) {
		float t = radians((float)i);
		float x = x0 - sin(t) * r;
		float y = y0 + cos(t) * r;
		EERIEDraw2DLine(lx, ly, x, y, z, col);
		lx = x;
		ly = y;
	}
}

void EERIEDraw2DLine(float x0, float y0, float x1, float y1, float z, Color col) {

	TexturedVertex v[2];
	v[0].p.x = x0;
	v[0].p.y = y0;
	v[0].p.z = v[1].p.z = z;
	v[1].p.x = x1;
	v[1].p.y = y1;
	v[1].color = v[0].color = col.toBGRA();
	v[1].rhw = v[0].rhw = 1.f;

	GRenderer->ResetTexture(0);
	EERIEDRAWPRIM(Renderer::LineList, v, 2);
}

void EERIEDraw2DRect(float x0, float y0, float x1, float y1, float z, Color col) {

	TexturedVertex v[5];
	v[4].p.x = v[3].p.x = v[0].p.x = x0;
	v[4].p.y = v[1].p.y = v[0].p.y = y0;
	v[2].p.x = v[1].p.x = x1;
	v[3].p.y = v[2].p.y = y1;
	v[4].p.z = v[3].p.z = v[2].p.z = v[1].p.z = v[0].p.z = z;
	v[4].color = v[3].color = v[2].color = v[1].color = v[0].color = col.toBGRA();
	v[4].rhw = v[3].rhw = v[2].rhw = v[1].rhw = v[0].rhw = 1.f;

	GRenderer->ResetTexture(0);
	EERIEDRAWPRIM(Renderer::LineStrip, v, 5);
}

void EERIEDrawFill2DRectDegrad(float x0, float y0, float x1, float y1, float z, Color cold, Color cole) {

	TexturedVertex v[4];
	v[0].p.x = v[2].p.x = x0;
	v[0].p.y = v[1].p.y = y0;
	v[1].p.x = v[3].p.x = x1;
	v[2].p.y = v[3].p.y = y1;
	v[0].color = v[1].color = cold.toBGRA();
	v[2].color = v[3].color = cole.toBGRA();
	v[0].p.z = v[1].p.z = v[2].p.z = v[3].p.z = z;
	v[3].rhw = v[2].rhw = v[1].rhw = v[0].rhw = 1.f;

	GRenderer->ResetTexture(0);
	EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);
}

void DrawLineSphere(const EERIE_SPHERE & sphere, Color color) {

	if(sphere.radius <= 0)
		return;

	static const size_t sections = 64;

	size_t rings = sphere.radius / 10;
	if(rings < 7)
		rings = 7;

	std::vector<TexturedVertex> vertices;

	bool skip = false;

	for(size_t i = 1; i < rings - 1; i++) {
		float a = i * (PI / (rings - 1));
		for(size_t j = 0; j <= sections; j++) {
			float b = j * ((2 * PI) / sections);

			Vec3f pos;
			pos.x = cos(b) * sin(a);
			pos.y = sin(b) * sin(a);
			pos.z = cos(a);

			pos *= sphere.radius;
			pos += sphere.origin;

			Vec3f temp;
			TexturedVertex out;
			EE_RT(pos, temp);
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

void EERIEDraw3DCylinder(const EERIE_CYLINDER & cyl, Color col) {

	#define STEPCYL 16
	for(long i = 0; i < 360 - STEPCYL; i += STEPCYL) {

		float es = sin(radians(MAKEANGLE((float)i))) * cyl.radius;
		float ec = cos(radians(MAKEANGLE((float)i))) * cyl.radius;
		float es2 = sin(radians(MAKEANGLE((float)(i + STEPCYL)))) * cyl.radius;
		float ec2 = cos(radians(MAKEANGLE((float)(i + STEPCYL)))) * cyl.radius;

		// Draw low pos
		EERIEDraw3DLine(cyl.origin + Vec3f(es, 0.f, ec), cyl.origin + Vec3f(es2, 0.f, ec2),  col);
		// Draw vertical
		Vec3f from = cyl.origin + Vec3f(es, 0.f, ec);
		EERIEDraw3DLine(from, from + Vec3f(0.f, cyl.height, 0.f),  col);
		// Draw high pos
		Vec3f from2 = cyl.origin + Vec3f(es, cyl.height, ec);
		Vec3f to = cyl.origin + Vec3f(es2, cyl.height, ec2);
		EERIEDraw3DLine(from2, to,  col);
	}
}

void EERIEDraw3DCylinderBase(const EERIE_CYLINDER & cyl, Color col) {

	#define STEPCYL 16
	for(long i = 0; i < 360 - STEPCYL; i += STEPCYL) {

		float es = sin(radians(MAKEANGLE((float)i))) * cyl.radius;
		float ec = cos(radians(MAKEANGLE((float)i))) * cyl.radius;
		float es2 = sin(radians(MAKEANGLE((float)(i + STEPCYL)))) * cyl.radius;
		float ec2 = cos(radians(MAKEANGLE((float)(i + STEPCYL)))) * cyl.radius;

		// Draw low pos
		EERIEDraw3DLine(cyl.origin + Vec3f(es, 0.f, ec), cyl.origin + Vec3f(es2, 0.f, ec2),  col);
	}
}

void EERIEDrawTrue3DLine(const Vec3f & orgn, const Vec3f & dest, Color col) {

	Vec3f vect = dest - orgn;
	float m = ffsqrt(glm::length2(vect));

	if(m <= 0)
		return;

	vect *= 1 / m;

	Vec3f cpos = orgn;

	while(m > 0) {
		float dep=std::min(m,30.f);
		Vec3f tpos = cpos + (vect * dep);
		EERIEDraw3DLine(cpos, tpos, col);
		cpos = tpos;
		m -= dep;
	}
}

void EERIEDraw3DLine(const Vec3f & orgn, const Vec3f & dest, Color col) {

	TexturedVertex v[2];
	TexturedVertex in;

	in.p = orgn;
	EE_RTP(&in,&v[0]);
	if(v[0].p.z < 0.f) {
		return;
	}

	in.p = dest;
	EE_RTP(&in,&v[1]);
	if(v[1].p.z<0.f) {
		return;
	}

	GRenderer->ResetTexture(0);
	v[1].color = v[0].color = col.toBGRA();

	EERIEDRAWPRIM(Renderer::LineList, v, 2);
}


void EERIEPOLY_DrawWired(EERIEPOLY * ep, Color color) {

	TexturedVertex ltv[5];
	ltv[0] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f_ZERO);
	ltv[1] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f_X_AXIS);
	ltv[2] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f(1.f, 1.f));
	ltv[3] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f_Y_AXIS);
	ltv[4] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f_Y_AXIS);

	long to = (ep->type & POLY_QUAD) ? 4 : 3;

	memcpy(ltv,ep->tv,sizeof(TexturedVertex)*to);
	ltv[0].p.z-=0.0002f;
	ltv[1].p.z-=0.0002f;
	ltv[2].p.z-=0.0002f;
	ltv[3].p.z-=0.0002f;

	if(to == 4) {
		memcpy(&ltv[2],&ep->tv[3],sizeof(TexturedVertex));
		memcpy(&ltv[3],&ep->tv[2],sizeof(TexturedVertex));
		memcpy(&ltv[4],&ep->tv[0],sizeof(TexturedVertex));
		ltv[4].p.z-=0.0002f;
	} else {
		memcpy(&ltv[to],&ltv[0],sizeof(TexturedVertex));
	}

	GRenderer->ResetTexture(0);

	ColorBGRA col = color.toBGRA();
	if(col)
		ltv[0].color=ltv[1].color=ltv[2].color=ltv[3].color=ltv[4].color=col;
	else if(to == 4)
		ltv[0].color=ltv[1].color=ltv[2].color=ltv[3].color=ltv[4].color=0xFF00FF00;
	else
		ltv[0].color=ltv[1].color=ltv[2].color=ltv[3].color=0xFFFFFF00;

	EERIEDRAWPRIM(Renderer::LineStrip, ltv, to + 1);
}

void EERIEPOLY_DrawNormals(EERIEPOLY * ep) {
	TexturedVertex ltv[5];
	ltv[0] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f_ZERO);
	ltv[1] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f_X_AXIS);
	ltv[2] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f(1.f, 1.f));
	ltv[3] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f_Y_AXIS);
	ltv[4] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f_Y_AXIS);

	TexturedVertex lv;
	long to = (ep->type & POLY_QUAD) ? 4 : 3;

	lv.p = ep->center;
	EE_RTP(&lv,&ltv[0]);
	lv.p += ep->norm * 10.f;
	EE_RTP(&lv,&ltv[1]);
	GRenderer->ResetTexture(0);
	ltv[1].color=ltv[0].color=0xFFFF0000;

	if ((ltv[1].p.z>0.f) && (ltv[0].p.z>0.f))
		EERIEDRAWPRIM(Renderer::LineList, ltv, 3);

	for(long h = 0; h < to; h++) {
		lv.p = ep->v[h].p;
		EE_RTP(&lv,&ltv[0]);
		lv.p += ep->nrml[h] * 10.f;
		EE_RTP(&lv,&ltv[1]);
		GRenderer->ResetTexture(0);
		ltv[1].color=ltv[0].color=Color::yellow.toBGR();

		if ((ltv[1].p.z>0.f) &&  (ltv[0].p.z>0.f))
			EERIEDRAWPRIM(Renderer::LineList, ltv, 3);
	}
}
