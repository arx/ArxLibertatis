/*
 * Copyright 2015-2016 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GUI_WIDGET_TEXTINPUTWIDGET_H
#define ARX_GUI_WIDGET_TEXTINPUTWIDGET_H

#include <boost/function.hpp>

#include "graphics/Color.h"
#include "gui/widget/Widget.h"

class Font;

class TextInputWidget: public Widget {
	
	std::string m_text;
	Font * m_font;
	
public:
	
	TextInputWidget(Font * font, const std::string & text, Vec2f pos = Vec2f_ZERO);
	
	bool click();
	
	void render(bool mouseOver = false);
	
	void setText(const std::string & text);
	const std::string & text() const { return m_text; }
	
	Font * font() const { return m_font; }
	
	virtual WidgetType type() const {
		return WidgetType_TextInput;
	}
	
};

#endif // ARX_GUI_WIDGET_TEXTINPUTWIDGET_H
