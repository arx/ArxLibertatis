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

#if ARX_COMPILER_MSVC
	#pragma warning(push)
	#pragma warning(disable:4512)
#endif
#include <boost/program_options.hpp>
#if ARX_COMPILER_MSVC
	#pragma warning(pop)
#endif

#include "core/Config.h"
#include "io/fs/FilePath.h"
#include "io/fs/Filesystem.h"
#include "io/log/Logger.h"
#include "platform/CrashHandler.h"
#include "platform/Environment.h"
#include "platform/String.h"

#include "Configure.h"

namespace po = boost::program_options;

using std::string;

static fs::path findSubdirectory(const std::string & where, const fs::path & dir,
                                 fs::path * to_create = NULL) {
	
	string prefixes = expandEnvironmentVariables(where);
	
	bool create_exists = false;
	
	size_t start = 0;
	while(true) {
		size_t end = prefixes.find(env_list_seperator, start);
		fs::path prefix = prefixes.substr(start, (end == string::npos) ? end : (end - start));
		
		fs::path subdir = prefix / dir;
		if(fs::is_directory(subdir)) {
			return subdir;
		}
		
		if(to_create) {
			if(to_create->empty() || (!create_exists && fs::is_directory(prefix))) {
				*to_create = subdir;
				create_exists = fs::is_directory(prefix);
			}
		}
		
		if(end == string::npos) {
			break;
		} else {
			start = end + 1;
		}
	}
	
	return fs::path();
}

static void findDataDirectory() {
	
	config.paths.data.clear();
	
	string temp;
	if(getSystemConfiguration("DataDir", temp)) {
		config.paths.data = temp;
		LogDebug("Got data directory from registry: " << config.paths.data);
		return;
	}
	
#ifdef DATA_DIR
	
	fs::path dir = expandEnvironmentVariables(DATA_DIR);
	
#ifdef DATA_DIR_PREFIXES 
	if(dir.is_relative()) {
		config.paths.data = findSubdirectory(DATA_DIR_PREFIXES, dir);
		if(!config.paths.data.empty()) {
			LogDebug("Got data directory from DATA_DIR_PREFIXES: " << config.paths.data);
			return;
		}
	}
#endif // DATA_DIR_PREFIXES
	
	if(fs::is_directory(dir)) {
		config.paths.data = dir;
		LogDebug("Got data directory from DATA_DIR: " << config.paths.data);
		return;
	}
	
#endif // DATA_DIR
	
	LogDebug("No data directory found.");
}

static bool findUserDirectory() {
	
	config.paths.user.clear();
	
	string temp;
	if(getSystemConfiguration("UserDir", temp)) {
		config.paths.user = temp;
		LogDebug("Got user directory from registry: " << config.paths.user);
		return true;
	}
	
#ifdef USER_DIR
	
	fs::path dir = expandEnvironmentVariables(USER_DIR);
	
	fs::path to_create;
#ifdef USER_DIR_PREFIXES
	if(dir.is_relative()) {
		config.paths.user = findSubdirectory(USER_DIR_PREFIXES, dir, &to_create);
		if(!config.paths.user.empty()) {
			LogDebug("Got user directory from USER_DIR_PREFIXES: " << config.paths.user);
			return true;
		}
	}
#endif // USER_DIR_PREFIXES
	
	if(fs::is_directory(dir)) {
		config.paths.user = dir;
		LogDebug("Got user directory from USER_DIR: " << config.paths.user);
		return true;
	}
	
	// Create a new user directory.
	if(!config.paths.data.empty()) {
		if(!to_create.empty()) {
			config.paths.user = to_create;
			LogDebug("Selected new user directory from USER_DIR_PREFIXES: " << config.paths.user);
		} else {
			config.paths.user = dir;
			LogDebug("Selected new user directory from USER_DIR: " << config.paths.user);
		}
		return true;
	}
	
#endif // USER_DIR
	
	// Use the current directory for both data and config files.
	config.paths.user = ".";
	LogDebug("Using working directory as user directory: " << config.paths.user);
	return false;
}

static void findConfigDirectory(bool create) {
	
	config.paths.config.clear();
	
#ifdef CONFIG_DIR
	
	fs::path dir = expandEnvironmentVariables(CONFIG_DIR);
	
	fs::path to_create;
#ifdef CONFIG_DIR_PREFIXES
	if(dir.is_relative()) {
		config.paths.config = findSubdirectory(CONFIG_DIR_PREFIXES, dir, &to_create);
		if(!config.paths.config.empty()) {
			LogDebug("Got config directory from CONFIG_DIR_PREFIXES: " << config.paths.config);
			return;
		}
	}
#endif // USER_DIR_PREFIXES
	
	if(fs::is_directory(dir)) {
		config.paths.config = dir;
		LogDebug("Got config directory from CONFIG_DIR: " << config.paths.config);
		return;
	}
	
	// Create a new config directory.
	if(!config.paths.data.empty() || create) {
		if(!to_create.empty()) {
			config.paths.config = to_create;
			LogDebug("Selected new config directory from CONFIG_DIR_PREFIXES: "
			         << config.paths.config);
		} else {
			config.paths.config = dir;
			LogDebug("Selected new config directory from CONFIG_DIR: " << config.paths.config);
		}
		return;
	}
	
#else
	ARX_UNUSED(create);
#endif // CONFIG_DIR
	
	// Use the user directory as the config directory.
	config.paths.config = config.paths.user;
	LogDebug("Using user directory as config directory: " << config.paths.config);
}

static void listDirectoriesFor(std::ostream & os, const string & regKey,
                               const string & suffix = string(),
                               const string & where = string()) {
	
#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	if(!regKey.empty()) {
		os << " - Registry key {HKCU,HKLM}\\Software\\ArxLibertatis\\" << regKey << '\n';
		string temp;
		if(getSystemConfiguration(regKey, temp)) {
			os << "   = " << fs::path(temp) << '\n';
		}
	}
#else
	ARX_UNUSED(regKey);
#endif
	
	if(suffix.empty()) {
		return;
	}
	fs::path dir = expandEnvironmentVariables(suffix);
	
	if(!where.empty() && dir.is_relative()) {
		
		string prefixes = expandEnvironmentVariables(where);
		
		os << " - \"" << suffix << '"';
		if(dir.string() != suffix) {
			os << " = " << fs::path(dir);
		}
		os << " in one of \"" << where << '"';
		if(prefixes != where) {
			os << "\n    = \"" << prefixes << '"';
		}
		os << ":\n";
		
		std::set<fs::path> prefixset;
		
		size_t start = 0;
		while(true) {
			size_t end = prefixes.find(env_list_seperator, start);
			fs::path prefix = prefixes.substr(start, (end == string::npos) ? end : (end - start));
			if(prefixset.find(prefix) == prefixset.end()) {
				prefixset.insert(prefix);
				os << "  * " << (prefix / dir) << '\n';
			}
			if(end == string::npos) {
				break;
			} else {
				start = end + 1;
			}
		}
	}
	
	os << " - \"" << suffix << '"';
	if(dir.string() != suffix) {
		os << " = " << fs::path(dir);
	}
	os << '\n';
	
}

static void listDirectories(std::ostream & os, bool data, bool user, bool cfg) {
	
	if(data) {
		os << "\nData directories (data files):\n";
		os << " - --data-dir (-d) command-line parameter\n";
#ifdef DATA_DIR
# ifdef DATA_DIR_PREFIXES 
		listDirectoriesFor(os, "DataDir", DATA_DIR, DATA_DIR_PREFIXES);
# else
		listDirectoriesFor(os, "DataDir", DATA_DIR);
# endif // DATA_DIR_PREFIXES
#else // DATA_DIR
		listDirectoriesFor(os, "DataDir");
#endif // DATA_DIR
		os << "selected: ";
		if(config.paths.data.empty()) {
			os << "(none)\n";
		} else {
			os << config.paths.data << '\n';
		}
	}
	
	if(user) {
		os << "\nUser directories (save files, data files):\n";
		os << " - --user-dir (-u) command-line parameter\n";
#ifdef USER_DIR
# ifdef USER_DIR_PREFIXES 
		listDirectoriesFor(os, "UserDir", USER_DIR, USER_DIR_PREFIXES);
# else
		listDirectoriesFor(os, "UserDir", USER_DIR);
# endif // USER_DIR_PREFIXES
#else // USER_DIR
		listDirectoriesFor(os, "UserDir");
#endif // USER_DIR
		os << " - Current working directory\n";
		os << "selected: ";
		if(config.paths.user.empty()) {
			os << "(none)\n";
		} else {
			os << config.paths.user << '\n';
		}
	}
	
	if(cfg) {
		os << "\nConfig directories:\n";
		os << " - --config-dir (-c) command-line parameter\n";
#ifdef CONFIG_DIR
# ifdef CONFIG_DIR_PREFIXES 
		listDirectoriesFor(os, std::string(), CONFIG_DIR, CONFIG_DIR_PREFIXES);
# else
		listDirectoriesFor(os, std::string(), CONFIG_DIR);
# endif // USER_DIR_PREFIXES
#endif // CONFIG_DIR
		os << " - The selected user directory\n";
		os << "selected: ";
		if(config.paths.config.empty()) {
			os << "(none)\n";
		} else {
			os << config.paths.config << '\n';
		}
	}
	os << '\n';
}

static bool findMainDataFile() {
	
	fs::path data_file = "data.pak";
	
	if(fs::is_regular_file(config.paths.data / data_file)) {
		return true;
	}
	
	if(fs::is_regular_file(config.paths.user / data_file)) {
		return true;
	}
	
	// In case we have not run migration yet, also check for different casings.
	if(!config.paths.user.empty() && fs::is_directory(config.paths.user)) {
		for(fs::directory_iterator it(config.paths.user); !it.end(); ++it) {
			if(toLowercase(it.name()) == data_file.string()) {
				return true;
			}
		}
	}
	
	std::ostringstream oss;
	oss << "Could not find " << data_file << " in either the data or user directory:\n";
	listDirectories(oss, true, true, false);
	
	LogCritical << oss.str();
	
	return false;
}

bool createUserAndConfigDirectory() {
	
	if(!findMainDataFile()) {
		return false;
	}
	
	if(config.paths.user.empty() || config.paths.config.empty()) {
		LogCritical << "No user / config directory available.";
		return false;
	}
	
	if(!fs::is_directory(config.paths.user)) {
		if(!fs::create_directories(config.paths.user)) {
			LogCritical << "Error creating user directory at " << config.paths.user;
			return false;
		}
		LogInfo << "Created new user directory at " << config.paths.user;
	}
	
	if(!fs::is_directory(config.paths.config)) {
		if(!fs::create_directories(config.paths.config)) {
			LogCritical << "Error creating config directory at " << config.paths.config;
			return false;
		}
		LogInfo << "Created new config directory at " << config.paths.config;
	}
	
	if(config.paths.data == config.paths.user) {
		config.paths.data.clear();
	}

	return true;
}

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

	po::options_description options_desc("Arx Libertatis Options");
	options_desc.add_options()
		("help,h", "Show supported options.")
		("no-data-dir,n", "Don't automatically detect a data directory.")
		("data-dir,d", po::value<string>(), "Where to find the data files.")
		("user-dir,u", po::value<string>(), "Where to store save files.")
		("config-dir,c", po::value<string>(), "Where to store config files.")
		("debug,g", po::value<string>(), "Log level settings.")
		("list-dirs,l", "List the searched user and data directories.")
	;
	
	po::variables_map options;
	
	try {
		
#if ARX_PLATFORM != ARX_PLATFORM_WIN32
		po::store(po::parse_command_line(argc, argv, options_desc), options);
#else
		std::vector<string> args = po::split_winmain(command_line);
		po::store(po::command_line_parser(args).options(options_desc).run(), options);
#endif
		
		po::notify(options);
		
		if(options.count("help")) {
			std::cout << options_desc << std::endl;
			return false;
		}
		
		po::variables_map::const_iterator debug = options.find("debug");
		if(debug != options.end()) {
			Logger::configure(debug->second.as<string>());
		}
		
		defineSystemDirectories();
		
		po::variables_map::const_iterator data_dir = options.find("data-dir");
		if(data_dir != options.end()) {
			config.paths.data = data_dir->second.as<string>();
			LogDebug("Got data directory from command-line: " << config.paths.data);
		} else if(options.count("no-data-dir")) {
			LogDebug("Disabled data directory.");
		} else {
			findDataDirectory();
		}
		if(!config.paths.data.empty() && !fs::is_directory(config.paths.data)) {
			LogWarning << "Data directory " << config.paths.data << " does not exist.";
			config.paths.data.clear();
		}
		
		bool hasProperUserDir = true;
		po::variables_map::const_iterator user_dir = options.find("user-dir");
		if(user_dir != options.end()) {
			config.paths.user = user_dir->second.as<string>();
			LogDebug("Got user directory from command-line: " << config.paths.user);
		} else {
			hasProperUserDir = findUserDirectory();
		}
		
		po::variables_map::const_iterator config_dir = options.find("config-dir");
		if(config_dir != options.end()) {
			config.paths.config = config_dir->second.as<string>();
			LogDebug("Got config directory from command-line: " << config.paths.config);
		} else {
			findConfigDirectory(hasProperUserDir);
		}
		
		if(options.count("list-dirs")) {
			listDirectories(std::cout, true, true, true);
			return false;
		}
		
	} catch(po::error & e) {
		std::cerr << "Error parsing command-line: " << e.what() << "\n\n";
		std::cout << options_desc << std::endl;
		return false;
	}

	return true;
}
