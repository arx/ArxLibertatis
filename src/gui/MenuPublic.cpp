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

#include "gui/MenuPublic.h"

#include <stddef.h>
#include <iomanip>
#include <cstdlib>
#include <sstream>
#include <string>

#include "animation/Animation.h"
#include "animation/AnimationRender.h"

#include "core/Application.h"
#include "core/ArxGame.h"
#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"
#include "core/SaveGame.h"

#include "game/Player.h"

#include "gui/Interface.h"
#include "gui/Menu.h"
#include "gui/MenuWidgets.h"
#include "gui/menu/MenuFader.h"

#include "graphics/Math.h"
#include "graphics/Renderer.h"

#include "input/Input.h"

#include "io/fs/Filesystem.h"
#include "io/resource/ResourcePath.h"
#include "io/log/Logger.h"

#include "math/Types.h"
#include "math/Vector.h"

#include "scene/GameSound.h"
#include "scene/LoadLevel.h"
#include "scene/Light.h"

#include "window/RenderWindow.h"

extern bool bQuickGenFirstClick;

void ARXMenu_Private_Options_Video_SetResolution(bool fullscreen, int _iWidth, int _iHeight) {
	
	if(!GRenderer) {
		return;
	}
	
	config.video.resolution = Vec2i(_iWidth, _iHeight);
	
	if(!fullscreen) {
		if(config.video.resolution == Vec2i(0)) {
			LogInfo << "Configuring automatic fullscreen resolution selection";
		} else {
			LogInfo << "Configuring fullscreen resolution to " << DisplayMode(config.video.resolution);
		}
	}
	
	RenderWindow * window = mainApp->getWindow();
	
	if(window->isFullScreen() != fullscreen || fullscreen) {
		
		GRenderer->Clear(Renderer::ColorBuffer);
		
		mainApp->getWindow()->showFrame();
		
		mainApp->setWindowSize(fullscreen);
		
	}
}

void ARXMenu_Options_Video_SetFogDistance(float distance) {
	config.video.fogDistance = glm::clamp(distance, 0.f, 10.f);
}

void ARXMenu_Options_Video_SetDetailsQuality(int lod) {
	
	config.video.levelOfDetail = glm::clamp(lod, 0, 2);
	
	switch(config.video.levelOfDetail) {
		case 0: {
			setMaxLLights(6);
			break;
		}
		case 1: {
			setMaxLLights(10);
			break;
		}
		case 2: {
			setMaxLLights(15);
			break;
		}
	}
	
}

void ARXMenu_Options_Video_SetGamma(float gamma) {
	config.video.gamma = glm::clamp(gamma, 0.f, 10.f);
	mainApp->getWindow()->setGamma(1.f + (gamma / 5.f - 1.f) * 0.5f);
}

void ARXMenu_Options_Audio_SetMasterVolume(float volume) {
	
	config.audio.volume = glm::clamp(volume, 0.f, 10.f);
	
	float fVolume = config.audio.volume * 0.1f;
	if(config.audio.muteOnFocusLost && !mainApp->getWindow()->hasFocus()) {
		fVolume = 0.f;
	}
	
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerMenu, fVolume);
}

void ARXMenu_Options_Audio_SetSfxVolume(float volume) {
	
	config.audio.sfxVolume = glm::clamp(volume, 0.f, 10.f);
	
	float fVolume = config.audio.sfxVolume * 0.1f;
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerMenuSample, fVolume);
}

void ARXMenu_Options_Audio_SetSpeechVolume(float volume) {
	
	config.audio.speechVolume = glm::clamp(volume, 0.f, 10.f);
	
	float fVolume = config.audio.speechVolume * 0.1f;
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerMenuSpeech, fVolume);
}

void ARXMenu_Options_Audio_SetAmbianceVolume(float volume) {
	
	config.audio.ambianceVolume = glm::clamp(volume, 0.f, 10.f);
	
	float fVolume = config.audio.ambianceVolume * 0.1f;
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerMenuAmbiance, fVolume);
}

void ARXMenu_Options_Audio_ApplyGameVolumes() {
	
	ARX_SOUND_MixerPause(ARX_SOUND_MixerMenu);
	ARX_SOUND_MixerResume(ARX_SOUND_MixerGame);
	
	float volume = config.audio.volume * 0.1f;
	if(config.audio.muteOnFocusLost && !mainApp->getWindow()->hasFocus()) {
		volume = 0.f;
	}
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerGame, volume);
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerGameSample, config.audio.sfxVolume * 0.1f);
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerGameSpeech, config.audio.speechVolume * 0.1f);
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerGameAmbiance, config.audio.ambianceVolume * 0.1f);
}

void ARXMenu_Options_Audio_SetMuted(bool mute) {
	float volume = mute ? 0.f : config.audio.volume * 0.1f;
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerMenu, volume);
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerGame, volume);
}

void ARXMenu_Options_Audio_SetDevice(const std::string & device) {
	
	config.audio.device = device;
	
	/*
	 * TODO This is ugly and doesn't save all currently playing samples - only looping ones,
	 * and those aren't restored at the same playback position. Ideally the audio subsystem
	 * should be able to switch backends internally.
	 */
	
	std::vector< std::pair<res::path, size_t> > animationSamples = ARX_SOUND_PushAnimSamples();
	
	std::string playlist = ARX_SOUND_AmbianceSavePlayList();
	
	ARX_SOUND_Release();
	ARX_SOUND_Init();
	
	ARX_SOUND_MixerPause(ARX_SOUND_MixerGame);
	ARX_SOUND_MixerResume(ARX_SOUND_MixerMenu);
	
	ARX_SOUND_PlayMenuAmbiance(AMB_MENU);
	
	ARXMenu_Options_Audio_SetMasterVolume(config.audio.volume);
	ARXMenu_Options_Audio_SetSfxVolume(config.audio.sfxVolume);
	ARXMenu_Options_Audio_SetSpeechVolume(config.audio.speechVolume);
	ARXMenu_Options_Audio_SetAmbianceVolume(config.audio.ambianceVolume);
	
	ARX_SOUND_AmbianceRestorePlayList(playlist.data(), playlist.size());
	
	ARX_SOUND_PopAnimSamples(animationSamples);
}

void ARXMenu_ResumeGame() {
	ARX_Menu_Resources_Release();
	g_gameTime.resume(GameTime::PauseMenu);
	EERIEMouseButton = 0;
	ARXmenu.requestMode(Mode_InGame);
}

void ARXMenu_NewQuest() {
	MenuFader_start(Fade_In, Mode_CharacterCreation);
	bQuickGenFirstClick = true;
	player.gold = 0;
	ARX_PLAYER_MakeFreshHero();
}
