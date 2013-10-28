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
#include <cstring>

#include <stdlib.h>
#include <stdarg.h>

#include "Configure.h"
#include "platform/Platform.h"

#if ARX_PLATFORM == ARX_PLATFORM_WIN32
#include <windows.h>
#endif

#if ARX_HAVE_OPEN && ARX_HAVE_DUP2
#include <fcntl.h>
#endif

#if ARX_HAVE_EXECVP
#include <unistd.h>
#endif

#if ARX_HAVE_POSIX_SPAWNP
#include <spawn.h>
extern char ** environ;
#endif

#if ARX_HAVE_WAITPID
#include <sys/wait.h>
#endif

#include "io/fs/FilePath.h"
#include "platform/Environment.h"
#include "util/String.h"


namespace platform {

static int run(const fs::path & exe, bool wait, const char * const args[]) {
	
#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	
	// Format the command line arguments
	std::ostringstream cmdline;
	for(size_t i = 0; args[i] != NULL; i++) {
		if(i == 0) {
			continue; // skip the program name
		} else if(i != 1) {
			cmdline << ' ';
		}
		cmdline << util::escapeString(args[i], "\\\" '$!");
	}
	
	STARTUPINFO si;
	std::memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION pi;
	std::memset(&pi, 0, sizeof(PROCESS_INFORMATION));
	
	if(!CreateProcess(exe.string().c_str(), cmdline, 0, 0, 0, 0, 0, 0, &si, &pi)) {
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
	
	#if ARX_HAVE_OPEN
	static int dev_null = open("/dev/null", O_RDWR);
	#endif
	
	char ** argv = const_cast<char **>(args);
	
#if ARX_HAVE_POSIX_SPAWNP
	
	// Fast POSIX implementation: posix_spawnp avoids unnecessary vm copies
	
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
	pid_t pid;
	(void)posix_spawnp(&pid, exe.string().c_str(), file_actionsp, attrp, argv, environ);
	
#elif ARX_HAVE_FORK && ARX_HAVE_EXECVP
	
	// Compatibility POSIX implementation
	
	// Start a new process
	pid_t pid = fork();
	if(pid == 0) {
		
		// Redirect standard input, output and error to /dev/null
		#if ARX_HAVE_OPEN && ARX_HAVE_DUP2
		if(dev_null > 0) {
			(void)dup2(dev_null, 0);
			(void)dup2(dev_null, 1);
			(void)dup2(dev_null, 2);
		}
		#endif
		
		// Detach the child process from the parent
		#if ARX_HAVE_SETPGRP
		(void)setpgrp();
		#endif
		
		// Run the executable
		(void)execvp(exe.string().c_str(), argv);
		
		exit(128);
	}
	
#else
	(void)argv;
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
	(void)wait;
	# warning "Waiting for processes not supported on this system."
	#endif
	
	return 0;
	
#endif
	
}

int run(const fs::path & exe, const char * const args[]) {
	return run(exe, true, args);
}

void runAsync(const fs::path & exe, const char * const args[]) {
	(void)run(exe, false, args);
}

void runHelper(const std::string & name, ...) {
	
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
	
	runAsync(getHelperExecutable(name), argv);
}

} // namespace platform
