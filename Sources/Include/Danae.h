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
// DANAE.H
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		DANAE Editor
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
#define WIN32_LEAN_AND_MEAN
#ifndef DANAE_H
#define DANAE_H

//-----------------------------------------------------------------------------
#include "EERIEApp.h"
#include "EERIETypes.h"
#include "EERIEPoly.h"
#include "ARX_Player.h"
#include "ARX_Script.h"
#include "ARX_Physics.h"
#include "ARX_Interactive.h"
#include "DANAE_Debugger.h"
#include "DANAESaveLoad.h"


//-----------------------------------------------------------------------------
#define MAX_GOLD_COINS_VISUALS	7
#define IMPROVED_FOCAL			320.f
#define INC_FOCAL				75.f
#define DEC_FOCAL				50.f
#define IDR_MENU1				101
#define IDI_MAIN_ICON			101

//-----------------------------------------------------------------------------
// Offsets for book & Ratio for drawing it
extern EERIE_RGB	FADECOLOR;
extern TextureContainer * TC_fire2;
extern TextureContainer * TC_fire;
extern TextureContainer * TC_smoke;
extern TextureContainer * TC_missile;
extern TextureContainer * GoldCoinsTC[MAX_GOLD_COINS_VISUALS];
extern EERIE_3DOBJ * defaultobj;
extern EERIE_3DOBJ * firesphere;
extern EERIE_3DOBJ * cabal;
extern EERIE_3DOBJ * cameraobj;
extern EERIE_3DOBJ * markerobj;
extern EERIE_3D lastteleport;
extern EERIE_CAMERA bookcam;
extern HINSTANCE hInstance;
extern HINSTANCE hInst;
extern EERIE_S2D DANAEMouse;
extern EERIE_CAMERA subj, map;
extern EERIE_3D moveto;
extern EERIE_S2D STARTDRAG;
extern EERIE_3DOBJ * GoldCoinsObj[MAX_GOLD_COINS_VISUALS];
extern EERIE_3DOBJ * nodeobj;
extern EERIE_3D Mscenepos;
extern EERIE_MULTI3DSCENE * mse;
extern EERIE_CAMERA * Kam;
extern EERIE_BACKGROUND * curbkg;
extern EERIE_BACKGROUND DefaultBkg;
extern INTERACTIVE_OBJ * COMBINE;
extern char LastLoadedScene[256];
extern char TELEPORT_TO_LEVEL[64];
extern char TELEPORT_TO_POSITION[64];
extern float PULSATE;
extern float PULSS;
extern float _framedelay;
extern float PLAYER_BASE_RADIUS;
extern float PLAYER_BASE_HEIGHT;
extern float BASE_FOCAL;
extern float BOOKDECX;
extern float BOOKDECY;
extern float Xratio;
extern float Yratio;
extern long Bilinear;
extern long	FADEDURATION;
extern long	FADEDIR;
extern float FrameDiff;
extern long FirstFrame;
extern long EDITMODE;
extern long EDITION;
extern long DEBUGNPCMOVE;
extern long DEBUGSYS;
extern long SHOW_TORCH;
extern long USE_COLLISIONS;
extern long CURRENTLEVEL;
extern long TELEPORT_TO_ANGLE;
extern long ADDED_IO_NOT_SAVED;
extern long DANAESIZX;
extern long DANAESIZY;
extern long DANAECENTERX;
extern long DANAECENTERY;
extern long NEED2DFX;
extern short Cross;
extern short broken;
extern unsigned long FADESTART;
extern unsigned long AimTime;
extern bool ARXPausedTimer;
extern float FrameTime, LastFrameTime;
//-----------------------------------------------------------------------------
typedef struct
{
	float intensity;
	float frequency;
	unsigned long start;
	unsigned long duration;
	long	flags;
} QUAKE_FX_STRUCT;

//-----------------------------------------------------------------------------
// Class DANAE
//-----------------------------------------------------------------------------
class DANAE : public CD3DApplication
{

	protected:
		HRESULT OneTimeSceneInit();
		HRESULT DeleteDeviceObjects();
		HRESULT Render();
		HRESULT FrameMove(FLOAT fTimeKey);
		HRESULT FinalCleanup();
		void ManageKeyMouse();
		BOOL ManageEditorControls();
		void ManagePlayerControls();
		void DrawAllInterface();
		void DrawAllInterfaceFinish();
		void GoFor2DFX();
		HRESULT BeforeRun();
	public:
		HRESULT InitDeviceObjects();
 
		LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		DANAE();
		long MustRefresh;

		bool DANAEStartRender();
		bool DANAEEndRender();
};
extern DANAE danaeApp;

//-----------------------------------------------------------------------------
void SetEditMode(long ed, const bool stop_sound = true);
void AddQuakeFX(float intensity, float duration, float period, long flags);
//-----------------------------------------------------------------------------
void Danae_Registry_Read(char * string, char * text, char * defaultstr = "", long maxsize = 256);
void Danae_Registry_ReadValue(char * string, long * value, long defaultvalue = 0);
void Danae_Registry_Write(char * string, char * text);
void Danae_Registry_WriteValue(char * string, DWORD value);
void SetFilteringMode(LPDIRECT3DDEVICE7 m_pd3dDevice, long mode);
void SendGameReadyMsg();
void DanaeSwitchFullScreen();
void DANAE_KillCinematic();

#endif
