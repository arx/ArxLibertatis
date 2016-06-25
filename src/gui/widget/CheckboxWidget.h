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

#ifndef ARX_GUI_WIDGET_CHECKBOXWIDGET_H
#define ARX_GUI_WIDGET_CHECKBOXWIDGET_H

#include <boost/function.hpp>

#include "gui/widget/Widget.h"

class TextWidget;
class TextureContainer;

class CheckboxWidget : public Widget {
	
public:
	explicit CheckboxWidget(TextWidget * label);
	virtual ~CheckboxWidget();
	
	void Move(const Vec2f & offset);
	bool OnMouseClick();
	void Update();
	
	void renderCommon();
	void Render();
	void RenderMouseOver();
	
	int					iState;
	int					iOldState;
	
	boost::function<void(int)> stateChanged;	// NOLINT
	
	virtual WidgetType type() const {
		return WidgetType_Checkbox;
	};
	
private:
	TextureContainer * m_textureOff;
	TextureContainer * m_textureOn;
	TextWidget	* m_label;
};

#endif // ARX_GUI_WIDGET_CHECKBOXWIDGET_H
