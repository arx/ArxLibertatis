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

#ifndef ARX_GAME_MAGIC_SPELLDATA_H
#define ARX_GAME_MAGIC_SPELLDATA_H

#include <string>

#include "game/magic/Rune.h"
#include "game/magic/Spell.h"

#include "graphics/data/TextureContainer.h"

struct SPELL_ICON {
	TextureContainer * tc;
	std::string name;
	std::string description;
	long level;
	SpellType spellid;
	Rune symbols[6];
	bool bSecret;
	bool m_hasDuration;
	bool bAudibleAtStart;
	
	SPELL_ICON()
		: tc(NULL)
		, level(0)
		, spellid(SPELL_NONE)
		, bSecret(false)
		, m_hasDuration(true)
		, bAudibleAtStart(false)
	{
		for(long j = 0; j < 6; j++) {
			symbols[j] = RUNE_NONE;
		}
	}
};

extern SPELL_ICON spellicons[SPELL_TYPES_COUNT];

void spellDataInit();
void spellDataRelease();

#endif // ARX_GAME_MAGIC_SPELLDATA_H
