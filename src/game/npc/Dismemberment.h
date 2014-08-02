/*
 * Copyright 2014 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GAME_NPC_DISMEMBERMENT_H
#define ARX_GAME_NPC_DISMEMBERMENT_H

#include "math/Types.h"

class Entity;

void ARX_NPC_RestoreCuts();

/*!
 * \brief Attempt to cut something on NPC
 */
void ARX_NPC_TryToCutSomething(Entity * target, const Vec3f * pos);

#endif // ARX_GAME_NPC_DISMEMBERMENT_H
