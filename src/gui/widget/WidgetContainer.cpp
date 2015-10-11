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

#include "gui/widget/WidgetContainer.h"

#include <boost/foreach.hpp>

#include "core/ArxGame.h"
#include "graphics/DrawLine.h"

WidgetContainer::WidgetContainer() {

	m_widgets.clear();

	std::vector<Widget*>::iterator i;

	for(i = m_widgets.begin(); i != m_widgets.end(); ++i) {
		Widget *widget = *i;
		delete widget;
	}
}

WidgetContainer::~WidgetContainer() {

	for(std::vector<Widget*>::iterator it = m_widgets.begin(), it_end = m_widgets.end(); it != it_end; ++it)
		delete *it;
}

void WidgetContainer::add(Widget *widget) {

	m_widgets.push_back(widget);
}

Widget * WidgetContainer::getAtPos(const Vec2s& mousePos) const {

	std::vector<Widget*>::const_iterator i;

	for(i = m_widgets.begin(); i != m_widgets.end(); ++i) {
		Widget * widget = *i;
		
		if(!widget->getCheck())
			continue;
		
		Widget * mouseOverWidget = widget->IsMouseOver(mousePos);
		
		if(mouseOverWidget)
			return mouseOverWidget;
	}

	return NULL;
}

Widget * WidgetContainer::GetZoneNum(size_t index) {
	return m_widgets[index];
}

Widget * WidgetContainer::GetZoneWithID(MenuButton _iID) {

	for(std::vector<Widget*>::iterator i = m_widgets.begin(), i_end = m_widgets.end(); i != i_end; ++i) {
		if(Widget * widget = (*i)->GetZoneWithID(_iID))
			return widget;
	}

	return NULL;
}

void WidgetContainer::Move(const Vec2i & offset) {

	for(std::vector<Widget*>::iterator i = m_widgets.begin(), i_end = m_widgets.end(); i != i_end; ++i) {
		(*i)->Move(offset);
	}
}

size_t WidgetContainer::GetNbZone() {
	return m_widgets.size();
}

void WidgetContainer::drawDebug()
{
	if(g_debugInfo != InfoPanelGuiDebug)
		return;

	BOOST_FOREACH(Widget * zone, m_widgets) {
		drawLineRectangle(Rectf(zone->m_rect), 0.f, Color::red);
	}
}
