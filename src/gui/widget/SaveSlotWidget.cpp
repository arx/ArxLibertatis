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

#include <sstream>
#include <iomanip>

#include "core/Localisation.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/font/Font.h"
#include "gui/menu/MenuCursor.h"
#include "util/Unicode.h"

SaveSlotWidget::SaveSlotWidget(SavegameHandle savegame, size_t i, Font * font, const Rectf & rect)
	: TextWidget(font, std::string(), rect.topLeft())
	, m_savegame(savegame)
{
	
	if(savegame == SavegameHandle()) {
		
		std::ostringstream text;
		text << '-' << std::setfill('0') << std::setw(4) << i << '-';
		setText(text.str());
		
	} else {
		
		const SaveGame & save = savegames[savegame.handleData()];
		
		if(save.quicksave) {
			
			std::ostringstream text;
			text << getLocalised("system_menus_main_quickloadsave", "Quicksave") << ' ' << i << "   " << save.time;
			setText(text.str());
			
		} else {
			
			std::string text = save.name +  "   " + save.time;
			size_t length = save.name.length();
			while(length > 0 && font->getTextSize(text).width() > rect.width()) {
				length--;
				while(length > 0 && util::UTF8::isContinuationByte(save.name[length])) {
					length--;
				}
				text = save.name.substr(0, length) + "…   " + save.time;
			}
			setText(text);
			
		}
		
	}
	
}

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
