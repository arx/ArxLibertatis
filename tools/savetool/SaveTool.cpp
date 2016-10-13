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

#include <iostream>

#include "io/SaveBlock.h"
#include "io/fs/Filesystem.h"
#include "io/log/Logger.h"

#include "platform/WindowsMain.h"

#include "savetool/SaveFix.h"
#include "savetool/SaveRename.h"
#include "savetool/SaveView.h"

static void print_help() {
	std::cout << "usage: savetool <command> <savefile> [<options>...]\n"
	             "commands are:\n"
	             " - extract <savefile>\n"
	             " - add <savefile> [<files>...]\n"
	             " - fix <savefile>\n"
	             " - rename <savefile> <newname>\n"
	             " - view <savefile> [<ident>]\n";
}

static int main_extract(SaveBlock & save, int argc, char ** argv) {
	
	(void)argv;
	
	if(argc != 0) {
		return -1;
	}
	
	if(!save.open()) {
		return 2;
	}
	
	std::vector<std::string> files = save.getFiles();
	
	for(std::vector<std::string>::iterator file = files.begin(); file != files.end(); ++file) {
		
		size_t size;
		char * data = save.load(*file, size);
		if(!data) {
			std::cerr << "error loading " << *file << " from save\n";
			continue;
		}
		
		fs::ofstream h(*file, std::ios_base::out | std::ios_base::binary);
		if(!h.is_open()) {
			std::cerr << "error opening " << *file << " for writing\n";
			free(data);
			continue;
		}
		
		if(h.write(data, size).fail()) {
			std::cerr << "error writing to " << *file << '\n';
		}
		
		free(data);
	}
	
	return 0;
}

static int main_add(SaveBlock & save, int argc, char ** argv) {
	
	if(!save.open(true)) {
		return 2;
	}
	
	for(int i = 0; i < argc; i++) {
		
		size_t size;
		char * data = fs::read_file(argv[i], size);
		
		if(!data) {
			std::cerr << "error loading " << argv[i];
		} else {
			
			std::string name = argv[i];
			size_t pos = name.find_last_of("/\\");
			if(pos != std::string::npos) {
				name = name.substr(pos + 1);
			}
			
			if(!save.save(name, data, size)) {
				std::cerr << "error writing " << name << " to save";
			}
			
			delete[] data;
		}
		
	}
	
	save.flush("pld");
	
	return 0;
}

int utf8_main(int argc, char ** argv) {
	
	Logger::initialize();
	
	if(argc < 3) {
		print_help();
		return 1;
	}
	
	std::string command = argv[1];
	
	fs::path savefile = argv[2];
	
	if(fs::is_directory(savefile)) {
		savefile /= "gsave.sav";
	}
	
	SaveBlock save(savefile);
	
	argc -= 3;
	argv += 3;
	
	int ret = -1;
	if(command == "e" || command == "extract") {
		ret = main_extract(save, argc, argv);
	} else if(command == "a" || command == "add") {
		ret = main_add(save, argc, argv);
	} else if(command == "f" || command == "fix") {
		ret = main_fix(save, argc, argv);
	} else if(command == "r" || command == "rename") {
		ret = main_rename(save, argc, argv);
	} else if(command == "v" || command == "view") {
		ret = main_view(save, argc, argv);
	}
	
	if(ret == -1) {
		print_help();
	}
	
	return ret;
}
