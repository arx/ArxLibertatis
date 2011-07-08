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

#include "core/Core.h"

#include <cassert>
#include <cstdio>

#include <fstream>
#include <sstream>

#include <windows.h>
#include <shellapi.h>

#include "ai/Paths.h"
#include "ai/PathFinderManager.h"

#include "animation/Animation.h"
#include "animation/CinematicKeyframer.h"

#include "core/Config.h"
#include "core/Dialog.h"
#include "core/Resource.h"
#include "core/Localisation.h"
#include "core/GameTime.h"

#include "game/Missile.h"
#include "game/Damage.h"
#include "game/Equipment.h"
#include "game/Map.h"
#include "game/Player.h"
#include "game/Levels.h"
#include "game/Inventory.h"
#include "game/NPC.h"

#include "gui/MenuPublic.h"
#include "gui/Menu.h"
#include "gui/MenuWidgets.h"
#include "gui/Speech.h"
#include "gui/MiniMap.h"
#include "gui/TextManager.h"

#include "graphics/VertexBuffer.h"
#include "graphics/GraphicsEnum.h"
#include "graphics/GraphicsModes.h"
#include "graphics/Frame.h"
#include "graphics/Draw.h"
#include "graphics/data/FTL.h"
#include "graphics/data/CinematicTexture.h"
#include "graphics/effects/Fog.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/ParticleManager.h"
#include "graphics/direct3d/Direct3DRenderer.h"
#include "graphics/texture/TextureStage.h"

#include "input/Input.h"

#include "io/FilePath.h"
#include "io/Registry.h"
#include "io/PakManager.h"
#include "io/Filesystem.h"
#include "io/Logger.h"
#include "io/CinematicLoad.h"
#include "io/Screenshot.h"
 
#include "physics/Collisions.h"
#include "physics/Attractors.h"

#include "platform/String.h"
#include "platform/Random.h"
#include "platform/Thread.h"

#include "scene/LinkedObject.h"
#include "scene/CinematicSound.h"
#include "scene/ChangeLevel.h"
#include "scene/Scene.h"
#include "scene/GameSound.h"
#include "scene/LoadLevel.h"
#include "scene/Interactive.h"
#include "scene/Light.h"
#include "scene/Object.h"

#include "scripting/ScriptEvent.h"
#include "scripting/ScriptDebugger.h"

using std::min;
using std::max;

void DemoFileCheck();

//-----------------------------------------------------------------------------

#define MAX_EXPLO 24
#define FFH_S_OK 1
#define FFH_GOTO_FINISH 2
extern INTERACTIVE_OBJ * CURPATHFINDIO;


//-----------------------------------------------------------------------------

HRESULT DANAEFinalCleanup();
void ClearGame();
static void ShowInfoText();

//-----------------------------------------------------------------------------

extern long LAST_PORTALS_COUNT;
extern TextManager	*pTextManage;
extern float FORCE_TIME_RESTORE;
extern CMenuState		*pMenu;
extern short uw_mode;
extern long SPECIAL_DRAGINTER_RENDER;
extern HWND		PRECALC;
extern INTERACTIVE_OBJ * CURRENT_TORCH;
extern EERIE_3DOBJ * fogobj;
extern bool		bSkipVideoIntro;
extern std::string SCRIPT_SEARCH_TEXT;
extern std::string ShowText;
extern std::string ShowText2;
extern float Full_Jump_Height;
extern float	MAX_ALLOWED_PER_SECOND;
extern float	InventoryX;
extern float	PROGRESS_BAR_COUNT;
extern float	PROGRESS_BAR_TOTAL;
extern float	vdist;
extern float	FLOATTEST;
extern float	_MAX_CLIP_DIST;
extern long		LastSelectedIONum;
extern long		FistParticles;
extern long		INTER_DRAW;
extern long		INTER_COMPUTE;
extern long		USEINTERNORM;
extern long		FAKE_DIR;
extern long		DONT_WANT_PLAYER_INZONE;
extern long		DeadTime;
extern long		INTREATZONECOUNT;
extern long		TOTPDL;
extern float		ARXTotalPausedTime;
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
extern bool bOLD_CLIPP;
extern float OLD_PROGRESS_BAR_COUNT;
extern E_ARX_STATE_MOUSE	eMouseState;

void DanaeRestoreFullScreen();

extern long FORCE_FRONT_DRAW;
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

extern Color ulBKGColor;
long LAST_LOCK_SUCCESSFULL=0;
extern EERIEMATRIX ProjectionMatrix;

extern CircularVertexBuffer<TexturedVertex> * pDynamicVertexBuffer_TLVERTEX; // VB using TLVERTEX format.
extern CircularVertexBuffer<SMY_VERTEX3> * pDynamicVertexBuffer;

extern std::string pStringMod;

static const float INC_FOCAL = 75.0f;

//-----------------------------------------------------------------------------
// Our Main Danae Application.& Instance and Project
DANAE danaeApp;
HINSTANCE hInstance;
PROJECT Project;

//-----------------------------------------------------------------------------
Vec3f LASTCAMPOS;
Anglef LASTCAMANGLE;
Vec3f PUSH_PLAYER_FORCE;
Cinematic			*ControlCinematique=NULL;	// 2D Cinematic Controller
ParticleManager	*pParticleManager = NULL;
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
TextureContainer *	Z_map=NULL;
TextureContainer *	Boom=NULL;
//TextureContainer *	zbtex=NULL;
TextureContainer *	mecanism_tc=NULL;
TextureContainer *	arrow_left_tc=NULL;

#ifdef BUILD_EDIT_LOADSAVE
extern long NEED_ANCHORS;
EERIE_MULTI3DSCENE * mse = NULL;
#endif

long NEED_TEST_TEXT=0;

SPELL_ICON spellicons[SPELL_COUNT];
bool bGToggleCombatModeWithKey;

Vec2s DANAEMouse;
Vec3f moveto;
Vec3f Mscenepos;
Vec3f lastteleport;
EERIE_3DOBJ * GoldCoinsObj[MAX_GOLD_COINS_VISUALS];// 3D Objects For Gold Coins
EERIE_3DOBJ	* arrowobj=NULL;			// 3D Object for arrows
EERIE_3DOBJ * cameraobj=NULL;			// Camera 3D Object		// NEEDTO: Remove for Final
EERIE_3DOBJ * markerobj=NULL;			// Marker 3D Object		// NEEDTO: Remove for Final
EERIE_3DOBJ * nodeobj=NULL;				// Node 3D Object		// NEEDTO: Remove for Final
EERIE_3DOBJ * eyeballobj=NULL;			// EyeBall 3D Object	// NEEDTO: Load dynamically
EERIE_3DOBJ * cabal=NULL;				// Cabalistic 3D Object // NEEDTO: Load dynamically
EERIE_BACKGROUND DefaultBkg;
EERIE_CAMERA TCAM[32];
EERIE_CAMERA subj,mapcam,bookcam,raycam,conversationcamera;
EERIE_CAMERA DynLightCam;

INTERACTIVE_OBJ * CAMERACONTROLLER=NULL;
std::string WILLADDSPEECH;

Vec2s STARTDRAG;
INTERACTIVE_OBJ * COMBINE=NULL;

QUAKE_FX_STRUCT QuakeFx;
std::string LAST_FAILED_SEQUENCE = "None";
// START - Information for Player Teleport between/in Levels-------------------------------------
char TELEPORT_TO_LEVEL[64];
char TELEPORT_TO_POSITION[64];
long TELEPORT_TO_ANGLE;
long TELEPORT_TO_CONFIRM=1;
// END -   Information for Player Teleport between/in Levels---------------------------------------
char LastLoadedDLF[512];
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

long EXTERNALVIEW=0;
long LASTEXTERNALVIEW=1;
long EXTERNALVIEWING=0;
long lSLID_VALUE=0;
long _NB_=0;
long LOAD_N_DONT_ERASE=0;
long NO_TIME_INIT=0;
long DANAESIZX=640;
long DANAESIZY=480;
long DANAECENTERX;
long DANAECENTERY;
long CurrFightPos=0;
long NO_PLAYER_POSITION_RESET=0;
long CURRENT_BASE_FOCAL=310;
long CINE_PRELOAD=0;
long PLAY_LOADED_CINEMATIC=0;
long PauseScript=0;
long ADDED_IO_NOT_SAVED=0;
long WILL_RELOAD_ALL_TEXTURES=0;	// Set To 1 if Textures are to be reloaded from disk and restored.
float BOW_FOCAL=0;
long PlayerWeaponBlocked=-1;
long SHOW_TORCH=0;
float FrameDiff=0;
long CYRIL_VERSION=0;
float GLOBAL_LIGHT_FACTOR=0.85f;

//-----------------------------------------------------------------------------
// Don't touch FINAL_COMMERCIAL_DEMO anymore
// Comment #define REAL_DEMO for non-demo Version
// UNcomment #define REAL_DEMO for demo Version
//TODO(lubosz): uncommenting this define causes stack overflow
//#define REAL_DEMO
#ifdef REAL_DEMO
long FINAL_COMMERCIAL_DEMO =1;
#else
long FINAL_COMMERCIAL_DEMO =0;
#endif

float IN_FRONT_DIVIDER_ITEMS	=0.7505f;
long GLOBAL_FORCE_PLAYER_IN_FRONT	=1;
long USE_NEW_SKILLS=1;

long USE_LIGHT_OPTIM	=1;
// set to 0 for dev mode
long FINAL_COMMERCIAL_GAME = 1;   // <--------------	fullgame
long ALLOW_CHEATS		 =1;
long FOR_EXTERNAL_PEOPLE =0;
long USE_OLD_MOUSE_SYSTEM=1;
long NO_TEXT_AT_ALL		= 0;
long LAST_CONVERSATION	= 0;
long FAST_SPLASHES		= 0;
long FORCE_SHOW_FPS		= 0;
long FINAL_RELEASE		= 0;
long AUTO_FULL_SCREEN	= 0;
long SHOW_INGAME_MINIMAP= 1;
long DEBUG_FRUSTRUM		= 0;
//-------------------------------------------------------------------------------
long STRIKE_TIME		= 0;
long STOP_KEYBOARD_INPUT= 0;
long REQUEST_SPEECH_SKIP= 0;
long CURRENTLEVEL		= -1;
long NOBUILDMAP			= 0;
long DONT_ERASE_PLAYER	= 0;
float LastFrameTicks		= 0;
long SPLASH_THINGS_STAGE= 0;
long STARTED_A_GAME		= 0;
long INTRO_NOT_LOADED	= 1;
long ARX_CONVERSATION_MODE=-1;
long ARX_CONVERSATION_LASTIS=-1;
long BOOKBUTTON			= 0;
long LASTBOOKBUTTON		= 0;

long FASTmse			= 0;

//-----------------------------------------------------------------------------
// EDITOR FLAGS/Vars
//-----------------------------------------------------------------------------
// Flag used to Launch Moulinex
long LOADEDD = 0; // Is a Level Loaded ?
#ifdef BUILD_EDITOR
long EDITMODE = 0; // EditMode (1) or GameMode (0) ?
long EDITION=EDITION_IO; // Sub-EditMode
long MOULINEX = 0;
long LASTMOULINEX = -1;
long KILL_AT_MOULINEX_END = 0;
long USE_COLLISIONS = 1;
long WILLLOADLEVEL = 0; // Is a LoadLevel command waiting ?
long WILLSAVELEVEL = 0; // Is a SaveLevel command waiting ?
long NODIRCREATION = 0; // No IO Directory Creation ?
HWND MESH_REDUCTION_WINDOW = NULL;
const char * GTE_TITLE;
char * GTE_TEXT;
long GTE_SIZE;
long CHANGE_LEVEL_PROC_RESULT = -1;
long DEBUGNPCMOVE = 0; // Debug NPC Movements
static long TSU_LIGHTING = 0; // must be 0 at start !
static long PROCESS_ALL_THEO = 1;
static long PROCESS_LEVELS = 1;
static long PROCESS_NO_POPUP = 0;
static long PROCESS_ONLY_ONE_LEVEL = -1;
long GAME_EDITOR = 1;
static long NEED_EDITOR = 1;
long TRUEFIGHT = 0;
#endif

long ARX_CONVERSATION=0;
long CHANGE_LEVEL_ICON=-1;

long ARX_MOUSE_OVER=0;
//-----------------------------------------------------------------------------
// DEBUG FLAGS/Vars
//-----------------------------------------------------------------------------
long LaunchDemo=0;
long FirstFrame=1;
unsigned long WILLADDSPEECHTIME=0;
unsigned long AimTime;
unsigned char ARX_FLARES_Block=1;
float LastFrameTime=0;
float FrameTime=0;
unsigned long PlayerWeaponBlockTime=0;
unsigned long FRAMETICKS=0;
unsigned long SPLASH_START=0;
//-----------------------------------------------------------------------------
extern float sp_max_start;
Color3f FADECOLOR;

long DURING_LOCK=0;
long START_NEW_QUEST=0;
long LAST_WEAPON_TYPE=-1;
long	FADEDURATION=0;
long	FADEDIR=0;
unsigned long FADESTART=0;

float Original_framedelay=0.f;

float PULSATE;
long EXITING=0;

long USE_PORTALS = 3;

//-----------------------------------------------------------------------------
// Toolbar Buttons Def
//-----------------------------------------------------------------------------

#ifdef BUILD_EDITOR

// TODO maybe this is a wine-specific bug?
#define TBBUTTON_INIT {0,0} // was: 0L

TBBUTTON tbButtons [] = {
{0, DANAE_B001, TBSTATE_ENABLED, TBSTYLE_BUTTON, TBBUTTON_INIT, 0, 0},
{1, DANAE_B002, TBSTATE_ENABLED, TBSTYLE_BUTTON, TBBUTTON_INIT, 1, 0},
{0, 0, TBSTATE_ENABLED | TBSTATE_WRAP   , TBSTYLE_SEP, TBBUTTON_INIT, 0, 0},
{8, DANAE_B009, TBSTATE_ENABLED, TBSTYLE_BUTTON, TBBUTTON_INIT, 8, 0},
{12, DANAE_B013, TBSTATE_ENABLED, TBSTYLE_BUTTON, TBBUTTON_INIT, 12, 0},

{0, 0, TBSTATE_ENABLED , TBSTYLE_SEP, TBBUTTON_INIT, 0, 0},

{13, DANAE_B003, TBSTATE_ENABLED, TBSTYLE_CHECK, TBBUTTON_INIT, 13, 0},
{16, DANAE_B005, TBSTATE_ENABLED, TBSTYLE_BUTTON, TBBUTTON_INIT, 16, 0},
{0, 0, TBSTATE_ENABLED , TBSTYLE_SEP, TBBUTTON_INIT, 0, 0},
{5, DANAE_B006, TBSTATE_ENABLED, TBSTYLE_BUTTON, TBBUTTON_INIT, 5, 0},
{14, DANAE_B004, TBSTATE_ENABLED, TBSTYLE_CHECK, TBBUTTON_INIT, 14, 0},
{2, DANAE_B007, TBSTATE_ENABLED, TBSTYLE_CHECK, TBBUTTON_INIT, 2, 0},
{3, DANAE_B008, TBSTATE_ENABLED, TBSTYLE_CHECK, TBBUTTON_INIT, 3, 0},
{17, DANAE_B010, TBSTATE_ENABLED, TBSTYLE_CHECK, TBBUTTON_INIT, 17, 0},
{18, DANAE_B011, TBSTATE_ENABLED, TBSTYLE_CHECK, TBBUTTON_INIT, 18, 0},
{11, DANAE_B014, TBSTATE_ENABLED, TBSTYLE_CHECK, TBBUTTON_INIT, 11, 0}, // Particles Button
{0, 0, TBSTATE_ENABLED , TBSTYLE_SEP, TBBUTTON_INIT, 0, 0},
{19, DANAE_B012, TBSTATE_ENABLED, TBSTYLE_BUTTON, TBBUTTON_INIT, 19, 0},
{0, 0, TBSTATE_ENABLED , TBSTYLE_SEP, TBBUTTON_INIT, 0, 0},
{20, DANAE_B015, TBSTATE_ENABLED, TBSTYLE_BUTTON, TBBUTTON_INIT, 20, 0},
{0, 0, TBSTATE_ENABLED , TBSTYLE_SEP, TBBUTTON_INIT, 0, 0},
{21, DANAE_B016, TBSTATE_ENABLED, TBSTYLE_BUTTON, TBBUTTON_INIT, 21, 0},
{0, 0, TBSTATE_ENABLED , TBSTYLE_SEP, TBBUTTON_INIT, 0, 0},
};

#endif

//-----------------------------------------------------------------------------

void LoadSysTextures();
HRESULT DANAEFinalCleanup();
void ShowFPS();
void ShowTestText();
void ManageNONCombatModeAnimations();
#ifdef BUILD_EDITOR
void LaunchMoulinex();
#endif

//-----------------------------------------------------------------------------

// Sends ON GAME_READY msg to all IOs
void SendGameReadyMsg()
{
	LogDebug << "SendGameReadyMsg";
	SendMsgToAllIO(SM_GAME_READY,"");
}

void DANAE_KillCinematic()
{
	if (	(ControlCinematique)
		&&	(ControlCinematique->projectload)	)
	{
		ControlCinematique->projectload=false;
		ControlCinematique->OneTimeSceneReInit();
		ControlCinematique->DeleteDeviceObjects();
		PLAY_LOADED_CINEMATIC=0;
		CINE_PRELOAD=0;
	}
}

#ifdef BUILD_EDITOR

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

void Danae_Registry_WriteValue(const char * string, DWORD value)
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

void Danae_Registry_Write(const char * string, const char * text)
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

void Danae_Registry_Read(const char * string, char * text, const char * defaultstr,long maxsize)
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
			memcpy(text,defaultstr,min(maxsize+1,(long)strlen(defaultstr)+1));
			text[min(maxsize,(long)strlen(defaultstr))]=0;
			}
			else text[0]=0;
		}
	} else {
		text[0] = 0;
	}
}

//-----------------------------------------------------------------------------------------------

void Danae_Registry_ReadValue(const char * string, long * value, long defaultvalue)
{
	if (!FINAL_RELEASE)
	{
		if (DanaeKey==NULL) Danae_Registry_Open();

		if (DanaeKey!=NULL)
		{
			ReadRegKeyValue( DanaeKey, string, value);
			Danae_Registry_Close();
		}
		else 
			*value = defaultvalue;
	} else {
		*value = 0;
	}
}

// END - DANAE Registery Funcs ******************************************************************
//-----------------------------------------------------------------------------------------------
#endif // BUILD_EDITOR


void DanaeSwitchFullScreen()
{
	if (danaeApp.m_pDeviceInfo->bWindowed) // switching to fullscreen
	{
#ifdef BUILD_EDITOR
		KillInterTreeView();
#endif
	}

		int nb=danaeApp.m_pDeviceInfo->dwNumModes;

		for(int i=0;i<nb;i++)
		{

			ARX_CHECK_NOT_NEG( config.video.bpp );

			if( danaeApp.m_pDeviceInfo->pddsdModes[i].ddpfPixelFormat.dwRGBBitCount == ARX_CAST_UINT( config.video.bpp ) )
			{
				ARX_CHECK_NOT_NEG( config.video.width );
				ARX_CHECK_NOT_NEG( config.video.height );

				if( ( danaeApp.m_pDeviceInfo->pddsdModes[i].dwWidth == ARX_CAST_UINT( config.video.width ) ) &&
					( danaeApp.m_pDeviceInfo->pddsdModes[i].dwHeight == ARX_CAST_UINT( config.video.height ) ) )
				{

					danaeApp.m_pDeviceInfo->ddsdFullscreenMode=danaeApp.m_pDeviceInfo->pddsdModes[i];
					danaeApp.m_pDeviceInfo->dwCurrentMode=i;
					break;
				}
			}
		}

		config.video.bpp = danaeApp.m_pFramework->bitdepth = danaeApp.m_pDeviceInfo->ddsdFullscreenMode.ddpfPixelFormat.dwRGBBitCount;
		config.video.height = danaeApp.m_pFramework->m_dwRenderHeight = danaeApp.m_pDeviceInfo->ddsdFullscreenMode.dwHeight;
		config.video.width = danaeApp.m_pFramework->m_dwRenderWidth = danaeApp.m_pDeviceInfo->ddsdFullscreenMode.dwWidth;

	if(pMenu)
	{
		pMenu->bReInitAll=true;
	}

	ARX_Text_Close();
	danaeApp.SwitchFullScreen();

	AdjustUI();

	LoadScreen();
}

void AdjustUI()
{
	// Sets Danae Screen size depending on windowed/full-screen state
	DANAESIZX = danaeApp.m_pFramework->m_dwRenderWidth;
	DANAESIZY = danaeApp.m_pFramework->m_dwRenderHeight;

	if (danaeApp.m_pDeviceInfo->bWindowed)
		DANAESIZY -= danaeApp.m_pFramework->Ystart;

	// Now computes screen center
	DANAECENTERX = DANAESIZX>>1;
	DANAECENTERY = DANAESIZY>>1;

	// Computes X & Y screen ratios compared to a standard 640x480 screen
	Xratio = DANAESIZX * ( 1.0f / 640 );
	Yratio = DANAESIZY * ( 1.0f / 480 );

	ARX_Text_Init();

	if(pMenu)
		pMenu->bReInitAll=true;
}

void DanaeRestoreFullScreen() {

	danaeApp.m_pDeviceInfo->bWindowed=!danaeApp.m_pDeviceInfo->bWindowed;
	danaeApp.SwitchFullScreen();

	AdjustUI();

	LoadScreen();
}

//-----------------------------------------------------------------------------------------------

extern void InitTileLights();

//-----------------------------------------------------------------------------

void InitializeDanae()
{
	InitTileLights();
	
	char levelPath[512];
	EERIEMathPrecalc();
	ARX_MISSILES_ClearAll();
	ARX_SPELLS_Init();

	ARX_SPELLS_ClearAllSymbolDraw();
	ARX_PARTICLES_ClearAll();
	ARX_BOOMS_ClearAll();
	ARX_MAGICAL_FLARES_FirstInit();

	strcpy(LastLoadedScene,"");
	strcpy(levelPath,"Graph\\Levels\\Level");

	switch (Project.demo)
	{
		case NOLEVEL:
			levelPath[0] = 0;
			break;
		case LEVELDEMO:
			strcat(levelPath, "Demo\\");
			break;
		case LEVELDEMO2:
			strcat(levelPath, "Demo2\\");
			break;
		case LEVELDEMO3:
			strcat(levelPath, "Demo3\\");
			break;
		case LEVELDEMO4:
			strcat(levelPath, "Demo4\\");
			break;
		case LEVEL0:
			strcat(levelPath, "0\\");
			break;
		case LEVEL1:
			strcat(levelPath, "1\\");
			break;
		case LEVEL2:
			strcat(levelPath, "2\\");
			break;
		case LEVEL3:
			strcat(levelPath, "3\\");
			break;
		case LEVEL4:
			strcat(levelPath, "4\\");
			break;
		case LEVEL5:
			strcat(levelPath, "5\\");
			break;
		case LEVEL6:
			strcat(levelPath, "6\\");
			break;
		case LEVEL7:
			strcat(levelPath, "7\\");
			break;
		case LEVEL8:
			strcat(levelPath, "8\\");
			break;
		case LEVEL9:
			strcat(levelPath, "9\\");
			break;
		case LEVEL10:
			strcat(levelPath, "10\\");
			break;
		case LEVEL11:
			strcat(levelPath, "11\\");
			break;
		case LEVEL12:
			strcat(levelPath, "12\\");
			break;
		case LEVEL13:
			strcat(levelPath, "13\\");
			break;
		case LEVEL14:
			strcat(levelPath, "14\\");
			break;
		case LEVEL15:
			strcat(levelPath, "15\\");
			break;
		case LEVEL16:
			strcat(levelPath, "16\\");
			break;
		case LEVEL17:
			strcat(levelPath, "17\\");
			break;
		case LEVEL18:
			strcat(levelPath, "18\\");
			break;
		case LEVEL19:
			strcat(levelPath, "19\\");
			break;
		case LEVEL20:
			strcat(levelPath, "20\\");
			break;
		case LEVEL21:
			strcat(levelPath, "21\\");
			break;
		case LEVEL22:
			strcat(levelPath, "22\\");
			break;
		case LEVEL23:
			strcat(levelPath, "23\\");
			break;
		case LEVEL24:
			strcat(levelPath, "24\\");
			break;
		case LEVEL25:
			strcat(levelPath, "25\\");
			break;
		case LEVEL26:
			strcat(levelPath, "26\\");
			break;
		case LEVEL27:
			strcat(levelPath, "27\\");
			break;
		default:
			levelPath[0] = 0;
	}

	memset(&DefaultBkg,0,sizeof(EERIE_BACKGROUND));
	ACTIVEBKG=&DefaultBkg;
	InitBkg(ACTIVEBKG,MAX_BKGX,MAX_BKGZ,BKG_SIZX,BKG_SIZZ);
	InitNodes(1);

	player.size.y=subj.size.a=-PLAYER_BASE_HEIGHT;
	player.size.x=subj.size.b=PLAYER_BASE_RADIUS;
	player.size.z=subj.size.g=PLAYER_BASE_RADIUS;
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
	subj.bkgcolor = Color::none;

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

	mapcam.pos.x=1500.f;
	mapcam.pos.y=-6000.f;
	mapcam.pos.z=1500.f;
	mapcam.angle.a=90.f;
	mapcam.angle.b=0.f;
	mapcam.angle.g=0.f;
	mapcam.clip.left=0; 
	mapcam.clip.top=0;
	mapcam.clip.right=640;
	mapcam.clip.bottom=480;
	mapcam.clipz0=0.001f;
	mapcam.clipz1=0.999f;
	mapcam.centerx=(mapcam.clip.right-mapcam.clip.left)/2;
	mapcam.centery=(mapcam.clip.bottom-mapcam.clip.top)/2;
	mapcam.AddX=320.f;
	mapcam.AddY=240.f;
	mapcam.focal=400.f;
	mapcam.Zdiv=3000.f;
	mapcam.Zmul=1.f/mapcam.Zdiv;
	mapcam.clip3D=1000;
	mapcam.type=CAM_TOPVIEW;
	mapcam.bkgcolor = Color::fromBGRA(0x001F1F55);
	SetActiveCamera(&mapcam);
	SetCameraDepth(10000.f);
	danaeApp.MustRefresh=true;

	for (long i=0;i<32;i++)
		memcpy(&TCAM[i],&subj,sizeof(EERIE_CAMERA));

	ACTIVEBKG->ambient.r = 0.09f;
	ACTIVEBKG->ambient.g = 0.09f;
	ACTIVEBKG->ambient.b = 0.09f;
	ACTIVEBKG->ambient255.r=ACTIVEBKG->ambient.r*255.f;
	ACTIVEBKG->ambient255.g=ACTIVEBKG->ambient.g*255.f;
	ACTIVEBKG->ambient255.b=ACTIVEBKG->ambient.b*255.f;

	LoadSysTextures();
	CreateInterfaceTextureContainers();

//	LaunchDemo = 0;

	if (LaunchDemo) {
		LogInfo << "Launching Demo";

		if ((FINAL_RELEASE) && (config.video.fullscreen || AUTO_FULL_SCREEN )) {
			LogDebug << "Switching to Fullscreen";
			DanaeSwitchFullScreen();
		}
		LaunchDemo=0;
		SPLASH_THINGS_STAGE=11;
	} else if (levelPath[0]!=0)	{
		LogInfo << "Launching Level " << levelPath;
		if (FastSceneLoad(levelPath)) {
			FASTmse = 1;
		} else {
#ifdef BUILD_EDIT_LOADSAVE
			ARX_SOUND_PlayCinematic("Editor_Humiliation.wav");
			mse = PAK_MultiSceneToEerie(levelPath);
#else
			LogError << "FastSceneLoad failed";
#endif
		}
		EERIEPOLY_Compute_PolyIn();
		strcpy(LastLoadedScene,levelPath);
	}

#ifdef BUILD_EDITOR
	if(GAME_EDITOR && !MOULINEX) {
		LaunchInteractiveObjectsApp( danaeApp.m_hWnd);
	}
#endif

}

//-----------------------------------------------------------------------------

void forInternalPeople(LPSTR strCmdLine) {
	
	LogDebug << "not FOR_EXTERNAL_PEOPLE";
	
#ifdef BUILD_EDITOR
	
	char * param[10];
	
	param[0] = strtok(strCmdLine, " ");
	
	for(long j = 1; j < 10; j++) {
		param[j] = strtok(NULL," ");
	}
	
	long parampos = 0;
	if((param[parampos] != NULL)) {
		LogInfo << "PARAMS";
		FINAL_RELEASE=0;
		GAME_EDITOR=1;
		
		if (!strcasecmp(param[parampos],"editor")) {
			LogInfo << "PARAM EDITOR";
			NEED_ANCHORS=1;
		} else {
			NEED_ANCHORS=1;
			USE_FAST_SCENES=0;
			LogInfo << "PARAM MOULINEX";
			
			if (param[parampos][0]=='-') {
				long posflags=parampos;
				PROCESS_NO_POPUP=1;
				PROCESS_ALL_THEO=0;
				PROCESS_LEVELS=0;
				PROCESS_ONLY_ONE_LEVEL=-1;
				
				if ((IsIn(param[posflags],"u")) || (IsIn(param[posflags],"U"))) {
					parampos++;
					PROCESS_ONLY_ONE_LEVEL=atoi(param[parampos]);
				}
				
				if ((IsIn(param[posflags],"o")) || (IsIn(param[posflags],"O"))) {
					PROCESS_ALL_THEO=1;
				}
				
				if ((IsIn(param[posflags],"f")) || (IsIn(param[posflags],"F"))) {
					NEED_ANCHORS=0;
					USE_FAST_SCENES=1;
					NOCHECKSUM=1;
				}
				
				if ((IsIn(param[posflags],"l")) || (IsIn(param[posflags],"L"))) {
					PROCESS_LEVELS=1;
				}
				
				if ((IsIn(param[posflags],"t")) || (IsIn(param[posflags],"T"))) {
					TSU_LIGHTING=1;
				}
				
				parampos++;
			} else {
				PROCESS_ALL_THEO=1;
				PROCESS_LEVELS=1;
			}
			
			if(!strcasecmp(param[parampos],"moulinex")) {
				LogInfo << "Launching moulinex";
				MOULINEX=1;
				KILL_AT_MOULINEX_END=1;
			}
		}
	} else {
		LogInfo << "FRGE";
		GAME_EDITOR=1;
		if (FINAL_RELEASE)
			GAME_EDITOR=0;
	}
	
#else
	ARX_UNUSED(strCmdLine);
#endif
}

HWND mainWindow = 0;

// Let's use main for now on all platforms
// TODO: On Windows, we might want to use WinMain in the Release target for example
int main(int argc, char ** argv) {
	
	(void)argc, (void)argv;
	
	LPSTR strCmdLine = GetCommandLine();
	hInstance = GetModuleHandle(0);
	
	long i;
	
	if (FINAL_COMMERCIAL_GAME) {
		LogDebug << "FINAL_COMMERCIAL_GAME";
		FOR_EXTERNAL_PEOPLE=1;
	} else if (FINAL_COMMERCIAL_DEMO)	{
		LogDebug << "FINAL_COMMERCIAL_DEMO";
		FOR_EXTERNAL_PEOPLE=1;
	}

	if(FOR_EXTERNAL_PEOPLE) {
		LogDebug << "FOR_EXTERNAL_PEOPLE";
		ALLOW_CHEATS		= 0;
		NO_TEXT_AT_ALL		= 1;

		FAST_SPLASHES		= 0;
		FORCE_SHOW_FPS		= 0;
		FINAL_RELEASE		= 1;
#ifdef _DEBUG
		AUTO_FULL_SCREEN	= 0;
#endif
		DEBUG_FRUSTRUM		= 0;
#ifdef BUILD_EDITOR
		GAME_EDITOR = 0;
		NEED_EDITOR = 0;
		TRUEFIGHT = 0;
#endif
	}
	
	// Initialize config first, before anything else.
	const char RESOURCE_CONFIG[] = "cfg.ini";
	const char RESOURCE_CONFIG_DEFAULT[] = "cfg_default.ini";
	if(!config.init(RESOURCE_CONFIG, RESOURCE_CONFIG_DEFAULT)) {
		LogWarning << "Could not read config files " << RESOURCE_CONFIG << " and " << RESOURCE_CONFIG_DEFAULT;
	}
	
	Random::seed();
	
	CalcFPS(true);

	ARX_MAPMARKER_Init();

	for (i=0;i<8;i++)
		scursor[i]=NULL;

	ARX_SPELLS_CancelSpellTarget();

	for (i=0;i<MAX_EXPLO;i++) explo[i]=NULL;

	USE_FAST_SCENES = 1;
	LogDebug << "Danae Start";

	Project.vsync = true;
	LogDebug << "Project Init";

	if (!FOR_EXTERNAL_PEOPLE)
		forInternalPeople(strCmdLine);

	NOCHECKSUM=0;

	if(FINAL_RELEASE) {
		
		LogInfo << "FINAL RELEASE";
		NOBUILDMAP=1;
		NOCHECKSUM=1;
		
		const char PAK_DATA[] = "data.pak";
		LogDebug << PAK_DATA;
		if(PAK_AddPak(PAK_DATA)) {
			LogDebug << "LoadMode OK";
		} else {
			LogFatal << "Unable to find main data file " << PAK_DATA;
		}
		
		const char PAK_LOC[] = "loc.pak";
		LogDebug << "LocPAK";
		if(!PAK_AddPak(PAK_LOC)) {
			const char PAK_LOC_DEFAULT[] = "loc_default.pak";
			if(!PAK_AddPak(PAK_LOC_DEFAULT)) {
				LogFatal << "Unable to find localisation file " << PAK_LOC << " or " << PAK_LOC_DEFAULT;
			}
		}
		
		LogDebug << "data2PAK";
		const char PAK_DATA2[] = "data2.pak";
		if(!PAK_AddPak(PAK_DATA2)) {
			LogFatal << "Unable to find aux data file " << PAK_DATA2;
		}
		
		const char PAK_SFX[] = "sfx.pak";
		if(!PAK_AddPak(PAK_SFX)) {
			LogFatal << "Unable to find sfx data file " << PAK_SFX;
		}
		
		const char PAK_SPEECH[] = "speech.pak";
		if(!PAK_AddPak(PAK_SPEECH)) {
			const char PAK_SPEECH_DEFAULT[] = "speech_default.pak";
			if(!PAK_AddPak(PAK_SPEECH_DEFAULT)) {
				LogFatal << "Unable to find speech data file " << PAK_SPEECH << " or " << PAK_SPEECH_DEFAULT;
			}
		}
		
	} else {
		LogInfo << "TRUEFILE LM";
		//TODO(lubosz): dirty hack to initialize the pak manager
		PAK_AddPak("");
	}
	
	GRenderer = new Direct3DRenderer();
	
	LocalisationInit();
	
	//delete current for clean save.........
	char txttemp[256];

	for(unsigned uiNum=0; uiNum < 20; ++uiNum) {
		sprintf(txttemp,"Save%s\\Cur%04d\\",LOCAL_SAVENAME,uiNum);

		if (DirectoryExist(txttemp))
			KillAllDirectory(txttemp);
	}

	ARX_INTERFACE_NoteInit();
	LogDebug << "Note Init";
	PUSH_PLAYER_FORCE = Vec3f::ZERO;
	ARX_SPECIAL_ATTRACTORS_Reset();
	LogDebug << "Attractors Init";
	ARX_SPELLS_Precast_Reset();
	LogDebug << "Spell Init";
	
	for(size_t t = 0; t < MAX_GOLD_COINS_VISUALS; t++) {
		GoldCoinsObj[t]=NULL;
		GoldCoinsTC[t]=NULL;
	}

	LogDebug << "GC Init";
	memset(LOCAL_SAVENAME,0,60);
	LogDebug << "LSV Init";
	ModeLight=MODE_DYNAMICLIGHT | MODE_DEPTHCUEING;

	memset(&DefaultBkg,0,sizeof(EERIE_BACKGROUND));
	memset(TELEPORT_TO_LEVEL,0,64);
	memset(TELEPORT_TO_POSITION,0,64);
	LogDebug << "Mset";
	
	EERIE_ANIMMANAGER_Init();
	LogDebug << "AnimManager Init";
	ARX_SCRIPT_EventStackInit();
	LogDebug << "EventStack Init";
	ARX_EQUIPMENT_Init();
	LogDebug << "AEQ Init";
	memset(_CURRENTLOAD_,0,256);

#ifdef BUILD_EDITOR
	char temp[256];

	Danae_Registry_Read("LastWorkingDir",temp,"");

	if (temp[0]==0)	{
		Danae_Registry_WriteValue("WND_IO_DlgProc_POSX",0);
		Danae_Registry_WriteValue("WND_IO_DlgProc_POSY",0);
		Danae_Registry_WriteValue("WND_LightPrecalc_POSX",0);
		Danae_Registry_WriteValue("WND_LightPrecalc_POSY",0);
		Danae_Registry_WriteValue("WND_LightOptions_POSX",0);
		Danae_Registry_WriteValue("WND_LightOptions_POSY",0);
		LogDebug << "RegData Read";
	}

	Danae_Registry_Read("LOCAL_SAVENAME",LOCAL_SAVENAME,"",16);
#endif

	if (!FOR_EXTERNAL_PEOPLE) {
		char stemp[256];
		u32 ls = 64;
		GetComputerName(stemp, &ls);

		if (!strcasecmp(stemp,"max")) {
			CYRIL_VERSION=1;
			AUTO_FULL_SCREEN=0;
		}
	}	

	ARX_CHANGELEVEL_MakePath();
	LogDebug << "ACL MakePath";

	LastLoadedDLF[0]=0;
	ARX_SCRIPT_Timer_FirstInit(512);
	LogDebug << "Timer Init";
	ARX_FOGS_FirstInit();
	LogDebug << "Fogs Init";

	EERIE_LIGHT_GlobalInit();
	LogDebug << "Lights Init";
	
	LogDebug << "Svars Init";

	// Script Test
	lastteleport.x=0.f;
	lastteleport.y=PLAYER_BASE_HEIGHT;
	lastteleport.z=0.f;

	inter.init=0;
	InitInter(10);

	memset(&player,0,sizeof(ARXCHARACTER));
	ARX_PLAYER_InitPlayer();

	CleanInventory();

	ARX_SPEECH_FirstInit();
	ARX_CONVERSATION_FirstInit();
	ARX_SPEECH_Init();
	ARX_SPEECH_ClearAll();
	QuakeFx.intensity=0.f;

	LogDebug << "Launching DANAE";

	if (!FINAL_COMMERCIAL_DEMO && !FINAL_COMMERCIAL_GAME) {
		if (!LoadLibrary("RICHED32.DLL")) {
			LogError  << "DanaeScriptEditor :: IDS_RICHED_LOAD_FAIL";
		}
	}

	if (FINAL_RELEASE) {
		LogDebug << "FINAL_RELEASE";
		LaunchDemo=1;
		Project.TextureSize=0;
		Project.TextureBits=16;
		Project.bits=32;
		Project.compatibility=0;
		Project.demo=LEVEL10;
		Project.ambient=1;
		Project.multiplayer=0;
		NOCHECKSUM=1;
	}
#ifdef BUILD_EDITOR
	else if (!MOULINEX) {
//		DialogBox( hInstance,MAKEINTRESOURCE(IDD_STARTOPTIONS), NULL, StartProc );
		LogError << "not MOULINEX ";
	}
#endif
	else {
		LogInfo << "default LEVELDEMO2";
		Project.demo=LEVELDEMO2;
	}

	LogDebug << "After Popup";
	atexit(ClearGame);

	if (LaunchDemo)	{
		LogInfo << "LaunchDemo";

#ifdef BUILD_EDITOR
		if(FINAL_RELEASE) {
			GAME_EDITOR=0;
		} else {
			GAME_EDITOR=1;
		}
#endif

		NOBUILDMAP=1;
		NOCHECKSUM=1;
	}

	if(LAST_CHINSTANCE != -1) {
		ARX_CHANGELEVEL_MakePath();
		LogInfo << "Clearing current game directory " << CurGamePath;
		KillAllDirectory(CurGamePath);
		CreateDirectory(CurGamePath,NULL);
	}

	Project.improve=0;
	Project.interpolatemouse = 0;

	danaeApp.d_dlgframe=0;

#ifdef BUILD_EDITOR
	if(MOULINEX) {
		danaeApp.WndSizeX = 800;
		danaeApp.WndSizeY = 12;
	} else
#endif
	{
		danaeApp.WndSizeX = config.video.width;
		danaeApp.WndSizeY = config.video.height;
		danaeApp.Fullscreen = config.video.fullscreen;
	}

#ifdef BUILD_EDITOR
	if ((GAME_EDITOR && !MOULINEX && !FINAL_RELEASE) || NEED_EDITOR) {
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
		danaeApp.ToolBar->String.clear();
	}
	else
#endif
	{
		danaeApp.CreationFlags= WCF_NOSTDPOPUP;
#ifdef BUILD_EDITOR
		if(GAME_EDITOR) danaeApp.CreationFlags|= WCF_ACCEPTFILES;
#endif
	}

	LogDebug << "Application Creation";
	g_pD3DApp = &danaeApp;

	if( FAILED( danaeApp.Create( hInstance ) ) )
		return 0;
	mainWindow = danaeApp.m_hWnd;
	
	AdjustUI();

	LogInfo << "Application Creation Success";

	danaeApp.m_pFramework->bitdepth=Project.bits;
	
	LogDebug << "Sound init";
	if(ARX_SOUND_Init()) {
		LogInfo << "Sound init success";
	} else {
		LogWarning << "Sound init failed";
	}
	
	LogDebug << "Input init";
	if(ARX_INPUT_Init()) {
		LogInfo << "Input init success";
	} else {
		LogError << "Input init failed";
		return 0;
	}

	ARX_SetAntiAliasing();
	ARXMenu_Options_Video_SetFogDistance(config.video.fogDistance);
	ARXMenu_Options_Video_SetTextureQuality(config.video.textureSize);
	ARXMenu_Options_Video_SetLODQuality(config.video.meshReduction);
	ARXMenu_Options_Video_SetDetailsQuality(config.video.levelOfDetail);
	ARXMenu_Options_Video_SetGamma(config.video.gamma);
	ARXMenu_Options_Audio_SetMasterVolume(config.audio.volume);
	ARXMenu_Options_Audio_SetSfxVolume(config.audio.sfxVolume);
	ARXMenu_Options_Audio_SetSpeechVolume(config.audio.speechVolume);
	ARXMenu_Options_Audio_SetAmbianceVolume(config.audio.ambianceVolume);
	ARXMenu_Options_Audio_ApplyGameVolumes();

	ARXMenu_Options_Control_SetInvertMouse(config.input.invertMouse);
	ARXMenu_Options_Control_SetAutoReadyWeapon(config.input.autoReadyWeapon);
	ARXMenu_Options_Control_SetMouseLookToggleMode(config.input.mouseLookToggle);
	ARXMenu_Options_Control_SetMouseSensitivity(config.input.mouseSensitivity);
	ARXMenu_Options_Control_SetAutoDescription(config.input.autoDescription);
	
	if(config.video.textureSize==2)Project.TextureSize=0;
	if(config.video.textureSize==1)Project.TextureSize=2;
	if(config.video.textureSize==0)Project.TextureSize=64;

	ARX_MINIMAP_FirstInit();
		
	//read from cfg file
	if ( config.language.length() == 0 ) {
		config.language = "english";
		LogWarning << "Falling back to default localisationpath";
	}

	char tex[512];

#ifdef BUILD_EDITOR
	if(GAME_EDITOR) {
		sprintf(tex,"DANAE Project");
	} else
#endif
	{
		sprintf(tex,"ARX Fatalis");
	}

#ifdef BUILD_EDITOR
	if(MOULINEX)
		sprintf(tex,"MOULINEX");
#endif

	strcat(tex, arxVersion.c_str());
	SetWindowText(danaeApp.m_hWnd, tex);
	
	Project.interfacergb.r = 0.46f;
	Project.interfacergb.g = 0.46f;
	Project.interfacergb.b = 1.f;

	Project.torch.r=1.f;
	Project.torch.g = 0.8f;
	Project.torch.b = 0.66666f;
	LogDebug << "InitializeDanae";
	InitializeDanae();
	
	LogInfo << "InitializeDanae Success";
	LogDebug << "DanaeApp RUN";
	danaeApp.m_bReady = true;

	HRESULT hr=danaeApp.Run();

	return hr;
}

//*************************************************************************************
// DANAE()
//  Application constructor. Sets attributes for the app.
//*************************************************************************************
DANAE::DANAE() : CD3DApplication()
{
	m_strWindowTitle  = TEXT("ARX Fatalis");
	m_bAppUseZBuffer  = true;
	m_bAppUseStereo   = false;
	m_bShowStats      = true;
	m_fnConfirmDevice = NULL;
	m_hWnd=NULL;
}

//*************************************************************************************
// INTERACTIVE_OBJ * FlyingOverObject(EERIE_S2D * pos)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Returns IO under cursor, be it in inventories or in scene
//   Returns NULL if no IO under cursor
//*************************************************************************************
// Nuky - 01-02-11 - simplified prototype and recoded with new CD3DApplicationScopedLock
//                   class and recursion
INTERACTIVE_OBJ * FlyingOverObject(Vec2s * pos, bool mustlock)
{
	INTERACTIVE_OBJ* io = NULL;

	if (mustlock)
	{
		CD3DApplicationScopedLock lock(danaeApp);
		if (lock)
			return FlyingOverObject(pos, false);
	}

	if ((io = GetFromInventory(pos)) != NULL)
		return io;
	if (InInventoryPos(pos))
		return NULL;

	if ((io = InterClick(pos)) != NULL)
		return io;

	return NULL;
}
extern long cur_rf;
extern unsigned long FALLING_TIME;
extern long ARX_NPC_ApplyCuts(INTERACTIVE_OBJ * io);

//*************************************************************************************

void LoadSysTextures()
{
	char temp[256];

	for (long i=1;i<10;i++)
	{
		sprintf(temp,"Graph\\Particles\\shine%ld.bmp",i);
		flaretc.shine[i]=TextureContainer::LoadUI(temp);

	}

	for (size_t i=0;i<SPELL_COUNT;i++)
	{
		// TODO use constructor for initialization
		for (long j = 0; j < 6; j++) spellicons[i].symbols[j] = RUNE_NONE;
		spellicons[i].level = 0;
		spellicons[i].spellid = SPELL_NONE;
		spellicons[i].tc = NULL;
		spellicons[i].bSecret = false;
		spellicons[i].bDuration = true;
		spellicons[i].bAudibleAtStart = false;
	}
	
	long i;

	SPELL_ICON * current;

	// Magic_Sight Level 1
	current=&spellicons[SPELL_MAGIC_SIGHT];
	current->name = getLocalised("system_spell_name_magic_sight");
	current->description = getLocalised("system_spell_description_magic_sight");
	current->level=1;
	current->spellid=SPELL_MAGIC_SIGHT;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_magic_sight.bmp");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_VISTA;

	// Magic_Missile Level 1
	current=&spellicons[SPELL_MAGIC_MISSILE];	
	current->name = getLocalised("system_spell_name_magic_projectile");
	current->description = getLocalised("system_spell_description_magic_projectile");
	current->level=1;
	current->spellid=SPELL_MAGIC_MISSILE;
	current->bDuration = false;
	current->bAudibleAtStart = true;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_magic_missile.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_TAAR;

	// Ignit Level 1
	current=&spellicons[SPELL_IGNIT];	
	current->name = getLocalised("system_spell_name_ignit");
	current->description = getLocalised("system_spell_description_ignit");
	current->level=1;
	current->spellid=SPELL_IGNIT;
	current->bDuration = false;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_ignite.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_YOK;

	// Douse Level 1
	current=&spellicons[SPELL_DOUSE];	
	current->name = getLocalised("system_spell_name_douse");
	current->description = getLocalised("system_spell_description_douse");
	current->level=1;
	current->spellid=SPELL_DOUSE;
	current->bDuration = false;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_douse.bmp");
	current->symbols[0]=RUNE_NHI;
	current->symbols[1]=RUNE_YOK;

	// Activate_Portal Level 1
	current=&spellicons[SPELL_ACTIVATE_PORTAL];
	current->name = getLocalised("system_spell_name_activate_portal");
	current->description = getLocalised("system_spell_description_activate_portal");
	current->level=1;
	current->spellid=SPELL_ACTIVATE_PORTAL;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_activate_portal.bmp");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_SPACIUM;
	current->bSecret = true;

	// Heal Level 2
	current=&spellicons[SPELL_HEAL];	
	current->name = getLocalised("system_spell_name_heal");
	current->description = getLocalised("system_spell_description_heal");
	current->level=2;
	current->spellid=SPELL_HEAL;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_heal.bmp");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_VITAE;

	// Detect_trap Level 2
	current=&spellicons[SPELL_DETECT_TRAP];	
	current->name = getLocalised("system_spell_name_detect_trap");
	current->description = getLocalised("system_spell_description_detect_trap");
	current->level=2;
	current->spellid=SPELL_DETECT_TRAP;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_detect_trap.bmp");
	current->symbols[0]=RUNE_MORTE;
	current->symbols[1]=RUNE_COSUM;
	current->symbols[2]=RUNE_VISTA;

	// Armor Level 2
	current=&spellicons[SPELL_ARMOR];	
	current->name = getLocalised("system_spell_name_armor");
	current->description = getLocalised("system_spell_description_armor");
	current->level=2;
	current->spellid=SPELL_ARMOR;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_armor.bmp");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_KAOM;

	// Lower Armor Level 2
	current=&spellicons[SPELL_LOWER_ARMOR];	
	current->name = getLocalised("system_spell_name_lower_armor");
	current->description = getLocalised("system_spell_description_lower_armor");
	current->level=2;
	current->spellid=SPELL_LOWER_ARMOR;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_lower_armor.bmp");
	current->symbols[0]=RUNE_RHAA;
	current->symbols[1]=RUNE_KAOM;

	// Harm Level 2
	current=&spellicons[SPELL_HARM];	
	current->name = getLocalised("system_spell_name_harm");
	current->description = getLocalised("system_spell_description_harm");
	current->level=2;
	current->spellid=SPELL_HARM;
	current->bAudibleAtStart = true;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_harm.bmp");
	current->symbols[0]=RUNE_RHAA;
	current->symbols[1]=RUNE_VITAE;
	current->bSecret = true;

	// Speed Level 3
	current=&spellicons[SPELL_SPEED];	
	current->name = getLocalised("system_spell_name_speed");
	current->description = getLocalised("system_spell_description_speed");
	current->level=3;
	current->spellid=SPELL_SPEED;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_speed.bmp");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_MOVIS;

	// Reveal Level 3
	current=&spellicons[SPELL_DISPELL_ILLUSION];	
	current->name = getLocalised("system_spell_name_reveal");
	current->description = getLocalised("system_spell_description_reveal");
	current->level=3;
	current->spellid=SPELL_DISPELL_ILLUSION;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_reveal.bmp");
	current->symbols[0]=RUNE_NHI;
	current->symbols[1]=RUNE_STREGUM;
	current->symbols[2]=RUNE_VISTA;

	// Fireball Level 3
	current=&spellicons[SPELL_FIREBALL];	
	current->name = getLocalised("system_spell_name_fireball");
	current->description = getLocalised("system_spell_description_fireball");
	current->level=3;
	current->spellid=SPELL_FIREBALL;
	current->bDuration = false;
	current->bAudibleAtStart = true;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_fireball.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_YOK;
	current->symbols[2]=RUNE_TAAR;

	// Create Food Level 3
	current=&spellicons[SPELL_CREATE_FOOD];	
	current->name = getLocalised("system_spell_name_create_food");
	current->description = getLocalised("system_spell_description_create_food");
	current->level=3;
	current->spellid=SPELL_CREATE_FOOD;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_create_food.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_VITAE;
	current->symbols[2]=RUNE_COSUM;

	// Ice Projectile Level 3
	current=&spellicons[SPELL_ICE_PROJECTILE];	
	current->name = getLocalised("system_spell_name_ice_projectile");
	current->description = getLocalised("system_spell_description_ice_projectile");
	current->level=3;
	current->spellid=SPELL_ICE_PROJECTILE;
	current->bDuration = false;
	current->bAudibleAtStart = true;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_iceball.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_FRIDD;
	current->symbols[2]=RUNE_TAAR;
	current->bSecret = true;

	// Bless Level 4
	current=&spellicons[SPELL_BLESS];	
	current->name = getLocalised("system_spell_name_sanctify");
	current->description = getLocalised("system_spell_description_sanctify");
	current->level=4;
	current->spellid=SPELL_BLESS;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_bless.bmp");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_STREGUM;
	current->symbols[2]=RUNE_VITAE;

	// Dispel_Field Level 4
	current=&spellicons[SPELL_DISPELL_FIELD];	
	current->name = getLocalised("system_spell_name_dispell_field");
	current->description = getLocalised("system_spell_description_dispell_field");
	current->level=4;
	current->spellid=SPELL_DISPELL_FIELD;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_dispell_field.bmp");
	current->symbols[0]=RUNE_NHI;

	current->symbols[1]=RUNE_SPACIUM;

	// Cold Protection Level 4
	current=&spellicons[SPELL_COLD_PROTECTION];	
	current->name = getLocalised("system_spell_name_cold_protection");
	current->description = getLocalised("system_spell_description_cold_protection");
	current->level=4;
	current->spellid=SPELL_COLD_PROTECTION;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_protection_cold.bmp");
	current->symbols[0]=RUNE_FRIDD;
	current->symbols[1]=RUNE_KAOM;
	current->bSecret = true;

	// Fire Protection Level 4
	current=&spellicons[SPELL_FIRE_PROTECTION];	
	current->name = getLocalised("system_spell_name_fire_protection");
	current->description = getLocalised("system_spell_description_fire_protection");
	current->level=4;
	current->spellid=SPELL_FIRE_PROTECTION;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_protection_fire.bmp");
	current->symbols[0]=RUNE_YOK;
	current->symbols[1]=RUNE_KAOM;

	// Telekinesis Level 4
	current=&spellicons[SPELL_TELEKINESIS];	
	current->name = getLocalised("system_spell_name_telekinesis");
	current->description = getLocalised("system_spell_description_telekinesis");
	current->level=4;
	current->spellid=SPELL_TELEKINESIS;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_telekinesis.bmp");
	current->symbols[0]=RUNE_SPACIUM;
	current->symbols[1]=RUNE_COMUNICATUM;

	// Curse Level 4
	current=&spellicons[SPELL_CURSE];	
	current->name = getLocalised("system_spell_name_curse");
	current->description = getLocalised("system_spell_description_curse");
	current->level=4;
	current->spellid=SPELL_CURSE;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_curse.bmp");
	current->symbols[0]=RUNE_RHAA;
	current->symbols[1]=RUNE_STREGUM;
	current->symbols[2]=RUNE_VITAE;
	current->bSecret = true;

	// Rune of Guarding Level 5
	current=&spellicons[SPELL_RUNE_OF_GUARDING];	
	current->name = getLocalised("system_spell_name_rune_guarding");
	current->description = getLocalised("system_spell_description_rune_guarding");
	current->level=5;
	current->spellid=SPELL_RUNE_OF_GUARDING;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_rune_guarding.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_MORTE;
	current->symbols[2]=RUNE_COSUM;

	// Levitate Level 5
	current=&spellicons[SPELL_LEVITATE];	
	current->name = getLocalised("system_spell_name_levitate");
	current->description = getLocalised("system_spell_description_levitate");
	current->level=5;
	current->spellid=SPELL_LEVITATE;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_levitate.bmp");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_SPACIUM;
	current->symbols[2]=RUNE_MOVIS;

	// Cure Poison Level 5
	current=&spellicons[SPELL_CURE_POISON];	
	current->name = getLocalised("system_spell_name_cure_poison");
	current->description = getLocalised("system_spell_description_cure_poison");
	current->level=5;
	current->spellid=SPELL_CURE_POISON;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_cure_poison.bmp");
	current->symbols[0]=RUNE_NHI;
	current->symbols[1]=RUNE_CETRIUS;

	// Repel Undead Level 5
	current=&spellicons[SPELL_REPEL_UNDEAD];	
	current->name = getLocalised("system_spell_name_repel_undead");
	current->description = getLocalised("system_spell_description_repel_undead");
	current->level=5;
	current->spellid=SPELL_REPEL_UNDEAD;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_repel_undead.bmp");
	current->symbols[0]=RUNE_MORTE;
	current->symbols[1]=RUNE_KAOM;

	// Poison Projection Level 5
	current=&spellicons[SPELL_POISON_PROJECTILE];	
	current->name = getLocalised("system_spell_name_poison_projection");
	current->description = getLocalised("system_spell_description_poison_projection");
	current->level=5;
	current->spellid=SPELL_POISON_PROJECTILE;
	current->bDuration = false;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_poison_projection.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_CETRIUS;
	current->symbols[2]=RUNE_TAAR;
	current->bSecret = true;

	// Raise Dead Level 6
	current=&spellicons[SPELL_RISE_DEAD];	
	current->name = getLocalised("system_spell_name_raise_dead");
	current->description = getLocalised("system_spell_description_raise_dead");
	current->level=6;
	current->spellid=SPELL_RISE_DEAD;
	current->bAudibleAtStart = true;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_raise_dead.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_MORTE;
	current->symbols[2]=RUNE_VITAE;

	// Paralyse Dead Level 6
	current=&spellicons[SPELL_PARALYSE];	
	current->name = getLocalised("system_spell_name_paralyse");
	current->description = getLocalised("system_spell_description_paralyse");
	current->level=6;
	current->spellid=SPELL_PARALYSE;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_paralyse.bmp");
	current->symbols[0]=RUNE_NHI;
	current->symbols[1]=RUNE_MOVIS;

	// Create Field Dead Level 6
	current=&spellicons[SPELL_CREATE_FIELD];	
	current->name = getLocalised("system_spell_name_create_field");
	current->description = getLocalised("system_spell_description_create_field");
	current->level=6;
	current->spellid=SPELL_CREATE_FIELD;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_create_field.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_KAOM;
	current->symbols[2]=RUNE_SPACIUM;

	// Disarm Trap Level 6
	current=&spellicons[SPELL_DISARM_TRAP];	
	current->name = getLocalised("system_spell_name_disarm_trap");
	current->description = getLocalised("system_spell_description_disarm_trap");
	current->level=6;
	current->spellid=SPELL_DISARM_TRAP;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_disarm_trap.bmp");
	current->symbols[0]=RUNE_NHI;
	current->symbols[1]=RUNE_MORTE;
	current->symbols[2]=RUNE_COSUM;

	// Slow_Down Level 6 // SECRET SPELL
	current=&spellicons[SPELL_SLOW_DOWN];	
	current->name = getLocalised("system_spell_name_slowdown");
	current->description = getLocalised("system_spell_description_slowdown");
	current->level=6;
	current->spellid=SPELL_SLOW_DOWN;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_slow_down.bmp");
	current->symbols[0]=RUNE_RHAA;
	current->symbols[1]=RUNE_MOVIS;
	current->bSecret = true;

	// Flying Eye Level 7
	current=&spellicons[SPELL_FLYING_EYE];	
	current->name = getLocalised("system_spell_name_flying_eye");
	current->description = getLocalised("system_spell_description_flying_eye");
	current->level=7;
	current->spellid=SPELL_FLYING_EYE;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_flying_eye.bmp");
	current->symbols[0]=RUNE_VISTA;
	current->symbols[1]=RUNE_MOVIS;

	// Fire Field Eye Level 7
	current=&spellicons[SPELL_FIRE_FIELD];	
	current->name = getLocalised("system_spell_name_fire_field");
	current->description = getLocalised("system_spell_description_fire_field");
	current->level=7;
	current->spellid=SPELL_FIRE_FIELD;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_create_fire_field.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_YOK;
	current->symbols[2]=RUNE_SPACIUM;

	// Ice Field Level 7
	current=&spellicons[SPELL_ICE_FIELD];	
	current->name = getLocalised("system_spell_name_ice_field");
	current->description = getLocalised("system_spell_description_ice_field");
	current->level=7;
	current->spellid=SPELL_ICE_FIELD;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_create_cold_field.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_FRIDD;
	current->symbols[2]=RUNE_SPACIUM;
	current->bSecret = true;

	// Lightning Strike Level 7
	current=&spellicons[SPELL_LIGHTNING_STRIKE];	
	current->name = getLocalised("system_spell_name_lightning_strike");
	current->description = getLocalised("system_spell_description_lightning_strike");
	current->level=7;
	current->spellid=SPELL_LIGHTNING_STRIKE;
	current->bDuration = false;
	current->bAudibleAtStart = true;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_lightning_strike.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_FOLGORA;
	current->symbols[2]=RUNE_TAAR;

	// Confusion Level 7
	current=&spellicons[SPELL_CONFUSE];	
	current->name = getLocalised("system_spell_name_confuse");
	current->description = getLocalised("system_spell_description_confuse");
	current->level=7;
	current->spellid=SPELL_CONFUSE;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_confuse.bmp");
	current->symbols[0]=RUNE_RHAA;
	current->symbols[1]=RUNE_VISTA;

	// Invisibility Level 8
	current=&spellicons[SPELL_INVISIBILITY];	
	current->name = getLocalised("system_spell_name_invisibility");
	current->description = getLocalised("system_spell_description_invisibility");
	current->level=8;
	current->spellid=SPELL_INVISIBILITY;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_invisibility.bmp");
	current->symbols[0]=RUNE_NHI;
	current->symbols[1]=RUNE_VISTA;

	// Mana Drain Level 8
	current=&spellicons[SPELL_MANA_DRAIN];	
	current->name = getLocalised("system_spell_name_mana_drain");
	current->description = getLocalised("system_spell_description_mana_drain");
	current->level=8;
	current->spellid=SPELL_MANA_DRAIN;
	current->bAudibleAtStart = true;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_drain_mana.bmp");
	current->symbols[0]=RUNE_STREGUM;
	current->symbols[1]=RUNE_MOVIS;

	// Explosion Level 8
	current=&spellicons[SPELL_EXPLOSION];	
	current->name = getLocalised("system_spell_name_explosion");
	current->description = getLocalised("system_spell_description_explosion");
	current->level=8;
	current->spellid=SPELL_EXPLOSION;
	current->bDuration = false;
	current->bAudibleAtStart = true;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_explosion.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_MEGA;
	current->symbols[2]=RUNE_MORTE;

	// Enchant Weapon Level 8
	current=&spellicons[SPELL_ENCHANT_WEAPON];	
	current->name = getLocalised("system_spell_name_enchant_weapon");
	current->description = getLocalised("system_spell_description_enchant_weapon");
	current->level=8;
	current->spellid=SPELL_ENCHANT_WEAPON;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_enchant_weapon.bmp");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_STREGUM;
	current->symbols[2]=RUNE_COSUM;

	// Life Drain Level 8 // SECRET SPELL
	current=&spellicons[SPELL_LIFE_DRAIN];	
	current->name = getLocalised("system_spell_name_life_drain");
	current->description = getLocalised("system_spell_description_life_drain");
	current->level=8;
	current->spellid=SPELL_LIFE_DRAIN;
	current->bAudibleAtStart = true;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_drain_life.bmp");
	current->symbols[0]=RUNE_VITAE;
	current->symbols[1]=RUNE_MOVIS;
	current->bSecret = true;

	// Summon Creature Level 9
	current=&spellicons[SPELL_SUMMON_CREATURE];	
	current->name = getLocalised("system_spell_name_summon_creature");
	current->description = getLocalised("system_spell_description_summon_creature");
	current->level=9;
	current->spellid=SPELL_SUMMON_CREATURE;
	current->bAudibleAtStart = true;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_summon_creature.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_VITAE;
	current->symbols[2]=RUNE_TERA;

	// FAKE Summon Creature Level 9
	current=&spellicons[SPELL_FAKE_SUMMON];	
	current->name = getLocalised("system_spell_name_summon_creature");
	current->description = getLocalised("system_spell_description_summon_creature");
	current->level=9;
	current->spellid=SPELL_FAKE_SUMMON;
	current->bAudibleAtStart = true;
	current->bSecret = true;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_summon_creature.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_VITAE;
	current->symbols[2]=RUNE_TERA;

	// Negate Magic Level 9
	current=&spellicons[SPELL_NEGATE_MAGIC];	
	current->name = getLocalised("system_spell_name_negate_magic");
	current->description = getLocalised("system_spell_description_negate_magic");
	current->level=9;
	current->spellid=SPELL_NEGATE_MAGIC;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_negate_magic.bmp");
	current->symbols[0]=RUNE_NHI;
	current->symbols[1]=RUNE_STREGUM;
	current->symbols[2]=RUNE_SPACIUM;

	// Incinerate Level 9
	current=&spellicons[SPELL_INCINERATE];	
	current->name = getLocalised("system_spell_name_incinerate");
	current->description = getLocalised("system_spell_description_incinerate");
	current->level=9;
	current->spellid=SPELL_INCINERATE;
	current->bDuration = false;
	current->bAudibleAtStart = true;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_incinerate.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_MEGA;
	current->symbols[2]=RUNE_YOK;

	// Mass paralyse Creature Level 9
	current=&spellicons[SPELL_MASS_PARALYSE];	
	current->name = getLocalised("system_spell_name_mass_paralyse");
	current->description = getLocalised("system_spell_description_mass_paralyse");
	current->level=9;
	current->spellid=SPELL_MASS_PARALYSE;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_mass_paralyse.bmp");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_NHI;
	current->symbols[2]=RUNE_MOVIS;

	// Mass Lightning Strike Level 10
	current=&spellicons[SPELL_MASS_LIGHTNING_STRIKE];	
	current->name = getLocalised("system_spell_name_mass_lightning_strike");
	current->description = getLocalised("system_spell_description_mass_lightning_strike");
	current->level=10;
	current->spellid=SPELL_MASS_LIGHTNING_STRIKE;
	current->bDuration = false;
	current->bAudibleAtStart = true;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_mass_lighting_strike.bmp");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_FOLGORA;
	current->symbols[2]=RUNE_SPACIUM;

	// Control Target Level 10
	current=&spellicons[SPELL_CONTROL_TARGET];	
	current->name = getLocalised("system_spell_name_control_target");
	current->description = getLocalised("system_spell_description_control_target");
	current->level=10;
	current->spellid=SPELL_CONTROL_TARGET;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_control_target.bmp");
	current->symbols[0]=RUNE_MOVIS;
	current->symbols[1]=RUNE_COMUNICATUM;

	// Freeze time Level 10
	current=&spellicons[SPELL_FREEZE_TIME];	
	current->name = getLocalised("system_spell_name_freeze_time");
	current->description = getLocalised("system_spell_description_freeze_time");
	current->level=10;
	current->spellid=SPELL_FREEZE_TIME;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_freeze_time.bmp");
	current->symbols[0] = RUNE_RHAA;
	current->symbols[1]=RUNE_TEMPUS;

	// Mass incinerate Level 10
	current=&spellicons[SPELL_MASS_INCINERATE];	
	current->name = getLocalised("system_spell_name_mass_incinerate");
	current->description = getLocalised("system_spell_description_mass_incinerate");
	current->level=10;
	current->spellid=SPELL_MASS_INCINERATE;
	current->bDuration = false;
	current->bAudibleAtStart = true;
	current->tc=TextureContainer::LoadUI("Graph\\Interface\\Icons\\Spell_mass_incinerate.bmp");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_AAM;
	current->symbols[2]=RUNE_MEGA;
	current->symbols[3]=RUNE_YOK;

	Flying_Eye=			TextureContainer::LoadUI("Graph\\particles\\Flying_Eye_Fx.bmp");
	specular=			TextureContainer::LoadUI("Graph\\particles\\specular.bmp");
	enviro=				TextureContainer::LoadUI("Graph\\particles\\enviro.bmp");
	sphere_particle=	TextureContainer::LoadUI("Graph\\particles\\sphere.bmp");
	inventory_font=		TextureContainer::LoadUI("Graph\\interface\\font\\font10x10_inventory.bmp");
	npc_fight=			TextureContainer::LoadUI("Graph\\interface\\icons\\follower_attack.bmp");
	npc_follow=			TextureContainer::LoadUI("Graph\\interface\\icons\\follower_follow.bmp");
	npc_stop=			TextureContainer::LoadUI("Graph\\interface\\icons\\follower_stop.bmp");
	flaretc.lumignon=	TextureContainer::LoadUI("Graph\\Particles\\lumignon.bmp");
	flaretc.lumignon2=	TextureContainer::LoadUI("Graph\\Particles\\lumignon2.bmp");
	flaretc.plasm=		TextureContainer::LoadUI("Graph\\Particles\\plasm.bmp");
	tflare=				TextureContainer::LoadUI("Graph\\Particles\\flare.bmp");
	ombrignon=			TextureContainer::LoadUI("Graph\\particles\\ombrignon.bmp");
	teleportae=			TextureContainer::LoadUI("Graph\\particles\\teleportae.bmp");
	TC_fire=			TextureContainer::LoadUI("Graph\\particles\\fire.bmp");
	TC_fire2=			TextureContainer::LoadUI("Graph\\particles\\fire2.bmp");
	TC_smoke=			TextureContainer::LoadUI("Graph\\particles\\smoke.bmp");
	//zbtex=				TextureContainer::LoadUI("Graph\\particles\\zbtex.bmp");
	TextureContainer::LoadUI("Graph\\particles\\missile.bmp");
	Z_map=				TextureContainer::LoadUI("Graph\\interface\\misc\\z-map.bmp");
	Boom=				TextureContainer::LoadUI("Graph\\Particles\\boom.bmp");
	lightsource_tc=		TextureContainer::LoadUI("Graph\\Particles\\light.bmp");
	stealth_gauge_tc=	TextureContainer::LoadUI("Graph\\interface\\Icons\\Stealth_Gauge.bmp");
	arx_logo_tc=		TextureContainer::LoadUI("Graph\\interface\\Icons\\Arx_logo_32.bmp");
	iconequip[0]=		TextureContainer::LoadUI("Graph\\interface\\Icons\\equipment_sword.bmp");
	iconequip[1]=		TextureContainer::LoadUI("Graph\\interface\\Icons\\equipment_shield.bmp");
	iconequip[2]=		TextureContainer::LoadUI("Graph\\interface\\Icons\\equipment_helm.bmp");
	iconequip[3]=		TextureContainer::LoadUI("Graph\\interface\\Icons\\equipment_chest.bmp");
	iconequip[4]=		TextureContainer::LoadUI("Graph\\interface\\Icons\\equipment_leggings.bmp");
	mecanism_tc=		TextureContainer::LoadUI("Graph\\interface\\Cursors\\Mecanism.bmp");
	arrow_left_tc=		TextureContainer::LoadUI("Graph\\interface\\Icons\\Arrow_left.bmp");

	for (i=0;i<MAX_EXPLO;i++)
	{
		char temp[256];
		sprintf(temp,"Graph\\Particles\\fireb_%02ld.bmp",i+1);
		explo[i]= TextureContainer::LoadUI(temp);
	}

	blood_splat=TextureContainer::LoadUI("Graph\\Particles\\new_blood2.bmp");

	EERIE_DRAW_SetTextureZMAP(Z_map);
	EERIE_DRAW_sphere_particle=sphere_particle;
	EERIE_DRAW_square_particle=TextureContainer::LoadUI("Graph\\particles\\square.bmp");

	TextureContainer::LoadUI("Graph\\Particles\\fire_hit.bmp");
	TextureContainer::LoadUI("Graph\\Particles\\light.bmp");
	TextureContainer::LoadUI("Graph\\Particles\\blood01.bmp");
	TextureContainer::LoadUI("Graph\\Particles\\cross.bmp");

	//INTERFACE LOADING
	TextureContainer::LoadUI("Graph\\interface\\bars\\Empty_gauge_Red.bmp");
	TextureContainer::LoadUI("Graph\\interface\\bars\\Empty_gauge_Blue.bmp");
	TextureContainer::LoadUI("Graph\\interface\\bars\\Filled_gauge_Blue.bmp");
	TextureContainer::LoadUI("Graph\\interface\\bars\\Filled_gauge_Red.bmp");
	TextureContainer::LoadUI("Graph\\Interface\\Icons\\Book.bmp");
	TextureContainer::LoadUI("Graph\\Interface\\Icons\\Backpack.bmp");
	TextureContainer::LoadUI("Graph\\Interface\\Icons\\Lvl_Up.bmp");
	TextureContainer::LoadUI("Graph\\Interface\\Icons\\Steal.bmp");
	TextureContainer::LoadUI("Graph\\Interface\\Icons\\cant_steal_item.bmp");
	TextureContainer::LoadUI("Graph\\Interface\\Inventory\\hero_inventory.bmp");
	TextureContainer::LoadUI("Graph\\Interface\\Inventory\\scroll_up.bmp");
	TextureContainer::LoadUI("Graph\\Interface\\Inventory\\scroll_down.bmp");
	TextureContainer::LoadUI("Graph\\Interface\\Inventory\\Hero_inventory_link.bmp");
	TextureContainer::LoadUI("Graph\\Interface\\Inventory\\ingame_inventory.bmp");
	//TextureContainer::LoadUI("Graph\\Interface\\Inventory\\ingame_sub_inv.bmp");
	TextureContainer::LoadUI("Graph\\Interface\\Inventory\\Gold.bmp");

	TextureContainer::LoadUI("Graph\\Interface\\Inventory\\inv_pick.bmp");
	TextureContainer::LoadUI("Graph\\Interface\\Inventory\\inv_close.bmp");

	// MENU2
	TextureContainer::LoadUI("graph\\interface\\cursors\\cursor00.bmp");
	TextureContainer::LoadUI("graph\\interface\\cursors\\cursor01.bmp");
	TextureContainer::LoadUI("graph\\interface\\cursors\\cursor02.bmp");
	TextureContainer::LoadUI("graph\\interface\\cursors\\cursor03.bmp");
	TextureContainer::LoadUI("graph\\interface\\cursors\\cursor04.bmp");
	TextureContainer::LoadUI("graph\\interface\\cursors\\cursor05.bmp");
	TextureContainer::LoadUI("graph\\interface\\cursors\\cursor06.bmp");
	TextureContainer::LoadUI("graph\\interface\\cursors\\cursor07.bmp");
	TextureContainer::LoadUI("graph\\interface\\cursors\\cruz.bmp");
	TextureContainer::LoadUI("Graph\\Interface\\menus\\menu_main_background.bmp");
	TextureContainer::LoadUI("Graph\\interface\\menus\\menu_console_background.bmp");
	TextureContainer::LoadUI("Graph\\interface\\menus\\menu_console_background_border.bmp");

	//CURSORS LOADING
	TextureContainer::LoadUI("Graph\\Interface\\cursors\\cursor.bmp");
	TextureContainer::LoadUI("Graph\\Interface\\cursors\\magic.bmp");
	TextureContainer::LoadUI("Graph\\Interface\\cursors\\interaction_on.bmp");
	TextureContainer::LoadUI("Graph\\Interface\\cursors\\interaction_off.bmp");
	TextureContainer::LoadUI("Graph\\Interface\\cursors\\target_on.bmp");
	TextureContainer::LoadUI("Graph\\Interface\\cursors\\target_off.bmp");
	TextureContainer::LoadUI("Graph\\Interface\\cursors\\drop.bmp");
	TextureContainer::LoadUI("Graph\\Interface\\cursors\\throw.bmp");

	for (i=0;i<8;i++)
	{
		char temp[256];
		sprintf(temp,"Graph\\Interface\\cursors\\cursor%02ld.bmp",i);
		scursor[i]=TextureContainer::LoadUI(temp);
	}

	pTCCrossHair = TextureContainer::LoadUI("graph\\interface\\cursors\\cruz.bmp");

	TextureContainer::LoadUI("Graph\\Interface\\bars\\aim_empty.bmp");
	TextureContainer::LoadUI("Graph\\Interface\\bars\\aim_maxi.bmp");
	TextureContainer::LoadUI("Graph\\Interface\\bars\\flash_gauge.bmp");
}

void ClearSysTextures() {
	for(size_t i = 0; i < SPELL_COUNT; i++) {
		if(!spellicons[i].name.empty())
			//free(spellicons[i].name);
			spellicons[i].name.clear();

		if(!spellicons[i].description.empty())
			//free(spellicons[i].description);
			spellicons[i].description.clear();
	}
}

//*************************************************************************************
// OneTimeSceneInit()
//  Called Once during initial app startup
//*************************************************************************************
HRESULT DANAE::OneTimeSceneInit()
{
	return S_OK;
}
Vec3f ePos;
extern EERIE_CAMERA * ACTIVECAM;
void LaunchWaitingCine()
{
	LogDebug << "LaunchWaitingCine " << CINE_PRELOAD;

	if (ACTIVECAM)
	{
		ePos.x = ACTIVECAM->pos.x;
		ePos.y = ACTIVECAM->pos.y;
		ePos.z = ACTIVECAM->pos.z;
	}

	DANAE_KillCinematic();
	
	const char RESOURCE_GRAPH_INTERFACE_ILLUSTRATIONS[] = "Graph\\interface\\illustrations\\";
	
	char temp2[256];
	strcpy(temp2,RESOURCE_GRAPH_INTERFACE_ILLUSTRATIONS);
	strcat(temp2,WILL_LAUNCH_CINE);

	if (PAK_FileExist(temp2))
	{
		ControlCinematique->OneTimeSceneReInit();

		if (LoadProject(ControlCinematique,RESOURCE_GRAPH_INTERFACE_ILLUSTRATIONS,WILL_LAUNCH_CINE))
		{				

			if (CINE_PRELOAD) {
				LogDebug << "only preloaded cinematic";
				PLAY_LOADED_CINEMATIC=0;
			} else {
				LogDebug << "starting cinematic";
				PLAY_LOADED_CINEMATIC=1;
				ARX_TIME_Pause();
			}

			strcpy(LAST_LAUNCHED_CINE,WILL_LAUNCH_CINE);
		}

	}

	WILL_LAUNCH_CINE[0]=0;
}
static void PlayerLaunchArrow_Test(float aimratio,float poisonous,Vec3f * pos,Anglef * angle)
{
	Vec3f position;
	Vec3f vect;
	Vec3f dvect;
	EERIEMATRIX mat;
	EERIE_QUAT quat;
	float anglea;
	float angleb;
	float velocity;

	position.x=pos->x;
	position.y=pos->y;
	position.z=pos->z;
	anglea=radians(angle->a);
	angleb=radians(angle->b);
	vect.x=-EEsin(angleb)*EEcos(anglea);
	vect.y= EEsin(anglea);
	vect.z= EEcos(angleb)*EEcos(anglea);
	Vec3f upvect(0,0,-1);
	VRotateX(&upvect,anglea);
	VRotateY(&upvect,angleb);
	upvect = Vec3f(0,-1,0);
	VRotateX(&upvect,anglea);
	VRotateY(&upvect,angleb);
	MatrixSetByVectors(&mat,&dvect,&upvect);
	QuatFromMatrix(quat,mat);
	velocity=(aimratio+0.3f);

	if (velocity<0.9f) velocity=0.9f;

	Vec3f v1,v2;
	Vec3f vv(0,0,1);
	float aa=angle->a;
	float ab=90-angle->b;
	Vector_RotateZ(&v1,&vv,aa);
	VRotateY(&v1,ab);
	vv = Vec3f(0,-1,0);
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
		(float)(player.Full_Skill_Projectile + player.Full_Attribute_Dexterity )*( 1.0f / 50 ));

	ARX_THROWN_OBJECT_Throw(
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
	Vec3f position;
	Vec3f vect;
	Vec3f dvect;
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
		position = inter.iobj[0]->obj->vertexlist3[inter.iobj[0]->obj->fastaccess.left_attach].v;
	}

	anglea=radians(player.angle.a);
	angleb=radians(player.angle.b);
	vect.x=-EEsin(angleb)*EEcos(anglea);
	vect.y= EEsin(anglea);
	vect.z= EEcos(angleb)*EEcos(anglea);

	Vec3f upvect(0,0,-1);
	VRotateX(&upvect,anglea);
	VRotateY(&upvect,angleb);

	upvect = Vec3f(0,-1,0);
	VRotateX(&upvect,anglea);
	VRotateY(&upvect,angleb);
	MatrixSetByVectors(&mat,&dvect,&upvect);
	QuatFromMatrix(quat,mat);

	velocity=(aimratio+0.3f);

	if (velocity<0.9f) velocity=0.9f;

	Vec3f v1,v2;
	Vec3f vv(0,0,1);
	float aa=player.angle.a;
	float ab=90-player.angle.b;
	Vector_RotateZ(&v1,&vv,aa);
	VRotateY(&v1,ab);
	vv = Vec3f(0,-1,0);
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
		(float)(player.Full_Skill_Projectile + player.Full_Attribute_Dexterity )*( 1.0f / 50 ));

	ARX_THROWN_OBJECT_Throw(
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
		Anglef angle;
		Vec3f pos;
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

//*************************************************************************************
// FrameMove()
//  Called once per frame.
//*************************************************************************************
HRESULT DANAE::FrameMove()
{
	
#ifdef BUILD_EDITOR
	// To disable for demo
	if (	!FINAL_COMMERCIAL_DEMO
		&& !FINAL_COMMERCIAL_GAME
		)
	{
		if (this->kbd.inkey[INKEY_F4])
		{
			this->kbd.inkey[INKEY_F4]=0;
			ARX_TIME_Pause();
			DialogBox( (HINSTANCE)GetWindowLongPtr( this->m_hWnd, GWLP_HINSTANCE ),
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
				Pause(true);
				DialogBox( (HINSTANCE)GetWindowLongPtr( this->m_hWnd, GWLP_HINSTANCE ),
					MAKEINTRESOURCE(IDD_OPTIONS), this->m_hWnd, OptionsProc );
				EERIE_LIGHT_ChangeLighting();
				Pause(false);
				ARX_TIME_UnPause();
			}
			else if (this->kbd.inkey[INKEY_F3])
			{
				this->kbd.inkey[INKEY_F3]=0;
				ARX_TIME_Pause();
				Pause(true);
				DialogBox( (HINSTANCE)GetWindowLongPtr( this->m_hWnd, GWLP_HINSTANCE ),
							MAKEINTRESOURCE(IDD_OPTIONS2), this->m_hWnd, OptionsProc_2 );
				Pause(false);
				ARX_TIME_UnPause();
			}
			else if (this->kbd.inkey[INKEY_O])
			{
				this->kbd.inkey[INKEY_O]=0;
			}
		}
	}
#endif // BUILD_EDITOR

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

#ifdef BUILD_EDITOR
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
#endif
	{
#ifdef BUILD_EDITOR
		EDITMODE=0;
#endif
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
		delete ssol;
		ssol=NULL;
		ssol_count=0;
	}

	if(slight)
	{
		delete slight;
		slight=NULL;
		slight_count=0;
	}

	if(srune)
	{
		delete srune;
		srune=NULL;
		srune_count=0;
	}

	if(smotte)
	{
		delete smotte;
		smotte=NULL;
		smotte_count=0;
	}

	if(stite)
	{
		delete stite;
		stite=NULL;
		stite_count=0;
	}

	if(smissile)
	{
		delete smissile;
		smissile=NULL;
		smissile_count=0;
	}

	if(spapi)
	{
		delete spapi;
		spapi=NULL;
		spapi_count=0;
	}

	if(sfirewave)
	{
		delete sfirewave;
		sfirewave=NULL;
		sfirewave_count=0;
	}

	if(svoodoo)
	{
		delete svoodoo;
		svoodoo=NULL;
		svoodoo_count=0;
	}
}

//-----------------------------------------------------------------------------

void ReleaseDanaeBeforeRun()
{
	if(necklace.lacet)
	{
		delete necklace.lacet;
		necklace.lacet=NULL;
	}

	for (long i=0; i<20; i++)
	{
		if(necklace.runes[i]) {
			delete necklace.runes[i];
			necklace.runes[i] = NULL;
		}

		if (necklace.pTexTab[i])
		{

			necklace.pTexTab[i] = NULL;
		}
	}

	if(eyeballobj) {
		delete eyeballobj;
		eyeballobj = NULL;
	}

	if(cabal) {
		delete cabal;
		cabal = NULL;
	}

	if(nodeobj) {
		delete nodeobj;
		nodeobj = NULL;
	}

	if(fogobj) {
		delete fogobj;
		fogobj = NULL;
	}

	if(cameraobj) {
		delete cameraobj;
		cameraobj = NULL;
	}

	if(markerobj) {
		delete markerobj;
		markerobj = NULL;
	}

	if(arrowobj) {
		delete arrowobj;
		arrowobj = NULL;
	}

	for(size_t i = 0; i < MAX_GOLD_COINS_VISUALS; i++) {
		if(GoldCoinsObj[i]) {
			delete GoldCoinsObj[i];
			GoldCoinsObj[i] = NULL;
		}
	}

}

HRESULT DANAE::BeforeRun()
{

	LogDebug << "Before Run...";

	ControlCinematique = new Cinematic(danaeApp.m_pFramework->m_dwRenderWidth, danaeApp.m_pFramework->m_dwRenderHeight);
	LogDebug << "Initializing ControlCinematique " << danaeApp.m_pFramework->m_dwRenderWidth << "x" << danaeApp.m_pFramework->m_dwRenderHeight;
	memset(&necklace,0,sizeof(ARX_NECKLACE));
	long old=GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE;
	GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE=-1;
	necklace.lacet =                   loadObject("Graph\\Interface\\book\\runes\\lacet.teo");
	necklace.runes[RUNE_AAM] =         loadObject("Graph\\Interface\\book\\runes\\runes_aam.teo");
	necklace.runes[RUNE_CETRIUS] =     loadObject("Graph\\Interface\\book\\runes\\runes_citrius.teo");
	necklace.runes[RUNE_COMUNICATUM] = loadObject("Graph\\Interface\\book\\runes\\runes_comunicatum.teo");
	necklace.runes[RUNE_COSUM] =       loadObject("Graph\\Interface\\book\\runes\\runes_cosum.teo");
	necklace.runes[RUNE_FOLGORA] =     loadObject("Graph\\Interface\\book\\runes\\runes_folgora.teo");
	necklace.runes[RUNE_FRIDD] =       loadObject("Graph\\Interface\\book\\runes\\runes_fridd.teo");
	necklace.runes[RUNE_KAOM] =        loadObject("Graph\\Interface\\book\\runes\\runes_kaom.teo");
	necklace.runes[RUNE_MEGA] =        loadObject("Graph\\Interface\\book\\runes\\runes_mega.teo");
	necklace.runes[RUNE_MORTE] =       loadObject("Graph\\Interface\\book\\runes\\runes_morte.teo");
	necklace.runes[RUNE_MOVIS] =       loadObject("Graph\\Interface\\book\\runes\\runes_movis.teo");
	necklace.runes[RUNE_NHI] =         loadObject("Graph\\Interface\\book\\runes\\runes_nhi.teo");
	necklace.runes[RUNE_RHAA] =        loadObject("Graph\\Interface\\book\\runes\\runes_rhaa.teo");
	necklace.runes[RUNE_SPACIUM] =     loadObject("Graph\\Interface\\book\\runes\\runes_spacium.teo");
	necklace.runes[RUNE_STREGUM] =     loadObject("Graph\\Interface\\book\\runes\\runes_stregum.teo");
	necklace.runes[RUNE_TAAR] =        loadObject("Graph\\Interface\\book\\runes\\runes_taar.teo");
	necklace.runes[RUNE_TEMPUS] =      loadObject("Graph\\Interface\\book\\runes\\runes_tempus.teo");
	necklace.runes[RUNE_TERA] =        loadObject("Graph\\Interface\\book\\runes\\runes_tera.teo");
	necklace.runes[RUNE_VISTA] =       loadObject("Graph\\Interface\\book\\runes\\runes_vista.teo");
	necklace.runes[RUNE_VITAE] =       loadObject("Graph\\Interface\\book\\runes\\runes_vitae.teo");
	necklace.runes[RUNE_YOK] =         loadObject("Graph\\Interface\\book\\runes\\runes_yok.teo");

	necklace.pTexTab[RUNE_AAM]			= TextureContainer::LoadUI("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_Aam[icon].BMP");
	necklace.pTexTab[RUNE_CETRIUS]		= TextureContainer::LoadUI("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_cetrius[icon].BMP");
	necklace.pTexTab[RUNE_COMUNICATUM]	= TextureContainer::LoadUI("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_comunicatum[icon].BMP");
	necklace.pTexTab[RUNE_COSUM]		= TextureContainer::LoadUI("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_cosum[icon].BMP");
	necklace.pTexTab[RUNE_FOLGORA]		= TextureContainer::LoadUI("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_folgora[icon].BMP");
	necklace.pTexTab[RUNE_FRIDD]		= TextureContainer::LoadUI("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_fridd[icon].BMP");
	necklace.pTexTab[RUNE_KAOM]			= TextureContainer::LoadUI("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_kaom[icon].BMP");
	necklace.pTexTab[RUNE_MEGA]			= TextureContainer::LoadUI("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_mega[icon].BMP");
	necklace.pTexTab[RUNE_MORTE]		= TextureContainer::LoadUI("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_morte[icon].BMP");
	necklace.pTexTab[RUNE_MOVIS]		= TextureContainer::LoadUI("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_movis[icon].BMP");
	necklace.pTexTab[RUNE_NHI]			= TextureContainer::LoadUI("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_nhi[icon].BMP");
	necklace.pTexTab[RUNE_RHAA]			= TextureContainer::LoadUI("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_rhaa[icon].BMP");
	necklace.pTexTab[RUNE_SPACIUM]		= TextureContainer::LoadUI("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_spacium[icon].BMP");
	necklace.pTexTab[RUNE_STREGUM]		= TextureContainer::LoadUI("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_stregum[icon].BMP");
	necklace.pTexTab[RUNE_TAAR]			= TextureContainer::LoadUI("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_taar[icon].BMP");
	necklace.pTexTab[RUNE_TEMPUS]		= TextureContainer::LoadUI("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_tempus[icon].BMP");
	necklace.pTexTab[RUNE_TERA]			= TextureContainer::LoadUI("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_tera[icon].BMP");
	necklace.pTexTab[RUNE_VISTA]		= TextureContainer::LoadUI("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_vista[icon].BMP");
	necklace.pTexTab[RUNE_VITAE]		= TextureContainer::LoadUI("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_vitae[icon].BMP");
	necklace.pTexTab[RUNE_YOK]			= TextureContainer::LoadUI("\\Graph\\Obj3D\\Interactive\\Items\\Magic\\Rune_aam\\rune_yok[icon].BMP");

	for(size_t i = 0; i<RUNE_COUNT-1; i++) { // TODO why -1?
		if(necklace.pTexTab[i]) {
			necklace.pTexTab[i]->CreateHalo();
		}
	}

	// TODO the .teo files are not shipped with the game, only the textures are
	// TODO this is the only place where _LoadTheObj is used
	EERIE_3DOBJ * _fogobj;
	_fogobj=		_LoadTheObj("Editor\\Obj3D\\fog_generator.teo","node_TEO MAPS\\");
	ARX_FOGS_Set_Object(_fogobj);
	eyeballobj = _LoadTheObj("Editor\\Obj3D\\eyeball.teo","eyeball_TEO MAPS\\");
	cabal = _LoadTheObj("Editor\\Obj3D\\cabal.teo","cabal_TEO MAPS\\");
	nodeobj = _LoadTheObj("Editor\\Obj3D\\node.teo","node_TEO MAPS\\");
	
	cameraobj = loadObject("Graph\\Obj3D\\Interactive\\System\\Camera\\Camera.teo");
	markerobj = loadObject("Graph\\Obj3D\\Interactive\\System\\Marker\\Marker.teo");
	arrowobj = loadObject("Graph\\Obj3D\\Interactive\\Items\\Weapons\\arrow\\arrow.teo");

	for(size_t i = 0; i < MAX_GOLD_COINS_VISUALS; i++) {
		char temp[256];

		if (i==0)
			strcpy(temp,	"Graph\\Obj3D\\Interactive\\Items\\Jewelry\\Gold_coin\\Gold_coin.teo");
		else
			sprintf(temp,	"Graph\\Obj3D\\Interactive\\Items\\Jewelry\\Gold_coin\\Gold_coin" PRINT_SIZE_T ".teo",i+1);

		GoldCoinsObj[i] = loadObject(temp);

		if (i==0)
			strcpy(temp,	"Graph\\Obj3D\\Interactive\\Items\\Jewelry\\Gold_coin\\Gold_coin[icon].bmp");
		else
			sprintf(temp,	"Graph\\Obj3D\\Interactive\\Items\\Jewelry\\Gold_coin\\Gold_coin" PRINT_SIZE_T "[icon].bmp",i+1);

		GoldCoinsTC[i] =	TextureContainer::LoadUI(temp);
	}

	Movable=				TextureContainer::LoadUI("Graph\\Interface\\Cursors\\wrong.bmp");
	ChangeLevel=			TextureContainer::LoadUI("Graph\\Interface\\Icons\\change_lvl.bmp");

	ARX_PLAYER_LoadHeroAnimsAndMesh();

	GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE=old;

#ifdef BUILD_EDITOR
	// Need to create Map
	if (iCreateMap)
		DANAE_Manage_CreateMap();
#endif

	danaeApp.GetZBufferMax();

	return S_OK;
}

//*************************************************************************************

void FirstTimeThings() {
	
	eyeball.exist=0;
	WILLADDSPEECHTIME=0;
	WILLADDSPEECH.clear();

	if (!LOADEDD)
	{
		RemoveAllBackgroundActions();
	}

	_NB_++;

	for(size_t i = 0; i < MAX_DYNLIGHTS; i++) {
		DynLight[i].exist = 0;
	}

	LastFrameTime=FrameTime;
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

long NO_GMOD_RESET=0;

void FirstFrameProc() {
	
	if (pParticleManager == NULL)
	{
		pParticleManager = new ParticleManager();
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
	
	FirstTimeThings();

	if (!LOAD_N_DONT_ERASE)
	{

		CleanInventory();
		ARX_SCRIPT_Timer_ClearAll();
		UnlinkAllLinkedObjects();
		ARX_SCRIPT_ResetAll(0);
	}

	SecondaryInventory=NULL;
	TSecondaryInventory=NULL;
	ARX_FOGS_Render();

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
}
Vec3f LastValidPlayerPos;
Vec3f	WILL_RESTORE_PLAYER_POSITION;
long WILL_RESTORE_PLAYER_POSITION_FLAG=0;
extern long FLAG_ALLOW_CLOTHES;

//*************************************************************************************

long FirstFrameHandling()
{	
	LogDebug << "FirstFrameHandling";
	Vec3f trans;
	FirstFrame=-1;

	ARX_PARTICLES_FirstInit();
	ARX_SPELLS_Init_Rects();
	ARX_FOGS_TimeReset();

	PROGRESS_BAR_COUNT+=2.f;
	LoadLevelScreen();
	
	FirstFrameProc();
	
	if (FASTmse)
	{
		FASTmse=0;

		if(LOADEDD) {
			trans = Mscenepos;
			player.pos = loddpos + trans;
		} else {
			player.pos.y +=PLAYER_BASE_HEIGHT;
		}

		PROGRESS_BAR_COUNT+=4.f;
		LoadLevelScreen();
	}
#ifdef BUILD_EDIT_LOADSAVE
	else if(mse) {
		Mscenepos.x=-mse->cub.xmin-(mse->cub.xmax-mse->cub.xmin)*( 1.0f / 2 )+((float)ACTIVEBKG->Xsize*(float)ACTIVEBKG->Xdiv)*( 1.0f / 2 );
		Mscenepos.z=-mse->cub.zmin-(mse->cub.zmax-mse->cub.zmin)*( 1.0f / 2 )+((float)ACTIVEBKG->Zsize*(float)ACTIVEBKG->Zdiv)*( 1.0f / 2 );
		float t1=(float)(long)(mse->point0.x/BKG_SIZX);
		float t2=(float)(long)(mse->point0.z/BKG_SIZZ);
		t1=mse->point0.x-t1*BKG_SIZX;
		t2=mse->point0.z-t2*BKG_SIZZ;
		Mscenepos.x=(float)((long)(Mscenepos.x/BKG_SIZX))*BKG_SIZX+(float)BKG_SIZX*( 1.0f / 2 );
		Mscenepos.z=(float)((long)(Mscenepos.z/BKG_SIZZ))*BKG_SIZZ+(float)BKG_SIZZ*( 1.0f / 2 );
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

		trans = mse->pos;

		ReleaseMultiScene(mse);
		mse=NULL;

		if(!NO_PLAYER_POSITION_RESET) {
			if(LOADEDD) {
				player.pos = loddpos + trans;
			} else {
				player.pos.y += PLAYER_BASE_HEIGHT;
			}
		}

		NO_PLAYER_POSITION_RESET=0;

		PROGRESS_BAR_COUNT+=1.f;
		LoadLevelScreen();
	}
#endif // BUILD_EDIT_LOADSAVE
	else {
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
	mapcam.pos.x = lastteleport.x=subj.pos.x=moveto.x=player.pos.x;
				lastteleport.y=subj.pos.y=moveto.y=player.pos.y;
	mapcam.pos.z = lastteleport.z=subj.pos.z=moveto.z=player.pos.z;
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

	PROGRESS_BAR_COUNT+=1.f;
	LoadLevelScreen();

	FirstFrame=0;
	FRAME_COUNT=0;
	PrepareIOTreatZone(1);
	CURRENTLEVEL=GetLevelNumByName(LastLoadedScene);
	
#ifdef BUILD_EDITOR
	iCreateMap=0;
	if ((CURRENTLEVEL>=0) && !(NOBUILDMAP) && GAME_EDITOR)
	{
		if (NeedMapCreation())	
			iCreateMap=1;
		else
			iCreateMap=0;
	}
#endif

	if (!NO_TIME_INIT)
		ARX_TIME_Init();

	LastFrameTime=FrameTime;

 PROGRESS_BAR_COUNT+=1.f;
 LoadLevelScreen();
		
	if (ITC.Get("presentation")) 
	{
        delete ITC.Get("presentation");
		ITC.Set("presentation", NULL);
	}

	if (DONT_WANT_PLAYER_INZONE)
	{
		player.inzone=NULL;
		DONT_WANT_PLAYER_INZONE=0;
	}

#ifdef BUILD_EDITOR
	if(MOULINEX) {
		LaunchMoulinex();
	}
#endif

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
		player.pos = WILL_RESTORE_PLAYER_POSITION;
		inter.iobj[0]->pos = WILL_RESTORE_PLAYER_POSITION;
		inter.iobj[0]->pos.y+=170.f;
		INTERACTIVE_OBJ * io=inter.iobj[0];

		for (size_t i=0;i<io->obj->vertexlist.size();i++)
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
	LoadLevelScreen(-2);
	
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

long Player_Arrow_Count() {
	
	long count = 0;
	
	if(player.bag) {
		for(int iNbBag = 0; iNbBag < player.bag; iNbBag++) {
			for(size_t j = 0; j < INVENTORY_Y; j++) {
				for(size_t i = 0; i < INVENTORY_X; i++) {
					INTERACTIVE_OBJ * io = inventory[iNbBag][i][j].io;
					if(io) {
						if(GetName(io->filename) == "Arrows") {
							if(io->durability >= 1.f) {
								ARX_CHECK_LONG(io->durability);
								count += static_cast<long>(io->durability);
							}
						}
					}
				}
			}
		}
	}
	
	return count;
}

INTERACTIVE_OBJ * Player_Arrow_Count_Decrease() {
	
	INTERACTIVE_OBJ * io = NULL;
	
	if(player.bag) {
		for(int iNbBag = 0; iNbBag < player.bag; iNbBag++) {
			for(size_t j = 0; j < INVENTORY_Y; j++) {
				for(size_t i = 0; i < INVENTORY_X;i++) {
					INTERACTIVE_OBJ * ioo = inventory[iNbBag][i][j].io;
					if(ioo) {
						if(GetName(ioo->filename) == "Arrows") {
							if(ioo->durability >= 1.f) {
								if(!io) {
									io = ioo;
								} else if(io->durability > ioo->durability) {
									io = ioo;
								}
							}
						}
					}
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
							ARX_SPEECH_AddSpeech(io,str,ANIM_TALK_NEUTRAL,ARX_SPEECH_FLAG_NOTEXT);
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
							ARX_SPEECH_AddSpeech(io,str,ANIM_TALK_NEUTRAL,ARX_SPEECH_FLAG_NOTEXT);
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
							ARX_SPEECH_AddSpeech(io,str,ANIM_TALK_NEUTRAL,ARX_SPEECH_FLAG_NOTEXT);
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
							ARX_SPEECH_AddSpeech(io,str,ANIM_TALK_NEUTRAL,ARX_SPEECH_FLAG_NOTEXT);
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

			if(useanim->cur_anim == alist[ANIM_MISSILE_STRIKE_CYCLE]) {
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
void ManageFade()
{
	float tim=((float)ARX_TIME_Get()-(float)FADESTART);

	if (tim<=0.f) return;

	float Visibility=tim/(float)FADEDURATION;

	if (FADEDIR>0) Visibility=1.f-Visibility;

	if (Visibility>1.f) Visibility=1.f;

	if(Visibility < 0.f) {
		FADEDIR = 0;
		return;
	}

	LAST_FADEVALUE=Visibility;
	GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	
	EERIEDrawBitmap(0.f,0.f, (float)DANAESIZX, (float)DANAESIZY, 0.0001f, NULL, Color::gray(Visibility));

	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	float col=Visibility;
	EERIEDrawBitmap(0.f,0.f,(float)DANAESIZX,(float)DANAESIZY,0.0001f,
	                NULL, Color(col * FADECOLOR.r, col * FADECOLOR.g, col * FADECOLOR.b));
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
}

extern long cur_mr;
TextureContainer * Mr_tc=NULL;

void CheckMr()
{
	if (cur_mr==3)
	{
		if (GDevice && Mr_tc)
		{
			EERIEDrawBitmap(DANAESIZX-(128.f*Xratio), 0.f, (float)128*Xratio, (float)128*Yratio,0.0001f,
			                Mr_tc, Color::gray(0.5f + PULSATE * (1.0f/10)));
		}
		else
		{
			Mr_tc=TextureContainer::LoadUI("graph\\particles\\(Fx)_Mr.bmp");
		}
	}
}
void DrawImproveVisionInterface()
{
	if(ombrignon)
	{
		float mod = 0.6f + PULSATE * 0.35f;
		EERIEDrawBitmap(0.f,0.f,(float)DANAESIZX,(float)DANAESIZY,0.0001f, ombrignon, Color3f((0.5f+PULSATE*( 1.0f / 10 ))*mod,0.f,0.f).to<u8>());
	}
}

float MagicSightFader=0.f;
void DrawMagicSightInterface()
{
	if (eyeball.exist==1) return;

	if (Flying_Eye)
	{
		GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);										
		float col=(0.75f+PULSATE*( 1.0f / 20 ));

		if (col>1.f) col=1.f;

		if (eyeball.exist<0)
		{
			col=(float)(-eyeball.exist)*( 1.0f / 100 );
		}
		else if (eyeball.exist>2)
		{
			col = 1.f - eyeball.size.x;
		}

		EERIEDrawBitmap(0.f, 0.f, (float)DANAESIZX, (float)DANAESIZY, 0.0001f, Flying_Eye, Color::gray(col));

		if (MagicSightFader>0.f)
		{
			col=MagicSightFader;
			EERIEDrawBitmap(0.f, 0.f, (float)DANAESIZX, (float)DANAESIZY, 0.0001f, NULL, Color::gray(col));
			MagicSightFader-=Original_framedelay*( 1.0f / 400 );

			if (MagicSightFader<0.f)
				MagicSightFader=0.f;
		}

		GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	}
}

//*************************************************************************************

void RenderAllNodes()
{
	Anglef angle(Anglef::ZERO);
	float xx,yy;

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);

	for (long i=0;i<nodes.nbmax;i++)
	{
		if (nodes.nodes[i].exist)
		{

			DrawEERIEInter(nodeobj,
				&angle,&nodes.nodes[i].pos,NULL);
			nodes.nodes[i].bboxmin.x=(short)BBOXMIN.x;
			nodes.nodes[i].bboxmin.y=(short)BBOXMIN.y;
			nodes.nodes[i].bboxmax.x=(short)BBOXMAX.x;
			nodes.nodes[i].bboxmax.y=(short)BBOXMAX.y;

			if ((nodeobj->vertexlist[nodeobj->origin].vert.sz>0.f) && (nodeobj->vertexlist[nodeobj->origin].vert.sz<0.9f))
			{
				xx=nodeobj->vertexlist[nodeobj->origin].vert.sx-40.f;
				yy=nodeobj->vertexlist[nodeobj->origin].vert.sy-40.f;
				ARX_TEXT_Draw(hFontInBook, xx, yy, nodes.nodes[i].UName, Color::yellow); //font
			}

			if(nodes.nodes[i].selected) {
				EERIEDraw2DLine(nodes.nodes[i].bboxmin.x, nodes.nodes[i].bboxmin.y, nodes.nodes[i].bboxmax.x, nodes.nodes[i].bboxmin.y, 0.01f, Color::yellow);
				EERIEDraw2DLine(nodes.nodes[i].bboxmax.x, nodes.nodes[i].bboxmin.y, nodes.nodes[i].bboxmax.x, nodes.nodes[i].bboxmax.y, 0.01f, Color::yellow);
				EERIEDraw2DLine(nodes.nodes[i].bboxmax.x, nodes.nodes[i].bboxmax.y, nodes.nodes[i].bboxmin.x, nodes.nodes[i].bboxmax.y, 0.01f, Color::yellow);
				EERIEDraw2DLine(nodes.nodes[i].bboxmin.x, nodes.nodes[i].bboxmax.y, nodes.nodes[i].bboxmin.x, nodes.nodes[i].bboxmin.y, 0.01f, Color::yellow);
			}

			for(size_t j = 0; j < MAX_LINKS; j++) {
				if(nodes.nodes[i].link[j]!=-1) {
					EERIEDrawTrue3DLine(nodes.nodes[i].pos, nodes.nodes[nodes.nodes[i].link[j]].pos, Color::green);
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
		QuakeFx.frequency*=( 1.0f / 2 );
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
		float periodicity=EEsin((float)FrameTime*QuakeFx.frequency*( 1.0f / 100 ));

		if ((periodicity>0.5f) && (QuakeFx.flags & 1))
			ARX_SOUND_PlaySFX(SND_QUAKE, NULL, 1.0F - 0.5F * QuakeFx.intensity);

		float truepower=periodicity*QuakeFx.intensity*itmod*( 1.0f / 100 );
		float halfpower=truepower*( 1.0f / 2 );
		ACTIVECAM->pos.x+=rnd()*truepower-halfpower;
		ACTIVECAM->pos.y+=rnd()*truepower-halfpower;
		ACTIVECAM->pos.z+=rnd()*truepower-halfpower;
		ACTIVECAM->angle.a+=rnd()*truepower-halfpower;
		ACTIVECAM->angle.g+=rnd()*truepower-halfpower;
		ACTIVECAM->angle.b+=rnd()*truepower-halfpower;
	}
}

#ifdef BUILD_EDITOR
void LaunchMoulinex()
{
	char tx[256];

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
			danaeApp.FinalCleanup();
			exit(0);
		}
		else LogError << ("Moulinex Successfull");

		return;
	}

	long lvl=MOULINEX-1;

	if (PROCESS_ONLY_ONE_LEVEL!=-1)
		lvl=PROCESS_ONLY_ONE_LEVEL;

	LogDebug << "Moulinex Lvl " << lvl;

	if (LASTMOULINEX!=-1)
	{
		char saveto[256];
		long lastlvl;

		if (PROCESS_ONLY_ONE_LEVEL!=-1)
			lastlvl=PROCESS_ONLY_ONE_LEVEL;
		else
			lastlvl=MOULINEX-2;

		GetLevelNameByNum(lastlvl,tx);
		sprintf(saveto,"Graph\\Levels\\Level%s\\level%s.dlf",tx,tx);

		if (FileExist(saveto))
		{
			LightMode oldmode = ModeLight;
			ModeLight=MODE_NORMALS | MODE_RAYLAUNCH | MODE_STATICLIGHT | MODE_DYNAMICLIGHT | MODE_DEPTHCUEING;

			if (TSU_LIGHTING) ModeLight|=MODE_SMOOTH;

			EERIERemovePrecalcLights();
			EERIEPrecalcLights(0,0,999999,999999);
			DanaeSaveLevel(saveto);
			ModeLight = oldmode;
		}

		if (PROCESS_ONLY_ONE_LEVEL!=-1)
		{
			danaeApp.FinalCleanup();
			exit(0);
		}
	}

	if (MOULINEX>=32)
	{
		MOULINEX=0;
		LASTMOULINEX=-1;

		if (KILL_AT_MOULINEX_END)
		{
			danaeApp.FinalCleanup();
			exit(0);
		}
		else LogError << ("Moulinex Successfull");

		return;
	}

	if (PROCESS_ONLY_ONE_LEVEL!=-1)
	{
		lvl=PROCESS_ONLY_ONE_LEVEL;
	}

	{
		char loadfrom[256];

		GetLevelNameByNum(lvl,tx);

		if (strcasecmp(tx,"NONE"))
		{
			sprintf(loadfrom,"Graph\\Levels\\Level%s\\level%s.dlf",tx,tx);

			if (FileExist(loadfrom))
			{
				if (CDP_LIGHTOptions!=NULL) SendMessage(CDP_LIGHTOptions,WM_CLOSE,0,0);

				if (CDP_FogOptions!=NULL) SendMessage(CDP_FogOptions,WM_CLOSE,0,0);

				CDP_LIGHTOptions=NULL;
				CDP_FogOptions=NULL;
				SetEditMode(1);
				DanaeClearLevel();
				DanaeLoadLevel(loadfrom);
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
#endif

void DANAE_StartNewQuest()
{
	player.Interface = INTER_LIFE_MANA | INTER_MINIBACK | INTER_MINIBOOK;
	PROGRESS_BAR_TOTAL = 108;
	OLD_PROGRESS_BAR_COUNT=PROGRESS_BAR_COUNT=0;
	LoadLevelScreen(1);
	char loadfrom[256];
	sprintf(loadfrom,"Graph\\Levels\\Level1\\Level1.dlf");
	DONT_ERASE_PLAYER=1;							
	DanaeClearAll();
	PROGRESS_BAR_COUNT+=2.f;
	LoadLevelScreen();
	DanaeLoadLevel(loadfrom);
	FORBID_SAVE=0;
	FirstFrame=1;
	START_NEW_QUEST=0;
	STARTED_A_GAME=1;
	BLOCK_PLAYER_CONTROLS = 0;
	FADEDURATION=0;
	FADEDIR=0;
	player.Interface = INTER_LIFE_MANA | INTER_MINIBACK | INTER_MINIBOOK;
}

bool DANAE_ManageSplashThings()
{
	GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapClamp);

	if (SPLASH_THINGS_STAGE>10)
	{
		if (EDITMODE || !config.firstRun )
		{
			GInput->update();

			if (GInput->bKeyTouched)
			{
				REFUSE_GAME_RETURN=1;
				FORBID_SAVE=0;
				FirstFrame=1;
				SPLASH_THINGS_STAGE=0;
				INTRO_NOT_LOADED=0;
				ARXmenu.currentmode=AMCM_MAIN;
				ARX_MENU_Launch();
			}

			if (GInput->actionPressed(Keyboard::Key_Escape))
			{
				REFUSE_GAME_RETURN=1;
				SPLASH_THINGS_STAGE = 14;
			}
		}

		if (FAST_SPLASHES)
			SPLASH_THINGS_STAGE=14;

		if (SPLASH_THINGS_STAGE==11)
		{
			
			if (SPLASH_START==0) //firsttime
				SPLASH_START = ARX_TIME_GetUL();

			ARX_INTERFACE_ShowFISHTANK();

			unsigned long tim = ARX_TIME_GetUL();
			float pos=(float)tim-(float)SPLASH_START;

			if (pos>3600)
			{
				SPLASH_START=0;
				SPLASH_THINGS_STAGE++;
			}

			GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);
			return true;
			
		}

		if (SPLASH_THINGS_STAGE==12)
		{
			if (SPLASH_START==0) //firsttime
			{
				SPLASH_START = ARX_TIME_GetUL();
				ARX_SOUND_PlayInterface(SND_PLAYER_HEART_BEAT);
			}

			ARX_INTERFACE_ShowARKANE();
			unsigned long tim = ARX_TIME_GetUL();
			float pos=(float)tim-(float)SPLASH_START;

			if (pos>3600)
			{
				SPLASH_START=0;
				SPLASH_THINGS_STAGE++;
			}

			GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);
			return true;
		}

		if (SPLASH_THINGS_STAGE==13)
		{
			ARX_INTERFACE_KillFISHTANK();
			ARX_INTERFACE_KillARKANE();
			char loadfrom[256];

			REFUSE_GAME_RETURN=1;
			sprintf(loadfrom,"Graph\\Levels\\Level10\\level10.dlf");
			OLD_PROGRESS_BAR_COUNT=PROGRESS_BAR_COUNT=0;
			PROGRESS_BAR_TOTAL = 108;
			LoadLevelScreen(10);	

			DanaeLoadLevel(loadfrom);
			FORBID_SAVE=0;
			FirstFrame=1;
			SPLASH_THINGS_STAGE=0;
			INTRO_NOT_LOADED=0;

			if ( config.firstRun )
				config.firstRun = false;

			GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);
			return true;
			
		}

		if (SPLASH_THINGS_STAGE > 13)
		{
			FORBID_SAVE=0;
			FirstFrame=1;
			SPLASH_THINGS_STAGE=0;
			INTRO_NOT_LOADED=0;

			if ( config.firstRun )
				config.firstRun = false;

			GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);
			return true;
		}
	}

	GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);
	return false;
}

//*************************************************************************************
// Manages Currently playing 2D cinematic
//*************************************************************************************
long DANAE_Manage_Cinematic()
{
	
	float FrameTicks=ARX_TIME_Get( false );

	if (PLAY_LOADED_CINEMATIC==1)
	{
		LogDebug << "really starting cinematic now";
		LastFrameTicks=FrameTicks;
		PLAY_LOADED_CINEMATIC=2;
	}

	PlayTrack(ControlCinematique);
	ControlCinematique->InitDeviceObjects();
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	ControlCinematique->Render(FrameTicks-LastFrameTicks);

	//fin de l'anim
	if ((!ControlCinematique->key)
		|| (GInput->isKeyPressedNowUnPressed(Keyboard::Key_Escape))
		|| (GInput->isKeyPressedNowUnPressed(Keyboard::Key_Escape)))
	{			
		ControlCinematique->projectload=false;
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

#ifdef BUILD_EDITOR
void DanaeItemAdd()
{
	INTERACTIVE_OBJ * tmp=AddInteractive(ItemToBeAdded,0,IO_IMMEDIATELOAD);

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
				ScriptEvent::send(&inter.iobj[num]->script,SM_INIT,"",inter.iobj[num],"");
			}

			if (inter.iobj[num] && inter.iobj[num]->over_script.data)
			{
				ScriptEvent::send(&inter.iobj[num]->over_script,SM_INIT,"",inter.iobj[num],"");
			}

			if (inter.iobj[num] && inter.iobj[num]->script.data)
			{
				ScriptEvent::send(&inter.iobj[num]->script,SM_INITEND,"",inter.iobj[num],"");
			}

			if (inter.iobj[num] && inter.iobj[num]->over_script.data)
			{
				ScriptEvent::send(&inter.iobj[num]->over_script,SM_INITEND,"",inter.iobj[num],"");
			}
		}
	}

	ItemToBeAdded[0]=0;
}
#endif

void ReMappDanaeButton()
{
	bool bNoAction=true;
	int iButton=config.actions[CONTROLS_CUST_ACTION].key[0];

	if(iButton!=-1)
	{
		if(GInput->getMouseButtonDoubleClick(iButton,300))
		{
			LastEERIEMouseButton=EERIEMouseButton;
			EERIEMouseButton|=4;
			EERIEMouseButton&=~1;
			bNoAction=false;
		}
	}

	if(bNoAction)
	{
		iButton=config.actions[CONTROLS_CUST_ACTION].key[1];

		if(iButton!=-1)
		{
			if(GInput->getMouseButtonDoubleClick(iButton,300))
			{
				LastEERIEMouseButton=EERIEMouseButton;
				EERIEMouseButton|=4;
				EERIEMouseButton&=~1;
			}
		}
	}

	bNoAction=true;
	iButton=config.actions[CONTROLS_CUST_ACTION].key[0];

	if(iButton!=-1)
	{
		if(	((iButton & Mouse::ButtonBase) && (GInput->getMouseButtonNowPressed(iButton)))||
			((!(iButton & Mouse::ButtonBase)) && GInput->isKeyPressedNowPressed(iButton)) )
		{
			LastEERIEMouseButton=EERIEMouseButton;
			EERIEMouseButton|=1;

			if (EERIEMouseButton&4) EERIEMouseButton&=~1;

			bNoAction=false;
		}
	}

	if(bNoAction)
	{
		iButton=config.actions[CONTROLS_CUST_ACTION].key[1];

		if(iButton!=-1)
		{
			if( ((iButton & Mouse::ButtonBase) && (GInput->getMouseButtonNowPressed(iButton)))||
				((!(iButton & Mouse::ButtonBase)) && GInput->isKeyPressedNowPressed(iButton)) )
			{
				LastEERIEMouseButton=EERIEMouseButton;
				EERIEMouseButton|=1;

				if (EERIEMouseButton&4) EERIEMouseButton&=~1;
			}
		}
	}

	bNoAction=true;
	iButton=config.actions[CONTROLS_CUST_ACTION].key[0];

	if(iButton!=-1)
	{
		if(	((iButton & Mouse::ButtonBase) && (GInput->getMouseButtonNowUnPressed(iButton)))||
			((!(iButton & Mouse::ButtonBase)) && GInput->isKeyPressedNowPressed(iButton)) )
		{
			LastEERIEMouseButton=EERIEMouseButton;
			EERIEMouseButton&=~1;
			EERIEMouseButton&=~4;
			bNoAction=false;
		}
	}

	if(bNoAction)
	{
		iButton=config.actions[CONTROLS_CUST_ACTION].key[1];

		if(iButton!=-1)
		{
			if( ((iButton & Mouse::ButtonBase) && (GInput->getMouseButtonNowUnPressed(iButton)))||
				((!(iButton & Mouse::ButtonBase)) && GInput->isKeyPressedNowPressed(iButton)) )
			{
				LastEERIEMouseButton=EERIEMouseButton;
				EERIEMouseButton&=~1;
				EERIEMouseButton&=~4;
			}
		}
	}

	bNoAction=true;
	iButton=config.actions[CONTROLS_CUST_MOUSELOOK].key[0];

	if(iButton!=-1)
	{
		if(	((iButton & Mouse::ButtonBase) && (GInput->getMouseButtonNowPressed(iButton)))||
			((!(iButton & Mouse::ButtonBase)) && GInput->isKeyPressedNowPressed(iButton)) )
		{
			EERIEMouseButton|=2;
			bNoAction=false;
		}
	}

	if(bNoAction)
	{
		iButton=config.actions[CONTROLS_CUST_MOUSELOOK].key[1];

		if(iButton!=-1)
		{
			if( ((iButton & Mouse::ButtonBase) && (GInput->getMouseButtonNowPressed(iButton)))||
				((!(iButton & Mouse::ButtonBase)) && GInput->isKeyPressedNowPressed(iButton)) )
			{
				EERIEMouseButton|=2;
			}
		}
	}

	bNoAction=true;
	iButton=config.actions[CONTROLS_CUST_MOUSELOOK].key[0];

	if(iButton!=-1)
	{
		if(	((iButton & Mouse::ButtonBase) && (GInput->getMouseButtonNowUnPressed(iButton)))||
			((!(iButton & Mouse::ButtonBase)) && GInput->isKeyPressedNowUnPressed(iButton)) )
		{
			EERIEMouseButton&=~2;
			bNoAction=false;
		}
	}

	if(bNoAction)
	{
		iButton=config.actions[CONTROLS_CUST_MOUSELOOK].key[1];

		if(iButton!=-1)
		{
			if( ((iButton & Mouse::ButtonBase) && (GInput->getMouseButtonNowUnPressed(iButton)))||
				((!(iButton & Mouse::ButtonBase)) && GInput->isKeyPressedNowUnPressed(iButton)) )
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

long WILL_QUICKLOAD=0;
long WILL_QUICKSAVE=0;

// TODO what is the point of this function?
void CorrectValue(unsigned long * cur,unsigned long * dest)
{
	if ((*cur=*dest))
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
void ShowValue(unsigned long * cur,unsigned long * dest, const char * str)
{
	iVPOS+=1;
	CorrectValue(cur,dest);
	Color3f rgb;

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

	float width=(float)(*cur)*( 1.0f / 500 );
	EERIEDrawBitmap(0, ARX_CLEAN_WARN_CAST_FLOAT(iVPOS * 16), width, 8, 0.000091f, NULL, rgb.to<u8>());
	danaeApp.OutputText(static_cast<u32>(width), iVPOS * 16 - 2, str);

}

extern long NEED_INTRO_LAUNCH;

HRESULT DANAE::Render() {
	
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
		// Under 10 FPS the whole game slows down to avoid unexpected results...
		_framedelay=(float)FrameDiff;
	}
	else
	{
		// Nuky - added this security because sometimes when hitting ESC, FrameDiff would get negative
		if (LastFrameTime>FrameTime)
		{
			LastFrameTime=FrameTime;
		}
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

	if( GInput->isKeyPressedNowPressed(Keyboard::Key_F12) )
	{
		EERIE_PORTAL_ReleaseOnlyVertexBuffer();
		ComputePortalVertexBuffer();
	}

	ACTIVECAM = &subj;

	if (	(!FINAL_COMMERCIAL_DEMO)
		&&	(!FINAL_COMMERCIAL_GAME)
		&&  (ARXmenu.currentmode==AMCM_OFF)	)
	{
		if(	GInput->isKeyPressedNowPressed(Keyboard::Key_Y) )
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

	if (this->m_pFramework->m_bHasMoved)
	{
		LogDebug << "has moved";
		
		DanaeRestoreFullScreen();

		this->m_pFramework->m_bHasMoved=false;

		AdjustUI();
	}

	// Get DirectInput Infos
	if ((!USE_OLD_MOUSE_SYSTEM))
	{
		GInput->update();
		ReMappDanaeButton();
	}

	// Manages Splash Screens if needed
	if(DANAE_ManageSplashThings()) {
		goto norenderend;
	}

	// Clicked on New Quest ? (TODO:need certainly to be moved somewhere else...)
	if (START_NEW_QUEST)
	{
		LogDebug << "start quest";
		DANAE_StartNewQuest();
	}

	// Update Various Player Infos for this frame.
	if (FirstFrame==0)
		ARX_PLAYER_Frame_Update();
	
	// Project need to reload all textures ???
	if (WILL_RELOAD_ALL_TEXTURES)
	{
		LogDebug << "reload all textures";
		//ReloadAllTextures(); TODO is this needed for changing resolutions in-game?
		WILL_RELOAD_ALL_TEXTURES=0;
	}

	// Are we being teleported ?
	if ((TELEPORT_TO_LEVEL[0]) && (CHANGE_LEVEL_ICON==200))
	{
		LogDebug << "teleport to " << TELEPORT_TO_LEVEL << " " << TELEPORT_TO_POSITION << " "
		         << TELEPORT_TO_ANGLE;
		CHANGE_LEVEL_ICON=-1;
		ARX_CHANGELEVEL_Change(TELEPORT_TO_LEVEL, TELEPORT_TO_POSITION, TELEPORT_TO_ANGLE, 0);
		memset(TELEPORT_TO_LEVEL,0,64);
		memset(TELEPORT_TO_POSITION,0,64);
	}

	if (NEED_INTRO_LAUNCH)
	{
		LogDebug << "need intro launch";
		SetEditMode(0);
		BLOCK_PLAYER_CONTROLS=1;
		ARX_INTERFACE_PlayerInterfaceModify(0,0);
		ARX_Menu_Resources_Release();
		ARXmenu.currentmode=AMCM_OFF;
		ARX_TIME_UnPause();
		SPLASH_THINGS_STAGE=14;
		NEED_INTRO_LAUNCH=0;
		REFUSE_GAME_RETURN=1;
		const char RESOURCE_LEVEL_10[] = "Graph\\Levels\\Level10\\level10.dlf";
		OLD_PROGRESS_BAR_COUNT=PROGRESS_BAR_COUNT=0;
		PROGRESS_BAR_TOTAL = 108;
		LoadLevelScreen(10);	
		DanaeLoadLevel(RESOURCE_LEVEL_10);
		FORBID_SAVE=0;
		FirstFrame=1;
		SPLASH_THINGS_STAGE=0;
		INTRO_NOT_LOADED=0;
		GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);
		return false;
	}
		
	//Setting long from long
	subj.centerx = DANAECENTERX;
	subj.centery = DANAECENTERY;

	//Casting long to float
	subj.posleft = subj.transform.xmod = ARX_CLEAN_WARN_CAST_FLOAT( DANAECENTERX );
	subj.postop	 = subj.transform.ymod = ARX_CLEAN_WARN_CAST_FLOAT( DANAECENTERY );

	// Finally computes current focal
	BASE_FOCAL=(float)CURRENT_BASE_FOCAL+(BOW_FOCAL*( 1.0f / 4 ));

	// SPECIFIC code for Snapshot MODE... to insure constant capture framerate

	PULSATE=EEsin(FrameTime / 800);
	METALdecal=EEsin(FrameTime / 50) / 200 ;
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

#ifdef BUILD_EDITOR
		if(MOULINEX) {
			LaunchMoulinex();
		}
#endif
	}
	else // Manages our first frameS
	{
		LogDebug << "first frame";
		ARX_TIME_Get();
		long ffh=FirstFrameHandling();

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

	GRenderer->SetRenderState(Renderer::Fog, false);

	if(ARX_Menu_Render()) {
		goto norenderend;
	}

	if (WILL_QUICKSAVE)
	{
		::SnapShot *pSnapShot=new ::SnapShot(NULL,"sct",true);
		pSnapShot->GetSnapShotDim(160,100);
		delete pSnapShot;

		if (WILL_QUICKSAVE>=2)
		{
			ARX_QuickSave();
			WILL_QUICKSAVE=0;
		}
		else WILL_QUICKSAVE++;
	}

	if (WILL_QUICKLOAD)
	{
		WILL_QUICKLOAD=0;

		if (ARX_QuickLoad())
			NEED_SPECIAL_RENDEREND=1;
	}

	if (NEED_SPECIAL_RENDEREND)
	{
		NEED_SPECIAL_RENDEREND=0;
		goto norenderend;
	}

	GRenderer->SetRenderState(Renderer::Fog, true);
	GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);

	// Are we displaying a 2D cinematic ? Yes = manage it
	if (	PLAY_LOADED_CINEMATIC
		&&	ControlCinematique
			&&	ControlCinematique->projectload)
	{
		if (DANAE_Manage_Cinematic()==1)
			goto norenderend;

		goto renderend;
	}

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

	// SUBJECTIVE VIEW UPDATE START  *********************************************************
	{
		// Clear screen & Z buffers
		if(desired.flags & GMOD_DCOLOR) {
			GRenderer->Clear(Renderer::ColorBuffer | Renderer::DepthBuffer, current.depthcolor.to<u8>());
		}
		else
		{
			subj.bkgcolor=ulBKGColor;
			GRenderer->Clear(Renderer::ColorBuffer | Renderer::DepthBuffer, subj.bkgcolor);
		}

		//-------------------------------------------------------------------------------
		//															DRAW CINEMASCOPE 16/9
		if(CINEMA_DECAL != 0.f) {
			Rect rectz[2];
			rectz[0].left = rectz[1].left = 0;
			rectz[0].right = rectz[1].right	=	DANAESIZX;
			rectz[0].top = 0;
			ARX_CHECK_LONG(CINEMA_DECAL * Yratio);
			long lMulResult = static_cast<long>(CINEMA_DECAL * Yratio);
			rectz[0].bottom = lMulResult;
			rectz[1].top = DANAESIZY - lMulResult;
			rectz[1].bottom = DANAESIZY;
			GRenderer->Clear(Renderer::ColorBuffer | Renderer::DepthBuffer, Color::none, 0.0f, 2, rectz);
		}
		//-------------------------------------------------------------------------------

	if(!GRenderer->BeginScene())
	{
		return E_FAIL;
	}
	
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);

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

		long tFrameDiff = Original_framedelay;
	
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
				long step=min(50L,tFrameDiff);

				if (inter.iobj[0]->ioflags & IO_FREEZESCRIPT) step=0;


				float iCalc = step*speedfactor ;
				ARX_CHECK_ULONG(iCalc);

				assert(inter.iobj[0]->obj != NULL);
				EERIEDrawAnimQuat(inter.iobj[0]->obj, &inter.iobj[0]->animlayer[0], &inter.iobj[0]->angle,
				                  &inter.iobj[0]->pos, ARX_CLEAN_WARN_CAST_ULONG(iCalc), inter.iobj[0], false);

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

			assert(inter.iobj[0]->obj != NULL);
			EERIEDrawAnimQuat(inter.iobj[0]->obj, &inter.iobj[0]->animlayer[0], &inter.iobj[0]->angle,
			                  &inter.iobj[0]->pos, ARX_CLEAN_WARN_CAST_ULONG(val), inter.iobj[0], false);


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
		float t=radians(player.angle.b);
		Vec3f tt;

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

		if (inter.iobj[0])
		{
			long id = inter.iobj[0]->obj->fastaccess.view_attach;

			if (id!=-1)
			{
				subj.pos.x=inter.iobj[0]->obj->vertexlist3[id].v.x;
				subj.pos.y=inter.iobj[0]->obj->vertexlist3[id].v.y;
				subj.pos.z=inter.iobj[0]->obj->vertexlist3[id].v.z;

				Vec3f vect;
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
		subj.pos.x=(subj.pos.x+subj.d_pos.x)*( 1.0f / 2 );
		subj.pos.y=(subj.pos.y+subj.d_pos.y)*( 1.0f / 2 );
		subj.pos.z=(subj.pos.z+subj.d_pos.z)*( 1.0f / 2 );

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
				for(size_t k = 0 ; k < MAX_ASPEECH; k++) {
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

			if (rnd()>0.4f) conversationcamera.d_angle.a=(1.f-rnd()*2.f)*( 1.0f / 30 );

			if (rnd()>0.4f) conversationcamera.d_angle.b=(1.f-rnd()*1.2f)*( 1.0f / 5 );

			if (rnd()>0.4f) conversationcamera.d_angle.g=(1.f-rnd()*2.f)*( 1.0f / 40 );

			if (rnd()>0.5f)
			{
				conversationcamera.size.a=MAKEANGLE(180.f+rnd()*20.f-10.f);
				conversationcamera.size.b=0.f;
				conversationcamera.size.g=0.f;
				conversationcamera.d_angle.g=0.08f;
				conversationcamera.d_angle.b=0.f;
				conversationcamera.d_angle.a = 0.f;
			}
		}
		else
		{
			conversationcamera.size += conversationcamera.d_angle * FrameDiff;
		}

		Vec3f sourcepos,targetpos;

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
			float t=radians(player.angle.b);
			sourcepos.x=targetpos.x+(float)EEsin(t)*100.f;
			sourcepos.y=targetpos.y;
			sourcepos.z=targetpos.z-(float)EEcos(t)*100.f;
			}

		Vec3f vect,vec2;
		vect.x=targetpos.x-sourcepos.x;
		vect.y=targetpos.y-sourcepos.y;
		vect.z=targetpos.z-sourcepos.z;
		float mag=1.f/Vector_Magnitude(&vect);
		vect.x*=mag;
		vect.y*=mag;
		vect.z*=mag;
		float dist=250.f-conversationcamera.size.g;

		if (dist<0.f) dist=(90.f-(dist*( 1.0f / 20 )));
		else if (dist<90.f) dist=90.f;

		_YRotatePoint(&vect,&vec2,EEcos(radians(conversationcamera.size.a)),EEsin(radians(conversationcamera.size.a)));
		
		sourcepos.x=targetpos.x-vec2.x*dist;
		sourcepos.y=targetpos.y-vec2.y*dist;
		sourcepos.z=targetpos.z-vec2.z*dist;

		if (conversationcamera.size.b!=0.f)
			sourcepos.y+=120.f-conversationcamera.size.b*( 1.0f / 10 );

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

		for(size_t i = 0; i < MAX_ASPEECH; i++) {
			if ((aspeech[i].exist) && (aspeech[i].cine.type>0))
			{
				valid=i;
				break;
			}
		}

		if (valid>=0)
		{
			CinematicSpeech * acs=&aspeech[valid].cine;
			INTERACTIVE_OBJ * io=aspeech[valid].io;
			float rtime=(float)(ARX_TIME_Get()-aspeech[valid].time_creation)/(float)aspeech[valid].duration;

			if (rtime<0) rtime=0;

			if (rtime>1) rtime=1;

			float itime=1.f-rtime;

			if ((rtime>=0.f) && (rtime<=1.f) && io)
			{
				float alpha,beta,distance,_dist;

				switch (acs->type)
				{
					case ARX_CINE_SPEECH_KEEP: {
						subj.pos.x=acs->pos1.x;
						subj.pos.y=acs->pos1.y;
						subj.pos.z=acs->pos1.z;
						subj.angle.a=acs->pos2.x;
						subj.angle.b=acs->pos2.y;
						subj.angle.g=acs->pos2.z;
						EXTERNALVIEW=1;
						break;
					}
					case ARX_CINE_SPEECH_ZOOM: {
						//need to compute current values
						alpha=acs->startangle.a*itime+acs->endangle.a*rtime;
						beta=acs->startangle.b*itime+acs->endangle.b*rtime;
						distance=acs->startpos*itime+acs->endpos*rtime;
						Vec3f targetpos = acs->pos1;
						conversationcamera.pos.x=-EEsin(radians(MAKEANGLE(io->angle.b+beta)))*distance+targetpos.x;
						conversationcamera.pos.y= EEsin(radians(MAKEANGLE(io->angle.a+alpha)))*distance+targetpos.y;
						conversationcamera.pos.z= EEcos(radians(MAKEANGLE(io->angle.b+beta)))*distance+targetpos.z;						
						SetTargetCamera(&conversationcamera,targetpos.x,targetpos.y,targetpos.z);
						subj.pos.x=conversationcamera.pos.x;
						subj.pos.y=conversationcamera.pos.y;
						subj.pos.z=conversationcamera.pos.z;
						subj.angle.a=MAKEANGLE(-conversationcamera.angle.a);
						subj.angle.b=MAKEANGLE(conversationcamera.angle.b-180.f);
						subj.angle.g=0.f;
						EXTERNALVIEW=1;
						break;
					}
					case ARX_CINE_SPEECH_SIDE_LEFT:
					case ARX_CINE_SPEECH_SIDE: {

						if (ValidIONum(acs->ionum))
						{

							const Vec3f & from = acs->pos1;
							const Vec3f & to = acs->pos2;

							Vec3f vect = (to - from).getNormalized();

							Vec3f vect2;
							if (acs->type==ARX_CINE_SPEECH_SIDE_LEFT)
							{
								Vector_RotateY(&vect2,&vect,-90);
							}
							else
							{
								Vector_RotateY(&vect2,&vect,90);
							}

							distance=acs->f0*itime+acs->f1*rtime;
							vect2 *= distance;
							_dist = dist(from, to);
							Vec3f tfrom = from + vect * acs->startpos * (1.0f / 100) * _dist;
							Vec3f tto = from + vect * acs->endpos * (1.0f / 100) * _dist;
							Vec3f targetpos;
							targetpos.x=tfrom.x*itime+tto.x*rtime;
							targetpos.y=tfrom.y*itime+tto.y*rtime+acs->f2;
							targetpos.z=tfrom.z*itime+tto.z*rtime;
							conversationcamera.pos.x=targetpos.x+vect2.x;
							conversationcamera.pos.y=targetpos.y+vect2.y+acs->f2;
							conversationcamera.pos.z=targetpos.z+vect2.z;
							SetTargetCamera(&conversationcamera,targetpos.x,targetpos.y,targetpos.z);
							subj.pos = conversationcamera.pos;
							subj.angle.a=MAKEANGLE(-conversationcamera.angle.a);
							subj.angle.b=MAKEANGLE(conversationcamera.angle.b-180.f);
							subj.angle.g=0.f;
							EXTERNALVIEW=1;
						}

						break;
					}
					case ARX_CINE_SPEECH_CCCLISTENER_R:
					case ARX_CINE_SPEECH_CCCLISTENER_L:
					case ARX_CINE_SPEECH_CCCTALKER_R:
					case ARX_CINE_SPEECH_CCCTALKER_L: {

						//need to compute current values
						if (ValidIONum(acs->ionum))
						{
							Vec3f targetpos;
							if ((acs->type==ARX_CINE_SPEECH_CCCLISTENER_L)
								|| (acs->type==ARX_CINE_SPEECH_CCCLISTENER_R))
							{
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
							
							distance=(acs->startpos*itime+acs->endpos*rtime)*( 1.0f / 100 );						
							
							Vec3f vect;
							vect.x=conversationcamera.pos.x-targetpos.x;
							vect.y=conversationcamera.pos.y-targetpos.y;
							vect.z=conversationcamera.pos.z-targetpos.z;
							Vec3f vect2;
							Vector_RotateY(&vect2,&vect,90);
							vect2.normalize();
							Vec3f vect3 = vect.getNormalized();

							vect = vect * distance + vect3 * 80.f;
							vect2 *= 45.f;

							if ((acs->type==ARX_CINE_SPEECH_CCCLISTENER_R)
								|| (acs->type==ARX_CINE_SPEECH_CCCTALKER_R))
							{
								vect2 = -vect2;
							}

							conversationcamera.pos = vect + targetpos + vect2;
							SetTargetCamera(&conversationcamera,targetpos.x,targetpos.y,targetpos.z);
							subj.pos = conversationcamera.pos;
							subj.angle.a=MAKEANGLE(-conversationcamera.angle.a);
							subj.angle.b=MAKEANGLE(conversationcamera.angle.b-180.f);
							subj.angle.g=0.f;
							EXTERNALVIEW=1;
						}

						break;
					}
					case ARX_CINE_SPEECH_NONE: break;
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
		DeadCameraDistance+=(float)FrameDiff*( 1.0f / 80 )*((mdist-DeadCameraDistance)/mdist)*2.f;

		if (DeadCameraDistance>mdist) DeadCameraDistance=mdist;

		Vec3f targetpos;

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

#ifdef BUILD_EDITOR
		if(!GAME_EDITOR)
			BLOCK_PLAYER_CONTROLS=1;
#endif
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

			Vec3f targetpos;

		targetpos.x=CAMERACONTROLLER->pos.x;
		targetpos.y=CAMERACONTROLLER->pos.y+PLAYER_BASE_HEIGHT;
		targetpos.z=CAMERACONTROLLER->pos.z;

			float delta_angle = AngleDifference(currentbeta, CAMERACONTROLLER->angle.b);
			float delta_angle_t = delta_angle * FrameDiff * ( 1.0f / 1000 );

			if (EEfabs(delta_angle_t) > EEfabs(delta_angle)) delta_angle_t = delta_angle;

			currentbeta += delta_angle_t;
		float t=radians(MAKEANGLE(currentbeta));
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
		Vec3f pos,pos2;
			USE_CINEMATICS_PATH._curtime = ARX_TIME_Get();

		USE_CINEMATICS_PATH._curtime+=50;
		long pouet2=ARX_PATHS_Interpolate(&USE_CINEMATICS_PATH,&pos);
		USE_CINEMATICS_PATH._curtime-=50;
		long pouet=ARX_PATHS_Interpolate(&USE_CINEMATICS_PATH,&pos2);

		if ((pouet!=-1) && (pouet2!=-1))
		{
			if(USE_CINEMATICS_CAMERA == 2) {
				subj.pos = pos;
				subj.d_angle = subj.angle;
				pos2 = (pos2 + pos) * (1.0f/2);
				SetTargetCamera(&subj, pos2.x, pos2.y, pos2.z);
			} else {
				DebugSphere(pos.x, pos.y, pos.z, 2, 50, Color::red);
			}

			if (USE_CINEMATICS_PATH.aupflags & ARX_USEPATH_FLAG_FINISHED) // was .path->flags
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

	if (EDITMODE) GRenderer->SetRenderState(Renderer::Fog, false);

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
	{
		float t = radians(MAKEANGLE(ACTIVECAM->angle.b));			
		Vec3f front(-EEsin(t), 0.f, EEcos(t));
		front.normalize();
		Vec3f up(0.f, 1.f, 0.f);
		ARX_SOUND_SetListener(&ACTIVECAM->pos, &front, &up);
	}

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

		if ((!EXTERNALVIEW) && GLOBAL_FORCE_PLAYER_IN_FRONT)
			FORCE_FRONT_DRAW=1;

		if (inter.iobj[0]->invisibility>0.9f) inter.iobj[0]->invisibility=0.9f;

		assert(inter.iobj[0]->obj != NULL);
		EERIEDrawAnimQuat(inter.iobj[0]->obj, &inter.iobj[0]->animlayer[0], &inter.iobj[0]->angle,
		                  &inter.iobj[0]->pos, 0, inter.iobj[0]);
		
		ACTIVECAM->use_focal=restore;
		FORCE_FRONT_DRAW=0;
	}

	// SUBJECTIVE VIEW UPDATE START  *********************************************************
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::DepthTest, true);

	if (FirstFrame==0)
	{
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
			GRenderer->GetTextureStage(0)->SetMipMapLODBias(val);
			ARX_SCENE_Render(1);
			val=-0.3f;
			GRenderer->GetTextureStage(0)->SetMipMapLODBias(val);
		}
		else {
			ARX_SCENE_Render(1);
		}

	}

#ifdef BUILD_EDITOR
	if (EDITION==EDITION_PATHWAYS)
	{
		ARX_PATHS_RedrawAll();
	}
#endif

	// Begin Particles ***************************************************************************
	if (!(Project.hide & HIDE_PARTICLES))
	{
		if (pParticleManager)
		{
				pParticleManager->Update(ARX_CLEAN_WARN_CAST_LONG(FrameDiff));
			pParticleManager->Render();
		}

		GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);			
		GRenderer->SetRenderState(Renderer::DepthWrite, false);
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		ARX_FOGS_Render();

		ARX_PARTICLES_Render(&subj);
		UpdateObjFx();
		
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);

	}

	// End Particles ***************************************************************************

	if (!EDITMODE) // Playing Game
	{
		// Checks Magic Flares Drawing
		if (!PLAYER_PARALYSED)
		{
			if (EERIEMouseButton & 1)
			{
				if ((ARX_FLARES_Block==0) && (CurrSlot<(long)MAX_SLOT)) 
					ARX_SPELLS_AddPoint(DANAEMouse);
				else
				{
					CurrPoint=0;
					ARX_FLARES_Block=0;
					CurrSlot=1;
				}
			}
			else if (ARX_FLARES_Block==0)
				ARX_FLARES_Block=1;
		}

		ARX_SPELLS_Precast_Check();
		ARX_SPELLS_ManageMagic();
		ARX_SPELLS_UpdateSymbolDraw();

		ManageTorch();

		// Renders Magical Flares
		if (	!((player.Interface & INTER_MAP )
			&&  (!(player.Interface & INTER_COMBATMODE)))
			&&	flarenum
			)
		{
			ARX_MAGICAL_FLARES_Draw(FRAMETICKS);
				FRAMETICKS = ARXTimeUL();
		}
	}
#ifdef BUILD_EDITOR
	else  // EDITMODE == true
	{
		if (!(Project.hide & HIDE_NODES))
				RenderAllNodes();

		std::stringstream ss("EDIT MODE - Selected ");
		ss <<  NbIOSelected;
		ARX_TEXT_Draw(hFontInBook, 100, 2, ss.str(), Color::yellow);
	
		if (EDITION==EDITION_FOGS)
			ARX_FOGS_RenderAll();
	}
	
	// To remove for Final Release but needed until then !
	if (ItemToBeAdded[0]!=0)
		DanaeItemAdd();
#endif
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);

	// Checks some specific spell FX
	CheckMr();

	if (Project.improve)
		DrawImproveVisionInterface();
	else
	{
		if ((subj.focal<BASE_FOCAL))
		{
			subj.focal+=INC_FOCAL;

			if (subj.focal>BASE_FOCAL) subj.focal=BASE_FOCAL;
		}
		else if (subj.focal>BASE_FOCAL) subj.focal=BASE_FOCAL;
	}

	if (eyeball.exist!=0)
	{
		DrawMagicSightInterface();
	}

		if (PLAYER_PARALYSED)
	{
		GRenderer->SetRenderState(Renderer::DepthWrite, false);
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);

		EERIEDrawBitmap(0.f, 0.f, (float)DANAESIZX, (float)DANAESIZY, 0.0001f, NULL, Color(71, 71, 255));
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		GRenderer->SetRenderState(Renderer::DepthWrite, true);
	}

	if (FADEDIR)
	{
		ManageFade();
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	
	// Reset Last Key
	danaeApp.kbd.lastkey=-1;

	// Red screen fade for damages.
	ARX_DAMAGE_Show_Hit_Blood();

	// Manage Notes/Books opened on screen
	GRenderer->SetRenderState(Renderer::Fog, false);
	ARX_INTERFACE_NoteManage();

	finish:; //----------------------------------------------------------------
	// Update spells
	ARX_SPELLS_Update();
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::Fog, true);

	// Manage Death visual & Launch menu...
	if (DeadTime>2000)
		ARX_PLAYER_Manage_Death();

	//-------------------------------------------------------------------------

	// INTERFACE
		// Remove the Alphablend State if needed : NO Z Clear
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetRenderState(Renderer::Fog, false);

	// Draw game interface if needed
	if (ARXmenu.currentmode == AMCM_OFF)
	if (!(Project.hide & HIDE_INTERFACE) && !CINEMASCOPE)
	{
		GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapClamp);
		DrawAllInterface();
		DrawAllInterfaceFinish();

		if (	(player.Interface & INTER_MAP )
			&&  (!(player.Interface & INTER_COMBATMODE))
			&&	flarenum
			)
		{
			GRenderer->SetRenderState(Renderer::DepthTest, false);
			ARX_MAGICAL_FLARES_Draw(FRAMETICKS);
			GRenderer->SetRenderState(Renderer::DepthTest, true);
			FRAMETICKS = ARXTimeUL();
		}
	}

	GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	PopAllTriangleList();
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	PopAllTriangleListTransparency();
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);

	GRenderer->SetRenderState(Renderer::Fog, true);
		this->GoFor2DFX();
	GRenderer->SetRenderState(Renderer::Fog, false);
	GRenderer->Clear(Renderer::DepthBuffer);

	// Speech Management
	if (!EDITMODE)
	{
		ARX_SPEECH_Check();
		ARX_SPEECH_Update();
	}

	GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);

	if(pTextManage && !pTextManage->Empty())
	{
		pTextManage->Update(FrameDiff);
		pTextManage->Render();
	}

	if (SHOW_INGAME_MINIMAP && ((PLAY_LOADED_CINEMATIC == 0) && (!CINEMASCOPE) && (!BLOCK_PLAYER_CONTROLS) && (ARXmenu.currentmode == AMCM_OFF))
		&& (!(player.Interface & INTER_MAP )	))
	{
			long	SHOWLEVEL = ARX_LEVELS_GetRealNum(CURRENTLEVEL);

		if ((SHOWLEVEL>=0) && (SHOWLEVEL<32))
			ARX_MINIMAP_Show(SHOWLEVEL,1,1);
	}

		//-------------------------------------------------------------------------

	// CURSOR Rendering
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);

	if (DRAGINTER)
	{
		ARX_INTERFACE_RenderCursor();

		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		PopAllTriangleList();
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		PopAllTriangleListTransparency();
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);

		ARX_INTERFACE_HALO_Flush();
	}
	else
	{
		ARX_INTERFACE_HALO_Flush();
		ARX_INTERFACE_RenderCursor();
	}

	GRenderer->SetRenderState(Renderer::Fog, true);

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
			ShowTestText();
		}

		if (!NO_TEXT_AT_ALL)
		{
			if (ViewMode & VIEWMODE_INFOTEXT)
			{
				ShowInfoText();
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
				sprintf(tex,"2DPortals_ROOM: %ld",LAST_ROOM);
				break;
			case 2:
				sprintf(tex,"3DPortals_ROOM: %ld - Vis %ld",LAST_ROOM,LAST_PORTALS_COUNT);
				break;
			case 3:
				sprintf(tex,"3DPortals_ROOM(Transform): %ld - Vis %ld",LAST_ROOM,LAST_PORTALS_COUNT);
				break;
			case 4:
				sprintf(tex,"3DPortals_ROOM(TransformSC): %ld - Vis %ld",LAST_ROOM,LAST_PORTALS_COUNT);
				break;
			}

			danaeApp.OutputText( 320, 240, tex );
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
		if (GInput->actionNowPressed(CONTROLS_CUST_QUICKLOAD) && !WILL_QUICKLOAD)
		{
			WILL_QUICKLOAD=1;
		}

		if (GInput->actionNowPressed(CONTROLS_CUST_QUICKSAVE) && !WILL_QUICKSAVE)
		{
			iTimeToDrawD7=2000;
			WILL_QUICKSAVE=1;
		}

		ARX_DrawAfterQuickLoad();
	}

	GRenderer->EndScene();

	//--------------NORENDEREND---------------------------------------------------
	norenderend:
		;

	if(GInput->isKeyPressedNowPressed(Keyboard::Key_F10))
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

	LastFrameTime=FrameTime;
	LastMouseClick=EERIEMouseButton;

#ifdef BUILD_EDITOR
		DANAE_DEBUGGER_Update();
#endif

	return S_OK;
}

void DANAE::GoFor2DFX()
{
	TexturedVertex lv,ltvv;

	long needed = 0;

	for (long i=0;i<TOTPDL;i++)
	{
		EERIE_LIGHT * el=PDL[i];

		if (el->extras & EXTRAS_FLARE)
		{
			if(distSqr(ACTIVECAM->pos, el->pos) < square(2200)) {
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
		float temp_increase=_framedelay*( 1.0f / 1000 )*4.f;
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

					if ((ltvv.rhw > 0.f) &&
						(ltvv.sx>0.f) &&
						(ltvv.sy>(CINEMA_DECAL*Yratio)) &&
						(ltvv.sx<DANAESIZX) &&
						(ltvv.sy<(DANAESIZY-(CINEMA_DECAL*Yratio)))
						)
					{
						Vec3f vector;
						vector.x=lv.sx-ACTIVECAM->pos.x;
						vector.y=lv.sy-ACTIVECAM->pos.y;
						vector.z=lv.sz-ACTIVECAM->pos.z;
						float fNorm = 50.f / sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
						vector.x*=fNorm;
						vector.y*=fNorm;
						vector.z*=fNorm;
						TexturedVertex ltvv2;
						lv.sx-=vector.x;
						lv.sy-=vector.y;
						lv.sz-=vector.z;
						specialEE_RTP(&lv,&ltvv2);

						float fZFar=ProjectionMatrix._33*(1.f/(ACTIVECAM->cdepth*fZFogEnd))+ProjectionMatrix._43;

						Vec3f	hit;
						EERIEPOLY	*tp=NULL;
						Vec2s ees2dlv;
						Vec3f ee3dlv;
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

					}

					if (el->temp<0.f) el->temp=0.f;
					else if (el->temp>.8f) el->temp=.8f;
				}
			}
		}

		DURING_LOCK = 0;
		// End 2D Pass ***************************************************************************

		{
			GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
			GRenderer->SetRenderState(Renderer::AlphaBlending, true);		
			GRenderer->SetRenderState(Renderer::DepthWrite, false);
			GRenderer->SetCulling(Renderer::CullNone);
			GRenderer->SetRenderState(Renderer::DepthTest, false);
			GRenderer->SetFogColor(Color::none);

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
						specialEE_RT((TexturedVertex *)&lv,(Vec3f *)&ltvv);
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

						EERIEDrawSprite(&lv, siz, tflare, Color3f(v*el->rgb.r,v*el->rgb.g,v*el->rgb.b).to<u8>(), ltvv.sz);

					}
				}
			}

			GRenderer->SetRenderState(Renderer::DepthTest, true);
		}
	}

	GRenderer->SetRenderState(Renderer::DepthWrite, true);
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

	sprintf( tex,"Last Failed Sequence : %s",LAST_FAILED_SEQUENCE.c_str() );
	danaeApp.OutputText( 0, 64, tex );
}
extern float CURRENT_PLAYER_COLOR;
extern int TSU_TEST_COLLISIONS;
extern long TSU_TEST;

long TSU_TEST_NB = 0;
long TSU_TEST_NB_LIGHT = 0;

static void ShowInfoText() {
	
	unsigned long uGAT = ARXTimeUL() / 1000;
	long GAT=(long)uGAT;
	char tex[256];
	float fpss2=1000.f/_framedelay;
	LASTfpscount++;
	
	float fps2v=max(fpss2,LASTfps2);
	float fps2vmin=min(fpss2,LASTfps2);
	
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

	sprintf(tex, "%ld Prims %4.02f fps ( %3.02f - %3.02f ) [%3.0fms] INTER:%ld/%ld MIPMESH %d [%3.06f", EERIEDrawnPolys, FPS, fps2min, fps2, _framedelay, INTER_DRAW, INTER_COMPUTE, 0, vdist);
	danaeApp.OutputText( 70, 32, tex );

	float poss=-666.66f;
	EERIEPOLY * ep=CheckInPolyPrecis(player.pos.x,player.pos.y,player.pos.z);
	float tempo=0.f;

	if ((ep) && (GetTruePolyY(ep,&player.pos,&tempo)))
		poss=tempo;

	sprintf(tex,"Position  x:%7.0f y:%7.0f [%7.0f] z:%6.0f a%3.0f b%3.0f FOK %3.0f",player.pos.x,player.pos.y+player.size.y,poss,player.pos.z,player.angle.a,player.angle.b,ACTIVECAM->focal);
	danaeApp.OutputText( 70, 48, tex );
	sprintf(tex,"AnchorPos x:%6.0f y:%6.0f z:%6.0f TIME %lds Part %ld - %d  Lkey %d",player.pos.x-Mscenepos.x,player.pos.y+player.size.y-Mscenepos.y,player.pos.z-Mscenepos.z
		,GAT,ParticleCount,player.doingmagic,danaeApp.kbd.lastkey);
	danaeApp.OutputText( 70, 64, tex );

	if (player.onfirmground==0) danaeApp.OutputText( 200, 280, "OFFGRND" );

	sprintf(tex,"Jump %f cinema %f %d %d - Pathfind %ld(%s)",player.jumplastposition,CINEMA_DECAL,DANAEMouse.x,DANAEMouse.y,EERIE_PATHFINDER_Get_Queued_Number(), PATHFINDER_WORKING ? "Working" : "Idled");
	danaeApp.OutputText( 70, 80, tex );
	INTERACTIVE_OBJ * io=ARX_SCRIPT_Get_IO_Max_Events();

	char temp[256];

	if (io==NULL)
		sprintf(tex,"Events %ld (IOmax N/A) Timers %ld",ScriptEvent::totalCount,ARX_SCRIPT_CountTimers());
	else 
	{
		strcpy(temp,GetName(io->filename).c_str());	
		sprintf(tex,"Events %ld (IOmax %s_%04ld %d) Timers %ld",ScriptEvent::totalCount,temp,io->ident,io->stat_count,ARX_SCRIPT_CountTimers());
	}

	danaeApp.OutputText( 70, 94, tex );

	io=ARX_SCRIPT_Get_IO_Max_Events_Sent();

	if (io!=NULL)
	{
		strcpy(temp,GetName(io->filename).c_str());	
		sprintf(tex,"Max SENDER %s_%04ld %d)",temp,io->ident,io->stat_sent);
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

	sprintf(tex, "TSU_TEST %ld - nblights %ld - nb %ld", TSU_TEST, TSU_TEST_NB_LIGHT, TSU_TEST_NB);
	danaeApp.OutputText( 100, 208, tex );
	TSU_TEST_NB = 0;
	TSU_TEST_NB_LIGHT = 0;

#ifdef BUILD_EDITOR
	if ((!EDITMODE) && (ValidIONum(LastSelectedIONum)))
	{
		io = inter.iobj[LastSelectedIONum];

	  if (io)
	  {
		  if (io==inter.iobj[0])
		  {
			  	sprintf(tex,"%4.0f %4.0f %4.0f - %4.0f %4.0f %4.0f -- %3.0f %d/%ld targ %ld beh %ld",io->pos.x,
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
				  
				sprintf(tex,"%4.0f %4.0f %4.0f - %4.0f %4.0f %4.0f -- %3.0f %d/%ld targ %ld beh %ld",io->pos.x,
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
					sprintf(tex,"PF_%ld", (long)io->_npcdata->pathfind.flags);
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
#endif // BUILD_EDITOR

	long zap=IsAnyPolyThere(player.pos.x,player.pos.z);
	sprintf(tex,"POLY %ld LASTLOCK %ld",zap,LAST_LOCK_SUCCESSFULL);		
	danaeApp.OutputText( 270, 220, tex );

	sprintf(tex,"COLOR %3.0f Stealth %3.0f",CURRENT_PLAYER_COLOR,GetPlayerStealth());
	danaeApp.OutputText( 270, 200, tex );

	ARX_SCRIPT_Init_Event_Stats();
}

//-----------------------------------------------------------------------------

extern long POLYIN;
extern long LAST_LLIGHT_COUNT;
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
	
	float fps2v=max(fpss2,LASTfps2);
	float fps2vmin=min(fpss2,LASTfps2);
	
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

	sprintf(tex,"%ld Prims %4.02f fps ( %3.02f - %3.02f ) [%3.0fms] INTER:%ld/%ld INTREAT:%ld"
	        , EERIEDrawnPolys, FPS, fps2min, fps2, _framedelay, INTER_DRAW, INTER_COMPUTE, INTREATZONECOUNT);
	danaeApp.OutputText( 70, DANAESIZY-100+32, tex );

	TOTAL_CHRONO=0;
//	TODO(lubosz): Don't get this by extern global
//	sprintf(tex,"%4.0f MCache %ld[%ld] FP %3.0f %3.0f Llights %ld/%ld TOTIOPDL %ld TOTPDL %ld"
//		,inter.iobj[0]->pos.y, meshCache.size(),MCache_GetSize(),Original_framedelay,_framedelay,LAST_LLIGHT_COUNT,MAX_LLIGHTS,TOTIOPDL,TOTPDL);

	if (LAST_LLIGHT_COUNT>MAX_LLIGHTS)
		strcat(tex," EXCEEDING LIMIT !!!");

	danaeApp.OutputText(70,DANAESIZY-170-144,tex);
	sprintf(tex,"Pos %10.3f Screen %10.3f"
		,LAST_FZPOS,LAST_FZSCREEN);
	danaeApp.OutputText(320,200,tex);
}

void ARX_SetAntiAliasing() {
	GRenderer->SetAntialiasing(config.video.antialiasing);
}

HRESULT DANAE::InitDeviceObjects()
{
	// Enable Z-buffering RenderState
	GRenderer->SetRenderState(Renderer::DepthTest, true);
	
	// Restore All Textures RenderState
	GRenderer->RestoreAllTextures();

	ARX_PLAYER_Restore_Skin();
	
	// Disable Lighting RenderState
	GRenderer->SetRenderState(Renderer::Lighting, false);

	// Setup Texture Border RenderState
	GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);

	GRenderer->GetTextureStage(1)->DisableColor();
	
	// Fog
	float fogEnd = 0.48f;
	float fogStart = fogEnd * 0.65f;
	GRenderer->SetFogParams(Renderer::FogLinear, fogStart, fogEnd);
	GRenderer->SetFogColor(current.depthcolor.to<u8>());
	GRenderer->SetRenderState(Renderer::Fog, true);
	
	SetZBias(0);

	ComputePortalVertexBuffer();
	VertexBuffer<SMY_VERTEX3> * vb3 = GRenderer->createVertexBuffer3(4000, Renderer::Stream);
	pDynamicVertexBuffer = new CircularVertexBuffer<SMY_VERTEX3>(vb3);
	
	VertexBuffer<TexturedVertex> * vb = GRenderer->createVertexBufferTL(4000, Renderer::Stream);
	pDynamicVertexBuffer_TLVERTEX = new CircularVertexBuffer<TexturedVertex>(vb);

	if(pMenu)
	{
		pMenu->bReInitAll=true;
	}

	ARX_SetAntiAliasing();

	m_pD3D->EvictManagedTextures();

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
#ifdef BUILD_EDITOR
	KillInterTreeView();
#endif
	return S_OK;
}

//*************************************************************************************
// DeleteDeviceObjects()
//  Called when the app is exitting, or the device is being changed,
//  this function deletes any device dependant objects.
//*************************************************************************************
HRESULT DANAE::DeleteDeviceObjects() {
	
	GRenderer->ReleaseAllTextures();
	
	if(pDynamicVertexBuffer_TLVERTEX) {
		delete pDynamicVertexBuffer_TLVERTEX;
		pDynamicVertexBuffer_TLVERTEX = NULL;
	}
	
	if(pDynamicVertexBuffer) {
		delete pDynamicVertexBuffer;
		pDynamicVertexBuffer = NULL;
	}
	
	EERIE_PORTAL_ReleaseOnlyVertexBuffer();
	
	return S_OK;
}

//*************************************************************************************
// MsgProc()
//   Overrides StdMsgProc
//*************************************************************************************
LRESULT DANAE::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	
	switch(uMsg) {
		
		case WM_ACTIVATE: {
			if(GInput) {
				if(wParam==WA_INACTIVE) {
					GInput->unacquireDevices();
				} else {
					GInput->reset();
					GInput->unacquireDevices();
					GInput->acquireDevices();
				}
			}
			break;
		}
		
		case WM_SYSCOMMAND: // To avoid ScreenSaver Interference

			if ((wParam & 0xFFF0)== SC_SCREENSAVE ||
				(wParam & 0xFFF0)== SC_MONITORPOWER)
			{
				return 0;
			}

		break;

#ifdef BUILD_EDITOR

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
				return false;
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
				hDrop = (HANDLE) wParam;
				number=DragQueryFile((HDROP)hDrop,0,temp,512);

				if (number > 0)
				{

						strcpy(ItemToBeAdded,temp);
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
					Pause(true);
					DialogBox( (HINSTANCE)GetWindowLongPtr( danaeApp.m_hWnd, GWLP_HINSTANCE ),
							MAKEINTRESOURCE(IDD_SEARCH), danaeApp.m_hWnd, ScriptSearchProc);

					if (SCRIPT_SEARCH_TEXT[0])
						ARX_SCRIPT_LaunchScriptSearch(SCRIPT_SEARCH_TEXT);

					Pause(false);
					ARX_TIME_UnPause();
				break;
				case DANAE_B014:

					if (EDITION==EDITION_PARTICLES)
					{
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B014,false); //Particles
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B011,false); //Pathways
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B010,false); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B004,false); //Nodes
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B007,false); //Lights
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B008,false); //Fogs
						EDITION=EDITION_IO;
					}
					else 
					{
						EDITION=EDITION_PARTICLES;
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B011,false); //Pathways						
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B010,false); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B004,false); //Nodes
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B007,false); //Lights
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B008,false); //Fogs
					}

				break;
				case DANAE_B011: 

					if (EDITION==EDITION_PATHWAYS) 
					{
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B014,false); //Particles
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B011,false); //Pathways
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B010,false); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B004,false); //Nodes
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B007,false); //Lights
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B008,false); //Fogs
						EDITION=EDITION_IO;
					}
					else
					{
						EDITION=EDITION_PATHWAYS;
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B014,false); //Particles
						//SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B011,false); //Pathways						
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B010,false); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B004,false); //Nodes
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B007,false); //Lights
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B008,false); //Fogs
					}

				break;
				case DANAE_B010: 

					if (EDITION==EDITION_ZONES) 
					{
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B014,false); //Particles
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B011,false); //Pathways
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B010,false); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B004,false); //Nodes
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B007,false); //Lights
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B008,false); //Fogs
						EDITION=EDITION_IO;
					}
					else 
					{
						EDITION=EDITION_ZONES;
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B014,false); //Particles
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B011,false); //Pathways						
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B004,false); //Nodes
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B007,false); //Lights
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B008,false); //Fogs
					}

				break;
				case DANAE_B007:

					if (EDITION==EDITION_LIGHTS)
					{
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B014,false); //Particles
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B011,false); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B010,false); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B004,false); //Nodes
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B007,false); //Lights
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B008,false); //Fogs
						EDITION=EDITION_IO;
					}
					else
					{
						EDITION=EDITION_LIGHTS;
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B014,false); //Particles
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B011,false); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B010,false); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B004,false); //Nodes
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B008,false); //Fogs
					}

				break;
				case DANAE_B006:

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

					if (EDITMODE==1) SetEditMode(0);

				break;
				case DANAE_B003:

					if (PauseScript==1) PauseScript=0;
					else PauseScript=1;

				break;
				case DANAE_B004:

					if (EDITION==EDITION_NODES)
					{
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B014,false); //Particles
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B011,false); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B010,false); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B004,false); //Nodes
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B007,false); //Lights
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B008,false); //Fogs
						EDITION=EDITION_IO;
					}
					else
					{
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B014,false); //Particles
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B011,false); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B010,false); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B007,false); //Lights
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B008,false); //Fogs
						EDITION=EDITION_NODES;
					}

				break;
				case DANAE_B008:

					if (EDITION == EDITION_FOGS)
					{
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B014,false); //Particles
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B011,false); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B010,false); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B004,false); //Nodes
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B007,false); //Lights
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B008,false); //Fogs
						EDITION=EDITION_IO;
					}
					else
					{
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B014,false); //Particles
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B011,false); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B010,false); //Zones
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B004,false); //Nodes
						SendMessage(danaeApp.ToolBar->hWnd,TB_CHECKBUTTON ,DANAE_B007,false); //Lights
						EDITION=EDITION_FOGS;
					}

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
							Pause(true);
							DialogBox( (HINSTANCE)GetWindowLongPtr( danaeApp.m_hWnd, GWLP_HINSTANCE ),
								MAKEINTRESOURCE(IDD_MESHREDUCTION), danaeApp.m_hWnd, MeshReductionProc);
							Pause(false);
							ARX_TIME_UnPause();				
						}
						else
						MESH_REDUCTION_WINDOW=(CreateDialogParam( (HINSTANCE)GetWindowLongPtr( danaeApp.m_hWnd, GWLP_HINSTANCE ),
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
					Pause(true);
					DialogBox( (HINSTANCE)GetWindowLongPtr( hWnd, GWLP_HINSTANCE ),
							   MAKEINTRESOURCE(IDD_OPTIONS), hWnd, OptionsProc );
					EERIE_LIGHT_ChangeLighting();
					Pause(false);
					ARX_TIME_UnPause();

				break;
				case DANAE_MENU_UNFREEZEALLINTER:
					ARX_INTERACTIVE_UnfreezeAll();
				break;
				case DANAE_MENU_RESETSHADOWS:
					ARX_TIME_Pause();
					Pause(true);

					if(OKBox("Remove Casts Shadows Flag from all Lights ?","DANAE Confirm Box")) {
						for(size_t i=0;i<MAX_LIGHTS;i++) {
							if(GLight[i]) {
								GLight[i]->extras |= EXTRAS_NOCASTED;
							}
						}
					}

					Pause(false);
					ARX_TIME_UnPause();
				break;
				case DANAE_MENU_RECALC:

					if (PRECALC==NULL)
					{
						if (danaeApp.m_pFramework->m_bIsFullscreen)
						{
							ARX_TIME_Pause();
							Pause(true);
							DialogBox( (HINSTANCE)GetWindowLongPtr( danaeApp.m_hWnd, GWLP_HINSTANCE ),
								MAKEINTRESOURCE(IDD_PRECALC), danaeApp.m_hWnd, PrecalcProc);
							Pause(false);
							ARX_TIME_UnPause();				
						}
						else
						PRECALC=(CreateDialogParam( (HINSTANCE)GetWindowLongPtr( danaeApp.m_hWnd, GWLP_HINSTANCE ),
							MAKEINTRESOURCE(IDD_PRECALC), danaeApp.m_hWnd, PrecalcProc,0 ));
					}

				break;
				case DANAE_MENU_LOCALLIST:

					if (ValidIONum(LastSelectedIONum))
					{
						ShowText = "";

						if (inter.iobj[LastSelectedIONum]->script.data!=NULL)
							MakeLocalText(&inter.iobj[LastSelectedIONum]->script,ShowText);
						else if (inter.iobj[LastSelectedIONum]->over_script.data!=NULL)
							MakeLocalText(&inter.iobj[LastSelectedIONum]->over_script,ShowText);

						ShowTextWindowtext = "Local Variables";
						DialogBox(hInstance, (LPCTSTR)IDD_SHOWTEXT, NULL, (DLGPROC)ShowTextDlg);					
					}
					else  
						LogError << ("No Interactive Object Selected");

				break;
				case DANAE_MENU_GLOBALLIST:
					ShowText = "";
					MakeGlobalText(ShowText);
					ShowTextWindowtext ="Global Variables";
					DialogBox(hInstance, (LPCTSTR)IDD_SHOWTEXT, NULL, (DLGPROC)ShowTextDlg);					
				break;
				case DANAE_MENU_INTEROBJLIST:
					LaunchInteractiveObjectsApp(this->m_hWnd);
				break;
				case DANAE_MENU_IMPORTSCN:
					ARX_TIME_Pause();
					Pause(true);
					LogError << ("Unavailable Command");
					Pause(false);
					ARX_TIME_UnPause();
				break;
				case DANAE_MENU_UPDATELOCALISATION:
					ARX_TIME_Pause();
					Pause(true);
					LocalisationInit();
					Pause(false);
					ARX_TIME_UnPause();
				break;
				case DANAE_MENU_UPDATESOUNDS:
					ARX_TIME_Pause();
					Pause(true);
					LogError << ("Unavailable Command");
					Pause(false);
					ARX_TIME_UnPause();
				break;
				case DANAE_MENU_UPDATESCENE:
					ARX_TIME_Pause();
					Pause(true);
					LogError << ("Unavailable Command");
					Pause(false);
					ARX_TIME_UnPause();
				break;
				case DANAE_MENU_UPDATEALLSCRIPTS:

					if (!EDITMODE)
						LogError << ("Command Only Available in EDITOR mode!!!");
					else
					{
						ARX_TIME_Pause();
						Pause(true);

						if (OKBox("Reload All Scripts ?","Confirm"))
						ReloadAllScripts();

						Pause(false);
						ARX_TIME_UnPause();
					}

				break;
				case DANAE_MENU_UPDATEALLOBJECTS:
					ARX_TIME_Pause();
					Pause(true);
					LogError << ("Unavailable Command");
					Pause(false);
					ARX_TIME_UnPause();
				break;
				case DANAE_MENU_UPDATEALLTEXTURES:
					ARX_TIME_Pause();
					Pause(true);
					//ReloadAllTextures();
					Pause(false);
					ARX_TIME_UnPause();
				break;
				case DANAE_MENU_UPDATEALLANIMS:

					if (!EDITMODE)
						LogError << ("Command Only Available in EDITOR mode!!!");
					else
					{
						ARX_TIME_Pause();
						Pause(true);
						EERIE_ANIMMANAGER_ReloadAll();
						Pause(false);
						ARX_TIME_UnPause();
					}

				break;
				case DANAE_MENU_ANIMATIONSLIST:
				{
					long tr;
					long memsize;
					ARX_TIME_Pause();
					Pause(true);
					tr=EERIE_ANIMMANAGER_Count(ShowText,&memsize);
					std::stringstream ss;
					ss << "Animations " << tr << ' ' << (memsize>>10) << " Ko";
					ShowTextWindowtext = ss.str();
					//sprintf(ShowTextWindowtext,"Animations %d %d Ko",tr,memsize>>10);
					DialogBox(hInstance, (LPCTSTR)IDD_SHOWTEXTBIG, NULL, (DLGPROC)ShowTextDlg);
					Pause(false);
					ARX_TIME_UnPause();
				}
				break;
				case DANAE_MENU_TEXLIST:
				{
					long _tr;
					long _memsize;
					long _memmip;
					ARX_TIME_Pause();
					Pause(true);
					_tr=CountTextures(ShowText,&_memsize,&_memmip);
					std::stringstream ss;
					ss << "Textures " << _tr << ' ' << (_memsize>>10) << " Ko MIPsize " << (_memmip>>10) << " Ko";
					ShowTextWindowtext = ss.str();
					//sprintf(ShowTextWindowtext,"Textures %d %d Ko MIPsize %d Ko",_tr,_memsize>>10,_memmip>>10);
					DialogBox(hInstance, (LPCTSTR)IDD_SHOWTEXTBIG, NULL, (DLGPROC)ShowTextDlg);
					Pause(false);
					ARX_TIME_UnPause();
				}
				break;
				case DANAE_MENU_PROJECTPATH:
					LogWarning << "not implemented";
					//HERMESFolderSelector("","Choose Working Folder"); first param receives folder
					//SetWindowTitle(hWnd,"DANAE Project");
					//chdir("GRAPH\\LEVELS\\");
					break;
				case DANAE_MENU_NEWLEVEL:
					ARX_TIME_Pause();
					Pause(true);

					if (OKBox("Do You Really Want to Start\na New Level ?","DANAE Confirm Box"))
						DanaeClearLevel();

					Pause(false);
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
					Pause(true);			
					LogError << ("Unavailable Command");
					Pause(false);
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
					Pause(true);
					DialogBox( (HINSTANCE)GetWindowLongPtr( hWnd, GWLP_HINSTANCE ),
							   MAKEINTRESOURCE(IDD_DANAEABOUT), hWnd, AboutProc );
					Pause(false);
					ARX_TIME_UnPause();
				break;
				case DANAE_MENU_OPTIONS:
					ARX_TIME_Pause();
					Pause(true);
					DialogBox( (HINSTANCE)GetWindowLongPtr( hWnd, GWLP_HINSTANCE ),
							   MAKEINTRESOURCE(IDD_OPTIONS), hWnd, OptionsProc );
					Pause(false);
					ARX_TIME_UnPause();
				break;
				case DANAE_MENU_OPTIONS2:
					ARX_TIME_Pause();
					Pause(true);
					DialogBox( (HINSTANCE)GetWindowLongPtr( this->m_hWnd, GWLP_HINSTANCE ),
							   MAKEINTRESOURCE(IDD_OPTIONS2), this->m_hWnd, OptionsProc_2 );
					Pause(false);
					ARX_TIME_UnPause();
				break;
			}
		}
		break;

#endif // BUILD_EDITOR

	}

	return CD3DApplication::MsgProc( hWnd, uMsg, wParam, lParam );
}

void ReleaseSystemObjects() {
	
	if(hero) {
		delete hero;
		hero=NULL;
	}

	if (inter.iobj[0]) {
		inter.iobj[0]->obj = NULL;
		ReleaseInter(inter.iobj[0]);
		inter.iobj[0] = NULL;

		if (inter.iobj)	{
			free(inter.iobj);
			inter.iobj = NULL;
		}
	}

	if(eyeballobj) {
		delete eyeballobj;
		eyeballobj = NULL;
	}

	if(cabal) {
		delete cabal;
		cabal = NULL;
	}

	if(nodeobj) {
		delete nodeobj;
		nodeobj = NULL;
	}

	if(fogobj) {
		delete fogobj;
		fogobj = NULL;
	}

	if(cameraobj) {
		delete cameraobj;
		cameraobj = NULL;
	}

	if(markerobj) {
		delete markerobj;
		markerobj = NULL;
	}

	if(arrowobj) {
		delete arrowobj;
		arrowobj = NULL;
	}

	for(size_t i = 0; i < MAX_GOLD_COINS_VISUALS; i++) {
		if(GoldCoinsObj[i]) {
			delete GoldCoinsObj[i];
			GoldCoinsObj[i] = NULL;
		}
	}
}

void ClearGame() {
	
	ARX_Menu_Resources_Release();
	ARX_TIME_UnPause();
	
	ShowWindow(danaeApp.m_hWnd, SW_MINIMIZE | SW_HIDE);
	
	ARX_MINIMAP_PurgeTC();
	
	if(DURING_LOCK) {
		danaeApp.Unlock();
	}
	
	KillInterfaceTextureContainers();
	Menu2_Close();
	DanaeClearLevel(2);
	TextureContainer::DeleteAll();

	if(ControlCinematique) {
		delete ControlCinematique;
		ControlCinematique=NULL;
	}
	
	//configuration
	config.save();
	
	RoomDrawRelease();
	EXITING=1;
	TREATZONE_Release();
	ClearTileLights();

	//texts and textures
	ClearSysTextures();
	FreeSaveGameList();

	if (pParticleManager) {
		delete pParticleManager;
		pParticleManager = NULL;
	}

	//sound
	ARX_SOUND_Release();
	MCache_ClearAll();

	//pathfinding
	ARX_PATH_ReleaseAllPath();
	ReleaseSystemObjects();
	
	//background
	ClearBackground(ACTIVEBKG);

	//animations
	EERIE_ANIMMANAGER_ClearAll();

	//Scripts
	if (svar) {
		for (long i=0; i<NB_GLOBALS; i++) {
			if (svar[i].text) {
				free(svar[i].text);
				svar[i].text=NULL;
			}
		}

		free(svar);
		svar=NULL;
	}

	ARX_SCRIPT_Timer_ClearAll();

	if(scr_timer) {
		delete[] scr_timer;
		scr_timer = NULL;
	}

	//Speech
	ARX_SPEECH_ClearAll();
	ARX_Text_Close();
	
	//object loaders from beforerun
	ReleaseDanaeBeforeRun();
	PAK_Close();
	
#ifdef BUILD_EDITOR
	if (danaeApp.ToolBar) {
		free(danaeApp.ToolBar);
		danaeApp.ToolBar=NULL;
	}
#endif
	
	ReleaseNode();
	
	//Halo
	ReleaseHalo();
	FreeSnapShot();
	ARX_INPUT_Release();
	
	danaeApp.Cleanup3DEnvironment();
	
	delete GRenderer;
	
	LogInfo << "Clean shutdown";
}
