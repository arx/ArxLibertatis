
#include <string>
#include <cassert>
using std::string;

#include <sys/stat.h>
#include <cstdio>
#include <cstdlib>
#include <sstream>
using std::ostringstream;

#include <algorithm>
using std::transform;

#include "io/Filesystem.h"
#include "io/PakReader.h"
#include "io/PakEntry.h"

void dump(PakDirectory & dir, const string  & dirname = string()) {
	
	CreateFullPath(dirname);
	
	for(PakDirectory::files_iterator i = dir.files_begin(); i != dir.files_end(); ++i) {
		
		string filename = dirname + i->first;
		
		PakFile * file = i->second;
		
		printf("%s\n", filename.c_str());
		
		FILE * f = fopen(filename.c_str(), "wb");
		if(!f) {
			printf("error opening file for writing: %s\n", filename.c_str());
			exit(1);
		}
		
		if(file->size() > 0) {
			
			char * data = (char*)file->readAlloc();
			assert(data != NULL);
			
			if(fwrite(data, file->size(), 1, f) != 1) {
				printf("error writing to file: %s\n", filename.c_str());
				fclose(f);
				exit(1);
			}
			
			free(data);
			
		}
		
		fclose(f);
	}
	
	for(PakDirectory::dirs_iterator i = dir.dirs_begin(); i != dir.dirs_end(); ++i) {
		dump(i->second, dirname + i->first + '/');
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
