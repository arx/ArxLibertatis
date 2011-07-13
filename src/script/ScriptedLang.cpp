
#include "script/ScriptedLang.h"

#include "ai/Paths.h"
#include "core/GameTime.h"
#include "game/Inventory.h"
#include "graphics/Math.h"
#include "platform/String.h"
#include "io/Logger.h"
#include "scene/Interactive.h"
#include "script/ScriptEvent.h"
#include "script/ScriptUtils.h"

using std::string;

extern SCRIPT_EVENT AS_EVENT[];

namespace script {

namespace {

class NopCommand : public Command {
	
public:
	
	NopCommand() : Command("nop") { }
	
	Result execute(Context & context) {
		
		ARX_UNUSED(context);
		
		DebugScript("");
		
		return Success;
	}
	
};

class GotoCommand : public Command {
	
	bool sub;
	
public:
	
	GotoCommand(string command, bool _sub = false) : Command(command), sub(_sub) { }
	
	Result execute(Context & context) {
		
		string label = context.getLowercase();
		
		DebugScript(' ' << label);
		
		size_t pos = context.skipCommand();
		if(pos != (size_t)-1) {
			ScriptWarning << "unexpected text at " << pos;
		}
		
		if(!context.jumpToLabel(label)) {
			if(label != "main_alert") {// TODO(broken-scripts)
				ScriptError << "error jumping to label \"" << label << '"';
			}
			return AbortError;
		}
		
		return Jumped;
	}
	
};

class AbortCommand : public Command {
	
	Result result;
	
public:
	
	AbortCommand(string command, Result _result) : Command(command), result(_result) { }
	
	Result execute(Context & context) {
		
		ARX_UNUSED(context);
		
		DebugScript("");
		
		return result;
	}
	
};

class RandomCommand : public Command {
	
public:
	
	RandomCommand() : Command("random") { }
	
	Result execute(Context & context) {
		
		float chance = clamp(context.getFloat(), 0.f, 100.f);
		
		DebugScript(' ' << chance);
		
		float t = rnd() * 100.f;
		if(chance < t) {
			context.skipStatement();
		}
		
		return Jumped;
	}
	
};

class ReturnCommand : public Command {
	
public:
	
	ReturnCommand() : Command("return") { }
	
	Result execute(Context & context) {
		
		DebugScript("");
		
		if(!context.returnToCaller()) {
			ScriptError << "return failed";
			return AbortError;
		}
		
		return Jumped;
	}
	
};

class SetMainEventCommand : public Command {
	
public:
	
	SetMainEventCommand(const string & command) : Command(command, ANY_IO) { }
	
	Result execute(Context & context) {
		
		string event = context.getLowercase();
		
		DebugScript(' ' << event);
		
		ARX_SCRIPT_SetMainEvent(context.getIO(), event);
		
		return Success;
	}
	
};

class StartStopTimerCommand : public Command {
	
	const bool start;
	
public:
	
	StartStopTimerCommand(const string & command, bool _start) : Command(command), start(_start) { }
	
	Result execute(Context & context) {
		
		string timer = context.getLowercase();
		
		DebugScript(' ' << timer);
		
		long t;
		if(timer == "timer1") {
			t = 0;
		} else if(timer == "timer2") {
			t = 1;
		} else if(timer == "timer3") {
			t = 2;
		} else if(timer == "timer4") {
			t = 3;
		} else {
			ScriptWarning << "invalid timer: " << timer;
			return Failed;
		}
		
		EERIE_SCRIPT * script = context.getMaster();
		if(start) {
			script->timers[t] = ARXTimeUL();
			if(script->timers[t] == 0) {
				script->timers[t] = 1;
			}
		} else {
			script->timers[t] = 0;
		}
		
		return Success;
	}
	
};

class SendEventCommand : public Command {
	
	enum SendTarget {
		SEND_NPC = 1,
		SEND_ITEM = 2,
		SEND_FIX = 4
	};
	DECLARE_FLAGS(SendTarget, SendTargets)
	
public:
	
	SendEventCommand() : Command("sendevent") { }
	
	Result execute(Context & context) {
		
		SendTargets sendto = 0;
		bool radius = false;
		bool zone = false;
		bool group = false;
		HandleFlags("gfinrz") {
			group = (flg & flag('g'));
			sendto |= (flg & flag('f')) ? SEND_FIX : (SendTargets)0;
			sendto |= (flg & flag('i')) ? SEND_ITEM : (SendTargets)0;
			sendto |= (flg & flag('n')) ? SEND_NPC : (SendTargets)0;
			radius = (flg & flag('r'));
			zone = (flg & flag('z'));
		}
		if(!sendto) {
			sendto = SEND_NPC;
		}
		
		string groupname;
		if(group) {
			groupname = toLowercase(context.getStringVar(context.getLowercase()));
		}
		
		string event = context.getLowercase();
		
		string zonename;
		if(zone) {
			zonename = toLowercase(context.getStringVar(context.getLowercase()));
		}
		
		float rad = 0.f;
		if(radius) {
			rad = context.getFloat();
		}
		
		string target;
		if(!group && !zone && !radius) {
			target = context.getLowercase();
			
			// work around broken scripts
			for(size_t i = 0; i < SM_MAXCMD; i++) {
				if(!strcasecmp(target, AS_EVENT[i].name.c_str() + 3)) { // TODO(case-sensitive) strcasecmp
					std::swap(target, event);
					break;
				}
			}
		}
		
		string params = context.getWord();
		
		DebugScript(' ' << options << " g=\"" << groupname << "\" e=\"" << event << "\" r=" << rad << " t=\"" << target << "\" p=\"" << params << '"');
		
		INTERACTIVE_OBJ * oes = EVENT_SENDER;
		EVENT_SENDER = context.getIO();
		
		INTERACTIVE_OBJ * io = context.getIO();
		
		if(radius) { // SEND EVENT TO ALL OBJECTS IN A RADIUS
			
			for(long l = 0 ; l < inter.nbmax ; l++) {
				
				if(!inter.iobj[l] || inter.iobj[l] == io || (inter.iobj[l]->ioflags & (IO_CAMERA|IO_MARKER))) {
					continue;
				}
				
				if(group && !IsIOGroup(inter.iobj[l], groupname)) {
					continue;
				}
				
				if(((sendto & SEND_NPC) && (inter.iobj[l]->ioflags & IO_NPC))
				   || ((sendto & SEND_FIX) && (inter.iobj[l]->ioflags & IO_FIX))
				   || ((sendto & SEND_ITEM) && (inter.iobj[l]->ioflags & IO_ITEM))) {
					
					Vec3f _pos, _pos2;
					GetItemWorldPosition(inter.iobj[l], &_pos);
					GetItemWorldPosition(io, &_pos2);
					
					if(distSqr(_pos, _pos2) <= square(rad)) {
						io->stat_sent++;
						Stack_SendIOScriptEvent(inter.iobj[l], SM_NULL, params, event);
					}
				}
			}
			
		} else if(zone) { // SEND EVENT TO ALL OBJECTS IN A ZONE
			
			ARX_PATH * ap = ARX_PATH_GetAddressByName(zonename);
			if(!ap) {
				ScriptWarning << "unknown zone: " << zonename;
				EVENT_SENDER = oes;
				return Failed;
			}
			
			for(long l = 0; l < inter.nbmax; l++) {
				
				if(!inter.iobj[l] || (inter.iobj[l]->ioflags & (IO_CAMERA|IO_MARKER))) {
					continue;
				}
				
				if(group && !IsIOGroup(inter.iobj[l], groupname)) {
					continue;
				}
				
				if(((sendto & SEND_NPC) && (inter.iobj[l]->ioflags & IO_NPC))
				   || ((sendto & SEND_FIX) && (inter.iobj[l]->ioflags & IO_FIX))
				   || ((sendto & SEND_ITEM) && (inter.iobj[l]->ioflags & IO_ITEM))) {
					
					Vec3f _pos;
					GetItemWorldPosition(inter.iobj[l], &_pos);
					
					if(ARX_PATH_IsPosInZone(ap, _pos.x, _pos.y, _pos.z)) {
						io->stat_sent++;
						Stack_SendIOScriptEvent(inter.iobj[l], SM_NULL, params, event);
					}
				}
			}
			
		} else if(group) { // sends an event to all members of a group
			
			for(long l = 0; l < inter.nbmax; l++) {
				
				if(!inter.iobj[l] || inter.iobj[l] == io || !IsIOGroup(inter.iobj[l], groupname)) {
					continue;
				}
				
				io->stat_sent++;
				Stack_SendIOScriptEvent(inter.iobj[l], SM_NULL, params, event);
			}
			
		} else { // single object event
			
			long t = GetTargetByNameTarget(target);
			if(t == -2) {
				t = GetInterNum(io);
			}
			
			if(!ValidIONum(t)) {
				ScriptWarning << "invalid target: " << target;
				EVENT_SENDER = oes;
				return Failed;
			}
			
			io->stat_sent++;
			Stack_SendIOScriptEvent(inter.iobj[t], SM_NULL, params, event);
		}
		
		EVENT_SENDER = oes;
		
		return Success;
	}
	
};

class SetCommand : public Command {
	
public:
	
	SetCommand() : Command("set") { }
	
	Result execute(Context & context) {
		
		HandleFlags("a") {
			if(flg & flag('a')) {
				ScriptWarning << "broken 'set -a' script command used";
			}
		}
		
		string var = context.getLowercase();
		string val = context.getWord();
		
		DebugScript(' ' << var << " \"" << val << '"');
		
		if(var.empty()) {
			ScriptWarning << "missing var name";
			return Failed;
		}
		
		EERIE_SCRIPT & es = *context.getMaster();
		
		switch(var[0]) {
			
			case '$': { // global text
				string v = context.getStringVar(val);
				SCRIPT_VAR * sv = SETVarValueText(svar, NB_GLOBALS, var, v);
				if(!sv) {
					ScriptWarning << "unable to set var " << var << " to \"" << v << '"';
					return Failed;
				}
				sv->type = TYPE_G_TEXT;
				break;
			}
			
			case '\xA3': { // local text
				string v = context.getStringVar(val);
				SCRIPT_VAR * sv = SETVarValueText(es.lvar, es.nblvar, var, v);
				if(!sv) {
					ScriptWarning << "unable to set var " << var << " to \"" << v << '"';
					return Failed;
				}
				sv->type = TYPE_L_TEXT;
				break;
			}
			
			case '#': { // global long
				long v = (long)context.getFloatVar(val);
				SCRIPT_VAR * sv = SETVarValueLong(svar, NB_GLOBALS, var, v);
				if(!sv) {
					ScriptWarning << "unable to set var " << var << " to \"" << v << '"';
					return Failed;
				}
				sv->type = TYPE_G_LONG;
				break;
			}
			
			case '\xA7': { // local long
				long v = (long)context.getFloatVar(val);
				SCRIPT_VAR * sv = SETVarValueLong(es.lvar, es.nblvar, var, v);
				if(!sv) {
					ScriptWarning << "unable to set var " << var << " to \"" << v << '"';
					return Failed;
				}
				sv->type = TYPE_L_LONG;
				break;
			}
			
			case '&': { // global float
				float v = context.getFloatVar(val);
				SCRIPT_VAR * sv = SETVarValueFloat(svar, NB_GLOBALS, var, v);
				if(!sv) {
					ScriptWarning << "unable to set var " << var << " to \"" << v << '"';
					return Failed;
				}
				sv->type = TYPE_G_FLOAT;
				break;
			}
			
			case '@': { // local float
				float v = context.getFloatVar(val);
				SCRIPT_VAR * sv = SETVarValueFloat(es.lvar, es.nblvar, var, v);
				if(!sv) {
					ScriptWarning << "unable to set var " << var << " to \"" << v << '"';
					return Failed;
				}
				sv->type = TYPE_L_FLOAT;
				break;
			}
			
			default: {
				ScriptWarning << "unknown variable type: " << var;
				return Failed;
			}
			
		}
		
		return Success;
	}
	
};

class SetEventCommand : public Command {
	
	typedef std::map<string, DisabledEvent> Events;
	Events events;
	
public:
	
	SetEventCommand() : Command("setevent") {
		events["collide_npc"] = DISABLE_COLLIDE_NPC;
		events["chat"] = DISABLE_CHAT;
		events["hit"] = DISABLE_HIT;
		events["inventory2_open"] = DISABLE_INVENTORY2_OPEN;
		events["detectplayer"] = DISABLE_DETECT;
		events["hear"] = DISABLE_HEAR;
		events["aggression"] = DISABLE_AGGRESSION;
		events["main"] = DISABLE_MAIN;
		events["cursormode"] = DISABLE_CURSORMODE;
		events["explorationmode"] = DISABLE_EXPLORATIONMODE;
	}
	
	Result execute(Context & context) {
		
		string name = context.getLowercase();
		bool enable = context.getBool();
		
		DebugScript(' ' << name << ' ' << enable);
		
		Events::const_iterator it = events.find(name);
		if(it == events.end()) {
			ScriptWarning << "unknown event: " << name;
			return Failed;
		}
		
		if(enable) {
			context.getMaster()->allowevents &= ~it->second;
		} else {
			context.getMaster()->allowevents |= it->second;
		}
		
		return Success;
	}
	
};

}

void setupScriptedLang() {
	
	ScriptEvent::registerCommand(new NopCommand);
	ScriptEvent::registerCommand(new GotoCommand("goto"));
	ScriptEvent::registerCommand(new GotoCommand("gosub", true));
	ScriptEvent::registerCommand(new AbortCommand("accept", Command::AbortAccept));
	ScriptEvent::registerCommand(new AbortCommand("refuse", Command::AbortRefuse));
	ScriptEvent::registerCommand(new RandomCommand);
	ScriptEvent::registerCommand(new ReturnCommand);
	ScriptEvent::registerCommand(new SetMainEventCommand("setstatus"));
	ScriptEvent::registerCommand(new SetMainEventCommand("setmainevent"));
	ScriptEvent::registerCommand(new StartStopTimerCommand("starttimer", true));
	ScriptEvent::registerCommand(new StartStopTimerCommand("stoptimer", false));
	ScriptEvent::registerCommand(new SendEventCommand);
	ScriptEvent::registerCommand(new SetCommand);
	ScriptEvent::registerCommand(new SetEventCommand);
	
}

} // namespace script
