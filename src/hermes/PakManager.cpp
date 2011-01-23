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
// Code: S�bastien Scieux
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include <stddef.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <cstring>
#include <cstdlib>

#include <hermes/PakManager.h>
#include <hermes/PakReader.h>
#include <hermes/PakEntry.h>
#include <hermes/HashMap.h>
#include <hermes/Filesystem.h>


using std::vector;


// TODO prefer real files over those in PAK?

PakManager * pPakManager = NULL;

// TODO remove param
void PAK_AddPak(const char * pakfile)
{

	if (FileExist(pakfile))
	{
		if (!pPakManager) pPakManager = new PakManager();

		pPakManager->RemovePak(pakfile);
		pPakManager->AddPak(pakfile);
	}

}

void PAK_Close()
{
	if (pPakManager) delete pPakManager;

	pPakManager = NULL;
}

// TODO size_t argument
void * _PAK_FileLoadMallocZero(const char * name, long * sizeRead)
{

	int size;
	size = pPakManager->GetSize(name);

	if (size > 0)
	{
		char * mem = (char *)malloc(size + 2);

		pPakManager->Read(name, mem);
		
		mem[size] = 0;
		mem[size + 1] = 0;

		if (sizeRead) *sizeRead = size + 2;

		return mem;
	}
	else
	{
		if (sizeRead) *sizeRead = size;

		return NULL;
	}
}

// TODO size_t for argument
void * _PAK_FileLoadMalloc(const char * name, long * sizeRead)
{

	int size = 0;
	void * mem = pPakManager->ReadAlloc(name, &size);

	if (sizeRead && mem) *sizeRead = size;

	return mem;
}

long _PAK_DirectoryExist(const char * name) {

	long leng = strlen(name);

	char temp[256];
	strcpy(temp, name); // TODO this copy can be avoided
	long l = leng ;
	if (temp[l] != '\\' && temp[l] != '/') strcat(temp, "\\");

	vector<PakDirectory *> pvRepertoire(pPakManager->ExistDirectory(temp));

	if (!pvRepertoire.size())
	{
		return false;
	}

	return true;
}

long PAK_DirectoryExist(const char * name) {
	
	long ret = 0;
	
	ret = _PAK_DirectoryExist(name);
	
	if(!ret) {
		ret = DirectoryExist(name);
	}
	
	return ret;
}

// TODO return should be bool
long PAK_FileExist(const char * name) {
	
	long ret = 0;

	ret = pPakManager->ExistFile(name) ? 1 : 0;
	
	if(!ret) {
		ret = FileExist(name);
	}
	
	return ret;
}

// TODO parameter should be size_t
void * PAK_FileLoadMalloc(const char * name, long * sizeLoaded) {
	
	void * ret = NULL;
	
	ret = _PAK_FileLoadMalloc(name, sizeLoaded);
	
	if(!ret) {
		ret = FileLoadMalloc(name, sizeLoaded);
	}
	
	return ret;
}

void * PAK_FileLoadMallocZero(const char * name, long * SizeLoadMalloc) {
	
	void * ret = NULL;
	
	ret = _PAK_FileLoadMallocZero(name, SizeLoadMalloc);
	
	if(!ret) {
		ret = FileLoadMallocZero(name, SizeLoadMalloc);
	}
	
	return ret;
}

long PAK_ftell(FILE * stream) {
	
	PakFileHandle * pfh = (PakFileHandle *)stream;
	assert(pfh->active);
	
	if(pfh->reader) {
		return pfh->reader->fTell(pfh);
	}
	
	return ftell(pfh->truefile);
}

FILE * PAK_fopen(const char * filename, const char * mode) {
	
	PakFileHandle * pfh = pPakManager->fOpen(filename);
	if(pfh) {
		return (FILE *)pfh;
	}
	
	FILE * fh = fopen(filename, mode);
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
	
	return (FILE *)pfh;
}

int PAK_fclose(FILE * stream) {
	
	PakFileHandle * pfh = (PakFileHandle *)stream;
	assert(pfh->active);
	
	if(pfh->reader) {
		return pfh->reader->fClose(pfh);
	}
	
	int ret = fclose(pfh->truefile);
	
	pfh->active = false;
	delete pfh;
	
	return ret;
}

size_t PAK_fread(void * buffer, size_t size, size_t count, FILE * stream) {
	
	PakFileHandle * pfh = (PakFileHandle *)stream;
	assert(pfh->active);
	
	if(pfh->reader) {
		return pfh->reader->fRead(buffer, size, count, pfh);
	}
	
	return fread(buffer, size, count, pfh->truefile);
}

int PAK_fseek(FILE * stream, long offset, int origin) {
	
	PakFileHandle * pfh = (PakFileHandle *)stream;
	assert(pfh->active);
	
	if(pfh->reader) {
		return pfh->reader->fSeek(pfh, offset, origin);
	}
	
	return fseek(pfh->truefile, offset, origin);
}

//-----------------------------------------------------------------------------
PakManager::PakManager()
{
	vLoadPak.clear();
}

//-----------------------------------------------------------------------------
PakManager::~PakManager()
{
	vector<PakReader *>::iterator i;

	for (i = vLoadPak.begin(); i < vLoadPak.end(); i++)
	{
		delete(*i);
	}

	vLoadPak.clear();
}

//-----------------------------------------------------------------------------
bool PakManager::AddPak(const char * pakname)
{
	PakReader * pLoadPak = new PakReader();
	if(!pLoadPak->Open(pakname)) {
		delete pLoadPak;
		return false;
	}

	if (!pLoadPak->root)
	{
		delete pLoadPak;
		return false;
	}

	vLoadPak.push_back(pLoadPak);
	return true;
}

//-----------------------------------------------------------------------------
bool PakManager::RemovePak(const char * _lpszName)
{
	vector<PakReader *>::iterator i;

	for (i = vLoadPak.begin(); i < vLoadPak.end(); i++)
	{
		PakReader * pLoadPak = *i;

		if (pLoadPak)
		{
			if (!strcasecmp((const char *)_lpszName, pLoadPak->pakname))
			{
				delete(*i);
				vLoadPak.erase(i);
				return true;
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
bool PakManager::Read(const char * filename, void * buffer)
{
	vector<PakReader *>::iterator i;

	if ((filename[0] == '\\') ||
	        (filename[0] == '/'))
	{
		filename++;
	}

	for (i = vLoadPak.begin(); i < vLoadPak.end(); i++)
	{
		if ((*i)->Read(filename, buffer))
		{
			printf("\e[1;32mRead from PAK:\e[m\t%s\n", filename);
			return true;
		}
	}

	printf("\e[1;33mCan't read from PAK:\e[m\t%s\n", filename);
	return false;
}

//-----------------------------------------------------------------------------
void * PakManager::ReadAlloc(const char * filename, int * sizeRead)
{
	vector<PakReader *>::iterator i;

	if ((filename[0] == '\\') ||
	        (filename[0] == '/'))
	{
		filename++;
	}
	
	// TODO change parameter type
	size_t size = *sizeRead;

	for (i = vLoadPak.begin(); i < vLoadPak.end(); i++)
	{
		void * pMem;

		if ((pMem = (*i)->ReadAlloc(filename, &size)))
		{
			printf("\e[1;32mRead from PAK (a):\e[m\t%s\n", filename);
			*sizeRead = size;
			return pMem;
		}
	}

	printf("\e[1;33mRead from PAK (a):\e[m\t%s\n", filename);
	*sizeRead = size;
	return NULL;
}

//-----------------------------------------------------------------------------
// return should be size_t?
int PakManager::GetSize(const char * filename)
{
	vector<PakReader *>::iterator i;

	if ((filename[0] == '\\') ||
	        (filename[0] == '/'))
	{
		filename++;
	}

	for (i = vLoadPak.begin(); i < vLoadPak.end(); i++)
	{
		int iTaille;

		if ((iTaille = (*i)->GetSize(filename)) > 0)
		{
			printf("\e[1;32mGot size in PAK:\e[m\t%s (%d)\n", filename, iTaille);
			return iTaille;
		}
	}

	printf("\e[1;33mCan't get size in PAK:\e[m\t%s\n", filename);
	return -1;
}

//-----------------------------------------------------------------------------
PakFileHandle * PakManager::fOpen(const char * filename)
{
	vector<PakReader *>::iterator i;

	if ((filename[0] == '\\') ||
	        (filename[0] == '/'))
	{
		filename++;
	}

	for (i = vLoadPak.begin(); i < vLoadPak.end(); i++)
	{
		PakFileHandle * pPakFile;

		if ((pPakFile = (*i)->fOpen(filename, "rb")))
		{
			printf("\e[1;32mOpened from PAK:\e[m\t%s\n", filename);
			return pPakFile;
		}
	}

	printf("\e[1;33mCan't open from PAK:\e[m\t%s\n", filename);
	return NULL;
}

//-----------------------------------------------------------------------------
int PakManager::fClose(PakFileHandle * _pPakFile)
{
	vector<PakReader *>::iterator i;


	for (i = vLoadPak.begin(); i < vLoadPak.end(); i++)
	{
		if ((*i)->fClose(_pPakFile) != EOF)
		{
			return 0;
		}
	}

	return EOF;
}

//-----------------------------------------------------------------------------
int PakManager::fRead(void * _pMem, int _iSize, int _iCount, PakFileHandle * _pPackFile)
{
	vector<PakReader *>::iterator i;

	for (i = vLoadPak.begin(); i < vLoadPak.end(); i++)
	{
		int iTaille;

		if ((iTaille = (*i)->fRead(_pMem, _iSize, _iCount, _pPackFile)))
		{
			return iTaille;
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------
int PakManager::fSeek(PakFileHandle * _pPackFile, int _iSeek, int _iMode)
{
	vector<PakReader *>::iterator i;

	for (i = vLoadPak.begin(); i < vLoadPak.end(); i++)
	{
		if (!((*i)->fSeek(_pPackFile, _iSeek, _iMode)))
		{
			return 0;
		}
	}

	return 1;
}

//-----------------------------------------------------------------------------
int PakManager::fTell(PakFileHandle * _pPackFile)
{
	vector<PakReader *>::iterator i;

	for (i = vLoadPak.begin(); i < vLoadPak.end(); i++)
	{
		int iOffset;

		if ((iOffset = (*i)->fTell(_pPackFile)) >= 0)
		{
			return iOffset;
		}
	}

	return -1;
}

//-----------------------------------------------------------------------------
vector<PakDirectory *>* PakManager::ExistDirectory(const char * name)
{
	vector<PakReader *>::iterator i;

	if ((name[0] == '\\') ||
	        (name[0] == '/'))
	{
		name++;
	}

	vector<PakDirectory *> *pvRepertoire = new vector<PakDirectory *>;
	pvRepertoire->clear();

	for (i = vLoadPak.begin(); i < vLoadPak.end(); i++)
	{
		PakDirectory * pRep;

		if ((pRep = (*i)->root->getDirectory(name)))
		{
			pvRepertoire->insert(pvRepertoire->end(), pRep);
		}
	}
	return pvRepertoire;
}

bool PakManager::ExistFile(const char * name) {
	
	if((name[0] == '\\') || (name[0] == '/')) {
		name++;
	}
	
	for(vector<PakReader *>::iterator i = vLoadPak.begin(); i < vLoadPak.end(); i++) {
		if((*i)->getFile(name)) {
			printf("\e[1;32mFound in PAK:\e[m\t%s\n", name);
			return true;
		}
	}
	
	printf("\e[1;33mCan't find in PAK:\e[m\t%s\n", name);
	return false;
}
