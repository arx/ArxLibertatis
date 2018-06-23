/*
 * Copyright 2015-2016 Arx Libertatis Team (see the AUTHORS file)
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

class PanelWidget : public Widget {
	
public:
	PanelWidget();
	virtual ~PanelWidget();
	
	void Move(const Vec2f & offset);
	void AddElement(Widget * widget);
	
	void Update();
	void render(bool mouseOver = false);
	bool OnMouseClick() { return false; }
	Widget * IsMouseOver(const Vec2f & mousePos);
	
	virtual WidgetType type() const {
		return WidgetType_Panel;
	}
	
	std::vector<Widget *> m_children;
};

#endif // ARX_GUI_WIDGET_PANELWIDGET_H
