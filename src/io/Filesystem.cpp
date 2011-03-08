/*
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/

#include "io/Filesystem.h"

#include <cstdio>
#include <cassert>

#include <windows.h>

#include "io/Logger.h"

long KillAllDirectory(const char * path) {
	LogInfo << "KillAllDirectory "<< path;
	
	HANDLE idx;
	WIN32_FIND_DATA fl;
	char pathh[512];
	sprintf(pathh, "%s*.*", path);
	
	if ((idx = FindFirstFile(pathh, &fl)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			LogDebug << " - \"" << fl.cFileName << "\"\n";
			if (fl.cFileName[0] != '.')
			{
				if (fl.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					sprintf(pathh, "%s%s\\", path, fl.cFileName);
					KillAllDirectory(pathh);
					RemoveDirectory(pathh);
				}
				else
				{
					sprintf(pathh, "%s%s", path, fl.cFileName);
					DeleteFile(pathh);
				}
			}

		}
		while(FindNextFile(idx, &fl));

		FindClose(idx);
	}

	RemoveDirectory(path);
	return 1;
}

bool DirectoryExist(const string & _name) {
	
	// FindFirstFile does not expect a slash at the end
	string name = _name;
	if(!name.empty() && *name.rbegin() == '\\') {
		name.resize(name.length() - 1);
	}
	
	HANDLE idx;
	WIN32_FIND_DATA fd;
	if ((idx = FindFirstFile(name.c_str(), &fd)) != INVALID_HANDLE_VALUE) {
		FindClose(idx);
		char initial[256];
		GetCurrentDirectory(255, initial);
		
		if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			return true;
		}
	}
	
	return false;
}

// File handle 0 is reserved for error.
#define GETHANDLE(x)  (HANDLE)((ULONG_PTR)(handle) - 1)
#define MAKEHANDLE(x) (FileHandle)((ULONG_PTR)(handle) + 1)
const FileHandle INVALID_FILE_HANDLE = (FileHandle)NULL;

bool FileExist(const char * name) {
	
	FileHandle handle = FileOpenRead(name);
	if(!handle) {
		LogError << "Didn't find " << name;
		return false;
	}
	
	FileClose(handle);
	LogInfo << "Found " << name;
	return true;
}

long FileTell(FileHandle handle) {
	return (SetFilePointer(GETHANDLE(handle), 0, NULL, FILE_CURRENT));
}

static FileHandle FileOpen(const char * name, DWORD mode, DWORD opt) {
	
	HANDLE handle = CreateFile(name, mode, 0, NULL, opt, 0, 0);

	if(handle == INVALID_HANDLE_VALUE) {
		LogError << "Can't open " << name;
		return INVALID_FILE_HANDLE;
	}
	LogInfo << "Opened "<< name;
	
	assert(MAKEHANDLE(handle) != INVALID_FILE_HANDLE);
	
	return MAKEHANDLE(handle);
}

FileHandle FileOpenRead(const char * name) {
	return FileOpen(name, GENERIC_READ, OPEN_EXISTING);
}

FileHandle FileOpenWrite(const char * name) {
	return FileOpen(name, GENERIC_WRITE, CREATE_ALWAYS);
}

FileHandle FileOpenReadWrite(const char * name) {
	return FileOpen(name, GENERIC_READ|GENERIC_WRITE, OPEN_ALWAYS);
}

bool FileDelete(const std::string & file) {
	return DeleteFile(file.c_str()) == TRUE;
}

bool FileMove(const std::string & oldname, const std::string & newname) {
	return MoveFile(oldname.c_str(), newname.c_str()) == TRUE;
}

long FileClose(FileHandle handle) {
	return CloseHandle(GETHANDLE(handle));
}

long FileRead(FileHandle handle, void * adr, size_t size) {
	DWORD ret;
	ReadFile(GETHANDLE(handle), adr, size, &ret, NULL);
	return ret;
}

long FileWrite(FileHandle handle, const void * adr, size_t size) {
	DWORD ret;
	WriteFile(GETHANDLE(handle), adr, size, &ret, NULL);
	return ret;
}

long FileSeek(FileHandle handle, int offset, long mode) {
	return SetFilePointer(GETHANDLE(handle), offset, NULL, mode);
}

void	* FileLoadMallocZero(const char * name, size_t * SizeLoadMalloc)
{
	FileHandle handle;
	long	size1, size2;
	unsigned char	* adr;

	handle = FileOpenRead(name);

	if (!handle)
	{
		
				if (SizeLoadMalloc != NULL) *SizeLoadMalloc = 0;

				return NULL;
	}

	FileSeek(handle, 0, FILE_SEEK_END);
	size1 = FileTell(handle) + 2;
	adr = (unsigned char *)malloc(size1);

	if (!adr)
	{
				FileClose(handle);

				if (SizeLoadMalloc != NULL) *SizeLoadMalloc = 0;

				return NULL;
	}

	FileSeek(handle, 0, FILE_SEEK_START);
	size2 = FileRead(handle, adr, size1 - 2);
	FileClose(handle);

	if (size1 != size2 + 2)
	{
		free(adr);
		
				if (SizeLoadMalloc != NULL) *SizeLoadMalloc = 0;

				return NULL;
	}

	if (SizeLoadMalloc != NULL) *SizeLoadMalloc = size1;

	adr[size1-1] = 0;
	adr[size1-2] = 0;
	return(adr);
}

void * FileLoadMalloc(const char * name, size_t * SizeLoadMalloc)
{
	FileHandle handle;
	long	size1, size2;
	unsigned char	* adr;

	handle = FileOpenRead(name);

	if (!handle)
	{
				if (SizeLoadMalloc != NULL) *SizeLoadMalloc = 0;
				return(NULL);

	}

	FileSeek(handle, 0, FILE_SEEK_END);
	size1 = FileTell(handle);
	adr = (unsigned char *)malloc(size1);

	if (!adr)
	{
				FileClose(handle);

				if (SizeLoadMalloc != NULL) *SizeLoadMalloc = 0;

				return NULL;
	}
	
	FileSeek(handle, 0, FILE_SEEK_START);
	size2 = FileRead(handle, adr, size1);
	FileClose(handle);
	
	if (size1 != size2)
	{
		free(adr);
				if (SizeLoadMalloc != NULL) *SizeLoadMalloc = 0;
				return NULL;
	
	}
	
	if (SizeLoadMalloc != NULL) *SizeLoadMalloc = size1;
	
	return(adr);
}

bool CreateFullPath(const string & path) {
	
	LogInfo << "CreateFullPath(" << path << ")";
	
	size_t last = 0;
	
	while(true) {
		
		size_t pos = path.find_first_of("/\\", last);
		if(pos == string::npos) {
			break;
		}
		
		pos++;
		CreateDirectory(path.substr(0, pos).c_str(), NULL);
		last = pos;
	}
	
	return DirectoryExist(path);
}

