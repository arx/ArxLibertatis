/*
 * Copyright 2011-2021 Arx Libertatis Team (see the AUTHORS file)
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

#include "animation/Animation.h"

#include "core/Config.h"
#include "core/Core.h"
#include "core/Localisation.h"
#include "core/GameTime.h"

#include "game/EntityManager.h"
#include "game/NPC.h"
#include "game/Player.h"

#include "gui/Interface.h"
#include "gui/Text.h"
#include "gui/TextManager.h"

#include "graphics/Draw.h"
#include "graphics/DrawLine.h"
#include "graphics/Math.h"
#include "graphics/font/Font.h"

#include "io/resource/ResourcePath.h"
#include "io/log/Logger.h"

#include "scene/GameSound.h"
#include "scene/Interactive.h"

#include "script/ScriptEvent.h"


extern bool EXTERNALVIEW;
extern bool REQUEST_SPEECH_SKIP;

const size_t MAX_ASPEECH = 100;
static Speech g_aspeech[MAX_ASPEECH];

Speech * getSpeechForEntity(const Entity & entity) {
	
	for(Speech & speech : g_aspeech) {
		if(speech.exist && speech.io == &entity) {
			return &speech;
		}
	}
	
	return nullptr;
}

static void ARX_CONVERSATION_CheckAcceleratedSpeech() {
	
	if(REQUEST_SPEECH_SKIP) {
		for(size_t i = 0; i < MAX_ASPEECH; i++) {
			if((g_aspeech[i].exist) && !(g_aspeech[i].flags & ARX_SPEECH_FLAG_UNBREAKABLE)) {
				g_aspeech[i].duration = 0;
			}
		}
		REQUEST_SPEECH_SKIP = false;
	}
}

void ARX_SPEECH_FirstInit() {
	for(size_t i = 0 ; i < MAX_ASPEECH ; i++) {
		g_aspeech[i] = Speech();
	}
}

static long ARX_SPEECH_GetFree() {
	
	for(size_t i = 0; i < MAX_ASPEECH; i++) {
		if(!g_aspeech[i].exist) {
			g_aspeech[i].cine.type = ARX_CINE_SPEECH_NONE;
			return i;
		}
	}
	
	return -1;
}

static void ARX_SPEECH_Release(Speech & speech) {
	
	if(!speech.exist) {
		return;
	}
	
	ARX_SOUND_Stop(speech.sample);
	speech.sample = audio::SourcedSample();
	
	if(ValidIOAddress(speech.io) && speech.io->animlayer[2].cur_anim) {
		AcquireLastAnim(speech.io);
		speech.io->animlayer[2].cur_anim = nullptr;
	}
	
	speech = Speech();
	
}

void ARX_SPEECH_ReleaseIOSpeech(const Entity & entity) {
	
	if(Speech * speech = getSpeechForEntity(entity)) {
		ARX_SPEECH_Release(*speech);
	}
	
}

void ARX_SPEECH_Reset() {
	for(size_t i = 0; i < MAX_ASPEECH; i++) {
		ARX_SPEECH_Release(g_aspeech[i]);
	}
}

static void endSpeech(Speech & speech) {
	
	const EERIE_SCRIPT * script = speech.es;
	Entity * scriptEntity = speech.ioscript;
	size_t scrpos = speech.scrpos;
	
	ARX_SPEECH_Release(speech);
	
	if(script && ValidIOAddress(scriptEntity)) {
		ScriptEvent::resume(script, scriptEntity, scrpos);
	}
	
}

void ARX_SPEECH_ClearIOSpeech(const Entity & entity) {
	
	if(Speech * speech = getSpeechForEntity(entity)) {
		endSpeech(*speech);
	}
	
}


Speech * ARX_SPEECH_AddSpeech(Entity & speaker, std::string_view data, long mood, SpeechFlags flags) {
	
	if(data.empty()) {
		return nullptr;
	}
	
	ARX_SPEECH_ClearIOSpeech(speaker);
	
	long num = ARX_SPEECH_GetFree();
	if(num < 0) {
		return nullptr;
	}
	
	g_aspeech[num] = Speech();
	g_aspeech[num].exist = 1;
	g_aspeech[num].time_creation = g_gameTime.now();
	g_aspeech[num].io = &speaker;
	g_aspeech[num].duration = 2s; // Minimum value
	g_aspeech[num].flags = flags;
	g_aspeech[num].sample = audio::SourcedSample();
	g_aspeech[num].mood = mood;

	LogDebug("speech \"" << data << '"');
	
	res::path sample = data;
	
	if(flags & ARX_SPEECH_FLAG_NOTEXT) {
		
		// For non-conversation speech choose a random variant
		
		long count = getLocalisedKeyCount(data);
		long variant = 1;
		
		// TODO For some samples there are no corresponding entries
		// in the localization file  (utext_*.ini) -> count will be 0
		// We should probably just count the number of sample files
		
		if(count > 1) {
			do {
				variant = Random::get(1, count);
			} while(speaker.lastspeechflag == variant);
			speaker.lastspeechflag = checked_range_cast<short>(variant);
		}
		
		LogDebug(" -> " << variant << " / " << count);
		
		if(variant > 1) {
			sample.append(std::to_string(variant));
		}
		
	} else {
		
		speaker.lastspeechflag = 0;
		g_aspeech[num].text = getLocalised(data, "\x01");
		if(g_aspeech[num].text == "\x01") {
			g_aspeech[num].text.clear();
			LogWarning << "Speech requested with text but localisation is missing: " << data;
		}
		g_aspeech[num].duration = std::max(g_aspeech[num].duration,
		                                   GameDuration(s64(g_aspeech[num].text.length() + 1) * 100ms));
	}
	
	Entity * source = (g_aspeech[num].flags & ARX_SPEECH_FLAG_OFFVOICE) ? nullptr : &speaker;
	g_aspeech[num].sample = ARX_SOUND_PlaySpeech(sample, nullptr, source);
	
	// TODO Next lines must be removed (use callback instead)
	g_aspeech[num].duration = ARX_SOUND_GetDuration(g_aspeech[num].sample.getSampleId());
	
	if((speaker.ioflags & IO_NPC) && !(g_aspeech[num].flags & ARX_SPEECH_FLAG_OFFVOICE)) {
		g_aspeech[num].duration = g_aspeech[num].duration / speaker._npcdata->speakpitch;
	}
	
	if(g_aspeech[num].duration < 500ms) {
		g_aspeech[num].duration = 2s;
	}
	
	return &g_aspeech[num];
}

void ARX_SPEECH_Update() {
	
	GameInstant now = g_gameTime.now();
	
	if(cinematicBorder.isActive() || BLOCK_PLAYER_CONTROLS) {
		ARX_CONVERSATION_CheckAcceleratedSpeech();
	}
	
	for(Speech & speech : g_aspeech) {
		
		if(!speech.exist) {
			continue;
		}
		
		if(Entity * speaker = speech.io) {
			
			if(speech.flags & ARX_SPEECH_FLAG_OFFVOICE) {
				ARX_SOUND_RefreshSpeechPosition(speech.sample);
			} else {
				ARX_SOUND_RefreshSpeechPosition(speech.sample, speaker);
			}
			
			if((speaker != entities.player() || EXTERNALVIEW) && ValidIOAddress(speaker)) {
				if(!speaker->anims[speech.mood]) {
					speech.mood = ANIM_TALK_NEUTRAL;
				}
				if(ANIM_HANDLE * anim = speaker->anims[speech.mood]) {
					AnimLayer & layer2 = speaker->animlayer[2];
					if(layer2.cur_anim != anim || (layer2.flags & EA_ANIMEND)) {
						changeAnimation(speaker, 2, anim);
					}
				}
			}
			
		}
		
		// checks finished speech
		if(now >= speech.time_creation + speech.duration) {
			endSpeech(speech);
		}
		
	}
	
	if(!cinematicBorder.isActive() || cinematicBorder.CINEMA_DECAL < 100.f) {
		return;
	}
	
	for(Speech & speech : g_aspeech) {
		
		if(!speech.exist || speech.text.empty()) {
			continue;
		}
		
		Vec2i sSize = hFontInGame->getTextSize(speech.text);
		
		float fZoneClippHeight = static_cast<float>(sSize.y * 3);
		float fStartYY = 100 * g_sizeRatio.y;
		float fStartY = float((int(fStartYY) - int(fZoneClippHeight)) / 2);
		float fDepY = float(g_size.height()) - fStartYY + fStartY - speech.fDeltaY + sSize.y;
		float fZoneClippY = fDepY + speech.fDeltaY;
		
		float fAdd = fZoneClippY + fZoneClippHeight;
		
		Rect::Num y = checked_range_cast<Rect::Num>(fZoneClippY);
		Rect::Num h = checked_range_cast<Rect::Num>(fAdd);
		Rect clippingRect(0, y + 1, g_size.width(), h);
		if(config.interface.limitSpeechWidth) {
			s32 w = std::min(g_size.width(), s32(640 * g_sizeRatio.y));
			clippingRect.left = (g_size.width() - w) / 2;
			clippingRect.right = (g_size.width() + w) / 2;
		}
		
		float height = float(ARX_UNICODE_DrawTextInRect(hFontInGame,
		                                                Vec2f(clippingRect.left + 10.f, fDepY + fZoneClippHeight),
		                                                clippingRect.right - 10.f, speech.text,
		                                                Color::white, &clippingRect));
		
		UseRenderState state(render2D().blend(BlendZero, BlendInvSrcColor));
		EERIEDrawFill2DRectDegrad(Vec2f(0.f, fZoneClippY - 1.f),
		                          Vec2f(float(g_size.width()), fZoneClippY + float(sSize.y) * 1.5f),
		                          0.f, Color::white, Color::black);
		EERIEDrawFill2DRectDegrad(Vec2f(0.f, fZoneClippY + fZoneClippHeight - float(sSize.y) * 1.5f),
		                          Vec2f(float(g_size.width()), fZoneClippY + fZoneClippHeight),
		                          0.f, Color::black, Color::white);
		
		height += fZoneClippHeight;
		
		if(speech.fDeltaY > height) {
			continue;
		}
		
		float fDTime;
		
		if(speech.sample != audio::SourcedSample()) {
			
			GameDuration duration = ARX_SOUND_GetDuration(speech.sample.getSampleId());
			if(duration == 0) {
				duration = 4s;
			}
			
			fDTime = height * (g_gameTime.lastFrameDuration() / duration);
			float fTimeOneLine = sSize.y * fDTime;
			
			if(speech.iTimeScroll >= fTimeOneLine) {
				float fResteLine = sSize.y - speech.fPixelScroll;
				float fTimePlus = fResteLine * (g_gameTime.lastFrameDuration() / duration);
				fDTime -= fTimePlus;
				speech.fPixelScroll = 0.f;
				speech.iTimeScroll = 0;
			}
			speech.iTimeScroll += checked_range_cast<int>(g_framedelay);
			
		} else {
			
			fDTime = height * (g_gameTime.lastFrameDuration() / 4s);
			
		}
		
		speech.fDeltaY += fDTime;
		speech.fPixelScroll += fDTime;
		
	}
	
}

Speech * getCinematicSpeech() {
	
	for(Speech & speech : g_aspeech) {
		if(speech.exist && speech.cine.type != ARX_CINE_SPEECH_NONE) {
			return &speech;
		}
	}
	
	return nullptr;
}
