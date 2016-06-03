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


#include <stddef.h>
#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <set>
#include <sstream>
#include <algorithm>
#include <cstdlib>

#include "platform/Platform.h"

#if ARX_PLATFORM == ARX_PLATFORM_WIN32
#include <windows.h>
#include <stdlib.h>
#endif

#include <boost/foreach.hpp>
#include <boost/version.hpp>

#include <glm/glm.hpp>

#include <zlib.h>

#include "core/Benchmark.h"
#include "core/Config.h"
#include "core/Core.h"
#include "core/Version.h"

#include "gui/Credits.h"

#include "io/fs/Filesystem.h"
#include "io/fs/SystemPaths.h"
#include "io/log/CriticalLogger.h"
#include "io/log/FileLogger.h"
#include "io/log/Logger.h"

#include "math/Random.h"

#include "platform/Compiler.h"
#include "platform/CrashHandler.h"
#include "platform/Environment.h"
#include "platform/profiler/Profiler.h"
#include "platform/ProgramOptions.h"
#include "platform/Time.h"
#include "platform/WindowsMain.h"

#include "util/String.h"
#include "util/cmdline/Parser.h"

/*
 * Under OS X we want SDLmain to replace the entry point with its own.
 * This is needed to initialize NSApplication - otherwise we will later
 * crash when trying to use SDL windowing functions.
 */
#if ARX_PLATFORM == ARX_PLATFORM_MACOSX
	#include <SDL_main.h>
#else
	#undef main /* in case SDL.h was already included */
#endif


static void showCommandLineHelp(const util::cmdline::interpreter<std::string> & options,
                                std::ostream & os) {
	
	os << "Usage: arx [options]\n\n";
	
	os << arx_name << " Options:\n";
	os << options << std::endl;
	
}

static void handleHelpOption() {
	
	// Register all program options in the command line interpreter
	util::cmdline::interpreter<std::string> cli;
	BaseOption::registerAll(cli);
	
	showCommandLineHelp(cli, std::cout);
	
	std::exit(EXIT_SUCCESS);
}

ARX_PROGRAM_OPTION("help", "h", "Show supported options", &handleHelpOption);

static ExitStatus parseCommandLine(int argc, char ** argv) {
	
	platform::initializeEnvironment(argv[0]);
	
	// Register all program options in the command line interpreter
	util::cmdline::interpreter<std::string> cli;
	BaseOption::registerAll(cli);
	
	try {
		
		util::cmdline::parse(cli, argc, argv);
		
	} catch(util::cmdline::error & e) {
		
		std::cerr << e.what() << "\n\n";
		showCommandLineHelp(cli, std::cerr);
		
		return ExitFailure;
	}
	
	return RunProgram;
}

int utf8_main(int argc, char ** argv) {
	
	// Initialize Random now so that the crash handler can use it
	Random::seed();
	
	// Initialize the crash handler
	{
		CrashHandler::initialize(argc, argv);
		std::string command_line;
		for(int i = 1; i < argc; i++) {
			command_line += util::escapeString(argv[i], "\\\" '$!");
			command_line += ' ';
		}
		CrashHandler::setVariable("Command line", command_line);
	}
	
	{
		std::ostringstream oss;
		oss << ARX_COMPILER_VERNAME;
		CrashHandler::setVariable("Compiler", oss.str());
		credits::setLibraryCredits("compiler", oss.str());
		CrashHandler::setVariable("CMake", cmake_version);
		credits::setLibraryCredits("build", "CMake " + cmake_version);
		oss.str(std::string());
		oss << (BOOST_VERSION / 100000) << '.' << (BOOST_VERSION / 100 % 1000)
		    << '.' << (BOOST_VERSION % 100);
		CrashHandler::setVariable("Boost version", oss.str());
		credits::setLibraryCredits("boost", "Boost " + oss.str());
		oss.str(std::string());
		oss << GLM_VERSION_MAJOR << '.' << GLM_VERSION_MINOR << '.' << GLM_VERSION_PATCH
		    << '.' << GLM_VERSION_REVISION;
		CrashHandler::setVariable("GLM version", oss.str());
		credits::setLibraryCredits("math", "GLM " + oss.str());
		oss.str(std::string());
		oss << zlibVersion();
		CrashHandler::setVariable("zlib version (headers)", ZLIB_VERSION);
		CrashHandler::setVariable("zlib version (runtime)", oss.str());
		credits::setLibraryCredits("deflate", "zlib " + oss.str());
		credits::setLibraryCredits("image", "stb_image");
	}
	
	// Also intialize the logging system early as we might need it
	Logger::initialize();
	CrashHandler::registerCrashCallback(Logger::quickShutdown);
	Logger::add(new logger::CriticalErrorDialog);
	
	// Parse the command line and process options
	ExitStatus status = parseCommandLine(argc, argv);
	
	platform::initializeTime();
	benchmark::begin(benchmark::Startup);
	
	// Setup user, config and data directories
	if(status == RunProgram) {
		status = fs::paths.init();
	}
	
	if(status == RunProgram) {
		
		// Configure the crash report location
		CrashHandler::setReportLocation(fs::paths.user / "crashes");
		CrashHandler::deleteOldReports(/* nb to keep = */1);
		
		// Now that data directories are initialized, create a log file
		{
			fs::path logFile = fs::paths.user / "arx.log";
			Logger::add(new logger::File(logFile));
			CrashHandler::addAttachedFile(logFile);
		}
		
		profiler::initialize();
		
		// 14: Start the game already!
		LogInfo << "Starting " << arx_name << ' ' << arx_version;
		runGame();
		
	}
	
	benchmark::shutdown();
	
	// Shutdown the logging system
	// If there has been a critical error, a dialog will be shown now
	Logger::shutdown();
	
	CrashHandler::shutdown();
	
	return (status == ExitFailure) ? EXIT_FAILURE : EXIT_SUCCESS;
}
