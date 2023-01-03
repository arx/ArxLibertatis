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
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#ifndef ARX_SCRIPT_SCRIPT_H
#define ARX_SCRIPT_SCRIPT_H

#include <stddef.h>
#include <cstring>
#include <limits>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "core/TimeTypes.h"
#include "platform/Platform.h"
#include "util/Flags.h"


class PakFile;
class Entity;

namespace script { class Context; }

enum ScriptMessage {
	// No event
	SM_NULL = 0,
	// Events used by the engine
	SM_ACTION,
	SM_AGGRESSION,
	SM_BACKSTAB,
	SM_BOOK_CLOSE,
	SM_BOOK_OPEN,
	SM_BREAK,
	SM_CHAT,
	SM_CINE_END,
	SM_CLICKED,
	SM_COLLIDE_DOOR,
	SM_COLLIDE_FIELD,
	SM_COLLIDE_NPC,
	SM_COLLISION_ERROR,
	SM_COLLISION_ERROR_DETAIL,
	SM_COMBINE,
	SM_CONTROLLEDZONE_ENTER,
	SM_CONTROLLEDZONE_LEAVE,
	SM_CONTROLS_OFF,
	SM_CONTROLS_ON,
	SM_CRITICAL,
	SM_CURSORMODE,
	SM_CUSTOM,
	SM_DEAD,
	SM_DETECTPLAYER,
	SM_DIE,
	SM_ENTERZONE,
	SM_EQUIPIN,
	SM_EQUIPOUT,
	SM_EXPLORATIONMODE,
	SM_GAME_READY,
	SM_HEAR,
	SM_HIT,
	SM_IDENTIFY,
	SM_INIT,
	SM_INITEND,
	SM_INVENTORY2_CLOSE,
	SM_INVENTORY2_OPEN,
	SM_INVENTORYIN,
	SM_INVENTORYUSE,
	SM_KEY_PRESSED,
	SM_LEAVEZONE,
	SM_LOAD,
	SM_LOSTTARGET,
	SM_MAIN,
	SM_OUCH,
	SM_PATHEND,
	SM_PATHFINDER_FAILURE,
	SM_PATHFINDER_SUCCESS,
	SM_REACHEDTARGET,
	SM_RELOAD,
	SM_SPELLCAST,
	SM_SPELLEND,
	SM_STEAL,
	SM_STRIKE,
	SM_SUMMONED,
	SM_TREATOUT,
	SM_UNDETECTPLAYER,
	SM_WAYPOINT,
	// Events only used by scripts
	// TODO remove once script event with custom names are indexed and sendevent is fixed in scripts
	SM_FIGHT,
	SM_INVENTORYOUT,
	SM_MOVE,
	SM_RESET,
	SM_SPELLDECISION,
	SM_TRAP_DISARMED,
	//! Special event constants
	SM_MAXCMD,
	SM_EXECUTELINE,
	SM_DUMMY
};

enum ValueType {
	TYPE_TEXT = 1,
	TYPE_FLOAT = 2,
	TYPE_LONG = 3
};

struct SCRIPT_VAR {
	
	std::string name;
	
	long ival = 0;
	float fval = 0.f;
	std::string text;
	
};

bool isLocalVariable(std::string_view name);

std::ostream & operator<<(std::ostream & os, const SCRIPT_VAR & var);

enum DisabledEvent {
	DISABLE_HIT             = 1 << 0,
	DISABLE_CHAT            = 1 << 1,
	DISABLE_INVENTORY2_OPEN = 1 << 2,
	DISABLE_HEAR            = 1 << 3,
	DISABLE_DETECT          = 1 << 4,
	DISABLE_AGGRESSION      = 1 << 5,
	DISABLE_MAIN            = 1 << 6,
	DISABLE_COLLIDE_NPC     = 1 << 7,
	DISABLE_CURSORMODE      = 1 << 8,
	DISABLE_EXPLORATIONMODE = 1 << 9
};
DECLARE_FLAGS(DisabledEvent, DisabledEvents)
DECLARE_FLAGS_OPERATORS(DisabledEvents)

typedef std::vector<SCRIPT_VAR> SCRIPT_VARIABLES;

struct EERIE_SCRIPT {
	
	bool valid = false;
	std::string data;
	size_t shortcut[SM_MAXCMD];

	EERIE_SCRIPT() noexcept {
		memset(&shortcut, 0, sizeof(shortcut));
	}
	
};

struct SCR_TIMER {
	
	std::string name;
	short exist;
	bool idle;
	long count;
	GameDuration interval;
	size_t pos;
	GameInstant start;
	Entity * io;
	const EERIE_SCRIPT * es;
	
	explicit SCR_TIMER(Entity * entity = nullptr, std::string && timerName = std::string()) noexcept
		: name(std::move(timerName))
		, exist(entity != nullptr)
		, idle(false)
		, count(0)
		, interval(0)
		, pos(0)
		, start(0)
		, io(entity)
		, es(nullptr)
	{ }
	
};

enum AnimationNumber {
	
	ANIM_NONE = -1,
	
	ANIM_WAIT = 0,
	ANIM_WALK = 1,
	ANIM_WALK2 = 2,
	ANIM_WALK3 = 3,
	ANIM_ACTION = 8,
	ANIM_ACTION2 = 9,
	ANIM_ACTION3 = 10,
	ANIM_HIT1 = 11,
	ANIM_STRIKE1 = 12,
	ANIM_DIE = 13,
	ANIM_WAIT2 = 14,
	ANIM_RUN = 15,
	ANIM_RUN2 = 16,
	ANIM_RUN3 = 17,
	ANIM_ACTION4 = 18,
	ANIM_ACTION5 = 19,
	ANIM_ACTION6 = 20,
	ANIM_ACTION7 = 21,
	ANIM_ACTION8 = 22,
	ANIM_ACTION9 = 23,
	ANIM_ACTION10 = 24,
	ANIM_TALK_NEUTRAL = 30,
	ANIM_TALK_HAPPY = 31,
	ANIM_TALK_ANGRY = 32,
	ANIM_WALK_BACKWARD = 33,
	
	ANIM_BARE_READY = 34,
	ANIM_BARE_UNREADY = ANIM_BARE_READY + 1,
	ANIM_BARE_WAIT = ANIM_BARE_READY + 2,
	ANIM_BARE_STRIKE_LEFT_START = ANIM_BARE_READY + 3,
	ANIM_BARE_STRIKE_LEFT_CYCLE = ANIM_BARE_READY + 4,
	ANIM_BARE_STRIKE_LEFT = ANIM_BARE_READY + 5,
	ANIM_BARE_STRIKE_RIGHT_START = ANIM_BARE_READY + 6,
	ANIM_BARE_STRIKE_RIGHT_CYCLE = ANIM_BARE_READY + 7,
	ANIM_BARE_STRIKE_RIGHT = ANIM_BARE_READY + 8,
	ANIM_BARE_STRIKE_TOP_START = ANIM_BARE_READY + 9,
	ANIM_BARE_STRIKE_TOP_CYCLE = ANIM_BARE_READY + 10,
	ANIM_BARE_STRIKE_TOP = ANIM_BARE_READY + 11,
	ANIM_BARE_STRIKE_BOTTOM_START = ANIM_BARE_READY + 12,
	ANIM_BARE_STRIKE_BOTTOM_CYCLE = ANIM_BARE_READY + 13,
	ANIM_BARE_STRIKE_BOTTOM = ANIM_BARE_READY + 14,
	
	ANIM_1H_READY_PART_1 = ANIM_BARE_STRIKE_BOTTOM + 1,
	ANIM_1H_READY_PART_2 = ANIM_1H_READY_PART_1 + 1,
	ANIM_1H_UNREADY_PART_1 = ANIM_1H_READY_PART_1 + 2,
	ANIM_1H_UNREADY_PART_2 = ANIM_1H_READY_PART_1 + 3,
	ANIM_1H_WAIT = ANIM_1H_READY_PART_1 + 4,
	ANIM_1H_STRIKE_LEFT_START = ANIM_1H_READY_PART_1 + 5,
	ANIM_1H_STRIKE_LEFT_CYCLE = ANIM_1H_READY_PART_1 + 6,
	ANIM_1H_STRIKE_LEFT = ANIM_1H_READY_PART_1 + 7,
	ANIM_1H_STRIKE_RIGHT_START = ANIM_1H_READY_PART_1 + 8,
	ANIM_1H_STRIKE_RIGHT_CYCLE = ANIM_1H_READY_PART_1 + 9,
	ANIM_1H_STRIKE_RIGHT = ANIM_1H_READY_PART_1 + 10,
	ANIM_1H_STRIKE_TOP_START = ANIM_1H_READY_PART_1 + 11,
	ANIM_1H_STRIKE_TOP_CYCLE = ANIM_1H_READY_PART_1 + 12,
	ANIM_1H_STRIKE_TOP = ANIM_1H_READY_PART_1 + 13,
	ANIM_1H_STRIKE_BOTTOM_START = ANIM_1H_READY_PART_1 + 14,
	ANIM_1H_STRIKE_BOTTOM_CYCLE = ANIM_1H_READY_PART_1 + 15,
	ANIM_1H_STRIKE_BOTTOM = ANIM_1H_READY_PART_1 + 16,
	
	ANIM_2H_READY_PART_1 = ANIM_1H_STRIKE_BOTTOM + 1,
	ANIM_2H_READY_PART_2 = ANIM_2H_READY_PART_1 + 1,
	ANIM_2H_UNREADY_PART_1 = ANIM_2H_READY_PART_1 + 2,
	ANIM_2H_UNREADY_PART_2 = ANIM_2H_READY_PART_1 + 3,
	ANIM_2H_WAIT = ANIM_2H_READY_PART_1 + 4,
	ANIM_2H_STRIKE_LEFT_START = ANIM_2H_READY_PART_1 + 5,
	ANIM_2H_STRIKE_LEFT_CYCLE = ANIM_2H_READY_PART_1 + 6,
	ANIM_2H_STRIKE_LEFT = ANIM_2H_READY_PART_1 + 7,
	ANIM_2H_STRIKE_RIGHT_START = ANIM_2H_READY_PART_1 + 8,
	ANIM_2H_STRIKE_RIGHT_CYCLE = ANIM_2H_READY_PART_1 + 9,
	ANIM_2H_STRIKE_RIGHT = ANIM_2H_READY_PART_1 + 10,
	ANIM_2H_STRIKE_TOP_START = ANIM_2H_READY_PART_1 + 11,
	ANIM_2H_STRIKE_TOP_CYCLE = ANIM_2H_READY_PART_1 + 12,
	ANIM_2H_STRIKE_TOP = ANIM_2H_READY_PART_1 + 13,
	ANIM_2H_STRIKE_BOTTOM_START = ANIM_2H_READY_PART_1 + 14,
	ANIM_2H_STRIKE_BOTTOM_CYCLE = ANIM_2H_READY_PART_1 + 15,
	ANIM_2H_STRIKE_BOTTOM = ANIM_2H_READY_PART_1 + 16,
	
	ANIM_DAGGER_READY_PART_1 = ANIM_2H_STRIKE_BOTTOM + 1,
	ANIM_DAGGER_READY_PART_2 = ANIM_DAGGER_READY_PART_1 + 1,
	ANIM_DAGGER_UNREADY_PART_1 = ANIM_DAGGER_READY_PART_1 + 2,
	ANIM_DAGGER_UNREADY_PART_2 = ANIM_DAGGER_READY_PART_1 + 3,
	ANIM_DAGGER_WAIT = ANIM_DAGGER_READY_PART_1 + 4,
	ANIM_DAGGER_STRIKE_LEFT_START = ANIM_DAGGER_READY_PART_1 + 5,
	ANIM_DAGGER_STRIKE_LEFT_CYCLE = ANIM_DAGGER_READY_PART_1 + 6,
	ANIM_DAGGER_STRIKE_LEFT = ANIM_DAGGER_READY_PART_1 + 7,
	ANIM_DAGGER_STRIKE_RIGHT_START = ANIM_DAGGER_READY_PART_1 + 8,
	ANIM_DAGGER_STRIKE_RIGHT_CYCLE = ANIM_DAGGER_READY_PART_1 + 9,
	ANIM_DAGGER_STRIKE_RIGHT = ANIM_DAGGER_READY_PART_1 + 10,
	ANIM_DAGGER_STRIKE_TOP_START = ANIM_DAGGER_READY_PART_1 + 11,
	ANIM_DAGGER_STRIKE_TOP_CYCLE = ANIM_DAGGER_READY_PART_1 + 12,
	ANIM_DAGGER_STRIKE_TOP = ANIM_DAGGER_READY_PART_1 + 13,
	ANIM_DAGGER_STRIKE_BOTTOM_START = ANIM_DAGGER_READY_PART_1 + 14,
	ANIM_DAGGER_STRIKE_BOTTOM_CYCLE = ANIM_DAGGER_READY_PART_1 + 15,
	ANIM_DAGGER_STRIKE_BOTTOM = ANIM_DAGGER_READY_PART_1 + 16,
	
	ANIM_MISSILE_READY_PART_1 = ANIM_DAGGER_STRIKE_BOTTOM + 1,
	ANIM_MISSILE_READY_PART_2 = ANIM_MISSILE_READY_PART_1 + 1,
	ANIM_MISSILE_UNREADY_PART_1 = ANIM_MISSILE_READY_PART_1 + 2,
	ANIM_MISSILE_UNREADY_PART_2 = ANIM_MISSILE_READY_PART_1 + 3,
	ANIM_MISSILE_WAIT = ANIM_MISSILE_READY_PART_1 + 4,
	ANIM_MISSILE_STRIKE_PART_1 = ANIM_MISSILE_READY_PART_1 + 5,
	ANIM_MISSILE_STRIKE_PART_2 = ANIM_MISSILE_READY_PART_1 + 6,
	ANIM_MISSILE_STRIKE_CYCLE = ANIM_MISSILE_READY_PART_1 + 7,
	ANIM_MISSILE_STRIKE = ANIM_MISSILE_READY_PART_1 + 8,
	
	ANIM_SHIELD_START = ANIM_MISSILE_STRIKE + 1,
	ANIM_SHIELD_CYCLE = ANIM_SHIELD_START + 1,
	ANIM_SHIELD_HIT = ANIM_SHIELD_START + 2,
	ANIM_SHIELD_END = ANIM_SHIELD_START + 3,
	
	ANIM_CAST_START = ANIM_SHIELD_END + 1,
	ANIM_CAST_CYCLE = ANIM_CAST_START + 1,
	ANIM_CAST = ANIM_CAST_START + 2,
	ANIM_CAST_END = ANIM_CAST_START + 3,
	
	ANIM_DEATH_CRITICAL = ANIM_CAST_END + 1,
	ANIM_CROUCH = ANIM_CAST_END + 2,
	ANIM_CROUCH_WALK = ANIM_CAST_END + 3,
	ANIM_CROUCH_WALK_BACKWARD = ANIM_CAST_END + 4,
	ANIM_LEAN_RIGHT = ANIM_CAST_END + 5,
	ANIM_LEAN_LEFT = ANIM_CAST_END + 6,
	ANIM_JUMP = ANIM_CAST_END + 7,
	ANIM_HOLD_TORCH = ANIM_CAST_END + 8,
	ANIM_WALK_MINISTEP = ANIM_CAST_END + 9,
	ANIM_STRAFE_RIGHT = ANIM_CAST_END + 10,
	ANIM_STRAFE_LEFT = ANIM_CAST_END + 11,
	ANIM_MEDITATION = ANIM_CAST_END + 12,
	ANIM_WAIT_SHORT = ANIM_CAST_END + 13,
	
	ANIM_FIGHT_WALK_FORWARD = ANIM_CAST_END + 14,
	ANIM_FIGHT_WALK_BACKWARD = ANIM_CAST_END + 15,
	ANIM_FIGHT_WALK_MINISTEP = ANIM_CAST_END + 16,
	ANIM_FIGHT_STRAFE_RIGHT = ANIM_CAST_END + 17,
	ANIM_FIGHT_STRAFE_LEFT = ANIM_CAST_END + 18,
	ANIM_FIGHT_WAIT = ANIM_CAST_END + 19,
	
	ANIM_LEVITATE = ANIM_CAST_END + 20,
	ANIM_CROUCH_START = ANIM_CAST_END + 21,
	ANIM_CROUCH_WAIT = ANIM_CAST_END + 22,
	ANIM_CROUCH_END = ANIM_CAST_END + 23,
	ANIM_JUMP_ANTICIPATION = ANIM_CAST_END + 24,
	ANIM_JUMP_UP = ANIM_CAST_END + 25,
	ANIM_JUMP_CYCLE = ANIM_CAST_END + 26,
	ANIM_JUMP_END = ANIM_CAST_END + 27,
	ANIM_TALK_NEUTRAL_HEAD = ANIM_CAST_END + 28,
	ANIM_TALK_ANGRY_HEAD = ANIM_CAST_END + 29,
	ANIM_TALK_HAPPY_HEAD = ANIM_CAST_END + 30,
	ANIM_STRAFE_RUN_LEFT = ANIM_CAST_END + 31,
	ANIM_STRAFE_RUN_RIGHT = ANIM_CAST_END + 32,
	ANIM_CROUCH_STRAFE_LEFT = ANIM_CAST_END + 33,
	ANIM_CROUCH_STRAFE_RIGHT = ANIM_CAST_END + 34,
	ANIM_WALK_SNEAK = ANIM_CAST_END + 35,
	ANIM_GRUNT = ANIM_CAST_END + 36,
	ANIM_JUMP_END_PART2 = ANIM_CAST_END + 37,
	ANIM_HIT_SHORT = ANIM_CAST_END + 38,
	ANIM_U_TURN_LEFT = ANIM_CAST_END + 39,
	ANIM_U_TURN_RIGHT = ANIM_CAST_END + 40,
	ANIM_RUN_BACKWARD = ANIM_CAST_END + 41,
	ANIM_U_TURN_LEFT_FIGHT = ANIM_CAST_END + 42,
	ANIM_U_TURN_RIGHT_FIGHT = ANIM_CAST_END + 43,
	
};

const AnimationNumber ANIM_DEFAULT = ANIM_WAIT;

enum ScriptResult {
	ACCEPT = 1,
	DESTRUCTIVE = 2,
	REFUSE = -1,
	BIGERROR = -2
};

class ScriptEventName {
	
	ScriptMessage m_id = SM_NULL;
	std::string m_name;
	
public:
	
	ScriptEventName() noexcept = default;
	/* implicit */ ScriptEventName(ScriptMessage id) noexcept : m_id(id) { }
	explicit ScriptEventName(std::string_view name) : m_name(name) { }
	/* implicit */ ScriptEventName(const char * name) : m_name(name) { }
	
	[[nodiscard]] bool operator==(ScriptMessage id) const noexcept {
		return m_id == id;
	}
	
	[[nodiscard]] bool operator!=(ScriptMessage id) const noexcept {
		return m_id != id;
	}
	
	[[nodiscard]] static ScriptEventName parse(std::string_view name);
	
	[[nodiscard]] ScriptMessage getId() const noexcept { return m_id; }
	[[nodiscard]] const std::string & getName() const noexcept { return m_name; }
	
	[[nodiscard]] std::string_view toString() const noexcept;
	
	[[nodiscard]] DisabledEvents toDisabledEventsMask() const noexcept;
	
};

std::ostream & operator<<(std::ostream & os, const ScriptEventName & event);

class ScriptParameters : public std::vector<std::string> {
	
	bool m_peekOnly = false;
	
public:
	
	ScriptParameters() arx_noexcept_default
	/* implicit */ ScriptParameters(std::string_view parameter) {
		emplace_back(parameter);
	}
	/* implicit */ ScriptParameters(std::string parameter) {
		emplace_back(std::move(parameter));
	}
	/* implicit */ ScriptParameters(const char * parameter)
		: ScriptParameters(std::string_view(parameter))
	{ }
	template <typename T>
	/* implicit */ ScriptParameters(T parameter,
	                                typename std::enable_if_t<std::numeric_limits<T>::is_specialized, bool>
	                                /* enable */ = true)
		: ScriptParameters(std::to_string(parameter))
	{ }
	
	[[nodiscard]] static ScriptParameters parse(std::string_view str);
	
	using std::vector<std::string>::push_back;
	
	template <typename T>
	typename std::enable_if_t<std::numeric_limits<T>::is_specialized> push_back(T parameter) {
		push_back(std::to_string(parameter));
	}
	
	/*!
	 * Sets if execution should abort as soon as a command is reached that would modify any state.
	 *
	 * Speech commands are not considered to modify state for this purpose unless they are chained with
	 * other commands.
	 *
	 * If such a command is reached the script result will be \ref DESTRUCTIVE.
	 */
	void setPeekOnly(bool enable = true) noexcept { m_peekOnly = enable; }
	[[nodiscard]] bool isPeekOnly() const noexcept { return m_peekOnly; }
	
};

std::ostream & operator<<(std::ostream & os, const ScriptParameters & parameters);

extern SCRIPT_VARIABLES svar;
extern std::vector<SCR_TIMER> g_scriptTimers;
extern long FORBID_SCRIPT_IO_CREATION;

void ARX_SCRIPT_Timer_Check();
void ARX_SCRIPT_Timer_ClearAll();
void ARX_SCRIPT_Timer_Clear_For_IO(Entity * io);
SCR_TIMER & createScriptTimer(Entity * io, std::string && name);
 
void ARX_SCRIPT_EventStackExecute(size_t limit = 20);
void ARX_SCRIPT_EventStackExecuteAll();
void ARX_SCRIPT_EventStackInit();
void ARX_SCRIPT_EventStackClear(bool check_exist = true);
void ARX_SCRIPT_ResetObject(Entity * io, bool init);
void ARX_SCRIPT_Reset(Entity * io, bool init);
bool scriptTimerExists(Entity * io, std::string_view name);
void ARX_SCRIPT_ComputeShortcuts(EERIE_SCRIPT & es);
void ARX_SCRIPT_AllowInterScriptExec();
size_t ARX_SCRIPT_CountTimers();
void ARX_SCRIPT_ResetAll(bool init);
void ARX_SCRIPT_EventStackClearForIo(Entity * io);
Entity * ARX_SCRIPT_Get_IO_Max_Events();
Entity * ARX_SCRIPT_Get_IO_Max_Events_Sent();

void ManageCasseDArme(Entity * io);
void ReleaseScript(EERIE_SCRIPT * es);
void ARX_SCRIPT_Init_Event_Stats();
ScriptResult SendInitScriptEvent(Entity * io);

//! Generates a random name for an unnamed timer
std::string getDefaultScriptTimerName(Entity * io, std::string_view prefix = "timer");

// Use to set the value of a script variable
SCRIPT_VAR * SETVarValueText(SCRIPT_VARIABLES & svf, std::string_view name, std::string && val);
SCRIPT_VAR * SETVarValueLong(SCRIPT_VARIABLES & svf, std::string_view name, long val);
SCRIPT_VAR * SETVarValueFloat(SCRIPT_VARIABLES & svf, std::string_view name, float val);

// Use to get the value of a script variable
long GETVarValueLong(const SCRIPT_VARIABLES & svf, std::string_view name);
float GETVarValueFloat(const SCRIPT_VARIABLES & svf, std::string_view name);
std::string GETVarValueText(const SCRIPT_VARIABLES & svf, std::string_view name);
const SCRIPT_VAR * GetVarAddress(const SCRIPT_VARIABLES & svf, std::string_view name);

ValueType getSystemVar(const script::Context & context, std::string_view name, std::string & txtcontent, float * fcontent, long * lcontent);
void ARX_SCRIPT_Timer_Clear_All_Locals_For_IO(Entity * io);
void ARX_SCRIPT_Timer_Clear_By_Name_And_IO(std::string_view timername, Entity * io);

ScriptResult SendIOScriptEvent(Entity * sender, Entity * entity, const ScriptEventName & event,
                               const ScriptParameters & parameters = ScriptParameters());

ScriptResult SendMsgToAllIO(Entity * sender, const ScriptEventName & event,
                            const ScriptParameters & parameters = ScriptParameters());

void Stack_SendIOScriptEvent(Entity * sender, Entity * entity, const ScriptEventName & event,
                             const ScriptParameters & parameters = ScriptParameters());

/*!
 * Finds the first occurrence of str in the script that is followed
 * by a separator (a character of value less then or equal 32)
 * 
 * \return The position of str in the script or -1 if str was not found.
 */
size_t FindScriptPos(const EERIE_SCRIPT * es, std::string_view str);

void CloneLocalVars(Entity * ioo, Entity * io);
void ARX_SCRIPT_Free_All_Global_Variables();

void loadScript(EERIE_SCRIPT & script, PakFile * file);

#endif // ARX_SCRIPT_SCRIPT_H
