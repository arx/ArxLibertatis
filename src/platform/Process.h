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

#ifndef ARX_PLATFORM_PROCESS_H
#define ARX_PLATFORM_PROCESS_H

#include <string>

namespace fs { class path; }

namespace platform {

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

} // namespace platform

#endif // ARX_PLATFORM_PROCESS_H
