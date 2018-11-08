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

#include <boost/lexical_cast.hpp>

#include "core/Core.h"
#include "core/Localisation.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/font/Font.h"
#include "gui/Text.h"
#include "gui/menu/MenuCursor.h"
#include "util/Unicode.h"

SaveSlotWidget::SaveSlotWidget(SavegameHandle savegame, size_t i, Font * font, const Rectf & rect)
	: m_font(font)
	, m_savegame(savegame)
	, m_dateOffset(rect.width())
	, m_selected(false)
{
	
	m_rect = rect;
	m_rect.bottom = m_rect.top + float(font->getLineHeight());
	
	if(savegame == SavegameHandle()) {
		
		std::ostringstream text;
		text << '-' << std::setfill('0') << std::setw(4) << i << '-';
		m_name = text.str();
		
	} else {
		
		const SaveGame & save = savegames[savegame];
		
		m_dateOffset = rect.width() - float(font->getTextSize("0000-00-00    00:00").width());
		
		if(save.quicksave) {
			
			std::ostringstream text;
			text << getLocalised("system_menus_main_quickloadsave") << " #" << i;
			m_name = text.str();
			
		} else {
			
			m_name = save.name;
			size_t length = save.name.length();
			while(length > 0 && float(font->getTextSize(m_name).width()) > m_dateOffset - m_rect.height()) {
				length--;
				while(length > 0 && util::UTF8::isContinuationByte(save.name[length])) {
					length--;
				}
				m_name = save.name.substr(0, length) + "…";
			}
			
		}
		
		std::time_t now = std::time(NULL);
		std::tm n = *std::localtime(&now);
		std::tm t = *std::localtime(&save.stime);
		
		if(t.tm_year == n.tm_year && t.tm_yday == n.tm_yday) {
			m_date = getLocalised("system_today");
		} else if(t.tm_year == n.tm_year && t.tm_yday + 1 == n.tm_yday) {
			m_date = getLocalised("system_yesterday");
		} else if(t.tm_year == n.tm_year && t.tm_yday <= n.tm_yday && t.tm_yday + 7 > n.tm_yday) {
			switch(t.tm_wday) {
				case 0: m_date = getLocalised("system_sunday"); break;
				case 1: m_date = getLocalised("system_monday"); break;
				case 2: m_date = getLocalised("system_tuesday"); break;
				case 3: m_date = getLocalised("system_wednesday"); break;
				case 4: m_date = getLocalised("system_thursday"); break;
				case 5: m_date = getLocalised("system_saturday"); break;
				case 6: m_date = getLocalised("system_friday"); break;
				default: arx_unreachable();
			}
		} else if((t.tm_year == n.tm_year && t.tm_mon <= n.tm_mon)
		          || (t.tm_year + 1 == n.tm_year && t.tm_mon + 12 > n.tm_mon)) {
			switch(t.tm_mon) {
				case 0: m_date = getLocalised("system_january"); break;
				case 1: m_date = getLocalised("system_february"); break;
				case 2: m_date = getLocalised("system_march"); break;
				case 3: m_date = getLocalised("system_april"); break;
				case 4: m_date = getLocalised("system_may"); break;
				case 5: m_date = getLocalised("system_june"); break;
				case 6: m_date = getLocalised("system_july"); break;
				case 7: m_date = getLocalised("system_august"); break;
				case 8: m_date = getLocalised("system_september"); break;
				case 9: m_date = getLocalised("system_october"); break;
				case 10: m_date = getLocalised("system_november"); break;
				case 11: m_date = getLocalised("system_december"); break;
				default: arx_unreachable();
			}
			m_date += " " + boost::lexical_cast<std::string>(t.tm_mday);
		} else {
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

void SaveSlotWidget::render(bool mouseOver) {
	
	Color color = Color(232, 204, 142);
	if(!m_enabled) {
		color = Color::gray(0.5f);
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
	
	const res::path & image = savegames[m_savegame].thumbnail;
	if(!image.empty()) {
		TextureContainer * t = TextureContainer::LoadUI(image, TextureContainer::NoColorKey);
		if(t != g_thumbnailCursor.m_loadTexture) {
			delete g_thumbnailCursor.m_loadTexture;
			g_thumbnailCursor.m_loadTexture = t;
		}
		g_thumbnailCursor.m_renderTexture = g_thumbnailCursor.m_loadTexture;
	}
	
}
