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

#include <boost/foreach.hpp>

#include "io/fs/FilePath.h"
#include "io/fs/Filesystem.h"
#include "io/fs/FileStream.h"
#include "io/resource/PakReader.h"
#include "io/resource/PakEntry.h"
#include "io/resource/ResourcePath.h"
#include "io/log/Logger.h"

#include "platform/ProgramOptions.h"
#include "platform/WindowsMain.h"

#include "util/Unicode.h"
#include "util/cmdline/CommandLine.h"


static void dump(PakDirectory & dir, const fs::path & dirname) {
	
	if(!fs::create_directories(dirname)) {
		LogWarning << "Failed to create target directory";
	}
	
	for(PakDirectory::files_iterator i = dir.files_begin(); i != dir.files_end(); ++i) {
		// TODO this should really be done when loading the pak file
		fs::path path = dirname / util::convert<util::ISO_8859_1, util::UTF8>(i->first);
		
		fs::ofstream ofs(path, fs::fstream::out | fs::fstream::binary | fs::fstream::trunc);
		if(!ofs.is_open()) {
			LogError << "Error opening file for writing: " << path;
			exit(1);
		}
		
		PakFile * file = i->second;
		char * data = (char*)file->readAlloc();
		arx_assert(file->size() == 0 || data != NULL);
		
		if(ofs.write(data, file->size()).fail()) {
			LogError << "Error writing to file: " << path;
			exit(1);
		}
		
		free(data);
		
		std::cout << path.string() << '\n';
	}
	
	for(PakDirectory::dirs_iterator i = dir.dirs_begin(); i != dir.dirs_end(); ++i) {
		fs::path path = dirname / util::convert<util::ISO_8859_1, util::UTF8>(i->first);
		dump(i->second, path);
	}
	
}

static fs::path g_outputDir;
static std::vector<fs::path> g_archives;

static void handleOutputDirOption(const std::string & outputDir) {
	g_outputDir = outputDir;
}

static void handlePositionalArgument(const std::string & file) {
	g_archives.push_back(file);
}

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
		dump(resources, g_outputDir);
	}
	
	return 0;
}
