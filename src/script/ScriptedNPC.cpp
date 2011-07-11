
#include "script/ScriptedNPC.h"

#include "game/NPC.h"
#include "graphics/Math.h"
#include "graphics/data/Mesh.h"
#include "gui/Speech.h"
#include "gui/Interface.h"
#include "io/Logger.h"
#include "io/FilePath.h"
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
		} else if(command != "none") {
			LogWarning << "unexpected command: behavior " << options << " \"" << command << '"';
		}
		
		LogDebug << "behavior " << options << " \"" << command << "\" " << behavior_param;
		
		ARX_CHECK_LONG(behavior_param);
		ARX_NPC_Behaviour_Change(io, behavior, static_cast<long>(behavior_param));
		
		return Success;
	}
	
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
	
};

class SetBloodCommand : public Command {
	
public:
	
	SetBloodCommand() : Command("setblood", IO_NPC) { }
	
	Result execute(Context & context) {
		
		float r = context.getFloat();
		float g = context.getFloat();
		float b = context.getFloat();
		
		LogDebug << "setblood " << r << ' ' << g << ' ' << b;
		
		context.getIO()->_npcdata->blood_color = Color3f(r, g, b).to<u8>();
		
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
		
		LogDebug << "setspeakpitch " << pitch;
		
		context.getIO()->_npcdata->speakpitch = pitch;
		
		return Success;
	}
	
};

class SetSpeedCommand : public Command {
	
public:
	
	SetSpeedCommand() : Command("setspeed", ANY_IO) { }
	
	Result execute(Context & context) {
		
		float speed = clamp(context.getFloat(), 0.f, 10.f);
		
		LogDebug << "setspeed " << speed;
		
		context.getIO()->basespeed = speed;
		
		return Success;
	}
	
};

class SetStareFactorCommand : public Command {
	
public:
	
	SetStareFactorCommand() : Command("setstarefactor", IO_NPC) { }
	
	Result execute(Context & context) {
		
		float stare_factor = context.getFloat();
		
		LogDebug << "setstarefactor " << stare_factor;
		
		context.getIO()->_npcdata->stare_factor = stare_factor;
		
		return Success;
	}
	
};

class SetNPCStatCommand : public Command {
	
public:
	
	SetNPCStatCommand() : Command("setnpcstat", IO_NPC) { }
	
	Result execute(Context & context) {
		
		string stat = context.getLowercase();
		float value = context.getFloat();
		
		LogDebug << "setnpcstat " << stat << ' ' << value;
		
		if(!ARX_NPC_SetStat(*context.getIO(), stat, value)) {
			LogWarning << "unknown stat name: setnpcstat " << stat << ' ' << value;
			return Failed;
		}
		
		return Success;
	}
	
};

class SetXPValueCommand : public Command {
	
public:
	
	SetXPValueCommand() : Command("setxpvalue", IO_NPC) { }
	
	Result execute(Context & context) {
		
		float xpvalue = context.getFloat();
		if(xpvalue < 0) {
			xpvalue = 0;
		}
		
		LogDebug << "setxpvalue " << xpvalue;
		
		context.getIO()->_npcdata->xpvalue = (long)xpvalue;
		
		return Success;
	}
	
};

class SetMoveModeCommand : public Command {
	
public:
	
	SetMoveModeCommand() : Command("setmovemode", IO_NPC) { }
	
	Result execute(Context & context) {
		
		string mode = context.getLowercase();
		
		LogDebug << "setmovemode " << mode;
		
		INTERACTIVE_OBJ * io = context.getIO();
		if(mode == "walk") {
			ARX_NPC_ChangeMoveMode(io, WALKMODE);
		} else if(mode == "run") {
			ARX_NPC_ChangeMoveMode(io, RUNMODE);
		} else if(mode == "none") {
			ARX_NPC_ChangeMoveMode(io, NOMOVEMODE);
		} else if(mode == "sneak") {
			ARX_NPC_ChangeMoveMode(io, SNEAKMODE);
		} else {
			LogWarning << "unexpected move mode: " << mode;
			return Failed;
		}
		
		return Success;
	}
	
};

class SetWeaponCommand : public Command {
	
public:
	
	SetWeaponCommand() : Command("setweapon", IO_NPC) { }
	
	Result execute(Context & context) {
		
		INTERACTIVE_OBJ * io = context.getIO();
		
		io->GameFlags &= ~GFLAG_HIDEWEAPON;
		string options = context.getFlags();
		if(!options.empty()) {
			u64 flg = flags(options);
			if(flg & flag('h')) {
				io->GameFlags |= GFLAG_HIDEWEAPON;
			} else if(!flg || (flg & ~flag('h'))) {
				LogWarning << "unexpected flags: setequip " << options;
			}
		}
		
		string weapon = loadPath(context.getLowercase());
		
		LogDebug << "setweapon " << options << ' ' << weapon;
		
		strcpy(io->_npcdata->weaponname, weapon.c_str());
		Prepare_SetWeapon(io, weapon);
		
		return Success;
	}
	
};

class SetLifeCommand : public Command {
	
public:
	
	SetLifeCommand() : Command("setlife", IO_NPC) { }
	
	Result execute(Context & context) {
		
		float life = context.getFloat();
		
		LogDebug << "setlife " << life;
		
		context.getIO()->_npcdata->maxlife = context.getIO()->_npcdata->life = life;
		
		return Success;
	}
	
};

class SetTargetCommand : public Command {
	
public:
	
	SetTargetCommand() : Command("settarget", ANY_IO) { }
	
	Result execute(Context & context) {
		
		INTERACTIVE_OBJ * io = context.getIO();
		if(io->ioflags & IO_NPC) {
			io->_npcdata->pathfind.flags &= ~(PATHFIND_ALWAYS|PATHFIND_ONCE|PATHFIND_NO_UPDATE);
		}
		
		string options = context.getFlags();
		if(!options.empty()) {
			u64 flg = flags(options);
			if(io->ioflags & IO_NPC) {
				if(flg & flag('s')) {
					io->_npcdata->pathfind.flags |= PATHFIND_ONCE;
				}
				if(flg & flag('a')) {
					io->_npcdata->pathfind.flags |= PATHFIND_ALWAYS;
				}
				if(flg & flag('n')) {
					io->_npcdata->pathfind.flags |= PATHFIND_NO_UPDATE;
				}
			}
			if(!flg || (flg & ~flags("san"))) {
				LogWarning << "unexpected flags: settarget " << options;
			}
		}
		
		long old_target = -12;
		if(io->ioflags & IO_NPC) {
			if(io->_npcdata->reachedtarget) {
				old_target = io->targetinfo;
			}
			if(io->_npcdata->behavior & (BEHAVIOUR_FLEE|BEHAVIOUR_WANDER_AROUND)) {
				old_target = -12;
			}
		}
		
		string target = context.getLowercase();
		if(target == "object") {
			target = context.getLowercase();
		}
		target = toLowercase(context.getStringVar(target));
		long t = GetTargetByNameTarget(target);
		if(t == -2) {
			t = GetInterNum(io);
		}
		
		LogDebug << "settarget " << options << ' ' << target;
		
		if(io->ioflags & IO_CAMERA) {
			io->_camdata->cam.translatetarget = Vec3f::ZERO;
		}
		
		if(ValidIONum(t)) {
			io->targetinfo = t;
			GetTargetPos(io);
		}
		
		if(target == "path") {
			io->targetinfo = TARGET_PATH;
			GetTargetPos(io);
		} else if(target == "none") {
			io->targetinfo = TARGET_NONE;
		}
		
		if(old_target != t) {
			if(io->ioflags & IO_NPC) {
				io->_npcdata->reachedtarget = 0;
			}
			ARX_NPC_LaunchPathfind(io, t);
		}
		
		return Success;
	}
	
};

class ForceDeathCommand : public Command {
	
public:
	
	ForceDeathCommand() : Command("forcedeath", ANY_IO) { }
	
	Result execute(Context & context) {
		
		string target = context.getLowercase();
		
		LogDebug << "forcedeath " << target;
		
		long t = GetTargetByNameTarget(target);
		if(t == -2) {
			t = GetInterNum(context.getIO());
		}
		if(!t) {
			LogWarning << "forcedeath: unknown target: " << target;
			return Failed;
		}
		
		ARX_DAMAGES_ForceDeath(inter.iobj[t], context.getIO());
		
		return Success;
	}
	
};

class PathfindCommand : public Command {
	
public:
	
	PathfindCommand() : Command("pathfind", IO_NPC) { }
	
	Result execute(Context & context) {
		
		string target = context.getLowercase();
		
		LogDebug << "pathfind " << target;
		
		long t = GetTargetByNameTarget(target);
		ARX_NPC_LaunchPathfind(context.getIO(), t);
		
		return Success;
	}
	
};

}

void setupScriptedNPC() {
	
	ScriptEvent::registerCommand(new BehaviourCommand);
	ScriptEvent::registerCommand(new ReviveCommand);
	ScriptEvent::registerCommand(new SpellcastCommand);
	ScriptEvent::registerCommand(new SpeakCommand);
	ScriptEvent::registerCommand(new SetDetectCommand);
	ScriptEvent::registerCommand(new SetBloodCommand);
	ScriptEvent::registerCommand(new SetSpeakPitchCommand);
	ScriptEvent::registerCommand(new SetSpeedCommand);
	ScriptEvent::registerCommand(new SetStareFactorCommand);
	ScriptEvent::registerCommand(new SetNPCStatCommand);
	ScriptEvent::registerCommand(new SetXPValueCommand);
	ScriptEvent::registerCommand(new SetMoveModeCommand);
	ScriptEvent::registerCommand(new SetWeaponCommand);
	ScriptEvent::registerCommand(new SetLifeCommand);
	ScriptEvent::registerCommand(new SetTargetCommand);
	ScriptEvent::registerCommand(new ForceDeathCommand);
	ScriptEvent::registerCommand(new PathfindCommand);
	
}

} // namespace script
