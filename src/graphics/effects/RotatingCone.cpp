/*
 * Copyright 2015 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/effects/RotatingCone.h"

#include "animation/AnimationRender.h"
#include "core/GameTime.h"
#include "graphics/RenderBatcher.h"
#include "math/Random.h"

RotatingCone::RotatingCone()
	: m_def(16)
	, ulCurrentTime(0)
	, m_currdurationang(0)
	, m_ang(0.f)
	, m_coneScale(0.f)
	, m_tsouffle(NULL)
{
	conenbvertex = m_def * 2 + 2;
	conenbfaces = m_def * 2 + 2;
	coned3d = (TexturedVertexUntransformed *)malloc(conenbvertex * sizeof(TexturedVertexUntransformed));
	conevertex = (Vec3f *)malloc(conenbvertex * sizeof(Vec3f));
	coneind = (unsigned short *)malloc(conenbvertex * sizeof(unsigned short));
}

RotatingCone::~RotatingCone() {
	
	free(coned3d);
	free(conevertex);
	free(coneind);
}

void RotatingCone::Init(float rbase, float rhaut, float hauteur) {
	
	Vec3f * vertex = conevertex;
	unsigned short * pind = coneind;
	unsigned short ind = 0;
	float a = 0.f;
	float da = 360.f / (float)m_def;
	
	int nb = conenbvertex >> 1;
	
	while(nb) {
		*pind++ = ind++;
		*pind++ = ind++;
		*vertex++ = Vec3f(rhaut * std::cos(glm::radians(a)), -hauteur, rhaut * std::sin(glm::radians(a)));
		*vertex++ = Vec3f(rbase * std::cos(glm::radians(a)), 0.f, rbase * std::sin(glm::radians(a)));
		a += da;
		nb--;
	}
	
	m_tsouffle = TextureContainer::Load("graph/obj3d/textures/(fx)_sebsouffle");
}

void RotatingCone::Update(float timeDelta, Vec3f pos, float coneScale) {
	
	m_currdurationang += timeDelta;
	ulCurrentTime += timeDelta;
	
	m_pos = pos;
	m_coneScale = coneScale;
	
	m_ang = (float)m_currdurationang / 1000.f;
	
	if(m_ang > 1.f) {
		m_currdurationang = 0;
		m_ang = 1.f;
	}
}

void RotatingCone::Render() {
	
	float u = m_ang;
	float du = .99999999f / (float)m_def;
	
	Vec3f * vertex = conevertex;
	TexturedVertexUntransformed * d3dv = coned3d;
	int nb = (conenbvertex) >> 1;
	
	while(nb) {
		Vec3f d3dvs;
		d3dvs.x = m_pos.x + (vertex + 1)->x + ((vertex->x - (vertex + 1)->x) * m_coneScale);
		d3dvs.y = m_pos.y + (vertex + 1)->y + ((vertex->y - (vertex + 1)->y) * m_coneScale);
		d3dvs.z = m_pos.z + (vertex + 1)->z + ((vertex->z - (vertex + 1)->z) * m_coneScale);
		
		d3dv->p = d3dvs;
		int col = Random::get(0, 80);
		
		if(!arxtime.is_paused())
			d3dv->color = Color::grayb(col).toRGB(col);
		
		d3dv->uv.x = u;
		d3dv->uv.y = 0.f;
		vertex++;
		d3dv++;
		
		d3dvs.x = m_pos.x + vertex->x;
		d3dvs.y = m_pos.y;
		d3dvs.z = m_pos.z + vertex->z;
		
		d3dv->p = d3dvs;
		col = Random::get(0, 80);
		
		if(!arxtime.is_paused())
			d3dv->color = Color::black.toRGB(col);
		
		d3dv->uv.x = u;
		d3dv->uv.y = 1.f;
		vertex++;
		d3dv++;
		
		u += du;
		nb--;
	}
	
	//tracé du cone back
	
	RenderMaterial mat;
	mat.setDepthTest(true);
	mat.setBlendType(RenderMaterial::Additive);
	mat.setWrapMode(TextureStage::WrapMirror);
	mat.setTexture(m_tsouffle);
	mat.setCulling(CullCW);

	int i = conenbfaces - 2;
	int j = 0;

	while(i--) {
		drawTriangle(mat, &coned3d[j]);
		j++;
	}

	//tracé du cone front
	mat.setCulling(CullCCW);

	i = conenbfaces - 2;
	j = 0;

	while(i--) {
		drawTriangle(mat, &coned3d[j]);
		j++;
	}
	
}
