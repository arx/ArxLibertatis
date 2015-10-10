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

	vMenuZone.clear();

	std::vector<Widget*>::iterator i;

	for(i = vMenuZone.begin(); i != vMenuZone.end(); ++i) {
		Widget *zone = *i;
		delete zone;
	}
}

WidgetContainer::~WidgetContainer() {

	for(std::vector<Widget*>::iterator it = vMenuZone.begin(), it_end = vMenuZone.end(); it != it_end; ++it)
		delete *it;
}

void WidgetContainer::AddZone(Widget *_pMenuZone) {

	vMenuZone.push_back(_pMenuZone);
}

Widget * WidgetContainer::CheckZone(const Vec2s& mousePos) const {

	std::vector<Widget*>::const_iterator i;

	for(i = vMenuZone.begin(); i != vMenuZone.end(); ++i) {
		Widget *zone = *i;
		
		if(!zone->getCheck())
			continue;
		
		Widget * pRef = zone->IsMouseOver(mousePos);
		
		if(pRef)
			return pRef;
	}

	return NULL;
}

Widget * WidgetContainer::GetZoneNum(size_t index) {
	return vMenuZone[index];
}

Widget * WidgetContainer::GetZoneWithID(int _iID) {

	for(std::vector<Widget*>::iterator i = vMenuZone.begin(), i_end = vMenuZone.end(); i != i_end; ++i) {
		if(Widget *zone = (*i)->GetZoneWithID(_iID))
			return zone;
	}

	return NULL;
}

void WidgetContainer::Move(const Vec2i & offset) {

	for(std::vector<Widget*>::iterator i = vMenuZone.begin(), i_end = vMenuZone.end(); i != i_end; ++i) {
		(*i)->Move(offset);
	}
}

size_t WidgetContainer::GetNbZone() {
	return vMenuZone.size();
}

void WidgetContainer::DrawZone()
{
	if(g_debugInfo != InfoPanelGuiDebug)
		return;

	BOOST_FOREACH(Widget * zone, vMenuZone) {
		drawLineRectangle(Rectf(zone->rZone), 0.f, Color::red);
	}
}
