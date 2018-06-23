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

#include "input/Input.h"

static const char emptyKeybindLabel[] = "---";

KeybindWidget::KeybindWidget(ControlAction keybindAction, int keybindIndex, Font * font, Vec2f pos)
	: TextWidget(font, emptyKeybindLabel, pos)
	, m_keybindAction(keybindAction)
	, m_keybindIndex(keybindIndex)
	, m_key(ActionKey::UNUSED)
{
	
	eState = GETTOUCH;
	
}

bool KeybindWidget::OnMouseClick() {
	
	bool ret = TextWidget::OnMouseClick();
	
	if(enabled && eState == GETTOUCH) {
		eState = GETTOUCH_TIME;
		lOldColor = lColorHighlight;
		return true;
	}
	
	return ret;
}

void KeybindWidget::setKey(int keyId) {
	
	lColorHighlight = lOldColor;
	eState = GETTOUCH;
	
	if(keyId == m_key) {
		return;
	}
	m_key = keyId;
	
	std::string text = GInput->getKeyName(keyId, true);
	if(text.empty()) {
		text = emptyKeybindLabel;
	}
	SetText(text);
	
}
