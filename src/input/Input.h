/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include <stddef.h>

#include "core/Config.h"
#include "core/TimeTypes.h"
#include "input/InputKey.h"
#include "input/Keyboard.h"
#include "input/Mouse.h"
#include "math/Vector.h"
#include "math/Types.h"

class Window;
class TextInputHandler;

extern long EERIEMouseButton;
extern long LastMouseClick;

inline bool eeMouseDown1() {
	return (EERIEMouseButton & 1) && !(LastMouseClick & 1);
}

inline bool eeMouseUp1() {
	return !(EERIEMouseButton & 1) && (LastMouseClick & 1);
}

inline bool eeMousePressed1() {
	return (EERIEMouseButton & 1) != 0;
}

inline bool eeMouseDoubleClick1() {
	return (EERIEMouseButton & 4) && !(LastMouseClick & 4);
}

inline bool eeMouseDown2() {
	return (EERIEMouseButton & 2) && !(LastMouseClick & 2);
}

inline bool eeMouseUp2() {
	return !(EERIEMouseButton & 2) && (LastMouseClick & 2);
}

inline bool eeMousePressed2() {
	return (EERIEMouseButton & 2) != 0;
}


class Input {
	
public:
	
	static const std::string KEY_NONE;
	
	Input();
	
	bool init(Window * window);
	void reset();
	
	void update(float time);
	
	static std::string getKeyName(InputKeyId key);
	static InputKeyId getKeyId(const std::string & keyName);
	
	std::string getKeyDisplayName(InputKeyId key);
	
	// Action
	
	bool actionNowPressed(ControlAction actionId) const;
	bool actionPressed(ControlAction actionId) const;
	bool actionNowReleased(ControlAction actionId) const;
	
	// Mouse
	
	void setMouseMode(Mouse::Mode mode);
	
	const Vec2s & getMousePosition() const { return iMouseA; }
	const Vec2f & getRelativeMouseMovement() const { return m_mouseMovement; }
	bool isMouseInWindow() const { return mouseInWindow; }
	void setMousePosAbs(const Vec2s & mousePos);
	
	void setRawMouseInput(bool enabled);
	
	void setMouseSensitivity(int sensitivity);
	void setMouseAcceleration(int acceleration);
	void setInvertMouseY(bool invert);
	
	bool getMouseButton(int buttonId) const;
	int getMouseButtonClicked() const;
	bool getMouseButtonRepeat(int buttonId) const;
	bool getMouseButtonNowPressed(int buttonId) const;
	bool getMouseButtonNowUnPressed(int buttonId) const;
	bool getMouseButtonDoubleClick(int buttonId) const;
	
	int getMouseWheelDir() const { return iWheelDir; }
	
	// Keyboard
	
	int getKeyPressed() const { return iKeyId; }
	bool isAnyKeyPressed() const { return iKeyId >= 0; }
	bool isKeyPressed(int keyId) const;
	bool isKeyPressedNowPressed(int keyId) const;
	bool isKeyPressedNowUnPressed(int keyId) const;
	
	/*!
	 * Enter text input mode and send all text to the given handler
	 *
	 * While text input mode is enabled, all key presses will first be sent to the input handler's
	 * \ref TextInputHandler::keyPressed method. If that method returns \c true, the key press is not
	 * hidden from the rest of the input system.
	 *
	 * Use \ref stopTextInput() to end text input mode.
	 *
	 * This function can be called multiple times in which the box and handler from the last
	 * call will be used. A single \ref stopTextInput() call will end text input mode and will
	 * not restore previous boxes or handlers.
	 *
	 * \param box Rectangle of the text input field (used to position helper windows by input methods)
	 */
	void startTextInput(const Rect & box, TextInputHandler * handler);
	
	/*!
	 * End text input mode
	 */
	void stopTextInput();
	
private:
	
	void centerMouse();
	
	class InputBackend * backend;
	
	// Mouse
	
	bool m_useRawMouseInput;
	Mouse::Mode m_mouseMode;
	Vec2f m_mouseMovement;
	Vec2s iMouseA;
	Vec2s m_lastMousePosition;
	bool  mouseInWindow;
	
	float m_mouseSensitivity;
	int   m_mouseAcceleration;
	bool  m_invertMouseY;
	int   iWheelDir;
	
	bool  bMouseButton[Mouse::ButtonCount];
	bool  bOldMouseButton[Mouse::ButtonCount];
	
	PlatformInstant iMouseTime[Mouse::ButtonCount][2];
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
