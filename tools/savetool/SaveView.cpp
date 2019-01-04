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

#include "savetool/SaveView.h"

#include <iostream>
#include <iomanip>
#include <sstream>

#include <boost/foreach.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/io/ios_state.hpp>

#include "Configure.h"

#include "ai/Paths.h"
#include "core/Localisation.h"
#include "core/Config.h"
#include "game/Player.h"
#include "gui/Interface.h"
#include "io/SaveBlock.h"
#include "io/fs/FilePath.h"
#include "io/fs/SystemPaths.h"
#include "io/log/Logger.h"
#include "io/resource/PakReader.h"
#include "scene/SaveFormat.h"
#include "scene/Interactive.h"
#include "util/String.h"

static std::ostream & operator<<(std::ostream & strm, const SavedVec3 & vec) {
	return strm << '(' << vec.x << ", " << vec.y << ", " << vec.z << ')';
}

static std::ostream & operator<<(std::ostream & strm, const SavedAnglef & a) {
	return strm << '(' << a.a << ", " << a.b << ", " << a.g << ')';
}

static std::ostream & operator<<(std::ostream & strm, const SavedColor & c) {
	return strm << '(' << c.r << ", " << c.g << ", " << c.b << ')';
}

static std::string loadUnlocalized(const std::string & str) {
	
	// if the section name has the qualifying brackets "[]", cut them off
	if(!str.empty() && str[0] == '[' && str[str.length() - 1] == ']') {
		return str.substr(1, str.length() - 2);
	}
	
	return str;
}

template <class F, class E>
inline void print_flag(F & flags, E flag, const std::string & name) {
	if(flags & flag) {
		std::cout << ' ' << name;
		flags &= int(~flag);
	}
}

template <class F>
inline void print_unknown_flags(F flags) {
	if(flags) {
		boost::io::ios_all_saver coutFlags(std::cout);
		std::cout << " (unknown:0x" << std::hex << flags << ')';
		coutFlags.restore();
	}
}

const char * spellNames[] = {
	"magic sight",
	"magic missile",
	"ignite",
	"douse",
	"activate portal",
	"heal",
	"detect trap",
	"armor",
	"lower armor",
	"harm",
	"speed",
	"dispell illusion",
	"fireball",
	"create food",
	"ice projectile",
	"bless",
	"dispell field",
	"fire projection",
	"telekinesis",
	"curse",
	"cold protection",
	"rune of guarding",
	"levitate",
	"cure poison",
	"repel undead",
	"poision projectile",
	"rise dead",
	"paralyse",
	"create field",
	"disarm trap",
	"slow down",
	"flying eye",
	"fire field",
	"ice field",
	"lightning strike",
	"confuse",
	"invisibility",
	"mana drain",
	"explosion",
	"enchant weapon",
	"life drain",
	"summon creature",
	"negate magic",
	"incinerate",
	"mass paralyse",
	"mass lightning strike",
	"control target",
	"freeze time",
	"mass incinerate",
	"fake summon",
	"(missing)",
	"teleport"
};

const char * runeNames[] = {
	"aam",
	"nhi",
	"mega",
	"yok",
	"taar",
	"kaom",
	"vitae",
	"vista",
	"stregum",
	"morte",
	"cosum",
	"communicatum",
	"movis",
	"tempus",
	"folgora",
	"spacium",
	"tera",
	"cetrius",
	"rhaa",
	"fridd",
	"akbaa"
};

const char * animationNames[] = {
	"wait",
	"walk",
	"walk2",
	"walk3",
	"animation #4",
	"animation #5",
	"animation #6",
	"animation #7",
	"action",
	"action2",
	"action3",
	"hit1",
	"strike1",
	"die",
	"wait2",
	"run",
	"run2",
	"run3",
	"action4",
	"action5",
	"action6",
	"action7",
	"action8",
	"action9",
	"action10",
	"animation #25",
	"animation #26",
	"animation #27",
	"animation #28",
	"animation #29",
	"talk_neutral",
	"talk_happy",
	"talk_angry",
	"walk_backward",
	"bare_ready",
	"bare_unready",
	"bare_wait",
	"bare_strike_left_start",
	"bare_strike_left_cycle",
	"bare_strike_left",
	"bare_strike_right_start",
	"bare_strike_right_cycle",
	"bare_strike_right",
	"bare_strike_top_start",
	"bare_strike_top_cycle",
	"bare_strike_top",
	"bare_strike_bottom_start",
	"bare_strike_bottom_cycle",
	"bare_strike_bottom",
	"1h_ready_part_1",
	"1h_ready_part_2",
	"1h_unready_part_1",
	"1h_unready_part_2",
	"1h_wait",
	"1h_strike_left_start",
	"1h_strike_left_cycle",
	"1h_strike_left",
	"1h_strike_right_start",
	"1h_strike_right_cycle",
	"1h_strike_right",
	"1h_strike_top_start",
	"1h_strike_top_cycle",
	"1h_strike_top",
	"1h_strike_bottom_start",
	"1h_strike_bottom_cycle",
	"1h_strike_bottom",
	"2h_ready_part_1",
	"2h_ready_part_2",
	"2h_unready_part_1",
	"2h_unready_part_2",
	"2h_wait",
	"2h_strike_left_start",
	"2h_strike_left_cycle",
	"2h_strike_left",
	"2h_strike_right_start",
	"2h_strike_right_cycle",
	"2h_strike_right",
	"2h_strike_top_start",
	"2h_strike_top_cycle",
	"2h_strike_top",
	"2h_strike_bottom_start",
	"2h_strike_bottom_cycle",
	"2h_strike_bottom",
	"dagger_ready_part_1",
	"dagger_ready_part_2",
	"dagger_unready_part_1",
	"dagger_unready_part_2",
	"dagger_wait",
	"dagger_strike_left_start",
	"dagger_strike_left_cycle",
	"dagger_strike_left",
	"dagger_strike_right_start",
	"dagger_strike_right_cycle",
	"dagger_strike_right",
	"dagger_strike_top_start",
	"dagger_strike_top_cycle",
	"dagger_strike_top",
	"dagger_strike_bottom_start",
	"dagger_strike_bottom_cycle",
	"dagger_strike_bottom",
	"missile_ready_part_1",
	"missile_ready_part_2",
	"missile_unready_part_1",
	"missile_unready_part_2",
	"missile_wait",
	"missile_strike_part_1",
	"missile_strike_part_2",
	"missile_strike_cycle",
	"missile_strike",
	"shield_start",
	"shield_cycle",
	"shield_hit",
	"shield_end",
	"cast_start",
	"cast_cycle",
	"cast",
	"cast_end",
	"death_critical",
	"crouch",
	"crouch_walk",
	"crouch_walk_backward",
	"lean_right",
	"lean_left",
	"jump",
	"hold_torch",
	"walk_ministep",
	"strafe_right",
	"strafe_left",
	"meditation",
	"wait_short",
	"fight_walk_forward",
	"fight_walk_backward",
	"fight_walk_ministep",
	"fight_strafe_right",
	"fight_strafe_left",
	"fight_wait",
	"levitate",
	"crouch_start",
	"crouch_wait",
	"crouch_end",
	"jump_anticipation",
	"jump_up",
	"jump_cycle",
	"jump_end",
	"talk_neutral_head",
	"talk_angry_head",
	"talk_happy_head",
	"strafe_run_left",
	"strafe_run_right",
	"crouch_strafe_left",
	"crouch_strafe_right",
	"walk_sneak",
	"grunt",
	"jump_end_part2",
	"hit_short",
	"u_turn_left",
	"u_turn_right",
	"run_backward",
	"u_turn_left_fight",
	"u_turn_right_fight"
};

const char * equipitemNames[] = {
	"Strength",
	"Dexterity",
	"Constitution",
	"Mind",
	"Stealth skill",
	"Mecanism skill",
	"Intuition skill",
	"Etheral link skill",
	"Object knowledge skill",
	"Casting skill",
	"Projectile skill",
	"Close combat skill",
	"Defense skill",
	"Armor class",
	"Magic resistance",
	"Poison resistance",
	"Critical hit",
	"Damage",
	"Duration",
	"Aim time",
	"Identify value",
	"Life",
	"Mana",
	"Maximum life",
	"Maximum mana",
	"Special 1",
	"Special 2",
	"Special 3",
	"Special 4"
};

static std::ostream & print_anim_flags(std::ostream & strm, u32 flags) {
	if(!flags) strm << " (none)";
	if(flags & EA_LOOP) strm << " loop";
	if(flags & EA_REVERSE) strm << " reverse";
	if(flags & EA_PAUSED) strm << " pause";
	if(flags & EA_ANIMEND) strm << " end";
	if(flags & EA_STATICANIM) strm << " static";
	if(flags & EA_STOPEND) strm << " stopped";
	if(flags & EA_FORCEPLAY) strm << " forced";
	if(flags & EA_EXCONTROL) strm << " excontrol";
	return strm;
}

static std::ostream & print_item_type(std::ostream & strm, s32 type) {
	if(!type) strm << " (none)";
	if(type & OBJECT_TYPE_WEAPON) strm << " weapon";
	if(type & OBJECT_TYPE_DAGGER) strm << " dagger";
	if(type & OBJECT_TYPE_1H) strm << " 1h";
	if(type & OBJECT_TYPE_2H) strm << " 2h";
	if(type & OBJECT_TYPE_BOW) strm << " bow";
	if(type & OBJECT_TYPE_SHIELD) strm << " shield";
	if(type & OBJECT_TYPE_FOOD) strm << " food";
	if(type & OBJECT_TYPE_GOLD) strm << " gold";
	if(type & OBJECT_TYPE_ARMOR) strm << " armor";
	if(type & OBJECT_TYPE_HELMET) strm << " helmet";
	if(type & OBJECT_TYPE_RING) strm << " ring";
	if(type & OBJECT_TYPE_LEGGINGS) strm << " leggings";
	return strm;
}

static std::ostream & print_behavior(std::ostream & strm, u32 behavior) {
	if(!behavior) strm << " (none)";
	if(behavior & BEHAVIOUR_NONE) strm << " none";
	if(behavior & BEHAVIOUR_FRIENDLY) strm << " friendly";
	if(behavior & BEHAVIOUR_MOVE_TO) strm << " move_to";
	if(behavior & BEHAVIOUR_WANDER_AROUND) strm << " wander_around";
	if(behavior & BEHAVIOUR_FLEE) strm << " flee";
	if(behavior & BEHAVIOUR_HIDE) strm << " hide";
	if(behavior & BEHAVIOUR_LOOK_FOR) strm << " look_for";
	if(behavior & BEHAVIOUR_SNEAK) strm << " sneak";
	if(behavior & BEHAVIOUR_FIGHT) strm << " fight";
	if(behavior & BEHAVIOUR_DISTANT) strm << " distant";
	if(behavior & BEHAVIOUR_MAGIC) strm << " magic";
	if(behavior & BEHAVIOUR_GUARD) strm << " guard";
	if(behavior & BEHAVIOUR_GO_HOME) strm << " go_home";
	if(behavior & BEHAVIOUR_LOOK_AROUND) strm << " look_around";
	if(behavior & BEHAVIOUR_STARE_AT) strm << " stare_at";
	return strm;
}

static std::ostream & print_tactics(std::ostream & strm, s32 tactics) {
	switch(tactics) {
		case 0: strm << "none"; break;
		case 1: strm << "side"; break;
		case 2: strm << "side + back"; break;
		default: strm << "(unknown)";
	}
	return strm;
}

static std::ostream & print_movemode(std::ostream & strm, s32 movemode) {
	switch(movemode) {
		case WALKMODE: strm << "walking"; break;
		case RUNMODE: strm << "running"; break;
		case NOMOVEMODE: strm << "no movement"; break;
		case SNEAKMODE: strm << "sneaking"; break;
		default: strm << "(unknown)";
	}
	return strm;
}

static void print_spell(s32 spell) {
	
	if(size_t(spell) < ARRAY_SIZE(spellNames)) {
		std::cout << spellNames[spell];
	} else {
		std::cout << "(unknown)";
	}
	
}

static int print_variables(size_t n, const char * dat, size_t & pos, const std::string & p,
                           VariableType s, VariableType f, VariableType l) {
	
	if(n) {
		std::cout << p <<  "Variables:";
		for(size_t i = 0; i < n; i++) {
			
			const ARX_CHANGELEVEL_VARIABLE_SAVE * avs;
			avs = reinterpret_cast<const ARX_CHANGELEVEL_VARIABLE_SAVE *>(dat + pos);
			pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);
			
			std::string name = boost::to_lower_copy(util::loadString(avs->name));
			
			VariableType type;
			if(avs->type == s || avs->type == f || avs->type == l) {
				type = VariableType(avs->type);
			} else if(avs->name[0] == '$' || avs->name[0] == '\xA3') {
				type = s;
			} else if(avs->name[0] == '&' || avs->name[0] == '@') {
				type = f;
			} else if(avs->name[0] == '#' || avs->name[0] == 's') {
				type = l;
			} else {
				std::cout << "Error: unknown script variable type: " << avs->type;
				return -2;
			}
			
			name = name.substr(1);
			
			std::cout << '\n';
			if(type == s) {
				std::string value = boost::to_lower_copy(util::loadString(dat + pos, long(avs->fval)));
				pos += long(avs->fval);
				std::cout << p << "  s " << name << " = \"" << value << '"';
			} else if(type == f) {
				std::cout << p << "  f " << name << " = " << avs->fval;
			} else if(type == l) {
				std::cout << p << "  l " << name << " = " << long(avs->fval);
			}
			
		}
		std::cout << '\n';
	}
	
	return 0;
}

static void print_animations(const char (&anims)[SAVED_MAX_ANIMS][256]) {
	
	std::cout << "\nAnimations:";
	bool hasAnims = false;
	for(size_t i = 0; i < SAVED_MAX_ANIMS; i++) {
		
		res::path anim = res::path::load(util::loadString(anims[i]));
		if(anim.empty()) {
			continue;
		}
		
		hasAnims = true;
		std::cout << '\n';
		
		if(i < ARRAY_SIZE(animationNames)) {
			std::cout << "  - " << animationNames[i] << ": " << anim;
		} else {
			std::cout << "  - animation #" << i << ": " << anim;
		}
	}
	if(!hasAnims) {
		std::cout << " (none)";
	}
	std::cout << '\n';
}

static void print_anim_layers(const SavedAnimUse animlayer[SAVED_MAX_ANIM_LAYERS],
                              const std::string & pf = std::string()) {
	
	for(size_t i = 0; i < SAVED_MAX_ANIM_LAYERS; i++) {
		const SavedAnimUse & layer = animlayer[i];
		
		if(layer.next_anim == ANIM_NONE && layer.cur_anim == ANIM_NONE) {
			continue;
		}
		
		std::cout << '\n' << pf << "Animation layer #" << i << ":\n";
		
		if(layer.next_anim != ANIM_NONE) {
			if(size_t(layer.next_anim) < ARRAY_SIZE(animationNames)) {
				std::cout << pf << "  Next animation: " << animationNames[layer.next_anim] << '\n';
			} else {
				std::cout << pf << "  Next animation: animation #" << layer.next_anim << '\n';
			}
		}
		
		if(layer.cur_anim != ANIM_NONE) {
			if(size_t(layer.cur_anim) < ARRAY_SIZE(animationNames)) {
				std::cout << pf << "  Current animation: " << animationNames[layer.cur_anim] << '\n';
			} else {
				std::cout << pf << "  Current animation: animation #" << layer.cur_anim << '\n';
			}
		}
		
		if(layer.altidx_next) std::cout << pf << "  Next alternative: " << layer.altidx_next << '\n';
		if(layer.altidx_cur) std::cout << pf << "  Current alternative: " << layer.altidx_cur << '\n';
		
		if(layer.flags) print_anim_flags(std::cout << pf << "  Flags:", layer.flags) << '\n';
		if(layer.nextflags) print_anim_flags(std::cout << pf << "  Next flags:", layer.nextflags) << '\n';
		
	}
	
}

Config config;

static void print_level(long level) {
	boost::io::ios_all_saver coutFlags(std::cout);
	std::cout << level << " (lvl" << std::setw(3) << std::setfill('0') << level << ')';
	coutFlags.restore();
}

static void print_type(s32 type) {
	switch(type) {
		case TYPE_NPC: std::cout << "NPC"; break;
		case TYPE_ITEM: std::cout << "Item"; break;
		case TYPE_FIX: std::cout << "Fixed"; break;
		case TYPE_CAMERA: std::cout << "Camera"; break;
		case TYPE_MARKER: std::cout << "Marker"; break;
		default: std::cout << "(unknown)";
	}
}

static void print_spellcast_flags(s32 flags) {
	if(!flags) std::cout << " (none)";
	print_flag(flags, SPELLCAST_FLAG_NODRAW, "nodraw");
	print_flag(flags, SPELLCAST_FLAG_NOANIM, "noanim");
	print_flag(flags, SPELLCAST_FLAG_NOMANA, "nomana");
	print_flag(flags, SPELLCAST_FLAG_PRECAST, "precast");
	print_flag(flags, SPELLCAST_FLAG_LAUNCHPRECAST, "launchprecast");
	print_flag(flags, SPELLCAST_FLAG_NOCHECKCANCAST, "forced");
	print_flag(flags, SPELLCAST_FLAG_NOSOUND, "nosound");
	print_flag(flags, SPELLCAST_FLAG_RESTORE, "restore");
	print_unknown_flags(flags);
}

static void print_physics(const SavedIOPhysics & physics) {
	if(physics.cyl.origin.toVec3() != Vec3f(0.f) || physics.cyl.radius != 0.f || physics.cyl.height != 0.f) std::cout << "  Cylinder: origin=" << physics.cyl.origin << " radius=" << physics.cyl.radius << " height=" << physics.cyl.height << '\n';
	if(physics.startpos.toVec3() != Vec3f(0.f)) std::cout << "  Start position: " << physics.startpos << '\n';
	if(physics.targetpos.toVec3() != Vec3f(0.f)) std::cout << "  Target position: " << physics.targetpos << '\n';
	if(physics.velocity.toVec3() != Vec3f(0.f)) std::cout << "  Velocity: " << physics.velocity << '\n';
	if(physics.forces.toVec3() != Vec3f(0.f)) std::cout << "  Forces: " << physics.forces << '\n';
}

static void print_ident(SaveBlock & save, const std::string & ident) {
	
	if(ident.empty()) {
		std::cout << "(none)";
		return;
	}
	
	std::cout << ident;
	
	if(ident == "none" || ident == "self" || ident == "me" || ident == "player") {
		return;
	}
	
	std::string buffer = save.load(ident);
	if(buffer.empty()) {
		std::cout << " (unknown)";
		return;
	}
	
	const char * dat = buffer.data();
	
	size_t pos = 0;
	const ARX_CHANGELEVEL_IO_SAVE & ais = *reinterpret_cast<const ARX_CHANGELEVEL_IO_SAVE *>(dat + pos);
	pos += sizeof(ARX_CHANGELEVEL_IO_SAVE);
	if(pos > buffer.size()) {
		std::cout << " (bad save)";
		return;
	} else if(ais.version != ARX_GAMESAVE_VERSION) {
		std::cout << " (bad version: " << ais.version << ')';
		return;
	}
	
	std::string locname = loadUnlocalized(boost::to_lower_copy(util::loadString(ais.locname)));
	if(!locname.empty()) {
		std::string name = getLocalised(locname);
		if(name.empty()) {
			std::cout << " = " << locname;
		} else {
			std::cout << " = \"" << name << '"';
		}
	}
	
	std::cout << " (";
	print_type(ais.savesystem_type);
	std::cout << ')';
	
}

template <size_t M, size_t N>
static void print_inventory(SaveBlock & save, const char (&slot_io)[M][N][SIZE_ID],
                            const s32 (&slot_show)[M][N]) {
	
	bool hasItems = false;
	for(size_t m = 0; m < M; m++) {
		for(size_t n = 0; n < N; n++) {
			
			std::string name = boost::to_lower_copy(util::loadString(slot_io[m][n]));
			if((name.empty() || name == "none") || !slot_show[m][n]) {
				continue;
			}
			
			hasItems = true;
			
			std::cout << "  - (" << m << ", " << n << "): ";
			print_ident(save, name);
			std::cout << '\n';
		}
	}
	if(!hasItems) {
		std::cout << "  (empty)\n";
	}
	
}

static void print_player_movement(s32 movement) {
	if(!movement) std::cout << "(none)";
	print_flag(movement, PLAYER_MOVE_WALK_FORWARD, "forward");
	print_flag(movement, PLAYER_MOVE_WALK_BACKWARD, "backward");
	print_flag(movement, PLAYER_MOVE_STRAFE_LEFT, "strafing_left");
	print_flag(movement, PLAYER_MOVE_STRAFE_RIGHT, "strafing_right");
	print_flag(movement, PLAYER_MOVE_JUMP, "jumping");
	print_flag(movement, PLAYER_MOVE_STEALTH, "stealth");
	print_flag(movement, PLAYER_ROTATE, "rotating");
	print_flag(movement, PLAYER_CROUCH, "crouching");
	print_flag(movement, PLAYER_LEAN_LEFT, "leaning_left");
	print_flag(movement, PLAYER_LEAN_RIGHT, "leaning_right");
	print_unknown_flags(movement);
}

static void print_item(SaveBlock & save, const char (&ident)[64],  const std::string & name) {
	
	std::string i = boost::to_lower_copy(util::loadString(ident));
	if(i.empty() || i == "none") {
		return;
	}
	
	std::cout << "  " << name << ": ";
	print_ident(save, i);
	std::cout << '\n';
}

static void print_version(u64 version) {
	
	if(!version) {
		std::cout << "(unknown)";
		return;
	}
	
	u16 major = (version >> 48) & 0xffff;
	u16 minor = (version >> 32) & 0xffff;
	u16 patch = (version >> 16) & 0xffff;
	u16 build = (version >> 0) & 0xffff;
	
	std::cout << major << '.' << minor << '.' << patch << '.' << build;
}

static int view_pld(const char * dat, size_t size) {
	
	size_t pos = 0;
	const ARX_CHANGELEVEL_PLAYER_LEVEL_DATA & pld = *reinterpret_cast<const ARX_CHANGELEVEL_PLAYER_LEVEL_DATA *>(dat + pos);
	pos += sizeof(ARX_CHANGELEVEL_PLAYER_LEVEL_DATA);
	
	if(pld.version != ARX_GAMESAVE_VERSION) {
		std::cout << "bad version: " << pld.version << '\n';
		return 3;
	}
	
	std::string name = util::loadString(pld.name);
	if(name == "ARX_QUICK_ARX" || name == "ARX_QUICK_ARX1") {
		std::cout << "Type: quicksave\n";
	} else {
		std::cout << "Name: \"" << name << "\"\n";
	}
	
	std::cout << "Current level: ";
	print_level(pld.level);
	std::cout << '\n';
	
	std::cout << "Game time: " << pld.time << '\n';
	
	if(pld.playthroughStart) {
		std::time_t time = std::time_t(pld.playthroughStart);
		const struct tm & t = *std::gmtime(&time);
		std::ostringstream oss;
		std::cout << "Playthrough start: " << std::setfill('0') << (t.tm_year + 1900)
		          << "-" << std::setw(2) << (t.tm_mon + 1)
		          << "-" << std::setw(2) << t.tm_mday
		          << " " << std::setfill(' ') << std::setw(2) << t.tm_hour
		          << ":" << std::setfill('0') << std::setw(2) << t.tm_min
		          << ":" << std::setw(2) << t.tm_sec
		          << " UTC" << '\n';
	}
	
	if(pld.playthroughId) {
		std::cout << "Playthrough ID: 0x" << std::setfill('0') << std::setw(16)
		          << std::hex << pld.playthroughId << std::dec << '\n';
	}
	
	if(pld.newestALVersion) {
		std::cout << "Played using: AL ";
		if(pld.oldestALVersion != pld.newestALVersion) {
			print_version(pld.oldestALVersion);
			std::cout << " to ";
		}
		print_version(pld.newestALVersion);
		std::cout << "\n";
		if(pld.newestALVersion != pld.oldestALVersion || pld.lastALVersion != pld.newestALVersion) {
			std::cout << "Last played using: AL ";
			print_version(pld.lastALVersion);
			std::cout << "\n";
		}
	}
	
	std::string engine = util::loadString(pld.lastEngineVersion);
	if(!engine.empty()) {
		std::cout << "Saved by: " << engine << '\n';
	}
	
	arx_assert(size >= pos);
	ARX_UNUSED(size), ARX_UNUSED(pos);
	
	return 0;
}

static int view_globals(const char * dat, size_t size) {
	
	size_t pos = 0;
	const ARX_CHANGELEVEL_SAVE_GLOBALS & pld = *reinterpret_cast<const ARX_CHANGELEVEL_SAVE_GLOBALS *>(dat + pos);
	pos += sizeof(ARX_CHANGELEVEL_SAVE_GLOBALS);
	
	if(pld.version != ARX_GAMESAVE_VERSION) {
		std::cout << "bad version: " << pld.version << '\n';
		return 3;
	}
	
	print_variables(pld.nb_globals, dat, pos, "", TYPE_G_TEXT, TYPE_G_FLOAT, TYPE_G_LONG);
	
	arx_assert(size >= pos);
	ARX_UNUSED(size);
	
	return 0;
}

static std::string equip_slot_name(size_t i) {
	switch(i) {
		case 0: return "Left ring";
		case 1: return "Right ring";
		case 2: return "Weapon";
		case 3: return "Shield";
		case 4: return "Torch";
		case 5: return "Armor";
		case 6: return "Helmet";
		case 7: return "Leggings";
		default: {
			std::ostringstream oss;
			oss << "Slot #" << i;
			return oss.str();
		}
	}
}

static int view_player(SaveBlock & save, const char * dat, size_t size) {
	
	size_t pos = 0;
	const ARX_CHANGELEVEL_PLAYER & asp = *reinterpret_cast<const ARX_CHANGELEVEL_PLAYER *>(dat + pos);
	pos += sizeof(ARX_CHANGELEVEL_PLAYER);
	
	if(asp.version != 0) {
		std::cout << "bad player save version: " << asp.version << '\n';
		return 3;
	}
	
	std::cout << "Aim time: " << asp.AimTime << '\n';
	std::cout << "Angle: " << asp.angle << '\n';
	std::cout << "Armor class: " << asp.armor_class << '\n';
	std::cout << "Critical hit: " << asp.Critical_Hit << '\n';
	
	if(asp.Current_Movement) {
		std::cout << "Current movement:";
		print_player_movement(asp.Current_Movement);
		std::cout << '\n';
	}
	
	if(asp.damages != 0.f) std::cout << "Damages: " << asp.damages << '\n';
	if(asp.doingmagic) std::cout << "Doing magic: " << asp.doingmagic << '\n';
	
	if(asp.playerflags) {
		std::cout << "Flags: ";
		s32 flags = asp.playerflags;
		print_flag(flags, PLAYERFLAGS_NO_MANA_DRAIN, "no_mana_drain");
		print_flag(flags, PLAYERFLAGS_INVULNERABILITY, "invulnerable");
		std::cout << '\n';
	}
	
	if(!asp.Global_Magic_Mode) std::cout << "Magic disabled!\n";
	
	std::string teleportToLevel = boost::to_lower_copy(util::loadString(asp.TELEPORT_TO_LEVEL));
	if(!teleportToLevel.empty()) std::cout << "Teleporting to level: " << teleportToLevel << '\n';
	
	std::string teleportToPosition = boost::to_lower_copy(util::loadString(asp.TELEPORT_TO_POSITION));
	if(!teleportToPosition.empty()) {
		std::cout << "Teleporting to: ";
		print_ident(save, teleportToPosition);
		std::cout << '\n';
	}
	
	if(!teleportToLevel.empty() || !teleportToPosition.empty()) {
		std::cout << "Teleporting to angle: " << asp.TELEPORT_TO_ANGLE << '\n';
	}
	
	if(asp.CHANGE_LEVEL_ICON != -1) std::cout << "Change level icon: " << asp.CHANGE_LEVEL_ICON << '\n';
	
	if(asp.Interface) {
		std::cout << "Interface:";
		s16 interface_flags = asp.Interface;
		print_flag(interface_flags, INTER_PLAYERBOOK, "player_book");
		print_flag(interface_flags, INTER_INVENTORY, "inventory");
		print_flag(interface_flags, INTER_INVENTORYALL, "inventoryall");
		print_flag(interface_flags, INTER_MINIBOOK, "minibook");
		print_flag(interface_flags, INTER_MINIBACK, "miniback");
		print_flag(interface_flags, INTER_LIFE_MANA, "life_mana");
		print_flag(interface_flags, INTER_COMBATMODE, "combatmode");
		print_flag(interface_flags, INTER_NOTE, "note");
		print_flag(interface_flags, INTER_STEAL, "steal");
		print_flag(interface_flags, INTER_NO_STRIKE, "no_strike");
		print_unknown_flags(interface_flags);
		std::cout << '\n';
	}
	
	if(asp.falling) std::cout << "Falling: " << asp.falling << '\n';
	std::cout << "Gold: " << asp.gold << '\n';
	if(asp.invisibility != 0.f) std::cout << "Invisibility: " << asp.invisibility << '\n';
	std::string inzone = boost::to_lower_copy(util::loadString(asp.inzone));
	if(!inzone.empty()) std::cout << "In zone: " << inzone << '\n';
	
	if(asp.jumpphase) {
		std::cout << "Jump phase: ";
		switch(asp.jumpphase) {
			case 1: std::cout << "anticipation"; break;
			case 2: std::cout << "moving up"; break;
			case 3: std::cout << "moving down"; break;
			case 4: std::cout << "finishing"; break;
			default: std::cout << "(unknown: " << asp.jumpphase << ')';
		}
		std::cout << '\n';
	}
	
	if(asp.Last_Movement != asp.Current_Movement) {
		std::cout << "Last movement:";
		print_player_movement(asp.Last_Movement);
		std::cout << '\n';
	}
	
	std::cout << "Level: " << asp.level << '\n';
	std::cout << "Life: " << asp.life << " / " << asp.maxlife << '\n';
	std::cout << "Mana: " << asp.mana << " / " << asp.maxmana << '\n';
	if(asp.poison != 0.f) std::cout << "Poison: " << asp.poison << '\n';
	if(asp.hunger != 0.f) std::cout << "Hunger: " << asp.hunger << '\n';
	std::cout << "Number of bags: " << asp.bag << '\n';
	
	if(asp.misc_flags) {
		std::cout << "Misc flags:";
		s16 interface_flags = asp.misc_flags;
		print_flag(interface_flags, 1, "on_firm_ground");
		print_flag(interface_flags, 2, "will_return_to_combat_mode");
		print_unknown_flags(interface_flags);
		std::cout << '\n';
	}
	
	std::cout << "Position: " << asp.pos << '\n';
	std::cout << "Magic resistance: " << asp.resist_magic << '\n';
	std::cout << "Poison resistance: " << asp.resist_poison << '\n';
	if(asp.Attribute_Redistribute) std::cout << "Available attribute points: " << asp.Attribute_Redistribute << '\n';
	if(asp.Skill_Redistribute) std::cout << "Availbale skill points: " << asp.Skill_Redistribute << '\n';
	
	if(asp.rune_flags) {
		std::cout << "Runes:";
		u32 runes = asp.rune_flags;
		print_flag(runes, FLAG_AAM, "aam");
		print_flag(runes, FLAG_CETRIUS, "cetrius");
		print_flag(runes, FLAG_COMUNICATUM, "comunicatum");
		print_flag(runes, FLAG_COSUM, "cosum");
		print_flag(runes, FLAG_FOLGORA, "folgora");
		print_flag(runes, FLAG_FRIDD, "fridd");
		print_flag(runes, FLAG_KAOM, "kaom");
		print_flag(runes, FLAG_MEGA, "mega");
		print_flag(runes, FLAG_MORTE, "morte");
		print_flag(runes, FLAG_MOVIS, "movis");
		print_flag(runes, FLAG_NHI, "nhi");
		print_flag(runes, FLAG_RHAA, "rhaa");
		print_flag(runes, FLAG_SPACIUM, "spacium");
		print_flag(runes, FLAG_STREGUM, "stregum");
		print_flag(runes, FLAG_TAAR, "taar");
		print_flag(runes, FLAG_TEMPUS, "tempus");
		print_flag(runes, FLAG_TERA, "tera");
		print_flag(runes, FLAG_VISTA, "vista");
		print_flag(runes, FLAG_VITAE, "vitae");
		print_flag(runes, FLAG_YOK, "yok");
		print_unknown_flags(runes);
		std::cout << '\n';
	}
	std::cout << "Size: " << asp.size << '\n';
	
	std::cout << "\nAttributes:\n";
	std::cout << "  Constitution: " << asp.Attribute_Constitution << '\n';
	std::cout << "  Dexterity: " << asp.Attribute_Dexterity << '\n';
	std::cout << "  Mind: " << asp.Attribute_Mind << '\n';
	std::cout << "  Strength: " << asp.Attribute_Strength << '\n';
	
	std::cout << "\nSkills:\n";
	std::cout << "  Stealth: " << asp.Skill_Stealth << '\n';
	std::cout << "  Mecanism: " << asp.Skill_Mecanism << '\n';
	std::cout << "  Intuition: " << asp.Skill_Intuition << '\n';
	std::cout << "  Etheral link: " << asp.Skill_Etheral_Link << '\n';
	std::cout << "  Object knowledge: " << asp.Skill_Object_Knowledge << '\n';
	std::cout << "  Casting: " << asp.Skill_Casting << '\n';
	std::cout << "  Projectile: " << asp.Skill_Projectile << '\n';
	std::cout << "  Close combat: " << asp.Skill_Close_Combat << '\n';
	std::cout << "  Defense: " << asp.Skill_Defense << '\n';
	
	// TODO asp.minimap
	
	print_animations(asp.anims);
	
	std::cout << "\nPhysics:\n";
	print_physics(asp.physics);
	
	for(s16 i = 0; i < asp.bag; i++) {
		std::cout << "\nBag #" << i << ":\n";
		print_inventory(save, asp.id_inventory[i], asp.inventory_show[i]);
	}
	
	if(asp.nb_PlayerQuest) {
		std::cout << "\nQuests:\n";
		if(size < pos + (asp.nb_PlayerQuest * SAVED_QUEST_SLOT_SIZE)) {
			std::cerr << "truncated data\n";
			return -1;
		}
		for(int i = 0; i < asp.nb_PlayerQuest; i++) {
			std::string quest = loadUnlocalized(boost::to_lower_copy(util::loadString(dat + pos, SAVED_QUEST_SLOT_SIZE)));
			std::cout << "  - " << quest << " = \"" << getLocalised(quest) << "\"\n";
			pos += SAVED_QUEST_SLOT_SIZE;
		}
	}
	
	if(asp.keyring_nb) {
		std::cout << "\nKeys:\n";
		if(size < pos + (asp.keyring_nb * SAVED_KEYRING_SLOT_SIZE)) {
			std::cerr << "truncated data\n";
			return -1;
		}
		for(int i = 0; i < asp.keyring_nb; i++) {
			std::string key = boost::to_lower_copy(util::loadString(dat + pos, SAVED_KEYRING_SLOT_SIZE));
			std::cout << " - " << key << '\n';
			pos += SAVED_KEYRING_SLOT_SIZE;
		}
	}
	
	if(asp.Nb_Mapmarkers) {
		std::cout << "\nMap markers:\n";
		if(size < pos + (asp.Nb_Mapmarkers * sizeof(SavedMapMarkerData))) {
			std::cerr << "truncated data\n";
			return -1;
		}
		for(int i = 0; i < asp.Nb_Mapmarkers; i++) {
			const SavedMapMarkerData * acmd = reinterpret_cast<const SavedMapMarkerData *>(dat + pos);
			pos += sizeof(SavedMapMarkerData);
			std::string name = loadUnlocalized(boost::to_lower_copy(util::loadString(acmd->name)));
			
			std::cout << " - (" << acmd->x << ", " << acmd->y << " @lvl" << std::setw(3) << std::setfill('0') << acmd->lvl << ") " << name << " =\"" << getLocalised(name) << "\"\n";
		}
	}
	
	for(size_t i = 0; i < SAVED_MAX_PRECAST; i++) {
		
		const SavedPrecast & p = asp.precast[i];
		
		if(p.typ < 0) {
			continue;
		}
		
		std::cout << "\nPrecast #" << i << ":\n";
		
		std::cout << "  Spell: ";
		print_spell(p.typ);
		std::cout << '\n';
		std::cout << "  Level: " << p.level << '\n';
		std::cout << "  Launch time: " << p.launch_time << '\n';
		std::cout << "  Duration: " << p.duration << '\n';
		std::cout << "  Flags:";
		print_spellcast_flags(p.flags);
		std::cout << '\n';
		
	}
	
	std::cout << "\nEquipment:\n";
	
	print_item(save, asp.equipsecondaryIO, "Secondary");
	print_item(save, asp.equipshieldIO, "Shield");
	print_item(save, asp.leftIO, "Left");
	print_item(save, asp.rightIO, "Right");
	print_item(save, asp.curtorch, "Torch");
	
	assert(SAVED_MAX_EQUIPED == MAX_EQUIPED);
	for(size_t i = 0; i < SAVED_MAX_EQUIPED; i++) {
		print_item(save, asp.equiped[i], equip_slot_name(i));
	}
	
	arx_assert(size >= pos);
	ARX_UNUSED(size);
	
	return 0;
}

struct PlayingAmbiance {
	char name[256];
	f32 volume;
	s32 loop;
	s32 type;
};

static int view_level(SaveBlock & save, const char * dat, size_t size) {
	
	size_t pos = 0;
	const ARX_CHANGELEVEL_INDEX & asi = *reinterpret_cast<const ARX_CHANGELEVEL_INDEX *>(dat + pos);
	pos += sizeof(ARX_CHANGELEVEL_INDEX);
	
	if(asi.version != ARX_GAMESAVE_VERSION) {
		std::cout << "bad version: " << asi.version << '\n';
		return 3;
	}
	
	std::cout << "Game time: " << asi.time << '\n';
	std::cout << "Dynamic lights: " << asi.nb_lights << '\n';
	
	if(asi.nb_inter) {
		
		std::cout << "\nInteractive objects:\n";
		
		for(s32 i = 0; i < asi.nb_inter; i++) {
			
			const ARX_CHANGELEVEL_IO_INDEX & io = *reinterpret_cast<const ARX_CHANGELEVEL_IO_INDEX *>(dat + pos);
			pos += sizeof(ARX_CHANGELEVEL_IO_INDEX);
			
			std::ostringstream oss;
			oss << res::path::load(util::loadString(io.filename)).basename() << '_'
			    << std::setfill('0') << std::setw(4) << io.ident;
			
			std::cout << "  - ";
			print_ident(save, oss.str());
			std::cout << '\n';
		}
		
	}
	
	if(asi.nb_paths) {
		
		std::cout << "\nPaths:\n";
		
		for(s32 i = 0; i < asi.nb_paths; i++) {
			
			const ARX_CHANGELEVEL_PATH & p = *reinterpret_cast<const ARX_CHANGELEVEL_PATH *>(dat + pos);
			pos += sizeof(ARX_CHANGELEVEL_PATH);
			
			std::cout << "  - " << boost::to_lower_copy(util::loadString(p.name));
			std::string controller = boost::to_lower_copy(util::loadString(p.controled));
			if(!controller.empty() && controller != "none") {
				std::cout << ": controlled by ";
				print_ident(save, controller);
			}
			std::cout << '\n';
			
		}
		
	}
	
	arx_assert(asi.ambiances_data_size % sizeof(PlayingAmbiance) == 0);
	
	// Restore Ambiances
	for(size_t i = 0; i < asi.ambiances_data_size / sizeof(PlayingAmbiance); i++) {
		
		const PlayingAmbiance & a = *reinterpret_cast<const PlayingAmbiance *>(dat + pos);
		pos += sizeof(PlayingAmbiance);
		
		std::cout << "\nAmbiance #" << i << ":\n";
		std::cout << "  Name: " << res::path::load(a.name) << '\n';
		if(a.volume != 1) std::cout << "  Volume: " << a.volume << '\n';
		if(a.loop) {
			std::cout << "  Loop mode: ";
			switch(a.loop) {
				case 1: std::cout << "play once"; break;
				default: std::cout << "(unknown: " << a.loop << ')';
			}
			std::cout << '\n';
		}
		std::cout << "  Type: ";
		switch(a.type) {
			case 0: std::cout << "menu"; break;
			case 1: std::cout << "script"; break;
			case 2: std::cout << "zone"; break;
			default: std::cout << "(unknown: " << a.type << ')';
		}
		std::cout << '\n';
	}
	
	arx_assert(size >= pos);
	ARX_UNUSED(size);
	
	return 0;
}

static void print_io_header(SaveBlock & save, const ARX_CHANGELEVEL_IO_SAVE & ais) {
	
	std::cout << "Type: "; print_type(ais.savesystem_type); std::cout << '\n';
	
	std::cout << "Filename: " << res::path::load(util::loadString(ais.filename)) << '\n';
	std::cout << "Instance: " << ais.ident << '\n';
	
	std::cout << "Flags:";
	if(!ais.ioflags) std::cout << " (none)";
	if(ais.ioflags & IO_UNDERWATER) std::cout << " underwater";
	if(ais.ioflags & IO_FREEZESCRIPT) std::cout << " freezescript";
	if(ais.ioflags & IO_ITEM) std::cout << " item";
	if(ais.ioflags & IO_NPC) std::cout << " npc";
	if(ais.ioflags & IO_FIX) std::cout << " fixed";
	if(ais.ioflags & IO_NOSHADOW) std::cout << " noshadow";
	if(ais.ioflags & IO_CAMERA) std::cout << " camera";
	if(ais.ioflags & IO_MARKER) std::cout << " marker";
	if(ais.ioflags & IO_ICONIC) std::cout << " iconic";
	if(ais.ioflags & IO_NO_COLLISIONS) std::cout << " no_collisions";
	if(ais.ioflags & IO_GOLD) std::cout << " gold";
	if(ais.ioflags & IO_INVULNERABILITY) std::cout << " invulnerability";
	if(ais.ioflags & IO_NO_PHYSICS_INTERPOL) std::cout << " no_physics_interpol";
	if(ais.ioflags & IO_HIT) std::cout << " hit";
	if(ais.ioflags & IO_PHYSICAL_OFF) std::cout << " physical_off";
	if(ais.ioflags & IO_MOVABLE) std::cout << " movable";
	if(ais.ioflags & IO_UNIQUE) std::cout << " unique";
	if(ais.ioflags & IO_SHOP) std::cout << " shop";
	if(ais.ioflags & IO_BLACKSMITH) std::cout << " blacksmith";
	if(ais.ioflags & IO_NOSAVE) std::cout << " nosave";
	if(ais.ioflags & IO_FORCEDRAW) std::cout << " forcedraw";
	if(ais.ioflags & IO_FIELD) std::cout << " field";
	if(ais.ioflags & IO_BUMP) std::cout << " bump";
	if(ais.ioflags & IO_ANGULAR) std::cout << " angular";
	if(ais.ioflags & IO_BODY_CHUNK) std::cout << " body_chunk";
	if(ais.ioflags & IO_ZMAP) std::cout << " zmap";
	if(ais.ioflags & IO_INVERTED) std::cout << " inverted";
	if(ais.ioflags & IO_JUST_COLLIDE) std::cout << " just_collide";
	if(ais.ioflags & IO_FIERY) std::cout << " fiery";
	if(ais.ioflags & IO_NO_NPC_COLLIDE) std::cout << " no_npc_collide";
	if(ais.ioflags & IO_CAN_COMBINE) std::cout << " can_combine";
	if(ais.ioflags & (1 << 31)) std::cout << " (unknown)";
	std::cout << '\n';
	
	if(ais.pos.toVec3() != Vec3f(0.f) || ais.initpos.toVec3() != Vec3f(0.f)) {
		std::cout << "Position: " << ais.pos;
		if(ais.pos.toVec3() != ais.initpos.toVec3()) {
			std::cout << " initial: " << ais.initpos;
		}
		std::cout << '\n';
	}
	if(ais.lastpos.toVec3() != ais.pos.toVec3()) std::cout << "Last position: " << ais.lastpos << '\n';
	if(ais.move.toVec3() != Vec3f(0.f)) std::cout << "Movement: " << ais.move << '\n';
	if(ais.lastmove.toVec3() != ais.move.toVec3()) std::cout << "Last movement: " << ais.lastmove << '\n';
	if(Anglef(ais.angle) != Anglef() || Anglef(ais.initangle) != Anglef()) {
		std::cout << "Angle: " << ais.angle;
		if(Anglef(ais.angle) != Anglef(ais.initangle)) {
			std::cout << " initial: " << ais.initangle;
		}
		std::cout << '\n';
	}
	if(ais.scale != 1.f) std::cout << "Scale: " << ais.scale << '\n';
	if(ais.weight != 0.f) std::cout << "Weight: " << ais.weight << '\n';
	
	std::string locname = loadUnlocalized(boost::to_lower_copy(util::loadString(ais.locname)));
	if(!locname.empty()) std::cout << "Name: " << locname << " = \"" << getLocalised(locname) << "\"\n";
	
	if(ais.gameFlags) {
		std::cout << "Game flags:";
		if(ais.gameFlags & GFLAG_INTERACTIVITY) std::cout << " interactivity";
		if(ais.gameFlags & GFLAG_ISINTREATZONE) std::cout << " isintreatzone";
		if(ais.gameFlags & GFLAG_WASINTREATZONE) std::cout << " wasintreatzone";
		if(ais.gameFlags & GFLAG_NEEDINIT) std::cout << " needinit";
		if(ais.gameFlags & GFLAG_INTERACTIVITYHIDE) std::cout << " interactivityhide";
		if(ais.gameFlags & GFLAG_DOOR) std::cout << " door";
		if(ais.gameFlags & GFLAG_INVISIBILITY) std::cout << " invisibility";
		if(ais.gameFlags & GFLAG_NO_PHYS_IO_COL) std::cout << " no_phys_io_col";
		if(ais.gameFlags & GFLAG_VIEW_BLOCKER) std::cout << " view_blocker";
		if(ais.gameFlags & GFLAG_PLATFORM) std::cout << " platform";
		if(ais.gameFlags & GFLAG_ELEVATOR) std::cout << " elevator";
		if(ais.gameFlags & GFLAG_MEGAHIDE) std::cout << " megahide";
		if(ais.gameFlags & GFLAG_HIDEWEAPON) std::cout << " hideweapon";
		if(ais.gameFlags & GFLAG_NOGORE) std::cout << " nogore";
		if(ais.gameFlags & GFLAG_GOREEXPLODE) std::cout << " goreexplode";
		if(ais.gameFlags & GFLAG_NOCOMPUTATION) std::cout << " nocomputation";
		std::cout << '\n';
	}
	
	if(ais.material != MATERIAL_NONE) {
		std::cout << "Material: ";
		switch(ais.material) {
			case MATERIAL_NONE: std::cout << "(none)"; break;
			case MATERIAL_WEAPON: std::cout << "Weapon"; break;
			case MATERIAL_FLESH: std::cout << "Flesh"; break;
			case MATERIAL_METAL: std::cout << "Metal"; break;
			case MATERIAL_GLASS: std::cout << "Glass"; break;
			case MATERIAL_CLOTH: std::cout << "Cloth"; break;
			case MATERIAL_WOOD: std::cout << "Wood"; break;
			case MATERIAL_EARTH: std::cout << "Earth"; break;
			case MATERIAL_WATER: std::cout << "Water"; break;
			case MATERIAL_ICE: std::cout << "Ice"; break;
			case MATERIAL_GRAVEL: std::cout << "Gravel"; break;
			case MATERIAL_STONE: std::cout << "Stone"; break;
			case MATERIAL_FOOT_LARGE: std::cout << "Large foot"; break;
			case MATERIAL_FOOT_BARE: std::cout << "Bare foot"; break;
			case MATERIAL_FOOT_SHOE: std::cout << "Shoe"; break;
			case MATERIAL_FOOT_METAL: std::cout << "Metal foot"; break;
			case MATERIAL_FOOT_STEALTH: std::cout << "Stealth foot"; break;
			default: std::cout << "(unknown)";
		}
		std::cout << '\n';
	}
	
	std::cout << "Level: " << ais.level << " true level: " << ais.truelevel << '\n';
	if(ais.scriptload) std::cout << "Loaded by a script: " << ais.scriptload << '\n';
	
	std::cout << "Show: ";
	switch(ais.show) {
		case SHOW_FLAG_NOT_DRAWN: std::cout << "not drawn"; break;
		case SHOW_FLAG_IN_SCENE: std::cout << "in scene"; break;
		case SHOW_FLAG_LINKED: std::cout << "linked"; break;
		case SHOW_FLAG_IN_INVENTORY: std::cout << "in inventory"; break;
		case SHOW_FLAG_HIDDEN: std::cout << "hidden"; break;
		case SHOW_FLAG_TELEPORTING: std::cout << "teleporting"; break;
		case SHOW_FLAG_KILLED: std::cout << "killed"; break;
		case SHOW_FLAG_MEGAHIDE: std::cout << "megahide"; break;
		case SHOW_FLAG_ON_PLAYER: std::cout << "on player"; break;
		case SHOW_FLAG_DESTROYED: std::cout << "destroyed"; break;
		default: std::cout << "(unknown)";
	}
	std::cout << '\n';
	
	if(ais.collision) {
		std::cout << "Collision:";
		s16 iocf = ais.collision;
		print_flag(iocf, COLLIDE_WITH_PLAYER, "player");
		print_flag(iocf, COLLIDE_WITH_WORLD, "world");
		print_unknown_flags(iocf);
		std::cout << '\n';
	}
	
	std::string mainevent = boost::to_lower_copy(util::loadString(ais.mainevent));
	if(!mainevent.empty()) {
		std::cout << "Main script event: " << mainevent << '\n';
	}
	
	{
		std::string target = boost::to_lower_copy(util::loadString(ais.id_targetinfo));
		if(target != "self") {
			std::cout << "Target: ";
			print_ident(save, target);
			std::cout << '\n';
		}
	}
	if(ais.basespeed != 1.f) std::cout << "Base speed: " << ais.basespeed << '\n';
	if(ais.speed_modif != 0.f) std::cout << "Speed modifier: " << ais.speed_modif << '\n';
	if(ais.frameloss != 0.f) std::cout << "Frame loss: " << ais.frameloss << '\n';
	
	if(ais.spellcast_data.castingspell >= 0 || ais.spellcast_data.spell_flags) {
	
		if(ais.spellcast_data.castingspell >= 0) {
			std::cout << "Casting spell: ";
			print_spell(ais.spellcast_data.castingspell);
			std::cout << '\n';
		}
		
		std::cout << "Runes to draw:";
		for(size_t i = 0; i < 4; i++) {
			if(ais.spellcast_data.symb[i] == RUNE_NONE) {
				std::cout << " (none)";
			} else if(size_t(ais.spellcast_data.symb[i]) < ARRAY_SIZE(runeNames)) {
				std::cout << ' ' << runeNames[i];
			} else {
				std::cout << " (unknown)";
			}
		}
		std::cout << '\n';
		
		if(ais.spellcast_data.spell_flags) {
			std::cout << "Spell flags:";
			print_spellcast_flags(ais.spellcast_data.spell_flags);
			std::cout << '\n';
		}
	}
	
	if(ais.spellcast_data.spell_level) std::cout << "Spell level: " << ais.spellcast_data.spell_level << '\n';
	if(ais.spellcast_data.duration) std::cout << "Spell duration: " << ais.spellcast_data.duration << '\n';
	
	if(ais.rubber != 1.5f) std::cout << "Rubber: " << ais.rubber << '\n';
	if(ais.max_durability != 100) std::cout << "Max durability: " << ais.max_durability << '\n';
	if(ais.poisonous) std::cout << "Poisonous: " << ais.poisonous << '\n';
	if(ais.poisonous_count) std::cout << "Poisonous count: " << ais.poisonous_count << '\n';
	
	if(ais.head_rot != 0.f) std::cout << "Head rotation: " << ais.head_rot << '\n';
	if(ais.damager_damages) std::cout << "Damage dealt: " << ais.damager_damages << '\n';
	
	if(ais.damager_type) {
		std::cout << "Damage type:";
		if(ais.damager_type & DAMAGE_TYPE_FIRE) std::cout << " fire";
		if(ais.damager_type & DAMAGE_TYPE_MAGICAL) std::cout << " magical";
		if(ais.damager_type & DAMAGE_TYPE_LIGHTNING) std::cout << " lightning";
		if(ais.damager_type & DAMAGE_TYPE_COLD) std::cout << " cold";
		if(ais.damager_type & DAMAGE_TYPE_POISON) std::cout << " poison";
		if(ais.damager_type & DAMAGE_TYPE_GAS) std::cout << " gas";
		if(ais.damager_type & DAMAGE_TYPE_METAL) std::cout << " metal";
		if(ais.damager_type & DAMAGE_TYPE_WOOD) std::cout << " wood";
		if(ais.damager_type & DAMAGE_TYPE_STONE) std::cout << " stone";
		if(ais.damager_type & DAMAGE_TYPE_ACID) std::cout << " acid";
		if(ais.damager_type & DAMAGE_TYPE_ORGANIC) std::cout << " organic";
		if(ais.damager_type & DAMAGE_TYPE_PER_SECOND) std::cout << " per_second";
		if(ais.damager_type & DAMAGE_TYPE_DRAIN_LIFE) std::cout << " drain_life";
		if(ais.damager_type & DAMAGE_TYPE_DRAIN_MANA) std::cout << " drain_mana";
		if(ais.damager_type & DAMAGE_TYPE_PUSH) std::cout << " push";
		if(ais.damager_type & DAMAGE_TYPE_FAKEFIRE) std::cout << " fakefire";
		if(ais.damager_type & DAMAGE_TYPE_FIELD) std::cout << " field";
		if(ais.damager_type & DAMAGE_TYPE_NO_FIX) std::cout << " no_fix";
		std::cout << '\n';
	}
	
	if(ais.type_flags) print_item_type(std::cout << "Item type:", ais.type_flags) << '\n';
	
	std::string stepmat = boost::to_lower_copy(util::loadString(ais.stepmaterial));
	if(!stepmat.empty()) std::cout << "Step material: " << stepmat << '\n';
	
	std::string armormat = boost::to_lower_copy(util::loadString(ais.armormaterial));
	if(!armormat.empty()) std::cout << "Armor material: " << armormat << '\n';
	
	std::string weaponmat = boost::to_lower_copy(util::loadString(ais.weaponmaterial));
	if(!weaponmat.empty()) std::cout << "Weapon material: " << weaponmat << '\n';
	
	std::string strikespeech = loadUnlocalized(util::loadString(ais.strikespeech));
	if(!strikespeech.empty()) std::cout << "Strike speech: " << strikespeech << " = \"" << getLocalised(strikespeech) << "\"\n";
	
	if(ais.secretvalue != -1) std::cout << "Secret value: " << int(ais.secretvalue) << '\n';
	std::string shop = boost::to_lower_copy(util::loadString(ais.shop_category));
	if(!shop.empty()) std::cout << "Shop category: " << shop << '\n';
	if(ais.shop_multiply != 1) std::cout << "Shop multiply: " << ais.shop_multiply << '\n';
	
	if(ais.aflags) {
		std::cout << "Additional flags:";
		if(ais.aflags & 1) std::cout << " hit";
		if(ais.aflags & ~1) std::cout << " (unknown)";
		std::cout << '\n';
	}
	
	if(ais.ignition != 0.f) std::cout << "Ignition: " << ais.ignition << '\n';
	
	res::path invskin = res::path::load(util::loadString(ais.inventory_skin));
	if(!invskin.empty()) std::cout << "Inventory skin: " << invskin << '\n';
	
	print_animations(ais.anims);
	
	print_anim_layers(ais.animlayer);
	
	std::cout << "\nPhysics:\n";
	if(ais.velocity.toVec3() != Vec3f(0.f)) std::cout << "  Velocity: " << ais.velocity << '\n';
	if(ais.stopped) std::cout << "  Stopped: " << ais.stopped << '\n';
	print_physics(ais.physics);
	if(ais.original_radius != 0.f) std::cout << "  Original radius: " << ais.original_radius << '\n';
	if(ais.original_height != 0.f) std::cout << "  Original height: " << ais.original_height << '\n';
	
	for(size_t i = 0; s32(i) < ais.nb_linked; i++) {
		std::cout << "\nLinked object #" << i << ":\n";
		std::cout << "  Group: " << ais.linked_data[i].lgroup << '\n';
		std::cout << "  Indices: " << ais.linked_data[i].lidx << ", " << ais.linked_data[i].lidx2 << '\n';
		std::cout << "  Origin: " << ais.linked_data[i].modinfo.link_origin << '\n';
		std::cout << "  Position: " << ais.linked_data[i].modinfo.link_position << '\n';
		std::cout << "  Scale: " << ais.linked_data[i].modinfo.scale << '\n';
		std::cout << "  Rotation: " << ais.linked_data[i].modinfo.rot << '\n';
		std::cout << "  Flags: " << ais.linked_data[i].modinfo.flags << '\n';
		std::cout << "  Ident: ";
		print_ident(save, boost::to_lower_copy(util::loadString(ais.linked_data[i].linked_id)));
		std::cout << '\n';
	}
	
	if(ais.halo.flags) {
		std::cout << '\n';
		std::cout << "Halo color: " << ais.halo.color << '\n';
		std::cout << "Halo radius: " << ais.halo.radius << '\n';
		std::cout << "Halo flags:";
		if(ais.halo.flags & HALO_ACTIVE) std::cout << " active";
		if(ais.halo.flags & HALO_NEGATIVE) std::cout << " negative";
		if(ais.halo.flags & HALO_DYNLIGHT) std::cout << " dynlight";
		if(ais.halo.flags & ~7) std::cout << " (unknown)";
		std::cout << '\n';
		std::cout << "Halo offset: " << ais.halo.offset << '\n';
		std::cout << '\n';
	}
	
	std::string pathname = boost::to_lower_copy(util::loadString(ais.usepath_name));
	if(!pathname.empty() || ais.usepath_aupflags) {
		std::cout << "\nUse path: " << pathname;
		std::cout << "  start time: " << ais.usepath_starttime;
		std::cout << "  time: " << ais.usepath_curtime;
		std::cout << "  flags:";
		if(!ais.usepath_aupflags) std::cout << " (none)";
		if(ais.usepath_aupflags & ARX_USEPATH_FLAG_FINISHED) std::cout << " finished";
		if(ais.usepath_aupflags & ARX_USEPATH_WORM_SPECIFIC) std::cout << " worm";
		if(ais.usepath_aupflags & ARX_USEPATH_FOLLOW_DIRECTION) std::cout << " follow";
		if(ais.usepath_aupflags & ARX_USEPATH_FORWARD) std::cout << " forward";
		if(ais.usepath_aupflags & ARX_USEPATH_BACKWARD) std::cout << " backward";
		if(ais.usepath_aupflags & ARX_USEPATH_PAUSE) std::cout << " pause";
		if(ais.usepath_aupflags & ARX_USEPATH_FLAG_ADDSTARTPOS) std::cout << " addstartpos";
		std::cout << '\n';
		std::cout << "  initial position: " << ais.usepath_initpos << '\n';
		std::cout << "  last waypoint: " << ais.usepath_lastWP << '\n';
	}
	
}

static void print_io_timer(size_t i, const ARX_CHANGELEVEL_TIMERS_SAVE * ats) {
	
	std::cout << "\nTimer #" << i << ":\n";
	
	if(ats->flags) {
		std::cout << "  Flags:";
		if(ats->flags & 1) std::cout << " idle";
		if(ats->flags & ~1) std::cout << " (unknown)";
		std::cout << '\n';
	}
	
	std::cout << "  Script: " << (ats->script ? "overriding" : "base") << '\n';
	std::cout << "  Interval: " << ats->interval << "ms\n";
	std::cout << "  Name: " << boost::to_lower_copy(util::loadString(ats->name)) << '\n';
	std::cout << "  Position: " << ats->pos << '\n';
	std::cout << "  Remaining: " << ats->remaining << "ms\n";
	if(ats->count == 0) {
		std::cout << "  Count: ∞\n";
	} else {
		std::cout << "  Count: " << ats->count << '\n';
	}
	
}

static void print_io_script(size_t i, const ARX_CHANGELEVEL_SCRIPT_SAVE * ass) {
	
	if(i) {
		std::cout << "\nOverriding script:\n";
	} else {
		std::cout << "\nBase script:\n";
	}
	
	if(ass->allowevents) {
		std::cout << "  Disabled events:";
		if(ass->allowevents & DISABLE_HIT) std::cout << " hit";
		if(ass->allowevents & DISABLE_CHAT) std::cout << " chat";
		if(ass->allowevents & DISABLE_INVENTORY2_OPEN) std::cout << " inventory2_open";
		if(ass->allowevents & DISABLE_HEAR) std::cout << " hear";
		if(ass->allowevents & DISABLE_DETECT) std::cout << " detect";
		if(ass->allowevents & DISABLE_AGGRESSION) std::cout << " aggression";
		if(ass->allowevents & DISABLE_MAIN) std::cout << " main";
		if(ass->allowevents & DISABLE_COLLIDE_NPC) std::cout << " collide_npc";
		if(ass->allowevents & DISABLE_CURSORMODE) std::cout << " cursormode";
		if(ass->allowevents & DISABLE_EXPLORATIONMODE) std::cout << " explorationmode";
		std::cout << '\n';
	}
	
}

static void print_io_npc(SaveBlock & save, const ARX_CHANGELEVEL_IO_SAVE & ais,
                         const ARX_CHANGELEVEL_NPC_IO_SAVE * as) {
	
	std::cout << "\nNPC Data:\n";
	
	if(as->absorb != 0.f) std::cout << "  Absorption: " << as->absorb << '\n';
	if(as->aimtime != 0.f) std::cout << "  Aim time: " << as->aimtime << '\n';
	std::cout << "  Armor class: " << as->armor_class << '\n';
	
	if(as->behavior != BEHAVIOUR_NONE) print_behavior(std::cout << "  Behavior:", as->behavior) << '\n';
	if(as->behavior_param != 0.f) std::cout << "  Behavior parameter: " << as->behavior_param << '\n';
	
	if(as->collid_state) std::cout << "  Collision state: " << as->collid_state << '\n';
	if(as->collid_time) std::cout << "  Collision time: " << as->collid_time << '\n';
	if(as->cut) std::cout << "  Cut: " << as->cut << '\n';
	if(as->damages != 0.f) std::cout << "  Damages: " << as->damages << '\n';
	if(as->detect) std::cout << "  Detect: " << as->detect << '\n';
	if(as->fightdecision) std::cout << "  Fight decision: " << as->fightdecision << '\n';
	
	print_item(save, as->id_weapon, "Weapon");
	
	if(as->lastmouth != 0.f) std::cout << "  Last mouth: " << as->lastmouth << '\n';
	if(as->look_around_inc != 0.f) std::cout << "  Look around status: " << as->look_around_inc << '\n';
	std::cout << "  Life: " << as->life << " / " << as->maxlife << '\n';
	std::cout << "  Mana: " << as->mana << " / " << as->maxmana << '\n';
	
	print_movemode(std::cout << "  Movement mode: ", as->movemode) << '\n';
	
	if(as->moveproblem != 0.f) std::cout << "  Movement problem: " << as->moveproblem << '\n';
	if(as->reachedtarget) std::cout << "  Reached target: " << as->reachedtarget << '\n';
	if(as->speakpitch != 1) std::cout << "  Speak pitch: " << as->speakpitch << '\n';
	
	if(as->tactics) print_tactics(std::cout << "  Tactics: ", as->tactics) << '\n';
	
	std::cout << "  To hit: " << as->tohit << '\n';
	if(as->weaponinhand) std::cout << "  Weapon in hand: " << as->weaponinhand << '\n';
	if(as->weapontype) print_item_type(std::cout << "  Weapon type:", as->weapontype) << '\n';
	std::cout << "  XP: " << as->xpvalue << '\n';
	
	bool s = false;
	for(size_t i = 0; i < MAX_STACKED_BEHAVIOR; i++) {
		
		const SavedBehaviour & b = as->stacked[i];
		
		if(!b.exist) {
			continue;
		}
		s = true;
		
		std::cout << "\n  Stacked behavior #" << i << ":\n";
		
		print_behavior(std::cout << "    Behavior:", b.behavior) << '\n';
		std::cout << "    Behavior parameter: " << b.behavior_param << '\n';
		print_tactics(std::cout << "    Tactics: ", b.tactics) << '\n';
		print_movemode(std::cout << "    Movement mode: ", b.movemode) << '\n';
		
		print_anim_layers(b.animlayer, "    ");
		
	}
	
	if(s) {
		std::cout << '\n';
	}
	
	for(size_t i = 0; i < MAX_STACKED_BEHAVIOR; i++) {
		std::string target = boost::to_lower_copy(util::loadString(as->stackedtarget[i]));
		if(target != "none" && !target.empty()) {
			std::cout << "  Stacked target #" << i << ": ";
			print_ident(save, target);
			std::cout << '\n';
		}
	}
	
	std::cout << "  Critical: " << as->critical << '\n';
	std::cout << "  Reach: " << as->reach << '\n';
	if(as->backstab_skill != 0.f) std::cout << "  Backstab skill: " << as->backstab_skill << '\n';
	if(as->poisonned != 0.f) std::cout << "  Poisoned: " << as->poisonned << '\n';
	std::cout << "  Resist poison: " << int(as->resist_poison) << '\n';
	std::cout << "  Resist magic: " << int(as->resist_magic) << '\n';
	std::cout << "  Resist fire: " << int(as->resist_fire) << '\n';
	if(as->strike_time) std::cout << "  Strike time: " << as->strike_time << '\n';
	if(as->walk_start_time) std::cout << "  Walk start time: " << as->walk_start_time << '\n';
	if(as->aiming_start) std::cout << "  Aiming time: " << as->aiming_start << '\n';
	
	NPCFlags npcflags = NPCFlags::load(as->npcflags);
	if(npcflags) {
		std::cout << "  NPC flags:";
		if(npcflags & NPCFLAG_BACKSTAB) std::cout << " backstab";
		if(npcflags & ~NPCFLAG_BACKSTAB) std::cout << " (unknown)";
		std::cout << '\n';
	}
	
	std::cout << "  Detect: " << as->fDetect << '\n';
	if(as->cuts) std::cout << "  Cuts: " << as->cuts << '\n';
	
	if(as->pathfind.truetarget != TARGET_NONE) {
		std::cout << "  True target: ";
		switch(as->pathfind.truetarget) {
			case TARGET_PATH: std::cout << "path"; break;
			case TARGET_PLAYER: std::cout << "player"; break;
			default: std::cout << "(unknown)";
		}
		std::cout << '\n';
	}
	
	if(ais.saveflags & SAVEFLAGS_EXTRA_ROTATE) {
		
		std::cout << "\n  Extra rotate flags (unused): " << as->ex_rotate.flags << '\n';
		
		for(size_t i = 0; i < SAVED_MAX_EXTRA_ROTATE; i++) {
			std::cout << "    Extra rotate #" << i << ": group=" << as->ex_rotate.group_number[i] << " rotation=" << as->ex_rotate.group_rotate[i] << '\n';
		}
		
	}
	
	Color c = Color::fromBGRA(ColorBGRA(as->blood_color));
	if(c != Color::red) {
		std::cout << "  Blood color: (" << int(c.r) << ", " << int(c.g) << ", " << int(c.b) << ")\n";
	}
	
}

static void print_io_item(const ARX_CHANGELEVEL_IO_SAVE & ais,
                          const ARX_CHANGELEVEL_ITEM_IO_SAVE * ai) {
	
	std::cout << "\nItem Data:\n";
	
	std::cout << "  Price: " << ai->price << '\n';
	if(ai->count != 1 || ai->maxcount != 1) {
		std::cout << "  Count: " << ai->count << " / " << ai->maxcount << '\n';
	}
	if(ai->food_value != 0) {
		std::cout << "  Food value: " << int(ai->food_value) << '\n';
	}
	std::cout << "  Steal value: " << int(ai->stealvalue) << '\n';
	if(ai->playerstacksize != 1) {
		std::cout << "  Player stack size: " << ai->playerstacksize << '\n';
	}
	if(ai->LightValue != -1) {
		std::cout << "  Light value: " << ai->LightValue << '\n';
	}
	
	if(ais.system_flags & SYSTEM_FLAG_EQUIPITEMDATA) {
		
		for(size_t i = 0; i < SAVED_IO_EQUIPITEM_ELEMENT_Number; i++) {
			
			const SavedEquipItemElement & e = ai->equipitem.elements[i];
			
			if(e.value == 0 && e.flags == 0) {
				continue;
			}
			
			if(i < ARRAY_SIZE(equipitemNames)) {
				std::cout << "\n  " << equipitemNames[i] << " modifier:";
			} else {
				std::cout << "\n  EquipItem #" << i << ':';
			}
			
			if(e.flags || e.special) {
				std::cout << "\n    Value: ";
			} else {
				std::cout << ' ';
			}
			
			std::cout << e.value << '\n';
			if(e.flags) {
				std::cout << "    Flags:";
				if(e.flags & 1) std::cout << " percentile";
				if(e.flags & ~1) std::cout << " (unknown)";
				std::cout << '\n';
			}
			if(e.special) {
				std::cout << "    Special: ";
				if(e.special == IO_SPECIAL_ELEM_PARALYZE) {
					std::cout << "paralyze";
				} else if(e.special == IO_SPECIAL_ELEM_DRAIN_LIFE) {
					std::cout << "drain life";
				} else {
					std::cout << e.special;
				}
				std::cout << '\n';
			}
			
		}
	}
	
}

static void print_io_fixed(const ARX_CHANGELEVEL_FIX_IO_SAVE * af) {
	
	std::cout << "\nFixed Data:\n";
	
	if(af->trapvalue != -1) std::cout << "  Trap value: " << int(af->trapvalue) << '\n';
	
}

static void print_io_camera(const ARX_CHANGELEVEL_CAMERA_IO_SAVE * ac) {
	
	std::cout << "\nCamera Data:\n";
	
	std::cout << "  Transform:\n";
	std::cout << "    pos=(" << ac->cam.pos.x << ", " << ac->cam.pos.y << ", " << ac->cam.pos.z << ")\n";
	std::cout << "    ycos=" << ac->cam.ycos << " ysin=" << ac->cam.ysin << " xsin=" << ac->cam.xsin << " xcos=" << ac->cam.xcos << '\n';
	std::cout << "    use_focal=" << ac->cam.use_focal1 << '\n';
	std::cout << "    mod=(" << ac->cam.xmod << ", " << ac->cam.ymod << ", " << ac->cam.zmod << ")\n";
	
	std::cout << "  Position: " << ac->cam.pos2;
	std::cout << "  cos=(" << ac->cam.Xcos << ", " << ac->cam.Ycos << ", " << ac->cam.Zcos << ")\n";
	std::cout << "  sin=(" << ac->cam.Xsin << ", " << ac->cam.Ysin << ", " << ac->cam.Zsin << ")\n";
	std::cout << "  focal=" << ac->cam.focal << " use_focal=" << ac->cam.use_focal << '\n';
	std::cout << "  Zmul=" << ac->cam.Zmul << '\n';
	std::cout << "  posleft=" << ac->cam.posleft << " postop=" << ac->cam.postop << '\n';
	std::cout << "  xmod=" << ac->cam.xmod2 << " ymod=" << ac->cam.ymod2 << '\n';
	
	const SavedMatrix & m = ac->cam.matrix;
	std::cout << "  Matrix:\n";
	std::cout << "    " << m._11 << ", " << m._12 << ", " << m._13 << ", " << m._14 << '\n';
	std::cout << "    " << m._21 << ", " << m._22 << ", " << m._23 << ", " << m._24 << '\n';
	std::cout << "    " << m._31 << ", " << m._32 << ", " << m._33 << ", " << m._34 << '\n';
	std::cout << "    " << m._41 << ", " << m._42 << ", " << m._43 << ", " << m._44 << '\n';
	
	std::cout << "  Angle: " << ac->cam.angle << '\n';
	std::cout << "  Destination position: " << ac->cam.d_pos << '\n';
	std::cout << "  Destination angle: " << ac->cam.d_angle << '\n';
	std::cout << "  Last target: " << ac->cam.lasttarget << '\n';
	std::cout << "  Last position: " << ac->cam.lastpos << '\n';
	std::cout << "  Last translate target: " << ac->cam.translatetarget << '\n';
	std::cout << "  Last info valid: " << ac->cam.lastinfovalid << '\n';
	std::cout << "  Norm: " << ac->cam.norm << '\n';
	std::cout << "  Fade color: " << ac->cam.fadecolor << '\n';
	
	std::cout << "  Clip: (" << ac->cam.clip.left << ", " << ac->cam.clip.top << ") -- (" << ac->cam.clip.right << ", " << ac->cam.clip.bottom << ")\n";
	
	std::cout << "  clipz0: " << ac->cam.clipz0 << '\n';
	std::cout << "  clipz1: " << ac->cam.clipz1 << '\n';
	std::cout << "  centerx: " << ac->cam.centerx << '\n';
	std::cout << "  centery: " << ac->cam.centery << '\n';
	std::cout << "  smoothing: " << ac->cam.smoothing << '\n';
	std::cout << "  AddX: " << ac->cam.AddX << '\n';
	std::cout << "  AddY: " << ac->cam.AddY << '\n';
	std::cout << "  Xsnap: " << ac->cam.Xsnap << '\n';
	std::cout << "  Zsnap: " << ac->cam.Zsnap << '\n';
	std::cout << "  Zdiv: " << ac->cam.Zdiv << '\n';
	std::cout << "  clip3D: " << ac->cam.clip3D << '\n';
	
	std::cout << "  Type: ";
	switch(ac->cam.type) {
		case CAM_SUBJVIEW: std::cout << "subject view"; break;
		case CAM_TOPVIEW: std::cout << "top down view"; break;
		default: std::cout << "(unknown)";
	}
	std::cout << '\n';
	
	std::cout << "  bkgcolor: " << ac->cam.bkgcolor << '\n';
	std::cout << "  nbdrawn: " << ac->cam.nbdrawn << '\n';
	std::cout << "  cdepth: " << ac->cam.cdepth << '\n';
	std::cout << "  size: " << ac->cam.size << '\n';
	
}

static void print_io_tweaker(const SavedTweakerInfo * sti) {
	
	std::cout << "\nTweaker Data:\n";
	
	std::cout << "  Filename: " << res::path::load(util::loadString(sti->filename)) << '\n';
	std::cout << "  Old skin: \"" << boost::to_lower_copy(util::loadString(sti->skintochange)) << "\"\n";
	std::cout << "  New skin: " << res::path::load(util::loadString(sti->skinchangeto)) << '\n';
	
}

static void print_io_tweak(size_t i, const SavedTweakInfo * sti) {
	
	std::cout << "\nTweak #" << i << ":\n";
	
	std::cout << "  Type:";
	if(!sti->type) std::cout << " (none)";
	if(sti->type & TWEAK_REMOVE) std::cout << " remove";
	if(sti->type & TWEAK_HEAD) std::cout << " head";
	if(sti->type & TWEAK_TORSO) std::cout << " torso";
	if(sti->type & TWEAK_LEGS) std::cout << " legs";
	if(sti->type & TWEAK_TYPE_SKIN) std::cout << " skin";
	if(sti->type & TWEAK_TYPE_ICON) std::cout << " icon";
	if(sti->type & TWEAK_TYPE_MESH) std::cout << " mesh";
	std::cout << '\n';
	
	res::path param1 = res::path::load(util::loadString(sti->param1));
	if(!param1.empty()) {
		std::cout << "  Parameter 1: " << param1 << '\n';
	}
	
	res::path param2 = res::path::load(util::loadString(sti->param2));
	if(!param2.empty()) {
		std::cout << "  Parameter 2: " << param2 << '\n';
	}
	
}

static int view_io(SaveBlock & save, const char * dat, size_t size) {
	
	size_t pos = 0;
	const ARX_CHANGELEVEL_IO_SAVE & ais = *reinterpret_cast<const ARX_CHANGELEVEL_IO_SAVE *>(dat + pos);
	pos += sizeof(ARX_CHANGELEVEL_IO_SAVE);
	
	if(ais.version != ARX_GAMESAVE_VERSION) {
		std::cout << "bad version: " << ais.version << '\n';
		return 3;
	}
	
	print_io_header(save, ais);
	
	for(size_t i = 0; i < size_t(ais.nbtimers); i++) {
		const ARX_CHANGELEVEL_TIMERS_SAVE * ats;
		ats = reinterpret_cast<const ARX_CHANGELEVEL_TIMERS_SAVE *>(dat + pos);
		pos += sizeof(ARX_CHANGELEVEL_TIMERS_SAVE);
		print_io_timer(i, ats);
	}
	
	for(size_t i = 0; i < 2; i++) {
		
		const ARX_CHANGELEVEL_SCRIPT_SAVE * ass;
		ass = reinterpret_cast<const ARX_CHANGELEVEL_SCRIPT_SAVE *>(dat + pos);
		pos += sizeof(ARX_CHANGELEVEL_SCRIPT_SAVE);
		
		if(!ass->allowevents && !ass->nblvar) {
			continue;
		}
		
		print_io_script(i, ass);
		
		if(print_variables(ass->nblvar, dat, pos, "  ", TYPE_L_TEXT, TYPE_L_FLOAT, TYPE_L_LONG)) {
			return -1;
		}
		
	}
	
	switch(ais.savesystem_type) {
		
		case TYPE_NPC: {
			const ARX_CHANGELEVEL_NPC_IO_SAVE * as;
			as = reinterpret_cast<const ARX_CHANGELEVEL_NPC_IO_SAVE *>(dat + pos);
			pos += sizeof(ARX_CHANGELEVEL_NPC_IO_SAVE);
			print_io_npc(save, ais, as);
			break;
		}
		
		case TYPE_ITEM: {
			const ARX_CHANGELEVEL_ITEM_IO_SAVE * ai;
			ai = reinterpret_cast<const ARX_CHANGELEVEL_ITEM_IO_SAVE *>(dat + pos);
			pos += sizeof(ARX_CHANGELEVEL_ITEM_IO_SAVE);
			print_io_item(ais, ai);
			break;
		}
		
		case TYPE_FIX: {
			const ARX_CHANGELEVEL_FIX_IO_SAVE * af;
			af = reinterpret_cast<const ARX_CHANGELEVEL_FIX_IO_SAVE *>(dat + pos);
			pos += sizeof(ARX_CHANGELEVEL_FIX_IO_SAVE);
			print_io_fixed(af);
			break;
		}
		
		case TYPE_CAMERA: {
			const ARX_CHANGELEVEL_CAMERA_IO_SAVE * ac;
			ac = reinterpret_cast<const ARX_CHANGELEVEL_CAMERA_IO_SAVE *>(dat + pos);
			pos += sizeof(ARX_CHANGELEVEL_CAMERA_IO_SAVE);
			print_io_camera(ac);
			break;
		}
		
		case TYPE_MARKER: {
			pos += sizeof(ARX_CHANGELEVEL_MARKER_IO_SAVE);
			break;
		}
	}
	
	if(ais.system_flags & SYSTEM_FLAG_INVENTORY) {
		std::cout << "\nInventory:\n";
		const ARX_CHANGELEVEL_INVENTORY_DATA_SAVE & i = *reinterpret_cast<const ARX_CHANGELEVEL_INVENTORY_DATA_SAVE *>(dat + pos);
		pos += sizeof(ARX_CHANGELEVEL_INVENTORY_DATA_SAVE);
		std::cout << "  Size: " << i.sizex << 'x' << i.sizey << '\n';
		print_inventory(save, i.slot_io, i.slot_show);
	}
	
	if(ais.system_flags & SYSTEM_FLAG_TWEAKER_INFO) {
		const SavedTweakerInfo * sti = reinterpret_cast<const SavedTweakerInfo *>(dat + pos);
		pos += sizeof(SavedTweakerInfo);
		print_io_tweaker(sti);
	}
	
	if(ais.nb_iogroups) {
		std::cout << "\nIO groups:";
		for(s16 i = 0; i < ais.nb_iogroups; i++) {
			const SavedGroupData * sgd = reinterpret_cast<const SavedGroupData *>(dat + pos);
			pos += sizeof(SavedGroupData);
			std::cout << ' ' << boost::to_lower_copy(util::loadString(sgd->name));
		}
		std::cout << '\n';
	}
	
	for(size_t i = 0; i < size_t(ais.Tweak_nb); i++) {
		const SavedTweakInfo * sti = reinterpret_cast<const SavedTweakInfo *>(dat + pos);
		pos += sizeof(SavedTweakInfo);
		print_io_tweak(i, sti);
	}
	
	arx_assert(size >= pos);
	ARX_UNUSED(size);
	
	return 0;
}

static bool is_level(const std::string & name) {
	
	if(name.length() != 6) {
		return false;
	}
	
	if(name.compare(0, 3, "lvl", 3)) {
		return false;
	}
	
	return (isdigit(name[3]) && isdigit(name[4]) && isdigit(name[5]));
}

int main_view(SaveBlock & save, const std::vector<std::string> & args) {
	
	if(args.size() > 1) {
		return -1;
	}
	
	g_resources = new PakReader();
	
	do {
		// TODO share this list with the game code
		static const char * const filenames[2] = { "loc.pak", "loc_default.pak" };
		if(g_resources->addArchive(fs::findDataFile(filenames[0]))) {
			break;
		}
		if(filenames[1] && g_resources->addArchive(fs::findDataFile(filenames[1]))) {
			break;
		}
		std::ostringstream oss;
		oss << "Missing data file: \"" << filenames[0] << "\"";
		if(filenames[1]) {
			oss << " (or \"" << filenames[1] << "\")";
		}
		LogWarning << oss.str();
	} while(false);
	BOOST_REVERSE_FOREACH(const fs::path & base, fs::getDataDirs()) {
		const char * dirname = "localisation";
		g_resources->addFiles(base / dirname, dirname);
	}
	
	initLocalisation();
	
	if(!save.open()) {
		std::cerr << "failed to open savefile\n";
		return 2;
	}
	
	if(args.empty()) {
		
		std::cout << "\nInfo: pld\n";
		std::cout << "\nPlayer: player\n";
		
		std::cout << "\nLevels: \n";
		
		const long MAX_LEVEL = 24;
		for(long i = 0; i <= MAX_LEVEL; i++) {
			
			std::ostringstream ss;
			ss << "lvl" << std::setfill('0') << std::setw(3) << i;
			
			if(save.hasFile(ss.str())) {
				std::cout << " - " << ss.str() << '\n';
			}
		}
		
		return 0;
	}
	
	const std::string & name = args[0];
	
	std::string buffer = save.load(name);
	if(buffer.empty()) {
		std::cerr << name << " not found\n";
		return 3;
	}
	
	std::cout << '\n';
	
	int ret;
	if(name == "pld") {
		ret = view_pld(buffer.data(), buffer.size());
	} else if(name == "player") {
		ret = view_player(save, buffer.data(), buffer.size());
	} else if(name == "globals") {
		ret = view_globals(buffer.data(), buffer.size());
	} else if(is_level(name)) {
		ret = view_level(save, buffer.data(), buffer.size());
	} else {
		ret = view_io(save, buffer.data(), buffer.size());
	}
	
	return ret;
}
