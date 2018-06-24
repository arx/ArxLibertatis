/*
 * Copyright 2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "gui/widget/SaveSlotWidget.h"

#include "graphics/data/TextureContainer.h"
#include "gui/menu/MenuCursor.h"

SaveSlotWidget::SaveSlotWidget(SavegameHandle savegame, Font * font, const std::string & text, Vec2f pos)
	: TextWidget(font, text, pos)
	, m_savegame(savegame)
{ }

void SaveSlotWidget::render(bool mouseOver) {
	
	TextWidget::render(mouseOver);
	
	if(!mouseOver || m_savegame == SavegameHandle()) {
		return;
	}
	
	const res::path & image = savegames[m_savegame.handleData()].thumbnail;
	if(!image.empty()) {
		TextureContainer * t = TextureContainer::LoadUI(image, TextureContainer::NoColorKey);
		if(t != g_thumbnailCursor.m_loadTexture) {
			delete g_thumbnailCursor.m_loadTexture;
			g_thumbnailCursor.m_loadTexture = t;
		}
		g_thumbnailCursor.m_renderTexture = g_thumbnailCursor.m_loadTexture;
	}
	
}
