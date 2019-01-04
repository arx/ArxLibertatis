/*
 * Copyright 2013-2018 Arx Libertatis Team (see the AUTHORS file)
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

/*!
 * Defines and functions to identify the host operating system.
 */
#ifndef ARX_PLATFORM_OS_H
#define ARX_PLATFORM_OS_H

#include <string>

#include "platform/Platform.h"

namespace platform {

/*!
 * \brief Get the name and version of the runtime host operating system
 *
 * On POSIX systems this matches the sysname and release members of the uname output.
 *
 * \return the os name or an empty string if it could not be determined.
 */
std::string getOSName();

/*!
 * \brief Get the processor architecture of the runtime host
 *
 * This may be different from the process architecture on multilib systems.
 *
 * On Windows this is either x86_64 or x86.
 * On POSIX systems this matches the machine member of the uname output.
 *
 * \return the CPU architecture or an empty string if it could not be determined.
 */
std::string getOSArchitecture();

/*!
 * \brief Get the distribution name and version of the runtime host operating system
 *
 * \return the distribution name and version or an empty string if not applicable.
 */
std::string getOSDistribution();

/*!
 * \brief Get the name and version of the system C library
 */
std::string getCLibraryVersion();

/*!
 * \brief Get the name and version of the system threading
 */
std::string getThreadLibraryVersion();

/*!
 * \brief Get the branding name of the CPU in the system
 *
 * \return the cpu branding name or an empty string if it could not be determined.
 */
std::string getCPUName();

struct MemoryInfo {
	u64 total;
	u64 available;
};

/*!
 * \brief Get the total and available physical memory size
 *
 * \return the total and avaialbe memory size or 0 if it could not be determined.
 */
MemoryInfo getMemoryInfo();

} // namespace platform

#endif // ARX_PLATFORM_OS_H
