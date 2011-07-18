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
#include <cassert>
#include <algorithm>

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

PakDirectory * PakDirectory::addDirectory(strref path) {
	
	const char * pos = find_first_of(path.begin(), path.end(), DIR_SEP, DIR_SEP + sizeof(DIR_SEP));
	strref name = path.substr(0, pos - path.begin());
	
	PakDirectory * dir = &dirs[name];
	
	if(pos != path.end() && pos + 1 != path.end()) {
		return dir->addDirectory(path.substr(pos - path.begin() + 1));
	} else {
		return dir;
	}
}

static char BADPATHCHAR[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"; // TODO(case-sensitive) remove

PakDirectory * PakDirectory::getDirectory(strref path) {
	
	const char * pos = find_first_of(path.begin(), path.end(), DIR_SEP, DIR_SEP + sizeof(DIR_SEP));
	strref name = path.substr(0, pos - path.begin());
	
	if(name.empty()) {
		return this;
	}
	
	arx_assert(std::find_first_of(name.begin(), name.end(), BADPATHCHAR, BADPATHCHAR + sizeof(BADPATHCHAR)) == name.end()); // TODO(case-sensitive) remove
	
	dirs_iterator dir = dirs.find(name);
	if(dir == dirs.end()) {
		return NULL;
	}
	
	if(pos != path.end() && pos + 1 != path.end()) {
		return dir->second.getDirectory(path.substr(pos - path.begin() + 1));
	} else {
		return &dir->second;
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

PakFile * PakDirectory::getFile(const string & path) {
	
	arx_assert(path[0] != '/');
	
	size_t pos = path.find_last_of(DIR_SEP);
	
	PakDirectory * d = this;
	if(pos != string::npos) {
		d = getDirectory(strref(path, 0, pos));
		if(!d) {
			return NULL;
		}
	}
	
	arx_assert(std::find_first_of(path.begin() + ((pos == string::npos) ? 0 : pos), path.end(), BADPATHCHAR, BADPATHCHAR + sizeof(BADPATHCHAR)) == path.end()); // TODO(case-sensitive) remove
	
	files_iterator file = d->files.find((pos == string::npos) ? path : path.substr(pos + 1));
	if(file == d->files.end()) {
		return NULL;
	}
	
	return file->second;
}
