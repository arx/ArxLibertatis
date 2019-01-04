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

#include "gui/widget/ButtonWidget.h"

#include "core/Core.h"
#include "graphics/DrawLine.h"
#include "graphics/data/TextureContainer.h"
#include "gui/menu/MenuCursor.h"
#include "input/Input.h"

ButtonWidget::ButtonWidget(const Vec2f & size, const res::path & texture)
	: m_texture(TextureContainer::Load(texture))
{
	
	arx_assert(m_texture);
	
	m_rect = Rectf(size.x, size.y);
	
}

ButtonWidget::~ButtonWidget() { }

void ButtonWidget::render(bool mouseOver) {
	
	UseRenderState state(render2D().blendAdditive());
	
	Color color = m_enabled ? Color::white : Color::gray(0.25f);
	
	EERIEDrawBitmap(m_rect, 0, m_texture, color);
	
	if(mouseOver) {
		EERIEDrawBitmap(m_rect, 0, m_texture, color);
	}
	
}
