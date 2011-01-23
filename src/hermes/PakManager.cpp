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
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// HERMESMain
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		HUM...hum...
//
// Updates: (date) (person) (update)
//
// Code: Sébastien Scieux
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <cstring>
#include <cstdlib>

#include <hermes/PakManager.h>
#include <hermes/PakReader.h>F
#include <hermes/PakEntry.h>
#include <hermes/Filesystem.h>


using std::vector;


// TODO prefer real files over those in PAK?

PakManager * pPakManager = NULL;

bool PAK_AddPak(const char * pakfile) {
	
	if(!pPakManager) {
		pPakManager = new PakManager();
	}
	
	return pPakManager->AddPak(pakfile);
}

void PAK_Close() {
	
	if(pPakManager) {
		delete pPakManager;
	}
	
	pPakManager = NULL;
}

void * _PAK_FileLoadMallocZero(const char * name, size_t * sizeRead) {
	
	size_t size = pPakManager->GetSize(name);
	if(size == 0) {
		if(sizeRead) {
			*sizeRead = size;
		}
		return NULL;
	}
	
	char * mem = (char *)malloc(size + 2);
	
	if(!pPakManager->Read(name, mem)) {
		delete mem;
		if(sizeRead) {
			*sizeRead = 0;
		}
		return NULL;
	}
	
	mem[size] = 0;
	mem[size + 1] = 0;
	
	if(sizeRead) {
		*sizeRead = size + 2;
	}
	
	return mem;
}

bool _PAK_DirectoryExist(const char * name) {
	
	size_t len = strlen(name);
	char temp[256];
	strcpy(temp, name); // TODO this copy can be avoided
	if(len != 0 && temp[len - 1] != '\\' && temp[len - 1] != '/') {
		strcat(temp, "\\");
	}
	
	vector<PakDirectory *> pvRepertoire(pPakManager->ExistDirectory(temp));
	
	if(pvRepertoire.empty()) {
		return false;
	}
	
	return true;
}

bool PAK_DirectoryExist(const char * name) {
	
	if(_PAK_DirectoryExist(name)) {
		return true;
	}
	
	return DirectoryExist(name);
}

bool PAK_FileExist(const char * name) {
	
	if(pPakManager->ExistFile(name)) {
		return true;
	}
	
	return FileExist(name);
}

void * PAK_FileLoadMalloc(const char * name, size_t * sizeLoaded) {
	
	void * ret = NULL;
	
	ret = pPakManager->ReadAlloc(name, sizeLoaded);
	
	if(!ret) {
		ret = FileLoadMalloc(name, sizeLoaded);
	}
	
	return ret;
}

void * PAK_FileLoadMallocZero(const char * name, size_t * sizeLoaded) {
	
	void * ret = NULL;
	
	ret = _PAK_FileLoadMallocZero(name, sizeLoaded);
	
	if(!ret) {
		ret = FileLoadMallocZero(name, sizeLoaded);
	}
	
	return ret;
}

long PAK_ftell(PakFileHandle * pfh) {
	
	assert(pfh->active);
	
	if(pfh->reader) {
		return pfh->reader->fTell(pfh);
	}
	
	return FileTell(pfh->truefile);
}

PakFileHandle * PAK_fopen(const char * filename) {
	
	PakFileHandle * pfh = pPakManager->fOpen(filename);
	if(pfh) {
		return pfh;
	}
	
	FileHandle fh = FileOpenRead(filename);
	if(!fh) {
		return NULL;
	}
	
	pfh = new PakFileHandle;
	if(!pfh) {
		return NULL;
	}
	
	pfh->active = true;
	pfh->reader = NULL;
	pfh->truefile = fh;
	
	return pfh;
}

int PAK_fclose(PakFileHandle * pfh) {
	
	assert(pfh->active);
	
	if(pfh->reader) {
		return pfh->reader->fClose(pfh);
	}
	
	int ret = FileCloseRead(pfh->truefile);
	
	pfh->active = false;
	delete pfh;
	
	return ret;
}

size_t PAK_fread(void * buffer, size_t size, size_t count, PakFileHandle * pfh) {
	
	assert(pfh->active);
	
	if(pfh->reader) {
		return pfh->reader->fRead(buffer, size, count, pfh);
	}
	
	return FileRead(pfh->truefile, buffer, size * count);
}

int PAK_fseek(PakFileHandle * pfh, int offset, long origin) {
	
	assert(pfh->active);
	
	if(pfh->reader) {
		return pfh->reader->fSeek(pfh, offset, origin);
	}
	
	return FileSeek(pfh->truefile, offset, origin);
}

PakManager::PakManager() : loadedPaks() {
}

PakManager::~PakManager() {
	
	for(vector<PakReader *>::iterator i = loadedPaks.begin(); i != loadedPaks.end(); i++) {
		delete(*i);
	}
	
	loadedPaks.clear();
}

bool PakManager::AddPak(const char * pakname) {
	
	PakReader * reader = new PakReader();
	if(!reader->Open(pakname)) {
		delete reader;
		return false;
	}
	
	if(!reader->root) {
		delete reader;
		return false;
	}
	
	RemovePak(pakname);
	
	loadedPaks.push_back(reader);
	return true;
}

bool PakManager::RemovePak(const char * pakname) {
	
	for(vector<PakReader *>::iterator i = loadedPaks.begin(); i != loadedPaks.end(); i++) {
		
		PakReader * reader = *i;
		
		assert(reader != NULL);
		
		if(!strcasecmp((const char *)pakname, reader->pakname)) {
			delete(reader);
			loadedPaks.erase(i);
			return true;
		}
		
	}
	
	return false;
}

bool PakManager::Read(const char * filename, void * buffer) {
	
	if((filename[0] == '\\') || (filename[0] == '/')) {
		filename++;
	}

	for(vector<PakReader *>::iterator i = loadedPaks.begin(); i != loadedPaks.end(); i++) {
		if((*i)->Read(filename, buffer)) {
			printf("\e[1;32mRead from PAK:\e[m\t%s\n", filename);
			return true;
		}
	}
	
	printf("\e[1;33mCan't read from PAK:\e[m\t%s\n", filename);
	return false;
}

void * PakManager::ReadAlloc(const char * filename, size_t * sizeRead) {
	
	if((filename[0] == '\\') || (filename[0] == '/')) {
		filename++;
	}
	
	for(vector<PakReader *>::iterator i = loadedPaks.begin(); i != loadedPaks.end(); i++) {
		void * buf;
		if((buf = (*i)->ReadAlloc(filename, sizeRead))) {
			printf("\e[1;32mRead from PAK (a):\e[m\t%s\n", filename);
			return buf;
		}
	}
	
	printf("\e[1;33mRead from PAK (a):\e[m\t%s\n", filename);
	if(sizeRead) {
		*sizeRead = 0;
	}
	return NULL;
}

// return should be size_t?
size_t PakManager::GetSize(const char * filename) {
	
	if ((filename[0] == '\\') || (filename[0] == '/')) {
		filename++;
	}
	
	for (vector<PakReader *>::iterator i = loadedPaks.begin(); i != loadedPaks.end(); i++) {
		int size;
		if((size = (*i)->GetSize(filename)) > 0) {
			printf("\e[1;32mGot size in PAK:\e[m\t%s (%d)\n", filename, size);
			return size;
		}
	}
	
	printf("\e[1;33mCan't get size in PAK:\e[m\t%s\n", filename);
	return -1;
}

PakFileHandle * PakManager::fOpen(const char * filename) {
	
	if((filename[0] == '\\') || (filename[0] == '/')) {
		filename++;
	}
	
	for (vector<PakReader *>::iterator i = loadedPaks.begin(); i != loadedPaks.end(); i++) {
		PakFileHandle * pfh;
		if((pfh = (*i)->fOpen(filename, "rb"))) {
			printf("\e[1;32mOpened from PAK:\e[m\t%s\n", filename);
			return pfh;
		}
	}
	
	printf("\e[1;33mCan't open from PAK:\e[m\t%s\n", filename);
	return NULL;
}

int PakManager::fClose(PakFileHandle * pfh) {
	
	assert(pfh->reader != NULL);
	
	return pfh->reader->fClose(pfh);
}

size_t PakManager::fRead(void * buf, size_t size, size_t count, PakFileHandle * pfh) {
	
	assert(pfh->reader != NULL);
	
	return pfh->reader->fRead(buf, size, count, pfh);
}

int PakManager::fSeek(PakFileHandle * pfh, int offset, long whence) {
	
	assert(pfh->reader != NULL);
	
	return pfh->reader->fSeek(pfh, offset, whence);
}

int PakManager::fTell(PakFileHandle * pfh) {
	
	assert(pfh->reader != NULL);
	
	return pfh->reader->fTell(pfh);
}

vector<PakDirectory *> * PakManager::ExistDirectory(const char * name) {
	
	if((name[0] == '\\') || (name[0] == '/')) {
		name++;
	}
	
	vector<PakDirectory *> * directories = new vector<PakDirectory *>();
	
	for (vector<PakReader *>::iterator i = loadedPaks.begin(); i != loadedPaks.end(); i++) {
		PakDirectory * dir;
		if((dir = (*i)->root->getDirectory(name))) {
			directories->insert(directories->end(), dir);
		}
	}
	return directories;
}

bool PakManager::ExistFile(const char * name) {
	
	if((name[0] == '\\') || (name[0] == '/')) {
		name++;
	}
	
	for(vector<PakReader *>::iterator i = loadedPaks.begin(); i != loadedPaks.end(); i++) {
		if((*i)->getFile(name)) {
			printf("\e[1;32mFound in PAK:\e[m\t%s\n", name);
			return true;
		}
	}
	
	printf("\e[1;33mCan't find in PAK:\e[m\t%s\n", name);
	return false;
}
