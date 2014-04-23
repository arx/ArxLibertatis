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

#ifndef ARX_GAME_MAGIC_SPELLS_SPELLSLVL07_H
#define ARX_GAME_MAGIC_SPELLS_SPELLSLVL07_H

#include "game/Spells.h"

bool FlyingEyeSpellLaunch(long i, TextureContainer * tc4);
void FlyingEyeSpellEnd(size_t i);
void FlyingEyeSpellKill(TextureContainer* tc4, long i);
void FlyingEyeSpellUpdate(size_t i, unsigned long tim, const long framediff3);

void FireFieldSpellLaunch(long i, SpellType typ, long duration);
void FireFieldSpellEnd(size_t i);
void FireFieldSpellKill(long i);
void FireFieldSpellUpdate(size_t i, float timeDelta);

void IceFieldSpellLaunch(long i, long duration, SpellType typ);
void IceFieldSpellEnd(size_t i);
void IceFieldSpellKill(long i);
void IceFieldSpellUpdate(size_t i, float timeDelta);

void LightningStrikeSpellLaunch(long i);
void LightningStrikeSpellEnd(size_t i);
void LightningStrikeKill(long i);
void LightningStrinkeSpellUpdate(size_t i, float timeDelta);

void ConfuseSpellLaunch(long i, bool & notifyAll, long duration);
void ConfuseSpellEnd(size_t i);
void ConfuseSpellUpdate(size_t i, float timeDelta);

#endif // ARX_GAME_MAGIC_SPELLS_SPELLSLVL07_H
