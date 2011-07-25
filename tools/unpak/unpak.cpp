
#include <string>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <algorithm>

#include "io/Filesystem.h"
#include "io/PakReader.h"
#include "io/PakEntry.h"
#include "io/FilePath.h"
#include "io/FileStream.h"

using std::transform;
using std::ostringstream;
using std::string;

void dump(PakDirectory & dir, const fs::path & dirname = fs::path()) {
	
	fs::create_directories(dirname);
	
	for(PakDirectory::files_iterator i = dir.files_begin(); i != dir.files_end(); ++i) {
		
		fs::path filename = dirname / i->first;
		
		PakFile * file = i->second;
		
		printf("%s\n", filename.string().c_str());
		
		fs::ofstream ofs(filename, fs::fstream::out | fs::fstream::binary | fs::fstream::trunc);
		if(!ofs.is_open()) {
			printf("error opening file for writing: %s\n", filename.string().c_str());
			exit(1);
		}
		
		if(file->size() > 0) {
			
			char * data = (char*)file->readAlloc();
			assert(data != NULL);
			
			if(ofs.write(data, file->size()).fail()) {
				printf("error writing to file: %s\n", filename.string().c_str());
				exit(1);
			}
			
			free(data);
			
		}
		
	}
	
	for(PakDirectory::dirs_iterator i = dir.dirs_begin(); i != dir.dirs_end(); ++i) {
		dump(i->second, dirname / i->first);
	}
	
}

int main(int argc, char ** argv) {
	
	if(argc < 2) {
		printf("usage: unpak <pakfile> [<pakfile>...]\n");
		return 1;
	}
	
	for(int i = 1; i < argc; i++) {
		
		PakReader pak;
		if(!pak.addArchive(argv[i])) {
			printf("error opening PAK file\n");
			return 1;
		}
		
		dump(pak);
		
	}
	
}
