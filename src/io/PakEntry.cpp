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

#include "io/PakEntry.h"

#include <cstdlib>
#include <cstring>
#include <algorithm>

#include "io/log/Logger.h"
#include "io/FilePath.h"
#include "platform/Platform.h"

using std::string;
using std::find_first_of;
using std::malloc;

PakFile::~PakFile() {
	
	if(_alternative) {
		delete _alternative;
	}
}

char * PakFile::readAlloc() const {
	
	char * buffer = (char*)malloc(size());
	
	read(buffer);
	
	return buffer;
}

PakDirectory::PakDirectory() { }

PakDirectory::~PakDirectory() {
	
	for(files_iterator file = files_begin(); file != files_end(); ++file) {
		delete file->second;
	}
}

PakDirectory * PakDirectory::addDirectory(const fs::path & path) {
	
	if(path.empty()) {
		return this;
	}
	
	PakDirectory * dir = this;
	size_t pos = 0;
	while(true) {
		
		size_t end = path.string().find(fs::path::dir_sep, pos);
		
		if(end == string::npos) {
			return &dir->dirs[path.string().substr(pos)];
		}
		dir = &dir->dirs[path.string().substr(pos, end - pos)];
		
		pos = end + 1;
	}
	
}

static char BADPATHCHAR[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\\"; // TODO(case-sensitive) remove

PakDirectory * PakDirectory::getDirectory(const fs::path & path) {
	
	arx_assert_msg(std::find_first_of(path.string().begin(), path.string().end(), BADPATHCHAR, BADPATHCHAR + sizeof(BADPATHCHAR)) == path.string().end(), "bad pak path: \"%s\"", path.string().c_str()); ARX_UNUSED(BADPATHCHAR); // TODO(case-sensitive) remove
	
	if(path.empty()) {
		return this;
	} else if(path.is_up()) {
		LogWarning << "bad path: " << path;
	}
	
	PakDirectory * dir = this;
	size_t pos = 0;
	while(true) {
		
		size_t end = path.string().find(fs::path::dir_sep, pos);
		
		string name;
		if(end == string::npos) {
			name = path.string().substr(pos);
		} else {
			name = path.string().substr(pos, end - pos);
		}
		
		dirs_iterator entry = dir->dirs.find(name);
		if(entry == dir->dirs.end()) {
			return NULL;
		}
		dir = &entry->second;
		
		if(end == string::npos) {
			return dir;
		}
		pos = end + 1;
	};
	
}

PakFile * PakDirectory::getFile(const fs::path & path) {
	
	arx_assert_msg(std::find_first_of(path.string().begin(), path.string().end(), BADPATHCHAR, BADPATHCHAR + sizeof(BADPATHCHAR)) == path.string().end(), "bad pak path: \"%s\"", path.string().c_str()); ARX_UNUSED(BADPATHCHAR); // TODO(case-sensitive) remove
	
	if(path.empty()) {
		return NULL;
	} if(path.is_up()) {
		LogWarning << "bad path: " << path;
	}
	
	PakDirectory * dir = this;
	size_t pos = 0;
	while(true) {
		
		size_t end = path.string().find(fs::path::dir_sep, pos);
		
		if(end == string::npos) {
			files_iterator file = dir->files.find(path.string().substr(pos));
			return (file == dir->files.end()) ? NULL : file->second;
		}
		
		dirs_iterator entry = dir->dirs.find(path.string().substr(pos, end - pos));
		if(entry == dir->dirs.end()) {
			return NULL;
		}
		dir = &entry->second;
		
		pos = end + 1;
	}
	
}

void PakDirectory::addFile(const string & name, PakFile * file) {
	
	std::map<std::string, PakFile *>::iterator old = files.find(name);
	
	if(old == files.end()) {
		files[name] = file;
	} else {
		file->_alternative = old->second;
		old->second = file;
	}
}

void PakDirectory::removeFile(const string & name) {
	
	std::map<std::string, PakFile *>::iterator old = files.find(name);
	
	if(old != files.end()) {
		delete old->second;
		files.erase(old);
	}
}
