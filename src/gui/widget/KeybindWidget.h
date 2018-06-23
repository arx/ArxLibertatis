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

#include "core/Config.h"
#include "gui/widget/TextWidget.h"
#include "gui/widget/Widget.h"
#include "math/Vector.h"

class Font;

class KeybindWidget: public TextWidget {
	
public:
	
	ControlAction m_keybindAction;
	int m_keybindIndex;
	
public:
	
	KeybindWidget(ControlAction keybindAction, int keybindIndex, Font * font, const Vec2f pos = Vec2f_ZERO);
	
	bool OnMouseClick();
	
	virtual WidgetType type() const {
		return WidgetType_Keybind;
	}
	
	void setKey(int keyId);
	
};

#endif // ARX_GUI_WIDGET_KEYBINDWIDGET_H

