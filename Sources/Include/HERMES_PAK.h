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

#ifndef HERMES_PAK_H
#define HERMES_PAK_H

#include "hermes_pack_public.h"
#include <vector>
using namespace std;

extern char PAK_WORKDIR[256];
extern ULONG g_pak_workdir_len;


#define LOAD_TRUEFILE			1
#define LOAD_PACK				2
#define LOAD_PACK_THEN_TRUEFILE	3
#define LOAD_TRUEFILE_THEN_PACK	4

extern long CURRENT_LOADMODE;
extern EVE_LOADPACK *pLoadPack;
void * PAK_FileLoadMalloc(char *name,long * SizeLoadMalloc=NULL);
void * PAK_FileLoadMallocZero(char *name,long * SizeLoadMalloc=NULL);

// use only for READ !!!!
void PAK_SetLoadMode(long mode,char * pakfile,char * workdir=NULL);
FILE * PAK_fopen(const char *filename, const char *mode );
size_t PAK_fread(void *buffer, size_t size, size_t count, FILE *stream );;
int PAK_fclose(FILE * stream);
long PAK_ftell(FILE * stream);
long PAK_DirectoryExist(char *name);
long PAK_FileExist(char *name);
int PAK_fseek(FILE * fic,long offset,int origin);

void PAK_NotFoundInit(char * fic);
bool PAK_NotFound(char * fic);

void PAK_Close();

//-----------------------------------------------------------------------------
class PakManager
{
public:
	vector<EVE_LOADPACK*> vLoadPak;
public:
	PakManager();
	~PakManager();

	bool AddPak(char *);
	bool RemovePak(char *);
	bool Read(char *,void *);
	void* ReadAlloc(char *,int *);
	int GetSize(char *);
	PACK_FILE* fOpen(char *);
	int fClose(PACK_FILE *);
	int fRead(void *, int, int, PACK_FILE *);
	int fSeek(PACK_FILE *,int,int);
	int fTell(PACK_FILE *);
	vector<EVE_REPERTOIRE*>* ExistDirectory(char *_lpszName);
	bool ExistFile(char *_lpszName);
};

#endif