/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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
#include <stdlib.h>

#include <boost/scoped_array.hpp>

#include "Configure.h"

#ifdef HAVE_WINAPI
#include <windows.h>
#include <shlobj.h>
#endif

#ifdef HAVE_WORDEXP_H
#include <wordexp.h>
#endif

std::string expandEvironmentVariables(const std::string & in) {
	
#if defined(HAVE_WORDEXP_H)
	
	wordexp_t p;
	
	if(wordexp(in.c_str(), &p, 0)) {
		return in;
	}
	
	std::ostringstream oss;
	for(size_t i = 0; i < p.we_wordc; i++) {
		oss << p.we_wordv[i];
	}
	
	wordfree(&p);
	
	return oss.str();
	
#elif defined(HAVE_WINAPI)
	
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

#ifdef HAVE_WINAPI
static bool getRegistryValue(HKEY hkey, const std::string & name, std::string & result) {
	
	static const char * key = "Software\\ArxLibertatis";
	
	DWORD length = 1024;
	boost::scoped_array<char> buffer(new char[length]);
	
	LONG ret = RegGetValueA(hkey, key, name.c_str(), RRF_RT_REG_SZ, NULL, buffer.get(), &length);
	if(ret == ERROR_MORE_DATA) {
		buffer.reset(new char[length]);
		ret = RegGetValueA(hkey, key, name.c_str(), RRF_RT_REG_SZ, NULL, buffer.get(), &length);
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
	
#ifdef HAVE_WINAPI
	
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

#if ARX_PLATFORM != ARX_PLATFORM_WIN32

void defineSystemDirectories() {
	
	const char * _home = getenv("HOME");
	std::string home = _home ? _home : "~";
	
	setenv("XDG_DATA_HOME", (home + "/.local/share").c_str(), 0);
	setenv("XDG_CONFIG_HOME", (home + "/.config").c_str(), 0);
	setenv("XDG_DATA_DIRS", "/usr/local/share/:/usr/share/", 0);
	setenv("XDG_CONFIG_DIRS", "/etc/xdg", 0);
	setenv("XDG_CACHE_HOME", (home + "/.cache").c_str(), 0);
}

#else

std::string ws2s(const std::wstring& s)
{    
    size_t slength = (int)s.length() + 1;
    size_t len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0); 
    std::string r(len, '\0');
    WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, &r[0], len, 0, 0); 
    return r;
}

// Obtain the right savegame paths for the platform
// XP is "%USERPROFILE%\My Documents\My Games"
// Vista and up : "%USERPROFILE%\Saved games"
void defineSystemDirectories() {
	std::string strPath;
	DWORD winver = GetVersion();

	// Vista and up
	if ((DWORD)(LOBYTE(LOWORD(winver))) >= 6) {
		// Don't hardlink with SHGetKnownFolderPath to allow the game to start on XP too!
		typedef HRESULT (WINAPI *PSHGetKnownFolderPath)(const GUID &rfid, DWORD dwFlags, HANDLE hToken, PWSTR* ppszPath); 

		CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
		
		PSHGetKnownFolderPath GetKnownFolderPath = (PSHGetKnownFolderPath)GetProcAddress(GetModuleHandleA("shell32.dll"), "SHGetKnownFolderPath");
		const GUID FOLDERID_SavedGames = {0x4C5C32FF, 0xBB9D, 0x43b0, {0xB5, 0xB4, 0x2D, 0x72, 0xE5, 0x4E, 0xAA, 0xA4}};

		LPWSTR wszPath = NULL;
		HRESULT hr = GetKnownFolderPath(FOLDERID_SavedGames, KF_FLAG_CREATE | KF_FLAG_NO_ALIAS, NULL, &wszPath);

		if (SUCCEEDED(hr)) {
            strPath = ws2s(wszPath); 
		}

		CoTaskMemFree(wszPath);
		CoUninitialize();
	} else if ((DWORD)(LOBYTE(LOWORD(winver))) == 5) { // XP
		CHAR szPath[MAX_PATH];
		HRESULT hr = SHGetFolderPathA(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, szPath);

		if (SUCCEEDED(hr)) {
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

#endif
