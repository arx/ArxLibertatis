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

#include "script/ScriptedAnimation.h"

#include <cstring>

#include "ai/Paths.h"
#include "core/GameTime.h"
#include "game/EntityManager.h"
#include "game/NPC.h"
#include "graphics/data/Mesh.h"
#include "io/resource/ResourcePath.h"
#include "scene/Interactive.h"
#include "script/ScriptUtils.h"

using std::string;
using std::strcpy;

namespace script {

namespace {

typedef std::map<string, AnimationNumber> Animations;
Animations animations;

AnimationNumber getAnimationNumber(const string & name) {
	
	Animations::const_iterator it = animations.find(name);
	
	return (it == animations.end()) ? ANIM_NONE : it->second;
}

class RotateCommand : public Command {
	
public:
	
	RotateCommand() : Command("rotate", AnyEntity) { }
	
	Result execute(Context & context) {
		
		Entity * io = context.getEntity();
		
		float t1 = context.getFloat();
		float t2 = context.getFloat();
		float t3 = context.getFloat();
		
		DebugScript(' ' << t1 << ' ' << t2 << ' ' << t3);
		
		io->angle.setYaw(io->angle.getYaw() + t1);
		io->angle.setPitch(io->angle.getPitch() + t2);
		io->angle.setRoll(io->angle.getRoll() + t3);
		
		io->animBlend.lastanimtime = 0;
		
		return Success;
	}
	
};

class ForceAnimCommand : public Command {
	
	static void forceAnim(Entity & io, ANIM_HANDLE * ea) {
		
		if(io.animlayer[0].cur_anim
		   && io.animlayer[0].cur_anim != io.anims[ANIM_DIE]
		   && io.animlayer[0].cur_anim != io.anims[ANIM_HIT1]) {
			AcquireLastAnim(&io);
		}
		
		FinishAnim(&io, io.animlayer[0].cur_anim);
		io.lastmove = Vec3f_ZERO;
		ANIM_Set(&io.animlayer[0], ea);
		io.animlayer[0].flags |= EA_FORCEPLAY;
		io.animlayer[0].nextflags = 0;
		
		CheckSetAnimOutOfTreatZone(&io, 0);
	}
	
public:
	
	ForceAnimCommand() : Command("forceanim", AnyEntity) { }
	
	Result execute(Context & context) {
		
		string anim = context.getWord();
		
		DebugScript(' ' << anim);
		
		AnimationNumber num = getAnimationNumber(anim);
		if(num == ANIM_NONE) {
			ScriptWarning << "unknown animation: " << anim;
			return Failed;
		}
		
		Entity & io = *context.getEntity();
		if(!io.anims[num]) {
			ScriptWarning << "animation " << anim << " not set";
			return Failed;
		}
		
		forceAnim(io, io.anims[num]);
		
		return Success;
	}
	
};

class ForceAngleCommand : public Command {
	
public:
	
	ForceAngleCommand() : Command("forceangle", AnyEntity) { }
	
	Result execute(Context & context) {
		
		float angle = MAKEANGLE(context.getFloat());
		
		DebugScript(' ' << angle);
		
		context.getEntity()->angle.setPitch(angle);
		
		return Success;
	}
	
};

class PlayAnimCommand : public Command {
	
	static void setNextAnim(Entity * io, ANIM_HANDLE * ea, long layer, bool loop, bool nointerpol) {
		
		if(IsDeadNPC(io)) {
			return;
		}
		
		if(!nointerpol) {
			AcquireLastAnim(io);
		}
		
		FinishAnim(io, io->animlayer[layer].cur_anim);
		ANIM_Set(&io->animlayer[layer], ea);
		io->animlayer[layer].next_anim = NULL;
		
		if(loop) {
			io->animlayer[layer].flags |= EA_LOOP;
		} else {
			io->animlayer[layer].flags &= ~EA_LOOP;
		}
		io->animlayer[layer].flags |= EA_FORCEPLAY;
	}
	
public:
	
	PlayAnimCommand() : Command("playanim") { }
	
	Result execute(Context & context) {
		
		Entity * iot = context.getEntity();
		long nu = 0;
		bool loop = false;
		bool nointerpol = false;
		bool execute = false;
		
		HandleFlags("123lnep") {
			if(flg & flag('1')) {
				nu = 0;
			}
			if(flg & flag('2')) {
				nu = 1;
			}
			if(flg & flag('3')) {
				nu = 2;
			}
			loop = test_flag(flg, 'l');
			nointerpol = test_flag(flg, 'n');
			execute = test_flag(flg, 'e');
			if(flg & flag('p')) {
				iot = entities.player();
				iot->move = iot->lastmove = Vec3f_ZERO;
			}
		}
		
		string anim = context.getWord();
		
		DebugScript(' ' << options << ' ' << anim);
		
		if(!iot) {
			ScriptWarning << "must either use -p or use with IO";
			return Failed;
		}
		
		ANIM_USE & layer = iot->animlayer[nu];
		
		if(anim == "none") {
			layer.cur_anim = NULL;
			layer.next_anim = NULL;
			return Success;
		}
		
		AnimationNumber num = getAnimationNumber(anim);
		if(num == ANIM_NONE) {
			ScriptWarning << "unknown anim: " << anim;
			return Failed;
		}
		
		if(!iot->anims[num]) {
			ScriptWarning << "animation " << anim << " not loaded";
			return Failed;
		}
		
		iot->ioflags |= IO_NO_PHYSICS_INTERPOL;
		setNextAnim(iot, iot->anims[num], nu, loop, nointerpol);
		
		if(!loop) {
			CheckSetAnimOutOfTreatZone(iot, nu);
		}
		
		if(iot == entities.player()) {
			layer.flags &= ~EA_STATICANIM;
		}
		
		if(execute) {
			
			string timername = "anim_" + ARX_SCRIPT_Timer_GetDefaultName();
			long num2 = ARX_SCRIPT_Timer_GetFree();
			if(num2 < 0) {
				ScriptError << "no free timer";
				return Failed;
			}
			
			size_t pos = context.skipCommand();
			if(pos != (size_t)-1) {
				scr_timer[num2].reset();
				ActiveTimers++;
				scr_timer[num2].es = context.getScript();
				scr_timer[num2].exist = 1;
				scr_timer[num2].io = context.getEntity();
				scr_timer[num2].msecs = 1000.f;
				// Don't assume that we successfully set the animation - use the current animation
				if(layer.cur_anim) {
					arx_assert(layer.altidx_cur >= 0 && layer.altidx_cur < layer.cur_anim->alt_nb);
					if(layer.cur_anim->anims[layer.altidx_cur]->anim_time > scr_timer[num2].msecs) {
						scr_timer[num2].msecs = layer.cur_anim->anims[layer.altidx_cur]->anim_time;
					}
				}
				scr_timer[num2].name = timername;
				scr_timer[num2].pos = pos;
				scr_timer[num2].tim = (unsigned long)(arxtime);
				scr_timer[num2].times = 1;
				scr_timer[num2].longinfo = 0;
			}
		}
		
		return Success;
	}
	
};

class LoadAnimCommand : public Command {
	
public:
	
	LoadAnimCommand() : Command("loadanim") { }
	
	Result execute(Context & context) {
		
		Entity * iot = context.getEntity();
		
		HandleFlags("p") {
			if(flg & flag('p')) {
				iot = entities.player();
			}
		}
		
		string anim = context.getWord();
		
		res::path file = res::path::load(context.getWord());
		
		DebugScript(' ' << options << ' ' << anim << ' ' << file);
		
		
		if(!iot) {
			ScriptWarning << "must either use -p or use with IO";
			return Failed;
		}
		
		AnimationNumber num = getAnimationNumber(anim);
		if(num == ANIM_NONE) {
			ScriptWarning << "unknown anim: " << anim;
			return Failed;
		}
		
		if(iot->anims[num]) {
			ReleaseAnimFromIO(iot, num);
		}
		
		if(file == "none") {
			iot->anims[num] = NULL;
			return Success;
		}
		
		res::path path;
		if(iot == entities.player() || (iot->ioflags & IO_NPC)) {
			path = ("graph/obj3d/anims/npc" / file).set_ext("tea");
		} else {
			path = ("graph/obj3d/anims/fix_inter" / file).set_ext("tea");
		}
		
		iot->anims[num] = EERIE_ANIMMANAGER_Load_NoWarning(path);
		
		if(!iot->anims[num]) {
			ScriptWarning << "animation not found: " << path;
			return Failed;
		}
		
		return Success;
	}
	
};

class MoveCommand : public Command {
	
public:
	
	MoveCommand() : Command("move", AnyEntity) { }
	
	Result execute(Context & context) {
		
		float dx = context.getFloat();
		float dy = context.getFloat();
		float dz = context.getFloat();
		
		DebugScript(' ' << dx << ' ' << dy << ' ' << dz);
		
		context.getEntity()->pos += Vec3f(dx, dy, dz);
		
		return Success;
	}
	
};

class UsePathCommand : public Command {
	
public:
	
	UsePathCommand() : Command("usepath", AnyEntity) { }
	
	Result execute(Context & context) {
		
		string type = context.getWord();
		
		DebugScript(' ' << type);
		
		ARX_USE_PATH * aup = context.getEntity()->usepath;
		if(!aup) {
			ScriptWarning << "no path set";
			return Failed;
		}
		
		if(type == "b") {
			aup->aupflags &= ~ARX_USEPATH_PAUSE;
			aup->aupflags &= ~ARX_USEPATH_FORWARD;
			aup->aupflags |= ARX_USEPATH_BACKWARD;
		} else if(type == "f") {
			aup->aupflags &= ~ARX_USEPATH_PAUSE;
			aup->aupflags |= ARX_USEPATH_FORWARD;
			aup->aupflags &= ~ARX_USEPATH_BACKWARD;
		} else if(type == "p") {
			aup->aupflags |= ARX_USEPATH_PAUSE;
			aup->aupflags &= ~ARX_USEPATH_FORWARD;
			aup->aupflags &= ~ARX_USEPATH_BACKWARD;
		} else {
			ScriptWarning << "unknown usepath type: " << type;
			return Failed;
		}
		
		return Success;
	}
	
};

class UnsetControlledZoneCommand : public Command {
	
public:
	
	UnsetControlledZoneCommand() : Command("unsetcontrolledzone") { }
	
	Result execute(Context & context) {
		
		string zone = context.getWord();
		
		DebugScript(' ' << zone);
		
		ARX_PATH * ap = ARX_PATH_GetAddressByName(zone);
		if(!ap) {
			ScriptWarning << "unknown zone: " << zone;
			return Failed;
		}
		
		ap->controled.clear();
		
		return Success;
	}
	
};

class SetPathCommand : public Command {
	
public:
	
	SetPathCommand() : Command("setpath", AnyEntity) { }
	
	Result execute(Context & context) {
		
		bool wormspecific = false;
		bool followdir = false;
		HandleFlags("wf") {
			wormspecific = test_flag(flg, 'w');
			followdir = test_flag(flg, 'f');
		}
		
		string name = context.getWord();
		
		DebugScript(' ' << options << ' ' << name);
		
		Entity * io = context.getEntity();
		if(name == "none") {
			free(io->usepath), io->usepath = NULL;
		} else {
			
			ARX_PATH * ap = ARX_PATH_GetAddressByName(name);
			if(!ap) {
				ScriptWarning << "unknown path: " << name;
				return Failed;
			}
			
			free(io->usepath), io->usepath = NULL;
			
			ARX_USE_PATH * aup = (ARX_USE_PATH *)malloc(sizeof(ARX_USE_PATH));
			aup->_starttime = aup->_curtime = arxtime;
			aup->aupflags = ARX_USEPATH_FORWARD;
			if(wormspecific) {
				aup->aupflags |= ARX_USEPATH_WORM_SPECIFIC | ARX_USEPATH_FLAG_ADDSTARTPOS;
			}
			if(followdir) {
				aup->aupflags |= ARX_USEPATH_FOLLOW_DIRECTION;
			}
			aup->initpos = io->initpos;
			aup->lastWP = -1;
			aup->path = ap;
			io->usepath = aup;
		}
		
		return Success;
	}
	
};

class SetControlledZoneCommand : public Command {
	
public:
	
	SetControlledZoneCommand() : Command("setcontrolledzone") { }
	
	Result execute(Context & context) {
		
		string name = context.getWord();
		
		DebugScript(' ' << name);
		
		ARX_PATH * ap = ARX_PATH_GetAddressByName(name);
		if(!ap) {
			ScriptWarning << "unknown zone: " << name;
			return Failed;
		}
		
		ap->controled = context.getEntity()->long_name();
		
		return Success;
	}
	
};

void initAnimationNumbers() {
	
	animations["wait"] = ANIM_WAIT;
	animations["wait2"] = ANIM_WAIT2;
	animations["walk"] = ANIM_WALK;
	animations["walk1"] = ANIM_WALK;
	animations["walk2"] = ANIM_WALK2;
	animations["walk3"] = ANIM_WALK3;
	animations["walk_backward"] = ANIM_WALK_BACKWARD;
	animations["walk_ministep"] = ANIM_WALK_MINISTEP;
	animations["wait_short"] = ANIM_WAIT_SHORT;
	animations["walk_sneak"] = ANIM_WALK_SNEAK;
	animations["action"] = ANIM_ACTION;
	animations["action1"] = ANIM_ACTION;
	animations["action2"] = ANIM_ACTION2;
	animations["action3"] = ANIM_ACTION3;
	animations["action4"] = ANIM_ACTION4;
	animations["action5"] = ANIM_ACTION5;
	animations["action6"] = ANIM_ACTION6;
	animations["action7"] = ANIM_ACTION7;
	animations["action8"] = ANIM_ACTION8;
	animations["action9"] = ANIM_ACTION9;
	animations["action10"] = ANIM_ACTION10;
	animations["hit1"] = ANIM_HIT1;
	animations["hit"] = ANIM_HIT1;
	animations["hold_torch"] = ANIM_HOLD_TORCH;
	animations["hit_short"] = ANIM_HIT_SHORT;
	animations["strike1"] = ANIM_STRIKE1;
	animations["strike"] = ANIM_STRIKE1;
	animations["shield_start"] = ANIM_SHIELD_START;
	animations["shield_cycle"] = ANIM_SHIELD_CYCLE;
	animations["shield_hit"] = ANIM_SHIELD_HIT;
	animations["shield_end"] = ANIM_SHIELD_END;
	animations["strafe_right"] = ANIM_STRAFE_RIGHT;
	animations["strafe_left"] = ANIM_STRAFE_LEFT;
	animations["strafe_run_left"] = ANIM_STRAFE_RUN_LEFT;
	animations["strafe_run_right"] = ANIM_STRAFE_RUN_RIGHT;
	animations["die"] = ANIM_DIE;
	animations["dagger_ready_part_1"] = ANIM_DAGGER_READY_PART_1;
	animations["dagger_ready_part_2"] = ANIM_DAGGER_READY_PART_2;
	animations["dagger_unready_part_1"] = ANIM_DAGGER_UNREADY_PART_1;
	animations["dagger_unready_part_2"] = ANIM_DAGGER_UNREADY_PART_2;
	animations["dagger_wait"] = ANIM_DAGGER_WAIT;
	animations["dagger_strike_left_start"] = ANIM_DAGGER_STRIKE_LEFT_START;
	animations["dagger_strike_left_cycle"] = ANIM_DAGGER_STRIKE_LEFT_CYCLE;
	animations["dagger_strike_left"] = ANIM_DAGGER_STRIKE_LEFT;
	animations["dagger_strike_right_start"] = ANIM_DAGGER_STRIKE_RIGHT_START;
	animations["dagger_strike_right_cycle"] = ANIM_DAGGER_STRIKE_RIGHT_CYCLE;
	animations["dagger_strike_right"] = ANIM_DAGGER_STRIKE_RIGHT;
	animations["dagger_strike_top_start"] = ANIM_DAGGER_STRIKE_TOP_START;
	animations["dagger_strike_top_cycle"] = ANIM_DAGGER_STRIKE_TOP_CYCLE;
	animations["dagger_strike_top"] = ANIM_DAGGER_STRIKE_TOP;
	animations["dagger_strike_bottom_start"] = ANIM_DAGGER_STRIKE_BOTTOM_START;
	animations["dagger_strike_bottom_cycle"] = ANIM_DAGGER_STRIKE_BOTTOM_CYCLE;
	animations["dagger_strike_bottom"] = ANIM_DAGGER_STRIKE_BOTTOM;
	animations["death_critical"] = ANIM_DEATH_CRITICAL;
	animations["run"] = ANIM_RUN;
	animations["run1"] = ANIM_RUN;
	animations["run2"] = ANIM_RUN2;
	animations["run3"] = ANIM_RUN3;
	animations["run_backward"] = ANIM_RUN_BACKWARD;
	animations["talk_neutral"] = ANIM_TALK_NEUTRAL;
	animations["talk_angry"] = ANIM_TALK_ANGRY;
	animations["talk_happy"] = ANIM_TALK_HAPPY;
	animations["talk_neutral_head"] = ANIM_TALK_NEUTRAL_HEAD;
	animations["talk_angry_head"] = ANIM_TALK_ANGRY_HEAD;
	animations["talk_happy_head"] = ANIM_TALK_HAPPY_HEAD;
	animations["bare_ready"] = ANIM_BARE_READY;
	animations["bare_unready"] = ANIM_BARE_UNREADY;
	animations["bare_wait"] = ANIM_BARE_WAIT;
	animations["bare_strike_left_start"] = ANIM_BARE_STRIKE_LEFT_START;
	animations["bare_strike_left_cycle"] = ANIM_BARE_STRIKE_LEFT_CYCLE;
	animations["bare_strike_left"] = ANIM_BARE_STRIKE_LEFT;
	animations["bare_strike_right_start"] = ANIM_BARE_STRIKE_RIGHT_START;
	animations["bare_strike_right_cycle"] = ANIM_BARE_STRIKE_RIGHT_CYCLE;
	animations["bare_strike_right"] = ANIM_BARE_STRIKE_RIGHT;
	animations["bare_strike_top_start"] = ANIM_BARE_STRIKE_TOP_START;
	animations["bare_strike_top_cycle"] = ANIM_BARE_STRIKE_TOP_CYCLE;
	animations["bare_strike_top"] = ANIM_BARE_STRIKE_TOP;
	animations["bare_strike_bottom_start"] = ANIM_BARE_STRIKE_BOTTOM_START;
	animations["bare_strike_bottom_cycle"] = ANIM_BARE_STRIKE_BOTTOM_CYCLE;
	animations["bare_strike_bottom"] = ANIM_BARE_STRIKE_BOTTOM;
	animations["1h_ready_part_1"] = ANIM_1H_READY_PART_1;
	animations["1h_ready_part_2"] = ANIM_1H_READY_PART_2;
	animations["1h_unready_part_1"] = ANIM_1H_UNREADY_PART_1;
	animations["1h_unready_part_2"] = ANIM_1H_UNREADY_PART_2;
	animations["1h_wait"] = ANIM_1H_WAIT;
	animations["1h_strike_left_start"] = ANIM_1H_STRIKE_LEFT_START;
	animations["1h_strike_left_cycle"] = ANIM_1H_STRIKE_LEFT_CYCLE;
	animations["1h_strike_left"] = ANIM_1H_STRIKE_LEFT;
	animations["1h_strike_right_start"] = ANIM_1H_STRIKE_RIGHT_START;
	animations["1h_strike_right_cycle"] = ANIM_1H_STRIKE_RIGHT_CYCLE;
	animations["1h_strike_right"] = ANIM_1H_STRIKE_RIGHT;
	animations["1h_strike_top_start"] = ANIM_1H_STRIKE_TOP_START;
	animations["1h_strike_top_cycle"] = ANIM_1H_STRIKE_TOP_CYCLE;
	animations["1h_strike_top"] = ANIM_1H_STRIKE_TOP;
	animations["1h_strike_bottom_start"] = ANIM_1H_STRIKE_BOTTOM_START;
	animations["1h_strike_bottom_cycle"] = ANIM_1H_STRIKE_BOTTOM_CYCLE;
	animations["1h_strike_bottom"] = ANIM_1H_STRIKE_BOTTOM;
	animations["2h_ready_part_1"] = ANIM_2H_READY_PART_1;
	animations["2h_ready_part_2"] = ANIM_2H_READY_PART_2;
	animations["2h_unready_part_1"] = ANIM_2H_UNREADY_PART_1;
	animations["2h_unready_part_2"] = ANIM_2H_UNREADY_PART_2;
	animations["2h_wait"] = ANIM_2H_WAIT;
	animations["2h_strike_left_start"] = ANIM_2H_STRIKE_LEFT_START;
	animations["2h_strike_left_cycle"] = ANIM_2H_STRIKE_LEFT_CYCLE;
	animations["2h_strike_left"] = ANIM_2H_STRIKE_LEFT;
	animations["2h_strike_right_start"] = ANIM_2H_STRIKE_RIGHT_START;
	animations["2h_strike_right_cycle"] = ANIM_2H_STRIKE_RIGHT_CYCLE;
	animations["2h_strike_right"] = ANIM_2H_STRIKE_RIGHT;
	animations["2h_strike_top_start"] = ANIM_2H_STRIKE_TOP_START;
	animations["2h_strike_top_cycle"] = ANIM_2H_STRIKE_TOP_CYCLE;
	animations["2h_strike_top"] = ANIM_2H_STRIKE_TOP;
	animations["2h_strike_bottom_start"] = ANIM_2H_STRIKE_BOTTOM_START;
	animations["2h_strike_bottom_cycle"] = ANIM_2H_STRIKE_BOTTOM_CYCLE;
	animations["2h_strike_bottom"] = ANIM_2H_STRIKE_BOTTOM;
	animations["missile_ready_part_1"] = ANIM_MISSILE_READY_PART_1;
	animations["missile_ready_part_2"] = ANIM_MISSILE_READY_PART_2;
	animations["missile_unready_part_1"] = ANIM_MISSILE_UNREADY_PART_1;
	animations["missile_unready_part_2"] = ANIM_MISSILE_UNREADY_PART_2;
	animations["missile_wait"] = ANIM_MISSILE_WAIT;
	animations["missile_strike_part_1"] = ANIM_MISSILE_STRIKE_PART_1;
	animations["missile_strike_part_2"] = ANIM_MISSILE_STRIKE_PART_2;
	animations["missile_strike_cycle"] = ANIM_MISSILE_STRIKE_CYCLE;
	animations["missile_strike"] = ANIM_MISSILE_STRIKE;
	animations["meditation"] = ANIM_MEDITATION;
	animations["cast_start"] = ANIM_CAST_START;
	animations["cast_cycle"] = ANIM_CAST_CYCLE;
	animations["cast"] = ANIM_CAST;
	animations["cast_end"] = ANIM_CAST_END;
	animations["crouch"] = ANIM_CROUCH;
	animations["crouch_walk"] = ANIM_CROUCH_WALK;
	animations["crouch_walk_backward"] = ANIM_CROUCH_WALK_BACKWARD;
	animations["crouch_strafe_left"] = ANIM_CROUCH_STRAFE_LEFT;
	animations["crouch_strafe_right"] = ANIM_CROUCH_STRAFE_RIGHT;
	animations["crouch_start"] = ANIM_CROUCH_START;
	animations["crouch_wait"] = ANIM_CROUCH_WAIT;
	animations["crouch_end"] = ANIM_CROUCH_END;
	animations["lean_right"] = ANIM_LEAN_RIGHT;
	animations["lean_left"] = ANIM_LEAN_LEFT;
	animations["levitate"] = ANIM_LEVITATE;
	animations["jump"] = ANIM_JUMP;
	animations["jump_anticipation"] = ANIM_JUMP_ANTICIPATION;
	animations["jump_up"] = ANIM_JUMP_UP;
	animations["jump_cycle"] = ANIM_JUMP_CYCLE;
	animations["jump_end"] = ANIM_JUMP_END;
	animations["jump_end_part2"] = ANIM_JUMP_END_PART2;
	animations["fight_walk_forward"] = ANIM_FIGHT_WALK_FORWARD;
	animations["fight_walk_backward"] = ANIM_FIGHT_WALK_BACKWARD;
	animations["fight_walk_ministep"] = ANIM_FIGHT_WALK_MINISTEP;
	animations["fight_strafe_right"] = ANIM_FIGHT_STRAFE_RIGHT;
	animations["fight_strafe_left"] = ANIM_FIGHT_STRAFE_LEFT;
	animations["fight_wait"] = ANIM_FIGHT_WAIT;
	animations["grunt"] = ANIM_GRUNT;
	animations["u_turn_left"] = ANIM_U_TURN_LEFT;
	animations["u_turn_right"] = ANIM_U_TURN_RIGHT;
	animations["u_turn_left_fight"] = ANIM_U_TURN_LEFT_FIGHT;
	animations["u_turn_right_fight"] = ANIM_U_TURN_RIGHT_FIGHT;
	
}

}

void setupScriptedAnimation() {
	
	initAnimationNumbers();
	
	ScriptEvent::registerCommand(new RotateCommand);
	ScriptEvent::registerCommand(new ForceAnimCommand);
	ScriptEvent::registerCommand(new ForceAngleCommand);
	ScriptEvent::registerCommand(new PlayAnimCommand);
	ScriptEvent::registerCommand(new LoadAnimCommand);
	ScriptEvent::registerCommand(new MoveCommand);
	ScriptEvent::registerCommand(new SetControlledZoneCommand);
	ScriptEvent::registerCommand(new SetPathCommand);
	ScriptEvent::registerCommand(new UsePathCommand);
	ScriptEvent::registerCommand(new UnsetControlledZoneCommand);
	
}

} // namespace script
