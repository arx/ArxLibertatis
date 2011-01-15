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
// ARX_Speech
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Speech & Conversation Management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
#ifndef ARX_SPEECH_H
#define ARX_SPEECH_H

#define MAX_ACTORS 10

#include <tchar.h>
#include "EERIEapp.h"
#include "EERIEPOLY.h"
#include "EERIEtypes.h"

const unsigned long MAX_SPEECH(9);

//-----------------------------------------------------------------------------
typedef struct
{
	long				type;
	EERIE_3D			startangle;
	EERIE_3D			endangle;
	float				startpos;
	float				endpos;
	float				f0;
	float				f1;
	float				f2;
	float				f3;
	long				ionum;
	EERIE_3D			pos1;
	EERIE_3D			pos2;
} ARX_CINEMATIC_SPEECH;

typedef struct
{
	long				actors_nb;
	long				actors[MAX_ACTORS];
	long				current;
} ARX_CONVERSATION_STRUCT;

typedef struct
{
	unsigned long	timecreation;
	unsigned long	duration;
	D3DCOLOR		color;
	char			name[64];
	_TCHAR		*	lpszUText;
	INTERACTIVE_OBJ * io;
} STRUCT_SPEECH;

typedef struct
{
	long				exist;
	long				sample;
	long				mood;
	long				flags; 
	unsigned long		time_creation;
	unsigned long		duration;
	float				fDeltaY;
	int					iTimeScroll;
	float				fPixelScroll;
	D3DCOLOR			color;
	_TCHAR		*		text;
	INTERACTIVE_OBJ	*	io;
	INTERACTIVE_OBJ	*	ioscript;
	ARX_CINEMATIC_SPEECH cine;
	EERIE_SCRIPT	*	es;
	long				scrpos;
} ARX_SPEECH;

#define MAX_ASPEECH						100

enum ARX_CINE_SPEECH_MODE
{
	ARX_CINE_SPEECH_NONE,
	ARX_CINE_SPEECH_ZOOM,         // uses start/endangle alpha & beta, startpos & endpos
	ARX_CINE_SPEECH_CCCTALKER_L,
	ARX_CINE_SPEECH_CCCTALKER_R,
	ARX_CINE_SPEECH_CCCLISTENER_L,
	ARX_CINE_SPEECH_CCCLISTENER_R,
	ARX_CINE_SPEECH_SIDE,
	ARX_CINE_SPEECH_KEEP,
	ARX_CINE_SPEECH_SIDE_LEFT
};

enum ARX_SPEECH_FLAG
{
	ARX_SPEECH_FLAG_UNBREAKABLE = 0x00000001,
	ARX_SPEECH_FLAG_OFFVOICE    = 0x00000002,
	ARX_SPEECH_FLAG_NOTEXT      = 0x00000004,
	ARX_SPEECH_FLAG_DIRECT_TEXT	= 0x00000008
};

//-----------------------------------------------------------------------------
#define PARAM_LOCALISED					1

//-----------------------------------------------------------------------------
extern ARX_SPEECH aspeech[MAX_ASPEECH];
extern ARX_CONVERSATION_STRUCT main_conversation;

//-----------------------------------------------------------------------------
void ARX_CONVERSATION_FirstInit();
void ARX_CONVERSATION_Reset();

void ARX_SPEECH_FirstInit();
void ARX_SPEECH_Reset();
void ARX_SPEECH_Update(LPDIRECT3DDEVICE7 pd3dDevice);
void ARX_SPEECH_Init();
void ARX_SPEECH_Check(LPDIRECT3DDEVICE7 pd3dDevice);
long ARX_SPEECH_Add(INTERACTIVE_OBJ * io, _TCHAR * _lpszUText, long duration = -1);
void ARX_SPEECH_ClearAll();
// data can be either a direct text or a localised string
// a localised string will be used to look for the duration of the sample
// & will play the sample.
// param can be 0 (plain text) or PARAM_LOCALISED (localised string)
long ARX_SPEECH_AddSpeech(INTERACTIVE_OBJ * io, char * data, long param, long mood, long flags = 0);
void ARX_SPEECH_ReleaseIOSpeech(INTERACTIVE_OBJ * io);
void ARX_SPEECH_ClearIOSpeech(INTERACTIVE_OBJ * io);
void ARX_SPEECH_Launch_No_Unicode_Seek(char * string, INTERACTIVE_OBJ * io_source, long mood = 0);
BOOL ApplySpeechPos(EERIE_CAMERA * conversationcamera, long is);

#endif
