/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#include "io/fs/FilePath.h"
#include "io/log/Logger.h"

using std::string;

namespace fs {

static std::string getLastErrorString() {
	LPSTR lpBuffer = NULL;
	std::string strError;

	if(FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 
					  NULL,
					  GetLastError(),
					  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					  (LPSTR) &lpBuffer,
					  0,
					  NULL) != 0) {
		strError = lpBuffer;
		LocalFree( lpBuffer );
	} else {
		std::ostringstream buffer; //! Buffer for the log message excluding level, file and line.
		buffer << "Unknown error (" << GetLastError() << ").";
		strError = buffer.str();
	}
	
	return strError;
}


bool exists(const path & p) {
	if(p.empty()) {
		return true;
	}

	DWORD result = GetFileAttributesA(p.string().c_str());
	return result != INVALID_FILE_ATTRIBUTES;
}

bool is_directory(const path & p) {
	if(p.empty()) {
		return true;
	}
	
	DWORD attributes = GetFileAttributesA(p.string().c_str());
	if(attributes == INVALID_FILE_ATTRIBUTES)
		return false;

	return (attributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
}

bool is_regular_file(const path & p) {
	DWORD attributes = GetFileAttributesA(p.string().c_str());
	if(attributes == INVALID_FILE_ATTRIBUTES)
		return false;

	return (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;	
}

std::time_t last_write_time(const path & p) {
	FILETIME creationTime;
	FILETIME accessTime;
	FILETIME modificationTime;

	HANDLE hFile = CreateFileA(p.string().c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return 0;

	std::time_t writeTime = 0;
	BOOL res = GetFileTime(hFile, &creationTime, &accessTime, &modificationTime);
	if(res)	{
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
	HANDLE hFile = CreateFileA(p.string().c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return (u64)-1;

	u64 fileSize = (u64)-1;

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
		succeeded &= RemoveDirectoryA(p.string().c_str()) == TRUE;
		if(!succeeded) {
			LogWarning << "RemoveDirectoryA(" << p << ") failed! " << getLastErrorString();
		}
	} else {
		succeeded &= DeleteFileA(p.string().c_str()) == TRUE;
		if(!succeeded) {
			LogWarning << "DeleteFileA(" << p << ") failed! " << getLastErrorString();
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

	bool ret = CreateDirectoryA(p.string().c_str(), NULL) == TRUE;
	if(!ret) {
		int lastError = GetLastError();
		ret = lastError == ERROR_ALREADY_EXISTS;

		if(!ret) {
			LogWarning << "CreateDirectoryA(" << p << ", NULL) failed! " << getLastErrorString();
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
	
	HANDLE handle = CreateFileA(p.string().c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if(handle == INVALID_HANDLE_VALUE) {
		return;
	}
	
	FILETIME filetime;
	GetSystemTimeAsFileTime(&filetime);
	SetFileTime(handle, &filetime, &filetime, &filetime);
	
	CloseHandle(handle);
}

bool copy_file(const path & from_p, const path & to_p, bool overwrite) {
	bool ret = CopyFileA(from_p.string().c_str(), to_p.string().c_str(), !overwrite) == TRUE;
	if(!ret) {
		LogWarning << "CopyFileA(" << from_p << ", " << to_p << ", " << !overwrite << ") failed! " << getLastErrorString();
	} else {
		update_last_write_time(to_p);
	}
	return ret;
}

bool rename(const path & old_p, const path & new_p, bool overwrite) {
	DWORD flags = overwrite ? MOVEFILE_REPLACE_EXISTING : 0;
	bool ret = MoveFileExA(old_p.string().c_str(), new_p.string().c_str(), flags) == TRUE;
	if(!ret) {
		LogWarning << "MoveFileExA(" << old_p << ", " << new_p << ", " << flags << ") failed! "
		           << getLastErrorString();
	}
	return ret;
}

path current_path() {
	std::vector<char> buffer(GetCurrentDirectoryA(0, NULL));
	DWORD length = GetCurrentDirectoryA(buffer.size(), &buffer.front());
	if(length == 0 || length + 1 >= buffer.size()) {
		buffer[0] = '\0';
	}
	return path(&buffer.front());
}

struct directory_iterator_data {
	WIN32_FIND_DATAA findData;
	HANDLE			 findHandle;
};

directory_iterator::directory_iterator(const path & p) {
	
	directory_iterator_data* itData = new directory_iterator_data();
	handle = itData;

	string searchPath = (p.empty() ? "." : p.string()) + "\\*";

	itData->findHandle = FindFirstFileA(searchPath.c_str(), &itData->findData); 
	if (itData->findHandle != INVALID_HANDLE_VALUE)
	{
		this->operator++();
	}	
};

directory_iterator::~directory_iterator() {

	directory_iterator_data* itData = (directory_iterator_data*)handle;

	if(itData->findHandle != INVALID_HANDLE_VALUE) {
		::FindClose(itData->findHandle);
	}
		
	delete itData;	
}

directory_iterator & directory_iterator::operator++() {

	directory_iterator_data* itData = (directory_iterator_data*)handle;
	arx_assert(itData->findHandle != INVALID_HANDLE_VALUE);
	
	bool cont = true;
	while(cont)
    {
		cont = FindNextFileA(itData->findHandle, &itData->findData) == TRUE;

        if(cont && itData->findData.cFileName[0] != '.')
        {
            break;
        }
    }

    if(!cont)
    {
        FindClose(itData->findHandle);
		itData->findHandle = INVALID_HANDLE_VALUE;
	}
	
	return *this;
}

bool directory_iterator::end() {
	directory_iterator_data* itData = (directory_iterator_data*)handle;
	return itData->findHandle == INVALID_HANDLE_VALUE;
}

string directory_iterator::name() {
	directory_iterator_data* itData = (directory_iterator_data*)handle;
	arx_assert(itData->findHandle != INVALID_HANDLE_VALUE);

	return itData->findData.cFileName;
}

bool directory_iterator::is_directory() {
	directory_iterator_data* itData = (directory_iterator_data*)handle;
	arx_assert(itData->findHandle != INVALID_HANDLE_VALUE);

	return (itData->findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
}

bool directory_iterator::is_regular_file() {
	directory_iterator_data* itData = (directory_iterator_data*)handle;
	arx_assert(itData->findHandle != INVALID_HANDLE_VALUE);

	return (itData->findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

} // namespace fs
