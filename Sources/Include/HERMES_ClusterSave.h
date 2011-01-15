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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ARX_Common.h>

#define PAK_VERSION	((1<<16)|0)

#define MAX_FILES		1024
#define EMPTY_CLUSTER	"___EMPTY___"

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
	void Set(char *_pcFileName,int _iTaille);
	void KillAll();
};


class CHachageString;

class CSaveBlock{
private:
	
	FILE		*hFile;
	int			iTailleBlock;
	int			iNbFiles;
	bool		bReWrite;
	CInfoFile *	sInfoFile;
	bool		bFirst;
	int			iVersion;
	bool ExpandNbFiles();
	CHachageString* pHachage;

public:
	char		*pcBlockName;
	CSaveBlock(char *_pcBlockName=NULL);
	~CSaveBlock();

	bool Defrag();
	bool BeginSave(bool _bCont,bool _bReWrite);
	bool EndSave(void);
	bool Save(char *_pcFileName,void *_pDatas,int _iSize);

	bool BeginRead(void);
	void EndRead(void);
	bool Read(char *_pcFileName,char *_pPtr);
	int	 GetSize(char *_pcFileName);
	bool ExistFile(char *_pcFileName);
	void ResetFAT(void);
};
