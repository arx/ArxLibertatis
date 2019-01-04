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

#include "input/Input.h"

#include <string>
#include <map>
#include <cmath>

#include <boost/lexical_cast.hpp>
#include <boost/static_assert.hpp>

#include "core/Application.h"
#include "core/Config.h"
#include "core/GameTime.h"
#include "graphics/Math.h"
#include "gui/Menu.h"
#include "input/InputBackend.h"
#include "io/log/Logger.h"
#include "window/RenderWindow.h"

Input * GInput = NULL;

long EERIEMouseButton = 0;
long LastMouseClick = 0;

struct KeyDescription {
	InputKeyId id;
	const char * name;
};

// All standard keys
// "+" should not appear in names as it is used as a separator
static const KeyDescription keysDescriptions[] = {
	{ Keyboard::Key_0, "0" },
	{ Keyboard::Key_1, "1" },
	{ Keyboard::Key_2, "2" },
	{ Keyboard::Key_3, "3" },
	{ Keyboard::Key_4, "4" },
	{ Keyboard::Key_5, "5" },
	{ Keyboard::Key_6, "6" },
	{ Keyboard::Key_7, "7" },
	{ Keyboard::Key_8, "8" },
	{ Keyboard::Key_9, "9" },
	{ Keyboard::Key_A, "A" },
	{ Keyboard::Key_B, "B" },
	{ Keyboard::Key_C, "C" },
	{ Keyboard::Key_D, "D" },
	{ Keyboard::Key_E, "E" },
	{ Keyboard::Key_F, "F" },
	{ Keyboard::Key_G, "G" },
	{ Keyboard::Key_H, "H" },
	{ Keyboard::Key_I, "I" },
	{ Keyboard::Key_J, "J" },
	{ Keyboard::Key_K, "K" },
	{ Keyboard::Key_L, "L" },
	{ Keyboard::Key_M, "M" },
	{ Keyboard::Key_N, "N" },
	{ Keyboard::Key_O, "O" },
	{ Keyboard::Key_P, "P" },
	{ Keyboard::Key_Q, "Q" },
	{ Keyboard::Key_R, "R" },
	{ Keyboard::Key_S, "S" },
	{ Keyboard::Key_T, "T" },
	{ Keyboard::Key_U, "U" },
	{ Keyboard::Key_V, "V" },
	{ Keyboard::Key_W, "W" },
	{ Keyboard::Key_X, "X" },
	{ Keyboard::Key_Y, "Y" },
	{ Keyboard::Key_Z, "Z" },
	{ Keyboard::Key_F1, "F1" },
	{ Keyboard::Key_F2, "F2" },
	{ Keyboard::Key_F3, "F3" },
	{ Keyboard::Key_F4, "F4" },
	{ Keyboard::Key_F5, "F5" },
	{ Keyboard::Key_F6, "F6" },
	{ Keyboard::Key_F7, "F7" },
	{ Keyboard::Key_F8, "F8" },
	{ Keyboard::Key_F9, "F9" },
	{ Keyboard::Key_F10, "F10" },
	{ Keyboard::Key_F11, "F11" },
	{ Keyboard::Key_F12, "F12" },
	{ Keyboard::Key_F13, "F13" },
	{ Keyboard::Key_F14, "F14" },
	{ Keyboard::Key_F15, "F15" },
	{ Keyboard::Key_F16, "F16" },
	{ Keyboard::Key_F17, "F17" },
	{ Keyboard::Key_F18, "F18" },
	{ Keyboard::Key_F19, "F19" },
	{ Keyboard::Key_F20, "F20" },
	{ Keyboard::Key_F21, "F21" },
	{ Keyboard::Key_F22, "F22" },
	{ Keyboard::Key_F23, "F23" },
	{ Keyboard::Key_F24, "F24" },
	{ Keyboard::Key_UpArrow, "Up" },
	{ Keyboard::Key_DownArrow, "Down" },
	{ Keyboard::Key_LeftArrow, "Left" },
	{ Keyboard::Key_RightArrow, "Right" },
	{ Keyboard::Key_Home, "Home" },
	{ Keyboard::Key_End, "End" },
	{ Keyboard::Key_PageUp, "PageUp" },
	{ Keyboard::Key_PageDown, "PageDown" },
	{ Keyboard::Key_Insert, "Insert" },
	{ Keyboard::Key_Delete, "Delete" },
	{ Keyboard::Key_Escape, "Escape" },
	{ Keyboard::Key_NumLock, "NumLock" },
	{ Keyboard::Key_NumPad0, "Numpad0" },
	{ Keyboard::Key_NumPad1, "Numpad1" },
	{ Keyboard::Key_NumPad2, "Numpad2" },
	{ Keyboard::Key_NumPad3, "Numpad3" },
	{ Keyboard::Key_NumPad4, "Numpad4" },
	{ Keyboard::Key_NumPad5, "Numpad5" },
	{ Keyboard::Key_NumPad6, "Numpad6" },
	{ Keyboard::Key_NumPad7, "Numpad7" },
	{ Keyboard::Key_NumPad8, "Numpad8" },
	{ Keyboard::Key_NumPad9, "Numpad9" },
	{ Keyboard::Key_NumPadEnter, "NumpadReturn" },
	{ Keyboard::Key_NumSubtract, "Numpad-" },
	{ Keyboard::Key_NumAdd, "NumpadPlus" },
	{ Keyboard::Key_NumMultiply, "Multiply" },
	{ Keyboard::Key_NumDivide, "Numpad/" },
	{ Keyboard::Key_NumPoint, "Numpad." },
	{ Keyboard::Key_NumComma, "Numpad," },
	{ Keyboard::Key_Num00, "Numpad00" },
	{ Keyboard::Key_Num000, "Numpad000" },
	{ Keyboard::Key_NumLeftParen, "Numpad(" },
	{ Keyboard::Key_NumRightParen, "Numpad)" },
	{ Keyboard::Key_NumLeftBrace, "Numpad{" },
	{ Keyboard::Key_NumRightBrace, "Numpad}" },
	{ Keyboard::Key_NumTab, "NumpadTab" },
	{ Keyboard::Key_NumBackspace, "NumpadBackspace" },
	{ Keyboard::Key_NumA, "NumpadA" },
	{ Keyboard::Key_NumB, "NumpadB" },
	{ Keyboard::Key_NumC, "NumpadC" },
	{ Keyboard::Key_NumD, "NumpadD" },
	{ Keyboard::Key_NumE, "NumpadE" },
	{ Keyboard::Key_NumF, "NumpadF" },
	{ Keyboard::Key_NumXor, "NumpadXor" },
	{ Keyboard::Key_NumPower, "NumpadPower" },
	{ Keyboard::Key_NumPercent, "Numpad%" },
	{ Keyboard::Key_NumLess, "Numpad<" },
	{ Keyboard::Key_NumGreater, "Numpad>" },
	{ Keyboard::Key_NumAmpersand, "Numpad&" },
	{ Keyboard::Key_NumDblAmpersand, "Numpad&&" },
	{ Keyboard::Key_NumVerticalBar, "Numpad|" },
	{ Keyboard::Key_NumDblVerticalBar, "Numpad||" },
	{ Keyboard::Key_NumColon, "Numpad:" },
	{ Keyboard::Key_NumHash, "Numpad#" },
	{ Keyboard::Key_NumSpace, "NumpadSpace" },
	{ Keyboard::Key_NumAt, "Numpad@" },
	{ Keyboard::Key_NumExclam, "Numpad!" },
	{ Keyboard::Key_NumMemStore, "NumpadMemStore" },
	{ Keyboard::Key_NumMemRecall, "NumpadMemRecall" },
	{ Keyboard::Key_NumMemClear, "NumpadMemClear" },
	{ Keyboard::Key_NumMemAdd, "NumpadMemAdd" },
	{ Keyboard::Key_NumMemSubtract, "NumpadMemSubtract" },
	{ Keyboard::Key_NumMemMultiply, "NumpadMemMultiply" },
	{ Keyboard::Key_NumMemDivide, "NumpadMemDivide" },
	{ Keyboard::Key_NumPlusMinus, "NumpadPlusMinus" },
	{ Keyboard::Key_NumClear, "NumpadClear" },
	{ Keyboard::Key_NumClearEntry, "NumpadClearEntry" },
	{ Keyboard::Key_NumBinary, "NumBinary" },
	{ Keyboard::Key_NumOctal, "NumOctal" },
	{ Keyboard::Key_NumDecimal, "NumDecimal" },
	{ Keyboard::Key_NumHexadecimal, "NumHexadecimal" },
	{ Keyboard::Key_LeftBracket, "[" },
	{ Keyboard::Key_LeftCtrl, "LeftControl" },
	{ Keyboard::Key_LeftAlt, "LeftAlt" },
	{ Keyboard::Key_LeftShift, "LeftShift" },
	{ Keyboard::Key_LeftWin, "LeftStart" },
	{ Keyboard::Key_RightBracket, "]" },
	{ Keyboard::Key_RightCtrl, "RightControl" },
	{ Keyboard::Key_RightAlt, "RightAlt" },
	{ Keyboard::Key_RightShift, "RightShift" },
	{ Keyboard::Key_RightWin, "RightStart" },
	{ Keyboard::Key_PrintScreen, "PrintScreen" },
	{ Keyboard::Key_ScrollLock, "ScrollLock" },
	{ Keyboard::Key_Pause, "Pause" },
	{ Keyboard::Key_Spacebar, "Space" },
	{ Keyboard::Key_Backspace, "Backspace" },
	{ Keyboard::Key_Enter, "Return" },
	{ Keyboard::Key_Tab, "Tab" },
	{ Keyboard::Key_Apps, "AppMenu" },
	{ Keyboard::Key_CapsLock, "Capital" },
	{ Keyboard::Key_Slash, "/" },
	{ Keyboard::Key_Backslash, "Backslash" },
	{ Keyboard::Key_Comma, "," },
	{ Keyboard::Key_Semicolon, ";" },
	{ Keyboard::Key_Period, "." },
	{ Keyboard::Key_Grave, "`" },
	{ Keyboard::Key_Apostrophe, "'" },
	{ Keyboard::Key_Minus, "-" },
	{ Keyboard::Key_Equals, "=" },
	{ Keyboard::Key_Execute, "Execute" },
	{ Keyboard::Key_Help, "Help" },
	{ Keyboard::Key_Menu, "Menu" },
	{ Keyboard::Key_Select, "Select" },
	{ Keyboard::Key_Stop, "Stop" },
	{ Keyboard::Key_Redo, "Redo" },
	{ Keyboard::Key_Undo, "Undo" },
	{ Keyboard::Key_Cut, "Cut" },
	{ Keyboard::Key_Copy, "Copy" },
	{ Keyboard::Key_Paste, "Paste" },
	{ Keyboard::Key_Find, "Find" },
	{ Keyboard::Key_Mute, "Mute" },
	{ Keyboard::Key_VolumeUp, "VolumeUp" },
	{ Keyboard::Key_VolumeDown, "VolumeDown" },
	{ Keyboard::Key_International1, "International1" },
	{ Keyboard::Key_International2, "International2" },
	{ Keyboard::Key_International3, "International3" },
	{ Keyboard::Key_International4, "International4" },
	{ Keyboard::Key_International5, "International5" },
	{ Keyboard::Key_International6, "International6" },
	{ Keyboard::Key_International7, "International7" },
	{ Keyboard::Key_International8, "International8" },
	{ Keyboard::Key_International9, "International9" },
	{ Keyboard::Key_Lang1, "Lang1" },
	{ Keyboard::Key_Lang2, "Lang2" },
	{ Keyboard::Key_Lang3, "Lang3" },
	{ Keyboard::Key_Lang4, "Lang4" },
	{ Keyboard::Key_Lang5, "Lang5" },
	{ Keyboard::Key_Lang6, "Lang6" },
	{ Keyboard::Key_Lang7, "Lang7" },
	{ Keyboard::Key_Lang8, "Lang8" },
	{ Keyboard::Key_Lang9, "Lang9" },
	{ Keyboard::Key_AltErase, "AltErase" },
	{ Keyboard::Key_SysReq, "SysReq" },
	{ Keyboard::Key_Cancel, "Cancel" },
	{ Keyboard::Key_Clear, "Clear" },
	{ Keyboard::Key_Prior, "Prior" },
	{ Keyboard::Key_Return2, "Return2" },
	{ Keyboard::Key_Separator, "Separator" },
	{ Keyboard::Key_Out, "Out" },
	{ Keyboard::Key_Oper, "Oper" },
	{ Keyboard::Key_ClearAgain, "ClearAgain" },
	{ Keyboard::Key_CrSel, "CrSel" },
	{ Keyboard::Key_ExSel, "ExSel" },
	{ Keyboard::Key_ThousandsSeparator, "ThousandsSeparator" },
	{ Keyboard::Key_DecimalSeparator, "DecimalSeparator" },
	{ Keyboard::Key_CurrencyUnit, "CurrencyUnit" },
	{ Keyboard::Key_CurrencySubUnit, "CurrencySubUnit" },
	{ Keyboard::Key_AudioNext, "AudioNext" },
	{ Keyboard::Key_AudioPrev, "AudioPrev" },
	{ Keyboard::Key_AudioStop, "AudioStop" },
	{ Keyboard::Key_AudioPlay, "AudioPlay" },
	{ Keyboard::Key_AudioMute, "AudioMute" },
	{ Keyboard::Key_Media, "Media" },
	{ Keyboard::Key_WWW, "WWW" },
	{ Keyboard::Key_Mail, "Mail" },
	{ Keyboard::Key_Calculator, "Calculator" },
	{ Keyboard::Key_Computer, "Computer" },
	{ Keyboard::Key_ACSearch, "ACSearch" },
	{ Keyboard::Key_ACHome, "ACHome" },
	{ Keyboard::Key_ACBack, "ACBack" },
	{ Keyboard::Key_ACForward, "ACForward" },
	{ Keyboard::Key_ACStop, "ACStop" },
	{ Keyboard::Key_ACRefresh, "ACRefresh" },
	{ Keyboard::Key_ACBookmarks, "ACBookmarks" },
};

static const std::string PREFIX_KEY = "Key_";
static const std::string PREFIX_BUTTON = "Button";
static const char SEPARATOR = '+';
const std::string Input::KEY_NONE = "---";

bool ARX_INPUT_Init(Window * window) {
	GInput = new Input();
	
	bool ret = GInput->init(window);
	if(!ret) {
		delete GInput;
		GInput = NULL;
	}

	return ret;
}

void ARX_INPUT_Release() {
	delete GInput;
	GInput = NULL;
}

Input::Input()
	: backend(NULL)
	, m_useRawMouseInput(true)
	, m_mouseMode(Mouse::Absolute)
	, m_mouseMovement(0.f)
	, iMouseA(0)
	, m_lastMousePosition(0)
	, mouseInWindow(false)
	, m_mouseSensitivity(0.f)
	, m_mouseAcceleration(0)
	, m_invertMouseY(false)
	, iWheelDir(0)
	, iKeyId(-1)
{
	setMouseSensitivity(2);
	reset();
}

bool Input::init(Window * window) {
	arx_assert(backend == NULL);
	
	backend = window->getInputBackend();
	if(backend == NULL) {
		return false;
	}
	
	int x, y;
	backend->getAbsoluteMouseCoords(x, y);
	m_lastMousePosition = Vec2s(x, y);
	
	return true;
}

void Input::reset() {
	
	m_mouseMovement = Vec2f(0.f);

	for(size_t i = 0; i < Mouse::ButtonCount; i++) {
		iMouseTime[i][0] = 0;
		iMouseTime[i][1] = 0;
		iMouseTimeSet[i] = 0;
		bMouseButton[i] = bOldMouseButton[i] = false;
		iOldNumClick[i] = 0;
	}

	iKeyId = -1;
	for(int i = 0; i < Keyboard::KeyCount; i++) {
		keysStates[i] = 0;
	}
	
	EERIEMouseButton = 0;
	iWheelDir = 0;
	
}

void Input::setMouseMode(Mouse::Mode mode) {
	
	if(m_mouseMode == mode) {
		return;
	}
	
	m_mouseMode = mode;
	
	if(backend && m_useRawMouseInput) {
		if(!backend->setMouseMode(mode)) {
			m_useRawMouseInput = false;
		}
	}
	
	if(m_mouseMode == Mouse::Absolute) {
		centerMouse();
	}
	
}

void Input::setRawMouseInput(bool enabled) {
	
	if(m_useRawMouseInput == enabled) {
		return;
	}
	
	m_useRawMouseInput = enabled;
	
	if(backend && m_mouseMode == Mouse::Relative) {
		if(!backend->setMouseMode(enabled ? Mouse::Relative : Mouse::Absolute)) {
			m_useRawMouseInput = false;
		}
	}
	
}

void Input::centerMouse() {
	setMousePosAbs(Vec2s(mainApp->getWindow()->getSize() / s32(2)));
}

void Input::setMousePosAbs(const Vec2s & mousePos) {
	
	if(backend) {
		backend->setAbsoluteMouseCoords(mousePos.x, mousePos.y);
	}
	
	m_lastMousePosition = iMouseA = mousePos;
}

void Input::update(float time) {

	backend->update();

	bool keyJustPressed = false;
	iKeyId = -1;
	int modifier = 0;

	for(int i = 0; i < Keyboard::KeyCount; i++) {
		if(isKeyPressed(i)) {
			
			if(Keyboard::isModifier(i)) {
				modifier = i;
			}

			if(keysStates[i] < 2) {
				keysStates[i]++;
			}

			if(!keyJustPressed) {
				if(keysStates[i] == 1) {
					iKeyId = i;
					keyJustPressed = true;
				} else {
					iKeyId = i;
				}
			}
		} else {
			if(keysStates[i] > 0) {
				keysStates[i]--;
			}
		}
	}
	
	if(modifier != 0 && iKeyId != modifier) {
		iKeyId |= (modifier << 16);
	}
	
	if(Keyboard::isModifier(iKeyId)) {
		for(int i = 0; i < Keyboard::KeyCount; i++) {
			if(!Keyboard::isModifier(i & 0xFFFF) && keysStates[i]) {
				iKeyId &= ~0xFFFF;
				iKeyId |= i;
			}
		}
	}
	
	const PlatformInstant now = g_platformTime.frameStart();
	
	for(int buttonId = Mouse::ButtonBase; buttonId < Mouse::ButtonMax; buttonId++) {
		int i = buttonId - Mouse::ButtonBase;
		
		int iNumClick;
		int iNumUnClick;
		backend->getMouseButtonClickCount(buttonId, iNumClick, iNumUnClick);
		
		iOldNumClick[i] += iNumClick + iNumUnClick;
		
		if(!bMouseButton[i] && iOldNumClick[i] == iNumUnClick) {
			iOldNumClick[i] = 0;
		}
		
		bOldMouseButton[i] = bMouseButton[i];
		
		if(bMouseButton[i]) {
			if(iOldNumClick[i]) {
				bMouseButton[i] = false;
			}
		} else {
			if(iOldNumClick[i]) {
				bMouseButton[i] = true;
			}
		}
		
		if(iOldNumClick[i]) {
			iOldNumClick[i]--;
		}
		
		if(iMouseTimeSet[i] > 1 || (iMouseTimeSet[i] == 1 && now - iMouseTime[i][0] > PlatformDurationMs(300))) {
			iMouseTime[i][0] = 0;
			iMouseTime[i][1] = 0;
			iMouseTimeSet[i] = 0;
		}
		
		if(getMouseButtonNowPressed(buttonId)) {
			switch(iMouseTimeSet[i]) {
			case 0:
				iMouseTime[i][0] = now;
				iMouseTimeSet[i]++;
				break;
			case 1:
				iMouseTime[i][1] = now;
				iMouseTimeSet[i]++;
				break;
			}
		}
	}
	
	// Get the new coordinates
	int absX, absY;
	mouseInWindow = backend->getAbsoluteMouseCoords(absX, absY);
	Vec2s newMousePosition(absX, absY);
	
	Vec2i wndSize = mainApp->getWindow()->getSize();
	if(absX >= 0 && absX < wndSize.x && absY >= 0 && absY < wndSize.y) {
		
		// Use the absolute mouse position reported by the backend, as is
		if(m_mouseMode == Mouse::Absolute) {
			iMouseA = newMousePosition;
		} else {
			iMouseA = wndSize / s32(2);
		}
		
	} else {
		mouseInWindow = false;
	}
	
	int relX, relY;
	backend->getRelativeMouseCoords(relX, relY, iWheelDir);
	
	if(m_mouseMode == Mouse::Relative) {
		
		if(m_useRawMouseInput) {
			m_mouseMovement = Vec2f(relX * 2, relY * 2);
		} else {
			m_mouseMovement = Vec2f(newMousePosition - m_lastMousePosition);
			if(newMousePosition != m_lastMousePosition) {
				centerMouse();
			} else {
				m_lastMousePosition = newMousePosition;
			}
		}
		
		// Use the sensitivity config value to adjust relative mouse mouvements
		m_mouseMovement *= m_mouseSensitivity;
		
		if(m_mouseAcceleration > 0 && time > 0.0f) {
			Vec2f speed = m_mouseMovement / (time * 0.14f);
			Vec2f sign(speed.x < 0 ? -1.f : 1.f, speed.y < 0 ? -1.f : 1.f);
			float exponent = 1.f + m_mouseAcceleration * 0.05f;
			speed.x = (std::pow(speed.x * sign.x + 1.f, exponent) - 1.f) * sign.x;
			speed.y = (std::pow(speed.y * sign.y + 1.f, exponent) - 1.f) * sign.y;
			m_mouseMovement = speed * (time * 0.14f);
		}
		
		if(m_invertMouseY) {
			m_mouseMovement.y *= -1.f;
		}
		
		if(!mouseInWindow) {
			LogWarning << "Cursor escaped the window while in relative input mode";
			centerMouse();
		}
		
	} else {
		m_mouseMovement = Vec2f(0.f);
		if(!m_useRawMouseInput) {
			m_lastMousePosition = newMousePosition;
		}
	}
	
	
}

static std::map<std::string, InputKeyId> keyNames;

std::string Input::getKeyName(InputKeyId key) {
	
	if(key == -1) {
		return std::string();
	}
	
	std::string name;
	
	std::string modifier;
	if(key & INPUT_COMBINATION_MASK) {
		// key combination
		modifier = getKeyName((key >> 16) & 0x0fff);
		key &= INPUT_MASK;
	}
	
	if(key >= InputKeyId(Mouse::ButtonBase) && key < InputKeyId(Mouse::ButtonMax)) {
		std::ostringstream oss;
		oss << PREFIX_BUTTON << int(key - Mouse::ButtonBase + 1);
		name = oss.str();
	} else if(key == InputKeyId(Mouse::Wheel_Up)) {
		name = "WheelUp";
	} else if(key == InputKeyId(Mouse::Wheel_Down)) {
		name = "WheelDown";
	} else if(key >= InputKeyId(Keyboard::KeyBase) && key < InputKeyId(Keyboard::KeyMax)) {
		BOOST_STATIC_ASSERT(ARRAY_SIZE(keysDescriptions) == size_t(Keyboard::KeyMax - Keyboard::KeyBase));
		const KeyDescription & entity = keysDescriptions[key - InputKeyId(Keyboard::KeyBase)];
		arx_assert(entity.id == key);
		name = entity.name;
	} else {
		arx_unreachable();
	}
	
	if(!modifier.empty()) {
		return modifier + SEPARATOR + name;
	} else {
		return name;
	}
	
}

std::string Input::getKeyDisplayName(InputKeyId key) {
	
	if(key == -1) {
		return std::string();
	}
	
	std::string name;
	
	std::string modifier;
	if(key & INPUT_COMBINATION_MASK) {
		// key combination
		modifier = getKeyDisplayName((key >> 16) & 0x0fff);
		key &= INPUT_MASK;
	}
	
	if(key == InputKeyId(Mouse::Button_0)) {
		name = "Left Click";
	} else if(key == InputKeyId(Mouse::Button_2)) {
		name = "Middle Click";
	} else if(key == InputKeyId(Mouse::Button_1)) {
		name = "Right Click";
	} else if(key >= InputKeyId(Mouse::ButtonBase) && key < InputKeyId(Mouse::ButtonMax)) {
		std::ostringstream oss;
		oss << "Mouse " << int(key - Mouse::ButtonBase + 1);
		name = oss.str();
	} else if(key == InputKeyId(Mouse::Wheel_Up)) {
		name = "Wheel Up";
	} else if(key == InputKeyId(Mouse::Wheel_Down)) {
		name = "Wheel Down";
	} else if(key >= InputKeyId(Keyboard::KeyBase) && key < InputKeyId(Keyboard::KeyMax)) {
		name = backend->getKeyName(Keyboard::Key(key));
		if(name.empty()) {
			BOOST_STATIC_ASSERT(ARRAY_SIZE(keysDescriptions) == size_t(Keyboard::KeyMax - Keyboard::KeyBase));
			const KeyDescription & entity = keysDescriptions[key - InputKeyId(Keyboard::KeyBase)];
			arx_assert(entity.id == key);
			name = entity.name;
		}
	} else {
		arx_unreachable();
	}
	
	if(!modifier.empty()) {
		return modifier + " + " + name;
	} else {
		return name;
	}
	
}

InputKeyId Input::getKeyId(const std::string & name) {
	
	// If a noneset key, return -1
	if(name.empty() || name == KEY_NONE) {
		return -1;
	}
	
	size_t sep = name.find(SEPARATOR);
	if(sep != std::string::npos) {
		InputKeyId modifier = getKeyId(name.substr(0, sep));
		InputKeyId key = getKeyId(name.substr(sep + 1));
		return (modifier < 0) ? key : (modifier << 16 | key);
	}
	
	if(!name.compare(0, PREFIX_KEY.length(), PREFIX_KEY)) {
		try {
			int key = boost::lexical_cast<int>(name.substr(PREFIX_KEY.length()));
			return key;
		} catch(const boost::bad_lexical_cast &) { }
	}
	
	if(!name.compare(0, PREFIX_BUTTON.length(), PREFIX_BUTTON)) {
		try {
			int key = boost::lexical_cast<int>(name.substr(PREFIX_BUTTON.length()));
			if(key >= 0 && key < Mouse::ButtonCount) {
				return Mouse::ButtonBase + key - 1;
			}
		} catch(const boost::bad_lexical_cast &) { }
	}
	
	if(keyNames.empty()) {
		// Initialize the key name -> id map.
		for(size_t i = 0; i < ARRAY_SIZE(keysDescriptions); i++) {
			keyNames[keysDescriptions[i].name] = keysDescriptions[i].id;
		}
		keyNames["WheelUp"] = InputKeyId(Mouse::Wheel_Up);
		keyNames["WheelDown"] = InputKeyId(Mouse::Wheel_Down);
	}
	
	std::map<std::string, InputKeyId>::const_iterator it = keyNames.find(name);
	if(it != keyNames.end()) {
		return it->second;
	}
	
	return -1;
}

void Input::setMouseSensitivity(int sensitivity) {
	const float maxExponent = 1.f / 6.f;
	const float minExponent = 2.f;
	float exponent = (maxExponent - minExponent) * float(sensitivity) * 0.1f + minExponent;
	m_mouseSensitivity = std::pow(0.7f, exponent) * (float(sensitivity) + 1.f) * 0.04f;
}

void Input::setMouseAcceleration(int acceleration) {
	m_mouseAcceleration = acceleration;
}

void Input::setInvertMouseY(bool invert) {
	m_invertMouseY = invert;
}

bool Input::isKeyPressed(int keyId) const {
	arx_assert(keyId >= Keyboard::KeyBase && keyId < Keyboard::KeyMax);

	return backend->isKeyboardKeyPressed(keyId);
}

bool Input::isKeyPressedNowPressed(int keyId) const {
	arx_assert(keyId >= Keyboard::KeyBase && keyId < Keyboard::KeyMax);

	return backend->isKeyboardKeyPressed(keyId) && (keysStates[keyId] == 1);
}

bool Input::isKeyPressedNowUnPressed(int keyId) const {
	arx_assert(keyId >= Keyboard::KeyBase && keyId < Keyboard::KeyMax);

	return !backend->isKeyboardKeyPressed(keyId) && (keysStates[keyId] == 1);
}

void Input::startTextInput(const Rect & box, TextInputHandler * handler) {
	backend->startTextInput(box, handler);
}

void Input::stopTextInput() {
	backend->stopTextInput();
}

bool Input::getMouseButton(int buttonId) const {
	arx_assert(buttonId >= Mouse::ButtonBase && buttonId < Mouse::ButtonMax);

	int buttonIdx = buttonId - Mouse::ButtonBase;
	return bMouseButton[buttonIdx] && !bOldMouseButton[buttonIdx];
}

bool Input::getMouseButtonRepeat(int buttonId) const {
	arx_assert(buttonId >= Mouse::ButtonBase && buttonId < Mouse::ButtonMax);

	int buttonIdx = buttonId - Mouse::ButtonBase;
	return bMouseButton[buttonIdx];
}

bool Input::getMouseButtonNowPressed(int buttonId) const {
	arx_assert(buttonId >= Mouse::ButtonBase && buttonId < Mouse::ButtonMax);

	int buttonIdx = buttonId - Mouse::ButtonBase;
	return bMouseButton[buttonIdx] && !bOldMouseButton[buttonIdx];
}

bool Input::getMouseButtonNowUnPressed(int buttonId) const {
	arx_assert(buttonId >= Mouse::ButtonBase && buttonId < Mouse::ButtonMax);

	int buttonIdx = buttonId - Mouse::ButtonBase;
	return !bMouseButton[buttonIdx] && bOldMouseButton[buttonIdx];
}

bool Input::getMouseButtonDoubleClick(int buttonId) const {
	arx_assert(buttonId >= Mouse::ButtonBase && buttonId < Mouse::ButtonMax);
	
	const PlatformDuration interval = PlatformDurationMs(300);
	
	int buttonIdx = buttonId - Mouse::ButtonBase;
	return (iMouseTimeSet[buttonIdx] == 2 && iMouseTime[buttonIdx][1] - iMouseTime[buttonIdx][0] < interval);
}

int Input::getMouseButtonClicked() const {
	
	for(int i = Mouse::ButtonBase; i < Mouse::ButtonMax; i++) {
		if(getMouseButtonNowPressed(i)) {
			return i;
		}
	}
	
	if(iWheelDir < 0) {
		return Mouse::Wheel_Down;
	}
	if(iWheelDir > 0) {
		return Mouse::Wheel_Up;
	}
	
	return 0;
}

bool Input::actionNowPressed(ControlAction actionId) const {
	
	for(size_t j = 0; j < ARRAY_SIZE(config.actions[actionId].key); j++) {
		
		InputKeyId key = config.actions[actionId].key[j];
		if(key == -1) {
			continue;
		}
		
		if(key & Mouse::ButtonBase) {
			if(ARXmenu.mode() != Mode_MainMenu && getMouseButtonNowPressed(key)) {
				return true;
			}
			continue;
		}
		
		if(key & Mouse::WheelBase) {
			if((key == Mouse::Wheel_Down) ? (getMouseWheelDir() < 0) : (getMouseWheelDir() > 0)) {
				return true;
			}
			continue;
		}
		
		bool bCombine = true;
		if(key & INPUT_COMBINATION_MASK) {
			if(!isKeyPressed((key >> 16) & INPUT_KEYBOARD_MASK)) {
				bCombine = false;
			}
		}
		
		if(isKeyPressedNowPressed(key & INPUT_KEYBOARD_MASK)) {
			return bCombine;
		}
		
	}
	
	return false;
}

static unsigned int uiOneHandedMagicMode = 0;
static unsigned int uiOneHandedStealth = 0;

bool Input::actionPressed(ControlAction actionId) const {
	
	if(actionId == CONTROLS_CUST_USE || actionId == CONTROLS_CUST_ACTION) {
		return false;
	}
	
	for(size_t j = 0; j < ARRAY_SIZE(config.actions[actionId].key); j++) {
		
		InputKeyId key = config.actions[actionId].key[j];
		if(key == -1) {
			continue;
		}
		
		if(key & Mouse::ButtonBase) {
			if(ARXmenu.mode() != Mode_MainMenu && getMouseButtonRepeat(key)) {
				return true;
			}
			continue;
		}
		
		if(key & Mouse::WheelBase) {
			if((key == Mouse::Wheel_Down) ? (getMouseWheelDir() < 0) : (getMouseWheelDir() > 0)) {
				return true;
			}
			continue;
		}
		
		bool bCombine = true;
		if(key & INPUT_COMBINATION_MASK) {
			if(!isKeyPressed((key >> 16) & INPUT_KEYBOARD_MASK)) {
				bCombine = false;
			}
		}
		
		if(isKeyPressed(key & INPUT_KEYBOARD_MASK)) {
			
			if(config.misc.forceToggle && actionId == CONTROLS_CUST_MAGICMODE) {
				if(bCombine) {
					if(uiOneHandedMagicMode == 0) {
						uiOneHandedMagicMode = 1;
					} else if(uiOneHandedMagicMode == 2) {
						uiOneHandedMagicMode = 3;
					}
					break;
				}
			} else if(config.misc.forceToggle && actionId == CONTROLS_CUST_STEALTHMODE) {
				if(bCombine) {
					if(uiOneHandedStealth == 0) {
						uiOneHandedStealth = 1;
					} else if(uiOneHandedStealth == 2) {
						uiOneHandedStealth = 3;
					}
					break;
				}
			} else {
				return bCombine;
			}
			
		} else if(config.misc.forceToggle) {
			if(actionId == CONTROLS_CUST_MAGICMODE) {
				if(!j && isKeyPressed(config.actions[actionId].key[1] & INPUT_KEYBOARD_MASK)) {
					continue;
				}
				if(uiOneHandedMagicMode == 1) {
					uiOneHandedMagicMode = 2;
				} else if(uiOneHandedMagicMode == 3) {
					uiOneHandedMagicMode = 0;
				}
			} else if(actionId == CONTROLS_CUST_STEALTHMODE) {
				if(!j && isKeyPressed(config.actions[actionId].key[1] & INPUT_KEYBOARD_MASK)) {
					continue;
				}
				if(uiOneHandedStealth == 1) {
					uiOneHandedStealth = 2;
				} else if(uiOneHandedStealth == 3) {
					uiOneHandedStealth = 0;
				}
			}
		}
		
	}
	
	if(config.misc.forceToggle) {
		
		if(actionId == CONTROLS_CUST_MAGICMODE) {
			if(uiOneHandedMagicMode == 1 || uiOneHandedMagicMode == 2) {
				return true;
			}
		} else if(actionId == CONTROLS_CUST_STEALTHMODE) {
			if(uiOneHandedStealth == 1 || uiOneHandedStealth == 2) {
				return true;
			}
		}
		
	}
	
	return false;
}

bool Input::actionNowReleased(ControlAction actionId) const {
	
	for(size_t j = 0; j < ARRAY_SIZE(config.actions[actionId].key); j++) {
		
		InputKeyId key = config.actions[actionId].key[j];
		if(key == -1) {
			continue;
		}
		
		if(key & Mouse::ButtonBase) {
			if(ARXmenu.mode() != Mode_MainMenu && getMouseButtonNowUnPressed(key)) {
				return true;
			}
			continue;
		}
		
		if(key & Mouse::WheelBase) {
			continue;
		}
		
		bool bCombine = true;
		if(key & INPUT_COMBINATION_MASK) {
			if(!isKeyPressed((key >> 16) & INPUT_KEYBOARD_MASK)) {
				bCombine = false;
			}
		}
		
		if(isKeyPressedNowUnPressed(key & INPUT_KEYBOARD_MASK)) {
			return bCombine;
		}
		
	}
	
	return false;
}
