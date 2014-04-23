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

#ifndef ARX_GAME_MAGIC_SPELLS_SPELLSLVL01_H
#define ARX_GAME_MAGIC_SPELLS_SPELLSLVL01_H

void MagicSightSpellLaunch(long duration, long i);
void MagicSightSpellEnd(long i);

void MagicMissileSpellLaunch(long i);
void MagicMissileSpellEnd(long i);
void MagicMissileSpellUpdate(long i, float timeDelta);

void IgnitSpellLaunch(long i);
void IgnitSpellEnd(long i);
void IgnitSpellUpdate(long i, float timeDelta);

void DouseSpellLaunch(long i);
void DouseSpellEnd(long i);
void DouseSpellUpdate(long i, float timeDelta);

void ActivatePortalSpellLaunch(long i);

#endif // ARX_GAME_MAGIC_SPELLS_SPELLSLVL01_H
