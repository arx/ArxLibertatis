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

#include "io/fs/FileStream.h"

#include "io/fs/FilePath.h"

#include "platform/Platform.h"
#include "platform/WindowsUtils.h"

namespace fs {

#if ARX_PLATFORM == ARX_PLATFORM_WIN32 && ARX_HAVE_CXX17_FSTREAM_WCHAR

ifstream::ifstream(const path & p, openmode mode)
	: std::ifstream(platform::WideString(p.string()), mode)
{ }

void ifstream::open(const path & p, openmode mode) {
	std::ifstream::open(platform::WideString(p.string()), mode);
}

ofstream::ofstream(const path & p, openmode mode)
	: std::ofstream(platform::WideString(p.string()), mode)
{ }

void ofstream::open(const path & p, openmode mode) {
	std::ofstream::open(platform::WideString(p.string()), mode);
}

fstream::fstream(const path & p, openmode mode)
	: std::fstream(platform::WideString(p.string()), mode)
{ }

void fstream::open(const path & p, openmode mode) {
	std::fstream::open(platform::WideString(p.string()), mode);
}

#else

ifstream::ifstream(const path & p, openmode mode)
	: std::ifstream(p.string().c_str(), mode)
{ }

void ifstream::open(const path & p, openmode mode) {
	std::ifstream::open(p.string().c_str(), mode);
}

ofstream::ofstream(const path & p, openmode mode)
	: std::ofstream(p.string().c_str(), mode)
{ }

void ofstream::open(const path & p, openmode mode) {
	std::ofstream::open(p.string().c_str(), mode);
}

fstream::fstream(const path & p, openmode mode)
	: std::fstream(p.string().c_str(), mode)
{ }

void fstream::open(const path & p, openmode mode) {
	std::fstream::open(p.string().c_str(), mode);
}

#endif

std::istream & read(std::istream & ifs, std::string & buf) {
	while(ifs.good()) {
		char c = static_cast<char>(ifs.get());
		if(c == '\0') {
			break;
		}
		buf.push_back(c);
	}
	return ifs;
}

} // namespace fs
