/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */
/* Based on:
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
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#ifndef ARX_GUI_SPEECH_H
#define ARX_GUI_SPEECH_H

#include <string>

#include "audio/AudioTypes.h"
#include "math/Angle.h"

struct EERIE_CAMERA;
struct EERIE_SCRIPT;
class Entity;

const size_t MAX_SPEECH = 9;

enum CinematicSpeechMode {
	ARX_CINE_SPEECH_NONE,
	ARX_CINE_SPEECH_ZOOM, // uses start/endangle alpha & beta, startpos & endpos
	ARX_CINE_SPEECH_CCCTALKER_L,
	ARX_CINE_SPEECH_CCCTALKER_R,
	ARX_CINE_SPEECH_CCCLISTENER_L,
	ARX_CINE_SPEECH_CCCLISTENER_R,
	ARX_CINE_SPEECH_SIDE,
	ARX_CINE_SPEECH_KEEP,
	ARX_CINE_SPEECH_SIDE_LEFT
};

struct CinematicSpeech {
	
	CinematicSpeechMode type;
	Anglef startangle;
	Anglef endangle;
	float startpos;
	float endpos;
	float f0;
	float f1;
	float f2;
	long ionum;
	Vec3f pos1;
	Vec3f pos2;
	
	void clear() {
		type = ARX_CINE_SPEECH_NONE;
		startangle = Anglef::ZERO;
		endangle = Anglef::ZERO;
		startpos = 0;
		endpos = 0;
		f0 = 0;
		f1 = 0;
		ionum = 0;
		pos1 = Vec3f_ZERO;
		pos2 = Vec3f_ZERO;
	}
	
};

const size_t MAX_ACTORS = 10;
struct ARX_CONVERSATION_STRUCT {
	long actors_nb;
	long actors[MAX_ACTORS];
	long current;
};

struct Notification {
	
	unsigned long timecreation;
	unsigned long duration;
	std::string text;
	
	void clear() {
		timecreation = 0;
		duration = 0;
		text.clear();
	}
	
};

enum SpeechFlag {
	ARX_SPEECH_FLAG_UNBREAKABLE = (1<<0),
	ARX_SPEECH_FLAG_OFFVOICE    = (1<<1),
	ARX_SPEECH_FLAG_NOTEXT      = (1<<2),
	ARX_SPEECH_FLAG_DIRECT_TEXT = (1<<3)
};
DECLARE_FLAGS(SpeechFlag, SpeechFlags);
DECLARE_FLAGS_OPERATORS(SpeechFlags);

struct ARX_SPEECH {
	
	long exist;
	audio::SampleId sample;
	long mood;
	SpeechFlags flags;
	unsigned long time_creation;
	unsigned long duration;
	float fDeltaY;
	int iTimeScroll;
	float fPixelScroll;
	std::string text;
	Entity * io;
	Entity * ioscript;
	CinematicSpeech cine;
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

const size_t MAX_ASPEECH = 100;
extern ARX_SPEECH aspeech[MAX_ASPEECH];
extern ARX_CONVERSATION_STRUCT main_conversation;

void ARX_CONVERSATION_FirstInit();
void ARX_CONVERSATION_Reset();

void ARX_SPEECH_FirstInit();
void ARX_SPEECH_Reset();
void ARX_SPEECH_Update();
void ARX_SPEECH_Init();
void ARX_SPEECH_Check();

/*!
 * Add a raw text message to the "system" log (top of the screen).
 * This message will be displayed as-is.
 */
long ARX_SPEECH_Add(const std::string & text, long duration = -1);
void ARX_SPEECH_ClearAll();

/*!
 * Add an entry to the conversation view.
 * @param data is a sample name / localised string id
 */
long ARX_SPEECH_AddSpeech(Entity * io, const std::string & data, long mood, SpeechFlags flags = 0);
void ARX_SPEECH_ReleaseIOSpeech(Entity * io);
void ARX_SPEECH_ClearIOSpeech(Entity * io);
void ARX_SPEECH_Launch_No_Unicode_Seek(const std::string & string, Entity * io_source);
bool ApplySpeechPos(EERIE_CAMERA * conversationcamera, long is);

#endif // ARX_GUI_SPEECH_H
