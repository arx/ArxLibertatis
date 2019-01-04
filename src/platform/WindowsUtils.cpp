/*
 * Copyright 2015-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include <algorithm>
#include <sstream>

#include <wchar.h>

#include <boost/algorithm/string/trim.hpp>

namespace platform {

void WideString::assign(const char * utf8, size_t utf8_length) {
	if(!dynamic() && utf8_length <= capacity()) {
		// Optimistically assume that the wide length is not longer than the utf8 length
		INT length = MultiByteToWideChar(CP_UTF8, 0, utf8, utf8_length, m_static, capacity());
		if(length || !utf8_length) {
			m_static[length] = L'\0';
			m_size = length;
			return;
		}
	}
	INT length = MultiByteToWideChar(CP_UTF8, 0, utf8, utf8_length, NULL, 0);
	allocate(length);
	MultiByteToWideChar(CP_UTF8, 0, utf8, utf8_length, data(), length);
}

void WideString::allocateDynamic(size_t size) {
	if(dynamic()) {
		str().resize(size, L'\0');
	} else {
		WCHAR backup[ARRAY_SIZE(m_static)];
		std::copy(m_static, m_static + m_size, backup);
		new(reinterpret_cast<char *>(&m_dynamic)) DynamicType(size, L'\0');
		std::copy(backup, backup + m_size, str().begin());
		m_size = size_t(-1);
	}
}

void WideString::allocate(size_t size) {
	if(!dynamic() && size <= capacity()) {
		m_static[size] = L'\0';
		m_size = size;
	} else {
		allocateDynamic(size);
	}
}

void WideString::resize(size_t newSize) {
	size_t oldSize = size();
	if(newSize != oldSize) {
		allocate(newSize);
		if(!dynamic() && newSize > oldSize) {
			std::fill(m_static + oldSize, m_static + newSize, L'\0');
		}
	}
}

void WideString::compact() {
	resize(wcslen(c_str()));
}

std::string WideString::toUTF8(const WCHAR * string, size_t length) {
	std::string utf8(length, '\0');
	// Optimistically assume that the utf8 length is not longer than the wide length
	INT utf8_length = WideCharToMultiByte(CP_UTF8, 0, string, length, &utf8[0], length, 0, 0);
	if(utf8_length || !length) {
		utf8.resize(utf8_length);
	} else {
		// Our assumption failed - /
		utf8_length = WideCharToMultiByte(CP_UTF8, 0, string, length, 0, 0, 0, 0);
		utf8.resize(utf8_length, '\0');
		WideCharToMultiByte(CP_UTF8, 0, string, length, &utf8[0], utf8_length, 0, 0);
	}
	return utf8;
}

std::string WideString::toUTF8(const WCHAR * string) {
	return toUTF8(string, wcslen(string));
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
