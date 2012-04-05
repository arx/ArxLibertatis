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

#include <QtGui/QApplication>

#include <stdio.h>
#include <fstream>

#include "platform/Platform.h"
#if ARX_PLATFORM == ARX_PLATFORM_WIN32
#include <tchar.h>
#endif

#include "core/Application.h"
#include "core/Config.h"
#include "core/Startup.h"

#include "io/fs/FilePath.h"
#include "io/fs/Filesystem.h"
#include "io/log/Logger.h"
#include "io/log/FileLogger.h"
#include "io/resource/PakReader.h"

#include "arxedit/ArxMainWindow.h"

static const char * default_paks[][2] = {
	{ "data.pak", NULL },
	{ "loc.pak", "loc_default.pak" },
	{ "data2.pak", NULL },
	{ "sfx.pak", NULL },
	{ "speech.pak", "speech_default.pak" },
};

static void add_paks(const fs::path & base, bool * found) {
	
	for(size_t i = 0; i < ARRAY_SIZE(default_paks); i++) {
		if(resources->addArchive(base / default_paks[i][0])) {
			found[i] = true;
		} else if(default_paks[i][1] && resources->addArchive(base / default_paks[i][1])) {
			found[i] = true;
		}
	}
	
	resources->addFiles(base / "editor", "editor");
	resources->addFiles(base / "game", "game");
	resources->addFiles(base / "graph", "graph");
	resources->addFiles(base / "localisation", "localisation");
	resources->addFiles(base / "misc", "misc");
	resources->addFiles(base / "sfx", "sfx");
	resources->addFiles(base / "speech", "speech");
}


bool addPaks() {
	resources = new PakReader;
	
	bool found[ARRAY_SIZE(default_paks)];
	std::fill_n(found, ARRAY_SIZE(default_paks), false);
	
	if(!config.paths.data.empty()) {
		add_paks(config.paths.data, found);
	}
	
	add_paks(config.paths.user, found);
	
	for(size_t i = 0; i < ARRAY_SIZE(default_paks); i++) {
		if(!found[i]) {
			if(config.paths.data.empty()) {
				if(default_paks[i][1]) {
					LogError << "Unable to find " << default_paks[i][0] << " or " << default_paks[i][1]
								<< " in " << config.paths.user;
				} else {
					LogError << "Unable to find " << default_paks[i][0] << " in " << config.paths.user;
				}
			} else {
				if(default_paks[i][1]) {
					LogError << "Unable to find " << default_paks[i][0] << " or " << default_paks[i][1]
								<< " in either " << config.paths.data << " or " << config.paths.user;
				} else {
					LogError << "Unable to find " << default_paks[i][0]
								<< " in either " << config.paths.data << " or " << config.paths.user;
				}
			}
			return false;
		}
	}
	
	return true;
}

#if defined(_WIN32) || defined(_WIN64)
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
	ARX_UNUSED(hInstance);
	ARX_UNUSED(hPrevInstance);
	ARX_UNUSED(nCmdShow);
	Q_INIT_RESOURCE(ArxEdit);
	QApplication app(__argc, __argv);
	
#else
int main(int argc, char **argv) {
	Q_INIT_RESOURCE(ArxEdit);
	QCoreApplication::setAttribute(Qt::AA_X11InitThreads);
	QApplication app(argc, argv);
#endif
	
	Logger::init();
	
#if ARX_PLATFORM != ARX_PLATFORM_WIN32
	parseCommandLine(argc, argv);
#else
	parseCommandLine(lpCmdLine);
#endif
	
	// Initialize config first, before anything else.
	fs::path configFile = config.paths.config / "cfg.ini";
	
	config.setOutputFile(configFile);
	config.init(configFile);
	
	// Also initialize the logging system early as we might need it.
	Logger::configure(config.misc.debug);
	
	// Now that data directories are initialized, create a log file.
	Logger::add(new logger::File(config.paths.user / "arxedit.log"));
	
	addPaks();
	
	ArxMainWindow w;
	w.initScene();
	w.show();
	
	return app.exec();
}

