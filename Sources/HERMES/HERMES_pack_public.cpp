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

#include "HERMES_pack_public.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#define FINAL_COMMERCIAL_GAME
//#define FINAL_COMMERCIAL_DEMO

//-----------------------------------------------------------------------------
EVE_LOADPACK::EVE_LOADPACK()
{
	lpszName = NULL;
	pfFile = NULL;
	pRoot = NULL;
	iSeekPak = 0;

	int iI = PACK_MAX_FREAD;

	while (--iI)
	{
		tPackFile[iI].bActif = false;
		tPackFile[iI].iID = 0;
		tPackFile[iI].iOffset = 0;
	}

	iPassKey = 0;
#ifdef FINAL_COMMERCIAL_GAME
	strcpy((char *)cKey, "AVQF3FCKE50GRIAYXJP2AMEYO5QGA0JGIIH2NHBTVOA1VOGGU5H3GSSIARKPRQPQKKYEOIAQG1XRX0J4F5OEAEFI4DD3LL45VJTVOA1VOGGUKE50GRIAYX");
#else
#ifdef FINAL_COMMERCIAL_DEMO
	strcpy((char *)cKey, "NSIARKPRQPHBTE50GRIH3AYXJP2AMF3FCEYAVQO5QGA0JGIIH2AYXKVOA1VOGGU5GSQKKYEOIAQG1XRX0J4F5OEAEFI4DD3LL45VJTVOA1VOGGUKE50GRI");
#else
	strcpy((char *)cKey, ""); //NO CRYPT
#endif
#endif

	pcFAT = NULL;
}

//-----------------------------------------------------------------------------
EVE_LOADPACK::~EVE_LOADPACK()
{
	if (lpszName)
	{
		free((void *)lpszName);
		lpszName = NULL;
	}

	if (pfFile) fclose(pfFile);

	if (pRoot) delete pRoot;

	if (pcFAT)
	{
		free((void *)pcFAT);
		pcFAT = NULL;
	}
}

//-----------------------------------------------------------------------------
int EVE_LOADPACK::ReadFAT_int()
{
	int i = *((int *)pcFAT);
	pcFAT += 4;
	iTailleFAT -= 4;

	UnCryptInt((unsigned int *)&i);

	return i;
}

//-----------------------------------------------------------------------------
char * EVE_LOADPACK::ReadFAT_string()
{
	char * t = pcFAT;
	int i = UnCryptString((unsigned char *)t) + 1;
	pcFAT += i;
	iTailleFAT -= i;

	return t;
}

//-----------------------------------------------------------------------------
bool EVE_LOADPACK::Open(char * _pcName)
{
	pfFile = fopen(_pcName, "rb");

	if (!pfFile) return false;

	iPassKey = 0;

	pRoot = new EVE_REPERTOIRE(NULL, NULL);
	fread((void *)&iTailleFAT, 1, 4, pfFile);
	fseek(pfFile, iTailleFAT, SEEK_SET);
	fread((void *)&iTailleFAT, 1, 4, pfFile);		//taille de la FAT

	if (pcFAT)
	{
		free((void *)pcFAT);
		pcFAT = NULL;
	}

	pcFAT = (char *)malloc(iTailleFAT);
	char * pcFATCopy = pcFAT;
	fread((void *)pcFAT, iTailleFAT, 1, pfFile);

	while (iTailleFAT)
	{
		char * pcName = ReadFAT_string();

		EVE_REPERTOIRE * pRepertoire = pRoot;

		if (*pcName != 0)
		{
			pRoot->AddSousRepertoire((unsigned char *)pcName);
			pRepertoire = pRoot->GetSousRepertoire((unsigned char *)pcName);
		}
		else
		{
			pcName = NULL;
		}

		int iNbFiles = ReadFAT_int();

		if ((pRepertoire) &&
		        (iNbFiles) &&
		        !(pRepertoire->pHachage))
		{
			int iNbHache = 1;

			while (iNbHache < iNbFiles) iNbHache <<= 1;

			int iNbHacheTroisQuart = (iNbHache * 3) / 4;

			if (iNbFiles > iNbHacheTroisQuart) iNbHache <<= 1;

			pRepertoire->pHachage = new CHachageString(iNbHache);
		}

		while (iNbFiles--)
		{
			char * pcNameFile = ReadFAT_string();
			EVE_TFILE * pFile = pRoot->AddFileToSousRepertoire((unsigned char *)pcName, (unsigned char *)pcNameFile);
			pFile->param = ReadFAT_int();
			pFile->param2 = ReadFAT_int();
			pFile->param3 = ReadFAT_int();
			pFile->taille = ReadFAT_int();
		}
	}

	pcFAT = pcFATCopy;

	lpszName = strdup((const char *)_pcName);

	fseek(pfFile, 0, SEEK_SET);

	return true;
}

//-----------------------------------------------------------------------------
void EVE_LOADPACK::Close()
{
	if (pfFile)
	{
		fclose(pfFile);
		pfFile = NULL;
	}

	if (pRoot)
	{
		delete pRoot;
		pRoot = NULL;
	}
}

//-----------------------------------------------------------------------------
static unsigned int ReadData(char * Buff, unsigned int * Size, void * Param)
{
	PAK_PARAM * pPP = (PAK_PARAM *)Param;

	int iRead = fread(Buff, 1, *Size, pPP->file);

	return (unsigned int)iRead;
}

//-----------------------------------------------------------------------------
static void WriteData(char * Buff, unsigned int * Size, void * Param)
{
	PAK_PARAM * pPP = (PAK_PARAM *) Param;

	ARX_CHECK_NOT_NEG(pPP->lSize);
	long lSize = min(ARX_CAST_ULONG(pPP->lSize), *Size);

	memcpy((void *) pPP->mem, (const void *) Buff, lSize);
	pPP->mem   += lSize;
	pPP->lSize -= lSize;
}

char pcWorkBuff[EXP_BUFFER_SIZE];
//-----------------------------------------------------------------------------
bool EVE_LOADPACK::Read(char * _pcName, void * _mem)
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

	EVE_REPERTOIRE * pDir;

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

	EVE_TFILE * pTFiles = (EVE_TFILE *)pDir->pHachage->GetPtrWithString((char *)pcFile);

	if (pTFiles)
	{
		fseek(pfFile, pTFiles->param - iSeekPak, SEEK_CUR);

		if (pTFiles->param2 & PAK)
		{
			PAK_PARAM sPP;
			sPP.file = pfFile;
			sPP.mem = (char *)_mem;
			sPP.lSize = pTFiles->param3;
			explode(ReadData, WriteData, pcWorkBuff, &sPP);
		}
		else
		{
			fread(_mem, 1, pTFiles->taille, pfFile);
		}

		iSeekPak = ftell(pfFile);

		if (pcDir) delete [] pcDir;

		if (pcFile) delete [] pcFile;

		return true;
	}

	if (pcDir) delete [] pcDir;

	if (pcFile) delete [] pcFile;

	return false;
}

//-----------------------------------------------------------------------------
void * EVE_LOADPACK::ReadAlloc(char * _pcName, int * _piTaille)
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

	EVE_REPERTOIRE * pDir;

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

	EVE_TFILE * pTFiles = (EVE_TFILE *)pDir->pHachage->GetPtrWithString((char *)pcFile);

	if (pTFiles)
	{
		void * mem;
		fseek(pfFile, pTFiles->param - iSeekPak, SEEK_CUR);

		if (pTFiles->param2 & PAK)
		{
			mem = malloc(pTFiles->param3);
			*_piTaille = (int)pTFiles->param3;

			if (!mem)
			{
				if (pcDir) delete [] pcDir;

				if (pcFile) delete [] pcFile;

				return NULL;
			}

			PAK_PARAM sPP;
			sPP.file = pfFile;
			sPP.mem = (char *)mem;
			sPP.lSize = pTFiles->param3;
			explode(ReadData, WriteData, pcWorkBuff, &sPP);
		}
		else
		{
			mem = malloc(pTFiles->taille);
			*_piTaille = (int)pTFiles->taille;

			if (!mem)
			{
				if (pcDir) delete [] pcDir;

				if (pcFile) delete [] pcFile;

				return NULL;
			}

			fread(mem, 1, pTFiles->taille, pfFile);
		}

		iSeekPak = ftell(pfFile);

		if (pcDir) delete [] pcDir;

		if (pcFile) delete [] pcFile;

		return mem;
	}

	if (pcDir) delete [] pcDir;

	if (pcFile) delete [] pcFile;

	return NULL;
}

//-----------------------------------------------------------------------------
int EVE_LOADPACK::GetSize(char * _pcName)
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

	EVE_REPERTOIRE * pDir;

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

	EVE_TFILE * pTFiles = (EVE_TFILE *)pDir->pHachage->GetPtrWithString((char *)pcFile);

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
			return pTFiles->taille;
		}
	}

	if (pcDir) delete [] pcDir;

	if (pcFile) delete [] pcFile;

	return -1;
}

//-----------------------------------------------------------------------------
PACK_FILE * EVE_LOADPACK::fOpen(const char * _pcName, const char * _pcMode)
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

	EVE_REPERTOIRE * pDir;

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

	EVE_TFILE * pTFiles = (EVE_TFILE *)pDir->pHachage->GetPtrWithString((char *)pcFile);

	if (pTFiles)
	{
		if (pcDir) delete [] pcDir;

		if (pcFile) delete [] pcFile;

		int iNb = PACK_MAX_FREAD;

		while (--iNb)
		{
			if (!tPackFile[iNb].bActif)
			{
				tPackFile[iNb].iID = (int)pcFAT;
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
int EVE_LOADPACK::fClose(PACK_FILE * _pPackFile)
{
	if ((!_pPackFile) ||
	        (!_pPackFile->bActif) ||
	        (_pPackFile->iID != ((int)pcFAT))) return EOF;

	_pPackFile->bActif = false;
	return 0;
}

//-----------------------------------------------------------------------------
static unsigned int ReadDataFRead(char * Buff, unsigned int * Size, void * Param)
{
	PAK_PARAM_FREAD * pPP = (PAK_PARAM_FREAD *)Param;

	int iRead = fread(Buff, 1, *Size, pPP->file);

	return (unsigned int)iRead;
}

//-----------------------------------------------------------------------------
static void WriteDataFRead(char * Buff, unsigned int * Size, void * Param)
{
	PAK_PARAM_FREAD * pPP = (PAK_PARAM_FREAD *)Param;

	if (pPP->iTailleW >= pPP->iTailleBase) return;

	pPP->iOffsetBase -= *Size;
	pPP->iOffsetCurr += *Size;

	if (pPP->iOffset < pPP->iOffsetCurr)
	{
		if (pPP->iOffsetBase < 0) pPP->iOffsetBase += *Size;

		int iSize = *Size - pPP->iOffsetBase;

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

		memcpy((void *)pPP->mem, (const void *)(Buff + pPP->iOffsetBase), iSize);
		pPP->mem += iSize;
		pPP->iOffsetBase = 0;
	}
}

//-----------------------------------------------------------------------------
int EVE_LOADPACK::fRead(void * _pMem, int _iSize, int _iCount, PACK_FILE * _pPackFile)
{
	if ((!_pPackFile) ||
	        (!_pPackFile->pFile) ||
	        (_pPackFile->iID != ((int) pcFAT))) return 0;

	int iTaille = _iSize * _iCount;

	if ((!_pPackFile) ||
	        (!iTaille) ||
	        (!_pPackFile->pFile)) return 0;

	EVE_TFILE * pTFiles = _pPackFile->pFile;

	if (pTFiles->param2 & PAK)
	{
		ARX_CHECK_NOT_NEG(_pPackFile->iOffset);
		if (ARX_CAST_UINT(_pPackFile->iOffset) >= _pPackFile->pFile->param3) return 0;

		fseek(pfFile, pTFiles->param - iSeekPak, SEEK_CUR);

		PAK_PARAM_FREAD sPP;
		sPP.file        = pfFile;
		sPP.mem         = (char *) _pMem;
		sPP.iOffsetCurr = 0;
		sPP.iOffset     = _pPackFile->iOffset;
		sPP.iOffsetBase = _pPackFile->iOffset;
		sPP.iTaille     = sPP.iTailleBase = iTaille;
		sPP.iTailleW    = 0;
		sPP.iTailleFic  = _pPackFile->pFile->param3;
		explode(ReadDataFRead, WriteDataFRead, pcWorkBuff, &sPP);
		iTaille         = sPP.iTailleW;
		iSeekPak        = ftell(pfFile);
	}
	else
	{
		ARX_CHECK_NOT_NEG(_pPackFile->iOffset);
		if (ARX_CAST_UINT(_pPackFile->iOffset) >= _pPackFile->pFile->taille) return 0;

		fseek(pfFile, pTFiles->param + _pPackFile->iOffset - iSeekPak, SEEK_CUR);

		ARX_CHECK_NOT_NEG(iTaille);

		if (pTFiles->taille < ARX_CAST_UINT(_pPackFile->iOffset + iTaille))
		{
			iTaille -= _pPackFile->iOffset + iTaille - pTFiles->taille;
		}

		fread(_pMem, 1, iTaille, pfFile);
		iSeekPak = ftell(pfFile);
	}

	_pPackFile->iOffset += iTaille;
	return iTaille;
}

//-----------------------------------------------------------------------------
int EVE_LOADPACK::fSeek(PACK_FILE * _pPackFile, unsigned long _lOffset, int _iOrigin)
{
	if ((!_pPackFile) ||
	        (!_pPackFile->pFile) ||
	        (_pPackFile->iID != ((int)pcFAT))) return 1;

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
				if (_lOffset > _pPackFile->pFile->taille) return 1;

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
				if (_lOffset > _pPackFile->pFile->taille) return 1;

				_pPackFile->iOffset = _pPackFile->pFile->taille - _lOffset;
			}

			break;
		case SEEK_CUR:

			if (_pPackFile->pFile->param2 & PAK)
			{
				int iOffset = _pPackFile->iOffset + _lOffset;

				if ((iOffset < 0) ||
			        (ARX_CAST_UINT(iOffset) > _pPackFile->pFile->param3))
				{
					return 1;
				}

				_pPackFile->iOffset = iOffset;
			}
			else
			{
				int iOffset = _pPackFile->iOffset + _lOffset;

				if ((iOffset < 0) ||
			        (ARX_CAST_UINT(iOffset) > _pPackFile->pFile->taille))
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
int EVE_LOADPACK::fTell(PACK_FILE * _pPackFile)
{
	if ((!_pPackFile) ||
	        (!_pPackFile->pFile) ||
	        (_pPackFile->iID != ((int)pcFAT))) return -1;

	return _pPackFile->iOffset;
}

//-----------------------------------------------------------------------------
void EVE_LOADPACK::CryptChar(unsigned char * _pChar)
{
#ifdef CRYPT_OFF
	return;
#endif
	unsigned int iTailleKey = strlen((const char *) cKey);
	int iDecalage = 0;
	
	*_pChar = ARX_CLEAN_WARN_CAST_UCHAR(((*_pChar) ^ cKey[iPassKey]) >> iDecalage);

	iPassKey++;

	if (iPassKey >= iTailleKey) iPassKey = 0;
}

//-----------------------------------------------------------------------------
void EVE_LOADPACK::UnCryptChar(unsigned char * _pChar)
{
#ifdef CRYPT_OFF
	return;
#endif

	unsigned int iTailleKey = strlen((const char *) cKey);

	int iDecalage = 0;
	*_pChar = ARX_CLEAN_WARN_CAST_UCHAR(((*_pChar) ^ cKey[iPassKey]) << iDecalage);

	iPassKey++;

	if (iPassKey >= iTailleKey) iPassKey = 0;
}
//-----------------------------------------------------------------------------
void EVE_LOADPACK::CryptString(unsigned char * _pTxt)
{
	unsigned char * pTxtCopy = (unsigned char *)_pTxt;
	int iTaille = strlen((const char *)_pTxt) + 1;

	while (iTaille--)
	{
		CryptChar(pTxtCopy);
		pTxtCopy++;
	}
}

//-----------------------------------------------------------------------------
int EVE_LOADPACK::UnCryptString(unsigned char * _pTxt)
{
	unsigned char * pTxtCopy = (unsigned char *)_pTxt;

	int iNbChar = 0;

	while (1)
	{
		UnCryptChar(pTxtCopy);

		if (!*pTxtCopy)
		{
			break;
		}

		pTxtCopy++;
		iNbChar++;
	}

	return iNbChar;
}

//-----------------------------------------------------------------------------
void EVE_LOADPACK::CryptShort(unsigned short * _pShort)
{
	unsigned char cA, cB;
	cA = ARX_CLEAN_WARN_CAST_UCHAR((*_pShort) & 0xFF);
	cB = ARX_CLEAN_WARN_CAST_UCHAR(((*_pShort) >> 8) & 0xFF);

	CryptChar(&cA);
	CryptChar(&cB);
	*_pShort = cA | (cB << 8);
}

//-----------------------------------------------------------------------------
void EVE_LOADPACK::UnCryptShort(unsigned short * _pShort)
{
	unsigned char cA, cB;
	cA = ARX_CLEAN_WARN_CAST_UCHAR((*_pShort) & 0xFF);
	cB = ARX_CLEAN_WARN_CAST_UCHAR(((*_pShort) >> 8) & 0xFF);

	UnCryptChar(&cA);
	UnCryptChar(&cB);
	*_pShort = cA | (cB << 8);
}

//-----------------------------------------------------------------------------
void EVE_LOADPACK::CryptInt(unsigned int * _iInt)
{
	unsigned short sA, sB;
	sA = (*_iInt) & 0xFFFF;
	sB = ((*_iInt) >> 16) & 0xFFFF;

	CryptShort(&sA);
	CryptShort(&sB);
	*_iInt = sA | (sB << 16);
}

//-----------------------------------------------------------------------------
void EVE_LOADPACK::UnCryptInt(unsigned int * _iInt)
{
	unsigned short sA, sB;
	sA = (*_iInt) & 0xFFFF;
	sB = ((*_iInt) >> 16) & 0xFFFF;

	UnCryptShort(&sA);
	UnCryptShort(&sB);
	*_iInt = sA | (sB << 16);
}

//-----------------------------------------------------------------------------
void EVE_LOADPACK::WriteSousRepertoire(char * pcAbs, EVE_REPERTOIRE * r)
{
	char EveTxtFile[256];
	strcpy((char *)EveTxtFile, pcAbs);

	if (r)
	{
		r->ConstructFullNameRepertoire(EveTxtFile);
	}

	CreateDirectory((const char *)EveTxtFile, NULL);

	EVE_TFILE * f = r->fichiers;
	int nb = r->nbfiles;

	while (nb--)
	{
		char	tTxt[512];
		strcpy(tTxt, EveTxtFile);
		strcat(tTxt, (const char *)f->name);
		int		iTaille;
		void	* pDat = this->ReadAlloc(tTxt + strlen((const char *)pcAbs), &iTaille);

		if (pDat)
		{
			printf("%s\n", tTxt);
			FILE * file;
			file = fopen(tTxt, "wb");

			if (file)
			{
				fwrite(pDat, 1, iTaille, file);
				fclose(file);
			}

			free((void *)pDat);
			f = f->fnext;
		}
		else
		{
			MessageBox(NULL, tTxt, "No Found!!", 0);
		}
	}


	EVE_REPERTOIRE * brep = r->fils;
	nb = r->nbsousreps;

	while (nb--)
	{
		EVE_REPERTOIRE * brepnext = brep->brothernext;
		WriteSousRepertoire(pcAbs, brep);
		brep = brepnext;
	}
}

//-----------------------------------------------------------------------------
void EVE_LOADPACK::WriteSousRepertoireZarbi(char * pcAbs, EVE_REPERTOIRE * r)
{
	char EveTxtFile[256];
	strcpy((char *)EveTxtFile, pcAbs);

	if (r)
	{
		r->ConstructFullNameRepertoire(EveTxtFile);
	}

	CreateDirectory((const char *)EveTxtFile, NULL);

	EVE_TFILE * f = r->fichiers;
	int nb = r->nbfiles;

	while (nb--)
	{
		char	tTxt[512];
		strcpy(tTxt, EveTxtFile);
		strcat(tTxt, (const char *)f->name);
		int		iTaille;

		void	* pDat = this->ReadAlloc(tTxt + strlen((const char *)pcAbs), &iTaille);
		int iTaille2 = iTaille;

		if (pDat)
		{
			printf("%s\n", tTxt);
			PACK_FILE * pPf = fOpen(tTxt + strlen((const char *)pcAbs), "rb");

			if (!pPf)
			{
				MessageBox(NULL, tTxt, "ERROR fopen!!", 0);
			}
			else
			{
				int nb2;
				char * pcDat = (char *)pDat;

				while (iTaille)
				{
					printf("%d\r", iTaille);

					if (iTaille < 50)
					{
						nb2 = iTaille;
					}
					else
					{
						nb2 = rand() % iTaille;

						if (!nb2) continue;
					}

					iTaille -= nb2;
					int nb3 = fRead(pcDat, 1, nb2, pPf);
					pcDat += nb2;

					if (nb3 != nb2)
					{
						MessageBox(NULL, tTxt, "ERROR fread!!", 0);
					}
				}

				printf("%d\n", iTaille);
				fClose(pPf);

				FILE * file;
				file = fopen(tTxt, "wb");

				if (file)
				{
					fwrite(pDat, 1, iTaille2, file);
					fclose(file);
				}

				free((void *)pDat);
				f = f->fnext;
			}
		}
		else
		{
			MessageBox(NULL, tTxt, "No Found!!", 0);
		}
	}


	EVE_REPERTOIRE * brep = r->fils;
	nb = r->nbsousreps;

	while (nb--)
	{
		EVE_REPERTOIRE * brepnext = brep->brothernext;
		WriteSousRepertoireZarbi(pcAbs, brep);
		brep = brepnext;
	}
}
