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

#include "gui/widget/TextWidget.h"

#include "core/Core.h"
#include "graphics/Color.h"
#include "graphics/font/Font.h"
#include "gui/Text.h"

TextWidget::TextWidget(Font * font, const std::string & text)
	: m_font(font)
	, m_display(Automatic)
{
	setText(text);
}

void TextWidget::setText(const std::string & text) {
	m_text = text;
	Vec2i textSize = m_font->getTextSize(m_text);
	m_rect = Rectf(m_rect.topLeft(), float(textSize.x + 1), float(textSize.y + 1));
}

void TextWidget::render(bool mouseOver) {
	
	Color color = Color(232, 204, 142);
	bool hasAction = targetPage() != NOP || clicked || doubleClicked;
	bool dynamic =  m_display == Dynamic || (m_display == Automatic && hasAction);
	if(m_display == Disabled || (dynamic && !m_enabled)) {
		color = Color::gray(0.5f);
	} else if(m_display == MouseOver || (dynamic && mouseOver)) {
		color = Color::white;
	}
	
	ARX_UNICODE_DrawTextInRect(m_font, m_rect.topLeft(), m_rect.right, m_text, color, NULL);
	
}
