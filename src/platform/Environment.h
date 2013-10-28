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

#ifndef ARX_PLATFORM_ENVIRONMENT_H
#define ARX_PLATFORM_ENVIRONMENT_H

#include <string>

#include "platform/Platform.h"

namespace fs { class path; }

namespace platform {

std::string expandEnvironmentVariables(const std::string & in);

bool getSystemConfiguration(const std::string & name, std::string & result);

void defineSystemDirectories(const char * argv0);

//! Get the path to the current running executable if possible or an empty string otherwise.
fs::path getExecutablePath();

/*!
 * Get the full path to a helper executable
 *
 * Tries to find a helper executable in the same directory as the current program, in the
 * parent directory, or in the libexec directory in the prefix where arx is installed.
 * If found, returns a full path to the executable.
 * Otherwise, returns a relative path containing only the executable name.
 *
 * @return a path or name suitable for CreateProcess(), exec*p() or system() calls.
 */
fs::path getHelperExecutable(const std::string & name);

#if ARX_PLATFORM != ARX_PLATFORM_WIN32
static const char * const env_list_seperators = ":";
#else
static const char * const env_list_seperators = ";";
#endif

} // namespace platform

#endif // ARX_PLATFORM_ENVIRONMENT_H
