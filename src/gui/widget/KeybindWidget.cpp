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
#include "core/GameTime.h"
#include "input/Input.h"
#include "window/RenderWindow.h"

static const char emptyKeybindLabel[] = "---";

KeybindWidget::KeybindWidget(ControlAction keybindAction, int keybindIndex, Font * font, Vec2f pos)
	: TextWidget(font, emptyKeybindLabel, pos)
	, m_key(ActionKey::UNUSED)
	, m_allowMouse(false)
	, m_keybindAction(keybindAction)
	, m_keybindIndex(keybindIndex)
{
	
	eState = GETTOUCH;
	
}

bool KeybindWidget::OnMouseClick() {
	
	bool ret = TextWidget::OnMouseClick();
	
	if(enabled && eState == GETTOUCH) {
		eState = GETTOUCH_TIME;
		lOldColor = lColorHighlight;
		m_allowMouse = false;
		return true;
	}
	
	return ret;
}

void KeybindWidget::Update() {
	
	if(eState != GETTOUCH_TIME) {
		return;
	}
	
	bool blink = true;
	if(mainApp->getWindow()->hasFocus()) {
		blink = timeWaveSquare(g_platformTime.frameStart(), PlatformDurationMs(400));
	}
	
	lColorHighlight = blink ? Color(255, 0, 0) : Color(50, 0, 0);
	
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
	
	if(keyId < 0) {
		if(m_allowMouse) {
			keyId = GInput->getMouseButtonClicked();
			if(!keyId) {
				keyId = -1;
			}
		} else {
			// Don't allow mouse input for the first update as the mouse is used to enter edit mode
			m_allowMouse = true;
		}
	}
	
	if(keyId < 0) {
		return;
	}
	
	if(keyId == Keyboard::Key_Escape) {
		keyId = ActionKey::UNUSED;
	}
	
	setKey(keyId);
	
}

void KeybindWidget::unfocus() {
	
	if(eState == GETTOUCH_TIME) {
		eState = GETTOUCH;
		lColorHighlight = lOldColor;
		m_allowMouse = false;
	}
	
}

void KeybindWidget::setKey(InputKeyId keyId) {
	
	unfocus();
	
	if(keyId == m_key) {
		return;
	}
	m_key = keyId;
	
	std::string text = GInput->getKeyName(keyId, true);
	if(text.empty()) {
		text = emptyKeybindLabel;
	}
	SetText(text);
	
	if(keyChanged) {
		keyChanged(this);
	}
	
}
