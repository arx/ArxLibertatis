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

#include "gui/MenuPublic.h"

#include <stddef.h>
#include <iomanip>
#include <cstdlib>
#include <sstream>
#include <string>

#include "animation/Animation.h"
#include "animation/AnimationRender.h"

#include "core/Application.h"
#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"
#include "core/SaveGame.h"

#include "game/Player.h"

#include "gui/Interface.h"
#include "gui/Menu.h"
#include "gui/MenuWidgets.h"

#include "graphics/Math.h"
#include "graphics/Renderer.h"

#include "input/Input.h"

#include "io/fs/Filesystem.h"
#include "io/resource/ResourcePath.h"
#include "io/log/Logger.h"

#include "math/Types.h"
#include "math/Vector.h"

#include "scene/ChangeLevel.h"
#include "scene/GameSound.h"
#include "scene/LoadLevel.h"
#include "scene/Light.h"

#include "window/RenderWindow.h"

extern bool bQuickGenFirstClick;

extern Rect g_size;

extern long LOADQUEST_SLOT;

extern long REFUSE_GAME_RETURN;

extern bool bFade;
extern bool	bFadeInOut;
extern int iFadeAction;

extern long ZMAPMODE;

//-----------------------------------------------------------------------------
void ARXMenu_Private_Options_Video_SetResolution(bool fullscreen, int _iWidth, int _iHeight) {
	
	if(!GRenderer) {
		return;
	}
	
	config.video.resolution = Vec2i(_iWidth, _iHeight);
	
	if(!fullscreen) {
		if(config.video.resolution == Vec2i_ZERO) {
			LogInfo << "Configuring automatic fullscreen resolution selection";
		} else {
			LogInfo << "Configuring fullscreen resolution to " << DisplayMode(config.video.resolution);
		}
	}
	
	RenderWindow * window = mainApp->getWindow();
	
	if(window->isFullScreen() != fullscreen || fullscreen) {
		
		GRenderer->Clear(Renderer::ColorBuffer | Renderer::DepthBuffer);
		GRenderer->EndScene();
		
		mainApp->getWindow()->showFrame();
		
		mainApp->setWindowSize(fullscreen);
		
		GRenderer->BeginScene();
		
	}
}

void ARXMenu_Options_Video_SetFogDistance(int _iFog) {
	config.video.fogDistance = clamp(_iFog, 0, 10);
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_SetDetailsQuality(int _iQuality)
{
	if (_iQuality > 3) _iQuality = 2;

	if (_iQuality < 0) _iQuality = 0;

	config.video.levelOfDetail = _iQuality;

	switch (config.video.levelOfDetail)
	{
		case 0:
			ZMAPMODE = 0;
			MAX_LLIGHTS = 6;
			break;
		case 1:
			ZMAPMODE = 1;
			MAX_LLIGHTS = 10;
			break;
		case 2:
			ZMAPMODE = 1;
			MAX_LLIGHTS = 15;
			break;
	}
}

//OPTIONS AUDIO

void ARXMenu_Options_Audio_SetMasterVolume(int _iVolume) {
	if (_iVolume > 10) _iVolume = 10;
	else if (_iVolume < 0) _iVolume = 0;
	float fVolume = ((float)_iVolume) * 0.1f;
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerMenu, fVolume);
	config.audio.volume = _iVolume;
}

void ARXMenu_Options_Audio_SetSfxVolume(int _iVolume) {
	if (_iVolume > 10) _iVolume = 10;
	else if (_iVolume < 0) _iVolume = 0;
	float fVolume = ((float)_iVolume) * 0.1f;
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerMenuSample, fVolume);
	config.audio.sfxVolume = _iVolume;
}

void ARXMenu_Options_Audio_SetSpeechVolume(int _iVolume) {
	if (_iVolume > 10) _iVolume = 10;
	else if (_iVolume < 0) _iVolume = 0;

	float fVolume = ((float)_iVolume) * 0.1f;
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerMenuSpeech, fVolume);
	config.audio.speechVolume = _iVolume;
}

void ARXMenu_Options_Audio_SetAmbianceVolume(int _iVolume) {
	if (_iVolume > 10) _iVolume = 10;
	else if (_iVolume < 0) _iVolume = 0;

	float fVolume = ((float)_iVolume) * 0.1f;
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerMenuAmbiance, fVolume);
	config.audio.ambianceVolume = _iVolume;
}

void ARXMenu_Options_Audio_ApplyGameVolumes() {
	ARX_SOUND_MixerSwitch(ARX_SOUND_MixerMenu, ARX_SOUND_MixerGame);
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerGame, config.audio.volume * 0.1f);
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerGameSample, config.audio.sfxVolume * 0.1f);
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerGameSpeech, config.audio.speechVolume * 0.1f);
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerGameAmbiance, config.audio.ambianceVolume * 0.1f);
}

bool ARXMenu_Options_Audio_SetEAX(bool _bEnable) {
	
	config.audio.eax = _bEnable;
	
	ARX_SOUND_PushAnimSamples();
	size_t ulSizeAmbiancePlayList;
	char * pAmbiancePlayList = ARX_SOUND_AmbianceSavePlayList(ulSizeAmbiancePlayList);
	
	ARX_SOUND_Release();
	ARX_SOUND_Init();
	
	ARX_SOUND_MixerSwitch(ARX_SOUND_MixerGame, ARX_SOUND_MixerMenu);
	ARX_SOUND_PlayMenuAmbiance(AMB_MENU);
	
	ARXMenu_Options_Audio_SetMasterVolume(config.audio.volume);
	ARXMenu_Options_Audio_SetSfxVolume(config.audio.sfxVolume);
	ARXMenu_Options_Audio_SetSpeechVolume(config.audio.speechVolume);
	ARXMenu_Options_Audio_SetAmbianceVolume(config.audio.ambianceVolume);

	if(pAmbiancePlayList) {
		ARX_SOUND_AmbianceRestorePlayList(pAmbiancePlayList, ulSizeAmbiancePlayList);
		free(pAmbiancePlayList);
	}

	ARX_SOUND_PopAnimSamples();

	return config.audio.eax;
}

void ARXMenu_Options_Control_GetInvertMouse(bool & enabled) {
	enabled = (INVERTMOUSE == 1);
}

void ARXMenu_Options_Control_SetInvertMouse(bool enable) {
	INVERTMOUSE = enable ? 1 : 0;
	config.input.invertMouse = enable;
}

void ARXMenu_Options_Control_SetMouseSensitivity(int sensitivity) {
	config.input.mouseSensitivity = clamp(sensitivity, 0, 10);
	GInput->setMouseSensitivity(config.input.mouseSensitivity);
}

//-----------------------------------------------------------------------------
//RESUME GAME
//-----------------------------------------------------------------------------
bool ARXMenu_CanResumeGame() {
	return !REFUSE_GAME_RETURN;
}

//-----------------------------------------------------------------------------
void ARXMenu_ResumeGame()
{
	ARX_Menu_Resources_Release();
	arxtime.resume();
	EERIEMouseButton = 0;
}

//-----------------------------------------------------------------------------
//NEW QUEST
//-----------------------------------------------------------------------------
void ARXMenu_NewQuest()
{
	bFadeInOut = true;	//fade out
	bFade = true;			//active le fade
	iFadeAction = AMCM_NEWQUEST;	//action a la fin du fade
	bQuickGenFirstClick = true;
	player.gold = 0;
	ARX_PLAYER_MakeFreshHero();
}

//-----------------------------------------------------------------------------
//LOAD QUEST
//-----------------------------------------------------------------------------
extern float PROGRESS_BAR_TOTAL;
extern float OLD_PROGRESS_BAR_COUNT;
extern float PROGRESS_BAR_COUNT;
void ARXMenu_LoadQuest(size_t num) {
	
	LOADQUEST_SLOT = num;

	ARX_SOUND_PlayMenu(SND_MENU_CLICK);
	REFUSE_GAME_RETURN = 0;
	ARX_MENU_Clicked_QUIT();
}

//SAVE QUEST
//-----------------------------------------------------------------------------
void ARXMenu_SaveQuest(const std::string & name, size_t num) {
	
	ARX_SOUND_MixerPause(ARX_SOUND_MixerMenu);
	
	savegames.save(name, num, savegame_thumbnail);
	
	ARX_SOUND_MixerResume(ARX_SOUND_MixerMenu);
}

//CREDITS
//-----------------------------------------------------------------------------
void ARXMenu_Credits()
{
	bFadeInOut = true;	//fade out
	bFade = true;			//active le fade
	iFadeAction = AMCM_CREDITS;	//action a la fin du fade
}

//QUIT
//-----------------------------------------------------------------------------
void ARXMenu_Quit()
{

	bFadeInOut = true;		//fade out
	bFade = true;				//active le fade
	iFadeAction = AMCM_OFF;	//action a la fin du fade
}
