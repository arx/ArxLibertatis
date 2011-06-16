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

#include "core/Application.h"
#include "graphics/data/Mesh.h"
#include "graphics/GraphicsTypes.h"

#define MAX_ACTORS 10
const unsigned int MAX_SPEECH = 9;

//-----------------------------------------------------------------------------
struct ARX_CINEMATIC_SPEECH {
	long				type;
	Anglef startangle;
	Anglef endangle;
	float				startpos;
	float				endpos;
	float				f0;
	float				f1;
	float				f2;
	float				f3;
	long				ionum;
	Vec3f			pos1;
	Vec3f			pos2;

	void clear() {
		type = 0;
		startangle = Anglef::ZERO;
		endangle = Anglef::ZERO;
		startpos = 0;
		endpos = 0;
		f0 = 0;
		f1 = 0;
		f3 = 0;
		ionum = 0;
		pos1 = Vec3f::ZERO;
		pos2 = Vec3f::ZERO;
	}
};

struct ARX_CONVERSATION_STRUCT
{
	long				actors_nb;
	long				actors[MAX_ACTORS];
	long				current;
};

struct STRUCT_SPEECH {
	
	unsigned long timecreation;
	unsigned long duration;
	Color color;
	char name[64];
	std::string lpszUText;
	INTERACTIVE_OBJ * io;
	
	void clear() {
		timecreation = 0;
		duration = 0;
		color = Color::none;
		name[0] = 0;
		lpszUText.clear();
		io = NULL;
	}
	
};

struct ARX_SPEECH {
	
	long exist;
	ArxSound sample;
	long mood;
	long flags;
	unsigned long time_creation;
	unsigned long duration;
	float fDeltaY;
	int iTimeScroll;
	float fPixelScroll;
	std::string text;
	INTERACTIVE_OBJ * io;
	INTERACTIVE_OBJ * ioscript;
	ARX_CINEMATIC_SPEECH cine;
	EERIE_SCRIPT * es;
	long scrpos;
	
	void clear() {
		exist = 0;
		sample = 0;
		mood = 0;
		flags = 0;
		time_creation = 0;
		duration = 0;
		fDeltaY = 0;
		iTimeScroll = 0;
		fPixelScroll = 0;
		text.clear();
		io = NULL;
		ioscript = NULL;
		cine.clear();
		es = NULL;
		scrpos = 0;
	}
	
};

#define MAX_ASPEECH 100

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

extern ARX_SPEECH aspeech[MAX_ASPEECH];
extern ARX_CONVERSATION_STRUCT main_conversation;

//-----------------------------------------------------------------------------
void ARX_CONVERSATION_FirstInit();
void ARX_CONVERSATION_Reset();

void ARX_SPEECH_FirstInit();
void ARX_SPEECH_Reset();
void ARX_SPEECH_Update();
void ARX_SPEECH_Init();
void ARX_SPEECH_Check();
long ARX_SPEECH_Add(INTERACTIVE_OBJ * io, const std::string& _lpszUText, long duration = -1);
void ARX_SPEECH_ClearAll();
// data can be either a direct text or a localised string
// a localised string will be used to look for the duration of the sample
// & will play the sample.
long ARX_SPEECH_AddSpeech(INTERACTIVE_OBJ * io, const std::string& data, long mood, long flags = 0);
void ARX_SPEECH_ReleaseIOSpeech(INTERACTIVE_OBJ * io);
void ARX_SPEECH_ClearIOSpeech(INTERACTIVE_OBJ * io);
void ARX_SPEECH_Launch_No_Unicode_Seek(const char * string, INTERACTIVE_OBJ * io_source, long mood = 0);
bool ApplySpeechPos(EERIE_CAMERA * conversationcamera, long is);

#endif
