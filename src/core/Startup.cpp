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


#include <stddef.h>
#include <string>
#include <vector>
#include <iostream>
#include <set>
#include <sstream>
#include <algorithm>

#include "platform/Platform.h"

#if ARX_PLATFORM == ARX_PLATFORM_WIN32
#include <windows.h>
#include <stdlib.h>
#endif

#if ARX_COMPILER_MSVC
	#pragma warning(push)
	#pragma warning(disable:4512)
#endif
#include <boost/program_options.hpp>
#if ARX_COMPILER_MSVC
	#pragma warning(pop)
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
#include "platform/CrashHandler.h"
#include "platform/Environment.h"
#include "platform/String.h"
#include "platform/Time.h"


namespace po = boost::program_options;

namespace {

enum ExitStatus {
	ExitSuccess,
	ExitFailure,
	RunProgram
};

} // anonymous namespace

static ExitStatus parseCommandLine(int argc, char ** argv) {
	
	std::string userDir;
	std::string configDir;
	std::vector<std::string> dataDirs;
	
	po::options_description options_desc("Arx Libertatis Options");
	options_desc.add_options()
		("help,h", "Show supported options")
		("no-data-dir,n", "Don't automatically detect data directories")
		("data-dir,d", po::value< std::vector<std::string> >(&dataDirs),
		               "Where to find the data files (can be repeated)")
		("user-dir,u", po::value<std::string>(&userDir),
		               "Where to store user-specific files")
		("config-dir,c", po::value<std::string>(&configDir),
		                 "Where to store config files")
		("debug,g", po::value<std::string>(), "Log level settings")
		("list-dirs,l", "List the searched user and data directories")
	;
	
	po::variables_map options;
	
	try {
		
		const char * argv0 = argv[0];
		po::store(po::parse_command_line(argc, argv, options_desc), options);
		
		po::notify(options);
		
		if(options.count("help")) {
			std::cout << options_desc << std::endl;
			return ExitSuccess;
		}
		
		po::variables_map::const_iterator debug = options.find("debug");
		if(debug != options.end()) {
			Logger::configure(debug->second.as<std::string>());
		}
		
		defineSystemDirectories(argv0);
		
		bool findData = (options.count("no-data-dir") == 0);
		bool listDirs = (options.count("list-dirs") != 0);
		
		std::vector<fs::path> addDirs;
		addDirs.resize(dataDirs.size());
		std::copy(dataDirs.begin(), dataDirs.end(), addDirs.begin());
		
		fs::paths.init(userDir, configDir, addDirs, findData, !listDirs);
		
		if(listDirs) {
			fs::paths.list(std::cout, " - --user-dir (-u) command-line parameter\n",
			               " - --config-dir (-c) command-line parameter\n",
			               " - --data-dir (-d) command-line parameters\n"
			               " only without --no-data-dir (-n): \n");
			std::cout << std::endl;
			return ExitSuccess;
		} else if(fs::paths.user.empty() || fs::paths.config.empty()) {
			LogCritical << "Could not select user or config directory.";
			return ExitFailure;
		}
		
	} catch(po::error & e) {
		std::cerr << "Error parsing command-line: " << e.what() << "\n\n";
		std::cout << options_desc << std::endl;
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
			command_line += argv[i]; // TODO escape
			command_line += ' ';
		}
		CrashHandler::setVariable("Command line", command_line);
	}
	
	// Also intialize the logging system early as we might need it
	Logger::initialize();
	CrashHandler::registerCrashCallback(Logger::quickShutdown);
	Logger::add(new logger::CriticalErrorDialog);
	
	ExitStatus status = parseCommandLine(argc, argv);
	
	// Parse the command-line and setup user, config and data directories
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
		LogInfo << "Starting " << version;
		runGame();
		
	}
	
	// Shutdown the logging system
	// If there has been a critical error, a dialog will be shown now
	Logger::shutdown();
	
	CrashHandler::shutdown();
	
	return (status == ExitFailure) ? EXIT_FAILURE : EXIT_SUCCESS;
}
