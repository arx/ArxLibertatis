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

#ifndef ARX_INPUT_INPUTBACKEND_H
#define ARX_INPUT_INPUTBACKEND_H

class Window;

#include "input/Keyboard.h"
#include "input/Mouse.h"
#include "math/Types.h"

class TextInputHandler;

class InputBackend {
	
public:
	
	virtual bool update() = 0;
	
	// Mouse
	virtual bool setMouseMode(Mouse::Mode mode) = 0;
	//! return true if the mouse position is currently being updated
	virtual bool getAbsoluteMouseCoords(int & absX, int & absY) const = 0;
	virtual void setAbsoluteMouseCoords(int absX, int absY) = 0;
	virtual void getRelativeMouseCoords(int & relX, int & relY, int & wheelDir) const = 0;
	virtual void getMouseButtonClickCount(int buttonId, int & _iNumClick, int & _iNumUnClick) const = 0;
	
	// Keyboard
	virtual bool isKeyboardKeyPressed(int keyId) const = 0;
	virtual void startTextInput(const Rect & box, TextInputHandler * handler) = 0;
	virtual void stopTextInput() = 0;
	virtual std::string getKeyName(Keyboard::Key key) const = 0;
	
protected:
	
	virtual ~InputBackend() { }
	
};

#endif // ARX_INPUT_INPUTBACKEND_H
