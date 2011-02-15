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
#ifndef EVE_LOAD
#define EVE_LOAD

class PakFile;
class PakDirectory;
class PakReader;

typedef long FileHandle;

#include <stddef.h>
#include <cstdio> // for FILE

#define PACK_MAX_FREAD	(256)

struct PakFileHandle {
	
	PakReader * reader;
	
	bool active;
	void * iID;
	int offset;
	PakFile * file;
	
	FileHandle truefile;
	
};

class PakReader {
	
private:
	
	const char * fat;
	FILE * file;
	PakFileHandle tPackFile[PACK_MAX_FREAD];
	
public:
	
	const char * pakname;
	PakDirectory * root;
	
private:
	
	int ReadFAT_int();
	char* ReadFAT_string();
	
public:
	
	PakReader();
	~PakReader();
	
	PakFile * getFile(const char * name);
	
	bool Open(const char * pakfile);
	void Close();
	bool Read(const char * name, void * buf);
	void * ReadAlloc(const char * name , size_t * sizeRead);
	int GetSize(const char * name);
	
	PakFileHandle * fOpen(const char * name, const char * mode);
	int fClose(PakFileHandle * h);
	size_t fRead(void * buf, size_t size, size_t n, PakFileHandle * h);
	int fSeek(PakFileHandle * h, int off, long whence);
	long fTell(PakFileHandle * h);
	
}; 

#endif
