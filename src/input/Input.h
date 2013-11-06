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
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#ifndef ARX_INPUT_INPUT_H
#define ARX_INPUT_INPUT_H

#include "input/Keyboard.h"
#include "input/Mouse.h"
#include "input/InputKey.h"
#include "math/Vector.h"

class Window;

class Input {
	
public:
	
	static const std::string KEY_NONE;
	
	Input();
	
	bool init(Window * window);
	void reset();
	
	void update();
	
	static std::string getKeyName(InputKeyId key, bool localizedName = false);
	static InputKeyId getKeyId(const std::string & keyName);
	
	// Action
	
	bool actionNowPressed(int actionId) const;
	bool actionPressed(int actionId) const;
	bool actionNowReleased(int actionId) const;
	
	// Mouse
	
	const Vec2s & getMousePosAbs() const { return iMouseA; }
	const Vec2s & getMousePosRel() const { return iMouseR; }
	bool isMouseInWindow() const { return mouseInWindow; }
	void setMousePosAbs(const Vec2s & mousePos);
	
	void setMouseSensitivity(int sensitivity);
	int getMouseSensitivity() const { return iSensibility; }
	
	bool hasMouseMoved() const { return iMouseR != Vec2s_ZERO; }
	bool getMouseButton(int buttonId) const;
	int getMouseButtonClicked() const;
	bool getMouseButtonRepeat(int buttonId) const;
	bool getMouseButtonNowPressed(int buttonId) const;
	bool getMouseButtonNowUnPressed(int buttonId) const;
	bool getMouseButtonDoubleClick(int buttonId, int timeMs) const;
	
	int getMouseWheelDir() const { return iWheelDir; }
	
	// Keyboard
	
	int getKeyPressed() const { return iKeyId; }
	bool isAnyKeyPressed() const { return iKeyId >= 0; }
	bool isKeyPressed(int keyId) const;
	bool isKeyPressedNowPressed(int keyId) const;
	bool isKeyPressedNowUnPressed(int keyId) const;
	bool getKeyAsText(int keyId, char & result) const;
	
private:
	
	class InputBackend * backend;
	
	// Mouse
	
	Vec2s iMouseR;
	Vec2s iMouseA;
	bool  mouseInWindow;
	
	int   iSensibility;
	int   iWheelDir;
	
	bool  bMouseButton[Mouse::ButtonCount];
	bool  bOldMouseButton[Mouse::ButtonCount];
	
	int   iMouseTime[Mouse::ButtonCount];
	int   iMouseTimeSet[Mouse::ButtonCount];
	int   iOldNumClick[Mouse::ButtonCount];
	
	// Keyboard
	
	int   iKeyId;                         // Id of the key pressed in the last update
	int   keysStates[Keyboard::KeyCount]; // 0: up, 1: just pressed/released, 2: pressed
	
};

extern Input * GInput;

bool ARX_INPUT_Init(Window * window);
void ARX_INPUT_Release();
 
#endif // ARX_INPUT_INPUT_H
