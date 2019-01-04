/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "script/ScriptedConversation.h"

#include <sstream>

#include "core/Localisation.h"
#include "graphics/Math.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/NPC.h"
#include "gui/Speech.h"
#include "gui/Interface.h"
#include "gui/Notification.h"
#include "io/resource/ResourcePath.h"
#include "scene/Interactive.h"
#include "scene/GameSound.h"
#include "script/ScriptUtils.h"


extern Vec3f LASTCAMPOS;
extern Anglef LASTCAMANGLE;

namespace script {

namespace {

class ConversationCommand : public Command {
	
public:
	
	ConversationCommand() : Command("conversation") { }
	
	Result execute(Context & context) {
		
		std::string param = context.getWord();
		if(param != "off") {
			LogWarning << "Invalid used of stubbed conversation command";
		}
		
		return Success;
	}
	
};

class PlayCommand : public Command {
	
public:
	
	PlayCommand() : Command("play", AnyEntity) { }
	
	Result execute(Context & context) {
		
		bool unique = false;
		SoundLoopMode loop = ARX_SOUND_PLAY_ONCE;
		float pitch = 1.f;
		bool stop = false;
		bool no_pos = false;
		
		HandleFlags("ilpso") {
			unique = test_flag(flg, 'i');
			if(flg & flag('l')) {
				loop = ARX_SOUND_PLAY_LOOPED;
			}
			if(flg & flag('p')) {
				pitch = Random::getf(0.9f, 1.1f);
			}
			stop = test_flag(flg, 's');
			no_pos = test_flag(flg, 'o');
		}
		
		res::path sample = res::path::load(context.getStringVar(context.getWord())).set_ext("wav");
		
		DebugScript(' ' << options << ' ' << sample);
		
		Entity * io = context.getEntity();
		if(stop) {
			ARX_SOUND_Stop(io->m_sound);
			io->m_sound = audio::SourcedSample();
		} else {
			
			if(unique) {
				ARX_SOUND_Stop(io->m_sound);
				io->m_sound = audio::SourcedSample();
			}
			
			audio::SourcedSample num = audio::SourcedSample();
			bool tooFar = false;
			// TODO(broken-scripts) should be a flag instead of depending on the event
			if(no_pos || SM_INVENTORYUSE == context.getMessage()) {
				num = ARX_SOUND_PlayScript(sample, tooFar, NULL, pitch, loop);
			} else {
				num = ARX_SOUND_PlayScript(sample, tooFar, io, pitch, loop);
			}
			
			if(unique) {
				io->m_sound = num;
			}
			
			if(!tooFar && num == audio::SourcedSample()) {
				ScriptWarning << "unable to load sound file " << sample;
				return Failed;
			}
			
		}
		
		return Success;
	}
	
};

class PlaySpeechCommand : public Command {
	
public:
	
	PlaySpeechCommand() : Command("playspeech") { }
	
	Result execute(Context & context) {
		
		res::path sample = res::path::load(context.getWord());
		
		DebugScript(' ' << sample);
		
		Entity * io = context.getEntity();
		
		// TODO check if we actually need to succeed if tooFar becomes true
		bool tooFar = false;
		audio::SourcedSample num = ARX_SOUND_PlaySpeech(sample, &tooFar, io && io->show == 1 ? io : NULL);
		
		if(!tooFar && num == audio::SourcedSample()) {
			ScriptWarning << "unable to load sound file " << sample;
			return Failed;
		}
		
		return Success;
	}
	
};

class HeroSayCommand : public Command {
	
public:
	
	HeroSayCommand() : Command("herosay") { }
	
	Result execute(Context & context) {
		
		HandleFlags("d") {
			if((flg & flag('d'))) {
				context.skipWord();
				return Success;
			}
		}
		
		std::string text = context.getStringVar(context.getWord());
		
		DebugScript(' ' << options << " \"" << text << '"');
		
		if(!text.empty() && text[0] == '[') {
			text = getLocalised(loadUnlocalized(text));
		}
		
		notification_add(text);
		
		return Success;
	}
	
	Result peek(Context & context) {
		
		/*
		 * TODO Technically this command has a side effect so it should abort non-destructive execution.
		 * However, the on combine event for books unconditionally uses this command. resulting
		 * in all books lighting up in the inventory when an item is selected for combining.
		 */
		
		(void)context.getFlags();
		
		context.skipWord();
		
		return Success;
	}
	
};

class SetSpeakPitchCommand : public Command {
	
public:
	
	SetSpeakPitchCommand() : Command("setspeakpitch", IO_NPC) { }
	
	Result execute(Context & context) {
		
		float pitch = context.getFloat();
		if(pitch < .6f) {
			pitch = .6f;
		}
		
		DebugScript(' ' << pitch);
		
		context.getEntity()->_npcdata->speakpitch = pitch;
		
		return Success;
	}
	
};

class SpeakCommand : public Command {
	
	static void computeACSPos(CinematicSpeech & acs, Entity * io, EntityHandle ionum) {
		
		if(io) {
			ActionPoint id = io->obj->fastaccess.view_attach;
			if(id != ActionPoint()) {
				acs.pos1 = actionPointPosition(io->obj, id);
			} else {
				acs.pos1 = io->pos + Vec3f(0.f, io->physics.cyl.height, 0.f);
			}
		}
		
		if(ValidIONum(ionum)) {
			Entity * ioo = entities[ionum];
			ActionPoint id = ioo->obj->fastaccess.view_attach;
			if(id != ActionPoint()) {
				acs.pos2 = actionPointPosition(ioo->obj, id);
			} else {
				acs.pos2 = ioo->pos + Vec3f(0.f, ioo->physics.cyl.height, 0.f);
			}
		}
	}
	
	static void parseParams(CinematicSpeech & acs, Context & context, Entity * speaker) {
		
		std::string target = context.getWord();
		Entity * t = entities.getById(target, context.getEntity());
		
		acs.ionum = (t == NULL) ? EntityHandle() : t->index();
		acs.startpos = context.getFloat();
		acs.endpos = context.getFloat();
		
		computeACSPos(acs, speaker, acs.ionum);
	}
	
	void parseCinematicSpeech(CinematicSpeech & acs, Context & context, Entity * speaker) {
		
		std::string command = context.getWord();
		
		if(command == "keep") {
			acs.type = ARX_CINE_SPEECH_KEEP;
			acs.pos1 = LASTCAMPOS;
			acs.pos2.x = LASTCAMANGLE.getPitch();
			acs.pos2.y = LASTCAMANGLE.getYaw();
			acs.pos2.z = LASTCAMANGLE.getRoll();
			
		} else if(command == "zoom") {
			acs.type = ARX_CINE_SPEECH_ZOOM;
			acs.startangle.setPitch(context.getFloat());
			acs.startangle.setYaw(context.getFloat());
			acs.endangle.setPitch(context.getFloat());
			acs.endangle.setYaw(context.getFloat());
			acs.startpos = context.getFloat();
			acs.endpos = context.getFloat();
			acs.ionum = context.getEntity()->index();
			computeACSPos(acs, speaker, acs.ionum);
			
		} else if(command == "ccctalker_l" || command == "ccctalker_r") {
			acs.type = (command == "ccctalker_r") ? ARX_CINE_SPEECH_CCCTALKER_R : ARX_CINE_SPEECH_CCCTALKER_L;
			parseParams(acs, context, speaker);
			
		} else if(command == "ccclistener_l" || command == "ccclistener_r") {
			acs.type = (command == "ccclistener_r") ? ARX_CINE_SPEECH_CCCLISTENER_R :  ARX_CINE_SPEECH_CCCLISTENER_L;
			parseParams(acs, context, speaker);
			
		} else if(command == "side" || command == "side_l" || command == "side_r") {
			acs.type = (command == "side_l") ? ARX_CINE_SPEECH_SIDE_LEFT : ARX_CINE_SPEECH_SIDE;
			parseParams(acs, context, speaker);
			acs.m_startdist = context.getFloat(); // startdist
			acs.m_enddist = context.getFloat(); // enddist
			acs.m_heightModifier = context.getFloat(); // height modifier
		} else {
			ScriptWarning << "unexpected command: " << command;
		}
		
	}
	
public:
	
	SpeakCommand() : Command("speak") { }
	
	Result execute(Context & context) {
		
		CinematicSpeech acs;
		acs.type = ARX_CINE_SPEECH_NONE;
		
		Entity * speaker = context.getEntity();
		
		bool unbreakable = false;
		SpeechFlags voixoff = 0;
		AnimationNumber mood = ANIM_TALK_NEUTRAL;
		HandleFlags("tuphaoc") {
			
			voixoff |= (flg & flag('t')) ? ARX_SPEECH_FLAG_NOTEXT : SpeechFlags(0);
			unbreakable = test_flag(flg, 'u');
			if(flg & flag('p')) {
				speaker = entities.player();
			}
			if(flg & flag('h')) {
				mood = ANIM_TALK_HAPPY;
			}
			if(flg & flag('a')) {
				mood = ANIM_TALK_ANGRY;
			}
			
			// Crash when we set speak pitch to 1,
			// Variable use for a division, 0 is not possible
			voixoff |= (flg & flag('o')) ? ARX_SPEECH_FLAG_OFFVOICE : SpeechFlags(0);
			
			if(flg & flag('c')) {
				parseCinematicSpeech(acs, context, speaker);
			}
			
		}
		
		std::string text = context.getWord();
		
		if(text == "killall") {
			
			if(!options.empty()) {
				ScriptWarning << "unexpected options: " << options << " killall";
			}
			
			ARX_SPEECH_Reset();
			return Success;
		}
		
		std::string data = loadUnlocalized(context.getStringVar(text));
		
		DebugScript(' ' << options << ' ' << data); // TODO debug more
		
		if(data.empty()) {
			ARX_SPEECH_ClearIOSpeech(context.getEntity());
			return Success;
		}
		
		
		if(!cinematicBorder.isActive()) {
			voixoff |= ARX_SPEECH_FLAG_NOTEXT;
		}
		
		long speechnum = ARX_SPEECH_AddSpeech(speaker, data, mood, voixoff);
		if(speechnum < 0) {
			return Failed;
		}
		
		size_t onspeechend = context.skipCommand();
		
		if(onspeechend != size_t(-1)) {
			g_aspeech[speechnum].scrpos = onspeechend;
			g_aspeech[speechnum].es = context.getScript();
			g_aspeech[speechnum].ioscript = context.getEntity();
			if(unbreakable) {
				g_aspeech[speechnum].flags |= ARX_SPEECH_FLAG_UNBREAKABLE;
			}
			g_aspeech[speechnum].cine = acs;
		}
		
		return Success;
	}
	
	Result peek(Context & context) {
		
		HandleFlags("tuphaoc") {
			
			if(flg & flag('c')) {
				CinematicSpeech acs;
				parseCinematicSpeech(acs, context, context.getEntity());
			}
			
		}
		
		std::string text = context.getWord();
		
		if(text == "killall") {
			return Success;
		}
		
		std::string data = loadUnlocalized(context.getStringVar(text));
		
		if(data.empty()) {
			return Success;
		}
		
		std::string command = context.getCommand(false);
		
		size_t onspeechend = context.skipCommand();
		
		if((!command.empty() && command != "nop") || onspeechend != size_t(-1)) {
			return AbortDestructive;
		}
		
		return Success;
	}
	
};

class SetStrikeSpeechCommand : public Command {
	
public:
	
	SetStrikeSpeechCommand() : Command("setstrikespeech", AnyEntity) { }
	
	Result execute(Context & context) {
		
		context.getEntity()->strikespeech = script::loadUnlocalized(context.getWord());
		
		DebugScript(' ' << context.getEntity()->strikespeech);
		
		return Success;
	}
	
};

} // anonymous namespace

void setupScriptedConversation() {
	
	ScriptEvent::registerCommand(new ConversationCommand);
	ScriptEvent::registerCommand(new PlayCommand);
	ScriptEvent::registerCommand(new PlaySpeechCommand);
	ScriptEvent::registerCommand(new HeroSayCommand);
	ScriptEvent::registerCommand(new SetSpeakPitchCommand);
	ScriptEvent::registerCommand(new SpeakCommand);
	ScriptEvent::registerCommand(new SetStrikeSpeechCommand);
	
}

} // namespace script
