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

#include "platform/WindowsUtils.h"

#include <sstream>

#include <boost/algorithm/string/trim.hpp>

namespace platform {

WideString::WideString(const std::string & utf8) {
	INT length = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.length(), NULL, 0);
	resize(length, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.length(), data(), length);
}

std::string WideString::toUTF8(const WCHAR * string, INT length) {
	INT utf8_length = WideCharToMultiByte(CP_ACP, 0, string, length, 0, 0, 0, 0);
	std::string utf8(utf8_length, '\0');
	WideCharToMultiByte(CP_ACP, 0, string, length, &utf8[0], utf8_length, 0, 0);
	return utf8;
}

std::string WideString::toUTF8() const {
	return toUTF8(c_str(), length());
}

std::string getErrorString(WORD error, HMODULE module) {
	
	DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS;
	if(module) {
		flags |= FORMAT_MESSAGE_FROM_HMODULE;
	} else {
		flags |= FORMAT_MESSAGE_FROM_SYSTEM;
	}
	
	LPWSTR buffer = NULL;
	DWORD n = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
	                         module, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	                         reinterpret_cast<LPWSTR>(&buffer), 0, NULL);
	if(n != 0) {
		std::string message = WideString::toUTF8(buffer, n);
		boost::trim(message);
		LocalFree(buffer);
		return message;
	} else {
		std::ostringstream oss;
		oss << "Unknown error (" << error << ").";
		return oss.str();
	}
	
}

} // namespace platform
