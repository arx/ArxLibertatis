/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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
/* Based on:
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#include "gui/Menu.h"

#include <cstdlib>
#include <sstream>
#include <iterator>
#include <iomanip>

#include "Configure.h"

#include "animation/Animation.h"

#include "core/Application.h"
#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"
#include "core/Localisation.h"

#include "game/Player.h"

#include "gui/Credits.h"
#include "gui/Interface.h"
#include "gui/MenuPublic.h"
#include "gui/MenuWidgets.h"
#include "gui/Text.h"
#include "gui/TextManager.h"
#include "gui/Cursor.h"
#include "gui/book/Book.h"
#include "gui/menu/MenuCursor.h"
#include "gui/menu/MenuFader.h"

#include "graphics/Draw.h"
#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/Fade.h"

#include "input/Input.h"

#include "io/resource/ResourcePath.h"
#include "io/Screenshot.h"
#include "io/fs/Filesystem.h"
#include "io/log/Logger.h"

#include "scene/LoadLevel.h"
#include "scene/GameSound.h"
#include "scene/Light.h"

extern TextManager * pTextManage;
extern ARX_INTERFACE_BOOK_MODE g_guiBookCurrentTopTab;
extern bool START_NEW_QUEST;
extern long OLD_FLYING_OVER;
extern long FLYING_OVER;



extern float ARXTimeMenu;

extern bool REQUEST_SPEECH_SKIP;

//-----------------------------------------------------------------------------
// Exported global variables

bool bQuickGenFirstClick = true;
ARX_MENU_DATA ARXmenu;
bool g_canResumeGame = true;

static long SP_HEAD = 0;

//-----------------------------------------------------------------------------
// Menu Sounds
//-----------------------------------------------------------------------------

MENU_DYNAMIC_DATA::MENU_DYNAMIC_DATA()
	: BookBackground(NULL)
{
	flyover[BOOK_STRENGTH] = getLocalised("system_charsheet_strength");
	flyover[BOOK_MIND] = getLocalised("system_charsheet_intel");
	flyover[BOOK_DEXTERITY] = getLocalised("system_charsheet_dex");
	flyover[BOOK_CONSTITUTION] = getLocalised("system_charsheet_consti");
	flyover[BOOK_STEALTH] = getLocalised("system_charsheet_stealth");
	flyover[BOOK_MECANISM] = getLocalised("system_charsheet_mecanism");
	flyover[BOOK_INTUITION] = getLocalised("system_charsheet_intuition");
	flyover[BOOK_ETHERAL_LINK] = getLocalised("system_charsheet_etheral_link");
	flyover[BOOK_OBJECT_KNOWLEDGE] = getLocalised("system_charsheet_objknoledge");
	flyover[BOOK_CASTING] = getLocalised("system_charsheet_casting");
	flyover[BOOK_PROJECTILE] = getLocalised("system_charsheet_projectile");
	flyover[BOOK_CLOSE_COMBAT] = getLocalised("system_charsheet_closecombat");
	flyover[BOOK_DEFENSE] = getLocalised("system_charsheet_defense");
	flyover[BUTTON_QUICK_GENERATION] = getLocalised("system_charsheet_quickgenerate");
	flyover[BUTTON_DONE] = getLocalised("system_charsheet_done");
	flyover[BUTTON_SKIN] = getLocalised("system_charsheet_skin");
	flyover[WND_ATTRIBUTES] = getLocalised("system_charsheet_atributes");
	flyover[WND_SKILLS] = getLocalised("system_charsheet_skills");
	flyover[WND_STATUS] = getLocalised("system_charsheet_status");
	flyover[WND_LEVEL] = getLocalised("system_charsheet_level");
	flyover[WND_XP] = getLocalised("system_charsheet_xpoints");
	flyover[WND_HP] = getLocalised("system_charsheet_hp");
	flyover[WND_MANA] = getLocalised("system_charsheet_mana");
	flyover[WND_AC] = getLocalised("system_charsheet_ac");
	flyover[WND_RESIST_MAGIC] = getLocalised("system_charsheet_res_magic");
	flyover[WND_RESIST_POISON] = getLocalised("system_charsheet_res_poison");
	flyover[WND_DAMAGE] = getLocalised("system_charsheet_damage");
}

void ARX_MENU_LaunchAmb(const std::string & _lpszAmb) {
	ARX_SOUND_PlayMenuAmbiance(_lpszAmb);
}

void ARX_Menu_Resources_Create() {
	
	delete ARXmenu.mda;
	ARXmenu.mda = new MENU_DYNAMIC_DATA();
	ARXmenu.mda->BookBackground = TextureContainer::LoadUI("graph/interface/book/character_sheet/char_creation_bg", TextureContainer::NoColorKey);
	
	ARXmenu.mda->str_button_quickgen = getLocalised("system_charsheet_button_quickgen");
	ARXmenu.mda->str_button_skin = getLocalised("system_charsheet_button_skin");
	ARXmenu.mda->str_button_done = getLocalised("system_charsheet_button_done");

}

void ARX_Menu_Resources_Release(bool _bNoSound) {
	
	config.save();
	
	if(ARXmenu.mda == NULL) {
		return;
	}
	
	delete ARXmenu.mda->BookBackground;
	delete ARXmenu.mda, ARXmenu.mda = NULL;
	
	//Synchronize game mixers with menu mixers and switch between them
	if(_bNoSound) {
		ARXMenu_Options_Audio_ApplyGameVolumes();
	}
	
	delete g_thumbnailCursor.m_loadTexture, g_thumbnailCursor.m_loadTexture = NULL;
}

extern bool TIME_INIT;

void ARX_MENU_Clicked_QUIT() {
	
	if(!g_canResumeGame) {
		return;
	}
	
	ARX_Menu_Resources_Release();
	ARXmenu.currentmode = AMCM_OFF;
	if(TIME_INIT) {
		arxtime.resume();
	}
}

void ARX_MENU_Clicked_NEWQUEST() {
	
	arxtime.resume();
	
	g_canResumeGame = false;
	
	ARX_PLAYER_Start_New_Quest();
	g_guiBookCurrentTopTab = BOOKMODE_STATS;
	player.skin = 0;
	ARX_PLAYER_Restore_Skin();
	ARXmenu.currentmode = AMCM_NEWQUEST;
}

static void ARX_MENU_NEW_QUEST_Clicked_QUIT() {
	START_NEW_QUEST = true;
	g_canResumeGame = true;
	ARX_MENU_Clicked_QUIT();
}

void ARX_MENU_Clicked_CREDITS() {
	ARXmenu.currentmode = AMCM_CREDITS;
	credits::reset();
	ARX_MENU_LaunchAmb(AMB_CREDITS);
}

void ARX_MENU_Launch(bool allowResume) {
	
	arxtime.update();
	ARXTimeMenu = arxtime.now_f();
	
	g_canResumeGame = allowResume;

	arxtime.pause();

	//Synchronize menu mixers with game mixers and switch between them
	ARX_SOUND_MixerSwitch(ARX_SOUND_MixerGame, ARX_SOUND_MixerMenu);

	ARX_SOUND_PlayMenuAmbiance(AMB_MENU);
	ARX_SOUND_PlayMenu(SND_MENU_CLICK);

	ARXmenu.currentmode = AMCM_MAIN;
	ARX_Menu_Resources_Create();
	Menu2_Open();
}

void ARX_Menu_Manage() {
	
	// looks for keys for each mode.
	switch(ARXmenu.currentmode) {
		case AMCM_OFF: {
			// Checks for ESC key
			if(GInput->isKeyPressedNowUnPressed(Keyboard::Key_Escape)) {
				if(cinematicBorder.isActive()) {
					// Disabling ESC capture while fading in or out.
					if(!FADEDIR) {
						if(SendMsgToAllIO(SM_KEY_PRESSED, "") != REFUSE) {
							REQUEST_SPEECH_SKIP = true;
						}
					}
				} else {
					GRenderer->getSnapshot(savegame_thumbnail, config.interface.thumbnailSize.x, config.interface.thumbnailSize.y);

					arxtime.pause();

					ARX_MENU_Launch(true);
					bFadeInOut=false;	//fade out
					g_menuFadeActive = true; //active le fade
					TRUE_PLAYER_MOUSELOOK_ON = false;

					ARX_PLAYER_PutPlayerInNormalStance();
				}
			}
			break;
		}
		case AMCM_NEWQUEST: {
			if(   GInput->isKeyPressedNowUnPressed(Keyboard::Key_Escape)
			   && !bFadeInOut // XS: Disabling ESC capture while fading in or out.
			) {
				ARX_SOUND_PlayMenu(SND_MENU_CLICK);
				ARXmenu.currentmode = AMCM_MAIN;
			}
			break;
		}
		case AMCM_MAIN: {
			if(   GInput->isKeyPressedNowUnPressed(Keyboard::Key_Escape)
			   && MENU_NoActiveWindow()
			   && g_canResumeGame
			) {
				arxtime.resume();
				ARX_MENU_Clicked_QUIT();
			}
			break;
		}
		case AMCM_CREDITS: {
			if(   GInput->isKeyPressedNowUnPressed(Keyboard::Key_Escape)
			   || GInput->isKeyPressedNowUnPressed(Keyboard::Key_Spacebar)
			) {
				ARX_SOUND_PlayMenu(SND_MENU_CLICK);
				MenuFader_start(true, true, AMCM_MAIN);
				ARX_MENU_LaunchAmb(AMB_MENU);
			}
			break;
		}
		default:
			break;
	}
}

//-----------------------------------------------------------------------------
// ARX Menu Rendering Func
// returns false if no menu needs to be displayed
//-----------------------------------------------------------------------------
void ARX_Menu_Render() {
	
	if(ARXmenu.currentmode == AMCM_OFF)
		return;
	
	bool br = Menu2_Render();
	
	if(br)
		return;
	
	if(ARXmenu.currentmode == AMCM_OFF)
		return;
	
	
	GRenderer->Clear(Renderer::ColorBuffer | Renderer::DepthBuffer);
	
	FLYING_OVER = 0;
	
	//-------------------------------------------------------------------------
	
	if(ARXmenu.currentmode == AMCM_NEWQUEST && ARXmenu.mda) {
		
		GRenderer->SetRenderState(Renderer::Fog, false);
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		
		if(ARXmenu.mda->BookBackground != NULL) {
			GRenderer->SetRenderState(Renderer::AlphaBlending, false);
			GRenderer->SetRenderState(Renderer::Fog, false);
			GRenderer->SetRenderState(Renderer::DepthWrite, false);
			GRenderer->SetRenderState(Renderer::DepthTest, false);
			
			EERIEDrawBitmap2(Rectf(Vec2f(0, 0), g_size.width(), g_size.height()), 0.9f, ARXmenu.mda->BookBackground, Color::white);
		}
		
		BOOKZOOM = 1;
		
		ARX_INTERFACE_ManageOpenedBook();
		
		
		if(ARXmenu.mda) {
			bool DONE = (player.Skill_Redistribute == 0 && player.Attribute_Redistribute == 0);
			
			if(!ARXmenu.mda->flyover[FLYING_OVER].empty() ) //=ARXmenu.mda->flyover[FLYING_OVER];
			{
				if(FLYING_OVER != OLD_FLYING_OVER) {
					
					int t = Random::get(0, 2);
					
					pTextManage->Clear();
					OLD_FLYING_OVER = FLYING_OVER;
					UNICODE_ARXDrawTextCenteredScroll(hFontInGame,
						(g_size.width() * 0.5f),
						12,
						(g_size.center().x) * 0.82f,
						ARXmenu.mda->flyover[FLYING_OVER],
						Color(232 + t, 204 + t, 143 + t),
						1000,
						0.01f,
						2);
				}
			} else {
				OLD_FLYING_OVER = -1;
			}
			
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
				FLYING_OVER = BUTTON_QUICK_GENERATION;
				
				if(eeMousePressed1());
				else if (eeMouseUp1())
				{
					player.m_cheatQuickGenButtonClickCount++;
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
			
			pTextManage->AddText(hFontMenu, ARXmenu.mda->str_button_quickgen, Vec2i(pos), color);
			
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
				FLYING_OVER = BUTTON_SKIN;
				
				if(eeMouseUp1()) {
					player.m_cheatSkinButtonClickCount++;
					BOOKZOOM = 1;
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
			
			pTextManage->AddText(hFontMenu, ARXmenu.mda->str_button_skin, Vec2i(pos), color);
			
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
				
				FLYING_OVER = BUTTON_DONE;
				
				if(DONE && eeMouseUp1()) {
					if(player.m_cheatSkinButtonClickCount == 8 && player.m_cheatQuickGenButtonClickCount == 10) {
						player.m_cheatSkinButtonClickCount = -2;
					} else if(player.m_cheatSkinButtonClickCount == -1) {
						ARX_PLAYER_MakeSpHero();
						player.skin = 4;
						ARX_PLAYER_Restore_Skin();
						player.m_cheatSkinButtonClickCount = 0;
						SP_HEAD = 1;
					} else {
						if(SP_HEAD) {
							player.skin = 4;
							ARX_PLAYER_Restore_Skin();
							SP_HEAD = 0;
						}
						
						ARX_SOUND_PlayMenu(SND_MENU_CLICK);
						
						MenuFader_start(true, true, AMCM_OFF);
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
			
			if(player.m_cheatSkinButtonClickCount < 0)
				color = Color(255, 0, 255);
			
			pTextManage->AddText(hFontMenu, ARXmenu.mda->str_button_done, Vec2i(pos), color);
		}
	}
	
	EERIE_LIGHT * light = lightHandleGet(torchLightHandle);
	light->pos.x = 0.f + GInput->getMousePosAbs().x - (g_size.width() >> 1);
	light->pos.y = 0.f + GInput->getMousePosAbs().y - (g_size.height() >> 1);
	
	if(pTextManage) {
		pTextManage->Update(g_framedelay);
		pTextManage->Render();
	}
	
	if(ARXmenu.currentmode != AMCM_CREDITS)
		ARX_INTERFACE_RenderCursor(true);
	
	if(ARXmenu.currentmode == AMCM_NEWQUEST) {
		if(ProcessFadeInOut(bFadeInOut)) {
			switch(iFadeAction) {
				case AMCM_OFF:
					arxtime.resume();
					ARX_MENU_NEW_QUEST_Clicked_QUIT();
					iFadeAction = -1;
					g_menuFadeActive = false;
					fFadeInOut = 0.f;
					
					if(pTextManage)
						pTextManage->Clear();
					
					break;
			}
		}
	}
}
