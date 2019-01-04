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

#include "gui/widget/CheckboxWidget.h"

#include <algorithm>

#include "core/Config.h"
#include "core/Core.h"
#include "graphics/DrawLine.h"
#include "graphics/Renderer.h"
#include "graphics/data/TextureContainer.h"
#include "gui/Hud.h"
#include "gui/MenuPublic.h"
#include "gui/menu/MenuCursor.h"
#include "gui/widget/TextWidget.h"

CheckboxWidget::CheckboxWidget(const Vec2f & size, Font * font, const std::string & label)
	: m_label(new TextWidget(font, label))
	, m_textureOff(TextureContainer::Load("graph/interface/menus/menu_checkbox_off"))
	, m_textureOn(TextureContainer::Load("graph/interface/menus/menu_checkbox_on"))
	, m_button(size.y, size.y)
	, m_checked(false)
{
	
	m_label->forceDisplay(TextWidget::Dynamic);
	
	arx_assert(m_textureOff);
	arx_assert(m_textureOn);
	
	m_rect = Rectf(std::max(m_button.width(), size.x), std::max(m_button.height(), m_label->m_rect.height()));
	
	float x = std::max(m_rect.right - 3.3f * m_button.width(), m_rect.center().x - m_button.width() / 2);
	float y = m_rect.center().y - m_button.height() / 2;
	m_button.moveTo(Vec2f(std::floor(x), std::floor(y)));
	
}

CheckboxWidget::~CheckboxWidget() {
	delete m_label;
}

void CheckboxWidget::move(const Vec2f & offset) {
	Widget::move(offset);
	m_label->move(offset);
	m_button.move(offset);
}

bool CheckboxWidget::click() {
	
	bool result = Widget::click();
	
	setChecked(!m_checked);
	
	return result;
}

void CheckboxWidget::render(bool mouseOver) {
	
	m_label->render(mouseOver);
	
	TextureContainer * texture = (m_checked ? m_textureOn : m_textureOff);
	Color color = m_enabled ? Color::white : Color::gray(0.25f);
	
	UseRenderState state(render2D().blendAdditive());
	
	EERIEDrawBitmap(m_button, 0.f, texture, color);
	
	if(mouseOver) {
		EERIEDrawBitmap(m_button, 0.f, texture, color);
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
