/*
 * Copyright 2015-2018 Arx Libertatis Team (see the AUTHORS file)
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
	: m_pos(0.f)
	, m_currdurationang(0)
	, m_ang(0.f)
	, m_coneScale(0.f)
	, m_tsouffle(NULL)
{ }

void RotatingCone::Init(float rbase, float rhaut, float hauteur) {
	
	Vec3f * vertex = conevertex;
	unsigned short * pind = coneind;
	unsigned short ind = 0;
	float a = 0.f;
	float da = 360.f / float(Def);
	
	int nb = VertexCount / 2;
	
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

void RotatingCone::Update(GameDuration timeDelta, Vec3f pos, float coneScale) {
	
	m_currdurationang += timeDelta;
	
	m_pos = pos;
	m_coneScale = coneScale;
	
	m_ang = m_currdurationang / GameDurationMs(1000);
	
	if(m_ang > 1.f) {
		m_currdurationang = 0;
		m_ang = 1.f;
	}
}

void RotatingCone::Render() {
	
	float u = m_ang;
	float du = .99999999f / float(Def);
	
	Vec3f * vertex = conevertex;
	TexturedVertexUntransformed * d3dv = coned3d;
	int nb = VertexCount / 2;
	
	while(nb) {
		
		d3dv->p = m_pos + *(vertex + 1) + ((*vertex - *(vertex + 1)) * m_coneScale);
		
		// TODO per-frame randomness
		int col = Random::get(0, 80);
		if(!g_gameTime.isPaused()) {
			d3dv->color = Color::gray(float(col) / 255.f).toRGB(col);
		}
		
		d3dv->uv.x = u;
		d3dv->uv.y = 0.f;
		vertex++;
		d3dv++;
		
		d3dv->p = m_pos + Vec3f(vertex->x, 0.f, vertex->z);
		
		// TODO per-frame randomness
		col = Random::get(0, 80);
		if(!g_gameTime.isPaused()) {
			d3dv->color = Color::black.toRGB(col);
		}
		
		d3dv->uv.x = u;
		d3dv->uv.y = 1.f;
		vertex++;
		d3dv++;
		
		u += du;
		nb--;
	}
	
	RenderMaterial mat;
	mat.setDepthTest(true);
	mat.setBlendType(RenderMaterial::Additive);
	mat.setWrapMode(TextureStage::WrapMirror);
	mat.setTexture(m_tsouffle);
	mat.setCulling(CullCW);
	
	int i = FaceCount - 2;
	int j = 0;
	while(i--) {
		drawTriangle(mat, &coned3d[j]);
		j++;
	}
	
	mat.setCulling(CullCCW);
	
	i = FaceCount - 2;
	j = 0;
	while(i--) {
		drawTriangle(mat, &coned3d[j]);
		j++;
	}
	
}
