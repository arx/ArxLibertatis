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
	: Widget(NOP)
{
	vElement.clear();
	pRef = this;
}

HorizontalPanelWidget::~HorizontalPanelWidget()
{
	BOOST_FOREACH(Widget * e, vElement) {
		delete e;
	}
}

void HorizontalPanelWidget::Move(const Vec2i & offset)
{
	m_rect.move(offset.x, offset.y);
	
	BOOST_FOREACH(Widget * e, vElement) {
		e->Move(offset);
	}
}

// patch on ajoute à droite en ligne
void HorizontalPanelWidget::AddElement(Widget* _pElem)
{
	vElement.push_back(_pElem);

	if(vElement.size() == 1) {
		m_rect = _pElem->m_rect;
	} else {
		m_rect.left = std::min(m_rect.left, _pElem->m_rect.left);
		m_rect.top = std::min(m_rect.top, _pElem->m_rect.top);
	}

	// + taille elem
	m_rect.right = std::max(m_rect.right, _pElem->m_rect.right);
	m_rect.bottom = std::max(m_rect.bottom, _pElem->m_rect.bottom);

	_pElem->Move(Vec2i(0, ((m_rect.height() - _pElem->m_rect.bottom) / 2)));
}

// patch on ajoute à droite en ligne
void HorizontalPanelWidget::AddElementNoCenterIn(Widget* _pElem)
{
	vElement.push_back(_pElem);

	if(vElement.size() == 1) {
		m_rect = _pElem->m_rect;
	} else {
		m_rect.left = std::min(m_rect.left, _pElem->m_rect.left);
		m_rect.top = std::min(m_rect.top, _pElem->m_rect.top);
	}

	// + taille elem
	m_rect.right = std::max(m_rect.right, _pElem->m_rect.right);
	m_rect.bottom = std::max(m_rect.bottom, _pElem->m_rect.bottom);
}

Widget* HorizontalPanelWidget::OnShortCut()
{
	BOOST_FOREACH(Widget * e, vElement) {
		if(e->OnShortCut())
			return e;
	}

	return NULL;
}

void HorizontalPanelWidget::Update(int _iTime)
{
	m_rect.right = m_rect.left;
	m_rect.bottom = m_rect.top;

	BOOST_FOREACH(Widget * e, vElement) {
		e->Update(_iTime);
		m_rect.right = std::max(m_rect.right, e->m_rect.right);
		m_rect.bottom = std::max(m_rect.bottom, e->m_rect.bottom);
	}
}

// TODO remove this
extern bool bNoMenu;

void HorizontalPanelWidget::Render() {

	if(bNoMenu)
		return;

	BOOST_FOREACH(Widget * e, vElement) {
		e->Render();
	}
}

Widget * HorizontalPanelWidget::GetZoneWithID(int _iID)
{
	BOOST_FOREACH(Widget * e, vElement) {
		if(Widget * pZone = e->GetZoneWithID(_iID))
			return pZone;
	}
	
	return NULL;
}

Widget * HorizontalPanelWidget::IsMouseOver(const Vec2s& mousePos) const {

	if(m_rect.contains(Vec2i(mousePos))) {
		BOOST_FOREACH(Widget * e, vElement) {
			if(e->getCheck() && e->m_rect.contains(Vec2i(mousePos))) {
				return e->pRef;
			}
		}
	}

	return NULL;
}
