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

#include "gui/widget/PanelWidget.h"

#include <boost/foreach.hpp>

PanelWidget::PanelWidget()
	: Widget()
{
	vElement.clear();
	pRef = this;
}

PanelWidget::~PanelWidget()
{
	BOOST_FOREACH(Widget * widget, vElement) {
		delete widget;
	}
}

void PanelWidget::Move(const Vec2f & offset)
{
	m_rect.move(offset.x, offset.y);
	
	BOOST_FOREACH(Widget * widget, vElement) {
		widget->Move(offset);
	}
}

// patch on ajoute à droite en ligne
void PanelWidget::AddElement(Widget* widget)
{
	vElement.push_back(widget);

	if(vElement.size() == 1) {
		m_rect = widget->m_rect;
	} else {
		m_rect.left = std::min(m_rect.left, widget->m_rect.left);
		m_rect.top = std::min(m_rect.top, widget->m_rect.top);
	}

	// + taille elem
	m_rect.right = std::max(m_rect.right, widget->m_rect.right);
	m_rect.bottom = std::max(m_rect.bottom, widget->m_rect.bottom);

	widget->Move(Vec2f(0, ((m_rect.height() - widget->m_rect.bottom) / 2)));
}

void PanelWidget::Update()
{
	m_rect.right = m_rect.left;
	m_rect.bottom = m_rect.top;

	BOOST_FOREACH(Widget * widget, vElement) {
		widget->Update();
		m_rect.right = std::max(m_rect.right, widget->m_rect.right);
		m_rect.bottom = std::max(m_rect.bottom, widget->m_rect.bottom);
	}
}

void PanelWidget::Render() {

	BOOST_FOREACH(Widget * widget, vElement) {
		widget->Render();
	}
}

Widget * PanelWidget::GetZoneWithID(MenuButton _iID)
{
	BOOST_FOREACH(Widget * widget, vElement) {
		if(Widget * pZone = widget->GetZoneWithID(_iID))
			return pZone;
	}
	
	return NULL;
}

Widget * PanelWidget::IsMouseOver(const Vec2f & mousePos) const {

	if(m_rect.contains(mousePos)) {
		BOOST_FOREACH(Widget * widget, vElement) {
			if(widget->getCheck() && widget->m_rect.contains(mousePos)) {
				return widget->pRef;
			}
		}
	}

	return NULL;
}
