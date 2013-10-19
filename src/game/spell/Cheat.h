/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

extern long cur_mx;
extern long cur_pnux;
extern long cur_pom;
extern long cur_rf;
extern long cur_mr;
extern long cur_sm;
extern long cur_bh;

extern long sp_arm;
extern long cur_arm;
extern long cur_sos;

extern long cur_mega;
extern float sp_max_start;
extern long sp_wep;
extern short uw_mode;

extern short uw_mode_pos;

extern long sp_max;

void Manage_sp_max();

void CheatReset();

void ApplyPasswall();
void ApplySPArm();
void ApplySPuw();
void ApplySPRf();
void ApplySPMax();
void ApplySPWep();
void ApplySPBow();
void ApplyCurPNux();
void ApplyCurMr();
void ApplyCurSOS();
void EERIE_OBJECT_SetBHMode();

void CheckMr();

#endif // ARX_GAME_SPELL_CHEAT_H
