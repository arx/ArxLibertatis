/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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

#include "script/Script.h"

#include <stddef.h>

#include <sstream>
#include <cstdio>
#include <algorithm>
#include <limits>

#include <boost/foreach.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "ai/Paths.h"

#include "core/GameTime.h"
#include "core/Core.h"
#include "core/Config.h"

#include "game/Camera.h"
#include "game/Damage.h"
#include "game/EntityManager.h"
#include "game/Equipment.h"
#include "game/Inventory.h"
#include "game/Item.h"
#include "game/NPC.h"
#include "game/Player.h"

#include "gui/Speech.h"

#include "graphics/particle/ParticleEffects.h"
#include "graphics/Math.h"

#include "io/resource/PakReader.h"
#include "io/log/Logger.h"

#include "platform/profiler/Profiler.h"

#include "scene/Scene.h"
#include "scene/Interactive.h"

#include "script/ScriptEvent.h"
#include "script/ScriptUtils.h"


extern long lChangeWeapon;
extern Entity * pIOChangeWeapon;

Entity * LASTSPAWNED = NULL;
SCRIPT_VARIABLES svar;

long FORBID_SCRIPT_IO_CREATION = 0;
std::vector<SCR_TIMER> g_scriptTimers;
static size_t g_activeScriptTimers = 0;

bool isLocalVariable(const std::string & name) {
	
	arx_assert(!name.empty());
	
	switch(name[0]) {
		case '\xA3': return true;
		case '\xA7': return true;
		case '@':    return true;
		default:     return false;
	}
	
}

std::ostream & operator<<(std::ostream & os, const SCRIPT_VAR & var) {
	
	arx_assert(!var.name.empty());
	
	os << var.name << " = ";
	
	switch(var.name[0]) {
		
		case '$':
		case '\xA3': {
			os << '\"' << var.text << '\"';
			break;
		}
		
		case '#':
		case '\xA7': {
			os << var.ival;
			break;
		}
		
		case '&':
		case '@': {
			os << var.fval;
			break;
		}
		
		default: {
			os << "(unknown variable type)" << var;
			break;
		}
		
	}
	
	return os;
}

ScriptEventName ScriptEventName::parse(const std::string & name) {
	
	for(size_t i = 1; i < SM_MAXCMD; i++) {
		const std::string & event = AS_EVENT[i].name;
		if(event.length() > 3 && event.compare(3, event.length() - 3, name) == 0) {
			return ScriptEventName(ScriptMessage(i));
		}
	}
	
	return ScriptEventName(name);
}

std::string ScriptEventName::toString() const {
	
	if(!getName().empty()) {
		arx_assert(getId() == SM_NULL);
		return getName();
	}
	
	arx_assert(getId() < SM_MAXCMD && AS_EVENT[getId()].name.length() > 3);
	return AS_EVENT[getId()].name.substr(3);
}

DisabledEvents ScriptEventName::toDisabledEventsMask() const {
	
	switch(getId()) {
		case SM_COLLIDE_NPC: return DISABLE_COLLIDE_NPC;
		case SM_CHAT: return DISABLE_CHAT;
		case SM_HIT: return DISABLE_HIT;
		case SM_INVENTORY2_OPEN: return DISABLE_INVENTORY2_OPEN;
		case SM_HEAR: return DISABLE_HEAR;
		case SM_UNDETECTPLAYER: return DISABLE_DETECT;
		case SM_DETECTPLAYER: return DISABLE_DETECT;
		case SM_AGGRESSION: return DISABLE_AGGRESSION;
		case SM_MAIN: return DISABLE_MAIN;
		case SM_CURSORMODE: return DISABLE_CURSORMODE;
		case SM_EXPLORATIONMODE: return DISABLE_EXPLORATIONMODE;
		default: return 0;
	}
	
}

std::ostream & operator<<(std::ostream & os, const ScriptEventName & event) {
	
	if(event == SM_EXECUTELINE) {
		return os << "executeline";
	}
	if(event == SM_DUMMY)  {
		return os << "dummy event";
	}
	if(!event.getName().empty()) {
		return os << "on " << event.getName() << " event";
	}
	
	arx_assert(event.getId() < SM_MAXCMD && AS_EVENT[event.getId()].name.length() > 3);
	return os << AS_EVENT[event.getId()].name << " event";
}

ScriptParameters ScriptParameters::parse(const std::string & str) {
	
	ScriptParameters result;
	
	if(str.empty()) {
		return result;
	}
	
	for(size_t start = 0; start < str.length(); ) {
		
		size_t end = str.find(' ', start);
		if(end == std::string::npos) {
			end = str.length();
		}
		
		result.push_back(str.substr(start, end - start));
		
		start = end + 1;
	}
	
	return result;
}

std::ostream & operator<<(std::ostream & os, const ScriptParameters & parameters) {
	
	os << '"';
	if(!parameters.empty()) {
		os << parameters[0];
		for(size_t i = 1; i < parameters.size(); i++) {
			os << ' ' << parameters[i];
		}
	}
	os << '"';
	
	return os;
}

size_t FindScriptPos(const EERIE_SCRIPT * es, const std::string & str) {
	
	// TODO(script-parser) remove, respect quoted strings
	
	for(size_t pos = 0; pos < es->data.size(); pos++) {
		
		pos = es->data.find(str, pos);
		if(pos == std::string::npos || pos + str.length() >= es->data.size()) {
			return size_t(-1);
		}
		
		if(u8(es->data[pos + str.length()]) > 32) {
			continue;
		}
		
		// Check if the line is commented out!
		for(size_t p = pos; es->data[p] != '/' || es->data[p + 1] != '/'; p--) {
			if(es->data[p] == '\n' || p == 0) {
				return pos + str.length();
			}
		}
		
	}
	
	return size_t(-1);
}

ScriptResult SendMsgToAllIO(Entity * sender, const ScriptEventName & event,
                            const ScriptParameters & parameters) {
	
	ScriptResult ret = ACCEPT;
	
	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * e = entities[handle];
		if(e) {
			if(SendIOScriptEvent(sender, e, event, parameters) == REFUSE) {
				ret = REFUSE;
			}
		}
	}
	
	return ret;
}

void ARX_SCRIPT_ResetObject(Entity * io, bool init) {
	
	if(!io)
		return;
	
	io->m_disabledEvents = 0;
	
	// Now go for Script INIT/RESET depending on Mode
	EntityHandle num = io->index();
	
	if(entities[num] && entities[num]->script.valid) {
		if(init) {
			ScriptEvent::send(&entities[num]->script, NULL, entities[num], SM_INIT);
		}
		if(entities[num]) {
			entities[num]->mainevent = SM_MAIN;
		}
	}
	
	// Do the same for Local Script
	if(entities[num] && entities[num]->over_script.valid && init) {
		ScriptEvent::send(&entities[num]->over_script, NULL, entities[num], SM_INIT);
	}
	
	// Sends InitEnd Event
	if(init) {
		if(entities[num] && entities[num]->script.valid) {
			ScriptEvent::send(&entities[num]->script, NULL, entities[num], SM_INITEND);
		}
		if(entities[num] && entities[num]->over_script.valid) {
			ScriptEvent::send(&entities[num]->over_script, NULL, entities[num], SM_INITEND);
		}
	}
	
	if(entities[num]) {
		entities[num]->gameFlags &= ~GFLAG_NEEDINIT;
	}
	
}

void ARX_SCRIPT_Reset(Entity * io, bool init) {
	
	// Release Script Over-Script Local Variables
	io->m_variables.clear();
	
	if(!io->scriptload) {
		ARX_SCRIPT_ResetObject(io, init);
	}
	
}

void ARX_SCRIPT_ResetAll(bool init) {
	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * e = entities[handle];
		
		if(e && !e->scriptload) {
			ARX_SCRIPT_Reset(e, init);
		}
	}
}

void ARX_SCRIPT_AllowInterScriptExec() {
	
	ARX_PROFILE_FUNC();
	
	// FIXME static local variable
	static long ppos = 0;
	
	if(g_gameTime.isPaused()) {
		return;
	}
	
	long heartbeat_count = std::min(long(entities.size()), 10l);
	
	for(long n = 0; n < heartbeat_count; n++) {
		
		EntityHandle i = EntityHandle(ppos++);
		if(i.handleData() >= long(entities.size())){
			ppos = 0;
			return;
		}
		
		if(entities[i] == NULL || !(entities[i]->gameFlags & GFLAG_ISINTREATZONE)) {
			continue;
		}
		
		// Copy the even name to a local variable as it may change during execution
		// and cause unexpected behavior in SendIOScriptEvent
		ScriptEventName event = entities[i]->mainevent;
		SendIOScriptEvent(NULL, entities[i], event);
		
	}
	
}

void ReleaseScript(EERIE_SCRIPT * es) {
	
	if(!es) {
		return;
	}
	
	es->valid = false;
	
	es->data.clear();
	
	memset(es->shortcut, 0, sizeof(es->shortcut));
	
}

ValueType getSystemVar(const script::Context & context, const std::string & name,
                       std::string & txtcontent, float * fcontent, long * lcontent) {
	
	arx_assert_msg(!name.empty() && name[0] == '^', "bad system variable: \"%s\"", name.c_str());
	
	char c = (name.length() < 2) ? '\0' : name[1];
	switch(c) {
		
		case '$': {
			
			if(name == "^$param1") {
				txtcontent = context.getParameter(0);
				return TYPE_TEXT;
			}
			
			if(name == "^$param2") {
				txtcontent = context.getParameter(1);
				return TYPE_TEXT;
			}
			
			if(name == "^$param3") {
				txtcontent = context.getParameter(2);
				return TYPE_TEXT;
			}
			
			if(name == "^$objontop") {
				txtcontent = "none";
				if(context.getEntity()) {
					MakeTopObjString(context.getEntity(), txtcontent);
				}
				return TYPE_TEXT;
			}
			
			break;
		}
		
		case '&': {
			
			if(name == "^&param1") {
				*fcontent = float(atof(context.getParameter(0).c_str()));
				return TYPE_FLOAT;
			}
			
			if(name == "^&param2") {
				*fcontent = float(atof(context.getParameter(1).c_str()));
				return TYPE_FLOAT;
			}
			
			if(name == "^&param3") {
				*fcontent = float(atof(context.getParameter(2).c_str()));
				return TYPE_FLOAT;
			}
			
			if(name == "^&playerdist") {
				if(context.getEntity()) {
					*fcontent = fdist(player.pos, context.getEntity()->pos);
					return TYPE_FLOAT;
				}
			}
			
			break;
		}
		
		case '#': {
			
			if(name == "^#playerdist") {
				if(context.getEntity()) {
					*lcontent = long(fdist(player.pos, context.getEntity()->pos));
					return TYPE_LONG;
				}
			}
			
			if(name == "^#param1") {
				*lcontent = atol(context.getParameter(0).c_str());
				return TYPE_LONG;
			}
			
			if(name == "^#param2") {
				*lcontent = atol(context.getParameter(1).c_str());
				return TYPE_LONG;
			}
			
			if(name == "^#param3") {
				*lcontent = atol(context.getParameter(2).c_str());
				return TYPE_LONG;
			}
			
			if(name == "^#timer1") {
				if(!context.getEntity() || context.getEntity()->m_scriptTimers[0] == 0) {
					*lcontent = 0;
				} else {
					*lcontent = toMsi(g_gameTime.now() - context.getEntity()->m_scriptTimers[0]);
				}
				return TYPE_LONG;
			}
			
			if(name == "^#timer2") {
				if(!context.getEntity() || context.getEntity()->m_scriptTimers[1] == 0) {
					*lcontent = 0;
				} else {
					*lcontent = toMsi(g_gameTime.now() - context.getEntity()->m_scriptTimers[1]);
				}
				return TYPE_LONG;
			}
			
			if(name == "^#timer3") {
				if(!context.getEntity() || context.getEntity()->m_scriptTimers[2] == 0) {
					*lcontent = 0;
				} else {
					*lcontent = toMsi(g_gameTime.now() - context.getEntity()->m_scriptTimers[2]);
				}
				return TYPE_LONG;
			}
			
			if(name == "^#timer4") {
				if(!context.getEntity() || context.getEntity()->m_scriptTimers[3] == 0) {
					*lcontent = 0;
				} else {
					*lcontent = toMsi(g_gameTime.now() - context.getEntity()->m_scriptTimers[3]);
				}
				return TYPE_LONG;
			}
			
			break;
		}
		
		case 'g': {
			
			if(name == "^gore") {
				*lcontent = 1;
				return TYPE_LONG;
			}
			
			if(name == "^gamedays") {
				*lcontent = static_cast<long>(toMsi(g_gameTime.now()) / 86400000);
				return TYPE_LONG;
			}
			
			if(name == "^gamehours") {
				*lcontent = static_cast<long>(toMsi(g_gameTime.now()) / 3600000);
				return TYPE_LONG;
			}
			
			if(name == "^gameminutes") {
				*lcontent = static_cast<long>(toMsi(g_gameTime.now()) / 60000);
				return TYPE_LONG;
			}
			
			if(name == "^gameseconds") {
				*lcontent = static_cast<long>(toMsi(g_gameTime.now()) / 1000);
				return TYPE_LONG;
			}
			
			break;
		}
		
		case 'a': {
			
			if(boost::starts_with(name, "^amount")) {
				if(context.getEntity() && (context.getEntity()->ioflags & IO_ITEM)) {
					*fcontent = context.getEntity()->_itemdata->count;
				} else {
					*fcontent = 0;
				}
				return TYPE_FLOAT;
			}
			
			if(name == "^arxdays") {
				*lcontent = static_cast<long>(toMsi(g_gameTime.now()) * 6 * 2 / 86400000);
				return TYPE_LONG;
			}
			
			if(name == "^arxhours") {
				*lcontent = static_cast<long>(toMsi(g_gameTime.now()) * 6 / 3600000);
				return TYPE_LONG;
			}
			
			if(name == "^arxminutes") {
				*lcontent = static_cast<long>(toMsi(g_gameTime.now()) * 6 / 60000);
				return TYPE_LONG;
			}
			
			if(name == "^arxseconds") {
				*lcontent = static_cast<long>(toMsi(g_gameTime.now()) * 6 / 1000);
				return TYPE_LONG;
			}
			
			if(name == "^arxtime_hours") {
				*lcontent = static_cast<long>(toMsi(g_gameTime.now()) * 6 / 3600000) % 12;
				if(*lcontent == 0) {
					*lcontent = 12;
				}
				return TYPE_LONG;
			}
			
			if(name == "^arxtime_minutes") {
				*lcontent = static_cast<long>(toMsi(g_gameTime.now()) * 6 / 60000) % 60;
				if(*lcontent == 0) {
					*lcontent = 60;
				}
				return TYPE_LONG;
			}
			
			if(name == "^arxtime_seconds") {
				*lcontent = static_cast<long>(toMsi(g_gameTime.now()) * 6 / 1000) % 60;
				if(*lcontent == 0) {
					*lcontent = 60;
				}
				return TYPE_LONG;
			}
			
			break;
		}
		
		case 'r': {
			
			if(boost::starts_with(name, "^realdist_")) {
				if(context.getEntity()) {
					const char * obj = name.c_str() + 10;
					
					if(!strcmp(obj, "player")) {
						if(context.getEntity()->requestRoomUpdate) {
							UpdateIORoom(context.getEntity());
						}
						long Player_Room = ARX_PORTALS_GetRoomNumForPosition(player.pos, 1);
						*fcontent = SP_GetRoomDist(context.getEntity()->pos, player.pos, context.getEntity()->room, Player_Room);
						return TYPE_FLOAT;
					}
					
					EntityHandle t = entities.getById(obj);
					if(ValidIONum(t)) {
						if((context.getEntity()->show == SHOW_FLAG_IN_SCENE
						    || context.getEntity()->show == SHOW_FLAG_IN_INVENTORY)
						   && (entities[t]->show == SHOW_FLAG_IN_SCENE
						       || entities[t]->show == SHOW_FLAG_IN_INVENTORY)) {
							
							Vec3f pos  = GetItemWorldPosition(context.getEntity());
							Vec3f pos2 = GetItemWorldPosition(entities[t]);
							
							if(context.getEntity()->requestRoomUpdate) {
								UpdateIORoom(context.getEntity());
							}
							
							if(entities[t]->requestRoomUpdate) {
								UpdateIORoom(entities[t]);
							}
							
							*fcontent = SP_GetRoomDist(pos, pos2, context.getEntity()->room, entities[t]->room);
							
						} else {
							// Out of this world item
							*fcontent = 99999999999.f;
						}
						return TYPE_FLOAT;
					}
					
					*fcontent = 99999999999.f;
					return TYPE_FLOAT;
				}
			}
			
			if(boost::starts_with(name, "^repairprice_")) {
				EntityHandle t = entities.getById(name.substr(13));
				if(ValidIONum(t)) {
					*fcontent = ARX_DAMAGES_ComputeRepairPrice(entities[t], context.getEntity());
				} else {
					*fcontent = 0;
				}
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^rnd_")) {
				const char * max = name.c_str() + 5;
				// TODO should max be inclusive or exclusive?
				// if inclusive, use proper integer random, otherwise fix rnd()?
				if(max[0]) {
					float t = float(atof(max));
					*fcontent = Random::getf(0.f, t);
					return TYPE_FLOAT;
				}
				*fcontent = 0;
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^rune_")) {
				std::string temp = name.substr(6);
				*lcontent = 0;
				if(temp == "aam") {
					*lcontent = player.rune_flags & FLAG_AAM;
				} else if(temp == "cetrius") {
					*lcontent = player.rune_flags & FLAG_CETRIUS;
				} else if(temp == "comunicatum") {
					*lcontent = player.rune_flags & FLAG_COMUNICATUM;
				} else if(temp == "cosum") {
					*lcontent = player.rune_flags & FLAG_COSUM;
				} else if(temp == "folgora") {
					*lcontent = player.rune_flags & FLAG_FOLGORA;
				} else if(temp == "fridd") {
					*lcontent = player.rune_flags & FLAG_FRIDD;
				} else if(temp == "kaom") {
					*lcontent = player.rune_flags & FLAG_KAOM;
				} else if(temp == "mega") {
					*lcontent = player.rune_flags & FLAG_MEGA;
				} else if(temp == "morte") {
					*lcontent = player.rune_flags & FLAG_MORTE;
				} else if(temp == "movis") {
					*lcontent = player.rune_flags & FLAG_MOVIS;
				} else if(temp == "nhi") {
					*lcontent = player.rune_flags & FLAG_NHI;
				} else if(temp == "rhaa") {
					*lcontent = player.rune_flags & FLAG_RHAA;
				} else if(temp == "spacium") {
					*lcontent = player.rune_flags & FLAG_SPACIUM;
				} else if(temp == "stregum") {
					*lcontent = player.rune_flags & FLAG_STREGUM;
				} else if(temp == "taar") {
					*lcontent = player.rune_flags & FLAG_TAAR;
				} else if(temp == "tempus") {
					*lcontent = player.rune_flags & FLAG_TEMPUS;
				} else if(temp == "tera") {
					*lcontent = player.rune_flags & FLAG_TERA;
				} else if(temp == "vista") {
					*lcontent = player.rune_flags & FLAG_VISTA;
				} else if(temp == "vitae") {
					*lcontent = player.rune_flags & FLAG_VITAE;
				} else if(temp == "yok") {
					*lcontent = player.rune_flags & FLAG_YOK;
				}
				return TYPE_LONG;
			}
			
			break;
		}
		
		case 'i': {
			
			if(boost::starts_with(name, "^inzone_")) {
				const char * zone = name.c_str() + 8;
				ARX_PATH * ap = ARX_PATH_GetAddressByName(zone);
				*lcontent = 0;
				if(context.getEntity() && ap) {
					if(ARX_PATH_IsPosInZone(ap, context.getEntity()->pos)) {
						*lcontent = 1;
					}
				}
				return TYPE_LONG;
			}
			
			if(boost::starts_with(name, "^ininitpos")) {
				*lcontent = 0;
				if(context.getEntity()) {
					Vec3f pos = GetItemWorldPosition(context.getEntity());
					if(pos == context.getEntity()->initpos)
						*lcontent = 1;
				}
				return TYPE_LONG;
			}
			
			if(boost::starts_with(name, "^inplayerinventory")) {
				*lcontent = 0;
				if(context.getEntity() && (context.getEntity()->ioflags & IO_ITEM) && IsInPlayerInventory(context.getEntity())) {
					*lcontent = 1;
				}
				return TYPE_LONG;
			}
			
			break;
		}
		
		case 'b': {
			
			if(boost::starts_with(name, "^behavior")) {
				txtcontent = "";
				if(context.getEntity() && (context.getEntity()->ioflags & IO_NPC)) {
					if(context.getEntity()->_npcdata->behavior & BEHAVIOUR_LOOK_AROUND) {
						txtcontent += "l";
					}
					if(context.getEntity()->_npcdata->behavior & BEHAVIOUR_SNEAK) {
						txtcontent += "s";
					}
					if(context.getEntity()->_npcdata->behavior & BEHAVIOUR_DISTANT) {
						txtcontent += "d";
					}
					if(context.getEntity()->_npcdata->behavior & BEHAVIOUR_MAGIC) {
						txtcontent += "m";
					}
					if(context.getEntity()->_npcdata->behavior & BEHAVIOUR_FIGHT) {
						txtcontent += "f";
					}
					if(context.getEntity()->_npcdata->behavior & BEHAVIOUR_GO_HOME) {
						txtcontent += "h";
					}
					if(context.getEntity()->_npcdata->behavior & BEHAVIOUR_FRIENDLY) {
						txtcontent += "r";
					}
					if(context.getEntity()->_npcdata->behavior & BEHAVIOUR_MOVE_TO) {
						txtcontent += "t";
					}
					if(context.getEntity()->_npcdata->behavior & BEHAVIOUR_FLEE) {
						txtcontent += "e";
					}
					if(context.getEntity()->_npcdata->behavior & BEHAVIOUR_LOOK_FOR) {
						txtcontent += "o";
					}
					if(context.getEntity()->_npcdata->behavior & BEHAVIOUR_HIDE) {
						txtcontent += "i";
					}
					if(context.getEntity()->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND) {
						txtcontent += "w";
					}
					if(context.getEntity()->_npcdata->behavior & BEHAVIOUR_GUARD) {
						txtcontent += "u";
					}
					if(context.getEntity()->_npcdata->behavior & BEHAVIOUR_STARE_AT) {
						txtcontent += "a";
					}
				}
				return TYPE_TEXT;
			}
			
			break;
		}
		
		case 's': {
			
			if(boost::starts_with(name, "^sender")) {
				if(!context.getSender()) {
					txtcontent = "none";
				} else if(context.getSender() == entities.player()) {
					txtcontent = "player";
				} else {
					txtcontent = context.getSender()->idString();
				}
				return TYPE_TEXT;
			}
			
			if(boost::starts_with(name, "^scale")) {
				*fcontent = (context.getEntity()) ? context.getEntity()->scale * 100.f : 0.f;
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^speaking")) {
				if(context.getEntity()) {
					if(ARX_SPEECH_isEntitySpeaking(context.getEntity())) {
						*lcontent = 1;
						return TYPE_LONG;
					}
				}
				*lcontent = 0;
				return TYPE_LONG;
			}
			
			break;
		}
		
		case 'm': {
			
			if(boost::starts_with(name, "^me")) {
				if(!context.getEntity()) {
					txtcontent = "none";
				} else if(context.getEntity() == entities.player()) {
					txtcontent = "player";
				} else {
					txtcontent = context.getEntity()->idString();
				}
				return TYPE_TEXT;
			}
			
			if(boost::starts_with(name, "^maxlife")) {
				*fcontent = 0;
				if(context.getEntity() && (context.getEntity()->ioflags & IO_NPC)) {
					*fcontent = context.getEntity()->_npcdata->lifePool.max;
				}
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^mana")) {
				*fcontent = 0;
				if(context.getEntity() && (context.getEntity()->ioflags & IO_NPC)) {
					*fcontent = context.getEntity()->_npcdata->manaPool.current;
				}
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^maxmana")) {
				*fcontent = 0;
				if(context.getEntity() && (context.getEntity()->ioflags & IO_NPC)) {
					*fcontent = context.getEntity()->_npcdata->manaPool.max;
				}
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^myspell_")) {
				SpellType id = GetSpellId(name.substr(9));
				if(id != SPELL_NONE) {
					if(spells.ExistAnyInstanceForThisCaster(id, context.getEntity()->index())) {
						*lcontent = 1;
						return TYPE_LONG;
					}
				}
				*lcontent = 0;
				return TYPE_LONG;
			}
			
			if(boost::starts_with(name, "^maxdurability")) {
				*fcontent = (context.getEntity()) ? context.getEntity()->max_durability : 0.f;
				return TYPE_FLOAT;
			}
			
			break;
		}
		
		case 'l': {
			
			if(boost::starts_with(name, "^life")) {
				*fcontent = 0;
				if(context.getEntity() && (context.getEntity()->ioflags & IO_NPC)) {
					*fcontent = context.getEntity()->_npcdata->lifePool.current;
				}
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^last_spawned")) {
				txtcontent = (LASTSPAWNED) ? LASTSPAWNED->idString() : "none";
				return TYPE_TEXT;
			}
			
			break;
		}
		
		case 'd': {
			
			if(boost::starts_with(name, "^dist_")) {
				if(context.getEntity()) {
					const char * obj = name.c_str() + 6;
					
					if(!strcmp(obj, "player")) {
						*fcontent = fdist(player.pos, context.getEntity()->pos);
						return TYPE_FLOAT;
					}
					
					EntityHandle t = entities.getById(obj);
					if(ValidIONum(t)) {
						if((context.getEntity()->show == SHOW_FLAG_IN_SCENE
						    || context.getEntity()->show == SHOW_FLAG_IN_INVENTORY)
						   && (entities[t]->show == SHOW_FLAG_IN_SCENE
						       || entities[t]->show == SHOW_FLAG_IN_INVENTORY)) {
							Vec3f pos  = GetItemWorldPosition(context.getEntity());
							Vec3f pos2 = GetItemWorldPosition(entities[t]);
							*fcontent = fdist(pos, pos2);
							return TYPE_FLOAT;
						}
					}
					
					*fcontent = 99999999999.f;
					return TYPE_FLOAT;
				}
			}
			
			if(boost::starts_with(name, "^demo")) {
				*lcontent = (g_resources->getReleaseType() & PakReader::Demo) ? 1 : 0;
				return TYPE_LONG;
			}
			
			if(boost::starts_with(name, "^durability")) {
				*fcontent = (context.getEntity()) ? context.getEntity()->durability : 0.f;
				return TYPE_FLOAT;
			}
			
			break;
		}
		
		case 'p': {
			
			if(boost::starts_with(name, "^price")) {
				*fcontent = 0;
				if(context.getEntity() && (context.getEntity()->ioflags & IO_ITEM)) {
					*fcontent = static_cast<float>(context.getEntity()->_itemdata->price);
				}
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^player_zone")) {
				txtcontent = (player.inzone) ? player.inzone->name : "none";
				return TYPE_TEXT;
			}
			
			if(boost::starts_with(name, "^player_life")) {
				*fcontent = player.Full_life; // TODO why not player.life like everywhere else?
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^poisoned")) {
				*fcontent = 0;
				if(context.getEntity() && (context.getEntity()->ioflags & IO_NPC)) {
					*fcontent = context.getEntity()->_npcdata->poisonned;
				}
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^poisonous")) {
				*fcontent = (context.getEntity()) ? context.getEntity()->poisonous : 0.f;
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^possess_")) {
				EntityHandle t = entities.getById(name.substr(9));
				if(ValidIONum(t)) {
					if(IsInPlayerInventory(entities[t])) {
						*lcontent = 1;
						return TYPE_LONG;
					}
					for(size_t i = 0; i < MAX_EQUIPED; i++) {
						if(ValidIONum(player.equiped[i]) && player.equiped[i] == t) {
							*lcontent = 2;
							return TYPE_LONG;
						}
					}
				}
				*lcontent = 0;
				return TYPE_LONG;
			}
			
			if(boost::starts_with(name, "^player_gold")) {
				*fcontent = static_cast<float>(player.gold);
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^player_maxlife")) {
				*fcontent = player.Full_maxlife;
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^player_attribute_strength")) {
				*fcontent = player.m_attributeFull.strength;
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^player_attribute_dexterity")) {
				*fcontent = player.m_attributeFull.dexterity;
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^player_attribute_constitution")) {
				*fcontent = player.m_attributeFull.constitution;
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^player_attribute_mind")) {
				*fcontent = player.m_attributeFull.mind;
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^player_skill_stealth")) {
				*fcontent = player.m_skillFull.stealth;
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^player_skill_mecanism")) {
				*fcontent = player.m_skillFull.mecanism;
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^player_skill_intuition")) {
				*fcontent = player.m_skillFull.intuition;
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^player_skill_etheral_link")) {
				*fcontent = player.m_skillFull.etheralLink;
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^player_skill_object_knowledge")) {
				*fcontent = player.m_skillFull.objectKnowledge;
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^player_skill_casting")) {
				*fcontent = player.m_skillFull.casting;
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^player_skill_projectile")) {
				*fcontent = player.m_skillFull.projectile;
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^player_skill_close_combat")) {
				*fcontent = player.m_skillFull.closeCombat;
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^player_skill_defense")) {
				*fcontent = player.m_skillFull.defense;
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^player_hunger")) {
				*fcontent = player.hunger;
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^player_poison")) {
				*fcontent = player.poison;
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^playercasting")) {
				for(size_t i = 0; i < MAX_SPELLS; i++) {
					const SpellBase * spell = spells[SpellHandle(i)];
					
					if(spell && spell->m_caster == EntityHandle_Player) {
						if(   spell->m_type == SPELL_LIFE_DRAIN
						   || spell->m_type == SPELL_HARM
						   || spell->m_type == SPELL_FIRE_FIELD
						   || spell->m_type == SPELL_ICE_FIELD
						   || spell->m_type == SPELL_LIGHTNING_STRIKE
						   || spell->m_type == SPELL_MASS_LIGHTNING_STRIKE
						) {
							*lcontent = 1;
							return TYPE_LONG;
						}
					}
				}
				*lcontent = 0;
				return TYPE_LONG;
			}
			
			if(boost::starts_with(name, "^playerspell_")) {
				std::string temp = name.substr(13);
				
				SpellType id = GetSpellId(temp);
				if(id != SPELL_NONE) {
					if(spells.ExistAnyInstanceForThisCaster(id, EntityHandle_Player)) {
						*lcontent = 1;
						return TYPE_LONG;
					}
				}
				
				if(temp == "invisibility" && entities.player()->invisibility > 0.3f) {
					*lcontent = 1;
					return TYPE_LONG;
				}
				
				*lcontent = 0;
				return TYPE_LONG;
			}
			
			break;
		}
		
		case 'n': {
			
			if(boost::starts_with(name, "^npcinsight")) {
				Entity * ioo = ARX_NPC_GetFirstNPCInSight(context.getEntity());
				if(!ioo) {
					txtcontent = "none";
				} else if(ioo == entities.player()) {
					txtcontent = "player";
				} else {
					txtcontent = ioo->idString();
				}
				return TYPE_TEXT;
			}
			
			break;
		}
		
		case 't': {
			
			if(boost::starts_with(name, "^target")) {
				if(!context.getEntity()) {
					txtcontent = "none";
				} else if(context.getEntity()->targetinfo == EntityHandle_Player) {
					txtcontent = "player";
				} else if(!ValidIONum(context.getEntity()->targetinfo)) {
					txtcontent = "none";
				} else {
					txtcontent = entities[context.getEntity()->targetinfo]->idString();
				}
				return TYPE_TEXT;
			}
			
			break;
		}
		
		case 'f': {
			
			if(boost::starts_with(name, "^focal")) {
				if(context.getEntity() && (context.getEntity()->ioflags & IO_CAMERA)) {
					*fcontent = context.getEntity()->_camdata->cam.focal;
					return TYPE_FLOAT;
				}
			}
			
			if(boost::starts_with(name, "^fighting")) {
				*lcontent = long(ARX_PLAYER_IsInFightMode());
				return TYPE_LONG;
			}
			
			break;
		}
		
		default: break;
		
	}
	
	*lcontent = 0;
	return TYPE_LONG;
}

void ARX_SCRIPT_Free_All_Global_Variables() {
	svar.clear();
}

void CloneLocalVars(Entity * ioo, Entity * io) {
	
	if(!ioo || !io) {
		return;
	}
	
	ioo->m_variables = io->m_variables;
}

static SCRIPT_VAR * GetFreeVarSlot(SCRIPT_VARIABLES & _svff) {
	_svff.resize(_svff.size() + 1);
	SCRIPT_VAR * v = &_svff.back();
	return v;
}

static SCRIPT_VAR * GetVarAddress(SCRIPT_VARIABLES & svf, const std::string & name) {
	
	for(SCRIPT_VARIABLES::iterator it = svf.begin(); it != svf.end(); ++it) {
		if(name == it->name) {
			return &(*it);
		}
	}
	
	return NULL;
}

const SCRIPT_VAR * GetVarAddress(const SCRIPT_VARIABLES & svf, const std::string & name) {
	
	for(SCRIPT_VARIABLES::const_iterator it = svf.begin(); it != svf.end(); ++it) {
		if(name == it->name) {
			return &(*it);
		}
	}
	
	return NULL;
}

long GETVarValueLong(const SCRIPT_VARIABLES & svf, const std::string & name) {
	
	const SCRIPT_VAR * tsv = GetVarAddress(svf, name);

	if (tsv == NULL) return 0;

	return tsv->ival;
}

float GETVarValueFloat(const SCRIPT_VARIABLES & svf, const std::string & name) {
	
	const SCRIPT_VAR * tsv = GetVarAddress(svf, name);

	if (tsv == NULL) return 0;

	return tsv->fval;
}

std::string GETVarValueText(const SCRIPT_VARIABLES & svf, const std::string & name) {
	
	const SCRIPT_VAR * tsv = GetVarAddress(svf, name);
	
	if (!tsv) return "";

	return tsv->text;
}

SCRIPT_VAR * SETVarValueLong(SCRIPT_VARIABLES & svf, const std::string & name, long val) {
	
	SCRIPT_VAR * tsv = GetVarAddress(svf, name);
	
	if (!tsv)
	{
		tsv = GetFreeVarSlot(svf);

		if (!tsv)
			return NULL;

		tsv->name = name;
	}

	tsv->ival = val;
	return tsv;
}

SCRIPT_VAR * SETVarValueFloat(SCRIPT_VARIABLES & svf, const std::string & name, float val) {
	
	SCRIPT_VAR * tsv = GetVarAddress(svf, name);
	
	if (!tsv)
	{
		tsv = GetFreeVarSlot(svf);

		if (!tsv)
			return NULL;

		tsv->name = name;
	}

	tsv->fval = val;
	return tsv;
}

SCRIPT_VAR * SETVarValueText(SCRIPT_VARIABLES & svf, const std::string & name, const std::string & val) {
	
	SCRIPT_VAR * tsv = GetVarAddress(svf, name);
	if(!tsv) {
		tsv = GetFreeVarSlot(svf);
		if(!tsv) {
			return NULL;
		}
		tsv->name = name;
	}
	
	tsv->text = val;
	
	return tsv;
}

struct QueuedEvent {
	
	bool exists;
	Entity * sender;
	Entity * entity;
	ScriptEventName event;
	ScriptParameters parameters;
	
	void clear() {
		exists = false;
		sender = NULL;
		entity = NULL;
		event = ScriptEventName();
		parameters.clear();
	}
	
};

// TODO use a queue
static QueuedEvent g_eventQueue[800];

void ARX_SCRIPT_EventStackInit() {
	ARX_SCRIPT_EventStackClear(false); // Clear everything in the stack
}

void ARX_SCRIPT_EventStackClear(bool check_exist) {
	LogDebug("clearing event queue");
	BOOST_FOREACH(QueuedEvent & event, g_eventQueue) {
		if(!check_exist || event.exists) {
			event.clear();
		}
	}
}

void ARX_SCRIPT_EventStackClearForIo(Entity * io) {
	BOOST_FOREACH(QueuedEvent & event, g_eventQueue) {
		if(event.exists && event.entity == io) {
			LogDebug("clearing queued " << event.event << " for " << io->idString());
			event.clear();
		}
	}
}

void ARX_SCRIPT_EventStackExecute(size_t limit) {
	
	ARX_PROFILE_FUNC();
	
	size_t count = 0;
	
	BOOST_FOREACH(QueuedEvent & event, g_eventQueue) {
		
		if(!event.exists) {
			continue;
		}
		
		if(ValidIOAddress(event.entity)) {
			Entity * sender = ValidIOAddress(event.sender) ? event.sender : NULL;
			LogDebug("running queued " << event.event << " for " << event.entity->idString());
			SendIOScriptEvent(sender, event.entity, event.event, event.parameters);
		} else {
			LogDebug("could not run queued " << event.event
			         << " params=\"" << event.parameters << "\" - entity vanished");
		}
		event.clear();
		
		// Abort if the event limit was reached
		if(++count >= limit) {
			return;
		}
		
	}
	
}

void ARX_SCRIPT_EventStackExecuteAll() {
	ARX_SCRIPT_EventStackExecute(std::numeric_limits<size_t>::max());
}

void Stack_SendIOScriptEvent(Entity * sender, Entity * entity, const ScriptEventName & event,
                             const ScriptParameters & parameters) {
	BOOST_FOREACH(QueuedEvent & entry, g_eventQueue) {
		if(!entry.exists) {
			entry.sender = sender;
			entry.entity = entity;
			entry.event = event;
			entry.parameters = parameters;
			entry.exists = true;
			return;
		}
	}
}

ScriptResult SendIOScriptEvent(Entity * sender, Entity * entity, const ScriptEventName & event,
                               const ScriptParameters & parameters) {
	
	ARX_PROFILE_FUNC();
	
	if(!entity) {
		return REFUSE;
	}
	
	EntityHandle num = entity->index();
	if(!entities[num]) {
		return REFUSE;
	}
	
	// Send the event to the instance script first
	if(entities[num]->over_script.valid) {
		ScriptResult ret = ScriptEvent::send(&entities[num]->over_script, sender, entities[num], event, parameters);
		if(ret == REFUSE || ret == DESTRUCTIVE || !entities[num]) {
			return !entities[num] ? REFUSE : ret;
		}
	}
	
	// If the instance script did not refuse the event also send it to the class script
	return ScriptEvent::send(&entities[num]->script, sender, entities[num], event, parameters);
}

ScriptResult SendInitScriptEvent(Entity * io) {
	
	if (!io) return REFUSE;

	EntityHandle num = io->index();
	
	if(entities[num] && entities[num]->script.valid) {
		ScriptEvent::send(&entities[num]->script, NULL, entities[num], SM_INIT);
	}
	
	if(entities[num] && entities[num]->over_script.valid) {
		ScriptEvent::send(&entities[num]->over_script, NULL, entities[num], SM_INIT);
	}
	
	if(entities[num] && entities[num]->script.valid) {
		ScriptEvent::send(&entities[num]->script, NULL, entities[num], SM_INITEND);
	}
	
	if(entities[num] && entities[num]->over_script.valid) {
		ScriptEvent::send(&entities[num]->over_script, NULL, entities[num], SM_INITEND);
	}
	
	return ACCEPT;
}

std::string getDefaultScriptTimerName(Entity * io, const std::string & prefix) {
	
	for(size_t i = 1; ; i++) {
		std::ostringstream oss;
		oss << prefix << '_' << i;
		if(!scriptTimerExists(io, oss.str())) {
			return oss.str();
		}
	}
	
}

SCR_TIMER & createScriptTimer(Entity * io, const std::string & name) {
	
	arx_assert(g_activeScriptTimers <= g_scriptTimers.size());
	
	g_activeScriptTimers++;
	
	if(g_activeScriptTimers != g_scriptTimers.size() + 1) {
		BOOST_FOREACH(SCR_TIMER & timer, g_scriptTimers) {
			if(!timer.exist) {
				timer = SCR_TIMER(io, name);
				return timer;
			}
		}
	}
	
	g_scriptTimers.push_back(SCR_TIMER(io, name));
	
	return g_scriptTimers.back();
}

size_t ARX_SCRIPT_CountTimers() {
	return g_activeScriptTimers;
}

static void clearTimer(SCR_TIMER & timer) {
	if(timer.exist) {
		LogDebug("clearing timer " << timer.name);
		timer.name.clear();
		timer.exist = 0;
		g_activeScriptTimers--;
	}
}

void ARX_SCRIPT_Timer_Clear_By_Name_And_IO(const std::string & timername, Entity * io) {
	BOOST_FOREACH(SCR_TIMER & timer, g_scriptTimers) {
		if(timer.exist && timer.io == io && timer.name == timername) {
			clearTimer(timer);
		}
	}
}

void ARX_SCRIPT_Timer_Clear_All_Locals_For_IO(Entity * io) {
	BOOST_FOREACH(SCR_TIMER & timer, g_scriptTimers) {
		if(timer.exist && timer.io == io && timer.es == &io->over_script) {
			clearTimer(timer);
		}
	}
}

void ARX_SCRIPT_Timer_ClearAll() {
	g_scriptTimers.clear();
	g_activeScriptTimers = 0;
}

void ARX_SCRIPT_Timer_Clear_For_IO(Entity * io) {
	BOOST_FOREACH(SCR_TIMER & timer, g_scriptTimers) {
		if(timer.exist && timer.io == io) {
			clearTimer(timer);
		}
	}
}

bool scriptTimerExists(Entity * io, const std::string & name) {
	
	if(g_activeScriptTimers != 0) {
		BOOST_FOREACH(const SCR_TIMER & timer, g_scriptTimers) {
			if(timer.exist && timer.io == io && timer.name == name) {
				return true;
			}
		}
	}
	
	return false;
}

static bool Manage_Specific_RAT_Timer(SCR_TIMER * st) {
	
	arx_assert(st->name == "_r_a_t_");
	
	Entity * io = st->io;
	GetTargetPos(io);
	Vec3f target = io->target - io->pos;
	target = glm::normalize(target);
	Vec3f targ = VRotateY(target, Random::getf(-30.f, 30.f));
	target = io->target + targ * 100.f;
	
	if(ARX_INTERACTIVE_ConvertToValidPosForIO(io, &target)) {
		ARX_INTERACTIVE_Teleport(io, target);
		Vec3f pos = io->pos;
		pos.y += io->physics.cyl.height * 0.5f;
		
		ARX_PARTICLES_Add_Smoke(pos, 3, 20);
		AddRandomSmoke(*io, 20);
		MakeCoolFx(io->pos);
		io->show = SHOW_FLAG_IN_SCENE;
		
		for(long kl = 0; kl < 10; kl++) {
			FaceTarget2(io);
		}
		
		io->gameFlags &= ~GFLAG_INVISIBILITY;
		st->count = 1;
	} else {
		st->count++;
		st->interval = GameDurationMsf(toMsf(st->interval) * 0.5f);
		if(st->interval < GameDurationMs(100))
			st->interval = GameDurationMs(100);
		
		return true;
	}
	
	return false;
}

void ARX_SCRIPT_Timer_Check() {
	
	ARX_PROFILE_FUNC();
	
	if(g_activeScriptTimers == 0) {
		return;
	}
	
	BOOST_FOREACH(SCR_TIMER & timer, g_scriptTimers) {
		
		if(!timer.exist) {
			continue;
		}
		
		GameInstant now = g_gameTime.now();
		GameInstant fire_time = timer.start + timer.interval;
		arx_assert(timer.start <= now);
		if(fire_time > now) {
			// Timer not ready to fire yet
			continue;
		}
		
		// Skip heartbeat timer events for far away objects
		if(timer.idle && !(timer.io->gameFlags & GFLAG_ISINTREATZONE)) {
			s64 increment = toMsi(now - timer.start) / toMsi(timer.interval);
			timer.start += GameDurationMs(toMsi(timer.interval) * increment);
			// TODO print full 64-bit time
			arx_assert_msg(timer.start <= now && timer.start + timer.interval > now,
			               "start=%ld wait=%ld now=%ld",
			               long(toMsi(timer.start)), long(toMsi(timer.interval)), long(toMsi(now)));
			continue;
		}
		
		const EERIE_SCRIPT * es = timer.es;
		Entity * io = timer.io;
		size_t pos = timer.pos;
		
		if(!es && Manage_Specific_RAT_Timer(&timer)) {
			continue;
		}
		
		#ifdef ARX_DEBUG
		std::string name = timer.name;
		#endif
		
		if(timer.count == 1) {
			clearTimer(timer);
		} else {
			if(timer.count != 0) {
				timer.count--;
			}
			timer.start += timer.interval;
		}
		
		if(es && ValidIOAddress(io)) {
			LogDebug("running timer \"" << name << "\" for entity " << io->idString());
			ScriptEvent::resume(es, io, pos);
		} else {
			LogDebug("could not run timer \"" << name << "\" - entity vanished");
		}
		
	}
}

void ARX_SCRIPT_Init_Event_Stats() {
	
	ScriptEvent::totalCount = 0;
	
	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * e = entities[handle];
		
		if(e) {
			e->stat_count = 0;
			e->stat_sent = 0;
		}
	}
}

Entity * ARX_SCRIPT_Get_IO_Max_Events() {
	
	long max = -1;
	Entity * result = NULL;
	
	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * e = entities[handle];
		
		if(e && e->stat_count > max) {
			result = e;
			max = e->stat_count;
		}
	}
	
	return result;
}

Entity * ARX_SCRIPT_Get_IO_Max_Events_Sent() {
	
	long max = -1;
	Entity * result = NULL;
	
	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * e = entities[handle];
		
		if(e && e->stat_sent > max) {
			result = e;
			max = e->stat_sent;
		}
	}
	
	return result;
}

void ManageCasseDArme(Entity * io) {
	
	if(!(io->type_flags & (OBJECT_TYPE_DAGGER | OBJECT_TYPE_1H | OBJECT_TYPE_2H | OBJECT_TYPE_BOW)))
		return;
	
	arx_assert(player.m_bags >= 0);
	arx_assert(player.m_bags <= 3);
	
	Entity * pObjMin = NULL;
	Entity * pObjMax = NULL;
	Entity * pObjFIX = NULL;
	
	for(size_t bag = 0; bag < size_t(player.m_bags); bag++)
	for(size_t y = 0; y < INVENTORY_Y; y++)
	for(size_t x = 0; x < INVENTORY_X; x++) {
		Entity * bagEntity = g_inventory[bag][x][y].io;
		
		if(bagEntity && bagEntity != io
		   && (bagEntity->type_flags & (OBJECT_TYPE_DAGGER | OBJECT_TYPE_1H | OBJECT_TYPE_2H | OBJECT_TYPE_BOW))
		) {
			
			if(   (io->ioflags & IO_ITEM)
			   && (bagEntity->ioflags & IO_ITEM)
			   && bagEntity->_itemdata->equipitem
			) {
				if(bagEntity->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Damages].value == io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Damages].value) {
					pIOChangeWeapon = bagEntity;
					lChangeWeapon = 2;
					return;
				} else {
					if(bagEntity->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Damages].value > io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Damages].value) {
						if(pObjMin) {
							if(bagEntity->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Damages].value < pObjMin->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Damages].value) {
								pObjMin = bagEntity;
							}
						} else {
							pObjMin = bagEntity;
						}
					} else {
						if(bagEntity->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Damages].value < io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Damages].value) {
							if(pObjMax) {
								if(bagEntity->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Damages].value > pObjMax->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Damages].value) {
									pObjMax = bagEntity;
								}
							} else {
								pObjMax = bagEntity;
							}
						}
					}
				}
			} else {
				if(!pObjFIX) {
					pObjFIX = bagEntity;
				}
			}
		}
		
		if(pObjMax) {
			pIOChangeWeapon = pObjMax;
			lChangeWeapon = 2;
		} else {
			if(pObjMin) {
				pIOChangeWeapon = pObjMin;
				lChangeWeapon = 2;
			} else {
				if(pObjFIX) {
					pIOChangeWeapon = pObjFIX;
					lChangeWeapon = 2;
				}
			}
		}
	}
}

void loadScript(EERIE_SCRIPT & script, PakFile * file) {
	
	if(!file) {
		return;
	}
	
	script.valid = true;
	
	script.data = file->read();
	
	boost::to_lower(script.data);
	
	ARX_SCRIPT_ComputeShortcuts(script);
	
}
