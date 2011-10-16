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

#ifndef ARX_INPUT_INPUTBACKEND_H
#define ARX_INPUT_INPUTBACKEND_H

class InputBackend {
public:
	virtual ~InputBackend() {}

	virtual bool init() = 0;
	virtual void acquireDevices() = 0;
	virtual void unacquireDevices() = 0;

	virtual bool update() = 0;	

	// Mouse 
	virtual void getMouseCoordinates(int & absX, int & absY, int & wheelDir) const = 0;
	virtual void setMouseCoordinates(int absX, int absY) = 0;
	virtual bool isMouseButtonPressed(int buttonId, int & _iDeltaTime) const = 0;
	virtual void getMouseButtonClickCount(int buttonId, int & _iNumClick, int & _iNumUnClick) const = 0;

	// Keyboard
	virtual bool isKeyboardKeyPressed(int keyId) const = 0;
	virtual bool getKeyAsText(int keyId, char& result) const = 0;
};

#endif // ARX_INPUT_INPUTBACKEND_H
