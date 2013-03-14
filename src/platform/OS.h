/*
 * Copyright 2013 Arx Libertatis Team (see the AUTHORS file)
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

namespace platform {

//! @return the name and version of the runtime host operating system
std::string getOSName();

//! @return a string identifying the runtime host operating system's architecture
std::string getOSArchitecture();

/*!
 * Get the distribution name and version of the runtime host operating system.
 *
 * @return the distribution name and version or an empty string if not applicable.
 */
std::string getOSDistribution();

} // namespace platform

#endif // ARX_PLATFORM_OS_H
