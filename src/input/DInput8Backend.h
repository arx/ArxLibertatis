/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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
/* Based on:
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

#ifndef ARX_INPUT_DINPUT8BACKEND_H
#define ARX_INPUT_DINPUT8BACKEND_H

#include "input/Input.h"
#include "input/InputBackend.h"

class DInput8Backend : public InputBackend
{
public:
	DInput8Backend();
	virtual ~DInput8Backend();

	virtual bool init();
	virtual bool update();

	virtual void acquireDevices();
	virtual void unacquireDevices();

	// Mouse 
	virtual void getMouseCoordinates(int & absX, int & absY, int & wheelDir) const;
	virtual void setMouseCoordinates(int absX, int absY);
	virtual bool isMouseButtonPressed(int buttonId, int & _iDeltaTime) const;
	virtual void getMouseButtonClickCount(int buttonId, int & _iNumClick, int & _iNumUnClick) const;

	// Keyboard
	virtual bool isKeyboardKeyPressed(int dikkey) const;
	virtual bool getKeyAsText(int keyId, char& result) const;

private:
	
	mutable int iLastMouseX;
	mutable int iLastMouseY;
	
};

#endif // ARX_INPUT_DINPUT8BACKEND_H
