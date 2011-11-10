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

#ifndef ARX_INPUT_SDLINPUTBACKEND_H
#define ARX_INPUT_SDLINPUTBACKEND_H

#include "input/InputBackend.h"
#include "input/Keyboard.h"
#include "input/Mouse.h"
#include "math/Vector2.h"

union SDL_Event;

class SDLInputBackend : public InputBackend {
	
public:
	
	SDLInputBackend();
	~SDLInputBackend();
	
	bool init();
	bool update();
	
	void acquireDevices();
	void unacquireDevices();
	
	// Mouse
	void getMouseCoordinates(int & absX, int & absY, int & wheelDir) const;
	void setMouseCoordinates(int absX, int absY);
	bool isMouseButtonPressed(int buttonId, int & deltaTime) const;
	void getMouseButtonClickCount(int buttonId, int & numClick, int & numUnClick) const;
	
	// Keyboard
	bool isKeyboardKeyPressed(int dikkey) const;
	bool getKeyAsText(int keyId, char& result) const;
	
private:
	
	void onInputEvent(const SDL_Event & event);
	
	int wheel;
	Vec2i cursor;
	bool keyStates[Keyboard::KeyCount];
	bool buttonStates[Mouse::ButtonCount];
	size_t clickCount[Mouse::ButtonCount];
	size_t unclickCount[Mouse::ButtonCount];
	
	int currentWheel;
	size_t currentClickCount[Mouse::ButtonCount];
	size_t currentUnclickCount[Mouse::ButtonCount];
	
	friend class SDLWindow;
};

#endif // ARX_INPUT_SDLINPUTBACKEND_H
