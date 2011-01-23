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


#ifndef ARX_HERMES_FILESYSTEM_H
#define ARX_HERMES_FILESYSTEM_H

#include <stddef.h>

#define PATH_SEPERATOR_STR "/"
#define PATH_SEPERATOR_CHR '/'

#define	FILE_SEEK_START 0
#define	FILE_SEEK_CURRENT 1
#define	FILE_SEEK_END 2

typedef long FileHandle;

long KillAllDirectory(const char * path);

bool FileExist(const char * name);
bool DirectoryExist(const char * name);
FileHandle FileOpenRead(const char * name);
FileHandle FileOpenWrite(const char * name);
long FileCloseWrite(FileHandle h);
long FileCloseRead(FileHandle h);
long FileRead(FileHandle h, void * adr, long size);
long FileWrite(FileHandle h, const void * adr, long size);
void * FileLoadMalloc(const char * name, size_t * sizeLoaded = 0);
void * FileLoadMallocZero(const char * name, size_t * sizeLoaded = 0);
long FileSeek(FileHandle handle, int offset, long mode);
long FileTell(FileHandle handle);

#endif // ARX_HERMES_FILESYSTEM_H
