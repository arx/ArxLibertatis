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

#ifndef ARX_GUI_WIDGET_BUTTONWIDGET_H
#define ARX_GUI_WIDGET_BUTTONWIDGET_H

#include <boost/function.hpp>

#include "gui/widget/TextWidget.h"
#include "gui/widget/Widget.h"

class TextureContainer;

class ButtonWidget: public Widget {
	
public:
	ButtonWidget(const Vec2f & pos, const Vec2f & size, const char * texturePath);
	~ButtonWidget();
	
public:
	void SetPos(Vec2f pos);
	void AddText(const std::string & label);
	bool OnMouseClick();
	void Update();
	void Render();
	void RenderMouseOver();
	
	boost::function<void()> clicked;
	
	virtual WidgetType type() const {
		return WidgetType_Button;
	};
	
private:
	Vec2f m_pos;
	Vec2f m_size;
	
	TextureContainer * m_texture;
};

#endif // ARX_GUI_WIDGET_BUTTONWIDGET_H
