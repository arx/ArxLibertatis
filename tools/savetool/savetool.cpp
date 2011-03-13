
#include <string>
#include <cassert>
#include <vector>
#include <cstdio>
using std::string;
using std::vector;
using std::printf;

#include "io/SaveBlock.h"
#include "io/Filesystem.h"

static void print_help() {
	printf("usage: savetool <command> <savefile> [<options>...]\n"
	       "commands are:\n"
	       " - extract <savefile>\n"
	       " - add <savefile> [<files>...]\n"
	);
}

static int main_extract(SaveBlock & save, int argc, char ** argv) {
	
	(void)argv;
	
	if(argc != 0) {
		print_help();
		return 1;
	}
	
	if(!save.BeginRead()) {
		printf("error opening save for reading\n");
		return 2;
	}
	
	vector<string> files = save.getFiles();
	
	for(vector<string>::iterator file = files.begin(); file != files.end(); ++file) {
		
		size_t size;
		char * data = save.load(*file, size);
		if(!data) {
			printf("error loading %s from save\n", file->c_str());
			continue;
		}
		
		FileHandle h = FileOpenWrite(file->c_str());
		if(!h) {
			printf("error opening %s for writing\n", file->c_str());
			continue;
		}
		
		if((size_t)FileWrite(h, data, size) != size) {
			printf("error writing %s\n", file->c_str());
		}
		
		FileClose(h);
		
	}
	
	return 0;
}

static int main_add(SaveBlock & save, int argc, char ** argv) {
	
	if(!save.BeginSave()) {
		printf("error opening save for writing\n");
		return 2;
	}
	
	for(int i = 0; i < argc; i++) {
		
		size_t size;
		char * data = (char*)FileLoadMalloc(argv[i], &size);
		
		if(!data) {
			printf("error loading %s\n", argv[i]);
		} else {
			
			string name = argv[i];
			size_t pos = name.find_last_of("/\\");
			if(pos != string::npos) {
				name = name.substr(pos + 1);
			}
			
			if(!save.save(name, data, size)) {
				printf("error writing %s to save\n", argv[i]);
			}
		}
		
	}
	
	save.flush();
	
	return 0;
}

int main(int argc, char ** argv) {
	
	if(argc < 3) {
		print_help();
		return 1;
	}
	
	string command = argv[1];
	SaveBlock save(argv[2]);
	
	argc -= 3;
	argv += 3;
	
	if(command == "e" || command == "extract") {
		return main_extract(save, argc, argv);
	} else if(command == "a" || command == "add") {
		return main_add(save, argc, argv);
	} else {
		print_help();
		return 1;
	}
	
}
