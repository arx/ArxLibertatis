/*
 * Copyright 2014-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "game/magic/Rune.h"

#include <string>
#include <vector>

#include <boost/foreach.hpp>

#include "game/Entity.h"
#include "math/Types.h"

boost::array<RuneInfo, RUNE_COUNT> runeInfos;

void RuneInfosFill() {
	runeInfos[RUNE_AAM] =         RuneInfo(RUNE_AAM,         "aam",         Vec2s(0, 2), "6666");
	runeInfos[RUNE_NHI] =         RuneInfo(RUNE_NHI,         "nhi",         Vec2s(4, 2), "4444");
	runeInfos[RUNE_MEGA] =        RuneInfo(RUNE_MEGA,        "mega",        Vec2s(2, 4), "88888");
	runeInfos[RUNE_YOK] =         RuneInfo(RUNE_YOK,         "yok",         Vec2s(0, 0), "222226666888");
	runeInfos[RUNE_TAAR] =        RuneInfo(RUNE_TAAR,        "taar",        Vec2s(0, 1), "666222666");
	runeInfos[RUNE_KAOM] =        RuneInfo(RUNE_KAOM,        "kaom",        Vec2s(3, 0), "44122366");
	runeInfos[RUNE_VITAE] =       RuneInfo(RUNE_VITAE,       "vitae",       Vec2s(0, 2), "66666888");
	runeInfos[RUNE_VISTA] =       RuneInfo(RUNE_VISTA,       "vista",       Vec2s(1, 0), "333111");
	runeInfos[RUNE_STREGUM] =     RuneInfo(RUNE_STREGUM,     "stregum",     Vec2s(0, 4), "8888833338888");
	runeInfos[RUNE_MORTE] =       RuneInfo(RUNE_MORTE,       "morte",       Vec2s(0, 2), "66666222");
	runeInfos[RUNE_COSUM] =       RuneInfo(RUNE_COSUM,       "cosum",       Vec2s(0, 2), "66666222244448888");
	runeInfos[RUNE_COMUNICATUM] = RuneInfo(RUNE_COMUNICATUM, "comunicatum", Vec2s(0, 0), "6666622244442226666");
	runeInfos[RUNE_MOVIS] =       RuneInfo(RUNE_MOVIS,       "movis",       Vec2s(0, 0), "666611116666");
	runeInfos[RUNE_TEMPUS] =      RuneInfo(RUNE_TEMPUS,      "tempus",      Vec2s(0, 4), "88886662226668866");
	runeInfos[RUNE_FOLGORA] =     RuneInfo(RUNE_FOLGORA,     "folgora",     Vec2s(0, 3), "99993333");
	runeInfos[RUNE_SPACIUM] =     RuneInfo(RUNE_SPACIUM,     "spacium",     Vec2s(4, 0), "44444222266688");
	runeInfos[RUNE_TERA] =        RuneInfo(RUNE_TERA,        "tera",        Vec2s(0, 3), "99922266");
	runeInfos[RUNE_CETRIUS] =     RuneInfo(RUNE_CETRIUS,     "cetrius",     Vec2s(0, 1), "33388886666");
	runeInfos[RUNE_RHAA] =        RuneInfo(RUNE_RHAA,        "rhaa",        Vec2s(2, 0), "22222");
	runeInfos[RUNE_FRIDD] =       RuneInfo(RUNE_FRIDD,       "fridd",       Vec2s(0, 4), "888886662222");
	runeInfos[RUNE_AKBAA] =       RuneInfo(RUNE_AKBAA,       "akbaa",       Vec2s(0, 0), "22666772222");
}
