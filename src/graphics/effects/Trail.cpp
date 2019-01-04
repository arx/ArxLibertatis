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

#include "graphics/effects/Trail.h"

#include "core/GameTime.h"
#include "math/Random.h"
#include "graphics/Renderer.h"
#include "graphics/RenderBatcher.h"
#include "graphics/effects/SpellEffects.h"

Trail::Trail(long duration, Color4f startColor, Color4f endColor, float startSize, float endSize)
	: m_timePerSegment(0)
	, m_lastSegmentDuration(0)
	, m_nextPosition(0.f)
{
	
	size_t segments = std::max(size_t(duration / 20), size_t(4));
	m_timePerSegment = GameDurationMsf(float(duration) / float(segments));
	
	m_positions.set_capacity(segments);
	m_segments.reserve(segments);
	
	Color4f colorDelta = endColor - startColor;
	float sizeDelta = endSize - startSize;
	for(size_t i = 0; i < segments; i++) {
		float factor = float(i) / float(segments);
		Color color(startColor + colorDelta * factor);
		float size = startSize + sizeDelta * factor;
		m_segments.push_back(TrailSegment(color, size));
	}
	
}

void Trail::SetNextPosition(Vec3f & nextPosition)
{
	m_nextPosition = nextPosition;
}

void Trail::Update(GameDuration timeDelta) {
	
	if(g_gameTime.isPaused()) {
		return;
	}
	
	m_lastSegmentDuration += timeDelta;
	
	if(m_lastSegmentDuration < m_timePerSegment) {
		if(!m_positions.empty())
			m_positions.pop_front();
	} else {
		m_lastSegmentDuration = 0;
	}
	
	m_positions.push_front(m_nextPosition);
}

void Trail::Render()
{
	RenderMaterial mat;
	mat.setCulling(CullNone);
	mat.setBlendType(RenderMaterial::Additive);
	
	for(size_t i = 0; i + 1 < m_positions.size() && i + 1 < m_segments.size(); i++) {
		Draw3DLineTexNew(mat, m_positions[i], m_positions[i + 1],
		                 m_segments[i].m_color, m_segments[i + 1].m_color,
		                 m_segments[i].m_size, m_segments[i + 1].m_size);
	}
}


ArrowTrail::ArrowTrail()
	: Trail(Random::get(130, 260), Color4f::gray(Random::getf(0.1f, 0.2f)), Color4f::black,
	        Random::getf(2.f, 4.f), 0.f)
{}

DebugTrail::DebugTrail()
	: Trail(500, Color4f::red, Color4f::green, 1.f, 4.f)
{}
