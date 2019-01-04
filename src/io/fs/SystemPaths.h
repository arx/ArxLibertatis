/*
 * Copyright 2012-2017 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_IO_FS_SYSTEMPATHS_H
#define ARX_IO_FS_SYSTEMPATHS_H

#include <vector>
#include <ostream>

#include "io/fs/FilePath.h"
#include "util/cmdline/CommandLine.h"

namespace fs {

/*!
 * \brief Initialize the system resource paths
 *
 * Uses arguments provided on the command line, if any.
 */
ExitStatus initSystemPaths();

//! \return A directory for saves and user-specific data files
const path & getUserDir();

//! \return A directory for config files
const path & getConfigDir();

//! \return Directories for data files
const std::vector<path> & getDataDirs();

/*!
 * \brief Get a list of all data search paths
 *
 * These are all the paths that are considered for the list returned by \ref getDataDirs
 */
std::vector<path> getDataSearchPaths();

/*!
	* \brief Find a resource in the data search path
	*
	* If resource is an absolute path, return it directoly if it exists.
	*
	* If resource is a relative path, look for it in each data directory
	* (ordered by decreasing priority) and return the first full path that
	* exists.
	*
	* \param resource the relative resource name to look for.
	*
	* \return an empty path or an existing file.
	*/
path findDataFile(const path & resource);

} // namespace fs

#endif // ARX_IO_FS_SYSTEMPATHS_H
