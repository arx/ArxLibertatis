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

#include "io/resource/PakEntry.h"

#include <cstdlib>
#include <algorithm>

#include "io/log/Logger.h"
#include "io/resource/ResourcePath.h"
#include "platform/Platform.h"

PakFile::~PakFile() {
	delete _alternative;
}

std::string PakFile::read() const {
	
	std::string buffer;
	buffer.resize(size());
	
	read(&buffer[0]);
	
	return buffer;
}

PakDirectory::PakDirectory() { }

PakDirectory::~PakDirectory() {
	
	for(files_iterator file = files_begin(); file != files_end(); ++file) {
		delete file->second;
	}
}

PakDirectory * PakDirectory::addDirectory(const res::path & path) {
	
	if(path.empty()) {
		return this;
	}
	
	PakDirectory * dir = this;
	size_t pos = 0;
	while(true) {
		
		size_t end = path.string().find(res::path::dir_sep, pos);
		
		if(end == std::string::npos) {
			return &dir->dirs[path.string().substr(pos)];
		}
		dir = &dir->dirs[path.string().substr(pos, end - pos)];
		
		pos = end + 1;
	}
	
}

#ifdef ARX_DEBUG
static const char BADPATHCHAR[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\\";
#endif

PakDirectory * PakDirectory::getDirectory(const res::path & path) {
	
	arx_assert_msg(path.string().find_first_of(BADPATHCHAR) == std::string::npos,
	               "bad pak path: \"%s\"", path.string().c_str());
	
	if(path.empty()) {
		return this;
	} else if(path.is_up()) {
		LogWarning << "Bad path: " << path;
	}
	
	PakDirectory * dir = this;
	size_t pos = 0;
	while(true) {
		
		size_t end = path.string().find(res::path::dir_sep, pos);
		
		std::string name;
		if(end == std::string::npos) {
			name = path.string().substr(pos);
		} else {
			name = path.string().substr(pos, end - pos);
		}
		
		dirs_iterator entry = dir->dirs.find(name);
		if(entry == dir->dirs.end()) {
			return NULL;
		}
		dir = &entry->second;
		
		if(end == std::string::npos) {
			return dir;
		}
		pos = end + 1;
	};
	
}

PakFile * PakDirectory::getFile(const res::path & path) {
	
	arx_assert_msg(path.string().find_first_of(BADPATHCHAR) == std::string::npos,
	               "bad pak path: \"%s\"", path.string().c_str());
	
	if(path.empty()) {
		return NULL;
	} else if(path.is_up()) {
		LogWarning << "Bad path: " << path;
	}
	
	PakDirectory * dir = this;
	size_t pos = 0;
	while(true) {
		
		size_t end = path.string().find(res::path::dir_sep, pos);
		
		if(end == std::string::npos) {
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

void PakDirectory::addFile(const std::string & name, PakFile * file) {
	
	std::map<std::string, PakFile *>::iterator old = files.find(name);
	
	if(old == files.end()) {
		files[name] = file;
	} else {
		file->_alternative = old->second;
		old->second = file;
	}
}

void PakDirectory::removeFile(const std::string & name) {
	
	std::map<std::string, PakFile *>::iterator old = files.find(name);
	
	if(old != files.end()) {
		delete old->second;
		files.erase(old);
	}
}

bool PakDirectory::removeDirectory(const std::string & name) {
	
	dirs_iterator old = dirs.find(name);
	
	if(old == dirs.end()) {
		return true;
	}
	
	if(old->second.empty()) {
		dirs.erase(old);
		return true;
	} else {
		return false;
	}
}
