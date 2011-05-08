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
#include "platform/Platform.h"

#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	#include <windows.h>
#else
	#include <sys/stat.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <cassert>

#include "io/Logger.h"

using std::fopen;
using std::fread;
using std::fseek;
using std::ftell;
using std::fwrite;
using std::fclose;
using std::remove;
using std::rename;
using std::malloc;
using std::free;
using std::string;

// File handle 0 is reserved for error.
#define GETHANDLE(handle)  (FILE*)(handle)
#define MAKEHANDLE(handle) (FileHandle)(handle)

const FileHandle INVALID_FILE_HANDLE = (FileHandle)NULL;


long FileTell(FileHandle handle) {
	return ftell(GETHANDLE(handle));
}

static FileHandle FileOpen(const std::string& name, const std::string& mode) {
	
	FILE * handle = fopen(name.c_str(), mode.c_str());
	if(!handle) {
		return INVALID_FILE_HANDLE;
	}
	
	assert(MAKEHANDLE(handle) != INVALID_FILE_HANDLE);
	
	return MAKEHANDLE(handle);
}

FileHandle FileOpenRead(const std::string& name) {
	return FileOpen(name, "rb");
}

FileHandle FileOpenWrite(const std::string& name) {
	return FileOpen(name, "wb");
}

FileHandle FileOpenReadWrite(const std::string& name) {
	return FileOpen(name, "r+");
}

bool FileDelete(const std::string & file) {
	return remove(file.c_str()) == 0;
}

bool FileMove(const std::string & oldname, const std::string & newname) {
	return rename(oldname.c_str(), newname.c_str()) == 0;
}

long FileClose(FileHandle handle) {
	return fclose(GETHANDLE(handle));
}

long FileRead(FileHandle handle, void * adr, size_t size) {
	return fread(adr, 1, size, GETHANDLE(handle));
}

long FileWrite(FileHandle handle, const void * adr, size_t size) {
	return fwrite(adr, 1, size, GETHANDLE(handle));
}

long FileSeek(FileHandle handle, int offset, long mode) {
	if(fseek(GETHANDLE(handle), offset, mode) == 0) {
		return FileTell(handle);
	} else {
		return -1;
	}
}

void * FileLoadMalloc(const std::string& name, size_t * SizeLoadMalloc)
{
	FileHandle handle;
	long	size1, size2;
	unsigned char	* adr;

	handle = FileOpenRead(name);

	if (!handle)
	{
		LogDebug << "FOR failed";
				if (SizeLoadMalloc != NULL) *SizeLoadMalloc = 0;
				return(NULL);

	}

	FileSeek(handle, 0, FILE_SEEK_END);
	size1 = FileTell(handle);
	adr = (unsigned char *)malloc(size1);

	if (!adr)
	{
		LogDebug << "malloc failed";
				FileClose(handle);

				if (SizeLoadMalloc != NULL) *SizeLoadMalloc = 0;

				return NULL;
	}
	
	FileSeek(handle, 0, FILE_SEEK_START);
	size2 = FileRead(handle, adr, size1);
	FileClose(handle);
	
	if (size1 != size2)
	{
		LogDebug << "fread failed " << size1 << " " << size2;
		free(adr);
				if (SizeLoadMalloc != NULL) *SizeLoadMalloc = 0;
				return NULL;
	
	}
	
	if (SizeLoadMalloc != NULL) *SizeLoadMalloc = size1;
	
	return(adr);
}

bool CreateFullPath(const std::string & path)
{
	LogInfo << "CreateFullPath(" << path << ")";
	
	size_t start = 0;
	
	while(true) {
		
		size_t pos = path.find_first_of("/\\", start);
		if(pos == string::npos) {
			break;
		}
		
		pos++;

#if ARX_PLATFORM == ARX_PLATFORM_WIN32
		CreateDirectory(path.substr(0, pos).c_str(), NULL);
#else
		mkdir(path.substr(0, pos).c_str(), 0777);
#endif
		start = pos + 1;
	}
	
#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	return DirectoryExist(path);
#else
	return true;
#endif
}

// TODO stub functions

void * FileLoadMallocZero(const string & name, size_t * SizeLoadMalloc) {
	(void)name;
	if(SizeLoadMalloc) {
		*SizeLoadMalloc = 0;
	}
	return NULL;
}

bool DirectoryExist(const string & _name) {
	(void)_name;
	return false;
}

bool FileExist(const std::string & name) {
	
	FileHandle handle = FileOpenRead(name);
	if(!handle) {
		LogError << "Didn't find " << name;
		return false;
	}
	
	FileClose(handle);
	LogInfo << "Found " << name;
	return true;
}
