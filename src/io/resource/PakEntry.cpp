/*
 * Copyright 2011-2021 Arx Libertatis Team (see the AUTHORS file)
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
#include <utility>

#include "io/log/Logger.h"
#include "io/resource/ResourcePath.h"
#include "platform/Platform.h"

PakDirectory * PakDirectory::addDirectory(const res::path & path) {
	
	if(path.empty()) {
		return this;
	}
	
	PakDirectory * dir = this;
	size_t pos = 0;
	while(true) {
		
		size_t end = path.string().find(res::path::dir_sep, pos);
		
		if(end == std::string::npos) {
			return &dir->m_dirs[path.string().substr(pos)];
		}
		dir = &dir->m_dirs[path.string().substr(pos, end - pos)];
		
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
		
		std::string_view name;
		if(end == std::string::npos) {
			name = std::string_view(path.string()).substr(pos);
		} else {
			name = std::string_view(path.string()).substr(pos, end - pos);
		}
		
		auto entry = dir->m_dirs.find(name);
		if(entry == dir->m_dirs.end()) {
			return nullptr;
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
		return nullptr;
	} else if(path.is_up()) {
		LogWarning << "Bad path: " << path;
	}
	
	PakDirectory * dir = this;
	size_t pos = 0;
	while(true) {
		
		size_t end = path.string().find(res::path::dir_sep, pos);
		
		if(end == std::string::npos) {
			auto file = dir->m_files.find(std::string_view(path.string()).substr(pos));
			return (file == dir->m_files.end()) ? nullptr : file->second.get();
		}
		
		auto entry = dir->m_dirs.find(std::string_view(path.string()).substr(pos, end - pos));
		if(entry == dir->m_dirs.end()) {
			return nullptr;
		}
		dir = &entry->second;
		
		pos = end + 1;
	}
	
}

void PakDirectory::addFile(std::string && name, std::unique_ptr<PakFile> file) {
	
	auto result = m_files.emplace(std::move(name), std::unique_ptr<PakFile>());
	if(!result.second) {
		file->m_alternative = std::move(result.first->second);
	}
	result.first->second = std::move(file);
	
}

void PakDirectory::removeFile(std::string_view name) {
	
	if(auto it = m_files.find(name); it != m_files.end()) {
		m_files.erase(it);
	}
	
}

bool PakDirectory::removeDirectory(std::string_view name) {
	
	auto old = m_dirs.find(name);
	
	if(old == m_dirs.end()) {
		return true;
	}
	
	if(old->second.empty()) {
		m_dirs.erase(old);
		return true;
	} else {
		return false;
	}
	
}
