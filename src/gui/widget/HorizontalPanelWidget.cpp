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

#include "gui/widget/HorizontalPanelWidget.h"

#include <boost/foreach.hpp>

HorizontalPanelWidget::HorizontalPanelWidget()
	: Widget()
{
	vElement.clear();
	pRef = this;
}

HorizontalPanelWidget::~HorizontalPanelWidget()
{
	BOOST_FOREACH(Widget * widget, vElement) {
		delete widget;
	}
}

void HorizontalPanelWidget::Move(const Vec2i & offset)
{
	m_rect.move(offset.x, offset.y);
	
	BOOST_FOREACH(Widget * widget, vElement) {
		widget->Move(offset);
	}
}

// patch on ajoute à droite en ligne
void HorizontalPanelWidget::AddElement(Widget* widget)
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

	widget->Move(Vec2i(0, ((m_rect.height() - widget->m_rect.bottom) / 2)));
}

// patch on ajoute à droite en ligne
void HorizontalPanelWidget::AddElementNoCenterIn(Widget * widget)
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
}

Widget* HorizontalPanelWidget::OnShortCut()
{
	BOOST_FOREACH(Widget * widget, vElement) {
		if(widget->OnShortCut())
			return widget;
	}

	return NULL;
}

void HorizontalPanelWidget::Update(int _iTime)
{
	m_rect.right = m_rect.left;
	m_rect.bottom = m_rect.top;

	BOOST_FOREACH(Widget * widget, vElement) {
		widget->Update(_iTime);
		m_rect.right = std::max(m_rect.right, widget->m_rect.right);
		m_rect.bottom = std::max(m_rect.bottom, widget->m_rect.bottom);
	}
}

// TODO remove this
extern bool bNoMenu;

void HorizontalPanelWidget::Render() {

	if(bNoMenu)
		return;

	BOOST_FOREACH(Widget * widget, vElement) {
		widget->Render();
	}
}

Widget * HorizontalPanelWidget::GetZoneWithID(MenuButton _iID)
{
	BOOST_FOREACH(Widget * widget, vElement) {
		if(Widget * pZone = widget->GetZoneWithID(_iID))
			return pZone;
	}
	
	return NULL;
}

Widget * HorizontalPanelWidget::IsMouseOver(const Vec2s& mousePos) const {

	if(m_rect.contains(Vec2i(mousePos))) {
		BOOST_FOREACH(Widget * widget, vElement) {
			if(widget->getCheck() && widget->m_rect.contains(Vec2i(mousePos))) {
				return widget->pRef;
			}
		}
	}

	return NULL;
}
