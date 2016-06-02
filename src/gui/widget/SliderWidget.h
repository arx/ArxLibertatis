/*
 * Copyright 2015 Arx Libertatis Team (see the AUTHORS file)
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

#include <boost/function.hpp>

#include "gui/widget/ButtonWidget.h"
#include "gui/widget/Widget.h"

//! Slider with value in the range [0..10]
class SliderWidget: public Widget {
	
public:
	explicit SliderWidget(const Vec2f & unscaled);
	virtual ~SliderWidget();
	
	void setMinimum(int minimum);
	
	void setValue(int value) { m_value = value; }
	int getValue() const { return m_value; }
	
	void Move(const Vec2f & offset);
	bool OnMouseClick();
	void Update();
	void Render();
	void RenderMouseOver();
	void EmptyFunction();
	
	boost::function<void(int)> valueChanged;	// NOLINT
	
private:
	ButtonWidget		*	pLeftButton;
	ButtonWidget		*	pRightButton;
	TextureContainer	* pTex1;
	TextureContainer	* pTex2;
	
	int	m_minimum;
	int	m_value;
};

#endif // ARX_GUI_WIDGET_SLIDERWIDGET_H
