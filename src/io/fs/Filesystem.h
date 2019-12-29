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

#ifndef ARX_IO_FS_FILESYSTEM_H
#define ARX_IO_FS_FILESYSTEM_H

#include <stddef.h>
#include <ctime>
#include <string>

#include "Configure.h"
#if ARX_HAVE_POSIX_FILESYSTEM
#include <sys/stat.h>
#include <dirent.h>
#if ARX_HAVE_AT_SYMLINK_NOFOLLOW
#include <fcntl.h>
#endif
#elif ARX_HAVE_WIN32_FILESYSTEM
#include <windows.h>
#else
#include <boost/filesystem/directory.hpp>
#endif

#include "platform/Platform.h"

namespace fs {

class path;

enum FileType {
	DoesNotExist,
	Directory,
	RegularFile,
	SymbolicLink,
	SpecialFile
};

/*!
 * \brief Check if a path exists and determine the type
 *
 * Never returns \ref SymbolicLink.
 */
FileType get_type(const path & p);

/*!
 * \brief Check if a path exists and determine the link type
 *
 * \return the file type or \ref SymbolicLink if p is a symbolic link
 */
FileType get_link_type(const path & p);

/*!
 * \brief Check if a file (directory or regular file) exists
 *
 * \return true if the file exists, false if it doesn't exist or there was an error
 */
inline bool exists(const path & p) {
	return get_type(p) != DoesNotExist;
}

/*!
 * \brief Check if a path points to a directory
 *
 * \return true if the p exists and is a directory, false otherwise
 */
inline bool is_directory(const path & p) {
	return get_type(p) == Directory;
}

/*!
 * \brief Check if a path points to a regular file
 *
 * \return true if the p exists and is a regular file, false otherwise.
 */
inline bool is_regular_file(const path & p) {
	return get_type(p) == RegularFile;
}

/*!
 * \brief Get the last write time of a file
 *
 * \return the last write time or 0 if there was an error (file doesn't exist, ...).
 */
std::time_t last_write_time(const path & p);

/*!
 * \brief Get the size of a file
 *
 * \return the filesize or (u64)-1 if there was an error (file doesn't exist, ...).
 */
u64 file_size(const path & p);

/*!
 * \brief Remove a file
 *
 * \return true if the file was removed or didn't exist.
 */
bool remove(const path & p);

/*!
 * \brief Remove an empty directory
 *
 * \return true if the file was removed or didn't exist.
 */
bool remove_directory(const path & p);

/*!
 * \brief Recursively remove a file or directory
 *
 * \return true if the file was removed or didn't exist.
 */
bool remove_all(const path & p);

/*!
 * \brief Create a single directory
 *
 * p.parent() must exist and be a directory.
 *
 * \return true if the directory was created or false if there was an error.
 */
bool create_directory(const path & p);

/*!
 * \brief Create a directory hierarchy
 *
 * All ancestors of p must either be a directory or not exist.
 *
 * \return true if the directory was created or false if there was an error.
 */
bool create_directories(const path & p);

/*!
 * \brief Copy a regular file
 *
 * from_p must exist and be a regular file.
 * to_p.parent() must exist and be a directory.
 * new_p must not be a directory, even if overwrite is true
 *
 * \return true if the file was copied or false if there was an error.
 */
bool copy_file(const path & from_p, const path & to_p, bool overwrite = false);

/*!
 * \brief Move a regular file or directory
 *
 * old_p must exist.
 * new_p.parent() must exist and be a directory.
 * new_p must not be a directory, even if overwrite is true
 *
 * \return true if the file was copied or false if there was an error.
 */
bool rename(const path & old_p, const path & new_p, bool overwrite = false);

/*!
 * \brief Read a file into an \ref std::string
 *
 * \param p The file to load.
 *
 * \return a string containing the file's contents
 */
std::string read(const path & p);

/*!
 * \brief Write a string into a file
 *
 * \param p The file to write to.
 * \param contents The content to write
 * \param size The size of the content to write
 *
 * \return true if the write succeeded
 */
bool write(const path & p, const char * contents, size_t size);

/*!
 * \brief Write an \ref std::string into a file
 *
 * \param p The file to write to.
 * \param contents The content to write
 *
 * \return true if the write succeeded
 */
bool write(const path & p, const std::string & contents);

/*!
 * \brief Get the current working directory
 */
path current_path();

/*!
 * \brief Class for iterating over the contents of a directory
 */
class directory_iterator {
	
	//! Prevent postfix ++
	
	const directory_iterator operator++(int dummy);
	
	//! Prevent assignment
	directory_iterator & operator=(const directory_iterator &);
	
	//! Prevent copy construction
	directory_iterator(const directory_iterator &);
	
	#if ARX_HAVE_POSIX_FILESYSTEM
	
	DIR * m_handle;
	#if !ARX_HAVE_DIRFD || !ARX_HAVE_FSTATAT || !ARX_HAVE_AT_SYMLINK_NOFOLLOW
	fs::path m_path;
	#endif
	dirent * m_entry;
	struct stat m_info;
	
	void read_entry();
	bool read_info();
	
	#elif ARX_HAVE_WIN32_FILESYSTEM
	
	HANDLE m_handle;
	WIN32_FIND_DATAW m_data;
	
	#else
	
	boost::filesystem::directory_iterator m_it;
	
	#endif
	
public:
	
	/*!
	 * \brief Start iterating over a directory
	 *
	 * If there was an error or the directoy contains no entries, \ref end() will return \c false.
	 * Otherwise, \ref end() will return \c true and information about the first directory entry
	 * can be queried using \ref name(), \ref is_directory() and \ref is_regular_file().
	 */
	explicit directory_iterator(const path & p);
	
	~directory_iterator();
	
	/*!
	 * \brief Advance to the next directory entry
	 *
	 * If there are no more directory entries, \ref end() will now return \c false.
	 * Otherwise, \ref end() will return \c true and information about the first directory entry
	 * can be queried using \ref name(), \ref is_directory() and \ref is_regular_file().
	 *
	 * \ref end() == \c false
	 *
	 * \remarks
	 * It is not safe to advace a single \ref directory_iterator may not be advanced from multiple
	 * threads or access it while another thread is advancing it.
	 */
	directory_iterator & operator++();
	
	/*!
	 * \brief Check if we have reached the end of the directory
	 *
	 * \return \c true iff the current directory entry is valid.
	 */
	bool end();
	
	/*!
	 * \brief Get the name of the current directory entry
	 *
	 * \return The plain filename of the directory entry - if you want a full path you will need
	 * to compose it using the path of the directory you are iterating over.
	 *
	 * \ref end() == \c false
	 */
	std::string name();
	
	/*!
	 * \brief Get the type of the current directory entry (file or subdirectory)
	 *
	 * Never returns \ref SymbolicLink.
	 */
	FileType type();
	
	/*!
	 * \brief Get the link type of the current directory entry
	 *
	 * \return the file type or \ref SymbolicLink if p is a symbolic link
	 */
	FileType link_type();
	
	/*!
	 * \brief Check if the current directory entry is a subdirectory
	 *
	 * The result of this function is equivalent to calling \ref fs::is_directory with the
	 * current file, but is potentially faster since it normally does not require additional syscalls.
	 *
	 * \ref end() == \c false
	 */
	bool is_directory() {
		return type() == Directory;
	}
	
	/*!
	 * \brief Check if the current directory entry is a plain file
	 *
	 * The result of this function is equivalent to calling \ref fs::is_regular_file with the
	 * current file, but is potentially faster since it normally does not require additional syscalls.
	 *
	 * \ref end() == \c false
	 */
	bool is_regular_file() {
		return type() == RegularFile;
	}
	
	/*!
	 * \brief Get the last write time of the current directory entry
	 *
	 * \return the last write time or 0 if there was an error
	 */
	std::time_t last_write_time();
	
	/*!
	 * \brief Get the file size of the current directory entry
	 *
	 * \return the filesize or (u64)-1 if there was an error
	 */
	u64 file_size();
	
};

} // namespace fs

#endif // ARX_IO_FS_FILESYSTEM_H
