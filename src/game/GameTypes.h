/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GAME_GAMETYPES_H
#define ARX_GAME_GAMETYPES_H

#include "util/HandleType.h"

typedef HandleType<struct EntityHandleTag,  long, -1> EntityHandle;
typedef HandleType<struct SpellHandleTag,   long, -1> SpellHandle;
typedef HandleType<struct PrecastHandleTag, long, -1> PrecastHandle;
typedef HandleType<struct DamageHandleTag,  long, -1> DamageHandle;

static const EntityHandle EntityHandle_Player = EntityHandle(0);
static const EntityHandle EntityHandle_Self   = EntityHandle(-2);

struct ResourcePool {
	
	float current;
	float max;
	
	ResourcePool()
		: current(0.f)
		, max(0.f)
	{ }
	
	explicit ResourcePool(float value)
		: current(value)
		, max(value)
	{ }
	
};

enum Material {
	MATERIAL_NONE,
	MATERIAL_WEAPON,
	MATERIAL_FLESH,
	MATERIAL_METAL,
	MATERIAL_GLASS,
	MATERIAL_CLOTH,
	MATERIAL_WOOD,
	MATERIAL_EARTH,
	MATERIAL_WATER,
	MATERIAL_ICE,
	MATERIAL_GRAVEL,
	MATERIAL_STONE,
	MATERIAL_FOOT_LARGE,
	MATERIAL_FOOT_BARE,
	MATERIAL_FOOT_SHOE,
	MATERIAL_FOOT_METAL,
	MATERIAL_FOOT_STEALTH,
	
	MAX_MATERIALS
};

#endif // ARX_GAME_GAMETYPES_H
