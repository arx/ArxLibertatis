/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "gui/Console.h"
#include "gui/Credits.h"

#include "io/fs/Filesystem.h"
#include "io/fs/SystemPaths.h"
#include "io/log/CriticalLogger.h"
#include "io/log/FileLogger.h"
#include "io/log/Logger.h"

#include "math/Random.h"

#include "platform/Compiler.h"
#include "platform/CrashHandler.h"
#include "platform/profiler/Profiler.h"
#include "platform/Thread.h"
#include "platform/Time.h"
#include "platform/WindowsMain.h"

#include "util/String.h"
#include "util/cmdline/CommandLine.h"

/*
 * Under macOS we want SDLmain to replace the entry point with its own.
 * This is needed to initialize NSApplication - otherwise we will later
 * crash when trying to use SDL windowing functions.
 */
#if ARX_PLATFORM == ARX_PLATFORM_MACOS
	#include <SDL_main.h>
#else
	#undef main /* in case SDL.h was already included */
#endif

int utf8_main(int argc, char ** argv) {
	
	// GCC -ffast-math disables denormal before main() - do the same for other compilers
	Thread::disableFloatDenormals();
	
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
		oss.str(std::string());
		oss << ARX_STDLIB_VERNAME;
		CrashHandler::setVariable("stdlib", oss.str());
		credits::setLibraryCredits("stdlib", oss.str());
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
		status = fs::initSystemPaths();
	}
	
	if(status == RunProgram) {
		
		// Configure the crash report location
		CrashHandler::setReportLocation(fs::getUserDir() / "crashes");
		CrashHandler::deleteOldReports(/* nb to keep = */1);
		
		// Now that data directories are initialized, create a log file
		{
			fs::path logFile = fs::getUserDir() / "arx.log";
			Logger::add(new logger::File(logFile));
			CrashHandler::addAttachedFile(logFile);
		}
		
		Logger::add(new MemoryLogger(&g_console.buffer()));
		
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
	
	Random::shutdown();
	
	return (status == ExitFailure) ? EXIT_FAILURE : EXIT_SUCCESS;
}
