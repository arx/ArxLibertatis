/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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

#include "platform/Thread.h"
#include "platform/WindowsUtils.h"

namespace fs {

FileType get_type(const path & p) {
	
	if(p.empty()) {
		return Directory;
	}
	
	DWORD attributes = GetFileAttributesW(platform::WideString(p.string()));
	if(attributes == INVALID_FILE_ATTRIBUTES) {
		return DoesNotExist;
	} else if(attributes & FILE_ATTRIBUTE_DIRECTORY) {
		return Directory;
	} else {
		return RegularFile;
	}
	
}

FileType get_link_type(const path & p) {
	
	if(p.empty()) {
		return Directory;
	}
	
	DWORD attributes = GetFileAttributesW(platform::WideString(p.string()));
	if(attributes == INVALID_FILE_ATTRIBUTES) {
		return DoesNotExist;
	} else if(attributes & FILE_ATTRIBUTE_REPARSE_POINT) {
		return SymbolicLink;
	} else if(attributes & FILE_ATTRIBUTE_DIRECTORY) {
		return Directory;
	} else {
		return RegularFile;
	}
	
}

static std::time_t filetime_to_time_t(const FILETIME & time) {
	
	// Convert FILETIME to time_t: http://support.microsoft.com/default.aspx?scid=KB;en-us;q167296
	LONGLONG value = time.dwHighDateTime;
	value <<= 32;
	value |= time.dwLowDateTime;
	value -= 116444736000000000;
	
	return std::time_t(value / 10000000);
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
		writeTime = filetime_to_time_t(modificationTime);
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
	
	for(int tries = 1;; tries++) {
		
		if(DeleteFileW(platform::WideString(p.string()))) {
			return true;
		}
		
		DWORD error = GetLastError();
		if(error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) {
			return true;
		}
		
		LogWarning << "Failed to remove file " << p << ": " << error << " = "
		           << platform::getErrorString(error) << " (try " << tries << ")";
		
		if(tries < 10 && (error == ERROR_ACCESS_DENIED || error == ERROR_SHARING_VIOLATION ||
		                  error == ERROR_LOCK_VIOLATION)) {
			Sleep(100);
			continue;
		}
		
		return false;
	}
	
}

bool remove_directory(const path & p) {
	
	for(int tries = 1;; tries++) {
		
		if(RemoveDirectoryW(platform::WideString(p.string()))) {
			return true;
		}
		
		DWORD error = GetLastError();
		if(error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) {
			return true;
		}
		if(error == ERROR_DIR_NOT_EMPTY) {
			return false;
		}
		
		LogWarning << "Failed to remove directory " << p << ": " << error << " = "
		           << platform::getErrorString(error) << " (try " << tries << ")";
		
		if(tries < 10 && (error == ERROR_ACCESS_DENIED || error == ERROR_SHARING_VIOLATION ||
		                  error == ERROR_LOCK_VIOLATION)) {
			Sleep(100);
			continue;
		}
		
		return false;
	}
	
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
	
	for(int tries = 1;; tries++) {
		
		if(CopyFileW(platform::WideString(from_p.string()), platform::WideString(to_p.string()), !overwrite)) {
			update_last_write_time(to_p);
			return true;
		}
		
		DWORD error = GetLastError();
		if(error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND || error == ERROR_FILE_EXISTS) {
			return false;
		}
		
		LogWarning << "Failed to copy file from " << from_p << " to " << to_p << ": " << error << " = "
		           << platform::getErrorString(error) << " (try " << tries << ")";
		
		if(tries < 10 && (error == ERROR_ACCESS_DENIED || error == ERROR_SHARING_VIOLATION ||
		                  error == ERROR_LOCK_VIOLATION)) {
			Sleep(100);
			continue;
		}
		
		return false;
	}
	
}

bool rename(const path & old_p, const path & new_p, bool overwrite) {
	
	DWORD flags = MOVEFILE_COPY_ALLOWED | (overwrite ? MOVEFILE_REPLACE_EXISTING : 0);
	for(int tries = 1;; tries++) {
		
		if(MoveFileExW(platform::WideString(old_p.string()), platform::WideString(new_p.string()), flags)) {
			return true;
		}
		
		DWORD error = GetLastError();
		if(error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND || error == ERROR_FILE_EXISTS) {
			return false;
		}
		
		LogWarning << "Failed to move file from " << old_p << " to " << new_p << ": " << error << " = "
		           << platform::getErrorString(error) << " (try " << tries << ")";
		
		if(tries < 10 && (error == ERROR_ACCESS_DENIED || error == ERROR_SHARING_VIOLATION ||
		                  error == ERROR_LOCK_VIOLATION)) {
			Sleep(100);
			continue;
		}
		
		return false;
	}
	
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

directory_iterator::directory_iterator(const path & p) : m_handle(INVALID_HANDLE_VALUE) {
	
	std::string searchPath = (p.empty() ? "." : p.string()) + "\\*";
	
	m_handle = FindFirstFileW(platform::WideString(searchPath), &m_data);
	if(m_handle != INVALID_HANDLE_VALUE) {
		if(!wcscmp(m_data.cFileName, L".") || !wcscmp(m_data.cFileName, L"..")) {
			operator++();
		}
	}
	
}

directory_iterator::~directory_iterator() {
	
	if(m_handle != INVALID_HANDLE_VALUE) {
		FindClose(m_handle);
	}
	
}

directory_iterator & directory_iterator::operator++() {
	
	arx_assert(m_handle != INVALID_HANDLE_VALUE);
	
	do {
		
		if(!FindNextFileW(m_handle, &m_data)) {
			CloseHandle(m_handle);
			m_handle = INVALID_HANDLE_VALUE;
			break;
		}
		
	} while(!wcscmp(m_data.cFileName, L".") || !wcscmp(m_data.cFileName, L".."));
	
	return *this;
}

bool directory_iterator::end() {
	return m_handle != INVALID_HANDLE_VALUE;
}

std::string directory_iterator::name() {
	
	arx_assert(m_handle != INVALID_HANDLE_VALUE);
	
	return platform::WideString::toUTF8(m_data.cFileName);
}

FileType directory_iterator::type() {
	
	arx_assert(m_handle != INVALID_HANDLE_VALUE);
	
	if(m_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		return Directory;
	} else {
		return RegularFile;
	}
	
}

FileType directory_iterator::link_type() {
	
	arx_assert(m_handle != INVALID_HANDLE_VALUE);
	
	if(m_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
		return SymbolicLink;
	} else if(m_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		return Directory;
	} else {
		return RegularFile;
	}
	
}

std::time_t directory_iterator::last_write_time() {
	
	arx_assert(m_handle != INVALID_HANDLE_VALUE);
	
	return filetime_to_time_t(m_data.ftLastWriteTime);
}

u64 directory_iterator::file_size() {
	
	arx_assert(m_handle != INVALID_HANDLE_VALUE);
	
	return (u64(m_data.nFileSizeHigh) << 32) + u64(m_data.nFileSizeLow);
}

} // namespace fs
