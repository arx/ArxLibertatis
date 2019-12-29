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

#define try
#define catch(a) if(false)
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#undef try
#undef catch

#include "io/fs/FilePath.h"

namespace fs {

namespace fs_boost = boost::filesystem;

static FileType status_to_filetype(const fs_boost::file_status & buf) {
	
	if(fs_boost::exists(buf)) {
		return DoesNotExist;
	}
	if(fs_boost::is_directory(buf)) {
		return Directory;
	}
	if(fs_boost::is_regular_file(buf)) {
		return RegularFile;
	}
	
	return SpecialFile;
}

FileType get_type(const path & p) {
	
	if(p.empty()) {
		return Directory;
	}
	
	boost::system::error_code ec;
	fs_boost::file_status buf = fs_boost::status(p.string(), ec);
	if(ec) {
		return DoesNotExist;
	}
	
	return status_to_filetype(buf);
}

std::time_t last_write_time(const path & p) {
	boost::system::error_code ec;
	std::time_t time = fs_boost::last_write_time(p.string(), ec);
	return ec ? 0 : time;
}

u64 file_size(const path & p) {
	boost::system::error_code ec;
	uintmax_t size = fs_boost::file_size(p.string(), ec);
	return ec ? u64(-1) : u64(size);
}

bool remove(const path & p) {
	boost::system::error_code ec;
	fs_boost::remove(p.string(), ec);
	return !ec || ec == boost::system::no_such_file_or_directory || ec == boost::system::not_a_directory;
}

bool create_directory(const path & p) {
	boost::system::error_code ec;
	fs_boost::create_directory(p.string(), ec);
	return !ec;
}

bool copy_file(const path & from_p, const path & to_p, bool overwrite) {
	boost::system::error_code ec;
	BOOST_SCOPED_ENUM(fs_boost::copy_option) o;
	if(overwrite) {
		o = fs_boost::copy_option::overwrite_if_exists;
	} else {
		o = fs_boost::copy_option::fail_if_exists;
	}
	fs_boost::copy_file(from_p.string(), to_p.string(), o, ec);
	return !ec;
}

bool rename(const path & old_p, const path & new_p, bool overwrite) {
	if(!overwrite && exists(new_p)) {
		return false;
	}
	boost::system::error_code ec;
	fs_boost::rename(old_p.string(), new_p.string(), ec);
	return !ec;
}

path current_path() {
	return fs_boost::current_path().string();
}

directory_iterator::directory_iterator(const path & p) {
	boost::system::error_code ec;
	m_handle = new fs_boost::directory_iterator(p.empty() ? "./" : p.string(), ec);
	if(ec) {
		delete reinterpret_cast<fs_boost::directory_iterator *>(m_handle);
		m_handle = new fs_boost::directory_iterator();
	}
}

directory_iterator::~directory_iterator() {
	delete reinterpret_cast<fs_boost::directory_iterator *>(m_handle);
}

directory_iterator & directory_iterator::operator++() {
	arx_assert(!end());
	boost::system::error_code ec;
	(*reinterpret_cast<fs_boost::directory_iterator *>(m_handle)).increment(ec);
	if(ec) {
		delete reinterpret_cast<fs_boost::directory_iterator *>(m_handle);
		m_handle = new fs_boost::directory_iterator();
	}
	return *this;
}

bool directory_iterator::end() {
	return (*reinterpret_cast<fs_boost::directory_iterator *>(m_handle) == fs_boost::directory_iterator());
}

std::string directory_iterator::name() {
	arx_assert(!end());
	return (*reinterpret_cast<fs_boost::directory_iterator *>(m_handle))->path().filename().string();
}

FileType directory_iterator::type() {
	
	arx_assert(!end());
	
	boost::system::error_code ec;
	fs_boost::file_status buf = (*reinterpret_cast<fs_boost::directory_iterator *>(m_handle))->status(ec);
	if(ec) {
		return DoesNotExist;
	}
	
	return status_to_filetype(buf);
}

} // namespace fs
