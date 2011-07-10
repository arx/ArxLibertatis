
#include "script/ScriptedNPC.h"

#include "game/NPC.h"
#include "graphics/Math.h"
#include "graphics/data/Mesh.h"
#include "gui/Speech.h"
#include "gui/Interface.h"
#include "io/Logger.h"
#include "platform/String.h"
#include "scene/Interactive.h"
#include "script/ScriptEvent.h"
#include "script/ScriptUtils.h"

using std::string;

extern long FRAME_COUNT;
extern Vec3f LASTCAMPOS;
extern Anglef LASTCAMANGLE;

namespace script {

namespace {

class BehaviourCommand : public Command {
	
public:
	
	BehaviourCommand() : Command("behavior", IO_NPC) { }
	
	Result execute(Context & context) {
		
		string options = context.getFlags();
		
		string command = context.getLowercase();
		
		INTERACTIVE_OBJ * io = context.getIO();
		
		if(options.empty()) {
			if(command == "stack") {
				LogDebug << "behavior " << options << ' ' << command; 
				ARX_NPC_Behaviour_Stack(io);
				return Success;
			} else if(command == "unstack") {
				LogDebug << "behavior " << options << ' ' << command; 
				ARX_NPC_Behaviour_UnStack(io);
				return Success;
			} else if(command == "unstackall") {
				LogDebug << "behavior " << options << ' ' << command; 
				ARX_NPC_Behaviour_Reset(io);
				return Success;
			}
		}
		
		Behaviour behavior = 0;
		
		if(!options.empty()) {
			u64 flg = flags(options);
			if(flg & flag('l')) {
				behavior |= BEHAVIOUR_LOOK_AROUND;
			}
			if(flg & flag('s')) {
				behavior |= BEHAVIOUR_SNEAK;
			}
			if(flg & flag('d')) {
				behavior |= BEHAVIOUR_DISTANT;
			}
			if(flg & flag('m')) {
				behavior |= BEHAVIOUR_MAGIC;
			}
			if(flg & flag('f')) {
				behavior |= BEHAVIOUR_FIGHT;
			}
			if(flg & flag('a')) {
				behavior |= BEHAVIOUR_STARE_AT;
			}
			if(flg & flags("012")) {
				io->_npcdata->tactics = 0;
			}
			if(!flg || (flg & ~flags("lsdmfa012"))) {
				LogWarning << "unexpected flags: behavior " << options;
			}
		}
		
		float behavior_param = 0.f;
		if(command == "go_home") {
			behavior |= BEHAVIOUR_GO_HOME;
		} else if(command == "friendly") {
			io->_npcdata->movemode = NOMOVEMODE;
			behavior |= BEHAVIOUR_FRIENDLY;
		} else if(command == "move_to") {
			io->_npcdata->movemode = WALKMODE;
			behavior |= BEHAVIOUR_MOVE_TO;
		} else if(command == "flee") {
			behavior_param = context.getFloat();
			io->_npcdata->movemode = RUNMODE;
			behavior |= BEHAVIOUR_FLEE;
		} else if(command == "look_for") {
			behavior_param = context.getFloat();
			io->_npcdata->movemode = WALKMODE;
			behavior |= BEHAVIOUR_LOOK_FOR;
		} else if(command == "hide") {
			behavior_param = context.getFloat();
			io->_npcdata->movemode = WALKMODE;
			behavior |= BEHAVIOUR_HIDE;
		} else if(command == "wander_around") {
			behavior_param = context.getFloat();
			io->_npcdata->movemode = WALKMODE;
			behavior |= BEHAVIOUR_WANDER_AROUND;
		} else if(command == "guard") {
			behavior |= BEHAVIOUR_GUARD;
			io->targetinfo = -2;
			io->_npcdata->movemode = NOMOVEMODE;
		} else {
			LogWarning << "unexpected command: behavior " << options << " \"" << command << '"';
		}
		
		LogDebug << "behavior " << options << " \"" << command << "\" " << behavior_param;
		
		ARX_CHECK_LONG(behavior_param);
		ARX_NPC_Behaviour_Change(io, behavior, static_cast<long>(behavior_param));
		
		return Success;
	}
	
	~BehaviourCommand() { }
	
};

class ReviveCommand : public Command {
	
public:
	
	ReviveCommand() : Command("revive", ANY_IO) { }
	
	Result execute(Context & context) {
		
		string options = context.getFlags();
		
		bool init = false;
		
		if(!options.empty()) {
			u64 flg = flags(options);
			if(flg & flag('i')) {
				init = true;
			} else if(!flg || (flg & ~flag('i'))) {
				LogWarning << "unexpected flags: revive " << options;
			}
		}
		
		ARX_NPC_Revive(context.getIO(), init ? 1 : 0);
		
		LogDebug << "revive " << options;
		
		return Success;
	}
	
	~ReviveCommand() { }
	
};

class SpellcastCommand : public Command {
	
public:
	
	SpellcastCommand() : Command("spellcast", ANY_IO) { }
	
	Result execute(Context & context) {
		
		SpellcastFlags spflags = 0;
		long duration = -1;
		bool haveDuration = 0;
		
		string options = context.getFlags();
		if(!options.empty()) {
			u64 flg = flags(options);
			
			if(!flg || (flg & ~flags("kdxmsfz"))) {
				LogWarning << "unexpected flags: spellcast " << options;
			}
			
			if(flg & flag('k')) {
				
				string spellname = context.getLowercase();
				Spell spellid = GetSpellId(spellname);
				
				LogDebug << "spellcast " << options << ' ' << spellname;
				
				long from = GetInterNum(context.getIO());
				if(ValidIONum(from)) {
					long sp = ARX_SPELLS_GetInstanceForThisCaster(spellid, from);
					if(sp >= 0) {
						spells[sp].tolive = 0;
					}
				}
				
				return Success;
			}
			
			if(flg & flag('d')) {
				spflags |= SPELLCAST_FLAG_NOCHECKCANCAST;
				duration = context.getFloat();
				if(duration <= 0) {
					duration = 99999999; // TODO should this be FLT_MAX?
				}
				haveDuration = 1;
			}
			if(flg & flag('x')) {
				spflags |= SPELLCAST_FLAG_NOSOUND;
			}
			if(flg & flag('m')) {
				spflags |= SPELLCAST_FLAG_NOCHECKCANCAST | SPELLCAST_FLAG_NODRAW;
			}
			if(flg & flag('s')) {
				spflags |= SPELLCAST_FLAG_NOCHECKCANCAST | SPELLCAST_FLAG_NOANIM;
			}
			if(flg & flag('f')) {
				spflags |= SPELLCAST_FLAG_NOCHECKCANCAST | SPELLCAST_FLAG_NOMANA;
			}
			if(flg & flag('z')) {
				spflags |= SPELLCAST_FLAG_RESTORE;
			}
		}
		
		long level = clamp(static_cast<long>(context.getFloat()), 1l, 10l);
		if(!haveDuration) {
			duration = 1000 + level * 2000;
		}
		
		string spellname = context.getLowercase();
		Spell spellid = GetSpellId(spellname);
		
		string target = context.getLowercase();
		long t = GetTargetByNameTarget(target);
		if(t <= -1) {
			t = GetInterNum(context.getIO());
		}
		
		if(!ValidIONum(t) || spellid == SPELL_NONE) {
			return Failed;
		}
		
		if(context.getIO() != inter.iobj[0]) {
			spflags |= SPELLCAST_FLAG_NOCHECKCANCAST;
		}
		
		LogDebug << "spellcast " << spellname << ' ' << level << ' ' << target << ' ' << spflags << ' ' << duration; 
		
		TryToCastSpell(context.getIO(), spellid, level, t, spflags, duration);
		
		return Success;
	}
	
	~SpellcastCommand() { }
	
};

class SpeakCommand : public Command {
	
	static void computeACSPos(CinematicSpeech & acs, INTERACTIVE_OBJ * io, long ionum) {
		
		if(io) {
			long id = io->obj->fastaccess.view_attach;
			if(id != -1) {
				acs.pos1 = io->obj->vertexlist3[id].v;
			} else {
				acs.pos1 = io->pos + Vec3f(0.f, io->physics.cyl.height, 0.f);
			}
		}
		
		if(ValidIONum(ionum)) {
			INTERACTIVE_OBJ * ioo = inter.iobj[ionum];
			long id = ioo->obj->fastaccess.view_attach;
			if(id != -1) {
				acs.pos2 = ioo->obj->vertexlist3[id].v;
			} else {
				acs.pos2 = ioo->pos + Vec3f(0.f, ioo->physics.cyl.height, 0.f);
			}
		}
	}
	
	static void parseParams(CinematicSpeech & acs, Context & context, bool player) {
		
		string target = context.getLowercase();
		acs.ionum = GetTargetByNameTarget(target);
		if(acs.ionum == -2) {
			acs.ionum = GetInterNum(context.getIO());
		}
		
		acs.startpos = context.getFloat();
		acs.endpos = context.getFloat();
		
		if(player) {
			computeACSPos(acs, inter.iobj[0], acs.ionum);
		} else {
			computeACSPos(acs, context.getIO(), acs.ionum);
		}
	}
	
public:
	
	SpeakCommand() : Command("speak") { }
	
	Result execute(Context & context) {
		
		CinematicSpeech acs;
		acs.type = ARX_CINE_SPEECH_NONE;
		
		string options = context.getFlags();
		
		string text = context.getLowercase();
		
		if(text == "killall") {
			
			if(!options.empty()) {
				LogWarning << "unexpected options: speak " << options << " killall";
			}
			
			ARX_SPEECH_Reset();
			return Success;
		}
		
		INTERACTIVE_OBJ * io = context.getIO();
		
		bool player = false, unbreakable = false;
		SpeechFlags voixoff = 0;
		AnimationNumber mood = ANIM_TALK_NEUTRAL;
		if(!options.empty()) {
			u64 flg = flags(options);
			if(!flg || (flg & ~flags("tuphaoc"))) {
				LogWarning << "unexpected flags: speak " << options << ' ' << text;
			}
			
			if(flg & flag('t')) {
				voixoff |= ARX_SPEECH_FLAG_NOTEXT;
			}
			if(flg & flag('u')) {
				unbreakable = 1;
			}
			if(flg & flag('p')) {
				player = 1;
			}
			if(flg & flag('h')) {
				mood = ANIM_TALK_HAPPY;
			}
			if(flg & flag('a')) {
				mood = ANIM_TALK_ANGRY;
			}
			
			if(flg & flag('o')) {
				voixoff |= ARX_SPEECH_FLAG_OFFVOICE;
				// Crash when we set speak pitch to 1,
				// Variable use for a division, 0 is not possible
			}
			
			if(flg & flag('c')) {
				
				FRAME_COUNT = 0;
				
				if(text == "keep") {
					acs.type = ARX_CINE_SPEECH_KEEP;
					acs.pos1 = LASTCAMPOS;
					acs.pos2.x = LASTCAMANGLE.a;
					acs.pos2.y = LASTCAMANGLE.b;
					acs.pos2.z = LASTCAMANGLE.g;
					
				} else if(text == "zoom") {
					acs.type = ARX_CINE_SPEECH_ZOOM;
					acs.startangle.a = context.getFloat();
					acs.startangle.b = context.getFloat();
					acs.endangle.a = context.getFloat();
					acs.endangle.b = context.getFloat();
					acs.startpos = context.getFloat();
					acs.endpos = context.getFloat();
					acs.ionum = GetInterNum(io);
					if(player) {
						computeACSPos(acs, inter.iobj[0], acs.ionum);
					} else {
						computeACSPos(acs, io, -1);
					}
					
				} else if(text == "ccctalker_l" || text == "ccctalker_r") {
					acs.type = (text == "ccctalker_r") ? ARX_CINE_SPEECH_CCCTALKER_R : ARX_CINE_SPEECH_CCCTALKER_L;
					parseParams(acs, context, player);
					
				} else if(text == "ccclistener_l" || text == "ccclistener_r") {
					acs.type = (text == "ccclistener_r") ? ARX_CINE_SPEECH_CCCLISTENER_R :  ARX_CINE_SPEECH_CCCLISTENER_L;
					parseParams(acs, context, player);
					
				} else if(text == "side" || text == "side_l" || text == "side_r") {
					acs.type = (text == "side_l") ? ARX_CINE_SPEECH_SIDE_LEFT : ARX_CINE_SPEECH_SIDE;
					parseParams(acs, context, player);
					acs.f0 = context.getFloat(); // startdist
					acs.f1 = context.getFloat(); // enddist
					acs.f2 = context.getFloat(); // height modifier
				} else {
					LogWarning << "unexpected command: speak " << options << ' ' << text;
				}
				
				text = context.getLowercase();
			}
		}
		
		string data = script::loadUnlocalized(toLowercase(context.getStringVar(text)));
		
		LogDebug << "speak " << options << ' ' << data; // TODO debug more
		
		if(data.empty()) {
			ARX_SPEECH_ClearIOSpeech(io);
			return Success;
		}
		
		
		if(!CINEMASCOPE) {
			voixoff |= ARX_SPEECH_FLAG_NOTEXT;
		}
		
		long speechnum;
		if(player) {
			speechnum = ARX_SPEECH_AddSpeech(inter.iobj[0], data, mood, voixoff);
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
	
	~SpeakCommand() { }
	
};

class SetDetectCommand : public Command {
	
public:
	
	SetDetectCommand() : Command("setdetect", IO_NPC) { }
	
	Result execute(Context & context) {
		
		string detectvalue = context.getLowercase();
		
		LogDebug << "setdetect " << detectvalue;
		
		if(detectvalue == "off") {
			context.getIO()->_npcdata->fDetect = -1;
		} else {
			context.getIO()->_npcdata->fDetect = clamp((int)context.getFloatVar(detectvalue), -1, 100);
		}
		
		return Success;
	}
	
	~SetDetectCommand() { }
	
};

}

void setupScriptedNPC() {
	
	ScriptEvent::registerCommand(new BehaviourCommand);
	ScriptEvent::registerCommand(new ReviveCommand);
	ScriptEvent::registerCommand(new SpellcastCommand);
	ScriptEvent::registerCommand(new SpeakCommand);
	ScriptEvent::registerCommand(new SetDetectCommand);
	
}

} // namespace script
