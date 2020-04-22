/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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

#include "animation/AnimationRender.h"
#include "math/Random.h"
#include "graphics/RenderBatcher.h"
#include "graphics/Vertex.h"


Trail::Trail(long duration, Color3f startColor, Color3f endColor, float startSize, float endSize)
	: m_timePerSegment(0)
	, m_lastSegmentDuration(0)
	, m_totalDuration(0)
	, m_nextPosition(0.f)
	, m_startColor(startColor)
	, m_endColor(endColor)
	, m_startSize(startSize)
	, m_endSize(endSize)
{
	
	size_t segments = std::max(size_t(duration / 20), size_t(4));
	m_timePerSegment = GameDurationMsf(float(duration) / float(segments));
	
	m_positions.set_capacity(segments + 1);
	
}

void Trail::SetNextPosition(const Vec3f & nextPosition)
{
	m_nextPosition = nextPosition;
}

void Trail::Update(GameDuration timeDelta) {
	
	if(timeDelta == GameDuration(0)) {
		return;
	}
	
	if(m_positions.empty()) {
		m_positions.push_front(m_nextPosition);
		return;
	}
	
	bool moving = (m_nextPosition != m_positions.front());
	
	if(moving && m_positions.size() == 1) {
		m_positions.push_front(m_nextPosition);
		m_lastSegmentDuration = 0;
		m_totalDuration = 0;
		return;
	}
	
	if(moving) {
		m_totalDuration = std::min(m_totalDuration + timeDelta, m_timePerSegment * (m_positions.capacity() - 2));
		m_lastSegmentDuration += timeDelta;
	} else {
		m_totalDuration = std::max(m_totalDuration - timeDelta, GameDuration(0));
		m_lastSegmentDuration = std::min(m_lastSegmentDuration, m_totalDuration);
	}
	
	if(m_lastSegmentDuration >= m_timePerSegment) {
		m_totalDuration += m_timePerSegment;
		while(m_lastSegmentDuration >= m_timePerSegment) {
			m_totalDuration -= m_timePerSegment;
			m_lastSegmentDuration -= m_timePerSegment;
		}
		m_positions.push_front(m_nextPosition);
	} else {
		m_positions.front() = m_nextPosition;
	}
	
}

void Trail::Render() {
	
	if(emtpy()) {
		return;
	}
	
	RenderMaterial mat;
	mat.setCulling(CullNone);
	mat.setBlendType(RenderMaterial::AlphaAdditive);
	mat.setDepthTest(true);
	mat.setDepthBias(8);
	
	/*
	 *              n
	 *      |--------------|
	 *  |===|=====|=====|=====|  <-- trail direction
	 *  |---|              |
	 *    t              lastPos
	 */
	
	float t = m_lastSegmentDuration / m_timePerSegment;
	arx_assume(t >= 0.f && t <= 1.f);
	float n = (m_totalDuration - m_lastSegmentDuration) / m_timePerSegment;
	arx_assume(n >= 0.f);
	
	arx_assert(m_positions.size() >= size_t(std::ceil(n)) + 2);
	Vec3f lastPos = m_positions[size_t(n) + 1];
	if(size_t(n) + 2 < m_positions.size()) {
		lastPos = glm::mix(m_positions[size_t(n) + 1], m_positions[size_t(n) + 2], glm::fract(n));
	}
	
	// Direction of the arrow trail as a whole
	// This valies by segment but we don't want the trail to rotate per segment'
	Vec3f trailDir = lastPos - m_positions[0];
	
	/* 
	 * The trail is made up of two quads per segment except the first segment, which is made up of
	 * four triangles which don't share the same plane. The trail plane is rotated around the trail line
	 * to always face the camera and the front center vertex is extruded based on the view angle to hide
	 * that the trail is flat.
	 *
	 *  From camera   From side
	 *
	 *       Trail front
	 *
	 *       side     front        front
	 *      |--->     <----|       |---->
	 *                        view
	 *  |---|---|     |----|   \   |----|    m_positions[0]
	 *  |\  |  /|      \   |    \| |   /
	 *  | \ | / |       \  |       |  /
	 *  |  \|/  |        \ |       | /
	 *  |---|---|         \|       |/        m_positions[1] (or interpolated to keep a constant front part time)
	 *  |   |   |     _    |       |
	 *  |   |   |     /|   |       |
	 *  |   |   |    /     |       |
	 *  |---|---|   view   |       |
	 *
	 *       Trail back
	 */
	
	// Direction of the camera from the trail start
	Vec3f cameraDir = g_camera->m_pos - m_positions[0];
	
	// Calculate (cosine of) the angle between the camera direction and the trail direction
	// Zero when looking at the trail from the side, -1 when looking from the front, 1 from the back
	trailDir = glm::normalize(trailDir);
	cameraDir = glm::normalize(cameraDir);
	float costheta = glm::dot(trailDir, cameraDir);
	
	// Calculate the direction in which to extrude the trail geometry from the line
	// This is the direction that is at a right angle to both the camera direction and the trail direction
	// We need to normalize the result length because the arguments are not orthogonal.
	Vec3f side = glm::normalize(glm::cross(cameraDir, trailDir));
	
	// Calculate the direction in which to extrude the front center vertex
	// This is the direction that is at a right angle to both the trail direction and side extrude direction,
	// modulated by cos(angle(cameraDir, trailDir)) to move the vertex towards the camera when looking from the
	// back of the trail and away from the camera when looking from the front of the trail.
	// This is done do hide that our trail is really flat by giving it some depth at at the wide end.
	// We don't need to normalize the result of the cross product as the arguments are orthogonal unit vectors.
	Vec3f front = costheta * glm::cross(side, trailDir);
	
	Color4f colorDelta = (m_endColor - m_startColor) * (1.f / (t + n));
	float sizeDelta = (m_endSize - m_startSize) * (1.f / (t + n));
	
	Color color = Color(m_startColor + colorDelta);
	float size = m_startSize + sizeDelta;
	
	Vec3f position = m_positions[1];
	if(m_totalDuration > m_lastSegmentDuration) {
		position = glm::mix(m_positions[2], m_positions[1], t);
	} else {
		color = Color(Color4f(m_endColor, t));
		size = m_endSize;
	}
	
	// We use alpha to fade out the front while using black to fade out the sides.
	// With alpha-modulated additive blending this gives us a nice bilinear fade at the front.
	Color::type frontAlpha = 0;
	Color sideColor = Color::black;
	
	{
		TexturedVertexUntransformed v[3];
		v[0].color = sideColor.toRGB(frontAlpha);
		v[1].color = sideColor.toRGBA();
		v[2].color = color.toRGBA();
		v[0].p = m_positions[0] - side * m_startSize;
		v[1].p = position - side * size;
		v[2].p = position;
		drawTriangle(mat, v);
	}
	
	{
		TexturedVertexUntransformed v[3];
		v[0].color = sideColor.toRGB(frontAlpha);
		v[1].color = color.toRGBA();
		v[2].color = sideColor.toRGBA();
		v[0].p = m_positions[0] + side * m_startSize;
		v[1].p = position;
		v[2].p = position + side * size;
		drawTriangle(mat, v);
	}
	
	{
		TexturedVertexUntransformed v[3];
		v[0].color = m_startColor.toRGB(frontAlpha);
		v[1].color = sideColor.toRGB(frontAlpha);
		v[2].color = color.toRGBA();
		v[0].p = m_positions[0] - front * m_startSize;
		v[1].p = m_positions[0] - side * m_startSize;
		v[2].p = position;
		drawTriangle(mat, v);
	}
	
	{
		TexturedVertexUntransformed v[3];
		v[0].color = sideColor.toRGB(frontAlpha);
		v[1].color = m_startColor.toRGB(frontAlpha);
		v[2].color = color.toRGBA();
		v[0].p = m_positions[0] + side * m_startSize;
		v[1].p = m_positions[0] - front * m_startSize;
		v[2].p = position;
		drawTriangle(mat, v);
	}
	
	/*
	 *    t         n
	 *  |---|--------------|
	 *  |===|=====|=====|=====|  <-- trail direction
	 *        |---| |---|  |
	 *        | t i | t   lastPos
	 *        |     |
	 *  position  nextPosition
	 *          
	 */
	
	for(size_t i = 2; i < size_t(std::ceil(n)) + 2; i++) {
		
		Color nextColor = Color(m_startColor + colorDelta * float(i));
		float nextSize = m_startSize + sizeDelta * float(i);
		
		Vec3f nextPosition;
		if(i + 1 == size_t(std::ceil(n)) + 2) {
			nextPosition = lastPos;
			nextColor = Color(m_endColor);
			nextSize = m_endSize;
		} else {
			arx_assert(i + 1 < m_positions.size());
			nextPosition = glm::mix(m_positions[i + 1], m_positions[i], t);
		}
		
		{
			TexturedQuad q;
			q.v[0].color = color.toRGBA();
			q.v[1].color = sideColor.toRGBA();
			q.v[2].color = sideColor.toRGBA();
			q.v[3].color = nextColor.toRGBA();
			q.v[0].p = position;
			q.v[1].p = position - side * size;
			q.v[2].p = nextPosition - side * nextSize;
			q.v[3].p = nextPosition;
			drawQuadRTP(mat, q);
		}
		
		{
			TexturedQuad q;
			q.v[0].color = sideColor.toRGBA();
			q.v[1].color = color.toRGBA();
			q.v[2].color = nextColor.toRGBA();
			q.v[3].color = sideColor.toRGBA();
			q.v[0].p = position + side * size;
			q.v[1].p = position;
			q.v[2].p = nextPosition;
			q.v[3].p = nextPosition + side * nextSize;
			drawQuadRTP(mat, q);
		}
		
		position = nextPosition;
		color = nextColor;
		size = nextSize;
		
	}
	
}

ArrowTrail::ArrowTrail()
	: Trail(Random::get(130, 260), Color3f::gray(Random::getf(0.2f, 0.4f)), Color3f::black,
	        Random::getf(2.f, 4.f), 0.f)
{}

DebugTrail::DebugTrail()
	: Trail(500, Color3f::red, Color3f::green, 1.f, 4.f)
{}
