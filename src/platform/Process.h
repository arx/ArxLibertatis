/*
 * Copyright 2013-2017 Arx Libertatis Team (see the AUTHORS file)
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

#include "platform/Platform.h"

#if ARX_PLATFORM == ARX_PLATFORM_WIN32
#include <windows.h>
#else
#include <sys/types.h>
#endif

namespace platform {
	
#if ARX_PLATFORM == ARX_PLATFORM_WIN32
typedef DWORD process_id;
typedef HANDLE process_handle;
#else
typedef pid_t process_id;
typedef pid_t process_handle;
#endif

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
int run(const char * exe, const char * const args[], bool detach = false);

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
inline int run(const char * const args[], bool detach = false) {
	return run(args[0], args, detach);
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
inline int run(const std::string & exe, const char * const args[], bool detach = false) {
	return run(exe.c_str(), args, detach);
}

/*!
 * \brief Start a program without waiting for it to finish
 *
 * The program's standard output/error is discarded.
 *
 * \param exe  the program to run, either an absolute path or a program name in the PATH.
 * \param args program arguments. The first argument should be the program name/path and
 *             the last argument must be NULL.
 *
 * \return a process handle that should be closed with \ref closeProcessHandle
 *         if no longer needed.
 */
process_handle runAsync(const char * exe, const char * const args[], bool detach = false);

/*!
 * \brief Start a program without waiting for it to finish
 *
 * The program's standard output/error is discarded.
 *
 * \param args program arguments. The first argument is the program to run, either an
 *             absolute path or a program name in the PATH. The last argument must be NULL.
 *
 * \return a process handle that should be closed with \ref closeProcessHandle
 *         if no longer needed.
 */
inline process_handle runAsync(const char * const args[], bool detach = false) {
	return runAsync(args[0], args, detach);
}

/*!
 * \brief Start a program without waiting for it to finish
 *
 * The program's standard output/error is discarded.
 *
 * \param exe  the program to run, either an absolute path or a program name in the PATH.
 * \param args program arguments. The first argument should be the program name/path and
 *             the last argument must be NULL.
 *
 * \return a process handle that should be closed with \ref closeProcessHandle
 *         if no longer needed.
 */
inline process_handle runAsync(const std::string & exe, const char * const args[],
                               bool detach = false) {
	return runAsync(exe.c_str(), args, detach);
}

//! Get the id of the current process
process_id getProcessId();

//! Get the process id from a handle returned by \ref runAsync
process_id getProcessId(process_handle process);

/*!
 * Check if a process is still running
 *
 * Note that that this check is subject to race conditions as the process id may be
 * reused after the process exits.
 *
 * If we have a handle to the process it is better to use \ref getProcessExitCode.
 */
bool isProcessRunning(process_id pid);

//! Forcibly exit a running process
void killProcess(process_id pid);

enum ProcessExitCode {
	StillRunning = -1,
	ExitSuccess = 0
};

/*!
 * Get the return code of a child process
 *
 * This function implicitly closes the process handle if the process exited.
 *
 * \param wait whether we should wait for the process to exit or return immediately.
 *
 * \return the process exit code, or \ref StillRunning if the process is still running or
 *         another negative number if there was an error.
 */
int getProcessExitCode(process_handle process, bool wait = true);

//! Clean up a process handle returned by \ref runAsync
void closeProcessHandle(process_handle process);

//! Clean up terminated child processes
void reapZombies();

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
int runHelper(const char * const args[], bool wait = false, bool detach = false);

#if ARX_PLATFORM == ARX_PLATFORM_WIN32

bool isWoW64Process(process_handle process);

#else

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

/*!
 * \brief Open a URL, file or directory in the appropriate program
 *
 * \param uri The URL, file or directory to open. This must be an absolute path.
 */
void launchDefaultProgram(const std::string & uri);

} // namespace platform

#endif // ARX_PLATFORM_PROCESS_H
