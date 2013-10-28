/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#include "Configure.h"

#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <sys/stat.h>
#include <sys/errno.h>
#include <dirent.h>
#include <unistd.h>

#include <boost/algorithm/string/case_conv.hpp>

#include "io/fs/FilePath.h"
#include "io/fs/FileStream.h"

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
	return stat(p.string().c_str(), &buf) ? (u64)-1 : (u64)buf.st_size;
}

bool remove(const path & p) {
	int ret = ::remove(p.string().c_str());
	return !ret || ret == ENOENT || ret == ENOTDIR;
}

bool remove_all(const path & p) {
	
	struct stat buf;
	if(stat(p.string().c_str(), &buf)) {
		return true;
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
	
	path parent = p.parent();
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

bool rename(const path & old_p, const path & new_p, bool overwrite) {
	
	if(!overwrite && exists(new_p)) {
#if ARX_HAVE_PATHCONF && ARX_HAVE_PC_CASE_SENSITIVE
		if(boost::to_lower_copy(old_p.string()) == boost::to_lower_copy(new_p.string())) {
			if(pathconf(old_p.string().c_str(), _PC_CASE_SENSITIVE)) {
				return false; // filesystem is case-sensitive and destination file already exists
			}
		} else {
			return false;
		}
#else
		return false;
#endif
	}
	
	return !::rename(old_p.string().c_str(), new_p.string().c_str());
}

path current_path() {
	
	size_t intitial_length = 1024;
#if ARX_HAVE_PATHCONF && ARX_HAVE_PC_NAME_MAX
	size_t path_max = pathconf(".", _PC_PATH_MAX);
	if(path_max <= 0) {
		intitial_length = 1024;
	} else if(path_max > 10240) {
		intitial_length = 10240;
	} else {
    intitial_length = path_max;
	}
#endif
	
	std::vector<char> buffer(intitial_length);
	
	while(true) {
	
		char * ptr = getcwd(&buffer.front(), buffer.size());
		if(ptr != NULL) {
			return ptr;
		} else if(errno != ERANGE) {
			return path();
		}
		
		buffer.reserve(buffer.size() * 2);
	}
	
}

#if ARX_HAVE_DIRFD && ARX_HAVE_FSTATAT

#define ITERATOR_HANDLE(handle)

#define DIR_HANDLE_INIT(p, h) h
#define DIR_HANDLE(h)         reinterpret_cast<DIR *>(h)
#define DIR_HANDLE_FREE(h)

static mode_t dirstat(void * handle, void * entry) {
	
	arx_assert(entry != NULL);
	int fd = dirfd(DIR_HANDLE(handle));
	arx_assert(fd != -1);
	
	const char * name = reinterpret_cast<dirent *>(entry)->d_name;
	struct stat buf;
	int ret = fstatat(fd, name, &buf, 0);
	arx_assert_msg(ret == 0, "fstatat failed: %d", ret); ARX_UNUSED(ret);
	
	return buf.st_mode;
}

#else

struct iterator_handle {
	fs::path path;
	DIR * handle;
	iterator_handle(const fs::path & p, DIR * h) : path(p), handle(h) { }
};

#define ITERATOR_HANDLE(handle) reinterpret_cast<iterator_handle *>(handle)

#define DIR_HANDLE_INIT(p, h)   new iterator_handle(p, h)
#define DIR_HANDLE(h)           ITERATOR_HANDLE(h)->handle
#define DIR_HANDLE_FREE(h)      delete ITERATOR_HANDLE(h)

static mode_t dirstat(void * handle, void * entry) {
	
	arx_assert(entry != NULL);
	fs::path file = ITERATOR_HANDLE(handle)->path / reinterpret_cast<dirent *>(entry)->d_name;
	
	struct stat buf;
	int ret = stat(file.string().c_str(), &buf);
	arx_assert_msg(ret == 0, "stat failed: %d", ret); ARX_UNUSED(ret);
	
	return buf.st_mode;
}

#endif

static void readdir(void * _handle, void * & _buf) {
	
	DIR * handle = DIR_HANDLE(_handle);
	
	dirent * buf = reinterpret_cast<dirent *>(_buf);
	
	do {
		
		dirent * entry;
		if(readdir_r(handle, buf, &entry) || !entry) {
			std::free(_buf), _buf = NULL;
			return;
		}
		
	} while(!strcmp(buf->d_name, ".") || !strcmp(buf->d_name, ".."));
	
}

directory_iterator::directory_iterator(const path & p) : buf(NULL) {
	
	handle = DIR_HANDLE_INIT(p, opendir(p.empty() ? "./" : p.string().c_str()));
	
	if(DIR_HANDLE(handle)) {
		
		// Allocate a large enough buffer for readdir_r.
		long name_max;
#if ((ARX_HAVE_DIRFD && ARX_HAVE_FPATHCONF) || ARX_HAVE_PATHCONF) && ARX_HAVE_PC_NAME_MAX
#  if ARX_HAVE_DIRFD && ARX_HAVE_FPATHCONF
		name_max = fpathconf(dirfd(DIR_HANDLE(handle)), _PC_NAME_MAX);
#  else
		name_max = pathconf(p.string().c_str(), _PC_NAME_MAX);
#  endif
		if(name_max == -1) {
#  if ARX_HAVE_NAME_MAX
			name_max = std::max(NAME_MAX, 255);
#  else
			arx_assert_msg(false, "cannot determine maximum dirname size");
#  endif
		}
#elif ARX_HAVE_NAME_MAX
		name_max = std::max(NAME_MAX, 255);
#else
#  error "buffer size for readdir_r cannot be determined"
#endif
		size_t size = (size_t)offsetof(dirent, d_name) + name_max + 1;
		if(size < sizeof(dirent)) {
			size = sizeof(dirent);
		}
		buf = std::malloc(size);
		
		readdir(handle, buf);
	}
};

directory_iterator::~directory_iterator() {
	if(handle) {
		closedir(DIR_HANDLE(handle));
		DIR_HANDLE_FREE(handle);
	}
	std::free(buf);
}

directory_iterator & directory_iterator::operator++() {
	arx_assert(buf != NULL);
	
	readdir(handle, buf);
	
	return *this;
}

bool directory_iterator::end() {
	return !buf;
}

std::string directory_iterator::name() {
	arx_assert(buf != NULL);
	return reinterpret_cast<dirent *>(buf)->d_name;
}

bool directory_iterator::is_directory() {
	return ((dirstat(handle, buf) & S_IFMT) == S_IFDIR);
}

bool directory_iterator::is_regular_file() {
	return ((dirstat(handle, buf) & S_IFMT) == S_IFREG);
}

#undef ITERATOR_HANDLE
#undef DIR_HANDLE_INIT
#undef DIR_HANDLE
#undef DIR_HANDLE_FREE

} // namespace fs
