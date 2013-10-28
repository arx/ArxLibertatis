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

enum ExitStatus {
	ExitSuccess,
	ExitFailure,
	RunProgram
};

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
 * @return a path or name suitable for CreateProcess(), execlp() or system() calls.
 */
fs::path getHelperExecutable(const std::string & name);

/*!
 * Start another executable and wait for it to finish.
 * The executable's standard output/error is discarded.
 *
 * @param exe  the program to run, either an absolute path or a program name in the PATH.
 * @param args program arguments. The first arguments should be the program name/path and
 *             the last argument should be NULL.
 *
 * @return the programs exit code or a negative value on error.
 */
int run(const fs::path & exe, const char * const args[]);

/*!
 * Start another executable without waiting for it to finish.
 * The executable's standard output/error is discarded.
 *
 * @param exe  the program to run, either an absolute path or a program name in the PATH.
 * @param args program arguments. The first arguments should be the program name/path and
 *             the last argument should be NULL.
 */
void runAsync(const fs::path & exe, const char * const args[]);

/*!
 * Equivalent to
 * @code
 *  fs::path exe = getHelperExecutable(name);
 *  const char * args[] = { exe.string.c_str(), ... };
 *  platform::runAsync(exe, args);
 * @endcode
 */
void runHelper(const std::string & name, ...);

#if ARX_PLATFORM != ARX_PLATFORM_WIN32
static const char * const env_list_seperators = ":";
#else
static const char * const env_list_seperators = ";";
#endif

} // namespace platform

#endif // ARX_PLATFORM_ENVIRONMENT_H
