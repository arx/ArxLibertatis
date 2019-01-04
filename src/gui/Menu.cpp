/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "cinematic/CinematicController.h"

#include "core/Application.h"
#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"
#include "core/Localisation.h"

#include "game/Player.h"

#include "gui/CharacterCreation.h"
#include "gui/Credits.h"
#include "gui/Interface.h"
#include "gui/MainMenu.h"
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

extern bool REQUEST_SPEECH_SKIP;

ARX_MENU_DATA ARXmenu;
bool g_canResumeGame = true;

void ARX_Menu_Resources_Release() {
	
	config.save();
	
	g_characterCreation.freeData();
	
	// Synchronize game mixers with menu mixers and switch between them
	ARXMenu_Options_Audio_ApplyGameVolumes();
	
	delete g_thumbnailCursor.m_loadTexture, g_thumbnailCursor.m_loadTexture = NULL;
}

void ARX_MENU_Clicked_QUIT() {
	
	arx_assert(g_canResumeGame);
	
	ARX_Menu_Resources_Release();
	ARXmenu.requestMode(Mode_InGame);
}

void ARX_MENU_Clicked_NEWQUEST() {
	
	g_canResumeGame = false;
	
	ARX_PLAYER_Start_New_Quest();
	g_playerBook.forcePage(BOOKMODE_STATS);
	player.skin = 0;
	ARX_PLAYER_Restore_Skin();
	ARXmenu.requestMode(Mode_CharacterCreation);
}

void ARX_MENU_Clicked_CREDITS() {
	ARXmenu.requestMode(Mode_Credits);
	credits::reset();
	ARX_SOUND_PlayMenuAmbiance(AMB_CREDITS);
}

void ARX_MENU_Launch(bool allowResume) {
	
	g_canResumeGame = allowResume;

	g_gameTime.pause(GameTime::PauseMenu);

	// Synchronize menu mixers with game mixers and switch between them
	ARX_SOUND_MixerPause(ARX_SOUND_MixerGame);
	ARX_SOUND_MixerResume(ARX_SOUND_MixerMenu);

	ARX_SOUND_PlayMenuAmbiance(AMB_MENU);
	ARX_SOUND_PlayMenu(g_snd.MENU_CLICK);

	ARXmenu.requestMode(Mode_MainMenu);
	
	g_playerBook.stats.loadStrings();
	g_characterCreation.loadData();
	
	if(pMenuCursor) {
		pMenuCursor->reset();
	}
}

void ARX_Menu_Manage() {
	
	// looks for keys for each mode.
	switch(ARXmenu.mode()) {
		case Mode_InGame: {
			// Checks for ESC key
			if(GInput->isKeyPressedNowUnPressed(Keyboard::Key_Escape)) {
				if(isInCinematic()) {
					cinematicEnd();
				} else if(cinematicBorder.isActive()) {
					// Disabling ESC capture while fading in or out.
					if(!FADEDIR && cinematicBorder.elapsedTime() >= GameDurationMs(3000)) {
						if(SendMsgToAllIO(NULL, SM_KEY_PRESSED) != REFUSE) {
							REQUEST_SPEECH_SKIP = true;
						}
					}
				} else if((player.Interface & INTER_PLAYERBOOK) || g_note.isOpen()) {
					g_playerBook.close();
					ARX_INTERFACE_NoteClose();
				} else {
					GRenderer->getSnapshot(savegame_thumbnail, config.interface.thumbnailSize.x, config.interface.thumbnailSize.y);
					
					g_gameTime.pause(GameTime::PauseMenu);
					
					ARX_MENU_Launch(true);
					MenuFader_start(Fade_Out, -1); // TODO: does this fader even work ?
					TRUE_PLAYER_MOUSELOOK_ON = false;

					ARX_PLAYER_PutPlayerInNormalStance();
				}
			}
			break;
		}
		case Mode_CharacterCreation: {
			if(   GInput->isKeyPressedNowUnPressed(Keyboard::Key_Escape)
			   && bFadeInOut == Fade_Out // TODO: comment seems incorrect -> // XS: Disabling ESC capture while fading in or out.
			) {
				ARX_SOUND_PlayMenu(g_snd.MENU_CLICK);
				ARXmenu.requestMode(Mode_MainMenu);
			}
			break;
		}
		case Mode_MainMenu: {
			if(   GInput->isKeyPressedNowUnPressed(Keyboard::Key_Escape)
			   && MENU_NoActiveWindow()
			   && g_canResumeGame
			) {
				g_gameTime.resume(GameTime::PauseMenu);
				ARX_MENU_Clicked_QUIT();
			}
			break;
		}
		case Mode_Credits: {
			if(   GInput->isKeyPressedNowUnPressed(Keyboard::Key_Escape)
			   || GInput->isKeyPressedNowUnPressed(Keyboard::Key_Spacebar)
			) {
				ARX_SOUND_PlayMenu(g_snd.MENU_CLICK);
				MenuFader_start(Fade_In, Mode_MainMenu);
				ARX_SOUND_PlayMenuAmbiance(AMB_MENU);
			}
			break;
		}
		default:
			break;
	}
}

void ARX_Menu_Render() {
	
	switch(ARXmenu.mode()) {
		case Mode_InGame: arx_unreachable();
		case Mode_CharacterCreation: {
			g_characterCreation.render();
			return;
		}
		case Mode_Credits: {
			credits::render();
			return;
		}
		case Mode_MainMenu: {
			MainMenuDoFrame();
			return;
		}
	}
}
