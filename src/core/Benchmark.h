/*
 * Copyright 2015-2017 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_CORE_BENCHMARK_H
#define ARX_CORE_BENCHMARK_H

namespace benchmark {

enum Status {
	None,
	Startup,
	Splash,
	Menu,
	LoadLevel,
	Scene,
	Cutscene,
	Cinematic,
	Shutdown
};

/*!
 * Start a new benchmark (frame)
 * 
 * Implicitly ends any running benchmark.
 */
void begin(Status status);

//! End the current benchmark
void shutdown();

bool isEnabled();

} // namespace benchmark

#endif // ARX_CORE_BENCHMARK_H
