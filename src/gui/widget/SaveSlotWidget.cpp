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
#include <ctime>

#include "core/Core.h"
#include "core/Localisation.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/font/Font.h"
#include "gui/Text.h"
#include "gui/menu/MenuCursor.h"
#include "scene/GameSound.h"
#include "util/Unicode.h"

SaveSlotWidget::SaveSlotWidget(SavegameHandle savegame, size_t i, Font * font, const Rectf & rect)
	: m_font(font)
	, m_savegame(savegame)
	, m_dateOffset(rect.width())
	, m_selected(false)
{
	
	m_rect = rect;
	m_rect.bottom = m_rect.top + font->getLineHeight();
	
	if(savegame == SavegameHandle()) {
		
		std::ostringstream text;
		text << '-' << std::setfill('0') << std::setw(4) << i << '-';
		m_name = text.str();
		
	} else {
		
		const SaveGame & save = savegames[savegame.handleData()];
		
		m_dateOffset = rect.width() - font->getTextSize("0000-00-00  00:00").width();
		
		if(save.quicksave) {
			
			std::ostringstream text;
			text << getLocalised("system_menus_main_quickloadsave", "Quicksave") << ' ' << i;
			m_name = text.str();
			
		} else {
			
			m_name = save.name;
			size_t length = save.name.length();
			while(length > 0 && font->getTextSize(m_name).width() > m_dateOffset - RATIO_X(20)) {
				length--;
				while(length > 0 && util::UTF8::isContinuationByte(save.name[length])) {
					length--;
				}
				m_name = save.name.substr(0, length) + "…";
			}
			
		}
		
		const struct tm & t = *std::localtime(&save.stime);
		
		{
			std::ostringstream oss;
			oss << std::setfill('0') << (t.tm_year + 1900) << "-"
			    << std::setw(2) << (t.tm_mon + 1) << "-"
			    << std::setw(2) << t.tm_mday;
			m_date = oss.str();
		}
		
		{
			std::ostringstream oss;
			oss << t.tm_hour << ":" << std::setfill('0') << std::setw(2) << t.tm_min;
			m_time = oss.str();
		}
		
	}
	
}

bool SaveSlotWidget::click() {
	
	bool result = Widget::click();
	
	if(!m_enabled) {
		return result;
	}
	
	ARX_SOUND_PlayMenu(SND_MENU_CLICK);
	
	if(clicked) {
		clicked(this);
	}
	
	return result;
}

bool SaveSlotWidget::doubleClick() {
	
	bool result = Widget::click();
	
	if(m_enabled) {
		if(doubleClicked) {
			doubleClicked(this);
		} else if(clicked) {
			clicked(this);
		}
	}
	
	return result;
}

void SaveSlotWidget::render(bool mouseOver) {
	
	Color color = Color(232, 204, 142);
	if(!m_enabled) {
		color = Color::grayb(127);
	} else if(m_selected || mouseOver) {
		color = Color::white;
	}
	
	ARX_UNICODE_DrawTextInRect(m_font, m_rect.topLeft(), m_rect.right, m_name, color);
	
	if(!m_date.empty()) {
		Vec2f datePos = m_rect.topLeft() + Vec2f(m_dateOffset, 0.f);
		ARX_UNICODE_DrawTextInRect(m_font, datePos, m_rect.right, m_date, color);
	}
	
	if(!m_time.empty()) {
		Vec2f timePos = m_rect.topRight() - Vec2f(m_font->getTextSize(m_time).width(), 0.f);
		ARX_UNICODE_DrawTextInRect(m_font, timePos, m_rect.right, m_time, color);
	}
	
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
