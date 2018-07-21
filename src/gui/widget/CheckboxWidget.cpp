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

CheckboxWidget::CheckboxWidget(Font * font, const std::string & label, float width) 
	: m_label(new TextWidget(font, label, Vec2f_ZERO))
	, m_checked(false)
{
	
	m_label->forceDisplay(TextWidget::Dynamic);
	
	m_textureOff = TextureContainer::Load("graph/interface/menus/menu_checkbox_off");
	m_textureOn = TextureContainer::Load("graph/interface/menus/menu_checkbox_on");
	arx_assert(m_textureOff);
	arx_assert(m_textureOn);
	
	m_rect = Rectf(width, m_label->m_rect.height());
	
}

CheckboxWidget::~CheckboxWidget() {
	delete m_label;
}

void CheckboxWidget::Move(const Vec2f & offset) {
	Widget::Move(offset);
	m_label->Move(offset);
}

bool CheckboxWidget::click() {
	
	bool result = Widget::click();
	
	ARX_SOUND_PlayMenu(SND_MENU_CLICK);
	
	setChecked(!m_checked);
	
	return result;
}

void CheckboxWidget::render(bool mouseOver) {
	
	float size = RATIO_Y(20);
	
	Rectf checkboxRect;
	checkboxRect.top = std::floor(m_rect.center().y - size / 2.f);
	checkboxRect.bottom = checkboxRect.top + size;
	checkboxRect.left = std::floor(m_rect.right - RATIO_X(56.f) - size / 2);
	checkboxRect.right = checkboxRect.left + size;
	
	TextureContainer * pTex = (m_checked ? m_textureOn : m_textureOff);
	Color color = m_enabled ? Color::white : Color(63, 63, 63, 255);
	
	m_label->render(mouseOver);
	
	UseRenderState state(render2D().blendAdditive());
	
	EERIEDrawBitmap(checkboxRect, 0.f, pTex, color);
	
	if(mouseOver) {
		EERIEDrawBitmap(checkboxRect, 0.f, pTex, color);
	}
	
}

void CheckboxWidget::setChecked(bool checked) {
	
	if(checked == m_checked) {
		return;
	}
	
	m_checked = checked;
	
	if(stateChanged) {
		stateChanged(m_checked);
	}
	
}
