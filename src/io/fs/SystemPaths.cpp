/*
 * Copyright 2012-2013 Arx Libertatis Team (see the AUTHORS file)
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

#include "io/fs/SystemPaths.h"

#include <string>
#include <set>
#include <algorithm>
#include <iterator>
#include <iostream>

#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>

#include "io/fs/Filesystem.h"
#include "io/fs/PathConstants.h"
#include "io/log/Logger.h"

#include "platform/Environment.h"
#include "platform/ProgramOptions.h"

namespace fs {

SystemPaths paths;

static path canonical(const fs::path & path) {
	return path.is_absolute() ? path : current_path() / path;
}

static std::vector<path> getSearchPaths(const char * input) {
	
	std::vector<path> result;
	
	if(input) {
		std::string decoded = platform::expandEnvironmentVariables(input);
		typedef boost::tokenizer< boost::char_separator<char> >  tokenizer;
		boost::char_separator<char> sep(platform::env_list_seperators);
		tokenizer tokens(decoded, sep);
		std::copy(tokens.begin(), tokens.end(), std::back_inserter(result));
	} else {
		result.push_back(".");
	}
	
	return result;
};

static path findUserPath(const char * name, const path & force,
                         const char * registry, const char * prefix,
                         const char * suffix, const path & fallback,
                         bool create) {
	
	// Prefer command-line options
	if(!force.empty()) {
		path dir = canonical(force);
		if(create && !create_directories(dir)) {
			LogCritical << "Could not create " << name << " directory " << dir << '.';
			return path();
		} else {
			LogDebug("using " << name << " dir from command-line: " << dir);
			return dir;
		}
	}
	
	// Check system settings (windows registry)
	std::string temp;
	if(registry && platform::getSystemConfiguration(registry, temp)) {
		path dir = canonical(temp);
		if(!create) {
			return dir;
		} else if(create_directories(dir)) {
			LogDebug("got " << name << " dir from registry: \"" << temp
			         << "\" = " << dir);
			return dir;
		} else {
			LogError << "Could not create " << name << " directory " << dir << '.';
			LogDebug("ignoring " << name << " dir from registry: \"" << temp << '"');
		}
	}
	
	// Search standard locations
	bool create_exists = false;
	path to_create;
	std::vector<path> prefixes = getSearchPaths(prefix);
	std::vector<path> suffixes = getSearchPaths(suffix);
	if(prefix || suffix) {
		BOOST_FOREACH(const path & prefix, prefixes) {
			BOOST_FOREACH(const path & suffix, suffixes) {
				
				path dir = canonical(prefix / suffix);
				
				if(is_directory(dir)) {
					LogDebug("got " << name << " dir from search: " << prefix
					         << " + " << suffix << " = " << dir);
					return dir;
				} else {
					LogDebug("ignoring " << name << " dir from search: " << prefix
					         << " + " << suffix << " = " << dir);
				}
				
				if(to_create.empty() || (!create_exists && is_directory(prefix))) {
					to_create = dir;
					create_exists = is_directory(prefix);
				}
			}
		}
	}
	
	// Create a new standard diretcory
	if(!to_create.empty()) {
		if(!create) {
			return to_create;
		} else if(!create_directories(to_create)) {
			LogError << "Could not create " << name << " directory " << to_create << '.';
		} else {
			LogInfo << "Created new " << name << " directory at " << to_create << '.';
			return to_create;
		}
	}
	
	LogDebug("using fallback " << name << " dir");
	return fallback;
}

static bool addSearchPath(std::vector<path> & result, const path & dir,
                          bool filter) {
	
	if(filter && !is_directory(dir)) {
		return false;
	}
	
	if(std::find(result.begin(), result.end(), dir) != result.end()) {
		return false;
	}
	
	result.push_back(dir);
	return true;
}

std::vector<path> SystemPaths::getSearchPaths(bool filter) const {
	
	std::vector<path> result;
	
	// Use the user diretcory as the highest-priority data directory
	if(!user.empty()) {
		result.push_back(user);
	}
	
	// Use paths specifed on the command-line
	BOOST_FOREACH(const path & dir, addData_) {
		path tmp = canonical(dir);
		if(addSearchPath(result, tmp, filter)) {
			LogDebug("got data dir from command-line: " << dir << " = " << tmp);
		}
	}
	
	// Skip search for system paths if requested
	if(!findData_) {
		return result;
	}
	
	// Check paths specified in environment variables
	path exepath = platform::getExecutablePath();
	#if ARX_PLATFORM != ARX_PLATFORM_WIN32
	if(!exepath.empty()) {
		std::string var = "${" + exepath.basename() + "_PATH}";
		std::vector<path> paths = fs::getSearchPaths(var.c_str());
		BOOST_FOREACH(const path & p, paths) {
			addSearchPath(result, p, filter);
		}
	}
	#endif
	
	// Check system settings (windows registry)
	std::string temp;
	if(platform::getSystemConfiguration("DataDir", temp)) {
		path dir = canonical(temp);
		if(addSearchPath(result, dir, filter)) {
			LogDebug("got data dir from registry: \"" << temp << "\" = " << dir);
		} else {
			LogDebug("ignoring data dir from registry: \"" << temp << "\" = " << dir);
		}
	}
	
	// Search the executable directory
	if(!exepath.empty()) {
		path dir = canonical(exepath.parent());
		bool ignored = false;
		if(ignore_exe_dir) {
			std::vector<path> ignored_dirs = fs::getSearchPaths(ignore_exe_dir);
			ignored = (std::find(ignored_dirs.begin(), ignored_dirs.end(), dir)
			           != ignored_dirs.end());
		}
		if(!ignored && addSearchPath(result, dir, filter)) {
			LogDebug("got data dir from exe: " << exepath << " -> " << dir);
		} else {
			LogDebug("ignoring data dir from exe: " << exepath << " -> " << dir);
		}
	}
	
	// Search standard locations
	std::vector<path> prefixes = fs::getSearchPaths(data_dir_prefixes);
	std::vector<path> suffixes = fs::getSearchPaths(data_dir);
	if(data_dir_prefixes || data_dir) {
		BOOST_FOREACH(const path & prefix, prefixes) {
			BOOST_FOREACH(const path & suffix, suffixes) {
				path dir = canonical(prefix / suffix);
				if(addSearchPath(result, dir, filter)) {
					LogDebug("got data dir from search: " << prefix
					         << " + " << suffix << " = " << dir);
				} else {
					LogDebug("ignoring data dir from search: " << prefix
					         << " + " << suffix << " = " << dir);
				}
			}
		}
	}
	
	return result;
}

static SystemPaths::InitParams cmdLineInitParams;

ExitStatus SystemPaths::init() {
	return init(cmdLineInitParams);
}

ExitStatus SystemPaths::init(const InitParams& initParams) {

	user = findUserPath("user", initParams.forceUser, "UserDir",
	                    user_dir_prefixes, user_dir, current_path(), !initParams.displaySearchDirs);

	config = findUserPath("config", initParams.forceConfig, "ConfigDir",
	                      config_dir_prefixes, config_dir, user, !initParams.displaySearchDirs);

	addData_ = initParams.dataDirs;

	findData_ = initParams.findData;
	
	data = getSearchPaths(true);

	if(initParams.displaySearchDirs) {
		fs::paths.list(std::cout, 
		               " - --user-dir (-u) command-line parameter\n",
		               " - --config-dir (-c) command-line parameter\n",
		               " - --data-dir (-d) command-line parameters\n"
		               " only without --no-data-dir (-n): \n");
		std::cout << std::endl;
		return ExitSuccess;
	} else if(fs::paths.user.empty() || fs::paths.config.empty()) {
		LogCritical << "Could not select user or config directory.";
		return ExitFailure;
	}

	return RunProgram;
}

path SystemPaths::find(const path & resource) const {
	
	if(resource.is_absolute()) {
		return exists(resource) ? resource : path();
	}
	
	BOOST_FOREACH(const path & prefix, data) {
		path full_path = prefix / resource;
		if(fs::exists(full_path)) {
			return full_path;
		}
	}
	
	return path();
}

static void listDirectoriesFor(std::ostream & os, const std::string & regKey,
                               const char * prefix = NULL,
                               const char * suffix = NULL) {
	
#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	if(!regKey.empty()) {
		os << " - Registry key {HKCU,HKLM}\\Software\\ArxLibertatis\\"
		   << regKey << '\n';
		std::string temp;
		if(platform::getSystemConfiguration(regKey, temp)) {
			os << "   = " << canonical(temp) << '\n';
		}
	}
#else
	ARX_UNUSED(regKey);
#endif
	
	std::vector<path> prefixes = getSearchPaths(prefix);
	std::vector<path> suffixes = getSearchPaths(suffix);
	if(prefix || suffix) {
		
		os << " - ";
		if(prefix) {
			os << '"' << prefix << '"';
		}
		if(prefix && suffix) {
			os << " / ";
		}
		if(suffix) {
			os << '"' << suffix << '"';
		}
		os << ":\n";
		
		std::set<path> result;
		
		BOOST_FOREACH(const path & prefix, prefixes) {
			BOOST_FOREACH(const path & suffix, suffixes) {
				path dir = canonical(prefix / suffix);
				if(result.find(dir) == result.end()) {
					os << "  -> " << dir << '\n';
					result.insert(dir);
				}
			}
		}
	}
	
}

void SystemPaths::list(std::ostream & os, const std::string & forceUser,
                       const std::string & forceConfig,
                       const std::string & forceData) {
	
	os << "\nUser directories (select first existing):\n";
	if(!forceUser.empty()) {
		os << forceUser;
	}
	listDirectoriesFor(os, "UserDir", user_dir_prefixes, user_dir);
	os << " - Current working directory\n";
	os << "selected: ";
	if(user.empty()) {
		os << "(none)\n";
	} else {
		os << user << '\n';
	}
	
	os << "\nConfig directories (select first existing):\n";
	if(!forceConfig.empty()) {
		os << forceConfig;
	}
	listDirectoriesFor(os, std::string(), config_dir_prefixes, config_dir);
	os << " - The selected user directory\n";
	os << "selected: ";
	if(config.empty()) {
		os << "(none)\n";
	} else {
		os << config << '\n';
	}
	
	os << "\nData directories (decreasing priority):\n";
	os << " - The selected user directory\n";
	if(!forceData.empty()) {
		os << forceData;
	}
	path exepath = platform::getExecutablePath();
	os << " - Paths specifed in ${" << exepath.basename() << "_PATH}\n";
	os << " - The directory containing the game executable\n";
	listDirectoriesFor(os, "DataDir", data_dir_prefixes, data_dir);
	os << "selected:";
	if(data.empty()) {
		os << " (none)\n";
	} else {
		os << '\n';
		BOOST_FOREACH(const path & dir, data) {
			os << " - " << dir << '\n';
		}
	}
}

} // namespace fs

void disableDataDir() {
	fs::cmdLineInitParams.findData = false;
}

void addDataDir(const std::string & dataDir) {
	fs::cmdLineInitParams.dataDirs.push_back(dataDir);
}

void setUserDir(const std::string & userDir) {
	fs::cmdLineInitParams.forceUser = userDir;
}

void setConfigDir(const std::string & configDir) {
	fs::cmdLineInitParams.forceConfig = configDir;
}

void listSearchDirs() {
	fs::cmdLineInitParams.displaySearchDirs = true;
}

ARX_PROGRAM_OPTION("no-data-dir", "n",
                   "Don't automatically detect data directories", &disableDataDir);
ARX_PROGRAM_OPTION("data-dir",    "d",
                   "Where to find the data files (can be repeated)", &addDataDir, "DIR");
ARX_PROGRAM_OPTION("user-dir",    "u",
                   "Where to store user-specific files", &setUserDir, "DIR");
ARX_PROGRAM_OPTION("config-dir",  "c",
                   "Where to store config files", &setConfigDir, "DIR");
ARX_PROGRAM_OPTION("list-dirs",   "l",
                   "List the searched user and data directories", &listSearchDirs);
