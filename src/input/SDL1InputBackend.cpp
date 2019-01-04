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

#include "input/SDL1InputBackend.h"

#include <boost/range/size.hpp>

#include "io/log/Logger.h"
#include "platform/Platform.h"
#include "util/Unicode.h"

#ifndef SDL_BUTTON_X1
#define SDL_BUTTON_X1 6
#endif
#ifndef SDL_BUTTON_X2
#define SDL_BUTTON_X2 7
#endif

static Keyboard::Key sdlToArxKey[SDLK_LAST];

static Mouse::Button sdlToArxButton[10];

SDL1InputBackend::SDL1InputBackend() : m_textHandler(NULL) {
	
	cursorInWindow = false;
	
	SDL_EventState(SDL_ACTIVEEVENT, SDL_ENABLE);
	SDL_EventState(SDL_KEYDOWN, SDL_ENABLE);
	SDL_EventState(SDL_KEYUP, SDL_ENABLE);
	SDL_EventState(SDL_MOUSEMOTION, SDL_ENABLE);
	SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_ENABLE);
	SDL_EventState(SDL_MOUSEBUTTONUP, SDL_ENABLE);
	
	std::fill_n(sdlToArxKey, ARRAY_SIZE(sdlToArxKey), Keyboard::Key_Invalid);
	
	sdlToArxKey[SDLK_BACKSPACE] = Keyboard::Key_Backspace;
	sdlToArxKey[SDLK_TAB] = Keyboard::Key_Tab;
	sdlToArxKey[SDLK_RETURN] = Keyboard::Key_Enter;
	sdlToArxKey[SDLK_PAUSE] = Keyboard::Key_Pause;
	sdlToArxKey[SDLK_ESCAPE] = Keyboard::Key_Escape;
	sdlToArxKey[SDLK_SPACE] = Keyboard::Key_Spacebar;
	sdlToArxKey[SDLK_EXCLAIM] = Keyboard::Key_1;
	sdlToArxKey[SDLK_QUOTEDBL] = Keyboard::Key_Apostrophe;
	sdlToArxKey[SDLK_HASH] = Keyboard::Key_3;
	sdlToArxKey[SDLK_DOLLAR] = Keyboard::Key_4;
	sdlToArxKey[SDLK_AMPERSAND] = Keyboard::Key_7;
	sdlToArxKey[SDLK_QUOTE] = Keyboard::Key_Apostrophe;
	sdlToArxKey[SDLK_LEFTPAREN] = Keyboard::Key_9;
	sdlToArxKey[SDLK_RIGHTPAREN] = Keyboard::Key_0;
	sdlToArxKey[SDLK_ASTERISK] = Keyboard::Key_8;
	sdlToArxKey[SDLK_PLUS] = Keyboard::Key_Equals;
	sdlToArxKey[SDLK_COMMA] = Keyboard::Key_Comma;
	sdlToArxKey[SDLK_MINUS] = Keyboard::Key_Minus;
	sdlToArxKey[SDLK_PERIOD] = Keyboard::Key_Period;
	sdlToArxKey[SDLK_SLASH] = Keyboard::Key_Slash;
	sdlToArxKey[SDLK_0] = Keyboard::Key_0;
	sdlToArxKey[SDLK_1] = Keyboard::Key_1;
	sdlToArxKey[SDLK_2] = Keyboard::Key_2;
	sdlToArxKey[SDLK_3] = Keyboard::Key_3;
	sdlToArxKey[SDLK_4] = Keyboard::Key_4;
	sdlToArxKey[SDLK_5] = Keyboard::Key_5;
	sdlToArxKey[SDLK_6] = Keyboard::Key_6;
	sdlToArxKey[SDLK_7] = Keyboard::Key_7;
	sdlToArxKey[SDLK_8] = Keyboard::Key_8;
	sdlToArxKey[SDLK_9] = Keyboard::Key_9;
	sdlToArxKey[SDLK_COLON] = Keyboard::Key_Semicolon;
	sdlToArxKey[SDLK_SEMICOLON] = Keyboard::Key_Semicolon;
	sdlToArxKey[SDLK_LESS] = Keyboard::Key_Comma;
	sdlToArxKey[SDLK_EQUALS] = Keyboard::Key_Equals;
	sdlToArxKey[SDLK_GREATER] = Keyboard::Key_Period;
	sdlToArxKey[SDLK_QUESTION] = Keyboard::Key_Slash;
	sdlToArxKey[SDLK_AT] = Keyboard::Key_2;
	sdlToArxKey[SDLK_LEFTBRACKET] = Keyboard::Key_LeftBracket;
	sdlToArxKey[SDLK_BACKSLASH] = Keyboard::Key_Backslash;
	sdlToArxKey[SDLK_RIGHTBRACKET] = Keyboard::Key_RightBracket;
	sdlToArxKey[SDLK_CARET] = Keyboard::Key_6;
	sdlToArxKey[SDLK_UNDERSCORE] = Keyboard::Key_Minus;
	sdlToArxKey[SDLK_BACKQUOTE] = Keyboard::Key_Grave;
	sdlToArxKey[SDLK_a] = Keyboard::Key_A;
	sdlToArxKey[SDLK_b] = Keyboard::Key_B;
	sdlToArxKey[SDLK_c] = Keyboard::Key_C;
	sdlToArxKey[SDLK_d] = Keyboard::Key_D;
	sdlToArxKey[SDLK_e] = Keyboard::Key_E;
	sdlToArxKey[SDLK_f] = Keyboard::Key_F;
	sdlToArxKey[SDLK_g] = Keyboard::Key_G;
	sdlToArxKey[SDLK_h] = Keyboard::Key_H;
	sdlToArxKey[SDLK_i] = Keyboard::Key_I;
	sdlToArxKey[SDLK_j] = Keyboard::Key_J;
	sdlToArxKey[SDLK_k] = Keyboard::Key_K;
	sdlToArxKey[SDLK_l] = Keyboard::Key_L;
	sdlToArxKey[SDLK_m] = Keyboard::Key_M;
	sdlToArxKey[SDLK_n] = Keyboard::Key_N;
	sdlToArxKey[SDLK_o] = Keyboard::Key_O;
	sdlToArxKey[SDLK_p] = Keyboard::Key_P;
	sdlToArxKey[SDLK_q] = Keyboard::Key_Q;
	sdlToArxKey[SDLK_r] = Keyboard::Key_R;
	sdlToArxKey[SDLK_s] = Keyboard::Key_S;
	sdlToArxKey[SDLK_t] = Keyboard::Key_T;
	sdlToArxKey[SDLK_u] = Keyboard::Key_U;
	sdlToArxKey[SDLK_v] = Keyboard::Key_V;
	sdlToArxKey[SDLK_w] = Keyboard::Key_W;
	sdlToArxKey[SDLK_x] = Keyboard::Key_X;
	sdlToArxKey[SDLK_y] = Keyboard::Key_Y;
	sdlToArxKey[SDLK_z] = Keyboard::Key_Z;
	sdlToArxKey[SDLK_DELETE] = Keyboard::Key_Delete;
	sdlToArxKey[SDLK_KP0] = Keyboard::Key_NumPad0;
	sdlToArxKey[SDLK_KP1] = Keyboard::Key_NumPad1;
	sdlToArxKey[SDLK_KP2] = Keyboard::Key_NumPad2;
	sdlToArxKey[SDLK_KP3] = Keyboard::Key_NumPad3;
	sdlToArxKey[SDLK_KP4] = Keyboard::Key_NumPad4;
	sdlToArxKey[SDLK_KP5] = Keyboard::Key_NumPad5;
	sdlToArxKey[SDLK_KP6] = Keyboard::Key_NumPad6;
	sdlToArxKey[SDLK_KP7] = Keyboard::Key_NumPad7;
	sdlToArxKey[SDLK_KP8] = Keyboard::Key_NumPad8;
	sdlToArxKey[SDLK_KP9] = Keyboard::Key_NumPad9;
	sdlToArxKey[SDLK_KP_PERIOD] = Keyboard::Key_NumPoint;
	sdlToArxKey[SDLK_KP_DIVIDE] = Keyboard::Key_NumDivide;
	sdlToArxKey[SDLK_KP_MULTIPLY] = Keyboard::Key_NumMultiply;
	sdlToArxKey[SDLK_KP_MINUS] = Keyboard::Key_NumSubtract;
	sdlToArxKey[SDLK_KP_PLUS] = Keyboard::Key_NumAdd;
	sdlToArxKey[SDLK_KP_ENTER] = Keyboard::Key_NumPadEnter;
	sdlToArxKey[SDLK_KP_EQUALS] = Keyboard::Key_NumPadEnter;
	sdlToArxKey[SDLK_UP] = Keyboard::Key_UpArrow;
	sdlToArxKey[SDLK_DOWN] = Keyboard::Key_DownArrow;
	sdlToArxKey[SDLK_RIGHT] = Keyboard::Key_RightArrow;
	sdlToArxKey[SDLK_LEFT] = Keyboard::Key_LeftArrow;
	sdlToArxKey[SDLK_INSERT] = Keyboard::Key_Insert;
	sdlToArxKey[SDLK_HOME] = Keyboard::Key_Home;
	sdlToArxKey[SDLK_END] = Keyboard::Key_End;
	sdlToArxKey[SDLK_PAGEUP] = Keyboard::Key_PageUp;
	sdlToArxKey[SDLK_PAGEDOWN] = Keyboard::Key_PageDown;
	sdlToArxKey[SDLK_F1] = Keyboard::Key_F1;
	sdlToArxKey[SDLK_F2] = Keyboard::Key_F2;
	sdlToArxKey[SDLK_F3] = Keyboard::Key_F3;
	sdlToArxKey[SDLK_F4] = Keyboard::Key_F4;
	sdlToArxKey[SDLK_F5] = Keyboard::Key_F5;
	sdlToArxKey[SDLK_F6] = Keyboard::Key_F6;
	sdlToArxKey[SDLK_F7] = Keyboard::Key_F7;
	sdlToArxKey[SDLK_F8] = Keyboard::Key_F8;
	sdlToArxKey[SDLK_F9] = Keyboard::Key_F9;
	sdlToArxKey[SDLK_F10] = Keyboard::Key_F10;
	sdlToArxKey[SDLK_F11] = Keyboard::Key_F11;
	sdlToArxKey[SDLK_F12] = Keyboard::Key_F12;
	sdlToArxKey[SDLK_F13] = Keyboard::Key_F13;
	sdlToArxKey[SDLK_F14] = Keyboard::Key_F14;
	sdlToArxKey[SDLK_F15] = Keyboard::Key_F15;
	sdlToArxKey[SDLK_NUMLOCK] = Keyboard::Key_NumLock;
	sdlToArxKey[SDLK_CAPSLOCK] = Keyboard::Key_CapsLock;
	sdlToArxKey[SDLK_SCROLLOCK] = Keyboard::Key_ScrollLock;
	sdlToArxKey[SDLK_RSHIFT] = Keyboard::Key_RightShift;
	sdlToArxKey[SDLK_LSHIFT] = Keyboard::Key_LeftShift;
	sdlToArxKey[SDLK_RCTRL] = Keyboard::Key_RightCtrl;
	sdlToArxKey[SDLK_LCTRL] = Keyboard::Key_LeftCtrl;
	sdlToArxKey[SDLK_RALT] = Keyboard::Key_RightAlt;
	sdlToArxKey[SDLK_LALT] = Keyboard::Key_LeftAlt;
	sdlToArxKey[SDLK_RMETA] = Keyboard::Key_RightWin;
	sdlToArxKey[SDLK_LMETA] = Keyboard::Key_LeftWin;
	sdlToArxKey[SDLK_LSUPER] = Keyboard::Key_RightWin;
	sdlToArxKey[SDLK_RSUPER] = Keyboard::Key_LeftWin;
	sdlToArxKey[SDLK_MODE] = Keyboard::Key_RightAlt;
	sdlToArxKey[SDLK_COMPOSE] = Keyboard::Key_Apps;
	sdlToArxKey[SDLK_PRINT] = Keyboard::Key_PrintScreen;
	
	std::fill_n(sdlToArxButton, ARRAY_SIZE(sdlToArxButton), Mouse::Button_Invalid);
	
	ARX_STATIC_ASSERT(9 < ARRAY_SIZE(sdlToArxButton), "array size mismatch");
	sdlToArxButton[8] = Mouse::Button_5;
	sdlToArxButton[9] = Mouse::Button_6;
	
	ARX_STATIC_ASSERT(SDL_BUTTON_LEFT < ARRAY_SIZE(sdlToArxButton), "array size mismatch");
	sdlToArxButton[SDL_BUTTON_LEFT] = Mouse::Button_0;
	ARX_STATIC_ASSERT(SDL_BUTTON_MIDDLE < ARRAY_SIZE(sdlToArxButton), "array size mismatch");
	sdlToArxButton[SDL_BUTTON_MIDDLE] = Mouse::Button_2;
	ARX_STATIC_ASSERT(SDL_BUTTON_RIGHT < ARRAY_SIZE(sdlToArxButton), "array size mismatch");
	sdlToArxButton[SDL_BUTTON_RIGHT] = Mouse::Button_1;
	ARX_STATIC_ASSERT(SDL_BUTTON_X1 < ARRAY_SIZE(sdlToArxButton), "array size mismatch");
	sdlToArxButton[SDL_BUTTON_X1] = Mouse::Button_3;
	ARX_STATIC_ASSERT(SDL_BUTTON_X2 < ARRAY_SIZE(sdlToArxButton), "array size mismatch");
	sdlToArxButton[SDL_BUTTON_X2] = Mouse::Button_4;
	
	
	wheel = 0;
	cursorAbs = Vec2i(0);
	cursorInWindow = false;
	std::fill_n(keyStates, ARRAY_SIZE(keyStates), false);
	std::fill_n(clickCount, ARRAY_SIZE(clickCount), 0);
	std::fill_n(unclickCount, ARRAY_SIZE(unclickCount), 0);
	
}

bool SDL1InputBackend::update() {
	
	currentWheel = wheel;
	std::copy(clickCount, clickCount + ARRAY_SIZE(clickCount), currentClickCount);
	std::copy(unclickCount, unclickCount + ARRAY_SIZE(unclickCount), currentUnclickCount);
	
	wheel = 0;
	
	std::fill_n(clickCount, ARRAY_SIZE(clickCount), 0);
	std::fill_n(unclickCount, ARRAY_SIZE(unclickCount), 0);
	
	return true;
}

bool SDL1InputBackend::setMouseMode(Mouse::Mode mode) {
	ARX_UNUSED(mode);
	// Raw mouse input is not supported!
	return false;
}

bool SDL1InputBackend::getAbsoluteMouseCoords(int & absX, int & absY) const {
	absX = cursorAbs.x, absY = cursorAbs.y;
	return cursorInWindow;
}

void SDL1InputBackend::setAbsoluteMouseCoords(int absX, int absY) {
	cursorAbs = Vec2i(absX, absY);
	SDL_WarpMouse(absX, absY);
}

void SDL1InputBackend::getRelativeMouseCoords(int & relX, int & relY, int & wheelDir) const {
	relX = 0, relY = 0, wheelDir = currentWheel;
}

void SDL1InputBackend::getMouseButtonClickCount(int buttonId, int & numClick, int & numUnClick) const {
	arx_assert(buttonId >= Mouse::ButtonBase && buttonId < Mouse::ButtonMax);
	size_t i = buttonId - Mouse::ButtonBase;
	numClick = currentClickCount[i], numUnClick = currentUnclickCount[i];
}

bool SDL1InputBackend::isKeyboardKeyPressed(int keyId) const {
	arx_assert(keyId >= Keyboard::KeyBase && keyId < Keyboard::KeyMax);
	return keyStates[keyId - Keyboard::KeyBase];
}

void SDL1InputBackend::startTextInput(const Rect & box, TextInputHandler * handler) {
	ARX_UNUSED(box);
	if(!m_textHandler) {
		SDL_EnableUNICODE(1);
	}
	m_textHandler = handler;
}

void SDL1InputBackend::stopTextInput() {
	if(m_textHandler) {
		SDL_EnableUNICODE(0);
	}
	m_textHandler = NULL;
}

std::string SDL1InputBackend::getKeyName(Keyboard::Key key) const {
	
	arx_assert(key >= Keyboard::KeyBase && key < Keyboard::KeyMax);
	
	for(size_t i = 0; i < size_t(boost::size(sdlToArxKey)); i++) {
		if(sdlToArxKey[i] == key) {
			if(const char * name = SDL_GetKeyName(SDLKey(i))) {
				return name;
			}
		}
	}
	
	return std::string();
}

void SDL1InputBackend::onEvent(const SDL_Event & event) {
	
	switch(event.type) {
		
		case SDL_ACTIVEEVENT: {
			if(event.active.state & SDL_APPMOUSEFOCUS) {
				if(!event.active.gain) {
					cursorInWindow = false;
				}
			}
			break;
		}
		
		case SDL_KEYDOWN: {
			if(m_textHandler && event.key.keysym.unicode >= 0x20 && event.key.keysym.unicode != 0x7f) {
				std::string text;
				text.resize(util::UTF8::length(event.key.keysym.unicode));
				std::string::iterator end = util::UTF8::write(text.begin(), event.key.keysym.unicode);
				arx_assert(end == text.end());
				ARX_UNUSED(end);
				m_textHandler->newText(text);
			}
		}
		// fall-through
		case SDL_KEYUP: {
			SDLKey key = event.key.keysym.sym;
			if(size_t(key) < ARRAY_SIZE(sdlToArxKey) && sdlToArxKey[size_t(key)] != Keyboard::Key_Invalid) {
				Keyboard::Key arxkey = sdlToArxKey[size_t(key)];
				if(m_textHandler && event.key.state == SDL_PRESSED) {
					KeyModifiers mod;
					mod.shift = (event.key.keysym.mod & KMOD_SHIFT) != 0;
					mod.control = (event.key.keysym.mod & KMOD_CTRL) != 0;
					mod.alt = (event.key.keysym.mod & KMOD_ALT) != 0;
					mod.gui = (event.key.keysym.mod & KMOD_META) != 0;
					mod.num = (event.key.keysym.mod & KMOD_NUM) != 0;
					if(m_textHandler->keyPressed(arxkey, mod)) {
						break;
					}
				}
				keyStates[arxkey - Keyboard::KeyBase] = (event.key.state == SDL_PRESSED);
			} else {
				LogWarning << "Unmapped SDL key: " << (int)key << " = " << SDL_GetKeyName(key);
			}
			break;
		}
		
		case SDL_MOUSEMOTION: {
			cursorAbs = Vec2i(event.motion.x, event.motion.y);
			cursorInWindow = true;
			break;
		}
		
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP: {
			Uint8 button = event.button.button;
			if(button == SDL_BUTTON_WHEELUP) {
				wheel++;
			} else if(button == SDL_BUTTON_WHEELDOWN) {
				wheel--;
			} else if(button < ARRAY_SIZE(sdlToArxButton) && sdlToArxButton[button] != Mouse::Button_Invalid) {
				size_t i = sdlToArxButton[button] - Mouse::ButtonBase;
				if(event.button.state == SDL_PRESSED) {
					clickCount[i]++;
				} else {
					unclickCount[i]++;
				}
			} else {
				LogWarning << "Unmapped SDL mouse button: " << (int)button;
			}
			break;
		}
		
	}
	
}
