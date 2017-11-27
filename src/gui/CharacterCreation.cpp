/*
 * Copyright 2017 Arx Libertatis Team (see the AUTHORS file)
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
#include "input/Input.h"
#include "scene/GameSound.h"


CharacterCreation g_characterCreation;

bool bQuickGenFirstClick = true;
static long SP_HEAD = 0;

extern long FLYING_OVER;

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
	
	// TODO unused button descriptions, readd ?
	// getLocalised("system_charsheet_quickgenerate");
	// getLocalised("system_charsheet_done");
	// getLocalised("system_charsheet_skin");
}

void CharacterCreation::freeData() {
	delete BookBackground;
	BookBackground = NULL;
}

void CharacterCreation::resetCheat() {
	m_cheatSkinButtonClickCount = 0;
	m_cheatQuickGenButtonClickCount = 0;
}

void CharacterCreation::render() {
	arx_assert(ARXmenu.mode() == Mode_CharacterCreation);
	
	GRenderer->Clear(Renderer::ColorBuffer);
	
	FLYING_OVER = 0;
	
	//-------------------------------------------------------------------------
	
	arx_assert(BookBackground);
	
	{
	UseRenderState state(render2D());
	
	EERIEDrawBitmap(Rectf(Vec2f(0, 0), g_size.width(), g_size.height()), 0.9f, BookBackground, Color::white);
	
	g_playerBook.stats.manageNewQuest();
		
	bool DONE = (player.Skill_Redistribute == 0 && player.Attribute_Redistribute == 0);
	
	Vec2f pos;
	pos.x = 0;
	pos.y = 313 * g_sizeRatio.y + (g_size.height() - 313 * g_sizeRatio.y) * 0.70f;
	
	Vec2f size = g_sizeRatio;
	size *= 100;
	
	Color color = Color::none;
	
	//---------------------------------------------------------------------
	// Button QUICK GENERATION
	pos.x = (g_size.width() - (513 * g_sizeRatio.x)) * 0.5f;
	
	const Rectf quickGenerateButtonMouseTestRect(
		pos,
		size.x,
		size.y
	);
	
	if(quickGenerateButtonMouseTestRect.contains(Vec2f(DANAEMouse))) {
		SpecialCursor = CURSOR_INTERACTION_ON;
		
		if(eeMousePressed1());
		else if (eeMouseUp1())
		{
			m_cheatQuickGenButtonClickCount++;
			int iSkin = player.skin;
			ARX_SOUND_PlayMenu(SND_MENU_CLICK);
			
			if(bQuickGenFirstClick) {
				ARX_PLAYER_MakeAverageHero();
				bQuickGenFirstClick = false;
			} else {
				ARX_PLAYER_QuickGeneration();
			}
			
			player.skin = checked_range_cast<char>(iSkin);
		}
		
		color = Color(255, 255, 255);
	}
	else
		color = Color(232, 204, 143);
	
	pTextManage->AddText(hFontMenu, str_button_quickgen, Vec2i(pos), color);
	
	//---------------------------------------------------------------------
	// Button SKIN
	pos.x = g_size.width() * 0.5f;
	
	const Rectf skinButtonMouseTestRect(
		pos,
		size.x,
		size.y
	);
	
	if(skinButtonMouseTestRect.contains(Vec2f(DANAEMouse))) {
		SpecialCursor = CURSOR_INTERACTION_ON;
		
		if(eeMouseUp1()) {
			m_cheatSkinButtonClickCount++;
			ARX_SOUND_PlayMenu(SND_MENU_CLICK);
			player.skin++;
			
			if(player.skin > 3)
				player.skin = 0;
			
			ARX_PLAYER_Restore_Skin();
		}
		
		color = Color(255, 255, 255);
	}
	else
		color = Color(232, 204, 143);
	
	pTextManage->AddText(hFontMenu, str_button_skin, Vec2i(pos), color);
	
	//---------------------------------------------------------------------
	// Button DONE
	pos.x = g_size.width() - (g_size.width() - 513 * g_sizeRatio.x) * 0.5f - 40 * g_sizeRatio.x;
	
	const Rectf doneButtonMouseTestRect(
		pos,
		size.x,
		size.y
	);
	
	if(doneButtonMouseTestRect.contains(Vec2f(DANAEMouse))) {
		if(DONE)
			SpecialCursor = CURSOR_INTERACTION_ON;
		
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
				
				ARX_SOUND_PlayMenu(SND_MENU_CLICK);
				
				MenuFader_start(Fade_In, Mode_InGame);
			}
		} else {
			if(DONE)
				color = Color(255, 255, 255);
			else
				color = Color(192, 192, 192);
		}
	} else {
		if(DONE)
			color = Color(232, 204, 143);
		else
			color = Color(192, 192, 192);
	}
	
	if(m_cheatSkinButtonClickCount < 0)
		color = Color(255, 0, 255);
	
	pTextManage->AddText(hFontMenu, str_button_done, Vec2i(pos), color);
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
		switch(iFadeAction) {
			case Mode_InGame:
				ARX_MENU_NEW_QUEST_Clicked_QUIT();
				MenuFader_reset();
				
				if(pTextManage)
					pTextManage->Clear();
				
				break;
		}
	}
}
