/*
 * Copyright 2014-2017 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GAME_MAGIC_SPELLRECOGNITION_H
#define ARX_GAME_MAGIC_SPELLRECOGNITION_H

#include <string>

#include "game/magic/Rune.h"

const size_t MAX_SPELL_SYMBOLS = 6;
extern Rune SpellSymbol[MAX_SPELL_SYMBOLS];

extern size_t CurrSpellSymbol;
extern std::string SpellMoves;
extern std::string LAST_FAILED_SEQUENCE;

void spellRecognitionInit();

void ARX_SPELLS_ResetRecognition();

void spellRecognitionPointsReset();
void ARX_SPELLS_AddPoint(const Vec2s & pos);

void ARX_SPELLS_Analyse();
void ARX_SPELLS_AnalyseSYMBOL();
bool ARX_SPELLS_AnalyseSPELL();

void ARX_SPELLS_Analyse_Alt();

#endif // ARX_GAME_MAGIC_SPELLRECOGNITION_H
