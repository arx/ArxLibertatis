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

#ifndef ARX_GUI_WIDGET_PANELWIDGET_H
#define ARX_GUI_WIDGET_PANELWIDGET_H

#include <vector>

#include "gui/widget/Widget.h"
#include "platform/Platform.h"

class PanelWidget arx_final : public Widget {
	
public:
	
	PanelWidget();
	virtual ~PanelWidget();
	
	void move(const Vec2f & offset);
	
	void add(Widget * widget);
	
	void update();
	void render(bool mouseOver = false);
	
	Widget * getWidgetAt(const Vec2f & mousePos);
	
	const std::vector<Widget *> & children() const { return m_children; }
	
	virtual WidgetType type() const {
		return WidgetType_Panel;
	}
	
private:
	
	std::vector<Widget *> m_children;
	
};

#endif // ARX_GUI_WIDGET_PANELWIDGET_H
