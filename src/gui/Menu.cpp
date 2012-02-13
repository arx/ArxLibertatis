/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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

#include "core/Config.h"
#include "core/GameTime.h"
#include "core/Application.h"
#include "core/Localisation.h"
#include "core/Unicode.hpp"
#include "core/Core.h"

#include "game/Player.h"

#include "gui/Credits.h"
#include "gui/Interface.h"
#include "gui/MenuPublic.h"
#include "gui/MenuWidgets.h"
#include "gui/Text.h"
#include "gui/TextManager.h"

#include "graphics/Draw.h"
#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"

#include "input/Input.h"

#include "io/resource/PakReader.h"
#include "io/resource/ResourcePath.h"
#include "io/Screenshot.h"
#include "io/fs/Filesystem.h"
#include "io/log/Logger.h"

#include "platform/String.h"

#include "scene/LoadLevel.h"
#include "scene/ChangeLevel.h"
#include "scene/GameSound.h"
#include "scene/Light.h"

using std::string;
using std::istringstream;

extern TextManager * pTextManage;
extern Anglef ePlayerAngle;
extern float Xratio, Yratio;
extern ARX_INTERFACE_BOOK_MODE Book_Mode;
extern long START_NEW_QUEST;
extern long LASTBOOKBUTTON;
extern long BOOKBUTTON;
extern long OLD_FLYING_OVER;
extern long FINAL_RELEASE;
extern long FLYING_OVER;
extern long BOOKZOOM;
extern long FRAME_COUNT;
extern float ARXTimeMenu;
extern float ARXOldTimeMenu;
extern long NEED_SPECIAL_RENDEREND;
extern bool bFadeInOut;
extern bool bFade;
extern int iFadeAction;
extern float fFadeInOut;
extern char SKIN_MOD;
extern char QUICK_MOD;


extern float ARXTimeMenu;
extern float ARXOldTimeMenu;
extern float ARXDiffTimeMenu;

extern long	REQUEST_SPEECH_SKIP;
extern bool TRUE_PLAYER_MOUSELOOK_ON;

extern TextureContainer * pTextureLoad;

bool MENU_NoActiveWindow();

//-----------------------------------------------------------------------------
// Exported global variables

bool bQuickGenFirstClick = true;
ARX_MENU_DATA ARXmenu;
long REFUSE_GAME_RETURN = 0;

long SP_HEAD = 0;

//-----------------------------------------------------------------------------
#define ARX_MENU_SIZE_Y 24



void ARX_MENU_CLICKSOUND() {
	ARX_SOUND_PlayMenu(SND_MENU_CLICK);
}

//-----------------------------------------------------------------------------
// Menu Sounds
//-----------------------------------------------------------------------------

void ARX_MENU_LaunchAmb(const string & _lpszAmb) {
	ARX_SOUND_PlayMenuAmbiance(_lpszAmb);
}

void ARX_Menu_Resources_Create() {
	
	if (ARXmenu.mda)
	{
		delete ARXmenu.mda;
		ARXmenu.mda = NULL;
	}

	ARXmenu.mda = new MENU_DYNAMIC_DATA();
	ARXmenu.mda->pTexCredits = TextureContainer::LoadUI("graph/interface/menus/menu_credits");
	ARXmenu.mda->BookBackground = TextureContainer::LoadUI("graph/interface/book/character_sheet/char_creation_bg", TextureContainer::NoColorKey);

	ARXmenu.mda->flyover[BOOK_STRENGTH] = getLocalised("system_charsheet_strength");
	ARXmenu.mda->flyover[BOOK_MIND] = getLocalised("system_charsheet_intel");
	ARXmenu.mda->flyover[BOOK_DEXTERITY] = getLocalised("system_charsheet_dex");
	ARXmenu.mda->flyover[BOOK_CONSTITUTION] = getLocalised("system_charsheet_consti");
	ARXmenu.mda->flyover[BOOK_STEALTH] = getLocalised("system_charsheet_stealth");
	ARXmenu.mda->flyover[BOOK_MECANISM] = getLocalised("system_charsheet_mecanism");
	ARXmenu.mda->flyover[BOOK_INTUITION] = getLocalised("system_charsheet_intuition");
	ARXmenu.mda->flyover[BOOK_ETHERAL_LINK] = getLocalised("system_charsheet_etheral_link");
	ARXmenu.mda->flyover[BOOK_OBJECT_KNOWLEDGE] = getLocalised("system_charsheet_objknoledge");
	ARXmenu.mda->flyover[BOOK_CASTING] = getLocalised("system_charsheet_casting");
	ARXmenu.mda->flyover[BOOK_PROJECTILE] = getLocalised("system_charsheet_projectile");
	ARXmenu.mda->flyover[BOOK_CLOSE_COMBAT] = getLocalised("system_charsheet_closecombat");
	ARXmenu.mda->flyover[BOOK_DEFENSE] = getLocalised("system_charsheet_defense");
	ARXmenu.mda->flyover[BUTTON_QUICK_GENERATION] = getLocalised("system_charsheet_quickgenerate");
	ARXmenu.mda->flyover[BUTTON_DONE] = getLocalised("system_charsheet_done");
	ARXmenu.mda->flyover[BUTTON_SKIN] = getLocalised("system_charsheet_skin");
	ARXmenu.mda->flyover[WND_ATTRIBUTES] = getLocalised("system_charsheet_atributes");
	ARXmenu.mda->flyover[WND_SKILLS] = getLocalised("system_charsheet_skills");
	ARXmenu.mda->flyover[WND_STATUS] = getLocalised("system_charsheet_status");
	ARXmenu.mda->flyover[WND_LEVEL] = getLocalised("system_charsheet_level");
	ARXmenu.mda->flyover[WND_XP] = getLocalised("system_charsheet_xpoints");
	ARXmenu.mda->flyover[WND_HP] = getLocalised("system_charsheet_hp");
	ARXmenu.mda->flyover[WND_MANA] = getLocalised("system_charsheet_mana");
	ARXmenu.mda->flyover[WND_AC] = getLocalised("system_charsheet_ac");
	ARXmenu.mda->flyover[WND_RESIST_MAGIC] = getLocalised("system_charsheet_res_magic");
	ARXmenu.mda->flyover[WND_RESIST_POISON] = getLocalised("system_charsheet_res_poison");
	ARXmenu.mda->flyover[WND_DAMAGE] = getLocalised("system_charsheet_damage");

	ARXmenu.mda->str_button_quickgen = getLocalised("system_charsheet_button_quickgen");
	ARXmenu.mda->str_button_skin = getLocalised("system_charsheet_button_skin");
	ARXmenu.mda->str_button_done = getLocalised("system_charsheet_button_done");

	
	// Load credits.
	
	string creditsFile = "localisation/ucredits_" +  config.language + ".txt";
	
	size_t creditsSize;
	u16 * creditsData = (u16*)resources->readAlloc(creditsFile, creditsSize);
	
	if(!creditsData) {
		LogWarning << "unable to read credits file " << creditsFile;
	} else {
		
		u16 * credits = creditsData;
		
		creditsSize /= sizeof(*credits);
		
		if(creditsSize >= 1 && credits[0] == 0xFEFF) {
			credits++;
			creditsSize--;
		}
		
		LogDebug("Loaded credits file: " << creditsFile << " of size " << creditsSize);
		
		// TODO move this to an external file once we ship our own resources
		ARXmenu.mda->str_cre_credits =
			"~ARX LIBERTATIS TEAM\n"
			"Daniel Scharrer (dscharrer)\n"
			"Erik Lund (Akhilla)\n"
			"Lubosz Sarnecki (lubosz)\n"
			"Sebastien Lussier (BobJelly)\n"
			"Elliot Rigets (adejr)\n"
			"\n~OTHER CONTRIBUTORS\n"
			"Chris Gray (chrismgray)\n"
			"guidoj\n"
			"Jonathan Powell (jfpowell)\n"
			"Philippe Cavalaria (Nuky)\n"
			"\n~TESTERS\n"
			"vytautas (aka. ProzacR)\n"
			"\n\n~Arx Libertatis is free software:\n"
			"you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.\n\n"
			"Arx Libertatis is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.\n\n"
			"You should have received a copy of the GNU General Public License along with Arx Libertatis. If not, see <http://www.gnu.org/licenses/>.\n\n"
			"\n~ORIGINAL ARX FATALIS CREDITS:\n\n\n";
		
		ARXmenu.mda->str_cre_credits.reserve(ARXmenu.mda->str_cre_credits.size() + creditsSize);
		
		UTF16ToUTF8(credits, &credits[creditsSize],
		            std::back_inserter(ARXmenu.mda->str_cre_credits));
		LogDebug("Converted to UTF8 string of length " << ARXmenu.mda->str_cre_credits.size());
		
		free(creditsData);
	}
}

//-----------------------------------------------------------------------------
void ARX_Menu_Resources_Release(bool _bNoSound)
{
	config.save();

	if (ARXmenu.mda == NULL)
		return;

	if (ARXmenu.mda->Background != NULL)
	{
		delete ARXmenu.mda->Background;
		ARXmenu.mda->Background = NULL;
	}

	if (ARXmenu.mda->BookBackground != NULL)
	{
		delete ARXmenu.mda->BookBackground;
		ARXmenu.mda->BookBackground = NULL;
	}
	
	delete ARXmenu.mda;
	ARXmenu.mda = NULL;

	//Synchronize game mixers with menu mixers and switch between them
	if(_bNoSound) {
		ARXMenu_Options_Audio_ApplyGameVolumes();
	}

	if (pTextureLoad)
	{
		delete pTextureLoad;
		pTextureLoad = NULL;
	}
}

extern long NO_TIME_INIT;
//-----------------------------------------------------------------------------
void ARX_MENU_Clicked_QUIT()
{
	if (REFUSE_GAME_RETURN) return;

	ARX_Menu_Resources_Release();
	ARXmenu.currentmode = AMCM_OFF;

	if (!NO_TIME_INIT)
		arxtime.resume();
}

void ARX_MENU_Clicked_NEWQUEST() {
	
	arxtime.resume();

	if (FINAL_RELEASE)
	{
		REFUSE_GAME_RETURN = 1;
	}

	ARX_PLAYER_Start_New_Quest();
	Book_Mode = BOOKMODE_STATS;
	player.skin = 0;
	ePlayerAngle.b = -25.f;
	ARX_PLAYER_Restore_Skin();
	ARXmenu.currentmode = AMCM_NEWQUEST;
}

//-----------------------------------------------------------------------------
void ARX_MENU_NEW_QUEST_Clicked_QUIT()
{
	START_NEW_QUEST = 1;
	REFUSE_GAME_RETURN = 0;
	NEED_SPECIAL_RENDEREND = 1;
	ARX_MENU_Clicked_QUIT();
}

//-----------------------------------------------------------------------------
void ARX_MENU_Clicked_CREDITS()
{
	ARXmenu.currentmode = AMCM_CREDITS;
	Credits::reset();
	ARX_MENU_LaunchAmb(AMB_CREDITS);
}

void ARX_MENU_Clicked_QUIT_GAME() {
	
#ifdef BUILD_EDITOR
	if(GAME_EDITOR) {
		ARX_MENU_Clicked_QUIT();
		return;
	}
#endif
	
	mainApp->Quit();
}

void ARX_MENU_Launch() {
	
	arxtime.pause();

	//Synchronize menu mixers with game mixers and switch between them
	ARX_SOUND_MixerSwitch(ARX_SOUND_MixerGame, ARX_SOUND_MixerMenu);

	ARX_SOUND_PlayMenuAmbiance(AMB_MENU);
	ARX_MENU_CLICKSOUND();

	ARXmenu.currentmode = AMCM_MAIN;
	ARX_Menu_Resources_Create();
}

void ARX_Menu_Manage() {
	
	// looks for keys for each mode.
	switch (ARXmenu.currentmode)
	{
		case AMCM_OFF:
		{
			// Checks for ESC key
			if (GInput->isKeyPressedNowUnPressed(Keyboard::Key_Escape))
			{
				if (CINEMASCOPE)
				{
					if (!FADEDIR)	// Disabling ESC capture while fading in or out.
					{
						if (SendMsgToAllIO(SM_KEY_PRESSED,"")!=REFUSE)
						{
							REQUEST_SPEECH_SKIP=1;				
						}
					}
				} else {
					
					GRenderer->getSnapshot(savegame_thumbnail, 160, 100);

					arxtime.pause();
					ARXTimeMenu=ARXOldTimeMenu=arxtime.get_updated();
					ARX_MENU_Launch();
					bFadeInOut=false;	//fade out
					bFade=true;			//active le fade
					TRUE_PLAYER_MOUSELOOK_ON = false;

					ARX_PLAYER_PutPlayerInNormalStance(1);
				}
			}
		}
		break;
		case AMCM_NEWQUEST:
		{
			if (GInput->isKeyPressedNowUnPressed(Keyboard::Key_Escape)
					&&	! bFadeInOut // XS: Disabling ESC capture while fading in or out.
			   )
			{
				ARX_MENU_CLICKSOUND();
				ARXmenu.currentmode = AMCM_MAIN;
			}
		}
		break;
		case AMCM_MAIN:

			if (GInput->isKeyPressedNowUnPressed(Keyboard::Key_Escape))
			{
				if ((MENU_NoActiveWindow())  && (!REFUSE_GAME_RETURN))
				{
					arxtime.resume();
					ARX_MENU_Clicked_QUIT();
				}
			}

			break;
		case AMCM_CREDITS:

			if ((GInput->isKeyPressedNowUnPressed(Keyboard::Key_Escape))
					|| (GInput->isKeyPressedNowUnPressed(Keyboard::Key_Spacebar)))
			{
				ARX_MENU_CLICKSOUND();
				bFadeInOut = true;	//fade out
				bFade = true;			//active le fade
				iFadeAction = AMCM_MAIN;

				ARX_MENU_LaunchAmb(AMB_MENU);
			}

			break;
		default:
			break;
	}
}
extern long PLAYER_INTERFACE_HIDE_COUNT;
extern long SPLASH_THINGS_STAGE;
//-----------------------------------------------------------------------------
// ARX Menu Rendering Func
// returns false if no menu needs to be displayed
//-----------------------------------------------------------------------------
bool ARX_Menu_Render() {
	
	if(ARXmenu.currentmode == AMCM_OFF) {
		return false;
	}

	FRAME_COUNT = 0;

	bool br = Menu2_Render();

	if (br)
	{
		return br;
	}

	if (ARXmenu.currentmode == AMCM_OFF)
	{
		return false;
	}

	if (GInput->getMouseButton(Mouse::Button_0))
	{
		EERIEMouseButton = 1;
		LastMouseClick = 1;
	}
	else if (GInput->getMouseButton(Mouse::Button_1))
	{
		EERIEMouseButton = 2;
		LastMouseClick = 2;
	}
	else
	{
		EERIEMouseButton = 0;
	}

	GRenderer->BeginScene();

	GRenderer->Clear(Renderer::ColorBuffer | Renderer::DepthBuffer);
	
	FLYING_OVER = 0;

	//-------------------------------------------------------------------------

	if ((ARXmenu.currentmode == AMCM_NEWQUEST) && (ARXmenu.mda))
	{
		if (ITC.Get("questbook") == NULL)
		{
			ARX_Menu_Resources_Release(false);
			ARX_Menu_Resources_Create();
			
			// TODO this is also in Interface.cpp

			ITC.Set("playerbook", "graph/interface/book/character_sheet/char_sheet_book");
			ITC.Set("ic_casting", "graph/interface/book/character_sheet/buttons_carac/icone_casting");
			ITC.Set("ic_close_combat", "graph/interface/book/character_sheet/buttons_carac/icone_close_combat");
			ITC.Set("ic_constitution", "graph/interface/book/character_sheet/buttons_carac/icone_constit");
			ITC.Set("ic_defense", "graph/interface/book/character_sheet/buttons_carac/icone_defense");
			ITC.Set("ic_dexterity", "graph/interface/book/character_sheet/buttons_carac/icone_dext");
			ITC.Set("ic_etheral_link", "graph/interface/book/character_sheet/buttons_carac/icone_etheral_link");
			ITC.Set("ic_mind", "graph/interface/book/character_sheet/buttons_carac/icone_intel");
			ITC.Set("ic_intuition", "graph/interface/book/character_sheet/buttons_carac/icone_intuition");
			ITC.Set("ic_mecanism", "graph/interface/book/character_sheet/buttons_carac/icone_mecanism");
			ITC.Set("ic_object_knowledge", "graph/interface/book/character_sheet/buttons_carac/icone_obj_knowledge");
			ITC.Set("ic_projectile", "graph/interface/book/character_sheet/buttons_carac/icone_projectile");
			ITC.Set("ic_stealth", "graph/interface/book/character_sheet/buttons_carac/icone_stealth");
			ITC.Set("ic_strength", "graph/interface/book/character_sheet/buttons_carac/icone_strenght");

			ITC.Set("questbook", "graph/interface/book/questbook");
			ITC.Set("ptexspellbook", "graph/interface/book/spellbook");
			ITC.Set("bookmark_char", "graph/interface/book/bookmark_char");
			ITC.Set("bookmark_magic", "graph/interface/book/bookmark_magic");
			ITC.Set("bookmark_map", "graph/interface/book/bookmark_map");
			ITC.Set("bookmark_quest", "graph/interface/book/bookmark_quest");

			ITC.Set("accessible_1", "graph/interface/book/accessible/accessible_1");
			ITC.Set("accessible_2", "graph/interface/book/accessible/accessible_2");
			ITC.Set("accessible_3", "graph/interface/book/accessible/accessible_3");
			ITC.Set("accessible_4", "graph/interface/book/accessible/accessible_4");
			ITC.Set("accessible_5", "graph/interface/book/accessible/accessible_5");
			ITC.Set("accessible_6", "graph/interface/book/accessible/accessible_6");
			ITC.Set("accessible_7", "graph/interface/book/accessible/accessible_7");
			ITC.Set("accessible_8", "graph/interface/book/accessible/accessible_8");
			ITC.Set("accessible_9", "graph/interface/book/accessible/accessible_9");
			ITC.Set("accessible_10", "graph/interface/book/accessible/accessible_10");
			ITC.Set("current_1", "graph/interface/book/current_page/current_1");
			ITC.Set("current_2", "graph/interface/book/current_page/current_2");
			ITC.Set("current_3", "graph/interface/book/current_page/current_3");
			ITC.Set("current_4", "graph/interface/book/current_page/current_4");
			ITC.Set("current_5", "graph/interface/book/current_page/current_5");
			ITC.Set("current_6", "graph/interface/book/current_page/current_6");
			ITC.Set("current_7", "graph/interface/book/current_page/current_7");
			ITC.Set("current_8", "graph/interface/book/current_page/current_8");
			ITC.Set("current_9", "graph/interface/book/current_page/current_9");
			ITC.Set("current_10", "graph/interface/book/current_page/current_10");

			ITC.Set("ptexcursorredist", "graph/interface/cursors/add_points");

			ITC.Set("ptexcornerleft", "graph/interface/book/left_corner_original");
			ITC.Set("ptexcornerright", "graph/interface/book/right_corner_original");

			ITC.Level = getLocalised("system_charsheet_player_lvl");
			ITC.Xp = getLocalised("system_charsheet_player_xp");

			ANIM_Set(&player.useanim, herowaitbook);

			player.useanim.flags |= EA_LOOP;

			ARXOldTimeMenu = ARXTimeMenu = arxtime.get_updated();
			ARXDiffTimeMenu = 0;
		}

		GRenderer->SetRenderState(Renderer::Fog, false);
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);

		if (ARXmenu.mda->BookBackground != NULL)
		{
			GRenderer->SetRenderState(Renderer::AlphaBlending, false);
			GRenderer->SetRenderState(Renderer::Fog, false);
			GRenderer->SetRenderState(Renderer::DepthWrite, false);
			GRenderer->SetRenderState(Renderer::DepthTest, false);

			EERIEDrawBitmap2(0, 0, static_cast<float>(DANAESIZX), static_cast<float>(DANAESIZY), 0.9f, ARXmenu.mda->BookBackground, Color::white);
		}

		BOOKZOOM = 1;

		ARX_INTERFACE_ManageOpenedBook();


		if (ARXmenu.mda)
		{
			long DONE = 0;

			if ((player.Skill_Redistribute == 0) && (player.Attribute_Redistribute == 0))
				DONE = 1;

			float ox, oy;
			ox = Xratio;
			oy = Yratio;
			LASTBOOKBUTTON = BOOKBUTTON;
			BOOKBUTTON = EERIEMouseButton;
			Xratio = ox;
			Yratio = oy;

			if (!ARXmenu.mda->flyover[FLYING_OVER].empty() ) //=ARXmenu.mda->flyover[FLYING_OVER];
			{
				if (FLYING_OVER != OLD_FLYING_OVER)
				{

					float fRandom	= rnd() * 2;

					int t = checked_range_cast<int>(fRandom);


					pTextManage->Clear();
					OLD_FLYING_OVER = FLYING_OVER;
					UNICODE_ARXDrawTextCenteredScroll(hFontInGame,
						(DANAESIZX * 0.5f),
						12,
						(DANAECENTERX) * 0.82f,
						ARXmenu.mda->flyover[FLYING_OVER],
						Color(232 + t, 204 + t, 143 + t),
						1000,
						0.01f,
						2);
				}
			}
			else
			{
				OLD_FLYING_OVER = -1;
			}
			
			float fPosX = 0;
			float fPosY = 313 * Yratio + (DANAESIZY - 313 * Yratio) * 0.70f;

			float fSizeX = 100 * Xratio;
			float fSizeY = 100 * Yratio;

			Color color = Color::none;

			//---------------------------------------------------------------------
			// Button QUICK GENERATION
			fPosX = (DANAESIZX - (513 * Xratio)) * 0.5f;

			if (MouseInRect(fPosX, fPosY, fPosX + fSizeX + 50, fPosY + fSizeY))
			{
				SpecialCursor = CURSOR_INTERACTION_ON;
				FLYING_OVER = BUTTON_QUICK_GENERATION;

				if (EERIEMouseButton & 1) ;
				else if ((!(EERIEMouseButton & 1)) && (LastMouseClick & 1))
				{
					QUICK_MOD++;
					int iSkin = player.skin;
					ARX_MENU_CLICKSOUND();

					if (bQuickGenFirstClick)
					{
						ARX_PLAYER_MakeAverageHero();
						bQuickGenFirstClick = false;
					}
					else
					{
						ARX_PLAYER_QuickGeneration();
					}

					player.skin = checked_range_cast<char>(iSkin);
				}

				color = Color(255, 255, 255);
			}
			else
				color = Color(232, 204, 143);

			pTextManage->AddText(hFontMenu, ARXmenu.mda->str_button_quickgen, static_cast<long>(fPosX), static_cast<long>(fPosY), color);

			//---------------------------------------------------------------------
			// Button SKIN
			fPosX = DANAESIZX * 0.5f;

			if (MouseInRect(fPosX, fPosY, fPosX + fSizeX, fPosY + fSizeY))
			{
				SpecialCursor = CURSOR_INTERACTION_ON;
				FLYING_OVER = BUTTON_SKIN;

				if ((!(EERIEMouseButton & 1)) && (LastMouseClick & 1))
				{
					SKIN_MOD++;
					BOOKZOOM = 1;
					ARX_MENU_CLICKSOUND();
					player.skin++;

					if (player.skin > 3)  player.skin = 0;

					switch (player.skin)
					{
						case 0:
							ePlayerAngle.b = -25.f;
							break;
						case 1:
							ePlayerAngle.b = -10.f;
							break;
						case 2:
							ePlayerAngle.b = 20.f;
							break;
						case 3:
							ePlayerAngle.b = 35.f;
							break;
					}

					ARX_PLAYER_Restore_Skin();
				}

				color = Color(255, 255, 255);
			}
			else
				color = Color(232, 204, 143);

			pTextManage->AddText(hFontMenu, ARXmenu.mda->str_button_skin, static_cast<long>(fPosX), static_cast<long>(fPosY), color);

			//---------------------------------------------------------------------
			// Button DONE
			fPosX = DANAESIZX - (DANAESIZX - 513 * Xratio) * 0.5f - 40 * Xratio;

			if (MouseInRect(fPosX, fPosY, fPosX + fSizeX, fPosY + fSizeY))
			{
				if (DONE) SpecialCursor = CURSOR_INTERACTION_ON;

				FLYING_OVER = BUTTON_DONE;

				if ((DONE) && (!(EERIEMouseButton & 1)) && (LastMouseClick & 1))
				{
					if ((SKIN_MOD == 8) && (QUICK_MOD == 10))
					{
						SKIN_MOD = -2;
					}
					else if (SKIN_MOD == -1)
					{
						ARX_PLAYER_MakeSpHero();
						player.skin = 4;
						ARX_PLAYER_Restore_Skin();
						SKIN_MOD = 0;
						SP_HEAD = 1;
					}
					else
					{
						if (SP_HEAD)
						{
							player.skin = 4;
							ARX_PLAYER_Restore_Skin();
							SP_HEAD = 0;
						}

						ARX_MENU_CLICKSOUND();

						bFadeInOut = true;		//fade out
						bFade = true;			//active le fade
						iFadeAction = AMCM_OFF;
					}
				}
				else
				{
					if (DONE)
						color = Color(255, 255, 255);
					else
						color = Color(192, 192, 192);
				}

			}
			else
			{
				if (DONE)
					color = Color(232, 204, 143);
				else
					color = Color(192, 192, 192);
			}

			if (SKIN_MOD < 0)
				color = Color(255, 0, 255);

			pTextManage->AddText(hFontMenu, ARXmenu.mda->str_button_done, static_cast<long>(fPosX), static_cast<long>(fPosY), color);
		}
	}

	DynLight[0].pos.x = 0.f + GInput->getMousePosAbs().x - (DANAESIZX >> 1);
	DynLight[0].pos.y = 0.f + GInput->getMousePosAbs().y - (DANAESIZY >> 1);

	if (pTextManage)
	{
		pTextManage->Update(FrameDiff);
		pTextManage->Render();
	}

	if (ARXmenu.currentmode != AMCM_CREDITS)
		ARX_INTERFACE_RenderCursor(1);

	if (ARXmenu.currentmode == AMCM_NEWQUEST)
	{
		if (ProcessFadeInOut(bFadeInOut, 0.1f))
		{
			switch (iFadeAction)
			{
				case AMCM_OFF:
					arxtime.resume();
					ARX_MENU_NEW_QUEST_Clicked_QUIT();
					iFadeAction = -1;
					bFade = false;
					fFadeInOut = 0.f;

					if (pTextManage)
						pTextManage->Clear();

					break;
			}
		}
	}

	GRenderer->EndScene();
	return true;
}
