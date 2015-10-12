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

#include "gui/widget/TextWidget.h"

#include "core/Config.h"
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

TextWidget::TextWidget(MenuButton _iID, Font* _pFont, const std::string& _pText, Vec2i pos)
	: Widget()
{
	m_id = _iID;

	m_font = _pFont;

	if(!_pText.compare("---")) {
		bTestYDouble=true;
	}
	
	m_rect.left = pos.x;
	m_rect.top = pos.y;
	
	SetText(_pText);
	
	lColor = Color(232, 204, 142);
	lColorHighlight=lOldColor=Color(255, 255, 255);

	pRef=this;

	bSelected = false;
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

void TextWidget::Update(int _iDTime) {
	(void)_iDTime;
}

// TODO remove this
extern CWindowMenu * pWindowMenu;

bool TextWidget::OnMouseDoubleClick() {

	switch(m_id) {
	case BUTTON_MENUEDITQUEST_LOAD:
		OnMouseClick();

		if(pWindowMenu) {
			for(size_t i = 0; i < pWindowMenu->m_pages.size(); i++) {
				MenuPage * page = pWindowMenu->m_pages[i];

				if(page->eMenuState == EDIT_QUEST_LOAD) {
					for(size_t j = 0; j < page->m_children.m_widgets.size(); j++) {
						Widget * widget = page->m_children.m_widgets[j]->GetZoneWithID(BUTTON_MENUEDITQUEST_LOAD_CONFIRM);

						if(widget) {
							widget->OnMouseClick();
						}
					}
				}
			}
		}

		return true;
	default:
		return false;
	}

	return false;
}

// TODO remove this
extern int newWidth;
extern int newHeight;
extern bool newFullscreen;
extern TextWidget * pLoadConfirm;
extern TextWidget * pDeleteConfirm;
extern CycleTextWidget * pMenuSliderResol;
extern CheckboxWidget * fullscreenCheckbox;
extern TextWidget * pDeleteButton;
extern bool bNoMenu;
extern MainMenu *mainMenu;

// true: block les zones de checks
bool TextWidget::OnMouseClick() {
	
	if(!enabled) {
		return false;
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

	if(m_id != BUTTON_MENUMAIN_RESUMEGAME) {
		ARX_SOUND_PlayMenu(SND_MENU_CLICK);
	}

	switch(m_id) {
		case BUTTON_INVALID: {
			return false;
		}
		break;
		// MENUMAIN
		case BUTTON_MENUMAIN_RESUMEGAME: {
			pTextManage->Clear();
			ARXMenu_ResumeGame();
			ARX_SOUND_PlayMenu(SND_MENU_CLICK);
		}
		break;
		case BUTTON_MENUMAIN_NEWQUEST: {
				
			if(!ARXMenu_CanResumeGame()) {
				ARXMenu_NewQuest();
			}
		}
		break;
		case BUTTON_MENUMAIN_OPTIONS: {
		}break;
		case BUTTON_MENUMAIN_CREDITS: {
			ARXMenu_Credits();
		}
		break;
		case BUTTON_MENUMAIN_QUIT: {
			ARXMenu_Quit();
		}
		break;
		case BUTTON_MENUNEWQUEST_CONFIRM: {
			ARXMenu_NewQuest();
		}
		break;
		// MENULOADQUEST
		case BUTTON_MENUOPTIONSVIDEO_INIT: {
			newWidth = config.video.resolution.x;
			newHeight = config.video.resolution.y;
			newFullscreen = config.video.fullscreen;
			break;
		}
		case BUTTON_MENUEDITQUEST_LOAD_INIT: {
			if(pWindowMenu)
			for(size_t i = 0; i < pWindowMenu->m_pages.size(); i++) {
				MenuPage * page = pWindowMenu->m_pages[i];
				
				if(page->eMenuState == EDIT_QUEST_LOAD) {
					page->m_savegame = m_savegame;
					
					for(size_t j = 0; j < page->m_children.m_widgets.size(); j++) {
						Widget * widget = page->m_children.m_widgets[j];
						
						if(widget->m_id == BUTTON_MENUEDITQUEST_LOAD) {
							((TextWidget *)widget)->bSelected = false;
						}
					}
				}
			}
		}
			break;
		case BUTTON_MENUEDITQUEST_LOAD: {
			if(pWindowMenu) {
				pLoadConfirm->SetCheckOn();
				pLoadConfirm->lColor = pLoadConfirm->lOldColor;
				pDeleteConfirm->SetCheckOn();
				pDeleteConfirm->lColor = pDeleteConfirm->lOldColor;
				
				for(size_t i = 0; i < pWindowMenu->m_pages.size(); i++) {
					MenuPage * page = pWindowMenu->m_pages[i];
					
					if(page->eMenuState == EDIT_QUEST_LOAD) {
						page->m_savegame = m_savegame;
						
						for(size_t j = 0; j < page->m_children.m_widgets.size(); j++) {
							Widget * widget = page->m_children.m_widgets[j];
							
							if(widget->m_id == BUTTON_MENUEDITQUEST_LOAD) {
								((TextWidget *)widget)->bSelected = false;
							}
						}
						bSelected = true;
					}
				}
			}
		}
		break;
		case BUTTON_MENUEDITQUEST_LOAD_CONFIRM: {
			if(pWindowMenu) {
				for(size_t i = 0; i < pWindowMenu->m_pages.size(); i++) {
					MenuPage * page = pWindowMenu->m_pages[i];
		
					if(page->eMenuState == EDIT_QUEST_LOAD) {
						
						m_savegame = page->m_savegame;
						if(m_savegame != SavegameHandle::Invalid) {
							m_targetMenu = MAIN;
							GRenderer->Clear(Renderer::DepthBuffer);
							ARXMenu_LoadQuest(m_savegame);
							bNoMenu=true;
							if(pTextManage) {
								pTextManage->Clear();
							}
							break;
						}
					}
				}
				
				pLoadConfirm->SetCheckOff();
				pLoadConfirm->lColor = Color::grayb(127);
				pDeleteConfirm->SetCheckOff();
				pDeleteConfirm->lColor = Color::grayb(127);
			}
		}
		break;
		case BUTTON_MENUEDITQUEST_LOAD_CONFIRM_BACK:
			pLoadConfirm->SetCheckOff();
			pLoadConfirm->lColor = Color::grayb(127);
			pDeleteConfirm->SetCheckOff();
			pDeleteConfirm->lColor = Color::grayb(127);
			break;
		// MENUSAVEQUEST
		case BUTTON_MENUEDITQUEST_SAVE: {
			if(pWindowMenu)
			for(size_t i = 0; i < pWindowMenu->m_pages.size(); i++) {
				MenuPage * page = pWindowMenu->m_pages[i];
				
				if(page->eMenuState == EDIT_QUEST_SAVE_CONFIRM) {
					page->m_savegame = m_savegame;
					TextWidget * me = (TextWidget *) page->m_children.m_widgets[1];
					
					if(me) {
						m_targetMenu = MAIN;
						ARXMenu_SaveQuest(me->m_text, me->m_savegame);
						break;
					}
				}
			}
		}
		break;
		
		// Delete save from the load menu
		case BUTTON_MENUEDITQUEST_DELETE_CONFIRM: {
			if(pWindowMenu) {
				for(size_t i = 0 ; i < pWindowMenu->m_pages.size(); i++) {
					MenuPage * page = pWindowMenu->m_pages[i];
					if(page->eMenuState == EDIT_QUEST_LOAD) {
						m_savegame = page->m_savegame;
						if(m_savegame != SavegameHandle::Invalid) {
							m_targetMenu = EDIT_QUEST_LOAD;
							mainMenu->bReInitAll = true;
							savegames.remove(m_savegame);
							break;
						}
					}
				}
			}
			pLoadConfirm->SetCheckOff();
			pLoadConfirm->lColor = Color::grayb(127);
			pDeleteConfirm->SetCheckOff();
			pDeleteConfirm->lColor = Color::grayb(127);
			break;
		}
			
		// Delete save from the save menu
		case BUTTON_MENUEDITQUEST_DELETE: {
			if(pWindowMenu) {
				for(size_t i = 0 ; i < pWindowMenu->m_pages.size(); i++) {
					MenuPage * page = pWindowMenu->m_pages[i];
					if(page->eMenuState == EDIT_QUEST_SAVE_CONFIRM) {
						page->m_savegame = m_savegame;
						TextWidget * me = (TextWidget *) page->m_children.m_widgets[1];
						if(me) {
							m_targetMenu = EDIT_QUEST_SAVE;
							mainMenu->bReInitAll = true;
							savegames.remove(me->m_savegame);
							break;
						}
					}
				}
			}
			break;
		}
			
		case BUTTON_MENUOPTIONSVIDEO_APPLY: {
			if(newWidth != config.video.resolution.x
			   || newHeight!=config.video.resolution.y
			   || newFullscreen != config.video.fullscreen
			) {
				ARXMenu_Private_Options_Video_SetResolution(newFullscreen, newWidth, newHeight);
				pMenuSliderResol->setOldValue(-1);
				fullscreenCheckbox->iOldState = -1;
			}
			mainMenu->bReInitAll=true;
		}
		break;
		case BUTTON_MENUOPTIONS_CONTROLS_BACK:
		{
			config.save();
		}
		break;
		default:
			break;
	}

	if(m_targetMenu == EDIT_QUEST_SAVE_CONFIRM) {
		for(size_t i = 0; i < pWindowMenu->m_pages.size(); i++) {
			MenuPage * page = pWindowMenu->m_pages[i];

			if(page->eMenuState == m_targetMenu) {
				page->m_savegame = m_savegame;
				TextWidget * me = (TextWidget *) page->m_children.m_widgets[1];

				if(me) {
					me->m_savegame = m_savegame;
					
					if(m_savegame != SavegameHandle::Invalid) {
						me->SetText(savegames[m_savegame].name);
						pDeleteButton->lColor = pDeleteButton->lOldColor;
						pDeleteButton->SetCheckOn();
					} else {
						pDeleteButton->lColor = Color::grayb(127);
						pDeleteButton->SetCheckOff();
						me->SetText(getLocalised("system_menu_editquest_newsavegame"));
					}
					
					page->AlignElementCenter(me);
				}
			}
		}
	}

	return false;
}

// true: block les zones de checks
Widget* TextWidget::OnShortCut() {

	if(m_shortcut==-1)
		return NULL;

	if(GInput->isKeyPressedNowUnPressed(m_shortcut)) {
		return this;
	}

	return NULL;
}

static void FontRenderText(Font * _pFont, const Rect & rzone,
                           const std::string & _pText, Color _c) {
	if(pTextManage && !rzone.empty()) {
		pTextManage->AddText(_pFont, _pText, rzone, _c);
	}
}

void TextWidget::Render() {

	if(bNoMenu)
		return;

	if(bSelected) {
		FontRenderText(m_font, m_rect, m_text, lColorHighlight);
	} else if(enabled) {
		FontRenderText(m_font, m_rect, m_text, lColor);
	} else {
		FontRenderText(m_font, m_rect, m_text, Color::grayb(127));
	}

}

extern MenuCursor * pMenuCursor;
extern TextureContainer *pTextureLoad;
extern TextureContainer *pTextureLoadRender;

void TextWidget::RenderMouseOver() {

	if(bNoMenu)
		return;

	pMenuCursor->SetMouseOver();

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	
	FontRenderText(m_font, m_rect, m_text, lColorHighlight);

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);

	switch(m_id) {
		case BUTTON_MENUEDITQUEST_LOAD:
		case BUTTON_MENUEDITQUEST_SAVEINFO: {
			
			if(m_savegame == SavegameHandle::Invalid) {
				pTextureLoadRender = NULL;
				break;
			}
			
			const res::path & image = savegames[m_savegame].thumbnail;
			if(!image.empty()) {
				TextureContainer * t = TextureContainer::LoadUI(image, TextureContainer::NoColorKey);
				if(t != pTextureLoad) {
					delete pTextureLoad;
					pTextureLoad = t;
				}
				pTextureLoadRender = pTextureLoad;
			}
			
			break;
		}
		
		default: {
			pTextureLoadRender = NULL;
			break;
		}
	}
}
