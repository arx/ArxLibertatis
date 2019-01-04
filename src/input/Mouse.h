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

#ifndef ARX_INPUT_MOUSE_H
#define ARX_INPUT_MOUSE_H

class Mouse {
	
public:
	
	enum Button {
		
		Button_Invalid = -1,
		
		ButtonBase = 0x20000000,
		
		Button_0 = ButtonBase,
		Button_1,
		Button_2,
		Button_3,
		Button_4,
		Button_5,
		Button_6,
		Button_7,
		
		ButtonMax,
		ButtonCount = ButtonMax - ButtonBase
	};
	
	enum Wheel {
		
		WheelBase = 0x10000000,
		
		Wheel_Up = WheelBase,
		Wheel_Down
	};
	
	enum Mode {
		Absolute,
		Relative
	};
	
};

#endif // ARX_INPUT_MOUSE_H
