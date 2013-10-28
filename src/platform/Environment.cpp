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

#include "platform/Environment.h"

#include <sstream>
#include <algorithm>
#include <vector>
#include <cstring>

#include <stdlib.h> // needed for setenv, realpath and more
#include <stdarg.h>

#include <boost/scoped_array.hpp>

#include "Configure.h"

#ifdef ARX_HAVE_WINAPI
#include <windows.h>
#include <shlobj.h>
#endif

#ifdef ARX_HAVE_WORDEXP_H
#include <wordexp.h>
#endif

#if defined(ARX_HAVE_OPEN) && defined(ARX_HAVE_DUP2)
#include <fcntl.h>
#endif

#if defined(ARX_HAVE_READLINK) || defined(ARX_HAVE_EXECVP)
#include <unistd.h>
#endif

#if defined(ARX_HAVE_POSIX_SPAWNP)
#include <spawn.h>
extern char ** environ;
#endif

#if defined(ARX_HAVE_WAITPID)
#include <sys/wait.h>
#endif

#if ARX_PLATFORM == ARX_PLATFORM_MACOSX
#include <mach-o/dyld.h>
#include <sys/param.h>
#endif

#ifdef ARX_HAVE_SYSCTL
#include <sys/types.h>
#include <sys/sysctl.h>
#include <fcntl.h>
#endif

#include "io/fs/PathConstants.h"
#include "io/fs/FilePath.h"
#include "io/fs/Filesystem.h"
#include "platform/Platform.h"
#include "util/String.h"


namespace platform {

std::string expandEnvironmentVariables(const std::string & in) {
	
#if defined(ARX_HAVE_WORDEXP_H)
	
	wordexp_t p;
	
	if(wordexp(in.c_str(), &p, 0)) {
		return in;
	}
	
	std::ostringstream oss;
	for(size_t i = 0; i < p.we_wordc; i++) {
		
		oss << p.we_wordv[i];
		
		if(i != (p.we_wordc-1))
			oss << " ";
	}
	
	wordfree(&p);
	
	return oss.str();
	
#elif defined(ARX_HAVE_WINAPI)
	
	size_t length = std::max<size_t>(in.length() * 2, 1024);
	boost::scoped_array<char> buffer(new char[length]);
	
	DWORD ret = ExpandEnvironmentStringsA(in.c_str(), buffer.get(), length);
	
	if(ret > length) {
		length = ret;
		buffer.reset(new char[length]);
		ret = ExpandEnvironmentStringsA(in.c_str(), buffer.get(), length);
	}
	
	if(ret == 0 || ret > length) {
		return in;
	}
	
	return std::string(buffer.get());
	
#else
# warning "Environment variable expansion not supported on this system."
	return in;
#endif
}

#ifdef ARX_HAVE_WINAPI
static bool getRegistryValue(HKEY hkey, const std::string & name, std::string & result) {
	
	boost::scoped_array<char> buffer(NULL);

	DWORD type = 0;
	DWORD length = 0;
	HKEY handle = 0;

	long ret = 0;

	ret = RegOpenKeyEx(hkey, "Software\\ArxLibertatis\\", 0, KEY_QUERY_VALUE, &handle);

	if (ret == ERROR_SUCCESS)
	{
		// find size of value
		ret = RegQueryValueEx(handle, name.c_str(), NULL, NULL, NULL, &length);

		if (ret == ERROR_SUCCESS && length > 0)
		{
			// allocate buffer and read in value
			buffer.reset(new char[length + 1]);
			ret = RegQueryValueEx(handle, name.c_str(), NULL, &type, LPBYTE(buffer.get()), &length);
			// ensure null termination
			buffer.get()[length] = 0;
		}

		RegCloseKey(handle);
	}

	if(ret == ERROR_SUCCESS) {
		result = buffer.get();
		return true;
	} else {
		return false;
	}
}
#endif

bool getSystemConfiguration(const std::string & name, std::string & result) {
	
#ifdef ARX_HAVE_WINAPI
	
	if(getRegistryValue(HKEY_CURRENT_USER, name, result)) {
		return true;
	}
	
	if(getRegistryValue(HKEY_LOCAL_MACHINE, name, result)) {
		return true;
	}
	
#else
	ARX_UNUSED(name), ARX_UNUSED(result);
#endif
	
	return false;
}

#if ARX_PLATFORM == ARX_PLATFORM_MACOSX

void defineSystemDirectories(const char * argv0) {
	ARX_UNUSED(argv0);
}

#elif defined(ARX_HAVE_WINAPI)

std::string ws2s(const std::basic_string<WCHAR> & s) {
	size_t slength = (int)s.length() + 1;
	size_t len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0); 
	std::string r(len, '\0');
	WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, &r[0], len, 0, 0); 
	return r;
}

// Those two values are from ShlObj.h, but requires _WIN32_WINNT >= _WIN32_WINNT_VISTA
static const int kfFlagCreate  = 0x00008000; // KF_FLAG_CREATE
static const int kfFlagNoAlias = 0x00001000; // KF_FLAG_NO_ALIAS

// Obtain the right savegame paths for the platform
// XP is "%USERPROFILE%\My Documents\My Games"
// Vista and up : "%USERPROFILE%\Saved Games"
void defineSystemDirectories(const char * argv0) {
	
	ARX_UNUSED(argv0);
	
	std::string strPath;
	DWORD winver = GetVersion();
	
	// Vista and up
	if((DWORD)(LOBYTE(LOWORD(winver))) >= 6) {
		// Don't hardlink with SHGetKnownFolderPath to allow the game to start on XP too!
		typedef HRESULT (WINAPI * PSHGetKnownFolderPath)(const GUID &rfid, DWORD dwFlags,
		                                                 HANDLE hToken, PWSTR* ppszPath); 
		
		CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
		
		PSHGetKnownFolderPath GetKnownFolderPath = (PSHGetKnownFolderPath)GetProcAddress(GetModuleHandleA("shell32.dll"), "SHGetKnownFolderPath");
		const GUID FOLDERID_SavedGames = {0x4C5C32FF, 0xBB9D, 0x43b0, {0xB5, 0xB4, 0x2D, 0x72, 0xE5, 0x4E, 0xAA, 0xA4}};
		
		LPWSTR wszPath = NULL;
		HRESULT hr = GetKnownFolderPath(FOLDERID_SavedGames, kfFlagCreate | kfFlagNoAlias, NULL, &wszPath);
		
		if(SUCCEEDED(hr)) {
			strPath = ws2s(wszPath);
		}
		
		CoTaskMemFree(wszPath);
		CoUninitialize();
	} else if((DWORD)(LOBYTE(LOWORD(winver))) == 5) { // XP
		CHAR szPath[MAX_PATH];
		HRESULT hr = SHGetFolderPathA(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL,
		                              SHGFP_TYPE_CURRENT, szPath);
		
		if(SUCCEEDED(hr)) {
			strPath = szPath; 
			strPath += "\\My Games";
		}
	} else {
		arx_assert_msg(false, "Unsupported windows version (below WinXP)");
	}
	
	if(!strPath.empty()) {
		SetEnvironmentVariable("FOLDERID_SavedGames", strPath.c_str());
	}
}

#else

static const char * executablePath = NULL;

void defineSystemDirectories(const char * argv0) {
	executablePath = argv0;
}

#endif

#if defined(ARX_HAVE_READLINK) && ARX_PLATFORM != ARX_PLATFORM_MACOSX
static bool try_readlink(std::vector<char> & buffer, const char * path) {
	
	int ret = readlink(path, &buffer.front(), buffer.size());
	while(ret >= 0 && std::size_t(ret) == buffer.size()) {
		buffer.resize(buffer.size() * 2);
		ret = readlink(path, &buffer.front(), buffer.size());
	}
	
	if(ret < 0) {
		return false;
	}
	
	buffer.resize(ret);
	return true;
}
#endif

fs::path getExecutablePath() {
	
#if ARX_PLATFORM == ARX_PLATFORM_MACOSX
	
	uint32_t bufsize = 0;
	
	// Obtain required size
	_NSGetExecutablePath(NULL, &bufsize);
	
	std::vector<char> exepath(bufsize);
	
	if(_NSGetExecutablePath(&exepath.front(), &bufsize) == 0) {
		char exerealpath[MAXPATHLEN];
		if(realpath(&exepath.front(), exerealpath)) {
			return exerealpath;
		}
	}
	
#elif defined(ARX_HAVE_WINAPI)
	
	std::vector<char> buffer;
	buffer.resize(MAX_PATH);
	if(GetModuleFileNameA(NULL, &*buffer.begin(), buffer.size()) > 0) {
		return fs::path(&*buffer.begin(), &*buffer.end());
	}
	
#else
	
	// Try to get the path from OS-specific procfs entries
	#ifdef ARX_HAVE_READLINK
	std::vector<char> buffer(1024);
	// Linux
	if(try_readlink(buffer, "/proc/self/exe")) {
		return fs::path(&*buffer.begin(), &*buffer.end());
	}
	// BSD
	if(try_readlink(buffer, "/proc/curproc/file")) {
		return fs::path(&*buffer.begin(), &*buffer.end());
	}
	// Solaris
	if(try_readlink(buffer, "/proc/self/path/a.out")) {
		return fs::path(&*buffer.begin(), &*buffer.end());
	}
	#endif
	
	// FreeBSD
	#if defined(ARX_HAVE_SYSCTL) && defined(CTL_KERN) && defined(KERN_PROC) \
	    && defined(KERN_PROC_PATHNAME) && ARX_PLATFORM == ARX_PLATFORM_BSD \
	    && defined(PATH_MAX)
	int mib[4];
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PATHNAME;
	mib[3] = -1;
	char pathname[PATH_MAX];
	size_t size = sizeof(pathname);
	int error = sysctl(mib, 4, pathname, &size, NULL, 0);
	if(error != -1 && size > 0 && size < sizeof(pathname)) {
		return util::loadString(pathname, size);
	}
	#endif
	
	// Solaris
	#ifdef ARX_HAVE_GETEXECNAME
	const char * execname = getexecname();
	if(execname != NULL) {
		return execname;
	}
	#endif
	
	// Fall back to argv[0] if possible
	if(executablePath != NULL) {
		std::string path(executablePath);
		if(path.find('/') != std::string::npos) {
			return path;
		}
	}
	
#endif
	
	// Give up - we couldn't determine the exe path.
	return fs::path();
}

fs::path getHelperExecutable(const std::string & name) {
	
	fs::path exe = getExecutablePath();
	if(!exe.empty()) {
		if(exe.is_relative()) {
			exe = fs::current_path() / exe;
		}
		fs::path helper = exe.parent() / name;
		if(fs::is_regular_file(helper)) {
			return helper;
		}
		helper = exe.parent().parent() / name;
		if(fs::is_regular_file(helper)) {
			return helper;
		}
	}
	
	if(fs::libexec_dir) {
		fs::path helper = fs::path(fs::libexec_dir) / name;
		if(fs::is_regular_file(helper)) {
			return helper;
		}
	}
	
	return fs::path(name);
}

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
	
	#if defined(ARX_HAVE_OPEN)
	static int dev_null = open("/dev/null", O_RDWR);
	#endif
	
	char ** argv = const_cast<char **>(args);
	
#if defined(ARX_HAVE_POSIX_SPAWNP) && 0
	
	// Fast POSIX implementation: posix_spawnp avoids unnecessary vm copies
	
	// Redirect standard input, output and error to /dev/null
	static posix_spawn_file_actions_t * file_actionsp = NULL;
	#if defined(ARX_HAVE_OPEN) && defined(ARX_HAVE_CLOSE)
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
	
#elif defined(ARX_HAVE_FORK) && defined(ARX_HAVE_EXECVP)
	
	// Compatibility POSIX implementation
	
	// Start a new process
	pid_t pid = fork();
	if(pid == 0) {
		
		// Redirect standard input, output and error to /dev/null
		#if defined(ARX_HAVE_OPEN) && defined(ARX_HAVE_DUP2) && defined(ARX_HAVE_CLOSE)
		if(dev_null > 0) {
			(void)dup2(dev_null, 0);
			(void)dup2(dev_null, 1);
			(void)dup2(dev_null, 2);
		}
		#endif
		
		// Detach the child process from the parent
		#ifdef ARX_HAVE_SETPGRP
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
	
	#if defined(ARX_HAVE_WAITPID)
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
