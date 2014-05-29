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

#include "platform/Platform.h"

#include <string>

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
int run(const char * exe, const char * const args[]);
inline int run(const char * const args[]) {
	return run(args[0], args);
}
inline int run(const std::string & exe, const char * const args[]) {
	return run(exe.c_str(), args);
}

/*!
 * Start another executable without waiting for it to finish.
 * The executable's standard output/error is discarded.
 *
 * @param exe  the program to run, either an absolute path or a program name in the PATH.
 * @param args program arguments. The first arguments should be the program name/path and
 *             the last argument should be NULL.
 */
void runAsync(const char * exe, const char * const args[]);
inline void runAsync(const char * const args[]) {
	return runAsync(args[0], args);
}
inline void runAsync(const std::string & exe, const char * const args[]) {
	return runAsync(exe.c_str(), args);
}

#if ARX_PLATFORM != ARX_PLATFORM_WIN32
/*!
 * Start another executable without waiting for it to finish.
 * The executable's standard output is retuned, standard error is discarded.
 *
 * @param exe  the program to run, either an absolute path or a program name in the PATH.
 * @param args program arguments. The first arguments should be the program name/path and
 *             the last argument should be NULL.
 * @param unlocalized true if localization environment varuables should be reset.
 */
std::string getOutputOf(const char * exe, const char * const args[],
                        bool unlocalized = false);
inline std::string getOutputOf(const char * const args[], bool unlocalized = false) {
	return getOutputOf(args[0], args, unlocalized);
}
inline std::string getOutputOf(const std::string & exe, const char * const args[],
                               bool unlocalized = false) {
	return getOutputOf(exe.c_str(), args, unlocalized);
}
#endif

/*!
 * Equivalent to
 * @code
 *  fs::path exe = getHelperExecutable(name);
 *  const char * args[] = { exe.string().c_str(), ... };
 *  platform::runAsync(args);
 * @endcode
 */
void runHelper(const char * name, ...);

} // namespace platform

#endif // ARX_PLATFORM_PROCESS_H
