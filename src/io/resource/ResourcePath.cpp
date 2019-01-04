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

#include "io/resource/ResourcePath.h"

#include <cctype>
#include <algorithm>

#include "platform/Platform.h"

namespace res {

const char path::dir_or_ext_sep[] = "/.";

namespace {

inline bool is_path_up(const std::string & str, size_t pos) {
	return (str.length() == pos + 2 && str[pos] == '.' && str[pos + 1] == '.')
	       || (str.length() >= pos + 3 && str[pos] == '.' && str[pos + 1] == '.'
	           && str[pos + 2] == path::dir_sep);
}

} // anonymous namespace

#ifdef ARX_DEBUG

void path::check() const {
	
	arx_assert_msg(pathstr.find('\\') == std::string::npos, "bad path: \"%s\"", pathstr.c_str());
	
	size_t pos = 0;
	while(is_path_up(pathstr, pos)) {
		pos += 3;
	}
	
	while(pos < pathstr.length()) {
		
		size_t next = pathstr.find(dir_sep, pos);
		if(next == std::string::npos) {
			next = pathstr.length();
		} else {
			arx_assert_msg(next + 1 != pathstr.length(), "bad path: \"%s\"", pathstr.c_str());
		}
		
		arx_assert_msg(next > pos, "bad path: \"%s\"", pathstr.c_str());
		
		arx_assert_msg(next - pos > 1 || pathstr[pos] != '.', "bad path: \"%s\"", pathstr.c_str());
		
		arx_assert_msg(next - pos != 2 || pathstr[pos] != '.' || pathstr[pos + 1] != '.',
		               "bad path: \"%s\"", pathstr.c_str());
		
		pos = next + 1;
	}
	
}

#endif // ARX_DEBUG

path path::resolve(const path & base, const path & branch) {
	
	if(base.empty()) {
		return branch;
	}
	
	size_t bpos = 0;
	size_t apos = base.pathstr.length();
	while(true) {
		
		size_t dirpos = base.pathstr.find_last_of(dir_sep, apos - 1);
		
		if(is_path_up(base.pathstr, (dirpos == std::string::npos) ? 0 : dirpos + 1)) {
			return path(base.pathstr.substr(0, apos) + dir_sep + branch.pathstr.substr(bpos));
		}
		
		if(dirpos == std::string::npos) {
			if(bpos + 3 >= branch.pathstr.length()) {
				return path();
			} else {
				return branch.pathstr.substr(bpos + 3);
			}
		}
		
		apos = dirpos, bpos += 3;
		
		if(!is_path_up(branch.pathstr, bpos)) {
			if(bpos >= branch.pathstr.length()) {
				return path(base.pathstr.substr(0, apos));
			} else {
				return path(base.pathstr.substr(0, apos) + dir_sep + branch.pathstr.substr(bpos));
			}
		}
		
	}
	
}

path path::operator/(const path & other) const {
	if(other.is_up()) {
		return resolve(*this, other);
	} else {
		return empty() ? other : other.empty() ? *this : path(pathstr + dir_sep + other.pathstr);
	}
}

path & path::operator/=(const path & other) {
	if(other.is_up()) {
		return *this = resolve(*this, other);
	} else if(other.empty()) {
		return *this;
	} else {
		return empty() ? (*this = other.pathstr) : ((pathstr += dir_sep).append(other.pathstr), *this);
	}
}

std::string path::basename() const {
	if(!has_info()) {
		return empty() ? std::string() : std::string("..");
	}
	size_t extpos = pathstr.find_last_of(dir_or_ext_sep);
	if(extpos == std::string::npos) {
		return pathstr;
	} else if(pathstr[extpos] != ext_sep) {
		return pathstr.substr(extpos + 1);
	} else if(extpos == 0) {
		return std::string();
	}
	size_t dirpos = pathstr.find_last_of(dir_sep, extpos - 1);
	if(dirpos == std::string::npos) {
		return pathstr.substr(0, extpos);
	} else {
		return pathstr.substr(dirpos + 1, extpos - dirpos - 1);
	}
}

std::string path::ext() const {
	if(!has_info()) {
		return std::string();
	}
	size_t extpos = pathstr.find_last_of(dir_or_ext_sep);
	if(extpos == std::string::npos || pathstr[extpos] != ext_sep) {
		return std::string();
	} else {
		return pathstr.substr(extpos);
	}
}

path & path::set_ext(const std::string & ext) {
	arx_assert_msg(ext.empty()
	               || (ext[0] != dir_sep
	                   && ext.find_first_of(dir_or_ext_sep, 1) == std::string::npos),
	               "bad file ext: \"%s\"", ext.c_str());
	if(!has_info() && !empty()) {
		return *this;
	}
	size_t extpos = pathstr.find_last_of(dir_or_ext_sep);
	if(extpos == std::string::npos || pathstr[extpos] != ext_sep) {
		return (((ext.empty() || ext[0] != ext_sep) ? (pathstr += ext_sep) : pathstr).append(ext), *this);
	} else {
		if(ext.empty() || ext[0] != ext_sep) {
			pathstr.resize(extpos + 1 + ext.size());
			std::copy(ext.begin(), ext.end(), pathstr.begin() + extpos + 1);
		} else {
			pathstr.resize(extpos + ext.size());
			std::copy(ext.begin() +  1, ext.end(), pathstr.begin() + extpos + 1);
		}
		return *this;
	}
}

path & path::remove_ext() {
	if(!has_info()) {
		return *this;
	}
	size_t extpos = pathstr.find_last_of(dir_or_ext_sep);
	if(extpos != std::string::npos && pathstr[extpos] == ext_sep) {
		pathstr.resize(extpos);
	}
	return *this;
}

path & path::set_filename(const std::string & filename) {
	arx_assert_msg(!filename.empty() && filename != "." && filename != ".."
	               && filename.find(dir_sep) == std::string::npos,
	               "bad filename: \"%s\"", filename.c_str());
	if(!has_info()) {
		return ((empty() ? pathstr = filename : (pathstr += dir_sep).append(filename)), *this);
	}
	size_t dirpos = pathstr.find_last_of(dir_sep);
	if(dirpos == std::string::npos) {
		return (*this = filename);
	} else {
		pathstr.resize(dirpos + 1 + filename.size());
		std::copy(filename.begin(), filename.end(), pathstr.begin() + dirpos + 1);
		return *this;
	}
}

path & path::set_basename(const std::string & basename) {
	
	arx_assert_msg(!basename.empty() && basename != "." && basename != ".."
	               && basename.find(dir_sep) == std::string::npos,
	               "bad basename: \"%s\"", basename.c_str());
	
	if(!has_info()) {
		return ((empty() ? pathstr = basename : (pathstr += dir_sep).append(basename)), *this);
	}
	
	size_t extpos = pathstr.find_last_of(dir_or_ext_sep);
	
	// handle paths without an extension.
	if(extpos == std::string::npos) {
		return (*this = basename);
	} else if(pathstr[extpos] != ext_sep) {
		pathstr.resize(extpos + 1 + basename.size());
		std::copy(basename.begin(), basename.end(), pathstr.begin() + extpos + 1);
		return *this;
	}
	
	size_t dirpos = (extpos == 0) ? std::string::npos : pathstr.find_last_of(dir_sep, extpos - 1);
	
	if(dirpos == std::string::npos) { // no parent path
		pathstr = basename + pathstr.substr(extpos);
	} else {
		pathstr = pathstr.substr(0, dirpos + 1) + basename + pathstr.substr(extpos);
	}
	
	return *this;
}

path & path::append_basename(const std::string & basename_part) {
	
	arx_assert_msg(basename_part != "." && basename_part != ".."
	               && basename_part.find(dir_sep) == std::string::npos,
	               "bad basename: \"%s\"", basename_part.c_str());
	
	if(!has_info()) {
		return ((empty() ? pathstr = basename_part : (pathstr += dir_sep).append(basename_part)), *this);
	}
	
	size_t extpos = pathstr.find_last_of(dir_or_ext_sep);
	if(extpos == std::string::npos || pathstr[extpos] != ext_sep) {
		return (pathstr += basename_part, *this);
	}
	size_t len = pathstr.length();
	pathstr.resize(pathstr.length() + basename_part.length());
	std::copy_backward(pathstr.begin() + extpos, pathstr.begin() + len, pathstr.end());
	std::copy(basename_part.begin(), basename_part.end(), pathstr.begin() + extpos);
	return *this;
}

path & path::append(const std::string & str) {
	
	arx_assert_msg(str != "." && str != ".." && str.find(dir_sep) == std::string::npos,
	               "cannot append: \"%s\"", str.c_str());
	
	pathstr += str;
	return *this;
}

bool path::has_ext(const std::string & str) const {
	
	arx_assert_msg(str.empty()
	               || (str[0] != dir_sep
	                   && str.find_first_of(dir_or_ext_sep, 1) == std::string::npos),
	               "bad file ext: \"%s\"", str.c_str());
	
	if(!has_info()) {
		return false;
	}
	
	size_t extpos = pathstr.find_last_of(dir_or_ext_sep);
	if(extpos == std::string::npos || pathstr[extpos] != ext_sep) {
		return false;
	} else if(str.empty()) {
		return true;
	} else if(str[0] == ext_sep) {
		return !pathstr.compare(extpos + 1, pathstr.length() - extpos - 1, str, 1, str.length() - 1);
	} else {
		return !pathstr.compare(extpos + 1, pathstr.length() - extpos - 1, str);
	}
}

path path::load(const std::string & str) {
	
	std::string copy;
	copy.resize(str.length());
	
	size_t istart = 0, ostart = 0;
	while(istart < str.length()) {
		
		size_t pos = str.find_first_of("/\\", istart);
		if(pos == std::string::npos) {
			pos = str.length();
		}
		
		size_t start = istart;
		istart = pos + 1;
		
		if(pos == start) {
			// double slash
			continue;
		}
		
		if(pos - start == 1 && str[start] == '.') {
			// '.'
			continue;
		}
		
		if(pos - start == 2 && str[start] == '.' && str[start + 1] == '.') {
			// '..'
			if(ostart == 0) {
				copy[ostart++] = '.', copy[ostart++] = '.';
			} else {
				size_t last = copy.find_last_of('/', ostart - 1);
				if(last == std::string::npos) {
					if(ostart == 2 && copy[0] == '.' && copy[1] == '.') {
						copy[ostart++] = '/', copy[ostart++] = '.', copy[ostart++] = '.';
					} else {
						ostart = 0;
					}
				} else if(ostart - last - 1 == 2 && copy[last + 1] == '.' && copy[last + 2] == '.') {
					copy[ostart++] = '/', copy[ostart++] = '.', copy[ostart++] = '.';
				} else {
					ostart = last;
				}
			}
			continue;
		}
		
		if(ostart != 0) {
			copy[ostart++] = '/';
		}
		
		for(size_t p = start; p < pos; p++) {
			copy[ostart++] = std::tolower(str[p]);
		}
		
	}
	
	copy.resize(ostart);
	
	return copy;
}

} // namespace res
