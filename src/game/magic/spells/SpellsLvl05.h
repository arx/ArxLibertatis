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

#ifndef ARX_GAME_MAGIC_SPELLS_SPELLSLVL05_H
#define ARX_GAME_MAGIC_SPELLS_SPELLSLVL05_H

#include "game/Spells.h"

void RuneOfGuardingSpellLaunch(long i, SpellType typ, long duration);
void RuneOfGuardingSpellUpdate(size_t i, float timeDelta);

void LevitateSpellLaunch(long duration, long i, SpellType typ);
void LevitateSpellEnd(size_t i);
void LevitateSpellUpdate(size_t i, float timeDelta);

void CurePoisonSpellLaunch(long i);
void CurePoisonSpellUpdate(size_t i, float timeDelta);

void RepelUndeadSpellLaunch(long duration, long i);
void RepelUndeadSpellKill(long i);
void RepelUndeadSpellUpdate(size_t i, float timeDelta);

void PoisonProjectileSpellLaunch(long i);
void PoisonProjectileSpellUpdate(size_t i, float timeDelta);

#endif // ARX_GAME_MAGIC_SPELLS_SPELLSLVL05_H
