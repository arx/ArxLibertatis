/*
 * Copyright 2011-2016 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_INPUT_KEYBOARD_H
#define ARX_INPUT_KEYBOARD_H

class Keyboard {
	
public:
	
	//! Key enumeration.
	enum Key {
		
		Key_Invalid = -1,
		
		KeyBase = 0,
		
		Key_0 = KeyBase,
		Key_1,
		Key_2,
		Key_3,
		Key_4,
		Key_5,
		Key_6,
		Key_7,
		Key_8,
		Key_9,
		
		Key_A,
		Key_B,
		Key_C,
		Key_D,
		Key_E,
		Key_F,
		Key_G,
		Key_H,
		Key_I,
		Key_J,
		Key_K,
		Key_L,
		Key_M,    
		Key_N,
		Key_O,
		Key_P,
		Key_Q,
		Key_R,
		Key_S,
		Key_T,
		Key_U,
		Key_V,
		Key_W,
		Key_X,
		Key_Y,
		Key_Z,
		
		Key_F1,
		Key_F2,
		Key_F3,
		Key_F4,
		Key_F5,
		Key_F6,
		Key_F7,
		Key_F8,
		Key_F9,
		Key_F10,
		Key_F11,
		Key_F12,
		Key_F13,
		Key_F14,
		Key_F15,
		
		Key_UpArrow,
		Key_DownArrow,
		Key_LeftArrow,
		Key_RightArrow,
		
		Key_Home,
		Key_End,
		Key_PageUp,
		Key_PageDown,
		Key_Insert,
		Key_Delete,
		
		Key_Escape,
		
		Key_NumLock,
		Key_NumPad0,
		Key_NumPad1,
		Key_NumPad2,
		Key_NumPad3,
		Key_NumPad4,
		Key_NumPad5,
		Key_NumPad6,
		Key_NumPad7,
		Key_NumPad8,
		Key_NumPad9,
		Key_NumPadEnter,
		Key_NumSubtract,        // (-) on numeric keypad
		Key_NumAdd,             // (+) on numeric keypad
		Key_NumMultiply,        // (*) on numeric keypad
		Key_NumDivide,          // (/) on numeric keypad
		Key_NumPoint,           // PERIOD (decimal point) on numeric keypad
		
		Key_LeftBracket,        // Left square bracket [
		Key_LeftCtrl,           // Left CTRL
		Key_LeftAlt,            // Left ALT
		Key_LeftShift,          // Left SHIFT
		Key_LeftWin,            // Left Windows logo key
		
		Key_RightBracket,       // Right square bracket ]
		Key_RightCtrl,          // Right CTRL
		Key_RightAlt,           // Right ALT
		Key_RightShift,         // Right SHIFT
		Key_RightWin,           // Right Windows logo key
		
		Key_PrintScreen,
		Key_ScrollLock,
		Key_Pause,
		
		Key_Spacebar,
		Key_Backspace,
		Key_Enter,              // ENTER on main keyboard 
		Key_Tab,
		
		Key_Apps,               // Application key
		Key_CapsLock,
		
		Key_Slash,              // (/) On main keyboard
		Key_Backslash,          // (\)
		Key_Comma,              // (,)
		Key_Semicolon,          // (;)
		Key_Period,             // (.) On main keyboard
		Key_Grave,              // (`) Grave accent
		Key_Apostrophe,         // (')
		Key_Minus,              // (-) On main keyboard
		Key_Equals,             // (=) On main keyboard
		
		KeyMax,
		KeyCount = KeyMax - KeyBase
	};
};

#endif // ARX_INPUT_KEYBOARD_H
