/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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

#include "io/Filesystem.h"

#include "io/FileStream.h"

namespace fs {

char * read_file(const path & p, size_t & size) {
	
	fs::ifstream ifs(p, fs::fstream::in | fs::fstream::binary | fs::fstream::ate);
	if(!ifs.is_open()) {
		return NULL;
	}
	
	size = ifs.tellg();
	
	char * buf = new char[size];
	
	if(ifs.seekg(0).read(buf, size).fail()) {
		delete[] buf;
		return NULL;
	}
	
	return buf;
}

} // namespace fs
