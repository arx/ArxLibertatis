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

#include "script/ScriptEvent.h"

#include <boost/foreach.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "core/GameTime.h"
#include "core/Core.h"

#include "game/Entity.h"
#include "game/NPC.h"

#include "gui/CinematicBorder.h"

#include "io/log/Logger.h"

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


long ScriptEvent::totalCount = 0;

SCRIPT_EVENT AS_EVENT[] = {
	SCRIPT_EVENT("on null"),
	SCRIPT_EVENT("on init"),
	SCRIPT_EVENT("on inventoryin"),
	SCRIPT_EVENT("on inventoryout"),
	SCRIPT_EVENT("on inventoryuse"),
	SCRIPT_EVENT("on sceneuse"),
	SCRIPT_EVENT("on equipin"),
	SCRIPT_EVENT("on equipout"),
	SCRIPT_EVENT("on main"),
	SCRIPT_EVENT("on reset"),
	SCRIPT_EVENT("on chat"),
	SCRIPT_EVENT("on action"),
	SCRIPT_EVENT("on dead"),
	SCRIPT_EVENT("on reachedtarget"),
	SCRIPT_EVENT("on fight"),
	SCRIPT_EVENT("on flee"),
	SCRIPT_EVENT("on hit"),
	SCRIPT_EVENT("on die"),
	SCRIPT_EVENT("on losttarget"),
	SCRIPT_EVENT("on treatin"),
	SCRIPT_EVENT("on treatout"),
	SCRIPT_EVENT("on move"),
	SCRIPT_EVENT("on detectplayer"),
	SCRIPT_EVENT("on undetectplayer"),
	SCRIPT_EVENT("on combine"),
	SCRIPT_EVENT("on npc_follow"),
	SCRIPT_EVENT("on npc_fight"),
	SCRIPT_EVENT("on npc_stay"),
	SCRIPT_EVENT("on inventory2_open"),
	SCRIPT_EVENT("on inventory2_close"),
	SCRIPT_EVENT("on custom"),
	SCRIPT_EVENT("on enter_zone"),
	SCRIPT_EVENT("on leave_zone"),
	SCRIPT_EVENT("on initend") ,
	SCRIPT_EVENT("on clicked") ,
	SCRIPT_EVENT("on insidezone"),
	SCRIPT_EVENT("on controlledzone_inside"),
	SCRIPT_EVENT("on leavezone"),
	SCRIPT_EVENT("on controlledzone_leave"),
	SCRIPT_EVENT("on enterzone"),
	SCRIPT_EVENT("on controlledzone_enter"),
	SCRIPT_EVENT("on load"),
	SCRIPT_EVENT("on spellcast"),
	SCRIPT_EVENT("on reload"),
	SCRIPT_EVENT("on collide_door"),
	SCRIPT_EVENT("on ouch"),
	SCRIPT_EVENT("on hear"),
	SCRIPT_EVENT("on summoned"),
	SCRIPT_EVENT("on spellend"),
	SCRIPT_EVENT("on spelldecision"),
	SCRIPT_EVENT("on strike"),
	SCRIPT_EVENT("on collision_error"),
	SCRIPT_EVENT("on waypoint"),
	SCRIPT_EVENT("on pathend"),
	SCRIPT_EVENT("on critical"),
	SCRIPT_EVENT("on collide_npc"),
	SCRIPT_EVENT("on backstab"),
	SCRIPT_EVENT("on aggression"),
	SCRIPT_EVENT("on collision_error_detail"),
	SCRIPT_EVENT("on game_ready"),
	SCRIPT_EVENT("on cine_end"),
	SCRIPT_EVENT("on key_pressed"),
	SCRIPT_EVENT("on controls_on"),
	SCRIPT_EVENT("on controls_off"),
	SCRIPT_EVENT("on pathfinder_failure"),
	SCRIPT_EVENT("on pathfinder_success"),
	SCRIPT_EVENT("on trap_disarmed"),
	SCRIPT_EVENT("on book_open"),
	SCRIPT_EVENT("on book_close"),
	SCRIPT_EVENT("on identify"),
	SCRIPT_EVENT("on break"),
	SCRIPT_EVENT("on steal"),
	SCRIPT_EVENT("on collide_field"),
	SCRIPT_EVENT("on cursormode"),
	SCRIPT_EVENT("on explorationmode"),
};

void ARX_SCRIPT_ComputeShortcuts(EERIE_SCRIPT & es) {
	for(size_t j = 1; j < SM_MAXCMD; j++) {
		es.shortcut[j] = FindScriptPos(&es, AS_EVENT[j].name);
	}
}

ScriptEvent::ScriptEvent() {
	// TODO Auto-generated constructor stub

}

ScriptEvent::~ScriptEvent() {
	// TODO Auto-generated destructor stub
}

static bool checkInteractiveObject(Entity * io, ScriptMessage msg, ScriptResult & ret) {
	
	io->stat_count++;
	
	if((io->gameFlags & GFLAG_MEGAHIDE) && msg != SM_RELOAD) {
		ret = ACCEPT;
		return true;
	}
	
	arx_assert(io->show != SHOW_FLAG_DESTROYED);
	
	if(io->ioflags & IO_FREEZESCRIPT) {
		ret = (msg == SM_LOAD) ? ACCEPT : REFUSE;
		return true;
	}
	
	if(io->ioflags & IO_NPC
	  && io->_npcdata->lifePool.current <= 0.f
	  && msg != SM_DEAD
	  && msg != SM_DIE
	  && msg != SM_EXECUTELINE
	  && msg != SM_RELOAD
	  && msg != SM_INVENTORY2_OPEN
	  && msg != SM_INVENTORY2_CLOSE) {
		ret = ACCEPT;
		return true;
	}
	
	// Change weapons if an equpped weapon entity breaks
	if(((io->ioflags & IO_FIX) || (io->ioflags & IO_ITEM)) && msg == SM_BREAK) {
		ManageCasseDArme(io);
	}
	
	return false;
}

namespace script {

namespace {

class ObsoleteCommand : public Command {
	
private:
	
	size_t nargs;
	
public:
	
	ObsoleteCommand(const std::string & command, size_t _nargs = 0) : Command(command), nargs(_nargs) { }
	
	Result execute(Context & context) {
		
		for(size_t i = 0; i < nargs; i++) {
			context.skipWord();
		}
		
		ScriptWarning << "obsolete command";
		
		return Failed;
	}
	
	~ObsoleteCommand() { }
	
};

} // anonymous namespace

} // namespace script

#define ScriptEventWarning ARX_LOG(isSuppressed(context, word) ? Logger::Debug : Logger::Warning) << ScriptContextPrefix(context) << event << ": "

#ifdef ARX_DEBUG
static const char * toString(ScriptResult ret) {
	switch(ret) {
		case ACCEPT: return "accept";
		case DESTRUCTIVE: return "destructive";
		case REFUSE: return "refuse";
		case BIGERROR: return "error";
		default: arx_unreachable();
	}
}
#endif

ScriptResult ScriptEvent::send(const EERIE_SCRIPT * es, Entity * sender, Entity * entity,
                               ScriptEventName event, const ScriptParameters & parameters,
                               size_t position) {
	
	ScriptResult ret = ACCEPT;
	
	totalCount++;
	
	arx_assert(entity);
	
	if(checkInteractiveObject(entity, event.getId(), ret)) {
		return ret;
	}
	
	if(!es->valid) {
		return ACCEPT;
	}
	
	if(entity->m_disabledEvents & event.toDisabledEventsMask()) {
		return REFUSE;
	}
	
	// Finds script position to execute code...
	size_t pos = position;
	if(!event.getName().empty()) {
		arx_assert(event.getId() == SM_NULL);
		arx_assert_msg(ScriptEventName::parse(event.getName()).getId() == SM_NULL, "non-canonical event name");
		pos = FindScriptPos(es, "on " + event.getName());
	} else if(event != SM_EXECUTELINE) {
		arx_assert(event.getId() < SM_MAXCMD);
		pos = es->shortcut[event.getId()];
		arx_assert(pos == size_t(-1) || pos <= es->data.size());
	}

	if(pos == size_t(-1)) {
		return ACCEPT;
	}
	
	LogDebug("--> " << event << " params=\"" << parameters << "\"" << " entity=" << entity->idString()
	         << (es == &entity->script ? " base" : " overriding") << " pos=" << pos);
	
	script::Context context(es, pos, sender, entity, event.getId(), parameters);
	
	if(event != SM_EXECUTELINE) {
		std::string word = context.getCommand();
		if(word != "{") {
			ScriptEventWarning << "<-- missing bracket after event, got \"" << word << "\"";
			return ACCEPT;
		}
	}
	
	size_t brackets = 1;
	
	for(;;) {
		
		std::string word = context.getCommand(event != SM_EXECUTELINE);
		if(word.empty()) {
			if(event == SM_EXECUTELINE && context.getPosition() != es->data.size()) {
				arx_assert(es->data[context.getPosition()] == '\n');
				LogDebug("<-- line end");
				return ACCEPT;
			}
			ScriptEventWarning << "<-- reached script end without accept / refuse / return";
			return ACCEPT;
		}
		
		// Remove all underscores from the command.
		word.resize(std::remove(word.begin(), word.end(), '_') - word.begin());
		
		Commands::const_iterator it = commands.find(word);
		
		if(it != commands.end()) {
			
			script::Command & command = *(it->second);
			
			script::Command::Result res;
			if(command.getEntityFlags()
			   && (command.getEntityFlags() != script::Command::AnyEntity
			       && !(command.getEntityFlags() & long(entity->ioflags)))) {
				ScriptEventWarning << "Command " << command.getName() << " needs an IO of type "
				                   << command.getEntityFlags();
				context.skipCommand();
				res = script::Command::Failed;
			} else if(parameters.isPeekOnly()) {
				res = it->second->peek(context);
			} else {
				res = it->second->execute(context);
			}
			
			if(res == script::Command::AbortAccept) {
				ret = ACCEPT;
				break;
			} else if(res == script::Command::AbortRefuse) {
				ret = REFUSE;
				break;
			} else if(res == script::Command::AbortError) {
				ret = BIGERROR;
				break;
			} else if(res == script::Command::AbortDestructive) {
				ret = DESTRUCTIVE;
				break;
			} else if(res == script::Command::Jumped) {
				if(event == SM_EXECUTELINE) {
					event = SM_DUMMY;
				}
				brackets = size_t(-1);
			}
			
		} else if(!word.compare(0, 2, ">>", 2)) {
			context.skipCommand(); // labels
		} else if(!word.compare(0, 5, "timer", 5)) {
			if(parameters.isPeekOnly()) {
				ret = DESTRUCTIVE;
				break;
			}
			script::timerCommand(word.substr(5), context);
		} else if(word == "{") {
			if(brackets != size_t(-1)) {
				brackets++;
			}
		} else if(word == "}") {
			if(brackets != size_t(-1)) {
				brackets--;
				if(brackets == 0) {
					if(isBlockEndSuprressed(context, word)) { // TODO(broken-scripts)
						brackets++;
					} else {
						ScriptEventWarning << "<-- event block ended without accept or refuse!";
						return ACCEPT;
					}
				}
			}
		} else {
			
			if(isBlockEndSuprressed(context, word)) { // TODO(broken-scripts)
				return ACCEPT;
			}
			
			ScriptEventWarning << "<-- unknown command: " << word;
			
			context.skipCommand();
		}
		
	}
	
	LogDebug("<-- " << event << " finished: " << toString(ret));
	
	return ret;
}

void ScriptEvent::registerCommand(script::Command * command) {
	
	typedef std::pair<Commands::iterator, bool> Res;
	
	Res res = commands.insert(std::make_pair(command->getName(), command));
	
	if(!res.second) {
		LogError << "Duplicate script command name: " + command->getName();
		delete command;
	}
	
}

void ScriptEvent::init() {
	
	size_t count = script::initSuppressions();
	
	script::setupScriptedAnimation();
	script::setupScriptedCamera();
	script::setupScriptedControl();
	script::setupScriptedConversation();
	script::setupScriptedInterface();
	script::setupScriptedInventory();
	script::setupScriptedIOControl();
	script::setupScriptedIOProperties();
	script::setupScriptedItem();
	script::setupScriptedLang();
	script::setupScriptedNPC();
	script::setupScriptedPlayer();
	script::setupScriptedVariable();
	
	registerCommand(new script::ObsoleteCommand("attachnpctoplayer"));
	registerCommand(new script::ObsoleteCommand("gmode", 1));
	registerCommand(new script::ObsoleteCommand("setrighthand", 1));
	registerCommand(new script::ObsoleteCommand("setlefthand", 1));
	registerCommand(new script::ObsoleteCommand("setshield", 1));
	registerCommand(new script::ObsoleteCommand("settwohanded"));
	registerCommand(new script::ObsoleteCommand("setonehanded"));
	registerCommand(new script::ObsoleteCommand("say"));
	registerCommand(new script::ObsoleteCommand("setdetachable", 1));
	registerCommand(new script::ObsoleteCommand("setstackable", 1));
	registerCommand(new script::ObsoleteCommand("setinternalname", 1));
	registerCommand(new script::ObsoleteCommand("detachnpcfromplayer"));
	
	LogInfo << "Scripting system initialized with " << commands.size() << " commands and " << count << " suppressions";
}

void ScriptEvent::shutdown() {
	// Delete all commands before clear
	for(Commands::iterator i = commands.begin(); i != commands.end(); ++i) {
		delete i->second;
	}
	// Remove all the commands
	commands.clear();
	LogInfo << "Scripting system shutdown";
}

void ScriptEvent::autocomplete(const std::string & prefix, AutocompleteHandler handler, void * context) {
	
	std::string cmd = boost::to_lower_copy(prefix);
	cmd.resize(std::remove(cmd.begin(), cmd.end(), '_') - cmd.begin());
	
	if(boost::starts_with("timer", cmd)) {
		if(!handler(context, "timer")) {
			return;
		}
	}
	
	BOOST_FOREACH(const Commands::value_type & v, commands) {
		if(boost::starts_with(v.first, cmd)) {
			if(!handler(context, v.first + " ")) {
				return;
			}
		}
	}
	
}

bool ScriptEvent::isCommand(const std::string & command) {
	
	if(boost::starts_with(command, "timer")) {
		return true;
	}
	
	return commands.find(command) != commands.end();
}

ScriptEvent::Commands ScriptEvent::commands;
