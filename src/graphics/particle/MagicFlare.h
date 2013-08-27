/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#include "game/Entity.h"
#include "game/Camera.h"

extern long flarenum;

void MagicFlareLoadTextures();
void MagicFlareSetCamera(EERIE_CAMERA * camera);

void MagicFlareReleaseEntity(Entity * io);
long MagicFlareCountNonFlagged();

void ARX_MAGICAL_FLARES_FirstInit();
void ARX_MAGICAL_FLARES_KillAll();
void AddFlare(const Vec2s & pos, float sm, short typ, Entity * io = NULL, bool bookDraw = false);
void FlareLine(const Vec2s &pos0, const Vec2s &pos1, Entity * io = NULL);
void ARX_MAGICAL_FLARES_Draw();

#endif // ARX_GRAPHICS_PARTICLE_MAGICFLARE_H
