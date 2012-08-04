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

#include "core/Startup.h"

#include <stddef.h>
#include <string>
#include <vector>
#include <iostream>
#include <set>
#include <sstream>
#include <algorithm>

#if ARX_COMPILER_MSVC
	#pragma warning(push)
	#pragma warning(disable:4512)
#endif
#include <boost/program_options.hpp>
#if ARX_COMPILER_MSVC
	#pragma warning(pop)
#endif

#include "core/Config.h"
#include "io/fs/SystemPaths.h"
#include "io/log/Logger.h"
#include "platform/CrashHandler.h"
#include "platform/Environment.h"
#include "platform/String.h"


namespace po = boost::program_options;


#if ARX_PLATFORM != ARX_PLATFORM_WIN32
bool parseCommandLine(int argc, char ** argv) {
	
	std::string command_line;
	for(int i = 1; i < argc; i++) {
		command_line += argv[i];
		command_line += ' ';
	}
	
#else
bool parseCommandLine(const char * command_line) {
#endif
	
	CrashHandler::setVariable("Command line", command_line);
	
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
		
#if ARX_PLATFORM != ARX_PLATFORM_WIN32
		po::store(po::parse_command_line(argc, argv, options_desc), options);
#else
		std::vector<std::string> args = po::split_winmain(command_line);
		po::store(po::command_line_parser(args).options(options_desc).run(),
		          options);
#endif
		
		po::notify(options);
		
		if(options.count("help")) {
			std::cout << options_desc << std::endl;
			return false;
		}
		
		po::variables_map::const_iterator debug = options.find("debug");
		if(debug != options.end()) {
			Logger::configure(debug->second.as<std::string>());
		}
		
		defineSystemDirectories();
		
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
			return false;
		} else if(fs::paths.user.empty()) {
			LogCritical << "Could not create user directory.";
			return false;
		}
		
	} catch(po::error & e) {
		std::cerr << "Error parsing command-line: " << e.what() << "\n\n";
		std::cout << options_desc << std::endl;
		return false;
	}
	
	return true;
}
