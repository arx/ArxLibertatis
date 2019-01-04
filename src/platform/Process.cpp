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

#include "platform/Process.h"

#include <sstream>
#include <vector>

#include <string.h>
#include <stdlib.h>

#include "Configure.h"
#include "platform/Platform.h"

#if ARX_PLATFORM == ARX_PLATFORM_WIN32
#include <windows.h>
#include <shellapi.h>
#include <objbase.h>
#else // ARX_PLATFORM != ARX_PLATFORM_WIN32

#if ARX_HAVE_OPEN
#include <fcntl.h>
#endif

#if (ARX_HAVE_FORK && ARX_HAVE_EXECVP) \
 || (ARX_HAVE_PIPE && ARX_HAVE_READ && ARX_HAVE_CLOSE) \
 || ARX_HAVE_GETPID
#include <unistd.h>
#endif

#if ARX_HAVE_KILL
#include <signal.h>
#endif

#if ARX_HAVE_POSIX_SPAWNP
#include <spawn.h>
#if defined(__FreeBSD__) && defined(__GNUC__) && __GNUC__ >= 4
/*
 * When combining -flto and -fvisibility=hidden we and up with a hidden
 * 'environ' symbol in crt1.o on FreeBSD 9, which causes the link to fail.
 */
extern char ** environ __attribute__((visibility("default"))); // NOLINT
#else
extern char ** environ; // NOLINT
#endif
#endif

#if ARX_HAVE_WAITPID
#include <sys/wait.h>
#endif

#endif // ARX_PLATFORM != ARX_PLATFORM_WIN32

#include "io/fs/FilePath.h"

#include "platform/Environment.h"
#include "platform/WindowsUtils.h"

#include "util/String.h"

#if ARX_PLATFORM != ARX_PLATFORM_WIN32 && ARX_HAVE_OPEN && !ARX_HAVE_O_CLOEXEC
#define O_CLOEXEC 0
#endif

namespace platform {

#if ARX_PLATFORM != ARX_PLATFORM_WIN32
static process_handle run(const char * exe, const char * const args[], int outfd,
                          bool unlocalized, bool detach) {
	
	char ** argv = const_cast<char **>(args);
	
	#if ARX_HAVE_OPEN
	static int dev_null = open("/dev/null", O_RDWR | O_CLOEXEC);
	#if !ARX_HAVE_O_CLOEXEC && ARX_HAVE_FCNTL
	fcntl(dev_null, F_SETFD, FD_CLOEXEC);
	#endif
	#endif
	
	pid_t pid = 0;
	
	#if ARX_HAVE_POSIX_SPAWNP
	
	// Fast POSIX implementation: posix_spawnp avoids unnecessary vm copies
	
	if(outfd <= 0 && !unlocalized) {
		
		// Redirect standard input, output and error to /dev/null
		static posix_spawn_file_actions_t * file_actionsp = NULL;
		#if ARX_HAVE_OPEN
		static posix_spawn_file_actions_t file_actions;
		if(detach && dev_null > 0 && !posix_spawn_file_actions_init(&file_actions)) {
			file_actionsp = &file_actions;
			(void)posix_spawn_file_actions_adddup2(file_actionsp, dev_null, 0);
			(void)posix_spawn_file_actions_adddup2(file_actionsp, dev_null, 1);
			(void)posix_spawn_file_actions_adddup2(file_actionsp, dev_null, 2);
		}
		#endif
		
		// Detach the child process from the parent
		static posix_spawnattr_t * attrp = NULL;
		static posix_spawnattr_t attr;
		if(detach && !posix_spawnattr_init(&attr)) {
			attrp = &attr;
			(void)posix_spawnattr_setflags(attrp, POSIX_SPAWN_SETPGROUP);
			(void)posix_spawnattr_setpgroup(attrp, 0);
		}
		
		// Run the executable in a new process
		if(posix_spawnp(&pid, exe, file_actionsp, attrp, argv, environ) != 0) {
			pid = 0;
		}
		
	}
	
	else
	
	#endif // ARX_HAVE_POSIX_SPAWNP
	
	{
		#if ARX_HAVE_FORK && ARX_HAVE_EXECVP
		
		// Compatibility POSIX implementation
		
		// Start a new process
		pid = fork();
		if(pid == 0) {
			
			// Redirect standard input, output and error to /dev/null
			#if ARX_HAVE_DUP2
			#if ARX_HAVE_OPEN
			if(detach && dev_null > 0) {
				(void)dup2(dev_null, 0);
				if(outfd <= 0) {
					dup2(dev_null, 1);
				}
				(void)dup2(dev_null, 2);
			}
			#endif
			if(outfd > 0) {
				(void)dup2(outfd, 1);
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
			(void)execvp(exe, argv);
			
			exit(-1);
		}
		
		#endif // ARX_HAVE_FORK && ARX_HAVE_EXECVP
	}
	
	#if !ARX_HAVE_POSIX_SPAWNP && !(ARX_HAVE_FORK && ARX_HAVE_EXECVP)
	ARX_UNUSED(argv), ARX_UNUSED(outfd), ARX_UNUSED(unlocalized), ARX_UNUSED(detach);
	#warning "Executing helper processes not supported on this system."
	#endif
	
	return (pid <= 0) ? 0 : pid;
}
#endif // ARX_PLATFORM != ARX_PLATFORM_WIN32

process_handle runAsync(const char * exe, const char * const args[], bool detach) {
	
#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	
	ARX_UNUSED(detach);
	
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
	
	STARTUPINFO si;
	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(PROCESS_INFORMATION));
	
	platform::WideString wexe(exe);
	platform::WideString wcmdline(oss.str());
	bool success = (CreateProcessW(wexe, wcmdline.data(), 0, 0, 0, 0, 0, 0, &si, &pi) != 0);
	
	if(!success) {
		return 0; // Could not start process
	}
	
	CloseHandle(pi.hThread);
	
	return pi.hProcess;
	
#else
	
	return run(exe, args, /*outfd=*/ 0, /*unlocalized=*/ false, detach);
	
#endif
	
}

process_id getProcessId() {
	#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	return GetCurrentProcessId();
	#elif ARX_HAVE_GETPID
	return getpid();
	#else
	#warning "Getting process id not supported on this system."
	return 0;
	#endif
}

process_id getProcessId(process_handle process) {
	#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	return process ? GetProcessId(process) : 0;
	#else
	return process;
	#endif
}

bool isProcessRunning(process_id pid) {
	
	if(!pid) {
		return false;
	}
	
	#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, pid);
	DWORD ret = WaitForSingleObject(process, 0);
	CloseHandle(process);
	return ret == WAIT_TIMEOUT;
	#elif ARX_HAVE_KILL
	return kill(pid, 0) == 0;
	#else
	return true;
	#endif
}

void killProcess(process_id pid) {
	
	if(!pid) {
		return;
	}
	
	#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	HANDLE process = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
	if(process) {
		TerminateProcess(process, 1);
		CloseHandle(process);
	}
	#elif ARX_HAVE_KILL
	// Kill the original, busy-waiting process.
	kill(pid, SIGKILL);
	#else
	#warning "Killing processes not supported on this system."
	return;
	#endif
}


int getProcessExitCode(process_handle process, bool wait) {
	
	if(!process) {
		return -2;
	}
	
	#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	
	if(wait) {
		WaitForSingleObject(process, INFINITE);
	}
	
	DWORD code = DWORD(-2);
	if(GetExitCodeProcess(process, &code) == FALSE) {
		CloseHandle(process);
		return -2;
	} else if(code == STILL_ACTIVE) {
		return StillRunning;
	}
	
	CloseHandle(process);
	
	return code;
	
	#elif ARX_HAVE_WAITPID
	
	int status;
	pid_t ret = waitpid(process, &status, wait ? 0 : WNOHANG);
	if(ret == 0) {
		return StillRunning;
	} else if(ret != process) {
		return -2;
	}
	if(WIFEXITED(status) && (WEXITSTATUS(status) >= 0 && WEXITSTATUS(status) < 127)) {
		return WEXITSTATUS(status);
	} else if(WIFSIGNALED(status)) {
		return -WTERMSIG(status);
	} else {
		return -2;
	}
	
	#else
	
	ARX_UNUSED(process), ARX_UNUSED(wait);
	# warning "Waiting for processes not supported on this system."
	return StillRunning;
	
	#endif
	
}

#if ARX_PLATFORM != ARX_PLATFORM_WIN32 && ARX_HAVE_WAITPID
static std::vector<process_handle> g_childProcesses;
#endif

void closeProcessHandle(process_handle process) {
	
	if(!process) {
		return;
	}
	
	#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	CloseHandle(process);
	#elif ARX_HAVE_WAITPID
	if(waitpid(process, NULL, WNOHANG) == 0) {
		g_childProcesses.push_back(process);
	}
	#else
	ARX_UNUSED(process);
	#endif
}

void reapZombies() {
	#if ARX_PLATFORM != ARX_PLATFORM_WIN32 && ARX_HAVE_WAITPID
	std::vector<process_handle>::iterator it = g_childProcesses.begin();
	while(it != g_childProcesses.end()) {
		if(waitpid(*it, NULL, WNOHANG) != 0) {
			it = g_childProcesses.erase(it);
		} else {
			++it;
		}
	}
	#endif
}

int run(const char * exe, const char * const args[], bool detach) {
	process_handle process = runAsync(exe, args, detach);
	return getProcessExitCode(process);
}

int runHelper(const char * const args[], bool wait, bool detach) {
	fs::path exe = getHelperExecutable(args[0]);
	process_handle process = runAsync(exe.string().c_str(), args, detach);
	if(wait) {
		return getProcessExitCode(process);
	} else {
		closeProcessHandle(process);
		return 0;
	}
}

#if ARX_PLATFORM == ARX_PLATFORM_WIN32

bool isWoW64Process(process_handle process) {
	
	typedef BOOL (WINAPI * IsWow64Process_t)(HANDLE, PBOOL);
	IsWow64Process_t IsWow64Process_p;
	
	// IsWow64Process is not available on all versions of Windows - load it dynamically.
	HMODULE handle = GetModuleHandleW(L"kernel32");
	IsWow64Process_p = (IsWow64Process_t)GetProcAddress(handle, "IsWow64Process");
	if(!IsWow64Process_p) {
		return false;
	}
	
	BOOL result;
	if(!IsWow64Process_p(process, &result)) {
		return false;
	}
	
	return result == TRUE;
}

#else

std::string getOutputOf(const char * exe, const char * const args[], bool unlocalized) {
	
	#if ARX_HAVE_PIPE && ARX_HAVE_READ && ARX_HAVE_CLOSE
	
	int pipefd[2];
	if(pipe(pipefd)) {
		return std::string();
	}
	
	process_handle process = run(exe, args, pipefd[1], unlocalized, false);
	if(!process) {
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
	
	closeProcessHandle(process);
	
	return result;
	
	#else
	ARX_UNUSED(exe), ARX_UNUSED(args), ARX_UNUSED(unlocalized);
	return std::string();
	# warning "Reading stdout of processes not supported on this system."
	#endif
	
}

#endif

void launchDefaultProgram(const std::string & uri) {
	
	#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	
	(void)ShellExecuteW(NULL, L"open", platform::WideString(uri), NULL, NULL, SW_SHOWNORMAL);
	
	CoUninitialize();
	
	#elif ARX_PLATFORM == ARX_PLATFORM_MACOS
	
	const char * command[] = { "open", uri.c_str(), NULL };
	runHelper(command);
	
	#else
	
	const char * command[] = { "xdg-open", uri.c_str(), NULL };
	runHelper(command);
	
	#endif
	
}

} // namespace platform
