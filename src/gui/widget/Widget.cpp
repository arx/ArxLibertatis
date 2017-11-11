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

#include "gui/widget/Widget.h"

#include "gui/MenuWidgets.h"
#include "gui/widget/CheckboxWidget.h"
#include "gui/widget/CycleTextWidget.h"
#include "input/Input.h"

Widget::Widget()
	: m_rect(0, 0, 0, 0)
	, m_id(BUTTON_INVALID)
	, m_savegame(0)
	, enabled(true)
	, bCheck(true)
{
	ePlace=NOCENTER;
	eState=TNOP;
	m_targetMenu = NOP;
	m_shortcut = ActionKey::UNUSED;
}

extern TextWidget * pMenuElementApply;

Widget::~Widget() {

	if(this == pMenuElementApply) {
		pMenuElementApply = NULL;
	}
}

void Widget::Move(const Vec2f & offset) {
	m_rect.move(offset.x, offset.y);
}

void Widget::SetPos(Vec2f pos) {
	
	Vec2f size = m_rect.size();
	
	m_rect.left   = pos.x;
	m_rect.top    = pos.y;
	m_rect.right  = pos.x + size.x;
	m_rect.bottom = pos.y + size.y;
}

Widget * Widget::IsMouseOver(const Vec2f & mousePos) {
	
	if(   mousePos.x >= m_rect.left
	   && mousePos.y >= m_rect.top
	   && mousePos.x <= m_rect.right
	   && mousePos.y <= m_rect.bottom
	) {
		return this;
	}

	return NULL;
}

void Widget::SetShortCut(int _iShortCut) {
	m_shortcut = _iShortCut;
}

void Widget::setEnabled(bool enable) {
	enabled = enable;
}

void Widget::SetCheckOff() {
	bCheck = false;
}

void Widget::SetCheckOn() {
	bCheck = true;
}

bool Widget::getCheck() {
	return bCheck;
}
