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

#include "gui/widget/KeybindWidget.h"

#include "core/Application.h"
#include "core/Core.h"
#include "core/GameTime.h"
#include "graphics/Color.h"
#include "graphics/font/Font.h"
#include "gui/Text.h"
#include "input/Input.h"
#include "scene/GameSound.h"
#include "window/RenderWindow.h"

KeybindWidget::KeybindWidget(ControlAction keybindAction, size_t keybindIndex, Font * font, Vec2f pos)
	: m_font(font)
	, m_action(keybindAction)
	, m_index(keybindIndex)
	, m_key(ActionKey::UNUSED - 1)
	, m_editing(false)
	, m_allowMouse(false)
{
	m_rect = Rectf(RATIO_2(pos), 0.f, 0.f);
	setKey(ActionKey::UNUSED);
}

bool KeybindWidget::OnMouseClick() {
	
	if(!enabled) {
		return false;
	}
	
	ARX_SOUND_PlayMenu(SND_MENU_CLICK);
	
	if(!m_editing) {
		m_editing = true;
		m_allowMouse = false;
		return true;
	}
	
	return false;
}

void KeybindWidget::Update() {
	
	if(!m_editing) {
		return;
	}
	
	InputKeyId keyId = GInput->getKeyPressed();
	
	if(GInput->isKeyPressed(Keyboard::Key_LeftShift) || GInput->isKeyPressed(Keyboard::Key_RightShift)
			|| GInput->isKeyPressed(Keyboard::Key_LeftCtrl) || GInput->isKeyPressed(Keyboard::Key_RightCtrl)
			|| GInput->isKeyPressed(Keyboard::Key_LeftAlt) || GInput->isKeyPressed(Keyboard::Key_RightAlt)) {
		if(!((keyId & INPUT_COMBINATION_MASK) >> 16)) {
			keyId = -1;
		}
	} else {
		if(GInput->isKeyPressedNowUnPressed(Keyboard::Key_LeftShift)) {
			keyId = Keyboard::Key_LeftShift;
		}
		if(GInput->isKeyPressedNowUnPressed(Keyboard::Key_RightShift)) {
			keyId = Keyboard::Key_RightShift;
		}
		if(GInput->isKeyPressedNowUnPressed(Keyboard::Key_LeftCtrl)) {
			keyId = Keyboard::Key_LeftCtrl;
		}
		if(GInput->isKeyPressedNowUnPressed(Keyboard::Key_RightCtrl)) {
			keyId = Keyboard::Key_RightCtrl;
		}
		if(GInput->isKeyPressedNowUnPressed(Keyboard::Key_LeftAlt)) {
			keyId = Keyboard::Key_LeftAlt;
		}
		if(GInput->isKeyPressedNowUnPressed(Keyboard::Key_RightAlt)) {
			keyId = Keyboard::Key_RightAlt;
		}
	}
	
	// Don't allow mouse input for the first update as the mouse is used to enter edit mode
	if(keyId < 0 && m_allowMouse) {
		keyId = GInput->getMouseButtonClicked();
		if(!keyId) {
			keyId = -1;
		}
	}
	m_allowMouse = true;
	
	if(keyId < 0) {
		return;
	}
	
	if(keyId == Keyboard::Key_Escape) {
		keyId = ActionKey::UNUSED;
	}
	
	setKey(keyId);
	
}

void KeybindWidget::Render() {
	
	bool blink = true;
	if(mainApp->getWindow()->hasFocus()) {
		blink = timeWaveSquare(g_platformTime.frameStart(), PlatformDurationMs(400));
	}
	
	Color editColor = blink ? Color(255, 0, 0) : Color(50, 0, 0);
	Color color = m_editing ? editColor : enabled ? Color(232, 204, 142) : Color::grayb(127);
	
	ARX_UNICODE_DrawTextInRect(m_font, m_rect.topLeft(), m_rect.right, m_text, color, NULL);
	
}

void KeybindWidget::RenderMouseOver() {
	
	if(!m_editing) {
		ARX_UNICODE_DrawTextInRect(m_font, m_rect.topLeft(), m_rect.right, m_text, Color::white, NULL);
	}
	
}

void KeybindWidget::unfocus() {
	
	if(m_editing) {
		m_editing = false;
		m_allowMouse = false;
	}
	
}

void KeybindWidget::setKey(InputKeyId keyId) {
	
	unfocus();
	
	if(keyId == m_key) {
		return;
	}
	m_key = keyId;
	
	m_text = GInput->getKeyName(keyId, true);
	if(m_text.empty()) {
		m_text = "---";
	}
	
	Vec2i textSize = m_font->getTextSize(m_text);
	m_rect = Rectf(m_rect.topLeft(), textSize.x + 1, textSize.y + 1);
	
	if(keyChanged) {
		keyChanged(this);
	}
	
}
