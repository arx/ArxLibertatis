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

#include "gui/widget/CycleTextWidget.h"

#include <boost/foreach.hpp>

#include "graphics/Renderer.h"
#include "gui/MenuPublic.h"
#include "gui/menu/MenuCursor.h"
#include "input/Input.h"
#include "scene/GameSound.h"

CycleTextWidget::CycleTextWidget(MenuButton _iID)
	: Widget()
{
	m_id = _iID;
	
	pLeftButton = new ButtonWidget(Vec2i_ZERO, "graph/interface/menus/menu_slider_button_left");
	pRightButton = new ButtonWidget(Vec2i_ZERO, "graph/interface/menus/menu_slider_button_right");

	vText.clear();

	iPos = 0;
	iOldPos = -1;

	m_rect.left   = 0;
	m_rect.top    = 0;
	m_rect.right  = pLeftButton->m_rect.width() + pRightButton->m_rect.width();
	m_rect.bottom = std::max(pLeftButton->m_rect.height(), pRightButton->m_rect.height());

	pRef = this;
}

CycleTextWidget::~CycleTextWidget() {
	delete pLeftButton;
	delete pRightButton;
	BOOST_FOREACH(TextWidget * e, vText) {
		delete e;
	}
}

void CycleTextWidget::selectLast() {
	iPos = vText.size() - 1;
}

void CycleTextWidget::AddText(TextWidget *_pText) {
	
	_pText->setEnabled(enabled);
	
	_pText->Move(Vec2i(m_rect.left + pLeftButton->m_rect.width(), m_rect.top + 0));
	vText.push_back(_pText);

	Vec2i textSize = _pText->m_rect.size();

	m_rect.right  = std::max(m_rect.right, m_rect.left + pLeftButton->m_rect.width() + pRightButton->m_rect.width() + textSize.x);
	m_rect.bottom = std::max(m_rect.bottom, m_rect.top + textSize.y);

	pLeftButton->SetPos(Vec2i(m_rect.left,
	                          m_rect.top + m_rect.height() / 2 - pLeftButton->m_rect.height() / 2));
	pRightButton->SetPos(Vec2i(m_rect.right - pRightButton->m_rect.width(),
	                           m_rect.top + m_rect.height() / 2 - pRightButton->m_rect.height() / 2));

	int dx=m_rect.width()-pLeftButton->m_rect.width()-pRightButton->m_rect.width();
	//on recentre tout
	std::vector<TextWidget*>::iterator it;

	for(it = vText.begin(); it < vText.end(); ++it) {
		TextWidget *pMenuElementText=*it;
		
		textSize = pMenuElementText->m_rect.size();

		int dxx=(dx-textSize.x)>>1;
		pMenuElementText->SetPos(Vec2i(pLeftButton->m_rect.right + dxx, m_rect.top + m_rect.height() / 2 - textSize.y/2));
	}
}

void CycleTextWidget::Move(const Vec2i & offset) {

	Widget::Move(offset);

	pLeftButton->Move(offset);
	pRightButton->Move(offset);

	for(std::vector<TextWidget*>::const_iterator i = vText.begin(), i_end = vText.end(); i != i_end; ++i)
		(*i)->Move(offset);
}

void CycleTextWidget::EmptyFunction() {

	//Touche pour la selection
	if(GInput->isKeyPressedNowPressed(Keyboard::Key_LeftArrow)) {
		iPos--;

		if(iPos <= 0)
			iPos = 0;
	} else {
		if(GInput->isKeyPressedNowPressed(Keyboard::Key_RightArrow)) {
			iPos++;

			arx_assert(iPos >= 0);

			if((size_t)iPos >= vText.size() - 1)
				iPos = vText.size() - 1;
		}
	}
}

bool CycleTextWidget::OnMouseClick() {
	
	if(!enabled) {
		return false;
	}
	
	ARX_SOUND_PlayMenu(SND_MENU_CLICK);

	if(iOldPos<0)
		iOldPos=iPos;
	
	const Vec2i cursor = Vec2i(GInput->getMousePosAbs());

	if(m_rect.contains(cursor)) {
		if(pLeftButton->m_rect.contains(cursor)) {
			iPos--;

			if(iPos < 0) {
				iPos = vText.size() - 1;
			}
		}
		
		if(pRightButton->m_rect.contains(cursor)) {
			iPos++;

			arx_assert(iPos >= 0);

			if(size_t(iPos) >= vText.size()) {
				iPos = 0;
			}
		}
	}

	if(m_onChange) {
		m_onChange(iPos, vText.at(iPos)->m_text);
	}
	
	return false;
}

void CycleTextWidget::Update(int _iTime) {

	pLeftButton->Update(_iTime);
	pRightButton->Update(_iTime);
}

// TODO remove this
extern bool bNoMenu;

void CycleTextWidget::Render() {
	
	if(bNoMenu)
		return;
	
	if(enabled) {
		pLeftButton->Render();
		pRightButton->Render();
	}
	
	if(iPos >= 0 && size_t(iPos) < vText.size() && vText[iPos]) {
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		vText[iPos]->Render();
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	}
}

void CycleTextWidget::setEnabled(bool enable) {
	Widget::setEnabled(enable);
	pLeftButton->setEnabled(enable);
	pRightButton->setEnabled(enable);
	for(size_t i = 0; i < vText.size(); i++) {
		vText[i]->setEnabled(enable);
	}
}

extern MenuCursor * pMenuCursor;

void CycleTextWidget::RenderMouseOver() {

	if(bNoMenu)
		return;

	pMenuCursor->SetMouseOver();

	Vec2i cursor = Vec2i(GInput->getMousePosAbs());
	
	if(!enabled) {
		return;
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);

	if(m_rect.contains(cursor)) {
		if(pLeftButton->m_rect.contains(cursor)) {
			pLeftButton->Render();
		}
		
		if(pRightButton->m_rect.contains(cursor)) {
			pRightButton->Render();
		}
	}
}
