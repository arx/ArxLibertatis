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

void defineXdgDirectories() {
	
	const char * _home = getenv("HOME");
	std::string home = _home ? _home : "~";
	
	setenv("XDG_DATA_HOME", (home + "/.local/share").c_str(), 0);
	setenv("XDG_CONFIG_HOME", (home + "/.config").c_str(), 0);
	setenv("XDG_DATA_DIRS", "/usr/local/share/:/usr/share/", 0);
	setenv("XDG_CONFIG_DIRS", "/etc/xdg", 0);
	setenv("XDG_CACHE_HOME", (home + "/.cache").c_str(), 0);
}

#endif
