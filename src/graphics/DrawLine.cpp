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
#include "graphics/data/Mesh.h"

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

void DrawLineSphere(const Sphere & sphere, Color color) {

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

void EERIEDraw3DLine(const Vec3f & orgn, const Vec3f & dest, Color color1, Color color2, float zbias) {
	
	TexturedVertex v[2];
	TexturedVertex in;
	
	in.p = orgn;
	EE_RTP(&in, &v[0]);
	if(v[0].p.z < 0.f) {
		return;
	}
	
	in.p = dest;
	EE_RTP(&in,&v[1]);
	if(v[1].p.z < 0.f) {
		return;
	}
	
	v[0].p.z -= zbias, v[1].p.z -= zbias;
	
	GRenderer->ResetTexture(0);
	v[0].color = color1.toBGRA();
	v[1].color = color2.toBGRA();
	
	EERIEDRAWPRIM(Renderer::LineList, v, 2);
}

void EERIEDraw3DLine(const Vec3f & orgn, const Vec3f & dest, Color color, float zbias) {
	EERIEDraw3DLine(orgn, dest, color, color, zbias);
}
