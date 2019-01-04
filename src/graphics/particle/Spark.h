/*
 * Copyright 2016-2018 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GRAPHICS_PARTICLE_SPARK_H
#define ARX_GRAPHICS_PARTICLE_SPARK_H

#include "math/Types.h"

enum SpawnSparkType {
	SpawnSparkType_Default = 0,
	SpawnSparkType_Failed = 1,
	SpawnSparkType_Success = 2
};

void ParticleSparkClear();
long ParticleSparkCount();

void ParticleSparkSpawn(const Vec3f & pos, unsigned count, SpawnSparkType type);
void ParticleSparkSpawnContinous(const Vec3f & pos, unsigned rate, SpawnSparkType type);

void ParticleSparkUpdate();

#endif // ARX_GRAPHICS_PARTICLE_SPARK_H
