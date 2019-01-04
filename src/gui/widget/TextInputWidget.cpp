/*
 * Copyright 2015-2019 Arx Libertatis Team (see the AUTHORS file)
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

#include "gui/widget/TextInputWidget.h"

#include "core/Application.h"
#include "core/Core.h"
#include "core/GameTime.h"
#include "graphics/Draw.h"
#include "graphics/DrawLine.h"
#include "graphics/Renderer.h"
#include "graphics/font/Font.h"
#include "gui/Text.h"
#include "input/Input.h"
#include "scene/GameSound.h"
#include "window/RenderWindow.h"

TextInputWidget::TextInputWidget(Font * font, const std::string & text, const Rectf & rect)
	: m_font(font)
	, m_editing(false)
	, m_maxLength(std::numeric_limits<size_t>::max())
{
	m_rect = rect;
	m_rect.bottom = m_rect.top + m_font->getLineHeight() + 1;
	setText(text);
}

bool TextInputWidget::keyPressed(Keyboard::Key key, KeyModifiers mod) {
	
	switch(key) {
		
		case Keyboard::Key_Escape:
		case Keyboard::Key_Enter:
		case Keyboard::Key_NumPadEnter: {
			ARX_SOUND_PlayMenu(g_snd.MENU_CLICK);
			unfocus();
			return true;
		}
		
		case Keyboard::Key_A: {
			if(mod.control) {
				selectAll();
				return true;
			}
		}
		
		default: break;
	}
	
	return BasicTextInput::keyPressed(key, mod);
}

void TextInputWidget::textUpdated() {
	
	// Limit text size to fit into the widget
	while(text().length() > m_maxLength) {
		if(cursorPos() == 0) {
			eraseRight();
		} else {
			eraseLeft();
		}
	}
	
}

bool TextInputWidget::click() {
	
	bool result = Widget::click();
	
	if(!m_enabled) {
		return result;
	}
	
	if(!m_editing) {
		m_editing = true;
		GInput->startTextInput(Rect(m_rect), this);
		return true;
	}
	
	std::string displayText = text();
	if(m_editing && !editText().empty()) {
		displayText = displayText.substr(0, cursorPos()) + editText() + displayText.substr(cursorPos());
	}
	
	int width = m_font->getTextSize(displayText).width();
	int posx = int(m_rect.center().x) - width / 2;
	if(width > m_rect.width()) {
		if(!m_editing) {
			posx = int(m_rect.left);
		} else {
			int cursor = m_font->getTextSize(displayText.begin(), displayText.begin() + cursorPos()).next();
			posx = int(m_rect.right) - std::min(int(m_rect.width() / 4), width - cursor) - cursor;
			posx = std::min(int(m_rect.left), posx);
		}
	}
	
	setCursorPos(m_font->getPosition(displayText, GInput->getMousePosition().x - posx));
	
	return result;
}

bool TextInputWidget::doubleClick() {
	
	bool result = Widget::click();
	
	if(!m_enabled) {
		return result;
	}
	
	if(!m_editing) {
		m_editing = true;
		GInput->startTextInput(Rect(m_rect), this);
		return true;
	}
	
	selectAll();
	
	return result;
}

void TextInputWidget::render(bool mouseOver) {
	
	Color color = Color(232, 204, 142);
	if(!m_enabled) {
		color = Color::gray(0.5f);
	} else if(mouseOver || m_editing) {
		color = Color::white;
	}
	
	std::string displayText = text();
	if(m_editing && !editText().empty()) {
		displayText = displayText.substr(0, cursorPos()) + editText() + displayText.substr(cursorPos());
	}
	size_t displayCursorPos = cursorPos() + editCursorPos();
	
	std::string::const_iterator begin = displayText.begin();
	std::string::const_iterator end = displayText.end();
	
	int width = m_font->getTextSize(displayText).advance();
	Vec2i pos = Vec2i(m_rect.topCenter() - Vec2f(width / 2, 0.f));
	if(width > m_rect.width()) {
		if(!m_editing) {
			pos.x = int(m_rect.left);
		} else {
			int cursor = m_font->getTextSize(begin, begin + cursorPos()).next();
			pos.x = int(m_rect.right) - std::min(int(m_rect.width() / 4), width - cursor) - cursor;
			pos.x = std::min(s32(m_rect.left), pos.x);
		}
	}
	
	// Highlight selection
	if(m_editing && selected()) {
		Rectf box = m_rect;
		box.left = std::max(float(pos.x), box.left);
		box.right = std::min(float(pos.x + width), box.right);
		Color selection = Color::yellow;
		selection.a = 30;
		EERIEDrawBitmap(box, 0.f, NULL, selection);
	}
	
	// Highlight edit area
	if(m_editing && !editText().empty()) {
		int left = m_font->getTextSize(begin, begin + cursorPos()).advance();
		int right = m_font->getTextSize(begin, begin + cursorPos() + editText().size()).advance();
		int height = m_font->getLineHeight();
		Rectf box = Rectf(Rect(pos + Vec2i(left, 0), right - left, height));
		Color selection = Color::yellow;
		selection.a = 40;
		EERIEDrawBitmap(box, 0.f, NULL, selection);
	}
	
	// Draw text
	GRenderer->SetScissor(Rect(m_rect));
	s32 x = m_font->draw(pos.x, pos.y, begin, end, color).next();
	GRenderer->SetScissor(Rect::ZERO);
	
	// Draw cursor
	if(m_editing) {
		bool blink = true;
		if(mainApp->getWindow()->hasFocus()) {
			blink = timeWaveSquare(g_platformTime.frameStart(), PlatformDurationMs(1200));
		}
		if(blink) {
			int cursor = x;
			if(cursorPos() != displayText.size()) {
				cursor = pos.x + m_font->getTextSize(begin, begin + displayCursorPos).next();
			}
			drawLine(Vec2f(cursor, pos.y), Vec2f(cursor, pos.y + m_font->getLineHeight()), 0.f, Color::white);
			if(editCursorLength() > 0) {
				int endX = pos.x + m_font->getTextSize(begin, begin + displayCursorPos + editCursorLength()).next();
				drawLine(Vec2f(endX, pos.y), Vec2f(endX, pos.y + m_font->getLineHeight()), 0.f, Color::white);
			}
		}
	}
	
}

void TextInputWidget::unfocus() {
	
	if(m_editing) {
		GInput->stopTextInput();
		setCursorPos();
		m_editing = false;
		if(unfocused) {
			unfocused(this);
		}
	}
	
}
