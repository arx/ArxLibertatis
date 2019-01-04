/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GAME_SPELL_CHEAT_H
#define ARX_GAME_SPELL_CHEAT_H

enum CheatRune {
	CheatRune_AAM,
	CheatRune_COMUNICATUM,
	CheatRune_KAOM,
	CheatRune_MEGA,
	CheatRune_SPACIUM,
	CheatRune_STREGUM,
	CheatRune_U,
	CheatRune_W,
	CheatRune_S,
	CheatRune_P,
	CheatRune_M,
	CheatRune_A,
	CheatRune_X,
	CheatRune_26,
	CheatRune_O,
	CheatRune_R,
	CheatRune_F,
	CheatRune_Passwall,
	CheatRune_ChangeSkin,

	CheatRune_None = 255
};

void handleCheatRuneDetection(CheatRune rune);


extern long BH_MODE;
extern long passwall;
extern long cur_mx;
extern long cur_pom;
extern long cur_rf;
extern long cur_mr;

extern long sp_arm;

extern long cur_mega;
extern long sp_wep;
extern short uw_mode;

extern long sp_max;

void CheatDrawText();

void CheatReset();
void CheatDetectionReset();

void CheckMr();

#endif // ARX_GAME_SPELL_CHEAT_H
