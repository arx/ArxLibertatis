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
#include "HERMES_ClusterSave.h"
#include "windows.h"
#include "HERMES_hachage.h"


#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

//------------------------------------------------------------------------
CCluster::CCluster(int _iTaille)
{
	iTaille = _iTaille;
	iNext = -1;
	pNext = NULL;
}

//------------------------------------------------------------------------
CCluster::~CCluster()
{
}

//------------------------------------------------------------------------
void CInfoFile::Set(char * _pcFileName, int _iTaille)
{
	if (pcFileName)
	{
		free((void *)pcFileName);
		pcFileName = NULL;
	}

	pcFileName = strdup(_pcFileName);
	iTaille = _iTaille;
	iNbCluster = 0;
	FirstCluster.iTaille = _iTaille;
	FirstCluster.iNext = -1;
	FirstCluster.pNext = NULL;
}

//------------------------------------------------------------------------
void CInfoFile::KillAll()
{
	if (pcFileName)
	{
		free((void *)pcFileName);
		pcFileName = NULL;
	}

	CCluster * _pClusterNext = FirstCluster.pNext;

	while (_pClusterNext)
	{
		CCluster * _pClusterNextNext = _pClusterNext->pNext;
		delete _pClusterNext;
		_pClusterNext = _pClusterNextNext;
	}

	FirstCluster.iTaille = 0;
	FirstCluster.iNext = 0;
	FirstCluster.pNext = NULL;
}

//------------------------------------------------------------------------
CSaveBlock::CSaveBlock(char * _pcBlockName)
{
	if (_pcBlockName)
	{
		pcBlockName = strdup(_pcBlockName);
	}
	else
	{
		pcBlockName = strdup("Save Block");
	}

	hFile = NULL;
	iTailleBlock = 0;
	iNbFiles = 0;
	sInfoFile = NULL;

	bFirst = false;

	pHachage = NULL;
}

//------------------------------------------------------------------------
CSaveBlock::~CSaveBlock()
{
	if (pcBlockName)
	{
		free((void *)pcBlockName);
	}

	while (iNbFiles--)
	{
		sInfoFile[iNbFiles].KillAll();
	}

	free(sInfoFile);
	sInfoFile = NULL;

	if (hFile)
	{
		fclose(hFile);
		hFile = NULL;
	}

	delete pHachage;
	pHachage = NULL;
}

//------------------------------------------------------------------------
void CSaveBlock::ResetFAT(void)
{
	iTailleBlock = 0;

	while (iNbFiles--)
	{
		sInfoFile[iNbFiles].KillAll();
	}

	free(sInfoFile);
	sInfoFile = NULL;
	iNbFiles = 0;
}

//------------------------------------------------------------------------
bool CSaveBlock::BeginRead(void)
{
	hFile = fopen((const char *)pcBlockName, (const char *)"rb");

	if (!hFile)
	{
		return false;
	}

	//READ FAT
	int _iI;

	fread((void *)&_iI, 1, 4, hFile);
	fseek(hFile, _iI + 4, SEEK_SET);

	fread((void *)&_iI, 1, 4, hFile);	//version

	fread((void *)&_iI, 1, 4, hFile);

	int iNbHache = 1;

	while (iNbHache < _iI) iNbHache <<= 1;

	int iNbHacheTroisQuart = (iNbHache * 3) / 4;

	if (_iI > iNbHacheTroisQuart) iNbHache <<= 1;

	pHachage = new CHachageString(iNbHache);

	while (_iI--)
	{
		char _pT[256], *_pTT = _pT;

		while (1)
		{
			fread((void *)_pTT, 1, 1, hFile);

			if (!*_pTT) break;

			_pTT++;
		}

		ExpandNbFiles();
		CInfoFile * _pInfoFile = &sInfoFile[iNbFiles-1];
		fread((void *)&_pInfoFile->iTaille, 1, 4, hFile);
		_pInfoFile->Set(_pT, _pInfoFile->iTaille);
		fread((void *)&_pInfoFile->iNbCluster, 1, 4, hFile);
		fseek(hFile, 4, SEEK_CUR);

		CCluster * _pCluster = &_pInfoFile->FirstCluster;
		fread((void *)&_pCluster->iTaille, 1, 4, hFile);
		fread((void *)&_pCluster->iNext, 1, 4, hFile);
		_pCluster->pNext = NULL;

		int _iJ = _pInfoFile->iNbCluster - 1;

		if (_iJ > 0)
		{
			while (_iJ--)
			{
				_pCluster->pNext = new CCluster(0);
				_pCluster = _pCluster->pNext;
				fread((void *)&_pCluster->iTaille, 1, 4, hFile);
				fread((void *)&_pCluster->iNext, 1, 4, hFile);
			}
		}
	}

	for (int i = 0;
	        i < iNbFiles;
	        ++i)
	{
		pHachage->AddString(sInfoFile[i].pcFileName, &sInfoFile[i]);
	}

	return true;
}

//------------------------------------------------------------------------
void CSaveBlock::EndRead(void)
{
	if (hFile)
	{
		fclose(hFile);
		hFile = NULL;
	}

	ResetFAT();

	delete pHachage;
	pHachage = NULL;
}

//------------------------------------------------------------------------
bool CSaveBlock::BeginSave(bool _bCont, bool _bReWrite)
{
	bReWrite = _bReWrite;

	hFile = fopen((const char *)pcBlockName, (const char *)"rb");

	if ((!hFile) ||
	        (!_bCont))
	{
		hFile = fopen((const char *)pcBlockName, (const char *)"wb");

		if (!hFile) return false;

		int _iI = 0;

		fwrite((const void *)&_iI, 1, 4, hFile);
		bFirst = true;
	}
	else
	{
		fread((void *)&iTailleBlock, 1, 4, hFile);
		fseek(hFile, iTailleBlock + 4, SEEK_SET);

		//READ FAT
		int _iI;

		fread((void *)&_iI, 1, 4, hFile);	//version
		iVersion = _iI;

		fread((void *)&_iI, 1, 4, hFile);

		while (_iI--)
		{
			char _pT[256], *_pTT = _pT;

			while (1)
			{
				fread((void *)_pTT, 1, 1, hFile);

				if (!*_pTT) break;

				_pTT++;
			}

			ExpandNbFiles();
			CInfoFile * _pInfoFile = &sInfoFile[iNbFiles-1];
			fread((void *)&_pInfoFile->iTaille, 1, 4, hFile);
			_pInfoFile->Set(_pT, _pInfoFile->iTaille);
			fread((void *)&_pInfoFile->iNbCluster, 1, 4, hFile);
			fseek(hFile, 4, SEEK_CUR);

			CCluster * _pCluster = &_pInfoFile->FirstCluster;
			fread((void *)&_pCluster->iTaille, 1, 4, hFile);
			fread((void *)&_pCluster->iNext, 1, 4, hFile);
			_pCluster->pNext = NULL;

			int _iJ = _pInfoFile->iNbCluster - 1;

			if (_iJ > 0)
			{
				while (_iJ--)
				{
					_pCluster->pNext = new CCluster(0);
					_pCluster = _pCluster->pNext;
					fread((void *)&_pCluster->iTaille, 1, 4, hFile);
					fread((void *)&_pCluster->iNext, 1, 4, hFile);
				}
			}
		}

		fseek(hFile, 4, SEEK_SET);
		void * _pPtr = malloc(iTailleBlock);
		fread((void *)_pPtr, iTailleBlock, 1, hFile);
		fclose(hFile);
		hFile = NULL;
		hFile = fopen((const char *)pcBlockName, (const char *)"w+b");

		if (!hFile) return false;

		_iI = 0;
		fwrite((const void *)&_iI, 4, 1, hFile);
		fwrite((const void *)_pPtr, iTailleBlock, 1, hFile);

		free((void *)_pPtr);
	}

	return true;
}

//------------------------------------------------------------------------
bool CSaveBlock::EndSave(void)
{
	if (!bFirst)
	{
		return Defrag();
	}

	int _version = PAK_VERSION + 1;
	fwrite((const void *)&_version, 1, 4, hFile);

	fwrite((const void *)&iNbFiles, 1, 4, hFile);

	for (int _iI = 0; _iI < iNbFiles; _iI++)
	{
		fwrite((const void *)sInfoFile[_iI].pcFileName, 1, strlen((const char *)sInfoFile[_iI].pcFileName) + 1, hFile);
		fwrite((const void *)&sInfoFile[_iI].iTaille, 1, 4, hFile);
		int _iT = sInfoFile[_iI].iNbCluster;
		fwrite((const void *)&_iT, 1, 4, hFile);
		_iT *= (sizeof(CCluster) - 4) + strlen((const char *)sInfoFile[_iI].pcFileName) + 1 + 20;
		fwrite((const void *)&_iT, 1, 4, hFile);

		CCluster * _pCluster = &sInfoFile[_iI].FirstCluster;

		while (_pCluster)
		{
			fwrite((const void *)&_pCluster->iTaille, 1, 4, hFile);
			fwrite((const void *)&_pCluster->iNext, 1, 4, hFile);
			_pCluster = _pCluster->pNext;
		}
	}

	fseek(hFile, 0, SEEK_SET);
	fwrite((const void *)&iTailleBlock, 1, 4, hFile);

	fclose(hFile);
	hFile = NULL;
	ResetFAT();
	return true;
}

//------------------------------------------------------------------------
bool CSaveBlock::Defrag()
{
	char txt[256];
	strcpy(txt, pcBlockName);
	strcat(txt, "DFG");
	FILE * fFileTemp = fopen(txt, "wb");

	int _version = PAK_VERSION + 1;
	fwrite((const void *)&_version, 1, 4, fFileTemp);

	for (int _iI = (iVersion == PAK_VERSION) ? 1 : 0; _iI < iNbFiles; _iI++)
	{
		CCluster * pCluster = &sInfoFile[_iI].FirstCluster;
		void * pMem = malloc(sInfoFile[_iI].iTaille);
		char * pcMem = (char *)pMem;
		int iRealSize = 0;

		while (pCluster)
		{
			fseek(hFile, pCluster->iNext + 4, SEEK_SET);

			//bug size old version
			if (iVersion == PAK_VERSION)
			{
				if ((iRealSize + pCluster->iTaille) > sInfoFile[_iI].iTaille)
				{
					pMem = realloc(pMem, iRealSize + pCluster->iTaille);
					pcMem = ((char *)pMem) + iRealSize;
					sInfoFile[_iI].iTaille = iRealSize + pCluster->iTaille;
				}
			}

			iRealSize += pCluster->iTaille;

			fread(pcMem, pCluster->iTaille, 1, hFile);
			pcMem += pCluster->iTaille;
			pCluster = pCluster->pNext;
		}

		fwrite(pMem, sInfoFile[_iI].iTaille, 1, fFileTemp);
		free(pMem);
	}

	fwrite((const void *)&_version, 1, 4, fFileTemp);
 
	int iOffset = 0;
	int iNbFilesTemp = (iVersion == PAK_VERSION) ? iNbFiles - 1 : iNbFiles; //-1;
	fwrite((const void *)&iNbFilesTemp, 1, 4, fFileTemp);

	for (int _iI = (iVersion == PAK_VERSION) ? 1 : 0; _iI < iNbFiles; _iI++)
	{
		fwrite((const void *)sInfoFile[_iI].pcFileName, 1, strlen((const char *)sInfoFile[_iI].pcFileName) + 1, fFileTemp);
		fwrite((const void *)&sInfoFile[_iI].iTaille, 1, 4, fFileTemp);
		int _iT = 0;
		fwrite((const void *)&_iT, 1, 4, fFileTemp);
		fwrite((const void *)&_iT, 1, 4, fFileTemp);

		//cluster
		fwrite((const void *)&sInfoFile[_iI].iTaille, 1, 4, fFileTemp);
		fwrite((const void *)&iOffset, 1, 4, fFileTemp);

		iOffset += sInfoFile[_iI].iTaille;
	}

	fseek(fFileTemp, 0, SEEK_SET);
	fwrite((const void *)&iOffset, 1, 4, fFileTemp);

	fclose(hFile);
	hFile = NULL;
	ResetFAT();

	fclose(fFileTemp);
	DeleteFile(pcBlockName);
	MoveFile(txt, pcBlockName);

	return true;
} 
 
bool CSaveBlock::ExpandNbFiles()
{
	iNbFiles++;
	sInfoFile = (CInfoFile *)realloc(sInfoFile, (iNbFiles) * sizeof(CInfoFile));
	memset(&sInfoFile[iNbFiles-1], 0, sizeof(CInfoFile));
	return true;
}

//------------------------------------------------------------------------
bool CSaveBlock::Save(char * _pcFileName, void * _pDatas, int _iSize)
{
	bool _bFound = false;
	CInfoFile * _pInfoFile = sInfoFile;
	int _iI = iNbFiles;

	while (_iI--)
	{
		if (!stricmp((const char *)_pInfoFile->pcFileName, (const char *)_pcFileName))
		{
			_bFound = true;
			break;
		}

		_pInfoFile++;
	}

	if (!_bFound)
	{
		ExpandNbFiles();
		_pInfoFile = &sInfoFile[iNbFiles-1];
		_pInfoFile->Set(_pcFileName, _iSize);

		_pInfoFile->FirstCluster.iNext = iTailleBlock;
	}
	else
	{
		if (bReWrite)
		{
			if (!_pDatas) return false;

			int iNbClusters = 0;
			int iSizeWrite = 0;
			int iSize = _iSize;
			CCluster * _pClusterCurr = &_pInfoFile->FirstCluster;
			CCluster * pLastClusterCurr = NULL;

			while ((_pClusterCurr) && (iSize > 0))
			{
				pLastClusterCurr = _pClusterCurr;
				fseek(hFile, _pClusterCurr->iNext + 4, SEEK_SET);
				iSizeWrite = __min(_pClusterCurr->iTaille, iSize);
				fwrite((const void *)_pDatas, iSizeWrite, 1, hFile);
				char * _pDatasTemp = (char *)_pDatas;
				_pDatasTemp += iSizeWrite;
				_pDatas = (void *)_pDatasTemp;
				iSize -= iSizeWrite;
				iNbClusters++;
				_pClusterCurr = _pClusterCurr->pNext;
			}

			_pInfoFile->iTaille = _iSize;

			if (iSize)
			{
				if (pLastClusterCurr)
				{
					pLastClusterCurr->pNext = new CCluster(iSize);
					pLastClusterCurr->pNext->iNext = iTailleBlock;
					pLastClusterCurr->pNext->pNext = NULL;
					iTailleBlock += iSize;
					fseek(hFile, 0, SEEK_END);
					fwrite((const void *)_pDatas, iSize, 1, hFile);
					_pInfoFile->iNbCluster++;
				}
			}
			else
			{
				if (pLastClusterCurr)
				{
					if (pLastClusterCurr->iTaille > iSizeWrite)
					{
						pLastClusterCurr->iTaille = iSizeWrite;
					}

					delete pLastClusterCurr->pNext;
					pLastClusterCurr->pNext = NULL;
					_pInfoFile->iTaille = _iSize;
					_pInfoFile->iNbCluster = iNbClusters;
				}
			}

			fseek(hFile, 0, SEEK_END);

			return true;
		}
		else
		{

			CCluster * _pCluster = &_pInfoFile->FirstCluster;

			while (_pCluster->pNext)
			{
				_pCluster = _pCluster->pNext;
			}

			_pCluster->pNext = new CCluster(_iSize);
			_pCluster->pNext->iNext = iTailleBlock;

			_pInfoFile->iTaille += _iSize;
		}
	}

	_pInfoFile->iNbCluster++;
	iTailleBlock += _iSize;

	if (!hFile) return false;

	if (_pDatas) fwrite((const void *)_pDatas, 1, _iSize, hFile);

	return true;
}

//------------------------------------------------------------------------
bool CSaveBlock::Read(char * _pcFileName, char * _pPtr)
{
	CInfoFile * _pInfoFile = (CInfoFile *)pHachage->GetPtrWithString(_pcFileName);

	if (!_pInfoFile)
	{
		return false;
	}


	CCluster * _pCluster = &_pInfoFile->FirstCluster;

	while (_pCluster)
	{
		fseek(hFile, _pCluster->iNext + 4, SEEK_SET);
		fread((void *)_pPtr, _pCluster->iTaille, 1, hFile);
		_pPtr += _pCluster->iTaille;
		_pCluster = _pCluster->pNext;
	}

	return true;
}

//------------------------------------------------------------------------
int CSaveBlock::GetSize(char * _pcFileName)
{
	CInfoFile * _pInfoFile = NULL;

	if (pHachage)
	{
		_pInfoFile = (CInfoFile *)pHachage->GetPtrWithString(_pcFileName);

		if (!_pInfoFile)
		{
			return -1;
		}
	}
	else
	{
		bool _bFound = false;
		CInfoFile * _pInfoFile = sInfoFile;
		int _iI = iNbFiles;

		while (_iI--)
		{
			if (!stricmp((const char *)_pInfoFile->pcFileName, (const char *)_pcFileName))
			{
				_bFound = true;
				break;
			}

			_pInfoFile++;
		}

		if (!_bFound)
		{
			return -1;
		}
	}

	int _iTaille = 0;
	CCluster * _pCluster = &_pInfoFile->FirstCluster;

	while (_pCluster)
	{
		_iTaille += _pCluster->iTaille;
		_pCluster = _pCluster->pNext;
	}

	return _iTaille;
}
//------------------------------------------------------------------------
bool CSaveBlock::ExistFile(char * _pcFileName)
{
	CInfoFile * _pInfoFile = NULL;

	if (pHachage)
	{
		_pInfoFile = (CInfoFile *)pHachage->GetPtrWithString(_pcFileName);
		return _pInfoFile ? true : false;
	}
	else
	{
		_pInfoFile = sInfoFile;
		int _iI = iNbFiles;

		while (_iI--)
		{
			if (!stricmp((const char *)_pInfoFile->pcFileName, (const char *)_pcFileName))
			{
				return true;
			}

			_pInfoFile++;
		}

		return false;
	}
}
