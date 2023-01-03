/*
 * Copyright 2015-2022 Arx Libertatis Team (see the AUTHORS file)
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

#include <memory>
#include <vector>

#include "gui/widget/Widget.h"
#include "platform/Platform.h"
#include "util/Range.h"

class PanelWidget final : public Widget {
	
public:
	
	void move(const Vec2f & offset) override;
	
	void add(std::unique_ptr<Widget> widget);
	
	void update() override;
	void render(bool mouseOver = false) override;
	
	Widget * getWidgetAt(const Vec2f & mousePos) override;
	
	auto children() const { return util::dereference(m_children); }
	
	WidgetType type() const override {
		return WidgetType_Panel;
	}
	
private:
	
	std::vector<std::unique_ptr<Widget>> m_children;
	
};

#endif // ARX_GUI_WIDGET_PANELWIDGET_H
