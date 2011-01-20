
#include <hermes/PakReader.h>
#include <hermes/PakEntry.h>

#include <string>
#include <cassert>
using std::string;

#include <sys/stat.h>
#include <cstdio>
#include <cstdlib>

#include <algorithm>
using std::transform;

void dump(const PakReader & pak, const PakDirectory * dir, string where = string()) {
	
	if(!dir) {
		return;
	}
	
	string dirname = where;
	if(dir->name) {
		string n = dir->name;
		transform(n.begin(), n.end(), n.begin(), tolower);
		if(!n.empty() && *n.rbegin() == '\\') {
			dirname += n.substr(0, n.size() - 1);
		} else {
			dirname += n;
		}
		dirname += "/";
	}
	
	mkdir(dirname.c_str(), 0777);
	
	//printf("%s", dirname.c_str());
	
	PakFile * file = dir->fichiers;
	while(file != NULL) {
		
		if(!file->name) {
			
		}
		
		string fn = file->name;
		transform(fn.begin(), fn.end(), fn.begin(), tolower);
		string filename = dirname + fn;
		
		printf("%s\n", filename.c_str());
		
		int size;
		const char * data = pak.ReadAlloc(filename.c_str(), &size);
		assert(data != NULL);
		
		FILE * f = fopen(filename.c_str(), "wb");
		if(!f) {
			printf("error opening file for writing: %s\n", filename.c_str());
			exit(1);
		}
		
		if(fwrite(data, size, 1, f) != 1) {
			printf("error writing to file for writing: %s\n", filename.c_str());
			exit(1);
		}
		
		fclose(f);
		
		free(data);
		
		file = file->fnext;
	}
	
	dump(pak, dir->fils, dirname);
	
	dump(pak, dir->brothernext, where);
	
}


int main(int argc, char ** argv) {
	
	if(argc != 2) {
		printf("usage: unpak <pakfile>\n");
		return 1;
	}
	
	PakReader pak;
	if(!pak.Open(argv[1])) {
		printf("error opening PAK file\n");
		return 1;
	}
	
	dump(pak, pak.pRoot);
	
}
