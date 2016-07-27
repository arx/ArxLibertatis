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

#include "core/ArxGame.h"

#include <stddef.h>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <sstream>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/foreach.hpp>

#include <glm/gtx/norm.hpp>

#include "ai/PathFinderManager.h"
#include "ai/Paths.h"

#include "animation/Animation.h"
#include "animation/AnimationRender.h"
#include "animation/Intro.h"

#include "cinematic/Cinematic.h"
#include "cinematic/CinematicController.h"

#include "core/Benchmark.h"
#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"
#include "core/Localisation.h"
#include "core/SaveGame.h"
#include "core/URLConstants.h"
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
#include "game/effect/Quake.h"

#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Color.h"
#include "graphics/Draw.h"
#include "graphics/DrawDebug.h"
#include "graphics/GraphicsModes.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/Vertex.h"
#include "graphics/VertexBuffer.h"
#include "graphics/data/FTL.h"
#include "graphics/data/Mesh.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/Fade.h"
#include "graphics/effects/Fog.h"
#include "graphics/font/Font.h"
#include "graphics/opengl/GLDebug.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/ParticleManager.h"
#include "graphics/particle/MagicFlare.h"
#include "graphics/particle/Spark.h"
#include "graphics/texture/TextureStage.h"

#include "gui/Cursor.h"
#include "gui/DebugHud.h"
#include "gui/Hud.h"
#include "gui/Interface.h"
#include "gui/LoadLevelScreen.h"
#include "gui/Menu.h"
#include "gui/MenuPublic.h"
#include "gui/MenuWidgets.h"
#include "gui/MiniMap.h"
#include "gui/Speech.h"
#include "gui/Text.h"
#include "gui/TextManager.h"

#include "input/Input.h"
#include "input/Keyboard.h"

#include "math/Angle.h"
#include "math/Types.h"
#include "math/Rectangle.h"
#include "math/Vector.h"

#include "physics/Attractors.h"

#include "io/fs/FilePath.h"
#include "io/fs/Filesystem.h"
#include "io/fs/SystemPaths.h"
#include "io/resource/PakReader.h"
#include "io/Screenshot.h"
#include "io/log/CriticalLogger.h"
#include "io/log/Logger.h"

#include "platform/Dialog.h"
#include "platform/Platform.h"
#include "platform/Process.h"
#include "platform/ProgramOptions.h"
#include "platform/profiler/Profiler.h"
#include "platform/Time.h"

#include "scene/ChangeLevel.h"
#include "scene/Interactive.h"
#include "scene/GameSound.h"
#include "scene/Light.h"
#include "scene/LoadLevel.h"
#include "scene/Object.h"
#include "scene/Scene.h"

#include "script/ScriptEvent.h"

#include "Configure.h"

#include "window/RenderWindow.h"

#if ARX_HAVE_SDL2
#include "window/SDL2Window.h"
#endif
#if ARX_HAVE_SDL1
#include "window/SDL1Window.h"
#endif

InfoPanels g_debugInfo = InfoPanelNone;

extern bool START_NEW_QUEST;
SavegameHandle LOADQUEST_SLOT = SavegameHandle(); // OH NO, ANOTHER GLOBAL! - TEMP PATCH TO CLEAN CODE FLOW
extern long DeadTime;

static const float CURRENT_BASE_FOCAL = 310.f;
static const float defaultCameraFocal = 350.f;

extern Cinematic* ControlCinematique;
extern EERIE_3DOBJ * arrowobj;

extern Entity * FlyingOverIO;
extern Color ulBKGColor;
extern EERIE_CAMERA conversationcamera;
extern ParticleManager * pParticleManager;
extern CircularVertexBuffer<TexturedVertex> * pDynamicVertexBuffer_TLVERTEX; // VB using TLVERTEX format.
extern CircularVertexBuffer<SMY_VERTEX3> * pDynamicVertexBuffer;

long STOP_KEYBOARD_INPUT= 0;

bool EXTERNALVIEW = false;
bool SHOW_INGAME_MINIMAP = true;

bool ARX_FLARES_Block = true;

Vec3f LASTCAMPOS;
Anglef LASTCAMANGLE;

// ArxGame constructor. Sets attributes for the app.
ArxGame::ArxGame()
	: m_wasResized(false)
	, m_gameInitialized(false)
{}

ArxGame::~ArxGame() {
}

bool ArxGame::initialize()
{
	bool init;
	
	init = initConfig();
	if(!init) {
		LogCritical << "Failed to initialize the config subsystem";
		return false;
	}
	
	init = initWindow();
	if(!init) {
		return false;
	}
	
	init = initGameData();
	if(!init) {
		return false;
	}
	
	init = initInput();
	if(!init) {
		return false;
	}
	
	init = initSound();
	if(!init) {
		return false;
	}
	
	init = initLocalisation();
	if(!init) {
		LogCritical << "Failed to initialize the localisation subsystem";
		return false;
	}
	
	init = initGame();
	if(!init) {
		LogCritical << "Failed to initialize game";
		return false;
	}
	
	return true;
}

static bool migrateFilenames(fs::path path, bool is_dir) {
	
	std::string name = path.filename();
	std::string lowercase = boost::to_lower_copy(name);
	
	bool migrated = true;
	
	if(lowercase != name) {
		
		fs::path dst = path.parent() / lowercase;
		
		LogInfo << "Renaming " << path << " to " << dst.filename();
		
		if(fs::rename(path, dst)) {
			path = dst;
		} else {
			migrated = false;
		}
	}
	
	if(is_dir) {
		for(fs::directory_iterator it(path); !it.end(); ++it) {
			migrated &= migrateFilenames(path / it.name(), it.is_directory());
		}
	}
	
	return migrated;
}

static bool migrateFilenames(const fs::path & configFile) {
	
	LogInfo << "Changing filenames to lowercase...";
	
	static const char * files[] = { "cfg.ini", "cfg_default.ini",
	 "sfx.pak", "loc.pak", "data2.pak", "data.pak", "speech.pak", "loc_default.pak", "speech_default.pak",
	 "save", "editor", "game", "graph", "localisation", "misc", "sfx", "speech" };
	
	std::set<std::string> fileset;
	for(size_t i = 0; i < ARRAY_SIZE(files); i++) {
		fileset.insert(files[i]);
	}
	
	bool migrated = true;
	
	for(fs::directory_iterator it(fs::paths.user); !it.end(); ++it) {
		std::string file = it.name();
		if(fileset.find(boost::to_lower_copy(file)) != fileset.end()) {
			migrated &= migrateFilenames(fs::paths.user / file, it.is_directory());
		}
	}
	
	if(!migrated) {
		LogCritical << "Could not rename all files to lowercase, please do so manually and set migration=1 under [misc] in " << configFile;
	}
	
	return migrated;
}

bool ArxGame::initConfig() {
	
	// Initialize config first, before anything else.
	fs::path configFile = fs::paths.config / "cfg.ini";
	
	config.setOutputFile(configFile);
	
	bool migrated = false;
	if(!fs::exists(configFile)) {
		
		migrated = migrateFilenames(configFile);
		if(!migrated) {
			return false;
		}
		
		fs::path oldConfigFile = fs::paths.user / "cfg.ini";
		if(fs::exists(oldConfigFile)) {
			if(!fs::rename(oldConfigFile, configFile)) {
				LogWarning << "Could not move " << oldConfigFile << " to "
				           << configFile;
			} else {
				LogInfo << "Moved " << oldConfigFile << " to " << configFile;
			}
		}
	}
	
	LogInfo << "Using config file " << configFile;
	if(!config.init(configFile)) {
		
		fs::path file = fs::paths.find("cfg_default.ini");
		if(!config.init(file)) {
			LogWarning << "Could not read config files cfg.ini and cfg_default.ini,"
			           << " using defaults";
		}
		
		// Save a default config file so users have a chance to edit it even if we crash.
		config.save();
	}
	
	Logger::configure(config.misc.debug);
	
	if(!migrated && config.misc.migration < Config::CaseSensitiveFilenames) {
		migrated = migrateFilenames(configFile);
		if(!migrated) {
			return false;
		}
	}
	if(migrated) {
		config.misc.migration = Config::CaseSensitiveFilenames;
	}
	
	if(!fs::create_directories(fs::paths.user / "save")) {
		LogWarning << "Failed to create save directory";
	}
	
	return true;
}

void ArxGame::setWindowSize(bool fullscreen) {
	
	if(fullscreen) {
		
		// Clamp to a sane resolution!
		if(config.video.resolution != Vec2i_ZERO) {
			config.video.resolution.x = std::max(config.video.resolution.x, s32(640));
			config.video.resolution.y = std::max(config.video.resolution.y, s32(480));
		}
		
		getWindow()->setFullscreenMode(config.video.resolution);
		
	} else {
		
		// Clamp to a sane window size!
		config.window.size.x = std::max(config.window.size.x, s32(640));
		config.window.size.y = std::max(config.window.size.y, s32(480));
		
		getWindow()->setWindowSize(config.window.size);
		
	}
}

bool ArxGame::initWindow(RenderWindow * window) {
	
	arx_assert(m_MainWindow == NULL);
	
	m_MainWindow = window;
	
	if(!m_MainWindow->initializeFramework()) {
		m_MainWindow = NULL;
		return false;
	}
	
	// Register ourself as a listener for this window messages
	m_MainWindow->addListener(this);
	m_MainWindow->getRenderer()->addListener(this);
	
	// Find the next best available fullscreen mode.
	if(config.video.resolution != Vec2i_ZERO) {
		const RenderWindow::DisplayModes & modes = window->getDisplayModes();
		DisplayMode mode = config.video.resolution;
		RenderWindow::DisplayModes::const_iterator i;
		i = std::lower_bound(modes.begin(), modes.end(), mode);
		if(i == modes.end()) {
			mode = *modes.rbegin();
		} else {
			mode = *i;
		}
		if(config.video.resolution != mode.resolution) {
			LogWarning << "Fullscreen mode " << config.video.resolution.x << 'x'
			           << config.video.resolution.y
			           << " not supported, using " << mode.resolution.x << 'x'
			           << mode.resolution.y << " instead";
			config.video.resolution = mode.resolution;
		}
	}
	
	m_MainWindow->setTitle(arx_name + " " + arx_version);
	m_MainWindow->setMinimizeOnFocusLost(config.window.minimizeOnFocusLost);
	m_MainWindow->setMinTextureUnits(3);
	m_MainWindow->setMaxMSAALevel(config.video.antialiasing ? 8 : 1);
	m_MainWindow->setVSync(config.video.vsync ? 1 : 0);
	
	setWindowSize(config.video.fullscreen);
	
	if(!m_MainWindow->initialize()) {
		m_MainWindow = NULL;
		return false;
	}
	
	if(GRenderer == NULL) {
		// We could not initialize all resources in onRendererInit().
		m_MainWindow = NULL;
		return false;
	}
	
	return true;
}

bool ArxGame::initWindow() {
	
	arx_assert(m_MainWindow == NULL);
	
	bool autoFramework = (config.window.framework == "auto");
	
	for(int i = 0; i < 2 && !m_MainWindow; i++) {
		bool first = (i == 0);
		
		bool matched = false;
		
		#if ARX_HAVE_SDL2
		if(!m_MainWindow && first == (autoFramework || config.window.framework == "SDL")) {
			matched = true;
			RenderWindow * window = new SDL2Window;
			if(!initWindow(window)) {
				delete window;
			}
		}
		#endif
		
		#if ARX_HAVE_SDL1
		if(!m_MainWindow && first == (autoFramework || config.window.framework == "SDL")) {
			matched = true;
			RenderWindow * window = new SDL1Window;
			if(!initWindow(window)) {
				delete window;
			}
		}
		#endif
		
		if(first && !matched) {
			LogError << "Unknown windowing framework: " << config.window.framework;
		}
	}
	
	if(!m_MainWindow) {
		LogCritical << "Graphics initialization failed";
		return false;
	}
	
	return true;
}

bool ArxGame::initInput() {
	
	LogDebug("Input init");
	bool init = ARX_INPUT_Init(m_MainWindow);
	if(!init) {
		LogCritical << "Input initialization failed";
	}
	
	return init;
}

bool ArxGame::initSound() {
	
	LogDebug("Sound init");
	bool init = ARX_SOUND_Init();
	if(!init) {
		LogWarning << "Sound initialization failed";
	}
	
	return true;
}

bool ArxGame::initGameData() {
	
	bool init = addPaks();
	if(!init) {
		LogCritical << "Failed to initialize the game data";
		return false;
	}
	
	savegames.update(true);
	
	return init;
}

TextureContainer * enviro = NULL;
TextureContainer * inventory_font = NULL;
TextureContainer * tflare = NULL;
TextureContainer * ombrignon = NULL;
TextureContainer * TC_fire = NULL;
TextureContainer * TC_fire2 = NULL;
TextureContainer * TC_smoke = NULL;
TextureContainer * Boom = NULL;
TextureContainer * arx_logo_tc = NULL;


static void LoadSysTextures() {
	
	MagicFlareLoadTextures();

	spellDataInit();

	enviro=				TextureContainer::LoadUI("graph/particles/enviro");
	inventory_font=		TextureContainer::LoadUI("graph/interface/font/font10x10_inventory");
	tflare=				TextureContainer::LoadUI("graph/particles/flare");
	ombrignon=			TextureContainer::LoadUI("graph/particles/ombrignon");
	TC_fire=			TextureContainer::LoadUI("graph/particles/fire");
	TC_fire2=			TextureContainer::LoadUI("graph/particles/fire2");
	TC_smoke=			TextureContainer::LoadUI("graph/particles/smoke");
	Boom=				TextureContainer::LoadUI("graph/particles/boom");
	arx_logo_tc=		TextureContainer::LoadUI("graph/interface/icons/arx_logo_32");
	
	TextureContainer::LoadUI("graph/particles/fire_hit");
	TextureContainer::LoadUI("graph/particles/light");
	
	//INTERFACE LOADING
	g_hudRoot.init();
	
	// Load book textures and text
	g_bookResouces.init();
	
	// MENU2
	TextureContainer::LoadUI("graph/interface/menus/menu_main_background", TextureContainer::NoColorKey);
	TextureContainer::LoadUI("graph/interface/menus/menu_console_background");
	TextureContainer::LoadUI("graph/interface/menus/menu_console_background_border");
}

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

LevelNumber LEVEL_TO_LOAD = LEVEL10;

static void loadLevel(u32 lvl) {
	if(lvl < NOLEVEL) {
		if(GameFlow::getTransition() != GameFlow::LoadingScreen) {
			LEVEL_TO_LOAD = static_cast<LevelNumber>(lvl);
			GameFlow::setTransition(GameFlow::LoadingScreen);
		}
	}
}
ARX_PROGRAM_OPTION("loadlevel", "", "Load a specific level", &loadLevel, "LEVELID");

extern SavegameHandle LOADQUEST_SLOT;
static void loadSlot(u32 saveSlot) {
	LOADQUEST_SLOT = SavegameHandle(saveSlot);
	GameFlow::setTransition(GameFlow::InGame);
}
ARX_PROGRAM_OPTION("loadslot", "", "Load a specific savegame slot", &loadSlot, "SAVESLOT");

static void skipLogo() {
	loadLevel(LEVEL10);
}
ARX_PROGRAM_OPTION("skiplogo", "", "Skip logos at startup", &skipLogo);

static bool HandleGameFlowTransitions() {
	
	const int TRANSITION_DURATION = 3600;
	static ArxInstant TRANSITION_START = ArxInstant_ZERO;

	if(GameFlow::getTransition() == GameFlow::NoTransition) {
		return false;
	}

	if(GInput->isAnyKeyPressed()) {
		ARXmenu.currentmode = AMCM_MAIN;
		ARX_MENU_Launch(false);
		GameFlow::setTransition(GameFlow::InGame);
	}
		
	if(GameFlow::getTransition() == GameFlow::FirstLogo) {
		benchmark::begin(benchmark::Splash);
		//firsttime
		if(TRANSITION_START == 0) {
			if(!ARX_INTERFACE_InitFISHTANK()) {
				GameFlow::setTransition(GameFlow::SecondLogo);
				return true;
			}
			
			arxtime.update();
			TRANSITION_START = arxtime.now();
		}

		ARX_INTERFACE_ShowFISHTANK();
		
		arxtime.update();
		float elapsed = arxtime.now_f() - TRANSITION_START;

		if(elapsed > TRANSITION_DURATION) {
			TRANSITION_START = ArxInstant_ZERO;
			GameFlow::setTransition(GameFlow::SecondLogo);
		}

		return true;			
	}

	if(GameFlow::getTransition() == GameFlow::SecondLogo) {
		benchmark::begin(benchmark::Splash);
		//firsttime
		if(TRANSITION_START == 0) {
			if(!ARX_INTERFACE_InitARKANE()) {
				GameFlow::setTransition(GameFlow::LoadingScreen);
				return true;
			}
			
			arxtime.update();
			TRANSITION_START = arxtime.now();
			ARX_SOUND_PlayInterface(SND_PLAYER_HEART_BEAT);
		}

		ARX_INTERFACE_ShowARKANE();
		
		arxtime.update();
		float elapsed = arxtime.now_f() - TRANSITION_START;

		if(elapsed > TRANSITION_DURATION) {
			TRANSITION_START = ArxInstant_ZERO;
			GameFlow::setTransition(GameFlow::LoadingScreen);
		}

		return true;
	}

	if(GameFlow::getTransition() == GameFlow::LoadingScreen) {
		ARX_INTERFACE_KillFISHTANK();
		ARX_INTERFACE_KillARKANE();
		char loadfrom[256];
		
		benchmark::begin(benchmark::LoadLevel);
		
		ARX_CHANGELEVEL_StartNew();
		
		sprintf(loadfrom, "graph/levels/level%d/level%d.dlf", LEVEL_TO_LOAD, LEVEL_TO_LOAD);
		progressBarReset();
		progressBarSetTotal(108);
		LoadLevelScreen(LEVEL_TO_LOAD);

		DanaeLoadLevel(loadfrom);
		GameFlow::setTransition(GameFlow::InGame);
		return true;
	}

	if(GameFlow::getTransition() == GameFlow::InGame) {
		GameFlow::setTransition(GameFlow::NoTransition);
		g_requestLevelInit = true;
		return true;
	}

	return false;
}


Vec3f PUSH_PLAYER_FORCE;
static EERIE_BACKGROUND DefaultBkg;

EERIE_CAMERA subj;
EERIE_CAMERA bookcam;
EERIE_CAMERA conversationcamera;

bool ArxGame::initGame()
{
	// Check if the game will be able to use the current game directory.
	if(!ARX_Changelevel_CurGame_Clear()) {
		LogCritical << "Error accessing current game directory";
		return false;
	}
	
	ScriptEvent::init();
	
	CalcFPS(true);
	
	g_miniMap.mapMarkerInit();
	
	ARX_SPELLS_CancelSpellTarget();
	
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
	TELEPORT_TO_LEVEL.clear();
	TELEPORT_TO_POSITION.clear();
	LogDebug("Mset");
	
	LogDebug("AnimManager Init");
	ARX_SCRIPT_EventStackInit();
	LogDebug("EventStack Init");
	ARX_EQUIPMENT_Init();
	LogDebug("AEQ Init");
	
	ARX_SCRIPT_Timer_FirstInit(512);
	LogDebug("Timer Init");
	ARX_FOGS_Clear();
	LogDebug("Fogs Init");
	
	EERIE_LIGHT_GlobalInit();
	LogDebug("Lights Init");
	
	LogDebug("Svars Init");
	
	// Script Test
	lastteleport = player.baseOffset();
	
	entities.init();
	
	player = ARXCHARACTER();
	ARX_PLAYER_InitPlayer();
	
	CleanInventory();
	
	ARX_SPEECH_FirstInit();
	ARX_SPEECH_Init();
	ARX_SPEECH_ClearAll();
	RemoveQuakeFX();
	
	LogDebug("Launching DANAE");
	
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
	GInput->setRawMouseInput(config.input.rawMouseInput);
	
	g_miniMap.firstInit(&player, resources, &entities);
	
	player.m_torchColor = Color3f(1.f, 0.8f, 0.66666f);
	LogDebug("InitializeDanae");
	
	InitTileLights();
	
	ARX_MISSILES_ClearAll();
	spells.init();

	ARX_SPELLS_ClearAllSymbolDraw();
	ARX_PARTICLES_ClearAll();
	ParticleSparkClear();
	ARX_MAGICAL_FLARES_FirstInit();
	
	LastLoadedScene.clear();
	
	DefaultBkg = EERIE_BACKGROUND();
	ACTIVEBKG=&DefaultBkg;
	InitBkg(ACTIVEBKG,MAX_BKGX,MAX_BKGZ,BKG_SIZX,BKG_SIZZ);
	
	player.size.y = -player.baseHeight();
	player.size.x = player.baseRadius();
	player.size.z = player.baseRadius();
	player.desiredangle = player.angle = subj.angle = Anglef(3.f, 268.f, 0.f);

	subj.orgTrans.pos = Vec3f(900.f, player.baseHeight(), 4340.f);
	subj.clip = Rect(0, 0, 640, 480);
	subj.center = subj.clip.center();
	subj.focal = defaultCameraFocal;
	subj.bkgcolor = Color::none;
	subj.cdepth = 2100.f;
	
	SetActiveCamera(&subj);

	bookcam = subj;
	conversationcamera = subj;
	
	bookcam.angle = Anglef::ZERO;
	bookcam.orgTrans.pos = Vec3f_ZERO;
	bookcam.focal = defaultCameraFocal;
	
	LoadSysTextures();
	cursorTexturesInit();
	
	LogInfo << "Launching splash screens";
	if(GameFlow::getTransition() == GameFlow::NoTransition) {
		GameFlow::setTransition(GameFlow::FirstLogo);
	}
	
	switch(resources->getReleaseType()) {
		
		case 0: LogWarning << "Neither demo nor full game data files loaded!"; break;
		
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
	
	LogDebug("Before Run...");
	
	const Vec2i & size = getWindow()->getSize();
	ControlCinematique = new Cinematic(size);
	
	long old = GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE;
	GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE = -1;
	
	gui::NecklaceInit();

	
	drawDebugInitialize();

	FlyingEye_Init();
	LoadSpellModels();
	
	cameraobj = loadObject("graph/obj3d/interactive/system/camera/camera.teo");
	markerobj = loadObject("graph/obj3d/interactive/system/marker/marker.teo");
	arrowobj = loadObject("graph/obj3d/interactive/items/weapons/arrow/arrow.teo");
	
	for(size_t i = 0; i < MAX_GOLD_COINS_VISUALS; i++) {
		
		std::ostringstream oss;
		
		if(i == 0) {
			oss << "graph/obj3d/interactive/items/jewelry/gold_coin/gold_coin.teo";
		} else {
			oss << "graph/obj3d/interactive/items/jewelry/gold_coin/gold_coin" << (i + 1) << ".teo";
		}
		
		GoldCoinsObj[i] = loadObject(oss.str());
		
		oss.str(std::string());
		
		if(i == 0) {
			oss << "graph/obj3d/interactive/items/jewelry/gold_coin/gold_coin[icon]";
		} else {
			oss << "graph/obj3d/interactive/items/jewelry/gold_coin/gold_coin" << (i + 1) << "[icon]";
		}
		
		GoldCoinsTC[i] = TextureContainer::LoadUI(oss.str());
	}
	
	ARX_PLAYER_LoadHeroAnimsAndMesh();
	
	GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE = old;
	
	m_gameInitialized = true;
	
	return true;
}

static const char * default_paks[][2] = {
	{ "data.pak", NULL },
	{ "loc.pak", "loc_default.pak" },
	{ "data2.pak", NULL },
	{ "sfx.pak", NULL },
	{ "speech.pak", "speech_default.pak" },
};

#if ARX_PLATFORM != ARX_PLATFORM_WIN32
static void runDataFilesInstaller() {
	static const char * const command[] = { "arx-install-data", "--gui", NULL };
	if(platform::runHelper(command, true) < 0) {
		std::ostringstream error;
		error << "Could not run `" << command[0] << "`.";
		platform::showErrorDialog(error.str(), "Critical Error - " + arx_name);
	}
}
#endif

bool ArxGame::addPaks() {
	
	arx_assert(!resources);
	
	resources = new PakReader;
	
	// Load required pak files
	bool missing = false;
	for(size_t i = 0; i < ARRAY_SIZE(default_paks); i++) {
		if(resources->addArchive(fs::paths.find(default_paks[i][0]))) {
			continue;
		}
		if(default_paks[i][1] && resources->addArchive(fs::paths.find(default_paks[i][1]))) {
			continue;
		}
		std::ostringstream oss;
		oss << "Missing required data file: \"" << default_paks[i][0] << "\"";
		if(default_paks[i][1]) {
			oss << " (or \"" << default_paks[i][1] << "\")";
		}
		LogError << oss.str();
		missing = true;
	}
	
	if(missing) {
		
		// Print the search path to the log
		std::ostringstream oss;
		oss << "Searched in these locations:\n";
		std::vector<fs::path> search = fs::paths.getSearchPaths();
		BOOST_FOREACH(const fs::path & dir, search) {
			oss << " * " << dir.string() << fs::path::dir_sep << "\n";
		}
		oss << "See " << url::help_install_data << " or `arx --list-dirs` for details.";
		LogInfo << oss.str();
		
		// Try to launch the data file installer on non-Windows systems
		#if ARX_PLATFORM != ARX_PLATFORM_WIN32
		const char * question = "Install the Arx Fatalis data files now?";
		logger::CriticalErrorDialog::setExitQuestion(question, runDataFilesInstaller);
		#endif
		
		// Construct an informative error message about missing files
		oss.str(std::string());
		oss << "Could not load required data files!\n";
		oss << "\nSee " << url::help_get_data << " for help.\n";
		LogCritical << oss.str();
		
		return false;
	}
	
	// Load optional patch files
	BOOST_REVERSE_FOREACH(const fs::path & base, fs::paths.data) {
		resources->addFiles(base / "editor", "editor");
		resources->addFiles(base / "game", "game");
		resources->addFiles(base / "graph", "graph");
		resources->addFiles(base / "localisation", "localisation");
		resources->addFiles(base / "misc", "misc");
		resources->addFiles(base / "sfx", "sfx");
		resources->addFiles(base / "speech", "speech");
	}
	
	return true;
}

static void ClearSysTextures() {
	for(size_t i = 0; i < SPELL_TYPES_COUNT; i++) {
		if(!spellicons[i].name.empty())
			//free(spellicons[i].name);
			spellicons[i].name.clear();

		if(!spellicons[i].description.empty())
			//free(spellicons[i].description);
			spellicons[i].description.clear();
	}
}

static void ReleaseSystemObjects() {
	
	delete hero, hero = NULL;
	
	if(entities.size() > 0 && entities.player() != NULL) {
		entities.player()->obj = NULL; // already deleted above (hero)
		delete entities.player();
		arx_assert(entities.size() > 0 && entities.player() == NULL);
	}
	
	FlyingEye_Release();
	ReleaseSpellModels();
	
	delete cameraobj, cameraobj = NULL;
	delete markerobj, markerobj = NULL;
	delete arrowobj, arrowobj = NULL;
	
	drawDebugRelease();

	BOOST_FOREACH(EERIE_3DOBJ * & obj, GoldCoinsObj) {
		delete obj, obj = NULL;
	}
}

long EXITING = 0;

void ArxGame::shutdown() {
	
	if(m_gameInitialized)
		shutdownGame();
	
	Application::shutdown();
	
	LogInfo << "Clean shutdown";
}

void ArxGame::shutdownGame() {
	
	ARX_Menu_Resources_Release();
	arxtime.resume();
	
	mainApp->getWindow()->hide();
	
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

	//sprites
	RenderBatcher::getInstance().reset();
	
	//Scripts
	svar.clear();
	
	
	ARX_SCRIPT_Timer_ClearAll();
	
	delete[] scr_timer, scr_timer = NULL;
	
	//Speech
	ARX_SPEECH_ClearAll();
	ARX_Text_Close();
	
	//object loaders from beforerun
	gui::ReleaseNecklace();
	
	delete resources;
	
	// Current game
	ARX_Changelevel_CurGame_Clear();
	
	//Halo
	ReleaseHalo();
	FreeSnapShot();
	ARX_INPUT_Release();
	
	if(getWindow()) {
		EERIE_PATHFINDER_Release();
		ARX_INPUT_Release();
		ARX_SOUND_Release();
	}
	
	ScriptEvent::shutdown();
	
}

void ArxGame::onWindowGotFocus(const Window &) {
	
	if(GInput) {
		GInput->reset();
	}
	
	if(config.audio.muteOnFocusLost) {
		ARXMenu_Options_Audio_SetMuted(false);
	}
	
}

void ArxGame::onWindowLostFocus(const Window &) {
	
	// TODO(option-control) add a config option for this
	ARX_INTERFACE_Combat_Mode(0);
	TRUE_PLAYER_MOUSELOOK_ON = false;
	PLAYER_MOUSELOOK_ON = false;
	
	// TODO(option-audio) add a config option to disable audio on focus loss
	
	if(config.audio.muteOnFocusLost) {
		ARXMenu_Options_Audio_SetMuted(true);
	}
	
}

void ArxGame::onResizeWindow(const Window & window) {
	
	arx_assert(window.getSize() != Vec2i_ZERO);
	
	// A new window size will require a new backbuffer
	// size, so the 3D structures must be changed accordingly.
	m_wasResized = true;
	
	if(window.isFullScreen()) {
		if(config.video.resolution == Vec2i_ZERO) {
			LogInfo << "Auto-selected fullscreen resolution " << window.getDisplayMode();
		} else {
			LogInfo << "Changed fullscreen resolution to " << window.getDisplayMode();
			config.video.resolution = window.getSize();
		}
	} else {
		LogInfo << "Changed window size to " << window.getDisplayMode();
		config.window.size = window.getSize();
	}
}

void ArxGame::onDestroyWindow(const Window &) {
	LogInfo << "Application window is being destroyed";
	quit();
}

void ArxGame::onToggleFullscreen(const Window & window) {
	config.video.fullscreen = window.isFullScreen();
}

/*!
 * \brief Message-processing loop. Idle time is used to render the scene.
 */
void ArxGame::run() {
	
	while(m_RunLoop) {
		
		ARX_PROFILE(Main Loop);
		
		platform::reapZombies();
		
		m_MainWindow->tick();
		if(!m_RunLoop) {
			break;
		}
		
		if(m_MainWindow->isVisible() && !m_MainWindow->isMinimized() && m_bReady) {
			doFrame();
			
			// Show the frame on the primary surface.
			m_MainWindow->showFrame();
		}
	}
	
	benchmark::begin(benchmark::Shutdown);
	
}

/*!
 * \brief Draws the scene.
 */
void ArxGame::doFrame() {
	
	ARX_PROFILE_FUNC();
	
	updateTime();

	updateInput();

	if(m_wasResized) {
		LogDebug("was resized");
		m_wasResized = false;
		DanaeRestoreFullScreen();
		g_hudRoot.recalcScale();
	}

	// Manages Splash Screens if needed
	if(HandleGameFlowTransitions())
		return;

	// Clicked on New Quest ? (TODO:need certainly to be moved somewhere else...)
	if(START_NEW_QUEST) {
		LogDebug("start quest");
		DANAE_StartNewQuest();
	}

	// Are we being teleported ?
	if(!TELEPORT_TO_LEVEL.empty() && CHANGE_LEVEL_ICON == 200) {
		benchmark::begin(benchmark::LoadLevel);
		LogDebug("teleport to " << TELEPORT_TO_LEVEL << " " << TELEPORT_TO_POSITION << " " << TELEPORT_TO_ANGLE);
		CHANGE_LEVEL_ICON = -1;
		ARX_CHANGELEVEL_Change(TELEPORT_TO_LEVEL, TELEPORT_TO_POSITION, TELEPORT_TO_ANGLE);
		TELEPORT_TO_LEVEL.clear();
		TELEPORT_TO_POSITION.clear();
	}

	if(LOADQUEST_SLOT != SavegameHandle()) {
		ARX_SlotLoad(LOADQUEST_SLOT);
		LOADQUEST_SLOT = SavegameHandle();
	}

	if(cinematicIsStopped()
	   && !cinematicBorder.isActive()
	   && !BLOCK_PLAYER_CONTROLS
	   && ARXmenu.currentmode == AMCM_OFF
	) {
		
		if(GInput->actionNowPressed(CONTROLS_CUST_QUICKLOAD)) {
			ARX_QuickLoad();
		}
		
		if(GInput->actionNowPressed(CONTROLS_CUST_QUICKSAVE)) {
			g_hudRoot.quickSaveIconGui.show();
			GRenderer->getSnapshot(savegame_thumbnail, config.interface.thumbnailSize.x, config.interface.thumbnailSize.y);
			ARX_QuickSave();
		}
		
	}
	
	if(g_requestLevelInit) {
		levelInit();
	} else {
		update();
		render();
	}
}

void ArxGame::updateFirstPersonCamera() {

	arx_assert(entities.player());
	
	Entity * io = entities.player();
	AnimLayer & layer1 = io->animlayer[1];
	ANIM_HANDLE ** alist = io->anims;

	if ( player.m_bowAimRatio
		&& (layer1.cur_anim!=alist[ANIM_MISSILE_STRIKE_PART_1])
		&& (layer1.cur_anim!=alist[ANIM_MISSILE_STRIKE_PART_2])
		&& (layer1.cur_anim!=alist[ANIM_MISSILE_STRIKE_CYCLE]))
	{
		player.m_bowAimRatio -= bowZoomFromDuration(Original_framedelay);

		if(player.m_bowAimRatio < 0)
			player.m_bowAimRatio = 0;
	}

	if(eyeball.exist == 2) {
		subj.d_pos = eyeball.pos;
		subj.d_angle = eyeball.angle;
		EXTERNALVIEW = true;
	} else if(EXTERNALVIEW) {
		for(long l=0; l < 250; l += 10) {
			Vec3f tt = player.pos;
			tt += angleToVectorXZ_180offset(player.angle.getPitch()) * float(l);
			tt += Vec3f(0.f, -50.f, 0.f);
			
			EERIEPOLY * ep = CheckInPoly(tt);
			if(ep) {
				subj.d_pos = tt;
			}
			else break;
		}

		subj.d_angle = player.angle;
		subj.d_angle.setYaw(subj.d_angle.getYaw() + 30.f);
	} else {
		subj.angle = player.angle;
		
		ActionPoint id = entities.player()->obj->fastaccess.view_attach;
		
		if(id != ActionPoint()) {
			subj.orgTrans.pos = actionPointPosition(entities.player()->obj, id);
			Vec3f vect;
			vect.x = subj.orgTrans.pos.x - player.pos.x;
			vect.y = 0;
			vect.z = subj.orgTrans.pos.z - player.pos.z;
			float len = ffsqrt(glm::length2(vect));
			
			if(len > 46.f) {
				float div = 46.f / len;
				vect.x *= div;
				vect.z *= div;
				subj.orgTrans.pos.x = player.pos.x + vect.x;
				subj.orgTrans.pos.z = player.pos.z + vect.z;
			}
		} else {
			subj.orgTrans.pos = player.basePosition();
		}
	}

	if(EXTERNALVIEW) {
		subj.orgTrans.pos = (subj.orgTrans.pos + subj.d_pos) * 0.5f;
		subj.angle = interpolate(subj.angle, subj.d_angle, 0.1f);
	}
}

void ArxGame::speechControlledCinematic() {

	/////////////////////////////////////////////
	// Now checks for speech controlled cinematic

	long valid=-1;

	for(size_t i = 0; i < MAX_ASPEECH; i++) {
		if(aspeech[i].exist && aspeech[i].cine.type > 0) {
			valid = i;
			break;
		}
	}

	if(valid >= 0) {
		const CinematicSpeech & acs = aspeech[valid].cine;
		const Entity * io = aspeech[valid].io;
		
		arxtime.update();
		float elapsed = arxtime.now_f() - aspeech[valid].time_creation;
		float rtime = elapsed / aspeech[valid].duration;

		rtime = glm::clamp(rtime, 0.f, 1.f);

		float itime=1.f-rtime;

		if(rtime >= 0.f && rtime <= 1.f && io) {
			switch(acs.type) {
			case ARX_CINE_SPEECH_KEEP: {
				subj.orgTrans.pos = acs.pos1;
				subj.angle.setYaw(acs.pos2.x);
				subj.angle.setPitch(acs.pos2.y);
				subj.angle.setRoll(acs.pos2.z);
				EXTERNALVIEW = true;
				break;
									   }
			case ARX_CINE_SPEECH_ZOOM: {
				//need to compute current values
				float alpha = acs.startangle.getYaw() * itime + acs.endangle.getYaw() * rtime;
				float beta = acs.startangle.getPitch() * itime + acs.endangle.getPitch() * rtime;
				float distance = acs.startpos * itime + acs.endpos * rtime;
				Vec3f targetpos = acs.pos1;
				
				conversationcamera.orgTrans.pos = angleToVectorXZ(io->angle.getPitch() + beta) * distance;
				conversationcamera.orgTrans.pos.y = std::sin(glm::radians(MAKEANGLE(io->angle.getYaw() + alpha))) * distance;
				conversationcamera.orgTrans.pos += targetpos;

				conversationcamera.setTargetCamera(targetpos);
				subj.orgTrans.pos = conversationcamera.orgTrans.pos;
				subj.angle.setYaw(MAKEANGLE(-conversationcamera.angle.getYaw()));
				subj.angle.setPitch(MAKEANGLE(conversationcamera.angle.getPitch()-180.f));
				subj.angle.setRoll(0.f);
				EXTERNALVIEW = true;
				break;
									   }
			case ARX_CINE_SPEECH_SIDE_LEFT:
			case ARX_CINE_SPEECH_SIDE: {
				if(ValidIONum(acs.ionum)) {
					const Vec3f & from = acs.pos1;
					const Vec3f & to = acs.pos2;

					Vec3f vect = glm::normalize(to - from);

					Vec3f vect2;
					if(acs.type==ARX_CINE_SPEECH_SIDE_LEFT) {
						vect2 = VRotateY(vect, -90.f);
					} else {
						vect2 = VRotateY(vect, 90.f);
					}

					float distance=acs.m_startdist*itime+acs.m_enddist*rtime;
					vect2 *= distance;
					float _dist = glm::distance(from, to);
					Vec3f tfrom = from + vect * acs.startpos * (1.0f / 100) * _dist;
					Vec3f tto = from + vect * acs.endpos * (1.0f / 100) * _dist;
					
					Vec3f targetpos = tfrom * itime + tto * rtime + Vec3f(0.f, acs.m_heightModifier, 0.f);

					conversationcamera.orgTrans.pos = targetpos + vect2 + Vec3f(0.f, acs.m_heightModifier, 0.f);
					conversationcamera.setTargetCamera(targetpos);
					subj.orgTrans.pos = conversationcamera.orgTrans.pos;
					subj.angle.setYaw(MAKEANGLE(-conversationcamera.angle.getYaw()));
					subj.angle.setPitch(MAKEANGLE(conversationcamera.angle.getPitch()-180.f));
					subj.angle.setRoll(0.f);
					EXTERNALVIEW = true;
				}

				break;
									   }
			case ARX_CINE_SPEECH_CCCLISTENER_R:
			case ARX_CINE_SPEECH_CCCLISTENER_L:
			case ARX_CINE_SPEECH_CCCTALKER_R:
			case ARX_CINE_SPEECH_CCCTALKER_L: {
				//need to compute current values
				if(ValidIONum(acs.ionum)) {
					Vec3f targetpos;
					if(acs.type == ARX_CINE_SPEECH_CCCLISTENER_L
						 || acs.type == ARX_CINE_SPEECH_CCCLISTENER_R) {
						conversationcamera.orgTrans.pos = acs.pos2;
						targetpos = acs.pos1;
					} else {
						conversationcamera.orgTrans.pos = acs.pos1;
						targetpos = acs.pos2;
					}

					float distance = (acs.startpos * itime + acs.endpos * rtime) * (1.0f/100);

					Vec3f vect = conversationcamera.orgTrans.pos - targetpos;
					Vec3f vect2 = VRotateY(vect, 90.f);;
					
					vect2 = glm::normalize(vect2);
					Vec3f vect3 = glm::normalize(vect);

					vect = vect * distance + vect3 * 80.f;
					vect2 *= 45.f;

					if ((acs.type==ARX_CINE_SPEECH_CCCLISTENER_R)
						|| (acs.type==ARX_CINE_SPEECH_CCCTALKER_R))
					{
						vect2 = -vect2;
					}

					conversationcamera.orgTrans.pos = vect + targetpos + vect2;
					conversationcamera.setTargetCamera(targetpos);
					subj.orgTrans.pos = conversationcamera.orgTrans.pos;
					subj.angle.setYaw(MAKEANGLE(-conversationcamera.angle.getYaw()));
					subj.angle.setPitch(MAKEANGLE(conversationcamera.angle.getPitch()-180.f));
					subj.angle.setRoll(0.f);
					EXTERNALVIEW = true;
				}

				break;
											  }
			case ARX_CINE_SPEECH_NONE: break;
			}

			LASTCAMPOS = subj.orgTrans.pos;
			LASTCAMANGLE = subj.angle;
		}
	}
}

void ArxGame::handlePlayerDeath() {
	if(player.lifePool.current <= 0) {
		DeadTime += static_cast<long>(g_framedelay);
		float mdist = glm::abs(player.physics.cyl.height)-60;

		float startDistance = 40.f;

		float startTime = 2000.f;
		float endTime = 7000.f;

		float DeadCameraDistance = startDistance + (mdist - startDistance) * ((DeadTime - startTime) / (endTime - startTime));

		Vec3f targetpos = player.pos;

		ActionPoint id  = entities.player()->obj->fastaccess.view_attach;
		ActionPoint id2 = GetActionPointIdx(entities.player()->obj, "chest2leggings");

		if(id != ActionPoint()) {
			targetpos = actionPointPosition(entities.player()->obj, id);
		}

		conversationcamera.orgTrans.pos = targetpos;

		if(id2 != ActionPoint()) {
			conversationcamera.orgTrans.pos = actionPointPosition(entities.player()->obj, id2);
		}

		conversationcamera.orgTrans.pos.y -= DeadCameraDistance;

		conversationcamera.setTargetCamera(targetpos);
		subj.orgTrans.pos=conversationcamera.orgTrans.pos;
		subj.angle.setYaw(MAKEANGLE(-conversationcamera.angle.getYaw()));
		subj.angle.setPitch(MAKEANGLE(conversationcamera.angle.getPitch()-180.f));
		subj.angle.setRoll(0);
		EXTERNALVIEW = true;
		BLOCK_PLAYER_CONTROLS = true;
	}
}

void ArxGame::updateActiveCamera() {

	ARX_PROFILE_FUNC();
	
	EERIE_CAMERA * cam = NULL;

	if(MasterCamera.exist) {
		if(MasterCamera.exist & 2) {
			MasterCamera.exist &= ~2;
			MasterCamera.exist |= 1;
			MasterCamera.io=MasterCamera.want_io;
		}

		cam = &MasterCamera.io->_camdata->cam;

		if(cam->focal < 100.f)
			cam->focal = 350.f;

		EXTERNALVIEW = true;
	} else {
		cam = &subj;
	}

	ManageQuakeFX(cam);

	SetActiveCamera(cam);
	PrepareCamera(cam, g_size);

	// Recenter Viewport depending on Resolution
	ACTIVECAM->center = Vec2i(g_size.center().x, g_size.center().y);
	ACTIVECAM->orgTrans.mod = Vec2f(ACTIVECAM->center);
}

void ArxGame::updateTime() {
	arxtime.update_frame_time();

	// before modulation by "GLOBAL_SLOWDOWN"
	Original_framedelay = arxtime.get_frame_delay();
	arx_assert(Original_framedelay >= 0.0f);

	// TODO this code shouldn't exist. ARXStartTime should be constant.
	if (GLOBAL_SLOWDOWN != 1.0f) {
		
		float drift = Original_framedelay * (1.0f - GLOBAL_SLOWDOWN) * 1000.0f;
		arxtime.increment_start_time(static_cast<u64>(drift));

		// recalculate frame delta
		arxtime.update_frame_time();
	}

	g_framedelay = arxtime.get_frame_delay();
	arx_assert(g_framedelay >= 0.0f);

	// limit fps above 10fps
	const float max_framedelay = 1000.0f / 10.0f;
	g_framedelay = g_framedelay > max_framedelay ? max_framedelay : g_framedelay;
}

void ArxGame::updateInput() {

	// Update input
	GInput->update();
	
	// Handle double clicks.
	const ActionKey & button = config.actions[CONTROLS_CUST_ACTION];
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
	
	
	// Overwrite the mouse button status when menu is active
	if(ARXmenu.currentmode != AMCM_OFF) {
		
		EERIEMouseButton = 0;
		
		if(GInput->getMouseButtonRepeat(Mouse::Button_0))
			EERIEMouseButton |= 1;
		else
			EERIEMouseButton &= ~1;
		
		if(GInput->getMouseButtonRepeat(Mouse::Button_1))
			EERIEMouseButton |= 2;
		else
			EERIEMouseButton &= ~2;
	}

	if(GInput->actionNowPressed(CONTROLS_CUST_TOGGLE_FULLSCREEN)) {
		setWindowSize(!getWindow()->isFullScreen());
	}

	if(GInput->isKeyPressedNowPressed(Keyboard::Key_F12)) {
		/*
		EERIE_PORTAL_ReleaseOnlyVertexBuffer();
		ComputePortalVertexBuffer();
		*/
		
		profiler::flush();
	}

	if(GInput->isKeyPressedNowPressed(Keyboard::Key_F11)) {

		g_debugInfo = static_cast<InfoPanels>(g_debugInfo + 1);

		if(g_debugInfo == InfoPanelEnumSize)
			g_debugInfo = InfoPanelNone;
	}

	if(g_debugInfo == InfoPanelDebugToggles) {
		
		for(size_t i = 0; i < ARRAY_SIZE(g_debugToggles); i++) {
			g_debugTriggers[i] = false;
			
			if(GInput->isKeyPressed(Keyboard::Key_NumPadEnter)) {
				if(   GInput->isKeyPressed(Keyboard::Key_NumPad0 + i)
				   && platform::getElapsedMs(g_debugTriggersTime[i]) > g_debugTriggersDecayDuration
				) {
					g_debugTriggersTime[i] = platform::getTimeMs();
					g_debugTriggers[i] = true;
				}
			} else {
				if(GInput->isKeyPressedNowPressed(Keyboard::Key_NumPad0 + i)) {
					g_debugToggles[i] = !g_debugToggles[i];
				}
			}
		}
	}

	if(GInput->isKeyPressedNowPressed(Keyboard::Key_F10)) {
		GetSnapShot();
	}

	if(GInput->isKeyPressedNowPressed(Keyboard::Key_ScrollLock)) {
		drawDebugCycleViews();
	}
	
#ifdef ARX_DEBUG
	if(GInput->isKeyPressedNowPressed(Keyboard::Key_Pause)) {
		if(!arxtime.is_paused()) {
			arxtime.pause();
		} else {
			arxtime.resume();
		}
	}
#endif
}

bool ArxGame::isInMenu() const {
	return ARXmenu.currentmode != AMCM_OFF;
}

void ArxGame::renderMenu() {
	
	GRenderer->SetRenderState(Renderer::Fog, false);
	
	ARX_Menu_Render();

	GRenderer->SetRenderState(Renderer::Fog, true);
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat); // << NEEDED?
}

extern int iHighLight;

void ArxGame::updateLevel() {

	arx_assert(entities.player());
	
	ARX_PROFILE_FUNC();
	
	RenderBatcher::getInstance().clear();

	if(!player.m_paralysed) {
		manageEditorControls();

		if(!BLOCK_PLAYER_CONTROLS) {
			managePlayerControls();
		}
	}

	{ ARX_PROFILE("Entity preprocessing");
	
	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity *entity = entities[handle];

		if(!entity)
			continue;

		if(entity->ignition > 0.f || (entity->ioflags & IO_FIERY))
			ManageIgnition(entity);

		//Highlight entity
		if(entity == FlyingOverIO && !(entity->ioflags & IO_NPC)) {
			entity->highlightColor = Color3f::gray(float(iHighLight));
		} else {
			entity->highlightColor = Color3f::black;
		}

		Cedric_ApplyLightingFirstPartRefactor(entity);

		float speedModifier = 0.f;

		if(entity == entities.player()) {
			if(cur_mr == 3)
				speedModifier += 0.5f;

			if(cur_rf == 3)
				speedModifier += 1.5f;
		}

		SpellBase * spell;
		
		spell = spells.getSpellOnTarget(entity->index(), SPELL_SPEED);
		if(spell) {
			speedModifier += spell->m_level * 0.1f;
		}
		
		spell = spells.getSpellOnTarget(entity->index(), SPELL_SLOW_DOWN);
		if(spell) {
			speedModifier -= spell->m_level * 0.05f;
		}
		
		entity->speed_modif = speedModifier;
	}
	}

	ARX_PLAYER_Manage_Movement();

	ARX_PLAYER_Manage_Visual();

	g_miniMap.setActiveBackground(ACTIVEBKG);
	g_miniMap.validatePlayerPos(CURRENTLEVEL, BLOCK_PLAYER_CONTROLS, g_guiBookCurrentTopTab);


	if(entities.player()->animlayer[0].cur_anim) {
		ManageNONCombatModeAnimations();
		
		{
			Entity * entity = entities.player();
			
			EERIEDrawAnimQuatUpdate(entity->obj,
			                        entity->animlayer,
			                        entity->angle,
			                        entity->pos,
			                        Original_framedelay,
			                        entity,
			                        true);
		}
		
		if((player.Interface & INTER_COMBATMODE) && entities.player()->animlayer[1].cur_anim)
			ManageCombatModeAnimations();

		if(entities.player()->animlayer[1].cur_anim)
			ManageCombatModeAnimationsEND();
	}

	updateFirstPersonCamera();
	
	ARX_SCRIPT_Timer_Check();

	speechControlledCinematic();

	handlePlayerDeath();
	
	UpdateCameras();

	ARX_PLAYER_FrameCheck(Original_framedelay);

	updateActiveCamera();

	ARX_GLOBALMODS_Apply();

	// Set Listener Position
	{
		std::pair<Vec3f, Vec3f> frontUp = angleToFrontUpVec(ACTIVECAM->angle);
		
		ARX_SOUND_SetListener(ACTIVECAM->orgTrans.pos, frontUp.first, frontUp.second);
	}

	// Check For Hiding/unHiding Player Gore
	if(EXTERNALVIEW || player.lifePool.current <= 0) {
		ARX_INTERACTIVE_Show_Hide_1st(entities.player(), 0);
	}

	if(!EXTERNALVIEW) {
		ARX_INTERACTIVE_Show_Hide_1st(entities.player(), 1);
	}

	PrepareIOTreatZone();
	ARX_PHYSICS_Apply();

	PrecalcIOLighting(ACTIVECAM->orgTrans.pos, ACTIVECAM->cdepth * 0.6f);

	ACTIVECAM->fadecolor = current.depthcolor;

	ARX_SCENE_Update();

	arx_assert(pParticleManager);
	pParticleManager->Update(static_cast<long>(g_framedelay));

	ARX_FOGS_Render();

	TreatBackgroundActions();

	// Checks Magic Flares Drawing
	if(!player.m_paralysed) {
		if(eeMousePressed1()) {
			if(!ARX_FLARES_Block) {
				ARX_SPELLS_AddPoint(DANAEMouse);
			} else {
				spellRecognitionPointsReset();
				ARX_FLARES_Block = false;
			}
		} else if(!ARX_FLARES_Block) {
			ARX_FLARES_Block = true;
		}
	}

	ARX_SPELLS_Precast_Check();
	ARX_SPELLS_ManageMagic();
	ARX_SPELLS_UpdateSymbolDraw();

	ManageTorch();

	{
		float magicSightZoom = 0.f;
		
		SpellBase * spell = spells.getSpellByCaster(PlayerEntityHandle, SPELL_MAGIC_SIGHT);
		if(spell) {
			ArxDuration duration = arxtime.now() - spell->m_timcreation;
			magicSightZoom = glm::clamp(float(duration) / 500.f, 0.f, 1.f);
		}
		
		float BASE_FOCAL = CURRENT_BASE_FOCAL
		                 + (magicSightZoom * -30.f)
		                 + (player.m_bowAimRatio * 177.5f);
		
		if(subj.focal < BASE_FOCAL) {
			static const float INC_FOCAL = 75.0f;
			subj.focal += INC_FOCAL;
		}

		if(subj.focal > BASE_FOCAL)
			subj.focal = BASE_FOCAL;
	}

	ARX_INTERACTIVE_DestroyIOdelayedExecute();
}

void ArxGame::renderLevel() {
	
	ARX_PROFILE_FUNC();
	
	// Clear screen & Z buffers
	if(desired.flags & GMOD_DCOLOR) {
		GRenderer->Clear(Renderer::ColorBuffer | Renderer::DepthBuffer, current.depthcolor.to<u8>());
	} else {
		subj.bkgcolor = ulBKGColor;
		GRenderer->Clear(Renderer::ColorBuffer | Renderer::DepthBuffer, subj.bkgcolor);
	}
	
	cinematicBorder.render();
	
	float fogEnd = fZFogEnd;
	float fogStart = fZFogStart;

	if(GRenderer->isFogInEyeCoordinates()) {
		fogEnd *= ACTIVECAM->cdepth;
		fogStart *= ACTIVECAM->cdepth;
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetFogParams(fogStart, fogEnd);
	GRenderer->SetFogColor(ulBKGColor);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::DepthTest, true);

	ARX_SCENE_Render();
	
	drawDebugRender();

	// Begin Particles
	arx_assert(pParticleManager);
	pParticleManager->Render();
	
	ARX_PARTICLES_Update(&subj);
	ParticleSparkUpdate();
	
	GRenderer->SetFogColor(ulBKGColor);
	GRenderer->SetRenderState(Renderer::DepthTest, true);
	GRenderer->SetBlendFunc(BlendOne, BlendOne);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	
	// End Particles

	// Renders Magical Flares
	if(!((player.Interface & INTER_MAP) && !(player.Interface & INTER_COMBATMODE))) {
		ARX_MAGICAL_FLARES_Update();
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);

	// Checks some specific spell FX
	CheckMr();

	if(player.m_improve) {
		DrawImproveVisionInterface();
	}

	if(eyeball.exist != 0)
		DrawMagicSightInterface();

	if(player.m_paralysed) {
		GRenderer->SetRenderState(Renderer::DepthWrite, false);
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		GRenderer->SetBlendFunc(BlendOne, BlendOne);

		EERIEDrawBitmap(Rectf(g_size), 0.0001f, NULL, Color(71, 71, 255));

		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		GRenderer->SetRenderState(Renderer::DepthWrite, true);
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);

	// Red screen fade for damages.
	ARX_DAMAGE_Show_Hit_Blood();

	// Manage Notes/Books opened on screen
	GRenderer->SetRenderState(Renderer::Fog, false);

	// Update spells
	ARX_SPELLS_Update();

	GRenderer->SetCulling(CullNone);
	GRenderer->SetRenderState(Renderer::Fog, true);
		
	GRenderer->SetFogColor(Color::none);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	RenderBatcher::getInstance().render();
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetFogColor(ulBKGColor);
	
	// Manage Death visual & Launch menu...
	ARX_PLAYER_Manage_Death();

	// INTERFACE
	RenderBatcher::getInstance().clear();

	// Remove the Alphablend State if needed : NO Z Clear
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetRenderState(Renderer::Fog, false);

	// Draw game interface if needed
	if(ARXmenu.currentmode == AMCM_OFF && !cinematicBorder.isActive()) {
	
		GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapClamp);
		GRenderer->SetRenderState(Renderer::DepthTest, false);
		
		ARX_INTERFACE_NoteManage();
		g_hudRoot.draw();
		
		if((player.Interface & INTER_MAP) && !(player.Interface & INTER_COMBATMODE)) {
			ARX_MAGICAL_FLARES_Update();
			GRenderer->SetRenderState(Renderer::DepthWrite, false);
			RenderBatcher::getInstance().render();
			GRenderer->SetRenderState(Renderer::DepthWrite, true);
		}
		
		GRenderer->SetRenderState(Renderer::DepthTest, true);
		GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
	
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		PopAllTriangleListOpaque();
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		PopAllTriangleListTransparency();
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	}	

	update2DFX();
	goFor2DFX();

	GRenderer->Clear(Renderer::DepthBuffer);

	// Speech Management
	ARX_SPEECH_Check();

	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);

	if(pTextManage && !pTextManage->Empty()) {
		pTextManage->Update(g_framedelay);
		pTextManage->Render();
	}

	if(SHOW_INGAME_MINIMAP
		&& cinematicIsStopped()
		&& !cinematicBorder.isActive()
		&& !BLOCK_PLAYER_CONTROLS
		&& !(player.Interface & INTER_MAP))
	{
		long SHOWLEVEL = ARX_LEVELS_GetRealNum(CURRENTLEVEL);

		if(SHOWLEVEL >= 0 && SHOWLEVEL < 32)
			g_miniMap.showPlayerMiniMap(SHOWLEVEL);
	}

	// CURSOR Rendering
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);

	if(DRAGINTER) {
		ARX_INTERFACE_RenderCursor();

		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		PopAllTriangleListOpaque();
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		PopAllTriangleListTransparency();
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);

		ARX_INTERFACE_HALO_Flush();
	} else {
		ARX_INTERFACE_HALO_Flush();
		ARX_INTERFACE_RenderCursor();
	}

	GRenderer->SetRenderState(Renderer::Fog, true);
	
	CheatDrawText();

	if(FADEDIR)
		ManageFade();
	
	GRenderer->SetScissor(Rect::ZERO);
	
	ARX_SPEECH_Update();
	
}

void ArxGame::update() {	

	LaunchWaitingCine();
}

void ArxGame::render() {
	
	ACTIVECAM = &subj;

	// Update Various Player Infos for this frame.
	ARX_PLAYER_Frame_Update();
		
	subj.center = Vec2i(g_size.center().x, g_size.center().y);
	subj.orgTrans.mod = Vec2f(subj.center);
	
	// SPECIFIC code for Snapshot MODE... to insure constant capture framerate

	PULSATE = std::sin(arxtime.get_frame_time() / 800);
	EERIEDrawnPolys = 0;

	// Checks for Keyboard & Moulinex
	{
	g_cursorOverBook = false;
	
	if(ARXmenu.currentmode == AMCM_OFF) { // Playing Game
		// Checks Clicks in Book Interface
		if(ARX_INTERFACE_MouseInBook()) {
			g_cursorOverBook = true;
		}
	}
	
	if((player.Interface & INTER_COMBATMODE) || PLAYER_MOUSELOOK_ON) {
		FlyingOverIO = NULL; // Avoid to check with those modes
	} else {
		if(!DRAGINTER) {
			if(!BLOCK_PLAYER_CONTROLS
			   && !TRUE_PLAYER_MOUSELOOK_ON
			   && !g_cursorOverBook
			   && eMouseState != MOUSE_IN_NOTE
			) {
				FlyingOverIO = FlyingOverObject(DANAEMouse);
			} else {
				FlyingOverIO = NULL;
			}
		}
	}
	
	if(!player.m_paralysed || ARXmenu.currentmode != AMCM_OFF) {
		if(!STOP_KEYBOARD_INPUT) {
			manageKeyMouse();
		} else {
			STOP_KEYBOARD_INPUT++;
			
			if(STOP_KEYBOARD_INPUT > 2)
				STOP_KEYBOARD_INPUT = 0;
		}
	}
	}

	if(CheckInPoly(player.pos)) {
		LastValidPlayerPos = player.pos;
	}

	// Updates Externalview
	EXTERNALVIEW = false;

	if(g_debugTriggers[1])
		g_hudRoot.bookIconGui.requestFX();
	
	if(isInMenu()) {
		benchmark::begin(benchmark::Menu);
		renderMenu();
	} else if(isInCinematic()) {
		benchmark::begin(benchmark::Cinematic);
		cinematicRender();
	} else {
		benchmark::begin(cinematicBorder.CINEMA_DECAL ? benchmark::Cutscene : benchmark::Scene);
		updateLevel();
		
		renderLevel();

#ifdef ARX_DEBUG
		if(g_debugToggles[9])
			renderLevel();
#endif
	}
	
	if(g_debugInfo != InfoPanelNone) {
		switch(g_debugInfo) {
		case InfoPanelFramerate: {
			CalcFPS();
			ShowFPS();
			break;
		}
		case InfoPanelFramerateGraph: {
			ShowFpsGraph();
			break;
		}
		case InfoPanelDebug: {
			ShowInfoText();
			break;
		}
		case InfoPanelDebugToggles: {
			ShowDebugToggles();
			break;
		}
		default: break;
		}
	}
	
	if(ARXmenu.currentmode == AMCM_OFF) {
		ARX_SCRIPT_AllowInterScriptExec();
		ARX_SCRIPT_EventStackExecute();
		// Updates Damages Spheres
		ARX_DAMAGES_UpdateAll();
		ARX_MISSILES_Update();

		ARX_PATH_UpdateAllZoneInOutInside();
	}

	arxtime.update_last_frame_time();
	LastMouseClick = EERIEMouseButton;
	
	gldebug::endFrame();
}

void ArxGame::update2DFX() {
	
	ARX_PROFILE_FUNC();
	
	TexturedVertex ltvv;

	Entity* pTableIO[256];
	size_t nNbInTableIO = 0;

	float temp_increase = g_framedelay * (1.0f/1000) * 4.f;

	bool bComputeIO = false;

	for(size_t i = 0; i < TOTPDL; i++) {
		EERIE_LIGHT *el = PDL[i];

		EERIE_BKG_INFO * bkgData = getFastBackgroundData(el->pos.x, el->pos.z);

		if(!bkgData || !bkgData->treat) {
			el->treat=0;
			continue;
		}

		if(el->extras & EXTRAS_FLARE) {
			Vec3f lv = el->pos;
			EE_RTP(lv, ltvv);
			el->m_flareFader -= temp_increase;

			if(!(player.Interface & INTER_COMBATMODE) && (player.Interface & INTER_MAP))
				continue;

			if(ltvv.rhw > 0.f &&
				ltvv.p.x > 0.f &&
				ltvv.p.y > (cinematicBorder.CINEMA_DECAL * g_sizeRatio.y) &&
				ltvv.p.x < g_size.width() &&
				ltvv.p.y < (g_size.height()-(cinematicBorder.CINEMA_DECAL * g_sizeRatio.y))
				)
			{
				Vec3f vector = lv - ACTIVECAM->orgTrans.pos;
				lv -= vector * (50.f / glm::length(vector));

				float fZFar=ACTIVECAM->ProjectionMatrix[2][2]*(1.f/(ACTIVECAM->cdepth*fZFogEnd))+ACTIVECAM->ProjectionMatrix[3][2];

				Vec3f hit;
				Vec2s ees2dlv;
				Vec3f ee3dlv = lv;

				ees2dlv.x = checked_range_cast<short>(ltvv.p.x);
				ees2dlv.y = checked_range_cast<short>(ltvv.p.y);

				if(!bComputeIO) {
					GetFirstInterAtPos(ees2dlv, 2, &ee3dlv, pTableIO, &nNbInTableIO);
					bComputeIO = true;
				}

				if(ltvv.p.z > fZFar ||
					EERIELaunchRay3(ACTIVECAM->orgTrans.pos, ee3dlv, hit, 1) ||
					GetFirstInterAtPos(ees2dlv, 3, &ee3dlv, pTableIO, &nNbInTableIO )
					)
				{
					el->m_flareFader -= temp_increase * 2.f;
				} else {
					el->m_flareFader += temp_increase * 2.f;
				}
			}

			el->m_flareFader = glm::clamp(el->m_flareFader, 0.f, .8f);
		}
	}
}

void ArxGame::goFor2DFX() {
	
	ARX_PROFILE_FUNC();
	
	GRenderer->SetRenderState(Renderer::Fog, true);
	GRenderer->SetBlendFunc(BlendOne, BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetCulling(CullNone);
	GRenderer->SetRenderState(Renderer::DepthTest, false);
	GRenderer->SetFogColor(Color::none);

	for(size_t i = 0; i < TOTPDL; i++) {
		const EERIE_LIGHT & el = *PDL[i];

		if(!el.exist || !el.treat)
			continue;

		if(el.extras & EXTRAS_FLARE) {
			if(el.m_flareFader > 0.f) {
				Vec3f ltvv = EE_RT(el.pos);
				
				float v = el.m_flareFader;

				if(FADEDIR) {
					v *= 1.f - LAST_FADEVALUE;
				}

				float siz;

				if(el.extras & EXTRAS_FIXFLARESIZE)
					siz = el.ex_flaresize;
				else
					siz = -el.ex_flaresize;

				EERIEDrawSprite(el.pos, siz, tflare, (el.rgb * v).to<u8>(), ltvv.z);

			}
		}
	}

	GRenderer->SetRenderState(Renderer::DepthTest, true);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::Fog, false);
}

void ArxGame::onRendererInit(Renderer & renderer) {
	
	arx_assert(GRenderer == NULL);
	
	GRenderer = &renderer;
	
	arx_assert(renderer.GetTextureStageCount() >= 3, "not enough texture units");
	arx_assert(m_MainWindow);
	
	renderer.Clear(Renderer::ColorBuffer);
	m_MainWindow->showFrame();
		
	// Enable Z-buffering RenderState
	renderer.SetRenderState(Renderer::DepthTest, true);
	
	// Restore All Textures RenderState
	renderer.RestoreAllTextures();

	ARX_PLAYER_Restore_Skin();
	
	// Setup Texture Border RenderState
	renderer.GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);

	renderer.GetTextureStage(1)->disableColor();
	
	// Fog
	float fogEnd = 0.48f;
	float fogStart = fogEnd * 0.65f;
	renderer.SetFogParams(fogStart, fogEnd);
	renderer.SetFogColor(current.depthcolor.to<u8>());
	renderer.SetRenderState(Renderer::Fog, true);
	
	renderer.SetDepthBias(0);

	ComputePortalVertexBuffer();
	VertexBuffer<SMY_VERTEX3> * vb3 = renderer.createVertexBuffer3(4000, Renderer::Stream);
	pDynamicVertexBuffer = new CircularVertexBuffer<SMY_VERTEX3>(vb3);
	
	size_t size = (config.video.bufferSize < 1) ? 32 * 1024 : config.video.bufferSize * 1024;
	VertexBuffer<TexturedVertex> * vb = renderer.createVertexBufferTL(size, Renderer::Stream);
	pDynamicVertexBuffer_TLVERTEX = new CircularVertexBuffer<TexturedVertex>(vb);
	
	MenuReInitAll();
	
	ARX_SetAntiAliasing();
	
	// The app is ready to go
	m_bReady = true;
}

void ArxGame::onRendererShutdown(Renderer & renderer) {
	
	if(GRenderer != &renderer) {
		// onRendererInit() failed
		return;
	}
	
	m_bReady = false;
	
	GRenderer->ReleaseAllTextures();

	delete pDynamicVertexBuffer_TLVERTEX, pDynamicVertexBuffer_TLVERTEX = NULL;
	delete pDynamicVertexBuffer, pDynamicVertexBuffer = NULL;
	
	EERIE_PORTAL_ReleaseOnlyVertexBuffer();
	
	GRenderer = NULL;
}
