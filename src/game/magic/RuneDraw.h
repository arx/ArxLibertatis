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

#ifndef ARX_GAME_MAGIC_RUNEDRAW_H
#define ARX_GAME_MAGIC_RUNEDRAW_H

#include <string>

#include "core/TimeTypes.h"
#include "game/magic/Rune.h"

class Entity;

struct SYMBOL_DRAW {
	
	GameInstant starttime;
	AnimationDuration elapsed;
	AnimationDuration duration;
	std::string sequence;
	Vec2s cPosStart;
	
	SYMBOL_DRAW()
		: starttime(0)
		, elapsed(0)
		, duration(0)
		, cPosStart(0)
	{ }
	
};

void ARX_SPELLS_Init_Rects();

void ARX_SPELLS_UpdateSymbolDraw();
void ARX_SPELLS_UpdateBookSymbolDraw(const Rect & bookRect);
void ARX_SPELLS_ClearAllSymbolDraw();

void ARX_SPELLS_RequestSymbolDraw(Entity * io, const std::string & name, GameDuration duration);
void ARX_SPELLS_RequestSymbolDraw2(Entity * io, Rune symb, GameDuration duration);

#endif // ARX_GAME_MAGIC_RUNEDRAW_H
