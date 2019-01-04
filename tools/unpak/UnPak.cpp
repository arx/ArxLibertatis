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

#include <vector>
#include <string>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <map>

#include <boost/foreach.hpp>

#include "io/fs/FilePath.h"
#include "io/fs/Filesystem.h"
#include "io/fs/FileStream.h"
#include "io/fs/SystemPaths.h"
#include "io/resource/PakReader.h"
#include "io/resource/PakEntry.h"
#include "io/resource/ResourcePath.h"
#include "io/log/Logger.h"

#include "platform/ProgramOptions.h"
#include "platform/WindowsMain.h"

#include "util/MD5.h"
#include "util/Unicode.h"
#include "util/cmdline/CommandLine.h"


enum UnpakAction {
	UnpakExtract,
	UnpakManifest,
	UnpakList,
};

static void processDirectory(PakDirectory & dir, const fs::path & prefix,
                             const res::path & dirname, UnpakAction action) {
	
	if(action == UnpakExtract) {
		fs::path dirpath = prefix / dirname.string();
		if(!fs::create_directory(dirpath)) {
			LogWarning << "Error creating directory: " << dirpath;
		}
	}
	
	{
		// Copy to map to ensure filenames are sorted
		typedef std::map<std::string, PakFile *> SortedFiles;
		SortedFiles files;
		for(PakDirectory::files_iterator i = dir.files_begin(); i != dir.files_end(); ++i) {
			// TODO this should really be done when loading the pak file
			std::string name = util::convert<util::ISO_8859_1, util::UTF8>(i->first);
			files[name] = i->second;
		}
		BOOST_FOREACH(const SortedFiles::value_type & entry, files) {
			res::path path = dirname / entry.first;
			
			if(action == UnpakExtract || action == UnpakManifest) {
				
				PakFile * file = entry.second;
				
				std::string buffer = file->read();
				
				if(action == UnpakExtract) {
					
					fs::path filepath = prefix / path.string();
					fs::ofstream ofs(filepath, fs::fstream::out | fs::fstream::binary | fs::fstream::trunc);
					if(!ofs.is_open()) {
						LogError << "Error opening file for writing: " << filepath;
						std::exit(1);
					}
					
					if(ofs.write(buffer.data(), buffer.size()).fail()) {
						LogError << "Error writing to file: " << filepath;
						std::exit(1);
					}
					
				}
				
				if(action == UnpakManifest) {
					std::cout << util::md5::compute(buffer) << " *";
				}
				
			}
			
			std::cout << path.string() << '\n';
		}
	}
	
	{
		// Copy to map to ensure dirnames are sorted
		typedef std::map<std::string, PakDirectory *> SortedDirs;
		SortedDirs subdirs;
		for(PakDirectory::dirs_iterator i = dir.dirs_begin(); i != dir.dirs_end(); ++i) {
			// TODO this should really be done when loading the pak file
			std::string name = util::convert<util::ISO_8859_1, util::UTF8>(i->first);
			subdirs[name] = &i->second;
		}
		BOOST_FOREACH(const SortedDirs::value_type & entry, subdirs) {
			res::path path = dirname / entry.first;
			if(action == UnpakManifest) {
				std::cout << "                                  ";
			}
			std::cout << path.string() << '/' << '\n';
			processDirectory(*entry.second, prefix, path, action);
		}
	}
	
}

static void processResources(PakReader & resources, const fs::path & prefix, UnpakAction action) {
	
	if(action == UnpakExtract && !prefix.empty()) {
		if(!fs::create_directories(prefix)) {
			LogWarning << "Error creating output directory: " << prefix;
		}
		LogInfo << "Extracting files to " << prefix;
	}
	
	PakReader::ReleaseFlags release = resources.getReleaseType();
	std::cout << "Type: ";
	bool first = true;
	if(release & PakReader::Demo) {
		std::cout << "demo";
		first = false;
	}
	if(release & PakReader::FullGame) {
		if(!first) {
			std::cout << ", ";
		}
		std::cout << "full game";
		first = false;
	}
	if(release & PakReader::Unknown) {
		if(!first) {
			std::cout << ", ";
		}
		std::cout << "unknown";
		first = false;
	}
	if(release & PakReader::External) {
		if(!first) {
			std::cout << ", ";
		}
		std::cout << "external";
	}
	std::cout << "\n";
	
	processDirectory(resources, prefix, res::path(), action);
}

static UnpakAction g_action = UnpakExtract;
static fs::path g_outputDir;
static bool g_addDefaultArchives = false;
static std::vector<fs::path> g_archives;
static bool g_quiet = false;

static void handleExtractOption() {
	g_action = UnpakExtract;
}

static void handleManifestOption() {
	g_action = UnpakManifest;
}

static void handleListOption() {
	g_action = UnpakList;
}

static void handleOutputDirOption(const std::string & outputDir) {
	g_outputDir = outputDir;
}

static void handleAllOption() {
	g_addDefaultArchives = true;
}

static void handleQuietOption() {
	g_quiet = true;
}

static void handlePositionalArgument(const std::string & file) {
	g_archives.push_back(file);
}

ARX_PROGRAM_OPTION("extract", "e", "Extract archive contents", &handleExtractOption)

ARX_PROGRAM_OPTION("manifest", "m", "Print archive manifest", &handleManifestOption)

ARX_PROGRAM_OPTION("list", "", "List archive contents", &handleListOption)

ARX_PROGRAM_OPTION_ARG("output-dir", "o", "Directory to extract files to", &handleOutputDirOption, "DIR")

ARX_PROGRAM_OPTION("all", "a", "Process all pak files loaded by the game", &handleAllOption)

ARX_PROGRAM_OPTION("quiet", "q", "Don't print log output", &handleQuietOption)

ARX_PROGRAM_OPTION_ARG("", "", "PAK archives to process", &handlePositionalArgument, "DIRS")

static void addResourceDir(PakReader & resources, const fs::path & base) {
	
	// TODO share this list with the game code
	static const char * const resource_dirs[] = {
		"editor", "game", "graph", "localisation", "misc", "sfx", "speech",
	};
	
	BOOST_FOREACH(const char * dirname, resource_dirs) {
		resources.addFiles(base / dirname, dirname);
	}
	
}

int utf8_main(int argc, char ** argv) {
	
	ARX_UNUSED(g_resources);
	
	Logger::initialize();
	
	// Parse the command line and process options
	ExitStatus status = parseCommandLine(argc, argv);
	
	if(g_quiet) {
		Logger::shutdown();
	}
	
	if(status == RunProgram && (g_addDefaultArchives || g_archives.empty())) {
		status = fs::initSystemPaths();
	}
	
	// When no archives have been specified, extract all archives to a default location
	// This is useful for (Windows) users to be able to extract everything by simply launching arxunpak
	if(status == RunProgram && g_archives.empty() && !g_addDefaultArchives) {
		g_addDefaultArchives = true;
		if(g_outputDir.empty() && g_action == UnpakExtract) {
			g_outputDir = fs::getUserDir() / "unpacked";
		}
	}
	
	PakReader resources;
	
	if(status == RunProgram && g_addDefaultArchives) {
		// TODO share this list with the game code
		static const char * const default_paks[][2] = {
			{ "data.pak", NULL },
			{ "loc.pak", "loc_default.pak" },
			{ "data2.pak", NULL },
			{ "sfx.pak", NULL },
			{ "speech.pak", "speech_default.pak" },
		};
		BOOST_FOREACH(const char * const * const filenames, default_paks) {
			if(resources.addArchive(fs::findDataFile(filenames[0]))) {
				continue;
			}
			if(filenames[1] && resources.addArchive(fs::findDataFile(filenames[1]))) {
				continue;
			}
			std::ostringstream oss;
			oss << "Missing required data file: \"" << filenames[0] << "\"";
			if(filenames[1]) {
				oss << " (or \"" << filenames[1] << "\")";
			}
			LogError << oss.str();
		}
		BOOST_REVERSE_FOREACH(const fs::path & base, fs::getDataDirs()) {
			addResourceDir(resources, base);
		}
	}
	
	if(status == RunProgram) {
		BOOST_FOREACH(const fs::path & archive, g_archives) {
			if(fs::is_regular_file(archive)) {
				if(!resources.addArchive(archive)) {
					LogCritical << "Could not open archive " << archive << "!";
					status = ExitFailure;
					break;
				}
			} else if(fs::is_directory(archive)) {
				addResourceDir(resources, archive);
			} else {
				LogCritical << "File or directory " << archive << " does not exist!";
				status = ExitFailure;
				break;
			}
		}
	}
	
	if(status == RunProgram) {
		processResources(resources, g_outputDir, g_action);
	}
	
	if(!g_quiet) {
		Logger::shutdown();
	}
	
	return (status == ExitFailure) ? EXIT_FAILURE : EXIT_SUCCESS;
}
