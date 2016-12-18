/*
 * Copyright 2016 Arx Libertatis Team (see the AUTHORS file)
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

#include "input/TextInput.h"

bool TextInputHandler::keyPressed(Keyboard::Key key, KeyModifiers mod) {
	
	switch(key) {
		
		case Keyboard::Key_0:
		case Keyboard::Key_1:
		case Keyboard::Key_2:
		case Keyboard::Key_3:
		case Keyboard::Key_4:
		case Keyboard::Key_5:
		case Keyboard::Key_6:
		case Keyboard::Key_7:
		case Keyboard::Key_8:
		case Keyboard::Key_9:
		case Keyboard::Key_A:
		case Keyboard::Key_B:
		case Keyboard::Key_C:
		case Keyboard::Key_D:
		case Keyboard::Key_E:
		case Keyboard::Key_F:
		case Keyboard::Key_G:
		case Keyboard::Key_H:
		case Keyboard::Key_I:
		case Keyboard::Key_J:
		case Keyboard::Key_K:
		case Keyboard::Key_L:
		case Keyboard::Key_M:
		case Keyboard::Key_N:
		case Keyboard::Key_O:
		case Keyboard::Key_P:
		case Keyboard::Key_Q:
		case Keyboard::Key_R:
		case Keyboard::Key_S:
		case Keyboard::Key_T:
		case Keyboard::Key_U:
		case Keyboard::Key_V:
		case Keyboard::Key_W:
		case Keyboard::Key_X:
		case Keyboard::Key_Y:
		case Keyboard::Key_Z:
		case Keyboard::Key_NumPadEnter:
		case Keyboard::Key_NumSubtract:
		case Keyboard::Key_NumAdd:
		case Keyboard::Key_NumMultiply:
		case Keyboard::Key_NumDivide:
		case Keyboard::Key_LeftBracket:
		case Keyboard::Key_RightBracket:
		case Keyboard::Key_Spacebar:
		case Keyboard::Key_Slash:
		case Keyboard::Key_Backslash:
		case Keyboard::Key_Comma:
		case Keyboard::Key_Semicolon:
		case Keyboard::Key_Period:
		case Keyboard::Key_Grave:
		case Keyboard::Key_Apostrophe:
		case Keyboard::Key_Minus:
		case Keyboard::Key_Equals:
			return !mod.control;
		
		case Keyboard::Key_NumPad0:
		case Keyboard::Key_NumPad1:
		case Keyboard::Key_NumPad2:
		case Keyboard::Key_NumPad3:
		case Keyboard::Key_NumPad4:
		case Keyboard::Key_NumPad5:
		case Keyboard::Key_NumPad6:
		case Keyboard::Key_NumPad7:
		case Keyboard::Key_NumPad8:
		case Keyboard::Key_NumPad9:
		case Keyboard::Key_NumPoint:
			return mod.num && !mod.control;
		
		default: return false;
		
	}
	
}
