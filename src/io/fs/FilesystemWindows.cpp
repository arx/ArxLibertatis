/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "io/fs/Filesystem.h"

#include "Configure.h"

#include <vector>

#include <windows.h>
#include <wchar.h>

#include "io/fs/FilePath.h"
#include "io/log/Logger.h"

#include "platform/WindowsUtils.h"

namespace fs {

bool exists(const path & p) {
	
	if(p.empty()) {
		return true;
	}
	
	DWORD result = GetFileAttributesW(platform::WideString(p.string()));
	return result != INVALID_FILE_ATTRIBUTES;
}

bool is_directory(const path & p) {
	
	if(p.empty()) {
		return true;
	}
	
	DWORD attributes = GetFileAttributesW(platform::WideString(p.string()));
	if(attributes == INVALID_FILE_ATTRIBUTES) {
		return false;
	}
	
	return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool is_regular_file(const path & p) {
	
	DWORD attributes = GetFileAttributesW(platform::WideString(p.string()));
	if(attributes == INVALID_FILE_ATTRIBUTES) {
		return false;
	}
	
	return (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

std::time_t last_write_time(const path & p) {
	
	FILETIME creationTime;
	FILETIME accessTime;
	FILETIME modificationTime;
	
	HANDLE hFile = CreateFileW(platform::WideString(p.string()), GENERIC_READ,
	                           FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
	                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE) {
		return 0;
	}
	
	std::time_t writeTime = 0;
	BOOL res = GetFileTime(hFile, &creationTime, &accessTime, &modificationTime);
	if(res) {
		// Convert FILETIME to time_t: http://support.microsoft.com/default.aspx?scid=KB;en-us;q167296
		LONGLONG value = modificationTime.dwHighDateTime;
		value <<= 32;
		value |= modificationTime.dwLowDateTime;
		value -= 116444736000000000;
		writeTime = (std::time_t)(value / 10000000);
	}
	
	::CloseHandle(hFile);
	
	return writeTime;
}

u64 file_size(const path & p) {
	
	HANDLE hFile = CreateFileW(platform::WideString(p.string()), GENERIC_READ,
	                           FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
	                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE) {
		return u64(-1);
	}
	
	u64 fileSize = u64(-1);
	LARGE_INTEGER retSize;
	BOOL res = GetFileSizeEx(hFile, &retSize);
	if(res) {
		fileSize = retSize.QuadPart;
	}
	
	::CloseHandle(hFile);
	
	return fileSize;
}

bool remove(const path & p) {
	
	bool succeeded = true;
	
	if(is_directory(p)) {
		succeeded &= RemoveDirectoryW(platform::WideString(p.string())) == TRUE;
		if(!succeeded) {
			LogWarning << "RemoveDirectory(" << p << ") failed: " << platform::getErrorString();
		}
	} else {
		succeeded &= DeleteFileW(platform::WideString(p.string())) == TRUE;
		if(!succeeded) {
			LogWarning << "DeleteFile(" << p << ") failed: " << platform::getErrorString();
		}
	}
	
	return succeeded;
}

bool remove_all(const path & p) {
	
	if(!exists(p)) {
		return true;
	}
	
	bool succeeded = true;
	
	if(is_directory(p)) {
		for(directory_iterator it(p); !it.end(); ++it) {
			if(it.is_regular_file()) {
				succeeded &= remove(p / it.name());
			} else {
				succeeded &= remove_all(p / it.name());
			}
		}
	}
	
	succeeded &= remove(p);
	
	return succeeded;
}

bool create_directory(const path & p) {
	
	if(p.empty()) {
		return true;
	}
	
	bool ret = CreateDirectoryW(platform::WideString(p.string()), NULL) == TRUE;
	if(!ret) {
		int lastError = GetLastError();
		ret = lastError == ERROR_ALREADY_EXISTS;
		if(!ret) {
			LogWarning << "CreateDirectory(" << p << ", NULL) failed: "
			           << platform::getErrorString();
		}
	}
	
	return ret;
}

bool create_directories(const path & p) {
	
	if(p.empty()) {
		return true;
	}
	
	path parent = p.parent();
	if(!exists(parent)) {
		if(!create_directories(parent)) {
			return false;
		}
	}
	
	return create_directory(p);
}

static void update_last_write_time(const path & p) {
	
	HANDLE handle = CreateFileW(platform::WideString(p.string()), GENERIC_WRITE, 0,
	                            NULL, OPEN_EXISTING, 0, NULL);
	if(handle == INVALID_HANDLE_VALUE) {
		return;
	}
	
	FILETIME filetime;
	GetSystemTimeAsFileTime(&filetime);
	SetFileTime(handle, &filetime, &filetime, &filetime);
	
	CloseHandle(handle);
}

bool copy_file(const path & from_p, const path & to_p, bool overwrite) {
	
	bool ret = CopyFileW(platform::WideString(from_p.string()),
	                     platform::WideString(to_p.string()), !overwrite) == TRUE;
	if(!ret) {
		LogWarning << "CopyFile(" << from_p << ", " << to_p << ", " << !overwrite
		           << ") failed: " << platform::getErrorString();
	} else {
		update_last_write_time(to_p);
	}
	
	return ret;
}

bool rename(const path & old_p, const path & new_p, bool overwrite) {
	
	DWORD flags = overwrite ? MOVEFILE_REPLACE_EXISTING : 0;
	bool ret = MoveFileExW(platform::WideString(old_p.string()),
	                       platform::WideString(new_p.string()), flags) == TRUE;
	if(!ret) {
		LogWarning << "MoveFileEx(" << old_p << ", " << new_p << ", " << flags << ") failed: "
		           << platform::getErrorString();
	}
	
	return ret;
}

path current_path() {
	
	platform::WideString buffer;
	buffer.allocate(buffer.capacity());
	
	DWORD length = GetCurrentDirectoryW(buffer.size(), buffer.data());
	if(length > buffer.size()) {
		buffer.allocate(length);
		length = GetCurrentDirectoryW(buffer.size(), buffer.data());
	}
	
	if(length == 0 || length > buffer.size()) {
		return path();
	}
	
	buffer.resize(length);
	
	return path(buffer.toUTF8());
}

directory_iterator::directory_iterator(const path & p) : m_handle(INVALID_HANDLE_VALUE), m_buffer(NULL) {
	
	std::string searchPath = (p.empty() ? "." : p.string()) + "\\*";
	
	WIN32_FIND_DATAW * data = new WIN32_FIND_DATAW;
	m_handle = FindFirstFileW(platform::WideString(searchPath), data);
	if(m_handle != INVALID_HANDLE_VALUE) {
		m_buffer = data;
		if(!wcscmp(data->cFileName, L".") || !wcscmp(data->cFileName, L"..")) {
			operator++();
		}
	} else {
		delete data;
	}
	
}

directory_iterator::~directory_iterator() {
	
	if(m_handle != INVALID_HANDLE_VALUE) {
		FindClose(m_handle);
	}
	
	delete reinterpret_cast<WIN32_FIND_DATAW *>(m_buffer);
}

directory_iterator & directory_iterator::operator++() {
	
	arx_assert(m_buffer != NULL);
	arx_assert(m_handle != INVALID_HANDLE_VALUE);
	
	WIN32_FIND_DATAW * data = reinterpret_cast<WIN32_FIND_DATAW *>(m_buffer);
	
	do {
		
		if(!FindNextFileW(m_handle, data)) {
			delete data;
			m_buffer = NULL;
			break;
		}
		
	} while(!wcscmp(data->cFileName, L".") || !wcscmp(data->cFileName, L".."));
	
	return *this;
}

bool directory_iterator::end() {
	return !m_buffer;
}

std::string directory_iterator::name() {
	
	arx_assert(m_buffer != NULL);
	
	const WIN32_FIND_DATAW * data = reinterpret_cast<const WIN32_FIND_DATAW *>(m_buffer);
	
	return platform::WideString::toUTF8(data->cFileName);
}

bool directory_iterator::is_directory() {
	
	arx_assert(m_buffer != NULL);
	
	const WIN32_FIND_DATAW * data = reinterpret_cast<const WIN32_FIND_DATAW *>(m_buffer);
	
	return (data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool directory_iterator::is_regular_file() {
	
	arx_assert(m_buffer != NULL);
	
	const WIN32_FIND_DATAW * data = reinterpret_cast<const WIN32_FIND_DATAW *>(m_buffer);
	
	return (data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

} // namespace fs
