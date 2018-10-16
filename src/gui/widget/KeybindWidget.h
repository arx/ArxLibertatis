/*
 * Copyright 2018 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GUI_WIDGET_KEYBINDWIDGET_H
#define ARX_GUI_WIDGET_KEYBINDWIDGET_H

#include <string>

#include <boost/function.hpp>

#include "core/Config.h"
#include "gui/widget/Widget.h"
#include "input/InputKey.h"
#include "math/Vector.h"
#include "platform/Platform.h"

class Font;

class KeybindWidget arx_final : public Widget {
	
	std::string m_text;
	Font * m_font;
	ControlAction m_action;
	size_t m_index;
	InputKeyId m_key;
	bool m_editing;
	bool m_allowMouse;
	
public:
	
	boost::function<void(KeybindWidget * /* widget */)> keyChanged;
	
	KeybindWidget(ControlAction keybindAction, size_t keybindIndex, Font * font);
	
	bool click();
	
	void update();
	
	void render(bool mouseOver = false);
	
	bool wantFocus() const { return m_editing; }
	
	void unfocus();
	
	virtual WidgetType type() const {
		return WidgetType_Keybind;
	}
	
	void setKey(InputKeyId keyId);
	
	ControlAction action() { return m_action; }
	size_t index() const { return m_index; }
	InputKeyId key() const { return m_key; }
	
};

#endif // ARX_GUI_WIDGET_KEYBINDWIDGET_H

