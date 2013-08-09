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

#include "graphics/effects/Trail.h"

#include "core/GameTime.h"
#include "math/Random.h"
#include "graphics/Renderer.h"
#include "graphics/effects/SpellEffects.h"


Trail::Trail(Vec3f & initialPosition)
	: m_nextPosition(initialPosition)
	, m_positions(20)
{
	int segments = Random::get(8, 16);

	Color4f startColor = Color4f::gray(Random::getf(0.1f, 0.2f));
	Color4f endColor = Color4f::black;

	float startSize = Random::getf(2.f, 4.f);
	float endSize = 0.f;

//	startColor = Color4f::red;
//	endColor = Color4f::green;

	Color4f colorDelta = endColor - startColor;
	float sizeDelta = endSize - startSize;

	for(int i = 0; i < segments; i++) {
		float factor = (float)i / (float)segments;

		Color4f color = startColor + colorDelta * factor;
		float size = startSize + sizeDelta * factor;

		m_segments.push_back(TrailSegment(Color(color.r*255, color.g*255, color.b*255, color.a*255), size));
	}

	Update();
}

void Trail::SetNextPosition(Vec3f & nextPosition)
{
	m_nextPosition = nextPosition;
}

void Trail::Update() {
	if(arxtime.is_paused())
		return;

	m_positions.push_front(m_nextPosition);
}

void Trail::Render()
{
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->ResetTexture(0);

	for(size_t i = 0; i + 1 < m_positions.size() && i + 1 < m_segments.size(); i++) {
		Draw3DLineTex2(m_positions[i],
					   m_positions[i + 1],
					   m_segments[i].m_size,
					   m_segments[i].m_color,
					   m_segments[i + 1].m_color);
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendZero);
}
