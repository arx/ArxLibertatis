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

#include "gui/widget/TextWidget.h"

#include "core/Core.h"
#include "graphics/font/Font.h"
#include "gui/Text.h"
#include "scene/GameSound.h"

TextWidget::TextWidget(Font * font, const std::string & text, Vec2f pos) {
	
	m_font = font;
	
	m_rect = Rectf(RATIO_2(pos), 0.f, 0.f);
	
	SetText(text);
	
	lColor = Color(232, 204, 142);
	lColorHighlight = lOldColor = Color::white;
	
	bSelected = false;
	
}

void TextWidget::SetText(const std::string & text) {
	m_text = text;
	Vec2i textSize = m_font->getTextSize(m_text);
	m_rect = Rectf(m_rect.topLeft(), textSize.x + 1, textSize.y + 1);
}

void TextWidget::OnMouseDoubleClick() {
	if(doubleClicked) {
		doubleClicked(this);
	}
}

bool TextWidget::OnMouseClick() {
	
	if(!enabled) {
		return false;
	}
	
	ARX_SOUND_PlayMenu(SND_MENU_CLICK);
	
	if(clicked) {
		clicked(this);
	}
	
	switch(eState) {
		case EDIT: {
			eState = EDIT_TIME;
			return true;
		}
		default: break;
	}
	
	return false;
}

void TextWidget::render(bool mouseOver) {
	
	Color color = lColor;
	if(!enabled) {
		color = Color::grayb(127);
	} else if(mouseOver || bSelected) {
		color = lColorHighlight;
	}
	
	ARX_UNICODE_DrawTextInRect(m_font, m_rect.topLeft(), m_rect.right, m_text, color, NULL);
	
}
