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
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#include "script/Script.h"

#include <stddef.h>

#include <sstream>
#include <cstdio>
#include <algorithm>
#include <limits>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/foreach.hpp>

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


#define MAX_SSEPARAMS 5

extern long lChangeWeapon;
extern Entity * pIOChangeWeapon;

Entity * LASTSPAWNED = NULL;
Entity * EVENT_SENDER = NULL;
SCRIPT_VARIABLES svar;

static char SSEPARAMS[MAX_SSEPARAMS][64];
long FORBID_SCRIPT_IO_CREATION = 0;
SCR_TIMER * scr_timer = NULL;
long ActiveTimers = 0;

long FindScriptPos(const EERIE_SCRIPT * es, const std::string & str) {
	
	// TODO(script-parser) remove, respect quoted strings
	
	const char * start = es->data;
	const char * end = es->data + es->size;
	
	while(true) {
		
		const char * dat = std::search(start, end, str.begin(), str.end());
		if(dat + str.length() >= end) {
			return -1;
		}
		
		start = dat + 1;
		if(((unsigned char)dat[str.length()]) > 32) {
			continue;
		}
		
		// Check if the line is commented out!
		for(const char * search = dat; search[0] != '/' || search[1] != '/'; search--) {
			if(*search == '\n' || search == es->data) {
				return dat - es->data;
			}
		}
		
	}
	
	return -1;
}

ScriptResult SendMsgToAllIO(ScriptMessage msg, const std::string & params) {
	
	ScriptResult ret = ACCEPT;
	
	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * e = entities[handle];
		
		if(e) {
			if(SendIOScriptEvent(e, msg, params) == REFUSE) {
				ret = REFUSE;
			}
		}
	}
	
	return ret;
}

void ARX_SCRIPT_SetMainEvent(Entity * io, const std::string & newevent) {
	
	if(!io) {
		return;
	}
	
	if(newevent == "main") {
		io->mainevent.clear();
	} else {
		io->mainevent = newevent;
	}
}

//*************************************************************************************
//*************************************************************************************
void ARX_SCRIPT_ResetObject(Entity * io, bool init) {
	
	if(!io)
		return;
	
	// Now go for Script INIT/RESET depending on Mode
	EntityHandle num = io->index();
	
	if(entities[num] && entities[num]->script.data) {
		entities[num]->script.allowevents = 0;
		if(init)
			ScriptEvent::send(&entities[num]->script, SM_INIT, "", entities[num], "");
		if(entities[num])
			ARX_SCRIPT_SetMainEvent(entities[num], "main");
	}
	
	// Do the same for Local Script
	if(entities[num] && entities[num]->over_script.data) {
		entities[num]->over_script.allowevents = 0;
		if(init)
			ScriptEvent::send(&entities[num]->over_script, SM_INIT, "", entities[num], "");
	}
	
	// Sends InitEnd Event
	if(init) {
		if(entities[num] && entities[num]->script.data)
			ScriptEvent::send(&entities[num]->script, SM_INITEND, "", entities[num], "");
		if(entities[num] && entities[num]->over_script.data)
			ScriptEvent::send(&entities[num]->over_script, SM_INITEND, "", entities[num], "");
	}
	
	if(entities[num])
		entities[num]->gameFlags &= ~GFLAG_NEEDINIT;
	
}

void ARX_SCRIPT_Reset(Entity * io, bool init) {
	
	//Release Script Local Variables
	io->script.lvar.clear();
		
	//Release Script Over-Script Local Variables
	io->over_script.lvar.clear();
	
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
	
	if(arxtime.is_paused()) {
		return;
	}
	
	EVENT_SENDER = NULL;
	
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
		
		if(!entities[i]->mainevent.empty()) {
			
			// Copy the even name to a local variable as it may change during execution
			// and cause unexpected behavior in SendIOScriptEvent
			std::string event = entities[i]->mainevent;
			
			SendIOScriptEvent(entities[i], SM_NULL, std::string(), event);
			
		} else {
			SendIOScriptEvent(entities[i], SM_MAIN);
		}
	}
}

static void ARX_SCRIPT_ReleaseLabels(EERIE_SCRIPT * es) {
	
	if(!es || !es->labels) {
		return;
	}
	
	for(long i = 0; i < es->nb_labels; i++) {
		free(es->labels[i].string);
	}
	free(es->labels);
	es->labels = NULL;
	es->nb_labels = 0;
}

void ReleaseScript(EERIE_SCRIPT * es) {
	
	if(!es) {
		return;
	}
	
	es->lvar.clear();
	
	free(es->data);
	es->data = NULL;
	
	ARX_SCRIPT_ReleaseLabels(es);
	memset(es->shortcut, 0, sizeof(long) * MAX_SHORTCUT);
}

ValueType getSystemVar(const EERIE_SCRIPT * es, Entity * entity, const std::string & name,
                       std::string& txtcontent, float * fcontent,long * lcontent) {
	
	arx_assert(!name.empty() && name[0] == '^', "bad system variable: \"%s\"", name.c_str());
	
	char c = (name.length() < 2) ? '\0' : name[1];
	switch(c) {
		
		case '$': {
			
			if(name == "^$param1") {
				txtcontent = SSEPARAMS[0];
				return TYPE_TEXT;
			}
			
			if(name == "^$param2") {
				txtcontent = SSEPARAMS[1];
				return TYPE_TEXT;
			}
			
			if(name == "^$param3") {
				txtcontent = SSEPARAMS[2];
				return TYPE_TEXT;
			}
			
			if(name == "^$objontop") {
				txtcontent = "none";
				if(entity) {
					MakeTopObjString(entity, txtcontent);
				}
				return TYPE_TEXT;
			}
			
			break;
		}
		
		case '&': {
			
			if(name == "^&param1") {
				*fcontent = (float)atof(SSEPARAMS[0]);
				return TYPE_FLOAT;
			}
			
			if(name == "^&param2") {
				*fcontent = (float)atof(SSEPARAMS[1]);
				return TYPE_FLOAT;
			}
			
			if(name == "^&param3") {
				*fcontent = (float)atof(SSEPARAMS[2]);
				return TYPE_FLOAT;
			}
			
			if(name == "^&playerdist") {
				if(entity) {
					*fcontent = fdist(player.pos, entity->pos);
					return TYPE_FLOAT;
				}
			}
			
			break;
		}
		
		case '#': {
			
			if(name == "^#playerdist") {
				if(entity) {
					*lcontent = (long)fdist(player.pos, entity->pos);
					return TYPE_LONG;
				}
			}
			
			if(name == "^#param1") {
				*lcontent = atol(SSEPARAMS[0]);
				return TYPE_LONG;
			}
			
			if(name == "^#param2") {
				*lcontent = atol(SSEPARAMS[1]);
				return TYPE_LONG;
			}
			
			if(name == "^#param3") {
				*lcontent = atol(SSEPARAMS[2]);
				return TYPE_LONG;
			}
			
			if(name == "^#timer1") {
				if(!entity || entity->script.timers[0] == 0) {
					*lcontent = 0;
				} else {
					*lcontent = long(arxtime.now() - es->timers[0]);
				}
				return TYPE_LONG;
			}
			
			if(name == "^#timer2") {
				if(!entity || entity->script.timers[1] == 0) {
					*lcontent = 0;
				} else {
					*lcontent = long(arxtime.now() - es->timers[1]);
				}
				return TYPE_LONG;
			}
			
			if(name == "^#timer3") {
				if(!entity || entity->script.timers[2] == 0) {
					*lcontent = 0;
				} else {
					*lcontent = long(arxtime.now() - es->timers[2]);
				}
				return TYPE_LONG;
			}
			
			if(name == "^#timer4") {
				if(!entity || entity->script.timers[3] == 0) {
					*lcontent = 0;
				} else {
					*lcontent = long(arxtime.now() - es->timers[3]);
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
				*lcontent = static_cast<long>(arxtime.now_f() / 86400000);
				return TYPE_LONG;
			}
			
			if(name == "^gamehours") {
				*lcontent = static_cast<long>(arxtime.now_f() / 3600000);
				return TYPE_LONG;
			}
			
			if(name == "^gameminutes") {
				*lcontent = static_cast<long>(arxtime.now_f() / 60000);
				return TYPE_LONG;
			}
			
			if(name == "^gameseconds") {
				*lcontent = static_cast<long>(arxtime.now_f() / 1000);
				return TYPE_LONG;
			}
			
			break;
		}
		
		case 'a': {
			
			if(boost::starts_with(name, "^amount")) {
				if(entity && (entity->ioflags & IO_ITEM)) {
					*fcontent = entity->_itemdata->count;
				} else {
					*fcontent = 0;
				}
				return TYPE_FLOAT;
			}
			
			if(name == "^arxdays") {
				*lcontent = static_cast<long>(arxtime.now_f() / 7200000);
				return TYPE_LONG;
			}
			
			if(name == "^arxhours") {
				*lcontent = static_cast<long>(arxtime.now_f() / 600000);
				return TYPE_LONG;
			}
			
			if(name == "^arxminutes") {
				*lcontent = static_cast<long>(arxtime.now_f() / 10000);
				return TYPE_LONG;
			}
			
			if(name == "^arxseconds") {
				*lcontent = static_cast<long>(arxtime.now_f() / 1000) * 6;
				return TYPE_LONG;
			}
			
			if(name == "^arxtime_hours") {
				*lcontent = static_cast<long>(arxtime.now_f() / 600000);
				while(*lcontent > 12) {
					*lcontent -= 12;
				}
				return TYPE_LONG;
			}
			
			if(name == "^arxtime_minutes") {
				*lcontent = static_cast<long>(arxtime.now_f() / 10000);
				while(*lcontent > 60) {
					*lcontent -= 60;
				}
				return TYPE_LONG;
			}
			
			if(name == "^arxtime_seconds") {
				*lcontent = static_cast<long>(arxtime.now_f() * 6 / 1000);
				while(*lcontent > 60) {
					*lcontent -= 60;
				}
				return TYPE_LONG;
			}
			
			break;
		}
		
		case 'r': {
			
			if(boost::starts_with(name, "^realdist_")) {
				if(entity) {
					const char * obj = name.c_str() + 10;
					
					if(!strcmp(obj, "player")) {
						if(entity->requestRoomUpdate) {
							UpdateIORoom(entity);
						}
						long Player_Room = ARX_PORTALS_GetRoomNumForPosition(player.pos, 1);
						*fcontent = SP_GetRoomDist(entity->pos, player.pos, entity->room, Player_Room);
						return TYPE_FLOAT;
					}
					
					EntityHandle t = entities.getById(obj);
					if(ValidIONum(t)) {
						if((entity->show == SHOW_FLAG_IN_SCENE
						    || entity->show == SHOW_FLAG_IN_INVENTORY)
						   && (entities[t]->show == SHOW_FLAG_IN_SCENE
						       || entities[t]->show == SHOW_FLAG_IN_INVENTORY)) {
							
							Vec3f pos  = GetItemWorldPosition(entity);
							Vec3f pos2 = GetItemWorldPosition(entities[t]);
							
							if(entity->requestRoomUpdate) {
								UpdateIORoom(entity);
							}
							
							if(entities[t]->requestRoomUpdate) {
								UpdateIORoom(entities[t]);
							}
							
							*fcontent = SP_GetRoomDist(pos, pos2, entity->room, entities[t]->room);
							
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
					*fcontent = ARX_DAMAGES_ComputeRepairPrice(entities[t], entity);
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
					float t = (float)atof(max);
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
				if(entity && ap) {
					if(ARX_PATH_IsPosInZone(ap, entity->pos)) {
						*lcontent = 1;
					}
				}
				return TYPE_LONG;
			}
			
			if(boost::starts_with(name, "^ininitpos")) {
				*lcontent = 0;
				if(entity) {
					Vec3f pos = GetItemWorldPosition(entity);
					if(pos == entity->initpos)
						*lcontent = 1;
				}
				return TYPE_LONG;
			}
			
			if(boost::starts_with(name, "^inplayerinventory")) {
				*lcontent = 0;
				if(entity && (entity->ioflags & IO_ITEM) && IsInPlayerInventory(entity)) {
					*lcontent = 1;
				}
				return TYPE_LONG;
			}
			
			break;
		}
		
		case 'b': {
			
			if(boost::starts_with(name, "^behavior")) {
				txtcontent = "";
				if(entity && (entity->ioflags & IO_NPC)) {
					if(entity->_npcdata->behavior & BEHAVIOUR_LOOK_AROUND) {
						txtcontent += "l";
					}
					if(entity->_npcdata->behavior & BEHAVIOUR_SNEAK) {
						txtcontent += "s";
					}
					if(entity->_npcdata->behavior & BEHAVIOUR_DISTANT) {
						txtcontent += "d";
					}
					if(entity->_npcdata->behavior & BEHAVIOUR_MAGIC) {
						txtcontent += "m";
					}
					if(entity->_npcdata->behavior & BEHAVIOUR_FIGHT) {
						txtcontent += "f";
					}
					if(entity->_npcdata->behavior & BEHAVIOUR_GO_HOME) {
						txtcontent += "h";
					}
					if(entity->_npcdata->behavior & BEHAVIOUR_FRIENDLY) {
						txtcontent += "r";
					}
					if(entity->_npcdata->behavior & BEHAVIOUR_MOVE_TO) {
						txtcontent += "t";
					}
					if(entity->_npcdata->behavior & BEHAVIOUR_FLEE) {
						txtcontent += "e";
					}
					if(entity->_npcdata->behavior & BEHAVIOUR_LOOK_FOR) {
						txtcontent += "o";
					}
					if(entity->_npcdata->behavior & BEHAVIOUR_HIDE) {
						txtcontent += "i";
					}
					if(entity->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND) {
						txtcontent += "w";
					}
					if(entity->_npcdata->behavior & BEHAVIOUR_GUARD) {
						txtcontent += "u";
					}
					if(entity->_npcdata->behavior & BEHAVIOUR_STARE_AT) {
						txtcontent += "a";
					}
				}
				return TYPE_TEXT;
			}
			
			break;
		}
		
		case 's': {
			
			if(boost::starts_with(name, "^sender")) {
				if(!EVENT_SENDER) {
					txtcontent = "none";
				} else if(EVENT_SENDER == entities.player()) {
					txtcontent = "player";
				} else {
					txtcontent = EVENT_SENDER->idString();
				}
				return TYPE_TEXT;
			}
			
			if(boost::starts_with(name, "^scale")) {
				*fcontent = (entity) ? entity->scale * 100.f : 0.f;
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^speaking")) {
				if(entity) {
					for(size_t i = 0; i < MAX_ASPEECH; i++) {
						if(aspeech[i].exist && entity == aspeech[i].io) {
							*lcontent = 1;
							return TYPE_LONG;
						}
					}
				}
				*lcontent = 0;
				return TYPE_LONG;
			}
			
			break;
		}
		
		case 'm': {
			
			if(boost::starts_with(name, "^me")) {
				if(!entity) {
					txtcontent = "none";
				} else if(entity == entities.player()) {
					txtcontent = "player";
				} else {
					txtcontent = entity->idString();
				}
				return TYPE_TEXT;
			}
			
			if(boost::starts_with(name, "^maxlife")) {
				*fcontent = 0;
				if(entity && (entity->ioflags & IO_NPC)) {
					*fcontent = entity->_npcdata->lifePool.max;
				}
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^mana")) {
				*fcontent = 0;
				if(entity && (entity->ioflags & IO_NPC)) {
					*fcontent = entity->_npcdata->manaPool.current;
				}
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^maxmana")) {
				*fcontent = 0;
				if(entity && (entity->ioflags & IO_NPC)) {
					*fcontent = entity->_npcdata->manaPool.max;
				}
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^myspell_")) {
				SpellType id = GetSpellId(name.substr(9));
				if(id != SPELL_NONE) {
					if(spells.ExistAnyInstanceForThisCaster(id, entity->index())) {
						*lcontent = 1;
						return TYPE_LONG;
					}
				}
				*lcontent = 0;
				return TYPE_LONG;
			}
			
			if(boost::starts_with(name, "^maxdurability")) {
				*fcontent = (entity) ? entity->max_durability : 0.f;
				return TYPE_FLOAT;
			}
			
			break;
		}
		
		case 'l': {
			
			if(boost::starts_with(name, "^life")) {
				*fcontent = 0;
				if(entity && (entity->ioflags & IO_NPC)) {
					*fcontent = entity->_npcdata->lifePool.current;
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
				if(entity) {
					const char * obj = name.c_str() + 6;
					
					if(!strcmp(obj, "player")) {
						*fcontent = fdist(player.pos, entity->pos);
						return TYPE_FLOAT;
					}
					
					EntityHandle t = entities.getById(obj);
					if(ValidIONum(t)) {
						if((entity->show == SHOW_FLAG_IN_SCENE
						    || entity->show == SHOW_FLAG_IN_INVENTORY)
						   && (entities[t]->show == SHOW_FLAG_IN_SCENE
						       || entities[t]->show == SHOW_FLAG_IN_INVENTORY)) {
							Vec3f pos  = GetItemWorldPosition(entity);
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
				*lcontent = (resources->getReleaseType() & PakReader::Demo) ? 1 : 0;
				return TYPE_LONG;
			}
			
			if(boost::starts_with(name, "^durability")) {
				*fcontent = (entity) ? entity->durability : 0.f;
				return TYPE_FLOAT;
			}
			
			break;
		}
		
		case 'p': {
			
			if(boost::starts_with(name, "^price")) {
				*fcontent = 0;
				if(entity && (entity->ioflags & IO_ITEM)) {
					*fcontent = static_cast<float>(entity->_itemdata->price);
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
				if(entity && (entity->ioflags & IO_NPC)) {
					*fcontent = entity->_npcdata->poisonned;
				}
				return TYPE_FLOAT;
			}
			
			if(boost::starts_with(name, "^poisonous")) {
				*fcontent = (entity) ? entity->poisonous : 0.f;
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
					
					if(spell && spell->m_caster == PlayerEntityHandle) {
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
					if(spells.ExistAnyInstanceForThisCaster(id, PlayerEntityHandle)) {
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
				Entity * ioo = ARX_NPC_GetFirstNPCInSight(entity);
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
				if(!entity) {
					txtcontent = "none";
				} else if(entity->targetinfo == PlayerEntityHandle) {
					txtcontent = "player";
				} else if(!ValidIONum(entity->targetinfo)) {
					txtcontent = "none";
				} else {
					txtcontent = entities[entity->targetinfo]->idString();
				}
				return TYPE_TEXT;
			}
			
			break;
		}
		
		case 'f': {
			
			if(boost::starts_with(name, "^focal")) {
				if(entity && (entity->ioflags & IO_CAMERA)) {
					*fcontent = entity->_camdata->cam.focal;
					return TYPE_FLOAT;
				}
			}
			
			if(boost::starts_with(name, "^fighting")) {
				*lcontent = long(ARX_PLAYER_IsInFightMode());
				return TYPE_LONG;
			}
			
			break;
		}
		
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
	
	ioo->script.lvar = io->script.lvar;
}

static SCRIPT_VAR * GetFreeVarSlot(SCRIPT_VARIABLES& _svff) {
	_svff.resize(_svff.size() + 1);
	SCRIPT_VAR * v = &_svff.back();
	return v;
}

static SCRIPT_VAR * GetVarAddress(SCRIPT_VARIABLES & svf, const std::string & name) {
	
	for(SCRIPT_VARIABLES::iterator it = svf.begin(); it != svf.end(); ++it) {
		if(it->type != TYPE_UNKNOWN) {
			if(name == it->name) {
				return &(*it);
			}
		}
	}

	return NULL;
}

static const SCRIPT_VAR * GetVarAddress(const SCRIPT_VARIABLES & svf,
                                        const std::string & name) {
	
	for(SCRIPT_VARIABLES::const_iterator it = svf.begin(); it != svf.end(); ++it) {
		if(it->type != TYPE_UNKNOWN) {
			if(name == it->name) {
				return &(*it);
			}
		}
	}

	return NULL;
}

long GETVarValueLong(const SCRIPT_VARIABLES& svf, const std::string & name) {
	
	const SCRIPT_VAR * tsv = GetVarAddress(svf, name);

	if (tsv == NULL) return 0;

	return tsv->ival;
}

float GETVarValueFloat(const SCRIPT_VARIABLES& svf, const std::string & name) {
	
	const SCRIPT_VAR * tsv = GetVarAddress(svf, name);

	if (tsv == NULL) return 0;

	return tsv->fval;
}

std::string GETVarValueText(const SCRIPT_VARIABLES& svf, const std::string & name) {
	
	const SCRIPT_VAR* tsv = GetVarAddress(svf, name);

	if (!tsv) return "";

	return tsv->text;
}

std::string GetVarValueInterpretedAsText(const std::string & temp1, const EERIE_SCRIPT * esss, Entity * io) {
	
	char var_text[256];
	float t1;

	if(!temp1.empty())
	{
		if (temp1[0] == '^')
		{
			long lv;
			float fv;
			std::string tv;

			switch (getSystemVar(esss,io,temp1,tv,&fv,&lv))//Arx: xrichter (2010-08-04) - fix a crash when $OBJONTOP return to many object name inside tv
			{
				case TYPE_TEXT:
					return tv;
					break;
				case TYPE_LONG:
					sprintf(var_text, "%ld", lv);
					return var_text;
					break;
				default:
					sprintf(var_text, "%f", double(fv));
					return var_text;
					break;
			}

		}
		else if (temp1[0] == '#')
		{
			long l1 = GETVarValueLong(svar, temp1);
			sprintf(var_text, "%ld", l1);
			return var_text;
		}
		else if (temp1[0] == '\xA7')
		{
			long l1 = GETVarValueLong(esss->lvar, temp1);
			sprintf(var_text, "%ld", l1);
			return var_text;
		}
		else if (temp1[0] == '&') t1 = GETVarValueFloat(svar, temp1);
		else if (temp1[0] == '@') t1 = GETVarValueFloat(esss->lvar, temp1);
		else if (temp1[0] == '$')
		{
			const SCRIPT_VAR * var = GetVarAddress(svar, temp1);

			if (!var) return "void";
			else return var->text;
		}
		else if (temp1[0] == '\xA3')
		{
			const SCRIPT_VAR * var = GetVarAddress(esss->lvar, temp1);

			if (!var) return "void";
			else return var->text;
		}
		else
		{
			return temp1;
		}
	}
	else
	{
		return "";
	}

	sprintf(var_text, "%f", double(t1));
	return var_text;
}

float GetVarValueInterpretedAsFloat(const std::string & temp1, const EERIE_SCRIPT * esss, Entity * io) {
	
	if(temp1[0] == '^') {
		long lv;
		float fv;
		std::string tv; 
		switch (getSystemVar(esss,io,temp1,tv,&fv,&lv)) {
			case TYPE_TEXT:
				return (float)atof(tv.c_str());
			case TYPE_LONG:
				return (float)lv;
			default:
				return fv;
		}
	} else if(temp1[0] == '#') {
		return (float)GETVarValueLong(svar, temp1);
	} else if(temp1[0] == '\xA7') {
		return (float)GETVarValueLong(esss->lvar, temp1);
	} else if(temp1[0] == '&') {
		return GETVarValueFloat(svar, temp1);
	} else if(temp1[0] == '@') {
		return GETVarValueFloat(esss->lvar, temp1);
	}
	
	return (float)atof(temp1.c_str());
}

SCRIPT_VAR* SETVarValueLong(SCRIPT_VARIABLES& svf, const std::string& name, long val)
{
	SCRIPT_VAR* tsv = GetVarAddress(svf, name);

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

SCRIPT_VAR* SETVarValueFloat(SCRIPT_VARIABLES& svf, const std::string& name, float val)
{
	SCRIPT_VAR* tsv = GetVarAddress(svf, name);

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

SCRIPT_VAR* SETVarValueText(SCRIPT_VARIABLES& svf, const std::string& name, const std::string& val)
{
	SCRIPT_VAR* tsv = GetVarAddress(svf, name);

	if (!tsv)
	{
		tsv = GetFreeVarSlot(svf);

		if (!tsv)
			return NULL;

		tsv->name = name.c_str();
	}
	
	tsv->text = val;
	
	return tsv;
}





void MakeGlobalText(std::string & tx)
{
	char texx[256];

	for(size_t i = 0; i < svar.size(); i++) {
		switch(svar[i].type) {
			case TYPE_G_TEXT:
				tx += svar[i].name;
				tx += " = ";
				tx += svar[i].text;
				tx += "\r\n";
				break;
			case TYPE_G_LONG:
				tx += svar[i].name;
				tx += " = ";
				sprintf(texx, "%ld", svar[i].ival);
				tx += texx;
				tx += "\r\n";
				break;
			case TYPE_G_FLOAT:
				tx += svar[i].name;
				tx += " = ";
				sprintf(texx, "%f", double(svar[i].fval));
				tx += texx;
				tx += "\r\n";
				break;
			case TYPE_UNKNOWN:
			case TYPE_L_TEXT:
			case TYPE_L_LONG:
			case TYPE_L_FLOAT:
				break;
		}
	}
}

void MakeLocalText(EERIE_SCRIPT * es, std::string& tx)
{
	char texx[256];

	if (es->master != NULL) es = es->master;

	for(SCRIPT_VARIABLES::const_iterator it = es->lvar.begin(); it != es->lvar.end(); ++it)
	{
		const SCRIPT_VAR& v = *it;
		switch (v.type)
		{
			case TYPE_L_TEXT:
				tx += v.name;
				tx += " = ";
				tx += v.text;
				tx += "\r\n";
				break;
			case TYPE_L_LONG:
				tx += v.name;
				tx += " = ";
				sprintf(texx, "%ld", v.ival);
				tx += texx;
				tx += "\r\n";
				break;
			case TYPE_L_FLOAT:
				tx += v.name;
				tx += " = ";
				sprintf(texx, "%f", double(v.fval));
				tx += texx;
				tx += "\r\n";
				break;
			case TYPE_UNKNOWN:
			case TYPE_G_TEXT:
			case TYPE_G_LONG:
			case TYPE_G_FLOAT:
				break;
		}
	}
}

//*************************************************************************************
// ScriptEvent::send																	//
// Sends a event to a script.														//
// returns ACCEPT to accept default EVENT processing								//
// returns REFUSE to refuse default EVENT processing								//
//*************************************************************************************
void MakeSSEPARAMS(const char * params)
{
	
	for (long i = 0; i < MAX_SSEPARAMS; i++)
	{
		SSEPARAMS[i][0] = 0;
	}

	if(params == NULL) {
		return;
	}

	long pos = 0;

	while(*params != '\0' && pos < MAX_SSEPARAMS) {
		
		size_t tokensize = 0;
		while(params[tokensize] != ' ' && params[tokensize] != '\0') {
			tokensize++;
		}
		
		arx_assert(tokensize < 64 - 1);
		memcpy(SSEPARAMS[pos], params, tokensize);
		SSEPARAMS[pos][tokensize] = 0;
		
		params += tokensize;
		
		if(*params != '\0') {
			params++;
		}
		
		pos++;
	}
}

struct QueuedEvent {
	
	bool          exists;
	Entity *      sender;
	Entity *      entity;
	ScriptMessage msg;
	std::string   params;
	std::string   eventname;
	
	void clear() {
		exists = false;
		sender = NULL;
		entity = NULL;
		msg = SM_NULL;
		params.clear();
		eventname.clear();
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
			LogDebug("clearing queued " << ScriptEvent::getName(event.msg, event.eventname)
			         << " for " << io->idString());
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
			EVENT_SENDER = ValidIOAddress(event.sender) ? event.sender : NULL;
			LogDebug("running queued " << ScriptEvent::getName(event.msg, event.eventname)
			         << " for " << event.entity->idString());
			SendIOScriptEvent(event.entity, event.msg, event.params, event.eventname);
		} else {
			LogDebug("could not run queued " << ScriptEvent::getName(event.msg, event.eventname)
			         << " params=\"" << event.params << "\" - entity vanished");
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

void Stack_SendIOScriptEvent(Entity * io, ScriptMessage msg, const std::string & params,
                             const std::string & eventname) {
	BOOST_FOREACH(QueuedEvent & event, g_eventQueue) {
		if(!event.exists) {
			event.sender = EVENT_SENDER;
			event.entity = io;
			event.msg = msg;
			event.params = params;
			event.eventname = eventname;
			event.exists = true;
			return;
		}
	}
}

static ScriptResult SendIOScriptEventReverse(Entity * io, ScriptMessage msg, const std::string& params, const std::string& eventname)
{
	// checks invalid IO
	if (!io) return REFUSE;

	EntityHandle num = io->index();
	
	// if this IO only has a Local script, send event to it
	if (entities[num] && !entities[num]->over_script.data)
	{
		return ScriptEvent::send(&entities[num]->script, msg, params, entities[num], eventname);
	}
	
	// If this IO has a Global script send to Local (if exists)
	// then to local if no overriden by Local
	if (entities[num] && (ScriptEvent::send(&entities[num]->script, msg, params, entities[num], eventname) != REFUSE))
	{
	
		if (entities[num])
			return (ScriptEvent::send(&entities[num]->over_script, msg, params, entities[num], eventname));
		else
			return REFUSE;
	}

	// Refused further processing.
	return REFUSE;
}

ScriptResult SendIOScriptEvent(Entity * io, ScriptMessage msg, const std::string& params, const std::string& eventname)
{
	
	ARX_PROFILE_FUNC();

	if(!io) {
		return REFUSE;
	}
	
	EntityHandle num = io->index();
	
	Entity * oes = EVENT_SENDER;

	if ((msg == SM_INIT) || (msg == SM_INITEND))
	{
		if (entities[num])
		{
			SendIOScriptEventReverse(entities[num], msg, params, eventname);
			EVENT_SENDER = oes;
		}
	}

	// if this IO only has a Local script, send event to it
	if (entities[num] && !entities[num]->over_script.data)
	{
		ScriptResult ret = ScriptEvent::send(&entities[num]->script, msg, params, entities[num], eventname);
		EVENT_SENDER = oes;
		return ret;
	}

	// If this IO has a Global script send to Local (if exists)
	// then to Global if no overriden by Local
	if (entities[num] && ScriptEvent::send(&entities[num]->over_script, msg, params, entities[num], eventname) != REFUSE) {
		EVENT_SENDER = oes;

		if (entities[num])
		{
			ScriptResult ret = ScriptEvent::send(&entities[num]->script, msg, params, entities[num], eventname);
			EVENT_SENDER = oes;
			return ret;
		}
		else
			return REFUSE;
	}

	// Refused further processing.
	return REFUSE;
}

ScriptResult SendInitScriptEvent(Entity * io) {
	
	if (!io) return REFUSE;

	Entity * oes = EVENT_SENDER;
	EVENT_SENDER = NULL;
	EntityHandle num = io->index();

	if (entities[num] && entities[num]->script.data)
	{
		ScriptEvent::send(&entities[num]->script, SM_INIT, "", entities[num], "");
	}

	if (entities[num] && entities[num]->over_script.data)
	{
		ScriptEvent::send(&entities[num]->over_script, SM_INIT, "", entities[num], "");
	}

	if (entities[num] && entities[num]->script.data)
	{
		ScriptEvent::send(&entities[num]->script, SM_INITEND, "", entities[num], "");
	}

	if (entities[num] && entities[num]->over_script.data)
	{
		ScriptEvent::send(&entities[num]->over_script, SM_INITEND, "", entities[num], "");
	}

	EVENT_SENDER = oes;
	return ACCEPT;
}

//! Checks if timer named texx exists.
static bool ARX_SCRIPT_Timer_Exist(const std::string & texx) {
	
	for(long i = 0; i < MAX_TIMER_SCRIPT; i++) {
		if(scr_timer[i].exist) {
			if(scr_timer[i].name == texx) {
				return true;
			}
		}
	}
	
	return false;
}

std::string ARX_SCRIPT_Timer_GetDefaultName() {
	
	for(size_t i = 1; ; i++) {
		
		std::ostringstream oss;
		oss << "timer_" << i;
		
		if(!ARX_SCRIPT_Timer_Exist(oss.str())) {
			return oss.str();
		}
	}
}

//*************************************************************************************
// Get a free script timer
//*************************************************************************************
long ARX_SCRIPT_Timer_GetFree() {
	
	for(long i = 0; i < MAX_TIMER_SCRIPT; i++) {
		if(!(scr_timer[i].exist))
			return i;
	}
	
	return -1;
}

//*************************************************************************************
// Count the number of active script timers...
//*************************************************************************************
long ARX_SCRIPT_CountTimers() {
	return ActiveTimers;
}

//*************************************************************************************
// ARX_SCRIPT_Timer_ClearByNum
// Clears a timer by its Index (long timer_idx) on the timers list
//*************************************************************************************
void ARX_SCRIPT_Timer_ClearByNum(long timer_idx) {
	if(scr_timer[timer_idx].exist) {
		LogDebug("clearing timer " << scr_timer[timer_idx].name);
		scr_timer[timer_idx].name.clear();
		ActiveTimers--;
		scr_timer[timer_idx].exist = 0;
	}
}

void ARX_SCRIPT_Timer_Clear_By_Name_And_IO(const std::string & timername, Entity * io) {
	for(long i = 0; i < MAX_TIMER_SCRIPT; i++) {
		if(scr_timer[i].exist && scr_timer[i].io == io && scr_timer[i].name == timername) {
			ARX_SCRIPT_Timer_ClearByNum(i);
		}
	}
}

void ARX_SCRIPT_Timer_Clear_All_Locals_For_IO(Entity * io)
{
	for (long i = 0; i < MAX_TIMER_SCRIPT; i++)
	{
		if (scr_timer[i].exist)
		{
			if ((scr_timer[i].io == io) && (scr_timer[i].es == &io->over_script))
				ARX_SCRIPT_Timer_ClearByNum(i);
		}
	}
}

//*************************************************************************************
// Initialise the timer list for the first time.
//*************************************************************************************
long MAX_TIMER_SCRIPT = 0;
void ARX_SCRIPT_Timer_FirstInit(long number)
{
	if (number < 100) number = 100;
	
	MAX_TIMER_SCRIPT = number;
	
	delete[] scr_timer;
	scr_timer = new SCR_TIMER[MAX_TIMER_SCRIPT];
	ActiveTimers = 0;
}

void ARX_SCRIPT_Timer_ClearAll()
{
	if (ActiveTimers)
		for (long i = 0; i < MAX_TIMER_SCRIPT; i++)
			ARX_SCRIPT_Timer_ClearByNum(i);

	ActiveTimers = 0;
}

void ARX_SCRIPT_Timer_Clear_For_IO(Entity * io) {
	for(long i = 0; i < MAX_TIMER_SCRIPT; i++) {
		if(scr_timer[i].exist && scr_timer[i].io == io) {
			ARX_SCRIPT_Timer_ClearByNum(i);
		}
	}
}

long ARX_SCRIPT_GetSystemIOScript(Entity * io, const std::string & name) {
	
	if(ActiveTimers) {
		for(long i = 0; i < MAX_TIMER_SCRIPT; i++) {
			if(scr_timer[i].exist && scr_timer[i].io == io && scr_timer[i].name == name) {
				return i;
			}
		}
	}
	
	return -1;
}

static bool Manage_Specific_RAT_Timer(SCR_TIMER * st) {
	
	Entity * io = st->io;
	GetTargetPos(io);
	Vec3f target = io->target - io->pos;
	target = glm::normalize(target);
	Vec3f targ = VRotateY(target, Random::getf(-30.f, 30.f));
	target = io->target + targ * 100.f;
	
	if(ARX_INTERACTIVE_ConvertToValidPosForIO(io, &target)) {
		ARX_INTERACTIVE_Teleport(io, target);
		Vec3f pos = io->pos;
		pos.y += io->physics.cyl.height * ( 1.0f / 2 );
		
		ARX_PARTICLES_Add_Smoke(pos, 3, 20);
		AddRandomSmoke(io, 20);
		MakeCoolFx(io->pos);
		io->show = SHOW_FLAG_IN_SCENE;
		
		for(long kl = 0; kl < 10; kl++) {
			FaceTarget2(io);
		}
		
		io->gameFlags &= ~GFLAG_INVISIBILITY;
		st->count = 1;
	} else {
		st->count++;
		st->interval = ArxDuration(st->interval * ( 1.0f / 2 ));
		if(st->interval < ArxDurationMs(100))
			st->interval = ArxDurationMs(100);
		
		return true;
	}
	
	return false;
}

void ARX_SCRIPT_Timer_Check() {
	
	ARX_PROFILE_FUNC();
	
	if(!ActiveTimers) {
		return;
	}
	
	for(long i = 0; i < MAX_TIMER_SCRIPT; i++) {
		
		SCR_TIMER * st = &scr_timer[i];
		if(!st->exist) {
			continue;
		}
		
		ArxInstant now = arxtime.now();
		ArxInstant fire_time = st->start + st->interval;
		arx_assert(st->start <= now);
		if(fire_time > now) {
			// Timer not ready to fire yet
			continue;
		}
		
		// Skip heartbeat timer events for far away objects
		if((st->flags & 1) && !(st->io->gameFlags & GFLAG_ISINTREATZONE)) {
			long increment = (now - st->start) / st->interval;
			st->start += st->interval * increment;
			// TODO print full 64-bit time
			arx_assert(st->start <= now && st->start + st->interval > now,
			           "start=%ld wait=%ld now=%ld",
			           long(toMs(st->start)), long(toMs(st->interval)), long(toMs(now)));
			continue;
		}
		
		EERIE_SCRIPT * es = st->es;
		Entity * io = st->io;
		long pos = st->pos;
		
		if(!es && st->name == "_r_a_t_") {
			if(Manage_Specific_RAT_Timer(st)) {
				continue;
			}
		}
		
		#ifdef ARX_DEBUG
		std::string name = st->name;
		#endif
		
		if(st->count == 1) {
			ARX_SCRIPT_Timer_ClearByNum(i);
		} else {
			if(st->count != 0) {
				st->count--;
			}
			st->start += st->interval;
		}
		
		if(es && ValidIOAddress(io)) {
			LogDebug("running timer \"" << name << "\" for entity " << io->idString());
			ScriptEvent::send(es, SM_EXECUTELINE, "", io, "", pos);
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
	
	arx_assert(player.bag >= 0);
	arx_assert(player.bag <= 3);
	
	Entity * pObjMin = NULL;
	Entity * pObjMax = NULL;
	Entity * pObjFIX = NULL;
	
	for(size_t bag = 0; bag < size_t(player.bag); bag++)
	for(size_t y = 0; y < INVENTORY_Y; y++)
	for(size_t x = 0; x < INVENTORY_X; x++) {
		Entity * bagEntity = inventory[bag][x][y].io;
		
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
	
	free(script.data);
	
	script.data = file->readAlloc();
	script.size = file->size();
	
	std::transform(script.data, script.data + script.size, script.data, ::tolower);
	
	script.allowevents = 0;
	
	script.lvar.clear();
	
	script.master = NULL;
	
	for(size_t j = 0; j < MAX_SCRIPTTIMERS; j++) {
		script.timers[j] = ArxInstant_ZERO;
	}
	
	ARX_SCRIPT_ComputeShortcuts(script);
	
}
