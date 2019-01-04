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

#include "gui/widget/CycleTextWidget.h"

#include <algorithm>

#include <boost/foreach.hpp>

#include "core/Core.h"
#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "gui/MenuPublic.h"
#include "gui/menu/MenuCursor.h"
#include "gui/widget/ButtonWidget.h"
#include "gui/widget/TextWidget.h"
#include "input/Input.h"

CycleTextWidget::CycleTextWidget(const Vec2f & size, Font * font, const std::string & label, Font * entryFont)
	: m_label(new TextWidget(font, label))
	, m_left(new ButtonWidget(Vec2f(size.y), "graph/interface/menus/menu_slider_button_left"))
	, m_right(new ButtonWidget(Vec2f(size.y), "graph/interface/menus/menu_slider_button_right"))
	, m_font(entryFont ? entryFont : font)
	, m_content(10 * size.y / 2, size.y)
	, m_value(0)
{
	
	m_label->forceDisplay(TextWidget::Dynamic);
	
	float minWidth = m_left->m_rect.width() + m_content.width() + m_right->m_rect.width();
	m_rect = Rectf(std::max(minWidth, size.x), std::max(m_content.height(), m_label->m_rect.height()));
	
	m_right->setPosition(Vec2f(m_rect.right - m_right->m_rect.width(),
	                           m_rect.center().y - m_right->m_rect.height() / 2));
	m_content.moveTo(Vec2f(m_right->m_rect.left - m_content.width(),
	                       m_rect.center().y - m_content.height() / 2));
	m_left->setPosition(Vec2f(m_content.left - m_left->m_rect.width(),
	                          m_rect.center().y - m_left->m_rect.height() / 2));
	
}

CycleTextWidget::~CycleTextWidget() {
	
	delete m_label;
	delete m_left;
	delete m_right;
	
	BOOST_FOREACH(Widget * entry, m_entries) {
		delete entry;
	}
	
}

void CycleTextWidget::selectLast() {
	m_value = int(m_entries.size() - 1);
}

void CycleTextWidget::addEntry(const std::string & label) {
	
	TextWidget * widget = new TextWidget(m_font, label);
	
	widget->forceDisplay(TextWidget::Dynamic);
	widget->setEnabled(m_enabled);
	
	m_entries.push_back(widget);
	
	float maxWidth = m_rect.width() - m_left->m_rect.width() - m_right->m_rect.width()
	                  - m_label->m_rect.width() - m_label->m_rect.height();
	maxWidth = std::max(maxWidth, m_content.width());
	if(widget->m_rect.width() > maxWidth) {
		widget->m_rect.right = widget->m_rect.left + maxWidth;
	}
	m_rect.bottom = m_rect.top + std::max(m_rect.height(), widget->m_rect.height());
	
	m_content.left = m_content.right - std::max(m_content.width(), widget->m_rect.width());
	m_content.bottom = m_content.top + m_rect.height();
	
	m_right->setPosition(Vec2f(m_rect.right - m_right->m_rect.width(),
	                           m_rect.center().y - m_right->m_rect.height() / 2));
	m_content.moveTo(Vec2f(m_right->m_rect.left - m_content.width(),
	                       m_rect.center().y - m_content.height() / 2));
	m_left->setPosition(Vec2f(m_content.left - m_left->m_rect.width(),
	                          m_rect.center().y - m_left->m_rect.height() / 2));
	
	m_label->setPosition(Vec2f(m_rect.left, m_rect.center().y - m_label->m_rect.height() / 2));
	
	BOOST_FOREACH(Widget * entry, m_entries) {
		entry->setPosition(m_content.center() - entry->m_rect.size() / 2.f);
	}
	
}

void CycleTextWidget::move(const Vec2f & offset) {
	
	Widget::move(offset);
	
	m_label->move(offset);
	m_left->move(offset);
	m_content.move(offset);
	m_right->move(offset);
	
	BOOST_FOREACH(Widget * entry, m_entries) {
		entry->move(offset);
	}
	
}

void CycleTextWidget::hover() {

	if(GInput->isKeyPressedNowPressed(Keyboard::Key_LeftArrow) || GInput->getMouseWheelDir() < 0) {
		newValue(m_value - 1);
	} else if(GInput->isKeyPressedNowPressed(Keyboard::Key_RightArrow) || GInput->getMouseWheelDir() > 0) {
		newValue(m_value + 1);
	}
	
}

bool CycleTextWidget::click() {
	
	bool result = Widget::click();
	
	if(!m_enabled) {
		return result;
	}
	
	const Vec2f cursor = Vec2f(GInput->getMousePosition());
	
	if(m_rect.contains(cursor)) {
		if(m_left->m_rect.contains(cursor)) {
			newValue(m_value - 1);
		} else {
			newValue(m_value + 1);
		}
	}
	
	return result;
}

void CycleTextWidget::setEnabled(bool enable) {
	
	Widget::setEnabled(enable);
	
	m_left->setEnabled(enable);
	m_right->setEnabled(enable);
	
	BOOST_FOREACH(Widget * entry, m_entries) {
		entry->setEnabled(enable);
	}
	
}

void CycleTextWidget::render(bool mouseOver) {
	
	m_label->render(mouseOver);
	
	Vec2f cursor = Vec2f(GInput->getMousePosition());
	
	if(m_enabled) {
		m_left->render(m_left->m_rect.contains(cursor));
		m_right->render(m_right->m_rect.contains(cursor));
	}
	
	if(m_value >= 0 && size_t(m_value) < m_entries.size()) {
		m_entries[size_t(m_value)]->render(m_content.contains(cursor));
	}
	
}

void CycleTextWidget::newValue(int value) {
	
	value = positive_modulo(value, int(m_entries.size()));
	
	if(value == m_value) {
		return;
	}
	
	m_value = value;
	
	if(valueChanged) {
		valueChanged(m_value, m_entries[size_t(m_value)]->text());
	}
	
}
