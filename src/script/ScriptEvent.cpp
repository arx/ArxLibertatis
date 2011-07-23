/*
 * ScriptEvent.cpp
 *
 *  Created on: Jan 31, 2011
 *      Author: bmonkey
 */

#include "script/ScriptEvent.h"

#include <cstdio>

#include "core/GameTime.h"
#include "core/Core.h"

#include "io/Logger.h"

#include "platform/String.h"

#include "script/ScriptUtils.h"
#include "script/ScriptedAnimation.h"
#include "script/ScriptedCamera.h"
#include "script/ScriptedControl.h"
#include "script/ScriptedConversation.h"
#include "script/ScriptedInterface.h"
#include "script/ScriptedInventory.h"
#include "script/ScriptedIOControl.h"
#include "script/ScriptedIOProperties.h"
#include "script/ScriptedItem.h"
#include "script/ScriptedLang.h"
#include "script/ScriptedNPC.h"
#include "script/ScriptedPlayer.h"
#include "script/ScriptedVariable.h"

using std::max;
using std::min;
using std::string;

extern float g_TimeStartCinemascope;

long ScriptEvent::totalCount = 0;

SCRIPT_EVENT AS_EVENT[] = {
	std::string("on null"),
	std::string("on init"),
	std::string("on inventoryin"),
	std::string("on inventoryout"),
	std::string("on inventoryuse"),
	std::string("on sceneuse"),
	std::string("on equipin"),
	std::string("on equipout"),
	std::string("on main"),
	std::string("on reset"),
	std::string("on chat"),
	std::string("on action"),
	std::string("on dead"),
	std::string("on reachedtarget"),
	std::string("on fight"),
	std::string("on flee"),
	std::string("on hit"),
	std::string("on die"),
	std::string("on losttarget"),
	std::string("on treatin"),
	std::string("on treatout"),
	std::string("on move"),
	std::string("on detectplayer"),
	std::string("on undetectplayer"),
	std::string("on combine"),
	std::string("on npc_follow"),
	std::string("on npc_fight"),
	std::string("on npc_stay"),
	std::string("on inventory2_open"),
	std::string("on inventory2_close"),
	std::string("on custom"),
	std::string("on enter_zone"),
	std::string("on leave_zone"),
	std::string("on initend") ,
	std::string("on clicked") ,
	std::string("on insidezone"),
	std::string("on controlledzone_inside"),
	std::string("on leavezone"),
	std::string("on controlledzone_leave"),
	std::string("on enterzone"),
	std::string("on controlledzone_enter"),
	std::string("on load"),
	std::string("on spellcast"),
	std::string("on reload"),
	std::string("on collide_door"),
	std::string("on ouch"),
	std::string("on hear"),
	std::string("on summoned"),
	std::string("on spellend"),
	std::string("on spelldecision"),
	std::string("on strike"),
	std::string("on collision_error"),
	std::string("on waypoint"),
	std::string("on pathend"),
	std::string("on critical"),
	std::string("on collide_npc"),
	std::string("on backstab"),
	std::string("on aggression"),
	std::string("on collision_error_detail"),
	std::string("on game_ready"),
	std::string("on cine_end"),
	std::string("on key_pressed"),
	std::string("on controls_on"),
	std::string("on controls_off"),
	std::string("on pathfinder_failure"),
	std::string("on pathfinder_success"),
	std::string("on trap_disarmed"),
	std::string("on book_open"),
	std::string("on book_close"),
	std::string("on identify"),
	std::string("on break"),
	std::string("on steal"),
	std::string("on collide_field"),
	std::string("on cursormode"),
	std::string("on explorationmode"),
	std::string("") // TODO is this really needed?
};

void ARX_SCRIPT_ComputeShortcuts(EERIE_SCRIPT& es)
{
	long nb = min((long)MAX_SHORTCUT, (long)SM_MAXCMD);

	for (long j = 1; j < nb; j++) {
		es.shortcut[j] = FindScriptPos(&es, AS_EVENT[j].name);
	}
}

ScriptEvent::ScriptEvent() {
	// TODO Auto-generated constructor stub

}

ScriptEvent::~ScriptEvent() {
	// TODO Auto-generated destructor stub
}

static bool checkInteractiveObject(INTERACTIVE_OBJ * io, ScriptMessage msg, ScriptResult & ret) {
	
	io->stat_count++;
	
	if((io->GameFlags & GFLAG_MEGAHIDE) && msg != SM_RELOAD) {
		ret = ACCEPT;
		return true;
	}
	
	if(io->show == SHOW_FLAG_DESTROYED) {
		ret = ACCEPT;
		return true;
	}
	
	if(io->ioflags & IO_FREEZESCRIPT) {
		ret = (msg == SM_LOAD) ? ACCEPT : REFUSE;
		return true;
	}
	
	if(io->ioflags & IO_NPC
	  && io->_npcdata->life <= 0.f
	  && msg != SM_DEAD
	  && msg != SM_DIE
	  && msg != SM_EXECUTELINE
	  && msg != SM_RELOAD
	  && msg != SM_INVENTORY2_OPEN
	  && msg != SM_INVENTORY2_CLOSE) {
		ret = ACCEPT;
		return true;
	}
	
	//change weapons if you break
	if((io->ioflags & IO_FIX || io->ioflags & IO_ITEM) && msg == SM_BREAK) {
		ManageCasseDArme(io);
	}
	
	return false;
}

extern long PauseScript;

namespace script {

namespace {

class ObsoleteCommand : public Command {
	
private:
	
	size_t nargs;
	
public:
	
	ObsoleteCommand(const string & command, size_t _nargs = 0) : Command(command), nargs(_nargs) { }
	
	Result execute(Context & context) {
		
		for(size_t i = 0; i < nargs; i++) {
			context.skipWord();
		}
		
		LogWarning << "obsolete command: " << getName();
		
		return Failed;
	}
	
	~ObsoleteCommand() { }
	
};

}

} // namespace script

#define ScriptEventWarning Logger(__FILE__,__LINE__, isSuppressed(context, word) ? Logger::Debug : Logger::Warning) << ScriptContextPrefix(context) << (((size_t)msg < sizeof(AS_EVENT)/sizeof(*AS_EVENT) - 1 && msg != SM_NULL) ? AS_EVENT[msg].name : "on " + evname) << ": "

using namespace script; // TODO(script-parser) remove once everythng has been moved to the script namespace

static const char * toString(ScriptResult ret) {
	switch(ret) {
		case ACCEPT: return "accept";
		case REFUSE: return "refuse";
		case BIGERROR: return "error";
		default: arx_assert(false); return NULL;
	}
}

ScriptResult ScriptEvent::send(EERIE_SCRIPT * es, ScriptMessage msg, const std::string& params, INTERACTIVE_OBJ * io, const std::string& evname, long info) {
	
	ScriptResult ret = ACCEPT;
	string eventname;
	long pos;
	
	totalCount++;
	
	if(io && checkInteractiveObject(io, msg, ret)) {
		return ret;
	}

	if ((EDITMODE || PauseScript)
			&& msg != SM_LOAD
			&& msg != SM_INIT
			&& msg != SM_INITEND) {
		return ACCEPT;
	}
	
	// Retrieves in esss script pointer to script holding variables.
	EERIE_SCRIPT * esss = (EERIE_SCRIPT *)es->master;
	if(esss == NULL) {
		esss = es;
	}
	
	// Finds script position to execute code...
	if (!evname.empty()) {
		eventname = "on " + evname;
		pos = FindScriptPos(es, eventname);
	} else {
		if (msg == SM_EXECUTELINE) {
			pos = info;
		} else {
			switch(msg) {
				case SM_COLLIDE_NPC:
					if (esss->allowevents & DISABLE_COLLIDE_NPC) return REFUSE;
					break;
				case SM_CHAT:
					if (esss->allowevents & DISABLE_CHAT) return REFUSE;
					break;
				case SM_HIT:
					if (esss->allowevents & DISABLE_HIT) return REFUSE;
					break;
				case SM_INVENTORY2_OPEN:
					if (esss->allowevents & DISABLE_INVENTORY2_OPEN) return REFUSE;
					break;
				case SM_HEAR:
					if (esss->allowevents & DISABLE_HEAR) return REFUSE;
					break;
				case SM_UNDETECTPLAYER:
				case SM_DETECTPLAYER:
					if (esss->allowevents & DISABLE_DETECT) return REFUSE;
					break;
				case SM_AGGRESSION:
					if (esss->allowevents & DISABLE_AGGRESSION) return REFUSE;
					break;
				case SM_MAIN:
					if (esss->allowevents & DISABLE_MAIN) return REFUSE;
					break;
				case SM_CURSORMODE:
					if (esss->allowevents & DISABLE_CURSORMODE) return REFUSE;
					break;
				case SM_EXPLORATIONMODE:
					if (esss->allowevents & DISABLE_EXPLORATIONMODE) return REFUSE;
					break;
				case SM_KEY_PRESSED: {
					float dwCurrTime = ARX_TIME_Get();
					if ((dwCurrTime - g_TimeStartCinemascope) < 3000) {
						LogDebug << "refusing SM_KEY_PRESSED";
						return REFUSE;
					}
					break;
				}
				default: break;
			}

			if (msg < (long)MAX_SHORTCUT) {
				pos = es->shortcut[msg];
			} else {
				
				arx_assert(msg != SM_EXECUTELINE && evname.empty());
				
				if(msg >= SM_MAXCMD) {
					LogDebug << "unknown message " << msg;
					return ACCEPT;
				}
				
				// TODO will never be reached as MAX_SHORTCUT > SM_MAXCMD
				
				pos = FindScriptPos(es, AS_EVENT[msg].name);
			}
		}
	}

	if (pos <= -1) {
		// TODO very noisy LogDebug << "cannot find event handler";
		return ACCEPT;
	}
	
	
	LogDebug << "--> SendScriptEvent event="
	         << (!evname.empty() ? evname
	            : ((size_t)msg < sizeof(AS_EVENT)/sizeof(*AS_EVENT) - 1) ? AS_EVENT[msg].name.substr(3)
	            : "(none)")
	         << " params=\"" << params << "\""
	         << " io=" << (io ? io->long_name() : "unknown")
	         << (io == NULL ? "" : es == &io->script ? " base" : " overriding")
	         << " pos=" << pos;

	MakeSSEPARAMS(params.c_str());

	if (msg != SM_EXECUTELINE) {
		if (!evname.empty()) {
			pos += eventname.length(); // adding 'ON ' length
		} else {
			pos += AS_EVENT[msg].name.length();
		}
	}
	
	Context context(es, pos, io, msg);
	
	if(msg != SM_EXECUTELINE) {
		string word = context.getCommand();
		if(word != "{") {
			ScriptEventWarning << "--> missing bracket after event, got \"" << word << "\"";
			return ACCEPT;
		}
	}
	
	size_t brackets = 1;
	
	for(;;) {
		
		string word = context.getCommand(msg != SM_EXECUTELINE);
		if(word.empty()) {
			if(msg == SM_EXECUTELINE && context.pos != es->size) {
				arx_assert(es->data[context.pos] == '\n');
				LogDebug << "--> line end";
				return ACCEPT;
			}
			ScriptEventWarning << "--> reached script end without accept / refuse / return";
			return ACCEPT;
		}
		
		// Remove all underscores from the command.
		word.resize(std::remove(word.begin(), word.end(), '_') - word.begin());
		
		Commands::const_iterator it = commands.find(word);
		
		if(it != commands.end()) {
			
			Command & command = *(it->second);
			
			Command::Result res;
			if(command.getIOFlags() && (!io || (command.getIOFlags() != Command::ANY_IO && !(command.getIOFlags() & io->ioflags)))) {
				ScriptEventWarning << "command " << command.getName() << " needs an IO of type " << command.getIOFlags();
				context.skipCommand();
				res = Command::Failed;
			} else {
				res = it->second->execute(context);
			}
			
			if(res == Command::AbortAccept) {
				ret = ACCEPT;
				break;
			} else if(res == Command::AbortRefuse) {
				ret = REFUSE;
				break;
			} else if(res == Command::AbortError) {
				ret =  BIGERROR;
				break;
			} else if(res == Command::Jumped) {
				if(msg == SM_EXECUTELINE) {
					msg = SM_DUMMY;
				}
			}
			
		} else if(!word.compare(0, 2, ">>", 2)) {
			context.skipCommand(); // comments and labels
		} else if(!word.compare(0, 5, "timer", 5)) {
			timerCommand(word.substr(5), context);
		} else if(word == "{") {
			brackets++;
		} else if(word == "}") {
			brackets--;
			if(brackets == 0) {
				ScriptEventWarning << "--> event block ended without accept or refuse!";
				return ACCEPT;
			}
		} else {
			
			ScriptEventWarning << "--> unknown command: " << word;
			
			io->ioflags |= IO_FREEZESCRIPT;
			return REFUSE;
			
		}
		
	}
	
	if(msg == SM_EXECUTELINE) {
		LogDebug << "--> executeline finished: " << toString(ret);
	} else if(evname != "") {
		LogDebug << "--> " << eventname << " event finished: " << toString(ret);
	} else if(msg != SM_DUMMY) {
		LogDebug << "--> " << AS_EVENT[msg].name.substr(3) << " event finished: " << toString(ret);
	} else {
		LogDebug << "--> dummy event finished: " << toString(ret);
	}
	
	return ret;
}

void ScriptEvent::registerCommand(Command * command) {
	
	typedef std::pair<Commands::iterator, bool> Res;
	
	Res res = commands.insert(std::make_pair(command->getName(), command));
	
	if(!res.second) {
		LogError << "duplicate script command name: " + command->getName();
		delete command;
	}
	
}

void ScriptEvent::init() {
	
	initSuppressions();
	
	setupScriptedAnimation();
	setupScriptedCamera();
	setupScriptedControl();
	setupScriptedConversation();
	setupScriptedInterface();
	setupScriptedInventory();
	setupScriptedIOControl();
	setupScriptedIOProperties();
	setupScriptedItem();
	setupScriptedLang();
	setupScriptedNPC();
	setupScriptedPlayer();
	setupScriptedVariable();
	
	registerCommand(new ObsoleteCommand("attachnpctoplayer"));
	registerCommand(new ObsoleteCommand("gmode", 1));
	registerCommand(new ObsoleteCommand("setrighthand", 1));
	registerCommand(new ObsoleteCommand("setlefthand", 1));
	registerCommand(new ObsoleteCommand("setshield", 1));
	registerCommand(new ObsoleteCommand("settwohanded"));
	registerCommand(new ObsoleteCommand("setonehanded"));
	registerCommand(new ObsoleteCommand("say"));
	registerCommand(new ObsoleteCommand("setdetachable", 1));
	registerCommand(new ObsoleteCommand("setstackable", 1));
	registerCommand(new ObsoleteCommand("setinternalname", 1));
	registerCommand(new ObsoleteCommand("detachnpcfromplayer"));
	
	LogInfo << "scripting system initialized with " << commands.size() << " commands";
}

ScriptEvent::Commands ScriptEvent::commands;
