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

#include "gui/widget/PanelWidget.h"

#include <utility>

#include "input/Input.h"

void PanelWidget::move(const Vec2f & offset) {
	
	m_rect.move(offset.x, offset.y);
	
	for(Widget & widget : children()) {
		widget.move(offset);
	}
	
}

void PanelWidget::add(std::unique_ptr<Widget> widget) {
	
	arx_assert(widget);
	
	if(m_children.empty()) {
		m_rect = widget->m_rect;
	} else {
		m_rect = m_rect | widget->m_rect;
	}
	
	widget->move(Vec2f(0, ((m_rect.height() - widget->m_rect.bottom) / 2)));
	
	m_children.emplace_back(std::move(widget));
	
}

void PanelWidget::update() {
	
	m_rect.right = m_rect.left;
	m_rect.bottom = m_rect.top;
	
	for(Widget & widget : children()) {
		widget.update();
		m_rect.right = std::max(m_rect.right, widget.m_rect.right);
		m_rect.bottom = std::max(m_rect.bottom, widget.m_rect.bottom);
	}
	
}

void PanelWidget::render(bool /* mouseOver */) {
	const Vec2f cursor = Vec2f(GInput->getMousePosition());
	for(Widget & widget : children()) {
		widget.render(widget.m_rect.contains(cursor));
	}
}

Widget * PanelWidget::getWidgetAt(const Vec2f & mousePos) {
	
	if(m_rect.contains(mousePos)) {
		for(Widget & widget : children()) {
			if(widget.isEnabled() && widget.m_rect.contains(mousePos)) {
				return &widget;
			}
		}
	}
	
	return nullptr;
}
