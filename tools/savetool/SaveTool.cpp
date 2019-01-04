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

#include <iostream>
#include <vector>
#include <string>

#include <boost/foreach.hpp>

#include "io/SaveBlock.h"
#include "io/fs/Filesystem.h"
#include "io/fs/SystemPaths.h"
#include "io/log/Logger.h"

#include "platform/ProgramOptions.h"
#include "platform/WindowsMain.h"

#include "savetool/SaveFix.h"
#include "savetool/SaveRename.h"
#include "savetool/SaveView.h"

#include "util/cmdline/CommandLine.h"

static void print_help() {
	std::cout << "usage: savetool <command> <savefile> [<options>...]\n"
	             "commands are:\n"
	             " - extract <savefile>\n"
	             " - add <savefile> [<files>...]\n"
	             " - fix <savefile>\n"
	             " - rename <savefile> <newname>\n"
	             " - view <savefile> [<ident>]\n";
}

static int main_extract(SaveBlock & save, const std::vector<std::string> & args) {
	
	if(!args.empty()) {
		return -1;
	}
	
	if(!save.open()) {
		return 2;
	}
	
	std::vector<std::string> files = save.getFiles();
	
	for(std::vector<std::string>::iterator file = files.begin(); file != files.end(); ++file) {
		
		std::string buffer = save.load(*file);
		if(buffer.empty()) {
			std::cerr << "error loading " << *file << " from save\n";
			continue;
		}
		
		fs::ofstream h(*file, std::ios_base::out | std::ios_base::binary);
		if(!h.is_open()) {
			std::cerr << "error opening " << *file << " for writing\n";
			continue;
		}
		
		if(h.write(buffer.data(), buffer.size()).fail()) {
			std::cerr << "error writing to " << *file << '\n';
		}
		
	}
	
	return 0;
}

static int main_add(SaveBlock & save, const std::vector<std::string> & args) {
	
	if(!save.open(true)) {
		return 2;
	}
	
	BOOST_FOREACH(fs::path file, args) {
		
		std::string data = fs::read(file);
		if(data.empty()) {
			std::cerr << "error loading " << file;
			continue;
		}
		
		std::string name = file.filename();
		if(!save.save(name, data.data(), data.size())) {
			std::cerr << "error writing " << name << " to save";
		}
		
	}
	
	save.flush("pld");
	
	return 0;
}

static std::vector<std::string> g_args;

static void handlePositionalArgument(const std::string & file) {
	g_args.push_back(file);
}

ARX_PROGRAM_OPTION_ARG("", "", "savetool arguments", &handlePositionalArgument, "ARGS")

int utf8_main(int argc, char ** argv) {
	
	Logger::initialize();
	
	// Parse the command line and process options
	ExitStatus status = parseCommandLine(argc, argv);
	
	if(status == RunProgram) {
		status = fs::initSystemPaths();
	}
	
	if(status == RunProgram && g_args.size() < 2) {
		print_help();
		status = ExitFailure;
	}
	
	if(status == RunProgram) {
		
		std::string command = g_args[0];
		
		fs::path savefile = g_args[1];
		
		if(fs::is_directory(savefile)) {
			savefile /= "gsave.sav";
		}
		
		SaveBlock save(savefile);
		
		g_args.erase(g_args.begin(), g_args.begin() + 2);
		
		int ret = -1;
		if(command == "e" || command == "extract") {
			ret = main_extract(save, g_args);
		} else if(command == "a" || command == "add") {
			ret = main_add(save, g_args);
		} else if(command == "f" || command == "fix") {
			ret = main_fix(save, g_args);
		} else if(command == "r" || command == "rename") {
			ret = main_rename(save, g_args);
		} else if(command == "v" || command == "view") {
			ret = main_view(save, g_args);
		}
		
		if(ret != 0) {
			if(ret == -1) {
				print_help();
			}
			status = ExitFailure;
		}
		
	}
	
	Logger::shutdown();
	
	return (status == ExitFailure) ? EXIT_FAILURE : EXIT_SUCCESS;
}
