/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_PLATFORM_RANDOM_H
#define ARX_PLATFORM_RANDOM_H

#include <stddef.h>

/*!
 * A simple but very fast random number generator.
 */
class Random {
	
	static const size_t MODULO = 2147483647;
	static const size_t FACTOR = 16807;
	static const size_t SHIFT = 91;
	static const size_t SEED = 43;
	
	static size_t current;
	
public:
	
	inline static size_t get() {
		return current = (current * FACTOR + SHIFT) % MODULO;
	}
	
	inline static float getf() {
		return get() * (1.f / MODULO);
	}
	
	static void seed();
	
};

#endif // ARX_PLATFORM_RANDOM_H
