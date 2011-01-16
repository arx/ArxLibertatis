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
#include "hermes_pak.h"
#include "hermesmain.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

bool bForceInPack = false;
long CURRENT_LOADMODE = LOAD_TRUEFILE;

PakManager * pPakManager = NULL;

char PAK_WORKDIR[256];
ULONG g_pak_workdir_len = 0;
char NOT_FOUND_FIC[256];
long WRITE_NOT_FOUND = 0;

void PAK_NotFoundInit(char * fich)
{
	strcpy(NOT_FOUND_FIC, fich);
	FILE * fic;

	if ((fic = fopen(NOT_FOUND_FIC, "w")) != NULL)
	{
		WRITE_NOT_FOUND = 1;
		fclose(fic);
	}
	else WRITE_NOT_FOUND = 0;
}

bool PAK_NotFound(char * fich)
{
	FILE * fic;

	if (WRITE_NOT_FOUND)
	{
		if ((fic = fopen(NOT_FOUND_FIC, "a+")) != NULL)
		{
			fprintf(fic, "Not Found %s\n", fich);
			fclose(fic);
			return true;
		}
	}

	return false;
}

void PAK_SetLoadMode(long mode, char * pakfile, char * workdir)
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

	if (workdir)
	{
		strcpy(PAK_WORKDIR, workdir);
		g_pak_workdir_len = strlen(PAK_WORKDIR);
	}
	else
	{
		strcpy(PAK_WORKDIR, "");
		g_pak_workdir_len = 0;
	}

}

void PAK_Close()
{
	if (pPakManager) delete pPakManager;

	pPakManager = NULL;
}
 
void * _PAK_FileLoadMallocZero(char * name, long * SizeLoadMalloc)
{
#ifdef TEST_PACK_EDITOR
	strcpy(PAK_WORKDIR, "\\\\arkaneserver\\public\\arx\\");
	g_pak_workdir_len = strlen(PAK_WORKDIR);
#endif

	if (g_pak_workdir_len >= strlen(name))
	{
		if (SizeLoadMalloc) *SizeLoadMalloc = 0;

		return NULL;
	}

	int iTaille;
	iTaille = pPakManager->GetSize(name + g_pak_workdir_len);

	if (iTaille > 0)
	{
		char * mem = (char *)malloc(iTaille + 2);

		pPakManager->Read(name + g_pak_workdir_len, mem);

		mem[iTaille]   = 0;
		mem[iTaille+1] = 0;

		if (SizeLoadMalloc) *SizeLoadMalloc = iTaille + 2;

		return mem;
	}
	else
	{
		if (SizeLoadMalloc) *SizeLoadMalloc = iTaille;

		return NULL;
	}
}
void * _PAK_FileLoadMalloc(char * name, long * SizeLoadMalloc)
{
#ifdef TEST_PACK_EDITOR
	strcpy(PAK_WORKDIR, "\\\\arkaneserver\\public\\arx\\");
	g_pak_workdir_len = strlen(PAK_WORKDIR);
#endif

	if (g_pak_workdir_len >= strlen(name))
	{
		if (SizeLoadMalloc) *SizeLoadMalloc = 0;

		return NULL;
	}

	int iTaille = 0;
	void * mem = pPakManager->ReadAlloc(name + g_pak_workdir_len, &iTaille);

	if ((SizeLoadMalloc) && mem) *SizeLoadMalloc = iTaille;

	if (mem == NULL) PAK_NotFound(name);

	return mem;
}
long _PAK_DirectoryExist(char * name)
{
#ifdef TEST_PACK_EDITOR
	strcpy(PAK_WORKDIR, "\\\\arkaneserver\\public\\arx\\");
	g_pak_workdir_len = strlen(PAK_WORKDIR);
#endif

	long leng = strlen(name);

	if (ARX_CAST_LONG(g_pak_workdir_len) >= leng)
	{
		return false;
	}

	char temp[256];
	strcpy(temp, name + g_pak_workdir_len);
	long l = leng - g_pak_workdir_len ;
	if (temp[l] != '\\') strcat(temp, "\\");

	vector<EVE_REPERTOIRE *>* pvRepertoire;
	pvRepertoire = pPakManager->ExistDirectory(temp);

	if (!pvRepertoire->size())
	{
		pvRepertoire->clear();
		delete pvRepertoire;
		return false;
	}

	pvRepertoire->clear();
	delete pvRepertoire;
	return true;
}

long PAK_DirectoryExist(char * name)
{
#ifdef TEST_PACK_EDITOR
	CURRENT_LOADMODE = LOAD_PACK_THEN_TRUEFILE;
#endif

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

long _PAK_FileExist(char * name)
{
#ifdef TEST_PACK_EDITOR
	strcpy(PAK_WORKDIR, "\\\\arkaneserver\\public\\arx\\");
	g_pak_workdir_len = strlen(PAK_WORKDIR);
#endif

	if (g_pak_workdir_len >= strlen(name))
	{
		return false;
	}

	char path[256];
	strcpy(path, name + g_pak_workdir_len);

	if (pPakManager->ExistFile(path)) return 1;

	PAK_NotFound(name);
	return 0;
}




long PAK_FileExist(char * name)
{
#ifdef TEST_PACK_EDITOR
	CURRENT_LOADMODE = LOAD_PACK_THEN_TRUEFILE;
#endif
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

void * PAK_FileLoadMalloc(char * name, long * SizeLoadMalloc)
{
#ifdef TEST_PACK_EDITOR
	CURRENT_LOADMODE = LOAD_PACK_THEN_TRUEFILE;
#endif

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

void * PAK_FileLoadMallocZero(char * name, long * SizeLoadMalloc)
{
#ifdef TEST_PACK_EDITOR
	CURRENT_LOADMODE = LOAD_PACK_THEN_TRUEFILE;
#endif

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

long PAK_ftell(FILE * stream);

long _PAK_ftell(FILE * stream)
{
	return pPakManager->fTell((PACK_FILE *)stream);
}
long PAK_ftell(FILE * stream)
{
#ifdef TEST_PACK_EDITOR
	CURRENT_LOADMODE = LOAD_PACK_THEN_TRUEFILE;
#endif

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

			if (ferror(stream) && (!bForceInPack))
			{
				ret = ftell(stream);
			}
			else
			{
				ret = _PAK_ftell(stream);
			}

			break;
	}

	return ret;
}


FILE * _PAK_fopen(const char * filename, const char * mode)
{
#ifdef TEST_PACK_EDITOR
	strcpy(PAK_WORKDIR, "\\\\arkaneserver\\public\\arx\\");
	g_pak_workdir_len = strlen(PAK_WORKDIR);
#endif

	if (g_pak_workdir_len >= strlen(filename))
	{
		return NULL;
	}

	return (FILE *)pPakManager->fOpen((char *)(filename + g_pak_workdir_len));
}

FILE * PAK_fopen(const char * filename, const char * mode)
{
#ifdef TEST_PACK_EDITOR
	CURRENT_LOADMODE = LOAD_PACK_THEN_TRUEFILE;
#endif

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
	return pPakManager->fClose((PACK_FILE *)stream);
}

int PAK_fclose(FILE * stream)
{
#ifdef TEST_PACK_EDITOR
	CURRENT_LOADMODE = LOAD_PACK_THEN_TRUEFILE;
#endif

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

			if (ferror(stream) && (!bForceInPack))
			{
				ret = fclose(stream);
			}
			else
			{
				ret = _PAK_fclose(stream);
			}

			break;
	}

	return ret;
}

size_t _PAK_fread(void * buffer, size_t size, size_t count, FILE * stream)
{
	return pPakManager->fRead(buffer, size, count, (PACK_FILE *)stream);
}

size_t PAK_fread(void * buffer, size_t size, size_t count, FILE * stream)
{
#ifdef TEST_PACK_EDITOR
	CURRENT_LOADMODE = LOAD_PACK_THEN_TRUEFILE;
#endif

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

			if (ferror(stream) && (!bForceInPack))
			{
				ret = fread(buffer, size, count, stream);
			}
			else
			{
				ret = _PAK_fread(buffer, size, count, stream);
			}

			break;
	}

	return ret;
}


int _PAK_fseek(FILE * fic, long offset, int origin)
{
	return pPakManager->fSeek((PACK_FILE *)fic, offset, origin);
}

int PAK_fseek(FILE * fic, long offset, int origin)
{
#ifdef TEST_PACK_EDITOR
	CURRENT_LOADMODE = LOAD_PACK_THEN_TRUEFILE;
#endif

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

			if (ferror(fic) && (!bForceInPack))
			{
				ret = fseek(fic, offset, origin);
			}
			else
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
	vector<EVE_LOADPACK *>::iterator i;

	for (i = vLoadPak.begin(); i < vLoadPak.end(); i++)
	{
		delete(*i);
	}

	vLoadPak.clear();
}

//-----------------------------------------------------------------------------
bool PakManager::AddPak(char * _lpszName)
{
	EVE_LOADPACK * pLoadPak = new EVE_LOADPACK();
	pLoadPak->Open(_lpszName);

	if (!pLoadPak->pRoot)
	{
		delete pLoadPak;
		return false;
	}

	vLoadPak.push_back(pLoadPak);
	return true;
}

//-----------------------------------------------------------------------------
bool PakManager::RemovePak(char * _lpszName)
{
	vector<EVE_LOADPACK *>::iterator i;

	for (i = vLoadPak.begin(); i < vLoadPak.end(); i++)
	{
		EVE_LOADPACK * pLoadPak = *i;

		if (pLoadPak)
		{
			if (!stricmp((const char *)_lpszName, pLoadPak->lpszName))
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
bool PakManager::Read(char * _lpszName, void * _pMem)
{
	vector<EVE_LOADPACK *>::iterator i;

	if ((_lpszName[0] == '\\') ||
	        (_lpszName[0] == '//'))
	{
		_lpszName++;
	}

	for (i = vLoadPak.begin(); i < vLoadPak.end(); i++)
	{
		if ((*i)->Read(_lpszName, _pMem))
		{
			return true;
		}
	}

	DrawDebugFile(_lpszName);

	return false;
}

//-----------------------------------------------------------------------------
void * PakManager::ReadAlloc(char * _lpszName, int * _piTaille)
{
	vector<EVE_LOADPACK *>::iterator i;

	if ((_lpszName[0] == '\\') ||
	        (_lpszName[0] == '//'))
	{
		_lpszName++;
	}

	for (i = vLoadPak.begin(); i < vLoadPak.end(); i++)
	{
		void * pMem;

		if ((pMem = (*i)->ReadAlloc(_lpszName, _piTaille)))
		{
			return pMem;
		}
	}

	DrawDebugFile(_lpszName);
	return NULL;
}

//-----------------------------------------------------------------------------
int PakManager::GetSize(char * _lpszName)
{
	vector<EVE_LOADPACK *>::iterator i;

	if ((_lpszName[0] == '\\') ||
	        (_lpszName[0] == '//'))
	{
		_lpszName++;
	}

	for (i = vLoadPak.begin(); i < vLoadPak.end(); i++)
	{
		int iTaille;

		if ((iTaille = (*i)->GetSize(_lpszName)) > 0)
		{
			return iTaille;
		}
	}

	DrawDebugFile(_lpszName);
	return -1;
}

//-----------------------------------------------------------------------------
PACK_FILE * PakManager::fOpen(char * _lpszName)
{
	vector<EVE_LOADPACK *>::iterator i;

	if ((_lpszName[0] == '\\') ||
	        (_lpszName[0] == '//'))
	{
		_lpszName++;
	}

	for (i = vLoadPak.begin(); i < vLoadPak.end(); i++)
	{
		PACK_FILE * pPakFile;

		if ((pPakFile = (*i)->fOpen((const char *)_lpszName, "rb")))
		{
			return pPakFile;
		}
	}

	DrawDebugFile(_lpszName);
	return NULL;
}

//-----------------------------------------------------------------------------
int PakManager::fClose(PACK_FILE * _pPakFile)
{
	vector<EVE_LOADPACK *>::iterator i;


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
int PakManager::fRead(void * _pMem, int _iSize, int _iCount, PACK_FILE * _pPackFile)
{
	vector<EVE_LOADPACK *>::iterator i;

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
int PakManager::fSeek(PACK_FILE * _pPackFile, int _iSeek, int _iMode)
{
	vector<EVE_LOADPACK *>::iterator i;

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
int PakManager::fTell(PACK_FILE * _pPackFile)
{
	vector<EVE_LOADPACK *>::iterator i;

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
vector<EVE_REPERTOIRE *>* PakManager::ExistDirectory(char * _lpszName)
{
	vector<EVE_LOADPACK *>::iterator i;

	if ((_lpszName[0] == '\\') ||
	        (_lpszName[0] == '//'))
	{
		_lpszName++;
	}

	vector<EVE_REPERTOIRE *> *pvRepertoire = new vector<EVE_REPERTOIRE *>;
	pvRepertoire->clear();

	for (i = vLoadPak.begin(); i < vLoadPak.end(); i++)
	{
		EVE_REPERTOIRE * pRep;

		if ((pRep = (*i)->pRoot->GetSousRepertoire((unsigned char *)_lpszName)))
		{
			pvRepertoire->insert(pvRepertoire->end(), pRep);
		}
	}
	return pvRepertoire;
}

//-----------------------------------------------------------------------------
bool PakManager::ExistFile(char * _lpszName)
{
	vector<EVE_LOADPACK *>::iterator i;

	if ((_lpszName[0] == '\\') ||
	        (_lpszName[0] == '//'))
	{
		_lpszName++;
	}


	char * pcDir = NULL;
	char * pcDir1 = (char *)EVEF_GetDirName((unsigned char *)_lpszName);

	if (pcDir1)
	{
		pcDir = new char[strlen((const char *)pcDir1)+2];
		strcpy((char *)pcDir, (const char *)pcDir1);
		strcat((char *)pcDir, "\\");
		delete [] pcDir1;
	}

	char * pcFile = (char *)EVEF_GetFileName((unsigned char *)_lpszName);

	for (i = vLoadPak.begin(); i < vLoadPak.end(); i++)
	{
		EVE_REPERTOIRE * pRep;

		if ((pRep = (*i)->pRoot->GetSousRepertoire((unsigned char *)pcDir)))
		{
			if (pRep->nbfiles)
			{
				EVE_TFILE * pTFiles = (EVE_TFILE *)pRep->pHachage->GetPtrWithString((char *)pcFile);

				if (pTFiles)
				{
					delete [] pcFile;
					delete [] pcDir;
					return true;
				}
			}
		}
	}

	DrawDebugFile(_lpszName);

	delete [] pcDir;
	delete [] pcFile;
	return false;
}
