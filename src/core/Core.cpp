/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include "core/Core.h"

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <sstream>
#include <vector>

#include <boost/algorithm/string/predicate.hpp>

#include "ai/Paths.h"
#include "ai/PathFinderManager.h"

#include "animation/Animation.h"
#include "animation/Cinematic.h"
#include "animation/CinematicKeyframer.h"

#include "core/Application.h"
#include "core/ArxGame.h"
#include "core/Config.h"
#include "core/Dialog.h"
#include "core/Localisation.h"
#include "core/GameTime.h"
#include "core/Version.h"

#include "game/Missile.h"
#include "game/Damage.h"
#include "game/Equipment.h"
#include "game/Player.h"
#include "game/Levels.h"
#include "game/Inventory.h"
#include "game/Spells.h"

#include "gui/MenuPublic.h"
#include "gui/Menu.h"
#include "gui/MenuWidgets.h"
#include "gui/Speech.h"
#include "gui/MiniMap.h"
#include "gui/TextManager.h"

#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Draw.h"
#include "graphics/GraphicsModes.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "graphics/Vertex.h"
#include "graphics/data/FTL.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/Fog.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/ParticleManager.h"
#include "graphics/texture/TextureStage.h"

#include "gui/Interface.h"
#include "gui/Text.h"

#include "input/Input.h"
#include "input/Keyboard.h"
#include "input/Mouse.h"

#include "io/FilePath.h"
#include "io/PakReader.h"
#include "io/CinematicLoad.h"
#include "io/Screenshot.h"
#include "io/log/FileLogger.h"
#include "io/log/Logger.h"

#include "math/Angle.h"
#include "math/Rectangle.h"
#include "math/Vector2.h"
#include "math/Vector3.h"
 
#include "physics/Collisions.h"
#include "physics/Attractors.h"

#include "platform/CrashHandler.h"
#include "platform/Flags.h"
#include "platform/Platform.h"

#include "scene/LinkedObject.h"
#include "scene/CinematicSound.h"
#include "scene/ChangeLevel.h"
#include "scene/Scene.h"
#include "scene/GameSound.h"
#include "scene/LoadLevel.h"
#include "scene/Interactive.h"
#include "scene/Light.h"
#include "scene/Object.h"

#include "script/Script.h"
#include "script/ScriptEvent.h"

#include "window/RenderWindow.h"

class TextManager;

using std::min;
using std::max;
using std::string;
using std::ostringstream;

#define MAX_EXPLO 24

void ClearGame();

extern TextManager	*pTextManage;
extern float FORCE_TIME_RESTORE;
extern CMenuState		*pMenu;
extern long SPECIAL_DRAGINTER_RENDER;
extern INTERACTIVE_OBJ * CURRENT_TORCH;
extern EERIE_3DOBJ * fogobj;
extern bool		bSkipVideoIntro;
extern string SCRIPT_SEARCH_TEXT;
extern string ShowText;
extern string ShowText2;
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
extern long		FAKE_DIR;
extern long		DONT_WANT_PLAYER_INZONE;
extern long		DeadTime;
extern long		INTREATZONECOUNT;
extern long		TOTPDL;
extern long		COLLIDED_CLIMB_POLY;
extern long LOOKING_FOR_SPELL_TARGET;
extern long PATHFINDER_WAIT;
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
extern float OLD_PROGRESS_BAR_COUNT;

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

extern EERIEMATRIX ProjectionMatrix;

//-----------------------------------------------------------------------------
// Our Main Danae Application.& Instance and Project
PROJECT Project;

//-----------------------------------------------------------------------------
Vec3f PUSH_PLAYER_FORCE;
Cinematic			*ControlCinematique=NULL;	// 2D Cinematic Controller
ParticleManager	*pParticleManager = NULL;
TextureContainer *  ombrignon = NULL;
TextureContainer *  teleportae = NULL;
TextureContainer *  Flying_Eye = NULL;
TextureContainer *	scursor[8];			// Animated Hand Cursor TC
TextureContainer *	pTCCrossHair;			// Animated Hand Cursor TC
TextureContainer *	iconequip[5];
TextureContainer *	GoldCoinsTC[MAX_GOLD_COINS_VISUALS]; // Gold Coins Icons
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
long ADDED_IO_NOT_SAVED = 0;
#endif

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

string WILLADDSPEECH;

Vec2s STARTDRAG;
INTERACTIVE_OBJ * COMBINE=NULL;

QUAKE_FX_STRUCT QuakeFx;
string LAST_FAILED_SEQUENCE = "none";
// START - Information for Player Teleport between/in Levels-------------------------------------
char TELEPORT_TO_LEVEL[64];
char TELEPORT_TO_POSITION[64];
long TELEPORT_TO_ANGLE;
// END -   Information for Player Teleport between/in Levels---------------------------------------
string WILL_LAUNCH_CINE;
char _CURRENTLOAD_[256];
fs::path LastLoadedScene;
string LAST_LAUNCHED_CINE;
float BASE_FOCAL=350.f;
float STRIKE_AIMTIME=0.f;
float SLID_VALUE=0.f;
float _framedelay;

float LASTfps2=0;
float fps2=0;
float fps2min=0;
long LASTfpscount=0;

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
float BOW_FOCAL=0;
long PlayerWeaponBlocked=-1;
long SHOW_TORCH=0;
float FrameDiff=0;
float GLOBAL_LIGHT_FACTOR=0.85f;

float IN_FRONT_DIVIDER_ITEMS	=0.7505f;
long USE_NEW_SKILLS=1;

long USE_LIGHT_OPTIM	=1;
// set to 0 for dev mode
long ALLOW_CHEATS		 =1;
long FOR_EXTERNAL_PEOPLE =0;
long FAST_SPLASHES		= 0;
long FINAL_RELEASE		= 0;
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

long CHANGE_LEVEL_ICON=-1;

long ARX_MOUSE_OVER=0;
//-----------------------------------------------------------------------------
// DEBUG FLAGS/Vars
//-----------------------------------------------------------------------------
long LaunchDemo=0;
long FirstFrame=1;
unsigned long WILLADDSPEECHTIME=0;
unsigned long AimTime;
float LastFrameTime=0;
float FrameTime=0;
unsigned long PlayerWeaponBlockTime=0;
unsigned long SPLASH_START=0;
//-----------------------------------------------------------------------------
Color3f FADECOLOR;

long START_NEW_QUEST=0;
long LAST_WEAPON_TYPE=-1;
long	FADEDURATION=0;
long	FADEDIR=0;
unsigned long FADESTART=0;

float Original_framedelay=0.f;

float PULSATE;
long EXITING=0;

long USE_PORTALS = 3;

Vec3f ePos;
extern EERIE_CAMERA * ACTIVECAM;

EERIE_CAMERA  * Kam;

//-----------------------------------------------------------------------------

void LoadSysTextures();
void ShowFPS();
void ManageNONCombatModeAnimations();

//-----------------------------------------------------------------------------


// Sends ON GAME_READY msg to all IOs
void SendGameReadyMsg()
{
	LogDebug("SendGameReadyMsg");
	SendMsgToAllIO(SM_GAME_READY);
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

static bool AdjustUI() {
	
	// Sets Danae Screen size depending on windowed/full-screen state
	DANAESIZX = mainApp->GetWindow()->GetSize().x;
	DANAESIZY = mainApp->GetWindow()->GetSize().y;
	
	// Now computes screen center
	DANAECENTERX = DANAESIZX>>1;
	DANAECENTERY = DANAESIZY>>1;
	
	// Computes X & Y screen ratios compared to a standard 640x480 screen
	Xratio = DANAESIZX * ( 1.0f / 640 );
	Yratio = DANAESIZY * ( 1.0f / 480 );
	
	if(!ARX_Text_Init()) {
		return false;
	}
	
	if(pMenu) {
		pMenu->bReInitAll=true;
	}
	
	return true;
}

void DanaeRestoreFullScreen() {
	
	if(pMenu) {
		pMenu->bReInitAll=true;
	}
	
	AdjustUI();

	LoadScreen();
}

//-----------------------------------------------------------------------------------------------

extern void InitTileLights();

//-----------------------------------------------------------------------------

void InitializeDanae()
{
	InitTileLights();
	
	ARX_MISSILES_ClearAll();
	ARX_SPELLS_Init();

	ARX_SPELLS_ClearAllSymbolDraw();
	ARX_PARTICLES_ClearAll();
	ARX_BOOMS_ClearAll();
	ARX_MAGICAL_FLARES_FirstInit();

	LastLoadedScene.clear();

	fs::path levelPath;
	fs::path levelFullPath;

	if(Project.demo != NOLEVEL) {
		char levelId[256];
		GetLevelNameByNum(Project.demo, levelId);
		levelPath = std::string("graph/levels/level") + levelId;
		levelFullPath = levelPath.string() + "/level" + levelId + ".dlf";
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

	if (LaunchDemo) {
		LogInfo << "Launching splash screens.";
		LaunchDemo=0;
		SPLASH_THINGS_STAGE=11;
	} else if (!levelPath.empty())	{
		LogInfo << "Launching Level " << levelPath;
		if (FastSceneLoad(levelPath)) {
			FASTmse = 1;
		} else {
#ifdef BUILD_EDIT_LOADSAVE
			ARX_SOUND_PlayCinematic("editor_humiliation.wav");
			mse = PAK_MultiSceneToEerie(levelPath);
#else
			LogError << "FastSceneLoad failed";
#endif
		}
		EERIEPOLY_Compute_PolyIn();
		LastLoadedScene = levelPath;
		USE_PLAYERCOLLISIONS=0;
	}
	
}

// Let's use main for now on all platforms
// TODO: On Windows, we might want to use WinMain in the Release target for example
int main(int argc, char ** argv) {
	
	(void)argc, (void)argv;
	
	initCrashHandler();
	
	Logger::init();
	
	Logger::add(new logger::File("arx.log", std::ios_base::out | std::ios_base::trunc));
	
	if(argc > 1 && boost::starts_with(argv[1], "--debug=")) {
		Logger::configure(argv[1] + 8);
	}
	
	FOR_EXTERNAL_PEOPLE = 1; // TODO remove this
	
	ALLOW_CHEATS = 0;
	FAST_SPLASHES = 0;
	FINAL_RELEASE = 1;
#ifdef BUILD_EDITOR
	GAME_EDITOR = 0;
	NEED_EDITOR = 0;
	TRUEFIGHT = 0;
#endif
	
	LogInfo << "Starting " << version;
	
	NOBUILDMAP=1;
	NOCHECKSUM=1;
	
	// TODO Time will be re-initialized later, but if we don't initialize it now casts to int might overflow.
	ARX_TIME_Init();
	
	mainApp = new ArxGame();
	if(!mainApp->Initialize()) {
		LogError << "Application failed to initialize properly";
		return -1;
	}
	
	ScriptEvent::init();
	
	CalcFPS(true);
	
	ARX_MAPMARKER_Init();
	
	memset(scursor, 0, sizeof(scursor));
	
	ARX_SPELLS_CancelSpellTarget();
	
	memset(explo, 0, sizeof(explo));
	
	USE_FAST_SCENES = 1;
	LogDebug("Danae Start");
	
	LogDebug("Project Init");
	
	NOCHECKSUM=0;
	
	ARX_INTERFACE_NoteInit();
	LogDebug("Note Init");
	PUSH_PLAYER_FORCE = Vec3f::ZERO;
	ARX_SPECIAL_ATTRACTORS_Reset();
	LogDebug("Attractors Init");
	ARX_SPELLS_Precast_Reset();
	LogDebug("Spell Init");
	
	for(size_t t = 0; t < MAX_GOLD_COINS_VISUALS; t++) {
		GoldCoinsObj[t]=NULL;
		GoldCoinsTC[t]=NULL;
	}
	
	LogDebug("LSV Init");
	ModeLight=MODE_DYNAMICLIGHT | MODE_DEPTHCUEING;
	
	memset(&DefaultBkg,0,sizeof(EERIE_BACKGROUND));
	memset(TELEPORT_TO_LEVEL,0,64);
	memset(TELEPORT_TO_POSITION,0,64);
	LogDebug("Mset");
	
	LogDebug("AnimManager Init");
	ARX_SCRIPT_EventStackInit();
	LogDebug("EventStack Init");
	ARX_EQUIPMENT_Init();
	LogDebug("AEQ Init");
	memset(_CURRENTLOAD_,0,256);
	
	ARX_SCRIPT_Timer_FirstInit(512);
	LogDebug("Timer Init");
	ARX_FOGS_FirstInit();
	LogDebug("Fogs Init");
	
	EERIE_LIGHT_GlobalInit();
	LogDebug("Lights Init");
	
	LogDebug("Svars Init");
	
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
	
	LogDebug("Launching DANAE");
	
	memset(&Project, 0, sizeof(PROJECT));
	
	if (FINAL_RELEASE) {
		LogDebug("FINAL_RELEASE");
		LaunchDemo=1;
		Project.demo=LEVEL10;
		NOCHECKSUM=1;
	} else {
		LogInfo << "default LEVELDEMO2";
		Project.demo=LEVELDEMO2;
	}
	
	LogDebug("After Popup");
	atexit(ClearGame);
	
	if(LaunchDemo) {
		
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
	
	if(!AdjustUI()) {
		return -1;
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
	ARXMenu_Options_Control_SetMouseSensitivity(config.input.mouseSensitivity);
	
	if(config.video.textureSize==2)Project.TextureSize=0;
	if(config.video.textureSize==1)Project.TextureSize=2;
	if(config.video.textureSize==0)Project.TextureSize=64;
	
	ARX_MINIMAP_FirstInit();
	
	Project.torch.r=1.f;
	Project.torch.g = 0.8f;
	Project.torch.b = 0.66666f;
	LogDebug("InitializeDanae");
	InitializeDanae();
	
	PakReader::ReleaseFlags rel = resources->getReleaseType();
	switch(rel) {
		case 0: LogWarning << "Neither demo nor full game data files loaded."; break;
		case PakReader::Demo: LogInfo << "Initialized Arx Fatalis (demo)"; break;
		case PakReader::FullGame: LogInfo << "Initialized Arx Fatalis (full game)"; break;
		case (int(PakReader::Demo) | int(PakReader::FullGame)): LogWarning << "Mixed demo and full game data files!"; break;
		default: ARX_DEAD_CODE();
	}
	
	mainApp->m_bReady = true;
	
	// Init all done, start the main loop
	mainApp->Run();
	
	return true;
}

//*************************************************************************************
// INTERACTIVE_OBJ * FlyingOverObject(EERIE_S2D * pos)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Returns IO under cursor, be it in inventories or in scene
//   Returns NULL if no IO under cursor
//*************************************************************************************
INTERACTIVE_OBJ * FlyingOverObject(Vec2s * pos)
{
	INTERACTIVE_OBJ* io = NULL;

	if ((io = GetFromInventory(pos)) != NULL)
		return io;
	if (InInventoryPos(pos))
		return NULL;

	if ((io = InterClick(pos)) != NULL)
		return io;

	return NULL;
}

extern unsigned long FALLING_TIME;
extern long ARX_NPC_ApplyCuts(INTERACTIVE_OBJ * io);

//*************************************************************************************

void LoadSysTextures()
{
	char temp[256];

	for (long i=1;i<10;i++)
	{
		sprintf(temp,"graph/particles/shine%ld", i);
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
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_magic_sight");
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
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_magic_missile");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_TAAR;

	// Ignit Level 1
	current=&spellicons[SPELL_IGNIT];	
	current->name = getLocalised("system_spell_name_ignit");
	current->description = getLocalised("system_spell_description_ignit");
	current->level=1;
	current->spellid=SPELL_IGNIT;
	current->bDuration = false;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_ignite");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_YOK;

	// Douse Level 1
	current=&spellicons[SPELL_DOUSE];	
	current->name = getLocalised("system_spell_name_douse");
	current->description = getLocalised("system_spell_description_douse");
	current->level=1;
	current->spellid=SPELL_DOUSE;
	current->bDuration = false;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_douse");
	current->symbols[0]=RUNE_NHI;
	current->symbols[1]=RUNE_YOK;

	// Activate_Portal Level 1
	current=&spellicons[SPELL_ACTIVATE_PORTAL];
	current->name = getLocalised("system_spell_name_activate_portal");
	current->description = getLocalised("system_spell_description_activate_portal");
	current->level=1;
	current->spellid=SPELL_ACTIVATE_PORTAL;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_activate_portal");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_SPACIUM;
	current->bSecret = true;

	// Heal Level 2
	current=&spellicons[SPELL_HEAL];	
	current->name = getLocalised("system_spell_name_heal");
	current->description = getLocalised("system_spell_description_heal");
	current->level=2;
	current->spellid=SPELL_HEAL;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_heal");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_VITAE;

	// Detect_trap Level 2
	current=&spellicons[SPELL_DETECT_TRAP];	
	current->name = getLocalised("system_spell_name_detect_trap");
	current->description = getLocalised("system_spell_description_detect_trap");
	current->level=2;
	current->spellid=SPELL_DETECT_TRAP;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_detect_trap");
	current->symbols[0]=RUNE_MORTE;
	current->symbols[1]=RUNE_COSUM;
	current->symbols[2]=RUNE_VISTA;

	// Armor Level 2
	current=&spellicons[SPELL_ARMOR];	
	current->name = getLocalised("system_spell_name_armor");
	current->description = getLocalised("system_spell_description_armor");
	current->level=2;
	current->spellid=SPELL_ARMOR;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_armor");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_KAOM;

	// Lower Armor Level 2
	current=&spellicons[SPELL_LOWER_ARMOR];	
	current->name = getLocalised("system_spell_name_lower_armor");
	current->description = getLocalised("system_spell_description_lower_armor");
	current->level=2;
	current->spellid=SPELL_LOWER_ARMOR;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_lower_armor");
	current->symbols[0]=RUNE_RHAA;
	current->symbols[1]=RUNE_KAOM;

	// Harm Level 2
	current=&spellicons[SPELL_HARM];	
	current->name = getLocalised("system_spell_name_harm");
	current->description = getLocalised("system_spell_description_harm");
	current->level=2;
	current->spellid=SPELL_HARM;
	current->bAudibleAtStart = true;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_harm");
	current->symbols[0]=RUNE_RHAA;
	current->symbols[1]=RUNE_VITAE;
	current->bSecret = true;

	// Speed Level 3
	current=&spellicons[SPELL_SPEED];	
	current->name = getLocalised("system_spell_name_speed");
	current->description = getLocalised("system_spell_description_speed");
	current->level=3;
	current->spellid=SPELL_SPEED;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_speed");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_MOVIS;

	// Reveal Level 3
	current=&spellicons[SPELL_DISPELL_ILLUSION];	
	current->name = getLocalised("system_spell_name_reveal");
	current->description = getLocalised("system_spell_description_reveal");
	current->level=3;
	current->spellid=SPELL_DISPELL_ILLUSION;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_reveal");
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
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_fireball");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_YOK;
	current->symbols[2]=RUNE_TAAR;

	// Create Food Level 3
	current=&spellicons[SPELL_CREATE_FOOD];	
	current->name = getLocalised("system_spell_name_create_food");
	current->description = getLocalised("system_spell_description_create_food");
	current->level=3;
	current->spellid=SPELL_CREATE_FOOD;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_create_food");
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
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_iceball");
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
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_bless");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_STREGUM;
	current->symbols[2]=RUNE_VITAE;

	// Dispel_Field Level 4
	current=&spellicons[SPELL_DISPELL_FIELD];	
	current->name = getLocalised("system_spell_name_dispell_field");
	current->description = getLocalised("system_spell_description_dispell_field");
	current->level=4;
	current->spellid=SPELL_DISPELL_FIELD;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_dispell_field");
	current->symbols[0]=RUNE_NHI;

	current->symbols[1]=RUNE_SPACIUM;

	// Cold Protection Level 4
	current=&spellicons[SPELL_COLD_PROTECTION];	
	current->name = getLocalised("system_spell_name_cold_protection");
	current->description = getLocalised("system_spell_description_cold_protection");
	current->level=4;
	current->spellid=SPELL_COLD_PROTECTION;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_protection_cold");
	current->symbols[0]=RUNE_FRIDD;
	current->symbols[1]=RUNE_KAOM;
	current->bSecret = true;

	// Fire Protection Level 4
	current=&spellicons[SPELL_FIRE_PROTECTION];	
	current->name = getLocalised("system_spell_name_fire_protection");
	current->description = getLocalised("system_spell_description_fire_protection");
	current->level=4;
	current->spellid=SPELL_FIRE_PROTECTION;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_protection_fire");
	current->symbols[0]=RUNE_YOK;
	current->symbols[1]=RUNE_KAOM;

	// Telekinesis Level 4
	current=&spellicons[SPELL_TELEKINESIS];	
	current->name = getLocalised("system_spell_name_telekinesis");
	current->description = getLocalised("system_spell_description_telekinesis");
	current->level=4;
	current->spellid=SPELL_TELEKINESIS;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_telekinesis");
	current->symbols[0]=RUNE_SPACIUM;
	current->symbols[1]=RUNE_COMUNICATUM;

	// Curse Level 4
	current=&spellicons[SPELL_CURSE];	
	current->name = getLocalised("system_spell_name_curse");
	current->description = getLocalised("system_spell_description_curse");
	current->level=4;
	current->spellid=SPELL_CURSE;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_curse");
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
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_rune_guarding");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_MORTE;
	current->symbols[2]=RUNE_COSUM;

	// Levitate Level 5
	current=&spellicons[SPELL_LEVITATE];	
	current->name = getLocalised("system_spell_name_levitate");
	current->description = getLocalised("system_spell_description_levitate");
	current->level=5;
	current->spellid=SPELL_LEVITATE;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_levitate");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_SPACIUM;
	current->symbols[2]=RUNE_MOVIS;

	// Cure Poison Level 5
	current=&spellicons[SPELL_CURE_POISON];	
	current->name = getLocalised("system_spell_name_cure_poison");
	current->description = getLocalised("system_spell_description_cure_poison");
	current->level=5;
	current->spellid=SPELL_CURE_POISON;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_cure_poison");
	current->symbols[0]=RUNE_NHI;
	current->symbols[1]=RUNE_CETRIUS;

	// Repel Undead Level 5
	current=&spellicons[SPELL_REPEL_UNDEAD];	
	current->name = getLocalised("system_spell_name_repel_undead");
	current->description = getLocalised("system_spell_description_repel_undead");
	current->level=5;
	current->spellid=SPELL_REPEL_UNDEAD;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_repel_undead");
	current->symbols[0]=RUNE_MORTE;
	current->symbols[1]=RUNE_KAOM;

	// Poison Projection Level 5
	current=&spellicons[SPELL_POISON_PROJECTILE];	
	current->name = getLocalised("system_spell_name_poison_projection");
	current->description = getLocalised("system_spell_description_poison_projection");
	current->level=5;
	current->spellid=SPELL_POISON_PROJECTILE;
	current->bDuration = false;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_poison_projection");
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
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_raise_dead");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_MORTE;
	current->symbols[2]=RUNE_VITAE;

	// Paralyse Dead Level 6
	current=&spellicons[SPELL_PARALYSE];	
	current->name = getLocalised("system_spell_name_paralyse");
	current->description = getLocalised("system_spell_description_paralyse");
	current->level=6;
	current->spellid=SPELL_PARALYSE;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_paralyse");
	current->symbols[0]=RUNE_NHI;
	current->symbols[1]=RUNE_MOVIS;

	// Create Field Dead Level 6
	current=&spellicons[SPELL_CREATE_FIELD];	
	current->name = getLocalised("system_spell_name_create_field");
	current->description = getLocalised("system_spell_description_create_field");
	current->level=6;
	current->spellid=SPELL_CREATE_FIELD;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_create_field");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_KAOM;
	current->symbols[2]=RUNE_SPACIUM;

	// Disarm Trap Level 6
	current=&spellicons[SPELL_DISARM_TRAP];	
	current->name = getLocalised("system_spell_name_disarm_trap");
	current->description = getLocalised("system_spell_description_disarm_trap");
	current->level=6;
	current->spellid=SPELL_DISARM_TRAP;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_disarm_trap");
	current->symbols[0]=RUNE_NHI;
	current->symbols[1]=RUNE_MORTE;
	current->symbols[2]=RUNE_COSUM;

	// Slow_Down Level 6 // SECRET SPELL
	current=&spellicons[SPELL_SLOW_DOWN];	
	current->name = getLocalised("system_spell_name_slowdown");
	current->description = getLocalised("system_spell_description_slowdown");
	current->level=6;
	current->spellid=SPELL_SLOW_DOWN;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_slow_down");
	current->symbols[0]=RUNE_RHAA;
	current->symbols[1]=RUNE_MOVIS;
	current->bSecret = true;

	// Flying Eye Level 7
	current=&spellicons[SPELL_FLYING_EYE];	
	current->name = getLocalised("system_spell_name_flying_eye");
	current->description = getLocalised("system_spell_description_flying_eye");
	current->level=7;
	current->spellid=SPELL_FLYING_EYE;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_flying_eye");
	current->symbols[0]=RUNE_VISTA;
	current->symbols[1]=RUNE_MOVIS;

	// Fire Field Eye Level 7
	current=&spellicons[SPELL_FIRE_FIELD];	
	current->name = getLocalised("system_spell_name_fire_field");
	current->description = getLocalised("system_spell_description_fire_field");
	current->level=7;
	current->spellid=SPELL_FIRE_FIELD;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_create_fire_field");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_YOK;
	current->symbols[2]=RUNE_SPACIUM;

	// Ice Field Level 7
	current=&spellicons[SPELL_ICE_FIELD];	
	current->name = getLocalised("system_spell_name_ice_field");
	current->description = getLocalised("system_spell_description_ice_field");
	current->level=7;
	current->spellid=SPELL_ICE_FIELD;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_create_cold_field");
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
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_lightning_strike");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_FOLGORA;
	current->symbols[2]=RUNE_TAAR;

	// Confusion Level 7
	current=&spellicons[SPELL_CONFUSE];	
	current->name = getLocalised("system_spell_name_confuse");
	current->description = getLocalised("system_spell_description_confuse");
	current->level=7;
	current->spellid=SPELL_CONFUSE;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_confuse");
	current->symbols[0]=RUNE_RHAA;
	current->symbols[1]=RUNE_VISTA;

	// Invisibility Level 8
	current=&spellicons[SPELL_INVISIBILITY];	
	current->name = getLocalised("system_spell_name_invisibility");
	current->description = getLocalised("system_spell_description_invisibility");
	current->level=8;
	current->spellid=SPELL_INVISIBILITY;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_invisibility");
	current->symbols[0]=RUNE_NHI;
	current->symbols[1]=RUNE_VISTA;

	// Mana Drain Level 8
	current=&spellicons[SPELL_MANA_DRAIN];	
	current->name = getLocalised("system_spell_name_mana_drain");
	current->description = getLocalised("system_spell_description_mana_drain");
	current->level=8;
	current->spellid=SPELL_MANA_DRAIN;
	current->bAudibleAtStart = true;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_drain_mana");
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
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_explosion");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_MEGA;
	current->symbols[2]=RUNE_MORTE;

	// Enchant Weapon Level 8
	current=&spellicons[SPELL_ENCHANT_WEAPON];	
	current->name = getLocalised("system_spell_name_enchant_weapon");
	current->description = getLocalised("system_spell_description_enchant_weapon");
	current->level=8;
	current->spellid=SPELL_ENCHANT_WEAPON;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_enchant_weapon");
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
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_drain_life");
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
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_summon_creature");
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
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_summon_creature");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_VITAE;
	current->symbols[2]=RUNE_TERA;

	// Negate Magic Level 9
	current=&spellicons[SPELL_NEGATE_MAGIC];	
	current->name = getLocalised("system_spell_name_negate_magic");
	current->description = getLocalised("system_spell_description_negate_magic");
	current->level=9;
	current->spellid=SPELL_NEGATE_MAGIC;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_negate_magic");
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
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_incinerate");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_MEGA;
	current->symbols[2]=RUNE_YOK;

	// Mass paralyse Creature Level 9
	current=&spellicons[SPELL_MASS_PARALYSE];	
	current->name = getLocalised("system_spell_name_mass_paralyse");
	current->description = getLocalised("system_spell_description_mass_paralyse");
	current->level=9;
	current->spellid=SPELL_MASS_PARALYSE;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_mass_paralyse");
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
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_mass_lighting_strike");
	current->symbols[0]=RUNE_AAM;
	current->symbols[1]=RUNE_FOLGORA;
	current->symbols[2]=RUNE_SPACIUM;

	// Control Target Level 10
	current=&spellicons[SPELL_CONTROL_TARGET];	
	current->name = getLocalised("system_spell_name_control_target");
	current->description = getLocalised("system_spell_description_control_target");
	current->level=10;
	current->spellid=SPELL_CONTROL_TARGET;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_control_target");
	current->symbols[0]=RUNE_MOVIS;
	current->symbols[1]=RUNE_COMUNICATUM;

	// Freeze time Level 10
	current=&spellicons[SPELL_FREEZE_TIME];	
	current->name = getLocalised("system_spell_name_freeze_time");
	current->description = getLocalised("system_spell_description_freeze_time");
	current->level=10;
	current->spellid=SPELL_FREEZE_TIME;
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_freeze_time");
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
	current->tc=TextureContainer::LoadUI("graph/interface/icons/spell_mass_incinerate");
	current->symbols[0]=RUNE_MEGA;
	current->symbols[1]=RUNE_AAM;
	current->symbols[2]=RUNE_MEGA;
	current->symbols[3]=RUNE_YOK;

	Flying_Eye=			TextureContainer::LoadUI("graph/particles/flying_eye_fx");
	specular=			TextureContainer::LoadUI("graph/particles/specular");
	enviro=				TextureContainer::LoadUI("graph/particles/enviro");
	sphere_particle=	TextureContainer::LoadUI("graph/particles/sphere");
	inventory_font=		TextureContainer::LoadUI("graph/interface/font/font10x10_inventory");
	npc_fight=			TextureContainer::LoadUI("graph/interface/icons/follower_attack");
	npc_follow=			TextureContainer::LoadUI("graph/interface/icons/follower_follow");
	npc_stop=			TextureContainer::LoadUI("graph/interface/icons/follower_stop");
	flaretc.lumignon=	TextureContainer::LoadUI("graph/particles/lumignon");
	flaretc.lumignon2=	TextureContainer::LoadUI("graph/particles/lumignon2");
	flaretc.plasm=		TextureContainer::LoadUI("graph/particles/plasm");
	tflare=				TextureContainer::LoadUI("graph/particles/flare");
	ombrignon=			TextureContainer::LoadUI("graph/particles/ombrignon");
	teleportae=			TextureContainer::LoadUI("graph/particles/teleportae");
	TC_fire=			TextureContainer::LoadUI("graph/particles/fire");
	TC_fire2=			TextureContainer::LoadUI("graph/particles/fire2");
	TC_smoke=			TextureContainer::LoadUI("graph/particles/smoke");
	TextureContainer::LoadUI("graph/particles/missile");
	Z_map=				TextureContainer::LoadUI("graph/interface/misc/z-map");
	Boom=				TextureContainer::LoadUI("graph/particles/boom");
	lightsource_tc=		TextureContainer::LoadUI("graph/particles/light");
	stealth_gauge_tc=	TextureContainer::LoadUI("graph/interface/icons/stealth_gauge");
	arx_logo_tc=		TextureContainer::LoadUI("graph/interface/icons/arx_logo_32");
	iconequip[0]=		TextureContainer::LoadUI("graph/interface/icons/equipment_sword");
	iconequip[1]=		TextureContainer::LoadUI("graph/interface/icons/equipment_shield");
	iconequip[2]=		TextureContainer::LoadUI("graph/interface/icons/equipment_helm");
	iconequip[3]=		TextureContainer::LoadUI("graph/interface/icons/equipment_chest");
	iconequip[4]=		TextureContainer::LoadUI("graph/interface/icons/equipment_leggings");
	mecanism_tc=		TextureContainer::LoadUI("graph/interface/cursors/mecanism");
	arrow_left_tc=		TextureContainer::LoadUI("graph/interface/icons/arrow_left");

	for (i=0;i<MAX_EXPLO;i++)
	{
		char temp[256];
		sprintf(temp,"graph/particles/fireb_%02ld",i+1);
		explo[i]= TextureContainer::LoadUI(temp);
	}

	blood_splat=TextureContainer::LoadUI("graph/particles/new_blood2");

	EERIE_DRAW_SetTextureZMAP(Z_map);
	EERIE_DRAW_sphere_particle=sphere_particle;
	EERIE_DRAW_square_particle=TextureContainer::LoadUI("graph/particles/square");

	TextureContainer::LoadUI("graph/particles/fire_hit");
	TextureContainer::LoadUI("graph/particles/light");
	TextureContainer::LoadUI("graph/particles/blood01");
	TextureContainer::LoadUI("graph/particles/cross");

	//INTERFACE LOADING
	TextureContainer::LoadUI("graph/interface/bars/empty_gauge_red");
	TextureContainer::LoadUI("graph/interface/bars/empty_gauge_blue");
	TextureContainer::LoadUI("graph/interface/bars/filled_gauge_blue");
	TextureContainer::LoadUI("graph/interface/bars/filled_gauge_red");
	TextureContainer::LoadUI("graph/interface/icons/book");
	TextureContainer::LoadUI("graph/interface/icons/backpack");
	TextureContainer::LoadUI("graph/interface/icons/lvl_up");
	TextureContainer::LoadUI("graph/interface/icons/steal");
	TextureContainer::LoadUI("graph/interface/icons/cant_steal_item");
	TextureContainer::LoadUI("graph/interface/inventory/hero_inventory");
	TextureContainer::LoadUI("graph/interface/inventory/scroll_up");
	TextureContainer::LoadUI("graph/interface/inventory/scroll_down");
	TextureContainer::LoadUI("graph/interface/inventory/hero_inventory_link");
	TextureContainer::LoadUI("graph/interface/inventory/ingame_inventory");
	TextureContainer::LoadUI("graph/interface/inventory/gold");

	TextureContainer::LoadUI("graph/interface/inventory/inv_pick");
	TextureContainer::LoadUI("graph/interface/inventory/inv_close");

	// MENU2
	TextureContainer::LoadUI("graph/interface/cursors/cursor00");
	TextureContainer::LoadUI("graph/interface/cursors/cursor01");
	TextureContainer::LoadUI("graph/interface/cursors/cursor02");
	TextureContainer::LoadUI("graph/interface/cursors/cursor03");
	TextureContainer::LoadUI("graph/interface/cursors/cursor04");
	TextureContainer::LoadUI("graph/interface/cursors/cursor05");
	TextureContainer::LoadUI("graph/interface/cursors/cursor06");
	TextureContainer::LoadUI("graph/interface/cursors/cursor07");
	TextureContainer::LoadUI("graph/interface/cursors/cruz");
	TextureContainer::LoadUI("graph/interface/menus/menu_main_background", TextureContainer::NoColorKey);
	TextureContainer::LoadUI("graph/interface/menus/menu_console_background");
	TextureContainer::LoadUI("graph/interface/menus/menu_console_background_border");

	//CURSORS LOADING
	TextureContainer::LoadUI("graph/interface/cursors/cursor");
	TextureContainer::LoadUI("graph/interface/cursors/magic");
	TextureContainer::LoadUI("graph/interface/cursors/interaction_on");
	TextureContainer::LoadUI("graph/interface/cursors/interaction_off");
	TextureContainer::LoadUI("graph/interface/cursors/target_on");
	TextureContainer::LoadUI("graph/interface/cursors/target_off");
	TextureContainer::LoadUI("graph/interface/cursors/drop");
	TextureContainer::LoadUI("graph/interface/cursors/throw");

	for (i=0;i<8;i++)
	{
		char temp[256];
		sprintf(temp,"graph/interface/cursors/cursor%02ld",i);
		scursor[i]=TextureContainer::LoadUI(temp);
	}

	pTCCrossHair = TextureContainer::LoadUI("graph/interface/cursors/cruz");

	TextureContainer::LoadUI("graph/interface/bars/aim_empty");
	TextureContainer::LoadUI("graph/interface/bars/aim_maxi");
	TextureContainer::LoadUI("graph/interface/bars/flash_gauge");
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

static void PlayerLaunchArrow_Test(float aimratio, float poisonous, Vec3f * pos, Anglef * angle) {
	
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

extern unsigned long LAST_JUMP_ENDTIME;

//*************************************************************************************
// Switches from/to Game Mode/Editor Mode
//*************************************************************************************
void SetEditMode(long ed, const bool stop_sound) {
	
	LAST_JUMP_ENDTIME = 0;
	
	if(!DONT_ERASE_PLAYER) {
		player.life = 0.1f;
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
		if(!FINAL_RELEASE)
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
			if(!FINAL_RELEASE)
				ARX_PLAYER_MakePowerfullHero();
			else
				ARX_PLAYER_MakeFreshHero();
		}
	}

	InitSnapShot("snapshot");
}
Vec3f LastValidPlayerPos;
Vec3f	WILL_RESTORE_PLAYER_POSITION;
long WILL_RESTORE_PLAYER_POSITION_FLAG=0;
extern long FLAG_ALLOW_CLOTHES;

//*************************************************************************************

void FirstFrameHandling()
{	
	LogDebug("FirstFrameHandling");
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
		Mscenepos.x=-mse->cub.xmin-(mse->cub.xmax-mse->cub.xmin)*.5f+((float)ACTIVEBKG->Xsize*(float)ACTIVEBKG->Xdiv)*.5f;
		Mscenepos.z=-mse->cub.zmin-(mse->cub.zmax-mse->cub.zmin)*.5f+((float)ACTIVEBKG->Zsize*(float)ACTIVEBKG->Zdiv)*.5f;
		float t1=(float)(long)(mse->point0.x/BKG_SIZX);
		float t2=(float)(long)(mse->point0.z/BKG_SIZZ);
		t1=mse->point0.x-t1*BKG_SIZX;
		t2=mse->point0.z-t2*BKG_SIZZ;
		Mscenepos.x=(float)((long)(Mscenepos.x/BKG_SIZX))*BKG_SIZX+(float)BKG_SIZX*.5f;
		Mscenepos.z=(float)((long)(Mscenepos.z/BKG_SIZZ))*BKG_SIZZ+(float)BKG_SIZZ*.5f;
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
	CURRENTLEVEL=GetLevelNumByName(LastLoadedScene.string());
	
#ifdef BUILD_EDITOR
	iCreateMap=0;
	if ((CURRENTLEVEL>=0) && !(NOBUILDMAP) && GAME_EDITOR)
	{
		//if (NeedMapCreation())	
		//	iCreateMap=1;
		//else
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

	long t = inter.getById("seat_stool1_0012");
	if(ValidIONum(t)) {
		inter.iobj[t]->ioflags |= IO_FORCEDRAW;
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
						if(io->short_name() == "arrows") {
							if(io->durability >= 1.f) {
								count += checked_range_cast<long>(io->durability);
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
						if(ioo->short_name() == "arrows") {
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

void strikeSpeak(INTERACTIVE_OBJ * io) {
	
	if(!StrikeAimtime()) {
		return;
	}
	
	const string * str;
	long equiped = player.equiped[EQUIP_SLOT_WEAPON];
	if(equiped != 0 && !inter.iobj[equiped]->strikespeech.empty()) {
		str = &inter.iobj[equiped]->strikespeech;
	} else if(!io->strikespeech.empty()) {
		str = &io->strikespeech;
	} else {
		return;
	}
	
	ARX_SPEECH_AddSpeech(io, *str, ANIM_TALK_NEUTRAL, ARX_SPEECH_FLAG_NOTEXT);
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

					strikeSpeak(io);

					SendIOScriptEvent(io,SM_STRIKE,"bare");
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

					strikeSpeak(io);

					SendIOScriptEvent(io,SM_STRIKE,"dagger");
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

					strikeSpeak(io);

					SendIOScriptEvent(io,SM_STRIKE,"1h");
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

					strikeSpeak(io);

					SendIOScriptEvent(io,SM_STRIKE,"2h");
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

				EERIE_LINKEDOBJ_LinkObjectToObject(io->obj, arrowobj, "left_attach", "attach", NULL);

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
				SendIOScriptEvent(io,SM_STRIKE,"bow");
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
		if (GRenderer && Mr_tc)
		{
			EERIEDrawBitmap(DANAESIZX-(128.f*Xratio), 0.f, (float)128*Xratio, (float)128*Yratio,0.0001f,
			                Mr_tc, Color::gray(0.5f + PULSATE * (1.0f/10)));
		}
		else
		{
			Mr_tc=TextureContainer::LoadUI("graph/particles/(fx)_mr");
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

			if ((nodeobj->vertexlist[nodeobj->origin].vert.p.z>0.f) && (nodeobj->vertexlist[nodeobj->origin].vert.p.z<0.9f))
			{
				xx=nodeobj->vertexlist[nodeobj->origin].vert.p.x-40.f;
				yy=nodeobj->vertexlist[nodeobj->origin].vert.p.y-40.f;
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
		QuakeFx.frequency*=.5f;
		QuakeFx.flags|=flags;

		if (flags & 1)
			ARX_SOUND_PlaySFX(SND_QUAKE, NULL, 1.0F - 0.5F * QuakeFx.intensity);
	}
	else
	{
		QuakeFx.intensity=intensity;

		QuakeFx.start = checked_range_cast<unsigned long>(FrameTime);

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
		float halfpower=truepower*.5f;
		ACTIVECAM->pos.x+=rnd()*truepower-halfpower;
		ACTIVECAM->pos.y+=rnd()*truepower-halfpower;
		ACTIVECAM->pos.z+=rnd()*truepower-halfpower;
		ACTIVECAM->angle.a+=rnd()*truepower-halfpower;
		ACTIVECAM->angle.g+=rnd()*truepower-halfpower;
		ACTIVECAM->angle.b+=rnd()*truepower-halfpower;
	}
}

void DANAE_StartNewQuest()
{
	player.Interface = INTER_LIFE_MANA | INTER_MINIBACK | INTER_MINIBOOK;
	PROGRESS_BAR_TOTAL = 108;
	OLD_PROGRESS_BAR_COUNT=PROGRESS_BAR_COUNT=0;
	LoadLevelScreen(1);
	char loadfrom[256];
	sprintf(loadfrom, "graph/levels/level1/level1.dlf");
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

bool DANAE_ManageSplashThings() {
	
	GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapClamp);
	
	if(SPLASH_THINGS_STAGE > 10) {
		
		GInput->update();
		
		if(GInput->isAnyKeyPressed()) {
			REFUSE_GAME_RETURN = 1;
			FORBID_SAVE = 0;
			FirstFrame=  1;
			SPLASH_THINGS_STAGE = 0;
			ARXmenu.currentmode = AMCM_MAIN;
			ARX_MENU_Launch();
		}
		
		if(GInput->isKeyPressed(Keyboard::Key_Escape)) {
			REFUSE_GAME_RETURN=1;
			SPLASH_THINGS_STAGE = 14;
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
			sprintf(loadfrom,"graph/levels/level10/level10.dlf");
			OLD_PROGRESS_BAR_COUNT=PROGRESS_BAR_COUNT=0;
			PROGRESS_BAR_TOTAL = 108;
			LoadLevelScreen(10);	

			DanaeLoadLevel(loadfrom);
			FORBID_SAVE=0;
			FirstFrame=1;
			SPLASH_THINGS_STAGE=0;

			GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);
			return true;
			
		}

		if (SPLASH_THINGS_STAGE > 13)
		{
			FORBID_SAVE=0;
			FirstFrame=1;
			SPLASH_THINGS_STAGE=0;

			GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);
			return true;
		}
	}

	GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);
	return false;
}

void LaunchWaitingCine() {
	
	LogDebug("LaunchWaitingCine " << CINE_PRELOAD);

	if(ACTIVECAM) {
		ePos = ACTIVECAM->pos;
	}
	
	DANAE_KillCinematic();
	
	fs::path cinematic = fs::path("graph/interface/illustrations") / WILL_LAUNCH_CINE;
	
	if(resources->getFile(cinematic)) {
		
		ControlCinematique->OneTimeSceneReInit();
		
		if(loadCinematic(ControlCinematique, cinematic)) {
			
			if(CINE_PRELOAD) {
				LogDebug("only preloaded cinematic");
				PLAY_LOADED_CINEMATIC = 0;
			} else {
				LogDebug("starting cinematic");
				PLAY_LOADED_CINEMATIC = 1;
				ARX_TIME_Pause();
			}
			
			LAST_LAUNCHED_CINE = WILL_LAUNCH_CINE;
		} else {
			LogWarning << "error loading cinematic " << cinematic;
		}
		
	} else {
		LogWarning << "could not find cinematic " << cinematic;
	}
	
	WILL_LAUNCH_CINE.clear();
}

//*************************************************************************************
// Manages Currently playing 2D cinematic
//*************************************************************************************
long DANAE_Manage_Cinematic()
{
	
	float FrameTicks=ARX_TIME_Get( false );

	if (PLAY_LOADED_CINEMATIC==1)
	{
		LogDebug("really starting cinematic now");
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
		SendMsgToAllIO(SM_CINE_END, LAST_LAUNCHED_CINE);
	}

	LastFrameTicks=FrameTicks;
	return 0;
}

void ReMappDanaeButton() {
	
	// Handle double clicks.
	const ActionKey & button = config.actions[CONTROLS_CUST_ACTION].key[0];
	if((button.key[0] != -1 && (button.key[0] & Mouse::ButtonBase)
	    && GInput->getMouseButtonDoubleClick(button.key[0], 300))
	   || (button.key[1] != -1 && (button.key[1] & Mouse::ButtonBase)
	    && GInput->getMouseButtonDoubleClick(button.key[1], 300))) {
		LastEERIEMouseButton = EERIEMouseButton;
		EERIEMouseButton |= 4;
		EERIEMouseButton &= ~1;
	}
	
	if(GInput->actionNowPressed(CONTROLS_CUST_ACTION)) {
		LastEERIEMouseButton = EERIEMouseButton;
		if(EERIEMouseButton & 4) {
			EERIEMouseButton &= ~1;
		} else {
			EERIEMouseButton |= 1;
		}
		
	}
	if(GInput->actionNowReleased(CONTROLS_CUST_ACTION)) {
		LastEERIEMouseButton = EERIEMouseButton;
		EERIEMouseButton &= ~1;
		EERIEMouseButton &= ~4;
	}
	
	if(GInput->actionNowPressed(CONTROLS_CUST_MOUSELOOK)) {
		EERIEMouseButton |= 2;
	}
	if(GInput->actionNowReleased(CONTROLS_CUST_MOUSELOOK)) {
		EERIEMouseButton &= ~2;
	}
	
}

void AdjustMousePosition()
{
	if (EERIEMouseGrab && GInput->hasMouseMoved())
	{
		Vec2s pos;
		pos.x = (short)(DANAESIZX >> 1);
		pos.y = (short)(DANAESIZY >> 1);

		if (!((ARXmenu.currentmode == AMCM_NEWQUEST)
				||	(player.Interface & INTER_MAP && (Book_Mode != BOOKMODE_MINIMAP)))) {
			GInput->setMousePosAbs(pos);
		}
	}
}

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
	EERIEDrawBitmap(0, static_cast<float>(iVPOS * 16), width, 8, 0.000091f, NULL, rgb.to<u8>());
	mainApp->OutputText(static_cast<u32>(width), iVPOS * 16 - 2, str);
}

void ShowTestText()
{
	char tex[256];

	mainApp->OutputText(0, 16, version);

	sprintf(tex,"Level : %s", LastLoadedScene.string().c_str());
	mainApp->OutputText( 0, 32, tex );

	sprintf(tex,"Position : %5.0f %5.0f %5.0f",player.pos.x,player.pos.y,player.pos.z);
	mainApp->OutputText( 0, 48, tex );

	sprintf( tex,"Last Failed Sequence : %s",LAST_FAILED_SEQUENCE.c_str() );
	mainApp->OutputText( 0, 64, tex );
}
extern float CURRENT_PLAYER_COLOR;
extern int TSU_TEST_COLLISIONS;
extern long TSU_TEST;

long TSU_TEST_NB = 0;
long TSU_TEST_NB_LIGHT = 0;

void ShowInfoText() {
	
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
	mainApp->OutputText( 70, 32, tex );

	float poss=-666.66f;
	EERIEPOLY * ep=CheckInPolyPrecis(player.pos.x,player.pos.y,player.pos.z);
	float tempo=0.f;

	if ((ep) && (GetTruePolyY(ep,&player.pos,&tempo)))
		poss=tempo;

	sprintf(tex,"Position  x:%7.0f y:%7.0f [%7.0f] z:%6.0f a%3.0f b%3.0f FOK %3.0f",player.pos.x,player.pos.y+player.size.y,poss,player.pos.z,player.angle.a,player.angle.b,ACTIVECAM->focal);
	mainApp->OutputText( 70, 48, tex );
	sprintf(tex,"AnchorPos x:%6.0f y:%6.0f z:%6.0f TIME %lds Part %ld - %d",player.pos.x-Mscenepos.x,player.pos.y+player.size.y-Mscenepos.y,player.pos.z-Mscenepos.z
		,GAT,ParticleCount,player.doingmagic);
	mainApp->OutputText( 70, 64, tex );

	if (player.onfirmground==0) mainApp->OutputText( 200, 280, "OFFGRND" );

	sprintf(tex,"Jump %f cinema %f %d %d - Pathfind %ld(%s)",player.jumplastposition,CINEMA_DECAL,DANAEMouse.x,DANAEMouse.y,EERIE_PATHFINDER_Get_Queued_Number(), PATHFINDER_WORKING ? "Working" : "Idled");
	mainApp->OutputText( 70, 80, tex );
	INTERACTIVE_OBJ * io=ARX_SCRIPT_Get_IO_Max_Events();

	if (io==NULL)
		sprintf(tex,"Events %ld (IOmax N/A) Timers %ld",ScriptEvent::totalCount,ARX_SCRIPT_CountTimers());
	else 
	{
		sprintf(tex,"Events %ld (IOmax %s %d) Timers %ld",ScriptEvent::totalCount, io->long_name().c_str(), io->stat_count,ARX_SCRIPT_CountTimers());
	}

	mainApp->OutputText( 70, 94, tex );

	io=ARX_SCRIPT_Get_IO_Max_Events_Sent();

	if(io) {
		sprintf(tex,"Max SENDER %s %d)", io->long_name().c_str(), io->stat_sent);
		mainApp->OutputText(70, 114, tex);
	}

	float slope=0.f;
	ep=CheckInPoly(player.pos.x,player.pos.y-10.f,player.pos.z);

	if (ep)
	{
		slope=ep->norm.y;
	}

	sprintf(tex,"Velocity %3.0f %3.0f %3.0f Slope %3.3f",player.physics.velocity.x,player.physics.velocity.y,player.physics.velocity.z,slope);
	mainApp->OutputText( 70, 128, tex );

	sprintf(tex, "TSU_TEST %ld - nblights %ld - nb %ld", TSU_TEST, TSU_TEST_NB_LIGHT, TSU_TEST_NB);
	mainApp->OutputText( 100, 208, tex );
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
					io->_npcdata->pathfind.truetarget, (long)io->_npcdata->behavior);
				mainApp->OutputText(170, 420, tex);
			sprintf(tex,"Life %4.0f/%4.0f Mana %4.0f/%4.0f Poisoned %3.1f Hunger %4.1f",player.life,player.maxlife,
					player.mana,player.maxmana,player.poison,player.hunger);
				mainApp->OutputText( 170, 320, tex );

		  }
		  else
		  {
			  if (io->ioflags & IO_NPC)
			  {
				  
				sprintf(tex,"%4.0f %4.0f %4.0f - %4.0f %4.0f %4.0f -- %3.0f %d/%ld targ %ld beh %ld",io->pos.x,
					io->pos.y,io->pos.z,io->move.x,
					io->move.y,io->move.z,io->_npcdata->moveproblem,io->_npcdata->pathfind.listpos,io->_npcdata->pathfind.listnb,
					io->_npcdata->pathfind.truetarget, (long)io->_npcdata->behavior);
				mainApp->OutputText(170, 420, tex);
				sprintf(tex,"Life %4.0f/%4.0f Mana %4.0f/%4.0f Poisoned %3.1f",io->_npcdata->life,io->_npcdata->maxlife,
					io->_npcdata->mana,io->_npcdata->maxmana,io->_npcdata->poisonned);
				mainApp->OutputText( 170, 320, tex );
				sprintf(tex,"AC %3.0f Absorb %3.0f",ARX_INTERACTIVE_GetArmorClass(io),io->_npcdata->absorb);
				mainApp->OutputText( 170, 335, tex );

				if (io->_npcdata->pathfind.flags  & PATHFIND_ALWAYS)
					mainApp->OutputText( 170, 360, "PF_ALWAYS" );
				else
				{
					sprintf(tex, "PF_%ld", (long)io->_npcdata->pathfind.flags);
					mainApp->OutputText(170, 360, tex);
				}
			  }

			  if (io->ioflags & IO_FIX)
			  {
				sprintf(tex,"Durability %4.0f/%4.0f Poisonous %3d count %d",io->durability,io->max_durability,io->poisonous,io->poisonous_count);
				mainApp->OutputText( 170, 320, tex );
			  }

			  if (io->ioflags & IO_ITEM)
			  {
				sprintf(tex,"Durability %4.0f/%4.0f Poisonous %3d count %d",io->durability,io->max_durability,io->poisonous,io->poisonous_count);
				mainApp->OutputText( 170, 320, tex );
			  }
		  }
	  }
	}
#endif // BUILD_EDITOR

	long zap=IsAnyPolyThere(player.pos.x,player.pos.z);
	sprintf(tex,"POLY %ld",zap);		
	mainApp->OutputText( 270, 220, tex );

	sprintf(tex,"COLOR %3.0f Stealth %3.0f",CURRENT_PLAYER_COLOR,GetPlayerStealth());
	mainApp->OutputText( 270, 200, tex );

	ARX_SCRIPT_Init_Event_Stats();
}

//-----------------------------------------------------------------------------

extern long POLYIN;
extern long LAST_LLIGHT_COUNT;
extern float PLAYER_CLIMB_THRESHOLD, player_climb;
extern float TOTAL_CHRONO;

void ShowFPS() {
	
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
	//mainApp->OutputText( 70, DANAESIZY-100+32, tex );

	TOTAL_CHRONO=0;
//	TODO(lubosz): Don't get this by extern global
//	sprintf(tex,"%4.0f MCache %ld[%ld] FP %3.0f %3.0f Llights %ld/%ld TOTIOPDL %ld TOTPDL %ld"
//		,inter.iobj[0]->pos.y, meshCache.size(),MCache_GetSize(),Original_framedelay,_framedelay,LAST_LLIGHT_COUNT,MAX_LLIGHTS,TOTIOPDL,TOTPDL);

	if (LAST_LLIGHT_COUNT>MAX_LLIGHTS)
		strcat(tex," EXCEEDING LIMIT !!!");

	mainApp->OutputText(70,20,tex);
}

void ARX_SetAntiAliasing() {
	GRenderer->SetAntialiasing(config.video.antialiasing);
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
	
	mainApp->GetWindow()->hide();
	
	ARX_MINIMAP_PurgeTC();
	
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
	
	delete resources;
	
	ReleaseNode();
	
	//Halo
	ReleaseHalo();
	FreeSnapShot();
	ARX_INPUT_Release();
	
	mainApp->Cleanup3DEnvironment();
	
	delete mainApp, mainApp = NULL;
	
	LogInfo << "Clean shutdown";
}
