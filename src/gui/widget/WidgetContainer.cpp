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

#include "gui/widget/WidgetContainer.h"

#include <boost/foreach.hpp>

#include "core/ArxGame.h"
#include "graphics/DrawLine.h"

WidgetContainer::WidgetContainer() { }

WidgetContainer::~WidgetContainer() {
	BOOST_FOREACH(Widget * w, m_widgets) {
		delete w;
	}
}

void WidgetContainer::add(Widget *widget) {

	m_widgets.push_back(widget);
}

Widget * WidgetContainer::getAtPos(const Vec2f & mousePos) const {
	
	BOOST_FOREACH(Widget * w, m_widgets) {
		
		if(!w->getCheck()) {
			continue;
		}
		
		Widget * mouseOverWidget = w->IsMouseOver(mousePos);
		if(mouseOverWidget) {
			return mouseOverWidget;
		}
		
	}
	
	return NULL;
}

void WidgetContainer::Move(const Vec2f & offset) {
	BOOST_FOREACH(Widget * w, m_widgets) {
		w->Move(offset);
	}
}

void WidgetContainer::drawDebug() {
	
	if(g_debugInfo != InfoPanelGuiDebug) {
		return;
	}
	
	BOOST_FOREACH(Widget * w, m_widgets) {
		drawLineRectangle(w->m_rect, 0.f, Color::red);
	}
	
}
