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

#ifndef ARX_GAME_MAGIC_SPELLS_SPELLSLVL02_H
#define ARX_GAME_MAGIC_SPELLS_SPELLSLVL02_H

#include "game/Spells.h"

class HealSpell {
public:
	static void Launch(long i, long duration);
	static void Update(size_t i, float framedelay);
};

class DetectTrapSpell {
public:
	static void Launch(long i, SpellType typ);
	static void End(size_t i);
	static void Update(size_t i, float timeDelta);
};

class ArmorSpell {
public:
	static void Launch(SpellType typ, long duration, long i);
	static void End(size_t i);
	static void Update(size_t i, float timeDelta);
};

class LowerArmorSpell {
public:
	static void Launch(SpellType typ, long duration, long i);
	static void End(long i);
	static void Update(size_t i, float timeDelta);
};

class HarmSpell {
public:
	static void Launch(long duration, long i);
	static void Kill(long i);
	static void Update(size_t i, float timeDelta);
};

#endif // ARX_GAME_MAGIC_SPELLS_SPELLSLVL02_H
