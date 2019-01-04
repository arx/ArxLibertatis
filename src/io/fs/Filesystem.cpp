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

#include "io/fs/Filesystem.h"

#include <iterator>

#include "io/fs/FileStream.h"

namespace fs {

std::string read(const path & p) {
	
	std::string result;
	
	fs::ifstream ifs(p, fs::fstream::in | fs::fstream::binary | fs::fstream::ate);
	if(ifs.is_open()) {
		result.resize(ifs.tellg());
		ifs.seekg(0).read(&result[0], result.size());
	} else {
		ifs.open(p, fs::fstream::in | fs::fstream::binary);
		if(ifs.is_open()) {
			result.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
		}
	}
	
	if(ifs.fail()) {
		result.clear();
	}
	
	return result;
}

bool write(const path & p, const char * contents, size_t size) {
	
	fs::ofstream ofs(p, fs::fstream::out | fs::fstream::binary | fs::fstream::trunc);
	if(!ofs.is_open()) {
		return false;
	}
	
	return !ofs.write(contents, size).fail();
}

bool write(const path & p, const std::string & contents) {
	return write(p, contents.data(), contents.size());
}

} // namespace fs
