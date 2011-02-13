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

#include "io/ClusterSave.h"

#include <cstdlib>

#include "io/Filesystem.h"
#include "io/HashMap.h"
#include "io/Logger.h"

const u32 SAV_VERSION_OLD = (1<<16) | 0;
const u32 SAV_VERSION = SAV_VERSION_OLD + 1;

using std::string;

class CCluster{
public:
	int			iTaille;
	int			iNext;
	CCluster	*pNext;
public:
	CCluster(int _iTaille=0);
	~CCluster();
};

class CInfoFile{
public:
	char		*pcFileName;
	int			iTaille;

	int			iNbCluster;
	CCluster	FirstCluster;
	CCluster*	pLastCluster;
public:
	void Set( const char *_pcFileName,int _iTaille);
	void KillAll();
};

using std::min;

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
void CInfoFile::Set( const char * _pcFileName, int _iTaille)
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
CSaveBlock::CSaveBlock(const string & _savefile) {
	
	pcBlockName = _savefile;

	hFile = NULL;
	iTailleBlock = 0;
	iNbFiles = 0;
	sInfoFile = NULL;

	bFirst = false;

	pHachage = NULL;
}

//------------------------------------------------------------------------
CSaveBlock::~CSaveBlock() {

	while (iNbFiles--)
	{
		sInfoFile[iNbFiles].KillAll();
	}

	free(sInfoFile);
	sInfoFile = NULL;

	if (hFile)
	{
		FileClose(hFile);
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

bool CSaveBlock::BeginRead() {
	
	LogDebug << "reading savefile " << pcBlockName;
	
	hFile = FileOpenRead(pcBlockName.c_str());
	if(!hFile) {
		LogWarning << "cannot open save file " << pcBlockName;
		return false;
	}

	//READ FAT

	s32 fatOffset;
	if(!FileRead(hFile, &fatOffset, 4)) {
		LogError << "cannot read fat offset for " << pcBlockName;
		return false;
	}
	LogDebug << "FAT offset is " << fatOffset;
	if(FileSeek(hFile, fatOffset + 4, SEEK_SET) != fatOffset + 4) {
		LogError << "cannot seek to FAT";
		return false;
	}

	s32 version;
	if(!FileRead(hFile, &version, 4)) {
		LogError << "cannot read save file version";
		return false;
	}
	LogDebug << "Version is " << version;

	s32 nFiles;
	if(!FileRead(hFile, &nFiles, 4)) {
		LogError << "cannot read file count";
		return false;
	}
	LogDebug << "number of files is " << nFiles;

	int iNbHache = 1;

	while (iNbHache < nFiles) iNbHache <<= 1;

	int iNbHacheTroisQuart = (iNbHache * 3) / 4;

	if (nFiles > iNbHacheTroisQuart) iNbHache <<= 1;

	pHachage = new HashMap(iNbHache);

	while(nFiles--) {
		
		char _pT[256], *_pTT = _pT;

		// Read the file name.
		while(true) {
			FileRead(hFile, _pTT, 1);
			if(!*_pTT) break;
			_pTT++;
		}

		ExpandNbFiles();
		CInfoFile * _pInfoFile = &sInfoFile[iNbFiles-1];
		FileRead(hFile, &_pInfoFile->iTaille, 4);
		_pInfoFile->Set(_pT, _pInfoFile->iTaille);
		FileRead(hFile, &_pInfoFile->iNbCluster, 4);
		FileSeek(hFile, 4, SEEK_CUR);

		CCluster * _pCluster = &_pInfoFile->FirstCluster;
		FileRead(hFile, &_pCluster->iTaille, 4);
		FileRead(hFile, &_pCluster->iNext, 4);
		_pCluster->pNext = NULL;

		int _iJ = _pInfoFile->iNbCluster - 1;

		if (_iJ > 0)
		{
			while (_iJ--)
			{
				_pCluster->pNext = new CCluster(0);
				_pCluster = _pCluster->pNext;
				FileRead(hFile, &_pCluster->iTaille, 4);
				FileRead(hFile, &_pCluster->iNext, 4);
			}
		}
	}

	for (int i = 0;
	        i < iNbFiles;
	        ++i)
	{
		pHachage->add(sInfoFile[i].pcFileName, &sInfoFile[i]);
	}

	return true;
}

//------------------------------------------------------------------------
void CSaveBlock::EndRead(void)
{
	if (hFile)
	{
		FileClose(hFile);
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

	hFile = FileOpenRead(pcBlockName.c_str());
	LogDebug << "FOR " << pcBlockName << " " << (hFile ? "ok" : "failed");

	if ((!hFile) ||
	        (!_bCont))
	{
		hFile = FileOpenWrite(pcBlockName.c_str());

		if (!hFile) {
			LogError << "could not open " << pcBlockName << " for writing";
			return false;
		}

		int _iI = 0;

		FileWrite(hFile, &_iI, 4);
		bFirst = true;
	}
	else
	{
		FileRead(hFile, &iTailleBlock, 4);
		FileSeek(hFile, iTailleBlock + 4, SEEK_SET);

		//READ FAT
		int _iI;

		FileRead(hFile, &_iI, 4);	//version
		iVersion = _iI;

		FileRead(hFile, &_iI, 4);

		while (_iI--)
		{
			char _pT[256], *_pTT = _pT;

			while (1)
			{
				FileRead(hFile, _pTT, 1);

				if (!*_pTT) break;

				_pTT++;
			}

			ExpandNbFiles();
			CInfoFile * _pInfoFile = &sInfoFile[iNbFiles-1];
			FileRead(hFile, &_pInfoFile->iTaille, 4);
			_pInfoFile->Set(_pT, _pInfoFile->iTaille);
			FileRead(hFile, &_pInfoFile->iNbCluster, 4);
			FileSeek(hFile, 4, SEEK_CUR);

			CCluster * _pCluster = &_pInfoFile->FirstCluster;
			FileRead(hFile, &_pCluster->iTaille, 4);
			FileRead(hFile, &_pCluster->iNext, 4);
			_pCluster->pNext = NULL;

			int _iJ = _pInfoFile->iNbCluster - 1;

			if (_iJ > 0)
			{
				while (_iJ--)
				{
					_pCluster->pNext = new CCluster(0);
					_pCluster = _pCluster->pNext;
					FileRead(hFile, &_pCluster->iTaille, 4);
					FileRead(hFile, &_pCluster->iNext, 4);
				}
			}
		}

		FileSeek(hFile, 4, SEEK_SET);
		void * _pPtr = malloc(iTailleBlock);
		FileRead(hFile, _pPtr, iTailleBlock);
		FileClose(hFile);
		hFile = NULL;
		hFile = FileOpenReadWrite(pcBlockName.c_str());

		if (!hFile) {
			LogError << "could not open " << pcBlockName << " read/write";
			return false;
		}

		_iI = 0;
		FileWrite(hFile, &_iI, 4);
		FileWrite(hFile, _pPtr, iTailleBlock);

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

	FileWrite(hFile, &SAV_VERSION, 4);

	FileWrite(hFile, &iNbFiles, 4);

	for (int _iI = 0; _iI < iNbFiles; _iI++)
	{
		FileWrite(hFile, sInfoFile[_iI].pcFileName, strlen((const char *)sInfoFile[_iI].pcFileName) + 1);
		FileWrite(hFile, &sInfoFile[_iI].iTaille, 4);
		int _iT = sInfoFile[_iI].iNbCluster;
		FileWrite(hFile, &_iT, 4);
		_iT *= (sizeof(CCluster) - 4) + strlen((const char *)sInfoFile[_iI].pcFileName) + 1 + 20;
		FileWrite(hFile, &_iT, 4);

		CCluster * _pCluster = &sInfoFile[_iI].FirstCluster;

		while (_pCluster)
		{
			FileWrite(hFile, &_pCluster->iTaille, 4);
			FileWrite(hFile, &_pCluster->iNext, 4);
			_pCluster = _pCluster->pNext;
		}
	}

	FileSeek(hFile, 0, SEEK_SET);
	FileWrite(hFile, &iTailleBlock, 4);

	FileClose(hFile);
	hFile = NULL;
	ResetFAT();
	return true;
}

//------------------------------------------------------------------------
bool CSaveBlock::Defrag()
{
	char txt[256];
	strcpy(txt, pcBlockName.c_str());
	strcat(txt, "DFG");
	FileHandle fFileTemp = FileOpenWrite(txt);

	FileWrite(fFileTemp, &SAV_VERSION, 4);

	for (int _iI = (iVersion == SAV_VERSION_OLD) ? 1 : 0; _iI < iNbFiles; _iI++)
	{
		CCluster * pCluster = &sInfoFile[_iI].FirstCluster;
		void * pMem = malloc(sInfoFile[_iI].iTaille);
		char * pcMem = (char *)pMem;
		int iRealSize = 0;

		while (pCluster)
		{
			FileSeek(hFile, pCluster->iNext + 4, SEEK_SET);

			//bug size old version
			if (iVersion == SAV_VERSION_OLD)
			{
				if ((iRealSize + pCluster->iTaille) > sInfoFile[_iI].iTaille)
				{
					pMem = realloc(pMem, iRealSize + pCluster->iTaille);
					pcMem = ((char *)pMem) + iRealSize;
					sInfoFile[_iI].iTaille = iRealSize + pCluster->iTaille;
				}
			}

			iRealSize += pCluster->iTaille;

			FileRead(hFile, pcMem, pCluster->iTaille);
			pcMem += pCluster->iTaille;
			pCluster = pCluster->pNext;
		}

		FileWrite(fFileTemp, pMem, sInfoFile[_iI].iTaille);
		free(pMem);
	}

	FileWrite(fFileTemp, &SAV_VERSION, 4);
 
	int iOffset = 0;
	int iNbFilesTemp = (iVersion == SAV_VERSION_OLD) ? iNbFiles - 1 : iNbFiles; //-1;
	FileWrite(fFileTemp, &iNbFilesTemp, 4);

	for (int _iI = (iVersion == SAV_VERSION_OLD) ? 1 : 0; _iI < iNbFiles; _iI++)
	{
		FileWrite(fFileTemp, sInfoFile[_iI].pcFileName, strlen((const char *)sInfoFile[_iI].pcFileName) + 1);
		FileWrite(fFileTemp, &sInfoFile[_iI].iTaille, 4);
		int _iT = 0;
		FileWrite(fFileTemp, &_iT, 4);
		FileWrite(fFileTemp, &_iT, 4);

		//cluster
		FileWrite(fFileTemp, &sInfoFile[_iI].iTaille, 4);
		FileWrite(fFileTemp, &iOffset, 4);

		iOffset += sInfoFile[_iI].iTaille;
	}

	FileSeek(fFileTemp, 0, SEEK_SET);
	FileWrite(fFileTemp, &iOffset, 4);

	FileClose(hFile);
	hFile = NULL;
	ResetFAT();

	FileClose(fFileTemp);
	FileDelete(pcBlockName);
	FileMove(txt, pcBlockName.c_str());

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
bool CSaveBlock::Save( const std::string& _pcFileName, void * _pDatas, int _iSize)
{
	bool _bFound = false;
	CInfoFile * _pInfoFile = sInfoFile;
	int _iI = iNbFiles;

	while (_iI--)
	{
		if (!strcasecmp(_pInfoFile->pcFileName, _pcFileName.c_str()))
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
		_pInfoFile->Set(_pcFileName.c_str(), _iSize);

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
				FileSeek(hFile, _pClusterCurr->iNext + 4, SEEK_SET);
                iSizeWrite = std::min(_pClusterCurr->iTaille, iSize);
				FileWrite(hFile, _pDatas, iSizeWrite);
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
					FileSeek(hFile, 0, SEEK_END);
					FileWrite(hFile, _pDatas, iSize);
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

			FileSeek(hFile, 0, SEEK_END);

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

	if (_pDatas) FileWrite(hFile, _pDatas, _iSize);

	return true;
}

//------------------------------------------------------------------------
bool CSaveBlock::Read( const std::string& _pcFileName, char* _pPtr)
{
	CInfoFile * _pInfoFile = (CInfoFile *)pHachage->get(_pcFileName);

	if (!_pInfoFile)
	{
		return false;
	}


	CCluster * _pCluster = &_pInfoFile->FirstCluster;

	while (_pCluster)
	{
		FileSeek(hFile, _pCluster->iNext + 4, SEEK_SET);
		FileRead(hFile, _pPtr, _pCluster->iTaille);
		_pPtr += _pCluster->iTaille;
		_pCluster = _pCluster->pNext;
	}

	return true;
}

//------------------------------------------------------------------------
int CSaveBlock::GetSize( const std::string& _pcFileName)
{
	CInfoFile * _pInfoFile = NULL;

	if (pHachage)
	{
		_pInfoFile = (CInfoFile *)pHachage->get(_pcFileName);

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
			if (!strcasecmp(_pInfoFile->pcFileName, _pcFileName.c_str()))
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
bool CSaveBlock::ExistFile( const std::string& _pcFileName)
{
	CInfoFile * _pInfoFile = NULL;

	if (pHachage)
	{
		_pInfoFile = (CInfoFile *)pHachage->get(_pcFileName);
		return _pInfoFile ? true : false;
	}
	else
	{
		_pInfoFile = sInfoFile;
		int _iI = iNbFiles;

		while (_iI--)
		{
			if (!strcasecmp(_pInfoFile->pcFileName, _pcFileName.c_str()))
			{
				return true;
			}

			_pInfoFile++;
		}

		return false;
	}
}
