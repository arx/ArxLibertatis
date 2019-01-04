/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GRAPHICS_EFFECTS_TRAIL_H
#define ARX_GRAPHICS_EFFECTS_TRAIL_H

#include <vector>
#include <boost/circular_buffer.hpp>

#include "core/TimeTypes.h"
#include "math/Vector.h"
#include "graphics/Color.h"

class Trail {

public:
	Trail(long duration, Color4f startColor, Color4f endColor, float startSize, float endSize);

	void SetNextPosition(Vec3f & nextPosition);

	void Update(GameDuration timeDelta);
	void Render();

private:
	struct TrailSegment {

		TrailSegment(Color color, float size)
			: m_color(color)
			, m_size(size)
		{}

		Color m_color;
		float m_size;
	};

	GameDuration m_timePerSegment;
	GameDuration m_lastSegmentDuration;
	std::vector<TrailSegment> m_segments;

	Vec3f m_nextPosition;
	boost::circular_buffer<Vec3f> m_positions;
};

class ArrowTrail : public Trail {
public:
	ArrowTrail();
};

class DebugTrail : public Trail {
public:
	DebugTrail();
};

#endif // ARX_GRAPHICS_EFFECTS_TRAIL_H
