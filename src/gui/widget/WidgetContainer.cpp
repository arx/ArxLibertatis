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

#include "gui/widget/WidgetContainer.h"

#include <utility>

#include "core/ArxGame.h"
#include "graphics/DrawLine.h"

void WidgetContainer::update() {
	for(Widget & widget : widgets()) {
		widget.update();
	}
}

void WidgetContainer::render(const Widget * selected) {
	for(Widget & widget : widgets()) {
		widget.render(&widget == selected);
	}
}

void WidgetContainer::add(std::unique_ptr<Widget> widget) {
	arx_assert(widget);
	m_widgets.emplace_back(std::move(widget));
}

Widget * WidgetContainer::getWidgetAt(const Vec2f & mousePos) const {
	
	for(Widget & widget : widgets()) {
		
		if(!widget.isEnabled()) {
			continue;
		}
		
		if(Widget * mouseOverWidget = widget.getWidgetAt(mousePos)) {
			return mouseOverWidget;
		}
		
	}
	
	return nullptr;
}

void WidgetContainer::move(const Vec2f & offset) {
	for(Widget & widget : widgets()) {
		widget.move(offset);
	}
}

void WidgetContainer::drawDebug() {
	
	if(g_debugInfo != InfoPanelGuiDebug) {
		return;
	}
	
	for(const Widget & widget : widgets()) {
		drawLineRectangle(widget.m_rect, 0.f, Color::red);
	}
	
}
