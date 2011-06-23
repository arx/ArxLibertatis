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

#include "gui/MenuPublic.h"

#include <cstdio>

#include "animation/Cinematic.h"

#include "core/Config.h"
#include "core/Core.h"
#include "core/Localisation.h"
#include "core/GameTime.h"

#include "game/Player.h"

#include "gui/Menu.h"
#include "gui/Interface.h"
#include "gui/Menu.h"
#include "gui/MenuWidgets.h"

#include "graphics/Math.h"
#include "graphics/Draw.h"
#include "graphics/GraphicsEnum.h"
#include "graphics/Frame.h"
#include "graphics/data/CinematicTexture.h"

#include "io/Filesystem.h"
#include "io/Logger.h"

#include "scene/GameSound.h"

extern CDirectInput * pGetInfoDirectInput;
extern bool bQuickGenFirstClick;
#ifdef BUILD_EDITOR
extern float FORCED_REDUCTION_VALUE;
#endif
extern long DANAESIZX;
extern long DANAESIZY;
extern long STARTED_A_GAME;
extern long WILL_RELOAD_ALL_TEXTURES;

extern long REFUSE_GAME_RETURN;
extern Cinematic * ControlCinematique;

bool bForceReInitAllTexture = false;
extern long CAN_REPLAY_INTRO;
extern bool bFade;
extern bool	bFadeInOut;
extern int iFadeAction;

extern long ZMAPMODE;
extern long MAX_LLIGHTS;
extern long FRAME_COUNT;

void		*	pAmbiancePlayList = NULL;
unsigned long	ulSizeAmbiancePlayList = 0;

void ARX_SOUND_PushAnimSamples();
void ARX_SOUND_PopAnimSamples();

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_GetResolution(int & _iWidth, int & _iHeight, int & _iBpp)
{
	_iWidth		= DANAESIZX;
	_iHeight	= DANAESIZY;
	_iBpp		= danaeApp.m_pFramework->bitdepth;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_GetBitPlane(int & _iBpp)
{
	_iBpp		= danaeApp.m_pFramework->bitdepth;
}

//-----------------------------------------------------------------------------
void ARXMenu_Private_Options_Video_SetResolution(int _iWidth, int _iHeight, int _iBpp)
{
	if (!GDevice) return;

	GRenderer->Clear(Renderer::ColorBuffer | Renderer::DepthBuffer);
	GRenderer->EndScene();

	danaeApp.m_pFramework->ShowFrame();

	ARX_Text_Close();

	HRESULT hr;
	Project.bits = _iBpp;
	danaeApp.m_pFramework->bitdepth = _iBpp;
	danaeApp.m_pFramework->m_dwRenderHeight = _iHeight;
	danaeApp.m_pFramework->m_dwRenderWidth = _iWidth;

	if (danaeApp.m_pDeviceInfo->bWindowed)
	{
		RECT rRect;
		RECT rRect2;
		GetClientRect(danaeApp.m_hWnd, &rRect);
		GetWindowRect(danaeApp.m_hWnd, &rRect2);
		int dx = (rRect2.right - rRect2.left) - (rRect.right - rRect.left);
		int dy = (rRect2.bottom - rRect2.top) - (rRect.bottom - rRect.top);
		SetWindowPos(danaeApp.m_hWnd,
			            HWND_TOP,
			            rRect2.left,
			            rRect2.top,
			            _iWidth + dx,
			            _iHeight + dy,
			            SWP_SHOWWINDOW);
	}
	else
	{
		int nb = danaeApp.m_pDeviceInfo->dwNumModes;

		for (int i = 0; i < nb; i++)
		{

			ARX_CHECK_NOT_NEG(_iBpp);

			if (danaeApp.m_pDeviceInfo->pddsdModes[i].ddpfPixelFormat.dwRGBBitCount == ARX_CAST_UINT(_iBpp))
			{
				ARX_CHECK_NOT_NEG(_iWidth);
				ARX_CHECK_NOT_NEG(_iHeight);

				if ((danaeApp.m_pDeviceInfo->pddsdModes[i].dwWidth == ARX_CAST_UINT(_iWidth)) &&
					    (danaeApp.m_pDeviceInfo->pddsdModes[i].dwHeight == ARX_CAST_UINT(_iHeight)))
				{

					danaeApp.m_pDeviceInfo->ddsdFullscreenMode = danaeApp.m_pDeviceInfo->pddsdModes[i];
					danaeApp.m_pDeviceInfo->dwCurrentMode = i;
				}
			}
		}
	}

	if (FAILED(hr = danaeApp.Change3DEnvironment()))
		LogError << "Error Changing Environment";

	AdjustUI();

	GRenderer->BeginScene();
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_GetFullscreen(bool & _bEnable)
{
	if (danaeApp.m_pDeviceInfo->bWindowed) _bEnable = false;
	else _bEnable = true;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_SetFullscreen(bool _bEnable) {
	DanaeSwitchFullScreen();
	config.video.fullscreen = _bEnable;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_GetFogDistance(int & _iFog)
{
	_iFog = config.video.fogDistance;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_SetFogDistance(int _iFog)
{
	if (_iFog > 10) _iFog = 10;

	if (_iFog < 0) _iFog = 0;

	config.video.fogDistance = _iFog;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_GetTextureQuality(int & _iQuality)
{
	if (Project.TextureSize == 0) _iQuality = 2;

	if (Project.TextureSize == 2) _iQuality = 1;

	if (Project.TextureSize == 64) _iQuality = 0;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_SetTextureQuality(int _iQuality)
{
	if (_iQuality > 2) _iQuality = 2;

	if (_iQuality < 0) _iQuality = 0;

	newTextureSize = _iQuality;
}

void SetGammaLumContrast()
{

	float fGammaMax = (1.f / 6.f);
	float fGammaMin = 2.f;

	float fPuissance = ((fGammaMax - fGammaMin) / 11.f) * ((float)(config.video.gamma + 1)) + fGammaMin;

	float fLuminosityMin = -.2f;
	float fLuminosityMax = .2f;
	float fLuminosity = ((fLuminosityMax - fLuminosityMin) / 11.f) * ((float)(config.video.luminosity + 1)) + fLuminosityMin;

	float fContrastMax = -.3f;
	float fContrastMin = .3f;
	float fContrast = ((fContrastMax - fContrastMin) / 11.f) * ((float)(config.video.contrast + 1)) + fContrastMin;

	float fRangeMin = 0.f + fContrast;
	float fRangeMax = 1.f - fContrast;
	float fdVal = (fRangeMax - fRangeMin) / 256.f;
	float fVal = 0.f;

	for (int iI = 0; iI < 256; iI++)
	{
		int iColor = (int)(65536.f * (fLuminosity + pow(fVal, fPuissance)));

		if (iColor < 0)
		{
			iColor = 0;
		}
		else
		{
			if (iColor > 65535)
			{
				iColor = 65535;
			}
		}


		ARX_CHECK_WORD(iColor);
		WORD wColor = static_cast<u16>(iColor);

		danaeApp.DDGammaRamp.red[iI] = wColor;
		danaeApp.DDGammaRamp.green[iI] = wColor;
		danaeApp.DDGammaRamp.blue[iI] = wColor;


		fVal += fdVal;
	}

	danaeApp.UpdateGamma();
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_GetGamma(int & _iGamma)
{
	_iGamma = config.video.gamma;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_SetGamma(int _iGamma)
{
	config.video.gamma = _iGamma;
	SetGammaLumContrast();
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_GetLuminosity(int & _iLuminosity)
{
	_iLuminosity = config.video.luminosity;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_SetLuminosity(int _iLuminosity)
{
	config.video.luminosity = _iLuminosity;
	SetGammaLumContrast();
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_GetContrast(int & _iContrast)
{
	_iContrast = config.video.contrast;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_SetContrast(int _iContrast)
{
	config.video.contrast = _iContrast;
	SetGammaLumContrast();
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_GetDetailsQuality(int & _iQuality)
{
	_iQuality = config.video.levelOfDetail;
}
extern long MAX_FRAME_COUNT;
extern long USEINTERNORM;
extern long DYNAMIC_NORMALS;
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
			MAX_FRAME_COUNT = 3;
			USEINTERNORM = 1; 
			DYNAMIC_NORMALS = 1; 
			break;
		case 1:
			ZMAPMODE = 1;
			MAX_LLIGHTS = 10; 
			MAX_FRAME_COUNT = 2;
			USEINTERNORM = 1; 
			DYNAMIC_NORMALS = 1; 
			break;
		case 2:
			ZMAPMODE = 1;
			MAX_LLIGHTS = 15; 
			MAX_FRAME_COUNT = 1;
			USEINTERNORM = 1; 
			DYNAMIC_NORMALS = 1; 
			break;
	}
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_GetLODQuality(int & _iQuality)
{

#ifdef BUILD_EDITOR
	float fForced = FORCED_REDUCTION_VALUE * ( 1.0f / 5000 ) ;
	ARX_CHECK_INT(fForced);

	config.video.meshReduction = ARX_CLEAN_WARN_CAST_INT(fForced);
#endif

	_iQuality = config.video.meshReduction;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_SetLODQuality(int _iQuality)
{
	if (_iQuality > 2) _iQuality = 2;
	else if (_iQuality < 0) _iQuality = 0;

	config.video.meshReduction = _iQuality;
#ifdef BUILD_EDITOR
	FORCED_REDUCTION_VALUE = ARX_CLEAN_WARN_CAST_FLOAT(_iQuality * 5000);
#endif
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
	
	int iOldGamma;
	ARXMenu_Options_Video_GetGamma(iOldGamma);
	ARXMenu_Options_Video_SetGamma((iOldGamma - 1) < 0 ? 0 : (iOldGamma - 1));
	
	config.audio.eax = _bEnable;
	
	ARX_SOUND_PushAnimSamples();
	ARX_SOUND_AmbianceSavePlayList(&pAmbiancePlayList, &ulSizeAmbiancePlayList);
	
	ARX_SOUND_Release();
	ARX_SOUND_Init();
	
	ARX_SOUND_MixerSwitch(ARX_SOUND_MixerGame, ARX_SOUND_MixerMenu);
	ARX_SOUND_PlayMenuAmbiance(AMB_MENU);
	
	ARXMenu_Options_Audio_SetMasterVolume(config.audio.volume);
	ARXMenu_Options_Audio_SetSfxVolume(config.audio.sfxVolume);
	ARXMenu_Options_Audio_SetSpeechVolume(config.audio.speechVolume);
	ARXMenu_Options_Audio_SetAmbianceVolume(config.audio.ambianceVolume);

	if (pAmbiancePlayList)
	{
		ARX_SOUND_AmbianceRestorePlayList(pAmbiancePlayList, ulSizeAmbiancePlayList);
		free((void *)pAmbiancePlayList);
	}

	ARX_SOUND_PopAnimSamples();

	ARXMenu_Options_Video_SetGamma(iOldGamma);

	return config.audio.eax;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Control_GetInvertMouse(bool & _bEnable)
{
	if (INVERTMOUSE == 1) _bEnable = true;
	else _bEnable = false;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Control_GetAutoReadyWeapon(bool & _bEnable)
{
	if (config.input.autoReadyWeapon == 1) _bEnable = true;
	else _bEnable = false;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Control_SetInvertMouse(bool _bEnable)
{
	if (_bEnable)
		INVERTMOUSE = 1;
	else INVERTMOUSE = 0;

	config.input.invertMouse = _bEnable;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Control_SetAutoReadyWeapon(bool _bEnable)
{
	if (_bEnable)
		config.input.autoReadyWeapon = 1;
	else config.input.autoReadyWeapon = 0;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Control_GetMouseLookToggleMode(bool & _bEnable)
{
	_bEnable = config.input.mouseLookToggle;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Control_SetMouseLookToggleMode(bool _bEnable)
{
	config.input.mouseLookToggle = _bEnable;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Control_GetMouseSensitivity(int & _iSensitivity)
{
	_iSensitivity = config.input.mouseSensitivity;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Control_SetMouseSensitivity(int _iSensitivity)
{
	if (_iSensitivity < 0)_iSensitivity = 0;
	else if (_iSensitivity > 10)_iSensitivity = 10;

	config.input.mouseSensitivity = _iSensitivity;
	pGetInfoDirectInput->SetSensibility(_iSensitivity);

	danaeApp.fMouseSensibility = ((float)config.input.mouseSensitivity) / 10.f;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Control_GetMouseSmoothing(bool & _bSmoothing)
{
	_bSmoothing = config.input.mouseSmoothing;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Control_SetMouseSmoothing(bool _bSmoothing)
{
	config.input.mouseSmoothing = _bSmoothing;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Control_GetAutoDescription(bool & _bEnable)
{
	_bEnable = config.input.autoDescription;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Control_SetAutoDescription(bool _bEnable)
{
	config.input.autoDescription = _bEnable;
}

//-----------------------------------------------------------------------------
//RESUME GAME
//-----------------------------------------------------------------------------
void ARXMenu_GetResumeGame(bool & _bEnable)
{
	if (REFUSE_GAME_RETURN) _bEnable = false;
	else _bEnable = true;
}

//-----------------------------------------------------------------------------
void ARXMenu_ResumeGame()
{
	ARX_Menu_Resources_Release();
	ARX_TIME_UnPause();
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
void ARXMenu_LoadQuest(long num)
{
	CAN_REPLAY_INTRO = 0;
	GRenderer->EndScene();

	ARX_SOUND_MixerPause(ARX_SOUND_MixerMenu);
	LoadSaveGame(num); 
	GRenderer->BeginScene();
}

//-----------------------------------------------------------------------------
void ARXMenu_DeleteQuest(long num)
{
	if (num != 0)
	{
		char temp[256];

		sprintf(temp, "Save%s\\save%04ld\\", LOCAL_SAVENAME, save_l[num].num);
		KillAllDirectory(temp);
		CreateSaveGameList();
	}
}

//SAVE QUEST
//-----------------------------------------------------------------------------
void ARXMenu_SaveQuest(long num)
{
	int iOldGamma;

	ARX_SOUND_MixerPause(ARX_SOUND_MixerMenu);
	ARXMenu_Options_Video_GetGamma(iOldGamma);
	ARXMenu_Options_Video_SetGamma((iOldGamma - 1) < 0 ? 0 : (iOldGamma - 1));
	UpdateSaveGame(num);
	ARXMenu_Options_Video_SetGamma(iOldGamma);
	ARX_SOUND_MixerResume(ARX_SOUND_MixerMenu);

	CreateSaveGameList();
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
