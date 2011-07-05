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

#ifndef ARX_WINDOW_DXINPUT_H
#define ARX_WINDOW_DXINPUT_H

#include "window/Input.h"


enum DXIButton {
	DXI_BUTTON0 = 0,
	DXI_BUTTON1 = 1,
	DXI_BUTTON2 = 2,
	DXI_BUTTON3 = 3,
	DXI_BUTTON4 = 4,
	DXI_BUTTON5 = 5,
	DXI_BUTTON6 = 6,
	DXI_BUTTON7 = 7,
	DXI_BUTTON8 = 8,
	DXI_BUTTON9 = 9,
	DXI_BUTTON10 = 10,
	DXI_BUTTON11 = 11,
	DXI_BUTTON12 = 12,
	DXI_BUTTON13 = 13,
	DXI_BUTTON14 = 14,
	DXI_BUTTON15 = 15,
	DXI_BUTTON16 = 16,
	DXI_BUTTON17 = 17,
	DXI_BUTTON18 = 18,
	DXI_BUTTON19 = 19,
	DXI_BUTTON20 = 20,
	DXI_BUTTON21 = 21,
	DXI_BUTTON22 = 22,
	DXI_BUTTON23 = 23,
	DXI_BUTTON24 = 24,
	DXI_BUTTON25 = 25,
	DXI_BUTTON26 = 26,
	DXI_BUTTON27 = 27,
	DXI_BUTTON28 = 28,
	DXI_BUTTON29 = 29,
	DXI_BUTTON30 = 30,
	DXI_BUTTON31 = 31
};

// TODO-input: remove the static / create a base class and add virtuals
class DX7Input : public Input
{
public:
	// TODO-input: call from constructor...
	static bool init();
	static void release();

	static bool update();

	static void acquireDevices();
	static void unacquireDevices();

	// Mouse 
	static bool getMouseCoordinates(int & mx, int & my, int & mz);
	static bool isMouseButtonPressed(int numb, int & _iDeltaTime);
	static void getMouseButtonClickCount(int numb, int & _iNumClick, int & _iNumUnClick);

	// Keyboard
	static bool isKeyboardKeyPressed(int dikkey);
	static int getKeyboardKeyPressed();
};

#endif // ARX_WINDOW_DXINPUT_H
