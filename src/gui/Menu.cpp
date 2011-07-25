/*
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
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// ARX_Menu
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Menu Management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "gui/Menu.h"

#include <cstdlib>
#include <sstream>
#include <cstdio>
#include <iterator>
#include <iomanip>

#ifndef DIRECTINPUT_VERSION
	#define DIRECTINPUT_VERSION 0x0700
#endif
#include <dinput.h>

#include <boost/smart_ptr/scoped_array.hpp>

#include "Configure.h"

#include "core/Config.h"
#include "core/GameTime.h"
#include "core/Application.h"
#include "core/Localisation.h"
#include "core/Unicode.hpp"
#include "core/Core.h"

#include "game/Equipment.h"
#include "game/Player.h"

#include "gui/MenuWidgets.h"
#include "gui/Text.h"
#include "gui/Interface.h"
#include "gui/Credits.h"
#include "gui/MenuPublic.h"
#include "gui/TextManager.h"

#include "graphics/Draw.h"
#include "graphics/Math.h"
#include "graphics/particle/Particle.h"
#include "graphics/particle/ParticleManager.h"
#include "graphics/particle/ParticleParams.h"
#include "graphics/font/Font.h"

#include "io/PakReader.h"
#include "io/Logger.h"
#include "io/FilePath.h"


#include "scene/LoadLevel.h"
#include "scene/Object.h"
#include "scene/ChangeLevel.h"
#include "scene/GameSound.h"
#include "scene/Light.h"

#include "window/DXInput.h"

using std::string;
using std::istringstream;

extern TextManager * pTextManage;
extern CDirectInput * pGetInfoDirectInput;
extern Anglef ePlayerAngle;
extern float Xratio, Yratio;
extern ARX_INTERFACE_BOOK_MODE Book_Mode;
extern long START_NEW_QUEST;
extern long INTRO_NOT_LOADED;
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

extern float PROGRESS_BAR_TOTAL;
extern float OLD_PROGRESS_BAR_COUNT;
extern float PROGRESS_BAR_COUNT;

extern float ARXTimeMenu;
extern float ARXOldTimeMenu;
extern float ARXDiffTimeMenu;

extern TextureContainer * pTextureLoad;

bool MENU_NoActiveWindow();
void GetTextSize(HFONT _hFont, const char* _lpszUText, int * _iWidth, int * _iHeight);

//-----------------------------------------------------------------------------
// Exported global variables

bool bQuickGenFirstClick = true;
ARX_MENU_DATA ARXmenu;
long ARXmenu_lastmode = -1;
long REFUSE_GAME_RETURN = 0;
unsigned long ARXmenu_starttick = 0;

long SP_HEAD = 0;

//-----------------------------------------------------------------------------
#define ARX_MENU_SIZE_Y 24

 
std::vector<SaveGame> save_l;

static int saveTimeCompare(const SaveGame & a, const SaveGame & b) {
	return (a.stime > b.stime);
}

void CreateSaveGameList() {
	
	LogDebug << "CreateSaveGameList";
	
	if(save_l.empty()) {
		save_l.resize(1);
		save_l[0].name = "New";
	}
	
	size_t oldCount = save_l.size() - 1;
#ifdef HAVE_DYNAMIC_STACK_ALLOCATION
	char found[oldCount];
#else
	boost::scoped_array<char> found(new char[oldCount]);
#endif
	std::fill_n(&found[0], oldCount, 0);
	
	bool newSaves = false;
	
	size_t maxlength = 0;
	
	fs::path savedir = "save";
	
	fs_boost::directory_iterator end;
	for(fs_boost::directory_iterator it(savedir.string()); it != end; ++it) {
		
		fs::path path = savedir / as_string(it->path().filename());
		string dirname = as_string(path.filename());
		
		if(dirname.compare(0, 4, "save") || !fs::is_directory(path)) {
			continue;
		}
		
		istringstream iss(dirname.substr(4));
		long num;
		iss >> num;
		
		std::time_t stime = fs::last_write_time(path / "gsave.sav");
		if(stime == 0) {
			continue;
		}
		
		size_t index = (size_t)-1;
		for(size_t i = 1; i <= oldCount; i++) {
			if(save_l[i].num == num) {
				index = i;
			}
		}
		if(index != (size_t)-1 && save_l[index].stime == stime) {
			found[index - 1] = 1;
			continue;
		}
		
		string name;
		float version;
		long level;
		unsigned long ignored;
		if(ARX_CHANGELEVEL_GetInfo(path, name, version, level, ignored) == -1) {
			LogWarning << "unable to get save file info for " << path;
			continue;
		}
		
		newSaves = true;
		
		SaveGame * save;
		if(index == (size_t)-1) {
			// Make another save game slot at the end
			save_l.resize(save_l.size() + 1);
			save = &save_l.back();
		} else {
			found[index - 1] = 2;
			save = &save_l[index];
		}
		
		save->name = name;
		save->version = version;
		save->level = level;
		save->stime = stime;
		save->num = num;
		
		save->quicksave = (name == "ARX_QUICK_ARX" || name == "ARX_QUICK_ARX1");
		
		fs::path thumbnail = path / "gsave.bmp";
		if(fs::exists(thumbnail)) {
			std::ostringstream oss;
			oss << "save/save" << std::setw(4) << std::setfill('0') << num << "/gsave.bmp";
			resources->removeFile(oss.str());
			resources->addFiles(thumbnail, oss.str());
		}
		
		maxlength = std::max(save->quicksave ? 9 : name.length(), maxlength);
		
		const struct tm & t = *localtime(&stime);
		std::ostringstream oss;
		oss << std::setfill('0') << (t.tm_year + 1900) << "-" << std::setw(2) << (t.tm_mon + 1) << "-" << std::setw(2) << t.tm_mday << "   " << std::setfill(' ') << std::setw(2) << t.tm_hour << ":" << std::setfill('0') << std::setw(2) << t.tm_min << ":" << std::setw(2) << t.tm_sec;
		save->time = oss.str();
	}
	
	size_t o = 1;
	for(size_t i = 1; i < save_l.size(); i++) {
		if(i > oldCount || found[i - 1]) {
			
			// print new savegames
			if(i > oldCount || found[i - 1] == 2) {
				
				std::ostringstream oss;
				if(save_l[i].quicksave) {
					oss << "(quicksave)" << std::setw(maxlength - 8) << ' ';
				} else {
					oss << "\"" << save_l[i].name << "\"" << std::setw(maxlength - save_l[i].name.length() + 1) << ' ';
				}
				
				oss << "  " << save_l[i].time << "   v" << save_l[i].version;
				LogInfo << "found save " << oss.str();
				
			}
			
			if(o != i) {
				save_l[o] = save_l[i];
			}
			o++;
		}
	}
	save_l.resize(o);
	
	if(newSaves && save_l.size() > 2) {
		std::sort(save_l.begin() + 1, save_l.end(), saveTimeCompare);
	}
	
	LogDebug << "found " << (save_l.size()-1) << " savegames";
	
}

//-----------------------------------------------------------------------------
void FreeSaveGameList() {
	save_l.clear();
}

//-----------------------------------------------------------------------------
void UpdateSaveGame(const long & i)
{
	//i == 0 -> new save game
	//i >  0 -> erase old savegame save_l[i].name
	if (i <= 0)
		ARX_CHANGELEVEL_Save(i, save_l[0].name);
	else
		ARX_CHANGELEVEL_Save(save_l[i].num, save_l[i].name);
}

//-----------------------------------------------------------------------------
void ARX_MENU_CLICKSOUND()
{
	ARX_SOUND_PlayMenu(SND_MENU_CLICK);
}

//-----------------------------------------------------------------------------
void LoadSaveGame(const long & i)
{
	ARX_MENU_CLICKSOUND();
	INTRO_NOT_LOADED = 1;
	LoadLevelScreen();	
	PROGRESS_BAR_TOTAL = 238;
	OLD_PROGRESS_BAR_COUNT = PROGRESS_BAR_COUNT = 0;
	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen(save_l[i].level);
	DanaeClearLevel();
	ARX_CHANGELEVEL_Load(save_l[i].num);
	REFUSE_GAME_RETURN = 0;
	NEED_SPECIAL_RENDEREND = 1;
	ARX_MENU_Clicked_QUIT();
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
	ARXmenu.mda->pTexCredits = TextureContainer::LoadUI("graph/interface/menus/menu_credits.bmp");
	ARXmenu.mda->BookBackground = TextureContainer::LoadUI("graph/interface/book/character_sheet/char_creation_bg.bmp");

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
		
		LogDebug << "Loaded credits file: " << creditsFile << " of size " << creditsSize;
		
		ARXmenu.mda->str_cre_credits.reserve(creditsSize);
		
		UTF16ToUTF8(credits, &credits[creditsSize],
		                         std::back_inserter(ARXmenu.mda->str_cre_credits));
		LogDebug << "Converted to UTF8 string of length " << ARXmenu.mda->str_cre_credits.size();
		
		free(creditsData);
	}
	
	CreateSaveGameList();
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

	if (INTRO_NOT_LOADED == 2) return;

	ARX_Menu_Resources_Release();
	ARXmenu.currentmode = AMCM_OFF;

	if (!NO_TIME_INIT)
		ARX_TIME_UnPause();
}
long CAN_REPLAY_INTRO = 1;
//-----------------------------------------------------------------------------
void ARX_MENU_Clicked_NEWQUEST()
{
	CAN_REPLAY_INTRO = 0;
	ARX_TIME_UnPause();

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
	INTRO_NOT_LOADED = 1;
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
extern long FINAL_COMMERCIAL_DEMO;
bool ARX_IsSteam();

void ARX_MENU_Clicked_QUIT_GAME() {
	
#ifdef BUILD_EDITOR
	if(GAME_EDITOR) {
		ARX_MENU_Clicked_QUIT();
		return;
	}
#endif
	
	exit(0);
}

void ARX_MENU_Launch() {
	
	ARX_TIME_Pause();

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
			/* Checked in Danae.cpp now ! */
			return;
			break;
		case AMCM_NEWQUEST:
		{
			if (pGetInfoDirectInput->IsVirtualKeyPressedNowUnPressed(DIK_ESCAPE)
					&&	! bFadeInOut // XS: Disabling ESC capture while fading in or out.
			   )
			{
				ARX_MENU_CLICKSOUND();
				ARXmenu.currentmode = AMCM_MAIN;
			}
		}
		break;
		case AMCM_MAIN:

			if (pGetInfoDirectInput->IsVirtualKeyPressedNowUnPressed(DIK_ESCAPE))
			{
				if ((MENU_NoActiveWindow())  && (!REFUSE_GAME_RETURN))
				{
					ARX_TIME_UnPause();
					ARX_MENU_Clicked_QUIT();
				}
			}

			break;
		case AMCM_CREDITS:

			if ((pGetInfoDirectInput->IsVirtualKeyPressedNowUnPressed(DIK_ESCAPE))
					|| (pGetInfoDirectInput->IsVirtualKeyPressedNowUnPressed(DIK_SPACE)))
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
long NEED_INTRO_LAUNCH = 0;
//-----------------------------------------------------------------------------
// ARX Menu Rendering Func
// returns false if no menu needs to be displayed
//-----------------------------------------------------------------------------
bool ARX_Menu_Render()
{
	// Auto-Launch Demo after 60 sec idle on Main Menu
	if ((ARXmenu.currentmode == AMCM_MAIN) && CAN_REPLAY_INTRO)
	{
		if ((ARXmenu_lastmode != AMCM_MAIN) || (pGetInfoDirectInput && (pGetInfoDirectInput->bTouch || pGetInfoDirectInput->bMouseMove)))
		{
			ARXmenu_starttick = ARX_TIME_GetUL(); //treat warning C4244 conversion from 'float' to 'unsigned long'
		}

		unsigned long tim = ARX_TIME_GetUL() - ARXmenu_starttick; //treat warning C4244 conversion from 'float' to 'unsigned long'

		if ((tim > 180000) && (REFUSE_GAME_RETURN))
		{
			NEED_INTRO_LAUNCH = 1;
		}
	}
	else
		ARXmenu_starttick = ARX_TIME_GetUL(); //treat warning C4244 conversion from 'float' to 'unsigned long'

	ARXmenu_lastmode = ARXmenu.currentmode;

	if (ARXmenu.currentmode == AMCM_OFF)
	{
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

	if (pGetInfoDirectInput->GetMouseButton(DXI_BUTTON0))
	{
		EERIEMouseButton = 1;
		LastMouseClick = 1;
	}
	else if (pGetInfoDirectInput->GetMouseButton(DXI_BUTTON1))
	{
		EERIEMouseButton = 2;
		LastMouseClick = 2;
	}
	else
	{
		EERIEMouseButton = 0;
	}

	if (!GRenderer->BeginScene())
	{
		return true;
	}

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

			ITC.Set("playerbook", "graph/interface/book/character_sheet/char_sheet_book.bmp");
			ITC.Set("ic_casting", "graph/interface/book/character_sheet/buttons_carac/icone_casting.bmp");
			ITC.Set("ic_close_combat", "graph/interface/book/character_sheet/buttons_carac/icone_close_combat.bmp");
			ITC.Set("ic_constitution", "graph/interface/book/character_sheet/buttons_carac/icone_constit.bmp");
			ITC.Set("ic_defense", "graph/interface/book/character_sheet/buttons_carac/icone_defense.bmp");
			ITC.Set("ic_dexterity", "graph/interface/book/character_sheet/buttons_carac/icone_dext.bmp");
			ITC.Set("ic_etheral_link", "graph/interface/book/character_sheet/buttons_carac/icone_etheral_link.bmp");
			ITC.Set("ic_mind", "graph/interface/book/character_sheet/buttons_carac/icone_intel.bmp");
			ITC.Set("ic_intuition", "graph/interface/book/character_sheet/buttons_carac/icone_intuition.bmp");
			ITC.Set("ic_mecanism", "graph/interface/book/character_sheet/buttons_carac/icone_mecanism.bmp");
			ITC.Set("ic_object_knowledge", "graph/interface/book/character_sheet/buttons_carac/icone_obj_knowledge.bmp");
			ITC.Set("ic_projectile", "graph/interface/book/character_sheet/buttons_carac/icone_projectile.bmp");
			ITC.Set("ic_stealth", "graph/interface/book/character_sheet/buttons_carac/icone_stealth.bmp");
			ITC.Set("ic_strength", "graph/interface/book/character_sheet/buttons_carac/icone_strenght.bmp");

			ITC.Set("questbook", "graph/interface/book/questbook.bmp");
			ITC.Set("ptexspellbook", "graph/interface/book/spellbook.bmp");
			ITC.Set("bookmark_char", "graph/interface/book/bookmark_char.bmp");
			ITC.Set("bookmark_magic", "graph/interface/book/bookmark_magic.bmp");
			ITC.Set("bookmark_map", "graph/interface/book/bookmark_map.bmp");
			ITC.Set("bookmark_quest", "graph/interface/book/bookmark_quest.bmp");

			ITC.Set("accessible_1", "graph/interface/book/accessible/accessible_1.bmp");
			ITC.Set("accessible_2", "graph/interface/book/accessible/accessible_2.bmp");
			ITC.Set("accessible_3", "graph/interface/book/accessible/accessible_3.bmp");
			ITC.Set("accessible_4", "graph/interface/book/accessible/accessible_4.bmp");
			ITC.Set("accessible_5", "graph/interface/book/accessible/accessible_5.bmp");
			ITC.Set("accessible_6", "graph/interface/book/accessible/accessible_6.bmp");
			ITC.Set("accessible_7", "graph/interface/book/accessible/accessible_7.bmp");
			ITC.Set("accessible_8", "graph/interface/book/accessible/accessible_8.bmp");
			ITC.Set("accessible_9", "graph/interface/book/accessible/accessible_9.bmp");
			ITC.Set("accessible_10", "graph/interface/book/accessible/accessible_10.bmp");
			ITC.Set("current_1", "graph/interface/book/current_page/current_1.bmp");
			ITC.Set("current_2", "graph/interface/book/current_page/current_2.bmp");
			ITC.Set("current_3", "graph/interface/book/current_page/current_3.bmp");
			ITC.Set("current_4", "graph/interface/book/current_page/current_4.bmp");
			ITC.Set("current_5", "graph/interface/book/current_page/current_5.bmp");
			ITC.Set("current_6", "graph/interface/book/current_page/current_6.bmp");
			ITC.Set("current_7", "graph/interface/book/current_page/current_7.bmp");
			ITC.Set("current_8", "graph/interface/book/current_page/current_8.bmp");
			ITC.Set("current_9", "graph/interface/book/current_page/current_9.bmp");
			ITC.Set("current_10", "graph/interface/book/current_page/current_10.bmp");

			ITC.Set("ptexcursorredist", "graph/interface/cursors/add_points.bmp");

			ITC.Set("ptexcornerleft", "graph/interface/book/left_corner_original.bmp");
			ITC.Set("ptexcornerright", "graph/interface/book/right_corner_original.bmp");

			ITC.Level = getLocalised("system_charsheet_player_lvl");
			ITC.Xp = getLocalised("system_charsheet_player_xp");

			ANIM_Set(&player.useanim, herowaitbook);

			player.useanim.flags |= EA_LOOP;

			ARXOldTimeMenu = ARXTimeMenu = ARX_TIME_Get();
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
					ARX_CHECK_INT(fRandom);

					int t	= ARX_CLEAN_WARN_CAST_INT(fRandom);


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


					ARX_CHECK_CHAR(iSkin);
					player.skin = ARX_CLEAN_WARN_CAST_CHAR(iSkin);

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

	DynLight[0].pos.x = 0.f + EERIEMouseX - (DANAESIZX >> 1);
	DynLight[0].pos.y = 0.f + EERIEMouseY - (DANAESIZY >> 1);

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
					ARX_TIME_UnPause();
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
