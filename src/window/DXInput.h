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
