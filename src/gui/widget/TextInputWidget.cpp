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

#include "gui/widget/TextInputWidget.h"

#include <limits>

#include "core/Application.h"
#include "core/Core.h"
#include "core/GameTime.h"
#include "graphics/Draw.h"
#include "graphics/Renderer.h"
#include "graphics/font/Font.h"
#include "gui/Text.h"
#include "input/Input.h"
#include "scene/GameSound.h"
#include "window/RenderWindow.h"

TextInputWidget::TextInputWidget(Font * font, const std::string & text, const Rectf & rect)
	: m_font(font)
{
	m_rect = rect;
	m_rect.bottom = m_rect.top + m_font->getTextSize("|").height();
	setText(text);
	eState = EDIT;
}

bool TextInputWidget::click() {
	
	bool result = Widget::click();
	
	if(!m_enabled) {
		return result;
	}
	
	ARX_SOUND_PlayMenu(SND_MENU_CLICK);
	
	if(eState == EDIT) {
		eState = EDIT_TIME;
		return true;
	}
	
	return result;
}

void TextInputWidget::update() {
	
	if(eState != EDIT_TIME) {
		return;
	}
	
	if(!GInput->isAnyKeyPressed()) {
		return;
	}
	
	if(GInput->isKeyPressed(Keyboard::Key_Enter)
	   || GInput->isKeyPressed(Keyboard::Key_NumPadEnter)
	   || GInput->isKeyPressed(Keyboard::Key_Escape)) {
		ARX_SOUND_PlayMenu(SND_MENU_CLICK);
		unfocus();
		return;
	}
	
	if(GInput->isKeyPressedNowPressed(Keyboard::Key_Backspace)) {
		if(!m_text.empty()) {
			m_text.resize(m_text.size() - 1);
		}
		return;
	}
	
	int key = GInput->getKeyPressed() & INPUT_KEYBOARD_MASK;
	
	char chr;
	if(!GInput->isKeyPressedNowPressed(key) || !GInput->getKeyAsText(key, chr)) {
		return;
	}
	
	int value = chr & 0xFF; // To prevent ascii chars between [128, 255] from causing an assertion n the functions below...
	if(!(isalnum(value) || isspace(value) || ispunct(value)) || chr == '\t' || chr == '*') {
		return;
	}
	
	m_text += chr;
	
	if(m_font->getTextSize(m_text).width() > m_rect.width()) {
		m_text.resize(m_text.size() - 1);
	}
	
}

void TextInputWidget::render(bool mouseOver) {
	
	Color color = Color(232, 204, 142);
	if(!m_enabled) {
		color = Color::grayb(127);
	} else if(mouseOver) {
		color = Color::white;
	}
	
	float width = m_font->getTextSize(m_text).width();
	Vec2f pos = m_rect.topCenter() - Vec2f(width / 2, 0.f);
	
	Rect clip(m_rect);
	ARX_UNICODE_DrawTextInRect(m_font, pos, std::numeric_limits<float>::infinity(), m_text, color, &clip);
	
	if(eState == EDIT_TIME) {
		bool blink = true;
		if(mainApp->getWindow()->hasFocus()) {
			blink = timeWaveSquare(g_platformTime.frameStart(), PlatformDurationMs(1200));
		}
		if(blink) {
			// Draw cursor
			float cursorPos = pos.x + width + 1;
			TexturedVertex v[4];
			GRenderer->ResetTexture(0);
			v[0].color = v[1].color = v[2].color = v[3].color = Color::white.toRGB();
			v[0].p = Vec3f(cursorPos, m_rect.top, 0.f);
			v[1].p = v[0].p + Vec3f(2.f, 0.f, 0.f);
			v[2].p = Vec3f(cursorPos, m_rect.bottom, 0.f);
			v[3].p = v[2].p + Vec3f(2.f, 0.f, 0.f);
			v[0].w = v[1].w = v[2].w = v[3].w = 1.f;
			EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);
		}
	}
	
}

void TextInputWidget::unfocus() {
	
	if(eState == EDIT_TIME) {
		eState = EDIT;
		if(unfocused) {
			unfocused(this);
		}
	}
	
}

void TextInputWidget::setText(const std::string & text) {
	m_text = text;
}
