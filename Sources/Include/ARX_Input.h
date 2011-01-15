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
#ifndef ARX_INPUT_H
#define ARX_INPUT_H

#define DIRECTINPUT_VERSION 0x0700
#include <dinput.h>
#include "MERCURY_dx_input.h"
#include "MERCURY_extern.h"

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

//-----------------------------------------------------------------------------
extern long ARX_XBOXPAD;
extern long GameImpulses[MAX_IMPULSES][MAX_IMPULSES_NB];

//-----------------------------------------------------------------------------
void ARX_INPUT_Init_Game_Impulses();
BOOL ARX_INPUT_Init(HINSTANCE hInst, HWND hWnd);
void ARX_INPUT_Release();
BOOL ARX_INPUT_GetSCIDAxis(int * jx, int * jy, int * jz);
 
//-----------------------------------------------------------------------------
BOOL ARX_IMPULSE_NowPressed(long ident);
BOOL ARX_IMPULSE_Pressed(long ident);
BOOL ARX_IMPULSE_NowUnPressed(long ident);
 

#endif
