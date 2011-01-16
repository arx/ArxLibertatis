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
// Code: Cyril Meynier
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

// Desc: HERMES main functionalities
#ifndef  HERMESMAIN_H
#define  HERMESMAIN_H

#define HERMES_PATH_SIZE	512

#include "HERMES_PAK.h"
#include "HERMESPerf.h"

#include <windows.h>
#include <io.h>

typedef struct {
	long	secs;
	long	mins;
	long	hours;
	long	days;
	long	months;
	long	years;
}HERMES_DATE_TIME;

typedef struct {
	char text[260];
	void * previous;
	void * next;
	void * child;
	void * father;
} DIR_NODE;

typedef struct {
	 long	start;
	 long	current;
} HERMESTIMER;

///////////////////// PACKING
//Always on for now...
typedef struct PassedParam
{
   char *pSource;                   /* Pointer to source buffer           */
   char *pDestination;              /* Pointer to destination buffer      */
   unsigned long SourceOffset;      /* Offset into the source buffer      */
   unsigned long DestinationOffset; /* Offset into the destination buffer */
   unsigned long CompressedSize;    /* Need this for extracting!          */
   unsigned long UnCompressedSize;  /* Size of uncompressed data file     */
   unsigned long BufferSize;
   unsigned long Crc;               /* Calculated CRC value               */
   unsigned long OrigCrc;           /* Original CRC value of data         */
} PARAM;


#define	FILE_SEEK_START		0
#define	FILE_SEEK_CURRENT	1
#define	FILE_SEEK_END		2

extern HWND		MAIN_PROGRAM_HANDLE;
extern long DEBUGG;
extern long DebugLvl[6];
extern UINT			GaiaWM;
extern DIR_NODE mainnode;

void File_Standardize(char * from,char * to);
char * HERMES_GaiaCOM_Receive();
 
long KillAllDirectory(char * path);
void HERMES_InitDebug();

void SAFEstrcpy(char * dest, char * src, unsigned long max);


void MakeUpcase(char * str);
unsigned char IsIn(char * strin, char * str);
unsigned char NC_IsIn(char * strin, char * str);

long FileExist(char *name);
long DirectoryExist(char *name);
long	FileOpenRead(char *name);
 
long	FileOpenWrite(char *name);      
long	FileCloseWrite(long h);
long	FileCloseRead(long h);
long	FileRead(long h, void *adr, long size);
long	FileWrite(long h, void *adr, long size);
void	*FileLoadMalloc(char *name,long * filesize=NULL);
void	*FileLoadMallocZero(char *name,long * filesize=NULL);
 
 
void GetDate(HERMES_DATE_TIME * hdt);
void SendConsole(char * dat,long level,long flag,HWND source);
void ForceSendConsole(char * dat,long level,long flag,HWND source);
 
 
void MemFree(void * adr);
bool OKBox(char * text,char *title);
void ShowPopup(char * text);
int ShowError(char * funcname, char * message, long fatality);
unsigned long MakeMemoryText(char * text);
BOOL CreateFullPath(char * path);

// Strings Funcs
char *StringCopy(char * destination,char * source,long maxsize);
bool HERMESFolderSelector(char *file_name,char *title);
void RemoveName(char *str);
char * GetName(char *str);
char * GetExt(char *str);
void SetExt(char *str,char *new_ext);
void AddToName(char *str,char *cat);
int HERMESFileSelectorOpen(PSTR pstrFileName, PSTR pstrTitleName,char *filter,HWND hWnd);
int HERMESFileSelectorSave(PSTR pstrFileName, PSTR pstrTitleName,char *filter,HWND hWnd);
long HERMES_CreateFileCheck(const char *name, char *scheck, const long &size, const float &id);
char * STD_Implode(char * from,long from_size,long * to_size);
char * STD_Explode(char * from,long from_size,long * to_size);
void STD_ExplodeNoAlloc(char * from,long from_size,char * to,long * to_size);
void ERROR_Log_Init(char * fic);
bool ERROR_Log(char * fic);
void HERMES_Memory_Security_On(long size);
void HERMES_Memory_Security_Off();
long HERMES_Memory_Emergency_Out(long size=0,char * info=NULL);
extern LARGE_INTEGER	start_chrono;
void StartBench();
unsigned long EndBench();
extern long NEED_BENCH;
#endif // HERMESMAIN_H
