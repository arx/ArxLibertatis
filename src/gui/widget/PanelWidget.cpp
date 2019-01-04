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

#include "gui/widget/PanelWidget.h"

#include <boost/foreach.hpp>

#include "input/Input.h"

PanelWidget::PanelWidget() { }

PanelWidget::~PanelWidget() {
	BOOST_FOREACH(Widget * w, m_children) {
		delete w;
	}
}

void PanelWidget::move(const Vec2f & offset) {
	
	m_rect.move(offset.x, offset.y);
	
	BOOST_FOREACH(Widget * w, m_children) {
		w->move(offset);
	}
	
}

void PanelWidget::add(Widget * widget) {
	
	m_children.push_back(widget);
	
	if(m_children.size() == 1) {
		m_rect = widget->m_rect;
	} else {
		m_rect = m_rect | widget->m_rect;
	}
	
	widget->move(Vec2f(0, ((m_rect.height() - widget->m_rect.bottom) / 2)));
	
}

void PanelWidget::update() {
	
	m_rect.right = m_rect.left;
	m_rect.bottom = m_rect.top;
	
	BOOST_FOREACH(Widget * w, m_children) {
		w->update();
		m_rect.right = std::max(m_rect.right, w->m_rect.right);
		m_rect.bottom = std::max(m_rect.bottom, w->m_rect.bottom);
	}
	
}

void PanelWidget::render(bool /* mouseOver */) {
	const Vec2f cursor = Vec2f(GInput->getMousePosition());
	BOOST_FOREACH(Widget * w, m_children) {
		w->render(w->m_rect.contains(cursor));
	}
}

Widget * PanelWidget::getWidgetAt(const Vec2f & mousePos) {
	
	if(m_rect.contains(mousePos)) {
		BOOST_FOREACH(Widget * w, m_children) {
			if(w->isEnabled() && w->m_rect.contains(mousePos)) {
				return w;
			}
		}
	}
	
	return NULL;
}
