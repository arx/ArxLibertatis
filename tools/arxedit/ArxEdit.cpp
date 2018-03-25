/*
 * Copyright 2018 Arx Libertatis Team (see the AUTHORS file)
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

#include <QApplication>

#include <stdio.h>
#include <fstream>
#include <iostream>

#include "io/fs/SystemPaths.h"
#include "io/log/Logger.h"
#include "io/log/FileLogger.h"

#include "platform/Environment.h"
#include "platform/Platform.h"
#include "platform/ProgramOptions.h"
#include "platform/WindowsMain.h"
#include "platform/Thread.h"

#include "math/Random.h"

#include "util/cmdline/Parser.h"

#include "ui/ArxEditWindow.h"

int utf8_main(int argc, char ** argv) {
	
	Q_INIT_RESOURCE(ArxEdit);
	
	Thread::disableFloatDenormals();
	Random::seed();
	Logger::initialize();
	platform::initializeEnvironment(argv[0]);
	
	// Register all program options in the command line interpreter
	util::cmdline::interpreter<std::string> cli;
	BaseOption::registerAll(cli);
	
	try {
		util::cmdline::parse(cli, argc, argv);
	} catch(util::cmdline::error & e) {
		std::ostream & os = std::cerr;
		
		os << e.what() << "\n\n";
		os << "Usage: arxedit [options]\n\n";
		os << "Arx Libertatis Editor Options:\n";
		os << cli << std::endl;
		return 1;
	}
	
	fs::initSystemPaths();
	
	QApplication app(argc, argv);
	
	ArxMainWindow w;
	w.show();
	
	return app.exec();
}

