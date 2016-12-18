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

#ifndef ARX_INPUT_TEXTINPUT_H
#define ARX_INPUT_TEXTINPUT_H

#include <string>
#include <stddef.h>

#include "input/Keyboard.h"

struct KeyModifiers {
	bool shift : 1;
	bool control : 1;
	bool alt : 1;
	bool gui : 1;
	bool num : 1;
};

class TextInputHandler {
	
public:
	
	/*!
	 * User has entered new (final) text
	 */
	virtual void newText(const std::string & text) = 0;
	
	/*!
	 * User is editing text input
	 *
	 * \param composition  Temporary text being edited
	 * \param cursorStart  Cursor within the temporary text
	 * \param cursorLength Length of the cursor in bytes
	 */
	virtual void editingText(const std::string & composition, size_t cursorStart, size_t cursorLength) = 0;
	
	/*!
	 * User has pressed a key
	 *
	 * The default implementation consumes key presses that could generate text input but does nothing else.
	 */
	virtual bool keyPressed(Keyboard::Key key, KeyModifiers mod);
	
};

#endif // ARX_INPUT_TEXTINPUT_H

