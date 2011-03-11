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

#include "io/PakManager.h"

#include <cassert>
#include <cstring>
#include <cstdlib>

#include "core/Common.h"

#include "io/PakReader.h"
#include "io/PakEntry.h"
#include "io/Filesystem.h"
#include "io/Logger.h"

using std::vector;

static const bool PREFER_LOCAL_FILES_OVER_PAK = true;

// TODO prefer real files over those in PAK?

PakManager * pPakManager = 0;

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

void * _PAK_FileLoadMallocZero(const string & name, size_t & sizeRead) {
	
	size_t size = pPakManager->GetSize(name);
	if(size == (size_t)-1) {
		sizeRead = size;
		return NULL;
	}
	
	char * mem = (char *)malloc(size + 2);
	
	if(!pPakManager->Read(name, mem)) {
		free(mem);
		sizeRead = 0;
		return NULL;
	}
	
	// TODO why use two nullbytes?
	mem[size] = '\0';
	mem[size + 1] = '\0';
	
	sizeRead = size + 2;
	
	return mem;
}

bool _PAK_DirectoryExist(const char * name) {
	
	size_t len = strlen(name);
	char temp[256];
	strcpy(temp, name); // TODO this copy can be avoided
	if(len != 0 && temp[len - 1] != '\\' && temp[len - 1] != '/') {
		strcat(temp, "\\");
	}
	
	return pPakManager->ExistDirectory(temp);
}

bool PAK_DirectoryExist(const std::string& name) {
	
	if(_PAK_DirectoryExist(name.c_str())) {
		return true;
	}
	
	return DirectoryExist(name.c_str());
}

bool PAK_FileExist(const char* name) {
	
	if(pPakManager->ExistFile(name)) {
		return true;
	}
	
	return FileExist(name);
}

void * PAK_FileLoadMalloc(const std::string& name, size_t& sizeLoaded) {
	LogDebug << "File Name " << name;
	
	void * ret = NULL;
	
	if(PREFER_LOCAL_FILES_OVER_PAK) {
		ret = FileLoadMalloc(name.c_str(), &sizeLoaded);
	}
	
	if(!ret) {
		ret = pPakManager->ReadAlloc(name, sizeLoaded);
	}
	
	if(!PREFER_LOCAL_FILES_OVER_PAK && !ret) {
		ret = FileLoadMalloc(name.c_str(), &sizeLoaded);
	}
	
	return ret;
}

void * PAK_FileLoadMallocZero(const std::string& name, size_t& sizeLoaded) {
	
	void * ret = NULL;
	
	if(PREFER_LOCAL_FILES_OVER_PAK) {
		ret = FileLoadMallocZero(name.c_str(), &sizeLoaded);
	}
	
	if(!ret) {
		ret = _PAK_FileLoadMallocZero(name, sizeLoaded);
	}
	
	if(!PREFER_LOCAL_FILES_OVER_PAK && !ret) {
		ret = FileLoadMallocZero(name.c_str(), &sizeLoaded);
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

PakFileHandle * FileOpenPFH(const char * filename) {
	
	FileHandle fh = FileOpenRead(filename);
	if(!fh) {
		return NULL;
	}
	
	PakFileHandle * pfh = new PakFileHandle;
	if(!pfh) {
		return NULL;
	}
	
	pfh->active = true;
	pfh->reader = NULL;
	pfh->truefile = fh;
	
	return pfh;
}

PakFileHandle * PAK_fopen(const char * filename) {
	
	PakFileHandle * pfh = NULL;
	
	if(PREFER_LOCAL_FILES_OVER_PAK) {
		pfh = FileOpenPFH(filename);
	}
	
	if(!pfh) {
		pfh = pPakManager->fOpen(filename);
	}
	
	if(!PREFER_LOCAL_FILES_OVER_PAK && !pfh) {
		pfh = FileOpenPFH(filename);
	}
	
	return pfh;
}

int PAK_fclose(PakFileHandle * pfh) {
	
	assert(pfh->active);
	
	if(pfh->reader) {
		return pfh->reader->fClose(pfh);
	}
	
	int ret = FileClose(pfh->truefile);
	
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
	
	for(vector<PakReader *>::iterator i = loadedPaks.begin(); i != loadedPaks.end(); ++i) {
		delete(*i);
	}
}

bool PakManager::AddPak(const std::string& pakname) {
	
	for(vector<PakReader *>::iterator i = loadedPaks.begin(); i != loadedPaks.end(); ++i) {
		if( !(*i)->pakname.empty() && !strcmp(pakname.c_str(), (*i)->pakname.c_str())) {
			// Already loaded.
			return true;
		}
	}

	PakReader * reader = new PakReader();
	if(!reader->Open(pakname)) {
		delete reader;
		return false;
	}
	
	if(!reader->root) {
		delete reader;
		return false;
	}
	
	loadedPaks.push_back(reader);
	return true;
}

bool PakManager::RemovePak(const std::string& pakname)
{
	LogInfo << "Remove Pack " << pakname;
	
	for(vector<PakReader *>::iterator i = loadedPaks.begin(); i != loadedPaks.end(); ++i) {
		
		PakReader * reader = *i;
		
		assert(reader != NULL);
		
		if(!strcasecmp(pakname.c_str(), reader->pakname.c_str())) {
			delete(reader);
			loadedPaks.erase(i);
			return true;
		}
		
	}
	
	return false;
}

bool PakManager::Read(const std::string& _filename, void * buffer) {

	const char* filename = _filename.c_str();
	
	if((filename[0] == '\\') || (filename[0] == '/')) {
		filename++;
	}

	for(vector<PakReader *>::iterator i = loadedPaks.begin(); i != loadedPaks.end(); ++i) {
		if((*i)->Read(filename, buffer)) {
			LogInfo << "Read from PAK "<< filename;
			return true;
		}
	}
	
	LogError << "Can't read from PAK "<< filename;
	return false;
}

void * PakManager::ReadAlloc(const std::string& _filename, size_t& sizeRead) {
	
	const char* filename = _filename.c_str();

	if((filename[0] == '\\') || (filename[0] == '/')) {
		filename++;
	}
	
	for(vector<PakReader *>::iterator i = loadedPaks.begin(); i != loadedPaks.end(); ++i) {
		void * buf;
		if((buf = (*i)->ReadAlloc(filename, sizeRead))) {
			LogInfo << "Read from PAK (a) "<< filename;
			return buf;
		}
	}
	
	LogError << "Can't read from PAK (a) "<< filename;

	sizeRead = 0;

	return 0;
}

// return should be size_t?
size_t PakManager::GetSize(const std::string& _filename) {
	
	const char* filename = _filename.c_str();

	if ((filename[0] == '\\') || (filename[0] == '/')) {
		filename++;
	}
	
	for (vector<PakReader *>::iterator i = loadedPaks.begin(); i != loadedPaks.end(); ++i) {
		int size;
		if((size = (*i)->GetSize(filename)) >= 0) {
			LogInfo << "Got size in PAK "<< filename << " "<<size;
			return size;
		}
	}
	
	LogError << "Can't get size in PAK "<< filename;
	return -1;
}

PakFileHandle * PakManager::fOpen(const std::string& _filename) {
	
	const char* filename = _filename.c_str();

	if((filename[0] == '\\') || (filename[0] == '/')) {
		filename++;
	}
	
	for (vector<PakReader *>::iterator i = loadedPaks.begin(); i != loadedPaks.end(); ++i) {
		PakFileHandle * pfh;
		if((pfh = (*i)->fOpen(filename))) {
			LogInfo << "Opened from PAK "<< filename;
			return pfh;
		}
	}
	
	LogError << "Can't open from PAK "<< filename;
	return NULL;
}

int PakManager::fClose(PakFileHandle * pfh) {
	
	assert(pfh->reader != NULL);
	
	return pfh->reader->fClose(pfh);
}

int PakManager::fRead(void * buf, size_t size, size_t count, PakFileHandle * pfh) {
	
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

bool PakManager::ExistDirectory(const std::string& _name) {
	
	const char* name = _name.c_str();

	if((name[0] == '\\') || (name[0] == '/')) {
		name++;
	}

	for (vector<PakReader *>::iterator i = loadedPaks.begin(); i != loadedPaks.end(); ++i) {
		PakDirectory * dir;
		if((dir = (*i)->root->getDirectory(name))) {
			return true;
		}
	}

	return false;
}

bool PakManager::GetDirectories(const std::string& _name, vector<PakDirectory*>& directories) {
	
	const char* name = _name.c_str();

	if((name[0] == '\\') || (name[0] == '/')) {
		name++;
	}

	for (vector<PakReader *>::iterator i = loadedPaks.begin(); i != loadedPaks.end(); ++i) {
		PakDirectory * dir;
		if((dir = (*i)->root->getDirectory(name))) {
			directories.push_back(dir);
		}
	}

	return !directories.empty();
}

bool PakManager::ExistFile(const std::string& _name) {
	
	const char* name = _name.c_str();

	if((name[0] == '\\') || (name[0] == '/')) {
		name++;
	}
	
	for(vector<PakReader *>::iterator i = loadedPaks.begin(); i != loadedPaks.end(); ++i) {
		if((*i)->getFile(name)) {
			LogInfo << "Found in PAK "<< name;
			return true;
		}
	}
	
	LogError << "Can't find in PAK "<< name;
	return false;
}
