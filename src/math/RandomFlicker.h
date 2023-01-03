/*
 * Copyright 2015-2022 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_MATH_RANDOMFLICKER_H
#define ARX_MATH_RANDOMFLICKER_H

#include <cmath>

#include <glm/glm.hpp>

#include "math/Random.h"
#include "platform/Platform.h"


namespace math {

/*!
 * Framerate-independent random flickering effect.
 *
 * Chooses random values between \c 0 and \c 1 and interpolates between them.
 */
class RandomFlicker {
	
public:
	
	constexpr RandomFlicker() arx_noexcept_default
	
	void update(float time) noexcept {
		
		m_time += time;
		
		if(m_time >= 1.f) {
			m_lastValue = m_nextValue;
			m_nextValue = Random::getf();
			m_time -= std::floor(m_time);
		}
		
	}
	
	[[nodiscard]] float get() const noexcept {
		return glm::mix(m_lastValue, m_nextValue, m_time);
	}
	
private:
	
	float m_time = 1.f;
	float m_lastValue = 0.5f;
	float m_nextValue = 0.5f;
	
};

} // namespace math

#endif // ARX_MATH_RANDOMFLICKER_H
