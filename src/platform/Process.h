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
 * \brief Start a program and wait for it to finish
 *
 * The executable's standard output/error is discarded.
 *
 * \param exe  the program to run, either an absolute path or a program name in the PATH.
 * \param args program arguments. The first argument should be the program name/path and
 *             the last argument must be NULL.
 *
 * \return the programs exit code or a negative value on error.
 */
int run(const char * exe, const char * const args[]);

/*!
 * \brief Start a program and wait for it to finish
 *
 * The executable's standard output/error is discarded.
 *
 * \param args program arguments. The first argument is the program to run, either an
 *             absolute path or a program name in the PATH. The last argument must be NULL.
 *
 * \return the programs exit code or a negative value on error.
 */
inline int run(const char * const args[]) {
	return run(args[0], args);
}

/*!
 * \brief Start a program and wait for it to finish
 *
 * The executable's standard output/error is discarded.
 *
 * \param exe  the program to run, either an absolute path or a program name in the PATH.
 * \param args program arguments. The first argument should be the program name/path and
 *             the last argument must be NULL.
 *
 * \return the programs exit code or a negative value on error.
 */
inline int run(const std::string & exe, const char * const args[]) {
	return run(exe.c_str(), args);
}

/*!
 * \brief Start a program without waiting for it to finish
 *
 * The program's standard output/error is discarded.
 *
 * \param exe  the program to run, either an absolute path or a program name in the PATH.
 * \param args program arguments. The first argument should be the program name/path and
 *             the last argument must be NULL.
 */
void runAsync(const char * exe, const char * const args[]);

/*!
 * \brief Start a program without waiting for it to finish
 *
 * The program's standard output/error is discarded.
 *
 * \param args program arguments. The first argument is the program to run, either an
 *             absolute path or a program name in the PATH. The last argument must be NULL.
 */
inline void runAsync(const char * const args[]) {
	runAsync(args[0], args);
}

/*!
 * \brief Start a program without waiting for it to finish
 *
 * The program's standard output/error is discarded.
 *
 * \param exe  the program to run, either an absolute path or a program name in the PATH.
 * \param args program arguments. The first argument should be the program name/path and
 *             the last argument must be NULL.
 */
inline void runAsync(const std::string & exe, const char * const args[]) {
	runAsync(exe.c_str(), args);
}

/*!
 * \brief Run a helper executable
 *
 * \param wait true if the helper should be run synchronously
 *
 * If \ref wait is \c false this call is equivalent to
 * \code platform::runAsync(getHelperExecutable(args[0]).string(), args); \endcode
 * Otherwise it is equivalent to
 * \code platform::run(getHelperExecutable(args[0]).string(), args); \endcode
 *
 * \return if \ref wait is \c true, the return code of the executed program.
 */
int runHelper(const char * const args[], bool wait = false);

/*!
 * \brief Run a helper executable
 *
 * Shortcut for
 * \code
const char * args[] = { name, ... };
platform::runHelper(args); \endcode
 */
#if ARX_HAVE_CXX11_VARIADIC_TEMPLATES
template <typename... Args>
inline void runHelper(const char * name, Args... args) {
	const char * const argArray[] = { name, args... };
	(void)runHelper(argArray);
}
#else
void runHelper(const char * name, ...);
#endif

#if ARX_PLATFORM != ARX_PLATFORM_WIN32

/*!
 * \brief Start a program and get its standard output
 *
 * The program's standard standard error is discarded.
 *
 * \param exe  the program to run, either an absolute path or a program name in the PATH.
 * \param args program arguments. The first argument should be the program name/path and
 *             the last argument must be NULL.
 * \param unlocalized true if localization environment varuables should be reset.
 *
 * \return the program's standard output, or an empty string if there was an error.
 */
std::string getOutputOf(const char * exe, const char * const args[],
                        bool unlocalized = false);

/*!
 * \brief Start a program and get its standard output
 *
 * The program's standard standard error is discarded.
 *
 * \param args program arguments. The first argument is the program to run, either an
 *             absolute path or a program name in the PATH. The last argument must be NULL.
 * \param unlocalized true if localization environment varuables should be reset.
 *
 * \return the program's standard output, or an empty string if there was an error.
 */
inline std::string getOutputOf(const char * const args[], bool unlocalized = false) {
	return getOutputOf(args[0], args, unlocalized);
}

/*!
 * \brief Start a program and get its standard output
 *
 * The program's standard standard error is discarded.
 *
 * \param exe  the program to run, either an absolute path or a program name in the PATH.
 * \param args program arguments. The first argument should be the program name/path and
 *             the last argument must be NULL.
 * \param unlocalized true if localization environment varuables should be reset.
 *
 * \return the program's standard output, or an empty string if there was an error.
 */
inline std::string getOutputOf(const std::string & exe, const char * const args[],
                               bool unlocalized = false) {
	return getOutputOf(exe.c_str(), args, unlocalized);
}

#endif

} // namespace platform

#endif // ARX_PLATFORM_PROCESS_H
