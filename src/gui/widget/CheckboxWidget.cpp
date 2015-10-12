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

#include "gui/widget/CheckboxWidget.h"

#include "core/Config.h"
#include "core/Core.h"
#include "graphics/DrawLine.h"
#include "graphics/Renderer.h"
#include "graphics/data/TextureContainer.h"
#include "gui/Hud.h"
#include "gui/MenuPublic.h"
#include "gui/menu/MenuCursor.h"
#include "gui/widget/TextWidget.h"
#include "gui/widget/CycleTextWidget.h"
#include "scene/GameSound.h"

CheckboxWidget::CheckboxWidget(TextWidget *_pText)
	: Widget()
{
	pRef = this; // TODO remove this
	
	arx_assert(_pText);
	
	m_textureOff = TextureContainer::Load("graph/interface/menus/menu_checkbox_off");
	m_textureOn = TextureContainer::Load("graph/interface/menus/menu_checkbox_on");
	arx_assert(m_textureOff);
	arx_assert(m_textureOn);
	arx_assert(m_textureOff->size() == m_textureOn->size());
	
	m_id = BUTTON_INVALID;
	iState    = 0;
	iOldState = -1;
	pText    = _pText;
	m_rect = _pText->m_rect;
	
	m_rect.right = m_rect.left + RATIO_X(245.f);
}

CheckboxWidget::~CheckboxWidget() {
	delete pText;
}

void CheckboxWidget::Move(const Vec2i & offset) {
	
	Widget::Move(offset);
	pText->Move(offset);
}

// TODO remove this
extern int newWidth;
extern int newHeight;
extern bool newFullscreen;
extern CycleTextWidget * pMenuSliderResol;
extern CheckboxWidget * fullscreenCheckbox;

bool CheckboxWidget::OnMouseClick() {
	
	if(iOldState<0)
		iOldState=iState;

	iState ++;

	//NB : It seems that iState cannot be negative (used as tabular index / used as bool) but need further approval
	arx_assert(iState >= 0);

	if((size_t)iState >= 2) {
		iState = 0;
	}

	ARX_SOUND_PlayMenu(SND_MENU_CLICK);

	switch (m_id) {
		case BUTTON_MENUOPTIONSVIDEO_FULLSCREEN: {
			newFullscreen = ((iState)?true:false);
			
			if(pMenuSliderResol) {
				pMenuSliderResol->setEnabled(newFullscreen);
			}
			
		}
		break;
		case BUTTON_MENUOPTIONSVIDEO_CROSSHAIR: {
			config.video.showCrosshair=(iState)?true:false;
		}
		break;
		case BUTTON_MENUOPTIONSVIDEO_ANTIALIASING: {
			config.video.antialiasing = iState ? true : false;
			ARX_SetAntiAliasing();
			break;
		}
		case BUTTON_MENUOPTIONSVIDEO_VSYNC: {
			config.video.vsync = iState ? true : false;
			break;
		}
		case BUTTON_MENUOPTIONSVIDEO_HUDSCALE: {
			config.video.hudScale = iState ? true : false;
			g_hudRoot.recalcScale();
			break;
		}
		case BUTTON_MENUOPTIONSAUDIO_EAX: {
			ARXMenu_Options_Audio_SetEAX(iState != 0);
			break;
		}
		case BUTTON_MENUOPTIONS_CONTROLS_INVERTMOUSE: {
				ARXMenu_Options_Control_SetInvertMouse((iState)?true:false);
			}
			break;
		case BUTTON_MENUOPTIONS_CONTROLS_AUTOREADYWEAPON: {
			config.input.autoReadyWeapon = (iState) ? true : false;
			break;
		}
		case BUTTON_MENUOPTIONS_CONTROLS_MOUSELOOK: {
			config.input.mouseLookToggle = (iState) ? true : false;
			break;
		}
		case BUTTON_MENUOPTIONS_CONTROLS_AUTODESCRIPTION: {
			config.input.autoDescription = (iState) ? true : false;
			break;
		}
		case BUTTON_MENUOPTIONSVIDEO_BACK: {
			if(pMenuSliderResol && pMenuSliderResol->getOldValue() >= 0) {
				pMenuSliderResol->setValue(pMenuSliderResol->getOldValue());
				pMenuSliderResol->setOldValue(-1);
				newWidth=config.video.resolution.x;
				newHeight=config.video.resolution.y;
			}
			
			if(fullscreenCheckbox && fullscreenCheckbox->iOldState >= 0) {
				fullscreenCheckbox->iState = fullscreenCheckbox->iOldState;
				fullscreenCheckbox->iOldState = -1;
				newFullscreen = config.video.fullscreen;
			}
			break;
		}
		default:
			break;
	}

	return false;
}

void CheckboxWidget::Update(int /*_iDTime*/)
{
}

void CheckboxWidget::renderCommon() {
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	
	Rectf checkboxRect;
	checkboxRect.top = m_rect.top;
	checkboxRect.left = m_rect.right - m_rect.height();
	checkboxRect.bottom = m_rect.bottom;
	checkboxRect.right = m_rect.right;
	
	TextureContainer *pTex = (iState == 0) ? m_textureOff : m_textureOn;
	Color color = (bCheck) ? Color::white : Color(63, 63, 63, 255);
	
	EERIEDrawBitmap2(checkboxRect, 0.f, pTex, color);
}

// TODO remove this
extern bool bNoMenu;

void CheckboxWidget::Render() {

	if(bNoMenu)
		return;
	
	renderCommon();
	
	pText->Render();
}

extern MenuCursor * pMenuCursor;

void CheckboxWidget::RenderMouseOver() {

	if(bNoMenu)
		return;

	pMenuCursor->SetMouseOver();

	renderCommon();
	
	pText->RenderMouseOver();
}
