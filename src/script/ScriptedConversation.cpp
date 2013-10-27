/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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
#include "io/resource/ResourcePath.h"
#include "scene/Interactive.h"
#include "scene/GameSound.h"
#include "script/ScriptUtils.h"

using std::string;

extern bool ARX_CONVERSATION;
extern Vec3f LASTCAMPOS;
extern Anglef LASTCAMANGLE;

namespace script {

namespace {

class ConversationCommand : public Command {
	
public:
	
	ConversationCommand() : Command("conversation") { }
	
	Result execute(Context & context) {
		
		string nbpeople = context.getWord();
		long nb_people = 0;
		if(!nbpeople.empty() && nbpeople[0] == '-') {
			std::istringstream iss(nbpeople.substr(1));
			iss >> nb_people;
			if(iss.bad()) {
				nb_people = 0;
			}
		}
		
		bool enabled = context.getBool();
		ARX_CONVERSATION = enabled;
		
		if(!nb_people || !enabled) {
			DebugScript(' ' << nbpeople << ' ' << enabled);
			return Success;
		}
		
		main_conversation.actors_nb = nb_people;
		
#ifdef ARX_DEBUG
		std::ostringstream oss;
		oss << ' ' << nbpeople << ' ' << enabled;
#endif
		
		for(long j = 0; j < nb_people; j++) {
			
			string target = context.getWord();
			Entity * t = entities.getById(target, context.getEntity());
			
#ifdef ARX_DEBUG
			oss << ' ' << target;
#endif
			
			main_conversation.actors[j] = (t == NULL) ? -1 : t->index();
		}
		
#ifdef ARX_DEBUG
		DebugScript(oss.rdbuf());
#endif
		
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
				pitch = 0.9F + 0.2F * rnd();
			}
			stop = test_flag(flg, 's');
			no_pos = test_flag(flg, 'o');
		}
		
		res::path sample = res::path::load(context.getStringVar(context.getWord())).set_ext("wav");
		
		DebugScript(' ' << options << ' ' << sample);
		
		Entity * io = context.getEntity();
		if(stop) {
			ARX_SOUND_Stop(io->sound);
			io->sound = audio::INVALID_ID;
			
		} else {
			
			if(unique && io->sound != audio::INVALID_ID) {
				ARX_SOUND_Stop(io->sound);
			}
			
			audio::SampleId num;
			// TODO(broken-scripts) should be a flag instead of depending on the event
			if(no_pos || SM_INVENTORYUSE == context.getMessage()) {
				num = ARX_SOUND_PlayScript(sample, NULL, pitch, loop);
			} else {
				num = ARX_SOUND_PlayScript(sample, io, pitch, loop);
			}
			
			if(unique) {
				io->sound = (num == ARX_SOUND_TOO_FAR) ? audio::INVALID_ID : num;
			}
			
			if(num == audio::INVALID_ID) {
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
		audio::SampleId num = ARX_SOUND_PlaySpeech(sample, io && io->show == 1 ? io : NULL);
		
		if(num == audio::INVALID_ID) {
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
		
		string text = context.getStringVar(context.getWord());
		
		DebugScript(' ' << options << " \"" << text << '"');
		
		if(!text.empty() && text[0] == '[') {
			text = getLocalised(loadUnlocalized(text), "Not Found");
		}
		
		ARX_SPEECH_Add(text);
		
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
	
	static void computeACSPos(CinematicSpeech & acs, Entity * io, long ionum) {
		
		if(io) {
			long id = io->obj->fastaccess.view_attach;
			if(id != -1) {
				acs.pos1 = io->obj->vertexlist3[id].v;
			} else {
				acs.pos1 = io->pos + Vec3f(0.f, io->physics.cyl.height, 0.f);
			}
		}
		
		if(ValidIONum(ionum)) {
			Entity * ioo = entities[ionum];
			long id = ioo->obj->fastaccess.view_attach;
			if(id != -1) {
				acs.pos2 = ioo->obj->vertexlist3[id].v;
			} else {
				acs.pos2 = ioo->pos + Vec3f(0.f, ioo->physics.cyl.height, 0.f);
			}
		}
	}
	
	static void parseParams(CinematicSpeech & acs, Context & context, bool player) {
		
		string target = context.getWord();
		Entity * t = entities.getById(target, context.getEntity());
		
		acs.ionum = (t == NULL) ? -1 : t->index();
		acs.startpos = context.getFloat();
		acs.endpos = context.getFloat();
		
		if(player) {
			computeACSPos(acs, entities.player(), acs.ionum);
		} else {
			computeACSPos(acs, context.getEntity(), acs.ionum);
		}
	}
	
public:
	
	SpeakCommand() : Command("speak") { }
	
	Result execute(Context & context) {
		
		CinematicSpeech acs;
		acs.type = ARX_CINE_SPEECH_NONE;
		
		Entity * io = context.getEntity();
		
		bool player = false, unbreakable = false;
		SpeechFlags voixoff = 0;
		AnimationNumber mood = ANIM_TALK_NEUTRAL;
		HandleFlags("tuphaoc") {
			
			voixoff |= (flg & flag('t')) ? ARX_SPEECH_FLAG_NOTEXT : SpeechFlags(0);
			unbreakable = test_flag(flg, 'u');
			player = test_flag(flg, 'p');
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
				
				string command = context.getWord();
				
				if(command == "keep") {
					acs.type = ARX_CINE_SPEECH_KEEP;
					acs.pos1 = LASTCAMPOS;
					acs.pos2.x = LASTCAMANGLE.getYaw();
					acs.pos2.y = LASTCAMANGLE.getPitch();
					acs.pos2.z = LASTCAMANGLE.getRoll();
					
				} else if(command == "zoom") {
					acs.type = ARX_CINE_SPEECH_ZOOM;
					acs.startangle.setYaw(context.getFloat());
					acs.startangle.setPitch(context.getFloat());
					acs.endangle.setYaw(context.getFloat());
					acs.endangle.setPitch(context.getFloat());
					acs.startpos = context.getFloat();
					acs.endpos = context.getFloat();
					acs.ionum = (io == NULL) ? -1 : io->index();
					if(player) {
						computeACSPos(acs, entities.player(), acs.ionum);
					} else {
						computeACSPos(acs, io, -1);
					}
					
				} else if(command == "ccctalker_l" || command == "ccctalker_r") {
					acs.type = (command == "ccctalker_r") ? ARX_CINE_SPEECH_CCCTALKER_R : ARX_CINE_SPEECH_CCCTALKER_L;
					parseParams(acs, context, player);
					
				} else if(command == "ccclistener_l" || command == "ccclistener_r") {
					acs.type = (command == "ccclistener_r") ? ARX_CINE_SPEECH_CCCLISTENER_R :  ARX_CINE_SPEECH_CCCLISTENER_L;
					parseParams(acs, context, player);
					
				} else if(command == "side" || command == "side_l" || command == "side_r") {
					acs.type = (command == "side_l") ? ARX_CINE_SPEECH_SIDE_LEFT : ARX_CINE_SPEECH_SIDE;
					parseParams(acs, context, player);
					acs.f0 = context.getFloat(); // startdist
					acs.f1 = context.getFloat(); // enddist
					acs.f2 = context.getFloat(); // height modifier
				} else {
					ScriptWarning << "unexpected command: " << options << ' ' << command;
				}
				
			}
		}
		
		string text = context.getWord();
		
		if(text == "killall") {
			
			if(!options.empty()) {
				ScriptWarning << "unexpected options: " << options << " killall";
			}
			
			ARX_SPEECH_Reset();
			return Success;
		}
		
		string data = loadUnlocalized(context.getStringVar(text));
		
		DebugScript(' ' << options << ' ' << data); // TODO debug more
		
		if(data.empty()) {
			ARX_SPEECH_ClearIOSpeech(io);
			return Success;
		}
		
		
		if(!CINEMASCOPE) {
			voixoff |= ARX_SPEECH_FLAG_NOTEXT;
		}
		
		long speechnum;
		if(player) {
			speechnum = ARX_SPEECH_AddSpeech(entities.player(), data, mood, voixoff);
		} else {
			speechnum = ARX_SPEECH_AddSpeech(io, data, mood, voixoff);
		}
		if(speechnum < 0) {
			return Failed;
		}
		
		size_t onspeechend = context.skipCommand();
		
		if(onspeechend != (size_t)-1) {
			aspeech[speechnum].scrpos = onspeechend;
			aspeech[speechnum].es = context.getScript();
			aspeech[speechnum].ioscript = io;
			if(unbreakable) {
				aspeech[speechnum].flags |= ARX_SPEECH_FLAG_UNBREAKABLE;
			}
			aspeech[speechnum].cine = acs;
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

}

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
