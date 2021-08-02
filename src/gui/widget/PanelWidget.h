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

#ifndef ARX_GUI_WIDGET_PANELWIDGET_H
#define ARX_GUI_WIDGET_PANELWIDGET_H

#include <vector>

#include "gui/widget/Widget.h"
#include "platform/Platform.h"

class PanelWidget final : public Widget {
	
public:
	
	~PanelWidget() override;
	
	void move(const Vec2f & offset) override;
	
	void add(Widget * widget);
	
	void update() override;
	void render(bool mouseOver = false) override;
	
	Widget * getWidgetAt(const Vec2f & mousePos) override;
	
	const std::vector<Widget *> & children() const { return m_children; }
	
	WidgetType type() const override {
		return WidgetType_Panel;
	}
	
private:
	
	std::vector<Widget *> m_children;
	
};

#endif // ARX_GUI_WIDGET_PANELWIDGET_H
