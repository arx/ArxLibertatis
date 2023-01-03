/*
 * Copyright 2011-2022 Arx Libertatis Team (see the AUTHORS file)
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
/* Based on:
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code').

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/

#include "gui/menu/MenuPage.h"

#include <cmath>
#include <algorithm>
#include <utility>

#include "core/Core.h"
#include "graphics/DrawLine.h"
#include "graphics/font/Font.h"
#include "gui/Text.h"
#include "gui/menu/MenuCursor.h"
#include "gui/widget/ButtonWidget.h"
#include "input/Input.h"

MenuPage::MenuPage(MENUSTATE id)
	: m_rowSpacing(10)
	, m_id(id)
	, m_initialized(false)
	, m_selected(nullptr)
	, m_focused(nullptr)
	, m_disableShortcuts(false)
{ }

void MenuPage::addCorner(std::unique_ptr<Widget> widget, Anchor anchor) {
	
	arx_assert(widget);
	
	Vec2f pos(0.f);
	
	switch(anchor) {
		case TopLeft:
		case BottomLeft: {
			pos.x = m_rect.left;
			break;
		}
		case TopCenter:
		case BottomCenter: {
			pos.x = m_rect.center().x - widget->m_rect.width() / 2;
			break;
		}
		case TopRight:
		case BottomRight: {
			pos.x = m_rect.right - widget->m_rect.width();
			break;
		}
	}
	
	switch(anchor) {
		case TopLeft:
		case TopCenter:
		case TopRight: {
			pos.y = m_rect.top;
			break;
		}
		case BottomLeft:
		case BottomCenter:
		case BottomRight: {
			pos.y = m_rect.bottom - widget->m_rect.height();
			break;
		}
	}
	
	widget->setPosition(pos);
	
	m_children.add(std::move(widget));
	
}

void MenuPage::addCenter(std::unique_ptr<Widget> widget, bool centerX) {
	
	arx_assert(widget);
	
	float x = m_content.left;
	if(centerX) {
		x = std::floor(m_content.center().x - widget->m_rect.width() / 2.f);
	}
	
	float height = widget->m_rect.height();
	float whitespace = (widget->type() == WidgetType_Spacer) ? widget->m_rect.height() : 0.f;
	for(const Widget & child : m_children.widgets()) {
		height += RATIO_Y(m_rowSpacing);
		height += child.m_rect.height();
		whitespace += RATIO_Y(m_rowSpacing);
		if(child.type() == WidgetType_Spacer) {
			whitespace += child.m_rect.height();
		}
	}
	
	float squish = 1.f;
	if(height > m_content.height() && whitespace > 0.f) {
		height -= whitespace;
		squish = std::max(0.f, m_content.height() - height) / whitespace;
		height += whitespace * squish;
	}
	
	float y = std::floor(m_content.center().y - height / 2.f);
	
	for(Widget & child : m_children.widgets()) {
		child.setPosition(Vec2f(child.m_rect.left, y));
		y += child.m_rect.height() * ((child.type() == WidgetType_Spacer) ? squish : 1.f);
		y += RATIO_Y(m_rowSpacing) * squish;
	}
	
	widget->setPosition(Vec2f(x, y));
	
	m_children.add(std::move(widget));
	
}

void MenuPage::addBackButton(MENUSTATE page) {
	
	auto cb = std::make_unique<ButtonWidget>(buttonSize(16, 16), "graph/interface/menus/back");
	cb->setTargetPage(page);
	cb->setShortcut(Keyboard::Key_Escape);
	addCorner(std::move(cb), BottomLeft);
	
}

void MenuPage::update(Vec2f pos) {
	
	m_children.move(pos - m_rect.topLeft());
	
	m_content.move(pos - m_rect.topLeft());
	
	m_rect.moveTo(pos);
	
	m_children.update();
	
	if(m_focused && !m_focused->wantFocus()) {
		m_focused->unfocus();
		m_focused = nullptr;
		m_disableShortcuts = true;
	}
	
	if(m_disableShortcuts) {
		
		bool isShortcutPressed = false;
		
		for(const Widget & widget : m_children.widgets()) {
			if(widget.shortcut() != ActionKey::UNUSED && GInput->isKeyPressed(widget.shortcut())) {
				isShortcutPressed = true;
			}
		}
		
		if(!isShortcutPressed) {
			m_disableShortcuts = false;
		}
		
	} else {
		
		for(Widget & widget : m_children.widgets()) {
			
			if(m_focused && &widget != m_focused) {
				continue;
			}
			
			if(widget.shortcut() != ActionKey::UNUSED && GInput->isKeyPressedNowUnPressed(widget.shortcut())) {
				
				m_selected = &widget;
				
				if(widget.click() && &widget != m_focused) {
					m_focused = &widget;
				}
				
				return;
			}
			
		}
		
	}
	
	m_selected = m_children.getWidgetAt(Vec2f(GInput->getMousePosition()));
	
	if(m_focused && m_selected != m_focused && GInput->getMouseButton(Mouse::Button_0)) {
		m_focused->unfocus();
		m_focused = nullptr;
	}
	
	if(m_selected && (!m_focused || m_focused == m_selected)) {
		
		if(GInput->getMouseButtonDoubleClick(Mouse::Button_0)) {
			if(m_selected->doubleClick()) {
				m_focused = m_selected;
			}
			return;
		}
		
		if(GInput->getMouseButton(Mouse::Button_0)) {
			if(m_selected->click()) {
				m_focused = m_selected;
			}
			return;
		}
		
		m_selected->hover();
		
	}
	
}

void MenuPage::render() {
	
	m_children.render(m_selected);
	
	if(m_selected) {
		pMenuCursor->SetMouseOver();
	}
	
}

void MenuPage::drawDebug() {
	drawLineRectangle(m_rect, 0.f, Color::green);
	drawLineRectangle(m_content, 0.f, Color::blue);
	m_children.drawDebug();
}


void MenuPage::reserveTop() {
	m_content.top = m_rect.top + float(hFontMenu->getLineHeight()) + 5.f * minSizeRatio();
}

void MenuPage::reserveBottom() {
	m_content.bottom = m_rect.bottom - float(hFontMenu->getLineHeight()) - 5.f * minSizeRatio();
}

Vec2f MenuPage::buttonSize(float x, float y) const {
	return Vec2f(x, y) * minSizeRatio();
}

Vec2f MenuPage::checkboxSize() const {
	return Vec2f(m_rect.width(), 20.f * minSizeRatio());
}

Vec2f MenuPage::sliderSize() const {
	return Vec2f(m_rect.width(), 16.f * minSizeRatio());
}

void MenuPage::focus() {
	
	if(!m_initialized) {
		m_initialized = true;
		init();
	}
	
}

void MenuPage::unfocus() {
	
	if(m_focused) {
		m_focused->unfocus();
		m_focused = nullptr;
	}
	
}

void MenuPage::activate(Widget * widget) {
	
	if(m_focused != widget) {
		unfocus();
		if(widget->click()) {
			m_focused = widget;
		}
	}
	
}

