/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#include "ai/PathFinderManager.h"
#include "ai/Paths.h"

#include "animation/Animation.h"
#include "animation/Cinematic.h"

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

#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Color.h"
#include "graphics/Draw.h"
#include "graphics/GraphicsModes.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "graphics/Vertex.h"
#include "graphics/VertexBuffer.h"
#include "graphics/data/Mesh.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/Fog.h"
#include "graphics/font/Font.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/ParticleManager.h"
#include "graphics/texture/TextureStage.h"

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
#include "math/MathFwd.h"
#include "math/Rectangle.h"
#include "math/Vector2.h"
#include "math/Vector3.h"

#include "io/fs/FilePath.h"
#include "io/fs/SystemPaths.h"
#include "io/resource/PakReader.h"
#include "io/Screenshot.h"
#include "io/log/Logger.h"

#include "platform/Flags.h"
#include "platform/Platform.h"

#include "scene/ChangeLevel.h"
#include "scene/Interactive.h"
#include "scene/GameSound.h"
#include "scene/Light.h"
#include "scene/LoadLevel.h"
#include "scene/Object.h"
#include "scene/Scene.h"

#include "Configure.h"
#include "core/URLConstants.h"

#ifdef ARX_HAVE_D3D9
#include "window/D3D9Window.h"
#endif
#ifdef ARX_HAVE_SDL
#include "window/SDLWindow.h"
#endif

static bool showFPS = false;

using std::string;

extern long PLAY_LOADED_CINEMATIC;
extern long START_NEW_QUEST;
long LOADQUEST_SLOT = -1; // OH NO, ANOTHER GLOBAL! - TEMP PATCH TO CLEAN CODE FLOW
extern long CHANGE_LEVEL_ICON;
extern long REFUSE_GAME_RETURN;
extern bool PLAYER_MOUSELOOK_ON;
extern bool TRUE_PLAYER_MOUSELOOK_ON;
extern long FRAME_COUNT;
extern long PLAYER_PARALYSED;
extern long STOP_KEYBOARD_INPUT;
extern long USEINTERNORM;
extern long cur_mr;
extern long cur_rf;
extern long STRIKE_TIME;
extern long DeadTime;
extern long TRANSPOLYSPOS;
extern long FORCE_FRONT_DRAW;
extern int iTimeToDrawD7;
extern long LaunchDemo;

extern short uw_mode;

extern long CURRENT_BASE_FOCAL;
extern float BOW_FOCAL;

extern float GLOBAL_SLOWDOWN;
extern float sp_max_start;
extern float LAST_FADEVALUE;

extern float PROGRESS_BAR_TOTAL;
extern float PROGRESS_BAR_COUNT;
extern float OLD_PROGRESS_BAR_COUNT;

extern void DANAE_KillCinematic();
extern void LaunchWaitingCine();

extern Cinematic* ControlCinematique;
extern EERIE_3DOBJ* eyeballobj;
extern EERIE_3DOBJ * arrowobj;
extern TextureContainer * Movable;
extern TextureContainer * tflare;
extern Entity * FlyingOverIO;
extern E_ARX_STATE_MOUSE eMouseState;
extern Vec3f LastValidPlayerPos;
extern Color ulBKGColor;
extern EERIE_CAMERA conversationcamera;
extern ParticleManager * pParticleManager;
extern CircularVertexBuffer<TexturedVertex> * pDynamicVertexBuffer_TLVERTEX; // VB using TLVERTEX format.
extern CircularVertexBuffer<SMY_VERTEX3> * pDynamicVertexBuffer;
extern CMenuState * pMenu;

extern EERIEMATRIX ProjectionMatrix;

TextureContainer * ChangeLevel = NULL;
TextureContainer * Movable = NULL;   // TextureContainer for Movable Items (Red Cross)

long BOOKBUTTON=0;
long LASTBOOKBUTTON=0;
long EXTERNALVIEW=0;
long ARX_CONVERSATION=0;
long ARX_CONVERSATION_MODE=-1;
long ARX_CONVERSATION_LASTIS=-1;
static long LAST_CONVERSATION = 0;
long SHOW_INGAME_MINIMAP= 1;
static unsigned long FRAMETICKS=0;

float PLAYER_ARMS_FOCAL = 350.f;

unsigned char ARX_FLARES_Block=1;

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

void ArxGame::setFullscreen(bool fullscreen) {
	
	if(fullscreen) {
		
		RenderWindow::DisplayMode mode(config.video.resolution, config.video.bpp);
		if(mode.resolution == Vec2i::ZERO) {
			mode = getWindow()->getDisplayModes().back();
		}
		
		// Clamp to a sane resolution!
		mode.resolution.x = std::max(mode.resolution.x, s32(640));
		mode.resolution.y = std::max(mode.resolution.y, s32(480));
		
		getWindow()->setFullscreenMode(mode.resolution, mode.depth);
		
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
	m_MainWindow->addRenderListener(this);
	
	const RenderWindow::DisplayModes & modes = window->getDisplayModes();
	
	RenderWindow::DisplayMode mode(config.video.resolution, config.video.bpp);
	
	if(config.video.resolution == Vec2i::ZERO) {
		
		// Use the largest available resolution.
		mode = modes.back();
		
	} else {
		
		// Find the next best available fullscreen mode.
		RenderWindow::DisplayModes::const_iterator i;
		i = std::lower_bound(modes.begin(), modes.end(), mode);
		if(i == modes.end()) {
			mode = *modes.rbegin();
		} else {
			mode = *i;
		}
		if(config.video.resolution != mode.resolution || unsigned(config.video.bpp) != mode.depth) {
			LogWarning << "fullscreen mode " << config.video.resolution.x << 'x' << config.video.resolution.y << '@' << config.video.bpp << " not supported, using " << mode.resolution.x << 'x' << mode.resolution.y 	<< '@' << mode.depth << " instead";
		}
		
	}
	
	// Clamp to a sane resolution and window size!
	mode.resolution.x = std::max(mode.resolution.x, s32(640));
	mode.resolution.y = std::max(mode.resolution.y, s32(480));
	config.window.size.x = std::max(config.window.size.x, s32(640));
	config.window.size.y = std::max(config.window.size.y, s32(480));
	
	Vec2i size = config.video.fullscreen ? mode.resolution : config.window.size;
	
	if(!m_MainWindow->initialize(version, size, config.video.fullscreen, mode.depth)) {
		m_MainWindow = NULL;
		return false;
	}
	
	if(GRenderer == NULL) {
		// We could not initialize all resources in onRendererInit().
		m_MainWindow = NULL;
		return false;
	}
	
	if(!m_MainWindow->isFullScreen() && config.video.resolution != Vec2i::ZERO) {
		config.video.resolution = mode.resolution;
	}
	config.video.bpp = mode.depth;
	
	return true;
}

bool ArxGame::initWindow() {
	
	arx_assert(m_MainWindow == NULL);
	
	bool autoFramework = (config.window.framework == "auto");
	
	for(int i = 0; i < 2 && !m_MainWindow; i++) {
		bool first = (i == 0);
		
		bool matched = false;
		
		#ifdef ARX_HAVE_D3D9
		if(!m_MainWindow && first == (autoFramework || config.window.framework == "D3D9")) {
			matched = true;
			RenderWindow * window = new D3D9Window;
			if(!initWindow(window)) {
				delete window;
			}
		}
		#endif

		#ifdef ARX_HAVE_SDL
		if(!m_MainWindow && first == (autoFramework || config.window.framework == "SDL")) {
			matched = true;
			RenderWindow * window = new SDLWindow;
			if(!initWindow(window)) {
				delete window;
			}
		}
		#endif
		
		if(first && !matched) {
			LogError << "unknown windowing framework: " << config.window.framework;
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
	bool init = ARX_INPUT_Init();
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
	
	savegames.update();
	
	return init;
}

static const char * default_paks[][2] = {
	{ "data.pak", NULL },
	{ "loc.pak", "loc_default.pak" },
	{ "data2.pak", NULL },
	{ "sfx.pak", NULL },
	{ "speech.pak", "speech_default.pak" },
};

bool ArxGame::addPaks() {
	
	arx_assert(!resources);
	
	resources = new PakReader;
	
	// Load required pak files
	std::vector<size_t> missing;
	for(size_t i = 0; i < ARRAY_SIZE(default_paks); i++) {
		if(resources->addArchive(fs::paths.find(default_paks[i][0]))) {
			continue;
		}
		if(default_paks[i][1]
		   && resources->addArchive(fs::paths.find(default_paks[i][1]))) {
			continue;
		}
		missing.push_back(i);
	}
	
	// Construct an informative error message about missing files
	if(!missing.empty()) {
		std::ostringstream oss;
		oss << "Could not load required ";
		oss << (missing.size() == 1 ? "file" : "files");
		size_t length = oss.tellp();
		for(size_t i = 0; i < missing.size(); i++) {
			if(i != 0) {
				if(i + 1 == missing.size()) {
					oss << " and", length += 4;
				} else {
					oss << ",", length++;
				}
			}
			size_t add = 1 + strlen(default_paks[missing[i]][0]) + 1;
			if(default_paks[missing[i]][1]) {
				add += 6 + strlen(default_paks[missing[i]][1]) + 2;
			}
			if(length + add > 75) {
				oss << "\n ", length = add + 1;
			} else {
				oss << ' ', length += add + 1;
			}
			oss << '"' << default_paks[missing[i]][0] << '"';
			if(default_paks[missing[i]][1]) {
				oss << " (or \"" << default_paks[missing[i]][1] << "\")";
				length += 3 + strlen(default_paks[missing[i]][1]) + 2;
			}
		}
		oss << ".\n\nSearched in these locations:\n";
		std::vector<fs::path> search = fs::paths.getSearchPaths();
		BOOST_FOREACH(const fs::path & dir, search) {
			oss << " * " << dir.string() << fs::path::dir_sep << "\n";
		}
		oss << "\nSee  " << url::help_get_data;
		oss << "  and  " << url::help_install_data << "\n";
		oss << "\nThe search path can be adjusted with command-line parameters.\n";
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
		GInput->unacquireDevices();
		GInput->acquireDevices();
	}
}

void ArxGame::onWindowLostFocus(const Window &) {
	
	if(GInput) {
		GInput->unacquireDevices();
	}
}

void ArxGame::onResizeWindow(const Window & window) {
	
	arx_assert(window.getSize() != Vec2i::ZERO);
	
	// A new window size will require a new backbuffer
	// size, so the 3D structures must be changed accordingly.
	wasResized = true;
	
	if(window.isFullScreen()) {
		if(config.video.resolution == Vec2i::ZERO) {
			LogInfo << "auto selected fullscreen resolution " << window.getSize().x << 'x' << window.getSize().y << '@' << window.getDepth();
		} else {
			LogInfo << "changed fullscreen resolution to " << window.getSize().x << 'x' << window.getSize().y << '@' << window.getDepth();
			config.video.resolution = window.getSize();
		}
	} else {
		LogInfo << "changed window size to " << window.getSize().x << 'x' << window.getSize().y;
		config.window.size = window.getSize();
	}
}

void ArxGame::onPaintWindow(const Window& window)
{
	ARX_UNUSED(window);
}

void ArxGame::onDestroyWindow(const Window &) {
	
	LogInfo << "Application window is being destroyed";
	quit();
}

void ArxGame::onToggleFullscreen(const Window & window) {
	config.video.fullscreen = window.isFullScreen();
	GInput->reset();
	wasResized = true;
}

//*************************************************************************************
// run()
// Message-processing loop. Idle time is used to render the scene.
//*************************************************************************************
void ArxGame::run() {
	
	beforeRun();
	
	while(m_RunLoop) {
		
		m_MainWindow->tick();
		if(!m_RunLoop) {
			break;
		}
		
		if(m_MainWindow->hasFocus() && m_bReady) {
			doFrame();
			
			// Show the frame on the primary surface.
			m_MainWindow->showFrame();
		}
	}
}

//*************************************************************************************
// doFrame()
// Draws the scene.
//*************************************************************************************
void ArxGame::doFrame() {
		
	updateTime();

	updateInput();

	if(wasResized) {
		LogDebug("was resized");
		wasResized = false;
		DanaeRestoreFullScreen();
	}

	// Manages Splash Screens if needed
	if(DANAE_ManageSplashThings())
		return;

	// Clicked on New Quest ? (TODO:need certainly to be moved somewhere else...)
	if (START_NEW_QUEST)
	{
		LogDebug("start quest");
		DANAE_StartNewQuest();
	}

	// Are we being teleported ?
	if ((TELEPORT_TO_LEVEL[0]) && (CHANGE_LEVEL_ICON==200))
	{
		LogDebug("teleport to " << TELEPORT_TO_LEVEL << " " << TELEPORT_TO_POSITION << " "
					<< TELEPORT_TO_ANGLE);
		CHANGE_LEVEL_ICON=-1;
		ARX_CHANGELEVEL_Change(TELEPORT_TO_LEVEL, TELEPORT_TO_POSITION, TELEPORT_TO_ANGLE);
		memset(TELEPORT_TO_LEVEL,0,64);
		memset(TELEPORT_TO_POSITION,0,64);
	}

	if (LOADQUEST_SLOT != -1) {
		ARX_SlotLoad(LOADQUEST_SLOT);
		LOADQUEST_SLOT = -1;
	}

	if ((PLAY_LOADED_CINEMATIC == 0) && (!CINEMASCOPE) && (!BLOCK_PLAYER_CONTROLS) && (ARXmenu.currentmode == AMCM_OFF))
	{
		if (GInput->actionNowPressed(CONTROLS_CUST_QUICKLOAD)) {
			ARX_QuickLoad();
		}

		if (GInput->actionNowPressed(CONTROLS_CUST_QUICKSAVE)) {
			iTimeToDrawD7=2000;
			GRenderer->getSnapshot(savegame_thumbnail, 160, 100);
			ARX_QuickSave();
		}

		ARX_DrawAfterQuickLoad();
	}
		
	if(FirstFrame) {
		FirstFrameHandling();
	} else {
		update();
		render();
	}
}

//*************************************************************************************
// Cleanup3DEnvironment()
// Cleanup scene objects
//*************************************************************************************
void ArxGame::cleanup3DEnvironment() {
	
	if(getWindow()) {
		finalCleanup();
	}
	
}

//*************************************************************************************
// OutputText()
// Draws text on the window.
//*************************************************************************************
void ArxGame::outputText(int x, int y, const string & str) {
	if (m_bReady) {
		hFontInGame->draw(x, y, str, Color(255, 255, 0));
	}
}

//*************************************************************************************
// OutputText()
// Draws text on the window using selected font and color
// at position defined by column,row.
//*************************************************************************************
void ArxGame::outputTextGrid(float column, float row, const std::string &text, const Color &color)
{
	Font *selected_font = hFontInGame;

	// find display size
	const Vec2i &window = getWindow()->getSize();

	const int tsize = selected_font->getLineHeight();


	// TODO: could use quadrants for width or something similar
	// TODO: could center text in column/row
	const Vector2<int> size(window.x / 4, selected_font->getLineHeight());

	const Vector2<int> spacing(2, 2);
	const Vector2<float> p(column + (column < 0), row + (row < 0));

	// offset text into the screen a bit
	const Vector2<int> offset((column < 0 ? window.x - tsize - size.x : tsize), (row < 0 ? window.y - tsize - size.y : tsize));

	// print the text directly using our selected font
	selected_font->draw(offset + Vector2<int>(p.x * (size + spacing).x, p.y * (size + spacing).y), text, color);

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
	
	EERIE_3DOBJ * _fogobj = LoadTheObj("editor/obj3d/fog_generator.teo", "node_teo maps");
	ARX_FOGS_Set_Object(_fogobj);
	
	eyeballobj = LoadTheObj("editor/obj3d/eyeball.teo", "eyeball_teo maps");
	cabal = LoadTheObj("editor/obj3d/cabal.teo", "cabal_teo maps");
	nodeobj = LoadTheObj("editor/obj3d/node.teo", "node_teo maps");
	
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
	ChangeLevel = TextureContainer::LoadUI("graph/interface/icons/change_lvl");
	
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
		BOW_FOCAL-=Original_framedelay;

		if (BOW_FOCAL<0) BOW_FOCAL=0;
	}

	if(eyeball.exist == 2) {
		subj.d_pos = eyeball.pos;
		subj.d_angle = eyeball.angle;
		EXTERNALVIEW = 1;
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
			EERIEPOLY * ep = EECheckInPoly(&tt);
			if(ep) {
				subj.d_pos = tt;
			}
			else break;
		}

		subj.d_angle.a=player.angle.a+30.f;
		subj.d_angle.b=player.angle.b;
		subj.d_angle.g=player.angle.g;
		
	} else {
	
		subj.angle = player.angle;
		
		if (entities.player()) {
			long id = entities.player()->obj->fastaccess.view_attach;

			if (id!=-1) {
				subj.orgTrans.pos = entities.player()->obj->vertexlist3[id].v;
				Vec3f vect;
				vect.x = subj.orgTrans.pos.x - player.pos.x;
				vect.y = 0;
				vect.z = subj.orgTrans.pos.z - player.pos.z;
				float len = ffsqrt(vect.lengthSqr());

				if (len>46.f) {
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

	if (EXTERNALVIEW) {
		subj.orgTrans.pos = (subj.orgTrans.pos + subj.d_pos) * 0.5f;
		subj.angle = interpolate(subj.angle, subj.d_angle, 0.1f);
	}
}

void ArxGame::updateConversationCamera() {

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
							if (aspeech[k].io==entities[main_conversation.actors[j]])
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

			if(ARX_CONVERSATION_MODE == -1) {
				ARX_CONVERSATION_MODE = 0;
				if(rnd() > 0.5f) {
					conversationcamera.size = Anglef(MAKEANGLE(170.f + rnd() * 20.f), 0.f, 0.f);
					conversationcamera.d_angle = Anglef(0.f, 0.f, 0.08f);
				} else {
					conversationcamera.size = Anglef(rnd() * 50.f, 0.f, rnd() * 50.f);
					conversationcamera.d_angle = Anglef::ZERO;
					if(rnd() > 0.4f) {
						conversationcamera.d_angle.a = (1.f - rnd() * 2.f) * (1.f / 30);
					}
					if(rnd() > 0.4f) {
						conversationcamera.d_angle.b = (1.f - rnd() * 1.2f) * 0.2f;
					}
					if(rnd() > 0.4f) {
						conversationcamera.d_angle.g = (1.f - rnd() * 2.f) * 0.025f;
					}
				}
			} else {
				conversationcamera.size += conversationcamera.d_angle * FrameDiff;
			}

			Vec3f sourcepos,targetpos;

			if(ApplySpeechPos(&conversationcamera, is)) {
				targetpos = conversationcamera.d_pos;
				sourcepos = conversationcamera.orgTrans.pos;
			} else {
				targetpos = player.pos;
				float t=radians(player.angle.b);
				sourcepos.x=targetpos.x+(float)EEsin(t)*100.f;
				sourcepos.y=targetpos.y;
				sourcepos.z=targetpos.z-(float)EEcos(t)*100.f;
			}

			Vec3f vec2;
			Vec3f vect = targetpos - sourcepos;
			fnormalize(vect);
			float dist=250.f-conversationcamera.size.g;

			if (dist<0.f) dist=(90.f-(dist*( 1.0f / 20 )));
			else if (dist<90.f) dist=90.f;

			YRotatePoint(&vect,&vec2,EEcos(radians(conversationcamera.size.a)),EEsin(radians(conversationcamera.size.a)));

			sourcepos = targetpos - vec2 * dist;

			if (conversationcamera.size.b!=0.f)
				sourcepos.y+=120.f-conversationcamera.size.b*( 1.0f / 10 );

			conversationcamera.orgTrans.pos = sourcepos;
			conversationcamera.setTargetCamera(targetpos.x,targetpos.y,targetpos.z);
			subj.orgTrans.pos = conversationcamera.orgTrans.pos;
			subj.angle.a = MAKEANGLE(-conversationcamera.angle.a);
			subj.angle.b = MAKEANGLE(conversationcamera.angle.b - 180.f);
			subj.angle.g = 0.f;
			EXTERNALVIEW = 1;
	}
	else
	{
		ARX_CONVERSATION_MODE=-1;
		ARX_CONVERSATION_LASTIS=-1;

		if (LAST_CONVERSATION)
		{
			AcquireLastAnim(entities.player());
			ANIM_Set(&entities.player()->animlayer[1],entities.player()->anims[ANIM_WAIT]);
			entities.player()->animlayer[1].flags|=EA_LOOP;
		}
	}
}

void ArxGame::speechControlledCinematic() {

	/////////////////////////////////////////////
	// Now checks for speech controlled cinematic

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
		Entity * io=aspeech[valid].io;
		float rtime=(float)(arxtime.get_updated()-aspeech[valid].time_creation)/(float)aspeech[valid].duration;

		if (rtime<0) rtime=0;

		if (rtime>1) rtime=1;

		float itime=1.f-rtime;

		if ((rtime>=0.f) && (rtime<=1.f) && io)
		{
			float alpha,beta,distance,_dist;

			switch (acs->type)
			{
			case ARX_CINE_SPEECH_KEEP: {
				subj.orgTrans.pos = acs->pos1;
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
				conversationcamera.orgTrans.pos.x=-EEsin(radians(MAKEANGLE(io->angle.b+beta)))*distance+targetpos.x;
				conversationcamera.orgTrans.pos.y= EEsin(radians(MAKEANGLE(io->angle.a+alpha)))*distance+targetpos.y;
				conversationcamera.orgTrans.pos.z= EEcos(radians(MAKEANGLE(io->angle.b+beta)))*distance+targetpos.z;
				conversationcamera.setTargetCamera(targetpos.x,targetpos.y,targetpos.z);
				subj.orgTrans.pos = conversationcamera.orgTrans.pos;
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
					conversationcamera.orgTrans.pos.x=targetpos.x+vect2.x;
					conversationcamera.orgTrans.pos.y=targetpos.y+vect2.y+acs->f2;
					conversationcamera.orgTrans.pos.z=targetpos.z+vect2.z;
					conversationcamera.setTargetCamera(targetpos.x,targetpos.y,targetpos.z);
					subj.orgTrans.pos = conversationcamera.orgTrans.pos;
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
					if(acs->type == ARX_CINE_SPEECH_CCCLISTENER_L
						 || acs->type == ARX_CINE_SPEECH_CCCLISTENER_R) {
						conversationcamera.orgTrans.pos = acs->pos2;
						targetpos = acs->pos1;
					} else {
						conversationcamera.orgTrans.pos = acs->pos1;
						targetpos = acs->pos2;
					}

					distance=(acs->startpos*itime+acs->endpos*rtime)*( 1.0f / 100 );

					Vec3f vect = conversationcamera.orgTrans.pos - targetpos;
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

					conversationcamera.orgTrans.pos = vect + targetpos + vect2;
					conversationcamera.setTargetCamera(targetpos.x,targetpos.y,targetpos.z);
					subj.orgTrans.pos = conversationcamera.orgTrans.pos;
					subj.angle.a=MAKEANGLE(-conversationcamera.angle.a);
					subj.angle.b=MAKEANGLE(conversationcamera.angle.b-180.f);
					subj.angle.g=0.f;
					EXTERNALVIEW=1;
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

	if (player.life<=0)
	{
		DeadTime += static_cast<long>(FrameDiff);
		float mdist = EEfabs(player.physics.cyl.height)-60;
		DeadCameraDistance+=(float)FrameDiff*( 1.0f / 80 )*((mdist-DeadCameraDistance)/mdist)*2.f;

		if (DeadCameraDistance>mdist) DeadCameraDistance=mdist;

		Vec3f targetpos = player.pos;

		long id  = entities.player()->obj->fastaccess.view_attach;
		long id2 = GetActionPointIdx( entities.player()->obj, "chest2leggings" );

		if (id!=-1) {
			targetpos = entities.player()->obj->vertexlist3[id].v;
		}

		conversationcamera.orgTrans.pos.x = targetpos.x;
		conversationcamera.orgTrans.pos.y = targetpos.y - DeadCameraDistance;
		conversationcamera.orgTrans.pos.z = targetpos.z;

		if (id2!=-1)
		{
			conversationcamera.orgTrans.pos.x=entities.player()->obj->vertexlist3[id2].v.x;
			conversationcamera.orgTrans.pos.y=entities.player()->obj->vertexlist3[id2].v.y-DeadCameraDistance;
			conversationcamera.orgTrans.pos.z=entities.player()->obj->vertexlist3[id2].v.z;
		}

		conversationcamera.setTargetCamera(targetpos.x,targetpos.y,targetpos.z);
		subj.orgTrans.pos=conversationcamera.orgTrans.pos;
		subj.angle.a=MAKEANGLE(-conversationcamera.angle.a);
		subj.angle.b=MAKEANGLE(conversationcamera.angle.b-180.f);
		subj.angle.g = 0;
		EXTERNALVIEW=1;
		BLOCK_PLAYER_CONTROLS=1;
	}
	else
	{
		DeadCameraDistance=0;
	}
}

void ArxGame::handleCameraController() {

	static float currentbeta = 0.f;

	if (CAMERACONTROLLER!=NULL)
	{
		if (lastCAMERACONTROLLER!=CAMERACONTROLLER)
		{
			currentbeta=CAMERACONTROLLER->angle.b;
		}

		Vec3f targetpos = CAMERACONTROLLER->pos + player.baseOffset();

		float delta_angle = AngleDifference(currentbeta, CAMERACONTROLLER->angle.b);
		float delta_angle_t = delta_angle * FrameDiff * ( 1.0f / 1000 );

		if (EEfabs(delta_angle_t) > EEfabs(delta_angle)) delta_angle_t = delta_angle;

		currentbeta += delta_angle_t;
		float t=radians(MAKEANGLE(currentbeta));
		conversationcamera.orgTrans.pos.x=targetpos.x+(float)EEsin(t)*160.f;
		conversationcamera.orgTrans.pos.y=targetpos.y+40.f;
		conversationcamera.orgTrans.pos.z=targetpos.z-(float)EEcos(t)*160.f;

		conversationcamera.setTargetCamera(targetpos.x,targetpos.y,targetpos.z);
		subj.orgTrans.pos = conversationcamera.orgTrans.pos;
		subj.angle.a=MAKEANGLE(-conversationcamera.angle.a);
		subj.angle.b=MAKEANGLE(conversationcamera.angle.b-180.f);
		subj.angle.g=0.f;
		EXTERNALVIEW=1;
	}

	lastCAMERACONTROLLER=CAMERACONTROLLER;
}

void ArxGame::updateActiveCamera() {

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

	ManageQuakeFX();

	// Prepare ActiveCamera
	PrepareCamera(ACTIVECAM);
	// Recenter Viewport depending on Resolution

	ACTIVECAM->center = Vec2i(DANAECENTERX, DANAECENTERY);
	ACTIVECAM->orgTrans.mod = ACTIVECAM->center.to<float>();
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

	// TODO eliminate FrameDiff == framedelay (replace)
	FrameDiff = framedelay;
}

void ArxGame::updateInput() {

	// Update input
	GInput->update();
	ReMappDanaeButton();
	AdjustMousePosition();

	if(GInput->actionNowPressed(CONTROLS_CUST_TOGGLE_FULLSCREEN)) {
		setFullscreen(!getWindow()->isFullScreen());
	}

	if (GInput->isKeyPressedNowPressed(Keyboard::Key_F12))
	{
		EERIE_PORTAL_ReleaseOnlyVertexBuffer();
		ComputePortalVertexBuffer();
	}

	if(GInput->isKeyPressedNowPressed(Keyboard::Key_F11)) {
		showFPS = !showFPS;
	}

	if(GInput->isKeyPressedNowPressed(Keyboard::Key_F10)) {
		GetSnapShot();
	}

	if (GInput->isKeyPressedNowPressed(Keyboard::Key_Spacebar)) {
		CAMERACONTROLLER=NULL;
	}
}

bool ArxGame::isInMenu() const {
	return ARXmenu.currentmode != AMCM_OFF;
}

void ArxGame::renderMenu() {
	
	GRenderer->SetRenderState(Renderer::Fog, false);
	
	ARX_Menu_Render();

	GRenderer->SetRenderState(Renderer::Fog, true);
	GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat); // << NEEDED?
}

bool ArxGame::isInCinematic() const {
	return PLAY_LOADED_CINEMATIC && ControlCinematique && ControlCinematique->projectload;
}

void ArxGame::renderCinematic() {

	DANAE_Manage_Cinematic();
}

void ArxGame::renderLevel() {

	if (!PLAYER_PARALYSED)
	{
		if (manageEditorControls()) goto finish;
	}

	if ((!BLOCK_PLAYER_CONTROLS) && (!PLAYER_PARALYSED))
	{
		managePlayerControls();
	}

	ARX_PLAYER_Manage_Movement();

	ARX_PLAYER_Manage_Visual();

	if (FRAME_COUNT<=0)
		ARX_MINIMAP_ValidatePlayerPos();

	// SUBJECTIVE VIEW UPDATE START  *********************************************************

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
	//               DRAW CINEMASCOPE 16/9
	if(CINEMA_DECAL != 0.f) {
		Rect rectz[2];
		rectz[0].left = rectz[1].left = 0;
		rectz[0].right = rectz[1].right = DANAESIZX;
		rectz[0].top = 0;
		long lMulResult = checked_range_cast<long>(CINEMA_DECAL * Yratio);
		rectz[0].bottom = lMulResult;
		rectz[1].top = DANAESIZY - lMulResult;
		rectz[1].bottom = DANAESIZY;
		GRenderer->Clear(Renderer::ColorBuffer | Renderer::DepthBuffer, Color::none, 0.0f, 2, rectz);
	}
	//-------------------------------------------------------------------------------

	GRenderer->BeginScene();

	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);

	if ( (entities.player()) && (entities.player()->animlayer[0].cur_anim) )
	{
		ManageNONCombatModeAnimations();
		long old=USEINTERNORM;
		USEINTERNORM=0;

		float speedfactor = entities.player()->basespeed + entities.player()->speed_modif;
		if(cur_mr==3)
			speedfactor+=0.5f;

		if(cur_rf==3)
			speedfactor+=1.5f;

		if(speedfactor < 0)
			speedfactor = 0;

		long tFrameDiff = Original_framedelay;

		float restore=ACTIVECAM->orgTrans.use_focal;
		if(!EXTERNALVIEW && !BOW_FOCAL)
			ACTIVECAM->orgTrans.use_focal = PLAYER_ARMS_FOCAL * Xratio;

		if((player.Interface & INTER_COMBATMODE) && STRIKE_TIME) { // need some precision for weapon...
			float cur=0;

			while(cur < tFrameDiff && !(entities.player()->ioflags & IO_FREEZESCRIPT)) {
				long step = min(50L, tFrameDiff);
				if(entities.player()->ioflags & IO_FREEZESCRIPT)
					step=0;

				float iCalc = step * speedfactor ;

				arx_assert(entities.player()->obj != NULL);
				EERIEDrawAnimQuat(entities.player()->obj, &entities.player()->animlayer[0], &entities.player()->angle,
					&entities.player()->pos, checked_range_cast<unsigned long>(iCalc), entities.player(), false, false);

				if ((player.Interface & INTER_COMBATMODE) && (entities.player()->animlayer[1].cur_anim != NULL))
					ManageCombatModeAnimations();

				if (entities.player()->animlayer[1].cur_anim!=NULL)
					ManageCombatModeAnimationsEND();

				cur+=step*speedfactor;
			}
		} else {
			float val = (float)tFrameDiff * speedfactor;

			if (entities.player()->ioflags & IO_FREEZESCRIPT)
				val=0;

			arx_assert(entities.player()->obj != NULL);
			EERIEDrawAnimQuat(entities.player()->obj, &entities.player()->animlayer[0], &entities.player()->angle,
				&entities.player()->pos, checked_range_cast<unsigned long>(val), entities.player(), false, false);

			if ((player.Interface & INTER_COMBATMODE) && (entities.player()->animlayer[1].cur_anim != NULL))
				ManageCombatModeAnimations();

			if (entities.player()->animlayer[1].cur_anim!=NULL)
				ManageCombatModeAnimationsEND();
		}
		ACTIVECAM->orgTrans.use_focal = restore;
		USEINTERNORM = old;
	}


	updateFirstPersonCamera();
	updateConversationCamera();

	////////////////////////
	// Checks SCRIPT TIMERS.
	ARX_SCRIPT_Timer_Check();

	speechControlledCinematic();

	handlePlayerDeath();


	/////////////////////////////////////
	LAST_CONVERSATION=ARX_CONVERSATION;

	handleCameraController();

	UpdateCameras();

	///////////////////////////////////////////
	ARX_PLAYER_FrameCheck(Original_framedelay);

	updateActiveCamera();

	ARX_GLOBALMODS_Apply();

	if (EDITMODE) GRenderer->SetRenderState(Renderer::Fog, false);

	// Set Listener Position
	{
		float t = radians(MAKEANGLE(ACTIVECAM->angle.b));
		Vec3f front(-EEsin(t), 0.f, EEcos(t));
		front.normalize();

		//TODO Hardcoded up vector
		Vec3f up(0.f, 1.f, 0.f);
		ARX_SOUND_SetListener(&ACTIVECAM->orgTrans.pos, &front, &up);
	}

	// Reset Transparent Polys Idx
	TRANSPOLYSPOS=0;

	// Check For Hiding/unHiding Player Gore
	if ((EXTERNALVIEW) || (player.life<=0))
	{
		ARX_INTERACTIVE_Show_Hide_1st(entities.player(),0);
	}

	if (!EXTERNALVIEW)
	{
		ARX_INTERACTIVE_Show_Hide_1st(entities.player(),1);
	}

	// NOW DRAW the player (Really...)
	if(entities.player() && entities.player()->animlayer[0].cur_anim) {
		float restore=ACTIVECAM->orgTrans.use_focal;

		if(!EXTERNALVIEW) {
			if(!BOW_FOCAL)
				ACTIVECAM->orgTrans.use_focal=PLAYER_ARMS_FOCAL*Xratio;

			FORCE_FRONT_DRAW=1;
		}

		if (entities.player()->invisibility>0.9f) entities.player()->invisibility=0.9f;

		arx_assert(entities.player()->obj != NULL);
		EERIEDrawAnimQuat(entities.player()->obj, &entities.player()->animlayer[0], &entities.player()->angle,
			&entities.player()->pos, 0, entities.player());

		ACTIVECAM->orgTrans.use_focal=restore;
		FORCE_FRONT_DRAW=0;
	}

	// SUBJECTIVE VIEW UPDATE START  *********************************************************
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::DepthTest, true);

	PrepareIOTreatZone();
	ARX_PHYSICS_Apply();

	if (FRAME_COUNT<=0)
			PrecalcIOLighting(&ACTIVECAM->orgTrans.pos, ACTIVECAM->cdepth * 0.6f);

	ACTIVECAM->fadecolor = current.depthcolor;

	if (uw_mode)
	{
		float val=10.f;
		GRenderer->GetTextureStage(0)->SetMipMapLODBias(val);
		ARX_SCENE_Render();
		val=-0.3f;
		GRenderer->GetTextureStage(0)->SetMipMapLODBias(val);
	} else {
		ARX_SCENE_Render();
	}

	// Begin Particles
	
	if(pParticleManager) {
		pParticleManager->Update(static_cast<long>(FrameDiff));
		pParticleManager->Render();
	}
	
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	ARX_FOGS_Render();
	
	ARX_PARTICLES_Render(&subj);
	UpdateObjFx();
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	
	// End Particles

	if (!EDITMODE) // Playing Game
	{
		// Checks Magic Flares Drawing
		if(!PLAYER_PARALYSED) {
			if(EERIEMouseButton & 1) {
				if(ARX_FLARES_Block == 0) {
					ARX_SPELLS_AddPoint(DANAEMouse);
				} else {
					CurrPoint = 0;
					ARX_FLARES_Block = 0;
				}
			} else if(ARX_FLARES_Block == 0) {
				ARX_FLARES_Block = 1;
			}
		}
		
		ARX_SPELLS_Precast_Check();
		ARX_SPELLS_ManageMagic();
		ARX_SPELLS_UpdateSymbolDraw();

		ManageTorch();

		// Renders Magical Flares
		if ( !((player.Interface & INTER_MAP )
			&& (!(player.Interface & INTER_COMBATMODE)))
			&& flarenum
			)
		{
			ARX_MAGICAL_FLARES_Draw(FRAMETICKS);
			FRAMETICKS = (unsigned long)(arxtime);
		}
	}
#ifdef BUILD_EDITOR
	else  // EDITMODE == true
	{
		RenderAllNodes();

		std::stringstream ss("EDIT MODE - Selected ");
		ss <<  NbIOSelected;
		ARX_TEXT_Draw(hFontInBook, 100, 2, ss.str(), Color::yellow);

		if (EDITION==EDITION_FOGS)
			ARX_FOGS_RenderAll();
	}
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
			static const float INC_FOCAL = 75.0f;
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

	// Red screen fade for damages.
	ARX_DAMAGE_Show_Hit_Blood();

	// Manage Notes/Books opened on screen
	GRenderer->SetRenderState(Renderer::Fog, false);

finish:

	// Update spells
	ARX_SPELLS_Update();
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::Fog, true);

	// Manage Death visual & Launch menu...
	if (DeadTime>2000)
		ARX_PLAYER_Manage_Death();

	// INTERFACE
	// Remove the Alphablend State if needed : NO Z Clear
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetRenderState(Renderer::Fog, false);

	// Draw game interface if needed
	if(ARXmenu.currentmode == AMCM_OFF && !CINEMASCOPE) {
	
		GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapClamp);
		GRenderer->SetRenderState(Renderer::DepthTest, false);
		
		ARX_INTERFACE_NoteManage();
		drawAllInterface();
		drawAllInterfaceFinish();

		if((player.Interface & INTER_MAP) && !(player.Interface & INTER_COMBATMODE)
       && flarenum) {
			ARX_MAGICAL_FLARES_Draw(FRAMETICKS);
			FRAMETICKS = (unsigned long)(arxtime);
		}
		
		GRenderer->SetRenderState(Renderer::DepthTest, true);
	}

	GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	PopAllTriangleList();
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	PopAllTriangleListTransparency();
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);

	GRenderer->SetRenderState(Renderer::Fog, true);
	goFor2DFX();
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

	if(SHOW_INGAME_MINIMAP
		&& (PLAY_LOADED_CINEMATIC == 0) 
		&& (!CINEMASCOPE) 
		&& (!BLOCK_PLAYER_CONTROLS) 
		&& !(player.Interface & INTER_MAP))
	{
		long SHOWLEVEL = ARX_LEVELS_GetRealNum(CURRENTLEVEL);

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

	if (sp_max_start)
		Manage_sp_max();

	GRenderer->EndScene();
}

void ArxGame::update() {
	
	if(!WILL_LAUNCH_CINE.empty()) {
		// A cinematic is waiting to be played...
		LaunchWaitingCine();
	}
}

void ArxGame::render() {
	
	ACTIVECAM = &subj;

	// Update Various Player Infos for this frame.
	ARX_PLAYER_Frame_Update();
		
	subj.center = Vec2i(DANAECENTERX, DANAECENTERY);
	subj.orgTrans.mod = subj.center.to<float>();

	// Finally computes current focal
	BASE_FOCAL=(float)CURRENT_BASE_FOCAL+(BOW_FOCAL*( 1.0f / 4 ));

	// SPECIFIC code for Snapshot MODE... to insure constant capture framerate

	PULSATE=EEsin(arxtime.get_frame_time() / 800);
	EERIEDrawnPolys=0;

	// EditMode Specific code
	if (EDITMODE)
	{
		TOTIOPDL=0;
		BLOCK_PLAYER_CONTROLS=0;
	}

	// Checks for Keyboard & Moulinex
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
					bookclick = DANAEMouse;
				}
			}
			else if (InSecondaryInventoryPos(&DANAEMouse))
				ARX_MOUSE_OVER|=ARX_MOUSE_OVER_INVENTORY_2;
			else if (InPlayerInventoryPos(&DANAEMouse))
				ARX_MOUSE_OVER|=ARX_MOUSE_OVER_INVENTORY;
		}

		if ( (player.Interface & INTER_COMBATMODE)
			|| (PLAYER_MOUSELOOK_ON) )
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

		if ( (!PLAYER_PARALYSED)
			|| (ARXmenu.currentmode != AMCM_OFF) )

		{
			if (!STOP_KEYBOARD_INPUT)
				manageKeyMouse();
			else
			{
				STOP_KEYBOARD_INPUT++;

				if (STOP_KEYBOARD_INPUT>2) STOP_KEYBOARD_INPUT=0;
			}
		}
	}

	if(CheckInPolyPrecis(player.pos.x,player.pos.y,player.pos.z)) {
		LastValidPlayerPos = player.pos;
	}

	// Updates Externalview
	EXTERNALVIEW = 0;

	if(isInMenu()) {
		renderMenu();
	} else if(isInCinematic()) {
		renderCinematic();
	} else {
		renderLevel();
	}
	
	if(showFPS) {
		GRenderer->BeginScene();
		CalcFPS();
		ShowFPS();
		GRenderer->EndScene();
	}
	
	if (LaunchDemo)
	{
		LaunchDemo=0;
		LaunchDummyParticle();
	}
	
	if (ARXmenu.currentmode == AMCM_OFF)
	{
		ARX_SCRIPT_AllowInterScriptExec();
		ARX_SCRIPT_EventStackExecute();
		// Updates Damages Spheres
		ARX_DAMAGES_UpdateAll();
		ARX_MISSILES_Update();

		ARX_PATH_UpdateAllZoneInOutInside();
	}

	arxtime.update_last_frame_time();
	LastMouseClick=EERIEMouseButton;
}

void EE_RT(TexturedVertex * in, Vec3f * out);

void ArxGame::goFor2DFX()
{
	TexturedVertex lv,ltvv;

	long needed = 0;

	for (long i=0;i<TOTPDL;i++)
	{
		EERIE_LIGHT * el=PDL[i];

		if (el->extras & EXTRAS_FLARE)
		{
			if(distSqr(ACTIVECAM->orgTrans.pos, el->pos) < square(2200)) {
				needed=1;
				break;
			}
		}
	}

	if (!needed) return;

					{
		Entity* pTableIO[256];
		int nNbInTableIO = 0;

		float temp_increase=framedelay*( 1.0f / 1000 )*4.f;
		{
			bool bComputeIO = false;

			for (int i=0;i<TOTPDL;i++)
			{
				EERIE_LIGHT * el=PDL[i];

				long lPosx=(long)(float)(el->pos.x*ACTIVEBKG->Xmul);
				long lPosz=(long)(float)(el->pos.z*ACTIVEBKG->Zmul);

				if( (lPosx<0)||
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
					lv.p = el->pos;
					specialEE_RTP(&lv,&ltvv);
					el->temp-=temp_increase;

					if (!(player.Interface & INTER_COMBATMODE)
						&& (player.Interface & INTER_MAP))
						continue;

					if ((ltvv.rhw > 0.f) &&
						(ltvv.p.x>0.f) &&
						(ltvv.p.y>(CINEMA_DECAL*Yratio)) &&
						(ltvv.p.x<DANAESIZX) &&
						(ltvv.p.y<(DANAESIZY-(CINEMA_DECAL*Yratio)))
						)
					{
						Vec3f vector = lv.p - ACTIVECAM->orgTrans.pos;
						lv.p -= vector * (50.f / vector.length());
						TexturedVertex ltvv2;
						specialEE_RTP(&lv, &ltvv2);

						float fZFar=ProjectionMatrix._33*(1.f/(ACTIVECAM->cdepth*fZFogEnd))+ProjectionMatrix._43;

						Vec3f hit;
						EERIEPOLY *tp=NULL;
						Vec2s ees2dlv;
						Vec3f ee3dlv = lv.p;

						ees2dlv.x = checked_range_cast<short>(ltvv.p.x);
						ees2dlv.y = checked_range_cast<short>(ltvv.p.y);


						if( !bComputeIO )
						{
							GetFirstInterAtPos(&ees2dlv, 2, &ee3dlv, pTableIO, &nNbInTableIO );
							bComputeIO = true;
						}

						if(
							(ltvv.p.z>fZFar)||
							EERIELaunchRay3(&ACTIVECAM->orgTrans.pos,&ee3dlv,&hit,tp,1)||
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
						lv.p = el->pos;
						lv.rhw = 1.f;
						EE_RT(&lv, &ltvv.p);
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

						EERIEDrawSprite(&lv, siz, tflare, (el->rgb * v).to<u8>(), ltvv.p.z);

					}
				}
			}

			GRenderer->SetRenderState(Renderer::DepthTest, true);
		}
	}

	GRenderer->SetRenderState(Renderer::DepthWrite, true);
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

void ArxGame::onRendererInit(RenderWindow & window) {
	
	arx_assert(GRenderer == NULL);
	
	GRenderer = window.getRenderer();
	
	if(GRenderer->GetTextureStageCount() < 3) {
		LogError << "Arx Libertatis needs at least 3 texture units,"
		         << " but only " << GRenderer->GetTextureStageCount() << " are available";
		GRenderer = NULL;
		return;
	}
	
	initDeviceObjects();
	
	// The app is ready to go
	m_bReady = true;
}

void ArxGame::onRendererShutdown(RenderWindow &) {
	
	if(!GRenderer) {
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
