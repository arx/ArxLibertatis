/*
 * Copyright 2017-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "gui/CharacterCreation.h"

#include "core/Core.h"
#include "core/GameTime.h"
#include "core/Localisation.h"
#include "game/Player.h"
#include "gui/Cursor.h"
#include "gui/Interface.h"
#include "gui/Menu.h"
#include "gui/MenuWidgets.h"
#include "gui/Text.h"
#include "gui/TextManager.h"
#include "gui/book/Book.h"
#include "gui/menu/MenuFader.h"
#include "graphics/Draw.h"
#include "graphics/Renderer.h"
#include "graphics/font/Font.h"
#include "input/Input.h"
#include "scene/GameSound.h"


CharacterCreation g_characterCreation;

bool bQuickGenFirstClick = true;
static long SP_HEAD = 0;

extern bool START_NEW_QUEST;

static void ARX_MENU_NEW_QUEST_Clicked_QUIT() {
	START_NEW_QUEST = true;
	g_canResumeGame = true;
	ARX_MENU_Clicked_QUIT();
}

CharacterCreation::CharacterCreation()
	: BookBackground(NULL)
	, m_cheatSkinButtonClickCount(0)
	, m_cheatQuickGenButtonClickCount(0)
{ }

void CharacterCreation::loadData() {
	BookBackground = TextureContainer::LoadUI("graph/interface/book/character_sheet/char_creation_bg", TextureContainer::NoColorKey);
	
	str_button_quickgen = getLocalised("system_charsheet_button_quickgen");
	str_button_skin = getLocalised("system_charsheet_button_skin");
	str_button_done = getLocalised("system_charsheet_button_done");
	
	m_desc_quickgen = getLocalised("system_charsheet_quickgenerate");
	m_desc_skin = getLocalised("system_charsheet_skin");
	m_desc_done = getLocalised("system_charsheet_done");
}

void CharacterCreation::freeData() {
	delete BookBackground;
	BookBackground = NULL;
}

void CharacterCreation::resetCheat() {
	m_cheatSkinButtonClickCount = 0;
	m_cheatQuickGenButtonClickCount = 0;
}

static void characterCreationAddDescription(const std::string & text) {
	
	const int t = 0;
	
	// TODO copy-paste
	UNICODE_ARXDrawTextCenteredScroll(hFontInGame,
		float(g_size.width()) * 0.5f,
		4,
		float(g_size.center().x) * 0.82f,
		text,
		Color(232 + t, 204 + t, 143 + t),
		PlatformDurationMs(1000),
		0.01f,
		3,
		0);
}

void CharacterCreation::render() {
	arx_assert(ARXmenu.mode() == Mode_CharacterCreation);
	
	GRenderer->Clear(Renderer::ColorBuffer);
	
	//-------------------------------------------------------------------------
	
	arx_assert(BookBackground);
	
	{
	UseRenderState state(render2D());
	
	EERIEDrawBitmap(Rectf(Vec2f(0, 0), g_size.width(), g_size.height()), 0.9f, BookBackground, Color::white);
	
	g_playerBook.stats.manageNewQuest();
		
	bool DONE = (player.Skill_Redistribute == 0 && player.Attribute_Redistribute == 0);
	
	Color color = Color::none;
	
	Rectf book = g_playerBook.getArea();
	
	Vec2f spacing = Vec2f(20.f, 30.f) * minSizeRatio();
	
	Vec2f quickGenerateButtonSize(hFontMenu->getTextSize(str_button_quickgen).size());
	Rectf quickGenerateButton(book.bottomLeft() + Vec2f(0.f, spacing.y),
	                          quickGenerateButtonSize.x, quickGenerateButtonSize.y);
	
	Vec2f doneButtonSize(hFontMenu->getTextSize(str_button_done).size());
	Rectf doneButton(book.bottomRight() + Vec2f(-doneButtonSize.x, spacing.y),
	                 doneButtonSize.x, doneButtonSize.y);
	
	Vec2f skinButtonSize(hFontMenu->getTextSize(str_button_skin).size());
	Rectf skinButton((quickGenerateButton.centerRight() + doneButton.centerLeft() - skinButtonSize) / 2.f,
	                 skinButtonSize.x, skinButtonSize.y);
	
	if(quickGenerateButton.right > skinButton.left - spacing.x) {
		quickGenerateButton.move(skinButton.left - spacing.x - quickGenerateButton.right, 0.f);
	}
	
	if(doneButton.left < skinButton.right + spacing.x) {
		doneButton.move(skinButton.right + spacing.x - doneButton.left, 0.f);
	}
	
	// Button QUICK GENERATION
	
	if(quickGenerateButton.contains(Vec2f(DANAEMouse))) {
		
		cursorSetInteraction();
		
		if(!eeMousePressed1() && eeMouseUp1()) {
			
			m_cheatQuickGenButtonClickCount++;
			int iSkin = player.skin;
			ARX_SOUND_PlayMenu(g_snd.MENU_CLICK);
			
			if(bQuickGenFirstClick) {
				ARX_PLAYER_MakeAverageHero();
				bQuickGenFirstClick = false;
			} else {
				ARX_PLAYER_QuickGeneration();
			}
			
			player.skin = checked_range_cast<char>(iSkin);
			
		}
		
		characterCreationAddDescription(m_desc_quickgen);
		color = Color::white;
	} else {
		color = Color(232, 204, 143);
	}
	
	ARX_UNICODE_DrawTextInRect(hFontMenu, quickGenerateButton.topLeft(), 999999, str_button_quickgen, color);
	
	// Button SKIN
	
	if(skinButton.contains(Vec2f(DANAEMouse))) {
		cursorSetInteraction();
		if(eeMouseUp1()) {
			m_cheatSkinButtonClickCount++;
			ARX_SOUND_PlayMenu(g_snd.MENU_CLICK);
			player.skin++;
			if(player.skin > 3) {
				player.skin = 0;
			}
			ARX_PLAYER_Restore_Skin();
		}
		
		characterCreationAddDescription(m_desc_skin);
		color = Color::white;
	} else {
		color = Color::rgb(0.91f, 0.8f, 0.56f);
	}
	
	ARX_UNICODE_DrawTextInRect(hFontMenu, skinButton.topLeft(), 999999, str_button_skin, color);
	
	// Button DONE
	
	if(doneButton.contains(Vec2f(DANAEMouse))) {
		if(DONE)
			cursorSetInteraction();
		
		if(DONE && eeMouseUp1()) {
			if(m_cheatSkinButtonClickCount == 8 && m_cheatQuickGenButtonClickCount == 10) {
				m_cheatSkinButtonClickCount = -2;
			} else if(m_cheatSkinButtonClickCount == -1) {
				ARX_PLAYER_MakeSpHero();
				player.skin = 4;
				ARX_PLAYER_Restore_Skin();
				m_cheatSkinButtonClickCount = 0;
				SP_HEAD = 1;
			} else {
				if(SP_HEAD) {
					player.skin = 4;
					ARX_PLAYER_Restore_Skin();
					SP_HEAD = 0;
				}
				
				ARX_SOUND_PlayMenu(g_snd.MENU_CLICK);
				
				MenuFader_start(Fade_In, Mode_InGame);
			}
		} else {
			if(DONE) {
				color = Color::white;
			} else {
				color = Color::gray(0.75f);
			}
		}
		
		characterCreationAddDescription(m_desc_done);
	} else {
		if(DONE) {
			color = Color(232, 204, 143);
		} else {
			color = Color::gray(0.75f);
		}
	}
	
	if(m_cheatSkinButtonClickCount < 0) {
		color = Color::magenta;
	}
	
	ARX_UNICODE_DrawTextInRect(hFontMenu, doneButton.topLeft(), 999999, str_button_done, color);
	}
	
	EERIE_LIGHT * light = lightHandleGet(torchLightHandle);
	light->pos.x = 0.f + GInput->getMousePosition().x - (g_size.width() >> 1);
	light->pos.y = 0.f + GInput->getMousePosition().y - (g_size.height() >> 1);
	
	if(pTextManage) {
		pTextManage->Update(g_platformTime.lastFrameDuration());
		pTextManage->Render();
	}
	
	ARX_INTERFACE_RenderCursor(true);
	
	if(MenuFader_process()) {
		if(iFadeAction == Mode_InGame) {
			ARX_MENU_NEW_QUEST_Clicked_QUIT();
			MenuFader_reset();
			if(pTextManage) {
				pTextManage->Clear();
			}
		}
	}
	
}
