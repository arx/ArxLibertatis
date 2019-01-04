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

#include "gui/widget/Widget.h"

#include "gui/MainMenu.h"
#include "gui/MenuWidgets.h"
#include "gui/widget/CheckboxWidget.h"
#include "gui/widget/CycleTextWidget.h"
#include "input/Input.h"
#include "scene/GameSound.h"

Widget::Widget()
	: m_rect(0, 0, 0, 0)
	, m_enabled(true)
	, m_targetPage(NOP)
	, m_shortcut(ActionKey::UNUSED)
{ }

Widget::~Widget() { }

bool Widget::click() {
	
	if(!m_enabled) {
		return false;
	}
	
	ARX_SOUND_PlayMenu(g_snd.MENU_CLICK);
	
	if(m_enabled && m_targetPage != NOP && g_mainMenu) {
		g_mainMenu->requestPage(m_targetPage);
	}
	
	if(clicked) {
		clicked(this);
	}
	
	return false;
	
}

bool Widget::doubleClick() {
	
	if(!doubleClicked) {
		return click();
	}
	
	if(!m_enabled) {
		return false;
	}
	
	ARX_SOUND_PlayMenu(g_snd.MENU_CLICK);
	
	doubleClicked(this);
	
	return false;
	
}

void Widget::move(const Vec2f & offset) {
	m_rect.move(offset.x, offset.y);
}

Widget * Widget::getWidgetAt(const Vec2f & mousePos) {
	return m_rect.contains(mousePos) ? this : NULL;
}

void Widget::setShortcut(int key) {
	m_shortcut = key;
}

void Widget::setEnabled(bool enable) {
	m_enabled = enable;
}
