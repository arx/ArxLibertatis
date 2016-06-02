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

TextWidget::TextWidget(MenuButton id, Font* font, const std::string& text, Vec2f pos)
	: Widget()
{
	m_id = id;

	m_font = font;
	
	Vec2f scaledPos = RATIO_2(pos);
	
	m_rect.left = scaledPos.x;
	m_rect.top = scaledPos.y;
	
	SetText(text);
	
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

void TextWidget::Update() {
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
extern TextWidget * pLoadConfirm;
extern TextWidget * pDeleteConfirm;
extern TextWidget * pDeleteButton;
extern bool bNoMenu;
extern MainMenu *mainMenu;

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
	
	switch(m_id) {
		// MENULOADQUEST
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
						if(m_savegame != SavegameHandle()) {
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
						if(m_savegame != SavegameHandle()) {
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
					
					if(m_savegame != SavegameHandle()) {
						me->SetText(savegames[m_savegame.handleData()].name);
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

extern MenuCursor * pMenuCursor;

void TextWidget::RenderMouseOver() {

	pMenuCursor->SetMouseOver();

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(BlendOne, BlendOne);
	
	FontRenderText(m_font, m_rect, m_text, lColorHighlight);

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);

	switch(m_id) {
		case BUTTON_MENUEDITQUEST_LOAD:
		case BUTTON_MENUEDITQUEST_SAVEINFO: {
			
			if(m_savegame == SavegameHandle()) {
				g_thumbnailCursor.clear();
				break;
			}
			
			const res::path & image = savegames[m_savegame.handleData()].thumbnail;
			if(!image.empty()) {
				TextureContainer * t = TextureContainer::LoadUI(image, TextureContainer::NoColorKey);
				if(t != g_thumbnailCursor.m_loadTexture) {
					delete g_thumbnailCursor.m_loadTexture;
					g_thumbnailCursor.m_loadTexture = t;
				}
				g_thumbnailCursor.m_renderTexture = g_thumbnailCursor.m_loadTexture;
			}
			
			break;
		}
		
		default: {
			g_thumbnailCursor.clear();
			break;
		}
	}
}
