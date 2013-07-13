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

/*
 * Under OS X we want SDLmain to replace the entry point with its own.
 * This is needed to initialize NSApplication - otherwise we will later
 * crash when trying to use SDL windowing functions.
 */
#if defined(__APPLE__) && defined(__MACH__)
	#include <SDL_main.h>
#else
	#undef main /* in case SDL.h was already included */
#endif

#include "core/Config.h"
#include "core/Core.h"
#include "core/Version.h"
#include "io/fs/Filesystem.h"
#include "io/fs/SystemPaths.h"
#include "io/log/CriticalLogger.h"
#include "io/log/FileLogger.h"
#include "io/log/Logger.h"
#include "math/Random.h"
#include "platform/Compiler.h"
#include "platform/CrashHandler.h"
#include "platform/Environment.h"
#include "platform/ProgramOptions.h"
#include "platform/Time.h"
#include "util/String.h"
#include "util/cmdline/Parser.h"

static void showCommandLineHelp(const util::cmdline::interpreter<std::string> & options,
                                std::ostream & os) {
	
	os << "Usage: arx [options]\n\n";
	
	os << "Arx Libertatis Options:\n";
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
	
	defineSystemDirectories(argv[0]);
	
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

#if ARX_PLATFORM != ARX_PLATFORM_WIN32
extern int main(int argc, char ** argv) {
#else
// For windows we can't use main() in a GUI application
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine,
                   INT nCmdShow) {
	ARX_UNUSED(hInstance);
	ARX_UNUSED(hPrevInstance);
	ARX_UNUSED(lpCmdLine);
	ARX_UNUSED(nCmdShow);
	int argc = __argc;
	char ** argv = __argv;
#endif
	
	// Initialize Random now so that the crash handler can use it
	Random::seed();
	
	// Initialize the crash handler
	{
		CrashHandler::initialize();
		CrashHandler::setVariable("Compiler", ARX_COMPILER_VERNAME);
		CrashHandler::setVariable("Boost version", BOOST_LIB_VERSION);
		std::string command_line;
		for(int i = 1; i < argc; i++) {
			command_line += util::escapeString(argv[i], "\\\" '$!");
			command_line += ' ';
		}
		CrashHandler::setVariable("Command line", command_line);
	}
	
	// Also intialize the logging system early as we might need it
	Logger::initialize();
	CrashHandler::registerCrashCallback(Logger::quickShutdown);
	Logger::add(new logger::CriticalErrorDialog);
	
	// Parse the command line and process options
	ExitStatus status = parseCommandLine(argc, argv);
	
	// Setup user, config and data directories
	if(status == RunProgram) {
		status = fs::paths.init();
	}
	
	if(status == RunProgram) {
		
		// Configure the crash report location
		CrashHandler::setReportLocation(fs::paths.user / "crashes");
		CrashHandler::deleteOldReports(/* nb to keep = */5);
		
		// Now that data directories are initialized, create a log file
		{
			fs::path logFile = fs::paths.user / "arx.log";
			Logger::add(new logger::File(logFile));
			CrashHandler::addAttachedFile(logFile);
		}
		
		Time::init();
		
		// 14: Start the game already!
		LogInfo << "Starting " << arx_version;
		runGame();
		
	}
	
	// Shutdown the logging system
	// If there has been a critical error, a dialog will be shown now
	Logger::shutdown();
	
	CrashHandler::shutdown();
	
	return (status == ExitFailure) ? EXIT_FAILURE : EXIT_SUCCESS;
}
