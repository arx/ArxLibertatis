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

#include "gui/widget/ButtonWidget.h"

#include "core/Core.h"
#include "graphics/DrawLine.h"
#include "graphics/data/TextureContainer.h"
#include "gui/menu/MenuCursor.h"
#include "input/Input.h"
#include "scene/GameSound.h"

ButtonWidget::ButtonWidget(const Vec2f & pos, const Vec2f & size, const char * texturePath)
	: Widget()
{
	m_pos = pos;
	m_size = size;
	
	pRef = this; //TODO remove this
	m_id = BUTTON_INVALID; //TODO remove this
	
	m_texture = TextureContainer::Load(texturePath);
	arx_assert(m_texture);
	
	Vec2f scaledPos = RATIO_2(pos);
	Vec2f scaledSize = RATIO_2(m_size);
	m_rect = Rectf(scaledPos, scaledSize.x, scaledSize.y);
}

ButtonWidget::~ButtonWidget() {
}

void ButtonWidget::SetPos(Vec2f pos) {
	
	Vec2f scaledSize = RATIO_2(m_size);
	m_rect = Rectf(pos, scaledSize.x, scaledSize.y);
}

bool ButtonWidget::OnMouseClick() {
	
	if(!enabled) {
		return false;
	}
	
	ARX_SOUND_PlayMenu(SND_MENU_CLICK);
	
	if(clicked) {
		clicked();
	}
	
	return false;
}

void ButtonWidget::Update() {
}

void ButtonWidget::Render() {

	Color color = (bCheck) ? Color::white : Color(63, 63, 63, 255);
	EERIEDrawBitmap2(m_rect, 0, m_texture, color);
}

extern MenuCursor * pMenuCursor;

void ButtonWidget::RenderMouseOver() {

	pMenuCursor->SetMouseOver();
	
	const Vec2f cursor = Vec2f(GInput->getMousePosAbs());
	if(m_rect.contains(cursor)) {
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		GRenderer->SetBlendFunc(BlendOne, BlendOne);
		
		Render();
		
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	}
}
