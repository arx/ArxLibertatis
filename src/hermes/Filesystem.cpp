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

#include <hermes/Filesystem.h>
#include <windows.h>
#include <cstdio>

long KillAllDirectory(const char * path) {
	printf("KillAllDirectory(%s)\n", path);
	
	WIN32_FIND_DATA FileInformation;             // File information
	
	HANDLE idx;
	WIN32_FIND_DATA fl;
	char pathh[512];
	sprintf(pathh, "%s*.*", path);
	
	if ((idx = FindFirstFile(pathh, &fl)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			printf(" - \"%s\"\n", fl.cFileName);
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

// TODO return should be bool
bool DirectoryExist(const char * name)
{
	HANDLE idx;
	WIN32_FIND_DATA fd;

	if ((idx = FindFirstFile(name, &fd)) == -1)
	{
		FindClose(idx);
		char initial[256];
		GetCurrentDirectory(255, initial);

		if (SetCurrentDirectory(name) == 0) // success
		{
			SetCurrentDirectory(initial);
			return true;
		}

		SetCurrentDirectory(initial);
		return false;
	}

	FindClose(idx);
	return true;
}

// TODO return should be bool
bool FileExist(const char * name)
{
	FileHandle i;

	if((i = FileOpenRead(name)) == 0) {
		printf("\e[1;31mDidn't find\e[m\t%s\n", name);
		return false;
	}
	
	FileCloseRead(i);
	printf("\e[1;32mFound\e[m\t%s\n", name);
	return true;
}

FileHandle	FileOpenRead(const char * name)
{
	long	handle;
	handle = CreateFile(name, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);

	if(handle < 0) {
		printf("\e[1;31mCan't open\e[m\t%s\n", name);
		return(0);
	}
	printf("\e[1;32mOpened\e[m\t%s\n", name);
	return(handle + 1);
}

long	FileTell(FileHandle handle)
{
	
	return (SetFilePointer((int)handle - 1, 0, NULL, FILE_CURRENT));
}

FileHandle	FileOpenWrite(const char * name)
{
	printf("FileOpenWrite(%s)\n", name);
	FileHandle	handle;

	handle = CreateFile(name, GENERIC_READ | GENERIC_WRITE, TRUNCATE_EXISTING, NULL, 0, 0, 0);

	if (handle < 0)	{
		return(0);
	}

	CloseHandle(handle);
	handle = CreateFile(name, GENERIC_WRITE, 0, NULL, 0, 0, 0);

	if (handle < 0) {
		return(0);
	}

	return(handle + 1);
}

long FileCloseRead(FileHandle handle)
{
	return(CloseHandle((int)handle - 1));
}

long FileCloseWrite(FileHandle handle)
{
	//_commit((int)handle - 1);
	return(CloseHandle((int)handle - 1));
}

long	FileRead(FileHandle handle, void * adr, long size)
{
	DWORD ret;
	ReadFile(handle - 1, adr, size, &ret, NULL);
	return ret;
}

long	FileWrite(FileHandle handle, const void * adr, long size)
{
	DWORD ret;
	WriteFile(handle - 1, adr, size, &ret, NULL);
	return ret;
}

long	FileSeek(long handle, int offset, long mode)
{
	return SetFilePointer((int)handle - 1, offset, NULL, mode);
}

void	* FileLoadMallocZero(const char * name, size_t * SizeLoadMalloc)
{
	long	handle;
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
				FileCloseRead(handle);

				if (SizeLoadMalloc != NULL) *SizeLoadMalloc = 0;

				return NULL;
	}

	FileSeek(handle, 0, FILE_SEEK_START);
	size2 = FileRead(handle, adr, size1 - 2);
	FileCloseRead(handle);

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

void	* FileLoadMalloc(const char * name, size_t * SizeLoadMalloc)
{
	long	handle;
	long	size1, size2;
	unsigned char	* adr;

retry:
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
				FileCloseRead(handle);

				if (SizeLoadMalloc != NULL) *SizeLoadMalloc = 0;

				return NULL;
	}
	
	FileSeek(handle, 0, FILE_SEEK_START);
	size2 = FileRead(handle, adr, size1);
	FileCloseRead(handle);
	
	if (size1 != size2)
	{
		free(adr);
				if (SizeLoadMalloc != NULL) *SizeLoadMalloc = 0;
				return NULL;
	
	}
	
	if (SizeLoadMalloc != NULL) *SizeLoadMalloc = size1;
	
	return(adr);
}

