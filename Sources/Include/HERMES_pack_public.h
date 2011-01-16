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
#ifndef EVE_LOAD
#define EVE_LOAD

#include "implode.h"
#include "HERMES_pack.h"

#define PACK_MAX_FREAD	(256)

typedef struct
{
	FILE *file;
	char *mem;
	long lSize;
} PAK_PARAM;

typedef struct
{
	FILE	*file;
	char	*mem;
	int		iOffsetCurr;
	int		iOffset;
	int		iOffsetBase;
	int		iTaille;
	int		iTailleBase;
	int		iTailleW;
	int		iTailleFic;
} PAK_PARAM_FREAD;

typedef struct
{
	bool		bActif;
	int			iID;
	int			iOffset;
	EVE_TFILE	*pFile;
} PACK_FILE;

class EVE_LOADPACK
{
private:
	FILE			*pfFile;
	char			*pcFAT;
	int				iTailleFAT;
	int				iSeekPak;
	PACK_FILE		tPackFile[PACK_MAX_FREAD];

	unsigned char	cKey[512];
	unsigned int	iPassKey;
public:
	char			*lpszName;
	EVE_REPERTOIRE	*pRoot;
private:
	int ReadFAT_int();
	char* ReadFAT_string();
public:
	EVE_LOADPACK();
	~EVE_LOADPACK();

	bool Open(char*);
	void Close();
	bool Read(char*,void*);
	void* ReadAlloc(char*,int*);
	int GetSize(char *_pcName);

	PACK_FILE * fOpen(const char*,const char*);
	int fClose(PACK_FILE*);
	int fRead(void*,int,int,PACK_FILE*);
	int fSeek(PACK_FILE*,unsigned long,int);
	int fTell(PACK_FILE *);

	void WriteSousRepertoire(char *pcAbs,EVE_REPERTOIRE *r);
	void WriteSousRepertoireZarbi(char *pcAbs,EVE_REPERTOIRE *r);

	void CryptChar(unsigned char*);
	void UnCryptChar(unsigned char*);
	void CryptString(unsigned char*);
	int UnCryptString(unsigned char*);
	void CryptShort(unsigned short*);
	void UnCryptShort(unsigned short*);
	void CryptInt(unsigned int*);
	void UnCryptInt(unsigned int*);
}; 

#endif