/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
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
#include "game/effect/ParticleSystems.h"
#include "game/effect/Quake.h"

#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Color.h"
#include "graphics/Draw.h"
#include "graphics/DrawDebug.h"
#include "graphics/GlobalFog.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/Vertex.h"
#include "graphics/VertexBuffer.h"
#include "graphics/data/FTL.h"
#include "graphics/data/Mesh.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/Fade.h"
#include "graphics/effects/Fog.h"
#include "graphics/effects/LightFlare.h"
#include "graphics/font/Font.h"
#include "graphics/opengl/GLDebug.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/ParticleManager.h"
#include "graphics/particle/MagicFlare.h"
#include "graphics/particle/Spark.h"
#include "graphics/texture/TextureStage.h"

#include "gui/Console.h"
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
#include "platform/Thread.h"

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

static const float CURRENT_BASE_FOCAL = 310.f;
static const float defaultCameraFocal = 350.f;

static const PlatformDuration runeDrawPointInterval = PlatformDurationMs(16); // ~60fps

extern EERIE_3DOBJ * arrowobj;

extern Entity * FlyingOverIO;
extern EERIE_CAMERA conversationcamera;
extern ParticleManager * pParticleManager;
extern CircularVertexBuffer<TexturedVertex> * pDynamicVertexBuffer_TLVERTEX; // VB using TLVERTEX format.
extern CircularVertexBuffer<SMY_VERTEX3> * pDynamicVertexBuffer;

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
	m_MainWindow->setVSync(benchmark::isEnabled() ? 0 : config.video.vsync);
	
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
	
	#if ARX_HAVE_SDL2
	if(!m_MainWindow) {
		RenderWindow * window = new SDL2Window;
		if(!initWindow(window)) {
			delete window;
		}
	}
	#endif
	
	#if ARX_HAVE_SDL1
	if(!m_MainWindow) {
		RenderWindow * window = new SDL1Window;
		if(!initWindow(window)) {
			delete window;
		}
	}
	#endif
	
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
	
}

class GameFlow {

public:
	enum Transition {
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

GameFlow::Transition GameFlow::s_currentTransition = GameFlow::FirstLogo;

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
ARX_PROGRAM_OPTION_ARG("loadlevel", "", "Load a specific level", &loadLevel, "LEVELID")

extern SavegameHandle LOADQUEST_SLOT;
static void loadSlot(u32 saveSlot) {
	LOADQUEST_SLOT = SavegameHandle(saveSlot);
	GameFlow::setTransition(GameFlow::InGame);
}
ARX_PROGRAM_OPTION_ARG("loadslot", "", "Load a specific savegame slot", &loadSlot, "SAVESLOT")

static void skipLogo() {
	loadLevel(LEVEL10);
}
ARX_PROGRAM_OPTION("skiplogo", "", "Skip logos at startup", &skipLogo)

static bool HandleGameFlowTransitions() {
	
	const PlatformDuration TRANSITION_DURATION = PlatformDurationMs(3600);
	static PlatformInstant TRANSITION_START = 0;

	if(GameFlow::getTransition() == GameFlow::InGame) {
		return false;
	}

	if(GInput->isAnyKeyPressed()) {
		ARXmenu.requestMode(Mode_MainMenu);
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
			
			TRANSITION_START = g_platformTime.frameStart();
		}

		ARX_INTERFACE_ShowFISHTANK();
		
		PlatformDuration elapsed = g_platformTime.frameStart() - TRANSITION_START;

		if(elapsed > TRANSITION_DURATION) {
			TRANSITION_START = 0;
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
			
			TRANSITION_START = g_platformTime.frameStart();
			ARX_SOUND_PlayInterface(SND_PLAYER_HEART_BEAT);
		}

		ARX_INTERFACE_ShowARKANE();
		
		PlatformDuration elapsed = g_platformTime.frameStart() - TRANSITION_START;

		if(elapsed > TRANSITION_DURATION) {
			TRANSITION_START = 0;
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
		return false;
	}

	return false;
}


Vec3f PUSH_PLAYER_FORCE;
static BackgroundData DefaultBkg;

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
	
	ARXMenu_Options_Video_SetFogDistance(config.video.fogDistance);
	ARXMenu_Options_Video_SetDetailsQuality(config.video.levelOfDetail);
	ARXMenu_Options_Video_SetGamma(config.video.gamma);
	ARXMenu_Options_Audio_SetMasterVolume(config.audio.volume);
	ARXMenu_Options_Audio_SetSfxVolume(config.audio.sfxVolume);
	ARXMenu_Options_Audio_SetSpeechVolume(config.audio.speechVolume);
	ARXMenu_Options_Audio_SetAmbianceVolume(config.audio.ambianceVolume);
	ARXMenu_Options_Audio_ApplyGameVolumes();
	
	GInput->setMouseSensitivity(config.input.mouseSensitivity);
	GInput->setMouseAcceleration(config.input.mouseAcceleration);
	GInput->setInvertMouseY(config.input.invertMouse);
	GInput->setRawMouseInput(config.input.rawMouseInput);
	
	g_miniMap.firstInit(&player, g_resources, &entities);
	
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
	
	DefaultBkg = BackgroundData();
	ACTIVEBKG = &DefaultBkg;
	InitBkg(ACTIVEBKG, MAX_BKGX, MAX_BKGZ, Vec2s(BKG_SIZX, BKG_SIZZ));
	
	player.size.y = -player.baseHeight();
	player.size.x = player.baseRadius();
	player.size.z = player.baseRadius();
	player.desiredangle = player.angle = subj.angle = Anglef(3.f, 268.f, 0.f);

	subj.orgTrans.pos = Vec3f(900.f, player.baseHeight(), 4340.f);
	subj.clip = Rect(0, 0, 640, 480);
	subj.center = subj.clip.center();
	subj.focal = defaultCameraFocal;
	subj.cdepth = 2100.f;
	
	SetActiveCamera(&subj);

	bookcam = subj;
	conversationcamera = subj;
	
	bookcam.angle = Anglef::ZERO;
	bookcam.orgTrans.pos = Vec3f_ZERO;
	bookcam.focal = defaultCameraFocal;
	bookcam.cdepth = 2200.f;
	
	LoadSysTextures();
	cursorTexturesInit();
	
	PakReader::ReleaseFlags release = g_resources->getReleaseType();
	if((release & PakReader::Demo) && (release & PakReader::FullGame)) {
		LogWarning << "Mixed demo and full game data files!";
		CrashHandler::setVariable("Data files", "mixed");
	} else if(release & PakReader::Demo) {
		LogInfo << "Initialized Arx Fatalis (demo)";
		CrashHandler::setVariable("Data files", "demo");
	} else if(release & PakReader::FullGame) {
		LogInfo << "Initialized Arx Fatalis (full game)";
		CrashHandler::setVariable("Data files", "full");
	} else {
		LogWarning << "Neither demo nor full game data files loaded!";
		CrashHandler::setVariable("Data files", "unknown");
	}
	
	LogDebug("Before Run...");
	
	cinematicInit();
	
	long old = GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE;
	GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE = -1;
	
	gui::NecklaceInit();

	
	drawDebugInitialize();

	FlyingEye_Init();
	LoadSpellModels();
	particleParametersInit();
	
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
	
	arx_assert(!g_resources);
	
	g_resources = new PakReader;
	
	// Load required pak files
	bool missing = false;
	for(size_t i = 0; i < ARRAY_SIZE(default_paks); i++) {
		if(g_resources->addArchive(fs::paths.find(default_paks[i][0]))) {
			continue;
		}
		if(default_paks[i][1] && g_resources->addArchive(fs::paths.find(default_paks[i][1]))) {
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
		g_resources->addFiles(base / "editor", "editor");
		g_resources->addFiles(base / "game", "game");
		g_resources->addFiles(base / "graph", "graph");
		g_resources->addFiles(base / "localisation", "localisation");
		g_resources->addFiles(base / "misc", "misc");
		g_resources->addFiles(base / "sfx", "sfx");
		g_resources->addFiles(base / "speech", "speech");
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
	
	mainApp->getWindow()->hide();
	
	Menu2_Close();
	DanaeClearLevel();
	TextureContainer::DeleteAll();
	
	cinematicDestroy();
	
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
	g_renderBatcher.reset();
	
	//Scripts
	svar.clear();
	
	
	ARX_SCRIPT_Timer_ClearAll();
	
	delete[] scr_timer, scr_timer = NULL;
	
	//Speech
	ARX_SPEECH_ClearAll();
	ARX_Text_Close();
	
	//object loaders from beforerun
	gui::ReleaseNecklace();
	
	delete g_resources;
	
	// Current game
	ARX_Changelevel_CurGame_Clear();
	
	//Halo
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
	ARX_INTERFACE_setCombatMode(COMBAT_MODE_OFF);
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
		}
	}
	
	benchmark::begin(benchmark::Shutdown);
	
}

/*!
 * \brief Draws the scene.
 */
void ArxGame::doFrame() {
	
	if(config.video.fpsLimit && !benchmark::isEnabled()) {
		
		PlatformDuration renderDuration = platform::getTime() - g_platformTime.frameStart();
		
		int targetFps = config.video.fpsLimit;
		if(config.video.vsync) {
			targetFps = std::max(targetFps, 480);
		}
		PlatformDuration targetDuration = PlatformDurationUs(1000000 / (targetFps + targetFps / 20 + 1) - 100);
		if(renderDuration < targetDuration) {
			Thread::sleep(targetDuration - renderDuration);
		}
		
	}
	
	ARX_PROFILE_FUNC();
	
	updateTime();

	updateInput();

	if(m_wasResized) {
		LogDebug("was resized");
		m_wasResized = false;
		MenuReInitAll();
		AdjustUI();
		LoadScreen();
		g_hudRoot.recalcScale();
	}

	// Manages Splash Screens if needed
	if(HandleGameFlowTransitions()) {
		m_MainWindow->showFrame();
		return;
	}

	// Clicked on New Quest ? (TODO:need certainly to be moved somewhere else...)
	if(START_NEW_QUEST) {
		LogDebug("start quest");
		DANAE_StartNewQuest();
	}

	// Are we being teleported ?
	if(!TELEPORT_TO_LEVEL.empty() && CHANGE_LEVEL_ICON != NoChangeLevel
	   && (CHANGE_LEVEL_ICON == ChangeLevelNow
	       || config.input.quickLevelTransition == ChangeLevelImmediately
	       || (config.input.quickLevelTransition == JumpToChangeLevel
	           && GInput->actionPressed(CONTROLS_CUST_JUMP)))) {
		// TODO allow binding the same key to multiple actions so that we can have a separate binding for this
		benchmark::begin(benchmark::LoadLevel);
		LogDebug("teleport to " << TELEPORT_TO_LEVEL << " " << TELEPORT_TO_POSITION << " " << TELEPORT_TO_ANGLE);
		CHANGE_LEVEL_ICON = NoChangeLevel;
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
	) {
		
		if(GInput->actionNowPressed(CONTROLS_CUST_QUICKLOAD) && savegames.size() > 0) {
			ARXmenu.requestMode(Mode_InGame);
			ARX_QuickLoad();
		}
		
		if(GInput->actionNowPressed(CONTROLS_CUST_QUICKSAVE) && ARXmenu.mode() == Mode_InGame) {
			g_hudRoot.quickSaveIconGui.show();
			GRenderer->getSnapshot(savegame_thumbnail, config.interface.thumbnailSize.x, config.interface.thumbnailSize.y);
			ARX_QuickSave();
			g_platformTime.updateFrame();
		}
		
	}
	
	if(g_requestLevelInit) {
		g_requestLevelInit = false;
		levelInit();
	} else {
		cinematicLaunchWaiting();
		render();
		m_MainWindow->showFrame();
	}
}

void ArxGame::updateFirstPersonCamera() {

	arx_assert(entities.player());
	
	Entity * io = entities.player();
	AnimLayer & layer1 = io->animlayer[1];
	ANIM_HANDLE ** alist = io->anims;

	if(player.m_bowAimRatio != 0.f
		&& (layer1.cur_anim!=alist[ANIM_MISSILE_STRIKE_PART_1])
		&& (layer1.cur_anim!=alist[ANIM_MISSILE_STRIKE_PART_2])
		&& (layer1.cur_anim!=alist[ANIM_MISSILE_STRIKE_CYCLE]))
	{
		player.m_bowAimRatio -= bowZoomFromDuration(toMs(g_platformTime.lastFrameDuration()));

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
			tt += angleToVectorXZ_180offset(player.angle.getYaw()) * float(l);
			tt += Vec3f(0.f, -50.f, 0.f);
			
			EERIEPOLY * ep = CheckInPoly(tt);
			if(ep) {
				subj.d_pos = tt;
			}
			else break;
		}

		subj.d_angle = player.angle;
		subj.d_angle.setPitch(subj.d_angle.getPitch() + 30.f);
	} else {
		subj.angle = player.angle;
		
		ActionPoint id = entities.player()->obj->fastaccess.view_attach;
		
		if(id != ActionPoint()) {
			subj.orgTrans.pos = actionPointPosition(entities.player()->obj, id);
			Vec3f vect;
			vect.x = subj.orgTrans.pos.x - player.pos.x;
			vect.y = 0;
			vect.z = subj.orgTrans.pos.z - player.pos.z;
			float len = ffsqrt(arx::length2(vect));
			
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
		
		const GameDuration elapsed = g_gameTime.now() - aspeech[valid].time_creation;
		float rtime = elapsed / aspeech[valid].duration;

		rtime = glm::clamp(rtime, 0.f, 1.f);

		float itime=1.f-rtime;

		if(rtime >= 0.f && rtime <= 1.f && io) {
			switch(acs.type) {
			case ARX_CINE_SPEECH_KEEP: {
				arx_assert(isallfinite(acs.pos1));
				
				subj.orgTrans.pos = acs.pos1;
				subj.angle.setPitch(acs.pos2.x);
				subj.angle.setYaw(acs.pos2.y);
				subj.angle.setRoll(acs.pos2.z);
				EXTERNALVIEW = true;
				break;
									   }
			case ARX_CINE_SPEECH_ZOOM: {
				arx_assert(isallfinite(acs.pos1));
				
				//need to compute current values
				float alpha = acs.startangle.getPitch() * itime + acs.endangle.getPitch() * rtime;
				float beta = acs.startangle.getYaw() * itime + acs.endangle.getYaw() * rtime;
				float distance = acs.startpos * itime + acs.endpos * rtime;
				Vec3f targetpos = acs.pos1;
				
				conversationcamera.orgTrans.pos = angleToVectorXZ(io->angle.getYaw() + beta) * distance;
				conversationcamera.orgTrans.pos.y = std::sin(glm::radians(MAKEANGLE(io->angle.getPitch() + alpha))) * distance;
				conversationcamera.orgTrans.pos += targetpos;

				conversationcamera.setTargetCamera(targetpos);
				subj.orgTrans.pos = conversationcamera.orgTrans.pos;
				subj.angle.setPitch(MAKEANGLE(-conversationcamera.angle.getPitch()));
				subj.angle.setYaw(MAKEANGLE(conversationcamera.angle.getYaw()-180.f));
				subj.angle.setRoll(0.f);
				EXTERNALVIEW = true;
				break;
									   }
			case ARX_CINE_SPEECH_SIDE_LEFT:
			case ARX_CINE_SPEECH_SIDE: {
				if(ValidIONum(acs.ionum)) {
					arx_assert(isallfinite(acs.pos1));
					arx_assert(isallfinite(acs.pos2));
					
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
					subj.angle.setPitch(MAKEANGLE(-conversationcamera.angle.getPitch()));
					subj.angle.setYaw(MAKEANGLE(conversationcamera.angle.getYaw()-180.f));
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
					arx_assert(isallfinite(acs.pos1));
					arx_assert(isallfinite(acs.pos2));
					
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
					subj.angle.setPitch(MAKEANGLE(-conversationcamera.angle.getPitch()));
					subj.angle.setYaw(MAKEANGLE(conversationcamera.angle.getYaw()-180.f));
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

extern GameDuration DeadTime;

void ArxGame::handlePlayerDeath() {
	if(player.lifePool.current <= 0) {
		DeadTime += g_gameTime.lastFrameDuration();
		float mdist = glm::abs(player.physics.cyl.height)-60;

		float startDistance = 40.f;

		GameDuration startTime = GameDurationMs(2000);
		GameDuration endTime = GameDurationMs(7000);

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
		subj.angle.setPitch(MAKEANGLE(-conversationcamera.angle.getPitch()));
		subj.angle.setYaw(MAKEANGLE(conversationcamera.angle.getYaw()-180.f));
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
	
	g_platformTime.updateFrame();
	
	if(g_requestLevelInit) {
		g_platformTime.overrideFrameDuration(0);
	}
	
	g_gameTime.update(g_platformTime.lastFrameDuration());
	
	g_framedelay = toMsf(g_gameTime.lastFrameDuration());
	
}

void ArxGame::updateInput() {

	// Update input
	GInput->update(toMs(g_platformTime.lastFrameDuration()));
	
	// Handle double clicks.
	const ActionKey & button = config.actions[CONTROLS_CUST_ACTION];
	if((button.key[0] != -1 && (button.key[0] & Mouse::ButtonBase)
	    && GInput->getMouseButtonDoubleClick(button.key[0]))
	   || (button.key[1] != -1 && (button.key[1] & Mouse::ButtonBase)
	    && GInput->getMouseButtonDoubleClick(button.key[1]))) {
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
	if(ARXmenu.mode() != Mode_InGame) {
		
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
				   && g_platformTime.frameStart() - g_debugTriggersTime[i] > g_debugTriggersDecayDuration
				) {
					g_debugTriggersTime[i] = g_platformTime.frameStart();
					g_debugTriggers[i] = true;
				}
			} else {
				if(GInput->isKeyPressedNowPressed(Keyboard::Key_NumPad0 + i)) {
					g_debugToggles[i] = !g_debugToggles[i];
				}
				if(GInput->isKeyPressed(Keyboard::Key_LeftShift)
				   && GInput->isKeyPressed(Keyboard::Key_LeftAlt)
				   && GInput->isKeyPressedNowPressed(Keyboard::Key_0 + i)) {
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
	
	g_console.update();
	
#ifdef ARX_DEBUG
	if(GInput->isKeyPressedNowPressed(Keyboard::Key_Pause)) {
		if(g_gameTime.isPaused() & GameTime::PauseUser) {
			g_gameTime.resume(GameTime::PauseUser);
		} else {
			g_gameTime.pause(GameTime::PauseUser);
		}
	}
#endif
}

extern int iHighLight;

void ArxGame::updateLevel() {

	arx_assert(entities.player());
	
	ARX_PROFILE_FUNC();
	
	g_renderBatcher.clear();
	
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
	g_miniMap.validatePlayerPos(CURRENTLEVEL, BLOCK_PLAYER_CONTROLS, g_playerBook.currentPage());


	if(entities.player()->animlayer[0].cur_anim) {
		ManageNONCombatModeAnimations();
		
		{
			AnimationDuration framedelay = toAnimationDuration(g_platformTime.lastFrameDuration());
			Entity * entity = entities.player();
			
			EERIEDrawAnimQuatUpdate(entity->obj,
			                        entity->animlayer,
			                        entity->angle,
			                        entity->pos,
			                        framedelay,
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

	ARX_PLAYER_FrameCheck(g_platformTime.lastFrameDuration());

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

	ARX_SCENE_Update();

	arx_assert(pParticleManager);
	pParticleManager->Update(g_gameTime.lastFrameDuration());

	ARX_FOGS_Render();

	TreatBackgroundActions();

	// Checks Magic Flares Drawing
	if(!player.m_paralysed) {
		if(eeMousePressed1()) {
			if(!ARX_FLARES_Block) {
				static PlatformDuration runeDrawPointElapsed = 0;
				if(!config.input.useAltRuneRecognition) {
					runeDrawPointElapsed += g_platformTime.lastFrameDuration();
					
					if(runeDrawPointElapsed >= runeDrawPointInterval) {
						ARX_SPELLS_AddPoint(DANAEMouse);
						while(runeDrawPointElapsed >= runeDrawPointInterval) {
							runeDrawPointElapsed -= runeDrawPointInterval;
						}
					}
				} else {
					ARX_SPELLS_AddPoint(DANAEMouse);
				}
			} else {
				spellRecognitionPointsReset();
				ARX_FLARES_Block = false;
			}
		} else if(!ARX_FLARES_Block) {
			ARX_FLARES_Block = true;
		}
	}

	ARX_SPELLS_Precast_Check();
	
	if(ARXmenu.mode() == Mode_InGame) {
		ARX_SPELLS_ManageMagic();
	}
	
	ARX_SPELLS_UpdateSymbolDraw();

	ManageTorch();

	{
		float magicSightZoom = 0.f;
		
		SpellBase * spell = spells.getSpellByCaster(EntityHandle_Player, SPELL_MAGIC_SIGHT);
		if(spell) {
			GameDuration duration = g_gameTime.now() - spell->m_timcreation;
			magicSightZoom = glm::clamp(duration / GameDurationMs(500), 0.f, 1.f);
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
	GRenderer->Clear(Renderer::ColorBuffer | Renderer::DepthBuffer, g_fogColor);
	
	cinematicBorder.render();
	
	GRenderer->SetAntialiasing(true);
	
	GRenderer->SetFogParams(fZFogStart * ACTIVECAM->cdepth, fZFogEnd * ACTIVECAM->cdepth);
	GRenderer->SetFogColor(g_fogColor);

	ARX_SCENE_Render();
	
	drawDebugRender();

	// Begin Particles
	arx_assert(pParticleManager);
	pParticleManager->Render();
	
	ARX_PARTICLES_Update(&subj);
	ParticleSparkUpdate();
	
	// End Particles

	// Renders Magical Flares
	if(!((player.Interface & INTER_PLAYERBOOK) && !(player.Interface & INTER_COMBATMODE))) {
		ARX_MAGICAL_FLARES_Update();
	}

	// Checks some specific spell FX
	CheckMr();

	if(player.m_improve) {
		DrawImproveVisionInterface();
	}

	if(eyeball.exist != 0)
		DrawMagicSightInterface();

	if(player.m_paralysed) {
		UseRenderState state(render2D().blendAdditive());
		EERIEDrawBitmap(Rectf(g_size), 0.0001f, NULL, Color(71, 71, 255));
	}

	// Red screen fade for damages.
	ARX_DAMAGE_Show_Hit_Blood();

	// Update spells
	ARX_SPELLS_Update();

	GRenderer->SetFogColor(Color::none);
	g_renderBatcher.render();
	GRenderer->SetFogColor(g_fogColor);
	
	GRenderer->SetAntialiasing(false);
	
	// Manage Death visual & Launch menu...
	ARX_PLAYER_Manage_Death();

	// INTERFACE
	g_renderBatcher.clear();
	
	// Draw game interface if needed
	if(ARXmenu.mode() == Mode_InGame && !cinematicBorder.isActive()) {
	
		GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapClamp);
		
		ARX_INTERFACE_NoteManage();
		g_hudRoot.draw();
		
		if((player.Interface & INTER_PLAYERBOOK) && !(player.Interface & INTER_COMBATMODE)) {
			ARX_MAGICAL_FLARES_Update();
			g_renderBatcher.render();
		}
		
		GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
		
	}

	update2DFX();
	goFor2DFX();

	GRenderer->Clear(Renderer::DepthBuffer);

	// Speech Management
	ARX_SPEECH_Check();

	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);

	if(pTextManage && !pTextManage->Empty()) {
		pTextManage->Update(g_platformTime.lastFrameDuration());
		pTextManage->Render();
	}

	if(SHOW_INGAME_MINIMAP
		&& cinematicIsStopped()
		&& !cinematicBorder.isActive()
		&& !BLOCK_PLAYER_CONTROLS
		&& !(player.Interface & INTER_PLAYERBOOK))
	{
		long SHOWLEVEL = ARX_LEVELS_GetRealNum(CURRENTLEVEL);

		if(SHOWLEVEL >= 0 && SHOWLEVEL < 32)
			g_miniMap.showPlayerMiniMap(SHOWLEVEL);
	}

	// CURSOR Rendering

	if(DRAGINTER) {
		ARX_INTERFACE_RenderCursor();
		PopAllTriangleListOpaque();
		PopAllTriangleListTransparency();
	} else {
		ARX_INTERFACE_RenderCursor();
	}

	CheatDrawText();

	if(FADEDIR)
		ManageFade();
	
	GRenderer->SetScissor(Rect::ZERO);
	
	ARX_SPEECH_Update();
	
}

void ArxGame::render() {
	
	ACTIVECAM = &subj;

	// Update Various Player Infos for this frame.
	ARX_PLAYER_Frame_Update();
		
	subj.center = Vec2i(g_size.center().x, g_size.center().y);
	subj.orgTrans.mod = Vec2f(subj.center);
	
	PULSATE = timeWaveSin(g_gameTime.now(), GameDurationMsf(5026.548245f));
	EERIEDrawnPolys = 0;

	// Checks for Keyboard & Moulinex
	{
	g_cursorOverBook = false;
	
	if(ARXmenu.mode() == Mode_InGame) { // Playing Game
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
	
	if(!player.m_paralysed || ARXmenu.mode() != Mode_InGame) {
			manageKeyMouse();
	}
	}

	if(CheckInPoly(player.pos)) {
		LastValidPlayerPos = player.pos;
	}

	// Updates Externalview
	EXTERNALVIEW = false;

	if(g_debugTriggers[1])
		g_hudRoot.bookIconGui.requestFX();
	
	if(ARXmenu.mode() != Mode_InGame) {
		benchmark::begin(benchmark::Menu);
		ARX_Menu_Render();
		GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat); // << NEEDED?
	} else if(isInCinematic()) {
		benchmark::begin(benchmark::Cinematic);
		cinematicRender();
	} else {
		benchmark::begin(cinematicBorder.CINEMA_DECAL != 0.f ? benchmark::Cutscene : benchmark::Scene);
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
			ShowFrameDurationPlot();
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
	
	g_console.draw();
	
	if(ARXmenu.mode() == Mode_InGame) {
		ARX_SCRIPT_AllowInterScriptExec();
		ARX_SCRIPT_EventStackExecute();
		// Updates Damages Spheres
		ARX_DAMAGES_UpdateAll();
		ARX_MISSILES_Update();

		ARX_PATH_UpdateAllZoneInOutInside();
	}

	LastMouseClick = EERIEMouseButton;
	
	gldebug::endFrame();
}

void ArxGame::onRendererInit(Renderer & renderer) {
	
	arx_assert(GRenderer == NULL);
	
	GRenderer = &renderer;
	
	arx_assert_msg(renderer.getTextureStageCount() >= 3, "not enough texture units");
	arx_assert(m_MainWindow);
	
	renderer.Clear(Renderer::ColorBuffer);
	m_MainWindow->showFrame();
	
	// Restore All Textures RenderState
	renderer.RestoreAllTextures();

	ARX_PLAYER_Restore_Skin();
	
	// Setup Texture Border RenderState
	renderer.GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
	
	// Fog
	float fogEnd = 0.48f;
	float fogStart = fogEnd * 0.65f;
	renderer.SetFogParams(fogStart, fogEnd);
	renderer.SetFogColor(g_fogColor);
	
	ComputePortalVertexBuffer();
	VertexBuffer<SMY_VERTEX3> * vb3 = renderer.createVertexBuffer3(4000, Renderer::Stream);
	pDynamicVertexBuffer = new CircularVertexBuffer<SMY_VERTEX3>(vb3);
	
	size_t size = (config.video.bufferSize < 1) ? 32 * 1024 : config.video.bufferSize * 1024;
	VertexBuffer<TexturedVertex> * vb = renderer.createVertexBufferTL(size, Renderer::Stream);
	pDynamicVertexBuffer_TLVERTEX = new CircularVertexBuffer<TexturedVertex>(vb);
	
	MenuReInitAll();
	
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
