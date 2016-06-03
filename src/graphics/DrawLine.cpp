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


void drawLine(const Vec2f & from, const Vec2f & to, float z, Color col) {
	
	TexturedVertex v[2];
	v[0].p.x = from.x;
	v[0].p.y = from.y;
	v[0].p.z = v[1].p.z = z;
	v[1].p.x = to.x;
	v[1].p.y = to.y;
	v[1].color = v[0].color = col.toRGBA();
	v[1].rhw = v[0].rhw = 1.f;

	GRenderer->ResetTexture(0);
	EERIEDRAWPRIM(Renderer::LineList, v, 2);
}

void drawLineRectangle(const Rectf & rect, float z, Color col) {

	TexturedVertex v[5];
	v[0].p = Vec3f(rect.bottomLeft(), z);
	v[1].p = Vec3f(rect.bottomRight(), z);
	v[2].p = Vec3f(rect.topRight(), z);
	v[3].p = Vec3f(rect.topLeft(), z);
	v[4].p = v[0].p;
	
	v[4].color = v[3].color = v[2].color = v[1].color = v[0].color = col.toRGBA();
	v[4].rhw = v[3].rhw = v[2].rhw = v[1].rhw = v[0].rhw = 1.f;

	GRenderer->ResetTexture(0);
	EERIEDRAWPRIM(Renderer::LineStrip, v, 5);
}

void EERIEDrawFill2DRectDegrad(Vec2f a, Vec2f b, float z, Color cold, Color cole) {

	TexturedVertex v[4];
	v[0].p.x = v[2].p.x = a.x;
	v[0].p.y = v[1].p.y = a.y;
	v[1].p.x = v[3].p.x = b.x;
	v[2].p.y = v[3].p.y = b.y;
	v[0].color = v[1].color = cold.toRGBA();
	v[2].color = v[3].color = cole.toRGBA();
	v[0].p.z = v[1].p.z = v[2].p.z = v[3].p.z = z;
	v[3].rhw = v[2].rhw = v[1].rhw = v[0].rhw = 1.f;

	GRenderer->ResetTexture(0);
	EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);
}

void drawLineSphere(const Sphere & sphere, Color color) {

	if(sphere.radius <= 0)
		return;

	static const size_t sections = 64;

	size_t rings = sphere.radius / 10;
	if(rings < 7)
		rings = 7;

	std::vector<TexturedVertex> vertices;

	bool skip = false;

	for(size_t i = 1; i < rings - 1; i++) {
		float a = i * (glm::pi<float>() / (rings - 1));
		for(size_t j = 0; j <= sections; j++) {
			float b = j * ((2 * glm::pi<float>()) / sections);

			Vec3f pos;
			pos.x = glm::cos(b) * glm::sin(a);
			pos.y = glm::sin(b) * glm::sin(a);
			pos.z = glm::cos(a);

			pos *= sphere.radius;
			pos += sphere.origin;
			
			TexturedVertex out;
			Vec3f temp = EE_RT(pos);
			EE_P(temp, out);

			if(skip) {
				skip = false;
				out.color = Color(0, 0, 0, 0).toRGBA();
				vertices.push_back(out);
			}

			out.color = color.toRGBA();
			vertices.push_back(out);

			if(j == sections) {
				skip = true;
				out.color = Color(0, 0, 0, 0).toRGBA();
				vertices.push_back(out);
			}
		}
	}

	GRenderer->ResetTexture(0);
	EERIEDRAWPRIM(Renderer::LineStrip, &vertices[0], vertices.size());
}

void drawLineCylinder(const Cylinder & cyl, Color col) {
	
	const int STEPCYL = 16;
	
	for(long i = 0; i < 360 - STEPCYL; i += STEPCYL) {
		
		Vec3f current = angleToVectorXZ(i) * cyl.radius;
		Vec3f next = angleToVectorXZ(i + STEPCYL) * cyl.radius;
		
		Vec3f from = cyl.origin + current;
		Vec3f to = cyl.origin + next;
		
		// Draw low pos
		drawLine(from, to, col);
		// Draw vertical
		drawLine(from, from + Vec3f(0.f, cyl.height, 0.f), col);
		// Draw high pos
		drawLine(from + Vec3f(0.f, cyl.height, 0.f), to + Vec3f(0.f, cyl.height, 0.f), col);
	}
}

void drawLine(const Vec3f & orgn, const Vec3f & dest, Color color1, Color color2, float zbias) {
	
	TexturedVertex v[2];
	
	EE_RTP(orgn, v[0]);
	if(v[0].p.z < 0.f) {
		return;
	}
	
	EE_RTP(dest, v[1]);
	if(v[1].p.z < 0.f) {
		return;
	}
	
	v[0].p.z -= zbias, v[1].p.z -= zbias;
	
	GRenderer->ResetTexture(0);
	v[0].color = color1.toRGBA();
	v[1].color = color2.toRGBA();
	
	EERIEDRAWPRIM(Renderer::LineList, v, 2);
}

void drawLine(const Vec3f & orgn, const Vec3f & dest, Color color, float zbias) {
	drawLine(orgn, dest, color, color, zbias);
}
