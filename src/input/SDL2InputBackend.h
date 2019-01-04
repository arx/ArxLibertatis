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

#ifndef ARX_INPUT_SDL2INPUTBACKEND_H
#define ARX_INPUT_SDL2INPUTBACKEND_H

#include <string>
#include <stddef.h>

#include <SDL.h>

#include "input/InputBackend.h"
#include "input/Keyboard.h"
#include "input/Mouse.h"
#include "input/TextInput.h"
#include "math/Vector.h"
#include "math/Types.h"
#include "platform/Platform.h"
#include "window/SDL2Window.h"

class SDL2InputBackend arx_final : public InputBackend {
	
public:
	
	explicit SDL2InputBackend(SDL2Window * window);
	
	bool update();
	
	// Mouse
	bool setMouseMode(Mouse::Mode mode);
	bool getAbsoluteMouseCoords(int & absX, int & absY) const;
	void setAbsoluteMouseCoords(int absX, int absY);
	void getRelativeMouseCoords(int & relX, int & relY, int & wheelDir) const;
	void getMouseButtonClickCount(int buttonId, int & numClick, int & numUnClick) const;
	
	// Keyboard
	bool isKeyboardKeyPressed(int keyId) const;
	void startTextInput(const Rect & box, TextInputHandler * handler);
	void stopTextInput();
	std::string getKeyName(Keyboard::Key key) const;
	
	void onEvent(const SDL_Event & event);
	
private:
	
	SDL2Window * m_window;
	
	TextInputHandler * m_textHandler;
	std::string m_editText;
	size_t m_editCursorPos;
	size_t m_editCursorLength;
	
	int wheel;
	Vec2i cursorAbs;
	Vec2i cursorRel;
	Vec2i cursorRelAccum;
	bool cursorInWindow;
	bool keyStates[Keyboard::KeyCount];
	size_t clickCount[Mouse::ButtonCount];
	size_t unclickCount[Mouse::ButtonCount];
	
	int currentWheel;
	size_t currentClickCount[Mouse::ButtonCount];
	size_t currentUnclickCount[Mouse::ButtonCount];
	
};

#endif // ARX_INPUT_SDL2INPUTBACKEND_H
