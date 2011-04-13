/*
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// ARX_Input
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Input interface (uses MERCURY input system)
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#ifndef ARX_WINDOW_INPUT_H
#define ARX_WINDOW_INPUT_H

#ifndef DIRECTINPUT_VERSION
	#define DIRECTINPUT_VERSION 0x0700
#endif
#include <dinput.h>

#include "window/DXInput.h"

//-----------------------------------------------------------------------------
#define INTERNAL_MOUSE_1	257
#define INTERNAL_MOUSE_2	258
#define INTERNAL_MOUSE_3	259
#define INTERNAL_MOUSE_4	260
#define INTERNAL_MOUSE_5	261
#define INTERNAL_MOUSE_6	262

#define INTERNAL_MOUSEWHEEL_DOWN	280
#define INTERNAL_MOUSEWHEEL_UP		281

#define INTERNAL_SCID			400

#define INTERNAL_JOYSTICK_1		297
#define INTERNAL_JOYSTICK_2		298
#define INTERNAL_JOYSTICK_3		299
#define INTERNAL_JOYSTICK_4		300
#define INTERNAL_JOYSTICK_5		301
#define INTERNAL_JOYSTICK_6		302
#define INTERNAL_JOYSTICK_7		303
#define INTERNAL_JOYSTICK_8		304
#define INTERNAL_JOYSTICK_9		305
#define INTERNAL_JOYSTICK_10	306
#define INTERNAL_JOYSTICK_11	307
#define INTERNAL_JOYSTICK_12	308
#define INTERNAL_JOYSTICK_13	309

#define MAX_IMPULSES						17
#define MAX_IMPULSES_NB						3

#define ARX_INPUT_IMPULSE_JUMP				0
#define ARX_INPUT_IMPULSE_COMBAT_MODE		1
#define ARX_INPUT_IMPULSE_MAGIC_MODE		2
#define ARX_INPUT_IMPULSE_STEALTH			3
#define ARX_INPUT_IMPULSE_WALK_FORWARD		4
#define ARX_INPUT_IMPULSE_WALK_BACKWARD		5
#define ARX_INPUT_IMPULSE_STRAFE_LEFT		6
#define ARX_INPUT_IMPULSE_STRAFE_RIGHT		7
#define ARX_INPUT_IMPULSE_MOUSE_LOOK		8
#define ARX_INPUT_IMPULSE_ACTION			9
#define ARX_INPUT_IMPULSE_INVENTORY			10
#define ARX_INPUT_IMPULSE_BOOK				11
#define ARX_INPUT_IMPULSE_ROTATE_LEFT		12
#define ARX_INPUT_IMPULSE_ROTATE_RIGHT		13
#define ARX_INPUT_IMPULSE_LEAN_RIGHT		14
#define ARX_INPUT_IMPULSE_LEAN_LEFT			15
#define ARX_INPUT_IMPULSE_CROUCH			16

// Enum for all the controlling actions
enum ControlAction
{
	CONTROLS_CUST_JUMP,
	CONTROLS_CUST_MAGICMODE,
	CONTROLS_CUST_STEALTHMODE,
	CONTROLS_CUST_WALKFORWARD,
	CONTROLS_CUST_WALKBACKWARD,
	CONTROLS_CUST_STRAFELEFT,
	CONTROLS_CUST_STRAFERIGHT,
	CONTROLS_CUST_LEANLEFT,
	CONTROLS_CUST_LEANRIGHT,
	CONTROLS_CUST_CROUCH,
	CONTROLS_CUST_MOUSELOOK,
	CONTROLS_CUST_ACTION,
	CONTROLS_CUST_INVENTORY,
	CONTROLS_CUST_BOOK,
	CONTROLS_CUST_BOOKCHARSHEET,
	CONTROLS_CUST_BOOKSPELL,
	CONTROLS_CUST_BOOKMAP,
	CONTROLS_CUST_BOOKQUEST,
	CONTROLS_CUST_DRINKPOTIONLIFE,
	CONTROLS_CUST_DRINKPOTIONMANA,
	CONTROLS_CUST_TORCH,

	CONTROLS_CUST_PRECAST1,
	CONTROLS_CUST_PRECAST2,
	CONTROLS_CUST_PRECAST3,
	CONTROLS_CUST_WEAPON,
	CONTROLS_CUST_QUICKLOAD,
	CONTROLS_CUST_QUICKSAVE,

	CONTROLS_CUST_TURNLEFT,
	CONTROLS_CUST_TURNRIGHT,
	CONTROLS_CUST_LOOKUP,
	CONTROLS_CUST_LOOKDOWN,

	CONTROLS_CUST_STRAFE,
	CONTROLS_CUST_CENTERVIEW,

	CONTROLS_CUST_FREELOOK,

	CONTROLS_CUST_PREVIOUS,
	CONTROLS_CUST_NEXT,

	CONTROLS_CUST_CROUCHTOGGLE,

	CONTROLS_CUST_UNEQUIPWEAPON,

	CONTROLS_CUST_CANCELCURSPELL,

	CONTROLS_CUST_MINIMAP,

	MAX_ACTION_KEY
};

enum InputButton
{
	DIK_BUTTON1 = 0x80000000 | DXI_BUTTON0,
	DIK_BUTTON2 = 0x80000000 | DXI_BUTTON1,
	DIK_BUTTON3 = 0x80000000 | DXI_BUTTON2,
	DIK_BUTTON4 = 0x80000000 | DXI_BUTTON3,
	DIK_BUTTON5 = 0x80000000 | DXI_BUTTON4,
	DIK_BUTTON6 = 0x80000000 | DXI_BUTTON5,
	DIK_BUTTON7 = 0x80000000 | DXI_BUTTON6,
	DIK_BUTTON8 = 0x80000000 | DXI_BUTTON7,
	DIK_BUTTON9 = 0x80000000 | DXI_BUTTON8,
	DIK_BUTTON10 = 0x80000000 | DXI_BUTTON9,
	DIK_BUTTON11 = 0x80000000 | DXI_BUTTON10,
	DIK_BUTTON12 = 0x80000000 | DXI_BUTTON11,
	DIK_BUTTON13 = 0x80000000 | DXI_BUTTON12,
	DIK_BUTTON14 = 0x80000000 | DXI_BUTTON13,
	DIK_BUTTON15 = 0x80000000 | DXI_BUTTON14,
	DIK_BUTTON16 = 0x80000000 | DXI_BUTTON15,
	DIK_BUTTON17 = 0x80000000 | DXI_BUTTON16,
	DIK_BUTTON18 = 0x80000000 | DXI_BUTTON17,
	DIK_BUTTON19 = 0x80000000 | DXI_BUTTON18,
	DIK_BUTTON20 = 0x80000000 | DXI_BUTTON19,
	DIK_BUTTON21 = 0x80000000 | DXI_BUTTON20,
	DIK_BUTTON22 = 0x80000000 | DXI_BUTTON21,
	DIK_BUTTON23 = 0x80000000 | DXI_BUTTON22,
	DIK_BUTTON24 = 0x80000000 | DXI_BUTTON23,
	DIK_BUTTON25 = 0x80000000 | DXI_BUTTON24,
	DIK_BUTTON26 = 0x80000000 | DXI_BUTTON25,
	DIK_BUTTON27 = 0x80000000 | DXI_BUTTON26,
	DIK_BUTTON28 = 0x80000000 | DXI_BUTTON27,
	DIK_BUTTON29 = 0x80000000 | DXI_BUTTON28,
	DIK_BUTTON30 = 0x80000000 | DXI_BUTTON29,
	DIK_BUTTON31 = 0x80000000 | DXI_BUTTON30,
	DIK_BUTTON32 = 0x80000000 | DXI_BUTTON31,


	DIK_WHEELUP = 0x40000000 | 0,
	DIK_WHEELDOWN = 0x40000000 | 1,
};

bool ARX_INPUT_Init(HINSTANCE hInst, HWND hWnd);
void ARX_INPUT_Release();

bool ARX_IMPULSE_NowPressed(long ident);
bool ARX_IMPULSE_Pressed(long ident);
bool ARX_IMPULSE_NowUnPressed(long ident);
 
#endif // ARX_WINDOW_INPUT_H
