/*
 * Copyright 2012-2013 Arx Libertatis Team (see the AUTHORS file)
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

enum ExitStatus {
	ExitSuccess,
	ExitFailure,
	RunProgram
};

namespace fs {

//! Localtions where files are stored on the system
struct SystemPaths {
	
	struct InitParams {
		
		/*! An overwrite value to use for the user dir.
		 *  If this is non-empty, standard search is skipped.
		 *  If this does not exists, the user dir is left empty.
		 */
		path forceUser;
		
		/*! An overwrite value to use for the config dir.
		 *  If this is non-empty, standard search is skipped.
		 *  If this does not exists, the config dir is left empty.
		 */
		path forceConfig;
		
		/*! Additional values for the data search path.
		 *  Will have higher priority than other system paths, but
		 *  higher lower priority than the user dir. 
		 */
		std::vector<path> dataDirs;
		
		bool findData;
		bool displaySearchDirs;
		
		InitParams() : findData(true), displaySearchDirs(false) {}
		
	};
	
	path user; //!< Directory for saves and user-specific data files
	path config; //!< Directory for config files
	std::vector<path> data; //!< Directories for data files
	
	/*!
	 * Initialize the system resource paths, using arguments provided on the
	 * command line, if any.
	 */
	ExitStatus init();
	
	/*!
	 * Initialize the system resource paths using the specified parameters.
	 * @note This version of init() will ignore arguments provided on the
	 * command line.
	 */
	ExitStatus init(const InitParams& initParams);
	
	std::vector<path> getSearchPaths(bool filterMissing = false) const;
	
	/*!
	 * Find a resource in the first data directory
	 *
	 * If resource is an absolute path, return it directoly if it exists.
	 *
	 * If resource is a relative path, look for it in each data directory
	 * (ordered by decreasing priority) and return the first full path that
	 * exists.
	 *
	 * @param resource the relative resource name to look for.
	 *
	 * @return an empty path or an existing file.
	 */
	path find(const path & resource) const;
	
private:
	
	void list(std::ostream & os, const std::string & forceUser = std::string(),
	          const std::string & forceConfig = std::string(),
	          const std::string & forceData = std::string());
	
	std::vector<path> addData_;
	bool findData_;
	
};

extern SystemPaths paths;

} // namespace fs

#endif // ARX_IO_FS_SYSTEMPATHS_H
