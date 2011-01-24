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

#include <string>

#include "HERMESPerf.h"

#include <windows.h>

#include <cstddef>

typedef struct {
	long	secs;
	long	mins;
	long	hours;
	long	days;
	long	months;
	long	years;
}HERMES_DATE_TIME;

///////////////////// PACKING
//Always on for now...
typedef struct PassedParam
{
	const char * pSource;                   /* Pointer to source buffer           */
	char * pDestination;              /* Pointer to destination buffer      */
	std::size_t SourceOffset;      /* Offset into the source buffer      */
	std::size_t DestinationOffset; /* Offset into the destination buffer */
	std::size_t CompressedSize;    /* Need this for extracting!          */
	std::size_t UnCompressedSize;  /* Size of uncompressed data file     */
	std::size_t BufferSize;
	unsigned long Crc;               /* Calculated CRC value               */
	unsigned long OrigCrc;           /* Original CRC value of data         */
} PARAM;


extern HWND		MAIN_PROGRAM_HANDLE;
extern long DEBUGG;
extern long DebugLvl[6];
extern unsigned int			GaiaWM;

void File_Standardize( const std::string& from, std::string& to );
char * HERMES_GaiaCOM_Receive();

void HERMES_InitDebug();

void SAFEstrcpy(char * dest, const char * src, unsigned long max);


void MakeUpcase( std::string& str);
unsigned char IsIn( const std::string& strin, const std::string& str);
unsigned char NC_IsIn( std::string strin, std::string str);

void GetDate(HERMES_DATE_TIME * hdt);
void SendConsole( const std::string& dat,long level,long flag,HWND source);
void ForceSendConsole( const std::string& dat,long level,long flag,HWND source);


void GetDate(HERMES_DATE_TIME * hdt);
void SendConsole(const char * dat,long level,long flag,HWND source);
void ForceSendConsole(const char * dat,long level,long flag,HWND source);

void MemFree(void * adr);
unsigned long MakeMemoryText(char * text);
bool CreateFullPath( const std::string& path );

// Strings Funcs
bool HERMESFolderSelector(char * file_name, const char * title);
void RemoveName( std::string& str );
std::string GetName( const std::string& str);
void SetExt( std::string& str, const std::string& new_ext );
char * GetExt(const char *str);
void AddToName(char *str, const char *cat);
int HERMESFileSelectorOpen(const char * pstrFileName, const char * pstrTitleName,const char *filter,HWND hWnd);
int HERMESFileSelectorSave(const char * pstrFileName, const char * pstrTitleName,const char *filter,HWND hWnd);
long HERMES_CreateFileCheck(const char *name, char *scheck, const long &size, const float &id);
char* STD_Explode( const std::string& from, size_t from_size, size_t& to_size);
void STD_ExplodeNoAlloc( const std::string& from, size_t from_size, char* to, size_t& to_size);
void ERROR_Log_Init( const std::string& fic);
bool ERROR_Log( const std::string& fic);
void HERMES_Memory_Security_On(long size);
void HERMES_Memory_Security_Off();
long HERMES_Memory_Emergency_Out(long size = 0, const char * info=NULL);
void StartBench();
unsigned long EndBench();
extern long NEED_BENCH;

#endif // HERMESMAIN_H
