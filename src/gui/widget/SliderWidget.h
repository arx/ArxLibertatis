/*
 * Copyright 2015-2021 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GUI_WIDGET_SLIDERWIDGET_H
#define ARX_GUI_WIDGET_SLIDERWIDGET_H

#include <functional>
#include <memory>
#include <string_view>

#include "gui/widget/ButtonWidget.h"
#include "gui/widget/TextWidget.h"
#include "gui/widget/Widget.h"
#include "platform/Platform.h"

class Font;
class TextureContainer;

//! Slider with value in the range [0..10]
class SliderWidget final : public Widget {
	
public:
	
	explicit SliderWidget(const Vec2f & size, Font * font, std::string_view label);
	
	void setMinimum(int minimum);
	
	void setValue(int value) { m_value = value; }
	int getValue() const { return m_value; }
	
	void move(const Vec2f & offset) override;
	bool click() override;
	void update() override;
	void render(bool mouseOver = false) override;
	void hover() override;
	
	std::function<void(int /* state */)> valueChanged;
	
	WidgetType type() const override {
		return WidgetType_Slider;
	}
	
private:
	
	void newValue(int value);
	
	std::unique_ptr<TextWidget> m_label;
	std::unique_ptr<ButtonWidget> m_left;
	std::unique_ptr<ButtonWidget> m_right;
	TextureContainer * m_textureOff;
	TextureContainer * m_textureOn;
	Rectf m_slider;
	
	int m_minimum;
	int m_value;
	
};

#endif // ARX_GUI_WIDGET_SLIDERWIDGET_H
