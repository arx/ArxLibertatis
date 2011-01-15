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
// DANAE.CPP
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		Danae Application Main File
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Included files
//-----------------------------------------------------------------------------
#include <windows.h>
#include <new.h>
#include <iostream>
#include <fstream>
#include "danae.h"
#include "danaedlg.h"
#include "../danae/Danae_resource.h"
#include "DANAE_VERSION.h"

#include <HERMESmain.h>
#include <HERMESConsole.h>
#include <HERMESnet.h>
#include <HERMES_PAK.h>
 
#include <EERIEUtil.h>
#include <EERIE_AVI.h>
#include <EERIEAnim.h>
#include <EERIEPathfinder.h>
#include <EERIECollisionSpheres.h>
#include <EERIEPhysicsBox.h>
#include <EERIEObject.h>
#include <EERIEPoly.h>
#include "EERIELinkedObj.h"

#include <ARX_C_Cinematique.h>
#include <ARX_Carte.h>
#include <ARX_ChangeLevel.h>
#include <ARX_CParticles.h>
#include <ARX_Collisions.h>
#include <ARX_Damages.h>
#include <ARX_Equipment.h>
#include "ARX_FTL.h"
#include <ARX_Fogs.h>
#include <ARX_GlobalMods.h>
#include <ARX_Input.h>
#include <ARX_Interface.h>
#include <ARX_Interactive.h>
#include <ARX_Levels.h>
#include "ARX_Loc.h"
#include <ARX_Menu.h>
#include "ARX_Menu2.h"
#include <ARX_Network.h>
#include <ARX_NPC.h>
#include <ARX_Particles.h>
#include <ARX_Paths.h>
#include <ARX_Scene.h>
#include <ARX_Script.h>
#include <ARX_Sound.h>
#include <ARX_Special.h>
#include <ARX_Speech.h>
#include <ARX_Spells.h>
#include <ARX_Time.h>
#include <ARX_Text.h>
#include "arx_missile.h"
#include "arx_cedric.h"
#include "ARX_HWTransform.h"
#include "ARX_MenuPublic.h"
#include "ARX_SnapShot.h"

#ifdef ARX_STEAM
#include "../steam/steam.h"

#pragma comment(lib,"steam.lib")
#endif

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

void DemoFileCheck();		

bool ARX_IsSteam()
{
#ifdef ARX_STEAM
	return true;
#else
	return false;
#endif
}

//-----------------------------------------------------------------------------

#define MAX_EXPLO 24
#define FFH_S_OK 1
#define FFH_GOTO_FINISH 2
extern INTERACTIVE_OBJ * CURPATHFINDIO;

//-----------------------------------------------------------------------------

HRESULT DANAEFinalCleanup();
void ClearGame();
void ShowInfoText(long COR);

//-----------------------------------------------------------------------------

extern long LAST_PORTALS_COUNT;
extern CARXTextManager	*pTextManage;
extern float FORCE_TIME_RESTORE;
extern CDirectInput		*pGetInfoDirectInput;
extern CMenuConfig		*pMenuConfig;
extern CMenuState		*pMenu;
extern SNAPSHOTINFO		snapshotdata;
extern short uw_mode;
extern long SPECIAL_DRAGINTER_RENDER;
extern HWND		PRECALC;
extern HWND		CDP_LIGHTOptions;
extern HWND		CDP_FogOptions;
extern EERIE_LIGHT *PDL[MAX_DYNLIGHTS];
extern INTERACTIVE_OBJ * CURRENT_TORCH;
extern EERIE_3D loddpos;
extern EERIE_3DOBJ * fogobj;
extern bool		bGameNotFirstLaunch;
extern bool		bSkipVideoIntro;
extern char		SCRIPT_SEARCH_TEXT[256];
extern char		ShowText[65536];
extern char		ShowText2[65536];
extern float Full_Jump_Height;
extern float	MAX_ALLOWED_PER_SECOND;
extern float	InventoryX;
extern float	PROGRESS_BAR_COUNT;
extern float	PROGRESS_BAR_TOTAL;
extern float	vdist;
extern float	FLOATTEST;
extern float	_MAX_CLIP_DIST;
extern float	BIGLIGHTPOWER;
extern long		LastSelectedIONum;
extern long		FistParticles;
extern long		INTER_DRAW;
extern long		INTER_COMPUTE;
extern long		USEINTERNORM;
extern long		NEED_ANCHORS;
extern long		FAKE_DIR;
extern long		DONT_WANT_PLAYER_INZONE;
extern long		DeadTime;
extern long		INTREATZONECOUNT;
extern long		TOTPDL;
extern float		ARXTotalPausedTime;
extern long		accepted;
extern long		COLLIDED_CLIMB_POLY;
extern long LOOKING_FOR_SPELL_TARGET;
extern long PATHFINDER_WAIT;
extern long		LAST_ROOM;
extern long TRUE_PLAYER_MOUSELOOK_ON;
extern unsigned char * grps;
extern long		LastSelectedIONum;
extern long		NOCHECKSUM;
extern unsigned long ROTATE_START;
extern float ARXTimeMenu;
extern float ARXOldTimeMenu;
extern long		REFUSE_GAME_RETURN;
extern long		PLAYER_MOUSELOOK_ON;
extern long		FRAME_COUNT;
extern bool bFadeInOut;
extern 	bool bFade;			//active le fade
extern long LastEERIEMouseButton;
extern long PLAYER_PARALYSED;
extern float fZFogEnd;
extern float fZFogStart;
extern bool bOLD_CLIPP;
extern bool bForceGDI;
extern bool bSoftRender;
extern bool bGMergeVertex;
extern float OLD_PROGRESS_BAR_COUNT;
extern E_ARX_STATE_MOUSE	eMouseState;

void DanaeRestoreFullScreen();

extern long FORCE_FRONT_DRAW;
extern bool bUSE_D3DFOG_INTER;
extern EERIE_3DOBJ * ssol;
extern long ssol_count;
extern EERIE_3DOBJ * slight;
extern long slight_count;
extern EERIE_3DOBJ * srune;
extern long srune_count;
extern EERIE_3DOBJ * smotte;
extern long smotte_count;
extern EERIE_3DOBJ * stite;
extern long stite_count;
extern EERIE_3DOBJ * smissile;
extern long smissile_count;
extern EERIE_3DOBJ * spapi;
extern long spapi_count;
extern EERIE_3DOBJ * sfirewave;
extern long sfirewave_count;
extern EERIE_3DOBJ * svoodoo;
extern long svoodoo_count;

//-----------------------------------------------------------------------------

extern INTERACTIVE_OBJ * FlyingOverIO;

extern unsigned long ulBKGColor;
long LAST_LOCK_SUCCESSFULL=0;
extern D3DMATRIX ProjectionMatrix;

extern CMY_DYNAMIC_VERTEXBUFFER *pDynamicVertexBufferBump;			// Duplicate pDynamicVertexBuffer for BUMP mapping.
extern CMY_DYNAMIC_VERTEXBUFFER *pDynamicVertexBuffer_TLVERTEX;		// VB using TLVERTEX format.
extern CMY_DYNAMIC_VERTEXBUFFER *pDynamicVertexBuffer_D3DVERTEX3_T;	// VB using D3DVERTEX3_T format.
extern CMY_DYNAMIC_VERTEXBUFFER * pDynamicVertexBuffer;
extern CMY_DYNAMIC_VERTEXBUFFER * pDynamicVertexBufferTransform;

extern char pStringMod[];
//-----------------------------------------------------------------------------
// Our Main Danae Application.& Instance
DANAE danaeApp;
HINSTANCE hInstance;

//-----------------------------------------------------------------------------
EERIE_3D LASTCAMPOS,LASTCAMANGLE;
EERIE_3D PUSH_PLAYER_FORCE;
CINEMATIQUE			*ControlCinematique=NULL;	// 2D Cinematic Controller
CParticleManager	*pParticleManager = NULL;
INTERACTIVE_OBJ		*lastCAMERACONTROLLER=NULL;
TextureContainer *  ombrignon = NULL;
TextureContainer *  teleportae = NULL;	
TextureContainer *  Flying_Eye = NULL;
TextureContainer *	scursor[8];			// Animated Hand Cursor TC
TextureContainer *	pTCCrossHair;			// Animated Hand Cursor TC
TextureContainer *	iconequip[5];
TextureContainer *	GoldCoinsTC[MAX_GOLD_COINS_VISUALS]; // Gold Coins Icons
TextureContainer *	ChangeLevel = NULL;		
TextureContainer *	Movable = NULL;			// TextureContainer for Movable Items (Red Cross)
TextureContainer *	explo[MAX_EXPLO];		// TextureContainer for animated explosion bitmaps (24 frames)
TextureContainer *	blood_splat = NULL;		// TextureContainer for blood splat particles

TextureContainer *	tflare = NULL;
TextureContainer *	npc_fight=NULL;	
TextureContainer *	npc_follow=NULL;	
TextureContainer *	npc_stop=NULL;	
TextureContainer *	sphere_particle=NULL;
TextureContainer *	inventory_font=NULL;
TextureContainer *	enviro=NULL;
TextureContainer *	specular=NULL;
TextureContainer *	lightsource_tc=NULL;
TextureContainer *	stealth_gauge_tc=NULL;
TextureContainer *	arx_logo_tc=NULL;
TextureContainer *	TC_fire2=NULL;
TextureContainer *	TC_fire=NULL;
TextureContainer *	TC_smoke=NULL;
TextureContainer *	TC_missile=NULL;
TextureContainer *	Z_map=NULL;
TextureContainer *	Boom=NULL;
TextureContainer *	zbtex=NULL;
TextureContainer *	mecanism_tc=NULL;
TextureContainer *	arrow_left_tc=NULL;
EERIE_MULTI3DSCENE * mse=NULL;

long NEED_TEST_TEXT=0;

SPELL_ICON spellicons[SPELL_COUNT];
bool bGToggleCombatModeWithKey;

EERIE_S2D DANAEMouse;
EERIE_3D moveto;
EERIE_3D Mscenepos;
EERIE_3D lastteleport;
EERIE_3D e3dPosBump;
EERIE_3DOBJ * GoldCoinsObj[MAX_GOLD_COINS_VISUALS];// 3D Objects For Gold Coins
EERIE_3DOBJ	* arrowobj=NULL;			// 3D Object for arrows
EERIE_3DOBJ * cameraobj=NULL;			// Camera 3D Object		// NEEDTO: Remove for Final
EERIE_3DOBJ * markerobj=NULL;			// Marker 3D Object		// NEEDTO: Remove for Final
EERIE_3DOBJ * nodeobj=NULL;				// Node 3D Object		// NEEDTO: Remove for Final
EERIE_3DOBJ * eyeballobj=NULL;			// EyeBall 3D Object	// NEEDTO: Load dynamically
EERIE_3DOBJ * cabal=NULL;				// Cabalistic 3D Object // NEEDTO: Load dynamically
EERIE_BACKGROUND DefaultBkg;
EERIE_CAMERA TCAM[32];
EERIE_CAMERA subj,map,bookcam,raycam,conversationcamera;
EERIE_CAMERA DynLightCam;

INTERACTIVE_OBJ * CAMERACONTROLLER=NULL;
_TCHAR WILLADDSPEECH[256];

EERIE_S2D STARTDRAG;
INTERACTIVE_OBJ * COMBINE=NULL;
HWND MESH_REDUCTION_WINDOW=NULL;

QUAKE_FX_STRUCT QuakeFx;
bool bALLOW_BUMP = false;
char * GTE_TITLE;
char * GTE_TEXT;
char LAST_FAILED_SEQUENCE[128]="None";
// START - Information for Player Teleport between/in Levels-------------------------------------
char TELEPORT_TO_LEVEL[64];
char TELEPORT_TO_POSITION[64];
long TELEPORT_TO_ANGLE;
long TELEPORT_TO_CONFIRM=1;
// END -   Information for Player Teleport between/in Levels---------------------------------------
char LastLoadedDLF[256];
char ItemToBeAdded[1024];
char WILL_LAUNCH_CINE[256];
char LOCAL_SAVENAME[64];
char _CURRENTLOAD_[256];
char LastLoadedScene[256];
char LAST_LAUNCHED_CINE[256];
float BASE_FOCAL=350.f;
float PLAYER_ARMS_FOCAL = 350.f;
float METALdecal=0.f;
float currentbeta=0.f;
float STRIKE_AIMTIME=0.f;
float SLID_VALUE=0.f;
float _framedelay;

float TIMEFACTOR=1.f;
float LASTfps2=0;
float fps2=0;
float fps2min=0;
long LASTfpscount=0;
long Bilinear=1;
long DEBUG1ST=0;
long DEBUGSYS=0;

long MAPUPDATE=0;
long EXTERNALVIEW=0;
long LASTEXTERNALVIEW=1;
long EXTERNALVIEWING=0;
long lSLID_VALUE=0;
long _NB_=0;
long POINTINTERPOLATION=1;
long LOAD_N_DONT_ERASE=0;
long NO_TIME_INIT=0;
long DANAESIZX=640;
long DANAESIZY=480;
long DANAECENTERX;
long DANAECENTERY;
long CurrFightPos=0;
long NO_PLAYER_POSITION_RESET=0;
long CURRENT_BASE_FOCAL=310;
long GTE_SIZE;
long CINE_PRELOAD=0;
long FOKMOD=0;
long PLAY_LOADED_CINEMATIC=0;
long PauseScript=0;
long A_FLARES=1;
long ADDED_IO_NOT_SAVED=0;
long WILL_RELOAD_ALL_TEXTURES=0;	// Set To 1 if Textures are to be reloaded from disk and restored.
long CHANGE_LEVEL_PROC_RESULT=-1;
float BOW_FOCAL=0;
long PlayerWeaponBlocked=-1;
long SHOW_TORCH=0;
float FrameDiff=0;
long CEDRIC_VERSION=0;
long CYRIL_VERSION=0;
float GLOBAL_LIGHT_FACTOR=0.85f;

//-----------------------------------------------------------------------------
// Don't touch FINAL_COMMERCIAL_DEMO anymore
// Comment #define REAL_DEMO for non-demo Version
// UNcomment #define REAL_DEMO for demo Version

#ifdef REAL_DEMO
long FINAL_COMMERCIAL_DEMO =1;
#else
long FINAL_COMMERCIAL_DEMO =0;
#endif

float GLOBAL_NPC_MIPMAP_BIAS	=-2.2f;
float GLOBAL_MIPMAP_BIAS		= 0;
float IN_FRONT_DIVIDER			= 0.75f;
float IN_FRONT_DIVIDER_FEET		=0.998f;
float IN_FRONT_DIVIDER_ITEMS	=0.7505f;
long GLOBAL_FORCE_PLAYER_IN_FRONT	=1;
long USE_NEW_SKILLS=1;
long ARX_SOUND_INIT=1;

long GORE_MODE=0;
long USE_LIGHT_OPTIM	=1;
long GLOBAL_FORCE_MINI_TEXTURE=0;
long FINAL_COMMERCIAL_GAME = 1;   // <--------------	fullgame
long GERMAN_VERSION = 0;
long FRENCH_VERSION = 0;
long CHINESE_VERSION = 0;
long EAST_EUROPE = 0;
long ALLOW_CHEATS		 =1;
long FOR_EXTERNAL_PEOPLE =0;
long USE_OLD_MOUSE_SYSTEM=1;
long NO_TEXT_AT_ALL		= 0;	   
long ARX_DEMO			= 0; 
long LAST_CONVERSATION	= 0;
long FAST_SPLASHES		= 0;
long FORCE_SHOW_FPS		= 0;
long FINAL_RELEASE		= 0;
long AUTO_FULL_SCREEN	= 0;
long SHOW_INGAME_MINIMAP= 1;
long DEBUG_FRUSTRUM		= 0;
long USE_D3DFOG			= 1; 
long GAME_EDITOR		= 1;
long NEED_EDITOR		= 1;
long USE_CEDRIC_ANIM	= 1;
extern long NEED_BENCH;
//-------------------------------------------------------------------------------
long STRIKE_TIME		= 0;
long STOP_KEYBOARD_INPUT= 0;
long REQUEST_SPEECH_SKIP= 0;
long CURRENTLEVEL		= -1;
long NOBUILDMAP			= 0; 
long TRUEFIGHT			= 0;
long DONT_ERASE_PLAYER	= 0;
float LastFrameTicks		= 0;
long SPLASH_THINGS_STAGE= 0;
long STARTED_A_GAME		= 0;
long INTRO_NOT_LOADED	= 1;
long SnapShotMode		= 0;
long ARX_CONVERSATION_MODE=-1;
long ARX_CONVERSATION_LASTIS=-1;
long BOOKBUTTON			= 0;
long LASTBOOKBUTTON		= 0;
long TSU_LIGHTING		= 0; // must be 0 at start !

long FASTmse			= 0;
long LASTMOULINEX		=-1;
long PROCESS_ALL_THEO	= 1;
long PROCESS_LEVELS		= 1;
long PROCESS_NO_POPUP	= 0;
long PROCESS_ONLY_ONE_LEVEL=-1;
long DONT_CHANGE_WORKINGDIR=0;

//-----------------------------------------------------------------------------
// EDITOR FLAGS/Vars
//-----------------------------------------------------------------------------
// Flag used to Launch Moulinex
long MOULINEX=0;
long KILL_AT_MOULINEX_END=0;
long HIPOLY=0;			// Are We Using Poly-Spawning Ray-Casted Shadows ?
long NODIRCREATION=0;	// No IO Directory Creation ?
long LOADEDD=0;			// Is a Level Loaded ?
long WILLLOADLEVEL=0;	// Is a LoadLevel command waiting ?
long WILLSAVELEVEL=0;	// Is a SaveLevel command waiting ?
long EDITMODE=0;		// EditMode (1) or GameMode (0) ?
long EDITION=EDITION_IO;	// Sub-EditMode
long USE_COLLISIONS=1;

long ARX_CONVERSATION=0;
long CHANGE_LEVEL_ICON=-1;

long ARX_MOUSE_OVER=0;
//-----------------------------------------------------------------------------
// DEBUG FLAGS/Vars
//-----------------------------------------------------------------------------
long DEBUGCODE=0;		// Debug Code to Console
long DEBUGNPCMOVE=0;	// Debug NPC Movements
long LaunchDemo=0;
long FirstFrame=1;
short Cross=0;
unsigned long WILLADDSPEECHTIME=0;
unsigned long AimTime;
unsigned char ARX_FLARES_Block=1;
float LastFrameTime=0;
float FrameTime=0;
unsigned long PlayerWeaponBlockTime=0;
unsigned long FRAMETICKS=0;
unsigned long SPLASH_START=0;
//-----------------------------------------------------------------------------
long LAST_FVAL=0;
float LAST_ZVAL=0;
extern float sp_max_start;
EERIE_RGB	FADECOLOR;

long DURING_LOCK=0;
long START_NEW_QUEST=0;
long LAST_WEAPON_TYPE=-1;
long	FADEDURATION=0;
long	FADEDIR=0;
unsigned long FADESTART=0;

float Original_framedelay=0.f;

float PULSATE;
float PULSS;
long EXITING=0;

//-----------------------------------------------------------------------------
// Toolbar Buttons Def
//-----------------------------------------------------------------------------
TBBUTTON tbButtons [] = 
{
{0, DANAE_B001, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
{1, DANAE_B002, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 1},
{0, 0, TBSTATE_ENABLED | TBSTATE_WRAP   , TBSTYLE_SEP, 0L, 0},
{8, DANAE_B009, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 8},
{12, DANAE_B013, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 12},

{0, 0, TBSTATE_ENABLED , TBSTYLE_SEP, 0L, 0},

{13, DANAE_B003, TBSTATE_ENABLED, TBSTYLE_CHECK, 0L, 13},
{16, DANAE_B005, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 16},
{0, 0, TBSTATE_ENABLED , TBSTYLE_SEP, 0L, 0},
{5, DANAE_B006, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 5},
{14, DANAE_B004, TBSTATE_ENABLED, TBSTYLE_CHECK, 0L, 14},
{2, DANAE_B007, TBSTATE_ENABLED, TBSTYLE_CHECK, 0L, 2},
{3, DANAE_B008, TBSTATE_ENABLED, TBSTYLE_CHECK, 0L, 3},
{17, DANAE_B010, TBSTATE_ENABLED, TBSTYLE_CHECK, 0L, 17},
{18, DANAE_B011, TBSTATE_ENABLED, TBSTYLE_CHECK, 0L, 18},
{11, DANAE_B014, TBSTATE_ENABLED, TBSTYLE_CHECK, 0L, 11}, // Particles Button
{0, 0, TBSTATE_ENABLED , TBSTYLE_SEP, 0L, 0},
{19, DANAE_B012, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 19},
{0, 0, TBSTATE_ENABLED , TBSTYLE_SEP, 0L, 0},
{20, DANAE_B015, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 20},
{0, 0, TBSTATE_ENABLED , TBSTYLE_SEP, 0L, 0},
{21, DANAE_B016, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 21},
{0, 0, TBSTATE_ENABLED , TBSTYLE_SEP, 0L, 0},
};

//-----------------------------------------------------------------------------

void LoadSysTextures();
HRESULT DANAEFinalCleanup();
void ShowFPS();
void ShowTestText();
void ManageNONCombatModeAnimations();
void LaunchMoulinex();

//-----------------------------------------------------------------------------

bool GetARXInstallPath(char * dest)
{
	char text[256];

	GetCurrentDirectory( 256, text );

	if (text[0]==0) return false;

	long len=strlen(text);

	if (text[len]!='\\') strcat(text,"\\");

	strcpy(dest,text);
	return true;
}

// Sends ON GAME_READY msg to all IOs
void SendGameReadyMsg()
{
	SendMsgToAllIO(SM_GAME_READY,"");
}

void DANAE_KillCinematic()
{
	if (	(ControlCinematique)
		&&	(ControlCinematique->projectload)	)
	{
		ControlCinematique->projectload=FALSE;
		ControlCinematique->OneTimeSceneReInit();
		ControlCinematique->DeleteDeviceObjects();
		PLAY_LOADED_CINEMATIC=0;
		CINE_PRELOAD=0;
	}
}

// START - DANAE Registery Funcs ****************************************************************

//-----------------------------------------------------------------------------------------------
HKEY    DanaeKey= NULL;
#define DANAEKEY_KEY     TEXT("Software\\Arkane_Studios\\DANAE")
//-----------------------------------------------------------------------------------------------

void Danae_Registry_Open()
{
	if (!FINAL_RELEASE)
	{
		RegCreateKeyEx( HKEY_CURRENT_USER, DANAEKEY_KEY, 0, NULL,
					  REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
					   &DanaeKey, NULL );
	}
}

//-----------------------------------------------------------------------------------------------

void Danae_Registry_Close()
{
	if (!FINAL_RELEASE)
	{
		RegCloseKey(DanaeKey);
		DanaeKey=NULL;
	}
}

//-----------------------------------------------------------------------------------------------

void Danae_Registry_WriteValue(char * string,DWORD value)
{
	if (!FINAL_RELEASE)
	{
		if (DanaeKey==NULL) Danae_Registry_Open();

		if (DanaeKey!=NULL)
		{
			WriteRegKeyValue( DanaeKey, string, value);
			Danae_Registry_Close();
		}
	}
}

//-----------------------------------------------------------------------------------------------

void Danae_Registry_Write(char * string,char * text)
{
	if (!FINAL_RELEASE)
	{
		if (DanaeKey==NULL) Danae_Registry_Open();

		if (DanaeKey!=NULL)
		{
			WriteRegKey( DanaeKey, string, text);
			Danae_Registry_Close();
		}
	}
}

//-----------------------------------------------------------------------------------------------

void Danae_Registry_Read(char * string,char * text,char *defaultstr,long maxsize)
{
	if (!FINAL_RELEASE)
	{
		if (DanaeKey==NULL) Danae_Registry_Open();

		if (DanaeKey!=NULL)
		{
			ReadRegKey( DanaeKey, string, text,maxsize,defaultstr);
			Danae_Registry_Close();
		}
		else
		{
			if ((defaultstr) && (defaultstr[0]!=0))
			{
			memcpy(text,defaultstr,__min(maxsize+1,(long)strlen(defaultstr)+1));
			text[__min(maxsize,(long)strlen(defaultstr))]=0;
			}
			else text[0]=0;
		}
	}
}

//-----------------------------------------------------------------------------------------------

void Danae_Registry_ReadValue(char * string,long * value,long defaultvalue)
{
	if (!FINAL_RELEASE)
	{
		if (DanaeKey==NULL) Danae_Registry_Open();

		if (DanaeKey!=NULL)
		{
			ReadRegKeyValue( DanaeKey, string, value, defaultvalue);
			Danae_Registry_Close();
		}
		else *value = defaultvalue;
	}
}

// END - DANAE Registery Funcs ******************************************************************
//-----------------------------------------------------------------------------------------------

void DanaeSwitchFullScreen()
{
	if (danaeApp.m_pDeviceInfo->bWindowed) // switching to fullscreen
	{
		KillInterTreeView();
	}

	if(pMenuConfig)
	{
		int nb=danaeApp.m_pDeviceInfo->dwNumModes;

		for(int i=0;i<nb;i++)
		{

			ARX_CHECK_NOT_NEG( pMenuConfig->iBpp );

			if( danaeApp.m_pDeviceInfo->pddsdModes[i].ddpfPixelFormat.dwRGBBitCount == ARX_CAST_UINT( pMenuConfig->iBpp ) )
			{
				ARX_CHECK_NOT_NEG( pMenuConfig->iWidth );
				ARX_CHECK_NOT_NEG( pMenuConfig->iHeight );

				if( ( danaeApp.m_pDeviceInfo->pddsdModes[i].dwWidth == ARX_CAST_UINT( pMenuConfig->iWidth ) ) &&
					( danaeApp.m_pDeviceInfo->pddsdModes[i].dwHeight == ARX_CAST_UINT( pMenuConfig->iHeight ) ) )
				{

					danaeApp.m_pDeviceInfo->ddsdFullscreenMode=danaeApp.m_pDeviceInfo->pddsdModes[i];
					danaeApp.m_pDeviceInfo->dwCurrentMode=i;
					break;
				}
			}
		}

		pMenuConfig->iNewBpp=pMenuConfig->iBpp=danaeApp.m_pFramework->bitdepth=danaeApp.m_pDeviceInfo->ddsdFullscreenMode.ddpfPixelFormat.dwRGBBitCount;
		pMenuConfig->iNewHeight=pMenuConfig->iHeight=danaeApp.m_pFramework->m_dwRenderHeight=danaeApp.m_pDeviceInfo->ddsdFullscreenMode.dwHeight;
		pMenuConfig->iNewWidth=pMenuConfig->iWidth=danaeApp.m_pFramework->m_dwRenderWidth=danaeApp.m_pDeviceInfo->ddsdFullscreenMode.dwWidth;
	}

	if(pMenu)
	{
		pMenu->bReInitAll=true;
	}

	ARX_Text_Close();
	danaeApp.SwitchFullScreen();

	if(	(danaeApp.m_pFramework->m_bIsFullscreen)&&
		(pMenuConfig) )
	{
		ARXMenu_Options_Video_SetGamma(pMenuConfig->iGamma);
	}

	GDevice=danaeApp.m_pFramework->GetD3DDevice();
	DANAESIZX=danaeApp.m_pFramework->m_dwRenderWidth;
	DANAESIZY=danaeApp.m_pFramework->m_dwRenderHeight;

	if (danaeApp.m_pDeviceInfo->bWindowed)
		DANAESIZY-=danaeApp.m_pFramework->Ystart;	

	DANAECENTERX=DANAESIZX>>1;
	DANAECENTERY=DANAESIZY>>1;

	Xratio=DANAESIZX*DIV640; 
	Yratio=DANAESIZY*DIV480; 

	ARX_Text_Init();

	extern float fInterfaceRatio;
	fInterfaceRatio = 1;

	LoadScreen(GDevice);
}

//-----------------------------------------------------------------------------------------------

void DanaeRestoreFullScreen()
{
	if(		pMenuConfig
		&&	pMenuConfig->bNoReturnToWindows )
	{
		pMenuConfig->bNoReturnToWindows=false;
		return;
	}

	danaeApp.m_pDeviceInfo->bWindowed=!danaeApp.m_pDeviceInfo->bWindowed;
	danaeApp.SwitchFullScreen();

	if(		(danaeApp.m_pFramework->m_bIsFullscreen)
		&&	(pMenuConfig) )
	{
		ARXMenu_Options_Video_SetGamma(pMenuConfig->iGamma);
	}

	GDevice=danaeApp.m_pFramework->GetD3DDevice();
	DANAESIZX=danaeApp.m_pFramework->m_dwRenderWidth;
	DANAESIZY=danaeApp.m_pFramework->m_dwRenderHeight;
	DANAECENTERX=DANAESIZX>>1;
	DANAECENTERY=DANAESIZY>>1;

	Xratio=DANAESIZX*DIV640; 
	Yratio=DANAESIZY*DIV480; 

	ARX_Text_Init();

	LoadScreen(GDevice);
}

//-----------------------------------------------------------------------------------------------

extern void InitTileLights();

//-----------------------------------------------------------------------------

void InitializeDanae()
{
	InitTileLights();
	snapshotdata.bits=16;
	strcpy(snapshotdata.filenames,"snap");
	strcpy(snapshotdata.path,"c:\\");
	snapshotdata.imgsec=25;
	snapshotdata.xsize=640;
	snapshotdata.ysize=480;
	snapshotdata.flag=1;
	
	char temp[512];
	char temp2[512];
	EERIEMathPrecalc();
	ARX_MISSILES_ClearAll();
	ARX_SPELLS_Init();

	ARX_SPELLS_ClearAllSymbolDraw();
	ARX_PARTICLES_ClearAll();
	ARX_BOOMS_ClearAll();
	ARX_MAGICAL_FLARES_FirstInit();

	strcpy(LastLoadedScene,"");
	strcpy(temp2,"Graph\\Levels\\Level");

	switch (Project.demo)
	{
		case NOLEVEL:
			temp2[0] = 0;
			break;
		case LEVELDEMO:
			strcat(temp2, "Demo\\");
			break;
		case LEVELDEMO2:
			strcat(temp2, "Demo2\\");
			break;
		case LEVELDEMO3:
			strcat(temp2, "Demo3\\");
			break;
		case LEVELDEMO4:
			strcat(temp2, "Demo4\\");
			break;
		case LEVEL0:
			strcat(temp2, "0\\");
			break;
		case LEVEL1:
			strcat(temp2, "1\\");
			break;
		case LEVEL2:
			strcat(temp2, "2\\");
			break;
		case LEVEL3:
			strcat(temp2, "3\\");
			break;
		case LEVEL4:
			strcat(temp2, "4\\");
			break;
		case LEVEL5:
			strcat(temp2, "5\\");
			break;
		case LEVEL6:
			strcat(temp2, "6\\");
			break;
		case LEVEL7:
			strcat(temp2, "7\\");
			break;
		case LEVEL8:
			strcat(temp2, "8\\");
			break;
		case LEVEL9:
			strcat(temp2, "9\\");
			break;
		case LEVEL10:
			strcat(temp2, "10\\");
			break;
		case LEVEL11:
			strcat(temp2, "11\\");
			break;
		case LEVEL12:
			strcat(temp2, "12\\");
			break;
		case LEVEL13:
			strcat(temp2, "13\\");
			break;
		case LEVEL14:
			strcat(temp2, "14\\");
			break;
		case LEVEL15:
			strcat(temp2, "15\\");
			break;
		case LEVEL16:
			strcat(temp2, "16\\");
			break;
		case LEVEL17:
			strcat(temp2, "17\\");
			break;
		case LEVEL18:
			strcat(temp2, "18\\");
			break;
		case LEVEL19:
			strcat(temp2, "19\\");
			break;
		case LEVEL20:
			strcat(temp2, "20\\");
			break;
		case LEVEL21:
			strcat(temp2, "21\\");
			break;
		case LEVEL22:
			strcat(temp2, "22\\");
			break;
		case LEVEL23:
			strcat(temp2, "23\\");
			break;
		case LEVEL24:
			strcat(temp2, "24\\");
			break;
		case LEVEL25:
			strcat(temp2, "25\\");
			break;
		case LEVEL26:
			strcat(temp2, "26\\");
			break;
		case LEVEL27:
			strcat(temp2, "27\\");
			break;
		default:
			temp2[0] = 0;
	}

	memset(&DefaultBkg,0,sizeof(EERIE_BACKGROUND));
	ACTIVEBKG=&DefaultBkg;
	InitBkg(ACTIVEBKG,MAX_BKGX,MAX_BKGZ,BKG_SIZX,BKG_SIZZ);	
	InitNodes(1);
	
	player.size.y=subj.size.y=-PLAYER_BASE_HEIGHT;
	player.size.x=subj.size.x=PLAYER_BASE_RADIUS;
	player.size.z=subj.size.z=PLAYER_BASE_RADIUS;
	player.desiredangle.a=player.angle.a=subj.angle.a=3.f;
	player.desiredangle.b=player.angle.b=subj.angle.b=268.f;
	player.desiredangle.g=player.angle.g=subj.angle.g=0.f;
	subj.pos.x=900.f;
	subj.pos.y=PLAYER_BASE_HEIGHT;
	subj.pos.z=4340.f;	
	subj.clip.left=0; 
	subj.clip.top=0;
	subj.clip.right=640;
	subj.clip.bottom=480;
	subj.clipz0=0.f;
	subj.clipz1=2.999f;
	subj.centerx=subj.clip.right/2;
	subj.centery=subj.clip.bottom/2;
	subj.AddX=320.f;
	subj.AddY=240.f;
	subj.focal=BASE_FOCAL;
	subj.Zdiv=3000.f;
	subj.Zmul=1.f/subj.Zdiv;
	subj.clip3D=60;
	subj.type=CAM_SUBJVIEW;
	subj.bkgcolor=0x00000000;

	SetActiveCamera(&subj);
	SetCameraDepth(2100.f);
	memcpy(&bookcam,&subj,sizeof(EERIE_CAMERA));
	memcpy(&raycam,&subj,sizeof(EERIE_CAMERA));
	memcpy(&conversationcamera,&subj,sizeof(EERIE_CAMERA));
	memcpy(&DynLightCam,&subj,sizeof(EERIE_CAMERA));

	raycam.centerx=320;	
	raycam.centery=320;	
	raycam.AddX=320.f;
	raycam.AddY=320.f;

	bookcam.angle.a=0.f;
	bookcam.angle.b=0.f;
	bookcam.angle.g=0.f;
	bookcam.pos.x = 0.f;
	bookcam.pos.y=0.f;
	bookcam.pos.z=0.f;
	bookcam.focal=BASE_FOCAL;

	map.pos.x=1500.f;
	map.pos.y=-6000.f;
	map.pos.z=1500.f;
	map.angle.a=90.f;
	map.angle.b=0.f;
	map.angle.g=0.f;
	map.clip.left=0; 
	map.clip.top=0;
	map.clip.right=640;
	map.clip.bottom=480;
	map.clipz0=0.001f;
	map.clipz1=0.999f;
	map.centerx=(map.clip.right-map.clip.left)/2;
	map.centery=(map.clip.bottom-map.clip.top)/2;
	map.AddX=320.f;
	map.AddY=240.f;
	map.focal=400.f;
	map.Zdiv=3000.f;
	map.Zmul=1.f/map.Zdiv;
	map.clip3D=1000;
	map.type=CAM_TOPVIEW;
	map.bkgcolor=0x001F1F55;
	SetActiveCamera(&map);
	SetCameraDepth(10000.f);
	danaeApp.MustRefresh=TRUE;

	for (long i=0;i<32;i++)
		memcpy(&TCAM[i],&subj,sizeof(EERIE_CAMERA));
	
	ACTIVEBKG->ambient.r = 0.09f;
	ACTIVEBKG->ambient.g = 0.09f;
	ACTIVEBKG->ambient.b = 0.09f;
	ACTIVEBKG->ambient255.r=ACTIVEBKG->ambient.r*255.f;
	ACTIVEBKG->ambient255.g=ACTIVEBKG->ambient.g*255.f;
	ACTIVEBKG->ambient255.b=ACTIVEBKG->ambient.b*255.f;

	ARX_Text_Init();

	LoadSysTextures();
	CreateInterfaceTextureContainers();

	if (MOULINEX)
	{
	}
	else if (LaunchDemo) 
	{
		if ((FINAL_RELEASE) 
			&& (pMenuConfig->bFullScreen || AUTO_FULL_SCREEN )
		   )
		{
			DanaeSwitchFullScreen();
		}

		LaunchDemo=0;
		SPLASH_THINGS_STAGE=11;
	}
	else if (temp2[0]!=0)
	{
		char ftemp[256];
		strcpy(ftemp,temp2);
		MakeDir(temp,temp2);

		if (FastSceneLoad(ftemp))
		{
			FASTmse=1;
			goto suite;
		}

		ARX_SOUND_PlayCinematic("Editor_Humiliation.wav");
		mse=PAK_MultiSceneToEerie(temp);				
	suite:
		;
		EERIEPOLY_Compute_PolyIn();
		strcpy(LastLoadedScene,temp2);
	}

	if ((GAME_EDITOR) && (!MOULINEX))
		LaunchInteractiveObjectsApp( danaeApp.m_hWnd);	
	}

void Dbg_str(char * txt)
{
}
__forceinline void LaunchCDROMCheck(long param)
{
	return;

	if (param==0)
	{
		// Randomize Check 5% chance per call.
		if (rnd()<0.95f)
			return;
	}

	//Place CDROM Checking code here...
}

//*************************************************************************************
// DANAEApp EntryPoint
//*************************************************************************************
int HandlerMemory(size_t stSize)
{
	if(	danaeApp.m_hWnd)
	{
		ShowWindow(danaeApp.m_hWnd,SW_MINIMIZE|SW_HIDE);
	}

	MessageBox(NULL,"Fatal memory error!!!","Arx fatalis",MB_ICONERROR);
	exit(-1);
}

//-----------------------------------------------------------------------------

HMODULE hSteamLibrary = NULL;

#ifdef ARX_STEAM
typedef int (STEAM_CALL *SteamStartupFn)( unsigned int uUsingMask, TSteamError *pError );
typedef int (STEAM_CALL *SteamCleanupFn)( TSteamError *pError );
typedef int	(STEAM_CALL	*SteamGetAppPurchaseCountryFn)( unsigned int uAppId, char *szCountryBuf, unsigned int uBufSize, int * pPurchaseTime, TSteamError *pError );
typedef int	(STEAM_CALL	*SteamIsSubscribedFn)( unsigned int uSubscriptionId, int *pbIsSubscribed, int *pbIsSubscriptionPending, TSteamError *pError );

SteamStartupFn pSteamStartup = NULL;
SteamCleanupFn pSteamCleanup = NULL;
SteamGetAppPurchaseCountryFn pSteamGetAppPurchaseCountry = NULL;
SteamIsSubscribedFn pSteamIsSubscribed = NULL;

bool InitSteam()
{
	hSteamLibrary = LoadLibrary( "steam.dll" );

	if( hSteamLibrary )
	{
		pSteamStartup = (SteamStartupFn)GetProcAddress( hSteamLibrary, "SteamStartup" );
		pSteamCleanup = (SteamCleanupFn)GetProcAddress( hSteamLibrary, "SteamCleanup" );
		pSteamGetAppPurchaseCountry = (SteamGetAppPurchaseCountryFn)GetProcAddress( hSteamLibrary, "SteamGetAppPurchaseCountry" );
		pSteamIsSubscribed = (SteamIsSubscribedFn)GetProcAddress( hSteamLibrary, "SteamIsSubscribed" );

		if( 
			(!pSteamStartup)||
			(!pSteamCleanup)||
			(!pSteamGetAppPurchaseCountry)||
			(!pSteamIsSubscribed)
			)
		{

			exit(0);
		}

		TSteamError Error;
		SteamStartEngine( &Error );

		if( !pSteamStartup( STEAM_USING_LOGGING|STEAM_USING_ACCOUNT|STEAM_USING_USERID, &Error ) )
		{

			exit(0);
		}
	}

	return hSteamLibrary? true : false;
}

//-----------------------------------------------------------------------------

bool ReleaseSteam()
{
	if( hSteamLibrary )
	{
		if( pSteamCleanup )
		{
			TSteamError Error;
			pSteamCleanup(&Error);
		}

		FreeLibrary( hSteamLibrary );
	}

	return true;
}

//-----------------------------------------------------------------------------

bool IsDemo()
{
	TSteamError steamError;
	int isSubscribed = 0, isPending = 1;
	int retVal;

	retVal = SteamIsAppSubscribed( 1700, &isSubscribed, &isPending, &steamError ); // ARKANE : modified for MM on steam

	if ( retVal && steamError.eSteamError == eSteamErrorNone )
	{
		if ( !isSubscribed ) // if they don't own HL2 this must be the demo!
		{
			return true;
		}
	}
	
	return false;
}
#endif

//-----------------------------------------------------------------------------

bool IsNoGore( void )
{
#ifdef ARX_STEAM
	char szCountry[128];
	int iPurchaseTime;
	bool bReducedGore = false;
	TSteamError steamError;
	memset( szCountry, 0, 128 );
	iPurchaseTime = 0;
	
	int bIsSubscribed1 = false;
	int bIsSubscribed2 = false;
	int bIsPending = false;
	pSteamGetAppPurchaseCountry( 1700, szCountry, 128, &iPurchaseTime, &steamError );

	if( !stricmp( szCountry, "de" ) )
	{
		return true;
	}

	return false;
#else
	return GERMAN_VERSION? true : false;
#endif
}

//-----------------------------------------------------------------------------

INT WINAPI WinMain( HINSTANCE _hInstance, HINSTANCE, LPSTR strCmdLine, INT )
{

#ifdef _DEBUG
	ARX_LOG_INIT();
#endif // _DEBUG

#ifdef ARX_STEAM
	InitSteam();

	if( IsDemo() )
	{
		FINAL_COMMERCIAL_DEMO = 1;
	}

#endif

	_set_new_mode(1);																//memory handler activated for malloc too
	_set_new_handler(HandlerMemory);

	ARX_MINIMAP_Reset();
	int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG); // Get current flag
	flag |= _CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF;// Turn on leak-checking bit
	_CrtSetDbgFlag(flag);															// Set flag to the new value
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
	
	long i;
	hInstance = _hInstance;

	if (FINAL_COMMERCIAL_GAME)
	{
		ARX_SOUND_INIT=1;
		FOR_EXTERNAL_PEOPLE=1;
		ARX_DEMO=0;
	}
	else if (FINAL_COMMERCIAL_DEMO)
	{
		ARX_SOUND_INIT=1;
		FOR_EXTERNAL_PEOPLE=1;
		ARX_DEMO=1;
	}

	if (FOR_EXTERNAL_PEOPLE)
	{
		ARX_SOUND_INIT		= 1;
		ALLOW_CHEATS		= 0;
		CEDRIC_VERSION		= 0;
		NO_TEXT_AT_ALL		= 1;

		if (!FINAL_COMMERCIAL_DEMO)
			ARX_DEMO		= 0; 

		FAST_SPLASHES		= 0;
		FORCE_SHOW_FPS		= 0;
		FINAL_RELEASE		= 1;
#ifdef _DEBUG
        AUTO_FULL_SCREEN	= 0;
#endif
		DEBUG_FRUSTRUM		= 0;
		USE_D3DFOG			= 1; // <-------------------- FOG
		GAME_EDITOR			= 0;
		NEED_EDITOR			= 0;
		TRUEFIGHT			= 0;
	}
	else if (CEDRIC_VERSION)
	{
		ARX_DEMO=0; 
		FAST_SPLASHES=1;
		FORCE_SHOW_FPS=1;
		FINAL_RELEASE=1; // 1 with pack or 0 without pack
		AUTO_FULL_SCREEN=0;
		USE_D3DFOG=1; 
	}

	CalcFPS(true);
	HERMES_Memory_Security_On(32000);
	
	ARX_MAPMARKER_Init();
	
	for (i=0;i<8;i++)	scursor[i]=NULL;

	ARX_DAMAGES_SCREEN_SPLATS_Init();
	ARX_SPELLS_CancelSpellTarget();

	for (i=0;i<MAX_EXPLO;i++) explo[i]=NULL;

	USE_FAST_SCENES = 1;
	Dbg_str("Danae Start"); 

	memset(&Project,0,sizeof(PROJECT));
	Project.vsync = true;
	Dbg_str("Project Init");

	if (!FOR_EXTERNAL_PEOPLE)
	{
		char * param[10];
		long parampos=0;
		
		param[0]=strtok(strCmdLine," ");

		for (long j=1;j<10;j++)
			param[j]=strtok(NULL," ");

		if ((param[parampos] != NULL))
		{
			if (!stricmp(param[parampos],"demo"))
			{
				ARX_DEMO=1;
			}
			else
			{
				Dbg_str("PARAMS");
				FINAL_RELEASE=0;
				GAME_EDITOR=1;

				if (!stricmp(param[parampos],"editor"))
				{
					Dbg_str("PARAM EDITOR");
					NEED_ANCHORS=1;
				}
				else
				{
					NEED_ANCHORS=1;
					USE_FAST_SCENES=0;
					Dbg_str("PARAM MOULINEX");

					if (param[parampos][0]=='-')
					{
						long posflags=parampos;
						PROCESS_NO_POPUP=1;
						PROCESS_ALL_THEO=0;
						PROCESS_LEVELS=0;
						PROCESS_ONLY_ONE_LEVEL=-1;

						if ((IsIn(param[posflags],"d")) || (IsIn(param[posflags],"D")))
						{
							parampos++;
							DONT_CHANGE_WORKINGDIR=1;

							if (PAK_DirectoryExist(param[parampos]))
								File_Standardize(param[parampos],Project.workingdir);
							else exit(0);
						}

						if ((IsIn(param[posflags],"u")) || (IsIn(param[posflags],"U")))
						{
							parampos++;
							PROCESS_ONLY_ONE_LEVEL=atoi(param[parampos]);
						}				

						if ((IsIn(param[posflags],"o")) || (IsIn(param[posflags],"O")))
						{
							PROCESS_ALL_THEO=1;
						}

						if ((IsIn(param[posflags],"f")) || (IsIn(param[posflags],"F")))
						{
							NEED_ANCHORS=0;
							USE_FAST_SCENES=1;
							NOCHECKSUM=1;
						}

						if ((IsIn(param[posflags],"l")) || (IsIn(param[posflags],"L")))
						{
							PROCESS_LEVELS=1;
						}

						if ((IsIn(param[posflags],"t")) || (IsIn(param[posflags],"T")))
						{
							TSU_LIGHTING=1;
						}

						parampos++;
					}
					else
					{
						PROCESS_ALL_THEO=1;
						PROCESS_LEVELS=1;
					}

					if (!stricmp(param[parampos],"moulinex"))
					{
						MOULINEX=1;
						KILL_AT_MOULINEX_END=1;
						
					}
				}
			}
		}
		else
		{
			Dbg_str("FRGE");
			GAME_EDITOR=1;

			if (FINAL_RELEASE) 
				GAME_EDITOR=0;
		}
	}

	NOCHECKSUM=0;

	if (!MOULINEX)
	{
		if (FINAL_RELEASE)
		{
			if (!GetARXInstallPath(Project.workingdir))
			{
				MessageBox(NULL,"Unable to Find Game Info\nPlease Reinstall ARX Fatalis","Error",MB_ICONEXCLAMATION | MB_OK);
				exit(0);
			}

			File_Standardize(Project.workingdir,Project.workingdir);
			Dbg_str("Got Install Path");
		}
		else 
		{
			Dbg_str("Not Installed");
			Danae_Registry_Read("LastWorkingDir",Project.workingdir,"c:\\arx\\",256);		
		}
	}

	if ((!MOULINEX) && FINAL_RELEASE)
	{
		Dbg_str("FINAL RELEASE");
		char pakfile[256];

		if(pStringMod[0])
		{
			sprintf(pakfile,"%s%s",Project.workingdir,pStringMod);
			Dbg_str(pakfile);

			if (FileExist(pakfile))
			{
				Dbg_str("FileExist");
				PAK_SetLoadMode(LOAD_PACK,pakfile);
				Dbg_str("LoadMode OK");
			
			}
		}

		sprintf(pakfile,"%sdata.pak",Project.workingdir);
		Dbg_str(pakfile);

		if (FileExist(pakfile))
		{
			NOBUILDMAP=1;
			NOCHECKSUM=1;
			Dbg_str("FileExist");
			PAK_SetLoadMode(LOAD_PACK,pakfile);
			Dbg_str("LoadMode OK");
			
		}
		else
		{
			Dbg_str("Exit");
			MessageBox(NULL, "Unable to Find Data File\nPlease Reinstall ARX Fatalis", "Arx Fatalis - Error", MB_ICONEXCLAMATION | MB_OK);
			exit(0); 
		}

		Dbg_str("LocPAK");
		sprintf(pakfile, "%sloc.pak", Project.workingdir);

		if( FileExist( pakfile ) )
		{
			PAK_SetLoadMode(LOAD_PACK, pakfile,Project.workingdir);
		}
		else
		{
			sprintf(pakfile, "%sloc_default.pak", Project.workingdir);

			if( FileExist( pakfile ) )
			{
				PAK_SetLoadMode(LOAD_PACK, pakfile,Project.workingdir);
			}
			else
			{
				MessageBox(NULL, "Unable to Find Data File\nPlease Reinstall ARX Fatalis", "Arx Fatalis - Error", MB_ICONEXCLAMATION | MB_OK);
				exit(0);
			}
		}

		Dbg_str("data2PAK");

		sprintf(pakfile, "%sdata2.pak", Project.workingdir);

		if (FileExist(pakfile))
			PAK_SetLoadMode(LOAD_PACK, pakfile,Project.workingdir);
		else
		{
			MessageBox(NULL, "Unable to Find Data File\nPlease Reinstall ARX Fatalis", "Arx Fatalis - Error", MB_ICONEXCLAMATION | MB_OK);
			exit(0);
		}
	}
	else
	{
		Dbg_str("TRUEFILE LM");
		PAK_SetLoadMode(LOAD_TRUEFILE,"");
		char fich[256];
		sprintf(fich,"%sPAK_NOT_FOUND.txt",Project.workingdir);
		PAK_NotFoundInit(fich);
		sprintf(fich,"%sERROR_Log.txt",Project.workingdir);
		ERROR_Log_Init(fich);
	}

	//delete current for clean save.........
	char txttemp[256];

	for(	unsigned int uiNum=0;
			uiNum<20;
			++uiNum)
	{
		sprintf(txttemp,"%sSave%s\\Cur%04d\\",Project.workingdir,LOCAL_SAVENAME,uiNum);

		if (DirectoryExist(txttemp))
			KillAllDirectory(txttemp);
	}

	ARX_INTERFACE_NoteInit();
	Dbg_str("Note Init");
	Vector_Init(&PUSH_PLAYER_FORCE);	
	ARX_SPECIAL_ATTRACTORS_Reset();
	Dbg_str("Attr Init");
	ARX_SPELLS_Precast_Reset();
	Dbg_str("ASP Init");
	
	for (long t=0;t<MAX_GOLD_COINS_VISUALS;t++)
	{
		GoldCoinsObj[t]=NULL;
		GoldCoinsTC[t]=NULL;
	}

	Dbg_str("GC Init");
	memset(LOCAL_SAVENAME,0,60);
	Dbg_str("LSV Init");
	ModeLight=MODE_DYNAMICLIGHT | MODE_DEPTHCUEING;

	memset(&DefaultBkg,0,sizeof(EERIE_BACKGROUND));
	memset(TELEPORT_TO_LEVEL,0,64);
	memset(TELEPORT_TO_POSITION,0,64);
	Dbg_str("Mset");
	
	EERIE_ANIMMANAGER_Init();
	Dbg_str("AnimManager Init");
	ARX_SCRIPT_EventStackInit();
	Dbg_str("EventStack Init");
	ARX_EQUIPMENT_Init();
	Dbg_str("AEQ Init");
	memset(_CURRENTLOAD_,0,256);

	char temp[256];

	Danae_Registry_Read("LastWorkingDir",temp,"");

	if (temp[0]==0)
	{
		Danae_Registry_WriteValue("WND_IO_DlgProc_POSX",0);
		Danae_Registry_WriteValue("WND_IO_DlgProc_POSY",0);
		Danae_Registry_WriteValue("WND_LightPrecalc_POSX",0);
		Danae_Registry_WriteValue("WND_LightPrecalc_POSY",0);
		Danae_Registry_WriteValue("WND_LightOptions_POSX",0);
		Danae_Registry_WriteValue("WND_LightOptions_POSY",0);
		Dbg_str("RegData Read");
	}

	Danae_Registry_Read("LOCAL_SAVENAME",LOCAL_SAVENAME,"",16);

	if (!FOR_EXTERNAL_PEOPLE)
	{
		char stemp[256];
		unsigned long ls = 64;
		GetComputerName(stemp, &ls);

		if (!stricmp(stemp,"max"))
		{
			CYRIL_VERSION=1;
			AUTO_FULL_SCREEN=0;

		}
	}	

	DemoFileCheck();

	ARX_CHANGELEVEL_MakePath();
	Dbg_str("ACL MakePath");

	LastLoadedDLF[0]=0;
	ARX_SCRIPT_Timer_FirstInit(512);
	Dbg_str("Timer Init");
	ARX_FOGS_FirstInit();
	Dbg_str("FGS Init");

	EERIE_LIGHT_GlobalInit();
	Dbg_str("Lights Init");
	
	Dbg_str("Svars Init");

	// Script Test
	lastteleport.x=0.f;
	lastteleport.y=PLAYER_BASE_HEIGHT;
	lastteleport.z=0.f;
	////////////////////

	Project.soundmode = ARX_SOUND_ON;
	inter.init=0;
	InitInter(10);	

	memset(&player,0,sizeof(ARXCHARACTER));
	ARX_PLAYER_InitPlayer();
	HERMES_InitDebug();

	CleanInventory();

	ARX_SPEECH_FirstInit();
	ARX_CONVERSATION_FirstInit();
	ARX_SPEECH_Init();
	ARX_SPEECH_ClearAll();
	QuakeFx.intensity=0.f;

	ForceSendConsole("Launching DANAE", 1, 0, (HWND)1);

	if (	(!FINAL_COMMERCIAL_DEMO)
		&&	(!FINAL_COMMERCIAL_GAME)	)
	{
		if (LoadLibrary(("RICHED32.DLL")) == NULL)
		{
			MessageBox(NULL, "DanaeScriptEditor :: IDS_RICHED_LOAD_FAIL", "", MB_OK|MB_ICONEXCLAMATION);
		}
	}
	
	if (FINAL_RELEASE)
	{
		LaunchDemo=1;

		File_Standardize(Project.workingdir, Project.workingdir);
		Project.TextureSize=0;
		Project.TextureBits=16;
		Project.bits=32;
		Project.compatibility=0;
		Project.demo=LEVEL10;
		Project.ambient=1;
		Project.multiplayer=0;
		NOCHECKSUM=1;
	}
	else if (!MOULINEX)
		DialogBox( hInstance,MAKEINTRESOURCE(IDD_STARTOPTIONS), NULL, StartProc );	
	else
	{
		Project.demo=LEVELDEMO2;

		if (!DONT_CHANGE_WORKINGDIR)
			Danae_Registry_Read("LastWorkingDir",Project.workingdir,"c:\\arx\\",256);		
	}

	Dbg_str("After Popup");
	atexit(ClearGame);
	char pakfile[256];
	sprintf(pakfile,"%sdata.pak",Project.workingdir);

	if (LaunchDemo)
	{
		Dbg_str("LaunchDemo");
		GAME_EDITOR=1;

		if (FINAL_RELEASE) GAME_EDITOR=0;

		{
			NOBUILDMAP=1;
			NOCHECKSUM=1;
		}
	}

	if (LAST_CHINSTANCE!=-1)
	{
		Dbg_str("KillDir");
		ARX_CHANGELEVEL_MakePath();
		KillAllDirectory(CurGamePath);
		CreateDirectory(CurGamePath,NULL);
	}	

	Project.improve=0;
	Project.interpolatemouse = 0;

	danaeApp.d_dlgframe=0;

	if (MOULINEX)
	{
		danaeApp.CreationSizeX=800;
		danaeApp.CreationSizeY = 12;
	}
	else
	{
		danaeApp.CreationSizeX=648;
		danaeApp.CreationSizeY = 552;
	}

	if (((GAME_EDITOR) && (!MOULINEX) && (!(FINAL_RELEASE))) || NEED_EDITOR)
	{
		GAME_EDITOR=1;
		danaeApp.CreationFlags= WCF_NOSTDPOPUP | WCF_ACCEPTFILES ;
		danaeApp.CreationMenu=IDR_DANAEMENU;

		//todo free
		danaeApp.ToolBar=(EERIETOOLBAR *)malloc(sizeof(EERIETOOLBAR));
		danaeApp.ToolBar->CreationToolBar = IDR_DANAETOOLS;
		danaeApp.ToolBar->Bitmap = IDB_DANAETB;
		danaeApp.ToolBar->Buttons=tbButtons;
		danaeApp.ToolBar->ToolBarNb=23;
		danaeApp.ToolBar->Type=EERIE_TOOLBAR_TOP;
		danaeApp.ToolBar->String=NULL;
	}
	else 
	{

		danaeApp.CreationFlags= WCF_NOSTDPOPUP;

		if (GAME_EDITOR) danaeApp.CreationFlags|= WCF_ACCEPTFILES;
	}

	Dbg_str("Application Creation");
	g_pD3DApp = &danaeApp;

    if( FAILED( danaeApp.Create( hInstance, strCmdLine ) ) )
		return 0;

	Dbg_str("Application Creation Success");
	ShowWindow(danaeApp.m_hWnd, SW_HIDE);
	MAIN_PROGRAM_HANDLE=danaeApp.m_hWnd;
	danaeApp.m_pFramework->bitdepth=Project.bits;

	if ((!MOULINEX) && (!FINAL_RELEASE))
	{
		char texx[64];
		strcpy(texx,"GaiaMessages");
		GaiaWM=RegisterWindowMessage(texx); 
	}

	Dbg_str("Sound Init");

	if (	(Project.soundmode != 0)
		&&	ARX_SOUND_INIT	)
		ARX_SOUND_Init(MAIN_PROGRAM_HANDLE);
	
	Dbg_str("Sound Init Success");

	Dbg_str("DInput Init");
	ARX_INPUT_Init_Game_Impulses();
	pGetInfoDirectInput = new CDirectInput();
	char szPath[256];
	sprintf( szPath, "%s\\cfg.ini", Project.workingdir );

	if( !FileExist( szPath ) )
	{
		sprintf(szPath, "%s\\cfg_default.ini", Project.workingdir );
	}

	pMenuConfig=new CMenuConfig(szPath);
	pMenuConfig->ReadAll();
	Dbg_str("DInput Init Success");

	if (pMenuConfig->bEAX)
	{
		ARXMenu_Options_Audio_SetEAX(true);
	}

	ARX_MINIMAP_FirstInit();
	ForceSendConsole("DANAE Runnning",1,0,(HWND)danaeApp.m_hWnd);

	i = 10;
	Dbg_str("AInput Init");

	while (!ARX_INPUT_Init(hInstance,danaeApp.m_hWnd))
	{		
		Sleep(30);
		i--;

		if (i==0)
		{
			ShowPopup("Unable To Initialize ARX INPUT, Leaving...");
			ARX_INPUT_Release();

			if (MAIN_PROGRAM_HANDLE!=NULL)
			SendMessage(MAIN_PROGRAM_HANDLE,WM_CLOSE,0,0);

			exit(0);
		}

		SetActiveWindow(danaeApp.m_hWnd);
		SetWindowPos(danaeApp.m_hWnd,HWND_TOP,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
	}

	Dbg_str("AInput Init Success");

	//read from cfg file
	if (strlen(Project.localisationpath)==0)
	{
		strcpy(Project.localisationpath,"english");
	}

	ShowWindow(danaeApp.m_hWnd, SW_SHOW);

	//-------------------------------------------------------------------------

	char tex[512];

	if (GAME_EDITOR)
		sprintf(tex,"DANAE Project - %s",Project.workingdir);
	else 
		sprintf(tex,"ARX Fatalis");

	if (MOULINEX)
		sprintf(tex,"MOULINEX - %s",Project.workingdir);

	SetWindowTitle(danaeApp.m_hWnd,tex);
	
	Project.interfacergb.r = 0.46f;
	Project.interfacergb.g = 0.46f;
	Project.interfacergb.b = 1.f;

	Project.torch.r=1.f;
	Project.torch.g = 0.8f;
	Project.torch.b = 0.66666f;
	Dbg_str("InitializeDanae");
	InitializeDanae();

	Dbg_str("InitializeDanae Success");
	Dbg_str("DanaeApp RUN");
	danaeApp.m_bReady = TRUE;
	char fic[256];
	sprintf(fic,"%sGraph\\Obj3D\\Interactive\\Player\\G.ASL",Project.workingdir);

	LaunchCDROMCheck(0);
	HRESULT hr=danaeApp.Run();

#ifdef ARX_STEAM
	ReleaseSteam();
#endif


#ifdef _DEBUG
	ARX_LOG_CLEAN();
#endif // _DEBUG

	return hr;
}

//*************************************************************************************
// DANAE()
//  Application constructor. Sets attributes for the app.
//*************************************************************************************
DANAE::DANAE() : CD3DApplication()
{
	m_strWindowTitle  = TEXT("ARX Fatalis");
    m_bAppUseZBuffer  = TRUE;
    m_bAppUseStereo   = FALSE;
    m_bShowStats      = TRUE;
    m_fnConfirmDevice = NULL;
	m_hWnd=NULL;
}

//-----------------------------------------------------------------------------

bool DANAE::DANAEStartRender()
{
	return m_pFramework->StartRender();
}

//-----------------------------------------------------------------------------

bool DANAE::DANAEEndRender()
{
	return m_pFramework->EndRender();
}

//-----------------------------------------------------------------------------

TextureContainer * _GetTexture_NoRefinement(char * text)
{

	return (GetTextureFile_NoRefinement(text));

}

//*************************************************************************************
// INTERACTIVE_OBJ * FlyingOverObject(EERIE_S2D * pos)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Returns IO under cursor, be it in inventories or in scene
//   Returns NULL if no IO under cursor
//*************************************************************************************
INTERACTIVE_OBJ * FlyingOverObject(EERIE_S2D * pos,long flag)
{

	if ((flag & 1) && danaeApp.Lock()) 
	{
		INTERACTIVE_OBJ * io=GetFromInventory(pos);

		if (io) 
		{
			danaeApp.Unlock();
			return io;
		}

		if (InInventoryPos(pos)) 
		{
			danaeApp.Unlock();
			return NULL;
		}

		if ((io=InterClick(pos,flag))!=NULL)		
		{
			danaeApp.Unlock();
			return io;	
		}

		danaeApp.Unlock();
		return NULL;
	}
	else
	{
		INTERACTIVE_OBJ * io=GetFromInventory(pos);

		if (io) 
			return io;

		if (InInventoryPos(pos)) 
			return NULL;

		if ((io=InterClick(pos,flag))!=NULL)		
			return io;	

		return NULL;
	}
}
extern long cur_rf;
extern unsigned long FALLING_TIME;
extern long ARX_NPC_ApplyCuts(INTERACTIVE_OBJ * io);

//*************************************************************************************

void LoadSysTextures()
{
	char temp[256];

	long i;

	for (i=1;i<10;i++)
	{
		sprintf(temp,"Graph\\Particles\\shine%d.bmp",i);
		flaretc.shine[i]=_GetTexture_NoRefinement(temp);

	}

	memset(spellicons, 0, sizeof(SPELL_ICON) * SPELL_COUNT);

	for (i=0;i<SPELL_COUNT;i++)
	{
		for (long j = 0; j < 6; j++) spellicons[i].symbols[j] = 255;

		spellicons[i].level = 0;
		spellicons[i].spellid = 0;
		spellicons[i].tc = NULL;
		spellicons[i].bSecret = false;
		spellicons[i].bDuration = true;
		spellicons[i].bAudibleAtStart = false;
	}

	SPELL_ICON * current;

	// Magic_Sight Level 1
	current=&spellicons[SPELL_MAGIC_SIGHT];	
	ARX_Allocate_Text(current->name, _T("system_spell_name_magic_sight"));
	ARX_Allocate_Text(current->description, _T("system_spell_description_magic_sight"));
	current->level=1;
	current->spellid=SPELL_MAGIC_SIGHT;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_magic_sight.bmp");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_VISTA;

	// Magic_Missile Level 1
	current=&spellicons[SPELL_MAGIC_MISSILE];	
	ARX_Allocate_Text(current->name, _T("system_spell_name_magic_projectile"));
	ARX_Allocate_Text(current->description, _T("system_spell_description_magic_projectile"));
	current->level=1;
	current->spellid=SPELL_MAGIC_MISSILE;
	current->bDuration = false;
	current->bAudibleAtStart = true;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_magic_missile.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_TAAR;

	// Ignit Level 1
	current=&spellicons[SPELL_IGNIT];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_ignit"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_ignit"));
	current->level=1;
	current->spellid=SPELL_IGNIT;
	current->bDuration = false;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_ignite.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_YOK;
	
	// Douse Level 1
	current=&spellicons[SPELL_DOUSE];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_douse"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_douse"));
	current->level=1;
	current->spellid=SPELL_DOUSE;
	current->bDuration = false;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_douse.bmp");
	current->symbols[0]=RUNE_NHI;
	current->symbols[1]=RUNE_YOK;

	// Activate_Portal Level 1
	current=&spellicons[SPELL_ACTIVATE_PORTAL];
	ARX_Allocate_Text(current->name,_T("system_spell_name_activate_portal"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_activate_portal"));
	current->level=1;
	current->spellid=SPELL_ACTIVATE_PORTAL;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_activate_portal.bmp");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_SPACIUM;
	current->bSecret = true;

	// Heal Level 2
	current=&spellicons[SPELL_HEAL];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_heal"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_heal"));
	current->level=2;
	current->spellid=SPELL_HEAL;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_heal.bmp");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_VITAE;

	// Detect_trap Level 2
	current=&spellicons[SPELL_DETECT_TRAP];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_detect_trap"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_detect_trap"));
	current->level=2;
	current->spellid=SPELL_DETECT_TRAP;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_detect_trap.bmp");
	current->symbols[0]=RUNE_MORTE;
	current->symbols[1]=RUNE_COSUM;
	current->symbols[2]=RUNE_VISTA;

	// Armor Level 2
	current=&spellicons[SPELL_ARMOR];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_armor"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_armor"));
	current->level=2;
	current->spellid=SPELL_ARMOR;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_armor.bmp");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_KAOM;

	// Lower Armor Level 2
	current=&spellicons[SPELL_LOWER_ARMOR];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_lower_armor"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_lower_armor"));
	current->level=2;
	current->spellid=SPELL_LOWER_ARMOR;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_lower_armor.bmp");
	current->symbols[0]=RUNE_RHAA;
	current->symbols[1]=RUNE_KAOM;

	// Harm Level 2
	current=&spellicons[SPELL_HARM];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_harm"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_harm"));
	current->level=2;
	current->spellid=SPELL_HARM;
	current->bAudibleAtStart = true;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_harm.bmp");
	current->symbols[0]=RUNE_RHAA;
	current->symbols[1]=RUNE_VITAE;
	current->bSecret = true;

	// Speed Level 3
	current=&spellicons[SPELL_SPEED];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_speed"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_speed"));
	current->level=3;
	current->spellid=SPELL_SPEED;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_speed.bmp");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_MOVIS;

	// Reveal Level 3
	current=&spellicons[SPELL_DISPELL_ILLUSION];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_reveal"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_reveal"));
	current->level=3;
	current->spellid=SPELL_DISPELL_ILLUSION;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_reveal.bmp");
	current->symbols[0]=RUNE_NHI;
	current->symbols[1]=RUNE_STREGUM;
	current->symbols[2]=RUNE_VISTA;

	// Fireball Level 3
	current=&spellicons[SPELL_FIREBALL];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_fireball"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_fireball"));
	current->level=3;
	current->spellid=SPELL_FIREBALL;
	current->bDuration = false;
	current->bAudibleAtStart = true;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_fireball.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_YOK;
	current->symbols[2]=RUNE_TAAR;

	// Create Food Level 3
	current=&spellicons[SPELL_CREATE_FOOD];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_create_food"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_create_food"));
	current->level=3;
	current->spellid=SPELL_CREATE_FOOD;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_create_food.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_VITAE;
	current->symbols[2]=RUNE_COSUM;

	// Ice Projectile Level 3
	current=&spellicons[SPELL_ICE_PROJECTILE];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_ice_projectile"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_ice_projectile"));
	current->level=3;
	current->spellid=SPELL_ICE_PROJECTILE;
	current->bDuration = false;
	current->bAudibleAtStart = true;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_iceball.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_FRIDD;
	current->symbols[2]=RUNE_TAAR;
	current->bSecret = true;

	// Bless Level 4
	current=&spellicons[SPELL_BLESS];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_sanctify"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_sanctify"));
	current->level=4;
	current->spellid=SPELL_BLESS;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_bless.bmp");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_STREGUM;
	current->symbols[2]=RUNE_VITAE;

	// Dispel_Field Level 4
	current=&spellicons[SPELL_DISPELL_FIELD];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_dispell_field"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_dispell_field"));
	current->level=4;
	current->spellid=SPELL_DISPELL_FIELD;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_dispell_field.bmp");
	current->symbols[0]=RUNE_NHI;

	current->symbols[1]=RUNE_SPACIUM;

	// Cold Protection Level 4
	current=&spellicons[SPELL_COLD_PROTECTION];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_cold_protection"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_cold_protection"));
	current->level=4;
	current->spellid=SPELL_COLD_PROTECTION;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_protection_cold.bmp");
	current->symbols[0]=RUNE_FRIDD;
	current->symbols[1]=RUNE_KAOM;
	current->bSecret = true;

	// Fire Protection Level 4
	current=&spellicons[SPELL_FIRE_PROTECTION];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_fire_protection"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_fire_protection"));
	current->level=4;
	current->spellid=SPELL_FIRE_PROTECTION;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_protection_fire.bmp");
	current->symbols[0]=RUNE_YOK;
	current->symbols[1]=RUNE_KAOM;

	// Telekinesis Level 4
	current=&spellicons[SPELL_TELEKINESIS];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_telekinesis"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_telekinesis"));
	current->level=4;
	current->spellid=SPELL_TELEKINESIS;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_telekinesis.bmp");
	current->symbols[0]=RUNE_SPACIUM;
	current->symbols[1]=RUNE_COMUNICATUM;

	// Curse Level 4
	current=&spellicons[SPELL_CURSE];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_curse"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_curse"));
	current->level=4;
	current->spellid=SPELL_CURSE;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_curse.bmp");
	current->symbols[0]=RUNE_RHAA;
	current->symbols[1]=RUNE_STREGUM;
	current->symbols[2]=RUNE_VITAE;
	current->bSecret = true;

	// Rune of Guarding Level 5
	current=&spellicons[SPELL_RUNE_OF_GUARDING];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_rune_guarding"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_rune_guarding"));
	current->level=5;
	current->spellid=SPELL_RUNE_OF_GUARDING;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_rune_guarding.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_MORTE;
	current->symbols[2]=RUNE_COSUM;
	
	// Levitate Level 5
	current=&spellicons[SPELL_LEVITATE];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_levitate"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_levitate"));
	current->level=5;
	current->spellid=SPELL_LEVITATE;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_levitate.bmp");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_SPACIUM;
	current->symbols[2]=RUNE_MOVIS;
	
	// Cure Poison Level 5
	current=&spellicons[SPELL_CURE_POISON];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_cure_poison"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_cure_poison"));
	current->level=5;
	current->spellid=SPELL_CURE_POISON;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_cure_poison.bmp");
	current->symbols[0]=RUNE_NHI;
	current->symbols[1]=RUNE_CETRIUS;

	// Repel Undead Level 5
	current=&spellicons[SPELL_REPEL_UNDEAD];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_repel_undead"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_repel_undead"));
	current->level=5;
	current->spellid=SPELL_REPEL_UNDEAD;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_repel_undead.bmp");
	current->symbols[0]=RUNE_MORTE;
	current->symbols[1]=RUNE_KAOM;

	// Poison Projection Level 5
	current=&spellicons[SPELL_POISON_PROJECTILE];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_poison_projection"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_poison_projection"));
	current->level=5;
	current->spellid=SPELL_POISON_PROJECTILE;
	current->bDuration = false;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_poison_projection.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_CETRIUS;
	current->symbols[2]=RUNE_TAAR;
	current->bSecret = true;

	// Raise Dead Level 6
	current=&spellicons[SPELL_RISE_DEAD];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_raise_dead"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_raise_dead"));
	current->level=6;
	current->spellid=SPELL_RISE_DEAD;
	current->bAudibleAtStart = true;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_raise_dead.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_MORTE;
	current->symbols[2]=RUNE_VITAE;

	// Paralyse Dead Level 6
	current=&spellicons[SPELL_PARALYSE];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_paralyse"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_paralyse"));
	current->level=6;
	current->spellid=SPELL_PARALYSE;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_paralyse.bmp");
	current->symbols[0]=RUNE_NHI;
	current->symbols[1]=RUNE_MOVIS;
	
	// Create Field Dead Level 6
	current=&spellicons[SPELL_CREATE_FIELD];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_create_field"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_create_field"));
	current->level=6;
	current->spellid=SPELL_CREATE_FIELD;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_create_field.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_KAOM;
	current->symbols[2]=RUNE_SPACIUM;
	
	// Disarm Trap Level 6
	current=&spellicons[SPELL_DISARM_TRAP];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_disarm_trap"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_disarm_trap"));
	current->level=6;
	current->spellid=SPELL_DISARM_TRAP;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_disarm_trap.bmp");
	current->symbols[0]=RUNE_NHI;
	current->symbols[1]=RUNE_MORTE;
	current->symbols[2]=RUNE_COSUM;
	
	// Slow_Down Level 6 // SECRET SPELL
	current=&spellicons[SPELL_SLOW_DOWN];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_slowdown"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_slowdown"));
	current->level=6;
	current->spellid=SPELL_SLOW_DOWN;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_slow_down.bmp");
	current->symbols[0]=RUNE_RHAA;
	current->symbols[1]=RUNE_MOVIS;
	current->bSecret = true;

	// Flying Eye Level 7
	current=&spellicons[SPELL_FLYING_EYE];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_flying_eye"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_flying_eye"));
	current->level=7;
	current->spellid=SPELL_FLYING_EYE;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_flying_eye.bmp");
	current->symbols[0]=RUNE_VISTA;
	current->symbols[1]=RUNE_MOVIS;

	// Fire Field Eye Level 7
	current=&spellicons[SPELL_FIRE_FIELD];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_fire_field"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_fire_field"));
	current->level=7;
	current->spellid=SPELL_FIRE_FIELD;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_create_fire_field.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_YOK;
	current->symbols[2]=RUNE_SPACIUM;
	
	// Ice Field Level 7
	current=&spellicons[SPELL_ICE_FIELD];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_ice_field"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_ice_field"));
	current->level=7;
	current->spellid=SPELL_ICE_FIELD;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_create_cold_field.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_FRIDD;
	current->symbols[2]=RUNE_SPACIUM;	
	current->bSecret = true;

	// Lightning Strike Level 7
	current=&spellicons[SPELL_LIGHTNING_STRIKE];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_lightning_strike"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_lightning_strike"));
	current->level=7;
	current->spellid=SPELL_LIGHTNING_STRIKE;
	current->bDuration = false;
	current->bAudibleAtStart = true;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_lightning_strike.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_FOLGORA;
	current->symbols[2]=RUNE_TAAR;
	
	// Confusion Level 7
	current=&spellicons[SPELL_CONFUSE];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_confuse"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_confuse"));
	current->level=7;
	current->spellid=SPELL_CONFUSE;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_confuse.bmp");
	current->symbols[0]=RUNE_RHAA;
	current->symbols[1]=RUNE_VISTA;

	// Invisibility Level 8
	current=&spellicons[SPELL_INVISIBILITY];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_invisibility"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_invisibility"));
	current->level=8;
	current->spellid=SPELL_INVISIBILITY;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_invisibility.bmp");
	current->symbols[0]=RUNE_NHI;
	current->symbols[1]=RUNE_VISTA;

	// Mana Drain Level 8
	current=&spellicons[SPELL_MANA_DRAIN];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_mana_drain"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_mana_drain"));
	current->level=8;
	current->spellid=SPELL_MANA_DRAIN;
	current->bAudibleAtStart = true;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_drain_mana.bmp");
	current->symbols[0]=RUNE_STREGUM;
	current->symbols[1]=RUNE_MOVIS;

	// Explosion Level 8
	current=&spellicons[SPELL_EXPLOSION];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_explosion"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_explosion"));
	current->level=8;
	current->spellid=SPELL_EXPLOSION;
	current->bDuration = false;
	current->bAudibleAtStart = true;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_explosion.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_MEGA;
	current->symbols[2]=RUNE_MORTE;
	
	// Enchant Weapon Level 8
	current=&spellicons[SPELL_ENCHANT_WEAPON];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_enchant_weapon"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_enchant_weapon"));
	current->level=8;
	current->spellid=SPELL_ENCHANT_WEAPON;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_enchant_weapon.bmp");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_STREGUM;
	current->symbols[2]=RUNE_COSUM;
	
	// Life Drain Level 8 // SECRET SPELL
	current=&spellicons[SPELL_LIFE_DRAIN];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_life_drain"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_life_drain"));
	current->level=8;
	current->spellid=SPELL_LIFE_DRAIN;
	current->bAudibleAtStart = true;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_drain_life.bmp");
	current->symbols[0]=RUNE_VITAE;
	current->symbols[1]=RUNE_MOVIS;
	current->bSecret = true;

	// Summon Creature Level 9
	current=&spellicons[SPELL_SUMMON_CREATURE];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_summon_creature"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_summon_creature"));
	current->level=9;
	current->spellid=SPELL_SUMMON_CREATURE;
	current->bAudibleAtStart = true;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_summon_creature.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_VITAE;
	current->symbols[2]=RUNE_TERA;
	
	// FAKE Summon Creature Level 9
	current=&spellicons[SPELL_FAKE_SUMMON];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_summon_creature"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_summon_creature"));
	current->level=9;
	current->spellid=SPELL_FAKE_SUMMON;
	current->bAudibleAtStart = true;
	current->bSecret = true;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_summon_creature.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_VITAE;
	current->symbols[2]=RUNE_TERA;
	
	// Negate Magic Level 9
	current=&spellicons[SPELL_NEGATE_MAGIC];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_negate_magic"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_negate_magic"));
	current->level=9;
	current->spellid=SPELL_NEGATE_MAGIC;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_negate_magic.bmp");
	current->symbols[0]=RUNE_NHI;
	current->symbols[1]=RUNE_STREGUM;
	current->symbols[2]=RUNE_SPACIUM;
	
	// Incinerate Level 9
	current=&spellicons[SPELL_INCINERATE];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_incinerate"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_incinerate"));
	current->level=9;
	current->spellid=SPELL_INCINERATE;
	current->bDuration = false;
	current->bAudibleAtStart = true;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_incinerate.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_MEGA;
	current->symbols[2]=RUNE_YOK;
	
	// Mass paralyse Creature Level 9
	current=&spellicons[SPELL_MASS_PARALYSE];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_mass_paralyse"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_mass_paralyse"));
	current->level=9;
	current->spellid=SPELL_MASS_PARALYSE;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_mass_paralyse.bmp");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_NHI;
	current->symbols[2]=RUNE_MOVIS;
	
	// Mass Lightning Strike Level 10
	current=&spellicons[SPELL_MASS_LIGHTNING_STRIKE];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_mass_lightning_strike"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_mass_lightning_strike"));
	current->level=10;
	current->spellid=SPELL_MASS_LIGHTNING_STRIKE;
	current->bDuration = false;
	current->bAudibleAtStart = true;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_mass_lighting_strike.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_FOLGORA;
	current->symbols[2]=RUNE_SPACIUM;

	// Control Target Level 10
	current=&spellicons[SPELL_CONTROL_TARGET];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_control_target"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_control_target"));
	current->level=10;
	current->spellid=SPELL_CONTROL_TARGET;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_control_target.bmp");
	current->symbols[0]=RUNE_MOVIS;
	current->symbols[1]=RUNE_COMUNICATUM;

	// Freeze time Level 10
	current=&spellicons[SPELL_FREEZE_TIME];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_freeze_time"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_freeze_time"));
	current->level=10;
	current->spellid=SPELL_FREEZE_TIME;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_freeze_time.bmp");
	current->symbols[0] = RUNE_RHAA;
	current->symbols[1]=RUNE_TEMPUS;

	// Mass incinerate Level 10
	current=&spellicons[SPELL_MASS_INCINERATE];	
	ARX_Allocate_Text(current->name,_T("system_spell_name_mass_incinerate"));
	ARX_Allocate_Text(current->description,_T("system_spell_description_mass_incinerate"));
	current->level=10;
	current->spellid=SPELL_MASS_INCINERATE;
	current->bDuration = false;
	current->bAudibleAtStart = true;
	current->tc=_GetTexture_NoRefinement("Graph\\Interface\\Icons\\Spell_mass_incinerate.bmp");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_AAM;
	current->symbols[2]=RUNE_MEGA;
	current->symbols[3]=RUNE_YOK;
	
	Flying_Eye=			_GetTexture_NoRefinement("Graph\\particles\\Flying_Eye_Fx.bmp");
	specular=			_GetTexture_NoRefinement("Graph\\particles\\specular.bmp");
	enviro=				_GetTexture_NoRefinement("Graph\\particles\\enviro.bmp");
	sphere_particle=	_GetTexture_NoRefinement("Graph\\particles\\sphere.bmp");
	inventory_font=		_GetTexture_NoRefinement("Graph\\interface\\font\\font10x10_inventory.bmp");
	npc_fight=			_GetTexture_NoRefinement("Graph\\interface\\icons\\follower_attack.bmp");
	npc_follow=			_GetTexture_NoRefinement("Graph\\interface\\icons\\follower_follow.bmp");
	npc_stop=			_GetTexture_NoRefinement("Graph\\interface\\icons\\follower_stop.bmp");
	flaretc.lumignon=	_GetTexture_NoRefinement("Graph\\Particles\\lumignon.bmp");
	flaretc.lumignon2=	_GetTexture_NoRefinement("Graph\\Particles\\lumignon2.bmp");
	flaretc.plasm=		_GetTexture_NoRefinement("Graph\\Particles\\plasm.bmp");
	tflare=				_GetTexture_NoRefinement("Graph\\Particles\\flare.bmp");
	ombrignon=			_GetTexture_NoRefinement("Graph\\particles\\ombrignon.bmp");
	teleportae=			_GetTexture_NoRefinement("Graph\\particles\\teleportae.bmp");
	TC_fire=			_GetTexture_NoRefinement("Graph\\particles\\fire.bmp");
	TC_fire2=			_GetTexture_NoRefinement("Graph\\particles\\fire2.bmp");
	TC_smoke=			_GetTexture_NoRefinement("Graph\\particles\\smoke.bmp");
	zbtex=				_GetTexture_NoRefinement("Graph\\particles\\zbtex.bmp");
	TC_missile=			_GetTexture_NoRefinement("Graph\\particles\\missile.bmp");
	Z_map=				_GetTexture_NoRefinement("Graph\\interface\\misc\\z-map.bmp");
	Boom=				_GetTexture_NoRefinement("Graph\\Particles\\boom.bmp");
	lightsource_tc=		_GetTexture_NoRefinement("Graph\\Particles\\light.bmp");
	stealth_gauge_tc=	_GetTexture_NoRefinement("Graph\\interface\\Icons\\Stealth_Gauge.bmp");
	arx_logo_tc=		_GetTexture_NoRefinement("Graph\\interface\\Icons\\Arx_logo_32.bmp");
	iconequip[0]=		_GetTexture_NoRefinement("Graph\\interface\\Icons\\equipment_sword.bmp");
	iconequip[1]=		_GetTexture_NoRefinement("Graph\\interface\\Icons\\equipment_shield.bmp");
	iconequip[2]=		_GetTexture_NoRefinement("Graph\\interface\\Icons\\equipment_helm.bmp");
	iconequip[3]=		_GetTexture_NoRefinement("Graph\\interface\\Icons\\equipment_chest.bmp");
	iconequip[4]=		_GetTexture_NoRefinement("Graph\\interface\\Icons\\equipment_leggings.bmp");
	mecanism_tc=		_GetTexture_NoRefinement("Graph\\interface\\Cursors\\Mecanism.bmp");
	arrow_left_tc=		_GetTexture_NoRefinement("Graph\\interface\\Icons\\Arrow_left.bmp");

	for (i=0;i<MAX_EXPLO;i++)
	{
		char temp[256];
		sprintf(temp,"Graph\\Particles\\fireb_%02d.bmp",i+1);
		explo[i]= _GetTexture_NoRefinement(temp);
	}

	blood_splat=_GetTexture_NoRefinement("Graph\\Particles\\new_blood2.bmp");

	EERIE_DRAW_SetTextureZMAP(0,Z_map);
	EERIE_DRAW_sphere_particle=sphere_particle;
	EERIE_DRAW_square_particle=_GetTexture_NoRefinement("Graph\\particles\\square.bmp");

	GetTextureFile_NoRefinement("Graph\\Particles\\fire_hit.bmp");
	GetTextureFile_NoRefinement("Graph\\Particles\\light.bmp");
	GetTextureFile_NoRefinement("Graph\\Particles\\blood01.bmp");
	GetTextureFile_NoRefinement("Graph\\Particles\\cross.bmp");

	//INTERFACE LOADING
	GetTextureFile_NoRefinement("Graph\\interface\\bars\\Empty_gauge_Red.bmp");
	GetTextureFile_NoRefinement("Graph\\interface\\bars\\Empty_gauge_Blue.bmp");
	GetTextureFile_NoRefinement("Graph\\interface\\bars\\Filled_gauge_Blue.bmp");
	GetTextureFile_NoRefinement("Graph\\interface\\bars\\Filled_gauge_Red.bmp");
	GetTextureFile_NoRefinement("Graph\\Interface\\Icons\\Book.bmp");
	GetTextureFile_NoRefinement("Graph\\Interface\\Icons\\Backpack.bmp");
	GetTextureFile_NoRefinement("Graph\\Interface\\Icons\\Lvl_Up.bmp");
	GetTextureFile_NoRefinement("Graph\\Interface\\Icons\\Steal.bmp");
	GetTextureFile_NoRefinement("Graph\\Interface\\Icons\\cant_steal_item.bmp");
	GetTextureFile_NoRefinement("Graph\\Interface\\Inventory\\hero_inventory.bmp");
	GetTextureFile_NoRefinement("Graph\\Interface\\Inventory\\scroll_up.bmp");
	GetTextureFile_NoRefinement("Graph\\Interface\\Inventory\\scroll_down.bmp");
	GetTextureFile_NoRefinement("Graph\\Interface\\Inventory\\Hero_inventory_link.bmp");
	GetTextureFile_NoRefinement("Graph\\Interface\\Inventory\\ingame_inventory.bmp");
	GetTextureFile_NoRefinement("Graph\\Interface\\Inventory\\ingame_sub_inv.bmp");
	GetTextureFile_NoRefinement("Graph\\Interface\\Inventory\\Gold.bmp");

	GetTextureFile_NoRefinement("Graph\\Interface\\Inventory\\inv_pick.bmp");
	GetTextureFile_NoRefinement("Graph\\Interface\\Inventory\\inv_close.bmp");

	// MENU2
	GetTextureFile_NoRefinement("graph\\interface\\cursors\\cursor00.bmp");
	GetTextureFile_NoRefinement("graph\\interface\\cursors\\cursor01.bmp");
	GetTextureFile_NoRefinement("graph\\interface\\cursors\\cursor02.bmp");
	GetTextureFile_NoRefinement("graph\\interface\\cursors\\cursor03.bmp");
	GetTextureFile_NoRefinement("graph\\interface\\cursors\\cursor04.bmp");
	GetTextureFile_NoRefinement("graph\\interface\\cursors\\cursor05.bmp");
	GetTextureFile_NoRefinement("graph\\interface\\cursors\\cursor06.bmp");
	GetTextureFile_NoRefinement("graph\\interface\\cursors\\cursor07.bmp");
	GetTextureFile_NoRefinement("graph\\interface\\cursors\\cruz.bmp");
	GetTextureFile_NoRefinement("Graph\\Interface\\menus\\menu_main_background.bmp");
	GetTextureFile_NoRefinement("Graph\\interface\\menus\\menu_console_background.bmp");
	GetTextureFile_NoRefinement("Graph\\interface\\menus\\menu_console_background_border.bmp");

	//CURSORS LOADING
	GetTextureFile_NoRefinement("Graph\\Interface\\cursors\\cursor.bmp");
	GetTextureFile_NoRefinement("Graph\\Interface\\cursors\\magic.bmp");
	GetTextureFile_NoRefinement("Graph\\Interface\\cursors\\interaction_on.bmp");
	GetTextureFile_NoRefinement("Graph\\Interface\\cursors\\interaction_off.bmp");
	GetTextureFile_NoRefinement("Graph\\Interface\\cursors\\target_on.bmp");
	GetTextureFile_NoRefinement("Graph\\Interface\\cursors\\target_off.bmp");
	GetTextureFile_NoRefinement("Graph\\Interface\\cursors\\drop.bmp");
	GetTextureFile_NoRefinement("Graph\\Interface\\cursors\\throw.bmp");

	for (i=0;i<8;i++)
	{
		char temp[256];
		sprintf(temp,"Graph\\Interface\\cursors\\cursor%02d.bmp",i);
		scursor[i]=MakeTCFromFile_NoRefinement(temp);
	}

	pTCCrossHair = MakeTCFromFile_NoRefinement("graph\\interface\\cursors\\cruz.bmp");

	GetTextureFile_NoRefinement("Graph\\Interface\\bars\\aim_empty.bmp");
	GetTextureFile_NoRefinement("Graph\\Interface\\bars\\aim_maxi.bmp");
	GetTextureFile_NoRefinement("Graph\\Interface\\bars\\flash_gauge.bmp");
}

void ARX_SOUND_Reinit()
{

}

void ClearSysTextures()
{
	long i;

	for (i=0;i<SPELL_COUNT;i++)
	{
		if (spellicons[i].name)
			free(spellicons[i].name);

		if (spellicons[i].description)
			free(spellicons[i].description);
	}
}

//*************************************************************************************
// OneTimeSceneInit()
//  Called Once during initial app startup
//*************************************************************************************
HRESULT DANAE::OneTimeSceneInit()
{
	ForceSendConsole("DANAE Runnning",1,0,(HWND)danaeApp.m_hWnd);
	return S_OK;
}
EERIE_3D ePos;
extern EERIE_CAMERA * ACTIVECAM;
void LaunchWaitingCine()
{

	if (ACTIVECAM)
	{
		ePos.x = ACTIVECAM->pos.x;
		ePos.y = ACTIVECAM->pos.y;
		ePos.z = ACTIVECAM->pos.z;
	}

	DANAE_KillCinematic();
	char temp1[256];
	char temp2[256];
	sprintf(temp1,"%sGraph\\interface\\illustrations\\",Project.workingdir);
	strcpy(temp2,temp1);
	strcat(temp2,WILL_LAUNCH_CINE);

	if (PAK_FileExist(temp2))
	{			
		ControlCinematique->OneTimeSceneReInit();						

		if (LoadProject(ControlCinematique,temp1,WILL_LAUNCH_CINE))
		{				

			if (CINE_PRELOAD) PLAY_LOADED_CINEMATIC=0;
			else
			{
				PLAY_LOADED_CINEMATIC=1;
				ARX_TIME_Pause();
			}

			strcpy(LAST_LAUNCHED_CINE,WILL_LAUNCH_CINE);
		}

	}

	WILL_LAUNCH_CINE[0]=0;
}
void PlayerLaunchArrow_Test(float aimratio,float poisonous,EERIE_3D * pos,EERIE_3D * angle)
{
	EERIE_3D position;
	EERIE_3D vect;
	EERIE_3D dvect,upvect;
	EERIEMATRIX mat;
	EERIE_QUAT quat;
	float anglea;
	float angleb;
	float velocity;

	position.x=pos->x;
	position.y=pos->y;
	position.z=pos->z;
	anglea=DEG2RAD(angle->a);
	angleb=DEG2RAD(angle->b);
	vect.x=-EEsin(angleb)*EEcos(anglea);
	vect.y= EEsin(anglea);
	vect.z= EEcos(angleb)*EEcos(anglea);
	Vector_Init(&upvect,0,0,-1);
	VRotateX(&upvect,anglea);
	VRotateY(&upvect,angleb);
	Vector_Init(&upvect,0,-1,0);
	VRotateX(&upvect,anglea);
	VRotateY(&upvect,angleb);
	MatrixSetByVectors(&mat,&dvect,&upvect);
	QuatFromMatrix(quat,mat);
	velocity=(aimratio+0.3f);

	if (velocity<0.9f) velocity=0.9f;

	EERIE_3D vv,v1,v2;
	Vector_Init(&vv,0,0,1);	
	float aa=angle->a;
	float ab=90-angle->b;
	Vector_RotateZ(&v1,&vv,aa);
	VRotateY(&v1,ab);
	Vector_Init(&vv,0,-1,0);
	Vector_RotateZ(&v2,&vv,aa);
	VRotateY(&v2,ab);
	EERIEMATRIX tmat;
	MatrixSetByVectors(&tmat,&v1,&v2);
	QuatFromMatrix(quat,tmat);

	float wd=(float)ARX_EQUIPMENT_Apply(
		inter.iobj[0],IO_EQUIPITEM_ELEMENT_Damages,1);

	float weapon_damages=wd;
	
	float damages=
		weapon_damages
		*(1.f+
		(float)(player.Full_Skill_Projectile + player.Full_Attribute_Dexterity )*DIV50);

	ARX_THROWN_OBJECT_Throw(ATO_TYPE_ARROW,
										0, //source
										&position,
										&vect,
										&upvect,
										&quat,
	                        velocity,
										damages,
										poisonous); //damages
}
extern long sp_max;
void PlayerLaunchArrow(float aimratio,float poisonous)
{
	EERIE_3D position;
	EERIE_3D vect;
	EERIE_3D dvect,upvect;
	EERIEMATRIX mat;
	EERIE_QUAT quat;
	float anglea;
	float angleb;
	float velocity;

	if ((sp_max) && (poisonous<3.f))
		poisonous=3.f;

	position.x=player.pos.x;
	position.y=player.pos.y+40.f;
	position.z=player.pos.z;

	if (inter.iobj[0]->obj->fastaccess.left_attach>=0)
	{
		Vector_Copy(&position,&inter.iobj[0]->obj->vertexlist3[inter.iobj[0]->obj->fastaccess.left_attach].v);
	}

	anglea=DEG2RAD(player.angle.a);
	angleb=DEG2RAD(player.angle.b);
	vect.x=-EEsin(angleb)*EEcos(anglea);
	vect.y= EEsin(anglea);
	vect.z= EEcos(angleb)*EEcos(anglea);

	Vector_Init(&upvect,0,0,-1);
	VRotateX(&upvect,anglea);
	VRotateY(&upvect,angleb);

	Vector_Init(&upvect,0,-1,0);
	VRotateX(&upvect,anglea);
	VRotateY(&upvect,angleb);
	MatrixSetByVectors(&mat,&dvect,&upvect);
	QuatFromMatrix(quat,mat);

	velocity=(aimratio+0.3f);

	if (velocity<0.9f) velocity=0.9f;

	EERIE_3D vv,v1,v2;
	Vector_Init(&vv,0,0,1);	
	float aa=player.angle.a;
	float ab=90-player.angle.b;
	Vector_RotateZ(&v1,&vv,aa);
	VRotateY(&v1,ab);
	Vector_Init(&vv,0,-1,0);
	Vector_RotateZ(&v2,&vv,aa);
	VRotateY(&v2,ab);
	EERIEMATRIX tmat;
	MatrixSetByVectors(&tmat,&v1,&v2);
	QuatFromMatrix(quat,tmat);

	float wd=(float)ARX_EQUIPMENT_Apply(
		inter.iobj[0],IO_EQUIPITEM_ELEMENT_Damages,1);

	float weapon_damages=wd;
	
	float damages=
		weapon_damages
		*(1.f+
		(float)(player.Full_Skill_Projectile + player.Full_Attribute_Dexterity )*DIV50);

	ARX_THROWN_OBJECT_Throw(ATO_TYPE_ARROW,
										0, //source
										&position,
										&vect,
										&upvect,
										&quat,
	                        velocity,
										damages,
										poisonous); //damages

	if (sp_max)
	{
		EERIE_3D angle;
		EERIE_3D pos;
		pos.x=player.pos.x;
		pos.y=player.pos.y+40.f;
		pos.z=player.pos.z;
		angle.a=player.angle.a;
		angle.b=player.angle.b+8;
		angle.g=player.angle.g;
		PlayerLaunchArrow_Test(aimratio,poisonous,&pos,&angle);
		angle.a=player.angle.a;
		angle.b=player.angle.b-8;		
		PlayerLaunchArrow_Test(aimratio,poisonous,&pos,&angle);
		angle.a=player.angle.a;
		angle.b=player.angle.b+4.f;		
		PlayerLaunchArrow_Test(aimratio,poisonous,&pos,&angle);
		angle.a=player.angle.a;
		angle.b=player.angle.b-4.f;		
		PlayerLaunchArrow_Test(aimratio,poisonous,&pos,&angle);
	}
}

extern void ARX_POLYSPLAT_Add(EERIE_3D * poss,long type,EERIE_RGB * col,float size,long flags);

//*************************************************************************************
// FrameMove()
//  Called once per frame.
//*************************************************************************************
HRESULT DANAE::FrameMove( FLOAT fTimeKey )
{
	// To disable for demo
	if (	!FINAL_COMMERCIAL_DEMO
		&& !FINAL_COMMERCIAL_GAME
		)
	{
		if (this->kbd.inkey[INKEY_F4]) 
		{
			this->kbd.inkey[INKEY_F4]=0;
			ARX_TIME_Pause();
			DialogBox( (HINSTANCE)GetWindowLong( this->m_hWnd, GWL_HINSTANCE ),
						   MAKEINTRESOURCE(IDD_LEVEL_SELECTOR), this->m_hWnd, ChangeLevelProc );

			if (CHANGE_LEVEL_PROC_RESULT!=-1)
			{
				char levelnum[256];
				char levelname[256];
				GetLevelNameByNum(CHANGE_LEVEL_PROC_RESULT,levelnum);
				sprintf(levelname,"LEVEL%s",levelnum);
				char leveltarget[256];
				strcpy(leveltarget,"no");

				ARX_CHECK_LONG( player.angle.b );
				ARX_CHANGELEVEL_Change( levelname, leveltarget, ARX_CLEAN_WARN_CAST_LONG( player.angle.b ), 0 );

			}

			ARX_TIME_UnPause();
		}	

		// To Remove For Final Release !!!
		if (ALLOW_CHEATS || GAME_EDITOR)
		{
			if (this->kbd.inkey[INKEY_F2]) 
			{
				this->kbd.inkey[INKEY_F2]=0;
				ARX_TIME_Pause();
				Pause(TRUE);
				DialogBox( (HINSTANCE)GetWindowLong( this->m_hWnd, GWL_HINSTANCE ),
					MAKEINTRESOURCE(IDD_OPTIONS), this->m_hWnd, OptionsProc );
				EERIE_LIGHT_ChangeLighting();
				Pause(FALSE);
				ARX_TIME_UnPause();
			}
			else if (this->kbd.inkey[INKEY_F3]) 
			{
				this->kbd.inkey[INKEY_F3]=0;
				ARX_TIME_Pause();
				Pause(TRUE);
				DialogBox( (HINSTANCE)GetWindowLong( this->m_hWnd, GWL_HINSTANCE ),
							MAKEINTRESOURCE(IDD_OPTIONS2), this->m_hWnd, OptionsProc_2 );
				Pause(FALSE);
				ARX_TIME_UnPause();
			}
			else if (this->kbd.inkey[INKEY_O]) 
			{
				this->kbd.inkey[INKEY_O]=0;
				ARX_SOUND_Reinit();
			}
		}
	}
	
	if (WILL_LAUNCH_CINE[0]) // Checks if a cinematic is waiting to be played...
	{
		LaunchWaitingCine();	
	}

    return S_OK;
}

extern unsigned long LAST_JUMP_ENDTIME;

//*************************************************************************************
// Switches from/to Game Mode/Editor Mode
//*************************************************************************************
void SetEditMode(long ed, const bool stop_sound)
{
	LAST_JUMP_ENDTIME=0;

	if (!DONT_ERASE_PLAYER)
		player.life=0.1f;

	DeadTime=0;
	ARX_GAME_Reset(1);

	EERIEMouseButton=0;

	if (ed)
	{
		EDITMODE=1;

		if(	((danaeApp.m_pFramework)&&
			(danaeApp.m_pFramework->m_bIsFullscreen))||
			(FINAL_COMMERCIAL_GAME) )
		{
			USE_OLD_MOUSE_SYSTEM=0;
		}
		else
		{
			USE_OLD_MOUSE_SYSTEM=1;
		}
	}
	else
	{
		EDITMODE=0;
		USE_OLD_MOUSE_SYSTEM=0;
	}	

	for (long i=0;i<inter.nbmax;i++)
	{
		if (inter.iobj[i]!=NULL )
		{
			if (inter.iobj[i]->show == SHOW_FLAG_HIDDEN) inter.iobj[i]->show = SHOW_FLAG_IN_SCENE;
			else if (inter.iobj[i]->show == SHOW_FLAG_KILLED) inter.iobj[i]->show = SHOW_FLAG_IN_SCENE;
		}
	}

	RestoreAllLightsInitialStatus();

	if (stop_sound) ARX_SOUND_MixerStop(ARX_SOUND_MixerGame);

	RestoreInitialIOStatus();

	if (ed)
	{
		ARX_PATH_ComputeAllBoundingBoxes();

		ARX_TIME_Pause();
	}
	else
	{
		ARX_SCRIPT_ResetAll(1);			
		EERIE_ANIMMANAGER_PurgeUnused();
	}

	if (!DONT_ERASE_PLAYER)
	{
		if (	(!FINAL_RELEASE) 
			&&	(!FINAL_COMMERCIAL_GAME)
			&&	(!FINAL_COMMERCIAL_DEMO))
			ARX_PLAYER_MakePowerfullHero();
		else
			ARX_PLAYER_MakeFreshHero();
	}
}

//-----------------------------------------------------------------------------

void DANAE_ReleaseAllDatasDynamic()
{

	if(ssol)
	{
		ReleaseEERIE3DObj(ssol);
		ssol=NULL;
		ssol_count=0;
	}

	if(slight)
	{
		ReleaseEERIE3DObj(slight);
		slight=NULL;
		slight_count=0;
	}

	if(srune)
	{
		ReleaseEERIE3DObj(srune);
		srune=NULL;
		srune_count=0;
	}

	if(smotte)
	{
		ReleaseEERIE3DObj(smotte);
		smotte=NULL;
		smotte_count=0;
	}

	if(stite)
	{
		ReleaseEERIE3DObj(stite);
		stite=NULL;
		stite_count=0;
	}

	if(smissile)
	{
		ReleaseEERIE3DObj(smissile);
		smissile=NULL;
		smissile_count=0;
	}

	if(spapi)
	{
		ReleaseEERIE3DObj(spapi);
		spapi=NULL;
		spapi_count=0;
	}

	if(sfirewave)
	{
		ReleaseEERIE3DObj(sfirewave);
		sfirewave=NULL;
		sfirewave_count=0;
	}

	if(svoodoo)
	{
		ReleaseEERIE3DObj(svoodoo);
		svoodoo=NULL;
		svoodoo_count=0;
	}
}

//-----------------------------------------------------------------------------

void ReleaseDanaeBeforeRun()
{
	if(necklace.lacet)
	{
		ReleaseEERIE3DObj(necklace.lacet);
		necklace.lacet=NULL;
	}

	for (long i=0; i<20; i++)
	{
		if(necklace.runes[i])
		{
			ReleaseEERIE3DObj(necklace.runes[i]);
			necklace.runes[i]=NULL;
		}

		if (necklace.pTexTab[i])
		{

			necklace.pTexTab[i]=NULL;
		}
	}

	if(eyeballobj)
	{
		ReleaseEERIE3DObj(eyeballobj);
		eyeballobj=NULL;
	}

	if(cabal)
	{
		ReleaseEERIE3DObj(cabal);
		cabal=NULL;
	}

	if(nodeobj)
	{
		ReleaseEERIE3DObj(nodeobj);
		nodeobj=NULL;
	}

	if(fogobj)
	{
		ReleaseEERIE3DObj(fogobj);
		fogobj=NULL;
	}

	if(cameraobj)
	{
		ReleaseEERIE3DObj(cameraobj);
		cameraobj=NULL;
	}

	if(markerobj)
	{
		ReleaseEERIE3DObj(markerobj);
		markerobj=NULL;
	}

	if(arrowobj)
	{
		ReleaseEERIE3DObj(arrowobj);
		arrowobj=NULL;
	}

	for (int i=0;i<MAX_GOLD_COINS_VISUALS;i++)
	{
		if(GoldCoinsObj[i])
		{
			ReleaseEERIE3DObj(GoldCoinsObj[i]);
			GoldCoinsObj[i]=NULL;
		}
	}

}

HRESULT DANAE::BeforeRun()
{
	long i;

	GDevice=m_pd3dDevice;

	ControlCinematique = new CINEMATIQUE(danaeApp.m_pFramework->GetD3DDevice(), danaeApp.m_pFramework->m_dwRenderWidth, danaeApp.m_pFramework->m_dwRenderHeight);
	memset(&necklace,0,sizeof(ARX_NECKLACE));
	long old=GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE;
	GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE=-1;
	necklace.lacet=						_LoadTheObj("Graph\\Interface\\book\\runes\\lacet.teo","..\\..\\..\\Obj3D\\textures\\");
	necklace.runes[RUNE_AAM]=			_LoadTheObj("Graph\\Interface\\book\\runes\\runes_aam.teo","..\\..\\..\\Obj3D\\textures\\");
	necklace.runes[RUNE_CETRIUS]=		_LoadTheObj("Graph\\Interface\\book\\runes\\runes_citrius.teo","..\\..\\..\\Obj3D\\textures\\");
	necklace.runes[RUNE_COMUNICATUM]=	_LoadTheObj("Graph\\Interface\\book\\runes\\runes_comunicatum.teo","..\\..\\..\\Obj3D\\textures\\");
	necklace.runes[RUNE_COSUM]=			_LoadTheObj("Graph\\Interface\\book\\runes\\runes_cosum.teo","..\\..\\..\\Obj3D\\textures\\");
	necklace.runes[RUNE_FOLGORA]=		_LoadTheObj("Graph\\Interface\\book\\runes\\runes_folgora.teo","..\\..\\..\\Obj3D\\textures\\");
	necklace.runes[RUNE_FRIDD]=			_LoadTheObj("Graph\\Interface\\book\\runes\\runes_fridd.teo","..\\..\\..\\Obj3D\\textures\\");
	necklace.runes[RUNE_KAOM]=			_LoadTheObj("Graph\\Interface\\book\\runes\\runes_kaom.teo","..\\..\\..\\Obj3D\\textures\\");
	necklace.runes[RUNE_MEGA]=			_LoadTheObj("Graph\\Interface\\book\\runes\\runes_mega.teo","..\\..\\..\\Obj3D\\textures\\");
	necklace.runes[RUNE_MORTE]=			_LoadTheObj("Graph\\Interface\\book\\runes\\runes_morte.teo","..\\..\\..\\Obj3D\\textures\\");
	necklace.runes[RUNE_MOVIS]=			_LoadTheObj("Graph\\Interface\\book\\runes\\runes_movis.teo","..\\..\\..\\Obj3D\\textures\\");
	necklace.runes[RUNE_NHI]=			_LoadTheObj("Graph\\Interface\\book\\runes\\runes_nhi.teo","..\\..\\..\\Obj3D\\textures\\");
	necklace.runes[RUNE_RHAA]=			_LoadTheObj("Graph\\Interface\\book\\runes\\runes_rhaa.teo","..\\..\\..\\Obj3D\\textures\\");
	necklace.runes[RUNE_SPACIUM]=		_LoadTheObj("Graph\\Interface\\book\\runes\\runes_spacium.teo","..\\..\\..\\Obj3D\\textures\\");
	necklace.runes[RUNE_STREGUM]=		_LoadTheObj("Graph\\Interface\\book\\runes\\runes_stregum.teo","..\\..\\..\\Obj3D\\textures\\");
	necklace.runes[RUNE_TAAR]=			_LoadTheObj("Graph\\Interface\\book\\runes\\runes_taar.teo","..\\..\\..\\Obj3D\\textures\\");
	necklace.runes[RUNE_TEMPUS]=		_LoadTheObj("Graph\\Interface\\book\\runes\\runes_tempus.teo","..\\..\\..\\Obj3D\\textures\\");
	necklace.runes[RUNE_TERA]=			_LoadTheObj("Graph\\Interface\\book\\runes\\runes_tera.teo","..\\..\\..\\Obj3D\\textures\\");
	necklace.runes[RUNE_VISTA]=			_LoadTheObj("Graph\\Interface\\book\\runes\\runes_vista.teo","..\\..\\..\\Obj3D\\textures\\");
	necklace.runes[RUNE_VITAE]=			_LoadTheObj("Graph\\Interface\\book\\runes\\runes_vitae.teo","..\\..\\..\\Obj3D\\textures\\");
	necklace.runes[RUNE_YOK]=			_LoadTheObj("Graph\\Interface\\book\\runes\\runes_yok.teo","..\\..\\..\\Obj3D\\textures\\");

	necklace.pTexTab[RUNE_AAM]			= MakeTCFromFile_NoRefinement("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_Aam[icon].BMP");
	necklace.pTexTab[RUNE_CETRIUS]		= MakeTCFromFile_NoRefinement("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_cetrius[icon].BMP");
	necklace.pTexTab[RUNE_COMUNICATUM]	= MakeTCFromFile_NoRefinement("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_comunicatum[icon].BMP");
	necklace.pTexTab[RUNE_COSUM]		= MakeTCFromFile_NoRefinement("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_cosum[icon].BMP");
	necklace.pTexTab[RUNE_FOLGORA]		= MakeTCFromFile_NoRefinement("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_folgora[icon].BMP");
	necklace.pTexTab[RUNE_FRIDD]		= MakeTCFromFile_NoRefinement("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_fridd[icon].BMP");
	necklace.pTexTab[RUNE_KAOM]			= MakeTCFromFile_NoRefinement("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_kaom[icon].BMP");
	necklace.pTexTab[RUNE_MEGA]			= MakeTCFromFile_NoRefinement("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_mega[icon].BMP");
	necklace.pTexTab[RUNE_MORTE]		= MakeTCFromFile_NoRefinement("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_morte[icon].BMP");
	necklace.pTexTab[RUNE_MOVIS]		= MakeTCFromFile_NoRefinement("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_movis[icon].BMP");
	necklace.pTexTab[RUNE_NHI]			= MakeTCFromFile_NoRefinement("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_nhi[icon].BMP");
	necklace.pTexTab[RUNE_RHAA]			= MakeTCFromFile_NoRefinement("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_rhaa[icon].BMP");
	necklace.pTexTab[RUNE_SPACIUM]		= MakeTCFromFile_NoRefinement("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_spacium[icon].BMP");
	necklace.pTexTab[RUNE_STREGUM]		= MakeTCFromFile_NoRefinement("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_stregum[icon].BMP");
	necklace.pTexTab[RUNE_TAAR]			= MakeTCFromFile_NoRefinement("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_taar[icon].BMP");
	necklace.pTexTab[RUNE_TEMPUS]		= MakeTCFromFile_NoRefinement("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_tempus[icon].BMP");
	necklace.pTexTab[RUNE_TERA]			= MakeTCFromFile_NoRefinement("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_tera[icon].BMP");
	necklace.pTexTab[RUNE_VISTA]		= MakeTCFromFile_NoRefinement("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_vista[icon].BMP");
	necklace.pTexTab[RUNE_VITAE]		= MakeTCFromFile_NoRefinement("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_vitae[icon].BMP");
	necklace.pTexTab[RUNE_YOK]			= MakeTCFromFile_NoRefinement("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_yok[icon].BMP");

	for (i = 0; i<NB_RUNES-1; i++)
	{
		if (necklace.pTexTab[i])
			necklace.pTexTab[i]->CreateHalo(GDevice);
	}

	EERIE_3DOBJ * _fogobj;
	_fogobj=		_LoadTheObj("Editor\\Obj3D\\fog_generator.teo","node_TEO MAPS\\");
	ARX_FOGS_Set_Object(_fogobj);

	eyeballobj=		_LoadTheObj("Editor\\Obj3D\\eyeball.teo","eyeball_TEO MAPS\\");
	cabal=			_LoadTheObj("Editor\\Obj3D\\cabal.teo","cabal_TEO MAPS\\");
	nodeobj=		_LoadTheObj("Editor\\Obj3D\\node.teo","node_TEO MAPS\\");	
	cameraobj=		_LoadTheObj("Graph\\Obj3D\\Interactive\\System\\Camera\\Camera.teo","..\\..\\..\\textures\\");
	markerobj=		_LoadTheObj("Graph\\Obj3D\\Interactive\\System\\Marker\\Marker.teo","..\\..\\..\\textures\\");
	arrowobj=		_LoadTheObj("Graph\\Obj3D\\Interactive\\Items\\Weapons\\arrow\\arrow.teo","..\\..\\..\\..\\textures\\");

	for (i=0;i<MAX_GOLD_COINS_VISUALS;i++)
	{
		char temp[256];

		if (i==0)
			strcpy(temp,	"Graph\\Obj3D\\Interactive\\Items\\Jewelry\\Gold_coin\\Gold_coin.teo");
		else
			sprintf(temp,	"Graph\\Obj3D\\Interactive\\Items\\Jewelry\\Gold_coin\\Gold_coin%d.teo",i+1);

		GoldCoinsObj[i]=	_LoadTheObj(temp,"..\\..\\..\\..\\textures\\");

		if (i==0)
			strcpy(temp,	"Graph\\Obj3D\\Interactive\\Items\\Jewelry\\Gold_coin\\Gold_coin[icon].bmp");
		else
			sprintf(temp,	"Graph\\Obj3D\\Interactive\\Items\\Jewelry\\Gold_coin\\Gold_coin%d[icon].bmp",i+1);

		GoldCoinsTC[i] =	MakeTCFromFile_NoRefinement(temp);
	}	

	Movable=				MakeTCFromFile_NoRefinement("Graph\\Interface\\Cursors\\wrong.bmp");
	ChangeLevel=			MakeTCFromFile_NoRefinement("Graph\\Interface\\Icons\\change_lvl.bmp");	

	ARX_PLAYER_LoadHeroAnimsAndMesh();
 
	GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE=old;

	// Need to create Map
	if (iCreateMap)
		DANAE_Manage_CreateMap();		

	danaeApp.GetZBufferMax();

	ARX_HWTransform_Init(m_pD3D);

	ARX_Localisation_Init();
	return S_OK;
}	

//*************************************************************************************

void FirstTimeThings(HWND m_hWnd,LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	static long done = 0;
	long i;
	eyeball.exist=0;		
	WILLADDSPEECHTIME=0;
	WILLADDSPEECH[0]=0;

	if (!LOADEDD)
	{
		RemoveAllBackgroundActions();			
	}

	_NB_++;	

	for (i=0;i<MAX_DYNLIGHTS;i++)
	{
		if ((DynLight[i].exist))
			DynLight[i].exist=0;
	}

	LastFrameTime=FrameTime;
	done = 1;
	return;
}

//*************************************************************************************

void ExitProc() 
{

	if (danaeApp.m_hWnd!=NULL) 
		SendMessage( danaeApp.m_hWnd, WM_QUIT, 0, 0 );

	exit(0);
}

//*************************************************************************************
	
void SetFilteringMode(LPDIRECT3DDEVICE7 m_pd3dDevice,long mode)
{
	if (POINTINTERPOLATION)
	{
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTFP_POINT  );	
	}
	else 
	{
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTFP_LINEAR  );	
	}

	switch (mode)
	{
		case 1:
			m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_LINEAR );
			m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_LINEAR );
		break;
		case 2:
			m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_ANISOTROPIC );
			m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_ANISOTROPIC );
		break;
		default:
			m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_POINT );
			m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_POINT );	
		break;
	}

	float val=-0.3f;
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MIPMAPLODBIAS, *((LPDWORD) (&val))  );
}
long NO_GMOD_RESET=0;

//*************************************************************************************

LPTHREAD_START_ROUTINE FirstFrameProc(char *pipo)
{
	if (pParticleManager == NULL)
	{
		pParticleManager = new CParticleManager();
	}

	if (!NO_GMOD_RESET)
		ARX_GLOBALMODS_Reset();

	NO_GMOD_RESET=0;

	STARTDRAG.x=0;
	STARTDRAG.y=0;
	DANAEMouse.x=0;
	DANAEMouse.y=0;
	bookclick.x=-1;
	bookclick.y=-1;
	
	if (!LOAD_N_DONT_ERASE) ARX_TIME_Init();

	strcpy(ItemToBeAdded,"");
	ARX_BOOMS_ClearAllPolyBooms();
	ARX_DAMAGES_Reset();
	ARX_MISSILES_ClearAll();
	ARX_SPELLS_ClearAll();
	ARX_SPELLS_ClearAllSymbolDraw();
	ARX_PARTICLES_ClearAll();

	if (!LOAD_N_DONT_ERASE)
	{
		CleanScriptLoadedIO();
		RestoreInitialIOStatus();
		DRAGINTER=NULL;
	}	

	ARX_SPELLS_ResetRecognition();			

	if (DEBUGCODE) 
		ForceSendConsole("...NEXT...",1,0,(HWND)1);
	
	FirstTimeThings(danaeApp.m_hWnd,NULL);		

	if (!LOAD_N_DONT_ERASE)
	{

		CleanInventory();
		ARX_SCRIPT_Timer_ClearAll();
		UnlinkAllLinkedObjects();
		ARX_SCRIPT_ResetAll(0);
	}

	SecondaryInventory=NULL;
	TSecondaryInventory=NULL;
	ARX_FOGS_Render(1);

	if (!LOAD_N_DONT_ERASE)
	{
		ARX_TIME_Init();

		if (!DONT_ERASE_PLAYER) ARX_PLAYER_InitPlayer();

		SLID_VALUE=0.f;
	}

	if (!LOAD_N_DONT_ERASE) 
	{
		player.life=player.maxlife;
		player.mana=player.maxmana;

		if (!DONT_ERASE_PLAYER)
		{
			if (	(!FINAL_RELEASE) 
			&&	(!FINAL_COMMERCIAL_GAME)
			&&	(!FINAL_COMMERCIAL_DEMO))
				ARX_PLAYER_MakePowerfullHero();
			else
				ARX_PLAYER_MakeFreshHero();
		}
	}

	InitSnapShot(NULL,"snapshot");
	return 0;
}
EERIE_3D LastValidPlayerPos;
EERIE_3D	WILL_RESTORE_PLAYER_POSITION;
long WILL_RESTORE_PLAYER_POSITION_FLAG=0;
extern long FLAG_ALLOW_CLOTHES;

//*************************************************************************************

long FirstFrameHandling(LPDIRECT3DDEVICE7 m_pd3dDevice)
{	
	EERIE_3D trans;
	FirstFrame=-1;
		
	ARX_PARTICLES_FirstInit();
	ARX_SPELLS_Init(m_pd3dDevice);
	ARX_FOGS_TimeReset();

	PROGRESS_BAR_COUNT+=2.f;
	LoadLevelScreen();
	
	FirstFrameProc(NULL);

	if (FASTmse)
	{
		FASTmse=0;

		if (LOADEDD) 
		{
			trans.x=Mscenepos.x;
			trans.y=Mscenepos.y;
			trans.z=Mscenepos.z;
			player.pos.x = loddpos.x+trans.x;
			player.pos.y = loddpos.y+trans.y;
			player.pos.z = loddpos.z+trans.z;
		}
		else 
		{
			player.pos.y +=PLAYER_BASE_HEIGHT;
		}

		PROGRESS_BAR_COUNT+=4.f;
		LoadLevelScreen();
	}
	else if (mse)
	{
		Mscenepos.x=-mse->cub.xmin-(mse->cub.xmax-mse->cub.xmin)*DIV2+((float)ACTIVEBKG->Xsize*(float)ACTIVEBKG->Xdiv)*DIV2;
		Mscenepos.z=-mse->cub.zmin-(mse->cub.zmax-mse->cub.zmin)*DIV2+((float)ACTIVEBKG->Zsize*(float)ACTIVEBKG->Zdiv)*DIV2;
		float t1=(float)(long)(mse->point0.x/BKG_SIZX);
		float t2=(float)(long)(mse->point0.z/BKG_SIZZ);
		t1=mse->point0.x-t1*BKG_SIZX;
		t2=mse->point0.z-t2*BKG_SIZZ;
		Mscenepos.x=(float)((long)(Mscenepos.x/BKG_SIZX))*BKG_SIZX+(float)BKG_SIZX*DIV2;
		Mscenepos.z=(float)((long)(Mscenepos.z/BKG_SIZZ))*BKG_SIZZ+(float)BKG_SIZZ*DIV2;
		mse->pos.x=Mscenepos.x=Mscenepos.x+BKG_SIZX-t1;
		mse->pos.z=Mscenepos.z=Mscenepos.z+BKG_SIZZ-t2;
		Mscenepos.y=mse->pos.y=-mse->cub.ymin-100.f-mse->point0.y;

		if (!NO_PLAYER_POSITION_RESET)
		{
			player.pos.x = mse->pos.x+mse->point0.x;
			player.pos.z = mse->pos.z+mse->point0.z;
			player.pos.y = mse->pos.y+mse->point0.y;
		}						

		EERIERemovePrecalcLights();
		
		PROGRESS_BAR_COUNT+=1.f;
		LoadLevelScreen();
		
		SceneAddMultiScnToBackground(mse);
		
		PROGRESS_BAR_COUNT+=2.f;
		LoadLevelScreen();
					
		trans.x=mse->pos.x;
		trans.y=mse->pos.y;
		trans.z=mse->pos.z;
			
		ReleaseMultiScene(mse);
		mse=NULL;

		if (!NO_PLAYER_POSITION_RESET)
		{
			if (LOADEDD) 
			{
				player.pos.x = loddpos.x+trans.x;
				player.pos.y = loddpos.y+trans.y;
				player.pos.z = loddpos.z+trans.z;
			}
			else player.pos.y +=PLAYER_BASE_HEIGHT;
		}

		NO_PLAYER_POSITION_RESET=0;
		
		PROGRESS_BAR_COUNT+=1.f;
		LoadLevelScreen();
	}
	else
	{
		PROGRESS_BAR_COUNT+=4.f;
		LoadLevelScreen();
	}

	if (CURRENT_TORCH) 
	{
		ARX_SOUND_PlaySFX(SND_TORCH_LOOP, NULL, 1.0F, ARX_SOUND_PLAY_LOOPED);
		SHOW_TORCH=1;
	}
	else 
	{
		SHOW_TORCH=0;
	}
	
	_NB_++;
	Kam=&subj;
	map.pos.x = lastteleport.x=subj.pos.x=moveto.x=player.pos.x;
				lastteleport.y=subj.pos.y=moveto.y=player.pos.y;
	map.pos.z = lastteleport.z=subj.pos.z=moveto.z=player.pos.z;
	lastteleport.y+=PLAYER_BASE_HEIGHT;

	subj.angle.a=player.angle.a;
	subj.angle.b=player.angle.b;
	subj.angle.g=player.angle.g;
		
	RestoreLastLoadedLightning();
	
	PROGRESS_BAR_COUNT+=1.f;
	LoadLevelScreen();
	
	if (!LOAD_N_DONT_ERASE) 
		SetEditMode(0);
	
	PROGRESS_BAR_COUNT+=1.f;
	LoadLevelScreen();
	
	LOAD_N_DONT_ERASE=0;
	DONT_ERASE_PLAYER=0;
	D3DTextr_TESTRestoreAllTextures( m_pd3dDevice );
	
	PROGRESS_BAR_COUNT+=1.f;
	LoadLevelScreen();
	
	FirstFrame=0;
	FRAME_COUNT=0;
	PrepareIOTreatZone(1);
	CURRENTLEVEL=GetLevelNumByName(LastLoadedScene);

	if ((CURRENTLEVEL>=0) && !(NOBUILDMAP) && GAME_EDITOR)
	{
		if (CURRENT_LOADMODE!=LOAD_TRUEFILE)
			iCreateMap=0;
		else if (NeedMapCreation())	
			iCreateMap=1;
		else 
			iCreateMap=0;
	}
	else iCreateMap=0;
	
	if (!NO_TIME_INIT) 
		ARX_TIME_Init();
		
	LastFrameTime=FrameTime;

 PROGRESS_BAR_COUNT+=1.f;
 LoadLevelScreen();
		
	if (ITC.presentation) 
	{
		D3DTextr_KillTexture(ITC.presentation);
		ITC.presentation=NULL;
	}

	if (DONT_WANT_PLAYER_INZONE)
	{
		player.inzone=NULL;
		DONT_WANT_PLAYER_INZONE=0;
	}

	if (MOULINEX)
	{
		LaunchMoulinex();
	}

 PROGRESS_BAR_COUNT+=1.f;
 LoadLevelScreen();

	player.desiredangle.a=player.angle.a=0.f;
	ARX_PLAYER_RectifyPosition();

	if (inter.iobj[0]) 
		inter.iobj[0]->_npcdata->vvpos=-99999;

	SendGameReadyMsg();
	PLAYER_MOUSELOOK_ON=0;
	player.Interface&=~INTER_NOTE;

	if (NO_TIME_INIT) 
	{
		ARX_TIME_Force_Time_Restore(FORCE_TIME_RESTORE);	
		LastFrameTime=FrameTime=FORCE_TIME_RESTORE;
	}
	else
	{
		ARX_TIME_UnPause();
	}	
	
	long t=GetTargetByNameTarget("SEAT_STOOL1_0012");

	if (ValidIONum(t))
	{
		inter.iobj[t]->ioflags|=IO_FORCEDRAW;
	}
	
	if (WILL_RESTORE_PLAYER_POSITION_FLAG)
	{
		Vector_Copy(&player.pos,&WILL_RESTORE_PLAYER_POSITION);
		Vector_Copy(&inter.iobj[0]->pos,&WILL_RESTORE_PLAYER_POSITION);
		inter.iobj[0]->pos.y+=170.f;
		INTERACTIVE_OBJ * io=inter.iobj[0];

		for (long i=0;i<io->obj->nbvertex;i++)
		{
			io->obj->vertexlist3[i].v.x=io->obj->vertexlist[i].v.x+inter.iobj[0]->pos.x;
			io->obj->vertexlist3[i].v.y=io->obj->vertexlist[i].v.y+inter.iobj[0]->pos.y;
			io->obj->vertexlist3[i].v.z=io->obj->vertexlist[i].v.z+inter.iobj[0]->pos.z;			
		}

		WILL_RESTORE_PLAYER_POSITION_FLAG=0;
	}

	if (!FLAG_ALLOW_CLOTHES)	
	{
		for (long i=0;i<inter.nbmax;i++)
		{
			if (	(inter.iobj[i]) 
				&&	(inter.iobj[i]->obj)	)
				inter.iobj[i]->obj->cdata=NULL;
		}
	}

	for (long ni=0;ni<inter.nbmax;ni++)
	{
		if (	(inter.iobj[ni]) 
			&&	(inter.iobj[ni]->ioflags & IO_NPC)
			&&	(inter.iobj[ni]->_npcdata->cuts)	)
				ARX_NPC_ApplyCuts(inter.iobj[ni]);
	}

	ResetVVPos(inter.iobj[0]);

 PROGRESS_BAR_COUNT+=1.f;
 LoadLevelScreen();
	LoadLevelScreen(NULL, -2);
	
	if (	(!CheckInPolyPrecis(player.pos.x,player.pos.y,player.pos.z))
		&&	(LastValidPlayerPos.x!=0.f)
		&&	(LastValidPlayerPos.y!=0.f)
		&&	(LastValidPlayerPos.z!=0.f)	)
	{
		player.pos.x=LastValidPlayerPos.x;
		player.pos.y=LastValidPlayerPos.y;
		player.pos.z=LastValidPlayerPos.z;
	}

	LastValidPlayerPos.x=player.pos.x;
	LastValidPlayerPos.y=player.pos.y;
	LastValidPlayerPos.z=player.pos.z;
	
	return FFH_GOTO_FINISH;
}

//*************************************************************************************

void ManageNONCombatModeAnimations()
{
	INTERACTIVE_OBJ * io=inter.iobj[0];

	if (!io) return;

	ANIM_USE * useanim3=&io->animlayer[3];
	ANIM_HANDLE ** alist=io->anims;

	// FIRST SHIELD Management !
	if (	(player.Current_Movement & PLAYER_LEAN_LEFT)			
		||	(player.Current_Movement & PLAYER_LEAN_RIGHT)			)
	{
	}
	else if ((player.equiped[EQUIP_SLOT_SHIELD] != 0) && !BLOCK_PLAYER_CONTROLS)
	{
		if ( (useanim3->cur_anim==NULL)  ||
			( (useanim3->cur_anim!=alist[ANIM_SHIELD_CYCLE])
			&& (useanim3->cur_anim!=alist[ANIM_SHIELD_HIT])
			&& (useanim3->cur_anim!=alist[ANIM_SHIELD_START]) ) )
		{
			AcquireLastAnim(io);	
			ANIM_Set(useanim3,alist[ANIM_SHIELD_START]);
		}
		else if ((useanim3->cur_anim==alist[ANIM_SHIELD_START])
			&& (useanim3->flags & EA_ANIMEND))
		{
			AcquireLastAnim(io);
			ANIM_Set(useanim3,alist[ANIM_SHIELD_CYCLE]);
			useanim3->flags|=EA_LOOP;
		}
	}
	else
	{
		if (useanim3->cur_anim==alist[ANIM_SHIELD_CYCLE])
		{
			AcquireLastAnim(io);	
			ANIM_Set(useanim3,alist[ANIM_SHIELD_END]);
		}
		else if ((useanim3->cur_anim==alist[ANIM_SHIELD_END])
			&& (useanim3->flags & EA_ANIMEND))
		{
			useanim3->cur_anim=NULL;			
		}
	}
}
long Player_Arrow_Count()
{
	long count=0;

	if (player.bag)
	for (int iNbBag=0; iNbBag<player.bag; iNbBag++) 
	for (long j=0;j<INVENTORY_Y;j++)
	for (long i=0;i<INVENTORY_X;i++)
	{
		INTERACTIVE_OBJ * io=inventory[iNbBag][i][j].io;

		if (io)
		{
			if (!stricmp(GetName(io->filename),"Arrows"))
			{
				if ( io->durability >= 1.f )

				{
					ARX_CHECK_LONG( io->durability );
					count += ARX_CLEAN_WARN_CAST_LONG( io->durability );
				}


			}
		}
	}

	return count;
}

INTERACTIVE_OBJ * Player_Arrow_Count_Decrease()
{
	INTERACTIVE_OBJ * io = NULL;
	
	if (player.bag)
	for (int iNbBag=0; iNbBag<player.bag; iNbBag++) 
	for (long j=0;j<INVENTORY_Y;j++)
	for (long i=0;i<INVENTORY_X;i++)
	{
		INTERACTIVE_OBJ * ioo = inventory[iNbBag][i][j].io;

		if (ioo)
		{
			if (!stricmp(GetName(ioo->filename),"Arrows"))
			{
				if (ioo->durability >= 1.f)
				{
					if (!io)
						io = ioo;
					else if (io->durability > ioo->durability)
						io = ioo;
				}
			}
		}
	}	

	return io;
}
float GLOBAL_SLOWDOWN=1.f;	

bool StrikeAimtime()
{	
	ARX_PLAYER_Remove_Invisibility();
	STRIKE_AIMTIME=(float)ARXTime-(float)AimTime;
	STRIKE_AIMTIME=STRIKE_AIMTIME*(1.f+(1.f-GLOBAL_SLOWDOWN));

	if (STRIKE_AIMTIME>player.Full_AimTime) 
		STRIKE_AIMTIME=1.f;
	else 
		STRIKE_AIMTIME=(float)STRIKE_AIMTIME/(float)player.Full_AimTime;

	if (STRIKE_AIMTIME<0.1f) STRIKE_AIMTIME=0.1f;

	if (STRIKE_AIMTIME>0.8f)
		return true;

	return false;
}

void ManageCombatModeAnimations()
{
	STRIKE_TIME=0;
	INTERACTIVE_OBJ * io=inter.iobj[0];

	if (!io) return;

	ANIM_USE * useanim=&io->animlayer[1];

	ANIM_HANDLE ** alist=io->anims;
	long j;
	long weapontype=ARX_EQUIPMENT_GetPlayerWeaponType();

	if ((weapontype==WEAPON_BARE) && (LAST_WEAPON_TYPE!=weapontype))
	{
		if (useanim->cur_anim!=alist[ANIM_BARE_WAIT])	
		{
			AcquireLastAnim(io);
			ANIM_Set(useanim,alist[ANIM_BARE_WAIT]);			
			AimTime=0;
		}
	}

	switch (weapontype)
	{
		case WEAPON_BARE:	// BARE HANDS PLAYER MANAGEMENT

			if (useanim->cur_anim==alist[ANIM_BARE_WAIT])	
			{
				AimTime=0;

				if (EERIEMouseButton & 1) 
				{		
					AcquireLastAnim(io);
					ANIM_Set(useanim,alist[ANIM_BARE_STRIKE_LEFT_START+CurrFightPos*3]);			
					io->aflags&=~IO_NPC_AFLAG_HIT_CLEAR;					
				}
			}

			// Now go for strike cycle...
			for (j=0;j<4;j++)
			{
				if (	(useanim->cur_anim==alist[ANIM_BARE_STRIKE_LEFT_START+j*3])
					&&	(useanim->flags & EA_ANIMEND)	)
				{
					AcquireLastAnim(io);
					ANIM_Set(useanim,alist[ANIM_BARE_STRIKE_LEFT_CYCLE+j*3]);
					AimTime = ARXTimeUL();
					useanim->flags|=EA_LOOP;
				}
				else if (	(useanim->cur_anim==alist[ANIM_BARE_STRIKE_LEFT_CYCLE+j*3])
						&& !(EERIEMouseButton & 1)  )
				{
					AcquireLastAnim(io);
					ANIM_Set(useanim,alist[ANIM_BARE_STRIKE_LEFT+j*3]);

					if (StrikeAimtime())
					{
						char str[128];
						str[0]=0;

						if (io->strikespeech)
							strcpy(str,io->strikespeech);

						if (player.equiped[EQUIP_SLOT_WEAPON]!=0)
						{
							if (inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]]->strikespeech)
								strcpy(str,inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]]->strikespeech);
						}

						if (str[0]!=0)
						{
							ARX_SPEECH_AddSpeech(io,str,PARAM_LOCALISED,ANIM_TALK_NEUTRAL,ARX_SPEECH_FLAG_NOTEXT);
						}
					}	

					SendIOScriptEvent(io,SM_STRIKE,"BARE");
					PlayerWeaponBlocked=-1;
					CurrFightPos=0;
					AimTime=0;
				}
				else if (useanim->cur_anim==alist[ANIM_BARE_STRIKE_LEFT+j*3])
				{
					if (useanim->flags & EA_ANIMEND)
					{
						AcquireLastAnim(io);
						ANIM_Set(useanim,alist[ANIM_BARE_WAIT]);
						useanim->flags|=EA_LOOP;
						CurrFightPos=0;
						AimTime = ARXTimeUL();
						PlayerWeaponBlocked=-1;
					}
					else if ((useanim->ctime > useanim->cur_anim->anims[useanim->altidx_cur]->anim_time * 0.2f)
								&&	(useanim->ctime < useanim->cur_anim->anims[useanim->altidx_cur]->anim_time*0.8f)
								&&	(PlayerWeaponBlocked==-1) )
						{
							if (useanim->cur_anim==alist[ANIM_BARE_STRIKE_LEFT])
							{
								STRIKE_TIME=1;
							long id = io->obj->fastaccess.left_attach;

								if (id!=-1)
								{
									EERIE_SPHERE sphere;
									sphere.origin.x=io->obj->vertexlist3[id].v.x;
									sphere.origin.y=io->obj->vertexlist3[id].v.y;
									sphere.origin.z=io->obj->vertexlist3[id].v.z;
									sphere.radius=25.f;

									if (FistParticles & 2) sphere.radius*=2.f;

									long num;

									if (CheckAnythingInSphere(&sphere,0,0,&num))										
									{
										float dmgs=(player.Full_damages+1)*STRIKE_AIMTIME;

										if (FistParticles & 2) dmgs*=1.5f;

										if (ARX_DAMAGES_TryToDoDamage(&io->obj->vertexlist3[id].v,dmgs,40,0))
										{
											if (FistParticles & 2) 
												ARX_SOUND_PlaySFX(SND_SPELL_LIGHTNING_START, &io->obj->vertexlist3[id].v);

											PlayerWeaponBlocked=useanim->ctime;
										PlayerWeaponBlockTime = ARXTimeUL();
										}

										{
										ARX_PARTICLES_Spawn_Spark(&sphere.origin, dmgs, 2);

											if (ValidIONum(num))
											{
												ARX_SOUND_PlayCollision(inter.iobj[num]->material,MATERIAL_FLESH, 1.f, 1.f, &sphere.origin, NULL);
											}
										}
										
									}
								}
							}
							else  // Strike Right
							{
								STRIKE_TIME=1;
							long id = io->obj->fastaccess.primary_attach;

								if (id!=-1)
								{
									EERIE_SPHERE sphere;
									sphere.origin.x=io->obj->vertexlist3[id].v.x;
									sphere.origin.y=io->obj->vertexlist3[id].v.y;
									sphere.origin.z=io->obj->vertexlist3[id].v.z;
									sphere.radius=25.f;

									if (FistParticles & 2) sphere.radius*=2.f;

									long num;

									if (CheckAnythingInSphere(&sphere,0,0,&num))									
									{
										float dmgs=(player.Full_damages+1)*STRIKE_AIMTIME;

										if (FistParticles & 2) dmgs*=1.5f;

										if (ARX_DAMAGES_TryToDoDamage(&io->obj->vertexlist3[id].v,dmgs,40,0))
										{
											if (FistParticles & 2) 
												ARX_SOUND_PlaySFX(SND_SPELL_LIGHTNING_START, &io->obj->vertexlist3[id].v);

											PlayerWeaponBlocked=useanim->ctime;
										PlayerWeaponBlockTime = ARXTimeUL();
										}

										{
										ARX_PARTICLES_Spawn_Spark(&sphere.origin, dmgs, 2);

											if (ValidIONum(num))
											{
												ARX_SOUND_PlayCollision(inter.iobj[num]->material,MATERIAL_FLESH, 1.f, 1.f, &sphere.origin, NULL);
											}
										}
										
									}
								}
							}
						}					
				}
			}		

		break;
		case WEAPON_DAGGER: // DAGGER PLAYER MANAGEMENT			
			// Waiting and receiving Strike Impulse
			if (useanim->cur_anim==alist[ANIM_DAGGER_WAIT]) 
			{
				AimTime = 0;

				if (EERIEMouseButton & 1)
				{		
					AcquireLastAnim(io);
					ANIM_Set(useanim,alist[ANIM_DAGGER_STRIKE_LEFT_START+CurrFightPos*3]);
					io->aflags&=~IO_NPC_AFLAG_HIT_CLEAR;
				}
			}

			// Now go for strike cycle...
			for (j=0;j<4;j++)
			{
				if ((useanim->cur_anim==alist[ANIM_DAGGER_STRIKE_LEFT_START+j*3])&& (useanim->flags & EA_ANIMEND))
				{
					AcquireLastAnim(io);
					ANIM_Set(useanim,alist[ANIM_DAGGER_STRIKE_LEFT_CYCLE+j*3]);
					AimTime = ARXTimeUL();
					useanim->flags|=EA_LOOP;
				}
				else if ((useanim->cur_anim==alist[ANIM_DAGGER_STRIKE_LEFT_CYCLE+j*3])
						&& !(EERIEMouseButton & 1))
				{
					AcquireLastAnim(io);
					ANIM_Set(useanim,alist[ANIM_DAGGER_STRIKE_LEFT+j*3]);

					if (StrikeAimtime())
					{
						char str[128];
						str[0]=0;

						if (io->strikespeech)
							strcpy(str,io->strikespeech);

						if (player.equiped[EQUIP_SLOT_WEAPON]!=0)
						{
							if (inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]]->strikespeech)
								strcpy(str,inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]]->strikespeech);
						}

						if (str[0]!=0)
						{
							ARX_SPEECH_AddSpeech(io,str,PARAM_LOCALISED,ANIM_TALK_NEUTRAL,ARX_SPEECH_FLAG_NOTEXT);
						}
					}	

					SendIOScriptEvent(io,SM_STRIKE,"DAGGER");
					CurrFightPos=0;
					AimTime=0;
				}
				else if (useanim->cur_anim==alist[ANIM_DAGGER_STRIKE_LEFT+j*3]) 
				{
					if (	(useanim->ctime > useanim->cur_anim->anims[useanim->altidx_cur]->anim_time*0.3f)
						&&	(useanim->ctime < useanim->cur_anim->anims[useanim->altidx_cur]->anim_time*0.7f))
					{
						STRIKE_TIME=1;

						if ((PlayerWeaponBlocked==-1) 
						&& (ARX_EQUIPMENT_Strike_Check(io,inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]],STRIKE_AIMTIME,0)))
						{
							PlayerWeaponBlocked=useanim->ctime;
							PlayerWeaponBlockTime = ARXTimeUL();
						}
					}

					if (useanim->flags & EA_ANIMEND)
					{
						AcquireLastAnim(io);
						ANIM_Set(useanim,alist[ANIM_DAGGER_WAIT]);
						useanim->flags&=~(EA_PAUSED | EA_REVERSE);
						useanim->flags|=EA_LOOP;
						CurrFightPos=0;
						AimTime = ARXTimeUL();
						PlayerWeaponBlocked=-1;
					}

					if ((PlayerWeaponBlocked!=-1) 
						&& (useanim->ctime<useanim->cur_anim->anims[useanim->altidx_cur]->anim_time*0.9f))							
						ARX_EQUIPMENT_Strike_Check(io,inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]],STRIKE_AIMTIME,1);
					
				}
			}

		break;
		case WEAPON_1H: // 1HANDED PLAYER MANAGEMENT
			// Waiting and Received Strike Impulse
			if (useanim->cur_anim==alist[ANIM_1H_WAIT])
			{
				AimTime = 0;

				if (EERIEMouseButton & 1)
				{		
					AcquireLastAnim(io);
					ANIM_Set(useanim,alist[ANIM_1H_STRIKE_LEFT_START+CurrFightPos*3]);
					io->aflags&=~IO_NPC_AFLAG_HIT_CLEAR;
				}
			}

			// Now go for strike cycle...
			for (j=0;j<4;j++)
			{
				if ((useanim->cur_anim==alist[ANIM_1H_STRIKE_LEFT_START+j*3])&& (useanim->flags & EA_ANIMEND))
				{
					AcquireLastAnim(io);
					ANIM_Set(useanim,alist[ANIM_1H_STRIKE_LEFT_CYCLE+j*3]);
					AimTime = ARXTimeUL();
					useanim->flags|=EA_LOOP;
				}
				else if ((useanim->cur_anim==alist[ANIM_1H_STRIKE_LEFT_CYCLE+j*3])
				         && !(EERIEMouseButton & 1))
				{
					AcquireLastAnim(io);
					ANIM_Set(useanim,alist[ANIM_1H_STRIKE_LEFT+j*3]);

					if (StrikeAimtime())
					{
						char str[128];
						str[0]=0;

						if (io->strikespeech)
							strcpy(str,io->strikespeech);

						if (player.equiped[EQUIP_SLOT_WEAPON]!=0)
						{
							if (inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]]->strikespeech)
								strcpy(str,inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]]->strikespeech);
						}

						if (str[0]!=0)
						{
							ARX_SPEECH_AddSpeech(io,str,PARAM_LOCALISED,ANIM_TALK_NEUTRAL,ARX_SPEECH_FLAG_NOTEXT);
						}
					}	

					SendIOScriptEvent(io,SM_STRIKE,"1H");
					CurrFightPos=0;
					AimTime=0;
				}
				else if (useanim->cur_anim==alist[ANIM_1H_STRIKE_LEFT+j*3]) 
				{
						if ((useanim->ctime > useanim->cur_anim->anims[useanim->altidx_cur]->anim_time*0.3f)
							&& (useanim->ctime < useanim->cur_anim->anims[useanim->altidx_cur]->anim_time*0.7f))
						{
							STRIKE_TIME=1;

							if ((PlayerWeaponBlocked==-1) 
								&& (ARX_EQUIPMENT_Strike_Check(io,inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]],STRIKE_AIMTIME,0)) )
							{
								PlayerWeaponBlocked=useanim->ctime;
							PlayerWeaponBlockTime = ARXTimeUL();
							}
						}

						if (useanim->flags & EA_ANIMEND)
						{
							AcquireLastAnim(io);
							ANIM_Set(useanim,alist[ANIM_1H_WAIT]);
							useanim->flags&=~(EA_PAUSED | EA_REVERSE);
							useanim->flags|=EA_LOOP;
							CurrFightPos=0;
							AimTime=0;
							PlayerWeaponBlocked=-1;
						}

						if ((PlayerWeaponBlocked!=-1) 
							&& (useanim->ctime<useanim->cur_anim->anims[useanim->altidx_cur]->anim_time*0.9f))
							ARX_EQUIPMENT_Strike_Check(io,inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]],STRIKE_AIMTIME,1);
					
				}
			}

		break;
		case WEAPON_2H: // 2HANDED PLAYER MANAGEMENT
			// Waiting and Receiving Strike Impulse
			if (useanim->cur_anim==alist[ANIM_2H_WAIT]) 
			{
				AimTime = 0;

				if (EERIEMouseButton & 1)
				{		
					AcquireLastAnim(io);
					ANIM_Set(useanim,alist[ANIM_2H_STRIKE_LEFT_START+CurrFightPos*3]);

					io->aflags&=~IO_NPC_AFLAG_HIT_CLEAR;
				}
			}

			// Now go for strike cycle...
			for (j=0;j<4;j++)
			{
				if (	(useanim->cur_anim==alist[ANIM_2H_STRIKE_LEFT_START+j*3])
					&&	(useanim->flags & EA_ANIMEND))
				{
					AcquireLastAnim(io);
					ANIM_Set(useanim,alist[ANIM_2H_STRIKE_LEFT_CYCLE+j*3]);
					AimTime = ARXTimeUL();
					useanim->flags|=EA_LOOP;
				}
				else if (	(useanim->cur_anim==alist[ANIM_2H_STRIKE_LEFT_CYCLE+j*3])
						&&	!(EERIEMouseButton & 1) )
				{

					AcquireLastAnim(io);
					ANIM_Set(useanim,alist[ANIM_2H_STRIKE_LEFT+j*3]);

					if (StrikeAimtime())
					{
						char str[128];
						str[0]=0;

						if (io->strikespeech)
							strcpy(str,io->strikespeech);

						if (player.equiped[EQUIP_SLOT_WEAPON]!=0)
						{
							if (inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]]->strikespeech)
								strcpy(str,inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]]->strikespeech);
						}

						if (str[0]!=0)
						{
							ARX_SPEECH_AddSpeech(io,str,PARAM_LOCALISED,ANIM_TALK_NEUTRAL,ARX_SPEECH_FLAG_NOTEXT);
						}
					}	

					SendIOScriptEvent(io,SM_STRIKE,"2H");
					CurrFightPos=0;
					AimTime=0;
				}
				else if (useanim->cur_anim==alist[ANIM_2H_STRIKE_LEFT+j*3]) 
				{
						if ((useanim->ctime > useanim->cur_anim->anims[useanim->altidx_cur]->anim_time*0.3f)
							&& (useanim->ctime < useanim->cur_anim->anims[useanim->altidx_cur]->anim_time*0.7f))
						{
							STRIKE_TIME=1;

							if ((PlayerWeaponBlocked==-1) 
								&& (ARX_EQUIPMENT_Strike_Check(io,inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]],STRIKE_AIMTIME,0)) )
							{
								PlayerWeaponBlocked=useanim->ctime;
							PlayerWeaponBlockTime = ARXTimeUL();
							}
						}

						if (useanim->flags & EA_ANIMEND)
						{
							AcquireLastAnim(io);
							ANIM_Set(useanim,alist[ANIM_2H_WAIT]);
							useanim->flags&=~(EA_PAUSED | EA_REVERSE);
							useanim->flags|=EA_LOOP;
							CurrFightPos=0;
							AimTime=0;
							PlayerWeaponBlocked=-1;
						}

						if ((PlayerWeaponBlocked!=-1) 
							&& (useanim->ctime<useanim->cur_anim->anims[useanim->altidx_cur]->anim_time*0.9f))
							ARX_EQUIPMENT_Strike_Check(io,inter.iobj[player.equiped[EQUIP_SLOT_WEAPON]],STRIKE_AIMTIME,1);
					
				}
			}			

		break;
		
		case WEAPON_BOW: // MISSILE PLAYER MANAGEMENT			

			if ((useanim->cur_anim == alist[ANIM_MISSILE_STRIKE_CYCLE]))
			{
				if (GLOBAL_SLOWDOWN!=1.f)
					BOW_FOCAL+=Original_framedelay;
				else
					BOW_FOCAL += _framedelay;

				if (BOW_FOCAL>710) BOW_FOCAL=710;
			}

			// Waiting and Receiving Strike Impulse	
			if (useanim->cur_anim==alist[ANIM_MISSILE_WAIT]) 
			{
				AimTime = ARXTimeUL();

				if ((EERIEMouseButton & 1) && (Player_Arrow_Count()>0))
				{
					AcquireLastAnim(io);
					ANIM_Set(useanim,alist[ANIM_MISSILE_STRIKE_PART_1]);
					io->aflags&=~IO_NPC_AFLAG_HIT_CLEAR;
				}
			}

			if ((useanim->cur_anim==alist[ANIM_MISSILE_STRIKE_PART_1])&& (useanim->flags & EA_ANIMEND))
			{
				AimTime = 0;
				AcquireLastAnim(io);
				ANIM_Set(useanim,alist[ANIM_MISSILE_STRIKE_PART_2]);

				EERIE_LINKEDOBJ_LinkObjectToObject(io->obj, arrowobj, "LEFT_ATTACH", "ATTACH", NULL);

			}

			// Now go for strike cycle...
			if ((useanim->cur_anim==alist[ANIM_MISSILE_STRIKE_PART_2])&& (useanim->flags & EA_ANIMEND))
			{
				AcquireLastAnim(io);
				ANIM_Set(useanim,alist[ANIM_MISSILE_STRIKE_CYCLE]);
				AimTime = ARXTimeUL();

				useanim->flags|=EA_LOOP;				
			}
			else if ((useanim->cur_anim==alist[ANIM_MISSILE_STRIKE_CYCLE]) 
				&& !(EERIEMouseButton & 1))
			{

				EERIE_LINKEDOBJ_UnLinkObjectFromObject(io->obj, arrowobj);
				AcquireLastAnim(io);
				ANIM_Set(useanim,alist[ANIM_MISSILE_STRIKE]);
				SendIOScriptEvent(io,SM_STRIKE,"BOW");				
				StrikeAimtime();
				STRIKE_AIMTIME=(float)(BOW_FOCAL)/710.f;
				INTERACTIVE_OBJ * ioo=Player_Arrow_Count_Decrease();
				float poisonous=0.f;

				if (ioo)
				{
					if (ioo->poisonous_count>0)
					{
						poisonous=ioo->poisonous;
						ioo->poisonous_count--;

						if (ioo->poisonous_count<=0)
							ioo->poisonous=0;
					}
					else poisonous=ioo->poisonous;

					ARX_DAMAGES_DurabilityLoss(ioo,1.f);

					if (ValidIOAddress(ioo))
					{
						if (ioo->durability<=0.f)
							ARX_INTERACTIVE_DestroyIO(ioo);
					}
				}

				PlayerLaunchArrow(STRIKE_AIMTIME,poisonous);
				AimTime=0;
			}
			else if (useanim->cur_anim==alist[ANIM_MISSILE_STRIKE])
			{
				BOW_FOCAL-=Original_framedelay;

				if (BOW_FOCAL<0) BOW_FOCAL=0;

				if (useanim->flags & EA_ANIMEND)
				{
					BOW_FOCAL=0;
					AcquireLastAnim(io);
					ANIM_Set(useanim,alist[ANIM_MISSILE_WAIT]);
					useanim->flags|=EA_LOOP;
					AimTime=0;
					PlayerWeaponBlocked=-1;
					EERIE_LINKEDOBJ_UnLinkObjectFromObject(io->obj, arrowobj);
				}
			}

		break;
	}	

	LAST_WEAPON_TYPE=weapontype;
}
void ManageCombatModeAnimationsEND()
{
	INTERACTIVE_OBJ * io=inter.iobj[0];
	ANIM_USE * useanim=&io->animlayer[1];

	ANIM_USE * useanim3=&io->animlayer[3];	
	ANIM_HANDLE ** alist=io->anims;

	if (	(useanim->cur_anim)
		&&(		(useanim->cur_anim==alist[ANIM_BARE_READY])
			||	(useanim->cur_anim==alist[ANIM_DAGGER_READY_PART_2])
			||	(useanim->cur_anim==alist[ANIM_DAGGER_READY_PART_1])
			||	(useanim->cur_anim==alist[ANIM_1H_READY_PART_2])
			||	(useanim->cur_anim==alist[ANIM_1H_READY_PART_1])
			||	(useanim->cur_anim==alist[ANIM_2H_READY_PART_2])
			||	(useanim->cur_anim==alist[ANIM_2H_READY_PART_1])
			||	(useanim->cur_anim==alist[ANIM_MISSILE_READY_PART_1])
			||	(useanim->cur_anim==alist[ANIM_MISSILE_READY_PART_2])	)
			)
		AimTime = ARXTimeUL();

	if (useanim->flags & EA_ANIMEND)
	{
		long weapontype=ARX_EQUIPMENT_GetPlayerWeaponType();

		if (useanim->cur_anim &&
			(	(useanim->cur_anim==io->anims[ANIM_BARE_UNREADY])
			||	(useanim->cur_anim==io->anims[ANIM_DAGGER_UNREADY_PART_2])
			||	(useanim->cur_anim==io->anims[ANIM_1H_UNREADY_PART_2])
			||	(useanim->cur_anim==io->anims[ANIM_2H_UNREADY_PART_2])
			||	(useanim->cur_anim==io->anims[ANIM_MISSILE_UNREADY_PART_2])	)	)
		{
			io->_npcdata->ex_rotate->flags|=EXTRA_ROTATE_REALISTIC;
			AcquireLastAnim(io);
			useanim->cur_anim=NULL;					
		}

		switch (weapontype)
		{
			case WEAPON_BARE:

				// Is Weapon Ready ? In this case go to Fight Wait anim
				if (useanim->cur_anim==alist[ANIM_BARE_READY])
				{

					AcquireLastAnim(io);

					if (player.Interface & INTER_NO_STRIKE)
					{
						player.Interface&=~INTER_NO_STRIKE;
						ANIM_Set(useanim,alist[ANIM_BARE_WAIT]);	
						useanim->flags|=EA_LOOP;
					}
					else
						ANIM_Set(useanim,alist[ANIM_BARE_STRIKE_LEFT_START+CurrFightPos*3]);			

					AimTime = ARXTimeUL();
					io->aflags&=~IO_NPC_AFLAG_HIT_CLEAR;
				}

			break;
			case WEAPON_DAGGER:

				// DAGGER ANIMS end
				if (alist[ANIM_DAGGER_READY_PART_1])
				{
					if (useanim->cur_anim==alist[ANIM_DAGGER_READY_PART_1])
					{
						AcquireLastAnim(io);
						ARX_EQUIPMENT_AttachPlayerWeaponToHand();
						ANIM_Set(useanim,alist[ANIM_DAGGER_READY_PART_2]);
					}
					else if (useanim->cur_anim==alist[ANIM_DAGGER_READY_PART_2])
					{
						AcquireLastAnim(io);

						if (player.Interface & INTER_NO_STRIKE)
						{
							player.Interface&=~INTER_NO_STRIKE;
							ANIM_Set(useanim,alist[ANIM_DAGGER_WAIT]);	
							useanim->flags|=EA_LOOP;
						}
						else
							ANIM_Set(useanim,alist[ANIM_DAGGER_STRIKE_LEFT_START+CurrFightPos*3]);			

						AimTime = ARXTimeUL();
						io->aflags&=~IO_NPC_AFLAG_HIT_CLEAR;
					}
					else if (useanim->cur_anim==alist[ANIM_DAGGER_UNREADY_PART_1])
					{
						AcquireLastAnim(io);
						ARX_EQUIPMENT_AttachPlayerWeaponToBack();
						ANIM_Set(useanim,alist[ANIM_DAGGER_UNREADY_PART_2]);
					}
				}

			break;
			case WEAPON_1H:	// 1H ANIMS end

				if (alist[ANIM_1H_READY_PART_1]!=NULL)
				{
					if (useanim->cur_anim==alist[ANIM_1H_READY_PART_1])
					{
						AcquireLastAnim(io);
						ARX_EQUIPMENT_AttachPlayerWeaponToHand();
						ANIM_Set(useanim,alist[ANIM_1H_READY_PART_2]);
					}
					else if (useanim->cur_anim==alist[ANIM_1H_READY_PART_2])
					{
						AcquireLastAnim(io);

						if (player.Interface & INTER_NO_STRIKE)
						{
							player.Interface&=~INTER_NO_STRIKE;
							ANIM_Set(useanim,alist[ANIM_1H_WAIT]);	
							useanim->flags|=EA_LOOP;
						}
						else
						ANIM_Set(useanim,alist[ANIM_1H_STRIKE_LEFT_START+CurrFightPos*3]);			

						AimTime = ARXTimeUL();
						io->aflags&=~IO_NPC_AFLAG_HIT_CLEAR;
					}
					else if (useanim->cur_anim==alist[ANIM_1H_UNREADY_PART_1])
					{
						AcquireLastAnim(io);
						ARX_EQUIPMENT_AttachPlayerWeaponToBack();
						ANIM_Set(useanim,alist[ANIM_1H_UNREADY_PART_2]);
					}
				}

			break;
			case WEAPON_2H:	// 2H ANIMS end

				if (alist[ANIM_2H_READY_PART_1])
				{
					if (useanim->cur_anim==alist[ANIM_2H_READY_PART_1])
					{
						AcquireLastAnim(io);
						ARX_EQUIPMENT_AttachPlayerWeaponToHand();
						ANIM_Set(useanim,alist[ANIM_2H_READY_PART_2]);
					}
					else if (useanim->cur_anim==alist[ANIM_2H_READY_PART_2])
					{
						AcquireLastAnim(io);

						if (player.Interface & INTER_NO_STRIKE)
						{
							player.Interface&=~INTER_NO_STRIKE;
							ANIM_Set(useanim,alist[ANIM_2H_WAIT]);	
							useanim->flags|=EA_LOOP;
						}
						else
							ANIM_Set(useanim,alist[ANIM_2H_STRIKE_LEFT_START+CurrFightPos*3]);			

						AimTime = ARXTimeUL();
						io->aflags&=~IO_NPC_AFLAG_HIT_CLEAR;
					}
					else if (useanim->cur_anim==alist[ANIM_2H_UNREADY_PART_1])
					{
						AcquireLastAnim(io);
						ARX_EQUIPMENT_AttachPlayerWeaponToBack();
						ANIM_Set(useanim,alist[ANIM_2H_UNREADY_PART_2]);
					}
				}

			break;
			case WEAPON_BOW:// MISSILE Weapon ANIMS end

				if (alist[ANIM_MISSILE_READY_PART_1])
				{
					if (useanim->cur_anim==alist[ANIM_MISSILE_READY_PART_1])
					{
						AcquireLastAnim(io);
						ARX_EQUIPMENT_AttachPlayerWeaponToHand();
						ANIM_Set(useanim,alist[ANIM_MISSILE_READY_PART_2]);
					}
					else if (useanim->cur_anim==alist[ANIM_MISSILE_READY_PART_2])
					{
						if (Player_Arrow_Count()>0)
						{
							AcquireLastAnim(io);

							if (player.Interface & INTER_NO_STRIKE)
							{
								player.Interface&=~INTER_NO_STRIKE;
								ANIM_Set(useanim,alist[ANIM_MISSILE_WAIT]);	
								useanim->flags|=EA_LOOP;
							}
							else
								ANIM_Set(useanim,alist[ANIM_MISSILE_STRIKE_PART_1]);

							io->aflags&=~IO_NPC_AFLAG_HIT_CLEAR;
						}
						else
						{
							AcquireLastAnim(io);
							ANIM_Set(useanim,alist[ANIM_MISSILE_WAIT]);
						}						

						EERIE_LINKEDOBJ_UnLinkObjectFromObject(io->obj, arrowobj);
					}
					else if (useanim->cur_anim==alist[ANIM_MISSILE_STRIKE_PART_1])
					{
						ANIM_Set(useanim,alist[ANIM_MISSILE_STRIKE_PART_2]);
					}
					else if (useanim->cur_anim==alist[ANIM_MISSILE_STRIKE_PART_2])
					{
						ANIM_Set(useanim,alist[ANIM_MISSILE_STRIKE_CYCLE]);
					}
					else if (useanim->cur_anim==alist[ANIM_MISSILE_UNREADY_PART_1])
					{
						AcquireLastAnim(io);
						ARX_EQUIPMENT_AttachPlayerWeaponToBack();
						ANIM_Set(useanim,alist[ANIM_MISSILE_UNREADY_PART_2]);
					}
				}

			break;
		}

		// Spell casting anims
		if ((alist[ANIM_CAST]) && (useanim->cur_anim==alist[ANIM_CAST]))
		{
			AcquireLastAnim(io);

			if (alist[ANIM_CAST_END])
				ANIM_Set(useanim,alist[ANIM_CAST_END]);
		}	
		else if ((alist[ANIM_CAST_END]) && (useanim->cur_anim==alist[ANIM_CAST_END]))
		{
			AcquireLastAnim(io);
			useanim->cur_anim=NULL;
			player.doingmagic=0;

			if (WILLRETURNTOCOMBATMODE)
			{
				player.Interface|=INTER_COMBATMODE;
				player.Interface|=INTER_NO_STRIKE;
				
				ARX_EQUIPMENT_LaunchPlayerReadyWeapon();
				WILLRETURNTOCOMBATMODE=0;
			}
		}
	}

	// Is the shield off ?
	if (useanim3->flags & EA_ANIMEND)
	{
		if ((io->anims[ANIM_SHIELD_END]) && (useanim3->cur_anim==io->anims[ANIM_SHIELD_END]))
		{
			AcquireLastAnim(io);
			useanim3->cur_anim=NULL;
		}
	}
}

float LAST_FADEVALUE=1.f;
void ManageFade(LPDIRECT3DDEVICE7 m_pd3dDevice)
{ 	
	float tim=((float)ARX_TIME_Get()-(float)FADESTART);

	if (tim<=0.f) return;

	float Visibility=tim/(float)FADEDURATION;
	
	if (FADEDIR>0) Visibility=1.f-Visibility;

	if (Visibility>1.f) Visibility=1.f;

	if (Visibility<0.f)
	{
		Visibility = 0.f; 
		FADEDIR = 0;
		return;
	}

	LAST_FADEVALUE=Visibility;
	m_pd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO );
	m_pd3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR );										
	SETZWRITE(m_pd3dDevice, FALSE );
	SETALPHABLEND(m_pd3dDevice,TRUE);
	
	EERIEDrawBitmap(m_pd3dDevice,0.f,0.f,(float)DANAESIZX,(float)DANAESIZY,0.0001f,
			NULL,_EERIERGB(Visibility));		

	m_pd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
	m_pd3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);	
	float col=Visibility;
	EERIEDrawBitmap(m_pd3dDevice,0.f,0.f,(float)DANAESIZX,(float)DANAESIZY,0.0001f,
			NULL,EERIERGB(col*FADECOLOR.r,col*FADECOLOR.g,col*FADECOLOR.b));		
	SETALPHABLEND(m_pd3dDevice,FALSE);
	SETZWRITE(m_pd3dDevice, TRUE );
}

extern long cur_mr;
TextureContainer * Mr_tc=NULL;

void CheckMr()
{
	if (cur_mr==3)
	{
		if (GDevice && Mr_tc && TextureContainer_Exist(Mr_tc))
		{
			if (!Mr_tc->m_pddsSurface)
				Mr_tc->Restore(GDevice);

			EERIEDrawBitmap(GDevice,DANAESIZX-(128.f*Xratio),0.f,(float)128*Xratio,(float)128*Yratio,0.0001f,
				Mr_tc,_EERIERGB(0.5f+PULSATE*DIV10));		
		}
		else
		{
			Mr_tc=MakeTCFromFile_NoRefinement("graph\\particles\\(Fx)_Mr.bmp");		
	}
}
}
void DrawImproveVisionInterface(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	if (ombrignon->m_pddsSurface) 
	{
		float mod = 0.6f + PULSATE * 0.35f;
		EERIEDrawBitmap(m_pd3dDevice,0.f,0.f,(float)DANAESIZX,(float)DANAESIZY,0.0001f,
			ombrignon,EERIERGB((0.5f+PULSATE*DIV10)*mod,0.f,0.f));		
	}
}
float MagicSightFader=0.f;
void DrawMagicSightInterface(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	if (eyeball.exist==1) return;

	if (Flying_Eye && Flying_Eye->m_pddsSurface) 
	{
		m_pd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO );
		m_pd3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR );										
		float col=(0.75f+PULSATE*DIV20);

		if (col>1.f) col=1.f;

		if (eyeball.exist<0) 
		{
			col=(float)(-eyeball.exist)*DIV100;
		}
		else if (eyeball.exist>2)
		{
			col = 1.f - eyeball.size.x;
		}

		EERIEDrawBitmap(m_pd3dDevice,0.f,0.f,(float)DANAESIZX,(float)DANAESIZY,0.0001f,
			Flying_Eye,_EERIERGB(col));		
		
		if (MagicSightFader>0.f)
		{
			col=MagicSightFader;
			EERIEDrawBitmap(m_pd3dDevice,0.f,0.f,(float)DANAESIZX,(float)DANAESIZY,0.0001f,
				NULL,_EERIERGB(col));		
			MagicSightFader-=Original_framedelay*DIV400;

			if (MagicSightFader<0.f)
				MagicSightFader=0.f;
		}

		m_pd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
		m_pd3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);	
	}
}
		
//*************************************************************************************
	
void RenderAllNodes(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	EERIE_3D angle;
	float xx,yy;
	long j;
	Vector_Init(&angle);

	SETALPHABLEND(m_pd3dDevice,FALSE);

	for (long i=0;i<nodes.nbmax;i++)
	{
		if (nodes.nodes[i].exist)
		{
			
			DrawEERIEInter(m_pd3dDevice,nodeobj,
				&angle,&nodes.nodes[i].pos,NULL);
			nodes.nodes[i].bboxmin.x=(short)BBOXMIN.x;
			nodes.nodes[i].bboxmin.y=(short)BBOXMIN.y;
			nodes.nodes[i].bboxmax.x=(short)BBOXMAX.x;
			nodes.nodes[i].bboxmax.y=(short)BBOXMAX.y;

			if ((nodeobj->vertexlist[nodeobj->origin].vert.sz>0.f) && (nodeobj->vertexlist[nodeobj->origin].vert.sz<0.9f))
			{
				xx=nodeobj->vertexlist[nodeobj->origin].vert.sx-40.f;
				yy=nodeobj->vertexlist[nodeobj->origin].vert.sy-40.f;
				ARX_TEXT_Draw(m_pd3dDevice, InBookFont, xx, yy, 0, 0, nodes.nodes[i].UName, EERIECOLOR_YELLOW);	//font
			}

			if (nodes.nodes[i].selected)
			{
				EERIEDraw2DLine(m_pd3dDevice, nodes.nodes[i].bboxmin.x,nodes.nodes[i].bboxmin.y,nodes.nodes[i].bboxmax.x,nodes.nodes[i].bboxmin.y,0.01f, EERIECOLOR_YELLOW);
				EERIEDraw2DLine(m_pd3dDevice, nodes.nodes[i].bboxmax.x,nodes.nodes[i].bboxmin.y,nodes.nodes[i].bboxmax.x,nodes.nodes[i].bboxmax.y,0.01f, EERIECOLOR_YELLOW);
				EERIEDraw2DLine(m_pd3dDevice, nodes.nodes[i].bboxmax.x,nodes.nodes[i].bboxmax.y,nodes.nodes[i].bboxmin.x,nodes.nodes[i].bboxmax.y,0.01f, EERIECOLOR_YELLOW);
				EERIEDraw2DLine(m_pd3dDevice, nodes.nodes[i].bboxmin.x,nodes.nodes[i].bboxmax.y,nodes.nodes[i].bboxmin.x,nodes.nodes[i].bboxmin.y,0.01f, EERIECOLOR_YELLOW);
			}

			for (j=0;j<MAX_LINKS;j++)
			{
				if (nodes.nodes[i].link[j]!=-1)
				{
					EERIEDrawTrue3DLine(m_pd3dDevice,&nodes.nodes[i].pos,&nodes.nodes[nodes.nodes[i].link[j]].pos,EERIECOLOR_GREEN);
				}
			}
		}
	}
}
				
void AddQuakeFX(float intensity,float duration,float period,long flags)
{
	if (QuakeFx.intensity>0.f)
	{
		QuakeFx.intensity+=intensity;

		QuakeFx.duration+=(unsigned long)duration;
		QuakeFx.frequency+=period;
		QuakeFx.frequency*=DIV2;
		QuakeFx.flags|=flags;

		if (flags & 1)
			ARX_SOUND_PlaySFX(SND_QUAKE, NULL, 1.0F - 0.5F * QuakeFx.intensity);
	}
	else
	{
		QuakeFx.intensity=intensity;


		ARX_CHECK_ULONG(FrameTime);
		QuakeFx.start = ARX_CLEAN_WARN_CAST_ULONG(FrameTime);

		
		QuakeFx.duration=(unsigned long)duration;
		QuakeFx.frequency=period;
		QuakeFx.flags=flags;

		if (flags & 1)
			ARX_SOUND_PlaySFX(SND_QUAKE, NULL, 1.0F - 0.5F * QuakeFx.intensity);
	}

	if (!(flags & 1))
	{
		if (QuakeFx.duration>1500) QuakeFx.duration=1500;

		if (QuakeFx.intensity>220) QuakeFx.intensity=220;		
	}
}
void ManageQuakeFX()
{
	if (QuakeFx.intensity>0.f)
	{
		float tim=(float)FrameTime-(float)QuakeFx.start;

		if (tim >= QuakeFx.duration)
		{
			QuakeFx.intensity=0.f;
			return;
		}

		float itmod=1.f-(tim/QuakeFx.duration);
		float periodicity=EEsin((float)FrameTime*QuakeFx.frequency*DIV100);

		if ((periodicity>0.5f) && (QuakeFx.flags & 1))
			ARX_SOUND_PlaySFX(SND_QUAKE, NULL, 1.0F - 0.5F * QuakeFx.intensity);

		float truepower=periodicity*QuakeFx.intensity*itmod*DIV100;
		float halfpower=truepower*DIV2;
		ACTIVECAM->pos.x+=rnd()*truepower-halfpower;
		ACTIVECAM->pos.y+=rnd()*truepower-halfpower;
		ACTIVECAM->pos.z+=rnd()*truepower-halfpower;
		ACTIVECAM->angle.a+=rnd()*truepower-halfpower;
		ACTIVECAM->angle.g+=rnd()*truepower-halfpower;
		ACTIVECAM->angle.b+=rnd()*truepower-halfpower;	
	}
}

void ProcessAllTheo(char * path)
{
	long idx;
	char pathh[512];
	struct _finddata_t fd;
	sprintf(pathh,"%s*.*",path);

	if ((idx=_findfirst(pathh,&fd))!=-1) 
	{
		do 
		{
			if (strcmp(fd.name,".") && strcmp(fd.name,".."))
			{
				if (fd.attrib & _A_SUBDIR)
				{
					char path2[512];
					sprintf(path2,"%s%s\\",path,fd.name);
					ProcessAllTheo(path2);
				}
				else
				{
					char ext[256];
					strcpy(ext,GetExt(fd.name));

					if (!stricmp(ext,".teo"))
					{
						char path2[512];
						char texpath[512];
						sprintf(path2,"%s%s",path,fd.name);
						sprintf(texpath,"%sGraph\\Obj3D\\Textures\\",Project.workingdir);
						EERIE_3DOBJ * temp;
						char tx[1024];
						sprintf(tx,"Moulinex %s (%s - %s)",fd.name,path2,texpath);
						ForceSendConsole(tx,1,0,NULL);		
						_ShowText(tx);		

						if (strstr(path2,"\\NPC\\"))							
							temp=TheoToEerie_Fast(texpath,path2,TTE_NPC,GDevice);
						else 
							temp=TheoToEerie_Fast(texpath,path2,0,GDevice);

						if (temp) 
						{
							ReleaseEERIE3DObj(temp);
							ReleaseAllTCWithFlag(0);
						}
					}
				}
			}
		}
		while (!(_findnext(idx, &fd)));

		_findclose(idx);
	}		
}
void LaunchMoulinex()
{
	char tx[256];

	if (PROCESS_ALL_THEO)
	{		
		sprintf(tx,"Moulinex THEO convertALL START________________");
		ForceSendConsole(tx,1,0,NULL);
		_ShowText(tx);
		ProcessAllTheo(Project.workingdir);
		sprintf(tx,"Moulinex THEO convertALL END__________________");
		ForceSendConsole(tx,1,0,NULL);
		_ShowText(tx);
		PROCESS_ALL_THEO=0;

		if (KILL_AT_MOULINEX_END)
		{
			DANAEFinalCleanup();
			exit(0);
		}
	}

	if (PROCESS_ONLY_ONE_LEVEL!=-1)
	{
		PROCESS_LEVELS=1;
	}

	if (PROCESS_LEVELS==0)
	{
		
		MOULINEX=0;
		LASTMOULINEX=-1;

		if (KILL_AT_MOULINEX_END)
		{
			DANAEFinalCleanup();
			exit(0);
		}
		else ShowPopup("Moulinex Successfull");

		return;
	}

	long lvl=MOULINEX-1;

	if (PROCESS_ONLY_ONE_LEVEL!=-1)
		lvl=PROCESS_ONLY_ONE_LEVEL;

	sprintf(tx,"Moulinex Lvl %d",lvl);
	ForceSendConsole(tx,1,0,NULL);
	_ShowText(tx);		

	if (LASTMOULINEX!=-1)
	{
		char saveto[256];
		long lastlvl;

		if (PROCESS_ONLY_ONE_LEVEL!=-1)
			lastlvl=PROCESS_ONLY_ONE_LEVEL;
		else
			lastlvl=MOULINEX-2;

		GetLevelNameByNum(lastlvl,tx);
		sprintf(saveto,"%s\\Graph\\Levels\\Level%s\\level%s.dlf",Project.workingdir,tx,tx);

		if (FileExist(saveto))
		{
			long oldmode=ModeLight;
			ModeLight=MODE_NORMALS | MODE_RAYLAUNCH | MODE_STATICLIGHT | MODE_DYNAMICLIGHT | MODE_DEPTHCUEING;

			if (TSU_LIGHTING) ModeLight|=MODE_SMOOTH;

			EERIERemovePrecalcLights();
			EERIEPrecalcLights(0,0,999999,999999);		
			DanaeSaveLevel(saveto);		
			ModeLight=oldmode;
		}

		if (PROCESS_ONLY_ONE_LEVEL!=-1)
		{
			DANAEFinalCleanup();
			exit(0);
		}
	}

	if (MOULINEX>=32) 
	{		
		MOULINEX=0;
		LASTMOULINEX=-1;

		if (KILL_AT_MOULINEX_END)
		{
			DANAEFinalCleanup();
			exit(0);
		}
		else ShowPopup("Moulinex Successfull");

		return;
	}
	
	if (PROCESS_ONLY_ONE_LEVEL!=-1)
	{
		lvl=PROCESS_ONLY_ONE_LEVEL;
	}

	{
		char loadfrom[256];
		
		GetLevelNameByNum(lvl,tx);

		if (stricmp(tx,"NONE"))
		{
			sprintf(loadfrom,"%s\\Graph\\Levels\\Level%s\\level%s.dlf",Project.workingdir,tx,tx);

			if (FileExist(loadfrom))
			{
				if (CDP_LIGHTOptions!=NULL) SendMessage(CDP_LIGHTOptions,WM_CLOSE,0,0);

				if (CDP_FogOptions!=NULL) SendMessage(CDP_FogOptions,WM_CLOSE,0,0);

				CDP_LIGHTOptions=NULL;
				CDP_FogOptions=NULL;
				SetEditMode(1);
				DanaeClearLevel();
				DanaeLoadLevel(GDevice,loadfrom);
				FORBID_SAVE=0;
				FirstFrame=1;
			}
		}
		
	}

	if (PROCESS_ONLY_ONE_LEVEL!=-1)
		LASTMOULINEX=PROCESS_ONLY_ONE_LEVEL;
	else LASTMOULINEX=MOULINEX;

	MOULINEX++;
}

void DANAE_StartNewQuest()
{
	player.Interface = INTER_LIFE_MANA | INTER_MINIBACK | INTER_MINIBOOK;
	PROGRESS_BAR_TOTAL = 108;
	OLD_PROGRESS_BAR_COUNT=PROGRESS_BAR_COUNT=0;
	LoadLevelScreen(GDevice,1);
	char loadfrom[256];
	sprintf(loadfrom,"%sGraph\\Levels\\Level1\\Level1.dlf",Project.workingdir);
	DONT_ERASE_PLAYER=1;							
	DanaeClearAll();
	PROGRESS_BAR_COUNT+=2.f;
	LoadLevelScreen();
	DanaeLoadLevel(GDevice,loadfrom);
	FORBID_SAVE=0;
	FirstFrame=1;
	START_NEW_QUEST=0;
	STARTED_A_GAME=1;
	BLOCK_PLAYER_CONTROLS = 0;
	FADEDURATION=0;
	FADEDIR=0;
	player.Interface = INTER_LIFE_MANA | INTER_MINIBACK | INTER_MINIBOOK;
}

BOOL DANAE_ManageSplashThings()
{
	GDevice->SetTextureStageState(0,D3DTSS_ADDRESS,D3DTADDRESS_CLAMP);
	
	SetFilteringMode(GDevice,Bilinear);		

	if (SPLASH_THINGS_STAGE>10)
	{
		if (EDITMODE || bGameNotFirstLaunch)
		{
			for (int i=0; i<256; i++)
			{
				pGetInfoDirectInput->iOneTouch[i] = 0;
			}

			pGetInfoDirectInput->GetInput();

			for (int i=0; i<256; i++)
			{
				if (pGetInfoDirectInput->iOneTouch[i] > 0)
				{
					REFUSE_GAME_RETURN=1;
					FORBID_SAVE=0;
					FirstFrame=1;
					SPLASH_THINGS_STAGE=0;
					INTRO_NOT_LOADED=0;
					ARXmenu.currentmode=AMCM_MAIN;
					ARX_MENU_Launch(GDevice);
				}
			}

			if (ARX_IMPULSE_Pressed(DIK_ESCAPE))
			{
				REFUSE_GAME_RETURN=1;
				SPLASH_THINGS_STAGE = 14;
			}
		}

		if (FAST_SPLASHES)
			SPLASH_THINGS_STAGE=14;
		
		if (SPLASH_THINGS_STAGE==11) 
		{
			// Playing the videos in startupvids.txt
			char startupvidsPath[256];
			sprintf(startupvidsPath,"%smisc\\startupvids.txt",Project.workingdir);			

			if ((FileExist(startupvidsPath)) && (SPLASH_START == 0))
			{
				std::ifstream stStartupVids;
				stStartupVids.open(startupvidsPath);
				char vidToPlay[64];
				char vidToPlayPath[256];

				while(stStartupVids.good())
				{
					stStartupVids.getline(vidToPlay,64);					
					sprintf(vidToPlayPath,"%smisc\\%s",Project.workingdir,vidToPlay);					
					bSkipVideoIntro = false; // We need to reset this else we'll skip all vids w/ one key pressed

					if (FileExist(vidToPlayPath)) 
					{
						LaunchAVI(danaeApp.m_hWnd,vidToPlayPath);	
						pGetInfoDirectInput->ResetAll(); // We need to reset all input else we'll skip all vids w/ one key pressed
					}
				}
			
				if (bSkipVideoIntro)
				{
					REFUSE_GAME_RETURN=1;
					FORBID_SAVE=0;
					FirstFrame=1;
					SPLASH_THINGS_STAGE=0;
					INTRO_NOT_LOADED=0;
					ARXmenu.currentmode=AMCM_MAIN;
					ARX_MENU_Launch(GDevice);
				}
					
				SPLASH_START=0;
				SPLASH_THINGS_STAGE++;
				GDevice->SetTextureStageState(0,D3DTSS_ADDRESS,D3DTADDRESS_WRAP);
				return TRUE;
			}
				
			if (SPLASH_START==0) //firsttime
				SPLASH_START = ARX_TIME_GetUL();
				
			ARX_INTERFACE_ShowFISHTANK(GDevice);
					
			unsigned long tim = ARX_TIME_GetUL();
			float pos=(float)tim-(float)SPLASH_START;

			if (pos>3600)
			{
				SPLASH_START=0;
				SPLASH_THINGS_STAGE++;
			}				

			GDevice->SetTextureStageState(0,D3DTSS_ADDRESS,D3DTADDRESS_WRAP);
			return TRUE;
			
		}

		if (SPLASH_THINGS_STAGE==12) 
		{
			if (SPLASH_START==0) //firsttime
			{
				SPLASH_START = ARX_TIME_GetUL();
				ARX_SOUND_PlayInterface(SND_PLAYER_HEART_BEAT);
			}

			ARX_INTERFACE_ShowARKANE(GDevice);
			unsigned long tim = ARX_TIME_GetUL();
			float pos=(float)tim-(float)SPLASH_START;

			if (pos>3600)
			{
				SPLASH_START=0;
				SPLASH_THINGS_STAGE++;
			}

			GDevice->SetTextureStageState(0,D3DTSS_ADDRESS,D3DTADDRESS_WRAP);
			return TRUE;
		}

		if (SPLASH_THINGS_STAGE==13)
		{
			ARX_INTERFACE_KillFISHTANK();
			ARX_INTERFACE_KillARKANE();					
			char loadfrom[256];

			if (CEDRIC_VERSION)
			{
				sprintf(loadfrom,"%sGraph\\Levels\\LevelDemo2\\levelDemo2.dlf",Project.workingdir);
				LoadLevelScreen(GDevice,29);	
			}
			else
			{
				REFUSE_GAME_RETURN=1;
				sprintf(loadfrom,"%sGraph\\Levels\\Level10\\level10.dlf",Project.workingdir);
				OLD_PROGRESS_BAR_COUNT=PROGRESS_BAR_COUNT=0;
				PROGRESS_BAR_TOTAL = 108;
				LoadLevelScreen(GDevice,10);	
			}

			DanaeLoadLevel(GDevice,loadfrom);
			FORBID_SAVE=0;
			FirstFrame=1;
			SPLASH_THINGS_STAGE=0;
			INTRO_NOT_LOADED=0;

			if (bGameNotFirstLaunch == false)
				bGameNotFirstLaunch = true;

			GDevice->SetTextureStageState(0,D3DTSS_ADDRESS,D3DTADDRESS_WRAP);
			return TRUE;
			
		}

		if (SPLASH_THINGS_STAGE > 13)
		{
			FORBID_SAVE=0;
			FirstFrame=1;
			SPLASH_THINGS_STAGE=0;
			INTRO_NOT_LOADED=0;

			if (bGameNotFirstLaunch == false)
				bGameNotFirstLaunch = true;

			GDevice->SetTextureStageState(0,D3DTSS_ADDRESS,D3DTADDRESS_WRAP);
			return TRUE;
		}
	}

	GDevice->SetTextureStageState(0,D3DTSS_ADDRESS,D3DTADDRESS_WRAP);
	return FALSE;
}

//*************************************************************************************
// Manages Currently playing 2D cinematic
//*************************************************************************************
long DANAE_Manage_Cinematic()
{
	float FrameTicks=ARX_TIME_Get( false );

	if (PLAY_LOADED_CINEMATIC==1)
	{
		LastFrameTicks=FrameTicks;
		PLAY_LOADED_CINEMATIC=2;
	}

	PlayTrack(ControlCinematique);
	ControlCinematique->InitDeviceObjects();
	danaeApp.m_pd3dDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);

	if(ControlCinematique->Render(FrameTicks-LastFrameTicks)==E_FAIL) 
		return 1;

	//fin de l'anim
	if ((!ControlCinematique->key)
		|| (pGetInfoDirectInput->IsVirtualKeyPressedNowUnPressed(DIK_ESCAPE))
		|| (pGetInfoDirectInput->IsVirtualKeyPressedNowUnPressed(DIK_ESCAPE)))
	{			
		ControlCinematique->projectload=FALSE;
		StopSoundKeyFramer();
		ControlCinematique->OneTimeSceneReInit();
		ControlCinematique->DeleteDeviceObjects();			
		ARX_TIME_UnPause();	
		PLAY_LOADED_CINEMATIC=0;
		bool bWasBlocked = false;

		if (BLOCK_PLAYER_CONTROLS)
			bWasBlocked = true;

		// !! avant le cine end
		if (ACTIVECAM)
		{
			ACTIVECAM->pos.x = ePos.x;
			ACTIVECAM->pos.y = ePos.y;
			ACTIVECAM->pos.z = ePos.z;
		}

		if (bWasBlocked)
			BLOCK_PLAYER_CONTROLS=1;

		ARX_SPEECH_Reset();
		SendMsgToAllIO(SM_CINE_END,LAST_LAUNCHED_CINE);
	}

	LastFrameTicks=FrameTicks;
	return 0;
}
void DanaeItemAdd()
{
	INTERACTIVE_OBJ * tmp=AddInteractive(GDevice,ItemToBeAdded,0,IO_IMMEDIATELOAD);

	if (tmp!=NULL) 
	{
		ARX_INTERACTIVE_HideGore(tmp);
		ADDED_IO_NOT_SAVED++;
		tmp->EditorFlags |= EFLAG_NOTSAVED;
		InterTreeViewItemAdd(tmp);
		RestoreInitialIOStatusOfIO(tmp);
		long num=GetInterNum(tmp);

		if (ValidIONum(num))
		{
			if (inter.iobj[num] && inter.iobj[num]->script.data) 
			{
				SendScriptEvent(&inter.iobj[num]->script,SM_INIT,"",inter.iobj[num],NULL);
			}

			if (inter.iobj[num] && inter.iobj[num]->over_script.data) 
			{
				SendScriptEvent(&inter.iobj[num]->over_script,SM_INIT,"",inter.iobj[num],NULL);
			}

			if (inter.iobj[num] && inter.iobj[num]->script.data) 
			{
				SendScriptEvent(&inter.iobj[num]->script,SM_INITEND,"",inter.iobj[num],NULL);
			}

			if (inter.iobj[num] && inter.iobj[num]->over_script.data) 
			{
				SendScriptEvent(&inter.iobj[num]->over_script,SM_INITEND,"",inter.iobj[num],NULL);
			}
		}
	}

	ItemToBeAdded[0]=0;
}

void ReMappDanaeButton()
{
	if(!pGetInfoDirectInput) return;

	bool bNoAction=true;
	int iButton=pMenuConfig->sakActionKey[CONTROLS_CUST_ACTION].iKey[0];

	if(iButton!=-1)
	{
		if(pGetInfoDirectInput->GetMouseButtonDoubleClick(iButton&~0x80000000,300))
		{
			LastEERIEMouseButton=EERIEMouseButton;
			EERIEMouseButton|=4;
			EERIEMouseButton&=~1;
			bNoAction=false;
		}
	}

	if(bNoAction)
	{
		iButton=pMenuConfig->sakActionKey[CONTROLS_CUST_ACTION].iKey[1];

		if(iButton!=-1)
		{
			if(pGetInfoDirectInput->GetMouseButtonDoubleClick(iButton&~0x80000000,300))
			{
				LastEERIEMouseButton=EERIEMouseButton;
				EERIEMouseButton|=4;
				EERIEMouseButton&=~1;
			}
		}
	}

	bNoAction=true;
	iButton=pMenuConfig->sakActionKey[CONTROLS_CUST_ACTION].iKey[0];

	if(iButton!=-1)
	{
		if(	((iButton&0x80000000)&&(pGetInfoDirectInput->GetMouseButtonNowPressed(iButton&~0x80000000)))||
			((!(iButton&0x80000000))&&pGetInfoDirectInput->IsVirtualKeyPressedNowPressed(iButton)) )
		{
			LastEERIEMouseButton=EERIEMouseButton;
			EERIEMouseButton|=1;

			if (EERIEMouseButton&4) EERIEMouseButton&=~1;

			bNoAction=false;
		}
	}

	if(bNoAction)
	{
		iButton=pMenuConfig->sakActionKey[CONTROLS_CUST_ACTION].iKey[1];

		if(iButton!=-1)
		{
			if( ((iButton&0x80000000)&&(pGetInfoDirectInput->GetMouseButtonNowPressed(iButton&~0x80000000)))||
				((!(iButton&0x80000000))&&pGetInfoDirectInput->IsVirtualKeyPressedNowPressed(iButton)) )
			{
				LastEERIEMouseButton=EERIEMouseButton;
				EERIEMouseButton|=1;

				if (EERIEMouseButton&4) EERIEMouseButton&=~1;
			}
		}
	}

	bNoAction=true;
	iButton=pMenuConfig->sakActionKey[CONTROLS_CUST_ACTION].iKey[0];

	if(iButton!=-1)
	{
		if(	((iButton&0x80000000)&&(pGetInfoDirectInput->GetMouseButtonNowUnPressed(iButton&~0x80000000)))||
			((!(iButton&0x80000000))&&pGetInfoDirectInput->IsVirtualKeyPressedNowPressed(iButton)) )
		{
			LastEERIEMouseButton=EERIEMouseButton;
			EERIEMouseButton&=~1;
			EERIEMouseButton&=~4;
			bNoAction=false;
		}
	}

	if(bNoAction)
	{
		iButton=pMenuConfig->sakActionKey[CONTROLS_CUST_ACTION].iKey[1];

		if(iButton!=-1)
		{
			if( ((iButton&0x80000000)&&(pGetInfoDirectInput->GetMouseButtonNowUnPressed(iButton&~0x80000000)))||
				((!(iButton&0x80000000))&&pGetInfoDirectInput->IsVirtualKeyPressedNowPressed(iButton)) )
			{
				LastEERIEMouseButton=EERIEMouseButton;
				EERIEMouseButton&=~1;
				EERIEMouseButton&=~4;
			}
		}
	}

	bNoAction=true;
	iButton=pMenuConfig->sakActionKey[CONTROLS_CUST_MOUSELOOK].iKey[0];

	if(iButton!=-1)
	{
		if(	((iButton&0x80000000)&&(pGetInfoDirectInput->GetMouseButtonNowPressed(iButton&~0x80000000)))||
			((!(iButton&0x80000000))&&pGetInfoDirectInput->IsVirtualKeyPressedNowPressed(iButton)) )
		{
			EERIEMouseButton|=2;
			bNoAction=false;
		}
	}

	if(bNoAction)
	{
		iButton=pMenuConfig->sakActionKey[CONTROLS_CUST_MOUSELOOK].iKey[1];

		if(iButton!=-1)
		{
			if( ((iButton&0x80000000)&&(pGetInfoDirectInput->GetMouseButtonNowPressed(iButton&~0x80000000)))||
				((!(iButton&0x80000000))&&pGetInfoDirectInput->IsVirtualKeyPressedNowPressed(iButton)) )
			{
				EERIEMouseButton|=2;
			}
		}
	}
	
	bNoAction=true;
	iButton=pMenuConfig->sakActionKey[CONTROLS_CUST_MOUSELOOK].iKey[0];

	if(iButton!=-1)
	{
		if(	((iButton&0x80000000)&&(pGetInfoDirectInput->GetMouseButtonNowUnPressed(iButton&~0x80000000)))||
			((!(iButton&0x80000000))&&pGetInfoDirectInput->IsVirtualKeyPressedNowUnPressed(iButton)) )
		{
			EERIEMouseButton&=~2;
			bNoAction=false;
		}
	}

	if(bNoAction)
	{
		iButton=pMenuConfig->sakActionKey[CONTROLS_CUST_MOUSELOOK].iKey[1];

		if(iButton!=-1)
		{
			if( ((iButton&0x80000000)&&(pGetInfoDirectInput->GetMouseButtonNowUnPressed(iButton&~0x80000000)))||
				((!(iButton&0x80000000))&&pGetInfoDirectInput->IsVirtualKeyPressedNowUnPressed(iButton)) )
			{
				EERIEMouseButton&=~2;
			}
		}
	}
}
long NEED_SPECIAL_RENDEREND=0;
long INTERPOLATE_BETWEEN_BONES=1;

extern int iTimeToDrawD7;
extern long INTERTRANSPOLYSPOS;

extern long TRANSPOLYSPOS;

extern bool bRenderInterList;
unsigned long BENCH_STARTUP=0;
unsigned long BENCH_PLAYER=0;
unsigned long BENCH_RENDER=0;
unsigned long BENCH_PARTICLES=0;
unsigned long BENCH_SPEECH=0;
unsigned long BENCH_SCRIPT=0;

unsigned long oBENCH_STARTUP=0;
unsigned long oBENCH_PLAYER=0;
unsigned long oBENCH_RENDER=0;
unsigned long oBENCH_PARTICLES=0;
unsigned long oBENCH_SPEECH=0;
unsigned long oBENCH_SCRIPT=0;

extern unsigned long BENCH_PATHFINDER;
unsigned long oBENCH_PATHFINDER=0;

extern unsigned long BENCH_SOUND;
unsigned long oBENCH_SOUND=0;

long WILL_QUICKLOAD=0;
long WILL_QUICKSAVE=0;
void DemoFileCheck()
{
	return;

	if (!FINAL_COMMERCIAL_DEMO)
	{
		char fic[256];
		sprintf(fic,"%sGraph\\Obj3D\\Interactive\\NPC\\Undead_Liche\\Undead_Liche.asl",Project.workingdir);

		if (!PAK_FileExist(fic))
		{
			FINAL_COMMERCIAL_DEMO=1;
		}
	}
}
void CorrectValue(unsigned long * cur,unsigned long * dest)
{
	if (*cur=*dest)
		return;

	if (*cur<*dest)
	{
		*cur = *dest;
		return;
	}

	if (*cur>=1)
	{
		*cur-=1;
		return;
	}

	*cur=0;
}
long iVPOS=0;
void ShowValue(unsigned long * cur,unsigned long * dest,char * str)
{
	iVPOS+=1;
	CorrectValue(cur,dest);
	D3DCOLOR col;
	EERIE_RGB rgb;

	switch (iVPOS)
	{
		case 0:
			rgb.r = 1;
			rgb.g = 0;
			rgb.b = 0;
			break;
		case 1:
			rgb.r = 0;
			rgb.g = 1;
			rgb.b = 0;
			break;
		case 2:
			rgb.r = 0;
			rgb.g = 0;
			rgb.b = 1;
			break;
		case 3:
			rgb.r = 1;
			rgb.g = 1;
			rgb.b = 0;
			break;
		case 4:
			rgb.r = 1;
			rgb.g = 0;
			rgb.b = 1;
			break;
		case 5:
			rgb.r = 0;
			rgb.g = 1;
			rgb.b = 1;
			break;
		case 6:
			rgb.r = 1;
			rgb.g = 1;
			rgb.b = 1;
			break;
		case 7:
			rgb.r = 1;
			rgb.g = 0.5f;
			rgb.b = 0.5f;
			break;
		case 8:
			rgb.r = 0.5;
			rgb.g = 0.5f;
			rgb.b = 1.f;
			break;
		default:
			rgb.r = 0.5f;
			rgb.g = 0.5f;
			rgb.b = 0.5f;
			break;
	}

	col=EERIERGB(rgb.r,rgb.g,rgb.b);
	float width=(float)(*cur)*DIV500;
	EERIEDrawBitmap(danaeApp.m_pd3dDevice, 0, ARX_CLEAN_WARN_CAST_FLOAT(iVPOS * 16), width, 8, 0.000091f, NULL, col);
	danaeApp.OutputText(ARX_CLEAN_WARN_CAST_DWORD(width), iVPOS * 16 - 2, str);

}

extern DWORD RenderStartTicks;
extern long NEED_INTRO_LAUNCH;

//-----------------------------------------------------------------------------

HRESULT DANAE::Render()
{
	FrameTime = ARX_TIME_Get();

	if (GLOBAL_SLOWDOWN!=1.f)
	{
		float ft;
		ft=FrameTime-LastFrameTime;
		Original_framedelay=ft*TIMEFACTOR;
			
		ft*=1.f-GLOBAL_SLOWDOWN;
		float minus;

		minus = ft;
		ARXTotalPausedTime+=minus;
		FrameTime = ARX_TIME_Get();

		if (LastFrameTime>FrameTime)
		{
			LastFrameTime=FrameTime;
		}

		ft=FrameTime-LastFrameTime;
	
		FrameDiff = ft;
		float FD;
		FD=FrameDiff;
		// Under 10 FPS the whole game slows down to avoid unexpected results...
		_framedelay=(float)FrameDiff;
	}
	else
	{
		FrameDiff = FrameTime-LastFrameTime;

		float FD;
		FD=FrameDiff;
		// Under 10 FPS the whole game slows down to avoid unexpected results...
		_framedelay=((float)(FrameDiff)*TIMEFACTOR);
		FrameDiff = _framedelay;

		Original_framedelay=_framedelay;

//	Original_framedelay = 1000/25;
		ARXTotalPausedTime+=FD-FrameDiff;
	}

static float _AvgFrameDiff = 150.f;
	if( FrameDiff > _AvgFrameDiff * 10.f )
	{
		FrameDiff = _AvgFrameDiff * 10.f;
	}
	else if ( FrameDiff > 15.f )
	{
		_AvgFrameDiff+= (FrameDiff - _AvgFrameDiff )*0.01f;
	}
	if (NEED_BENCH)
	{
		BENCH_STARTUP=0;
		BENCH_PLAYER=0;
		BENCH_RENDER=0;
		BENCH_PARTICLES=0;
		BENCH_SPEECH=0;
		BENCH_SCRIPT=0;
	}

	StartBench();

	RenderStartTicks	=	dwARX_TIME_Get();

	if(bForceGDI)
	{
		HDC hDC;

		if( SUCCEEDED( m_pddsRenderTarget->GetDC(&hDC) ) )
		{
			m_pddsRenderTarget->ReleaseDC(hDC);
		}
	}

	if(	(pGetInfoDirectInput)&&
		(pGetInfoDirectInput->IsVirtualKeyPressedNowPressed(DIK_F12)))
	{
		bGMergeVertex=!bGMergeVertex;
		EERIE_PORTAL_ReleaseOnlyVertexBuffer();
		ComputePortalVertexBuffer();
	}

	ACTIVECAM = &subj;

	if (	(!FINAL_COMMERCIAL_DEMO)
		&&	(!FINAL_COMMERCIAL_GAME)	
		&&  (ARXmenu.currentmode==AMCM_OFF)	)
	{
		if(	(pGetInfoDirectInput)&&
			(pGetInfoDirectInput->IsVirtualKeyPressedOneTouch(DIK_Y)) )
		{
			USE_OLD_MOUSE_SYSTEM=(USE_OLD_MOUSE_SYSTEM)?0:1;

			if(!USE_OLD_MOUSE_SYSTEM)
			{
				current.depthcolor.r=0.f;
				current.depthcolor.g=0.f;
				current.depthcolor.b=1.f;
			}
		}
	}

	//BUMP
	if(pMenuConfig->bBumpMapping)
	{
		e3dPosBump=player.pos;
	}
	
	// Sets our Global Device to Current Device
	GDevice=m_pd3dDevice;

	if (this->m_pFramework->m_bHasMoved)
	{
		DanaeRestoreFullScreen();

		this->m_pFramework->m_bHasMoved=FALSE;

		if(pMenu)
		{
			pMenu->bReInitAll=true;
		}

		DANAESIZX=danaeApp.m_pFramework->m_dwRenderWidth;
		DANAESIZY=danaeApp.m_pFramework->m_dwRenderHeight;

		if (danaeApp.m_pDeviceInfo->bWindowed)
			DANAESIZY-=danaeApp.m_pFramework->Ystart;	

		DANAECENTERX=DANAESIZX>>1;
		DANAECENTERY=DANAESIZY>>1;
		
		Xratio=DANAESIZX*DIV640; 
		Yratio=DANAESIZY*DIV480; 
	}

	// Get DirectInput Infos
	if ((!USE_OLD_MOUSE_SYSTEM))
	{
		pGetInfoDirectInput->GetInput();
		ReMappDanaeButton();
	}

	// Manages Splash Screens if needed
	if (DANAE_ManageSplashThings())
		goto norenderend;

	// Clicked on New Quest ? (TODO:need certainly to be moved somewhere else...)
	if (START_NEW_QUEST)
	{
		LaunchCDROMCheck(0);
		DANAE_StartNewQuest();
	}

	// Update Various Player Infos for this frame.
	if (FirstFrame==0)
		ARX_PLAYER_Frame_Update();

	// Checks for ESC key
	if (pGetInfoDirectInput->IsVirtualKeyPressedNowUnPressed(DIK_ESCAPE))
	{
		if (ARXmenu.currentmode == AMCM_OFF)
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
		}
		else
		{
			//create a screenshot temporaire pour la sauvegarde
			::SnapShot *pSnapShot=new ::SnapShot(NULL,"sct",true);
			pSnapShot->GetSnapShotDim(160,100);
			delete pSnapShot;

			ARX_TIME_Pause();
			ARXTimeMenu=ARXOldTimeMenu=ARX_TIME_Get();
			ARX_MENU_Launch(m_pd3dDevice);
			bFadeInOut=false;	//fade out
			bFade=true;			//active le fade
			pGetInfoDirectInput->iOneTouch[DIK_ESCAPE] = 0;
			TRUE_PLAYER_MOUSELOOK_ON = 0;

			ARX_PLAYER_PutPlayerInNormalStance(1);
		}
	}
	}
	
	// Project need to reload all textures ???
	if (WILL_RELOAD_ALL_TEXTURES) 
	{
		ReloadAllTextures(GDevice);

		if(ControlCinematique)
		{
			ControlCinematique->m_pd3dDevice=GDevice;
			ActiveAllTexture(ControlCinematique);
		}

		WILL_RELOAD_ALL_TEXTURES=0;
	}
	
	// Are we being teleported ?
	if ((TELEPORT_TO_LEVEL[0]) && (CHANGE_LEVEL_ICON==200))
	{
		CHANGE_LEVEL_ICON=-1;
		LaunchCDROMCheck(0);
		ARX_CHANGELEVEL_Change(TELEPORT_TO_LEVEL, TELEPORT_TO_POSITION, TELEPORT_TO_ANGLE, 0);
		memset(TELEPORT_TO_LEVEL,0,64);
		memset(TELEPORT_TO_POSITION,0,64);
	}

	if (NEED_INTRO_LAUNCH)
	{
		SetEditMode(0);
		BLOCK_PLAYER_CONTROLS=1;
		ARX_INTERFACE_PlayerInterfaceModify(0,0);
		ARX_Menu_Resources_Release();
		ARXmenu.currentmode=AMCM_OFF;
		ARX_TIME_UnPause();
		SPLASH_THINGS_STAGE=14;
		NEED_INTRO_LAUNCH=0;
		char loadfrom[256];
		REFUSE_GAME_RETURN=1;
		sprintf(loadfrom,"%sGraph\\Levels\\Level10\\level10.dlf",Project.workingdir);
		OLD_PROGRESS_BAR_COUNT=PROGRESS_BAR_COUNT=0;
		PROGRESS_BAR_TOTAL = 108;
		LoadLevelScreen(GDevice,10);	
		DanaeLoadLevel(GDevice,loadfrom);
		FORBID_SAVE=0;
		FirstFrame=1;
		SPLASH_THINGS_STAGE=0;
		INTRO_NOT_LOADED=0;
		GDevice->SetTextureStageState(0,D3DTSS_ADDRESS,D3DTADDRESS_WRAP);
		return FALSE;
	}

	// Little security Feature...
	#ifdef ASKPASS

		if ((rnd()>0.5f) && (!accepted)) exit(0);

	#endif

	// Sets Danae Screen size depending on windowed/full-screen state
	DANAESIZX=this->m_pFramework->m_dwRenderWidth;
	DANAESIZY=this->m_pFramework->m_dwRenderHeight;

	if ((m_pDeviceInfo->bWindowed) && (!FINAL_RELEASE))
		DANAESIZY-=this->m_pFramework->Ystart;	

	// Now computes screen center

	//Setting long from long
	subj.centerx = DANAECENTERX = DANAESIZX>>1;
	subj.centery = DANAECENTERY = DANAESIZY>>1;
	//Casting long to float
	subj.posleft = subj.transform.xmod = ARX_CLEAN_WARN_CAST_FLOAT( DANAECENTERX );
	subj.postop	 = subj.transform.ymod = ARX_CLEAN_WARN_CAST_FLOAT( DANAECENTERY );


	// Computes X & Y screen ratios compared to a standard 640x480 screen
	if (DANAESIZX == 640) Xratio = 1.f;
	else Xratio = DANAESIZX * DIV640;

	if (DANAESIZY == 480) Yratio = 1.f;
	else Yratio = DANAESIZY * DIV480;

	// Finally computes current focal
	BASE_FOCAL=(float)CURRENT_BASE_FOCAL+(float)FOKMOD+(BOW_FOCAL*DIV4);

	// SPECIFIC code for Snapshot MODE... to insure constant capture framerate
	if (	(SnapShotMode)
		&&	(!ARXPausedTime)	)
	{		
		ARXTotalPausedTime+=ARXTime-(LastFrameTime+(1000/snapshotdata.imgsec));		
	}

	PULSATE=EEsin(FrameTime*DIV800);
	METALdecal=EEsin(FrameTime*DIV50)*DIV200;
	PULSS=EEsin(FrameTime*DIV200)*DIVPI*DIV4+0.25f;
	EERIEDrawnPolys=0;
	
	// EditMode Specific code
	if (EDITMODE) 
	{
		TOTIOPDL=0;
		BLOCK_PLAYER_CONTROLS=0;
	}

	if (FirstFrame==0) // Checks for Keyboard & Moulinex
	{
		ARX_MOUSE_OVER=0;

		if (!EDITMODE && (ARXmenu.currentmode == AMCM_OFF)) // Playing Game
		{
			// Checks Clicks in Book Interface
			if (ARX_INTERFACE_MouseInBook())
			{
				ARX_MOUSE_OVER|=ARX_MOUSE_OVER_BOOK;
				LASTBOOKBUTTON=BOOKBUTTON;
				BOOKBUTTON=EERIEMouseButton;

				if ( ((EERIEMouseButton & 1) && !(LastMouseClick & 1) )
					|| ((EERIEMouseButton & 2) && !(LastMouseClick & 2) ) )
				{
					bookclick.x=DANAEMouse.x;
					bookclick.y=DANAEMouse.y;
				}		
			}
			else if (InSecondaryInventoryPos(&DANAEMouse)) 
				ARX_MOUSE_OVER|=ARX_MOUSE_OVER_INVENTORY_2;
			else if (InPlayerInventoryPos(&DANAEMouse)) 
				ARX_MOUSE_OVER|=ARX_MOUSE_OVER_INVENTORY;
		}
		
		if (	(player.Interface & INTER_COMBATMODE)
			||	(PLAYER_MOUSELOOK_ON) )
		{
			FlyingOverIO = NULL; // Avoid to check with those modes
		}
		else
		{
			if ((DRAGINTER == NULL) && (FRAME_COUNT<=0))
			{
				if (!BLOCK_PLAYER_CONTROLS && !TRUE_PLAYER_MOUSELOOK_ON && !(ARX_MOUSE_OVER & ARX_MOUSE_OVER_BOOK)
					&& (eMouseState != MOUSE_IN_NOTE)
				   )
					FlyingOverIO = FlyingOverObject(&DANAEMouse);
				else
					FlyingOverIO = NULL;
			}
		}

		if (	(!PLAYER_PARALYSED)
			||	(ARXmenu.currentmode != AMCM_OFF)	)

		{
			if (!STOP_KEYBOARD_INPUT) 
				ManageKeyMouse();
			else
			{
				STOP_KEYBOARD_INPUT++;

				if (STOP_KEYBOARD_INPUT>2) STOP_KEYBOARD_INPUT=0;
			}
		}

		if (MOULINEX)
			LaunchMoulinex();
	}
	else // Manages our first frameS
	{
		ARX_TIME_Get();
		long ffh=FirstFrameHandling( m_pd3dDevice);

		if (ffh== FFH_S_OK) return S_OK;

		if (ffh== FFH_GOTO_FINISH) goto norenderend;
	}

	if (CheckInPolyPrecis(player.pos.x,player.pos.y,player.pos.z))
	{
		LastValidPlayerPos.x=player.pos.x;
		LastValidPlayerPos.y=player.pos.y;
		LastValidPlayerPos.z=player.pos.z;
	}

	if ((!FINAL_RELEASE) && (ARXmenu.currentmode == AMCM_OFF))
	{
		if (this->kbd.inkey[INKEY_M]) 
		{
			USE_PORTALS++;

			if (USE_PORTALS>4) USE_PORTALS=0;

			if (USE_PORTALS==1) USE_PORTALS=2;

			this->kbd.inkey[INKEY_M]=0;
		}

		if (this->kbd.inkey[INKEY_P]) 
		{
			if (INTERPOLATE_BETWEEN_BONES)
				INTERPOLATE_BETWEEN_BONES=0;
			else
				INTERPOLATE_BETWEEN_BONES=1;

			this->kbd.inkey[INKEY_P]=0;
		}
	}

	// Updates Externalview 
	if (EXTERNALVIEWING) EXTERNALVIEW=1;
	else EXTERNALVIEW=0;
	
	GDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, false);

	if (ARX_Menu_Render(m_pd3dDevice)) 
	{
		goto norenderend;
	}

	if (WILL_QUICKSAVE)
	{
		::SnapShot *pSnapShot=new ::SnapShot(NULL,"sct",true);
		pSnapShot->GetSnapShotDim(160,100);
		delete pSnapShot;

		if (WILL_QUICKSAVE>=2)
		{
			LaunchCDROMCheck(0);
			ARX_QuickSave();
			WILL_QUICKSAVE=0;
		}
		else WILL_QUICKSAVE++;
	}

	if (WILL_QUICKLOAD)
	{
		WILL_QUICKLOAD=0;
		LaunchCDROMCheck(0);

		if (ARX_QuickLoad())
			NEED_SPECIAL_RENDEREND=1;
	}

	if (NEED_SPECIAL_RENDEREND)
	{
		NEED_SPECIAL_RENDEREND=0;
		goto norenderend;
	}

	GDevice->SetRenderState( D3DRENDERSTATE_FOGENABLE, true);
	GDevice->SetTextureStageState(0,D3DTSS_ADDRESS,D3DTADDRESS_WRAP);

	// Are we displaying a 2D cinematic ? Yes = manage it
	if (	PLAY_LOADED_CINEMATIC
		&&	ControlCinematique
	        &&	ControlCinematique->projectload)
	{
		if (DANAE_Manage_Cinematic()==1)
			goto norenderend;		

		goto renderend;
	}

	BENCH_STARTUP=EndBench();
	StartBench();
	
	if (ARXmenu.currentmode == AMCM_OFF)
	{
		if (!PLAYER_PARALYSED)
		{
			if	(ManageEditorControls()) goto finish;
		}

		if ((!BLOCK_PLAYER_CONTROLS) && (!PLAYER_PARALYSED))
		{
			ManagePlayerControls();
		}
	}

	ARX_PLAYER_Manage_Movement();

	ARX_PLAYER_Manage_Visual();

	if (FRAME_COUNT<=0) 
		ARX_MINIMAP_ValidatePlayerPos();

	BENCH_PLAYER=EndBench();
	
	// SUBJECTIVE VIEW UPDATE START  *********************************************************
	{
		// Clear screen & Z buffers
		if (desired.flags & GMOD_DCOLOR)
		{
			long DCOLOR=EERIERGB(current.depthcolor.r,current.depthcolor.g,current.depthcolor.b);
			m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,DCOLOR, 1.0f, 0L );
		}
		else
		{
			subj.bkgcolor=ulBKGColor;
			m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,subj.bkgcolor, 1.0f, 0L );
		}

		//-------------------------------------------------------------------------------
		//															DRAW CINEMASCOPE 16/9
		if (CINEMA_DECAL!=0.f)
		{
			D3DRECT rectz[2];
			rectz[0].x1 = rectz[1].x1 = 0;
			rectz[0].x2			=	rectz[1].x2	=	DANAESIZX;
			rectz[0].y1 = 0;

			ARX_CHECK_LONG( CINEMA_DECAL * Yratio );
			long	lMulResult	=	ARX_CLEAN_WARN_CAST_LONG( CINEMA_DECAL * Yratio );
			rectz[0].y2 		= lMulResult;
			rectz[1].y1 		= DANAESIZY - lMulResult;
			rectz[1].y2 = DANAESIZY;
			m_pd3dDevice->Clear( 2, rectz, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,0x00000000, 0.f, 0L );
		}
		//-------------------------------------------------------------------------------

	if(!danaeApp.DANAEStartRender()) 
	{
			return E_FAIL;
	}
	
	SETZWRITE(m_pd3dDevice, TRUE );
	SETALPHABLEND(m_pd3dDevice,FALSE);

	if ( (inter.iobj[0]) && (inter.iobj[0]->animlayer[0].cur_anim) )
	{
		ManageNONCombatModeAnimations();
		long old=USEINTERNORM;
		USEINTERNORM=0;
		float speedfactor;
		speedfactor=inter.iobj[0]->basespeed+inter.iobj[0]->speed_modif;

		if (cur_mr==3) speedfactor+=0.5f;

		if (cur_rf==3) speedfactor+=1.5f;

		if (speedfactor < 0) speedfactor = 0;

		long tFrameDiff;
		F2L(Original_framedelay,&tFrameDiff);
	
		if ((player.Interface & INTER_COMBATMODE) && (STRIKE_TIME))// need some precision for weapon...
		{
			float restore=ACTIVECAM->use_focal;

			if ((!EXTERNALVIEW) && (!BOW_FOCAL))
			{
				ACTIVECAM->use_focal=PLAYER_ARMS_FOCAL*Xratio;
			}

			float cur=0;

			while ((cur<tFrameDiff) && (!(inter.iobj[0]->ioflags & IO_FREEZESCRIPT)))
			{
				long step=__min(50,tFrameDiff);

				if (inter.iobj[0]->ioflags & IO_FREEZESCRIPT) step=0;


				float iCalc = step*speedfactor ;
				ARX_CHECK_ULONG(iCalc);

				EERIEDrawAnimQuat(	m_pd3dDevice,	inter.iobj[0]->obj,
					&inter.iobj[0]->animlayer[0],
					&inter.iobj[0]->angle,&inter.iobj[0]->pos, 
					ARX_CLEAN_WARN_CAST_ULONG(iCalc) 

					                  , inter.iobj[0], 0, 4);

					if ((player.Interface & INTER_COMBATMODE) && (inter.iobj[0]->animlayer[1].cur_anim != NULL))
				ManageCombatModeAnimations();
		
				if (inter.iobj[0]->animlayer[1].cur_anim!=NULL)
					ManageCombatModeAnimationsEND();

				cur+=step*speedfactor;
			}
			
			ACTIVECAM->use_focal=restore;
		}
		else
		{
			float restore=ACTIVECAM->use_focal;

			if ((!EXTERNALVIEW) && (!BOW_FOCAL))
			{
				ACTIVECAM->use_focal=PLAYER_ARMS_FOCAL*Xratio;
			}


			float val=(float)tFrameDiff*speedfactor;
			ARX_CHECK_LONG(val);
			
			if (inter.iobj[0]->ioflags & IO_FREEZESCRIPT) val=0;

			EERIEDrawAnimQuat(	m_pd3dDevice,	
				inter.iobj[0]->obj,
					&inter.iobj[0]->animlayer[0],
					&inter.iobj[0]->angle,
					&inter.iobj[0]->pos, 
					ARX_CLEAN_WARN_CAST_ULONG(val),
					inter.iobj[0],
					0,4);


				if ((player.Interface & INTER_COMBATMODE) && (inter.iobj[0]->animlayer[1].cur_anim != NULL))
				ManageCombatModeAnimations();
		
			if (inter.iobj[0]->animlayer[1].cur_anim!=NULL)
					ManageCombatModeAnimationsEND();

			ACTIVECAM->use_focal=restore;
		}

		USEINTERNORM=old;
	}

	INTERACTIVE_OBJ * io;
	io=inter.iobj[0];
	ANIM_USE * useanim;
	useanim=&io->animlayer[1];
	ANIM_HANDLE ** alist;
	alist=io->anims;

	if ( BOW_FOCAL
			&&	(useanim->cur_anim!=alist[ANIM_MISSILE_STRIKE_PART_1])
			&&	(useanim->cur_anim!=alist[ANIM_MISSILE_STRIKE_PART_2]) 
			&&  (useanim->cur_anim!=alist[ANIM_MISSILE_STRIKE_CYCLE]) )
		{
			BOW_FOCAL-=Original_framedelay;

			if (BOW_FOCAL<0) BOW_FOCAL=0;
		}

		if (eyeball.exist == 2)
		{
		subj.d_pos.x=eyeball.pos.x;
		subj.d_pos.y=eyeball.pos.y;
		subj.d_pos.z=eyeball.pos.z;		
		subj.d_angle.a=eyeball.angle.a;
		subj.d_angle.b=eyeball.angle.b;
		subj.d_angle.g=eyeball.angle.g;
		EXTERNALVIEW=1;
	}
	else if (EXTERNALVIEW)
	{
		float t=DEG2RAD(player.angle.b);
		EERIE_3D tt;

		for (long l=0;l<250;l+=10)
		{
			tt.x=player.pos.x+(float)EEsin(t)*(float)l;
			tt.y=player.pos.y-50.f;
			tt.z=player.pos.z-(float)EEcos(t)*(float)l;
			EERIEPOLY * ep =EECheckInPoly(&tt);

			if (ep)
			{
				subj.d_pos.x=tt.x;
				subj.d_pos.y=tt.y;
				subj.d_pos.z=tt.z;			
			}
			else break;
		}

		subj.d_angle.a=player.angle.a+30.f;
		subj.d_angle.b=player.angle.b;
		subj.d_angle.g=player.angle.g;
		EXTERNALVIEW=1;
	}
	else	
	{
		subj.angle.a=player.angle.a;
		subj.angle.b=player.angle.b;
		subj.angle.g=player.angle.g; 
		EXTERNALVIEW=0;
		long id;

		if (inter.iobj[0])
		{
				id = inter.iobj[0]->obj->fastaccess.view_attach;

			if (id!=-1)
			{
				subj.pos.x=inter.iobj[0]->obj->vertexlist3[id].v.x;
				subj.pos.y=inter.iobj[0]->obj->vertexlist3[id].v.y;
				subj.pos.z=inter.iobj[0]->obj->vertexlist3[id].v.z;

				EERIE_3D vect;
				vect.x=subj.pos.x-player.pos.x;
				vect.y=0;
				vect.z=subj.pos.z-player.pos.z;
				float len=Vector_Magnitude(&vect);

				if (len>46.f)
				{
					float div=46.f/len;
					vect.x*=div;
					vect.z*=div;
					subj.pos.x=player.pos.x+vect.x;
					subj.pos.z=player.pos.z+vect.z;
				}
			}
			else 
			{
				subj.pos.x=player.pos.x;
				subj.pos.y=player.pos.y;
				subj.pos.z=player.pos.z;
				subj.pos.y+=PLAYER_BASE_HEIGHT;
			}
	}
		}

	if (EXTERNALVIEW) 
	{
		subj.pos.x=(subj.pos.x+subj.d_pos.x)*DIV2;
		subj.pos.y=(subj.pos.y+subj.d_pos.y)*DIV2;
		subj.pos.z=(subj.pos.z+subj.d_pos.z)*DIV2;

		subj.angle.a=InterpolateAngle(subj.angle.a,subj.d_angle.a,0.1f);
		subj.angle.b=InterpolateAngle(subj.angle.b,subj.d_angle.b,0.1f);
		subj.angle.g=InterpolateAngle(subj.angle.g,subj.d_angle.g,0.1f);
	}
				
	if ((ARX_CONVERSATION) && (main_conversation.actors_nb))
	{
		// Decides who speaks !!
		if (main_conversation.current<0)		
		for (long j=0;j<main_conversation.actors_nb;j++)
		{
			if (main_conversation.actors[j]>=0)
			{
				for (long k=0;k<MAX_ASPEECH;k++)
				{
					if (aspeech[k].exist)
						if (aspeech[k].io==inter.iobj[main_conversation.actors[j]])
						{
							main_conversation.current=k;
							j=main_conversation.actors_nb+1;
							k=MAX_ASPEECH+1;
						}
				}
			}
		}

		long is=main_conversation.current;
		
		if (ARX_CONVERSATION_LASTIS!=is) ARX_CONVERSATION_MODE=-1;

		ARX_CONVERSATION_LASTIS=is;

		if (ARX_CONVERSATION_MODE==-1)
		{
			ARX_CONVERSATION_MODE=(long)(float)(rnd()*3.f+1.f);
			conversationcamera.size.a=rnd()*50.f;
			conversationcamera.size.b=0.f;
			conversationcamera.size.g=rnd()*50.f;
			conversationcamera.d_angle.a=0.f;
			conversationcamera.d_angle.b=0.f;
			conversationcamera.d_angle.g=0.f;

			if (rnd()>0.4f) conversationcamera.d_angle.a=(1.f-rnd()*2.f)*DIV30;

			if (rnd()>0.4f) conversationcamera.d_angle.b=(1.f-rnd()*1.2f)*DIV5;

			if (rnd()>0.4f) conversationcamera.d_angle.g=(1.f-rnd()*2.f)*DIV40;

			if (rnd()>0.5f) 
			{
				conversationcamera.size.a=MAKEANGLE(180.f+rnd()*20.f-10.f);
				conversationcamera.size.g=0.f;
				conversationcamera.d_angle.g=0.08f;
				conversationcamera.d_angle.b=0.f;
					conversationcamera.d_angle.a = 0.f;
				conversationcamera.size.b=0.f;
			}
		}
		else 
		{
			conversationcamera.size.a+=conversationcamera.d_angle.a*FrameDiff;
			conversationcamera.size.b+=conversationcamera.d_angle.b*FrameDiff;
			conversationcamera.size.g+=conversationcamera.d_angle.g*FrameDiff;
		}

		EERIE_3D sourcepos,targetpos;

		if (ApplySpeechPos(&conversationcamera,is))
		{
			targetpos.x=conversationcamera.d_pos.x;
			targetpos.y=conversationcamera.d_pos.y;
			targetpos.z=conversationcamera.d_pos.z;
			sourcepos.x=conversationcamera.pos.x;
			sourcepos.y=conversationcamera.pos.y;
			sourcepos.z=conversationcamera.pos.z;
		}
		else 
		{			
			targetpos.x=player.pos.x;
			targetpos.y=player.pos.y;
			targetpos.z=player.pos.z;
			float t=DEG2RAD(player.angle.b);
			sourcepos.x=targetpos.x+(float)EEsin(t)*100.f;
			sourcepos.y=targetpos.y;
			sourcepos.z=targetpos.z-(float)EEcos(t)*100.f;
			}
	
		EERIE_3D vect,vec2;
		vect.x=targetpos.x-sourcepos.x;
		vect.y=targetpos.y-sourcepos.y;
		vect.z=targetpos.z-sourcepos.z;
		float mag=1.f/Vector_Magnitude(&vect);
		vect.x*=mag;
		vect.y*=mag;
		vect.z*=mag;
		float dist=250.f-conversationcamera.size.g;

		if (dist<0.f) dist=(90.f-(dist*DIV20));
		else if (dist<90.f) dist=90.f;

		_YRotatePoint(&vect,&vec2,EEcos(DEG2RAD(conversationcamera.size.a)),EEsin(DEG2RAD(conversationcamera.size.a)));
		
		sourcepos.x=targetpos.x-vec2.x*dist;
		sourcepos.y=targetpos.y-vec2.y*dist;
		sourcepos.z=targetpos.z-vec2.z*dist;

		if (conversationcamera.size.b!=0.f)
			sourcepos.y+=120.f-conversationcamera.size.b*DIV10;

		conversationcamera.pos.x=sourcepos.x;
		conversationcamera.pos.y=sourcepos.y;
		conversationcamera.pos.z=sourcepos.z;
		SetTargetCamera(&conversationcamera,targetpos.x,targetpos.y,targetpos.z);
		subj.pos.x=conversationcamera.pos.x;
		subj.pos.y=conversationcamera.pos.y;
		subj.pos.z=conversationcamera.pos.z;
		subj.angle.a=MAKEANGLE(-conversationcamera.angle.a);
		subj.angle.b=MAKEANGLE(conversationcamera.angle.b-180.f);
		subj.angle.g=0.f;
		EXTERNALVIEW=1;
	}
	else 
	{
		ARX_CONVERSATION_MODE=-1;
		ARX_CONVERSATION_LASTIS=-1;
			
		if (LAST_CONVERSATION) 
		{
			AcquireLastAnim(inter.iobj[0]);
			ANIM_Set(&inter.iobj[0]->animlayer[1],inter.iobj[0]->anims[ANIM_WAIT]);
			inter.iobj[0]->animlayer[1].flags|=EA_LOOP;
		}
	}

		////////////////////////
	// Checks SCRIPT TIMERS.
	if (FirstFrame==0) 
		ARX_SCRIPT_Timer_Check();

	/////////////////////////////////////////////
	// Now checks for speech controlled cinematic
	{
		long valid=-1;

		for (long i=0;i<MAX_ASPEECH;i++)
		{
			if ((aspeech[i].exist) && (aspeech[i].cine.type>0))
			{
				valid=i;
				break;
			}
		}

		if (valid>=0)
		{
			ARX_CINEMATIC_SPEECH * acs=&aspeech[valid].cine;
			INTERACTIVE_OBJ * io=aspeech[valid].io;
			float rtime=(float)(ARX_TIME_Get()-aspeech[valid].time_creation)/(float)aspeech[valid].duration;

			if (rtime<0) rtime=0;

			if (rtime>1) rtime=1;

			float itime=1.f-rtime;			

			if ((rtime>=0.f) && (rtime<=1.f) && io)
			{
				float alpha,beta,distance,dist;

				switch (acs->type)
				{
					case ARX_CINE_SPEECH_KEEP:
						subj.pos.x=acs->pos1.x;
						subj.pos.y=acs->pos1.y;
						subj.pos.z=acs->pos1.z;
						subj.angle.a=acs->pos2.a;
						subj.angle.b=acs->pos2.b;
						subj.angle.g=acs->pos2.g;
						EXTERNALVIEW=1;	
					break;
					case ARX_CINE_SPEECH_ZOOM:
						//need to compute current values
						alpha=acs->startangle.a*itime+acs->endangle.a*rtime;
						beta=acs->startangle.b*itime+acs->endangle.b*rtime;
						distance=acs->startpos*itime+acs->endpos*rtime;
						EERIE_3D targetpos;
						targetpos.x=acs->pos1.x;
						targetpos.y=acs->pos1.y;
						targetpos.z=acs->pos1.z;
						conversationcamera.pos.x=-EEsin(DEG2RAD(MAKEANGLE(io->angle.b+beta)))*distance+targetpos.x;
						conversationcamera.pos.y= EEsin(DEG2RAD(MAKEANGLE(io->angle.a+alpha)))*distance+targetpos.y;
						conversationcamera.pos.z= EEcos(DEG2RAD(MAKEANGLE(io->angle.b+beta)))*distance+targetpos.z;						
						SetTargetCamera(&conversationcamera,targetpos.x,targetpos.y,targetpos.z);
						subj.pos.x=conversationcamera.pos.x;
						subj.pos.y=conversationcamera.pos.y;
						subj.pos.z=conversationcamera.pos.z;
						subj.angle.a=MAKEANGLE(-conversationcamera.angle.a);
						subj.angle.b=MAKEANGLE(conversationcamera.angle.b-180.f);
						subj.angle.g=0.f;
						EXTERNALVIEW=1;	
					break;
					case ARX_CINE_SPEECH_SIDE_LEFT:
					case ARX_CINE_SPEECH_SIDE:

						if (ValidIONum(acs->ionum))
						{

							EERIE_3D from,to,vect,vect2;
							from.x=acs->pos1.x;
							from.y=acs->pos1.y;
							from.z=acs->pos1.z;
							to.x=acs->pos2.x;
							to.y=acs->pos2.y;
							to.z=acs->pos2.z;
							
							vect.x=to.x-from.x;
							vect.y=to.y-from.y;
							vect.z=to.z-from.z;
							TRUEVector_Normalize(&vect);

							if (acs->type==ARX_CINE_SPEECH_SIDE_LEFT)
							{
								Vector_RotateY(&vect2,&vect,-90);
							}
							else
							{
								Vector_RotateY(&vect2,&vect,90);
							}

							distance=acs->f0*itime+acs->f1*rtime;
							vect2.x*=distance;
							vect2.y*=distance;
							vect2.z*=distance;
							dist=TRUEEEDistance3D(&from,&to);
							EERIE_3D tfrom,tto;
							tfrom.x=from.x+vect.x*acs->startpos*DIV100*dist;
							tfrom.y=from.y+vect.y*acs->startpos*DIV100*dist;
							tfrom.z=from.z+vect.z*acs->startpos*DIV100*dist;
							tto.x=from.x+vect.x*acs->endpos*DIV100*dist;
							tto.y=from.y+vect.y*acs->endpos*DIV100*dist;
							tto.z=from.z+vect.z*acs->endpos*DIV100*dist;
							targetpos.x=tfrom.x*itime+tto.x*rtime;
							targetpos.y=tfrom.y*itime+tto.y*rtime+acs->f2;
							targetpos.z=tfrom.z*itime+tto.z*rtime;
							conversationcamera.pos.x=targetpos.x+vect2.x;
							conversationcamera.pos.y=targetpos.y+vect2.y+acs->f2;
							conversationcamera.pos.z=targetpos.z+vect2.z;
							SetTargetCamera(&conversationcamera,targetpos.x,targetpos.y,targetpos.z);
							subj.pos.x=conversationcamera.pos.x;
							subj.pos.y=conversationcamera.pos.y;
							subj.pos.z=conversationcamera.pos.z;
							subj.angle.a=MAKEANGLE(-conversationcamera.angle.a);
							subj.angle.b=MAKEANGLE(conversationcamera.angle.b-180.f);
							subj.angle.g=0.f;
							EXTERNALVIEW=1;	
						}

					break;
					case ARX_CINE_SPEECH_CCCLISTENER_R:
					case ARX_CINE_SPEECH_CCCLISTENER_L:
					case ARX_CINE_SPEECH_CCCTALKER_R:
					case ARX_CINE_SPEECH_CCCTALKER_L:

						//need to compute current values
						if (ValidIONum(acs->ionum))
						{
							INTERACTIVE_OBJ * ioo=inter.iobj[acs->ionum];
							INTERACTIVE_OBJ * o1=io;
							INTERACTIVE_OBJ * o2=ioo;

							if ((acs->type==ARX_CINE_SPEECH_CCCLISTENER_L)
								|| (acs->type==ARX_CINE_SPEECH_CCCLISTENER_R))
							{
								o1=ioo;
								o2=io;								
								conversationcamera.pos.x=acs->pos2.x;
								conversationcamera.pos.y=acs->pos2.y;
								conversationcamera.pos.z=acs->pos2.z;
								targetpos.x=acs->pos1.x;
								targetpos.y=acs->pos1.y;
								targetpos.z=acs->pos1.z;
							}
							else
							{
								conversationcamera.pos.x=acs->pos1.x;
								conversationcamera.pos.y=acs->pos1.y;
								conversationcamera.pos.z=acs->pos1.z;
								targetpos.x=acs->pos2.x;
								targetpos.y=acs->pos2.y;
								targetpos.z=acs->pos2.z;
							}
							
							distance=(acs->startpos*itime+acs->endpos*rtime)*DIV100;						
							
							EERIE_3D vect,vect3;
							vect.x=conversationcamera.pos.x-targetpos.x;
							vect.y=conversationcamera.pos.y-targetpos.y;
							vect.z=conversationcamera.pos.z-targetpos.z;
							EERIE_3D vect2;
							Vector_RotateY(&vect2,&vect,90);
							TRUEVector_Normalize(&vect2);
							Vector_Copy(&vect3,&vect);
							TRUEVector_Normalize(&vect3);

							vect.x=vect.x*(distance)+vect3.x*80.f;
							vect.y=vect.y*(distance)+vect3.y*80.f;
							vect.z=vect.z*(distance)+vect3.z*80.f;
							vect2.x*=45.f;
							vect2.y*=45.f;
							vect2.z*=45.f;

							if ((acs->type==ARX_CINE_SPEECH_CCCLISTENER_R)
								|| (acs->type==ARX_CINE_SPEECH_CCCTALKER_R))
							{
								vect2.x=-vect2.x;
								vect2.y=-vect2.y;
								vect2.z=-vect2.z;
							}

							conversationcamera.pos.x=vect.x+targetpos.x+vect2.x;
							conversationcamera.pos.y=vect.y+targetpos.y+vect2.y;
							conversationcamera.pos.z=vect.z+targetpos.z+vect2.z; 
							SetTargetCamera(&conversationcamera,targetpos.x,targetpos.y,targetpos.z);
							subj.pos.x=conversationcamera.pos.x;
							subj.pos.y=conversationcamera.pos.y;
							subj.pos.z=conversationcamera.pos.z;
							subj.angle.a=MAKEANGLE(-conversationcamera.angle.a);
							subj.angle.b=MAKEANGLE(conversationcamera.angle.b-180.f);
							subj.angle.g=0.f;
							EXTERNALVIEW=1;	
						}

					break;
				}

				LASTCAMPOS.x=subj.pos.x;
				LASTCAMPOS.y=subj.pos.y;
				LASTCAMPOS.z=subj.pos.z;
				LASTCAMANGLE.a=subj.angle.a;
				LASTCAMANGLE.b=subj.angle.b;
				LASTCAMANGLE.g=subj.angle.g;
			}
		}
	}

	if (player.life<=0)
	{
			DeadTime	+=	ARX_CLEAN_WARN_CAST_LONG(FrameDiff);
		float mdist	=	EEfabs(player.physics.cyl.height)-60;
		DeadCameraDistance+=(float)FrameDiff*DIV80*((mdist-DeadCameraDistance)/mdist)*2.f;

		if (DeadCameraDistance>mdist) DeadCameraDistance=mdist;

		EERIE_3D targetpos;

		targetpos.x = player.pos.x;
			targetpos.y = player.pos.y;
			targetpos.z = player.pos.z;

			long id	 = inter.iobj[0]->obj->fastaccess.view_attach;
		long id2 = GetActionPointIdx( inter.iobj[0]->obj, "Chest2Leggings" );
		
		if (id!=-1)
		{
			targetpos.x = inter.iobj[0]->obj->vertexlist3[id].v.x;
			targetpos.y = inter.iobj[0]->obj->vertexlist3[id].v.y;
			targetpos.z = inter.iobj[0]->obj->vertexlist3[id].v.z;
		}

		conversationcamera.pos.x = targetpos.x;
		conversationcamera.pos.y = targetpos.y - DeadCameraDistance;
		conversationcamera.pos.z = targetpos.z;

		if (id2!=-1)
		{
				conversationcamera.pos.x=inter.iobj[0]->obj->vertexlist3[id2].v.x;
				conversationcamera.pos.y=inter.iobj[0]->obj->vertexlist3[id2].v.y-DeadCameraDistance;
				conversationcamera.pos.z=inter.iobj[0]->obj->vertexlist3[id2].v.z;
		}
		
		SetTargetCamera(&conversationcamera,targetpos.x,targetpos.y,targetpos.z);
		subj.pos.x=conversationcamera.pos.x;
		subj.pos.y=conversationcamera.pos.y;
		subj.pos.z=conversationcamera.pos.z;
		subj.angle.a=MAKEANGLE(-conversationcamera.angle.a);
		subj.angle.b=MAKEANGLE(conversationcamera.angle.b-180.f);
			subj.angle.g = 0;
		EXTERNALVIEW=1;

		if (!GAME_EDITOR)
			BLOCK_PLAYER_CONTROLS=1;
	}
	else 
	{
		DeadCameraDistance=0;
		
	}

		/////////////////////////////////////
	LAST_CONVERSATION=ARX_CONVERSATION;

	if ((this->kbd.inkey[INKEY_SPACE]) && (CAMERACONTROLLER!=NULL))
	{
		CAMERACONTROLLER=NULL;
		this->kbd.inkey[INKEY_SPACE]=0;
	}
	
	if (CAMERACONTROLLER!=NULL)
	{
		if (lastCAMERACONTROLLER!=CAMERACONTROLLER)
		{
			currentbeta=CAMERACONTROLLER->angle.b;
		}

			EERIE_3D targetpos;

		targetpos.x=CAMERACONTROLLER->pos.x;
		targetpos.y=CAMERACONTROLLER->pos.y+PLAYER_BASE_HEIGHT;
		targetpos.z=CAMERACONTROLLER->pos.z;

			float delta_angle = AngleDifference(currentbeta, CAMERACONTROLLER->angle.b);
			float delta_angle_t = delta_angle * FrameDiff * DIV1000;

			if (EEfabs(delta_angle_t) > EEfabs(delta_angle)) delta_angle_t = delta_angle;

			currentbeta += delta_angle_t;
		float t=DEG2RAD(MAKEANGLE(currentbeta));
		conversationcamera.pos.x=targetpos.x+(float)EEsin(t)*160.f;
		conversationcamera.pos.y=targetpos.y+40.f;
		conversationcamera.pos.z=targetpos.z-(float)EEcos(t)*160.f;
	
		SetTargetCamera(&conversationcamera,targetpos.x,targetpos.y,targetpos.z);		
		subj.pos.x=conversationcamera.pos.x;
		subj.pos.y=conversationcamera.pos.y;
		subj.pos.z=conversationcamera.pos.z;
		subj.angle.a=MAKEANGLE(-conversationcamera.angle.a);
		subj.angle.b=MAKEANGLE(conversationcamera.angle.b-180.f);
		subj.angle.g=0.f;
		EXTERNALVIEW=1;		
	}

	lastCAMERACONTROLLER=CAMERACONTROLLER;

	if ((USE_CINEMATICS_CAMERA) && (USE_CINEMATICS_PATH.path!=NULL))
	{
		EERIE_3D pos,pos2;
			USE_CINEMATICS_PATH._curtime = ARX_TIME_Get();

		USE_CINEMATICS_PATH._curtime+=50;
		long pouet2=ARX_PATHS_Interpolate(&USE_CINEMATICS_PATH,&pos);
		USE_CINEMATICS_PATH._curtime-=50;
		long pouet=ARX_PATHS_Interpolate(&USE_CINEMATICS_PATH,&pos2);

		if ((pouet!=-1) && (pouet2!=-1))
		{
		if (USE_CINEMATICS_CAMERA==2)
		{
			subj.pos.x=pos.x;
			subj.pos.y=pos.y;
			subj.pos.z=pos.z;		

			subj.d_angle.a=subj.angle.a;
			subj.d_angle.b=subj.angle.b;
			subj.d_angle.g=subj.angle.g;
			pos2.x=(pos2.x+pos.x)*DIV2;
			pos2.y=(pos2.y+pos.y)*DIV2;
			pos2.z=(pos2.z+pos.z)*DIV2;
			SetTargetCamera(&subj,pos2.x,pos2.y,pos2.z);

		}
		else DebugSphere(pos.x,pos.y,pos.z,2,50,0xFFFF0000);

		if (USE_CINEMATICS_PATH.path->flags & ARX_USEPATH_FLAG_FINISHED)
		{
			USE_CINEMATICS_CAMERA=0;
			USE_CINEMATICS_PATH.path=NULL;
		}		
		}
		else
		{
			USE_CINEMATICS_CAMERA=0;
			USE_CINEMATICS_PATH.path=NULL;		
		}
	}

	UpdateCameras();
		
		///////////////////////////////////////////
	ARX_PLAYER_FrameCheck(Original_framedelay);

	if (MasterCamera.exist)
	{
		if (MasterCamera.exist & 2)
		{
			MasterCamera.exist&=~2;
			MasterCamera.exist|=1;
			MasterCamera.io=MasterCamera.want_io;
			MasterCamera.aup=MasterCamera.want_aup;
			MasterCamera.cam=MasterCamera.want_cam;
		}

		if (MasterCamera.cam->focal<100.f) MasterCamera.cam->focal=350.f;

		SetActiveCamera(MasterCamera.cam);
		EXTERNALVIEW=1;
	}
	else
	{
		// Set active camera for this viewport
		SetActiveCamera(&subj);
	}

	ARX_GLOBALMODS_Apply();

	if (EDITMODE) GDevice->SetRenderState( D3DRENDERSTATE_FOGENABLE, false );

		ManageQuakeFX();
	
	// Prepare ActiveCamera
	PrepareCamera(ACTIVECAM);
	// Recenter Viewport depending on Resolution

	// setting long from long
	ACTIVECAM->centerx	= DANAECENTERX;
	ACTIVECAM->centery	= DANAECENTERY;
	// casting long to float
	ACTIVECAM->posleft	= ACTIVECAM->transform.xmod	= ARX_CLEAN_WARN_CAST_FLOAT( DANAECENTERX );
	ACTIVECAM->postop	= ACTIVECAM->transform.ymod	= ARX_CLEAN_WARN_CAST_FLOAT( DANAECENTERY );


	// Set Listener Position
	EERIE_3D front, up;
	float t;
	t=DEG2RAD(MAKEANGLE(ACTIVECAM->angle.b));			
	front.x=-EEsin(t);
	front.y=0.f;
	front.z=EEcos(t);
	TRUEVector_Normalize(&front);
	up.x=0.f;
	up.y=1.f;
	up.z=0.f;
	ARX_SOUND_SetListener(&ACTIVECAM->pos, &front, &up);

	// Reset Transparent Polys Idx
	INTERTRANSPOLYSPOS=TRANSPOLYSPOS=0;

	// Check For Hiding/unHiding Player Gore
	if ((EXTERNALVIEW) || (player.life<=0)) 
	{
		ARX_INTERACTIVE_Show_Hide_1st(inter.iobj[0],0);
	}

	if (!EXTERNALVIEW)
	{
		ARX_INTERACTIVE_Show_Hide_1st(inter.iobj[0],1);
	}

	LASTEXTERNALVIEW=EXTERNALVIEW;

	// NOW DRAW the player (Really...)
	if (	(inter.iobj[0])
		&&	(inter.iobj[0]->animlayer[0].cur_anim) 	)		
	{
		float restore=ACTIVECAM->use_focal;

		if ((!EXTERNALVIEW) && (!BOW_FOCAL))
		{
			ACTIVECAM->use_focal=PLAYER_ARMS_FOCAL*Xratio;
		}

		SetFilteringMode(m_pd3dDevice,Bilinear);		

		if ((!EXTERNALVIEW) && GLOBAL_FORCE_PLAYER_IN_FRONT)
			FORCE_FRONT_DRAW=1;

		if (inter.iobj[0]->invisibility>0.9f) inter.iobj[0]->invisibility=0.9f;

		EERIEDrawAnimQuat(	m_pd3dDevice,	inter.iobj[0]->obj,
				&inter.iobj[0]->animlayer[0],
				&inter.iobj[0]->angle,&inter.iobj[0]->pos, 0,inter.iobj[0],0,8);		
		ACTIVECAM->use_focal=restore;
		FORCE_FRONT_DRAW=0;
	}	

	// SUBJECTIVE VIEW UPDATE START  *********************************************************
	SetFilteringMode(m_pd3dDevice,Bilinear);		
	SETZWRITE(m_pd3dDevice, TRUE );
	danaeApp.EnableZBuffer();

	if (FirstFrame==0)
	{
		StartBench();
		PrepareIOTreatZone();
			ARX_PHYSICS_Apply();

		if (FRAME_COUNT<=0)
				PrecalcIOLighting(&ACTIVECAM->pos, ACTIVECAM->cdepth * 0.6f);

		ACTIVECAM->fadecolor.r=current.depthcolor.r;
		ACTIVECAM->fadecolor.g=current.depthcolor.g;
		ACTIVECAM->fadecolor.b=current.depthcolor.b;
		
		if (uw_mode)
		{
			float val=10.f;	
			m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MIPMAPLODBIAS, *((LPDWORD) (&val))  );
			ARX_SCENE_Render(m_pd3dDevice,1);
			val=-0.3f;
			m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MIPMAPLODBIAS, *((LPDWORD) (&val))  );
		}
		else
		{
			ARX_SCENE_Render(m_pd3dDevice,1);
		}			

		BENCH_RENDER=EndBench();
		
	}
	
	if (EDITION==EDITION_PATHWAYS)
	{
		ARX_PATHS_RedrawAll(m_pd3dDevice);
	}

	// Begin Particles ***************************************************************************
	if (!(Project.hide & HIDE_PARTICLES))
	{
		StartBench();

		if (pParticleManager)
		{
				pParticleManager->Update(ARX_CLEAN_WARN_CAST_LONG(FrameDiff));
			pParticleManager->Render(m_pd3dDevice);
		}

		m_pd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,   D3DBLEND_ONE );
		m_pd3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND,  D3DBLEND_ONE );			
		SETZWRITE(m_pd3dDevice, FALSE );
		SETALPHABLEND(m_pd3dDevice,TRUE);
		ARX_FOGS_Render(0);

		bool bNoVB	=	false;
		if( bSoftRender )
		{
			bNoVB = GET_FORCE_NO_VB();
			SET_FORCE_NO_VB( true );
		}

		ARX_PARTICLES_Render(m_pd3dDevice,&subj);		
		UpdateObjFx(m_pd3dDevice,&subj);
		if( bSoftRender ) SET_FORCE_NO_VB( bNoVB );
		
		SETALPHABLEND(m_pd3dDevice,FALSE);
		BENCH_PARTICLES=EndBench();
	
	}

	// End Particles ***************************************************************************

	if (!EDITMODE) // Playing Game
	{
		// Checks Magic Flares Drawing
		if (!PLAYER_PARALYSED)
		{
			if (EERIEMouseButton & 1)
			{
				if ((ARX_FLARES_Block==0) && (CurrSlot<MAX_SLOT)) 
					ARX_SPELLS_AddPoint(&DANAEMouse);
				else
				{
					CurrPoint=0;
					ARX_FLARES_Block=0;
					CurrSlot=1;
					LastSlot=0;
				}
			}
			else if (ARX_FLARES_Block==0)
				ARX_FLARES_Block=1;
		}

		ARX_SPELLS_Precast_Check();
		ARX_SPELLS_ManageMagic();
		ARX_SPELLS_UpdateSymbolDraw(m_pd3dDevice);

		ManageTorch();

		// Renders Magical Flares
		if (	!((player.Interface & INTER_MAP )
			&&  (!(player.Interface & INTER_COMBATMODE)))
			&&	flarenum
			)
		{
			ARX_MAGICAL_FLARES_Draw(m_pd3dDevice,FRAMETICKS);
				FRAMETICKS = ARXTimeUL();
		}
	}
	else  // EDITMODE == TRUE
	{
		if (!(Project.hide & HIDE_NODES))
				RenderAllNodes(m_pd3dDevice);

		_TCHAR texx[80];
		_stprintf(texx, _T("EDIT MODE - Selected %d"), NbIOSelected);
		ARX_TEXT_Draw(m_pd3dDevice,InBookFont,100,2,0,0,texx,EERIECOLOR_YELLOW);
	
		if (EDITION==EDITION_FOGS)
			ARX_FOGS_RenderAll( m_pd3dDevice);	
	}

	// To remove for Final Release but needed until then !
	if (ItemToBeAdded[0]!=0) 
		DanaeItemAdd();
	    
	SETALPHABLEND(danaeApp.m_pd3dDevice,TRUE);
	SETZWRITE(danaeApp.m_pd3dDevice, FALSE );

	// Checks some specific spell FX
	CheckMr();

	if (Project.improve) 
		DrawImproveVisionInterface(danaeApp.m_pd3dDevice);	
	else 
	{
		if ((subj.focal<BASE_FOCAL+FOKMOD))
		{
			subj.focal+=INC_FOCAL;

			if (subj.focal>BASE_FOCAL+FOKMOD) subj.focal=BASE_FOCAL+FOKMOD;
		}
		else if (subj.focal>BASE_FOCAL+FOKMOD) subj.focal=BASE_FOCAL+FOKMOD;
	}

	if (eyeball.exist!=0)
	{
		DrawMagicSightInterface(danaeApp.m_pd3dDevice);			
	}

		if (PLAYER_PARALYSED)
	{
		SETZWRITE(m_pd3dDevice, FALSE );
		SETALPHABLEND(m_pd3dDevice,TRUE);
		m_pd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
		m_pd3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);	
		
		EERIEDrawBitmap(m_pd3dDevice,0.f,0.f,(float)DANAESIZX,(float)DANAESIZY,0.0001f,
				NULL,EERIERGB(0.2f,0.2f,1.f));		
		SETALPHABLEND(m_pd3dDevice,FALSE);
		SETZWRITE(m_pd3dDevice, TRUE );
	}

	if (FADEDIR)
	{
		ManageFade(danaeApp.m_pd3dDevice);
	}

	SETALPHABLEND(danaeApp.m_pd3dDevice,FALSE);
	SETZWRITE(danaeApp.m_pd3dDevice, TRUE );
	
	// Reset Last Key
	danaeApp.kbd.lastkey=-1;	

	// Red screen fade for damages.
	ARX_DAMAGE_Show_Hit_Blood(GDevice);

	// Manage Notes/Books opened on screen
	GDevice->SetRenderState( D3DRENDERSTATE_FOGENABLE, false);
	ARX_INTERFACE_NoteManage();		
	
	finish:; //----------------------------------------------------------------
	// Update spells
	ARX_SPELLS_Update(m_pd3dDevice);
	SETCULL(GDevice,D3DCULL_NONE);
	GDevice->SetRenderState( D3DRENDERSTATE_FOGENABLE, true);

	// Manage Death visual & Launch menu...
	if (DeadTime>2000)
		ARX_PLAYER_Manage_Death();
	
	//-------------------------------------------------------------------------

	// INTERFACE
		// Remove the Alphablend State if needed : NO Z Clear
	SETALPHABLEND(m_pd3dDevice,FALSE);
	GDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, false);

	// Draw game interface if needed
	if (ARXmenu.currentmode == AMCM_OFF)
	if (!(Project.hide & HIDE_INTERFACE) && !CINEMASCOPE)
	{
		SETTEXTUREWRAPMODE(GDevice,D3DTADDRESS_CLAMP);
		DrawAllInterface();
		DrawAllInterfaceFinish();

		if (	(player.Interface & INTER_MAP )
			&&  (!(player.Interface & INTER_COMBATMODE))
			&&	flarenum
			)
		{
			GDevice->SetRenderState( D3DRENDERSTATE_ZENABLE, false);
			ARX_MAGICAL_FLARES_Draw(m_pd3dDevice,FRAMETICKS);
			EnableZBuffer();
					FRAMETICKS = ARXTimeUL();
		}
	}

	SETTEXTUREWRAPMODE(GDevice,D3DTADDRESS_WRAP);

	if(bRenderInterList)
	{
		SETALPHABLEND(GDevice,FALSE);
		PopAllTriangleList(true);
		SETALPHABLEND(GDevice,TRUE);
		PopAllTriangleListTransparency();
		SETALPHABLEND(GDevice,FALSE);
	}

	GDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, true);
		this->GoFor2DFX();
	GDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, false);
m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_ZBUFFER,0, 1.0f, 0L );	

	// Speech Management
	if (!EDITMODE)
	{
		StartBench();
		ARX_SPEECH_Check(danaeApp.m_pd3dDevice);
		ARX_SPEECH_Update(danaeApp.m_pd3dDevice);
		BENCH_SPEECH=EndBench();	
	}
	
	SETTEXTUREWRAPMODE(m_pd3dDevice,D3DTADDRESS_WRAP);

	if(pTextManage && pTextManage->vText.size())
	{
		danaeApp.DANAEEndRender();

		pTextManage->Update(FrameDiff);
		pTextManage->Render();
		danaeApp.DANAEStartRender();
	}

	if (SHOW_INGAME_MINIMAP && ((PLAY_LOADED_CINEMATIC == 0) && (!CINEMASCOPE) && (!BLOCK_PLAYER_CONTROLS) && (ARXmenu.currentmode == AMCM_OFF))
		&& (!(player.Interface & INTER_MAP )	))
	{
			long	SHOWLEVEL = ARX_LEVELS_GetRealNum(CURRENTLEVEL);

		if ((SHOWLEVEL>=0) && (SHOWLEVEL<32))
			ARX_MINIMAP_Show(GDevice,SHOWLEVEL,1,1);
	}

		//-------------------------------------------------------------------------

	// CURSOR Rendering
	SETALPHABLEND(GDevice, false);

	if (DRAGINTER)
	{
		ARX_INTERFACE_RenderCursor();

		if(bRenderInterList)
		{
			SETALPHABLEND(GDevice,FALSE);
			PopAllTriangleList(true);
			SETALPHABLEND(GDevice,TRUE);
			PopAllTriangleListTransparency();
			SETALPHABLEND(GDevice,FALSE);
		}

		ARX_INTERFACE_HALO_Flush(m_pd3dDevice);
	}
	else
	{
		ARX_INTERFACE_HALO_Flush(m_pd3dDevice);
		ARX_INTERFACE_RenderCursor();		
	}

	GDevice->SetRenderState( D3DRENDERSTATE_FOGENABLE, true);

	//----------------RENDEREND------------------------------------------------
	renderend:
		;

	if (sp_max_start)	
		Manage_sp_max();

	// Some Visual Debug/Info Text
	CalcFPS();

	if (!FINAL_COMMERCIAL_DEMO)
	{
	if ((NEED_TEST_TEXT) && (!FINAL_COMMERCIAL_DEMO))
	{
		danaeApp.DANAEEndRender();
		ShowTestText();
		danaeApp.DANAEStartRender();
	}

	if (!NO_TEXT_AT_ALL)
	{
		if (ViewMode & VIEWMODE_INFOTEXT)
		{
			ShowInfoText(0);
		}
		else if ((FORCE_SHOW_FPS) || CYRIL_VERSION)
		{
			ShowFPS();
		}
	}	
	
	if ((USE_PORTALS) && (NEED_TEST_TEXT) && (!FOR_EXTERNAL_PEOPLE))
	{
		char tex[250];

		switch(USE_PORTALS)
		{
		case 1:
			sprintf(tex,"2DPortals_ROOM: %d",LAST_ROOM);
			break;
		case 2:
			sprintf(tex,"3DPortals_ROOM: %d - Vis %d",LAST_ROOM,LAST_PORTALS_COUNT);
			break;
		case 3:
			sprintf(tex,"3DPortals_ROOM(Transform): %d - Vis %d",LAST_ROOM,LAST_PORTALS_COUNT);
			break;
		case 4:
			sprintf(tex,"3DPortals_ROOM(TransformSC): %d - Vis %d",LAST_ROOM,LAST_PORTALS_COUNT);
			break;
		}

		danaeApp.OutputText( 320, 240, tex );

		if (bRenderInterList)
		{
			danaeApp.OutputText( 320, 257, "Seb" );
		}

		if(bGMergeVertex)
		{
			danaeApp.OutputText( 0, 284, "Portal MergeVertex" );
		}
		else
		{
			danaeApp.OutputText( 0, 284, "Portal Non MergeVertex" );
		}
	}

	if((NEED_TEST_TEXT) && (!FOR_EXTERNAL_PEOPLE))
	{
		if(bOLD_CLIPP)
		{
			danaeApp.OutputText(0, 240, "New Clipp" );
		}
		else
		{
			danaeApp.OutputText(0,274,"New Clipp");
		}
	}
	}

	//----------------------------------------------------------------------------
	// Begin 2D Pass for Lense Flares

	if ((PLAY_LOADED_CINEMATIC == 0) && (!CINEMASCOPE) && (!BLOCK_PLAYER_CONTROLS) && (ARXmenu.currentmode == AMCM_OFF))
	{
		if (ARX_IMPULSE_NowPressed(CONTROLS_CUST_QUICKLOAD) && !WILL_QUICKLOAD)
		{
			WILL_QUICKLOAD=1;
		}

		if (ARX_IMPULSE_NowPressed(CONTROLS_CUST_QUICKSAVE) && !WILL_QUICKSAVE)
		{
			iTimeToDrawD7=2000;
			WILL_QUICKSAVE=1;
		}

		ARX_DrawAfterQuickLoad();
	}

	danaeApp.DANAEEndRender();

	//--------------NORENDEREND---------------------------------------------------
	norenderend:
		;

	if(pGetInfoDirectInput->IsVirtualKeyPressedNowPressed(DIK_F10))
	{
		GetSnapShot();
	}

	if ((LaunchDemo) && (FirstFrame == 0)) 
	{
		NOCHECKSUM=1;
		LaunchDemo=0;		
		LaunchDummyParticle();		
	}
	}
	StartBench();

	if (ARXmenu.currentmode == AMCM_OFF)
	{
		ARX_SCRIPT_AllowInterScriptExec();
		ARX_SCRIPT_EventStackExecute();
		// Updates Damages Spheres
		ARX_DAMAGES_UpdateAll();
		ARX_MISSILES_Update();
		
		if (FirstFrame==0)
			ARX_PATH_UpdateAllZoneInOutInside();
	}

	BENCH_SCRIPT=EndBench();

	LastFrameTime=FrameTime;
	LastMouseClick=EERIEMouseButton;

	if (DEBUGCODE) ForceSendConsole("RenderEnd_____________________________", 1, 0, (HWND)1);

		DANAE_DEBUGGER_Update();

	if (NEED_BENCH)
	{
		if(danaeApp.DANAEStartRender()) 
		{
			SETZWRITE(m_pd3dDevice, TRUE );
			SETALPHABLEND(m_pd3dDevice,FALSE);
			iVPOS=0;
			ShowValue(&oBENCH_STARTUP,&BENCH_STARTUP,"Startup");
			ShowValue(&oBENCH_PLAYER,&BENCH_PLAYER,"Player");
			ShowValue(&oBENCH_RENDER,&BENCH_RENDER,"Render");
			ShowValue(&oBENCH_PARTICLES,&BENCH_PARTICLES,"Particles");
			ShowValue(&oBENCH_SPEECH,&BENCH_SPEECH,"Speech");
			ShowValue(&oBENCH_SCRIPT,&BENCH_SCRIPT,"Script");
			ShowValue(&oBENCH_PATHFINDER,&BENCH_PATHFINDER,"Pathfinder");
			BENCH_PATHFINDER=0;
			ShowValue(&oBENCH_SOUND,&BENCH_SOUND,"Sound Thread");
			BENCH_SOUND=0;
			
			danaeApp.DANAEEndRender();
		}
	}

    return S_OK;
}

INTERACTIVE_OBJ * GetFirstInterAtPos(EERIE_S2D * pos,long flag=0, EERIE_3D* _pRef=NULL, INTERACTIVE_OBJ** _pTable = NULL, int* _pnNbInTable=NULL );

void DANAE::GoFor2DFX()
{
	D3DTLVERTEX lv,ltvv;	

	GDevice=danaeApp.m_pd3dDevice;

	long needed = 0;

	for (long i=0;i<TOTPDL;i++) 
	{
		EERIE_LIGHT * el=PDL[i];

		if (el->extras & EXTRAS_FLARE)
		{
			if ((EEDistance3D(&ACTIVECAM->pos, &el->pos) < 2200))
			{
				needed=1;
				break;
			}
		}
	}

	if (!needed) return;
						
					{
		INTERACTIVE_OBJ* pTableIO[256];
		int nNbInTableIO = 0;

		LAST_LOCK_SUCCESSFULL=1;
		float temp_increase=_framedelay*DIV1000*4.f;
		DURING_LOCK=1;
		{
			bool bComputeIO = false;

			for (int i=0;i<TOTPDL;i++) 
			{
				EERIE_LIGHT * el=PDL[i];
	
				long lPosx=(long)(float)(el->pos.x*ACTIVEBKG->Xmul);
				long lPosz=(long)(float)(el->pos.z*ACTIVEBKG->Zmul);

				if(	(lPosx<0)||
					(lPosx>=ACTIVEBKG->Xsize)||
					(lPosz<0)||
					(lPosz>=ACTIVEBKG->Zsize)||
					(!ACTIVEBKG->fastdata[lPosx][lPosz].treat) )
				{
					el->treat=0;
					continue;
				}

				if (el->extras & EXTRAS_FLARE)
				{
					lv.sx=el->pos.x;
					lv.sy=el->pos.y;
					lv.sz=el->pos.z;
					specialEE_RTP(&lv,&ltvv);
					el->temp-=temp_increase;

					if (!(player.Interface & INTER_COMBATMODE)
						&& (player.Interface & INTER_MAP))
						continue;

					LAST_ZVAL=ltvv.sz;

					if ((ltvv.rhw > 0.f) &&
						(ltvv.sx>0.f) &&
						(ltvv.sy>(CINEMA_DECAL*Yratio)) &&
						(ltvv.sx<DANAESIZX) &&
						(ltvv.sy<(DANAESIZY-(CINEMA_DECAL*Yratio)))
						)
					{		
						EERIE_3D vector;
						vector.x=lv.sx-ACTIVECAM->pos.x;
						vector.y=lv.sy-ACTIVECAM->pos.y;
						vector.z=lv.sz-ACTIVECAM->pos.z;
						float fNorm = 50.f / sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
						vector.x*=fNorm;
						vector.y*=fNorm;
						vector.z*=fNorm;
						D3DTLVERTEX ltvv2;
						lv.sx-=vector.x;
						lv.sy-=vector.y;
						lv.sz-=vector.z;
						specialEE_RTP(&lv,&ltvv2);

						float fZFire=ltvv2.sz*(float)danaeApp.zbuffer_max;
						float fZFar=ProjectionMatrix._33*(1.f/(ACTIVECAM->cdepth*fZFogEnd))+ProjectionMatrix._43;

						EERIE_3D	hit;
						EERIEPOLY	*tp=NULL;
						EERIE_S2D ees2dlv;
						EERIE_3D ee3dlv;
						ee3dlv.x = lv.sx;
						ee3dlv.y = lv.sy;
						ee3dlv.z = lv.sz;


						ARX_CHECK_SHORT(ltvv.sx) ;
						ARX_CHECK_SHORT(ltvv.sy) ;

						ees2dlv.x = ARX_CLEAN_WARN_CAST_SHORT(ltvv.sx) ;
						ees2dlv.y = ARX_CLEAN_WARN_CAST_SHORT(ltvv.sy) ;

				
						if( !bComputeIO )
						{
							GetFirstInterAtPos( &ees2dlv, 2, &ee3dlv, pTableIO, &nNbInTableIO );
							bComputeIO = true;
						}

						if(	
							(ltvv.sz>fZFar)||
							EERIELaunchRay3(&ACTIVECAM->pos,&ee3dlv,&hit,tp,1)||
							GetFirstInterAtPos(&ees2dlv, 3, &ee3dlv, pTableIO, &nNbInTableIO )
							)
						{
							el->temp-=temp_increase*2.f;
						}
						else
						{
							el->temp+=temp_increase*2.f;
						}

						LAST_ZVAL	=	fZFire;

						ARX_CHECK_LONG( danaeApp.zbuffer_max );
						LAST_FVAL	=	ARX_CLEAN_WARN_CAST_LONG( danaeApp.zbuffer_max );

					}

					if (el->temp<0.f) el->temp=0.f;
					else if (el->temp>.8f) el->temp=.8f;
				}
			}	
		}
		
		DURING_LOCK = 0;
		// End 2D Pass ***************************************************************************

		{
			GDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,   D3DBLEND_ONE);
			GDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,  D3DBLEND_ONE);
			SETALPHABLEND(GDevice,TRUE);		
			SETZWRITE(GDevice, FALSE );
			SETCULL(GDevice,D3DCULL_NONE);
			GDevice->SetRenderState( D3DRENDERSTATE_ZENABLE, FALSE);
			GDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR,  0);

			for (int i=0;i<TOTPDL;i++) 
			{
				EERIE_LIGHT * el=PDL[i];

				if ((!el->exist) || (!el->treat)) continue;

				if (el->extras & EXTRAS_FLARE)
				{
					if (el->temp>0.f)
					{
						lv.sx=el->pos.x;
						lv.sy=el->pos.y;
						lv.sz=el->pos.z;				
						lv.rhw=1.f;
 						specialEE_RT((D3DTLVERTEX *)&lv,(EERIE_3D *)&ltvv);
						float v=el->temp;

						if (FADEDIR)
						{
							v*=1.f-LAST_FADEVALUE;
						}

						float siz;

						if (el->extras & EXTRAS_FIXFLARESIZE)
							siz=el->ex_flaresize;
						else
							siz=-el->ex_flaresize;

						EERIEDrawSprite(GDevice,&lv, siz ,tflare,EERIERGB(v*el->rgb.r,v*el->rgb.g,v*el->rgb.b),ltvv.sz);						
							
					}
				}
			}

			GDevice->SetRenderState( D3DRENDERSTATE_ZENABLE, TRUE);
		}
	}	

	SETZWRITE(GDevice, TRUE );
}

void ShowTestText()
{
	char tex[256];	

	if (FINAL_COMMERCIAL_GAME)
		sprintf(tex,"Version : Final 1.00 - Build n%02.3f",DANAE_VERSION);
	else
		sprintf(tex,"Version : Demo  1.01 - Build n%02.3f",DANAE_VERSION);

	danaeApp.OutputText( 0, 16, tex );

	sprintf(tex,"Level : %s",LastLoadedScene);	
	danaeApp.OutputText( 0, 32, tex );

	sprintf(tex,"Position : %5.0f %5.0f %5.0f",player.pos.x,player.pos.y,player.pos.z);
	danaeApp.OutputText( 0, 48, tex );
	
	sprintf(tex,"Last Failed Sequence : %s",LAST_FAILED_SEQUENCE);
	danaeApp.OutputText( 0, 64, tex );
}
extern float CURRENT_PLAYER_COLOR;
extern int TSU_TEST_COLLISIONS;
extern long TSU_TEST;

long TSU_TEST_NB = 0;
long TSU_TEST_NB_LIGHT = 0;

void ShowInfoText(long COR)
{
	unsigned long uGAT = ARXTimeUL() / 1000;
	long GAT=(long)uGAT;
	char tex[256];
	float fpss2=1000.f/_framedelay;	
	LASTfpscount++;
	
	float fps2v=__max(fpss2,LASTfps2);
	float fps2vmin=__min(fpss2,LASTfps2);
	
	if (LASTfpscount>49) 
	{
		LASTfps2=0;
		LASTfpscount=0;
		fps2=fps2v;
		fps2min=fps2vmin;
	}
	else 
	{
		LASTfps2=fpss2;
	}

	sprintf(tex, "%d Prims %4.02f fps ( %3.02f - %3.02f ) [%3.0fms] INTER:%d/%d MIPMESH %d [%3.06f", EERIEDrawnPolys, FPS, fps2min, fps2, _framedelay, INTER_DRAW, INTER_COMPUTE, 0, vdist);
	danaeApp.OutputText( 70, 32, tex );

	float poss=-666.66f;
	EERIEPOLY * ep=CheckInPolyPrecis(player.pos.x,player.pos.y,player.pos.z);
	float tempo=0.f;

	if ((ep) && (GetTruePolyY(ep,&player.pos,&tempo)))
		poss=tempo;

	sprintf(tex,"Position  x:%7.0f y:%7.0f [%7.0f] z:%6.0f a%3.0f b%3.0f FOK %3.0f",player.pos.x,player.pos.y+player.size.y,poss,player.pos.z,player.angle.a,player.angle.b,ACTIVECAM->focal);
	danaeApp.OutputText( 70, 48, tex );
	sprintf(tex,"AnchorPos x:%6.0f y:%6.0f z:%6.0f TIME %ds Part %d - %d  Lkey %d SSM %d",player.pos.x-Mscenepos.x,player.pos.y+player.size.y-Mscenepos.y,player.pos.z-Mscenepos.z
		,GAT,ParticleCount,player.doingmagic,danaeApp.kbd.lastkey,SnapShotMode);
	danaeApp.OutputText( 70, 64, tex );

	if (player.onfirmground==0) danaeApp.OutputText( 200, 280, "OFFGRND" );

	sprintf(tex,"Jump %f cinema %f %d %d - Pathfind %d(%s)",player.jumplastposition,CINEMA_DECAL,DANAEMouse.x,DANAEMouse.y,EERIE_PATHFINDER_Get_Queued_Number(), PATHFINDER_WORKING ? "Working" : "Idled");
	danaeApp.OutputText( 70, 80, tex );
	INTERACTIVE_OBJ * io=ARX_SCRIPT_Get_IO_Max_Events();
	
	char temp[256];
	
	if (io==NULL)
		sprintf(tex,"Events %d (IOmax N/A) Timers %d",Event_Total_Count,ARX_SCRIPT_CountTimers());
	else 
	{
		strcpy(temp,GetName(io->filename));	
		sprintf(tex,"Events %d (IOmax %s_%04d %d) Timers %d",Event_Total_Count,temp,io->ident,io->stat_count,ARX_SCRIPT_CountTimers());
	}

	danaeApp.OutputText( 70, 94, tex );

	io=ARX_SCRIPT_Get_IO_Max_Events_Sent();

	if (io!=NULL)		
	{
		strcpy(temp,GetName(io->filename));	
		sprintf(tex,"Max SENDER %s_%04d %d)",temp,io->ident,io->stat_sent);
		danaeApp.OutputText( 70, 114, tex );
	}

	float slope=0.f;
	ep=CheckInPoly(player.pos.x,player.pos.y-10.f,player.pos.z);

	if (ep)
	{
		slope=ep->norm.y;	
	}

	sprintf(tex,"Velocity %3.0f %3.0f %3.0f Slope %3.3f",player.physics.velocity.x,player.physics.velocity.y,player.physics.velocity.z,slope);
	danaeApp.OutputText( 70, 128, tex );

	sprintf(tex, "TSU_TEST %d - nblights %d - nb %d", TSU_TEST, TSU_TEST_NB_LIGHT, TSU_TEST_NB);
	danaeApp.OutputText( 100, 208, tex );
	TSU_TEST_NB = 0;
	TSU_TEST_NB_LIGHT = 0;
	
	long pos=DXI_GetKeyIDPressed(DXI_KEYBOARD1);
	sprintf(tex,"%d",pos);
	danaeApp.OutputText( 70, 99, tex );
	int jx,jy,jz;

	if (ARX_INPUT_GetSCIDAxis(&jx,&jy,&jz))
	{
		sprintf(tex,"%d %d %d",jx,jy,jz);
		danaeApp.OutputText( 70, 299, tex );
	}

	if ((!EDITMODE) && (ValidIONum(LastSelectedIONum)))
	{
		io = inter.iobj[LastSelectedIONum];

	  if (io)
	  {
		  if (io==inter.iobj[0])
		  {
			  	sprintf(tex,"%4.0f %4.0f %4.0f - %4.0f %4.0f %4.0f -- %3.0f %d/%d targ %d beh %d",io->pos.x,
					io->pos.y,io->pos.z,io->move.x,
					io->move.y,io->move.z,io->_npcdata->moveproblem,io->_npcdata->pathfind.listpos,io->_npcdata->pathfind.listnb,
					io->_npcdata->pathfind.truetarget,io->_npcdata->behavior);
				danaeApp.OutputText( 170, 420, tex );
			sprintf(tex,"Life %4.0f/%4.0f Mana %4.0f/%4.0f Poisoned %3.1f Hunger %4.1f",player.life,player.maxlife,
					player.mana,player.maxmana,player.poison,player.hunger);		
				danaeApp.OutputText( 170, 320, tex );
				
		  }
		  else
		  {
			  if (io->ioflags & IO_NPC)
			  {
				  
				sprintf(tex,"%4.0f %4.0f %4.0f - %4.0f %4.0f %4.0f -- %3.0f %d/%d targ %d beh %d",io->pos.x,
					io->pos.y,io->pos.z,io->move.x,
					io->move.y,io->move.z,io->_npcdata->moveproblem,io->_npcdata->pathfind.listpos,io->_npcdata->pathfind.listnb,
					io->_npcdata->pathfind.truetarget,io->_npcdata->behavior);
				danaeApp.OutputText( 170, 420, tex );
				sprintf(tex,"Life %4.0f/%4.0f Mana %4.0f/%4.0f Poisoned %3.1f",io->_npcdata->life,io->_npcdata->maxlife,
					io->_npcdata->mana,io->_npcdata->maxmana,io->_npcdata->poisonned);		
				danaeApp.OutputText( 170, 320, tex );
				sprintf(tex,"AC %3.0f Absorb %3.0f",ARX_INTERACTIVE_GetArmorClass(io),io->_npcdata->absorb);
				danaeApp.OutputText( 170, 335, tex );

				if (io->_npcdata->pathfind.flags  & PATHFIND_ALWAYS)
					danaeApp.OutputText( 170, 360, "PF_ALWAYS" );
				else 
				{
					sprintf(tex,"PF_%d",io->_npcdata->pathfind.flags);
					danaeApp.OutputText( 170, 360, tex); 
				}
			  }

			  if (io->ioflags & IO_FIX)
			  {
				sprintf(tex,"Durability %4.0f/%4.0f Poisonous %3d count %d",io->durability,io->max_durability,io->poisonous,io->poisonous_count);		
				danaeApp.OutputText( 170, 320, tex );
			  }

			  if (io->ioflags & IO_ITEM)
			  {
				sprintf(tex,"Durability %4.0f/%4.0f Poisonous %3d count %d",io->durability,io->max_durability,io->poisonous,io->poisonous_count);		
				danaeApp.OutputText( 170, 320, tex );
			  }
		  }
	  }
	}

	long zap=IsAnyPolyThere(player.pos.x,player.pos.z);
	sprintf(tex,"POLY %d LASTLOCK %d",zap,LAST_LOCK_SUCCESSFULL);		
	danaeApp.OutputText( 270, 220, tex );

	sprintf(tex,"COLOR %3.0f Stealth %3.0f",CURRENT_PLAYER_COLOR,GetPlayerStealth());		
	danaeApp.OutputText( 270, 200, tex );
	
	ARX_SCRIPT_Init_Event_Stats();
}

//-----------------------------------------------------------------------------
	
extern long POLYIN;
extern long NOT_MOVED_AT_ALL;
extern long LAST_LLIGHT_COUNT;
extern long MCache_Number;
extern long MCache_GetSize();
extern float PLAYER_CLIMB_THRESHOLD, player_climb;
extern float TOTAL_CHRONO;
		
float LAST_FZPOS;
float LAST_FZSCREEN;
	
//-----------------------------------------------------------------------------
	
void ShowFPS()
{
	char tex[256];
	float fpss2=1000.f/_framedelay;	
	LASTfpscount++;
	
	float fps2v=__max(fpss2,LASTfps2);
	float fps2vmin=__min(fpss2,LASTfps2);
	
	if (LASTfpscount>49) 
	{
		LASTfps2=0;
		LASTfpscount=0;
		fps2=fps2v;
		fps2min=fps2vmin;
	}
	else 
	{
		LASTfps2=fpss2;
	}

	sprintf(tex,"%d Prims %4.02f fps ( %3.02f - %3.02f ) [%3.0fms] INTER:%d/%d INTREAT:%d"
	        , EERIEDrawnPolys, FPS, fps2min, fps2, _framedelay, INTER_DRAW, INTER_COMPUTE, INTREATZONECOUNT);
	danaeApp.OutputText( 70, DANAESIZY-100+32, tex );

	TOTAL_CHRONO=0;
	sprintf(tex,"%4.0f MCache %d[%d] NOT %d FP %3.0f %3.0f Llights %d/%d TOTIOPDL %d TOTPDL %d"
		,inter.iobj[0]->pos.y,MCache_Number,MCache_GetSize(),NOT_MOVED_AT_ALL,Original_framedelay,_framedelay,LAST_LLIGHT_COUNT,MAX_LLIGHTS,TOTIOPDL,TOTPDL);

	if (LAST_LLIGHT_COUNT>MAX_LLIGHTS)
		strcat(tex," EXCEEDING LIMIT !!!");

	danaeApp.OutputText(70,DANAESIZY-170-144,tex);
	sprintf(tex,"Pos %10.3f Screen %10.3f"
		,LAST_FZPOS,LAST_FZSCREEN);
	danaeApp.OutputText(320,200,tex);
}

void ARX_SetAntiAliasing()
{
	if(	(pMenuConfig)&&
		(pMenuConfig->bAntiAliasing) )
	{
		D3DDEVICEDESC7 devicedesc7;
		GDevice->GetCaps(&devicedesc7);

		if(devicedesc7.dpcTriCaps.dwRasterCaps&D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT)
		{
			if( FAILED( GDevice->SetRenderState(D3DRENDERSTATE_ANTIALIAS,D3DANTIALIAS_SORTINDEPENDENT) ) )
			{

			}
		}
		else
		{
			if(devicedesc7.dpcTriCaps.dwRasterCaps&D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT)
			{
				GDevice->SetRenderState(D3DRENDERSTATE_ANTIALIAS,D3DANTIALIAS_SORTDEPENDENT);
			}
		}
	}
	else
	{
		GDevice->SetRenderState(D3DRENDERSTATE_ANTIALIAS,D3DANTIALIAS_NONE);
	}
}



HRESULT DANAE::InitDeviceObjects()
{    
	GDevice=m_pd3dDevice;

	// Setup Base Material
	D3DMATERIAL7 mtrl;
	D3DUtil_InitMaterial( mtrl, 1.f, 0.f, 0.f );
    m_pd3dDevice->SetMaterial( &mtrl );
	// Enable texture perspective RenderState
    m_pd3dDevice->SetRenderState( D3DRENDERSTATE_TEXTUREPERSPECTIVE , TRUE );
	// Enable Z-buffering RenderState
	EnableZBuffer();
	// Setup Ambient Color RenderState
    m_pd3dDevice->SetRenderState( D3DRENDERSTATE_AMBIENT,  0x0a0a0a0a );
    // Restore All Textures RenderState
	ReloadAllTextures(m_pd3dDevice);
	ARX_PLAYER_Restore_Skin();
	// Setup Dither Mode
	m_pd3dDevice->SetRenderState( D3DRENDERSTATE_DITHERENABLE, FALSE );
	// Setup Specular RenderState
    m_pd3dDevice->SetRenderState( D3DRENDERSTATE_SPECULARENABLE, FALSE );
	// Setup LastPixel RenderState
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_LASTPIXEL, TRUE);
	// Setup Clipping RenderState
	m_pd3dDevice->SetRenderState( D3DRENDERSTATE_CLIPPING , TRUE);	
	// Disable Lighting RenderState
	m_pd3dDevice->SetRenderState( D3DRENDERSTATE_LIGHTING  , FALSE);
	// Setup Texture Border RenderState
	SETTEXTUREWRAPMODE(m_pd3dDevice, D3DTADDRESS_WRAP);
	// Setup Color Key RenderState
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILENABLE,false);
	m_pd3dDevice->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE);

	if (USE_D3DFOG)
	{
		m_pd3dDevice->SetRenderState( D3DRENDERSTATE_FOGCOLOR, D3DRGB(current.depthcolor.r,current.depthcolor.g,current.depthcolor.b));
		float zval = 1.f;
		m_pd3dDevice->SetRenderState( D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_LINEAR );
		m_pd3dDevice->SetRenderState( D3DRENDERSTATE_FOGTABLEDENSITY, *((LPDWORD) (&zval)));
		m_pd3dDevice->SetRenderState( D3DRENDERSTATE_FOGVERTEXMODE,  D3DFOG_NONE );
		zval = 0.48f;
		float zval2 = zval * 0.65f;
		m_pd3dDevice->SetRenderState( D3DRENDERSTATE_FOGTABLESTART, *((LPDWORD) (&zval2)));
		m_pd3dDevice->SetRenderState( D3DRENDERSTATE_FOGTABLEEND, *((LPDWORD) (&zval)));
		m_pd3dDevice->SetRenderState( D3DRENDERSTATE_FOGENABLE, true);
	}

	SetZBias(m_pd3dDevice,0);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);

	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_LOCALVIEWER,FALSE);
	m_pd3dDevice->SetTextureStageState(1,D3DTSS_ADDRESS,D3DTADDRESS_WRAP);
	m_pd3dDevice->SetTextureStageState(1,D3DTSS_MINFILTER,D3DTFN_LINEAR);
	m_pd3dDevice->SetTextureStageState(1,D3DTSS_MAGFILTER,D3DTFN_LINEAR);
	m_pd3dDevice->SetTextureStageState(2,D3DTSS_ADDRESS,D3DTADDRESS_WRAP);
	m_pd3dDevice->SetTextureStageState(2,D3DTSS_MINFILTER,D3DTFN_LINEAR);
	m_pd3dDevice->SetTextureStageState(2,D3DTSS_MAGFILTER,D3DTFN_LINEAR);

	ComputePortalVertexBuffer();
	pDynamicVertexBuffer				=	new CMY_DYNAMIC_VERTEXBUFFER(4000,FVF_D3DVERTEX3);
	pDynamicVertexBufferBump			=	new CMY_DYNAMIC_VERTEXBUFFER(4000,FVF_D3DVERTEX3);		// pDynamicVertexBuffer for BUMP mapping.
	pDynamicVertexBufferTransform		=	new CMY_DYNAMIC_VERTEXBUFFER(4000, FVF_D3DVERTEX );
	pDynamicVertexBuffer_TLVERTEX		=	new CMY_DYNAMIC_VERTEXBUFFER(4000, D3DFVF_TLVERTEX );	// VB using TLVERTEX format (creating).
	pDynamicVertexBuffer_D3DVERTEX3_T	=	new CMY_DYNAMIC_VERTEXBUFFER(4000, FVF_D3DVERTEX3_T );	// using D3DVERTEX3_T format (creating).

	if(pMenu)
	{
		pMenu->bReInitAll=true;
	}

	ARX_SetAntiAliasing();

	m_pD3D->EvictManagedTextures();

	return S_OK;
}
HRESULT DANAEFinalCleanup()
{
	EERIE_PATHFINDER_Release();
	ARX_INPUT_Release();
	ARX_SOUND_Release();
	KillInterTreeView();
    return S_OK;
}

//*************************************************************************************
// FinalCleanup()
// Called before the app exits
//*************************************************************************************
HRESULT DANAE::FinalCleanup()
{
	EERIE_PATHFINDER_Release();
	ARX_INPUT_Release();
	ARX_SOUND_Release();
	KillInterTreeView();
    return S_OK;
}

//*************************************************************************************
// DeleteDeviceObjects()
//  Called when the app is exitting, or the device is being changed,
//  this function deletes any device dependant objects.
//*************************************************************************************
HRESULT DANAE::DeleteDeviceObjects()
{
    D3DTextr_InvalidateAllTextures();

	if(pDynamicVertexBufferTransform)
	{
		delete pDynamicVertexBufferTransform;
		pDynamicVertexBufferTransform=NULL;
	}

	if(pDynamicVertexBuffer_TLVERTEX)
	{
		delete pDynamicVertexBuffer_TLVERTEX;
		pDynamicVertexBuffer_TLVERTEX=NULL;
	}

	if(pDynamicVertexBuffer)
	{
		delete pDynamicVertexBuffer;
		pDynamicVertexBuffer=NULL;
	}

	if( pDynamicVertexBufferBump )
	{
		delete pDynamicVertexBufferBump;
		pDynamicVertexBufferBump			=	NULL;
	}

	if( pDynamicVertexBuffer_D3DVERTEX3_T )
	{
		delete pDynamicVertexBuffer_D3DVERTEX3_T;
		pDynamicVertexBuffer_D3DVERTEX3_T	=	NULL;
	}

	EERIE_PORTAL_ReleaseOnlyVertexBuffer();

    return S_OK;
}

//*************************************************************************************
// MsgProc()
//   Overrides StdMsgProc
//*************************************************************************************
LRESULT DANAE::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam,
                                    LPARAM lParam )
{
	switch (uMsg) 
	{
		case WM_ACTIVATE:

		if(wParam==WA_INACTIVE)
		{
			DXI_SleepAllDevices();

			if (pGetInfoDirectInput)
			{
				pGetInfoDirectInput->bActive=false;
			}
		}
		else
		{
			if(pGetInfoDirectInput)
			{
				pGetInfoDirectInput->ResetAll();
				pGetInfoDirectInput->bActive=true;
			}

			DXI_SleepAllDevices();
			DXI_RestoreAllDevices();
		}

		break;
		case WM_SYSCOMMAND: // To avoid ScreenSaver Interference

			if ((wParam & 0xFFF0)== SC_SCREENSAVE ||
				(wParam & 0xFFF0)== SC_MONITORPOWER)
			{
				return 0;
			}

		break;
		case WM_USER+12: // GAIA Specific Message

			if (	!FINAL_COMMERCIAL_DEMO
				&& !FINAL_COMMERCIAL_GAME
				)
			{
				char texx[1024]; 
				strcpy(texx,HERMES_GaiaCOM_Receive());
				strcpy(ItemToBeAdded,texx);
			}

		break;
		case WM_CLOSE:

			if (	FINAL_COMMERCIAL_GAME
				||	FINAL_COMMERCIAL_DEMO
				||	OKBox("Do you REALLY want to quit ?","Danae WARNING"))
			{
				CheckIO_NOT_SAVED();
				ADDED_IO_NOT_SAVED=0;
			}
			else
			{
				return FALSE;
			}

			break;
		case WM_DROPFILES:

			if (	!FINAL_COMMERCIAL_DEMO
				&&	!FINAL_COMMERCIAL_GAME
				)
			{
				HANDLE	hDrop;
				long		number;
				char temp[512];
				char texx[512];
				hDrop = (HANDLE) wParam;
				number=DragQueryFile((HDROP)hDrop,0,temp,512); 

				if (number > 0)
				{
					strcpy(texx,temp);
					strcpy(temp,Project.workingdir);
					MakeUpcase(temp);
					MakeUpcase(texx);

					if (!specialstrcmp(texx, temp))
					{
						strcpy(ItemToBeAdded,texx);
					}
					else 
					{
						char warn[256];
						sprintf(warn,"Warning: You CANNOT add an object not coming from Project Path (%s)",Project.workingdir);
						ShowPopup(warn);
						strcpy(ItemToBeAdded,"");
					}
				}

				DragFinish((HDROP)hDrop);
				SetFocus(hWnd); 
				SetActiveWindow(hWnd); 			
			}

		break;
	
		case WM_COMMAND: 

			if (	!FINAL_COMMERCIAL_DEMO
			&& !FINAL_COMMERCIAL_GAME
			)
			{
            switch( LOWORD(wParam) )
		    {
				case IDM_DLF_CHECK:
					ARX_SAVELOAD_CheckDLFs();
				break;
				case IDM_ANYPOLY:
				{
					ARX_PLAYER_GotoAnyPoly();						
				}
				break;
				case IDM_MOULINEX:

					if (OKBox("This Can Take a Loooooooooooooong Time... Sure ?","MOULINEX Confirm Box"))
					{
						MOULINEX=1;
					}

				break;
				case DANAE_B016:
					DANAE_DEBUGGER_Launch(danaeApp.m_hWnd);
				break;
				case DANAE_B015:
					ARX_TIME_Pause();
					Pause(TRUE);
					DialogBox( (HINSTANCE)GetWindowLong( danaeApp.m_hWnd, GWL_HINSTANCE ),
							MAKEINTRESOURCE(IDD_SEARCH), danaeApp.m_hWnd, ScriptSearchProc);

					if (SCRIPT_SEARCH_TEXT[0])
						ARX_SCRIPT_LaunchScriptSearch(SCRIPT_SEARCH_TEXT);

					Pause(FALSE);
					ARX_TIME_UnPause();				
				break;
				case DANAE_B012: 
					LaunchSnapShotParamApp(this->m_hWnd);
				break;
				case DANAE_B014: 

					#ifndef NOEDITOR //////////////////

					if (EDITION==EDITION_PARTICLES) 
					{
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B014,FALSE); //Particles
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B011,FALSE); //Pathways
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B010,FALSE); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B004,FALSE); //Nodes
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B007,FALSE); //Lights
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B008,FALSE); //Fogs
						EDITION=EDITION_IO;
					}
					else 
					{
						EDITION=EDITION_PARTICLES;
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B011,FALSE); //Pathways						
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B010,FALSE); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B004,FALSE); //Nodes
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B007,FALSE); //Lights
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B008,FALSE); //Fogs
					}

					#endif //////////////////

				break;
				case DANAE_B011: 

					#ifndef NOEDITOR //////////////////

					if (EDITION==EDITION_PATHWAYS) 
					{
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B014,FALSE); //Particles
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B011,FALSE); //Pathways
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B010,FALSE); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B004,FALSE); //Nodes
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B007,FALSE); //Lights
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B008,FALSE); //Fogs
						EDITION=EDITION_IO;
					}
					else 
					{
						EDITION=EDITION_PATHWAYS;
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B014,FALSE); //Particles
						//SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B011,FALSE); //Pathways						
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B010,FALSE); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B004,FALSE); //Nodes
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B007,FALSE); //Lights
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B008,FALSE); //Fogs
					}

					#endif //////////////////

				break;
				case DANAE_B010: 

					#ifndef NOEDITOR //////////////////

					if (EDITION==EDITION_ZONES) 
					{
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B014,FALSE); //Particles
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B011,FALSE); //Pathways
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B010,FALSE); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B004,FALSE); //Nodes
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B007,FALSE); //Lights
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B008,FALSE); //Fogs
						EDITION=EDITION_IO;
					}
					else 
					{
						EDITION=EDITION_ZONES;
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B014,FALSE); //Particles
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B011,FALSE); //Pathways						
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B004,FALSE); //Nodes
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B007,FALSE); //Lights
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B008,FALSE); //Fogs
					}

					#endif //////////////////
				break;
				case DANAE_B007: 

					#ifndef NOEDITOR //////////////////

					if (EDITION==EDITION_LIGHTS) 
					{
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B014,FALSE); //Particles
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B011,FALSE); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B010,FALSE); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B004,FALSE); //Nodes
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B007,FALSE); //Lights
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B008,FALSE); //Fogs
						EDITION=EDITION_IO;
					}
					else 
					{
						EDITION=EDITION_LIGHTS;
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B014,FALSE); //Particles
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B011,FALSE); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B010,FALSE); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B004,FALSE); //Nodes
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B008,FALSE); //Fogs
					}

					#endif //////////////////

				break;
				case DANAE_B006:

					#ifndef NOEDITOR //////////////////

					if (EDITMODE==1) 
					{
							SetEditMode(0);
						ARX_TIME_Get();
						SendGameReadyMsg();
					}
					else 
					{
							SetEditMode(1);
						RestoreAllIOInitPos();
					}

					#endif //////////////////

					#ifdef NOEDITOR //////////////////

					if (EDITMODE==1) SetEditMode(0);

					#endif //////////////////

				break;
				case DANAE_B003:					

					if (PauseScript==1) PauseScript=0;
					else PauseScript=1;

				break;
				case DANAE_B004: 

					#ifndef NOEDITOR //////////////////

					if (EDITION==EDITION_NODES) 
					{
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B014,FALSE); //Particles
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B011,FALSE); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B010,FALSE); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B004,FALSE); //Nodes
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B007,FALSE); //Lights
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B008,FALSE); //Fogs
						EDITION=EDITION_IO;
					}
					else 
					{
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B014,FALSE); //Particles
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B011,FALSE); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B010,FALSE); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B007,FALSE); //Lights
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B008,FALSE); //Fogs
						EDITION=EDITION_NODES;
					}

					#endif //////////////////
				break;
				case DANAE_B008: 
					#ifndef NOEDITOR //////////////////
					
					if (EDITION == EDITION_FOGS) 
					{
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B014,FALSE); //Particles
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B011,FALSE); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B010,FALSE); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B004,FALSE); //Nodes
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B007,FALSE); //Lights
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B008,FALSE); //Fogs
						EDITION=EDITION_IO;
					}
					else 
					{
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B014,FALSE); //Particles
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B011,FALSE); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B010,FALSE); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B004,FALSE); //Nodes
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B007,FALSE); //Lights
						EDITION=EDITION_FOGS;
					}

					#endif //////////////////

				break;
				case DANAE_MENU_SAVEPATH:
					TextBox("Enter SavePath",LOCAL_SAVENAME,16);
					strcpy(LOCAL_SAVENAME,GTE_TEXT);
					Danae_Registry_Write("LOCAL_SAVENAME",LOCAL_SAVENAME);
					ARX_CHANGELEVEL_MakePath();
				break;
				case DANAE_MENU_MESH_REDUCTION:

					if (MESH_REDUCTION_WINDOW==NULL)
					{	
						if (danaeApp.m_pFramework->m_bIsFullscreen)
						{
							ARX_TIME_Pause();
							Pause(TRUE);
							DialogBox( (HINSTANCE)GetWindowLong( danaeApp.m_hWnd, GWL_HINSTANCE ),
								MAKEINTRESOURCE(IDD_MESHREDUCTION), danaeApp.m_hWnd, MeshReductionProc);
							Pause(FALSE);
							ARX_TIME_UnPause();				
						}
						else 
						MESH_REDUCTION_WINDOW=(CreateDialogParam( (HINSTANCE)GetWindowLong( danaeApp.m_hWnd, GWL_HINSTANCE ),
							MAKEINTRESOURCE(IDD_MESHREDUCTION), danaeApp.m_hWnd, MeshReductionProc,0 ));
					}

				break;
				case DANAE_B009:
					ARX_PARTICLES_ClearAll();
					ARX_MISSILES_ClearAll();
					ARX_SPELLS_ClearAll();					

					if (CDP_PATHWAYS_Options!=NULL) SendMessage(CDP_PATHWAYS_Options,WM_CLOSE,0,0);

					if (CDP_LIGHTOptions!=NULL) SendMessage(CDP_LIGHTOptions,WM_CLOSE,0,0);

					if (CDP_FogOptions!=NULL) SendMessage(CDP_FogOptions,WM_CLOSE,0,0);

					CDP_LIGHTOptions=NULL;
					CDP_FogOptions=NULL;				
					ARX_TIME_Pause();
					DanaeSwitchFullScreen();
					LaunchDummyParticle();
					ARX_TIME_UnPause();
				break;
				case DANAE_B013:
					ARX_TIME_Pause();
		           	Pause(TRUE);
                    DialogBox( (HINSTANCE)GetWindowLong( hWnd, GWL_HINSTANCE ),
                               MAKEINTRESOURCE(IDD_OPTIONS), hWnd, OptionsProc );
					EERIE_LIGHT_ChangeLighting();
                    Pause(FALSE);
					ARX_TIME_UnPause();
     
				break;
				case DANAE_MENU_UNFREEZEALLINTER:
					ARX_INTERACTIVE_UnfreezeAll();
				break;
				case DANAE_MENU_RESETSHADOWS:
					ARX_TIME_Pause();
                	Pause(TRUE);

					if (OKBox("Remove Casts Shadows Flag from all Lights ?","DANAE Confirm Box"))
					{
						for (long i=0;i<MAX_LIGHTS;i++)
						{
							if (GLight[i]!=NULL)
							{
								GLight[i]->extras |= EXTRAS_NOCASTED;
							}
						}
					}

					Pause(FALSE);
					ARX_TIME_UnPause();
				break;
				case DANAE_MENU_RECALC:

					if (PRECALC==NULL)
					{	
						if (danaeApp.m_pFramework->m_bIsFullscreen)
						{
							ARX_TIME_Pause();
							Pause(TRUE);
							DialogBox( (HINSTANCE)GetWindowLong( danaeApp.m_hWnd, GWL_HINSTANCE ),
								MAKEINTRESOURCE(IDD_PRECALC), danaeApp.m_hWnd, PrecalcProc);
							Pause(FALSE);
							ARX_TIME_UnPause();				
						}
						else 
						PRECALC=(CreateDialogParam( (HINSTANCE)GetWindowLong( danaeApp.m_hWnd, GWL_HINSTANCE ),
							MAKEINTRESOURCE(IDD_PRECALC), danaeApp.m_hWnd, PrecalcProc,0 ));
					}

				break;
				case DANAE_MENU_LOCALLIST:

					if (ValidIONum(LastSelectedIONum))
					{
						strcpy(ShowText,"");

						if (inter.iobj[LastSelectedIONum]->script.data!=NULL)
							MakeLocalText(&inter.iobj[LastSelectedIONum]->script,ShowText);
						else if (inter.iobj[LastSelectedIONum]->over_script.data!=NULL)
							MakeLocalText(&inter.iobj[LastSelectedIONum]->over_script,ShowText);

						strcpy(ShowTextWindowtext,"Local Variables");
						DialogBox(hInstance, (LPCTSTR)IDD_SHOWTEXT, NULL, (DLGPROC)ShowTextDlg);					
					}
					else  
						ShowPopup("No Interactive Object Selected");

				break;
				case DANAE_MENU_MEMORY:
					strcpy(ShowText,"");
					unsigned long msize;
					msize=MakeMemoryText(ShowText);
					sprintf(ShowTextWindowtext,"Allocated Memory %u bytes %u Kb",msize,msize>>10);
					CreateDialogParam( (HINSTANCE)GetWindowLong( this->m_hWnd, GWL_HINSTANCE ),
                            MAKEINTRESOURCE(IDD_SHOWTEXT), this->m_hWnd, (DLGPROC)ShowTextDlg,0 );
				break;
				case DANAE_MENU_GLOBALLIST:
					strcpy(ShowText,"");
					MakeGlobalText(ShowText);
					strcpy(ShowTextWindowtext,"Global Variables");
					DialogBox(hInstance, (LPCTSTR)IDD_SHOWTEXT, NULL, (DLGPROC)ShowTextDlg);					
				break;
				case DANAE_MENU_INTEROBJLIST:
					LaunchInteractiveObjectsApp(this->m_hWnd);
				break;
				case DANAE_MENU_LANGUAGE:
					ARX_TIME_Pause();
                	Pause(TRUE);			
					DialogBox( (HINSTANCE)GetWindowLong( this->m_hWnd, GWL_HINSTANCE ),
                            MAKEINTRESOURCE(IDD_LANGUAGEDIALOG), this->m_hWnd, LanguageOptionsProc);
					ARX_Localisation_Init();
					Pause(FALSE);
					ARX_TIME_UnPause();
				break;
				case DANAE_MENU_IMPORTSCN:
					ARX_TIME_Pause();
                	Pause(TRUE);
					ShowPopup("Unavailable Command");
					Pause(FALSE);
					ARX_TIME_UnPause();
				break;
				case DANAE_MENU_UPDATELOCALISATION:
					ARX_TIME_Pause();
                	Pause(TRUE);
					ARX_Localisation_Init();
					Pause(FALSE);
					ARX_TIME_UnPause();
				break;
				case DANAE_MENU_UPDATESOUNDS:
					ARX_TIME_Pause();
					Pause(TRUE);
					ShowPopup("Unavailable Command");
					Pause(FALSE);
					ARX_TIME_UnPause();
				break;
				case DANAE_MENU_UPDATESCENE:
					ARX_TIME_Pause();
                	Pause(TRUE);
					ShowPopup("Unavailable Command");
					Pause(FALSE);
					ARX_TIME_UnPause();
				break;
				case DANAE_MENU_UPDATEALLSCRIPTS:

					if (!EDITMODE)
						ShowPopup("Command Only Available in EDITOR mode!!!");
					else
					{
						ARX_TIME_Pause();
                		Pause(TRUE);

						if (OKBox("Reload All Scripts ?","Confirm"))
						ReloadAllScripts();

						Pause(FALSE);
						ARX_TIME_UnPause();
					}

				break;
				case DANAE_MENU_UPDATEALLOBJECTS:
					ARX_TIME_Pause();
                	Pause(TRUE);
					ShowPopup("Unavailable Command");
					Pause(FALSE);
					ARX_TIME_UnPause();
				break;
				case DANAE_MENU_UPDATEALLTEXTURES:
					ARX_TIME_Pause();
                	Pause(TRUE);
					ReloadAllTextures(GDevice);
					Pause(FALSE);
					ARX_TIME_UnPause();
				break;
				case DANAE_MENU_UPDATEALLANIMS:

					if (!EDITMODE)
						ShowPopup("Command Only Available in EDITOR mode!!!");
					else
					{					
						ARX_TIME_Pause();
                		Pause(TRUE);
						EERIE_ANIMMANAGER_ReloadAll();
						Pause(FALSE);
						ARX_TIME_UnPause();
					}

				break;
				case DANAE_MENU_ANIMATIONSLIST:
					long tr;
					long memsize;
					ARX_TIME_Pause();
                	Pause(TRUE);
					tr=EERIE_ANIMMANAGER_Count(ShowText,&memsize);
					sprintf(ShowTextWindowtext,"Animations %d %d Ko",tr,memsize>>10);
					DialogBox(hInstance, (LPCTSTR)IDD_SHOWTEXTBIG, NULL, (DLGPROC)ShowTextDlg);
					Pause(FALSE);
					ARX_TIME_UnPause();
				break;
				case DANAE_MENU_TEXLIST:
					long _tr;
					long _memsize;
					long _memmip;
					ARX_TIME_Pause();
                	Pause(TRUE);
					_tr=CountTextures(ShowText,&_memsize,&_memmip);
					sprintf(ShowTextWindowtext,"Textures %d %d Ko MIPsize %d Ko",_tr,_memsize>>10,_memmip>>10);
					DialogBox(hInstance, (LPCTSTR)IDD_SHOWTEXTBIG, NULL, (DLGPROC)ShowTextDlg);
					Pause(FALSE);
					ARX_TIME_UnPause();
				break;
				case DANAE_MENU_PROJECTPATH:
					char tex[512];
					HERMESFolderSelector(Project.workingdir,"Choose Working Folder");
					sprintf(tex,"DANAE Project - %s",Project.workingdir);
					Danae_Registry_Write("LastWorkingDir",Project.workingdir);
					SetWindowTitle(hWnd,tex);
					char tteexx[512];
					strcpy(tteexx,Project.workingdir);
					strcat(tteexx,"GRAPH\\LEVELS\\");
					_chdir(tteexx);
					break;
				case DANAE_MENU_NEWLEVEL:
					ARX_TIME_Pause();
                	Pause(TRUE);

					if (OKBox("Do You Really Want to Start\na New Level ?","DANAE Confirm Box"))
						DanaeClearLevel();

					Pause(FALSE);
					ARX_TIME_UnPause();
                break;
				case DANAE_MENU_PURGELEVEL:

					if (OKBox("This Can Be REALLY Dangerous !!! Sure ?","Confirm"))
					{
						BIG_PURGE();
					}

				break;
				case DANAE_MENU_FORCELOAD:

					if (OKBox("This Can Be Dangerous... Sure ?","Confirm"))
					{
						FAKE_DIR=1;
						WILLLOADLEVEL=1; 
					}

				break;
				case DANAE_MENU_LOADLEVEL:
					WILLLOADLEVEL=1; 
				break;
				case DANAE_MENU_SAVELEVEL:                	
					ARX_TIME_Pause();
                	Pause(TRUE);			
					ShowPopup("Unavailable Command");
					Pause(FALSE);
					ARX_TIME_UnPause();					
                break;
				case DANAE_MENU_SAVEAS:
					WILLSAVELEVEL=1;
                break;
				case DANAE_MENU_EXIT:

					if (OKBox("Do You Really\nWant to Quit DANAE ?","DANAE Confirm Box"))
						SendMessage( hWnd, WM_CLOSE, 0, 0 );	                

                break;
				case DANAE_MENU_ABOUT:
					ARX_TIME_Pause();
                	Pause(TRUE);
                    DialogBox( (HINSTANCE)GetWindowLong( hWnd, GWL_HINSTANCE ),
                               MAKEINTRESOURCE(IDD_DANAEABOUT), hWnd, AboutProc );
                    Pause(FALSE);
					ARX_TIME_UnPause();
                break;
				case DANAE_MENU_OPTIONS:
                	ARX_TIME_Pause();
					Pause(TRUE);
                    DialogBox( (HINSTANCE)GetWindowLong( hWnd, GWL_HINSTANCE ),
                               MAKEINTRESOURCE(IDD_OPTIONS), hWnd, OptionsProc );
                    Pause(FALSE);
					ARX_TIME_UnPause();
                break;
				case DANAE_MENU_OPTIONS2:
                	ARX_TIME_Pause();
					Pause(TRUE);
					DialogBox( (HINSTANCE)GetWindowLong( this->m_hWnd, GWL_HINSTANCE ),
                               MAKEINTRESOURCE(IDD_OPTIONS2), this->m_hWnd, OptionsProc_2 );
					Pause(FALSE);
					ARX_TIME_UnPause();
                break;
			}
        }

		break;
	}

    return CD3DApplication::MsgProc( hWnd, uMsg, wParam, lParam );
}
void ReleaseSystemObjects()
{
	if (hero)
	{
		ReleaseEERIE3DObj(hero);
		hero=NULL;
	}

	if (inter.iobj[0])
	{
		inter.iobj[0]->obj = NULL;
		ReleaseInter(inter.iobj[0]);
		inter.iobj[0] = NULL;

		if (inter.iobj)
		{
			free(inter.iobj);
			inter.iobj = NULL;
		}
	}

	if (eyeballobj)
	{
		ReleaseEERIE3DObj(eyeballobj);
		eyeballobj=NULL;
	}

	if (cabal)
	{
		ReleaseEERIE3DObj(cabal);
		cabal=NULL;
	}

	if (nodeobj)
	{
		ReleaseEERIE3DObj(nodeobj);
		nodeobj=NULL;
	}

	if (fogobj)
	{
		ReleaseEERIE3DObj(fogobj);
		fogobj=NULL;
	}

	if (cameraobj)
	{
		ReleaseEERIE3DObj(cameraobj);
		cameraobj=NULL;
	}

	if (markerobj)
	{
		ReleaseEERIE3DObj(markerobj);
		markerobj=NULL;
	}

	if (arrowobj)
	{
		ReleaseEERIE3DObj(arrowobj);
		arrowobj=NULL;
	}				

	for (long i=0;i<MAX_GOLD_COINS_VISUALS;i++)
	{
		if (GoldCoinsObj[i])
		{
			ReleaseEERIE3DObj(GoldCoinsObj[i]);
			GoldCoinsObj[i]=NULL;
		}		
	}
}

//-----------------------------------------------------------------------------

void ClearGameDEVICE()
{
	ShowWindow(danaeApp.m_hWnd,SW_MINIMIZE|SW_HIDE);

	ARX_MINIMAP_PurgeTC();

	if (DURING_LOCK)
		danaeApp.Unlock();

	KillInterfaceTextureContainers();

	Menu2_Close();

	DanaeClearLevel(2);

	D3DTextr_KillAllTextures();

	if(ControlCinematique)
	{
		delete ControlCinematique;
		ControlCinematique=NULL;
	}
}

void ClearTileLights();

//-----------------------------------------------------------------------------

void ClearGame()
{
	ARX_Menu_Resources_Release();

	//la configuration
	if(pMenuConfig)
	{
		pMenuConfig->SaveAll();
		delete pMenuConfig;
		pMenuConfig=NULL;
	}

	//dinput
	if(pGetInfoDirectInput)
	{
		delete pGetInfoDirectInput;
		pGetInfoDirectInput=NULL;
	}

	RoomDrawRelease();
	EXITING=1;
	TREATZONE_Release();
	ClearTileLights();

	//les textes et les textures
	ClearSysTextures();
	FreeSaveGameList();

	if (pParticleManager)
	{
		delete pParticleManager;
		pParticleManager = NULL;
	}

	//sound
	ARX_SOUND_Release();
	MCache_ClearAll();

	//pathfinding
	ARX_PATH_ReleaseAllPath();
	ReleaseSystemObjects();
	
	//eerie_background
	ClearBackground(ACTIVEBKG);

	//animations
	EERIE_ANIMMANAGER_ClearAll();

	//Scripts
	if (svar)
	{
		for (long i=0; i<NB_GLOBALS; i++)
		{
			if (svar[i].text)
			{
				free(svar[i].text);
				svar[i].text=NULL;
			}
		}

		free(svar);
		svar=NULL;
	}

	ARX_SCRIPT_Timer_ClearAll();

	if (scr_timer)
	{
		free(scr_timer);
		scr_timer=NULL;
	}

	//Speech
	ARX_SPEECH_ClearAll();
	ARX_Text_Close();

	//les objects loaders dans beforerun
	ReleaseDanaeBeforeRun();
	PAK_Close();

	if (danaeApp.ToolBar)
	{
		free(danaeApp.ToolBar);
		danaeApp.ToolBar=NULL;
	}

	ReleaseNode();

	//HAlo
	ReleaseHalo();
	HERMES_Memory_Security_Off();
	FreeSnapShot();
	ARX_INPUT_Release();
}
