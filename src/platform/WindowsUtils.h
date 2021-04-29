/*
 * Copyright 2015-2020 Arx Libertatis Team (see the AUTHORS file)
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
#include <cstring>
#include <string>

#include <windows.h>

#include <boost/range/size.hpp>

namespace fs { class path; }

namespace platform {

/*!
 * Utility class to deal with 'wide' strings required for Unicode support
 * in the Win32 API
 */
class WideString {
	
	typedef std::basic_string<WCHAR> DynamicType;
	
	union {
		WCHAR m_static[512 + 1];
		#if ARX_COMPILER_MSVC
		char m_dynamic[sizeof(DynamicType)];
		#else
		DynamicType m_dynamic;
		#endif
	};
	size_t m_size;
	
	bool dynamic() const { return m_size == size_t(-1); }
	
	DynamicType & str() {
		return *reinterpret_cast<DynamicType *>(&m_dynamic);
	}
	const DynamicType & str() const {
		return *reinterpret_cast<const DynamicType *>(&m_dynamic);
	}
	
	void allocateDynamic(size_t size);
	
public:
	
	size_t capacity() const { return size_t(boost::size(m_static)) - 1; }
	
	WideString(const char * utf8, size_t length) : m_size(0) { assign(utf8, length); }
	explicit WideString(const char * utf8) : m_size(0) { assign(utf8); }
	explicit WideString(const std::string & utf8) : m_size(0) { assign(utf8); }
	explicit WideString(size_t size) : m_size(0) { resize(size); }
	WideString() : m_size(0) { m_static[0] = '\0'; }
	
	~WideString() { if(dynamic()) { str().~DynamicType(); } }
	
	WCHAR * data() { return dynamic() ? &*str().begin() : m_static; }
	const WCHAR * data() const { return dynamic() ? str().data() : m_static; }
	
	const WCHAR * c_str() const { return dynamic() ? str().c_str() : m_static; }
	operator const WCHAR *() const { return c_str(); }
	
	size_t size() const { return dynamic() ? str().size() : m_size; }
	size_t length() const { return size(); }
	
	void reserve(size_t futureSize);
	
	//! Resize the buffer but leave the contents undefined
	void allocate(size_t size);
	
	//! Resize the buffer and fill any new characters with \c L'\0'
	void resize(size_t size);
	
	//! Resize the buffer to the first NULL byte
	void compact();
	
	void assign(const char * utf8, size_t length, size_t offset = 0);
	void assign(const char * utf8) { assign(utf8, std::strlen(utf8)); }
	void assign(const std::string & utf8) { assign(utf8.data(), utf8.length()); }
	
	void append(const char * utf8, size_t length) { assign(utf8, length, size()); }
	void append(const char * utf8) { append(utf8, std::strlen(utf8)); }
	void append(const std::string & utf8) { append(utf8.data(), utf8.length()); }
	
	WideString & operator=(const char * utf8) {
		assign(utf8);
		return *this;
	}
	WideString & operator=(const std::string & utf8) {
		assign(utf8);
		return *this;
	}
	
	void assign(const WCHAR * text, size_t length, size_t offset = 0);
	void assign(const WCHAR * text) { assign(text, std::wcslen(text)); }
	
	void append(const WCHAR * text, size_t length) { assign(text, length, size()); }
	void append(const WCHAR * text) { append(text, std::wcslen(text)); }
	
	WideString & operator=(const WCHAR * text) {
		assign(text);
		return *this;
	}
	
	std::string toUTF8() const;
	static std::string toUTF8(const WCHAR * string, size_t length);
	static std::string toUTF8(const WCHAR * string);
	static std::string toUTF8(const std::basic_string<WCHAR> & str) {
		return toUTF8(str.data(), str.length());
	}
	
};

class WinPath : public WideString {
	
public:
	
	explicit WinPath(const fs::path & path) : WideString() { assign(path); }
	explicit WinPath(size_t size) : WideString(size) { }
	WinPath() : WideString() { }
	
	using WideString::assign;
	void assign(const fs::path & path);
	
	using WideString::operator=;
	WideString & operator=(const fs::path & path) {
		assign(path);
		return *this;
	}
	
};

std::string getErrorString(DWORD error = GetLastError(), HMODULE module = NULL);

template <typename FunctionType>
FunctionType getProcAddress(HMODULE module, const char * symbol) {
	return reinterpret_cast<FunctionType>(reinterpret_cast<void(*)()>(GetProcAddress(module, symbol)));
}

} // namespace

#endif

#endif // ARX_PLATFORM_WINDOWSUTILS_H
