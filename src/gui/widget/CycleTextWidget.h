/*
 * Copyright 2015-2018 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GUI_WIDGET_CYCLETEXTWIDGET_H
#define ARX_GUI_WIDGET_CYCLETEXTWIDGET_H

#include <vector>
#include <string>

#include <boost/function.hpp>

#include "gui/widget/Widget.h"
#include "platform/Platform.h"

class Font;
class ButtonWidget;
class TextWidget;

class CycleTextWidget arx_final : public Widget {
	
public:
	
	explicit CycleTextWidget(const Vec2f & size, Font * font, const std::string & label,
	                         Font * entryFont = NULL);
	virtual ~CycleTextWidget();
	
	void setValue(int value) { m_value = value; }
	int getValue() const { return m_value; }
	
	void selectLast();
	
	void addEntry(const std::string & label);
	
	void move(const Vec2f & offset);
	bool click();
	void render(bool mouseOver = false);
	void hover();
	virtual void setEnabled(bool enable);
	
	boost::function<void(int, const std::string &)> valueChanged;
	
	virtual WidgetType type() const {
		return WidgetType_CycleText;
	}
	
private:
	
	void newValue(int value);
	
	TextWidget * m_label;
	ButtonWidget * m_left;
	ButtonWidget * m_right;
	Font * m_font;
	std::vector<TextWidget *> m_entries;
	Rectf m_content;
	
	int m_value;
	
};

#endif // ARX_GUI_WIDGET_CYCLETEXTWIDGET_H
