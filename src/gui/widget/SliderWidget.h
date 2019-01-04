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

#ifndef ARX_GUI_WIDGET_SLIDERWIDGET_H
#define ARX_GUI_WIDGET_SLIDERWIDGET_H

#include <string>

#include <boost/function.hpp>

#include "gui/widget/Widget.h"
#include "platform/Platform.h"

class Font;
class ButtonWidget;
class TextWidget;
class TextureContainer;

//! Slider with value in the range [0..10]
class SliderWidget arx_final : public Widget {
	
public:
	
	explicit SliderWidget(const Vec2f & size, Font * font, const std::string & label);
	virtual ~SliderWidget();
	
	void setMinimum(int minimum);
	
	void setValue(int value) { m_value = value; }
	int getValue() const { return m_value; }
	
	void move(const Vec2f & offset);
	bool click();
	void update();
	void render(bool mouseOver = false);
	void hover();
	
	boost::function<void(int /* state */)> valueChanged;
	
	virtual WidgetType type() const {
		return WidgetType_Slider;
	}
	
private:
	
	void newValue(int value);
	
	TextWidget * m_label;
	ButtonWidget * m_left;
	ButtonWidget * m_right;
	TextureContainer * m_textureOff;
	TextureContainer * m_textureOn;
	Rectf m_slider;
	
	int m_minimum;
	int m_value;
	
};

#endif // ARX_GUI_WIDGET_SLIDERWIDGET_H
