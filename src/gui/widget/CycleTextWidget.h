/*
 * Copyright 2015-2020 Arx Libertatis Team (see the AUTHORS file)
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

#include <functional>
#include <string_view>
#include <vector>

#include "gui/widget/Widget.h"
#include "platform/Platform.h"

class Font;
class ButtonWidget;
class TextWidget;

class CycleTextWidget final : public Widget {
	
public:
	
	explicit CycleTextWidget(const Vec2f & size, Font * font, std::string_view label = std::string_view(),
	                         Font * entryFont = nullptr);
	~CycleTextWidget() override;
	
	void setValue(int value) { m_value = value; }
	int getValue() const { return m_value; }
	
	void selectLast();
	
	void addEntry(std::string_view label);
	
	void move(const Vec2f & offset) override;
	bool click() override;
	void render(bool mouseOver = false) override;
	void hover() override;
	void setEnabled(bool enable) override;
	
	std::function<void(int, std::string_view)> valueChanged;
	
	WidgetType type() const override {
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
