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
#include "Danae.h"
#include "ARX_MenuPublic.h"
#include "ARX_Menu.h"
#include "ARX_Interface.h"
#include "ARX_sound.h"
#include "ARX_Menu.h"
#include "ARX_Menu2.h"
#include "ARX_loc.h"
#include "ARX_Time.h"
#include "EERIEmath.h"
#include "EERIEdraw.h"

#include "hermesmain.h"
#include "arx_c_cinematique.h"

#include <stdio.h>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

extern CMenuConfig * pMenuConfig;
extern CDirectInput * pGetInfoDirectInput;
extern LPDIRECT3DDEVICE7 GDevice;
extern bool bQuickGenFirstClick;
extern float FORCED_REDUCTION_VALUE;
extern long DANAESIZX;
extern long DANAESIZY;
extern long STARTED_A_GAME;
extern long WILL_RELOAD_ALL_TEXTURES;
extern long GAME_EDITOR;

extern long REFUSE_GAME_RETURN;
extern CINEMATIQUE * ControlCinematique;

bool bForceReInitAllTexture = false;
extern long CAN_REPLAY_INTRO;
extern bool bFade;
extern bool	bFadeInOut;
extern int iFadeAction;

extern long ZMAPMODE;
extern CMY_DYNAMIC_VERTEXBUFFER * pDynamicVertexBuffer;
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

extern float fInterfaceRatio;

//-----------------------------------------------------------------------------
void ARXMenu_Private_Options_Video_SetResolution(int _iWidth, int _iHeight, int _iBpp)
{
	if (!GDevice) return;

	GDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DRGBA(0, 0, 0, 0), 1.0f, 0L);
	danaeApp.DANAEEndRender();

	danaeApp.m_pFramework->ShowFrame();

	ARX_Text_Close();

	fInterfaceRatio = 1;

	HRESULT hr;
	Project.bits = _iBpp;
	danaeApp.m_pFramework->bitdepth = _iBpp;
	danaeApp.m_pFramework->m_dwRenderHeight = _iHeight;
	danaeApp.m_pFramework->m_dwRenderWidth = _iWidth;

	{
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
		{
			ShowPopup("Error Changing Environment");
		}
		else
		{
			GDevice = danaeApp.m_pFramework->GetD3DDevice();
			DANAESIZX = danaeApp.m_pFramework->m_dwRenderWidth;
			DANAESIZY = danaeApp.m_pFramework->m_dwRenderHeight;

			if (danaeApp.m_pDeviceInfo->bWindowed)
				DANAESIZY -= danaeApp.m_pFramework->Ystart;

			DANAECENTERX = DANAESIZX >> 1;
			DANAECENTERY = DANAESIZY >> 1;

			Xratio = DANAESIZX * DIV640;
			Yratio = DANAESIZY * DIV480;

			ARXMenu_Options_Video_SetGamma(pMenuConfig->iGamma);
			pMenuConfig->bNoReturnToWindows = true;
		}
	}

	GDevice = danaeApp.m_pd3dDevice;

	ARX_Text_Init();

	if (ControlCinematique)
	{
		ControlCinematique->m_pd3dDevice = GDevice;
		ActiveAllTexture(ControlCinematique);
	}

	danaeApp.DANAEStartRender();
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_GetFullscreen(bool & _bEnable)
{
	if (danaeApp.m_pDeviceInfo->bWindowed) _bEnable = false;
	else _bEnable = true;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_SetFullscreen(bool _bEnable)
{
	DanaeSwitchFullScreen();
	pMenuConfig->bFullScreen = _bEnable;

	if (ControlCinematique)
	{
		ControlCinematique->m_pd3dDevice = GDevice;
		ActiveAllTexture(ControlCinematique);
	}
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_GetFogDistance(int & _iFog)
{
	_iFog = pMenuConfig->iFogDistance;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_SetFogDistance(int _iFog)
{
	if (_iFog > 10) _iFog = 10;

	if (_iFog < 0) _iFog = 0;

	pMenuConfig->iFogDistance = _iFog;
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

	pMenuConfig->iNewTextureResol = _iQuality;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_GetBump(bool & _bEnable)
{
	_bEnable = pMenuConfig->bBumpMapping;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_SetBump(bool _bEnable)
{
	pMenuConfig->bNewBumpMapping = _bEnable;
}

//-----------------------------------------------------------------------------
void SetGammaLumContrast()
{
	if (!pMenuConfig) return;

	float fGammaMax = (1.f / 6.f);
	float fGammaMin = 2.f;

	float fPuissance = ((fGammaMax - fGammaMin) / 11.f) * ((float)(pMenuConfig->iGamma + 1)) + fGammaMin;

	float fLuminosityMin = -.2f;
	float fLuminosityMax = .2f;
	float fLuminosity = ((fLuminosityMax - fLuminosityMin) / 11.f) * ((float)(pMenuConfig->iLuminosite + 1)) + fLuminosityMin;

	float fContrastMax = -.3f;
	float fContrastMin = .3f;
	float fContrast = ((fContrastMax - fContrastMin) / 11.f) * ((float)(pMenuConfig->iContrast + 1)) + fContrastMin;

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
		WORD wColor = ARX_CLEAN_WARN_CAST_WORD(iColor);

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
	_iGamma = pMenuConfig->iGamma;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_SetGamma(int _iGamma)
{
	pMenuConfig->iGamma = _iGamma;
	SetGammaLumContrast();
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_GetLuminosity(int & _iLuminosity)
{
	_iLuminosity = pMenuConfig->iLuminosite;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_SetLuminosity(int _iLuminosity)
{
	pMenuConfig->iLuminosite = _iLuminosity;
	SetGammaLumContrast();
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_GetContrast(int & _iContrast)
{
	_iContrast = pMenuConfig->iContrast;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_SetContrast(int _iContrast)
{
	pMenuConfig->iContrast = _iContrast;
	SetGammaLumContrast();
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_GetDetailsQuality(int & _iQuality)
{
	_iQuality = pMenuConfig->iLevelOfDetails;
}
extern long MAX_FRAME_COUNT;
extern long USEINTERNORM;
extern long DYNAMIC_NORMALS;
//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_SetDetailsQuality(int _iQuality)
{
	if (_iQuality > 3) _iQuality = 2;

	if (_iQuality < 0) _iQuality = 0;

	pMenuConfig->iLevelOfDetails = _iQuality;

	switch (pMenuConfig->iLevelOfDetails)
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

	float fForced = FORCED_REDUCTION_VALUE * DIV5000 ;
	ARX_CHECK_INT(fForced);

	pMenuConfig->iMeshReduction = ARX_CLEAN_WARN_CAST_INT(fForced);


	_iQuality = pMenuConfig->iMeshReduction;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Video_SetLODQuality(int _iQuality)
{
	if (_iQuality > 2) _iQuality = 2;
	else if (_iQuality < 0) _iQuality = 0;

	pMenuConfig->iMeshReduction = _iQuality;
	FORCED_REDUCTION_VALUE = ARX_CLEAN_WARN_CAST_FLOAT(_iQuality * 5000);
}

//-----------------------------------------------------------------------------
bool ARXMenu_Options_Video_SetSoftRender()
{
	extern bool	bSoftRender;	bSoftRender	=	( pMenuConfig && pMenuConfig->bDebugSetting ) ;
	extern bool	bGATI8500;		bGATI8500	=	bSoftRender?true:bGATI8500;
	return bSoftRender;
}

//OPTIONS AUDIO
//-----------------------------------------------------------------------------
void ARXMenu_Options_Audio_GetMasterVolume(int & _iVolume)
{
	float fVolume = 0;
	fVolume = ARX_SOUND_MixerGetVolume(ARX_SOUND_MixerMenu);
	_iVolume = (int)(fVolume * 10.f);
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Audio_SetMasterVolume(int _iVolume)
{
	if (_iVolume > 10) _iVolume = 10;
	else if (_iVolume < 0) _iVolume = 0;

	float fVolume = ((float)_iVolume) * 0.1f;
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerMenu, fVolume);
	pMenuConfig->iMasterVolume = _iVolume;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Audio_GetSfxVolume(int & _iVolume)
{
	float fVolume = 0;
	fVolume = ARX_SOUND_MixerGetVolume(ARX_SOUND_MixerMenuSample);
	_iVolume = (int)(fVolume * 10.f);
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Audio_SetSfxVolume(int _iVolume)
{
	if (_iVolume > 10) _iVolume = 10;
	else if (_iVolume < 0) _iVolume = 0;

	float fVolume = ((float)_iVolume) * 0.1f;
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerMenuSample, fVolume);
	pMenuConfig->iSFXVolume = _iVolume;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Audio_GetSpeechVolume(int & _iVolume)
{
	float fVolume = 0;
	fVolume = ARX_SOUND_MixerGetVolume(ARX_SOUND_MixerMenuSpeech);
	_iVolume = (int)(fVolume * 10.f);
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Audio_SetSpeechVolume(int _iVolume)
{
	if (_iVolume > 10) _iVolume = 10;
	else if (_iVolume < 0) _iVolume = 0;

	float fVolume = ((float)_iVolume) * 0.1f;
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerMenuSpeech, fVolume);
	pMenuConfig->iSpeechVolume = _iVolume;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Audio_GetAmbianceVolume(int & _iVolume)
{
	float fVolume = 0;
	fVolume = ARX_SOUND_MixerGetVolume(ARX_SOUND_MixerMenuAmbiance);
	_iVolume = (int)(fVolume * 10.f);
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Audio_SetAmbianceVolume(int _iVolume)
{
	if (_iVolume > 10) _iVolume = 10;
	else if (_iVolume < 0) _iVolume = 0;

	float fVolume = ((float)_iVolume) * 0.1f;
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerMenuAmbiance, fVolume);
	pMenuConfig->iAmbianceVolume = _iVolume;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Audio_GetEAX(bool & _bEnable)
{
	_bEnable = pMenuConfig->bEAX;
}

//-----------------------------------------------------------------------------
bool ARXMenu_Options_Audio_SetEAX(bool _bEnable)
{
	int iOldGamma;
	ARXMenu_Options_Video_GetGamma(iOldGamma);
	ARXMenu_Options_Video_SetGamma((iOldGamma - 1) < 0 ? 0 : (iOldGamma - 1));

	pMenuConfig->bEAX = _bEnable;

	ARX_SOUND_PushAnimSamples();
	ARX_SOUND_AmbianceSavePlayList(&pAmbiancePlayList, &ulSizeAmbiancePlayList);

	ARX_SOUND_Release();
	ARX_SOUND_Init(danaeApp.m_hWnd);
	ARX_SOUND_EnableReverb(_bEnable);

	ARX_SOUND_MixerSwitch(ARX_SOUND_MixerGame, ARX_SOUND_MixerMenu);
	ARX_SOUND_PlayMenuAmbiance(AMB_MENU);

	ARXMenu_Options_Audio_SetMasterVolume(pMenuConfig->iMasterVolume);
	ARXMenu_Options_Audio_SetSfxVolume(pMenuConfig->iSFXVolume);
	ARXMenu_Options_Audio_SetSpeechVolume(pMenuConfig->iSpeechVolume);
	ARXMenu_Options_Audio_SetAmbianceVolume(pMenuConfig->iAmbianceVolume);

	if (pAmbiancePlayList)
	{
		ARX_SOUND_AmbianceRestorePlayList(pAmbiancePlayList, ulSizeAmbiancePlayList);
		free((void *)pAmbiancePlayList);
	}

	ARX_SOUND_PopAnimSamples();

	ARXMenu_Options_Video_SetGamma(iOldGamma);

	return pMenuConfig->bEAX;
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
	if (pMenuConfig->bAutoReadyWeapon == 1) _bEnable = true;
	else _bEnable = false;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Control_SetInvertMouse(bool _bEnable)
{
	if (_bEnable)
		INVERTMOUSE = 1;
	else INVERTMOUSE = 0;

	pMenuConfig->bInvertMouse = _bEnable;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Control_SetAutoReadyWeapon(bool _bEnable)
{
	if (_bEnable)
		pMenuConfig->bAutoReadyWeapon = 1;
	else pMenuConfig->bAutoReadyWeapon = 0;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Control_GetMouseLookToggleMode(bool & _bEnable)
{
	_bEnable = pMenuConfig->bMouseLookToggle;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Control_SetMouseLookToggleMode(bool _bEnable)
{
	pMenuConfig->bMouseLookToggle = _bEnable;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Control_GetMouseSensitivity(int & _iSensitivity)
{
	_iSensitivity = pMenuConfig->iMouseSensitivity;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Control_SetMouseSensitivity(int _iSensitivity)
{
	if (_iSensitivity < 0)_iSensitivity = 0;
	else if (_iSensitivity > 10)_iSensitivity = 10;

	pMenuConfig->iMouseSensitivity = _iSensitivity;
	pGetInfoDirectInput->SetSensibility(_iSensitivity);

	danaeApp.fMouseSensibility = ((float)pMenuConfig->iMouseSensitivity) / 10.f;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Control_GetMouseSmoothing(bool & _bSmoothing)
{
	_bSmoothing = pMenuConfig->bMouseSmoothing;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Control_SetMouseSmoothing(bool _bSmoothing)
{
	pMenuConfig->bMouseSmoothing = _bSmoothing;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Control_GetAutoDescription(bool & _bEnable)
{
	_bEnable = pMenuConfig->bAutoDescription;
}

//-----------------------------------------------------------------------------
void ARXMenu_Options_Control_SetAutoDescription(bool _bEnable)
{
	pMenuConfig->bAutoDescription = _bEnable;
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
	danaeApp.DANAEEndRender();

	ARX_SOUND_MixerPause(ARX_SOUND_MixerMenu);
	LoadSaveGame(num); 
	danaeApp.DANAEStartRender();
}

//-----------------------------------------------------------------------------
void ARXMenu_DeleteQuest(long num)
{
	if (num != 0)
	{
		char temp[256];

		sprintf(temp, "%sSave%s\\save%04d\\", Project.workingdir, LOCAL_SAVENAME, save_l[num+save_p].num);
		KillAllDirectory(temp);
		FreeSaveGameList();
		CreateSaveGameList();
		save_p = 0;
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
	UpdateSaveGame(save_p + num);
	ARXMenu_Options_Video_SetGamma(iOldGamma);
	ARX_SOUND_MixerResume(ARX_SOUND_MixerMenu);

	FreeSaveGameList();
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
