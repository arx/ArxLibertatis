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

#include "core/Config.h"
#include "core/Core.h"
#include "core/Localisation.h"
#include "core/SaveGame.h"
#include "graphics/Renderer.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/font/Font.h"
#include "gui/MainMenu.h"
#include "gui/MenuWidgets.h"
#include "gui/MenuPublic.h"
#include "gui/Text.h"
#include "gui/TextManager.h"
#include "gui/menu/MenuCursor.h"
#include "gui/widget/CheckboxWidget.h"
#include "gui/widget/CycleTextWidget.h"
#include "input/Input.h"
#include "scene/GameSound.h"

TextWidget::TextWidget(Font* font, const std::string& text, Vec2f pos)
	: Widget()
{
	m_font = font;
	
	Vec2f scaledPos = RATIO_2(pos);
	
	m_rect.left = scaledPos.x;
	m_rect.top = scaledPos.y;
	
	SetText(text);
	
	lColor = Color(232, 204, 142);
	lColorHighlight=lOldColor=Color(255, 255, 255);

	bSelected = false;
	
	m_isKeybind = false;
	m_keybindAction = CONTROLS_CUST_JUMP;
	m_keybindIndex = 0;
}

TextWidget::~TextWidget()
{
}

void TextWidget::SetText(const std::string & _pText)
{
	m_text = _pText;

	Vec2i textSize = m_font->getTextSize(_pText);

	m_rect.right  = textSize.x + m_rect.left + 1;
	m_rect.bottom = textSize.y + m_rect.top + 1;
}

void TextWidget::Update() {
}

void TextWidget::OnMouseDoubleClick() {
	
	if(doubleClicked) {
		doubleClicked(this);
	}
}

// true: block les zones de checks
bool TextWidget::OnMouseClick() {
	
	if(!enabled) {
		return false;
	}
	
	if(clicked) {
		clicked(this);
	}
	
	switch(eState) {
		case EDIT:
			eState=EDIT_TIME;
			return true;
		case GETTOUCH:
			eState=GETTOUCH_TIME;
			lOldColor=lColorHighlight;
			return true;
		default: break;
	}
	
	ARX_SOUND_PlayMenu(SND_MENU_CLICK);
	
	return false;
}

static void FontRenderText(Font * _pFont, const Rectf & rzone, const std::string & _pText, Color _c) {
	Rect rect(rzone);
	if(pTextManage && !rect.empty()) {
		pTextManage->AddText(_pFont, _pText, rect, _c);
	}
}

void TextWidget::Render() {

	if(bSelected) {
		FontRenderText(m_font, m_rect, m_text, lColorHighlight);
	} else if(enabled) {
		FontRenderText(m_font, m_rect, m_text, lColor);
	} else {
		FontRenderText(m_font, m_rect, m_text, Color::grayb(127));
	}

}

void TextWidget::RenderMouseOver() {

	pMenuCursor->SetMouseOver();
	
	FontRenderText(m_font, m_rect, m_text, lColorHighlight);
	
	g_thumbnailCursor.clear();
}
