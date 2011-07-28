/*
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/

#include "io/Filesystem.h"

#include "Configure.h"

#ifdef HAVE_POSIX_FILESYSTEM

#include <sys/stat.h>
#include <sys/errno.h>
#include <dirent.h>
#include <cstdio>
#include <cstring>

#include "io/FilePath.h"
#include "io/FileStream.h"

using std::string;

namespace fs {

bool exists(const path & p) {
	if(p.empty()) {
		return true;
	}
	struct stat buf;
	return !stat(p.string().c_str(), &buf);
}

bool is_directory(const path & p) {
	if(p.empty()) {
		return true;
	}
	struct stat buf;
	return !stat(p.string().c_str(), &buf) && ((buf.st_mode & S_IFMT) == S_IFDIR);
}

bool is_regular_file(const path & p) {
	struct stat buf;
	return !stat(p.string().c_str(), &buf) && ((buf.st_mode & S_IFMT) == S_IFREG);
}

std::time_t last_write_time(const path & p) {
	struct stat buf;
	return stat(p.string().c_str(), &buf) ? 0 : buf.st_mtime;
}

u64 file_size(const path & p) {
	struct stat buf;
	return stat(p.string().c_str(), &buf) ? 0 : buf.st_size;
}

bool remove(const path & p) {
	int ret = ::remove(p.string().c_str());
	return !ret || ret == ENOENT || ret == ENOTDIR;
}

bool remove_all(const path & p) {
	
	struct stat buf;
	if(stat(p.string().c_str(), &buf)) {
		return false;
	}
	
	if((buf.st_mode & S_IFMT) == S_IFDIR) {
		for(directory_iterator it(p); !it.end(); ++it) {
			remove_all(p / it.name());
		}
	}
	
	return remove(p);
}

bool create_directory(const path & p) {
	if(p.empty()) {
		return true;
	}
	int ret = mkdir(p.string().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	return !ret || is_directory(p);
}

bool create_directories(const path & p) {
	
	if(p.empty()) {
		return true;
	}
	
	fs::path parent = p.parent();
	if(!exists(parent)) {
		if(!create_directories(parent)) {
			return false;
		}
	}
	
	return create_directory(p);
}

bool copy_file(const path & from_p, const path & to_p, bool overwrite) {
	
	if(!overwrite && exists(to_p)) {
		return false;
	}
	
	fs::ifstream in(from_p, fs::fstream::in | fs::fstream::binary);
	if(!in.is_open()) {
		return false;
	}
	
	fs::ofstream out(to_p, fs::fstream::out | fs::fstream::binary | fs::fstream::trunc);
	if(!out.is_open()) {
		return false;
	}
	
	while(!in.eof()) {
		
		char buf[4096];
		
		if(in.read(buf, sizeof(buf)).bad()) {
			return false;
		}
		
		if(out.write(buf, in.gcount()).fail()) {
			return false;
		}
	}
	
	return true;
}

bool rename(const path & old_p, const path & new_p) {
	return !::rename(old_p.string().c_str(), new_p.string().c_str());
}

directory_iterator::directory_iterator(const fs::path & p) : buf(NULL) {
	
	handle = opendir(p.empty() ? "./" : p.string().c_str());
	
	if(handle) {
		dirent * d;
		do {
			buf = d = readdir(reinterpret_cast<DIR *>(handle));
		} while(d && (!strcmp(d->d_name, ".") || !strcmp(d->d_name, "..")));
	}
};

directory_iterator::~directory_iterator() {
	if(handle) {
		closedir(reinterpret_cast<DIR*>(handle));
	}
}

directory_iterator & directory_iterator::operator++() {
	
	dirent * d;
	do {
		buf = d = readdir(reinterpret_cast<DIR *>(handle));
	} while(d && (!strcmp(d->d_name, ".") || !strcmp(d->d_name, "..")));
	
	return *this;
}

bool directory_iterator::end() {
	return !buf;
}

string directory_iterator::name() {
	return reinterpret_cast<dirent *>(buf)->d_name;
}

#ifndef _DIRENT_HAVE_D_TYPE
#error d_type entry required in struct dirent
#endif

bool directory_iterator::is_directory() {
	return reinterpret_cast<dirent *>(buf)->d_type == DT_DIR;
}

bool directory_iterator::is_regular_file() {
	return reinterpret_cast<dirent *>(buf)->d_type == DT_REG;
}

} // namespace fs

#endif // HAVE_POSIX_FILESYSTEM
