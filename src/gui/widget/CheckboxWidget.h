/*
 * Copyright 2015-2019 Arx Libertatis Team (see the AUTHORS file)
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

#include <functional>
#include <string_view>

#include "gui/widget/Widget.h"
#include "platform/Platform.h"

class Font;
class TextWidget;
class TextureContainer;

class CheckboxWidget final : public Widget {
	
public:
	
	explicit CheckboxWidget(const Vec2f & size, Font * font, std::string_view label);
	virtual ~CheckboxWidget();
	
	void move(const Vec2f & offset) override;
	bool click() override;
	
	void render(bool mouseOver = false) override;
	
	void setChecked(bool checked);
	bool checked() const { return m_checked; }
	
	std::function<void(bool /* checked */)> stateChanged;
	
	virtual WidgetType type() const override {
		return WidgetType_Checkbox;
	}
	
private:
	
	TextWidget * m_label;
	TextureContainer * m_textureOff;
	TextureContainer * m_textureOn;
	Rectf m_button;
	
	bool m_checked;
	
};

#endif // ARX_GUI_WIDGET_CHECKBOXWIDGET_H
