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

#ifndef ARX_GAME_MAGIC_RUNE_H
#define ARX_GAME_MAGIC_RUNE_H

#include <string>
#include <vector>

#include <boost/array.hpp>

#include "math/Types.h"

class Entity;

// Spells symbol list
enum Rune {
	RUNE_AAM = 0,     // Create
	RUNE_NHI,         // Negate
	RUNE_MEGA,        // Improve
	RUNE_YOK,         // Fire
	RUNE_TAAR,        // Projectile
	RUNE_KAOM,        // Protection
	RUNE_VITAE,       // Life
	RUNE_VISTA,       // Vision
	RUNE_STREGUM,     // Magic
	RUNE_MORTE,       // Death
	RUNE_COSUM,       // Object
	RUNE_COMUNICATUM, // Communication
	RUNE_MOVIS,       // Movement
	RUNE_TEMPUS,      // Time
	RUNE_FOLGORA,     // Storm
	RUNE_SPACIUM,     // Space
	RUNE_TERA,        // Earth
	RUNE_CETRIUS,     // Poison
	RUNE_RHAA,        // Lower
	RUNE_FRIDD,       // Ice
	RUNE_AKBAA,       // Akbaa
	
	RUNE_COUNT,
	RUNE_NONE = 255
};

struct RuneInfo {
	
	RuneInfo()
	{}

	RuneInfo(Rune rune_, const std::string & name_, Vec2s startOffset_, const std::string & sequence_)
		: rune(rune_)
		, name(name_)
		, startOffset(startOffset_)
		, sequence(sequence_)
	{}

	Rune        rune;
	std::string name;
	Vec2s       startOffset;
	std::string sequence;
};

extern boost::array<RuneInfo, RUNE_COUNT> runeInfos;

void RuneInfosFill();


#endif // ARX_GAME_MAGIC_RUNE_H
