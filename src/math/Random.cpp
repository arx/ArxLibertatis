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

#include "math/Random.h"

#include <ctime>

ARX_THREAD_LOCAL Random::Generator * Random::rng = NULL;

void Random::seed() {
	if(!rng) {
		rng = new Generator;
	}
	
	rng->seed(size_t(std::time(NULL)));
}

void Random::seed(unsigned int seedVal) {
	if(!rng) {
		rng = new Generator;
	}
	
	rng->seed(seedVal);
}

void Random::shutdown() {
	delete rng;
	rng = NULL;
}
