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

#include <boost/foreach.hpp>

#include <glm/gtx/norm.hpp>

#include "ai/PathFinderManager.h"
#include "ai/Paths.h"

#include "animation/Animation.h"
#include "animation/AnimationRender.h"

#include "cinematic/Cinematic.h"
#include "cinematic/CinematicController.h"

#include "core/Core.h"
#include "core/Config.h"
#include "core/GameTime.h"
#include "core/Localisation.h"
#include "core/SaveGame.h"
#include "core/Version.h"

#include "game/Damage.h"
#include "game/EntityManager.h"
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
#include "graphics/data/Mesh.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/Fade.h"
#include "graphics/effects/Fog.h"
#include "graphics/font/Font.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/ParticleManager.h"
#include "graphics/particle/MagicFlare.h"
#include "graphics/texture/TextureStage.h"

#include "gui/Cursor.h"
#include "gui/DebugHud.h"
#include "gui/Hud.h"
#include "gui/Interface.h"
#include "gui/Menu.h"
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

#include "io/fs/FilePath.h"
#include "io/fs/SystemPaths.h"
#include "io/resource/PakReader.h"
#include "io/Screenshot.h"
#include "io/log/CriticalLogger.h"
#include "io/log/Logger.h"

#include "platform/Dialog.h"
#include "platform/Flags.h"
#include "platform/Platform.h"
#include "platform/Process.h"

#include "scene/ChangeLevel.h"
#include "scene/Interactive.h"
#include "scene/GameSound.h"
#include "scene/Light.h"
#include "scene/LoadLevel.h"
#include "scene/Object.h"
#include "scene/Scene.h"

#include "Configure.h"
#include "core/URLConstants.h"

#if ARX_HAVE_SDL2
#include "window/SDL2Window.h"
#endif
#if ARX_HAVE_SDL1
#include "window/SDL1Window.h"
#endif

enum InfoPanels {
	InfoPanelNone,
	InfoPanelFramerate,
	InfoPanelFramerateGraph,
	InfoPanelDebug,
	InfoPanelTest,
	InfoPanelDebugToggles,
	InfoPanelEnumSize
};

static InfoPanels g_debugInfo = InfoPanelNone;

using std::string;

extern long START_NEW_QUEST;
long LOADQUEST_SLOT = -1; // OH NO, ANOTHER GLOBAL! - TEMP PATCH TO CLEAN CODE FLOW
extern long PLAYER_PARALYSED;
extern long DeadTime;
extern long LaunchDemo;

static const float CURRENT_BASE_FOCAL = 310.f;
extern float BOW_FOCAL;

extern float GLOBAL_SLOWDOWN;
extern float LAST_FADEVALUE;

extern Cinematic* ControlCinematique;
extern EERIE_3DOBJ * arrowobj;

extern TextureContainer * tflare;
extern Entity * FlyingOverIO;
extern Vec3f LastValidPlayerPos;
extern Color ulBKGColor;
extern EERIE_CAMERA conversationcamera;
extern ParticleManager * pParticleManager;
extern CircularVertexBuffer<TexturedVertex> * pDynamicVertexBuffer_TLVERTEX; // VB using TLVERTEX format.
extern CircularVertexBuffer<SMY_VERTEX3> * pDynamicVertexBuffer;
extern CMenuState * pMenu;

extern glm::mat4x4 ProjectionMatrix;

TextureContainer * Movable = NULL;   // TextureContainer for Movable Items (Red Cross)

long STOP_KEYBOARD_INPUT= 0;

long BOOKBUTTON=0;
long LASTBOOKBUTTON=0;
bool EXTERNALVIEW = false;
bool ARX_CONVERSATION = false;
long ARX_CONVERSATION_MODE=-1;
long ARX_CONVERSATION_LASTIS=-1;
static bool LAST_CONVERSATION = 0;
bool SHOW_INGAME_MINIMAP = true;

bool ARX_FLARES_Block = true;

Vec3f LASTCAMPOS;
Anglef LASTCAMANGLE;
Entity * CAMERACONTROLLER=NULL;
Entity *lastCAMERACONTROLLER=NULL;

// ArxGame constructor. Sets attributes for the app.
ArxGame::ArxGame() : wasResized(false) { }

ArxGame::~ArxGame() {
}

bool ArxGame::initialize()
{
	bool init = Application::initialize();
	if(!init) {
		return false;
	}
	
	init = initGameData();
	if(!init) {
		LogCritical << "Failed to initialize the game data.";
		return false;
	}
	
	init = initLocalisation();
	if(!init) {
		LogCritical << "Failed to initialize the localisation subsystem.";
		return false;
	}
	
	create();
	
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
	
	m_MainWindow->setTitle(arx_version);
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
		LogCritical << "Graphics initialization failed.";
		return false;
	}
	
	return true;
}

bool ArxGame::initInput() {
	
	LogDebug("Input init");
	bool init = ARX_INPUT_Init(m_MainWindow);
	if(!init) {
		LogCritical << "Input initialization failed.";
	}
	
	return init;
}

bool ArxGame::initSound() {
	
	LogDebug("Sound init");
	bool init = ARX_SOUND_Init();
	if(!init) {
		LogWarning << "Sound initialization failed.";
	}
	
	return true;
}

bool ArxGame::initGameData() {
	
	bool init = addPaks();
	if(!init) {
		LogCritical << "Error loading pak files";
		return false;
	}

	ARX_SOUND_LoadData();
	
	savegames.update(true);
	
	return init;
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
		platform::showErrorDialog(error.str(), "Critical Error - " + arx_version);
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
		LogError << oss.str();
		
		// Try to launch the data file installer on non-Windows systems
		#if ARX_PLATFORM != ARX_PLATFORM_WIN32
		const char * question = "Install the Arx Fatalis data files now?";
		logger::CriticalErrorDialog::setExitQuestion(question, runDataFilesInstaller);
		#endif
		
		// Construct an informative error message about missing files
		oss.str(std::string());
		oss << "Could not load required data files.\n";
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

bool ArxGame::create() {
	
	return true;
}

void ArxGame::onWindowGotFocus(const Window &) {
	if(GInput) {
		GInput->reset();
	}
}

void ARX_INTERFACE_Combat_Mode(long i);

void ArxGame::onWindowLostFocus(const Window &) {
	// TODO(option-control) add a config option for this
	ARX_INTERFACE_Combat_Mode(0);
	TRUE_PLAYER_MOUSELOOK_ON = false;
	PLAYER_MOUSELOOK_ON = false;
	// TODO(option-audio) add a config option to disable audio on focus loss
}

void ArxGame::onResizeWindow(const Window & window) {
	
	arx_assert(window.getSize() != Vec2i_ZERO);
	
	// A new window size will require a new backbuffer
	// size, so the 3D structures must be changed accordingly.
	wasResized = true;
	
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
	
	beforeRun();
	
	while(m_RunLoop) {
		
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
}

/*!
 * \brief Draws the scene.
 */
void ArxGame::doFrame() {
		
	updateTime();

	updateInput();

	if(wasResized) {
		LogDebug("was resized");
		wasResized = false;
		DanaeRestoreFullScreen();
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
		LogDebug("teleport to " << TELEPORT_TO_LEVEL << " " << TELEPORT_TO_POSITION << " " << TELEPORT_TO_ANGLE);
		CHANGE_LEVEL_ICON = -1;
		ARX_CHANGELEVEL_Change(TELEPORT_TO_LEVEL, TELEPORT_TO_POSITION, TELEPORT_TO_ANGLE);
		TELEPORT_TO_LEVEL.clear();
		TELEPORT_TO_POSITION.clear();
	}

	if(LOADQUEST_SLOT != -1) {
		ARX_SlotLoad(LOADQUEST_SLOT);
		LOADQUEST_SLOT = -1;
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
			showQuickSaveIcon();
			GRenderer->getSnapshot(savegame_thumbnail, 160, 100);
			ARX_QuickSave();
		}
		
	}
	
	if(FirstFrame) {
		FirstFrameHandling();
	} else {
		update();
		render();
	}
}

/*!
 * \brief Cleanup scene objects
 */
void ArxGame::cleanup3DEnvironment() {
	
	if(getWindow()) {
		finalCleanup();
	}
	
}

bool ArxGame::beforeRun() {
	
	LogDebug("Before Run...");
	
	const Vec2i & size = getWindow()->getSize();
	ControlCinematique = new Cinematic(size.x, size.y);
	
	memset(&necklace,0,sizeof(ARX_NECKLACE));
	
	long old = GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE;
	GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE = -1;
	
	necklace.lacet = loadObject("graph/interface/book/runes/lacet.teo");
	
	necklace.runes[RUNE_AAM] =         loadObject("graph/interface/book/runes/runes_aam.teo");
	necklace.runes[RUNE_CETRIUS] =     loadObject("graph/interface/book/runes/runes_citrius.teo");
	necklace.runes[RUNE_COMUNICATUM] = loadObject("graph/interface/book/runes/runes_comunicatum.teo");
	necklace.runes[RUNE_COSUM] =       loadObject("graph/interface/book/runes/runes_cosum.teo");
	necklace.runes[RUNE_FOLGORA] =     loadObject("graph/interface/book/runes/runes_folgora.teo");
	necklace.runes[RUNE_FRIDD] =       loadObject("graph/interface/book/runes/runes_fridd.teo");
	necklace.runes[RUNE_KAOM] =        loadObject("graph/interface/book/runes/runes_kaom.teo");
	necklace.runes[RUNE_MEGA] =        loadObject("graph/interface/book/runes/runes_mega.teo");
	necklace.runes[RUNE_MORTE] =       loadObject("graph/interface/book/runes/runes_morte.teo");
	necklace.runes[RUNE_MOVIS] =       loadObject("graph/interface/book/runes/runes_movis.teo");
	necklace.runes[RUNE_NHI] =         loadObject("graph/interface/book/runes/runes_nhi.teo");
	necklace.runes[RUNE_RHAA] =        loadObject("graph/interface/book/runes/runes_rhaa.teo");
	necklace.runes[RUNE_SPACIUM] =     loadObject("graph/interface/book/runes/runes_spacium.teo");
	necklace.runes[RUNE_STREGUM] =     loadObject("graph/interface/book/runes/runes_stregum.teo");
	necklace.runes[RUNE_TAAR] =        loadObject("graph/interface/book/runes/runes_taar.teo");
	necklace.runes[RUNE_TEMPUS] =      loadObject("graph/interface/book/runes/runes_tempus.teo");
	necklace.runes[RUNE_TERA] =        loadObject("graph/interface/book/runes/runes_tera.teo");
	necklace.runes[RUNE_VISTA] =       loadObject("graph/interface/book/runes/runes_vista.teo");
	necklace.runes[RUNE_VITAE] =       loadObject("graph/interface/book/runes/runes_vitae.teo");
	necklace.runes[RUNE_YOK] =         loadObject("graph/interface/book/runes/runes_yok.teo");
	
	necklace.pTexTab[RUNE_AAM] = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_aam[icon]");
	necklace.pTexTab[RUNE_CETRIUS] = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_cetrius[icon]");
	necklace.pTexTab[RUNE_COMUNICATUM] = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_comunicatum[icon]");
	necklace.pTexTab[RUNE_COSUM] = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_cosum[icon]");
	necklace.pTexTab[RUNE_FOLGORA] = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_folgora[icon]");
	necklace.pTexTab[RUNE_FRIDD] = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_fridd[icon]");
	necklace.pTexTab[RUNE_KAOM] = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_kaom[icon]");
	necklace.pTexTab[RUNE_MEGA] = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_mega[icon]");
	necklace.pTexTab[RUNE_MORTE] = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_morte[icon]");
	necklace.pTexTab[RUNE_MOVIS] = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_movis[icon]");
	necklace.pTexTab[RUNE_NHI] = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_nhi[icon]");
	necklace.pTexTab[RUNE_RHAA] = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_rhaa[icon]");
	necklace.pTexTab[RUNE_SPACIUM] = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_spacium[icon]");
	necklace.pTexTab[RUNE_STREGUM] = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_stregum[icon]");
	necklace.pTexTab[RUNE_TAAR] = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_taar[icon]");
	necklace.pTexTab[RUNE_TEMPUS] = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_tempus[icon]");
	necklace.pTexTab[RUNE_TERA] = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_tera[icon]");
	necklace.pTexTab[RUNE_VISTA] = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_vista[icon]");
	necklace.pTexTab[RUNE_VITAE] = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_vitae[icon]");
	necklace.pTexTab[RUNE_YOK] = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_yok[icon]");
	
	for(size_t i = 0; i < RUNE_COUNT-1; i++) { // TODO why -1?
		if(necklace.pTexTab[i]) {
			necklace.pTexTab[i]->getHalo();
		}
	}
	
	drawDebugInitialize();

	FlyingEye_Init();
	
	cabal = LoadTheObj("editor/obj3d/cabal.teo", "cabal_teo maps");
	
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
		
		oss.str(string());
		
		if(i == 0) {
			oss << "graph/obj3d/interactive/items/jewelry/gold_coin/gold_coin[icon]";
		} else {
			oss << "graph/obj3d/interactive/items/jewelry/gold_coin/gold_coin" << (i + 1) << "[icon]";
		}
		
		GoldCoinsTC[i] = TextureContainer::LoadUI(oss.str());
	}
	
	Movable = TextureContainer::LoadUI("graph/interface/cursors/wrong");
	
	ARX_PLAYER_LoadHeroAnimsAndMesh();
	
	GLOBAL_EERIETEXTUREFLAG_LOADSCENE_RELEASE = old;
	
	return true;
}

void ArxGame::updateFirstPersonCamera() {

	Entity * io = entities.player();
	ANIM_USE * useanim = &io->animlayer[1];
	ANIM_HANDLE ** alist = io->anims;

	if ( BOW_FOCAL
		&& (useanim->cur_anim!=alist[ANIM_MISSILE_STRIKE_PART_1])
		&& (useanim->cur_anim!=alist[ANIM_MISSILE_STRIKE_PART_2])
		&& (useanim->cur_anim!=alist[ANIM_MISSILE_STRIKE_CYCLE]))
	{
		BOW_FOCAL -= Original_framedelay;

		if(BOW_FOCAL < 0)
			BOW_FOCAL = 0;
	}

	if(eyeball.exist == 2) {
		subj.d_pos = eyeball.pos;
		subj.d_angle = eyeball.angle;
		EXTERNALVIEW = true;
	} else if(EXTERNALVIEW) {
		float t=radians(player.angle.getPitch());

		for(long l=0; l < 250; l += 10) {
			Vec3f tt = player.pos;
			tt.x += std::sin(t)*(float)l;
			tt.y -= 50.f;
			tt.z -= std::cos(t)*(float)l;
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
		
		if(entities.player()) {
			long id = entities.player()->obj->fastaccess.view_attach;

			if(id!=-1) {
				subj.orgTrans.pos = entities.player()->obj->vertexlist3[id].v;
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
	}

	if(EXTERNALVIEW) {
		subj.orgTrans.pos = (subj.orgTrans.pos + subj.d_pos) * 0.5f;
		subj.angle = interpolate(subj.angle, subj.d_angle, 0.1f);
	}
}

void ArxGame::updateConversationCamera() {

	if(ARX_CONVERSATION && main_conversation.actors_nb) {
		// Decides who speaks !!
		if(main_conversation.current < 0)
			for(long j=0; j < main_conversation.actors_nb; j++) {
				if(main_conversation.actors[j] >= PlayerEntityHandle) {
					for(size_t k = 0 ; k < MAX_ASPEECH; k++) {
							if(aspeech[k].exist && aspeech[k].io == entities[main_conversation.actors[j]]) {
								main_conversation.current = k;
								j = main_conversation.actors_nb + 1;
								k = MAX_ASPEECH+1;
							}
					}
				}
			}

		long is = main_conversation.current;

		if(ARX_CONVERSATION_LASTIS != is)
			ARX_CONVERSATION_MODE = -1;

		ARX_CONVERSATION_LASTIS = is;

		if(ARX_CONVERSATION_MODE == -1) {
			ARX_CONVERSATION_MODE = 0;
			if(rnd() > 0.5f) {
				conversationcamera.size = Anglef(MAKEANGLE(170.f + rnd() * 20.f), 0.f, 0.f);
				conversationcamera.d_angle = Anglef(0.f, 0.f, 0.08f);
			} else {
				conversationcamera.size = Anglef(rnd() * 50.f, 0.f, rnd() * 50.f);
				conversationcamera.d_angle = Anglef::ZERO;
				if(rnd() > 0.4f) {
					conversationcamera.d_angle.setYaw((1.f - rnd() * 2.f) * (1.f / 30));
				}
				if(rnd() > 0.4f) {
					conversationcamera.d_angle.setPitch((1.f - rnd() * 1.2f) * 0.2f);
				}
				if(rnd() > 0.4f) {
					conversationcamera.d_angle.setRoll((1.f - rnd() * 2.f) * 0.025f);
				}
			}
		} else {
			conversationcamera.size += conversationcamera.d_angle * framedelay;
		}

		Vec3f sourcepos,targetpos;

		if(ApplySpeechPos(&conversationcamera, is)) {
			targetpos = conversationcamera.d_pos;
			sourcepos = conversationcamera.orgTrans.pos;
		} else {
			targetpos = player.pos;
			float t = radians(player.angle.getPitch());
			sourcepos.x=targetpos.x+std::sin(t)*100.f;
			sourcepos.y=targetpos.y;
			sourcepos.z=targetpos.z-std::cos(t)*100.f;
		}
		
		Vec3f vect = targetpos - sourcepos;
		vect = glm::normalize(vect);

		float dist = 250.f - conversationcamera.size.getRoll();
		if(dist < 0.f)
			dist = 90.f - dist * (1.f/20);
		else if(dist < 90.f)
			dist = 90.f;

		Vec3f vec2 = VRotateY(vect, conversationcamera.size.getYaw());

		sourcepos = targetpos - vec2 * dist;

		if(conversationcamera.size.getPitch() != 0.f)
			sourcepos.y += 120.f - conversationcamera.size.getPitch() * (1.f/10);

		conversationcamera.orgTrans.pos = sourcepos;
		conversationcamera.setTargetCamera(targetpos);
		subj.orgTrans.pos = conversationcamera.orgTrans.pos;
		subj.angle.setYaw(MAKEANGLE(-conversationcamera.angle.getYaw()));
		subj.angle.setPitch(MAKEANGLE(conversationcamera.angle.getPitch() - 180.f));
		subj.angle.setRoll(0.f);
		EXTERNALVIEW = true;
	} else {
		ARX_CONVERSATION_MODE = -1;
		ARX_CONVERSATION_LASTIS = -1;

		if(LAST_CONVERSATION) {
			changeAnimation(entities.player(), 1, entities.player()->anims[ANIM_WAIT], EA_LOOP);
		}
	}

	LAST_CONVERSATION = ARX_CONVERSATION;
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
		CinematicSpeech * acs=&aspeech[valid].cine;
		Entity * io=aspeech[valid].io;
		float rtime=(float)(arxtime.get_updated()-aspeech[valid].time_creation)/(float)aspeech[valid].duration;

		rtime = glm::clamp(rtime, 0.f, 1.f);

		float itime=1.f-rtime;

		if(rtime >= 0.f && rtime <= 1.f && io) {
			switch(acs->type) {
			case ARX_CINE_SPEECH_KEEP: {
				subj.orgTrans.pos = acs->pos1;
				subj.angle.setYaw(acs->pos2.x);
				subj.angle.setPitch(acs->pos2.y);
				subj.angle.setRoll(acs->pos2.z);
				EXTERNALVIEW = true;
				break;
									   }
			case ARX_CINE_SPEECH_ZOOM: {
				//need to compute current values
				float alpha = acs->startangle.getYaw() * itime + acs->endangle.getYaw() * rtime;
				float beta = acs->startangle.getPitch() * itime + acs->endangle.getPitch() * rtime;
				float distance = acs->startpos * itime + acs->endpos * rtime;
				Vec3f targetpos = acs->pos1;
				conversationcamera.orgTrans.pos.x=-std::sin(radians(MAKEANGLE(io->angle.getPitch()+beta)))*distance+targetpos.x;
				conversationcamera.orgTrans.pos.y= std::sin(radians(MAKEANGLE(io->angle.getYaw()+alpha)))*distance+targetpos.y;
				conversationcamera.orgTrans.pos.z= std::cos(radians(MAKEANGLE(io->angle.getPitch()+beta)))*distance+targetpos.z;
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
				if(ValidIONum(acs->ionum)) {
					const Vec3f & from = acs->pos1;
					const Vec3f & to = acs->pos2;

					Vec3f vect = glm::normalize(to - from);

					Vec3f vect2;
					if(acs->type==ARX_CINE_SPEECH_SIDE_LEFT) {
						vect2 = VRotateY(vect, -90.f);
					} else {
						vect2 = VRotateY(vect, 90.f);
					}

					float distance=acs->f0*itime+acs->f1*rtime;
					vect2 *= distance;
					float _dist = glm::distance(from, to);
					Vec3f tfrom = from + vect * acs->startpos * (1.0f / 100) * _dist;
					Vec3f tto = from + vect * acs->endpos * (1.0f / 100) * _dist;
					Vec3f targetpos;
					targetpos.x=tfrom.x*itime+tto.x*rtime;
					targetpos.y=tfrom.y*itime+tto.y*rtime+acs->f2;
					targetpos.z=tfrom.z*itime+tto.z*rtime;
					conversationcamera.orgTrans.pos.x=targetpos.x+vect2.x;
					conversationcamera.orgTrans.pos.y=targetpos.y+vect2.y+acs->f2;
					conversationcamera.orgTrans.pos.z=targetpos.z+vect2.z;
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
				if(ValidIONum(acs->ionum)) {
					Vec3f targetpos;
					if(acs->type == ARX_CINE_SPEECH_CCCLISTENER_L
						 || acs->type == ARX_CINE_SPEECH_CCCLISTENER_R) {
						conversationcamera.orgTrans.pos = acs->pos2;
						targetpos = acs->pos1;
					} else {
						conversationcamera.orgTrans.pos = acs->pos1;
						targetpos = acs->pos2;
					}

					float distance = (acs->startpos * itime + acs->endpos * rtime) * (1.0f/100);

					Vec3f vect = conversationcamera.orgTrans.pos - targetpos;
					Vec3f vect2 = VRotateY(vect, 90.f);;
					
					vect2 = glm::normalize(vect2);
					Vec3f vect3 = glm::normalize(vect);

					vect = vect * distance + vect3 * 80.f;
					vect2 *= 45.f;

					if ((acs->type==ARX_CINE_SPEECH_CCCLISTENER_R)
						|| (acs->type==ARX_CINE_SPEECH_CCCTALKER_R))
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
		DeadTime += static_cast<long>(framedelay);
		float mdist = EEfabs(player.physics.cyl.height)-60;

		float startDistance = 40.f;

		float startTime = 2000.f;
		float endTime = 7000.f;

		float DeadCameraDistance = startDistance + (mdist - startDistance) * ((DeadTime - startTime) / (endTime - startTime));

		Vec3f targetpos = player.pos;

		long id  = entities.player()->obj->fastaccess.view_attach;
		long id2 = GetActionPointIdx(entities.player()->obj, "chest2leggings");

		if(id != -1) {
			targetpos = entities.player()->obj->vertexlist3[id].v;
		}

		conversationcamera.orgTrans.pos = targetpos;

		if(id2 != -1) {
			conversationcamera.orgTrans.pos = entities.player()->obj->vertexlist3[id2].v;
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

void ArxGame::handleCameraController() {

	static float currentbeta = 0.f;

	if(CAMERACONTROLLER) {
		if(lastCAMERACONTROLLER != CAMERACONTROLLER)
			currentbeta = CAMERACONTROLLER->angle.getPitch();

		Vec3f targetpos = CAMERACONTROLLER->pos + player.baseOffset();

		float delta_angle = AngleDifference(currentbeta, CAMERACONTROLLER->angle.getPitch());
		float delta_angle_t = delta_angle * framedelay * ( 1.0f / 1000 );

		if(EEfabs(delta_angle_t) > EEfabs(delta_angle))
			delta_angle_t = delta_angle;

		currentbeta += delta_angle_t;
		float t=radians(MAKEANGLE(currentbeta));
		conversationcamera.orgTrans.pos.x=targetpos.x+std::sin(t)*160.f;
		conversationcamera.orgTrans.pos.y=targetpos.y+40.f;
		conversationcamera.orgTrans.pos.z=targetpos.z-std::cos(t)*160.f;

		conversationcamera.setTargetCamera(targetpos);
		subj.orgTrans.pos = conversationcamera.orgTrans.pos;
		subj.angle.setYaw(MAKEANGLE(-conversationcamera.angle.getYaw()));
		subj.angle.setPitch(MAKEANGLE(conversationcamera.angle.getPitch()-180.f));
		subj.angle.setRoll(0.f);
		EXTERNALVIEW = true;
	}

	lastCAMERACONTROLLER=CAMERACONTROLLER;
}

void ArxGame::updateActiveCamera() {

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

		arxtime.increment_start_time((u64)(Original_framedelay * (1.0f - GLOBAL_SLOWDOWN) * 1000.0f));

		// recalculate frame delta
		arxtime.update_frame_time();
	}

	framedelay = arxtime.get_frame_delay();
	arx_assert(framedelay >= 0.0f);

	// limit fps above 10fps
	const float max_framedelay = 1000.0f / 10.0f;
	framedelay = framedelay > max_framedelay ? max_framedelay : framedelay;
}

void ArxGame::updateInput() {

	// Update input
	GInput->update();
	ReMappDanaeButton();
	AdjustMousePosition();

	if(GInput->actionNowPressed(CONTROLS_CUST_TOGGLE_FULLSCREEN)) {
		setWindowSize(!getWindow()->isFullScreen());
	}

	if(GInput->isKeyPressedNowPressed(Keyboard::Key_F12)) {
		EERIE_PORTAL_ReleaseOnlyVertexBuffer();
		ComputePortalVertexBuffer();
	}

	if(GInput->isKeyPressedNowPressed(Keyboard::Key_F11)) {

		g_debugInfo = static_cast<InfoPanels>(g_debugInfo + 1);

		if(g_debugInfo == InfoPanelEnumSize)
			g_debugInfo = InfoPanelNone;
	}

	if(g_debugInfo == InfoPanelDebugToggles) {
		for(size_t i = 0; i < ARRAY_SIZE(g_debugToggles); i++) {
			if(GInput->isKeyPressedNowPressed(Keyboard::Key_NumPad0 + i)) {
				g_debugToggles[i] = !g_debugToggles[i];
			}
		}
	}

	if(GInput->isKeyPressedNowPressed(Keyboard::Key_F10)) {
		GetSnapShot();
	}

	if(GInput->isKeyPressedNowPressed(Keyboard::Key_ScrollLock)) {
		drawDebugCycleViews();
	}

	if(GInput->isKeyPressedNowPressed(Keyboard::Key_Spacebar)) {
		CAMERACONTROLLER = NULL;
	}
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

	RenderBatcher::getInstance().clear();

	if(!PLAYER_PARALYSED) {
		manageEditorControls();

		if(!BLOCK_PLAYER_CONTROLS) {
			managePlayerControls();
		}
	}


	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity *entity = entities[handle];

		if(!entity)
			continue;

		if(entity->ignition > 0.f || (entity->ioflags & IO_FIERY))
			ManageIgnition(entity);

		Cedric_ApplyLightingFirstPartRefactor(entity);

		//Highlight entity
		if(entity == FlyingOverIO && !(entity->ioflags & IO_NPC)) {
			entity->highlightColor = Color3f::gray(float(iHighLight));
		} else {
			entity->highlightColor = Color3f::black;
		}

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


	ARX_PLAYER_Manage_Movement();

	ARX_PLAYER_Manage_Visual();

	g_miniMap.setActiveBackground(ACTIVEBKG);
	g_miniMap.validatePlayerPos(CURRENTLEVEL, BLOCK_PLAYER_CONTROLS, Book_Mode);


	if(entities.player() && entities.player()->animlayer[0].cur_anim) {
		ManageNONCombatModeAnimations();

		AnimatedEntityUpdate(entities.player(), Original_framedelay);

		if((player.Interface & INTER_COMBATMODE) && entities.player()->animlayer[1].cur_anim)
			ManageCombatModeAnimations();

		if(entities.player()->animlayer[1].cur_anim)
			ManageCombatModeAnimationsEND();
	}

	updateFirstPersonCamera();
	updateConversationCamera();

	ARX_SCRIPT_Timer_Check();

	speechControlledCinematic();

	handlePlayerDeath();

	handleCameraController();
	UpdateCameras();

	ARX_PLAYER_FrameCheck(Original_framedelay);

	updateActiveCamera();

	ARX_GLOBALMODS_Apply();

	// Set Listener Position
	{
		std::pair<Vec3f, Vec3f> frontUp = angleToFrontUpVecForSound(ACTIVECAM->angle);
		
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

	PrecalcIOLighting(&ACTIVECAM->orgTrans.pos, ACTIVECAM->cdepth * 0.6f);

	ACTIVECAM->fadecolor = current.depthcolor;

	ARX_SCENE_Update();

	if(pParticleManager) {
		pParticleManager->Update(static_cast<long>(framedelay));
	}

	ARX_FOGS_Render();

	TreatBackgroundActions();

	// Checks Magic Flares Drawing
	if(!PLAYER_PARALYSED) {
		if(EERIEMouseButton & 1) {
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

	if(!Project.improve) {
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
	GRenderer->SetFogParams(Renderer::FogLinear, fogStart, fogEnd);
	GRenderer->SetFogColor(ulBKGColor);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::DepthTest, true);

	ARX_SCENE_Render();
	
	drawDebugRender();

	// Begin Particles
		
	if(pParticleManager) {
		pParticleManager->Render();
	}
	
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	ARX_PARTICLES_Update(&subj);
	
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
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

	if(Project.improve) {
		DrawImproveVisionInterface();
	}

	if(eyeball.exist != 0)
		DrawMagicSightInterface();

	if(PLAYER_PARALYSED) {
		GRenderer->SetRenderState(Renderer::DepthWrite, false);
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);

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

	GRenderer->SetCulling(Renderer::CullNone);
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
		drawAllInterface();
		
		if((player.Interface & INTER_MAP) && !(player.Interface & INTER_COMBATMODE)) {
			ARX_MAGICAL_FLARES_Update();
			GRenderer->SetRenderState(Renderer::DepthWrite, false);
			RenderBatcher::getInstance().render();
			GRenderer->SetRenderState(Renderer::DepthWrite, true);
		}
		
		GRenderer->SetRenderState(Renderer::DepthTest, true);
		GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
	
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
		PopAllTriangleList();
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		PopAllTriangleListTransparency();
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	}	

	update2DFX();
	goFor2DFX();

	GRenderer->Clear(Renderer::DepthBuffer);

	// Speech Management
	ARX_SPEECH_Check();
	ARX_SPEECH_Update();

	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);

	if(pTextManage && !pTextManage->Empty()) {
		pTextManage->Update(framedelay);
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
		PopAllTriangleList();
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		PopAllTriangleListTransparency();
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);

		ARX_INTERFACE_HALO_Flush();
	} else {
		ARX_INTERFACE_HALO_Flush();
		ARX_INTERFACE_RenderCursor();
	}

	GRenderer->SetRenderState(Renderer::Fog, true);

	if(sp_max_start)
		Manage_sp_max();

	if(FADEDIR)
		ManageFade();
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

	// Finally computes current focal
	BASE_FOCAL = CURRENT_BASE_FOCAL + (BOW_FOCAL * (1.f/4));

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
			LASTBOOKBUTTON = BOOKBUTTON;
			BOOKBUTTON = EERIEMouseButton;
			
			if(((EERIEMouseButton & 1) && !(LastMouseClick & 1))
			   || ((EERIEMouseButton & 2) && !(LastMouseClick & 2))
			) {
				bookclick = true;
			}
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
	
	if(!PLAYER_PARALYSED || ARXmenu.currentmode != AMCM_OFF) {
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

	if(isInMenu()) {
		renderMenu();
	} else if(isInCinematic()) {
		cinematicRender();
	} else {
		updateLevel();
		UpdateInterface();
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
		case InfoPanelTest: {
			ShowTestText();
			break;
		}
		case InfoPanelDebugToggles: {
			ShowDebugToggles();
			break;
		}
		default: break;
		}
	}
	
	if(LaunchDemo) {
		LaunchDemo = 0;
		LaunchDummyParticle();
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
}

void ArxGame::update2DFX()
{
	TexturedVertex lv,ltvv;

	Entity* pTableIO[256];
	int nNbInTableIO = 0;

	float temp_increase = framedelay * (1.0f/1000) * 4.f;

	bool bComputeIO = false;

	for(int i=0; i < TOTPDL; i++) {
		EERIE_LIGHT *el = PDL[i];

		FAST_BKG_DATA * bkgData = getFastBackgroundData(el->pos.x, el->pos.z);

		if(!bkgData || !bkgData->treat) {
			el->treat=0;
			continue;
		}

		if(el->extras & EXTRAS_FLARE) {
			lv.p = el->pos;
			EE_RTP(lv.p, &ltvv);
			el->temp -= temp_increase;

			if(!(player.Interface & INTER_COMBATMODE) && (player.Interface & INTER_MAP))
				continue;

			if(ltvv.rhw > 0.f &&
				ltvv.p.x > 0.f &&
				ltvv.p.y > (CINEMA_DECAL*g_sizeRatio.y) &&
				ltvv.p.x < g_size.width() &&
				ltvv.p.y < (g_size.height()-(CINEMA_DECAL*g_sizeRatio.y))
				)
			{
				Vec3f vector = lv.p - ACTIVECAM->orgTrans.pos;
				lv.p -= vector * (50.f / glm::length(vector));

				float fZFar=ProjectionMatrix[2][2]*(1.f/(ACTIVECAM->cdepth*fZFogEnd))+ProjectionMatrix[3][2];

				Vec3f hit;
				EERIEPOLY *tp=NULL;
				Vec2s ees2dlv;
				Vec3f ee3dlv = lv.p;

				ees2dlv.x = checked_range_cast<short>(ltvv.p.x);
				ees2dlv.y = checked_range_cast<short>(ltvv.p.y);

				if(!bComputeIO) {
					GetFirstInterAtPos(ees2dlv, 2, &ee3dlv, pTableIO, &nNbInTableIO);
					bComputeIO = true;
				}

				if(ltvv.p.z > fZFar ||
					EERIELaunchRay3(ACTIVECAM->orgTrans.pos, ee3dlv, &hit, tp, 1) ||
					GetFirstInterAtPos(ees2dlv, 3, &ee3dlv, pTableIO, &nNbInTableIO )
					)
				{
					el->temp-=temp_increase*2.f;
				} else {
					el->temp+=temp_increase*2.f;
				}
			}

			el->temp = glm::clamp(el->temp, 0.f, .8f);
		}
	}
}

void ArxGame::goFor2DFX()
{
	GRenderer->SetRenderState(Renderer::Fog, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthTest, false);
	GRenderer->SetFogColor(Color::none);

	for (int i=0; i < TOTPDL; i++) {
		EERIE_LIGHT *el = PDL[i];

		if(!el->exist || !el->treat)
			continue;

		if(el->extras & EXTRAS_FLARE) {
			if (el->temp > 0.f) {
				Vec3f ltvv;
				EE_RT(el->pos, ltvv);
				float v=el->temp;

				if(FADEDIR) {
					v *= 1.f - LAST_FADEVALUE;
				}

				float siz;

				if(el->extras & EXTRAS_FIXFLARESIZE)
					siz = el->ex_flaresize;
				else
					siz = -el->ex_flaresize;

				EERIEDrawSprite(el->pos, siz, tflare, (el->rgb * v).to<u8>(), ltvv.z);

			}
		}
	}

	GRenderer->SetRenderState(Renderer::DepthTest, true);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::Fog, false);
}


bool ArxGame::initDeviceObjects() {
	
	// Enable Z-buffering RenderState
	GRenderer->SetRenderState(Renderer::DepthTest, true);
	
	// Restore All Textures RenderState
	GRenderer->RestoreAllTextures();

	ARX_PLAYER_Restore_Skin();
	
	// Disable Lighting RenderState
	GRenderer->SetRenderState(Renderer::Lighting, false);

	// Setup Texture Border RenderState
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);

	GRenderer->GetTextureStage(1)->disableColor();
	
	// Fog
	float fogEnd = 0.48f;
	float fogStart = fogEnd * 0.65f;
	GRenderer->SetFogParams(Renderer::FogLinear, fogStart, fogEnd);
	GRenderer->SetFogColor(current.depthcolor.to<u8>());
	GRenderer->SetRenderState(Renderer::Fog, true);
	
	GRenderer->SetDepthBias(0);

	ComputePortalVertexBuffer();
	VertexBuffer<SMY_VERTEX3> * vb3 = GRenderer->createVertexBuffer3(4000, Renderer::Stream);
	pDynamicVertexBuffer = new CircularVertexBuffer<SMY_VERTEX3>(vb3);
	
	VertexBuffer<TexturedVertex> * vb = GRenderer->createVertexBufferTL(4000, Renderer::Stream);
	pDynamicVertexBuffer_TLVERTEX = new CircularVertexBuffer<TexturedVertex>(vb);

	RenderBatcher::getInstance().initialize();

	if(pMenu) {
		pMenu->bReInitAll=true;
	}

	ARX_SetAntiAliasing();

	return true;
}

bool ArxGame::finalCleanup() {
	
	EERIE_PATHFINDER_Release();
	ARX_INPUT_Release();
	ARX_SOUND_Release();
	
	return true;
}

void ArxGame::onRendererInit(Renderer & renderer) {
	
	arx_assert(GRenderer == NULL);
	
	GRenderer = &renderer;
	
	arx_assert(GRenderer->GetTextureStageCount() >= 3, "not enough texture units");
	
	if(m_MainWindow) {
		GRenderer->Clear(Renderer::ColorBuffer);
		m_MainWindow->showFrame();
	}
	
	initDeviceObjects();
	
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

	RenderBatcher::getInstance().shutdown();
	
	EERIE_PORTAL_ReleaseOnlyVertexBuffer();
	
	GRenderer = NULL;
}
