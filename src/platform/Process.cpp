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

#include "platform/Process.h"

#include <sstream>
#include <vector>

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "Configure.h"
#include "platform/Platform.h"

#if ARX_PLATFORM == ARX_PLATFORM_WIN32

#include <windows.h>

#else // ARX_PLATFORM != ARX_PLATFORM_WIN32

#if ARX_HAVE_OPEN
#include <fcntl.h>
#endif

#if (ARX_HAVE_FORK && ARX_HAVE_EXECVP) \
 || (ARX_HAVE_PIPE && ARX_HAVE_READ && ARX_HAVE_CLOSE)
#include <unistd.h>
#endif

#if ARX_HAVE_POSIX_SPAWNP
#include <spawn.h>
extern char ** environ;
#endif

#if ARX_HAVE_WAITPID
#include <sys/wait.h>
#endif

#endif // ARX_PLATFORM != ARX_PLATFORM_WIN32

#include "io/fs/FilePath.h"
#include "platform/Environment.h"
#include "util/String.h"


namespace platform {

#if ARX_PLATFORM != ARX_PLATFORM_WIN32
static int run(const std::string & exe, bool wait, const char * const args[],
               int stdout, bool unlocalized, bool detach) {
	
	char ** argv = const_cast<char **>(args);
	
	#if ARX_HAVE_OPEN
	static int dev_null = open("/dev/null", O_RDWR);
	if(stdout <= 0) {
		stdout = dev_null;
	}
	#endif
	
	pid_t pid = -1;
	
#if ARX_HAVE_POSIX_SPAWNP
	
	// Fast POSIX implementation: posix_spawnp avoids unnecessary vm copies
	
	if(stdout == dev_null && unlocalized == false && detach) {
		
		// Redirect standard input, output and error to /dev/null
		static posix_spawn_file_actions_t * file_actionsp = NULL;
		#if ARX_HAVE_OPEN
		static posix_spawn_file_actions_t file_actions;
		if(!file_actionsp && dev_null > 0 && !posix_spawn_file_actions_init(&file_actions)) {
			file_actionsp = &file_actions;
			(void)posix_spawn_file_actions_adddup2(file_actionsp, dev_null, 0);
			(void)posix_spawn_file_actions_adddup2(file_actionsp, dev_null, 1);
			(void)posix_spawn_file_actions_adddup2(file_actionsp, dev_null, 2);
		}
		#endif
		
		// Detach the child process from the parent
		static posix_spawnattr_t * attrp = NULL;
		static posix_spawnattr_t attr;
		if(!attrp && !posix_spawnattr_init(&attr)) {
			attrp = &attr;
			(void)posix_spawnattr_setflags(attrp, POSIX_SPAWN_SETPGROUP);
			(void)posix_spawnattr_setpgroup(attrp, 0);
		}
		
		// Run the executable in a new process
		(void)posix_spawnp(&pid, exe.c_str(), file_actionsp, attrp, argv, environ);
		
	}
	
	else
	
#endif
	
	{
#if ARX_HAVE_FORK && ARX_HAVE_EXECVP
		
		// Compatibility POSIX implementation
		
		// Start a new process
		pid = fork();
		if(pid == 0) {
			
			// Redirect standard input, output and error to /dev/null
			#if ARX_HAVE_DUP2
			#if ARX_HAVE_OPEN
			if(dev_null > 0) {
				(void)dup2(dev_null, 0);
				(void)dup2(dev_null, 2);
			}
			#endif
			if(stdout > 0) {
				(void)dup2(stdout, 1);
			}
			#endif
			
			// Detach the child process from the parent
			#if ARX_HAVE_SETPGID
			if(detach) {
				(void)setpgid(0, 0);
			}
			#endif
			
			// Turn off localization
			#if ARX_HAVE_SETENV
			if(unlocalized) {
				setenv("LANG", "C", 1);
				setenv("LC_ALL", "C", 1);
			}
			#endif
			
			// Run the executable
			(void)execvp(exe.c_str(), argv);
			
			exit(128);
		}
		
#endif
	}
	
#if !ARX_HAVE_POSIX_SPAWNP && !(ARX_HAVE_FORK && ARX_HAVE_EXECVP)
	ARX_UNUSED(argv), ARX_UNUSED(stdout), ARX_UNUSED(unlocalized), ARX_UNUSED(detach);
	#warning "Executing helper processes not supported on this system."
#endif
	
	if(pid < 0) {
		return -1;
	}
	
	#if ARX_HAVE_WAITPID
	if(wait) {
		int status;
		(void)waitpid(pid, &status, 0);
		if(WIFEXITED(status) && (WEXITSTATUS(status) >= 0 && WEXITSTATUS(status) < 127)) {
			return WEXITSTATUS(status);
		} else {
			return -1;
		}
	}
	#else
	ARX_UNUSED(wait);
	# warning "Waiting for processes not supported on this system."
	#endif
	
	return 0;
}
#endif // ARX_PLATFORM != ARX_PLATFORM_WIN32


static int run(const std::string & exe, bool wait, const char * const args[]) {
	
#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	
	// Format the command line arguments
	std::ostringstream oss;
	for(size_t i = 0; args[i] != NULL; i++) {
		if(i == 0) {
			continue; // skip the program name
		} else if(i != 1) {
			oss << ' ';
		}
		oss << util::escapeString(args[i], "\\\" '$!");
	}
	char * cmdline = strdup(oss.str().c_str());
	
	STARTUPINFO si;
	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(PROCESS_INFORMATION));
	
	bool success = (CreateProcess(exe.c_str(), cmdline, 0, 0, 0, 0, 0, 0, &si, &pi) != 0);
	
	free(cmdline);
	
	if(!success) {
		return -1; // Could not start process
	}
	
	int status = 0;
	if(wait) {
		status = WaitForSingleObject(pi.hProcess, INFINITE);
	}
	
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	
	return status;
	
#else
	
	return run(exe, wait, args, /*stdout=*/ 0, /*unlocalized=*/ false, /*detach=*/ true);
	
#endif
	
}

int run(const std::string & exe, const char * const args[]) {
	return run(exe, true, args);
}

void runAsync(const std::string & exe, const char * const args[]) {
	(void)run(exe, false, args);
}

void runHelper(const char * name, ...) {
	
	fs::path exe = getHelperExecutable(name);
	
	// Parse the argument list
	std::vector<const char *> arglist;
	arglist.push_back(exe.string().c_str());
	va_list args;
	va_start(args, name);
	while(true) {
		const char * arg = va_arg(args, const char *);
		arglist.push_back(arg);
		if(!arg) {
			break;
		}
	}
	va_end(args);
	const char * const * argv = &arglist.front();
	
	runAsync(exe.string(), argv);
}

#if ARX_PLATFORM != ARX_PLATFORM_WIN32
std::string getOutputOf(const std::string & exe, const char * const args[], bool unlocalized) {
	
	#if ARX_HAVE_PIPE && ARX_HAVE_READ && ARX_HAVE_CLOSE
	
	int pipefd[2];
	if(pipe(pipefd)) {
		return std::string();
	}
	
	if(run(exe, /*wait=*/ false, args, pipefd[1], unlocalized, false) < 0) {
		close(pipefd[0]);
		close(pipefd[1]);
		return std::string();
	}
	close(pipefd[1]);
	
	std::string result;
	while(true) {
		char buffer[1024];
		ssize_t count = read(pipefd[0], buffer, ARRAY_SIZE(buffer));
		if(count < 0) {
			return std::string();
		} else if(count == 0) {
			break;
		} else {
			result.append(buffer, count);
		}
	}
	
	close(pipefd[0]);
	
	return result;
	
	#else
	ARX_UNUSED(exe), ARX_UNUSED(args), ARX_UNUSED(unlocalized);
	return std::string();
	# warning "Reading stdout of processes not supported on this system."
	#endif
	
}
#endif

} // namespace platform
