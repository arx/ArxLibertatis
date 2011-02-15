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
#include <iostream>
#include <fstream>
#include <sstream>
#include <hermes/PakManager.h>
#include <hermes/PakReader.h>
#include <hermes/PakEntry.h>
#include <hermes/HashMap.h>
#include <HERMESMain.h>
#include "ARX_Common.h"

#include <stddef.h>

using std::vector;

bool bForceInPack = true;
long CURRENT_LOADMODE = LOAD_PACK_THEN_TRUEFILE;

PakManager * pPakManager = NULL;

void PAK_SetLoadMode(long mode, const char * pakfile)
{

	mode = LOAD_TRUEFILE_THEN_PACK;
	// nust be remed for editor mode

	CURRENT_LOADMODE = mode;

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
void * _PAK_FileLoadMalloc(const char * name, long * SizeLoadMalloc)
{

	int iTaille = 0;
	void * mem = pPakManager->ReadAlloc(name, &iTaille);

	if ((SizeLoadMalloc) && mem) *SizeLoadMalloc = iTaille;

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

	switch (CURRENT_LOADMODE)
	{
		case LOAD_TRUEFILE:
			ret = DirectoryExist(name);
			break;
		case LOAD_PACK:
			ret = _PAK_DirectoryExist(name);
			break;
		case LOAD_PACK_THEN_TRUEFILE:
			ret = _PAK_DirectoryExist(name);

			if (!ret)
				ret = DirectoryExist(name);

			break;
		case LOAD_TRUEFILE_THEN_PACK:

			if (bForceInPack)
			{
				ret = _PAK_DirectoryExist(name);
			}
			else
			{
				ret = DirectoryExist(name);

				if (!ret)
					ret = _PAK_DirectoryExist(name);
			}

			break;
	}

	return ret;
}

// TODO return should be bool
long _PAK_FileExist(const char * name) {
	return pPakManager->ExistFile(name) ? 1 : 0;
}


long PAK_FileExist(const char * name)
{
	long ret = 0;

	switch (CURRENT_LOADMODE)
	{
		case LOAD_TRUEFILE:
			ret	=	FileExist(name);
			break;
		case LOAD_PACK:
			ret	=	_PAK_FileExist(name);
			break;
		case LOAD_PACK_THEN_TRUEFILE:
			ret	=	_PAK_FileExist(name);

			if (!ret)
				ret	=	FileExist(name);

			break;
		case LOAD_TRUEFILE_THEN_PACK:

			if (bForceInPack)
			{
				ret	=	_PAK_FileExist(name);
			}
			else
			{
				ret	=	FileExist(name);

				if (!ret)
					ret	= _PAK_FileExist(name);
			}

			break;
	}

	return ret;
}

void * PAK_FileLoadMalloc(const char * name, long * SizeLoadMalloc)
{

	void * ret = NULL;

	switch (CURRENT_LOADMODE)
	{
		case LOAD_TRUEFILE:
			ret	=	FileLoadMalloc(name, SizeLoadMalloc);
			break;
		case LOAD_PACK:
			ret	=	_PAK_FileLoadMalloc(name, SizeLoadMalloc);
			break;
		case LOAD_PACK_THEN_TRUEFILE:
			ret	=	_PAK_FileLoadMalloc(name, SizeLoadMalloc);

			if (ret == NULL)
				if (PAK_FileExist(name))
					ret = FileLoadMalloc(name, SizeLoadMalloc);

			break;
		case LOAD_TRUEFILE_THEN_PACK:

			if (bForceInPack)
			{
				ret	=	_PAK_FileLoadMalloc(name, SizeLoadMalloc);
			}
			else
			{
				ret	=	FileLoadMalloc(name, SizeLoadMalloc);

				if (ret == NULL)
					ret	= _PAK_FileLoadMalloc(name, SizeLoadMalloc);
			}

			break;
	}

	return ret;
}

void * PAK_FileLoadMallocZero(const char * name, long * SizeLoadMalloc)
{

	void * ret = NULL;

	switch (CURRENT_LOADMODE)
	{
		case LOAD_TRUEFILE:
			ret	=	FileLoadMallocZero(name, SizeLoadMalloc);
			break;
		case LOAD_PACK:
			ret	=	_PAK_FileLoadMallocZero(name, SizeLoadMalloc);
			break;
		case LOAD_PACK_THEN_TRUEFILE:
			ret	=	_PAK_FileLoadMallocZero(name, SizeLoadMalloc);

			if (ret == NULL)
				if (PAK_FileExist(name))
					ret = FileLoadMallocZero(name, SizeLoadMalloc);

			break;
		case LOAD_TRUEFILE_THEN_PACK:

			if (bForceInPack)
			{
				ret	=	_PAK_FileLoadMallocZero(name, SizeLoadMalloc);
			}
			else
			{
				ret	=	FileLoadMallocZero(name, SizeLoadMalloc);

				if (ret == NULL)
					ret	=	_PAK_FileLoadMallocZero(name, SizeLoadMalloc);
			}

			break;
	}

	return ret;
}

long _PAK_ftell(FILE * stream)
{
	return pPakManager->fTell((PakFileHandle *)stream);
}
long PAK_ftell(FILE * stream)
{

	long ret = 0;

	switch (CURRENT_LOADMODE)
	{
		case LOAD_TRUEFILE:
			ret = ftell(stream);
			break;
		case LOAD_PACK:
			ret = _PAK_ftell(stream);
			break;
		case LOAD_PACK_THEN_TRUEFILE:
			ret = _PAK_ftell(stream);

			if (ret < 0)
				ret = ftell(stream);

			break;
		case LOAD_TRUEFILE_THEN_PACK:

			//if (ferror(stream) && (!bForceInPack)) // TODO hack
			//{
			//	ret = ftell(stream);
			//}
			//else
			{
				ret = _PAK_ftell(stream);
			}

			break;
	}

	return ret;
}


FILE * _PAK_fopen(const char * filename, const char * mode)
{
	return (FILE *)pPakManager->fOpen(filename);
}

FILE * PAK_fopen(const char * filename, const char * mode)
{

	FILE * ret = NULL;

	switch (CURRENT_LOADMODE)
	{
		case LOAD_TRUEFILE:
			ret = fopen(filename, mode);
			break;
		case LOAD_PACK:
			ret = _PAK_fopen(filename, mode);
			break;
		case LOAD_PACK_THEN_TRUEFILE:
			ret = _PAK_fopen(filename, mode);

			if (ret == NULL)
				ret = fopen(filename, mode);

			break;
		case LOAD_TRUEFILE_THEN_PACK:

			if (bForceInPack)
			{
				ret = _PAK_fopen(filename, mode);
			}
			else
			{
				ret = fopen(filename, mode);

				if (ret == NULL)
					ret = _PAK_fopen(filename, mode);
			}

			break;
	}

	return ret;
}

int _PAK_fclose(FILE * stream)
{
	return pPakManager->fClose((PakFileHandle *)stream);
}

int PAK_fclose(FILE * stream)
{

	int ret = 0;

	switch (CURRENT_LOADMODE)
	{
		case LOAD_TRUEFILE:
			ret = fclose(stream);
			break;
		case LOAD_PACK:
			ret = _PAK_fclose(stream);
			break;
		case LOAD_PACK_THEN_TRUEFILE:
			ret = _PAK_fclose(stream);

			if (ret == EOF)
				ret = fclose(stream);

			break;
		case LOAD_TRUEFILE_THEN_PACK:

			//if (ferror(stream) && (!bForceInPack)) TODO hack
			//{
			//	ret = fclose(stream);
			//}
			//else
			{
				ret = _PAK_fclose(stream);
			}

			break;
	}

	return ret;
}

size_t _PAK_fread(void * buffer, size_t size, size_t count, FILE * stream)
{
	return pPakManager->fRead(buffer, size, count, (PakFileHandle *)stream);
}

size_t PAK_fread(void * buffer, size_t size, size_t count, FILE * stream)
{

	size_t ret = 0;

	switch (CURRENT_LOADMODE)
	{
		case LOAD_TRUEFILE:
			ret = fread(buffer, size, count, stream);
			break;
		case LOAD_PACK:
			ret = _PAK_fread(buffer, size, count, stream);
			break;
		case LOAD_PACK_THEN_TRUEFILE:
			ret = _PAK_fread(buffer, size, count, stream);

			if (ret == NULL)
				ret = fread(buffer, size, count, stream);

			break;
		case LOAD_TRUEFILE_THEN_PACK:

			//if (ferror(stream) && (!bForceInPack)) TODO hack
			//{
			//	ret = fread(buffer, size, count, stream);
			//}
			//else
			{
				ret = _PAK_fread(buffer, size, count, stream);
			}

			break;
	}

	return ret;
}


int _PAK_fseek(FILE * fic, long offset, int origin)
{
	return pPakManager->fSeek((PakFileHandle *)fic, offset, origin);
}

int PAK_fseek(FILE * fic, long offset, int origin)
{

	int ret = 1;

	switch (CURRENT_LOADMODE)
	{
		case LOAD_TRUEFILE:
			
			ret = fseek(fic, offset, origin);
			break;
		case LOAD_PACK:
			
			ret = _PAK_fseek(fic, offset, origin);
			break;
		case LOAD_PACK_THEN_TRUEFILE:
			
			ret = _PAK_fseek(fic, offset, origin);

			if (ret == 1)
				ret = fseek(fic, offset, origin);

			break;
		case LOAD_TRUEFILE_THEN_PACK:

			//if (ferror(fic) && (!bForceInPack)) TODO hack
			//{
			//	ret = fseek(fic, offset, origin);
			//}
			//else
			{
				ret = _PAK_fseek(fic, offset, origin);
			}

			break;
	}

	return ret;
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
static void DrawDebugFile(char * _lpszName)
{
	return;

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

	DrawDebugFile(filename);

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

		if ((pMem = (*i)->ReadAlloc(filename, size)))
		{
			printf("\e[1;32mRead from PAK (a):\e[m\t%s\n", filename);
			*sizeRead = size;
			return pMem;
		}
	}

	DrawDebugFile(filename);
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

	DrawDebugFile(filename);
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

	DrawDebugFile(filename);
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
	
	DrawDebugFile(name);
	
	printf("\e[1;33mCan't find in PAK:\e[m\t%s\n", name);
	return false;
}
