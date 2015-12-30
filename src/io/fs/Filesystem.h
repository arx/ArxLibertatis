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

#ifndef ARX_IO_FS_FILESYSTEM_H
#define ARX_IO_FS_FILESYSTEM_H

#include <stddef.h>
#include <ctime>
#include <string>

#include "platform/Platform.h"

namespace fs {

class path;

/*!
 * \brief Check if a file (directory or regular file) exists
 *
 * \return true if the file exists, false if it doesn't exist or there was an error
 */
bool exists(const path & p);

/*!
 * \brief Check if a path points to a directory
 *
 * \return true if the p exists and is a directory, false otherwise
 */
bool is_directory(const path & p);

/*!
 * \brief Check if a path points to a regular file
 *
 * \return true if the p exists and is a regular file, false otherwise.
 */
bool is_regular_file(const path & p);

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
 * \brief Remove a file or empty directory
 *
 * \return true if the file was removed or didn't exist.
 */
bool remove(const path & p);

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
 * \brief Read a file into memory
 *
 * \param p The file to load.
 * \param size Will receive the size of the loaded file.
 *
 * \return a new[]-allocated buffer containing the file data or NULL on error.
 */
char * read_file(const path & p, size_t & size);

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
 *
 * \return true if the write succeeded
 */
bool write(const path & p, const char * contents, size_t size);

/*!
 * \brief Write an \ref std::string into a file
 *
 * \param p The file to write to.
 *
 * \return true if the write succeeded
 */
bool write(const path & p, const std::string & contents);

/*!
 * \brief Get the current working directory
 */
path current_path();

class directory_iterator {
	
	directory_iterator operator++(int dummy); //!< prevent postfix ++
	
	//! prevent assignment
	directory_iterator & operator=(const directory_iterator &);
	directory_iterator(const directory_iterator &);
	
	void * handle;
	void * buf;
	
public:
	
	explicit directory_iterator(const path & p);
	
	~directory_iterator();
	
	directory_iterator & operator++();
	
	bool end();
	
	std::string name();
	
	bool is_directory();
	
	bool is_regular_file();
	
};

}

#endif // ARX_IO_FS_FILESYSTEM_H
