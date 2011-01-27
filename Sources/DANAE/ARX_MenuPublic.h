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
#ifndef _ARX_MENU_EXPORT
#define _ARX_MENU_EXPORT

#include <ARX_Common.h>

//-----------------------------------------------------------------------------
//OPTIONS LANGUAGE
void ARXMenu_Options_Language_Get(int & _iLanguage);
void ARXMenu_Options_Language_Set(int _iLanguage);

//-----------------------------------------------------------------------------
//OPTIONS VIDEO
void ARXMenu_Options_Video_GetResolution(int &, int &, int &);
void ARXMenu_Options_Video_GetBitPlane(int & i_Bpp);
 

void ARXMenu_Options_Video_GetFullscreen(bool &);
void ARXMenu_Options_Video_SetFullscreen(bool);

void ARXMenu_Options_Video_GetFogDistance(int &);
void ARXMenu_Options_Video_SetFogDistance(int);
void ARXMenu_Options_Video_GetTextureQuality(int &);
void ARXMenu_Options_Video_SetTextureQuality(int);
void ARXMenu_Options_Video_GetBump(bool &);
void ARXMenu_Options_Video_SetBump(bool);
void ARXMenu_Options_Video_GetMeshReduction(int &);
void ARXMenu_Options_Video_SetMeshReduction(int);
void ARXMenu_Options_Video_GetGamma(int &);
void ARXMenu_Options_Video_SetGamma(int);
void ARXMenu_Options_Video_GetLuminosity(int &);
void ARXMenu_Options_Video_SetLuminosity(int);
void ARXMenu_Options_Video_GetContrast(int &);
void ARXMenu_Options_Video_SetContrast(int);

void ARXMenu_Options_Video_GetDetailsQuality(int &);
void ARXMenu_Options_Video_SetDetailsQuality(int);
void ARXMenu_Options_Video_GetLODQuality(int &);
void ARXMenu_Options_Video_SetLODQuality(int);
bool ARXMenu_Options_Video_SetSoftRender();

//-----------------------------------------------------------------------------
//OPTIONS AUDIO
void ARXMenu_Options_Audio_GetMasterVolume(int &);
void ARXMenu_Options_Audio_SetMasterVolume(int);
void ARXMenu_Options_Audio_GetSfxVolume(int &);
void ARXMenu_Options_Audio_SetSfxVolume(int);
void ARXMenu_Options_Audio_GetSpeechVolume(int &);
void ARXMenu_Options_Audio_SetSpeechVolume(int);
void ARXMenu_Options_Audio_GetAmbianceVolume(int &);
void ARXMenu_Options_Audio_SetAmbianceVolume(int);
void ARXMenu_Options_Audio_GetEAX(bool &);
bool ARXMenu_Options_Audio_SetEAX(bool);

//-----------------------------------------------------------------------------
//OPTIONS CONTROL
void ARXMenu_Options_Control_GetInvertMouse(bool &);
void ARXMenu_Options_Control_SetInvertMouse(bool);
void ARXMenu_Options_Control_GetAutoReadyWeapon(bool &);
void ARXMenu_Options_Control_SetAutoReadyWeapon(bool);
void ARXMenu_Options_Control_GetMouseLookToggleMode(bool &);
void ARXMenu_Options_Control_SetMouseLookToggleMode(bool);
void ARXMenu_Options_Control_GetMouseSensitivity(int &);
void ARXMenu_Options_Control_SetMouseSensitivity(int);
void ARXMenu_Options_Control_GetAutoDescription(bool &);
void ARXMenu_Options_Control_SetAutoDescription(bool);
void ARXMenu_Options_Control_GetMouseSmoothing(bool &);
void ARXMenu_Options_Control_SetMouseSmoothing(bool);

//-----------------------------------------------------------------------------
//RESUME GAME
void ARXMenu_GetResumeGame(bool &);
void ARXMenu_ResumeGame();

//-----------------------------------------------------------------------------
//NEW QUEST
void ARXMenu_NewQuest();

//-----------------------------------------------------------------------------
//LOAD QUEST
void ARXMenu_LoadQuest(long num);
void ARXMenu_DeleteQuest(long num);

//-----------------------------------------------------------------------------
//SAVE QUEST
void ARXMenu_SaveQuest(long num);

//-----------------------------------------------------------------------------
//CREDITS
void ARXMenu_Credits();

//-----------------------------------------------------------------------------
//QUIT
void ARXMenu_Quit();

#endif
