/*
 * Copyright 2011-2016 Arx Libertatis Team (see the AUTHORS file)
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
#include <boost/range/size.hpp>

#include "io/fs/FilePath.h"
#include "io/fs/Filesystem.h"
#include "io/fs/FileStream.h"
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

static void processDirectory(PakDirectory & dir, const fs::path & dirname, UnpakAction action) {
	
	if(action == UnpakExtract && !fs::create_directories(dirname)) {
		LogWarning << "Failed to create target directory";
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
			fs::path path = dirname / entry.first;
			
			if(action == UnpakExtract || action == UnpakManifest) {
				
				PakFile * file = entry.second;
				char * data = (char*)file->readAlloc();
				arx_assert(file->size() == 0 || data != NULL);
				
				if(action == UnpakExtract) {
					
					fs::ofstream ofs(path, fs::fstream::out | fs::fstream::binary | fs::fstream::trunc);
					if(!ofs.is_open()) {
						LogError << "Error opening file for writing: " << path;
						std::exit(1);
					}
					
					if(ofs.write(data, file->size()).fail()) {
						LogError << "Error writing to file: " << path;
						std::exit(1);
					}
					
				}
				
				if(action == UnpakManifest) {
					
					util::md5 hash;
					hash.init();
					hash.update(data, file->size());
					char checksum[hash.size];
					hash.finalize(checksum);
					
					for(size_t i = 0; i < size_t(boost::size(checksum)); i++) {
						std::cout << std::setfill('0') << std::hex << std::setw(2) << int(u8(checksum[i]));
					}
					std::cout << " *";
					
				}
				
				std::free(data);
				
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
			fs::path path = dirname / entry.first;
			if(action == UnpakManifest) {
				std::cout << "                                  ";
			}
			std::cout << path.string() << '/' << '\n';
			processDirectory(*entry.second, path, action);
		}
	}
	
}

static void processResources(PakReader & resources, const fs::path & dirname, UnpakAction action) {
	
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
	
	processDirectory(resources, dirname, action);
}

static UnpakAction g_action = UnpakExtract;
static fs::path g_outputDir;
static std::vector<fs::path> g_archives;

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

static void handlePositionalArgument(const std::string & file) {
	g_archives.push_back(file);
}

ARX_PROGRAM_OPTION("extract", "e", "Extract archive contents", &handleExtractOption)

ARX_PROGRAM_OPTION("manifest", "m", "Print archive manifest", &handleManifestOption)

ARX_PROGRAM_OPTION("list", "", "List archive contents", &handleListOption)

ARX_PROGRAM_OPTION_ARG("output-dir", "o", "Directory to extract files to", &handleOutputDirOption, "DIR")

ARX_PROGRAM_OPTION_ARG("", "", "PAK archives to process", &handlePositionalArgument, "DIRS")

int utf8_main(int argc, char ** argv) {
	
	ARX_UNUSED(resources);
	
	// Parse the command line and process options
	ExitStatus status = parseCommandLine(argc, argv);
	
	Logger::initialize();
	
	if(g_archives.empty()) {
		std::cout << "usage: unpak <pakfile> [<pakfile>...]\n";
		return 1;
	}
	
	PakReader resources;
	
	if(status == RunProgram) {
		BOOST_FOREACH(const fs::path & archive, g_archives) {
			if(fs::exists(archive)) {
				if(!resources.addArchive(archive)) {
					LogCritical << "Could not open archive " << archive << "!";
					status = ExitFailure;
					break;
				}
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
	
	return 0;
}
