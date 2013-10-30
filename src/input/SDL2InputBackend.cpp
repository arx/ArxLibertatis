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

#include "input/SDL2InputBackend.h"

#include <boost/static_assert.hpp>

#include "io/log/Logger.h"

static int sdlToArxKey[SDL_NUM_SCANCODES];

static int sdlToArxButton[10];

SDL2InputBackend::SDL2InputBackend(SDL2Window * window) : m_window(window) {
	
	arx_assert(window != NULL);
	
	cursorInWindow = false;
	
	SDL_EventState(SDL_WINDOWEVENT, SDL_ENABLE);
	SDL_EventState(SDL_KEYDOWN, SDL_ENABLE);
	SDL_EventState(SDL_KEYUP, SDL_ENABLE);
	SDL_EventState(SDL_MOUSEMOTION, SDL_ENABLE);
	SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_ENABLE);
	SDL_EventState(SDL_MOUSEBUTTONUP, SDL_ENABLE);
	
	std::fill_n(sdlToArxKey, ARRAY_SIZE(sdlToArxKey), -1);
	
	sdlToArxKey[SDL_SCANCODE_BACKSPACE] = Keyboard::Key_Backspace;
	sdlToArxKey[SDL_SCANCODE_TAB] = Keyboard::Key_Tab;
	sdlToArxKey[SDL_SCANCODE_RETURN] = Keyboard::Key_Enter;
	sdlToArxKey[SDL_SCANCODE_PAUSE] = Keyboard::Key_Pause;
	sdlToArxKey[SDL_SCANCODE_ESCAPE] = Keyboard::Key_Escape;
	sdlToArxKey[SDL_SCANCODE_SPACE] = Keyboard::Key_Spacebar;
	sdlToArxKey[SDL_SCANCODE_COMMA] = Keyboard::Key_Comma;
	sdlToArxKey[SDL_SCANCODE_MINUS] = Keyboard::Key_Minus;
	sdlToArxKey[SDL_SCANCODE_PERIOD] = Keyboard::Key_Period;
	sdlToArxKey[SDL_SCANCODE_SLASH] = Keyboard::Key_Slash;
	sdlToArxKey[SDL_SCANCODE_APOSTROPHE] = Keyboard::Key_Apostrophe;
	sdlToArxKey[SDL_SCANCODE_GRAVE] = Keyboard::Key_Grave;
	sdlToArxKey[SDL_SCANCODE_0] = Keyboard::Key_0;
	sdlToArxKey[SDL_SCANCODE_1] = Keyboard::Key_1;
	sdlToArxKey[SDL_SCANCODE_2] = Keyboard::Key_2;
	sdlToArxKey[SDL_SCANCODE_3] = Keyboard::Key_3;
	sdlToArxKey[SDL_SCANCODE_4] = Keyboard::Key_4;
	sdlToArxKey[SDL_SCANCODE_5] = Keyboard::Key_5;
	sdlToArxKey[SDL_SCANCODE_6] = Keyboard::Key_6;
	sdlToArxKey[SDL_SCANCODE_7] = Keyboard::Key_7;
	sdlToArxKey[SDL_SCANCODE_8] = Keyboard::Key_8;
	sdlToArxKey[SDL_SCANCODE_9] = Keyboard::Key_9;
	sdlToArxKey[SDL_SCANCODE_SEMICOLON] = Keyboard::Key_Semicolon;
	sdlToArxKey[SDL_SCANCODE_EQUALS] = Keyboard::Key_Equals;
	sdlToArxKey[SDL_SCANCODE_LEFTBRACKET] = Keyboard::Key_LeftBracket;
	sdlToArxKey[SDL_SCANCODE_BACKSLASH] = Keyboard::Key_Backslash;
	sdlToArxKey[SDL_SCANCODE_RIGHTBRACKET] = Keyboard::Key_RightBracket;
	sdlToArxKey[SDL_SCANCODE_A] = Keyboard::Key_A;
	sdlToArxKey[SDL_SCANCODE_B] = Keyboard::Key_B;
	sdlToArxKey[SDL_SCANCODE_C] = Keyboard::Key_C;
	sdlToArxKey[SDL_SCANCODE_D] = Keyboard::Key_D;
	sdlToArxKey[SDL_SCANCODE_E] = Keyboard::Key_E;
	sdlToArxKey[SDL_SCANCODE_F] = Keyboard::Key_F;
	sdlToArxKey[SDL_SCANCODE_G] = Keyboard::Key_G;
	sdlToArxKey[SDL_SCANCODE_H] = Keyboard::Key_H;
	sdlToArxKey[SDL_SCANCODE_I] = Keyboard::Key_I;
	sdlToArxKey[SDL_SCANCODE_J] = Keyboard::Key_J;
	sdlToArxKey[SDL_SCANCODE_K] = Keyboard::Key_K;
	sdlToArxKey[SDL_SCANCODE_L] = Keyboard::Key_L;
	sdlToArxKey[SDL_SCANCODE_M] = Keyboard::Key_M;
	sdlToArxKey[SDL_SCANCODE_N] = Keyboard::Key_N;
	sdlToArxKey[SDL_SCANCODE_O] = Keyboard::Key_O;
	sdlToArxKey[SDL_SCANCODE_P] = Keyboard::Key_P;
	sdlToArxKey[SDL_SCANCODE_Q] = Keyboard::Key_Q;
	sdlToArxKey[SDL_SCANCODE_R] = Keyboard::Key_R;
	sdlToArxKey[SDL_SCANCODE_S] = Keyboard::Key_S;
	sdlToArxKey[SDL_SCANCODE_T] = Keyboard::Key_T;
	sdlToArxKey[SDL_SCANCODE_U] = Keyboard::Key_U;
	sdlToArxKey[SDL_SCANCODE_V] = Keyboard::Key_V;
	sdlToArxKey[SDL_SCANCODE_W] = Keyboard::Key_W;
	sdlToArxKey[SDL_SCANCODE_X] = Keyboard::Key_X;
	sdlToArxKey[SDL_SCANCODE_Y] = Keyboard::Key_Y;
	sdlToArxKey[SDL_SCANCODE_Z] = Keyboard::Key_Z;
	sdlToArxKey[SDL_SCANCODE_DELETE] = Keyboard::Key_Delete;
	sdlToArxKey[SDL_SCANCODE_KP_0] = Keyboard::Key_NumPad0;
	sdlToArxKey[SDL_SCANCODE_KP_1] = Keyboard::Key_NumPad1;
	sdlToArxKey[SDL_SCANCODE_KP_2] = Keyboard::Key_NumPad2;
	sdlToArxKey[SDL_SCANCODE_KP_3] = Keyboard::Key_NumPad3;
	sdlToArxKey[SDL_SCANCODE_KP_4] = Keyboard::Key_NumPad4;
	sdlToArxKey[SDL_SCANCODE_KP_5] = Keyboard::Key_NumPad5;
	sdlToArxKey[SDL_SCANCODE_KP_6] = Keyboard::Key_NumPad6;
	sdlToArxKey[SDL_SCANCODE_KP_7] = Keyboard::Key_NumPad7;
	sdlToArxKey[SDL_SCANCODE_KP_8] = Keyboard::Key_NumPad8;
	sdlToArxKey[SDL_SCANCODE_KP_9] = Keyboard::Key_NumPad9;
	sdlToArxKey[SDL_SCANCODE_KP_PERIOD] = Keyboard::Key_NumPoint;
	sdlToArxKey[SDL_SCANCODE_KP_DIVIDE] = Keyboard::Key_NumDivide;
	sdlToArxKey[SDL_SCANCODE_KP_MULTIPLY] = Keyboard::Key_NumMultiply;
	sdlToArxKey[SDL_SCANCODE_KP_MINUS] = Keyboard::Key_NumSubtract;
	sdlToArxKey[SDL_SCANCODE_KP_PLUS] = Keyboard::Key_NumAdd;
	sdlToArxKey[SDL_SCANCODE_KP_ENTER] = Keyboard::Key_NumPadEnter;
	sdlToArxKey[SDL_SCANCODE_KP_EQUALS] = Keyboard::Key_NumPadEnter;
	sdlToArxKey[SDL_SCANCODE_UP] = Keyboard::Key_UpArrow;
	sdlToArxKey[SDL_SCANCODE_DOWN] = Keyboard::Key_DownArrow;
	sdlToArxKey[SDL_SCANCODE_RIGHT] = Keyboard::Key_RightArrow;
	sdlToArxKey[SDL_SCANCODE_LEFT] = Keyboard::Key_LeftArrow;
	sdlToArxKey[SDL_SCANCODE_INSERT] = Keyboard::Key_Insert;
	sdlToArxKey[SDL_SCANCODE_HOME] = Keyboard::Key_Home;
	sdlToArxKey[SDL_SCANCODE_AC_HOME] = Keyboard::Key_Home;
	sdlToArxKey[SDL_SCANCODE_END] = Keyboard::Key_End;
	sdlToArxKey[SDL_SCANCODE_PAGEUP] = Keyboard::Key_PageUp;
	sdlToArxKey[SDL_SCANCODE_PAGEDOWN] = Keyboard::Key_PageDown;
	sdlToArxKey[SDL_SCANCODE_F1] = Keyboard::Key_F1;
	sdlToArxKey[SDL_SCANCODE_F2] = Keyboard::Key_F2;
	sdlToArxKey[SDL_SCANCODE_F3] = Keyboard::Key_F3;
	sdlToArxKey[SDL_SCANCODE_F4] = Keyboard::Key_F4;
	sdlToArxKey[SDL_SCANCODE_F5] = Keyboard::Key_F5;
	sdlToArxKey[SDL_SCANCODE_F6] = Keyboard::Key_F6;
	sdlToArxKey[SDL_SCANCODE_F7] = Keyboard::Key_F7;
	sdlToArxKey[SDL_SCANCODE_F8] = Keyboard::Key_F8;
	sdlToArxKey[SDL_SCANCODE_F9] = Keyboard::Key_F9;
	sdlToArxKey[SDL_SCANCODE_F10] = Keyboard::Key_F10;
	sdlToArxKey[SDL_SCANCODE_F11] = Keyboard::Key_F11;
	sdlToArxKey[SDL_SCANCODE_F12] = Keyboard::Key_F12;
	sdlToArxKey[SDL_SCANCODE_F13] = Keyboard::Key_F13;
	sdlToArxKey[SDL_SCANCODE_F14] = Keyboard::Key_F14;
	sdlToArxKey[SDL_SCANCODE_F15] = Keyboard::Key_F15;
	sdlToArxKey[SDL_SCANCODE_NUMLOCKCLEAR] = Keyboard::Key_NumLock;
	sdlToArxKey[SDL_SCANCODE_CAPSLOCK] = Keyboard::Key_CapsLock;
	sdlToArxKey[SDL_SCANCODE_SCROLLLOCK] = Keyboard::Key_ScrollLock;
	sdlToArxKey[SDL_SCANCODE_RSHIFT] = Keyboard::Key_RightShift;
	sdlToArxKey[SDL_SCANCODE_LSHIFT] = Keyboard::Key_LeftShift;
	sdlToArxKey[SDL_SCANCODE_RCTRL] = Keyboard::Key_RightCtrl;
	sdlToArxKey[SDL_SCANCODE_LCTRL] = Keyboard::Key_LeftCtrl;
	sdlToArxKey[SDL_SCANCODE_RALT] = Keyboard::Key_RightAlt;
	sdlToArxKey[SDL_SCANCODE_LALT] = Keyboard::Key_LeftAlt;
	sdlToArxKey[SDL_SCANCODE_RGUI] = Keyboard::Key_RightWin;
	sdlToArxKey[SDL_SCANCODE_LGUI] = Keyboard::Key_LeftWin;
	sdlToArxKey[SDL_SCANCODE_MODE] = Keyboard::Key_RightAlt;
	sdlToArxKey[SDL_SCANCODE_APPLICATION] = Keyboard::Key_Apps;
	sdlToArxKey[SDL_SCANCODE_PRINTSCREEN] = Keyboard::Key_PrintScreen;
	
	std::fill_n(sdlToArxButton, ARRAY_SIZE(sdlToArxButton), -1);
	
	BOOST_STATIC_ASSERT(9 < ARRAY_SIZE(sdlToArxButton));
	sdlToArxButton[8] = Mouse::Button_5;
	sdlToArxButton[9] = Mouse::Button_6;
	
	BOOST_STATIC_ASSERT(SDL_BUTTON_LEFT < ARRAY_SIZE(sdlToArxButton));
	sdlToArxButton[SDL_BUTTON_LEFT] = Mouse::Button_0;
	BOOST_STATIC_ASSERT(SDL_BUTTON_MIDDLE < ARRAY_SIZE(sdlToArxButton));
	sdlToArxButton[SDL_BUTTON_MIDDLE] = Mouse::Button_2;
	BOOST_STATIC_ASSERT(SDL_BUTTON_RIGHT < ARRAY_SIZE(sdlToArxButton));
	sdlToArxButton[SDL_BUTTON_RIGHT] = Mouse::Button_1;
	BOOST_STATIC_ASSERT(SDL_BUTTON_X1 < ARRAY_SIZE(sdlToArxButton));
	sdlToArxButton[SDL_BUTTON_X1] = Mouse::Button_3;
	BOOST_STATIC_ASSERT(SDL_BUTTON_X2 < ARRAY_SIZE(sdlToArxButton));
	sdlToArxButton[SDL_BUTTON_X2] = Mouse::Button_4;
	
	
	wheel = 0;
	cursorAbs = Vec2i_ZERO;
	lastCursorAbs = Vec2i_ZERO;
	cursorInWindow = false;
	cursorRel = Vec2i_ZERO;
	std::fill_n(keyStates, ARRAY_SIZE(keyStates), false);
	std::fill_n(buttonStates, ARRAY_SIZE(buttonStates), false);
	std::fill_n(clickCount, ARRAY_SIZE(clickCount), 0);
	std::fill_n(unclickCount, ARRAY_SIZE(unclickCount), 0);
	
}

bool SDL2InputBackend::update() {
	
	currentWheel = wheel;
	std::copy(clickCount, clickCount + ARRAY_SIZE(clickCount), currentClickCount);
	std::copy(unclickCount, unclickCount + ARRAY_SIZE(unclickCount), currentUnclickCount);
	
	wheel = 0;
	
	cursorRel = cursorAbs - lastCursorAbs;
	lastCursorAbs = cursorAbs;
	
	std::fill_n(clickCount, ARRAY_SIZE(clickCount), 0);
	std::fill_n(unclickCount, ARRAY_SIZE(unclickCount), 0);
	
	return true;
}

bool SDL2InputBackend::getAbsoluteMouseCoords(int & absX, int & absY) const {
	absX = cursorAbs.x, absY = cursorAbs.y;
	return cursorInWindow;
}

void SDL2InputBackend::setAbsoluteMouseCoords(int absX, int absY) {
	lastCursorAbs = cursorAbs = Vec2i(absX, absY);
	SDL_WarpMouseInWindow(m_window->m_window, absX, absY);
}

void SDL2InputBackend::getRelativeMouseCoords(int & relX, int & relY, int & wheelDir) const {
	relX = cursorRel.x, relY = cursorRel.y, wheelDir = currentWheel;
}

bool SDL2InputBackend::isMouseButtonPressed(int buttonId, int & deltaTime) const  {
	arx_assert(buttonId >= Mouse::ButtonBase && buttonId < Mouse::ButtonMax);
	deltaTime = 0; // TODO
	return buttonStates[buttonId - Mouse::ButtonBase];
}

void SDL2InputBackend::getMouseButtonClickCount(int buttonId, int & numClick, int & numUnClick) const {
	arx_assert(buttonId >= Mouse::ButtonBase && buttonId < Mouse::ButtonMax);
	size_t i = buttonId - Mouse::ButtonBase;
	numClick = currentClickCount[i], numUnClick = currentUnclickCount[i];
}

bool SDL2InputBackend::isKeyboardKeyPressed(int keyId) const {
	arx_assert(keyId >= Keyboard::KeyBase && keyId < Keyboard::KeyMax);
	return keyStates[keyId - Keyboard::KeyBase];
}

void SDL2InputBackend::onEvent(const SDL_Event & event) {
	
	switch(event.type) {
		
		case SDL_WINDOWEVENT: {
			if(event.window.event == SDL_WINDOWEVENT_ENTER) {
				cursorInWindow = true;
			} else if(event.window.event == SDL_WINDOWEVENT_LEAVE) {
				cursorInWindow = false;
			}
			break;
		}
		
		case SDL_KEYDOWN:
		case SDL_KEYUP: {
			SDL_Scancode key = event.key.keysym.scancode;
			if(key >= 0 && size_t(key) < ARRAY_SIZE(sdlToArxKey) && sdlToArxKey[key] >= 0) {
				keyStates[sdlToArxKey[key] - Keyboard::KeyBase] = (event.key.state == SDL_PRESSED);
			} else {
				LogWarning << "Unmapped SDL key: " << (int)key << " = " << SDL_GetScancodeName(key);
			}
			break;
		}
		
		case SDL_MOUSEMOTION: {
			cursorAbs = Vec2i(event.motion.x, event.motion.y);
			cursorInWindow = true;
			break;
		}
		
		case SDL_MOUSEWHEEL: {
			wheel += event.wheel.y;
		}
		
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP: {
			Uint8 button = event.button.button;
			if(button < ARRAY_SIZE(sdlToArxButton) && sdlToArxButton[button] >= 0) {
				size_t i = sdlToArxButton[button] - Mouse::ButtonBase;
				if((event.button.state == SDL_PRESSED)) {
					buttonStates[i] = true, clickCount[i]++;
				} else {
					buttonStates[i] = false, unclickCount[i]++;
				}
			} else if(button != 0) {
				LogWarning << "Unmapped SDL mouse button: " << (int)button;
			}
			break;
		}
		
	}
	
}
