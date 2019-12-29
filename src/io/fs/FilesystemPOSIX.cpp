/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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
#include <cerrno>

#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include <boost/algorithm/string/case_conv.hpp>

#include "io/fs/FilePath.h"
#include "io/fs/FileStream.h"

namespace fs {

static FileType stat_to_filetype(const struct stat & buf) {
	
	if((buf.st_mode & S_IFMT) == S_IFDIR) {
		return Directory;
	}
	if((buf.st_mode & S_IFMT) == S_IFREG) {
		return RegularFile;
	}
	
	return SpecialFile;
}

FileType get_type(const path & p) {
	
	if(p.empty()) {
		return Directory;
	}
	
	struct stat buf;
	if(stat(p.string().c_str(), &buf)) {
		return DoesNotExist;
	}
	
	return stat_to_filetype(buf);
}

FileType get_link_type(const path & p) {
	
	if(p.empty()) {
		return Directory;
	}
	
	struct stat buf;
	if(lstat(p.string().c_str(), &buf)) {
		return DoesNotExist;
	}
	
	if((buf.st_mode & S_IFMT) == S_IFLNK) {
		return SymbolicLink;
	}
	
	return stat_to_filetype(buf);
}

std::time_t last_write_time(const path & p) {
	struct stat buf;
	return stat(p.string().c_str(), &buf) ? 0 : buf.st_mtime;
}

u64 file_size(const path & p) {
	struct stat buf;
	return stat(p.string().c_str(), &buf) ? u64(-1) : u64(buf.st_size);
}

bool remove(const path & p) {
	return !unlink(p.string().c_str()) || errno == ENOENT || errno == ENOTDIR;
}

bool remove_directory(const path & p) {
	return !rmdir(p.string().c_str()) || errno == ENOENT || errno == ENOTDIR;
}

bool create_directory(const path & p) {
	if(p.empty()) {
		return true;
	}
	int ret = mkdir(p.string().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	return !ret || is_directory(p);
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

void directory_iterator::read_entry() {
	
	do {
		
		#if ARX_HAVE_THREADSAFE_READDIR
		m_entry = readdir(m_handle);
		if(!m_entry) {
			return;
		}
		#else
		dirent * result;
		if(readdir_r(m_handle, m_entry, &result) || !result) {
			delete[] reinterpret_cast<char *>(m_entry);
			m_entry = NULL;
			return;
		}
		#endif
		
	} while(!strcmp(m_entry->d_name, ".") || !strcmp(m_entry->d_name, ".."));
	
	// We use st_nlink to remember if we already called stat() for this file
	m_info.st_nlink = 0;
	
}

bool directory_iterator::read_info() {
	
	if(m_info.st_nlink) {
		// We already called stat() for this file
		return true;
	}
	
	#if ARX_HAVE_DIRFD && ARX_HAVE_FSTATAT
	m_info.st_nlink = !fstatat(dirfd(m_handle), m_entry->d_name, &m_info, 0);
	#else
	m_info.st_nlink = !stat(m_path / m_entry->d_name, &m_info);
	#endif
	
	return m_info.st_nlink;
}

directory_iterator::directory_iterator(const path & p)
	: m_handle(opendir(p.empty() ? "./" : p.string().c_str()))
	#if !ARX_HAVE_DIRFD || !ARX_HAVE_FSTATAT || !ARX_HAVE_AT_SYMLINK_NOFOLLOW
	, m_path(p)
	#endif
	, m_entry(NULL)
{
	
	if(!m_handle) {
		return;
	}
	
	#if !ARX_HAVE_THREADSAFE_READDIR
	// Allocate a large enough buffer for readdir_r.
	long name_max;
	#if ((ARX_HAVE_DIRFD && ARX_HAVE_FPATHCONF) || ARX_HAVE_PATHCONF) && ARX_HAVE_PC_NAME_MAX
	#  if ARX_HAVE_DIRFD && ARX_HAVE_FPATHCONF
	name_max = fpathconf(dirfd(m_handle), _PC_NAME_MAX);
	#  else
	name_max = pathconf(p.string().c_str(), _PC_NAME_MAX);
	#  endif
	if(name_max == -1) {
		#if ARX_HAVE_NAME_MAX
		name_max = std::max(NAME_MAX, 255);
		#else
		arx_assert_msg(false, "cannot determine maximum dirname size");
		#endif
	}
	#elif ARX_HAVE_NAME_MAX
	name_max = std::max(NAME_MAX, 255);
	#else
	#  error "buffer size for readdir_r cannot be determined"
	#endif
	size_t size = size_t(offsetof(dirent, d_name)) + name_max + 1;
	if(size < sizeof(dirent)) {
		size = sizeof(dirent);
	}
	m_entry = reinterpret_cast<dirent *>(new char[size]);
	#endif // !ARX_HAVE_THREADSAFE_READDIR
	
	read_entry();
	
}

directory_iterator::~directory_iterator() {
	
	if(m_handle) {
		closedir(m_handle);
	}
	
	#if !ARX_HAVE_THREADSAFE_READDIR
	delete[] reinterpret_cast<char *>(m_entry);
	#endif
	
}

directory_iterator & directory_iterator::operator++() {
	
	arx_assert(m_entry != NULL);
	
	read_entry();
	
	return *this;
}

bool directory_iterator::end() {
	return !m_entry;
}

std::string directory_iterator::name() {
	
	arx_assert(m_entry != NULL);
	
	return m_entry->d_name;
}

FileType directory_iterator::type() {
	
	arx_assert(m_entry != NULL);
	
	#if defined(DT_DIR)
	if(m_entry->d_type == DT_DIR) {
		return Directory;
	}
	#endif
	#if defined(DT_REG)
	if(m_entry->d_type == DT_REG) {
		return RegularFile;
	}
	#endif
	#if defined(DT_UNKNOWN) && defined(DT_LNK)
	if(m_entry->d_type != DT_UNKNOWN && m_entry->d_type != DT_LNK) {
		return SpecialFile;
	}
	#endif
	
	if(!read_info()) {
		return DoesNotExist;
	}
	
	return stat_to_filetype(m_info);
}

FileType directory_iterator::link_type() {
	
	arx_assert(m_entry != NULL);
	
	#if defined(DT_DIR)
	if(m_entry->d_type == DT_DIR) {
		return Directory;
	}
	#endif
	#if defined(DT_REG)
	if(m_entry->d_type == DT_REG) {
		return RegularFile;
	}
	#endif
	#if defined(DT_LNK)
	if(m_entry->d_type == DT_LNK) {
		return SymbolicLink;
	}
	#endif
	#if defined(DT_UNKNOWN)
	if(m_entry->d_type != DT_UNKNOWN && m_entry->d_type != DT_LNK) {
		return SpecialFile;
	}
	#endif
	
	struct stat buf;
	#if ARX_HAVE_DIRFD && ARX_HAVE_FSTATAT && ARX_HAVE_AT_SYMLINK_NOFOLLOW
	int ret = fstatat(dirfd(m_handle), m_entry->d_name, &buf, AT_SYMLINK_NOFOLLOW);
	#else
	int ret = lstat(m_path / m_entry->d_name, &buf);
	#endif
	if(ret) {
		return DoesNotExist;
	}
	
	if((buf.st_mode & S_IFMT) == S_IFLNK) {
		return SymbolicLink;
	}
	
	return stat_to_filetype(buf);
}

std::time_t directory_iterator::last_write_time() {
	
	arx_assert(m_entry != NULL);
	
	if(!read_info()) {
		return 0;
	}
	
	return m_info.st_mtime;
}

u64 directory_iterator::file_size() {
	
	arx_assert(m_entry != NULL);
	
	if(!read_info()) {
		return u64(-1);
	}
	
	return u64(m_info.st_size);
}

} // namespace fs
