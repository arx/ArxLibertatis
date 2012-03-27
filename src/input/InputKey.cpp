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

#include "input/InputKey.h"

#include <map>
#include <string>
#include <sstream>

#include "input/Keyboard.h"
#include "input/Mouse.h"

#include "platform/Platform.h"

std::map<std::string, InputKeyId> keyNames;
const std::string PREFIX_KEY = "Key_";
const std::string PREFIX_BUTTON = "Button";
const char SEPARATOR = '+';
const std::string InputKey::KEY_NONE = "---";

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
};

std::string InputKey::getName(InputKeyId key, bool localizedName) {
	
	if(key == -1) {
		return std::string();
	}
	
	std::string name;
	
	std::string modifier;
	if(key & INPUT_COMBINATION_MASK) {
		// key combination
		modifier = getName((key >> 16) & 0x0fff, localizedName);
		key &= INPUT_MASK;
	}
	
	if(key >= (InputKeyId)Mouse::ButtonBase && key < (InputKeyId)Mouse::ButtonMax) {
		
		std::ostringstream oss;
		oss << PREFIX_BUTTON << (int)(key - Mouse::ButtonBase + 1);
		name = oss.str();
	
	} else if(key == (InputKeyId)Mouse::Wheel_Up) {
		name = "WheelUp";

	} else if(key == (InputKeyId)Mouse::Wheel_Down) {
		name = "WheelDown";

	} else {
		arx_assert(key >= 0 && key < (int)ARRAY_SIZE(keysDescriptions));
		const KeyDescription & entity = keysDescriptions[key];
		
		arx_assert(entity.id == key);
		name = entity.name;
	}
	
	if(name.empty()) {
		std::ostringstream oss;
		oss << PREFIX_KEY << (int)key;
		name = oss.str();
	}
	
	if(!modifier.empty()) {
		return modifier + SEPARATOR + name;
	} else {
		return name;
	}
}

InputKeyId InputKey::getId(const std::string & name) {
	
	// If a noneset key, return -1
	if(name.empty() || name == KEY_NONE) {
		return -1;
	}
	
	size_t sep = name.find(SEPARATOR);
	if(sep != std::string::npos) {
		InputKeyId modifier = getId(name.substr(0, sep));
		InputKeyId key = getId(name.substr(sep + 1));
		return (modifier << 16 | key);
	}
	
	if(!name.compare(0, PREFIX_KEY.length(), PREFIX_KEY)) {
		std::istringstream iss(name.substr(PREFIX_KEY.length()));
		int key;
		iss >> key;
		if(!iss.bad()) {
			return key;
		}
	}
	
	if(!name.compare(0, PREFIX_BUTTON.length(), PREFIX_BUTTON)) {
		std::istringstream iss(name.substr(PREFIX_BUTTON.length()));
		int key;
		iss >> key;
		if(!iss.bad() && key >= 0 && key < Mouse::ButtonCount) {
			return Mouse::ButtonBase + key - 1;
		}
	}
	
	if(keyNames.empty()) {
		// Initialize the key name -> id map.
		for(size_t i = 0; i < ARRAY_SIZE(keysDescriptions); i++) {
			keyNames[keysDescriptions[i].name] = keysDescriptions[i].id;
		}
		keyNames["WheelUp"] = (InputKeyId)Mouse::Wheel_Up;
		keyNames["WheelDown"] = (InputKeyId)Mouse::Wheel_Down;
	}
	
	std::map<std::string, InputKeyId>::const_iterator it = keyNames.find(name);
	if(it != keyNames.end()) {
		return it->second;
	}
	
	return -1;
}
