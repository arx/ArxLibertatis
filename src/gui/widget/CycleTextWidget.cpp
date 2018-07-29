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

#include "gui/widget/CycleTextWidget.h"

#include <algorithm>

#include <boost/foreach.hpp>

#include "core/Core.h"
#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "gui/MenuPublic.h"
#include "gui/menu/MenuCursor.h"
#include "input/Input.h"
#include "scene/GameSound.h"

CycleTextWidget::CycleTextWidget() {
	
	Vec2f buttonSize = RATIO_2(Vec2f(16, 16));
	
	pLeftButton = new ButtonWidget(buttonSize, "graph/interface/menus/menu_slider_button_left");
	pRightButton = new ButtonWidget(buttonSize, "graph/interface/menus/menu_slider_button_right");

	vText.clear();

	m_value = 0;

	m_rect.left   = 0;
	m_rect.top    = 0;
	m_rect.right  = pLeftButton->m_rect.width() + RATIO_X(80) + pRightButton->m_rect.width();
	m_rect.bottom = std::max(pLeftButton->m_rect.height(), pRightButton->m_rect.height());
}

CycleTextWidget::~CycleTextWidget() {
	delete pLeftButton;
	delete pRightButton;
	
	BOOST_FOREACH(Widget * w, vText) {
		delete w;
	}
}

void CycleTextWidget::selectLast() {
	m_value = int(vText.size() - 1);
}

void CycleTextWidget::AddText(TextWidget * _pText) {
	
	_pText->forceDisplay(TextWidget::Dynamic);
	_pText->setEnabled(m_enabled);
	
	_pText->Move(Vec2f(m_rect.left + pLeftButton->m_rect.width(), m_rect.top + 0));
	vText.push_back(_pText);

	Vec2f textSize = _pText->m_rect.size();

	m_rect.right  = std::max(m_rect.right, m_rect.left + pLeftButton->m_rect.width() + pRightButton->m_rect.width() + textSize.x);
	m_rect.bottom = std::max(m_rect.bottom, m_rect.top + textSize.y);

	pLeftButton->SetPos(Vec2f(m_rect.left,
	                          m_rect.top + m_rect.height() / 2 - pLeftButton->m_rect.height() / 2));
	pRightButton->SetPos(Vec2f(m_rect.right - pRightButton->m_rect.width(),
	                           m_rect.top + m_rect.height() / 2 - pRightButton->m_rect.height() / 2));

	float dx = m_rect.width() - pLeftButton->m_rect.width() - pRightButton->m_rect.width();
	
	for(std::vector<TextWidget *>::iterator it = vText.begin(); it < vText.end(); ++it) {
		TextWidget * pMenuElementText = *it;
		textSize = pMenuElementText->m_rect.size();
		float dxx = (dx - textSize.x) / 2.f;
		pMenuElementText->SetPos(Vec2f(pLeftButton->m_rect.right + dxx, m_rect.top + m_rect.height() / 2 - textSize.y / 2));
	}
	
}

void CycleTextWidget::Move(const Vec2f & offset) {
	
	Widget::Move(offset);
	
	pLeftButton->Move(offset);
	pRightButton->Move(offset);
	
	for(std::vector<TextWidget *>::const_iterator i = vText.begin(), i_end = vText.end(); i != i_end; ++i) {
		(*i)->Move(offset);
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
	
	ARX_SOUND_PlayMenu(g_snd.MENU_CLICK);
	
	const Vec2f cursor = Vec2f(GInput->getMousePosition());
	
	if(m_rect.contains(cursor)) {
		if(pLeftButton->m_rect.contains(cursor)) {
			newValue(m_value - 1);
		} else {
			newValue(m_value + 1);
		}
	}
	
	return result;
}

void CycleTextWidget::setEnabled(bool enable) {
	Widget::setEnabled(enable);
	pLeftButton->setEnabled(enable);
	pRightButton->setEnabled(enable);
	for(size_t i = 0; i < vText.size(); i++) {
		vText[i]->setEnabled(enable);
	}
}

void CycleTextWidget::render(bool /* mouseOver */) {
	
	Vec2f cursor = Vec2f(GInput->getMousePosition());
	
	if(m_enabled) {
		pLeftButton->render(pLeftButton->m_rect.contains(cursor));
		pRightButton->render(pRightButton->m_rect.contains(cursor));
	}
	
	if(m_value >= 0 && size_t(m_value) < vText.size() && vText[m_value]) {
		vText[m_value]->render(vText[m_value]->m_rect.contains(cursor));
	}
	
}

void CycleTextWidget::newValue(int value) {
	
	m_value = positive_modulo(value, int(vText.size()));
	
	if(valueChanged) {
		valueChanged(m_value, vText[size_t(m_value)]->text());
	}
	
}
