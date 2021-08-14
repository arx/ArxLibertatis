/*
 * Copyright 2014-2019 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GAME_MAGIC_SPELLDATA_H
#define ARX_GAME_MAGIC_SPELLDATA_H

#include <array>
#include <string_view>

#include "game/magic/Rune.h"
#include "game/magic/Spell.h"

#include "graphics/data/TextureContainer.h"

struct SPELL_ICON {
	
	TextureContainer * tc;
	std::string_view name;
	std::string_view description;
	long level;
	SpellType spellid;
	std::array<Rune, 6> symbols;
	bool bSecret;
	bool m_hasDuration;
	bool bAudibleAtStart;
	
	SPELL_ICON()
		: tc(nullptr)
		, level(0)
		, spellid(SPELL_NONE)
		, symbols({ RUNE_NONE, RUNE_NONE, RUNE_NONE, RUNE_NONE, RUNE_NONE, RUNE_NONE })
		, bSecret(false)
		, m_hasDuration(true)
		, bAudibleAtStart(false)
	{ }
	
};

extern std::array<SPELL_ICON, SPELL_TYPES_COUNT> spellicons;

void spellDataInit();
void spellDataRelease();

#endif // ARX_GAME_MAGIC_SPELLDATA_H
