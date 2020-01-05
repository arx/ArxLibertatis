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

#ifndef ARX_MATH_RANDOMFLICKER_H
#define ARX_MATH_RANDOMFLICKER_H

#include <glm/glm.hpp>

#include "math/Random.h"

namespace math {

/*!
 * Framerate-independent random flickering effect.
 *
 * Chooses random values between \c 0 and \c 1 and interpolates between them.
 */
class RandomFlicker {
	
public:
	
	RandomFlicker() : m_time(1.f), m_lastValue(0.5f), m_nextValue(0.5f) { }
	
	void reset() { m_time = 1.f, m_lastValue = 0.5f, m_nextValue = 0.5f; }
	
	void update(float time) {
		
		m_time += time * Random::getf(0.f, 2.f);
		
		if(m_time >= 1.f) {
			m_lastValue = m_nextValue;
			m_nextValue = Random::getf();
			m_time -= long(m_time);
		}
		
	}
	
	float get() const { return glm::mix(m_lastValue, m_nextValue, m_time); }
	
private:
	
	float m_time;
	float m_lastValue;
	float m_nextValue;
	
};

} // namespace math

#endif // ARX_MATH_RANDOMFLICKER_H
