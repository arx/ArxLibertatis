/*
 * Copyright 2015 Arx Libertatis Team (see the AUTHORS file)
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

#include "gui/widget/SliderWidget.h"

#include "core/Core.h"
#include "graphics/DrawLine.h"
#include "graphics/Renderer.h"
#include "graphics/data/TextureContainer.h"
#include "gui/MenuPublic.h"
#include "gui/menu/MenuCursor.h"
#include "input/Input.h"
#include "scene/GameSound.h"

SliderWidget::SliderWidget(MenuButton _iID, Vec2i pos)
	: Widget()
{
	m_id = _iID;
	
	pLeftButton = new ButtonWidget(pos, "graph/interface/menus/menu_slider_button_left");
	pRightButton = new ButtonWidget(pos, "graph/interface/menus/menu_slider_button_right");
	pTex1 = TextureContainer::Load("graph/interface/menus/menu_slider_on");
	pTex2 = TextureContainer::Load("graph/interface/menus/menu_slider_off");
	arx_assert(pTex1);
	arx_assert(pTex2);
	
	m_value = 0;

	m_rect.left   = pos.x;
	m_rect.top    = pos.y;
	m_rect.right  = pos.x + pLeftButton->m_rect.width() + pRightButton->m_rect.width() + 10 * std::max(pTex1->m_size.x, pTex2->m_size.x);
	m_rect.bottom = pos.y + std::max(pLeftButton->m_rect.height(), pRightButton->m_rect.height());
	
	pRightButton->Move(Vec2i(pLeftButton->m_rect.width() + 10 * std::max(pTex1->m_size.x, pTex2->m_size.x), 0));

	pRef = this;
}

SliderWidget::~SliderWidget() {
	delete pLeftButton;
	delete pRightButton;
}

void SliderWidget::Move(const Vec2i & offset) {
	Widget::Move(offset);
	pLeftButton->Move(offset);
	pRightButton->Move(offset);
}

void SliderWidget::EmptyFunction() {

	//Touche pour la selection
	if(GInput->isKeyPressedNowPressed(Keyboard::Key_LeftArrow)) {
		m_value--;

		if(m_value <= 0)
			m_value = 0;
	} else {
		if(GInput->isKeyPressedNowPressed(Keyboard::Key_RightArrow)) {
			m_value++;

			if(m_value >= 10)
				m_value = 10;
		}
	}
}

bool SliderWidget::OnMouseClick() {
	
	ARX_SOUND_PlayMenu(SND_MENU_CLICK);

	const Vec2i cursor = Vec2i(GInput->getMousePosAbs());
	
	if(m_rect.contains(cursor)) {
		if(pLeftButton->m_rect.contains(cursor)) {
			m_value--;
			if(m_value <= 0)
				m_value = 0;
		}
		
		if(pRightButton->m_rect.contains(cursor)) {
			m_value++;
			if(m_value >= 10)
				m_value = 10;
		}
	}
	
	switch (m_id) {
		// MENUOPTIONS_VIDEO
		case BUTTON_MENUOPTIONSVIDEO_FOG:
			ARXMenu_Options_Video_SetFogDistance(m_value);
			break;
		// MENUOPTIONS_AUDIO
		case BUTTON_MENUOPTIONSAUDIO_MASTER:
			ARXMenu_Options_Audio_SetMasterVolume(m_value);
			break;
		case BUTTON_MENUOPTIONSAUDIO_SFX:
			ARXMenu_Options_Audio_SetSfxVolume(m_value);
			break;
		case BUTTON_MENUOPTIONSAUDIO_SPEECH:
			ARXMenu_Options_Audio_SetSpeechVolume(m_value);
			break;
		case BUTTON_MENUOPTIONSAUDIO_AMBIANCE:
			ARXMenu_Options_Audio_SetAmbianceVolume(m_value);
			break;
		// MENUOPTIONS_CONTROLS
		case BUTTON_MENUOPTIONS_CONTROLS_MOUSESENSITIVITY:
			ARXMenu_Options_Control_SetMouseSensitivity(m_value);
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_QUICKSAVESLOTS: {
			m_value = std::max(m_value, 1);
			config.misc.quicksaveSlots = m_value;
			break;
		}
		default:
			break;
	}

	return false;
}

void SliderWidget::Update(int _iTime) {
	
	pLeftButton->Update(_iTime);
	pRightButton->Update(_iTime);
	pRightButton->SetPos(m_rect.topLeft());


	float fWidth = pLeftButton->m_rect.width() + RATIO_X(10 * std::max(pTex1->m_size.x, pTex2->m_size.x)) ;
	pRightButton->Move(Vec2i(fWidth, 0));

	m_rect.right = m_rect.left + pLeftButton->m_rect.width() + pRightButton->m_rect.width() + RATIO_X(10*std::max(pTex1->m_size.x, pTex2->m_size.x));

	m_rect.bottom = m_rect.top + std::max(pLeftButton->m_rect.height(), pRightButton->m_rect.height());
}

// TODO remove this
extern bool bNoMenu;

void SliderWidget::Render() {

	if(bNoMenu)
		return;

	pLeftButton->Render();
	pRightButton->Render();

	Vec2f pos(m_rect.left + pLeftButton->m_rect.width(), m_rect.top);
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	
	for(int i = 0; i < 10; i++) {
		TextureContainer * pTex = (i < m_value) ? pTex1 : pTex2;
		Rectf rect = Rectf(pos, RATIO_X(pTex->m_size.x), RATIO_Y(pTex->m_size.y));
		
		EERIEDrawBitmap2(rect, 0, pTex, Color::white);
		
		pos.x += rect.width();
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}

extern MenuCursor * pMenuCursor;

void SliderWidget::RenderMouseOver() {

	if(bNoMenu)
		return;

	pMenuCursor->SetMouseOver();

	const Vec2i cursor = Vec2i(GInput->getMousePosAbs());

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	
	if(m_rect.contains(cursor)) {
		if(pLeftButton->m_rect.contains(cursor)) {
			pLeftButton->Render();
		}
		
		if(pRightButton->m_rect.contains(cursor)) {
			pRightButton->Render();
		}
	}
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}
