/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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

#include "gui/Speech.h"

#include <cstdlib>
#include <cstdio>
#include <algorithm>

#include "core/Core.h"
#include "core/Localisation.h"
#include "core/GameTime.h"

#include "game/Player.h"

#include "gui/Interface.h"
#include "gui/Text.h"
#include "gui/TextManager.h"

#include "graphics/Draw.h"
#include "graphics/Math.h"
#include "graphics/font/Font.h"

#include "io/FilePath.h"
#include "io/log/Logger.h"

#include "scene/GameSound.h"
#include "scene/Interactive.h"

#include "script/ScriptEvent.h"

using std::min;
using std::max;
using std::string;
using std::transform;

extern TextureContainer *	arx_logo_tc;
extern long ARX_CONVERSATION;
extern long EXTERNALVIEW;
extern long REQUEST_SPEECH_SKIP;

ARX_SPEECH aspeech[MAX_ASPEECH];
long HIDESPEECH = 0;
Notification speech[MAX_SPEECH];


//-----------------------------------------------------------------------------
void ARX_SPEECH_Init()
{
	for (size_t i = 0 ; i < MAX_SPEECH ; i++ )
		speech[i].clear();
}

//-----------------------------------------------------------------------------
void ARX_SPEECH_MoveUp()
{
	if (speech[0].timecreation != 0)
	{
			speech[0].text.clear();
	}

	for (size_t j = 0; j < MAX_SPEECH - 1; j++)
	{
		speech[j] = speech[j+1];
	}

	speech[MAX_SPEECH-1].clear();
}

//-----------------------------------------------------------------------------
void ARX_SPEECH_ClearAll()
{
	for (size_t i = 0; i < MAX_SPEECH; i++)
	{
		if (speech[i].timecreation != 0) {
			speech[i].clear();
		}
	}
}

long ARX_SPEECH_Add(const string & text, long duration) {
	
	if(text.empty()) return -1;
	
	unsigned long tim = ARXTimeUL();
	if(tim == 0) {
		tim = 1;
	}
	
	if(speech[MAX_SPEECH - 1].timecreation != 0) {
		ARX_SPEECH_MoveUp();
	}
	
	for(size_t i = 0; i < MAX_SPEECH; i++) {
		if(speech[i].timecreation != 0) {
			continue;
		}
		
		// Sets creation time
		speech[i].timecreation = tim;
		
		// Sets/computes speech duration
		if(duration == -1) {
			speech[i].duration = 2000 + text.length() * 60;
		} else {
			speech[i].duration = duration;
		}
		
		speech[i].text = text;
		
		// Successfull allocation
		return speech[i].duration;
	}
	
	return -1;
}

//-----------------------------------------------------------------------------
bool CheckLastSpeech(int _iI)
{
	for (size_t i = _iI + 1; i < MAX_SPEECH; i++)
	{
		if ((speech[i].timecreation != 0) &&
				(!speech[i].text.empty()))
		{
			return false;
		}
	}

	return true;
}

void ARX_SPEECH_Render() {
	
	long igrec = 14;
	
	Vec2i sSize = hFontInBook->GetTextSize("p");
	sSize.y *= 3;
	
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	
	int iEnd = igrec + sSize.y;
	
	for(size_t i = 0; i < MAX_SPEECH; i++) {
		
		if(speech[i].timecreation == 0 || speech[i].text.empty()) {
			continue;
		}
		
		EERIEDrawBitmap(120 * Xratio - 16 * Xratio, static_cast<float>(igrec),
		                16 * Xratio, 16 * Xratio, .00001f, arx_logo_tc, Color::white);
		
		igrec += ARX_TEXT_DrawRect(hFontInBook, 120.f * Xratio, (float)igrec, 500 * Xratio,
		                           ' ' + speech[i].text, Color::white, NULL);
		if(igrec > iEnd && !CheckLastSpeech(i)) {
			ARX_SPEECH_MoveUp();
			break;
		}
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}

void ARX_SPEECH_Check()
{
	bool bClear = false;
	long exist = 0;

	for (size_t i = 0; i < MAX_SPEECH; i++)
	{
		if (speech[i].timecreation != 0)
		{
			if (ARXTime > speech[i].timecreation + speech[i].duration)
			{
				ARX_SPEECH_MoveUp();
				i--;
			}
			else exist++;

			bClear = true;
		}
	}

	if (bClear)
	{
		if (pTextManage)
		{
			pTextManage->Clear();
		}
	}

	if (exist) ARX_SPEECH_Render();
}

//-----------------------------------------------------------------------------
void ARX_SPEECH_Launch_No_Unicode_Seek(const string & text, INTERACTIVE_OBJ * io_source, long mood)
{
	mood = ANIM_TALK_NEUTRAL;
	long speechnum = ARX_SPEECH_AddSpeech(io_source, text, mood, ARX_SPEECH_FLAG_NOTEXT);

	if (speechnum >= 0)
	{
		aspeech[speechnum].scrpos = -1;
		aspeech[speechnum].es = NULL;
		aspeech[speechnum].ioscript = io_source;
		aspeech[speechnum].flags = 0;
		CinematicSpeech acs;
		acs.type = ARX_CINE_SPEECH_NONE;
		aspeech[speechnum].cine = acs;
	}
}


ARX_CONVERSATION_STRUCT main_conversation;
void ARX_CONVERSATION_FirstInit()
{
	main_conversation.actors_nb = 0;
	main_conversation.current = -1;
}
void ARX_CONVERSATION_Reset()
{
	main_conversation.actors_nb = 0;
	main_conversation.current = -1;
}

void ARX_CONVERSATION_CheckAcceleratedSpeech() {
	
	if(REQUEST_SPEECH_SKIP) {
		for(size_t i = 0; i < MAX_ASPEECH; i++) {
			if((aspeech[i].exist) && !(aspeech[i].flags & ARX_SPEECH_FLAG_UNBREAKABLE)) {
				aspeech[i].duration = 0;
			}
		}
		REQUEST_SPEECH_SKIP = 0;
	}
}


void ARX_SPEECH_FirstInit() {
	for(size_t i = 0 ; i < MAX_ASPEECH ; i++) {
		aspeech[i].clear();
	}
}

long ARX_SPEECH_GetFree() {
	
	for(size_t i = 0; i < MAX_ASPEECH; i++) {
		if(!aspeech[i].exist) {
			aspeech[i].cine.type = ARX_CINE_SPEECH_NONE;
			return i;
		}
	}
	
	return -1;
}

long ARX_SPEECH_GetIOSpeech(INTERACTIVE_OBJ * io) {
	
	for(size_t i = 0; i < MAX_ASPEECH; i++) {
		if(aspeech[i].exist && aspeech[i].io == io) {
			return i;
		}
	}
	
	return -1;
}

void ARX_SPEECH_Release(long i) {
	
	if(aspeech[i].exist) {
		
		ARX_SOUND_Stop(aspeech[i].sample);
		
		if(ValidIOAddress(aspeech[i].io) && aspeech[i].io->animlayer[2].cur_anim) {
			AcquireLastAnim(aspeech[i].io);
			aspeech[i].io->animlayer[2].cur_anim = NULL;
		}
		
		aspeech[i].clear();
	}
}

void ARX_SPEECH_ReleaseIOSpeech(INTERACTIVE_OBJ * io) {
	
	for(size_t i = 0; i < MAX_ASPEECH; i++) {
		if(aspeech[i].exist && aspeech->io == io) {
			ARX_SPEECH_Release(i);
		}
	}
}

void ARX_SPEECH_Reset() {
	for(size_t i = 0; i < MAX_ASPEECH; i++) {
		ARX_SPEECH_Release(i);
	}
}

void ARX_SPEECH_ClearIOSpeech(INTERACTIVE_OBJ * io) {
	
	if(!io) {
		return;
	}
	
	for(size_t i = 0; i < MAX_ASPEECH; i++) {
		
		if(!aspeech[i].exist || aspeech[i].io != io) {
			continue;
		}
		
		EERIE_SCRIPT * es = aspeech[i].es;
		INTERACTIVE_OBJ * io = aspeech[i].ioscript;
		long scrpos = aspeech[i].scrpos;
		ARX_SPEECH_Release(i);
		
		if(es && ValidIOAddress(io)) {
			ScriptEvent::send(es, SM_EXECUTELINE, "", io, "", scrpos);
		}
	}
}


long ARX_SPEECH_AddSpeech(INTERACTIVE_OBJ * io, const std::string& data, long mood, SpeechFlags flags) {
	
	if(data.empty()) {
		return -1;
	}
	
	ARX_SPEECH_ClearIOSpeech(io);
	
	long num = ARX_SPEECH_GetFree();
	if(num < 0) {
		return -1;
	}
	
	aspeech[num].exist = 1;
	aspeech[num].time_creation = ARX_TIME_GetUL();
	aspeech[num].io = io; // can be NULL
	aspeech[num].duration = 2000; // Minimum value
	aspeech[num].flags = flags;
	aspeech[num].sample = -1;
	aspeech[num].fDeltaY = 0.f;
	aspeech[num].iTimeScroll = 0;
	aspeech[num].fPixelScroll = 0.f;
	aspeech[num].mood = mood;

	long flg = 0;

	if (!(flags & ARX_SPEECH_FLAG_NOTEXT))
	{
		std::string _output = getLocalised(data);

		io->lastspeechflag = 0;
		aspeech[num].text.clear();
		aspeech[num].text = _output;
		aspeech[num].duration = max(aspeech[num].duration, (unsigned long)(strlen(_output.c_str()) + 1) * 100);
	}
	
	
	LogDebug("speech \"" << data << '"');

	if (flags & ARX_SPEECH_FLAG_NOTEXT)
	{
		long count = 0;

		count = getLocalisedKeyCount(data);

		do {
			flg = rnd() * count + 1;
		} while(io->lastspeechflag == flg && count > 1);

		if (flg > count) flg = count;
		else if (flg <= 0) flg = 1;
		
		LogDebug(" -> " << flg << " / " << count);

		io->lastspeechflag = (short)flg;
	}

	char speech_sample[256];
	if (flg > 1)
		sprintf(speech_sample, "%s%ld", data.c_str(), flg);
	else
		strcpy(speech_sample, data.c_str());

	if (aspeech[num].flags & ARX_SPEECH_FLAG_OFFVOICE)
		aspeech[num].sample = ARX_SOUND_PlaySpeech(speech_sample);
	else
		aspeech[num].sample = ARX_SOUND_PlaySpeech(speech_sample, io);
	if(aspeech[num].sample == ARX_SOUND_TOO_FAR) {
		aspeech[num].sample = audio::INVALID_ID;
	}

	//Next lines must be removed (use callback instead)
	aspeech[num].duration = (unsigned long)ARX_SOUND_GetDuration(aspeech[num].sample);

	if ((io->ioflags & IO_NPC) && !(aspeech[num].flags & ARX_SPEECH_FLAG_OFFVOICE)) {
		float fDiv = aspeech[num].duration /= io->_npcdata->speakpitch;
		aspeech[num].duration = static_cast<unsigned long>(fDiv);
	}

	if (aspeech[num].duration < 500) aspeech[num].duration = 2000;

	if (ARX_CONVERSATION && io)
		for (long j = 0; j < main_conversation.actors_nb; j++)
			if (main_conversation.actors[j] >= 0 && io == inter.iobj[main_conversation.actors[j]])
				main_conversation.current = num;

	return num;
}

void ARX_SPEECH_Update() {
	
	unsigned long tim = ARXTimeUL();

	if (CINEMASCOPE || BLOCK_PLAYER_CONTROLS) ARX_CONVERSATION_CheckAcceleratedSpeech();

	for (size_t i = 0 ; i < MAX_ASPEECH ; i++)
	{
		if (aspeech[i].exist)
		{
			INTERACTIVE_OBJ * io = aspeech[i].io;

			// updates animations
			if (io)
			{
				if (aspeech[i].flags & ARX_SPEECH_FLAG_OFFVOICE)
					ARX_SOUND_RefreshSpeechPosition(aspeech[i].sample);
				else
					ARX_SOUND_RefreshSpeechPosition(aspeech[i].sample, io);

				if (((io != inter.iobj[0]) || ((io == inter.iobj[0])  && (EXTERNALVIEW)))
						&&	ValidIOAddress(io))
				{
					if (io->anims[aspeech[i].mood] == NULL)	aspeech[i].mood = ANIM_TALK_NEUTRAL;

					if (io->anims[aspeech[i].mood] != NULL)
					{
						if ((io->animlayer[2].cur_anim != io->anims[aspeech[i].mood])
								||	(io->animlayer[2].flags & EA_ANIMEND))
						{
							AcquireLastAnim(io);
							ANIM_Set(&io->animlayer[2], io->anims[aspeech[i].mood]);
						}
					}
				}
			}

			// checks finished speech
			if (tim >= aspeech[i].time_creation + aspeech[i].duration)
			{
				EERIE_SCRIPT	*	es		= aspeech[i].es;
				INTERACTIVE_OBJ	* io		= aspeech[i].ioscript;
				long				scrpos	= aspeech[i].scrpos;
				ARX_SPEECH_Release(i);

				if ((es)
						&&	(ValidIOAddress(io)))
					ScriptEvent::send(es, SM_EXECUTELINE, "", io, "", scrpos);
			}
		}
	}

	for (size_t i = 0 ; i < MAX_ASPEECH ; i++)
	{
		ARX_SPEECH * speech = &aspeech[i];

		if (speech->exist)
		{
			if (!speech->text.empty())
			{
				if ((ARX_CONVERSATION) && (speech->io))
				{
					long ok = 0;

					for (long j = 0 ; j < main_conversation.actors_nb ; j++)
					{
						if (main_conversation.actors[j] >= 0)
							if (speech->io == inter.iobj[main_conversation.actors[j]])
							{
								ok = 1;
							}
					}

					if (!ok) goto next;
				}

				if (CINEMASCOPE && !HIDESPEECH)
				{
					if (CINEMA_DECAL >= 100.f)
					{
						Vec2i sSize = hFontInBook->GetTextSize(speech->text);
						
						float fZoneClippHeight	=	static_cast<float>(sSize.y * 3);
						float fStartYY			=	100 * Yratio;
						float fStartY			=	static_cast<float>(((int)fStartYY - (int)fZoneClippHeight) >> 1);
						float fDepY				=	((float)DANAESIZY) - fStartYY + fStartY - speech->fDeltaY + sSize.y;
						float fZoneClippY		=	fDepY + speech->fDeltaY;

						float fAdd = fZoneClippY + fZoneClippHeight ;

						Rect::Num y = checked_range_cast<Rect::Num>(fZoneClippY);
						Rect::Num h = checked_range_cast<Rect::Num>(fAdd);
						Rect clippingRect(0, y, DANAESIZX, h);
						float iTaille = (float)ARX_TEXT_DrawRect(
						                    hFontInBook,
						                    10.f,
						                    fDepY + fZoneClippHeight,
						                    -10.f + (float)DANAESIZX,
						                    speech->text,
						                    Color::white,
						                    &clippingRect);
						
						GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						GRenderer->SetRenderState(Renderer::DepthTest, false);
						EERIEDrawFill2DRectDegrad(0.f, fZoneClippY - 1.f,  static_cast<float>(DANAESIZX),
						                          fZoneClippY + (sSize.y * 3 / 4), 0.f, Color::white, Color::black);
						EERIEDrawFill2DRectDegrad(0.f, fZoneClippY + fZoneClippHeight - (sSize.y * 3 / 4),
						                          static_cast<float>(DANAESIZX), fZoneClippY + fZoneClippHeight,
						                          0.f, Color::black, Color::white);
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendZero);
						GRenderer->SetRenderState(Renderer::DepthTest, true);
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);
						
						iTaille += (int)fZoneClippHeight;

						if (((int)speech->fDeltaY) <= iTaille)
						{
							//vitesse du scroll
							float fDTime;

							if (speech->sample)
							{
								
								float duration = ARX_SOUND_GetDuration(speech->sample);
								if(duration == 0.0f) {
									duration = 4000.0f;
								}
								
								fDTime = ((float)iTaille * (float)FrameDiff) / duration; //speech->duration;
								float fTimeOneLine = ((float)sSize.y) * fDTime;

								if (((float)speech->iTimeScroll) >= fTimeOneLine)
								{
									float fResteLine = (float)sSize.y - speech->fPixelScroll;
									float fTimePlus = ((float)fResteLine * (float)FrameDiff) / duration;
									fDTime -= fTimePlus;
									speech->fPixelScroll = 0.f;
									speech->iTimeScroll = 0;
								}

								speech->iTimeScroll	+= checked_range_cast<int>(FrameDiff);
							}
							else
							{
								fDTime = ((float)iTaille * (float)FrameDiff) / 4000.0f;
							}

							speech->fDeltaY			+= fDTime;
							speech->fPixelScroll	+= fDTime;
						}
					}
				}
			}

		next:
			;
		}
	}


}

//-----------------------------------------------------------------------------
bool ApplySpeechPos(EERIE_CAMERA * conversationcamera, long is)
{
	if (is < 0) return false;

	if (aspeech[is].io == NULL)  return false;


	conversationcamera->d_pos.x = aspeech[is].io->pos.x;
	conversationcamera->d_pos.y = aspeech[is].io->pos.y + PLAYER_BASE_HEIGHT;
	conversationcamera->d_pos.z = aspeech[is].io->pos.z;
	float t = (aspeech[is].io->angle.b);
	conversationcamera->pos.x = conversationcamera->d_pos.x + (float)EEsin(t) * 100.f;
	conversationcamera->pos.y = conversationcamera->d_pos.y;
	conversationcamera->pos.z = conversationcamera->d_pos.z - (float)EEcos(t) * 100.f;

	return true;
}
