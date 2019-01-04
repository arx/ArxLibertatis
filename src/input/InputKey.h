/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_INPUT_INPUTKEY_H
#define ARX_INPUT_INPUTKEY_H

#include <string>

typedef int InputKeyId;

#define INPUT_KEYBOARD_MASK    0x0000ffff
#define INPUT_MOUSE_MASK       0x2000000f
#define INPUT_MOUSEWHEEL_MASK  0x1000000f
#define INPUT_MASK             (INPUT_KEYBOARD_MASK | INPUT_MOUSE_MASK | INPUT_MOUSEWHEEL_MASK)
#define INPUT_COMBINATION_MASK (~INPUT_MASK)

#endif // ARX_INPUT_INPUTKEY_H
