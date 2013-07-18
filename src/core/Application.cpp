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
//   Sébastien Scieux (Zbuffer)
//   Didier Pedreno  (ScreenSaver Problem Fix)
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved

#include "core/Application.h"

#include <stddef.h>
#include <algorithm>
#include <set>
#include <sstream>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/foreach.hpp>

#include "core/Config.h"
#include "core/GameTime.h"

#include "graphics/Renderer.h"

#include "io/fs/FilePath.h"
#include "io/fs/Filesystem.h"
#include "io/fs/SystemPaths.h"
#include "io/log/Logger.h"

#include "math/Random.h"

#include "platform/Platform.h"

#include "window/RenderWindow.h"

using std::max;
using std::string;

//-----------------------------------------------------------------------------
long EERIEMouseButton = 0;
long EERIEMouseGrab = 0;

Application * mainApp = 0;
float FPS;
LightMode ModeLight = 0;
ViewModeFlags ViewMode = 0;

static int iCurrZBias;

//*************************************************************************************
// Application()
// Constructor
//*************************************************************************************
Application::Application() : m_MainWindow(NULL) {
	m_bReady = true;
	m_RunLoop = true;
}

Application::~Application() {
		
}

bool Application::initialize() {
	
	bool init;
	
	init = initConfig();
	if(!init) {
		LogCritical << "Failed to initialize the config subsystem.";
		return false;
	}
	
	init = initWindow();
	if(!init) {
		LogCritical << "Failed to initialize the windowing subsystem.";
		return false;
	}
	
	init = initInput();
	if(!init) {
		LogCritical << "Failed to initialize the input subsystem.";
		return false;
	}
	
	init = initSound();
	if(!init) {
		LogCritical << "Failed to initialize the sound subsystem.";
		return false;
	}
	
	Random::seed();
	
	return true;
}

void Application::shutdown() {
	delete m_MainWindow, m_MainWindow = NULL;
}

void Application::quit() {
	m_RunLoop = false;
}

static bool migrateFilenames(fs::path path, bool is_dir) {
	
	string name = path.filename();
	string lowercase = boost::to_lower_copy(name);
	
	bool migrated = true;
	
	if(lowercase != name) {
		
		fs::path dst = path.parent() / lowercase;
		
		LogInfo << "Renaming " << path << " to " << dst.filename() << "";
		
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
	
	std::set<string> fileset;
	for(size_t i = 0; i < ARRAY_SIZE(files); i++) {
		fileset.insert(files[i]);
	}
	
	bool migrated = true;
	
	for(fs::directory_iterator it(fs::paths.user); !it.end(); ++it) {
		string file = it.name();
		if(fileset.find(boost::to_lower_copy(file)) != fileset.end()) {
			migrated &= migrateFilenames(fs::paths.user / file, it.is_directory());
		}
	}
	
	if(!migrated) {
		LogCritical << "Could not rename all files to lowercase, please do so manually and set migration=1 under [misc] in " << configFile;
	}
	
	return migrated;
}

bool Application::initConfig() {
	
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
		if(config.init(file)) {
			LogWarning << "Could not read config files cfg.ini and cfg_default.ini,"
			           << " using defaults.";
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

//*************************************************************************************
// Pause()
// Called in to toggle the pause state of the app. This function
// brings the GDI surface to the front of the display, so drawing
// output like message boxes and menus may be displayed.
//*************************************************************************************
void Application::pause(bool bPause)
{
	static u32 dwAppPausedCount = 0L;

	dwAppPausedCount += (bPause ? +1 : -1);
	m_bReady          = (dwAppPausedCount ? false : true);

	// Handle the first pause request (of many, nestable pause requests)
	if (bPause && (1 == dwAppPausedCount))
	{
		// Get a surface for the GDI
		//if (m_pFramework)
		//	m_pFramework->FlipToGDISurface(true); TODO
	}
}

//*************************************************************************************
//*************************************************************************************
void CalcFPS(bool reset)
{
	static float fFPS      = 0.0f;
	static float fLastTime = 0.0f;
	static u32 dwFrames  = 0L;

	if (reset)
	{
		dwFrames = 0;
		fLastTime = 0.f;
		FPS = fFPS = 7.f * FPS;
	}
	else
	{
		float tmp;

		// Keep track of the time lapse and frame count
		float fTime = arxtime.get_updated(false) * 0.001f;   // Get current time in seconds
		++dwFrames;

		tmp = fTime - fLastTime;

		// Update the frame rate once per second
		if (tmp > 1.f)
		{
			FPS = fFPS      = dwFrames / tmp ;
			fLastTime = fTime;
			dwFrames  = 0L;
		}
	}
}

void SetZBias(int _iZBias)
{
	if (_iZBias < 0)
	{
		_iZBias = 0;
		_iZBias = max(iCurrZBias, -_iZBias);
	}

	if (_iZBias == iCurrZBias) 
		return;

	iCurrZBias = _iZBias;

	GRenderer->SetDepthBias(iCurrZBias);
}
