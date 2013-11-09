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
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include "core/Core.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <iomanip>
#include <sstream>
#include <vector>

#include <boost/version.hpp>
#include <boost/foreach.hpp>

#include "Configure.h"

#include "ai/Paths.h"
#include "ai/PathFinderManager.h"

#include "animation/Animation.h"
#include "animation/AnimationRender.h"
#include "animation/Cinematic.h"
#include "animation/CinematicKeyframer.h"

#include "core/Application.h"
#include "core/ArxGame.h"
#include "core/Config.h"
#include "core/Localisation.h"
#include "core/GameTime.h"
#include "core/Version.h"

#include "game/Damage.h"
#include "game/EntityManager.h"
#include "game/Equipment.h"
#include "game/Inventory.h"
#include "game/Levels.h"
#include "game/Missile.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "game/spell/FlyingEye.h"
#include "game/spell/Cheat.h"

#include "gui/MenuPublic.h"
#include "gui/Menu.h"
#include "gui/MenuWidgets.h"
#include "gui/Speech.h"
#include "gui/MiniMap.h"
#include "gui/TextManager.h"

#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Draw.h"
#include "graphics/DrawLine.h"
#include "graphics/DrawDebug.h"
#include "graphics/font/Font.h"
#include "graphics/GraphicsModes.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "graphics/Vertex.h"
#include "graphics/data/FTL.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/Fog.h"
#include "graphics/image/Image.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/ParticleManager.h"
#include "graphics/particle/MagicFlare.h"
#include "graphics/texture/TextureStage.h"

#include "gui/Interface.h"
#include "gui/Text.h"

#include "input/Input.h"
#include "input/Keyboard.h"
#include "input/Mouse.h"

#include "io/fs/SystemPaths.h"
#include "io/resource/ResourcePath.h"
#include "io/resource/PakReader.h"
#include "io/CinematicLoad.h"
#include "io/Screenshot.h"
#include "io/log/Logger.h"

#include "math/Angle.h"
#include "math/Rectangle.h"
#include "math/Vector.h"
 
#include "physics/Collisions.h"
#include "physics/Attractors.h"

#include "platform/CrashHandler.h"
#include "platform/Flags.h"
#include "platform/Platform.h"
#include "platform/ProgramOptions.h"

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

Image savegame_thumbnail;

#define MAX_EXPLO 24

static bool initializeGame();
static void shutdownGame();

extern TextManager	*pTextManage;
extern float FORCE_TIME_RESTORE;
extern CMenuState		*pMenu;
extern float	InventoryX;
extern float	PROGRESS_BAR_COUNT;
extern float	PROGRESS_BAR_TOTAL;
extern long		DONT_WANT_PLAYER_INZONE;
extern long		TOTPDL;
extern long		COLLIDED_CLIMB_POLY;
extern long LastSelectedIONum;
extern float ARXTimeMenu;
extern float ARXOldTimeMenu;
extern bool		PLAYER_MOUSELOOK_ON;
extern bool bFadeInOut;
extern 	bool bFade;			//active le fade
extern float OLD_PROGRESS_BAR_COUNT;
#ifdef BUILD_EDITOR
long LastSelectedIONum = -1;
#endif

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
extern EERIE_3DOBJ * svoodoo;
extern long svoodoo_count;

//-----------------------------------------------------------------------------
// Our Main Danae Application.& Instance and Project
PROJECT Project;

//-----------------------------------------------------------------------------
Vec3f PUSH_PLAYER_FORCE;
Cinematic			*ControlCinematique=NULL;	// 2D Cinematic Controller
ParticleManager	*pParticleManager = NULL;
static TextureContainer * ombrignon = NULL;
TextureContainer *	scursor[8];			// Animated Hand Cursor TC
TextureContainer *	pTCCrossHair;			// Animated Hand Cursor TC
TextureContainer *	iconequip[5];
TextureContainer *	GoldCoinsTC[MAX_GOLD_COINS_VISUALS]; // Gold Coins Icons
TextureContainer *	explo[MAX_EXPLO];		// TextureContainer for animated explosion bitmaps (24 frames)
TextureContainer *	blood_splat = NULL;		// TextureContainer for blood splat particles

TextureContainer *	tflare = NULL;
static TextureContainer * npc_fight = NULL;
static TextureContainer * npc_follow = NULL;
static TextureContainer * npc_stop = NULL;
TextureContainer *	sphere_particle=NULL;
TextureContainer *	inventory_font=NULL;
TextureContainer *	enviro=NULL;
TextureContainer *	specular=NULL;
TextureContainer *	stealth_gauge_tc=NULL;
TextureContainer *	arx_logo_tc=NULL;
TextureContainer *	TC_fire2=NULL;
TextureContainer *	TC_fire=NULL;
TextureContainer *	TC_smoke=NULL;
static TextureContainer *	Z_map = NULL;
TextureContainer *	Boom=NULL;
//TextureContainer *	zbtex=NULL;
TextureContainer *	mecanism_tc=NULL;
TextureContainer *	arrow_left_tc=NULL;

#if BUILD_EDIT_LOADSAVE
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
EERIE_3DOBJ * cabal=NULL;				// Cabalistic 3D Object // NEEDTO: Load dynamically
static EERIE_BACKGROUND DefaultBkg;

EERIE_CAMERA subj,bookcam,raycam,conversationcamera;

string WILLADDSPEECH;

Vec2s STARTDRAG;
Entity * COMBINE=NULL;

QUAKE_FX_STRUCT QuakeFx;
string LAST_FAILED_SEQUENCE = "none";
// START - Information for Player Teleport between/in Levels-------------------------------------
char TELEPORT_TO_LEVEL[64];
char TELEPORT_TO_POSITION[64];
long TELEPORT_TO_ANGLE;
// END -   Information for Player Teleport between/in Levels---------------------------------------
std::string WILL_LAUNCH_CINE;
res::path LastLoadedScene;
static std::string LAST_LAUNCHED_CINE;
float BASE_FOCAL=350.f;
float STRIKE_AIMTIME=0.f;
float SLID_VALUE=0.f;
float framedelay=0.f;

static float LASTfps2 = 0;
static float fps2 = 0;
static float fps2min = 0;
static long LASTfpscount = 0;

long LOAD_N_DONT_ERASE=0;
long NO_TIME_INIT=0;

Rect g_size(640, 480);

long CurrFightPos=0;
long NO_PLAYER_POSITION_RESET=0;
long CURRENT_BASE_FOCAL=310;
long CINE_PRELOAD=0;
long PLAY_LOADED_CINEMATIC=0;
float BOW_FOCAL=0;
long PlayerWeaponBlocked=-1;

long REQUEST_SPEECH_SKIP= 0;
long CURRENTLEVEL		= -1;
long DONT_ERASE_PLAYER	= 0;
static float LastFrameTicks = 0;
long FASTmse			= 0;

//-----------------------------------------------------------------------------
// EDITOR FLAGS/Vars
//-----------------------------------------------------------------------------
// Flag used to Launch Moulinex
long LOADEDD = 0; // Is a Level Loaded ?

#ifdef BUILD_EDITOR
long DEBUGNPCMOVE = 0; // Debug NPC Movements
#endif

long CHANGE_LEVEL_ICON=-1;

long ARX_MOUSE_OVER=0;
//-----------------------------------------------------------------------------
// DEBUG FLAGS/Vars
//-----------------------------------------------------------------------------
long LaunchDemo=0;
bool FirstFrame=true;
unsigned long WILLADDSPEECHTIME=0;
unsigned long AimTime;
//-----------------------------------------------------------------------------
Color3f FADECOLOR;

long START_NEW_QUEST=0;
static long LAST_WEAPON_TYPE = -1;
long	FADEDURATION=0;
long	FADEDIR=0;
unsigned long FADESTART=0;

float Original_framedelay=0.f;

float PULSATE;
long EXITING=0;

bool USE_PORTALS = false;

Vec3f ePos;
extern EERIE_CAMERA * ACTIVECAM;


bool g_debugToggles[10];

//-----------------------------------------------------------------------------

void LoadSysTextures();
void ManageNONCombatModeAnimations();

//-----------------------------------------------------------------------------

class GameFlow {

public:
	enum Transition {
		NoTransition,
		FirstLogo,
		SecondLogo,
		LoadingScreen,
		InGame
	};

	static void setTransition(Transition newTransition) {
		s_currentTransition = newTransition;
	}

	static Transition getTransition() {
		return s_currentTransition;
	}

private:
	static Transition s_currentTransition;
};

GameFlow::Transition GameFlow::s_currentTransition = GameFlow::NoTransition;


// Sends ON GAME_READY msg to all IOs
void SendGameReadyMsg()
{
	LogDebug("SendGameReadyMsg");
	SendMsgToAllIO(SM_GAME_READY);
}

void DANAE_KillCinematic() {
	if(ControlCinematique && ControlCinematique->projectload) {
		ControlCinematique->projectload = false;
		ControlCinematique->OneTimeSceneReInit();
		ControlCinematique->DeleteDeviceObjects();
		PLAY_LOADED_CINEMATIC = 0;
		CINE_PRELOAD = 0;
	}
}

static bool AdjustUI() {
	
	// Sets Danae Screen size depending on windowed/full-screen state
	g_size = Rect(mainApp->getWindow()->getSize().x, mainApp->getWindow()->getSize().y);
		
	// Computes X & Y screen ratios compared to a standard 640x480 screen
	Xratio = g_size.width() * ( 1.0f / 640 );
	Yratio = g_size.height() * ( 1.0f / 480 );
	
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

extern void InitTileLights();

enum LevelNumber {
	LEVEL0     = 0,
	LEVEL1     = 1,
	LEVEL2     = 2,
	LEVEL3     = 3,
	LEVEL4     = 4,
	LEVEL5     = 5,
	LEVEL6     = 6,
	LEVEL7     = 7,
	LEVEL8     = 8,
	LEVEL9     = 9,
	LEVEL10    = 10,
	LEVEL11    = 11,
	LEVEL12    = 12,
	LEVEL13    = 13,
	LEVEL14    = 14,
	LEVEL15    = 15,
	LEVEL16    = 16,
	LEVEL17    = 17,
	LEVEL18    = 18,
	LEVEL19    = 19,
	LEVEL20    = 20,
	LEVEL21    = 21,
	LEVEL22    = 22,
	LEVEL23    = 23,
	LEVEL24    = 24,
	LEVEL25    = 25,
	LEVEL26    = 26,
	LEVEL27    = 27,
	LEVELDEMO  = 28,
	LEVELDEMO2 = 29,
	LEVELDEMO3 = 30,
	LEVELDEMO4 = 31,
	NOLEVEL    = 32
};

void InitializeDanae() {
	
	InitTileLights();
	
	ARX_MISSILES_ClearAll();
	ARX_SPELLS_Init();

	ARX_SPELLS_ClearAllSymbolDraw();
	ARX_PARTICLES_ClearAll();
	ARX_MAGICAL_FLARES_FirstInit();
	
	LastLoadedScene.clear();
	
	res::path levelPath;
	res::path levelFullPath;
	
	if(Project.demo != NOLEVEL) {
		char levelId[256];
		GetLevelNameByNum(Project.demo, levelId);
		levelPath = std::string("graph/levels/level") + levelId;
		levelFullPath = levelPath.string() + "/level" + levelId + ".dlf";
	}
	
	memset(&DefaultBkg, 0, sizeof(EERIE_BACKGROUND));
	ACTIVEBKG=&DefaultBkg;
	InitBkg(ACTIVEBKG,MAX_BKGX,MAX_BKGZ,BKG_SIZX,BKG_SIZZ);
	InitNodes(1);
	
	player.size.y = -player.baseHeight();
	player.size.x = player.baseRadius();
	player.size.z = player.baseRadius();
	subj.size.setYaw(player.size.y);
	subj.size.setPitch(player.size.x);
	subj.size.setRoll(player.size.z);
	player.desiredangle = player.angle = subj.angle = Anglef(3.f, 268.f, 0.f);

	subj.orgTrans.pos = Vec3f(900.f, player.baseHeight(), 4340.f);
	subj.clip = Rect(0, 0, 640, 480);
	subj.center = subj.clip.center();
	subj.focal = BASE_FOCAL;
	subj.bkgcolor = Color::none;
	
	SetActiveCamera(&subj);
	SetCameraDepth(subj, 2100.f);

	bookcam = subj;
	raycam = subj;
	conversationcamera = subj;
	
	raycam.clip = Rect(0, 0, 640, 640);
	raycam.center = raycam.clip.center();
	
	bookcam.angle = Anglef::ZERO;
	bookcam.orgTrans.pos = Vec3f_ZERO;
	bookcam.focal = BASE_FOCAL;
	
	ACTIVEBKG->ambient = Color3f(0.09f, 0.09f, 0.09f);
	ACTIVEBKG->ambient255 = ACTIVEBKG->ambient * 255.f;
	
	LoadSysTextures();
	CreateInterfaceTextureContainers();
	
	if(LaunchDemo) {
		LogInfo << "Launching splash screens.";
		LaunchDemo = 0;
		if(GameFlow::getTransition() == GameFlow::NoTransition) {
			GameFlow::setTransition(GameFlow::FirstLogo);
		}
	} else if(!levelPath.empty())	{
		LogInfo << "Launching Level " << levelPath;
		if (FastSceneLoad(levelPath)) {
			FASTmse = 1;
		} else {
#if BUILD_EDIT_LOADSAVE
			ARX_SOUND_PlayCinematic("editor_humiliation", false);
			mse = PAK_MultiSceneToEerie(levelPath);
#else
			LogError << "FastSceneLoad failed";
#endif
		}
		EERIEPOLY_Compute_PolyIn();
		LastLoadedScene = levelPath;
		USE_PLAYERCOLLISIONS = false;
	}
}

static bool initializeGame() {
	
	// TODO Time will be re-initialized later, but if we don't initialize it now casts to int might overflow.
	arxtime.init();
	
	mainApp = new ArxGame();
	if(!mainApp->initialize()) {
		// Fallback to a generic critical error in case none was set yet...
		LogCritical << "Application failed to initialize properly.";
		return false;
	}
	
	// Check if the game will be able to use the current game directory.
	if(!ARX_Changelevel_CurGame_Clear()) {
		LogCritical << "Error accessing current game directory.";
		return false;
	}
	
	ScriptEvent::init();
	
	CalcFPS(true);
	
	g_miniMap.mapMarkerInit();
	
	memset(scursor, 0, sizeof(scursor));
	
	ARX_SPELLS_CancelSpellTarget();
	
	memset(explo, 0, sizeof(explo));
	
	LogDebug("Danae Start");
	
	LogDebug("Project Init");
	
	PUSH_PLAYER_FORCE = Vec3f_ZERO;
	ARX_SPECIAL_ATTRACTORS_Reset();
	LogDebug("Attractors Init");
	ARX_SPELLS_Precast_Reset();
	LogDebug("Spell Init");
	
	for(size_t t = 0; t < MAX_GOLD_COINS_VISUALS; t++) {
		GoldCoinsObj[t] = NULL;
		GoldCoinsTC[t] = NULL;
	}
	
	LogDebug("LSV Init");
	ModeLight = MODE_DEPTHCUEING;
	
	memset(&DefaultBkg,0,sizeof(EERIE_BACKGROUND));
	memset(TELEPORT_TO_LEVEL,0,64);
	memset(TELEPORT_TO_POSITION,0,64);
	LogDebug("Mset");
	
	LogDebug("AnimManager Init");
	ARX_SCRIPT_EventStackInit();
	LogDebug("EventStack Init");
	ARX_EQUIPMENT_Init();
	LogDebug("AEQ Init");
	
	ARX_SCRIPT_Timer_FirstInit(512);
	LogDebug("Timer Init");
	ARX_FOGS_FirstInit();
	LogDebug("Fogs Init");
	
	EERIE_LIGHT_GlobalInit();
	LogDebug("Lights Init");
	
	LogDebug("Svars Init");
	
	// Script Test
	lastteleport = player.baseOffset();
	
	entities.init();
	
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
	
	LaunchDemo = 1;
	Project.demo = LEVEL10;
	
	if(!AdjustUI()) {
		return false;
	}
	
	ARX_SetAntiAliasing();
	ARXMenu_Options_Video_SetFogDistance(config.video.fogDistance);
	ARXMenu_Options_Video_SetDetailsQuality(config.video.levelOfDetail);
	ARXMenu_Options_Audio_SetMasterVolume(config.audio.volume);
	ARXMenu_Options_Audio_SetSfxVolume(config.audio.sfxVolume);
	ARXMenu_Options_Audio_SetSpeechVolume(config.audio.speechVolume);
	ARXMenu_Options_Audio_SetAmbianceVolume(config.audio.ambianceVolume);
	ARXMenu_Options_Audio_ApplyGameVolumes();
	
	ARXMenu_Options_Control_SetInvertMouse(config.input.invertMouse);
	ARXMenu_Options_Control_SetMouseSensitivity(config.input.mouseSensitivity);
	
	g_miniMap.firstInit(&player, resources, &entities);
	
	Project.torch = Color3f(1.f, 0.8f, 0.66666f);
	LogDebug("InitializeDanae");
	InitializeDanae();
	
	switch(resources->getReleaseType()) {
		
		case 0: LogWarning << "Neither demo nor full game data files loaded."; break;
		
		case PakReader::Demo: {
			LogInfo << "Initialized Arx Fatalis (demo)";
			CrashHandler::setVariable("Data files", "demo");
			break;
		}
		
		case PakReader::FullGame: {
			LogInfo << "Initialized Arx Fatalis (full game)";
			CrashHandler::setVariable("Data files", "full");
			break;
		}
		
		case (int(PakReader::Demo) | int(PakReader::FullGame)): {
			LogWarning << "Mixed demo and full game data files!";
			CrashHandler::setVariable("Data files", "mixed");
			break;
		}
		
		default: ARX_DEAD_CODE();
	}
	
	return true;
}

void runGame() {
	
	if(initializeGame()) {
		// Init all done, start the main loop
		mainApp->run();
		
		// TODO run cleanup on partial initialization
		shutdownGame();
	}
	
	if(mainApp) {
		mainApp->shutdown();
		delete mainApp;
		mainApp = NULL;
	}
}


//*************************************************************************************
// Entity * FlyingOverObject(EERIE_S2D * pos)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Returns IO under cursor, be it in inventories or in scene
//   Returns NULL if no IO under cursor
//*************************************************************************************
Entity * FlyingOverObject(Vec2s * pos)
{
	Entity* io = NULL;

	if((io = GetFromInventory(pos)) != NULL)
		return io;

	if(InInventoryPos(pos))
		return NULL;

	if((io = InterClick(pos)) != NULL)
		return io;

	return NULL;
}

extern long ARX_NPC_ApplyCuts(Entity * io);

void LoadSysTextures()
{
	MagicFlareLoadTextures();

	for(size_t i = 0; i < SPELL_COUNT; i++) {
		// TODO use constructor for initialization
		for(long j = 0; j < 6; j++)
			spellicons[i].symbols[j] = RUNE_NONE;

		spellicons[i].level = 0;
		spellicons[i].spellid = SPELL_NONE;
		spellicons[i].tc = NULL;
		spellicons[i].bSecret = false;
		spellicons[i].bDuration = true;
		spellicons[i].bAudibleAtStart = false;
	}
	
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

	specular=			TextureContainer::LoadUI("graph/particles/specular");
	enviro=				TextureContainer::LoadUI("graph/particles/enviro");
	sphere_particle=	TextureContainer::LoadUI("graph/particles/sphere");
	inventory_font=		TextureContainer::LoadUI("graph/interface/font/font10x10_inventory");
	npc_fight=			TextureContainer::LoadUI("graph/interface/icons/follower_attack");
	npc_follow=			TextureContainer::LoadUI("graph/interface/icons/follower_follow");
	npc_stop=			TextureContainer::LoadUI("graph/interface/icons/follower_stop");
	tflare=				TextureContainer::LoadUI("graph/particles/flare");
	ombrignon=			TextureContainer::LoadUI("graph/particles/ombrignon");
	TextureContainer::LoadUI("graph/particles/teleportae");
	TC_fire=			TextureContainer::LoadUI("graph/particles/fire");
	TC_fire2=			TextureContainer::LoadUI("graph/particles/fire2");
	TC_smoke=			TextureContainer::LoadUI("graph/particles/smoke");
	TextureContainer::LoadUI("graph/particles/missile");
	Z_map = TextureContainer::LoadUI("graph/interface/misc/z-map");
	Boom=				TextureContainer::LoadUI("graph/particles/boom");
	stealth_gauge_tc=	TextureContainer::LoadUI("graph/interface/icons/stealth_gauge");
	arx_logo_tc=		TextureContainer::LoadUI("graph/interface/icons/arx_logo_32");
	iconequip[0]=		TextureContainer::LoadUI("graph/interface/icons/equipment_sword");
	iconequip[1]=		TextureContainer::LoadUI("graph/interface/icons/equipment_shield");
	iconequip[2]=		TextureContainer::LoadUI("graph/interface/icons/equipment_helm");
	iconequip[3]=		TextureContainer::LoadUI("graph/interface/icons/equipment_chest");
	iconequip[4]=		TextureContainer::LoadUI("graph/interface/icons/equipment_leggings");
	mecanism_tc=		TextureContainer::LoadUI("graph/interface/cursors/mecanism");
	arrow_left_tc=		TextureContainer::LoadUI("graph/interface/icons/arrow_left");

	for(long i = 0; i < MAX_EXPLO; i++) {
		char temp[256];
		sprintf(temp,"graph/particles/fireb_%02ld",i+1);
		explo[i]= TextureContainer::LoadUI(temp);
	}

	blood_splat=TextureContainer::LoadUI("graph/particles/new_blood2");

	TextureContainer::LoadUI("graph/particles/square");
	
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
	
	for(long i = 0; i < 8; i++) {
		char temp[256];
		sprintf(temp,"graph/interface/cursors/cursor%02ld", i);
		scursor[i] = TextureContainer::LoadUI(temp);
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
	
	Vec3f vect;
	EERIE_QUAT quat;
	
	Vec3f position = *pos;
	float anglea = radians(angle->getYaw());
	float angleb = radians(angle->getPitch());
	vect.x=-EEsin(angleb)*EEcos(anglea);
	vect.y= EEsin(anglea);
	vect.z= EEcos(angleb)*EEcos(anglea);

	float velocity = aimratio + 0.3f;

	if(velocity < 0.9f)
		velocity = 0.9f;

	Vec3f v1,v2;
	Vec3f vv(0,0,1);
	float aa=angle->getYaw();
	float ab=90-angle->getPitch();
	Vector_RotateZ(&v1,&vv,aa);
	VRotateY(&v1,ab);
	vv = Vec3f(0,-1,0);
	Vector_RotateZ(&v2,&vv,aa);
	VRotateY(&v2,ab);
	EERIEMATRIX tmat;
	MatrixSetByVectors(&tmat,&v1,&v2);
	QuatFromMatrix(quat,tmat);

	float wd = getEquipmentBaseModifier(IO_EQUIPITEM_ELEMENT_Damages);
	// TODO Why ignore relative modifiers? Why not just use player.Full_damages?

	float weapon_damages=wd;

	float damages = weapon_damages*(1.f + (float)(player.Full_Skill_Projectile + player.Full_Attribute_Dexterity )*( 1.0f / 50 ));

	ARX_THROWN_OBJECT_Throw(/*source*/0, &position, &vect, &quat, velocity, damages, poisonous);
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
	
	BOOST_FOREACH(Entity * e, entities) {
		if(e && (e->show == SHOW_FLAG_HIDDEN || e->show == SHOW_FLAG_KILLED)) {
			e->show = SHOW_FLAG_IN_SCENE;
		}
	}

	RestoreAllLightsInitialStatus();

	if (stop_sound) ARX_SOUND_MixerStop(ARX_SOUND_MixerGame);

	RestoreInitialIOStatus();

	if (ed)
	{
		ARX_PATH_ComputeAllBoundingBoxes();

		arxtime.pause();
	}
	else
	{
		ARX_SCRIPT_ResetAll(1);
		EERIE_ANIMMANAGER_PurgeUnused();
	}
	
	if(!DONT_ERASE_PLAYER) {
		ARX_PLAYER_MakeFreshHero();
	}
	
}

void DANAE_ReleaseAllDatasDynamic() {
	delete ssol, ssol = NULL, ssol_count = 0;
	delete slight, slight = NULL, slight_count = 0;
	delete srune, srune = NULL, srune_count = 0;
	delete smotte, smotte = NULL, smotte_count = 0;
	delete stite, stite = NULL, stite_count = 0;
	delete smissile, smissile = NULL, smissile_count = 0;
	delete spapi, spapi = NULL, spapi_count = 0;
	delete svoodoo, svoodoo = NULL, svoodoo_count = 0;
}

void ReleaseDanaeBeforeRun() {
	
	delete necklace.lacet, necklace.lacet = NULL;
	
	for(long i = 0; i < 20; i++) { 
		delete necklace.runes[i], necklace.runes[i] = NULL;
		necklace.pTexTab[i] = NULL;
	}
	
	FlyingEye_Release();

	delete cabal, cabal = NULL;
	delete cameraobj, cameraobj = NULL;
	delete markerobj, markerobj = NULL;
	delete arrowobj, arrowobj = NULL;
	
	drawDebugRelease();

	BOOST_FOREACH(EERIE_3DOBJ * & obj, GoldCoinsObj) {
		delete obj, obj = NULL;
	}
	
}

void FirstTimeThings() {
	
	eyeball.exist=0;
	WILLADDSPEECHTIME=0;
	WILLADDSPEECH.clear();
	
	for(size_t i = 0; i < MAX_DYNLIGHTS; i++) {
		DynLight[i].exist = 0;
	}
	
	arxtime.update_last_frame_time();
}

//*************************************************************************************

long NO_GMOD_RESET=0;

void FirstFrameProc() {
	
	if(!pParticleManager)
		pParticleManager = new ParticleManager();

	if(!NO_GMOD_RESET)
		ARX_GLOBALMODS_Reset();

	NO_GMOD_RESET = 0;
	
	STARTDRAG = Vec2s_ZERO;
	DANAEMouse = Vec2s_ZERO;
	bookclick = Vec2s(-1, -1);
	
	if(!LOAD_N_DONT_ERASE)
		arxtime.init();

	ARX_BOOMS_ClearAllPolyBooms();
	ARX_DAMAGES_Reset();
	ARX_MISSILES_ClearAll();
	ARX_SPELLS_ClearAll();
	ARX_SPELLS_ClearAllSymbolDraw();
	ARX_PARTICLES_ClearAll();

	if(!LOAD_N_DONT_ERASE) {
		CleanScriptLoadedIO();
		RestoreInitialIOStatus();
		DRAGINTER=NULL;
	}

	ARX_SPELLS_ResetRecognition();
	
	FirstTimeThings();

	if(!LOAD_N_DONT_ERASE) {
		CleanInventory();
		ARX_SCRIPT_Timer_ClearAll();
		UnlinkAllLinkedObjects();
		ARX_SCRIPT_ResetAll(0);
	}

	SecondaryInventory=NULL;
	TSecondaryInventory=NULL;
	ARX_FOGS_Render();

	if(!LOAD_N_DONT_ERASE) {
		arxtime.init();

		if(!DONT_ERASE_PLAYER)
			ARX_PLAYER_InitPlayer();

		SLID_VALUE=0.f;

		player.life = player.maxlife;
		player.mana = player.maxmana;
		if(!DONT_ERASE_PLAYER) {
			ARX_PLAYER_MakeFreshHero();
		}
	}
	
	InitSnapShot(fs::paths.user / "snapshot");
}
Vec3f LastValidPlayerPos;
Vec3f	WILL_RESTORE_PLAYER_POSITION;
long WILL_RESTORE_PLAYER_POSITION_FLAG=0;

void FirstFrameHandling() {
	LogDebug("FirstFrameHandling");
	Vec3f trans;
	FirstFrame = true;

	ARX_PARTICLES_FirstInit();
	ARX_FOGS_TimeReset();
	
	PROGRESS_BAR_COUNT += 2.f;
	LoadLevelScreen();
	
	FirstFrameProc();
	
	if(FASTmse) {
		FASTmse = 0;
		if(LOADEDD) {
			trans = Mscenepos;
			player.pos = loddpos + trans;
		} else {
			player.pos.y += player.baseHeight();
		}
		PROGRESS_BAR_COUNT += 4.f;
		LoadLevelScreen();
	}
#if BUILD_EDIT_LOADSAVE
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
				player.pos.y += player.baseHeight();
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

	if(player.torch) {
		ARX_SOUND_PlaySFX(SND_TORCH_LOOP, NULL, 1.0F, ARX_SOUND_PLAY_LOOPED);
	}
	
	MagicFlareSetCamera(&subj);
	
	lastteleport = player.basePosition();
	subj.orgTrans.pos = moveto = player.pos;

	subj.angle = player.angle;
	
	RestoreLastLoadedLightning();

	PROGRESS_BAR_COUNT+=1.f;
	LoadLevelScreen();

	if(!LOAD_N_DONT_ERASE)
		SetEditMode(0);

	PROGRESS_BAR_COUNT+=1.f;
	LoadLevelScreen();

	LOAD_N_DONT_ERASE=0;
	DONT_ERASE_PLAYER=0;

	PROGRESS_BAR_COUNT+=1.f;
	LoadLevelScreen();

	FirstFrame=false;
	PrepareIOTreatZone(1);
	CURRENTLEVEL=GetLevelNumByName(LastLoadedScene.string());
	
	if(!NO_TIME_INIT)
		arxtime.init();
	
	arxtime.update_last_frame_time();
	
	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen();
	
	delete ITC.Get("presentation");
	ITC.Set("presentation", NULL);
	
	if(DONT_WANT_PLAYER_INZONE) {
		player.inzone = NULL;
		DONT_WANT_PLAYER_INZONE = 0;
	}
	
	PROGRESS_BAR_COUNT+=1.f;
	LoadLevelScreen();

	player.desiredangle.setYaw(0.f);
	player.angle.setYaw(0.f);
	ARX_PLAYER_RectifyPosition();

	if(entities.player())
		entities.player()->_npcdata->vvpos = -99999;

	SendGameReadyMsg();
	PLAYER_MOUSELOOK_ON = false;
	player.Interface &= ~INTER_NOTE;

	if(NO_TIME_INIT) {
		arxtime.force_time_restore(FORCE_TIME_RESTORE);
		arxtime.force_frame_time_restore(FORCE_TIME_RESTORE);
	} else {
		arxtime.resume();
	}

	long t = entities.getById("seat_stool1_0012");
	if(ValidIONum(t)) {
		entities[t]->ioflags |= IO_FORCEDRAW;
	}
	
	if(WILL_RESTORE_PLAYER_POSITION_FLAG) {
		Entity * io = entities.player();
		player.pos = WILL_RESTORE_PLAYER_POSITION;
		io->pos = player.basePosition();
		for(size_t i = 0; i < io->obj->vertexlist.size(); i++) {
			io->obj->vertexlist3[i].v = io->obj->vertexlist[i].v + io->pos;
		}
		WILL_RESTORE_PLAYER_POSITION_FLAG = 0;
	}
	
	for(size_t i = 0; i < entities.size(); i++) {
		if(entities[i] && (entities[i]->ioflags & IO_NPC)
		   && entities[i]->_npcdata->cuts) {
			ARX_NPC_ApplyCuts(entities[i]);
		}
	}
	
	ResetVVPos(entities.player());
	
	PROGRESS_BAR_COUNT+=1.f;
	LoadLevelScreen();
	LoadLevelScreen(-2);
	
	if (	(!CheckInPoly(player.pos.x,player.pos.y,player.pos.z))
		&&	(LastValidPlayerPos.x!=0.f)
		&&	(LastValidPlayerPos.y!=0.f)
		&&	(LastValidPlayerPos.z!=0.f)) {
		player.pos = LastValidPlayerPos;
	}

	LastValidPlayerPos = player.pos;
}

//*************************************************************************************

void ManageNONCombatModeAnimations()
{
	Entity *io = entities.player();

	if(!io)
		return;

	ANIM_USE * useanim3=&io->animlayer[3];
	ANIM_HANDLE ** alist=io->anims;

	if(player.Current_Movement & (PLAYER_LEAN_LEFT | PLAYER_LEAN_RIGHT))
		return;

	if(player.equiped[EQUIP_SLOT_SHIELD] != 0 && !BLOCK_PLAYER_CONTROLS) {
		if ( (useanim3->cur_anim==NULL)  ||
			( (useanim3->cur_anim!=alist[ANIM_SHIELD_CYCLE])
			&& (useanim3->cur_anim!=alist[ANIM_SHIELD_HIT])
			&& (useanim3->cur_anim!=alist[ANIM_SHIELD_START]) ) )
		{
			AcquireLastAnim(io);
			ANIM_Set(useanim3,alist[ANIM_SHIELD_START]);
		} else if(useanim3->cur_anim==alist[ANIM_SHIELD_START] && (useanim3->flags & EA_ANIMEND)) {
			AcquireLastAnim(io);
			ANIM_Set(useanim3,alist[ANIM_SHIELD_CYCLE]);
			useanim3->flags|=EA_LOOP;
		}
	} else {
		if(useanim3->cur_anim==alist[ANIM_SHIELD_CYCLE]) {
			AcquireLastAnim(io);
			ANIM_Set(useanim3,alist[ANIM_SHIELD_END]);
		} else if(useanim3->cur_anim == alist[ANIM_SHIELD_END] && (useanim3->flags & EA_ANIMEND)) {
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
					Entity * io = inventory[iNbBag][i][j].io;
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

Entity * Player_Arrow_Count_Decrease() {
	
	Entity * io = NULL;
	
	for(int iNbBag = 0; iNbBag < player.bag; iNbBag++) {
		for(size_t j = 0; j < INVENTORY_Y; j++) {
			for(size_t i = 0; i < INVENTORY_X; i++) {
				Entity * ioo = inventory[iNbBag][i][j].io;
				if(ioo && ioo->short_name() == "arrows" && ioo->durability >= 1.f) {
					if(!io || io->durability > ioo->durability)
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
	STRIKE_AIMTIME = float(arxtime) - (float)AimTime;
	STRIKE_AIMTIME = STRIKE_AIMTIME * (1.f+(1.f-GLOBAL_SLOWDOWN));

	if(STRIKE_AIMTIME > player.Full_AimTime)
		STRIKE_AIMTIME=1.f;
	else
		STRIKE_AIMTIME = (float)STRIKE_AIMTIME / (float)player.Full_AimTime;

	if(STRIKE_AIMTIME < 0.1f)
		STRIKE_AIMTIME = 0.1f;

	if(STRIKE_AIMTIME > 0.8f)
		return true;

	return false;
}

void strikeSpeak(Entity * io) {
	
	if(!StrikeAimtime()) {
		return;
	}
	
	const string * str;
	long equiped = player.equiped[EQUIP_SLOT_WEAPON];
	if(equiped != 0 && !entities[equiped]->strikespeech.empty()) {
		str = &entities[equiped]->strikespeech;
	} else if(!io->strikespeech.empty()) {
		str = &io->strikespeech;
	} else {
		return;
	}
	
	ARX_SPEECH_AddSpeech(io, *str, ANIM_TALK_NEUTRAL, ARX_SPEECH_FLAG_NOTEXT);
}

void ManageCombatModeAnimations()
{
	Entity *io = entities.player();
	if(!io)
		return;

	ANIM_USE * useanim=&io->animlayer[1];

	ANIM_HANDLE ** alist=io->anims;
	WeaponType weapontype = ARX_EQUIPMENT_GetPlayerWeaponType();

	if(weapontype == WEAPON_BARE && LAST_WEAPON_TYPE != weapontype) {
		if(useanim->cur_anim != alist[ANIM_BARE_WAIT]) {
			AcquireLastAnim(io);
			ANIM_Set(useanim, alist[ANIM_BARE_WAIT]);
			AimTime = 0;
		}
	}

	switch(weapontype) {
		case WEAPON_BARE:	// BARE HANDS PLAYER MANAGEMENT

			if(useanim->cur_anim == alist[ANIM_BARE_WAIT]) {
				AimTime = 0;

				if(EERIEMouseButton & 1) {
					AcquireLastAnim(io);
					ANIM_Set(useanim, alist[ANIM_BARE_STRIKE_LEFT_START+CurrFightPos*3]);
					io->isHit = false;
				}
			}

			// Now go for strike cycle...
			for(long j = 0; j < 4; j++) {
				if(useanim->cur_anim == alist[ANIM_BARE_STRIKE_LEFT_START+j*3] && (useanim->flags & EA_ANIMEND)) {
					AcquireLastAnim(io);
					ANIM_Set(useanim, alist[ANIM_BARE_STRIKE_LEFT_CYCLE+j*3]);
					AimTime = (unsigned long)(arxtime);
					useanim->flags |= EA_LOOP;
				} else if(useanim->cur_anim == alist[ANIM_BARE_STRIKE_LEFT_CYCLE+j*3] && !(EERIEMouseButton & 1)) {
					AcquireLastAnim(io);
					ANIM_Set(useanim, alist[ANIM_BARE_STRIKE_LEFT+j*3]);

					strikeSpeak(io);

					SendIOScriptEvent(io, SM_STRIKE, "bare");
					PlayerWeaponBlocked = -1;
					CurrFightPos = 0;
					AimTime = 0;
				} else if(useanim->cur_anim == alist[ANIM_BARE_STRIKE_LEFT+j*3]) {
					if(useanim->flags & EA_ANIMEND) {
						AcquireLastAnim(io);
						ANIM_Set(useanim, alist[ANIM_BARE_WAIT]);
						useanim->flags |= EA_LOOP;
						CurrFightPos = 0;
						AimTime = (unsigned long)(arxtime);
						PlayerWeaponBlocked = -1;
					} else if(useanim->ctime > useanim->cur_anim->anims[useanim->altidx_cur]->anim_time * 0.2f
							&& useanim->ctime < useanim->cur_anim->anims[useanim->altidx_cur]->anim_time * 0.8f
							&& PlayerWeaponBlocked == -1
					) {
						if(useanim->cur_anim == alist[ANIM_BARE_STRIKE_LEFT]) {
							long id = io->obj->fastaccess.left_attach;

							if(id != -1) {
								EERIE_SPHERE sphere;
								sphere.origin = io->obj->vertexlist3[id].v;
								sphere.radius = 25.f;

								long num;

								if(CheckAnythingInSphere(&sphere, 0, 0, &num)) {
									float dmgs = (player.Full_damages + 1) * STRIKE_AIMTIME;

									if(ARX_DAMAGES_TryToDoDamage(&io->obj->vertexlist3[id].v, dmgs, 40, 0)) {
										PlayerWeaponBlocked = useanim->ctime;
									}

									ARX_PARTICLES_Spawn_Spark(&sphere.origin, dmgs, 2);

									if(ValidIONum(num)) {
										ARX_SOUND_PlayCollision(entities[num]->material, MATERIAL_FLESH, 1.f, 1.f, &sphere.origin, NULL);
									}
								}
							}
						} else { // Strike Right
							long id = io->obj->fastaccess.primary_attach;

							if(id != -1) {
								EERIE_SPHERE sphere;
								sphere.origin = io->obj->vertexlist3[id].v;
								sphere.radius = 25.f;

								long num;

								if(CheckAnythingInSphere(&sphere, 0, 0, &num)) {
									float dmgs = (player.Full_damages + 1) * STRIKE_AIMTIME;

									if(ARX_DAMAGES_TryToDoDamage(&io->obj->vertexlist3[id].v, dmgs, 40, 0)) {
										PlayerWeaponBlocked = useanim->ctime;
									}

									ARX_PARTICLES_Spawn_Spark(&sphere.origin, dmgs, 2);

									if(ValidIONum(num)) {
										ARX_SOUND_PlayCollision(entities[num]->material, MATERIAL_FLESH, 1.f, 1.f, &sphere.origin, NULL);
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
			if(useanim->cur_anim == alist[ANIM_DAGGER_WAIT]) {
				AimTime = 0;

				if(EERIEMouseButton & 1) {
					AcquireLastAnim(io);
					ANIM_Set(useanim, alist[ANIM_DAGGER_STRIKE_LEFT_START+CurrFightPos*3]);
					io->isHit = false;
				}
			}

			// Now go for strike cycle...
			for(long j = 0; j < 4; j++) {
				if(useanim->cur_anim == alist[ANIM_DAGGER_STRIKE_LEFT_START+j*3] && (useanim->flags & EA_ANIMEND)) {
					AcquireLastAnim(io);
					ANIM_Set(useanim, alist[ANIM_DAGGER_STRIKE_LEFT_CYCLE+j*3]);
					AimTime = (unsigned long)(arxtime);
					useanim->flags |= EA_LOOP;
				} else if(useanim->cur_anim == alist[ANIM_DAGGER_STRIKE_LEFT_CYCLE+j*3] && !(EERIEMouseButton & 1)) {
					AcquireLastAnim(io);
					ANIM_Set(useanim, alist[ANIM_DAGGER_STRIKE_LEFT+j*3]);

					strikeSpeak(io);

					SendIOScriptEvent(io, SM_STRIKE, "dagger");
					CurrFightPos = 0;
					AimTime = 0;
				} else if(useanim->cur_anim == alist[ANIM_DAGGER_STRIKE_LEFT+j*3]) {
					if(useanim->ctime > useanim->cur_anim->anims[useanim->altidx_cur]->anim_time * 0.3f
						&& useanim->ctime < useanim->cur_anim->anims[useanim->altidx_cur]->anim_time * 0.7f)
					{
						if(PlayerWeaponBlocked == -1
							&& ARX_EQUIPMENT_Strike_Check(io, entities[player.equiped[EQUIP_SLOT_WEAPON]], STRIKE_AIMTIME, 0))
						{
							PlayerWeaponBlocked = useanim->ctime;
						}
					}

					if(useanim->flags & EA_ANIMEND) {
						AcquireLastAnim(io);
						ANIM_Set(useanim, alist[ANIM_DAGGER_WAIT]);
						useanim->flags &= ~(EA_PAUSED | EA_REVERSE);
						useanim->flags |= EA_LOOP;
						CurrFightPos = 0;
						AimTime = (unsigned long)(arxtime);
						PlayerWeaponBlocked = -1;
					}

					if(PlayerWeaponBlocked != -1 && useanim->ctime < useanim->cur_anim->anims[useanim->altidx_cur]->anim_time * 0.9f) {
						ARX_EQUIPMENT_Strike_Check(io, entities[player.equiped[EQUIP_SLOT_WEAPON]], STRIKE_AIMTIME, 1);
					}
				}
			}

		break;
		case WEAPON_1H: // 1HANDED PLAYER MANAGEMENT
			// Waiting and Received Strike Impulse
			if(useanim->cur_anim == alist[ANIM_1H_WAIT]) {
				AimTime = 0;

				if(EERIEMouseButton & 1) {
					AcquireLastAnim(io);
					ANIM_Set(useanim, alist[ANIM_1H_STRIKE_LEFT_START+CurrFightPos*3]);
					io->isHit = false;
				}
			}

			// Now go for strike cycle...
			for(long j = 0; j < 4; j++) {
				if(useanim->cur_anim == alist[ANIM_1H_STRIKE_LEFT_START+j*3] && (useanim->flags & EA_ANIMEND)) {
					AcquireLastAnim(io);
					ANIM_Set(useanim, alist[ANIM_1H_STRIKE_LEFT_CYCLE+j*3]);
					AimTime = (unsigned long)(arxtime);
					useanim->flags |= EA_LOOP;
				} else if(useanim->cur_anim == alist[ANIM_1H_STRIKE_LEFT_CYCLE+j*3] && !(EERIEMouseButton & 1)) {
					AcquireLastAnim(io);
					ANIM_Set(useanim, alist[ANIM_1H_STRIKE_LEFT+j*3]);

					strikeSpeak(io);

					SendIOScriptEvent(io, SM_STRIKE, "1h");
					CurrFightPos = 0;
					AimTime = 0;
				} else if(useanim->cur_anim == alist[ANIM_1H_STRIKE_LEFT+j*3]) {
					if(useanim->ctime > useanim->cur_anim->anims[useanim->altidx_cur]->anim_time * 0.3f
						&& useanim->ctime < useanim->cur_anim->anims[useanim->altidx_cur]->anim_time * 0.7f)
					{
						if(PlayerWeaponBlocked == -1
							&& ARX_EQUIPMENT_Strike_Check(io, entities[player.equiped[EQUIP_SLOT_WEAPON]], STRIKE_AIMTIME, 0))
						{
							PlayerWeaponBlocked = useanim->ctime;
						}
					}

					if(useanim->flags & EA_ANIMEND) {
						AcquireLastAnim(io);
						ANIM_Set(useanim, alist[ANIM_1H_WAIT]);
						useanim->flags &= ~(EA_PAUSED | EA_REVERSE);
						useanim->flags |= EA_LOOP;
						CurrFightPos = 0;
						AimTime = 0;
						PlayerWeaponBlocked = -1;
					}

					if(PlayerWeaponBlocked != -1 && useanim->ctime < useanim->cur_anim->anims[useanim->altidx_cur]->anim_time * 0.9f) {
						ARX_EQUIPMENT_Strike_Check(io, entities[player.equiped[EQUIP_SLOT_WEAPON]], STRIKE_AIMTIME, 1);
					}
				}
			}
		break;
		case WEAPON_2H: // 2HANDED PLAYER MANAGEMENT
			// Waiting and Receiving Strike Impulse
			if(useanim->cur_anim == alist[ANIM_2H_WAIT]) {
				AimTime = 0;

				if(EERIEMouseButton & 1) {
					AcquireLastAnim(io);
					ANIM_Set(useanim, alist[ANIM_2H_STRIKE_LEFT_START+CurrFightPos*3]);
					io->isHit = false;
				}
			}

			// Now go for strike cycle...
			for(long j = 0; j < 4; j++) {
				if(useanim->cur_anim == alist[ANIM_2H_STRIKE_LEFT_START+j*3] && (useanim->flags & EA_ANIMEND)) {
					AcquireLastAnim(io);
					ANIM_Set(useanim, alist[ANIM_2H_STRIKE_LEFT_CYCLE+j*3]);
					AimTime = (unsigned long)(arxtime);
					useanim->flags |= EA_LOOP;
				} else if(useanim->cur_anim == alist[ANIM_2H_STRIKE_LEFT_CYCLE+j*3] && !(EERIEMouseButton & 1)) {
					AcquireLastAnim(io);
					ANIM_Set(useanim, alist[ANIM_2H_STRIKE_LEFT+j*3]);

					strikeSpeak(io);

					SendIOScriptEvent(io, SM_STRIKE, "2h");
					CurrFightPos = 0;
					AimTime = 0;
				} else if(useanim->cur_anim == alist[ANIM_2H_STRIKE_LEFT+j*3]) {
					if(useanim->ctime > useanim->cur_anim->anims[useanim->altidx_cur]->anim_time * 0.3f
						&& useanim->ctime < useanim->cur_anim->anims[useanim->altidx_cur]->anim_time * 0.7f)
					{
						if(PlayerWeaponBlocked == -1
							&& ARX_EQUIPMENT_Strike_Check(io, entities[player.equiped[EQUIP_SLOT_WEAPON]], STRIKE_AIMTIME, 0))
						{
							PlayerWeaponBlocked = useanim->ctime;
						}
					}

					if(useanim->flags & EA_ANIMEND) {
						AcquireLastAnim(io);
						ANIM_Set(useanim, alist[ANIM_2H_WAIT]);
						useanim->flags &= ~(EA_PAUSED | EA_REVERSE);
						useanim->flags |= EA_LOOP;
						CurrFightPos = 0;
						AimTime = 0;
						PlayerWeaponBlocked = -1;
					}

					if(PlayerWeaponBlocked != -1 && useanim->ctime < useanim->cur_anim->anims[useanim->altidx_cur]->anim_time * 0.9f) {
						ARX_EQUIPMENT_Strike_Check(io, entities[player.equiped[EQUIP_SLOT_WEAPON]], STRIKE_AIMTIME, 1);
					}
				}
			}
		break;
		case WEAPON_BOW: // MISSILE PLAYER MANAGEMENT
			if(useanim->cur_anim == alist[ANIM_MISSILE_STRIKE_CYCLE]) {
				if(GLOBAL_SLOWDOWN != 1.f)
					BOW_FOCAL += Original_framedelay;
				else
					BOW_FOCAL += framedelay;

				if(BOW_FOCAL > 710)
					BOW_FOCAL = 710;
			}

			// Waiting and Receiving Strike Impulse
			if(useanim->cur_anim == alist[ANIM_MISSILE_WAIT]) {
				AimTime = (unsigned long)(arxtime);

				if((EERIEMouseButton & 1) && Player_Arrow_Count() > 0) {
					AcquireLastAnim(io);
					ANIM_Set(useanim, alist[ANIM_MISSILE_STRIKE_PART_1]);
					io->isHit = false;
				}
			}

			if(useanim->cur_anim == alist[ANIM_MISSILE_STRIKE_PART_1] && (useanim->flags & EA_ANIMEND)) {
				AimTime = 0;
				AcquireLastAnim(io);
				ANIM_Set(useanim, alist[ANIM_MISSILE_STRIKE_PART_2]);

				EERIE_LINKEDOBJ_LinkObjectToObject(io->obj, arrowobj, "left_attach", "attach", NULL);
			}

			// Now go for strike cycle...
			if(useanim->cur_anim == alist[ANIM_MISSILE_STRIKE_PART_2] && (useanim->flags & EA_ANIMEND)) {
				AcquireLastAnim(io);
				ANIM_Set(useanim, alist[ANIM_MISSILE_STRIKE_CYCLE]);
				AimTime = (unsigned long)(arxtime);

				useanim->flags |= EA_LOOP;
			} else if(useanim->cur_anim == alist[ANIM_MISSILE_STRIKE_CYCLE] && !(EERIEMouseButton & 1)) {
				EERIE_LINKEDOBJ_UnLinkObjectFromObject(io->obj, arrowobj);
				AcquireLastAnim(io);
				ANIM_Set(useanim,alist[ANIM_MISSILE_STRIKE]);
				SendIOScriptEvent(io, SM_STRIKE, "bow");
				StrikeAimtime();
				STRIKE_AIMTIME = (float)(BOW_FOCAL)/710.f;
				Entity * ioo = Player_Arrow_Count_Decrease();
				float poisonous = 0.f;

				if(ioo) {
					poisonous = ioo->poisonous;
					if(ioo->poisonous_count > 0) {
						ioo->poisonous_count--;

						if(ioo->poisonous_count <= 0)
							ioo->poisonous = 0;
					}

					ARX_DAMAGES_DurabilityLoss(ioo, 1.f);

					if(ValidIOAddress(ioo)) {
						if(ioo->durability <= 0.f)
							ARX_INTERACTIVE_DestroyIO(ioo);
					}
				}

				float aimratio = STRIKE_AIMTIME;

				if(sp_max && poisonous < 3.f)
					poisonous = 3.f;

				Vec3f orgPos;
				orgPos.x=player.pos.x;
				orgPos.y=player.pos.y+40.f;
				orgPos.z=player.pos.z;

				if(entities.player()->obj->fastaccess.left_attach >= 0) {
					orgPos = entities.player()->obj->vertexlist3[entities.player()->obj->fastaccess.left_attach].v;
				}

				Anglef orgAngle = player.angle;

				PlayerLaunchArrow_Test(aimratio, poisonous, &orgPos, &orgAngle);

				if(sp_max) {
					Anglef angle;
					Vec3f pos;
					pos.x = player.pos.x;
					pos.y = player.pos.y + 40.f;
					pos.z = player.pos.z;
					angle.setYaw(player.angle.getYaw());
					angle.setPitch(player.angle.getPitch() + 8);
					angle.setRoll(player.angle.getRoll());
					PlayerLaunchArrow_Test(aimratio, poisonous, &pos, &angle);
					angle.setYaw(player.angle.getYaw());
					angle.setPitch(player.angle.getPitch() - 8);
					PlayerLaunchArrow_Test(aimratio, poisonous, &pos, &angle);
					angle.setYaw(player.angle.getYaw());
					angle.setPitch(player.angle.getPitch() + 4.f);
					PlayerLaunchArrow_Test(aimratio, poisonous, &pos, &angle);
					angle.setYaw(player.angle.getYaw());
					angle.setPitch(player.angle.getPitch() - 4.f);
					PlayerLaunchArrow_Test(aimratio, poisonous, &pos, &angle);
				}

				AimTime = 0;
			} else if(useanim->cur_anim == alist[ANIM_MISSILE_STRIKE]) {
				BOW_FOCAL -= Original_framedelay;

				if(BOW_FOCAL < 0)
					BOW_FOCAL = 0;

				if(useanim->flags & EA_ANIMEND) {
					BOW_FOCAL = 0;
					AcquireLastAnim(io);
					ANIM_Set(useanim, alist[ANIM_MISSILE_WAIT]);
					useanim->flags |= EA_LOOP;
					AimTime = 0;
					PlayerWeaponBlocked = -1;
					EERIE_LINKEDOBJ_UnLinkObjectFromObject(io->obj, arrowobj);
				}
			}
		break;
	}

	LAST_WEAPON_TYPE = weapontype;
}

void ManageCombatModeAnimationsEND()
{
	Entity * io=entities.player();
	ANIM_USE * useanim=&io->animlayer[1];

	ANIM_USE * useanim3=&io->animlayer[3];
	ANIM_HANDLE ** alist=io->anims;

	if(useanim->cur_anim
		&&(		(useanim->cur_anim == alist[ANIM_BARE_READY])
			||	(useanim->cur_anim == alist[ANIM_DAGGER_READY_PART_2])
			||	(useanim->cur_anim == alist[ANIM_DAGGER_READY_PART_1])
			||	(useanim->cur_anim == alist[ANIM_1H_READY_PART_2])
			||	(useanim->cur_anim == alist[ANIM_1H_READY_PART_1])
			||	(useanim->cur_anim == alist[ANIM_2H_READY_PART_2])
			||	(useanim->cur_anim == alist[ANIM_2H_READY_PART_1])
			||	(useanim->cur_anim == alist[ANIM_MISSILE_READY_PART_1])
			||	(useanim->cur_anim == alist[ANIM_MISSILE_READY_PART_2])	)
	) {
		AimTime = (unsigned long)(arxtime);
	}

	if(useanim->flags & EA_ANIMEND) {
		WeaponType weapontype = ARX_EQUIPMENT_GetPlayerWeaponType();

		if(useanim->cur_anim &&
			(	(useanim->cur_anim == io->anims[ANIM_BARE_UNREADY])
			||	(useanim->cur_anim == io->anims[ANIM_DAGGER_UNREADY_PART_2])
			||	(useanim->cur_anim == io->anims[ANIM_1H_UNREADY_PART_2])
			||	(useanim->cur_anim == io->anims[ANIM_2H_UNREADY_PART_2])
			||	(useanim->cur_anim == io->anims[ANIM_MISSILE_UNREADY_PART_2])	)
		) {
			io->_npcdata->ex_rotate->flags |= EXTRA_ROTATE_REALISTIC;
			AcquireLastAnim(io);
			useanim->cur_anim = NULL;
		}

		switch(weapontype) {
			case WEAPON_BARE:
				// Is Weapon Ready ? In this case go to Fight Wait anim
				if(useanim->cur_anim == alist[ANIM_BARE_READY]) {

					AcquireLastAnim(io);

					if(player.Interface & INTER_NO_STRIKE) {
						player.Interface &= ~INTER_NO_STRIKE;
						ANIM_Set(useanim, alist[ANIM_BARE_WAIT]);
						useanim->flags |= EA_LOOP;
					} else {
						ANIM_Set(useanim, alist[ANIM_BARE_STRIKE_LEFT_START+CurrFightPos*3]);
					}

					AimTime = (unsigned long)(arxtime);
					io->isHit = false;
				}

			break;
			case WEAPON_DAGGER: // DAGGER ANIMS end

				if(alist[ANIM_DAGGER_READY_PART_1]) {
					if(useanim->cur_anim == alist[ANIM_DAGGER_READY_PART_1]) {
						AcquireLastAnim(io);
						ARX_EQUIPMENT_AttachPlayerWeaponToHand();
						ANIM_Set(useanim, alist[ANIM_DAGGER_READY_PART_2]);
					} else if(useanim->cur_anim == alist[ANIM_DAGGER_READY_PART_2]) {
						AcquireLastAnim(io);

						if(player.Interface & INTER_NO_STRIKE) {
							player.Interface &= ~INTER_NO_STRIKE;
							ANIM_Set(useanim, alist[ANIM_DAGGER_WAIT]);
							useanim->flags |= EA_LOOP;
						} else {
							ANIM_Set(useanim, alist[ANIM_DAGGER_STRIKE_LEFT_START+CurrFightPos*3]);
						}

						AimTime = (unsigned long)(arxtime);
						io->isHit = false;
					} else if(useanim->cur_anim == alist[ANIM_DAGGER_UNREADY_PART_1]) {
						AcquireLastAnim(io);
						ARX_EQUIPMENT_AttachPlayerWeaponToBack();
						ANIM_Set(useanim, alist[ANIM_DAGGER_UNREADY_PART_2]);
					}
				}

			break;
			case WEAPON_1H:	// 1H ANIMS end

				if(alist[ANIM_1H_READY_PART_1]) {
					if(useanim->cur_anim == alist[ANIM_1H_READY_PART_1]) {
						AcquireLastAnim(io);
						ARX_EQUIPMENT_AttachPlayerWeaponToHand();
						ANIM_Set(useanim, alist[ANIM_1H_READY_PART_2]);
					} else if(useanim->cur_anim == alist[ANIM_1H_READY_PART_2]) {
						AcquireLastAnim(io);

						if(player.Interface & INTER_NO_STRIKE) {
							player.Interface &= ~INTER_NO_STRIKE;
							ANIM_Set(useanim, alist[ANIM_1H_WAIT]);
							useanim->flags |= EA_LOOP;
						} else {
							ANIM_Set(useanim, alist[ANIM_1H_STRIKE_LEFT_START+CurrFightPos*3]);
						}

						AimTime = (unsigned long)(arxtime);
						io->isHit = false;
					} else if (useanim->cur_anim == alist[ANIM_1H_UNREADY_PART_1]) {
						AcquireLastAnim(io);
						ARX_EQUIPMENT_AttachPlayerWeaponToBack();
						ANIM_Set(useanim, alist[ANIM_1H_UNREADY_PART_2]);
					}
				}

			break;
			case WEAPON_2H:	// 2H ANIMS end

				if(alist[ANIM_2H_READY_PART_1]) {
					if(useanim->cur_anim == alist[ANIM_2H_READY_PART_1]) {
						AcquireLastAnim(io);
						ARX_EQUIPMENT_AttachPlayerWeaponToHand();
						ANIM_Set(useanim, alist[ANIM_2H_READY_PART_2]);
					} else if(useanim->cur_anim == alist[ANIM_2H_READY_PART_2]) {
						AcquireLastAnim(io);

						if(player.Interface & INTER_NO_STRIKE) {
							player.Interface &= ~INTER_NO_STRIKE;
							ANIM_Set(useanim, alist[ANIM_2H_WAIT]);
							useanim->flags |= EA_LOOP;
						} else {
							ANIM_Set(useanim, alist[ANIM_2H_STRIKE_LEFT_START+CurrFightPos*3]);
						}

						AimTime = (unsigned long)(arxtime);
						io->isHit = false;
					} else if(useanim->cur_anim == alist[ANIM_2H_UNREADY_PART_1]) {
						AcquireLastAnim(io);
						ARX_EQUIPMENT_AttachPlayerWeaponToBack();
						ANIM_Set(useanim, alist[ANIM_2H_UNREADY_PART_2]);
					}
				}

			break;
			case WEAPON_BOW:// MISSILE Weapon ANIMS end

				if(alist[ANIM_MISSILE_READY_PART_1]) {
					if(useanim->cur_anim == alist[ANIM_MISSILE_READY_PART_1]) {
						AcquireLastAnim(io);
						ARX_EQUIPMENT_AttachPlayerWeaponToHand();
						ANIM_Set(useanim, alist[ANIM_MISSILE_READY_PART_2]);
					} else if(useanim->cur_anim == alist[ANIM_MISSILE_READY_PART_2]) {
						if(Player_Arrow_Count() > 0) {
							AcquireLastAnim(io);

							if(player.Interface & INTER_NO_STRIKE) {
								player.Interface &= ~INTER_NO_STRIKE;
								ANIM_Set(useanim, alist[ANIM_MISSILE_WAIT]);
								useanim->flags |= EA_LOOP;
							} else {
								ANIM_Set(useanim, alist[ANIM_MISSILE_STRIKE_PART_1]);
							}

							io->isHit = false;
						} else {
							AcquireLastAnim(io);
							ANIM_Set(useanim, alist[ANIM_MISSILE_WAIT]);
						}

						EERIE_LINKEDOBJ_UnLinkObjectFromObject(io->obj, arrowobj);
					} else if(useanim->cur_anim == alist[ANIM_MISSILE_STRIKE_PART_1]) {
						ANIM_Set(useanim, alist[ANIM_MISSILE_STRIKE_PART_2]);
					} else if(useanim->cur_anim == alist[ANIM_MISSILE_STRIKE_PART_2]) {
						ANIM_Set(useanim, alist[ANIM_MISSILE_STRIKE_CYCLE]);
					} else if(useanim->cur_anim == alist[ANIM_MISSILE_UNREADY_PART_1]) {
						AcquireLastAnim(io);
						ARX_EQUIPMENT_AttachPlayerWeaponToBack();
						ANIM_Set(useanim, alist[ANIM_MISSILE_UNREADY_PART_2]);
					}
				}

			break;
		}

		// Spell casting anims
		if(alist[ANIM_CAST] && useanim->cur_anim == alist[ANIM_CAST]) {
			AcquireLastAnim(io);

			if(alist[ANIM_CAST_END])
				ANIM_Set(useanim, alist[ANIM_CAST_END]);
		} else if(alist[ANIM_CAST_END] && useanim->cur_anim == alist[ANIM_CAST_END]) {
			AcquireLastAnim(io);
			useanim->cur_anim = NULL;
			player.doingmagic = 0;

			if(WILLRETURNTOCOMBATMODE) {
				player.Interface |= INTER_COMBATMODE;
				player.Interface |= INTER_NO_STRIKE;

				ARX_EQUIPMENT_LaunchPlayerReadyWeapon();
				WILLRETURNTOCOMBATMODE = false;
			}
		}
	}

	// Is the shield off ?
	if(useanim3->flags & EA_ANIMEND) {
		if(io->anims[ANIM_SHIELD_END] && useanim3->cur_anim == io->anims[ANIM_SHIELD_END]) {
			AcquireLastAnim(io);
			useanim3->cur_anim = NULL;
		}
	}
}

float LAST_FADEVALUE=1.f;
void ManageFade()
{
	float tim = arxtime.get_updated() - (float)FADESTART;

	if(tim <= 0.f)
		return;

	float Visibility = tim / (float)FADEDURATION;

	if(FADEDIR > 0)
		Visibility = 1.f - Visibility;

	if(Visibility > 1.f)
		Visibility = 1.f;

	if(Visibility < 0.f) {
		FADEDIR = 0;
		return;
	}

	LAST_FADEVALUE=Visibility;
	GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	
	EERIEDrawBitmap(g_size, 0.0001f, NULL, Color::gray(Visibility));

	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	float col=Visibility;
	EERIEDrawBitmap(g_size, 0.0001f, NULL, Color(col * FADECOLOR.r, col * FADECOLOR.g, col * FADECOLOR.b));

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
}

void DrawImproveVisionInterface() {

	if(ombrignon) {
		float mod = 0.6f + PULSATE * 0.35f;
		EERIEDrawBitmap(g_size, 0.0001f, ombrignon, Color3f((0.5f+PULSATE*( 1.0f / 10 ))*mod,0.f,0.f).to<u8>());
	}
}

void AddQuakeFX(float intensity,float duration,float period,long flags) {

	if(QuakeFx.intensity > 0.f) {
		QuakeFx.intensity += intensity;

		QuakeFx.duration += (unsigned long)duration;
		QuakeFx.frequency += period;
		QuakeFx.frequency *= .5f;
		QuakeFx.flags |= flags;

		if(flags & 1)
			ARX_SOUND_PlaySFX(SND_QUAKE, NULL, 1.0F - 0.5F * QuakeFx.intensity);
	} else {
		QuakeFx.intensity = intensity;

		QuakeFx.start = checked_range_cast<unsigned long>(arxtime.get_frame_time());

		QuakeFx.duration = (unsigned long)duration;
		QuakeFx.frequency = period;
		QuakeFx.flags = flags;

		if(flags & 1)
			ARX_SOUND_PlaySFX(SND_QUAKE, NULL, 1.0F - 0.5F * QuakeFx.intensity);
	}

	if(!(flags & 1)) {
		if(QuakeFx.duration > 1500)
			QuakeFx.duration = 1500;

		if(QuakeFx.intensity > 220)
			QuakeFx.intensity = 220;
	}
}

void ManageQuakeFX(EERIE_CAMERA * cam) {
	if(QuakeFx.intensity>0.f) {
		float tim=(float)arxtime.get_frame_time()-(float)QuakeFx.start;

		if(tim >= QuakeFx.duration) {
			QuakeFx.intensity=0.f;
			return;
		}

		float itmod=1.f-(tim/QuakeFx.duration);
		float periodicity=EEsin((float)arxtime.get_frame_time()*QuakeFx.frequency*( 1.0f / 100 ));

		if(periodicity > 0.5f && (QuakeFx.flags & 1))
			ARX_SOUND_PlaySFX(SND_QUAKE, NULL, 1.0F - 0.5F * QuakeFx.intensity);

		float truepower = periodicity * QuakeFx.intensity * itmod * 0.01f;
		float halfpower = truepower * .5f;
		cam->orgTrans.pos += randomVec(-halfpower, halfpower);
		cam->angle.setYaw(cam->angle.getYaw() + rnd() * truepower - halfpower);
		cam->angle.setPitch(cam->angle.getPitch() + rnd() * truepower - halfpower);
		cam->angle.setRoll(cam->angle.getRoll() + rnd() * truepower - halfpower);
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
	DanaeClearLevel();
	PROGRESS_BAR_COUNT+=2.f;
	LoadLevelScreen();
	DanaeLoadLevel(loadfrom);
	FirstFrame=true;
	START_NEW_QUEST=0;
	BLOCK_PLAYER_CONTROLS = false;
	FADEDURATION=0;
	FADEDIR=0;
	player.Interface = INTER_LIFE_MANA | INTER_MINIBACK | INTER_MINIBOOK;
}

LevelNumber LEVEL_TO_LOAD = LEVEL10;

void loadLevel(u32 lvl) {
	
	if(lvl < NOLEVEL) {
		if(GameFlow::getTransition() != GameFlow::LoadingScreen) {
			LEVEL_TO_LOAD = static_cast<LevelNumber>(lvl);
			GameFlow::setTransition(GameFlow::LoadingScreen);
		}
	}
}
ARX_PROGRAM_OPTION("loadlevel", "", "Load a specific level", &loadLevel, "LEVELID");

extern long LOADQUEST_SLOT;
void loadSlot(u32 saveSlot) {

	LOADQUEST_SLOT = saveSlot;
	GameFlow::setTransition(GameFlow::InGame);
}
ARX_PROGRAM_OPTION("loadslot", "", "Load a specific savegame slot", &loadSlot, "SAVESLOT");

void skipLogo() {
	loadLevel(LEVEL10);
}
ARX_PROGRAM_OPTION("skiplogo", "", "Skip logos at startup", &skipLogo);

bool HandleGameFlowTransitions() {
	
	const int TRANSITION_DURATION = 3600;
	static unsigned long TRANSITION_START = 0;

	if(GameFlow::getTransition() == GameFlow::NoTransition) {
		return false;
	}

	if(GInput->isAnyKeyPressed()) {
		ARXmenu.currentmode = AMCM_MAIN;
		ARX_MENU_Launch(false);
		GameFlow::setTransition(GameFlow::InGame);
	}
		
	if(GameFlow::getTransition() == GameFlow::FirstLogo) {
		//firsttime
		if(TRANSITION_START == 0) {
			if(!ARX_INTERFACE_InitFISHTANK()) {
				GameFlow::setTransition(GameFlow::SecondLogo);
				return true;
			}

			TRANSITION_START = arxtime.get_updated_ul();
		}

		ARX_INTERFACE_ShowFISHTANK();

		unsigned long tim = arxtime.get_updated_ul();
		float pos = (float)tim - (float)TRANSITION_START;

		if(pos > TRANSITION_DURATION) {
			TRANSITION_START = 0;
			GameFlow::setTransition(GameFlow::SecondLogo);
		}

		return true;			
	}

	if(GameFlow::getTransition() == GameFlow::SecondLogo) {
		//firsttime
		if(TRANSITION_START == 0) {
			if(!ARX_INTERFACE_InitARKANE()) {
				GameFlow::setTransition(GameFlow::LoadingScreen);
				return true;
			}

			TRANSITION_START = arxtime.get_updated_ul();
			ARX_SOUND_PlayInterface(SND_PLAYER_HEART_BEAT);
		}

		ARX_INTERFACE_ShowARKANE();
		unsigned long tim = arxtime.get_updated_ul();
		float pos = (float)tim - (float)TRANSITION_START;

		if(pos > TRANSITION_DURATION) {
			TRANSITION_START = 0;
			GameFlow::setTransition(GameFlow::LoadingScreen);
		}

		return true;
	}

	if(GameFlow::getTransition() == GameFlow::LoadingScreen) {
		ARX_INTERFACE_KillFISHTANK();
		ARX_INTERFACE_KillARKANE();
		char loadfrom[256];

		sprintf(loadfrom, "graph/levels/level%d/level%d.dlf", LEVEL_TO_LOAD, LEVEL_TO_LOAD);
		OLD_PROGRESS_BAR_COUNT=PROGRESS_BAR_COUNT=0;
		PROGRESS_BAR_TOTAL = 108;
		LoadLevelScreen(LEVEL_TO_LOAD);

		DanaeLoadLevel(loadfrom);
		GameFlow::setTransition(GameFlow::InGame);
		return true;
	}

	if(GameFlow::getTransition() == GameFlow::InGame) {
		GameFlow::setTransition(GameFlow::NoTransition);
		FirstFrame = true;
		return true;
	}

	return false;
}

void LaunchWaitingCine() {
	
	LogDebug("LaunchWaitingCine " << CINE_PRELOAD);

	if(ACTIVECAM) {
		ePos = ACTIVECAM->orgTrans.pos;
	}
	
	DANAE_KillCinematic();
	
	res::path cinematic = res::path("graph/interface/illustrations") / WILL_LAUNCH_CINE;
	
	if(resources->getFile(cinematic)) {
		
		ControlCinematique->OneTimeSceneReInit();
		
		if(loadCinematic(ControlCinematique, cinematic)) {
			
			if(CINE_PRELOAD) {
				LogDebug("only preloaded cinematic");
				PLAY_LOADED_CINEMATIC = 0;
			} else {
				LogDebug("starting cinematic");
				PLAY_LOADED_CINEMATIC = 1;
				arxtime.pause();
			}
			
			LAST_LAUNCHED_CINE = WILL_LAUNCH_CINE;
		} else {
			LogWarning << "Error loading cinematic " << cinematic;
		}
		
	} else {
		LogWarning << "Could not find cinematic " << cinematic;
	}
	
	WILL_LAUNCH_CINE.clear();
}

// Manages Currently playing 2D cinematic
void DANAE_Manage_Cinematic() {
	
	float FrameTicks = arxtime.get_updated(false);
	
	if(PLAY_LOADED_CINEMATIC == 1) {
		LogDebug("really starting cinematic now");
		LastFrameTicks = FrameTicks;
		PLAY_LOADED_CINEMATIC=2;
	}
	
	PlayTrack(ControlCinematique);
	ControlCinematique->InitDeviceObjects();
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	
	ControlCinematique->Render(FrameTicks - LastFrameTicks);
	
	//fin de l'anim
	if ((!ControlCinematique->key)
		|| (GInput->isKeyPressedNowUnPressed(Keyboard::Key_Escape))
		|| (GInput->isKeyPressedNowUnPressed(Keyboard::Key_Escape)))
	{			
		ControlCinematique->projectload=false;
		StopSoundKeyFramer();
		ControlCinematique->OneTimeSceneReInit();
		ControlCinematique->DeleteDeviceObjects();
		arxtime.resume();
		PLAY_LOADED_CINEMATIC=0;
		
		bool bWasBlocked = false;
		if(BLOCK_PLAYER_CONTROLS) {
			bWasBlocked = true;
		}
		
		// !! avant le cine end
		if(ACTIVECAM) {
			ACTIVECAM->orgTrans.pos = ePos;
		}
		
		if(bWasBlocked) {
			BLOCK_PLAYER_CONTROLS = true;
		}
		
		ARX_SPEECH_Reset();
		SendMsgToAllIO(SM_CINE_END, LAST_LAUNCHED_CINE);
	}
	
	LastFrameTicks = FrameTicks;
}

void ReMappDanaeButton() {
	
	// Handle double clicks.
	const ActionKey & button = config.actions[CONTROLS_CUST_ACTION].key[0];
	if((button.key[0] != -1 && (button.key[0] & Mouse::ButtonBase)
	    && GInput->getMouseButtonDoubleClick(button.key[0], 300))
	   || (button.key[1] != -1 && (button.key[1] & Mouse::ButtonBase)
	    && GInput->getMouseButtonDoubleClick(button.key[1], 300))) {
		EERIEMouseButton |= 4;
		EERIEMouseButton &= ~1;
	}
	
	if(GInput->actionNowPressed(CONTROLS_CUST_ACTION)) {
		if(EERIEMouseButton & 4) {
			EERIEMouseButton &= ~1;
		} else {
			EERIEMouseButton |= 1;
		}
		
	}
	if(GInput->actionNowReleased(CONTROLS_CUST_ACTION)) {
		EERIEMouseButton &= ~1;
		EERIEMouseButton &= ~4;
	}
	
	if(GInput->actionNowPressed(CONTROLS_CUST_USE)) {
		EERIEMouseButton |= 2;
	}
	if(GInput->actionNowReleased(CONTROLS_CUST_USE)) {
		EERIEMouseButton &= ~2;
	}
	
}

void AdjustMousePosition()
{
	if (EERIEMouseGrab && GInput->hasMouseMoved())
	{
		Vec2s pos;
		pos.x = (short)(g_size.width() >> 1);
		pos.y = (short)(g_size.height() >> 1);

		if (!((ARXmenu.currentmode == AMCM_NEWQUEST)
				||	(player.Interface & INTER_MAP && (Book_Mode != BOOKMODE_MINIMAP)))) {
			GInput->setMousePosAbs(pos);
		}
	}
}

void ShowTestText()
{
	char tex[256];

	mainApp->outputText(0, 16, arx_version);

	sprintf(tex,"Level : %s", LastLoadedScene.string().c_str());
	hFontDebug->draw(0, 32, tex, Color::white);

	sprintf(tex,"Position : %5.0f %5.0f %5.0f",player.pos.x,player.pos.y,player.pos.z);
	hFontDebug->draw(0, 48, tex, Color::white);

	sprintf( tex,"Last Failed Sequence : %s",LAST_FAILED_SEQUENCE.c_str() );
	hFontDebug->draw(0, 64, tex, Color::white);
}

extern float CURRENT_PLAYER_COLOR;

void ShowInfoText() {
	
	unsigned long uGAT = (unsigned long)(arxtime) / 1000;
	long GAT=(long)uGAT;
	char tex[256];
	float fpss2=1000.f/framedelay;
	LASTfpscount++;
	
	float fps2v = std::max(fpss2, LASTfps2);
	float fps2vmin = std::min(fpss2, LASTfps2);
	
	if(LASTfpscount > 49)  {
		LASTfps2 = 0;
		LASTfpscount = 0;
		fps2 = fps2v;
		fps2min = fps2vmin;
	} else {
		LASTfps2 = fpss2;
	}
	
	sprintf(tex, "%ld Prims %4.02f fps ( %3.02f - %3.02f ) [%3.0fms]",
			EERIEDrawnPolys, FPS, fps2min, fps2, framedelay);
	hFontDebug->draw(70, 32, tex, Color::white);

	float poss = -666.66f;
	EERIEPOLY * ep = CheckInPoly(player.pos.x, player.pos.y, player.pos.z);
	float tempo = 0.f;

	if(ep && GetTruePolyY(ep, &player.pos, &tempo))
		poss = tempo;

	sprintf(tex, "Position  x:%7.0f y:%7.0f [%7.0f] z:%6.0f a%3.0f b%3.0f FOK %3.0f",
			player.pos.x, player.pos.y + player.size.y, poss, player.pos.z,
			player.angle.getYaw(), player.angle.getPitch(),
			ACTIVECAM->focal);
	hFontDebug->draw(70, 48, tex, Color::white);

	sprintf(tex, "AnchorPos x:%6.0f y:%6.0f z:%6.0f TIME %lds Part %ld - %d",
			player.pos.x - Mscenepos.x,
			player.pos.y + player.size.y - Mscenepos.y,
			player.pos.z - Mscenepos.z,
			GAT, getParticleCount(), player.doingmagic);
	hFontDebug->draw(70, 64, tex, Color::white);

	if(player.onfirmground == 0)
		hFontDebug->draw(200, 280, "OFFGRND", Color::white);

	sprintf(tex, "Jump %f cinema %f %d %d - Pathfind %ld(%s)",
			player.jumplastposition, CINEMA_DECAL,
			DANAEMouse.x, DANAEMouse.y,
			EERIE_PATHFINDER_Get_Queued_Number(),
			PATHFINDER_WORKING ? "Working" : "Idled");
	hFontDebug->draw(70, 80, tex, Color::white);

	Entity * io=ARX_SCRIPT_Get_IO_Max_Events();

	if(!io) {
		sprintf(tex, "Events %ld (IOmax N/A) Timers %ld",
				ScriptEvent::totalCount, ARX_SCRIPT_CountTimers());
	} else {
		sprintf(tex, "Events %ld (IOmax %s %d) Timers %ld",
				ScriptEvent::totalCount, io->long_name().c_str(),
				io->stat_count, ARX_SCRIPT_CountTimers());
	}
	hFontDebug->draw(70, 94, tex, Color::white);

	io = ARX_SCRIPT_Get_IO_Max_Events_Sent();

	if(io) {
		sprintf(tex, "Max SENDER %s %d)", io->long_name().c_str(), io->stat_sent);
		hFontDebug->draw(70, 114, tex, Color::white);
	}

	float slope = 0.f;
	ep = CheckInPoly(player.pos.x, player.pos.y - 10.f, player.pos.z);

	if(ep)
		slope = ep->norm.y;

	sprintf(tex, "Velocity %3.0f %3.0f %3.0f Slope %3.3f",
			player.physics.velocity.x, player.physics.velocity.y, player.physics.velocity.z, slope);
	hFontDebug->draw(70, 128, tex, Color::white);

#ifdef BUILD_EDITOR
	if(ValidIONum(LastSelectedIONum)) {
		io = entities[LastSelectedIONum];

		if(io) {
			if(io == entities.player()) {
				sprintf(tex, "%4.0f %4.0f %4.0f - %4.0f %4.0f %4.0f -- %3.0f %d/%ld targ %ld beh %ld",
						io->pos.x, io->pos.y, io->pos.z,
						io->move.x, io->move.y, io->move.z,
						io->_npcdata->moveproblem, io->_npcdata->pathfind.listpos, io->_npcdata->pathfind.listnb,
						io->_npcdata->pathfind.truetarget, (long)io->_npcdata->behavior);
				hFontDebug->draw(170, 420, tex, Color::white);

				sprintf(tex, "Life %4.0f/%4.0f Mana %4.0f/%4.0f Poisoned %3.1f Hunger %4.1f",
						player.life, player.maxlife, player.mana, player.maxmana, player.poison, player.hunger);
				hFontDebug->draw(170, 320, tex, Color::white);
			} else {
				if(io->ioflags & IO_NPC) {
					sprintf(tex, "%4.0f %4.0f %4.0f - %4.0f %4.0f %4.0f -- %3.0f %d/%ld targ %ld beh %ld",
							io->pos.x, io->pos.y, io->pos.z,
							io->move.x, io->move.y, io->move.z,
							io->_npcdata->moveproblem, io->_npcdata->pathfind.listpos, io->_npcdata->pathfind.listnb,
							io->_npcdata->pathfind.truetarget, (long)io->_npcdata->behavior);
					hFontDebug->draw(170, 420, tex, Color::white);

					sprintf(tex, "Life %4.0f/%4.0f Mana %4.0f/%4.0f Poisoned %3.1f",
							io->_npcdata->life, io->_npcdata->maxlife, io->_npcdata->mana,
							io->_npcdata->maxmana, io->_npcdata->poisonned);
					hFontDebug->draw(170, 320, tex, Color::white);

					sprintf(tex, "AC %3.0f Absorb %3.0f",
							ARX_INTERACTIVE_GetArmorClass(io), io->_npcdata->absorb);
					hFontDebug->draw(170, 335, tex, Color::white);

					if(io->_npcdata->pathfind.flags & PATHFIND_ALWAYS) {
						hFontDebug->draw(170, 360, "PF_ALWAYS", Color::white);
					} else {
						sprintf(tex, "PF_%ld", (long)io->_npcdata->pathfind.flags);
						hFontDebug->draw(170, 360, tex, Color::white);
					}
				}

				if(io->ioflags & IO_FIX) {
					sprintf(tex, "Durability %4.0f/%4.0f Poisonous %3d count %d",
							io->durability, io->max_durability, io->poisonous, io->poisonous_count);
					hFontDebug->draw(170, 320, tex, Color::white);
				}

				if(io->ioflags & IO_ITEM) {
					sprintf(tex, "Durability %4.0f/%4.0f Poisonous %3d count %d",
							io->durability, io->max_durability, io->poisonous, io->poisonous_count);
					hFontDebug->draw(170, 320, tex, Color::white);
				}
			}
		}
	}
#endif // BUILD_EDITOR

	long zap = IsAnyPolyThere(player.pos.x,player.pos.z);
	sprintf(tex, "POLY %ld", zap);
	hFontDebug->draw(270, 220, tex, Color::white);

	sprintf(tex, "COLOR %3.0f Stealth %3.0f",
			CURRENT_PLAYER_COLOR, GetPlayerStealth());
	hFontDebug->draw(270, 200, tex, Color::white);

	ARX_SCRIPT_Init_Event_Stats();
}

void ShowFPS() {
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(2) << FPS << " FPS";
	hFontDebug->draw(Vec2i(10, 10), oss.str(), Color::white);
}

void ShowDebugToggles() {
	for(size_t i = 0; i < ARRAY_SIZE(g_debugToggles); i++) {
		std::stringstream textStream;
		textStream << "Toggle " << i << ": " << (g_debugToggles[i] ? "true" : "false");
		hFontDebug->draw(0.f, i * hFontDebug->getLineHeight(), textStream.str(), Color::white);
	}
}

void ShowFpsGraph() {

	GRenderer->ResetTexture(0);
	
	static std::deque<float> lastFPSArray;
	lastFPSArray.push_front(1000 / arxtime.get_frame_delay());

	Vec2i windowSize = mainApp->getWindow()->getSize();
	if(lastFPSArray.size() == size_t(windowSize.x))
	{
		lastFPSArray.pop_back();
	}
	
	float avg = 0;
	float worst = lastFPSArray[0];

	std::vector<TexturedVertex> vertices;
	vertices.resize(lastFPSArray.size());

	const float SCALE_Y = 2.0f;

	for(size_t i = 0; i < lastFPSArray.size(); ++i)
	{
		float time = lastFPSArray[i];

		avg += lastFPSArray[i];
		worst = std::min(worst, lastFPSArray[i]);

		vertices[i].color = 0xFFFFFFFF;
		vertices[i].p.x = i;
		vertices[i].p.y = windowSize.y - (time * SCALE_Y);
		vertices[i].p.z = 1.0f;
		vertices[i].rhw = 1.0f;
	}
	avg /= lastFPSArray.size();
	
	EERIEDRAWPRIM(Renderer::LineStrip, &vertices[0], vertices.size());
	
	Color avgColor = Color::blue * 0.5f + Color::white * 0.5f;
	float avgPos = windowSize.y - (avg * SCALE_Y);
	EERIEDraw2DLine(0, avgPos,  windowSize.x, avgPos, 1.0f, Color::blue);
	
	Color worstColor = Color::red * 0.5f + Color::white * 0.5f;
	float worstPos = windowSize.y - (worst * SCALE_Y);
	EERIEDraw2DLine(0, worstPos,  windowSize.x, worstPos, 1.0f, Color::red);
	
	Font * font = hFontDebug;
	float lineOffset = font->getLineHeight() + 2;
	
	std::string labels[3] = { "Average: ", "Worst: ", "Current: " };
	Color colors[3] = { avgColor, worstColor, Color::white };
	float values[3] = { avg, worst, lastFPSArray[0] };
	
	std::string texts[3];
	float widths[3];
	static float labelWidth = 0.f;
	static float valueWidth = 0.f;
	for(size_t i = 0; i < 3; i++) {
		// Format value
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(2) << values[i] << " FPS";
		texts[i] = oss.str();
		// Calculate widths (could be done more efficiently for monospace fonts...)
		labelWidth = std::max(labelWidth, float(font->getTextSize(labels[i]).x));
		widths[i] = font->getTextSize(texts[i]).x;
		valueWidth = std::max(valueWidth, widths[i]);
	}
	
	float x = 10;
	float y = 10;
	float xend = x + labelWidth + 10 + valueWidth;
	for(size_t i = 0; i < 3; i++) {
		font->draw(Vec2i(x, y), labels[i], Color::gray(0.8f));
		font->draw(Vec2i(xend - widths[i], y), texts[i], colors[i]);
		y += lineOffset;
	}
	
}

void ARX_SetAntiAliasing() {
	GRenderer->SetAntialiasing(config.video.antialiasing);
}

void ReleaseSystemObjects() {
	
	delete hero, hero = NULL;
	
	if(entities.size() > 0 && entities.player() != NULL) {
		entities.player()->obj = NULL; // already deleted above (hero)
		delete entities.player();
		arx_assert(entities.size() > 0 && entities.player() == NULL);
	}
	
	FlyingEye_Release();

	delete cabal, cabal = NULL;
	delete cameraobj, cameraobj = NULL;
	delete markerobj, markerobj = NULL;
	delete arrowobj, arrowobj = NULL;
	
	drawDebugRelease();

	BOOST_FOREACH(EERIE_3DOBJ * & obj, GoldCoinsObj) {
		delete obj, obj = NULL;
	}
}

void shutdownGame() {
	
	ARX_Menu_Resources_Release();
	arxtime.resume();
	
	mainApp->getWindow()->hide();
	
	g_miniMap.purgeTexContainer();
	
	KillInterfaceTextureContainers();
	Menu2_Close();
	DanaeClearLevel(2);
	TextureContainer::DeleteAll();
	
	delete ControlCinematique, ControlCinematique = NULL;
	
	config.save();
	
	RoomDrawRelease();
	EXITING=1;
	TREATZONE_Release();
	ClearTileLights();
	
	// texts and textures
	ClearSysTextures();
	
	delete pParticleManager, pParticleManager = NULL;
	
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
	if(svar) {
		for(long i = 0; i < NB_GLOBALS; i++) {
			free(svar[i].text), svar[i].text = NULL;
		}
		free(svar), svar = NULL;
	}
	
	ARX_SCRIPT_Timer_ClearAll();
	
	delete[] scr_timer, scr_timer = NULL;
	
	//Speech
	ARX_SPEECH_ClearAll();
	ARX_Text_Close();
	
	//object loaders from beforerun
	ReleaseDanaeBeforeRun();
	
	delete resources;
	
	ReleaseNode();

	// Current game
	ARX_Changelevel_CurGame_Clear();
	
	//Halo
	ReleaseHalo();
	FreeSnapShot();
	ARX_INPUT_Release();
	
	mainApp->cleanup3DEnvironment();
	
	
	LogInfo << "Clean shutdown";
}
