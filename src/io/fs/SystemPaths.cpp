/*
 * Copyright 2012-2016 Arx Libertatis Team (see the AUTHORS file)
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

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>

#include "io/fs/Filesystem.h"
#include "io/fs/FileStream.h"
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
	}
	
	return result;
}

static path findUserPath(const char * name, const path & force,
                         const char * registry, platform::SystemPathId systemPathId,
                         const char * prefix, const char * suffix,
                         const path & fallback, bool create) {
	
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
	path to_create;
	std::vector<path> prefixes = getSearchPaths(prefix);
	std::vector<path> suffixes = getSearchPaths(suffix);
	if(!suffixes.empty()) {
		std::vector<path> paths = platform::getSystemPaths(systemPathId);
		prefixes.insert(prefixes.end(), paths.begin(), paths.end());
	}
	bool create_exists = false;
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

static bool addSearchPath(std::vector<path> & result, const path & dir, bool filter) {
	
	if(filter && !is_directory(dir)) {
		return false;
	}
	
	if(std::find(result.begin(), result.end(), dir) != result.end()) {
		return false;
	}
	
	result.push_back(dir);
	return true;
}

static bool addSearchRoot(std::vector<path> & result, const path & dir, bool filter) {
	
	// Add "data" subdirectory or dirs referenced in "data" file
	fs::path subdir = dir / "data";
	if(fs::is_regular_file(subdir)) {
		fs::ifstream ifs(subdir);
		std::string line;
		while(std::getline(ifs, line)) {
			fs::path datadir = dir / line;
			if(addSearchPath(result, datadir, filter)) {
				LogDebug("got data dir from data file in dir: " << datadir);
			} else {
				LogDebug("ignoring data dir from data file in dir: " << datadir);
			}
		}
	} else if(addSearchPath(result, subdir, filter)) {
		LogDebug("got data dir from data subdir: " << subdir);
	}
	
	return addSearchPath(result, dir, filter);
}

static std::string getSearchPathVar(const path & exepath) {
	#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	return "%" + boost::to_upper_copy(exepath.basename()) + "_PATH%";
	#else
	return "${" + exepath.basename() + "_PATH}";
	#endif
}

std::vector<path> SystemPaths::getSearchPaths(bool filter) const {
	
	std::vector<path> result;
	
	// Use the user diretcory as the highest-priority data directory
	if(!user.empty() && addSearchRoot(result, user, filter)) {
		LogDebug("got data dir from user dir: " << user);
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
	
	// Check system settings (windows registry)
	std::string temp;
	if(platform::getSystemConfiguration("DataDir", temp)) {
		path dir = canonical(temp);
		if(addSearchRoot(result, dir, filter)) {
			LogDebug("got data dir from registry: \"" << temp << "\" = " << dir);
		} else {
			LogDebug("ignoring data dir from registry: \"" << temp << "\" = " << dir);
		}
	}
	
	// Check paths specified in environment variables
	path exepath = platform::getExecutablePath();
	if(!exepath.empty()) {
		std::string var = getSearchPathVar(exepath);
		std::vector<path> paths = fs::getSearchPaths(var.c_str());
		BOOST_FOREACH(const path & p, paths) {
			#if ARX_PLATFORM == ARX_PLATFORM_WIN32
			if(p.string() == var) {
				continue; // not defined
			}
			#endif
			if(addSearchRoot(result, p, filter)) {
				LogDebug("got data dir from exe: " << var << " = " << p);
			} else {
				LogDebug("ignoring data dir from exe: " << var << " = " << p);
			}
		}
	}
	
	// Search the executable directory
	if(relative_data_dir && !exepath.empty()) {
		path exedir = canonical(exepath.parent());
		bool ignored = false;
		if(ignore_exe_dir) {
			std::vector<path> ignored_dirs = fs::getSearchPaths(ignore_exe_dir);
			ignored = (std::find(ignored_dirs.begin(), ignored_dirs.end(), exedir)
			           != ignored_dirs.end());
		}
		std::vector<path> relative_data_dirs = fs::getSearchPaths(relative_data_dir);
		BOOST_FOREACH(const path & p, relative_data_dirs) {
			path dir = exedir / p;
			if(!ignored && addSearchRoot(result, dir, filter)) {
				LogDebug("got data dir from exe: " << exepath << " + " << p << " -> " << dir);
			} else {
				LogDebug("ignoring data dir from exe: " << exepath << " + " << p << " -> " << dir);
			}
		}
	}
	
	// Search standard locations
	std::vector<path> prefixes = fs::getSearchPaths(data_dir_prefixes);
	std::vector<path> suffixes = fs::getSearchPaths(data_dir);
	BOOST_FOREACH(const path & prefix, prefixes) {
		BOOST_FOREACH(const path & suffix, suffixes) {
			path dir = canonical(prefix / suffix);
			if(addSearchRoot(result, dir, filter)) {
				LogDebug("got data dir from search: " << prefix
				         << " + " << suffix << " = " << dir);
			} else {
				LogDebug("ignoring data dir from search: " << prefix
				         << " + " << suffix << " = " << dir);
			}
		}
	}
	
	return result;
}

static SystemPaths::InitParams cmdLineInitParams;

ExitStatus SystemPaths::init() {
	return init(cmdLineInitParams);
}

ExitStatus SystemPaths::init(const InitParams & initParams) {
	
	user = findUserPath("user", initParams.forceUser, "UserDir", platform::UserDirPrefixes,
	                    user_dir_prefixes, user_dir, current_path(), !initParams.displaySearchDirs);
	
	config = findUserPath("config", initParams.forceConfig, "ConfigDir", platform::NoPath,
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

SystemPaths::SystemPaths()
	: findData_(true)
{}

static void listDirectoriesFor(std::ostream & os, const std::string & regKey,
                               platform::SystemPathId systemPathId,
                               const char * prefixVar = NULL,
                               const char * suffixVar = NULL) {
	
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
	
	std::vector<path> prefixes = getSearchPaths(prefixVar);
	std::vector<path> suffixes = getSearchPaths(suffixVar);
	std::vector<path> paths;
	if(suffixVar) {
		paths = platform::getSystemPaths(systemPathId);
		prefixes.insert(prefixes.end(), paths.begin(), paths.end());
	}
	if((prefixVar || !paths.empty()) && suffixVar) {
		
		os << " - ";
		if(prefixVar) {
			os << '"' << prefixVar << '"';
		}
		if(prefixVar && !paths.empty()) {
			os << " and ";
		}
		if(!paths.empty()) {
			os << " system paths";
		}
		os << " / ";
		if(suffixVar) {
			os << '"' << suffixVar << '"';
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
	listDirectoriesFor(os, "UserDir", platform::UserDirPrefixes, user_dir_prefixes, user_dir);
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
	listDirectoriesFor(os, "ConfigDir", platform::NoPath, config_dir_prefixes, config_dir);
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
	std::string exevar = getSearchPathVar(platform::getExecutablePath());
	os << " - Paths specifed in \"" << exevar << "\"\n";
	os << " - The directory containing the game executable\n";
	listDirectoriesFor(os, "DataDir", platform::NoPath, data_dir_prefixes, data_dir);
	os << "For all of these (except those specified via --data-dir),\n";
	os << "the \"data\" subdirectory or file will also be searched.\n";
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

static void disableDataDir() {
	fs::cmdLineInitParams.findData = false;
}

static void addDataDir(const std::string & dataDir) {
	fs::cmdLineInitParams.dataDirs.push_back(dataDir);
}

static void setUserDir(const std::string & userDir) {
	fs::cmdLineInitParams.forceUser = userDir;
}

static void setConfigDir(const std::string & configDir) {
	fs::cmdLineInitParams.forceConfig = configDir;
}

static void listSearchDirs() {
	fs::cmdLineInitParams.displaySearchDirs = true;
}

ARX_PROGRAM_OPTION("no-data-dir", "n",
                   "Don't automatically detect data directories", &disableDataDir)
ARX_PROGRAM_OPTION_ARG("data-dir",    "d",
                       "Where to find the data files (can be repeated)", &addDataDir, "DIR")
ARX_PROGRAM_OPTION_ARG("user-dir",    "u",
                       "Where to store user-specific files", &setUserDir, "DIR")
ARX_PROGRAM_OPTION_ARG("config-dir",  "c",
                       "Where to store config files", &setConfigDir, "DIR")
ARX_PROGRAM_OPTION("list-dirs",   "l",
                   "List the searched user and data directories", &listSearchDirs)
