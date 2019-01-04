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

#ifndef ARX_GRAPHICS_PARTICLE_MAGICFLARE_H
#define ARX_GRAPHICS_PARTICLE_MAGICFLARE_H

#include "math/Types.h"

class Entity;

void MagicFlareLoadTextures();

void MagicFlareReleaseEntity(Entity * io);
long MagicFlareCountNonFlagged();

void ARX_MAGICAL_FLARES_FirstInit();
void ARX_MAGICAL_FLARES_KillAll();
void MagicFlareChangeColor();
void AddFlare(const Vec2f & pos, float sm, short typ, Entity * io = NULL, bool bookDraw = false);
void FlareLine(Vec2f pos0, Vec2f pos1, Entity * io = NULL);
void ARX_MAGICAL_FLARES_Update();

#endif // ARX_GRAPHICS_PARTICLE_MAGICFLARE_H
