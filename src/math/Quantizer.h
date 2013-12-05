/*
 * Copyright 2013 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_MATH_QUANTIZER_H
#define ARX_MATH_QUANTIZER_H

namespace math {

/*!
 * Class to quantitize (time) values without loosing precison.
 */
class Quantizer {
	
public:
	
	Quantizer() : m_value(0.f) { }
	
	//! Resets the stored value to zero.
	void reset() { m_value = 0.f; }
	
	//! Adds a possibly fractional amount to the stored value.
	void add(float value) { m_value += value; }
	
	/*!
	 * Consumes the integer portion of the stored value.
	 * Returns the old integer portion and subtracts it from the stored value.
	 */
	int consume() {
		int integerValue = static_cast<int>(m_value);
		m_value -= float(integerValue);
		return integerValue;
	}
	
	/*!
	 * Adds a possibly fractional amount to the stored value and then consumes the integer
	 * portion of the stored value.
	 */
	int update(float value) {
		add(value);
		return consume();
	}
	
private:
	
	float m_value;
	
};

} // namespace math

#endif // ARX_MATH_QUANTIZER_H
