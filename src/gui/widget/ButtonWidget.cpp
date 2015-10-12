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

ButtonWidget::ButtonWidget(Vec2i pos, const char * texturePath)
	: Widget()
{
	pRef = this; //TODO remove this
	
	m_texture = TextureContainer::Load(texturePath);
	arx_assert(m_texture);
	
	m_id = BUTTON_INVALID;

	m_rect.left=pos.x;
	m_rect.top=pos.y;
	m_rect.right  = m_rect.left ;
	m_rect.bottom = m_rect.top ;
	
	s32 rZoneR = m_rect.left + RATIO_X(m_texture->m_size.x);
	s32 rZoneB = m_rect.top + RATIO_Y(m_texture->m_size.y);
	m_rect.right  = std::max(m_rect.right,  rZoneR);
	m_rect.bottom = std::max(m_rect.bottom, rZoneB);
}

ButtonWidget::~ButtonWidget() {
}

void ButtonWidget::SetPos(Vec2i pos)
{
	Widget::SetPos(pos);
	
	int iWidth = RATIO_X(m_texture->m_size.x);
	int iHeight = RATIO_Y(m_texture->m_size.y);
	
	m_rect.right = pos.x + iWidth;
	m_rect.bottom = pos.y + iHeight;
}

bool ButtonWidget::OnMouseClick() {
	
	if(!enabled) {
		return false;
	}
	
	ARX_SOUND_PlayMenu(SND_MENU_CLICK);

	return false;
}

void ButtonWidget::Update(int _iDTime) {
	(void)_iDTime;
}

// TODO remove this
extern bool bNoMenu;

void ButtonWidget::Render() {

	if(bNoMenu)
		return;
	
	Color color = (bCheck) ? Color::white : Color(63, 63, 63, 255);
	EERIEDrawBitmap2(Rectf(m_rect), 0, m_texture, color);
}

extern MenuCursor * pMenuCursor;

void ButtonWidget::RenderMouseOver() {

	if(bNoMenu)
		return;
	
	pMenuCursor->SetMouseOver();
	
	const Vec2i cursor = Vec2i(GInput->getMousePosAbs());
	if(m_rect.contains(cursor)) {
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
		
		Render();
		
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	}
}
