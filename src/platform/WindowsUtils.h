/*
 * Copyright 2015 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_PLATFORM_WINDOWSUTILS_H
#define ARX_PLATFORM_WINDOWSUTILS_H

#include "platform/Platform.h"

#if ARX_PLATFORM == ARX_PLATFORM_WIN32

#include <stddef.h>
#include <string>

#include <windows.h>

namespace platform {

/*!
 * Utility class to deal with 'wide' strings required for Unicode support
 * in the Win32 API
 */
class WideString : public std::basic_string<WCHAR> {
	
	typedef std::basic_string<WCHAR> Base;
	
public:
	
	/* implicit */ WideString(const std::string & utf8);
	explicit WideString(size_t size) : Base(size, L'\0') { }
	WideString() : Base() { }
	
	WCHAR * data() { return &*begin(); }
	const WCHAR * data() const { return Base::data(); }
	
	operator const WCHAR *() const { return c_str(); }
	
	std::string toUTF8() const;
	static std::string toUTF8(const WCHAR * string, INT length = -1);
	
};

std::string getErrorString(WORD error = GetLastError(), HMODULE module = NULL);

} // namespace

#endif

#endif // ARX_PLATFORM_WINDOWSUTILS_H
