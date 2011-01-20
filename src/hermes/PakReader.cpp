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
/*
           _.               _.    .    J)
        HNMMMM) .      L HNMMMM)  #H   H)     ()
       (MMMMMM) M)    (M(MMMMMM)  `H)._UL_   JM)
       `MF" `4) M)    NN`MF" `4)    JMMMMMM#.4F
        M       #)    M) M         (MF (0DANN.
        M       (M   .M  M  .NHH#L (N JMMMN.H#
        M       (M   (M  M   4##HF QQ(MF`"H#(M.
        M       `M   (N  M         #U(H(N (MJM)
       .M  __L   M)  M) .M  ___    4)U#NF (M0M)
       (MMMMMM   M)  M) (MMMMMM)     U#NHLJM(M`
       (MMMMN#   4) (M  (MMMMM#  _.  (H`HMM)JN
       (M""      (M (M  (M""   (MM)  (ML____M)
       (M        (M H)  (M     `"`  ..4MMMMM# D
       (M        `M M)  (M         JM)  """`  N#
       (M         MLM`  (M        #MF   (L    `F
       (M         MMM   (M        H`    (#
       (M         (MN   (M              (Q
       `M####H    (M)   `M####H         ``
        MMMMMM    `M`    NMMMMM
        `Q###F     "     `4###F   sebastien scieux @2001

*/

#include <hermes/PakReader.h>
#include <hermes/PakEntry.h>
#include <hermes/HashMap.h>

#define PAK 1

#include <cstring>
#include <algorithm>
using std::min;
using std::max;
using std::size_t;
using std::strlen;

#include <blast.h>

#include <cassert>

#include <stdint.h>

const uint8_t PAK_KEY_DEMO[] = "NSIARKPRQPHBTE50GRIH3AYXJP2AMF3FCEYAVQO5QGA0JGIIH2AYXKVOA1VOGGU5GSQKKYEOIAQG1XRX0J4F5OEAEFI4DD3LL45VJTVOA1VOGGUKE50GRI";
const uint8_t PAK_KEY_FULL[] = "AVQF3FCKE50GRIAYXJP2AMEYO5QGA0JGIIH2NHBTVOA1VOGGU5H3GSSIARKPRQPQKKYEOIAQG1XRX0J4F5OEAEFI4DD3LL45VJTVOA1VOGGUKE50GRIAYX";

static const uint8_t * selectKey(uint32_t first_bytes) {
	switch(first_bytes) {
		case 0x46515641:
			return PAK_KEY_FULL;
		case 0x4149534E:
			return PAK_KEY_DEMO;
		default:
			return NULL;
	}
}

//-----------------------------------------------------------------------------
PakReader::PakReader() {
	
	pakfile = NULL;
	file = NULL;
	pRoot = NULL;
	
	fat = NULL;
	
	int iI = PACK_MAX_FREAD;
	while(--iI) {
		tPackFile[iI].bActif = false;
		tPackFile[iI].iID = 0;
		tPackFile[iI].iOffset = 0;
	}
	
}

//-----------------------------------------------------------------------------
PakReader::~PakReader() {
	
	if(pakfile) {
		free((void *)pakfile);
		pakfile = NULL;
	}
	
	if(file) {
		fclose(file);
	}
	
	if(pRoot) {
		delete pRoot;
	}
	
	if(fat) {
		delete fat;
	}
	
}

static void pakDecrypt(uint8_t * fat, size_t fat_size, const uint8_t * key) {
	
	size_t keysize = strlen((const char *)key);
	
	for(size_t i = 0, ki = 0; i < fat_size; i++, ki = (ki + 1) % keysize) {
		fat[i] ^= key[ki];
	}
	
}

static const char * safeGetString(const char * & pos, uint32_t & fat_size) {
	
	const char * begin = pos;
	
	for(size_t i = 0; i < fat_size; i++) {
		if(pos[i] == 0) {
			fat_size -= i + 1;
			pos += i + 1;
			return begin;
		}
	}
	
	return NULL;
}

template <class T>
inline bool safeGet(T & data, const char * & pos, uint32_t & fat_size) {
	T nfiles;
	if(fat_size < sizeof(T)) {
		return false;
	}
	data = *reinterpret_cast<const T *>(pos);
	pos += sizeof(T);
	fat_size -= sizeof(T);
	return true;
}

//-----------------------------------------------------------------------------
bool PakReader::Open(const char * name) {
	
	if(pRoot || pakfile || file) {
		// already loaded
		return false;
	}
	
	file = fopen(name, "rb");
	
	if(!file) {
		printf("\e[1;35mCannot find PAK:\e[m\t%s\n", name);
		return false;
	}
	
	// Read fat location and size.
	uint32_t fat_offset;
	uint32_t fat_size;
	if(fread(&fat_offset, sizeof(fat_offset), 1, file) != 1) {
		printf("error reading FAT offset\n");
		fclose(file);
		return false;
	}
	if(fseek(file, fat_offset, SEEK_SET)) {
		printf("error seeking to FAT offset\n");
		fclose(file);
		return false;
	}
	if(fread(&fat_size, sizeof(fat_size), 1, file) != 1) {
		printf("error reading FAT size\n");
		fclose(file);
		return false;
	}
	
	// Read the whole FAT.
	char * newfat = new char[fat_size];
	if(fread(newfat, fat_size, 1, file) != 1) {
		printf("error reading FAT\n");
		return false;
	}
	
	// Decrypt the FAT.
	const char * key = selectKey(*(uint32_t*)newfat);
	if(key) {
		pakDecrypt(newfat, fat_size, key);
	} else {
		printf("WARNING: unknown PAK key ID 0x%08x, assuming no key\n", *(uint32_t*)newfat);
	}
	
	PakDirectory * newroot = new PakDirectory(NULL, NULL);
	
	const char * pos = newfat;
	
	while(fat_size) {
		
		const char * dirname = safeGetString(pos, fat_size);
		if(!dirname) {
			printf("error reading directory name from FAT, wrong key?\n");
			goto error;
		}
		
		PakDirectory * dir;
		if(*dirname != '\0') {
			dir = newroot->AddSousRepertoire((unsigned char *)dirname);
		} else {
			dir = newroot;
		}
		
		uint32_t nfiles;
		if(!safeGet(nfiles, pos, fat_size)) {
			printf("error reading file count from FAT, wrong key?\n");
			goto error;
		}
		
		if(nfiles && !dir->pHachage) {
			int hashsize = 1;
			while(hashsize < nfiles) {
				hashsize <<= 1;
			}
			int n = (hashsize * 3) / 4;
			if(nfiles > n) {
				hashsize <<= 1;
			}
			dir->pHachage = new HashMap(hashsize);
		}
		
		while(nfiles--) {
			
			char * filename =  safeGetString(pos, fat_size);
			if(!filename) {
				printf("error reading file name from FAT, wrong key?\n");
				goto error;
			}
			
			PakFile * file = dir->AddFileToSousRepertoire(NULL, filename);
			
			uint32_t param; // TODO more descriptive names
			uint32_t param2;
			uint32_t param3;
			uint32_t size;
			
			if(!safeGet(param, pos, fat_size) || !safeGet(param2, pos, fat_size)
			   || !safeGet(param3, pos, fat_size) || !safeGet(size, pos, fat_size)) {
				printf("error reading file attributes from FAT, wrong key?\n");
				goto error;
			}
			
			file->offset = param;
			file->param2 = param2;
			file->param3 = param3;
			file->size = size;
		}
		
	}
	
	if(pRoot) {
		delete pRoot;
	}
	pRoot = newroot;
	
	if(pakfile) {
		free(pakfile);
	}
	pakfile = strdup((const char *)name);
	
	if(fat) {
		delete fat;
	}
	fat = newfat;
	
	printf("\e[1;32mLoaded PAK:\e[m\t%s\n", name);
	return true;
	
	
error:
	
	delete newroot;
	
	delete newfat;
	
	return false;
	
}

//-----------------------------------------------------------------------------
void PakReader::Close()
{
	if (file)
	{
		fclose(file);
		file = NULL;
	}

	if (pRoot)
	{
		delete pRoot;
		pRoot = NULL;
	}
}

//-----------------------------------------------------------------------------
size_t ReadData(void * Param, const unsigned char ** buf) {
	
	PAK_PARAM * pPP = (PAK_PARAM *)Param;
	
	*buf = pPP->readbuf;
	
	int iRead = fread(pPP->readbuf, 1, PAK_READ_BUF_SIZE, pPP->file);

	return (unsigned int)iRead;
}

//-----------------------------------------------------------------------------
int WriteData(void * Param, unsigned char * buf, size_t len) {
	
	PAK_PARAM * pPP = (PAK_PARAM *) Param;
	
	size_t lSize = min(pPP->lSize, len);

	memcpy((void *) pPP->mem, (const void *) buf, lSize);
	pPP->mem   += lSize;
	pPP->lSize -= lSize;
	
	return 0;
}

//-----------------------------------------------------------------------------
bool PakReader::Read(char * _pcName, void * _mem)
{
	if ((!_pcName) ||
	        (!pRoot)) return false;

	char * pcDir = NULL;
	char * pcDir1 = (char *)EVEF_GetDirName((unsigned char *)_pcName);

	if (pcDir1)
	{
		pcDir = new char[strlen((const char *)pcDir1)+2];
		strcpy((char *)pcDir, (const char *)pcDir1);
		strcat((char *)pcDir, "\\");
		delete [] pcDir1;
	}

	char * pcFile = (char *)EVEF_GetFileName((unsigned char *)_pcName);

	PakDirectory * pDir;

	if (!pcDir)
	{
		pDir = pRoot;
	}
	else
	{
		pDir = pRoot->GetSousRepertoire((unsigned char *)pcDir);
	}

	if (!pDir)
	{
		if (pcDir) delete [] pcDir;

		if (pcFile) delete [] pcFile;

		return NULL;
	}

	if (!pDir->nbfiles)
	{
		if (pcDir) delete [] pcDir;

		if (pcFile) delete [] pcFile;

		return false;
	}

	PakFile * pTFiles = (PakFile *)pDir->pHachage->GetPtrWithString((char *)pcFile);

	if (pTFiles)
	{
		fseek(file, pTFiles->offset, SEEK_SET);

		if (pTFiles->param2 & PAK)
		{
			PAK_PARAM sPP;
			sPP.file = file;
			sPP.mem = (char *)_mem;
			sPP.lSize = pTFiles->param3;
			blast(ReadData, &sPP, WriteData, &sPP);
		}
		else
		{
			fread(_mem, 1, pTFiles->size, file);
		}

		if (pcDir) delete [] pcDir;

		if (pcFile) delete [] pcFile;

		return true;
	}

	if (pcDir) delete [] pcDir;

	if (pcFile) delete [] pcFile;

	return false;
}

//-----------------------------------------------------------------------------
void * PakReader::ReadAlloc(char * _pcName, int * _piTaille)
{
	if ((!_pcName) ||
	        (!pRoot)) return NULL;

	char * pcDir = NULL;
	char * pcDir1 = (char *)EVEF_GetDirName((unsigned char *)_pcName);

	if (pcDir1)
	{
		pcDir = new char[strlen((const char *)pcDir1)+2];
		strcpy((char *)pcDir, (const char *)pcDir1);
		strcat((char *)pcDir, "\\");
		delete [] pcDir1;
	}

	char * pcFile = (char *)EVEF_GetFileName((unsigned char *)_pcName);

	PakDirectory * pDir;

	if (!pcDir)
	{
		pDir = pRoot;
	}
	else
	{
		pDir = pRoot->GetSousRepertoire((unsigned char *)pcDir);
	}

	if(!pDir) {
		goto error;
	}

	if(!pDir->nbfiles) {
		goto error;
	}
	
	{

	PakFile * pTFiles = (PakFile *)pDir->pHachage->GetPtrWithString((char *)pcFile);

	if (pTFiles)
	{
		void * mem;
		fseek(file, pTFiles->offset, SEEK_SET);

		if (pTFiles->param2 & PAK)
		{
			mem = malloc(pTFiles->param3);
			*_piTaille = (int)pTFiles->param3;

			if(!mem) {
				goto error;
			}

			PAK_PARAM sPP;
			sPP.file = file;
			sPP.mem = (char *)mem;
			sPP.lSize = pTFiles->param3;
			blast(ReadData, &sPP, WriteData, &sPP);
		}
		else
		{
			mem = malloc(pTFiles->size);
			*_piTaille = (int)pTFiles->size;

			if(!mem) {
				goto error;
			}

			fread(mem, 1, pTFiles->size, file);
		}

		if (pcDir) delete [] pcDir;

		if (pcFile) delete [] pcFile;

		return mem;
	}
	
	}
	
error:
	
	if (pcDir) delete [] pcDir;
	
	if (pcFile) delete [] pcFile;
	
	return NULL;
}

//-----------------------------------------------------------------------------
int PakReader::GetSize(char * _pcName)
{
	if ((!_pcName) ||
	        (!pRoot)) return -1;

	char * pcDir = NULL;
	char * pcDir1 = (char *)EVEF_GetDirName((unsigned char *)_pcName);

	if (pcDir1)
	{
		pcDir = new char[strlen((const char *)pcDir1)+2];
		strcpy((char *)pcDir, (const char *)pcDir1);
		strcat((char *)pcDir, "\\");
		delete [] pcDir1;
	}

	char * pcFile = (char *)EVEF_GetFileName((unsigned char *)_pcName);

	PakDirectory * pDir;

	if (!pcDir)
	{
		pDir = pRoot;
	}
	else
	{
		pDir = pRoot->GetSousRepertoire((unsigned char *)pcDir);
	}

	if (!pDir)
	{
		if (pcDir) delete [] pcDir;

		if (pcFile) delete [] pcFile;

		return -1;
	}

	if (!pDir->nbfiles)
	{
		if (pcDir) delete [] pcDir;

		if (pcFile) delete [] pcFile;

		return false;
	}

	PakFile * pTFiles = (PakFile *)pDir->pHachage->GetPtrWithString((char *)pcFile);

	if (pTFiles)
	{
		if (pcDir) delete [] pcDir;

		if (pcFile) delete [] pcFile;

		if (pTFiles->param2 & PAK)
		{
			return pTFiles->param3;
		}
		else
		{
			return pTFiles->size;
		}
	}

	if (pcDir) delete [] pcDir;

	if (pcFile) delete [] pcFile;

	return -1;
}

//-----------------------------------------------------------------------------
PakFileHandle * PakReader::fOpen(const char * _pcName, const char * _pcMode)
{
	
	if ((!_pcName) ||
	        (!pRoot)) return NULL;

	char * pcDir = NULL;
	char * pcDir1 = (char *)EVEF_GetDirName((unsigned char *)_pcName);

	if (pcDir1)
	{
		pcDir = new char[strlen((const char *)pcDir1)+2];
		strcpy((char *)pcDir, (const char *)pcDir1);
		strcat((char *)pcDir, "\\");
		delete [] pcDir1;
	}

	char * pcFile = (char *)EVEF_GetFileName((unsigned char *)_pcName);

	PakDirectory * pDir;

	if (!pcDir)
	{
		pDir = pRoot;
	}
	else
	{
		pDir = pRoot->GetSousRepertoire((unsigned char *)pcDir);
	}

	if (!pDir)
	{
		if (pcDir) delete [] pcDir;

		if (pcFile) delete [] pcFile;

		return NULL;
	}

	if (!pDir->nbfiles)
	{
		if (pcDir) delete [] pcDir;

		if (pcFile) delete [] pcFile;

		return false;
	}

	PakFile * pTFiles = (PakFile *)pDir->pHachage->GetPtrWithString((char *)pcFile);

	if (pTFiles)
	{
		if (pcDir) delete [] pcDir;

		if (pcFile) delete [] pcFile;

		int iNb = PACK_MAX_FREAD;

		while (--iNb)
		{
			if (!tPackFile[iNb].bActif)
			{
				tPackFile[iNb].iID = (int)fat; // TODO why ose the FAT address here?
				tPackFile[iNb].bActif = true;
				tPackFile[iNb].iOffset = 0;
				tPackFile[iNb].pFile = pTFiles;
				return &tPackFile[iNb];
			}
		}

		return NULL;
	}

	if (pcDir) delete [] pcDir;

	if (pcFile) delete [] pcFile;

	return NULL;
}

//-----------------------------------------------------------------------------
int PakReader::fClose(PakFileHandle * _pPackFile)
{
	if ((!_pPackFile) ||
	        (!_pPackFile->bActif) ||
	        (_pPackFile->iID != ((int)fat))) return EOF;

	_pPackFile->bActif = false;
	return 0;
}

//-----------------------------------------------------------------------------
size_t ReadDataFRead(void * Param, const unsigned char ** buf) {
	
	PAK_PARAM_FREAD * pPP = (PAK_PARAM_FREAD *)Param;
	
	*buf = pPP->readbuf;

	int iRead = fread(pPP->readbuf, 1, PAK_READ_BUF_SIZE, pPP->file);

	return (unsigned int)iRead;
}

//-----------------------------------------------------------------------------
int WriteDataFRead(void * Param, unsigned char * buf, size_t len) {
	
	PAK_PARAM_FREAD * pPP = (PAK_PARAM_FREAD *)Param;

	if(pPP->iTailleW >= pPP->iTailleBase) {
		return 1;
	}

	pPP->iOffsetBase -= len;
	pPP->iOffsetCurr += len;

	if (pPP->iOffset < pPP->iOffsetCurr)
	{
		if (pPP->iOffsetBase < 0) pPP->iOffsetBase += len;

		int iSize = len - pPP->iOffsetBase;

		if (pPP->iTaille > iSize)
		{
			pPP->iTaille -= iSize;
		}
		else
		{
			iSize = pPP->iTaille;
		}

		if ((pPP->iTailleW + iSize) > pPP->iTailleFic)
		{
			iSize = pPP->iTailleFic - pPP->iTailleW;
		}

		pPP->iTailleW += iSize;

		memcpy((void *)pPP->mem, (const void *)(buf + pPP->iOffsetBase), iSize);
		pPP->mem += iSize;
		pPP->iOffsetBase = 0;
		
		return 0;
	} else {
		return 1;
	}
}

//-----------------------------------------------------------------------------
size_t PakReader::fRead(void * _pMem, size_t _iSize, size_t _iCount, PakFileHandle * _pPackFile)
{
	if ((!_pPackFile) ||
	        (!_pPackFile->pFile) ||
	        (_pPackFile->iID != ((int) fat))) return 0;

	int iTaille = _iSize * _iCount;

	if ((!_pPackFile) ||
	        (!iTaille) ||
	        (!_pPackFile->pFile)) return 0;

	PakFile * pTFiles = _pPackFile->pFile;

	if (pTFiles->param2 & PAK)
	{
		assert(_pPackFile->iOffset >= 0);
		if ((unsigned int)_pPackFile->iOffset >= _pPackFile->pFile->param3) return 0;

		fseek(file, pTFiles->offset, SEEK_SET);

		PAK_PARAM_FREAD sPP;
		sPP.file        = file;
		sPP.mem         = (char *) _pMem;
		sPP.iOffsetCurr = 0;
		sPP.iOffset     = _pPackFile->iOffset;
		sPP.iOffsetBase = _pPackFile->iOffset;
		sPP.iTaille     = sPP.iTailleBase = iTaille;
		sPP.iTailleW    = 0;
		sPP.iTailleFic  = _pPackFile->pFile->param3;
		blast(ReadDataFRead, &sPP, WriteDataFRead, &sPP);
		iTaille         = sPP.iTailleW;
	}
	else
	{
		assert(_pPackFile->iOffset >= 0);
		if ((unsigned int)_pPackFile->iOffset >= _pPackFile->pFile->size) return 0;

		fseek(file, pTFiles->offset + _pPackFile->iOffset, SEEK_SET);

		assert(iTaille >= 0);

		if (pTFiles->size < (unsigned int)_pPackFile->iOffset + iTaille)
		{
			iTaille -= _pPackFile->iOffset + iTaille - pTFiles->size;
		}

		fread(_pMem, 1, iTaille, file);
	}

	_pPackFile->iOffset += iTaille;
	return iTaille;
}

//-----------------------------------------------------------------------------
int PakReader::fSeek(PakFileHandle * _pPackFile, long _lOffset, int _iOrigin)
{
	if ((!_pPackFile) ||
	        (!_pPackFile->pFile) ||
	        (_pPackFile->iID != ((int)fat))) return 1;

	switch (_iOrigin)
	{
		case SEEK_SET:

			if (_lOffset < 0) return 1;

			if (_pPackFile->pFile->param2 & PAK)
			{
				if (_lOffset > _pPackFile->pFile->param3) return 1;

				_pPackFile->iOffset = _lOffset;
			}
			else
			{
				if (_lOffset > _pPackFile->pFile->size) return 1;

				_pPackFile->iOffset = _lOffset;
			}

			break;
		case SEEK_END:

			if (_lOffset < 0) return 1;

			if (_pPackFile->pFile->param2 & PAK)
			{
				if (_lOffset > _pPackFile->pFile->param3) return 1;

				_pPackFile->iOffset = _pPackFile->pFile->param3 - _lOffset;
			}
			else
			{
				if (_lOffset > _pPackFile->pFile->size) return 1;

				_pPackFile->iOffset = _pPackFile->pFile->size - _lOffset;
			}

			break;
		case SEEK_CUR:

			if (_pPackFile->pFile->param2 & PAK)
			{
				int iOffset = _pPackFile->iOffset + _lOffset;

				if ((iOffset < 0) ||
			        ((unsigned int)iOffset > _pPackFile->pFile->param3))
				{
					return 1;
				}

				_pPackFile->iOffset = iOffset;
			}
			else
			{
				int iOffset = _pPackFile->iOffset + _lOffset;

				if ((iOffset < 0) ||
			        ((unsigned int)iOffset > _pPackFile->pFile->size))
				{
					return 1;
				}

				_pPackFile->iOffset = iOffset;
			}

			break;
	}

	return 0;
}

//-----------------------------------------------------------------------------
long PakReader::fTell(PakFileHandle * _pPackFile)
{
	if ((!_pPackFile) ||
	        (!_pPackFile->pFile) ||
	        (_pPackFile->iID != ((int)fat))) return -1;

	return _pPackFile->iOffset;
}
