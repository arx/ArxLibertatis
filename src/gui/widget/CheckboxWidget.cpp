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

#include "gui/widget/CheckboxWidget.h"

#include "core/Config.h"
#include "core/Core.h"
#include "graphics/DrawLine.h"
#include "graphics/Renderer.h"
#include "graphics/data/TextureContainer.h"
#include "gui/Hud.h"
#include "gui/MenuPublic.h"
#include "gui/menu/MenuCursor.h"
#include "gui/widget/TextWidget.h"
#include "scene/GameSound.h"

CheckboxWidget::CheckboxWidget(TextWidget * label)
	: Widget()
{
	pRef = this; // TODO remove this
	m_id = BUTTON_INVALID; // TODO remove this
	
	arx_assert(label);
	
	m_textureOff = TextureContainer::Load("graph/interface/menus/menu_checkbox_off");
	m_textureOn = TextureContainer::Load("graph/interface/menus/menu_checkbox_on");
	arx_assert(m_textureOff);
	arx_assert(m_textureOn);
	arx_assert(m_textureOff->size() == m_textureOn->size());
	
	iState    = 0;
	iOldState = -1;
	
	m_label = label;
	m_rect = label->m_rect;
	
	m_rect.right = m_rect.left + RATIO_X(245.f);
}

CheckboxWidget::~CheckboxWidget() {
	delete m_label;
}

void CheckboxWidget::Move(const Vec2f & offset) {
	
	Widget::Move(offset);
	m_label->Move(offset);
}

bool CheckboxWidget::OnMouseClick() {
	
	if(iOldState<0)
		iOldState=iState;

	iState ++;

	//NB : It seems that iState cannot be negative (used as tabular index / used as bool) but need further approval
	arx_assert(iState >= 0);

	if((size_t)iState >= 2) {
		iState = 0;
	}

	ARX_SOUND_PlayMenu(SND_MENU_CLICK);
	
	if(stateChanged) {
		stateChanged(iState);
	}
	
	return false;
}

void CheckboxWidget::Update() {
}

void CheckboxWidget::renderCommon() {
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(BlendOne, BlendOne);
	
	Rectf checkboxRect;
	checkboxRect.top = m_rect.top;
	checkboxRect.left = m_rect.right - m_rect.height();
	checkboxRect.bottom = m_rect.bottom;
	checkboxRect.right = m_rect.right;
	
	TextureContainer *pTex = (iState == 0) ? m_textureOff : m_textureOn;
	Color color = (bCheck) ? Color::white : Color(63, 63, 63, 255);
	
	EERIEDrawBitmap2(checkboxRect, 0.f, pTex, color);
}

void CheckboxWidget::Render() {

	renderCommon();
	
	m_label->Render();
}

extern MenuCursor * pMenuCursor;

void CheckboxWidget::RenderMouseOver() {

	pMenuCursor->SetMouseOver();

	renderCommon();
	
	m_label->RenderMouseOver();
}
