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

#include "savetool/SaveView.h"

#include <iostream>
#include <iomanip>
#include <sstream>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/io/ios_state.hpp>

#include "Configure.h"

#include "ai/Paths.h"
#include "core/Localisation.h"
#include "core/Config.h"
#include "game/Player.h"
#include "gui/Interface.h"
#include "io/SaveBlock.h"
#include "io/resource/PakReader.h"
#include "scene/SaveFormat.h"
#include "scene/Interactive.h"
#include "util/String.h"

using std::endl;
using std::cerr;

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
		flags &= (int)~flag;
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
	
	if((size_t)spell < ARRAY_SIZE(spellNames)) {
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
				type = (VariableType)avs->type;
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
			
			std::cout << endl;
			if(type == s) {
				std::string value = boost::to_lower_copy(util::loadString(dat + pos, (long)avs->fval));
				pos += (long)avs->fval;
				std::cout << p << "  s " << name << " = \"" << value << '"';
			} else if(type == f) {
				std::cout << p << "  f " << name << " = " << avs->fval;
			} else if(type == l) {
				std::cout << p << "  l " << name << " = " << (long)avs->fval;
			}
			
		}
		std::cout << endl;
	}
	
	return 0;
}

static void print_animations(const char (&anims)[SAVED_MAX_ANIMS][256]) {
	
	std::cout << endl << "Animations:";
	bool hasAnims = false;
	for(size_t i = 0; i < SAVED_MAX_ANIMS; i++) {
		
		res::path anim = res::path::load(util::loadString(anims[i]));
		if(anim.empty()) {
			continue;
		}
		
		hasAnims = true;
		std::cout << endl;
		
		if(i < ARRAY_SIZE(animationNames)) {
			std::cout << "  - " << animationNames[i] << ": " << anim;
		} else {
			std::cout << "  - animation #" << i << ": " << anim;
		}
	}
	if(!hasAnims) {
		std::cout << " (none)";
	}
	std::cout << endl;
}

static void print_anim_layers(const SavedAnimUse animlayer[SAVED_MAX_ANIM_LAYERS],
                              const std::string & pf = std::string()) {
	
	for(size_t i = 0; i < SAVED_MAX_ANIM_LAYERS; i++) {
		const SavedAnimUse & layer = animlayer[i];
		
		if(layer.next_anim == ANIM_NONE && layer.cur_anim == ANIM_NONE) {
			continue;
		}
		
		std::cout << endl << pf << "Animation layer #" << i << ':' << endl;
		
		if(layer.next_anim != ANIM_NONE) {
			if((size_t)layer.next_anim < ARRAY_SIZE(animationNames)) {
				std::cout << pf << "  Next animation: " << animationNames[layer.next_anim] << endl;
			} else {
				std::cout << pf << "  Next animation: animation #" << layer.next_anim << endl;
			}
		}
		
		if(layer.cur_anim != ANIM_NONE) {
			if((size_t)layer.cur_anim < ARRAY_SIZE(animationNames)) {
				std::cout << pf << "  Current animation: " << animationNames[layer.cur_anim] << endl;
			} else {
				std::cout << pf << "  Current animation: animation #" << layer.cur_anim << endl;
			}
		}
		
		if(layer.altidx_next) std::cout << pf << "  Next alternative: " << layer.altidx_next << endl;
		if(layer.altidx_cur) std::cout << pf << "  Current alternative: " << layer.altidx_cur << endl;
		
		//std::cout << pf << "  ctim: " << layer.ctime << endl;
		
		if(layer.flags) print_anim_flags(std::cout << pf << "  Flags:", layer.flags) << endl;
		if(layer.nextflags) print_anim_flags(std::cout << pf << "  Next flags:", layer.nextflags) << endl;
		
		//std::cout << pf << "  Last frame: " << layer.lastframe << endl;
		//std::cout << pf << "  pour: " << layer.pour << endl;
		//std::cout << pf << "  fr: " << layer.fr << endl;
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
	if(physics.cyl.origin.toVec3() != Vec3f_ZERO || physics.cyl.radius || physics.cyl.height) std::cout << "  Cylinder: origin=" << physics.cyl.origin << " radius=" << physics.cyl.radius << " height=" << physics.cyl.height << endl;
	if(physics.startpos.toVec3() != Vec3f_ZERO) std::cout << "  Start position: " << physics.startpos << endl;
	if(physics.targetpos.toVec3() != Vec3f_ZERO) std::cout << "  Target position: " << physics.targetpos << endl;
	if(physics.velocity.toVec3() != Vec3f_ZERO) std::cout << "  Velocity: " << physics.velocity << endl;
	if(physics.forces.toVec3() != Vec3f_ZERO) std::cout << "  Forces: " << physics.forces << endl;
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
	
	size_t size;
	char * dat = save.load(ident, size);
	if(!dat) {
		std::cout << " (unknown)";
		return;
	}
	
	size_t pos = 0;
	ARX_CHANGELEVEL_IO_SAVE & ais = *reinterpret_cast<ARX_CHANGELEVEL_IO_SAVE *>(dat + pos);
	pos += sizeof(ARX_CHANGELEVEL_IO_SAVE);
	if(pos > size) {
		std::cout << " (bad save)";
		free(dat);
		return;
	} else if(ais.version != ARX_GAMESAVE_VERSION) {
		std::cout << " (bad version: " << ais.version << ')';
		free(dat);
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
	
	free(dat);
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
			
			std::cout << "  - (" << m << ", " << n << "): "; print_ident(save, name); std::cout << endl;
		}
	}
	if(!hasItems) {
		std::cout << "  (empty)" << endl;
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
	
	std::cout << "  " << name << ": "; print_ident(save, i); std::cout << endl;
}

static int view_pld(const char * dat, size_t size) {
	
	size_t pos = 0;
	const ARX_CHANGELEVEL_PLAYER_LEVEL_DATA & pld = *reinterpret_cast<const ARX_CHANGELEVEL_PLAYER_LEVEL_DATA *>(dat + pos);
	pos += sizeof(ARX_CHANGELEVEL_PLAYER_LEVEL_DATA);
	
	if(pld.version != ARX_GAMESAVE_VERSION) {
		std::cout << "bad version: " << pld.version << endl;
		return 3;
	}
	
	std::string name = util::loadString(pld.name);
	if(name == "ARX_QUICK_ARX" || name == "ARX_QUICK_ARX1") {
		std::cout << "Type: quicksave" << endl;
	} else {
		std::cout << "Name: \"" << name << '"' << endl;
	}
	
	std::cout << "Current level: "; print_level(pld.level); std::cout << endl;
	
	std::cout << "Game time: " << pld.time << endl;
	
	arx_assert(size >= pos); ARX_UNUSED(size), ARX_UNUSED(pos);
	
	return 0;
}

static int view_globals(const char * dat, size_t size) {
	
	size_t pos = 0;
	const ARX_CHANGELEVEL_SAVE_GLOBALS & pld = *reinterpret_cast<const ARX_CHANGELEVEL_SAVE_GLOBALS *>(dat + pos);
	pos += sizeof(ARX_CHANGELEVEL_SAVE_GLOBALS);
	
	if(pld.version != ARX_GAMESAVE_VERSION) {
		std::cout << "bad version: " << pld.version << endl;
		return 3;
	}
	
	print_variables(pld.nb_globals, dat, pos, "", TYPE_G_TEXT, TYPE_G_FLOAT, TYPE_G_LONG);
	
	arx_assert(size >= pos); ARX_UNUSED(size);
	
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
		std::cout << "bad player save version: " << asp.version << endl;
		//return 3;
	}
	
	std::cout << "Aim time: " << asp.AimTime << endl;
	std::cout << "Angle: " << asp.angle << endl;
	std::cout << "Armor class: " << asp.armor_class << endl;
	std::cout << "Critical hit: " << asp.Critical_Hit << endl;
	
	if(asp.Current_Movement) {
		std::cout << "Current movement:"; print_player_movement(asp.Current_Movement); std::cout << endl;
	}
	
	if(asp.damages) std::cout << "Damages: " << asp.damages << endl;
	if(asp.doingmagic) std::cout << "Doing magic: " << asp.doingmagic << endl;
	
	if(asp.playerflags) {
		std::cout << "Flags: ";
		s32 flags = asp.playerflags;
		print_flag(flags, PLAYERFLAGS_NO_MANA_DRAIN, "no_mana_drain");
		print_flag(flags, PLAYERFLAGS_INVULNERABILITY, "invulnerable");
		std::cout << endl;
	}
	
	if(!asp.Global_Magic_Mode) std::cout << "Magic disabled!" << endl;
	
	std::string teleportToLevel = boost::to_lower_copy(util::loadString(asp.TELEPORT_TO_LEVEL));
	if(!teleportToLevel.empty()) std::cout << "Teleporting to level: " << teleportToLevel << endl;
	
	std::string teleportToPosition = boost::to_lower_copy(util::loadString(asp.TELEPORT_TO_POSITION));
	if(!teleportToPosition.empty()) {
		std::cout << "Teleporting to: "; print_ident(save, teleportToPosition); std::cout << endl;
	}
	
	if(!teleportToLevel.empty() || !teleportToPosition.empty()) {
		std::cout << "Teleporting to angle: " << asp.TELEPORT_TO_ANGLE << endl;
	}
	
	if(asp.CHANGE_LEVEL_ICON != -1) std::cout << "Change level icon: " << asp.CHANGE_LEVEL_ICON << endl;
	
	if(asp.Interface) {
		std::cout << "Interface:";
		s16 interface_flags = asp.Interface;
		print_flag(interface_flags, INTER_MAP, "map");
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
		std::cout << endl;
	}
	
	if(asp.falling) std::cout << "Falling: " << asp.falling << endl;
	std::cout << "Gold: " << asp.gold << endl;
	if(asp.invisibility) std::cout << "Invisibility: " << asp.invisibility << endl;
	std::string inzone = boost::to_lower_copy(util::loadString(asp.inzone));
	if(!inzone.empty()) std::cout << "In zone: " << inzone << endl;
	
	if(asp.jumpphase) {
		std::cout << "Jump phase: ";
		switch(asp.jumpphase) {
			case 1: std::cout << "anticipation"; break;
			case 2: std::cout << "moving up"; break;
			case 3: std::cout << "moving down"; break;
			case 4: std::cout << "finishing"; break;
			default: std::cout << "(unknown: " << asp.jumpphase << ')';
		}
		std::cout << endl;
	}
	
	if(asp.Last_Movement != asp.Current_Movement) {
		std::cout << "Last movement:"; print_player_movement(asp.Last_Movement); std::cout << endl;
	}
	
	std::cout << "Level: " << asp.level << endl;
	std::cout << "Life: " << asp.life << " / " << asp.maxlife << endl;
	std::cout << "Mana: " << asp.mana << " / " << asp.maxmana << endl;
	if(asp.poison) std::cout << "Poison: " << asp.poison << endl;
	if(asp.hunger) std::cout << "Hunger: " << asp.hunger << endl;
	std::cout << "Number of bags: " << asp.bag << endl;
	
	if(asp.misc_flags) {
		std::cout << "Misc flags:";
		s16 interface_flags = asp.misc_flags;
		print_flag(interface_flags, 1, "on_firm_ground");
		print_flag(interface_flags, 2, "will_return_to_combat_mode");
		print_unknown_flags(interface_flags);
		std::cout << endl;
	}
	
	std::cout << "Position: " << asp.pos << endl;
	std::cout << "Magic resistance: " << asp.resist_magic << endl;
	std::cout << "Poison resistance: " << asp.resist_poison << endl;
	if(asp.Attribute_Redistribute) std::cout << "Available attribute points: " << asp.Attribute_Redistribute << endl;
	if(asp.Skill_Redistribute) std::cout << "Availbale skill points: " << asp.Skill_Redistribute << endl;
	
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
		std::cout << endl;
	}
	std::cout << "Size: " << asp.size << endl;
	
	std::cout << endl << "Attributes:" << endl;
	std::cout << "  Constitution: " << asp.Attribute_Constitution << endl;
	std::cout << "  Dexterity: " << asp.Attribute_Dexterity << endl;
	std::cout << "  Mind: " << asp.Attribute_Mind << endl;
	std::cout << "  Strength: " << asp.Attribute_Strength << endl;
	
	std::cout << endl << "Skills:" << endl;
	std::cout << "  Stealth: " << asp.Skill_Stealth << endl;
	std::cout << "  Mecanism: " << asp.Skill_Mecanism << endl;
	std::cout << "  Intuition: " << asp.Skill_Intuition << endl;
	std::cout << "  Etheral link: " << asp.Skill_Etheral_Link << endl;
	std::cout << "  Object knowledge: " << asp.Skill_Object_Knowledge << endl;
	std::cout << "  Casting: " << asp.Skill_Casting << endl;
	std::cout << "  Projectile: " << asp.Skill_Projectile << endl;
	std::cout << "  Close combat: " << asp.Skill_Close_Combat << endl;
	std::cout << "  Defense: " << asp.Skill_Defense << endl;
	
	// TODO asp.minimap
	
	print_animations(asp.anims);
	
	std::cout << endl << "Physics:" << endl;
	print_physics(asp.physics);
	
	for(s16 i = 0; i < asp.bag; i++) {
		std::cout << endl << "Bag #" << i << ':' << endl;
		print_inventory(save, asp.id_inventory[i], asp.inventory_show[i]);
	}
	
	if(asp.nb_PlayerQuest) {
		std::cout << endl << "Quests:" << endl;
		if(size < pos + (asp.nb_PlayerQuest * 80)) {
			cerr << "truncated data" << endl;
			return -1;
		}
		for(int i = 0; i < asp.nb_PlayerQuest; i++) {
			std::string quest = loadUnlocalized(boost::to_lower_copy(util::loadString(dat + pos, 80)));
			std::cout << "  - " << quest << " = \"" << getLocalised(quest) << '"' << endl;
			pos += 80;
		}
	}
	
	if(asp.keyring_nb) {
		std::cout << endl << "Keys:" << endl;
		if(size < pos + (asp.keyring_nb * SAVED_KEYRING_SLOT_SIZE)) {
			cerr << "truncated data" << endl;
			return -1;
		}
		for(int i = 0; i < asp.keyring_nb; i++) {
			std::string key = boost::to_lower_copy(util::loadString(dat + pos, SAVED_KEYRING_SLOT_SIZE));
			std::cout << " - " << key << endl;
			pos += SAVED_KEYRING_SLOT_SIZE;
		}
	}
	
	if(asp.Nb_Mapmarkers) {
		std::cout << endl << "Map markers:" << endl;
		if(size < pos + (asp.Nb_Mapmarkers * sizeof(SavedMapMarkerData))) {
			cerr << "truncated data" << endl;
			return -1;
		}
		for(int i = 0; i < asp.Nb_Mapmarkers; i++) {
			const SavedMapMarkerData * acmd = reinterpret_cast<const SavedMapMarkerData *>(dat + pos);
			pos += sizeof(SavedMapMarkerData);
			std::string name = loadUnlocalized(boost::to_lower_copy(util::loadString(acmd->name)));
			
			std::cout << " - (" << acmd->x << ", " << acmd->y << " @lvl" << std::setw(3) << std::setfill('0') << acmd->lvl << ") " << name << " =\"" << getLocalised(name) << '"' << endl;
		}
	}
	
	for(size_t i = 0; i < SAVED_MAX_PRECAST; i++) {
		
		const SavedPrecast & p = asp.precast[i];
		
		if(p.typ < 0) {
			continue;
		}
		
		std::cout << endl << "Precast #" << i << ':' << endl;
		
		std::cout << "  Spell: "; print_spell(p.typ); std::cout << endl;
		std::cout << "  Level: " << p.level << endl;
		std::cout << "  Launch time: " << p.launch_time << endl;
		std::cout << "  Duration: " << p.duration << endl;
		std::cout << "  Flags:"; print_spellcast_flags(p.flags); std::cout << endl;
		
	}
	
	std::cout << endl << "Equipment:" << endl;
	
	print_item(save, asp.equipsecondaryIO, "Secondary");
	print_item(save, asp.equipshieldIO, "Shield");
	print_item(save, asp.leftIO, "Left");
	print_item(save, asp.rightIO, "Right");
	print_item(save, asp.curtorch, "Torch");
	
	assert(SAVED_MAX_EQUIPED == MAX_EQUIPED);
	for(size_t i = 0; i < SAVED_MAX_EQUIPED; i++) {
		print_item(save, asp.equiped[i], equip_slot_name(i));
	}
	
	arx_assert(size >= pos); ARX_UNUSED(size);
	
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
		std::cout << "bad version: " << asi.version << endl;
		return 3;
	}
	
	std::cout << "Game time: " << asi.time << endl;
	std::cout << "Dynamic lights: " << asi.nb_lights << endl;
	
	if(asi.nb_inter) {
		
		std::cout << endl << "Interactive objects:" << endl;
		
		for(s32 i = 0; i < asi.nb_inter; i++) {
			
			const ARX_CHANGELEVEL_IO_INDEX & io = *reinterpret_cast<const ARX_CHANGELEVEL_IO_INDEX *>(dat + pos);
			pos += sizeof(ARX_CHANGELEVEL_IO_INDEX);
			
			std::ostringstream oss;
			oss << res::path::load(util::loadString(io.filename)).basename() << '_' << std::setfill('0') << std::setw(4) << io.ident;
			
			std::cout << "  - "; print_ident(save, oss.str()); std::cout << endl;
		}
		
	}
	
	if(asi.nb_paths) {
		
		std::cout << endl << "Paths:" << endl;
		
		for(s32 i = 0; i < asi.nb_paths; i++) {
			
			const ARX_CHANGELEVEL_PATH & p = *reinterpret_cast<const ARX_CHANGELEVEL_PATH*>(dat + pos);
			pos += sizeof(ARX_CHANGELEVEL_PATH);
			
			std::cout << "  - " << boost::to_lower_copy(util::loadString(p.name));
			std::string controller = boost::to_lower_copy(util::loadString(p.controled));
			if(!controller.empty() && controller != "none") {
				std::cout << ": controlled by "; print_ident(save, controller);
			}
			std::cout << endl;
		}
		
	}
	
	arx_assert(asi.ambiances_data_size % sizeof(PlayingAmbiance) == 0);
	
	// Restore Ambiances
	for(size_t i = 0; i < asi.ambiances_data_size / sizeof(PlayingAmbiance); i++) {
		
		const PlayingAmbiance & a = *reinterpret_cast<const PlayingAmbiance *>(dat + pos);
		pos += sizeof(PlayingAmbiance);
		
		std::cout << endl << "Ambiance #" << i << ':' << endl;
		std::cout << "  Name: " << res::path::load(a.name) << endl;
		if(a.volume != 1) std::cout << "  Volume: " << a.volume << endl;
		if(a.loop) {
			std::cout << "  Loop mode: ";
			switch(a.loop) {
				case 1: std::cout << "play once"; break;
				default: std::cout << "(unknown: " << a.loop << ')';
			}
			std::cout << endl;
		}
		std::cout << "  Type: ";
		switch(a.type) {
			case 0: std::cout << "menu"; break;
			case 1: std::cout << "script"; break;
			case 2: std::cout << "zone"; break;
			default: std::cout << "(unknown: " << a.type << ')';
		}
		std::cout << endl;
	}
	
	arx_assert(size >= pos); ARX_UNUSED(size), ARX_UNUSED(save);
	
	return 0;
}

static int view_io(SaveBlock & save, const char * dat, size_t size) {
	
	size_t pos = 0;
	const ARX_CHANGELEVEL_IO_SAVE & ais = *reinterpret_cast<const ARX_CHANGELEVEL_IO_SAVE *>(dat + pos);
	pos += sizeof(ARX_CHANGELEVEL_IO_SAVE);
	
	if(ais.version != ARX_GAMESAVE_VERSION) {
		std::cout << "bad version: " << ais.version << endl;
		return 3;
	}
	
	std::cout << "Type: "; print_type(ais.savesystem_type); std::cout << endl;
	
	std::cout << "Filename: " << res::path::load(util::loadString(ais.filename)) << endl;
	std::cout << "Instance: " << ais.ident << endl;
	
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
	if(ais.ioflags & (1<<31)) std::cout << " (unknown)";
	std::cout << endl;
	
	if(ais.pos.toVec3() != Vec3f_ZERO || ais.initpos.toVec3() != Vec3f_ZERO) {
		std::cout << "Position: " << ais.pos;
		if(ais.pos.toVec3() != ais.initpos.toVec3()) {
			std::cout << " initial: " << ais.initpos;
		}
		std::cout << endl;
	}
	if(ais.lastpos.toVec3() != ais.pos.toVec3()) std::cout << "Last position: " << ais.lastpos << endl;
	if(ais.move.toVec3() != Vec3f_ZERO) std::cout << "Movement: " << ais.move << endl;
	if(ais.lastmove.toVec3() != ais.move.toVec3()) std::cout << "Last movement: " << ais.lastmove << endl;
	if((Anglef)ais.angle != Anglef::ZERO || (Anglef)ais.initangle != Anglef::ZERO) {
		std::cout << "Angle: " << ais.angle;
		if((Anglef)ais.angle != (Anglef)ais.initangle) {
			std::cout << " initial: " << ais.initangle;
		}
		std::cout << endl;
	}
	if(ais.scale != 1) std::cout << "Scale: " << ais.scale << endl;
	if(ais.weight) std::cout << "Weight: " << ais.weight << endl;
	
	std::string locname = loadUnlocalized(boost::to_lower_copy(util::loadString(ais.locname)));
	if(!locname.empty()) std::cout << "Name: " << locname << " = \"" << getLocalised(locname) << '"' << endl;
	
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
		std::cout << endl;
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
		std::cout << endl;
	}
	
	std::cout << "Level: " << ais.level << " true level: " << ais.truelevel << endl;
	if(ais.scriptload) std::cout << "Loaded by a script: " << ais.scriptload << endl;
	
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
	std::cout << endl;
	
	if(ais.collision) {
		std::cout << "Collision:";
		s16 iocf = ais.collision;
		print_flag(iocf, COLLIDE_WITH_PLAYER, "player");
		print_flag(iocf, COLLIDE_WITH_WORLD, "world");
		print_unknown_flags(iocf);
		std::cout << endl;
	}
	
	std::string mainevent = boost::to_lower_copy(util::loadString(ais.mainevent));
	if(!mainevent.empty()) std::cout << "Main script event: " << mainevent << endl;
	
	std::string target = boost::to_lower_copy(util::loadString(ais.id_targetinfo));
	if(target != "self") {
		std::cout << "Target: "; print_ident(save, target); std::cout << endl;
	}
	if(ais.basespeed != 1) std::cout << "Base speed: " << ais.basespeed << endl;
	if(ais.speed_modif) std::cout << "Speed modifier: " << ais.speed_modif << endl;
	if(ais.frameloss) std::cout << "Frame loss: " << ais.frameloss << endl;
	
	if(ais.spellcast_data.castingspell >= 0 || ais.spellcast_data.spell_flags) {
	
		if(ais.spellcast_data.castingspell >= 0) {
			std::cout << "Casting spell: "; print_spell(ais.spellcast_data.castingspell); std::cout << endl;
		}
		
		std::cout << "Runes to draw:";
		for(size_t i = 0; i < 4; i++) {
			if(ais.spellcast_data.symb[i] == RUNE_NONE) {
				std::cout << " (none)";
			} else if((size_t)ais.spellcast_data.symb[i] < ARRAY_SIZE(runeNames)) {
				std::cout << ' ' << runeNames[i];
			} else {
				std::cout << " (unknown)";
			}
		}
		std::cout << endl;
		
		if(ais.spellcast_data.spell_flags) {
			std::cout << "Spell flags:"; print_spellcast_flags(ais.spellcast_data.spell_flags); std::cout << endl;
		}
	}
	
	if(ais.spellcast_data.spell_level) std::cout << "Spell level: " << ais.spellcast_data.spell_level << endl;
	if(ais.spellcast_data.duration) std::cout << "Spell duration: " << ais.spellcast_data.duration << endl;
	
	if(ais.rubber != 1.5f) std::cout << "Rubber: " << ais.rubber << endl;
	if(ais.max_durability != 100) std::cout << "Max durability: " << ais.max_durability << endl;
	if(ais.poisonous) std::cout << "Poisonous: " << ais.poisonous << endl;
	if(ais.poisonous_count) std::cout << "Poisonous count: " << ais.poisonous_count << endl;
	
	if(ais.head_rot) std::cout << "Head rotation: " << ais.head_rot << endl;
	if(ais.damager_damages) std::cout << "Damage dealt: " << ais.damager_damages << endl;
	
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
		std::cout << endl;
	}
	
	if(ais.type_flags) print_item_type(std::cout << "Item type:", ais.type_flags) << endl;
	
	std::string stepmat = boost::to_lower_copy(util::loadString(ais.stepmaterial));
	if(!stepmat.empty()) std::cout << "Step material: " << stepmat << endl;
	
	std::string armormat = boost::to_lower_copy(util::loadString(ais.armormaterial));
	if(!armormat.empty()) std::cout << "Armor material: " << armormat << endl;
	
	std::string weaponmat = boost::to_lower_copy(util::loadString(ais.weaponmaterial));
	if(!weaponmat.empty()) std::cout << "Weapon material: " << weaponmat << endl;
	
	std::string strikespeech = loadUnlocalized(util::loadString(ais.strikespeech));
	if(!strikespeech.empty()) std::cout << "Strike speech: " << strikespeech << " = \"" << getLocalised(strikespeech) << '"' << endl;
	
	if(ais.secretvalue != -1) std::cout << "Secret value: " << (int)ais.secretvalue << endl;
	std::string shop = boost::to_lower_copy(util::loadString(ais.shop_category));
	if(!shop.empty()) std::cout << "Shop category: " << shop << endl;
	if(ais.shop_multiply != 1) std::cout << "Shop multiply: " << ais.shop_multiply << endl;
	
	if(ais.aflags) {
		std::cout << "Additional flags:";
		if(ais.aflags & 1) std::cout << " hit";
		if(ais.aflags & ~1) std::cout << " (unknown)";
		std::cout << endl;
	}
	
	if(ais.ignition) std::cout << "Ignition: " << ais.ignition << endl;
	
	res::path invskin = res::path::load(util::loadString(ais.inventory_skin));
	if(!invskin.empty()) std::cout << "Inventory skin: " << invskin << endl;
	
	print_animations(ais.anims);
	
	print_anim_layers(ais.animlayer);
	
	std::cout << endl << "Physics:" << endl;
	if(ais.velocity.toVec3() != Vec3f_ZERO) std::cout << "  Velocity: " << ais.velocity << endl;
	if(ais.stopped) std::cout << "  Stopped: " << ais.stopped << endl;
	print_physics(ais.physics);
	if(ais.original_radius) std::cout << "  Original radius: " << ais.original_radius << endl;
	if(ais.original_height) std::cout << "  Original height: " << ais.original_height << endl;
	
	for(size_t i = 0; (s32)i < ais.nb_linked; i++) {
		std::cout << endl << "Linked object #" << i << ':' << endl;
		std::cout << "  Group: " << ais.linked_data[i].lgroup << endl;
		std::cout << "  Indices: " << ais.linked_data[i].lidx << ", " << ais.linked_data[i].lidx2 << endl;
		std::cout << "  Origin: " << ais.linked_data[i].modinfo.link_origin << endl;
		std::cout << "  Position: " << ais.linked_data[i].modinfo.link_position << endl;
		std::cout << "  Scale: " << ais.linked_data[i].modinfo.scale << endl;
		std::cout << "  Rotation: " << ais.linked_data[i].modinfo.rot << endl;
		std::cout << "  Flags: " << ais.linked_data[i].modinfo.flags << endl;
		std::cout << "  Ident: "; print_ident(save, boost::to_lower_copy(util::loadString(ais.linked_data[i].linked_id))); std::cout << endl;
	}
	
	if(ais.halo.flags) {
		std::cout << endl;
		std::cout << "Halo color: " << ais.halo.color << endl;
		std::cout << "Halo radius: " << ais.halo.radius << endl;
		std::cout << "Halo flags:";
		if(ais.halo.flags & HALO_ACTIVE) std::cout << " active";
		if(ais.halo.flags & HALO_NEGATIVE) std::cout << " negative";
		if(ais.halo.flags & HALO_DYNLIGHT) std::cout << " dynlight";
		if(ais.halo.flags & ~7) std::cout << " (unknown)";
		std::cout << endl;
		std::cout << "Halo offset: " << ais.halo.offset << endl;
		std::cout << endl;
	}
	
	std::string pathname = boost::to_lower_copy(util::loadString(ais.usepath_name));
	if(!pathname.empty() || ais.usepath_aupflags) {
		std::cout << endl << "Use path: " << pathname;
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
		std::cout << endl;
		std::cout << "  initial position: " << ais.usepath_initpos << endl;
		std::cout << "  last waypoint: " << ais.usepath_lastWP << endl;
	}
	
	for(int i = 0; i < ais.nbtimers; i++) {
		std::cout << endl << "Timer #" << i << ':' << endl;
		
		const ARX_CHANGELEVEL_TIMERS_SAVE * ats;
		ats = reinterpret_cast<const ARX_CHANGELEVEL_TIMERS_SAVE *>(dat + pos);
		pos += sizeof(ARX_CHANGELEVEL_TIMERS_SAVE);
		
		if(ats->flags) {
			std::cout << "  Flags:";
			if(ats->flags & 1) std::cout << " idle";
			if(ats->flags & ~1) std::cout << " (unknown)";
			std::cout << endl;
		}
		
		std::cout << "  Script: " << (ats->script ? "overriding" : "base") << endl;
		std::cout << "  Interval: " << ats->interval << "ms" << endl;
		std::cout << "  Name: " << boost::to_lower_copy(util::loadString(ats->name)) << endl;
		std::cout << "  Position: " << ats->pos << endl;
		std::cout << "  Remaining: " << ats->remaining << "ms" << endl;
		if(ats->count == 0) {
			std::cout << "  Count: ∞" << endl;
		} else {
			std::cout << "  Count: " << ats->count << endl;
		}
	}
	
	for(size_t i = 0; i < 2; i++) {
		
		const ARX_CHANGELEVEL_SCRIPT_SAVE * ass;
		ass = reinterpret_cast<const ARX_CHANGELEVEL_SCRIPT_SAVE *>(dat + pos);
		pos += sizeof(ARX_CHANGELEVEL_SCRIPT_SAVE);
		
		if(!ass->allowevents && !ass->nblvar) {
			continue;
		}
		
		if(i) {
			std::cout << endl << "Overriding script:" << endl;
		} else {
			std::cout << endl << "Base script:" << endl;
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
			std::cout << endl;
		}
		
		if(print_variables(ass->nblvar, dat, pos, "  ", TYPE_L_TEXT, TYPE_L_FLOAT, TYPE_L_LONG)) {
			return -1;
		}
	}
	
	switch(ais.savesystem_type) {
		
		case TYPE_NPC: {
			
			std::cout << endl << "NPC Data:" << endl;
			
			const ARX_CHANGELEVEL_NPC_IO_SAVE * as;
			as = reinterpret_cast<const ARX_CHANGELEVEL_NPC_IO_SAVE *>(dat + pos);
			pos += sizeof(ARX_CHANGELEVEL_NPC_IO_SAVE);
			
			if(as->absorb) std::cout << "  Absorption: " << as->absorb << endl;
			if(as->aimtime) std::cout << "  Aim time: " << as->aimtime << endl;
			std::cout << "  Armor class: " << as->armor_class << endl;
			
			if(as->behavior != BEHAVIOUR_NONE) print_behavior(std::cout << "  Behavior:", as->behavior) << endl;
			if(as->behavior_param) std::cout << "  Behavior parameter: " << as->behavior_param << endl;
			
			if(as->collid_state) std::cout << "  Collision state: " << as->collid_state << endl;
			if(as->collid_time) std::cout << "  Collision time: " << as->collid_time << endl;
			if(as->cut) std::cout << "  Cut: " << as->cut << endl;
			if(as->damages) std::cout << "  Damages: " << as->damages << endl;
			if(as->detect) std::cout << "  Detect: " << as->detect << endl;
			if(as->fightdecision) std::cout << "  Fight decision: " << as->fightdecision << endl;
			
			print_item(save, as->id_weapon, "Weapon");
			
			if(as->lastmouth) std::cout << "  Last mouth: " << as->lastmouth << endl;
			if(as->look_around_inc) std::cout << "  Look around status: " << as->look_around_inc << endl;
			std::cout << "  Life: " << as->life << " / " << as->maxlife << endl;
			std::cout << "  Mana: " << as->mana << " / " << as->maxmana << endl;
			
			print_movemode(std::cout << "  Movement mode: ", as->movemode) << endl;
			
			if(as->moveproblem) std::cout << "  Movement problem: " << as->moveproblem << endl;
			if(as->reachedtarget) std::cout << "  Reached target: " << as->reachedtarget << endl;
			if(as->speakpitch != 1) std::cout << "  Speak pitch: " << as->speakpitch << endl;
			
			if(as->tactics) print_tactics(std::cout << "  Tactics: ", as->tactics) << endl;
			
			std::cout << "  To hit: " << as->tohit << endl;
			if(as->weaponinhand) std::cout << "  Weapon in hand: " << as->weaponinhand << endl;
			if(as->weapontype) print_item_type(std::cout << "  Weapon type:", as->weapontype) << endl;
			std::cout << "  XP: " << as->xpvalue << endl;
			
			bool s = false;
			for(size_t i = 0; i < MAX_STACKED_BEHAVIOR; i++) {
				
				const SavedBehaviour & b = as->stacked[i];
				
				if(!b.exist) {
					continue;
				}
				s = true;
				
				std::cout << endl << "  Stacked behavior #" << i << ':' << endl;
				
				print_behavior(std::cout << "    Behavior:", b.behavior) << endl;
				std::cout << "    Behavior parameter: " << b.behavior_param << endl;
				print_tactics(std::cout << "    Tactics: ", b.tactics) << endl;
				print_movemode(std::cout << "    Movement mode: ", b.movemode) << endl;
				
				print_anim_layers(b.animlayer, "    ");
				
			}
			
			if(s) {
				std::cout << endl;
			}
			
			for(size_t i = 0; i < MAX_STACKED_BEHAVIOR; i++) {
				std::string target = boost::to_lower_copy(util::loadString(as->stackedtarget[i]));
				if(target != "none" && !target.empty()) {
					std::cout << "  Stacked target #" << i << ": "; print_ident(save, target); std::cout << endl;
				}
			}
			
			std::cout << "  Critical: " << as->critical << endl;
			std::cout << "  Reach: " << as->reach << endl;
			if(as->backstab_skill) std::cout << "  Backstab skill: " << as->backstab_skill << endl;
			if(as->poisonned) std::cout << "  Poisoned: " << as->poisonned << endl;
			std::cout << "  Resist poison: " << (int)as->resist_poison << endl;
			std::cout << "  Resist magic: " << (int)as->resist_magic << endl;
			std::cout << "  Resist fire: " << (int)as->resist_fire << endl;
			if(as->strike_time) std::cout << "  Strike time: " << as->strike_time << endl;
			if(as->walk_start_time) std::cout << "  Walk start time: " << as->walk_start_time << endl;
			if(as->aiming_start) std::cout << "  Aiming time: " << as->aiming_start << endl;
			
			NPCFlags npcflags = NPCFlags::load(as->npcflags);
			if(npcflags) {
				std::cout << "  NPC flags:";
				if(npcflags & NPCFLAG_BACKSTAB) std::cout << " backstab";
				if(npcflags & ~NPCFLAG_BACKSTAB) std::cout << " (unknown)";
				std::cout << endl;
			}
			
			std::cout << "  Detect: " << as->fDetect << endl;
			if(as->cuts) std::cout << "  Cuts: " << as->cuts << endl;
			
			if(as->pathfind.truetarget != TARGET_NONE) {
				std::cout << "  True target: ";
				switch(as->pathfind.truetarget) {
					case TARGET_PATH: std::cout << "path"; break;
					case TARGET_PLAYER: std::cout << "player"; break;
					default: std::cout << "(unknown)";
				}
				std::cout << endl;
			}
			
			if(ais.saveflags & SAVEFLAGS_EXTRA_ROTATE) {
				
				std::cout << endl << "  Extra rotate flags (unused): " << as->ex_rotate.flags << endl;
				
				for(size_t i = 0; i < SAVED_MAX_EXTRA_ROTATE; i++) {
					std::cout << "    Extra rotate #" << i << ": group=" << as->ex_rotate.group_number[i] << " rotation=" << as->ex_rotate.group_rotate[i] << endl;
				}
			}
			
			Color c = Color::fromBGRA(ColorBGRA(as->blood_color));
			if(c != Color::red) {
				std::cout << "  Blood color: (" << (int)c.r << ", " << (int)c.g << ", " << (int)c.b << ')' << endl;
			}
			
			break;
		}
		
		case TYPE_ITEM: {
			
			std::cout << endl << "Item Data:" << endl;
			
			const ARX_CHANGELEVEL_ITEM_IO_SAVE * ai;
			ai = reinterpret_cast<const ARX_CHANGELEVEL_ITEM_IO_SAVE *>(dat + pos);
			pos += sizeof(ARX_CHANGELEVEL_ITEM_IO_SAVE);
			
			std::cout << "  Price: " << ai->price << endl;
			if(ai->count != 1 || ai->maxcount != 1) {
				std::cout << "  Count: " << ai->count << " / " << ai->maxcount << endl;
			}
			if(ai->food_value != 0) {
				std::cout << "  Food value: " << (int)ai->food_value << endl;
			}
			std::cout << "  Steal value: " << (int)ai->stealvalue << endl;
			if(ai->playerstacksize != 1) {
				std::cout << "  Player stack size: " << ai->playerstacksize << endl;
			}
			if(ai->LightValue != -1) {
				std::cout << "  Light value: " << ai->LightValue << endl;
			}
			
			if(ais.system_flags & SYSTEM_FLAG_EQUIPITEMDATA) {
				
				for(size_t i = 0; i < SAVED_IO_EQUIPITEM_ELEMENT_Number; i++) {
					
					const SavedEquipItemElement & e = ai->equipitem.elements[i];
					
					if(e.value == 0 && e.flags == 0) {
						continue;
					}
					
					if(i < ARRAY_SIZE(equipitemNames)) {
						std::cout << endl << "  " << equipitemNames[i] << " modifier:";
					} else {
						std::cout << endl << "  EquipItem #" << i << ':';
					}
					
					if(e.flags || e.special) {
						std::cout << endl << "    Value: ";
					} else {
						std::cout << ' ';
					}
					
					std::cout << e.value << endl;
					if(e.flags) {
						std::cout << "    Flags:";
						if(e.flags & 1) std::cout << " percentile";
						if(e.flags & ~1) std::cout << " (unknown)";
						std::cout << endl;
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
						std::cout << endl;
					}
					
				}
			}
			
			break;
		}
		
		case TYPE_FIX: {
			
			std::cout << endl << "Fixed Data:" << endl;
			
			const ARX_CHANGELEVEL_FIX_IO_SAVE * af;
			af = reinterpret_cast<const ARX_CHANGELEVEL_FIX_IO_SAVE *>(dat + pos);
			pos += sizeof(ARX_CHANGELEVEL_FIX_IO_SAVE);
			
			if(af->trapvalue != -1) std::cout << "  Trap value: " << (int)af->trapvalue << endl;
			
			break;
		}
		
		case TYPE_CAMERA: {
			
			std::cout << endl << "Camera Data:" << endl;
			
			const ARX_CHANGELEVEL_CAMERA_IO_SAVE * ac;
			ac = reinterpret_cast<const ARX_CHANGELEVEL_CAMERA_IO_SAVE *>(dat + pos);
			pos += sizeof(ARX_CHANGELEVEL_CAMERA_IO_SAVE);
			
			const SavedTransform & t = ac->cam.transform;
			std::cout << "  Transform:" << endl;
			std::cout << "    pos=(" << t.pos.x << ", " << t.pos.y << ", " << t.pos.z << ')' << endl;
			std::cout << "    ycos=" << t.ycos << " ysin=" << t.ysin << " xsin=" << t.xsin << " xcos=" << t.xcos << endl;
			std::cout << "    use_focal=" << t.use_focal << endl;
			std::cout << "    mod=(" << t.xmod << ", " << t.ymod << ", " << t.zmod << ')' << endl;
			
			std::cout << "  Position: " << ac->cam.pos;
			std::cout << "  cos=(" << ac->cam.Xcos << ", " << ac->cam.Ycos << ", " << ac->cam.Zcos << ')' << endl;
			std::cout << "  sin=(" << ac->cam.Xsin << ", " << ac->cam.Ysin << ", " << ac->cam.Zsin << ')' << endl;
			std::cout << "  focal=" << ac->cam.focal << " use_focal=" << ac->cam.use_focal << endl;
			std::cout << "  Zmul=" << ac->cam.Zmul << endl;
			std::cout << "  posleft=" << ac->cam.posleft << " postop=" << ac->cam.postop << endl;
			std::cout << "  xmod=" << ac->cam.xmod << " ymod=" << ac->cam.ymod << endl;
			
			const SavedMatrix & m = ac->cam.matrix;
			std::cout << "  Matrix:" << endl;
			std::cout << "    " << m._11 << ", " << m._12 << ", " << m._13 << ", " << m._14 << endl;
			std::cout << "    " << m._21 << ", " << m._22 << ", " << m._23 << ", " << m._24 << endl;
			std::cout << "    " << m._31 << ", " << m._32 << ", " << m._33 << ", " << m._34 << endl;
			std::cout << "    " << m._41 << ", " << m._42 << ", " << m._43 << ", " << m._44 << endl;
			
			std::cout << "  Angle: " << ac->cam.angle << endl;
			std::cout << "  Destination position: " << ac->cam.d_pos << endl;
			std::cout << "  Destination angle: " << ac->cam.d_angle << endl;
			std::cout << "  Last target: " << ac->cam.lasttarget << endl;
			std::cout << "  Last position: " << ac->cam.lastpos << endl;
			std::cout << "  Last translate target: " << ac->cam.translatetarget << endl;
			std::cout << "  Last info valid: " << ac->cam.lastinfovalid << endl;
			std::cout << "  Norm: " << ac->cam.norm << endl;
			std::cout << "  Fade color: " << ac->cam.fadecolor << endl;
			
			std::cout << "  Clip: (" << ac->cam.clip.left << ", " << ac->cam.clip.top << ") -- (" << ac->cam.clip.right << ", " << ac->cam.clip.bottom << ')' << endl;
			
			std::cout << "  clipz0: " << ac->cam.clipz0 << endl;
			std::cout << "  clipz1: " << ac->cam.clipz1 << endl;
			std::cout << "  centerx: " << ac->cam.centerx << endl;
			std::cout << "  centery: " << ac->cam.centery << endl;
			std::cout << "  smoothing: " << ac->cam.smoothing << endl;
			std::cout << "  AddX: " << ac->cam.AddX << endl;
			std::cout << "  AddY: " << ac->cam.AddY << endl;
			std::cout << "  Xsnap: " << ac->cam.Xsnap << endl;
			std::cout << "  Zsnap: " << ac->cam.Zsnap << endl;
			std::cout << "  Zdiv: " << ac->cam.Zdiv << endl;
			std::cout << "  clip3D: " << ac->cam.clip3D << endl;
			
			std::cout << "  Type: ";
			switch(ac->cam.type) {
				case CAM_SUBJVIEW: std::cout << "subject view"; break;
				case CAM_TOPVIEW: std::cout << "top down view"; break;
				default: std::cout << "(unknown)";
			}
			std::cout << endl;
			
			std::cout << "  bkgcolor: " << ac->cam.bkgcolor << endl;
			std::cout << "  nbdrawn: " << ac->cam.nbdrawn << endl;
			std::cout << "  cdepth: " << ac->cam.cdepth << endl;
			std::cout << "  size: " << ac->cam.size << endl;
			
			break;
		}
		
		case TYPE_MARKER: {
			pos += sizeof(ARX_CHANGELEVEL_MARKER_IO_SAVE);
			break;
		}
	}
	
	if(ais.system_flags & SYSTEM_FLAG_INVENTORY) {
		
		std::cout << endl << "Inventory:" << endl;
		
		const ARX_CHANGELEVEL_INVENTORY_DATA_SAVE & i = *reinterpret_cast<const ARX_CHANGELEVEL_INVENTORY_DATA_SAVE *>(dat + pos);
		pos += sizeof(ARX_CHANGELEVEL_INVENTORY_DATA_SAVE);
		
		std::cout << "  Size: " << i.sizex << 'x' << i.sizey << endl;
		
		print_inventory(save, i.slot_io, i.slot_show);
		
	}
	
	if(ais.system_flags & SYSTEM_FLAG_TWEAKER_INFO) {
		
		std::cout << endl << "Tweaker Data:" << endl;
		
		const SavedTweakerInfo * sti = reinterpret_cast<const SavedTweakerInfo *>(dat + pos);
		pos += sizeof(SavedTweakerInfo);
		
		std::cout << "  Filename: " << res::path::load(util::loadString(sti->filename)) << endl;
		std::cout << "  Old skin: \"" << boost::to_lower_copy(util::loadString(sti->skintochange)) << '"' << endl;
		std::cout << "  New skin: " << res::path::load(util::loadString(sti->skinchangeto)) << endl;
	}
	
	if(ais.nb_iogroups) {
		std::cout << endl << "IO groups:";
		for(s16 i = 0; i < ais.nb_iogroups; i++) {
			const SavedGroupData * sgd = reinterpret_cast<const SavedGroupData *>(dat + pos);
			pos += sizeof(SavedGroupData);
			std::cout << ' ' << boost::to_lower_copy(util::loadString(sgd->name));
		}
		std::cout << endl;
	}
	
	for(s16 i = 0; i < ais.Tweak_nb; i++) {
		const SavedTweakInfo * sti = reinterpret_cast<const SavedTweakInfo *>(dat + pos);
		pos += sizeof(SavedTweakInfo);
		
		std::cout << endl << "Tweak #" << i << ':' << endl;
		
		std::cout << "  Type:";
		if(!sti->type) std::cout << " (none)";
		if(sti->type & TWEAK_REMOVE) std::cout << " remove";
		if(sti->type & TWEAK_HEAD) std::cout << " head";
		if(sti->type & TWEAK_TORSO) std::cout << " torso";
		if(sti->type & TWEAK_LEGS) std::cout << " legs";
		if(sti->type & TWEAK_TYPE_SKIN) std::cout << " skin";
		if(sti->type & TWEAK_TYPE_ICON) std::cout << " icon";
		if(sti->type & TWEAK_TYPE_MESH) std::cout << " mesh";
		std::cout << endl;
		
		res::path param1 = res::path::load(util::loadString(sti->param1));
		if(!param1.empty()) {
			std::cout << "  Parameter 1: " << param1 << endl;
		}
		
		res::path param2 = res::path::load(util::loadString(sti->param2));
		if(!param2.empty()) {
			std::cout << "  Parameter 2: " << param2 << endl;
		}
	}
	
	arx_assert(size >= pos); ARX_UNUSED(size);
	
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

int main_view(SaveBlock & save, int argc, char ** argv) {
	
	if(argc > 1) {
		return -1;
	}
	
	config.language = "english";
	
	resources = new PakReader();
	
	if(!resources->addArchive("loc.pak")) {
		cerr << "could not open loc.pak, run 'savetool view' from the game directory" << endl;
		return 3;
	}
	
	initLocalisation();
	
	if(!save.open()) {
		cerr << "failed to open savefile" << endl;
		return 2;
	}
	
	if(argc == 0) {
		
		std::cout << endl << "Info: pld" << endl;
		std::cout << endl << "Player: player" << endl;
		
		std::cout << endl << "Levels: " << endl;
		
		const long MAX_LEVEL = 24;
		for(long i = 0; i <= MAX_LEVEL; i++) {
			
			std::ostringstream ss;
			ss << "lvl" << std::setfill('0') << std::setw(3) << i;
			
			if(save.hasFile(ss.str())) {
				std::cout << " - " << ss.str() << endl;
			}
		}
		
		return 0;
	}
	
	std::string name = argv[0];
	
	size_t size;
	char * dat = save.load(name, size);
	if(!dat) {
		cerr << name << " not found" << endl;
		return 3;
	}
	
	std::cout << endl;
	
	int ret;
	if(name == "pld") {
		ret = view_pld(dat, size);
	} else if(name == "player") {
		ret = view_player(save, dat, size);
	} else if(name == "globals") {
		ret = view_globals(dat, size);
	} else if(is_level(name)) {
		ret = view_level(save, dat, size);
	} else {
		ret = view_io(save, dat, size);
	}
	
	free(dat);
	
	return ret;
}
