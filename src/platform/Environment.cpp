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

#include <stdlib.h> // needed for setenv, realpath and more

#include <boost/scoped_array.hpp>

#include "Configure.h"

#if ARX_PLATFORM == ARX_PLATFORM_WIN32
#include <windows.h>
#include <shlobj.h>
#endif

#if ARX_HAVE_WORDEXP
#include <wordexp.h>
#endif

#if ARX_HAVE_READLINK
#include <unistd.h>
#endif

#if ARX_PLATFORM == ARX_PLATFORM_MACOSX
#include <mach-o/dyld.h>
#include <sys/param.h>
#endif

#if ARX_HAVE_SYSCTL
#include <sys/types.h>
#include <sys/sysctl.h>
#include <fcntl.h>
#endif

#include "io/fs/PathConstants.h"
#include "io/fs/FilePath.h"
#include "io/fs/Filesystem.h"
#include "util/String.h"


namespace platform {

std::string expandEnvironmentVariables(const std::string & in) {
	
#if ARX_HAVE_WORDEXP
	
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
	
#elif ARX_PLATFORM == ARX_PLATFORM_WIN32
	
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

#if ARX_PLATFORM == ARX_PLATFORM_WIN32
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
	
#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	
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

#elif ARX_PLATFORM == ARX_PLATFORM_WIN32

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

#if ARX_HAVE_READLINK && ARX_PLATFORM != ARX_PLATFORM_MACOSX
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
	
#elif ARX_PLATFORM == ARX_PLATFORM_WIN32
	
	std::vector<char> buffer;
	buffer.resize(MAX_PATH);
	if(GetModuleFileNameA(NULL, &*buffer.begin(), buffer.size()) > 0) {
		return fs::path(&*buffer.begin(), &*buffer.end());
	}
	
#else
	
	// Try to get the path from OS-specific procfs entries
	#if ARX_HAVE_READLINK
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
	#if ARX_HAVE_SYSCTL && defined(CTL_KERN) && defined(KERN_PROC) \
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
	#if ARX_HAVE_GETEXECNAME
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

} // namespace platform
