/*
 * Copyright 2011-2022 Arx Libertatis Team (see the AUTHORS file)
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

#include <utility>

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

#include "util/String.h"


long ScriptEvent::totalCount = 0;

std::string_view ScriptEvent::name(ScriptMessage event) {
	
	switch(event) {
		case SM_NULL:                   return "on null";
		case SM_ACTION:                 return "on action";
		case SM_AGGRESSION:             return "on aggression";
		case SM_BACKSTAB:               return "on backstab";
		case SM_BOOK_CLOSE:             return "on book_close";
		case SM_BOOK_OPEN:              return "on book_open";
		case SM_BREAK:                  return "on break";
		case SM_CHAT:                   return "on chat";
		case SM_CINE_END:               return "on cine_end";
		case SM_CLICKED:                return "on clicked";
		case SM_COLLIDE_DOOR:           return "on collide_door";
		case SM_COLLIDE_FIELD:          return "on collide_field";
		case SM_COLLIDE_NPC:            return "on collide_npc";
		case SM_COLLISION_ERROR:        return "on collision_error";
		case SM_COLLISION_ERROR_DETAIL: return "on collision_error_detail";
		case SM_COMBINE:                return "on combine";
		case SM_CONTROLLEDZONE_ENTER:   return "on controlledzone_enter";
		case SM_CONTROLLEDZONE_LEAVE:   return "on controlledzone_leave";
		case SM_CONTROLS_OFF:           return "on controls_off";
		case SM_CONTROLS_ON:            return "on controls_on";
		case SM_CRITICAL:               return "on critical";
		case SM_CURSORMODE:             return "on cursormode";
		case SM_CUSTOM:                 return "on custom";
		case SM_DEAD:                   return "on dead";
		case SM_DETECTPLAYER:           return "on detectplayer";
		case SM_DIE:                    return "on die";
		case SM_ENTERZONE:              return "on enterzone";
		case SM_EQUIPIN:                return "on equipin";
		case SM_EQUIPOUT:               return "on equipout";
		case SM_EXPLORATIONMODE:        return "on explorationmode";
		case SM_GAME_READY:             return "on game_ready";
		case SM_HEAR:                   return "on hear";
		case SM_HIT:                    return "on hit";
		case SM_IDENTIFY:               return "on identify";
		case SM_INIT:                   return "on init";
		case SM_INITEND:                return "on initend";
		case SM_INVENTORY2_CLOSE:       return "on inventory2_close";
		case SM_INVENTORY2_OPEN:        return "on inventory2_open";
		case SM_INVENTORYIN:            return "on inventoryin";
		case SM_INVENTORYUSE:           return "on inventoryuse";
		case SM_KEY_PRESSED:            return "on key_pressed";
		case SM_LEAVEZONE:              return "on leavezone";
		case SM_LOAD:                   return "on load";
		case SM_LOSTTARGET:             return "on losttarget";
		case SM_MAIN:                   return "on main";
		case SM_OUCH:                   return "on ouch";
		case SM_PATHEND:                return "on pathend";
		case SM_PATHFINDER_FAILURE:     return "on pathfinder_failure";
		case SM_PATHFINDER_SUCCESS:     return "on pathfinder_success";
		case SM_REACHEDTARGET:          return "on reachedtarget";
		case SM_RELOAD:                 return "on reload";
		case SM_SPELLCAST:              return "on spellcast";
		case SM_SPELLEND:               return "on spellend";
		case SM_STEAL:                  return "on steal";
		case SM_STRIKE:                 return "on strike";
		case SM_SUMMONED:               return "on summoned";
		case SM_TREATOUT:               return "on treatout";
		case SM_UNDETECTPLAYER:         return "on undetectplayer";
		case SM_WAYPOINT:               return "on waypoint";
		case SM_FIGHT:                  return "on fight";
		case SM_INVENTORYOUT:           return "on inventoryout";
		case SM_MOVE:                   return "on move";
		case SM_RESET:                  return "on reset";
		case SM_SPELLDECISION:          return "on spelldecision";
		case SM_TRAP_DISARMED:          return "on trap_disarmed";
		case SM_MAXCMD:                 arx_unreachable();
		case SM_EXECUTELINE:            arx_unreachable();
		case SM_DUMMY:                  arx_unreachable();
	}
	
	arx_unreachable();
	
}

void ARX_SCRIPT_ComputeShortcuts(EERIE_SCRIPT & es) {
	for(size_t i = 1; i < SM_MAXCMD; i++) {
		es.shortcut[i] = FindScriptPos(&es, ScriptEvent::name(ScriptMessage(i)));
	}
}

static bool checkInteractiveObject(Entity * io, ScriptMessage msg, ScriptResult & ret) {
	
	io->stat_count++;
	
	if((io->gameFlags & GFLAG_MEGAHIDE) && msg != SM_RELOAD) {
		ret = ACCEPT;
		return true;
	}
	
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
	
	ObsoleteCommand(std::string_view command, size_t _nargs = 0) : Command(command), nargs(_nargs) { }
	
	Result execute(Context & context) override {
		
		for(size_t i = 0; i < nargs; i++) {
			context.skipWord();
		}
		
		ScriptWarning << "obsolete command";
		
		return Failed;
	}
	
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
                               ScriptEventName event, ScriptParameters parameters,
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
	
	script::Context context(es, pos, sender, entity, event.getId(), std::move(parameters));
	
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
		
		if(auto it = commands.find(word); it != commands.end()) {
			
			script::Command & command = *(it->second);
			
			script::Command::Result res;
			if(command.getEntityFlags()
			   && (command.getEntityFlags() != script::Command::AnyEntity
			       && !(command.getEntityFlags() & long(entity->ioflags)))) {
				ScriptEventWarning << "Command " << command.getName() << " needs an entity of type "
				                   << command.getEntityFlags();
				context.skipCommand();
				res = script::Command::Failed;
			} else if(context.getParameters().isPeekOnly()) {
				res = command.peek(context);
			} else {
				res = command.execute(context);
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
			if(context.getParameters().isPeekOnly()) {
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

void ScriptEvent::registerCommand(std::unique_ptr<script::Command> command) {
	std::string_view name = command->getName();
	[[maybe_unused]] auto res = commands.emplace(name, std::move(command));
	arx_assert_msg(res.second, "Duplicate script command name: %s", res.first->second->getName().c_str());
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
	
	registerCommand(std::make_unique<script::ObsoleteCommand>("attachnpctoplayer"));
	registerCommand(std::make_unique<script::ObsoleteCommand>("gmode", 1));
	registerCommand(std::make_unique<script::ObsoleteCommand>("setrighthand", 1));
	registerCommand(std::make_unique<script::ObsoleteCommand>("setlefthand", 1));
	registerCommand(std::make_unique<script::ObsoleteCommand>("setshield", 1));
	registerCommand(std::make_unique<script::ObsoleteCommand>("settwohanded"));
	registerCommand(std::make_unique<script::ObsoleteCommand>("setonehanded"));
	registerCommand(std::make_unique<script::ObsoleteCommand>("say"));
	registerCommand(std::make_unique<script::ObsoleteCommand>("setdetachable", 1));
	registerCommand(std::make_unique<script::ObsoleteCommand>("setstackable", 1));
	registerCommand(std::make_unique<script::ObsoleteCommand>("setinternalname", 1));
	registerCommand(std::make_unique<script::ObsoleteCommand>("detachnpcfromplayer"));
	
	LogInfo << "Scripting system initialized with " << commands.size() << " commands and " << count << " suppressions";
}

void ScriptEvent::shutdown() {
	// Remove all the commands
	commands.clear();
	LogInfo << "Scripting system shutdown";
}

void ScriptEvent::autocomplete(std::string_view prefix, AutocompleteHandler handler, void * context) {
	
	std::string cmd = util::toLowercase(prefix);
	cmd.resize(std::remove(cmd.begin(), cmd.end(), '_') - cmd.begin());
	
	if(boost::starts_with("timer", cmd)) {
		if(!handler(context, "timer")) {
			return;
		}
	}
	
	for(const auto & v : commands) {
		if(boost::starts_with(v.first, cmd)) {
			std::string command(v.first);
			command += " ";
			if(!handler(context, command)) {
				return;
			}
		}
	}
	
}

bool ScriptEvent::isCommand(std::string_view command) {
	
	if(boost::starts_with(command, "timer")) {
		return true;
	}
	
	return commands.find(command) != commands.end();
}

std::map<std::string_view, std::unique_ptr<script::Command>> ScriptEvent::commands;
