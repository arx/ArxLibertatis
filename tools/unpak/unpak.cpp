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

#include "io/PakReader.h"
#include "io/PakEntry.h"
//#define TEST_PAK_FREAD

void dump(PakReader & pak, const PakDirectory * dir, const string  & where = string()) {
	
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
	
	PakFile * file = dir->files;
	while(file != NULL) {
		
		if(file->name.empty()) {
			
		}
		
		string fn = file->name;
		transform(fn.begin(), fn.end(), fn.begin(), tolower);
		string filename = dirname + fn;
		
		printf("%s\n", filename.c_str());
		
		FILE * f = fopen(filename.c_str(), "wb");
		if(!f) {
			printf("error opening file for writing: %s\n", filename.c_str());
			exit(1);
		}
		
		if(file->size && (!(file->flags & PAK_FILE_COMPRESSED) || file->uncompressedSize)) {
			
			size_t size;
			char * data = (char*)pak.ReadAlloc(filename, size);
			assert(data != NULL);
			
			if(fwrite(data, size, 1, f) != 1) {
				printf("error writing to file for writing: %s\n", filename.c_str());
				fclose(f);
				exit(1);
			}
			
#ifdef TEST_PAK_FREAD
			string fdata;
			PakFileHandle * pfh = pak.fOpen(filename.c_str());
			assert(pfh != NULL);
			char buf[10];
			size_t s;
			while((s = pak.fRead(buf, 10, 1, pfh))) {
				fdata.append(buf, s);
			}
			pak.fClose(pfh);
			
			assert(size == fdata.size());
			assert(fdata == string(data, size));
#endif // TEST_PAK_FREAD
			
			free(data);
			
		}
		
		fclose(f);
		
		file = file->next;
	}
	
	dump(pak, dir->children, dirname);
	
	dump(pak, dir->next, where);
	
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
	
	dump(pak, pak.root);
	
}
