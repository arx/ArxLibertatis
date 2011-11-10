/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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

#include "script/ScriptedIOControl.h"

#include "core/Core.h"
#include "game/Inventory.h"
#include "game/Player.h"
#include "game/Missile.h"
#include "game/NPC.h"
#include "graphics/Math.h"
#include "graphics/data/Mesh.h"
#include "gui/Interface.h"
#include "io/FilePath.h"
#include "physics/Collisions.h"
#include "scene/Interactive.h"
#include "script/ScriptUtils.h"

using std::string;

extern INTERACTIVE_OBJ * LASTSPAWNED;
extern long CHANGE_LEVEL_ICON;

namespace script {

namespace {

class ReplaceMeCommand : public Command {
	
public:
	
	ReplaceMeCommand() : Command("replaceme", ANY_IO) { }
	
	Result execute(Context & context) {
		
		fs::path object = fs::path::load(context.getWord());
		
		DebugScript(' ' << object);
		
		INTERACTIVE_OBJ * io = context.getIO();
		
		fs::path file;
		if(io->ioflags & IO_NPC) {
			file = ("graph/obj3d/interactive/npc" / object).append(".teo");
		} else if(io->ioflags & IO_FIX) {
			file = ("graph/obj3d/interactive/fix_inter" / object).append(".teo");
		} else {
			file = ("graph/obj3d/interactive/items" / object).append(".teo");
		}
		
		Anglef last_angle = io->angle;
		INTERACTIVE_OBJ * ioo = AddInteractive(file, -1);
		if(!ioo) {
			return Failed;
		}
		
		LASTSPAWNED = ioo;
		ioo->scriptload = 1;
		ioo->initpos = io->initpos;
		ioo->pos = io->pos;
		ioo->angle = io->angle;
		ioo->move = io->move;
		ioo->show = io->show;
		
		if(io == DRAGINTER) {
			Set_DragInter(ioo);
		}
		
		long neww = GetInterNum(ioo);
		long oldd = GetInterNum(io);
		
		if((io->ioflags & IO_ITEM) && io->_itemdata->count > 1) {
			io->_itemdata->count--;
			SendInitScriptEvent(ioo);
			CheckForInventoryReplaceMe(ioo, io);
		} else {
			
			for(size_t i = 0; i < MAX_SPELLS; i++) {
				if(spells[i].exist && spells[i].caster == oldd) {
					spells[i].caster = neww;
				}
			}
			
			io->show = SHOW_FLAG_KILLED;
			ReplaceInAllInventories(io, ioo);
			SendInitScriptEvent(ioo);
			ioo->angle = last_angle;
			TREATZONE_AddIO(ioo, neww);
			
			for(int i = 0; i < MAX_EQUIPED; i++) {
				if(player.equiped[i] != 0 && ValidIONum(player.equiped[i])) {
					if(inter.iobj[player.equiped[i]] == io) {
						ARX_EQUIPMENT_UnEquip(inter.iobj[0], io, 1);
						ARX_EQUIPMENT_Equip(inter.iobj[0], ioo);
					}
				}
			}
			
			if(io->scriptload) {
				ReleaseInter(io);
				return AbortRefuse;
			} else {
				TREATZONE_RemoveIO(io);
			}
			
			return AbortRefuse;
		}
		
		return Success;
	}
	
};

class CollisionCommand : public Command {
	
public:
	
	CollisionCommand() : Command("collision", ANY_IO) { }
	
	Result execute(Context & context) {
		
		bool choice = context.getBool();
		
		DebugScript(' ' << choice);
		
		INTERACTIVE_OBJ * io = context.getIO();
		
		if(!choice) {
			io->ioflags |= IO_NO_COLLISIONS;
			return Success;
		}
		
		if(io->ioflags & IO_NO_COLLISIONS) {
			
			bool colliding = false;
			for(long k = 0; k < inter.nbmax; k++) {
				INTERACTIVE_OBJ * ioo = inter.iobj[k];
				if(ioo && IsCollidingIO(io, ioo)) {
					INTERACTIVE_OBJ * oes = EVENT_SENDER;
					EVENT_SENDER = ioo;
					Stack_SendIOScriptEvent(io, SM_COLLISION_ERROR_DETAIL);
					EVENT_SENDER = oes;
					colliding = true;
				}
			}
			
			if(colliding) {
				INTERACTIVE_OBJ * oes = EVENT_SENDER;
				EVENT_SENDER = NULL;
				Stack_SendIOScriptEvent(io, SM_COLLISION_ERROR);
				EVENT_SENDER = oes;
			}
		}
		
		io->ioflags &= ~IO_NO_COLLISIONS;
		
		return Success;
	}
	
};

class SpawnCommand : public Command {
	
public:
	
	SpawnCommand() : Command("spawn") { }
	
	Result execute(Context & context) {
		
		string type = context.getWord();
		
		if(type == "npc" || type == "item") {
			
			fs::path file = fs::path::load(context.getWord()); // object to spawn.
			
			string target = context.getWord(); // object ident for position
			INTERACTIVE_OBJ * t = inter.getById(target, context.getIO());
			if(!t) {
				ScriptWarning << "unknown target: npc " << file << ' ' << target;
				return Failed;
			}
			
			DebugScript(" npc " << file << ' ' << target);
			
			if(FORBID_SCRIPT_IO_CREATION) {
				return Failed;
			}
			
			if(type == "npc") {
				
				fs::path path = "graph/obj3d/interactive/npc" / file;
				
				INTERACTIVE_OBJ * ioo = AddNPC(path, IO_IMMEDIATELOAD);
				if(!ioo) {
					ScriptWarning << "failed to create npc " << path;
					return Failed;
				}
				
				LASTSPAWNED = ioo;
				ioo->scriptload = 1;
				ioo->pos = t->pos;
				
				ioo->angle = t->angle;
				MakeTemporaryIOIdent(ioo);
				SendInitScriptEvent(ioo);
				
				if(t->ioflags & IO_NPC) {
					float dist = t->physics.cyl.radius + ioo->physics.cyl.radius + 10;
					ioo->pos.x += -EEsin(radians(t->angle.b)) * dist;
					ioo->pos.z += EEcos(radians(t->angle.b)) * dist;
				}
				
				TREATZONE_AddIO(ioo, GetInterNum(ioo));
				
			} else {
				
				fs::path path = "graph/obj3d/interactive/items" / file;
				
				INTERACTIVE_OBJ * ioo = AddItem(path, IO_IMMEDIATELOAD);
				if(!ioo) {
					ScriptWarning << "failed to create item " << path;
					return Failed;
				}
				
				LASTSPAWNED = ioo;
				ioo->scriptload = 1;
				ioo->pos = t->pos;
				ioo->angle = t->angle;
				MakeTemporaryIOIdent(ioo);
				SendInitScriptEvent(ioo);
				
				TREATZONE_AddIO(ioo, GetInterNum(ioo));
				
			}
			
		} else if(type == "fireball") {
			
			INTERACTIVE_OBJ * io = context.getIO();
			if(!io) {
				ScriptWarning << "must be npc to spawn fireballs";
				return  Failed;
			}
			
			GetTargetPos(io);
			Vec3f pos = io->pos;
			
			if(io->ioflags & IO_NPC) {
				pos.y -= 80.f;
			}
			
			ARX_MISSILES_Spawn(io, MISSILE_FIREBALL, &pos, &io->target);
			
		} else {
			ScriptWarning << "unexpected type: " << type;
			return Failed;
		}
		
		return Success;
	}
	
};

class KillMeCommand : public Command {
	
public:
	
	KillMeCommand() : Command("killme", ANY_IO) { }
	
	Result execute(Context & context) {
		
		DebugScript("");
		
		INTERACTIVE_OBJ * io = context.getIO();
		if((io->ioflags & IO_ITEM) && io->_itemdata->count > 1) {
			io->_itemdata->count--;
		} else {
			io->show = SHOW_FLAG_KILLED;
			io->GameFlags &= ~GFLAG_ISINTREATZONE;
			RemoveFromAllInventories(io);
			ARX_DAMAGES_ForceDeath(io, EVENT_SENDER);
		}
		
		return Success;
	}
	
};

class PhysicalCommand : public Command {
	
public:
	
	PhysicalCommand() : Command("physical", ANY_IO) { }
	
	Result execute(Context & context) {
		
		string type = context.getWord();
		
		INTERACTIVE_OBJ * io = context.getIO();
		
		if(type == "on") {
			io->ioflags &= ~IO_PHYSICAL_OFF;
			DebugScript(" on");
			
		} else if(type == "off") {
			io->ioflags |= IO_PHYSICAL_OFF;
			DebugScript(" off");
			
		} else {
			
			float fval = context.getFloat();
			
			DebugScript(' ' << type << ' ' << fval);
			
			if(type == "height") {
				io->original_height = clamp(-fval, -165.f, -30.f);
				io->physics.cyl.height = io->original_height * io->scale;
			} else if(type == "radius") {
				io->original_radius = clamp(fval, 10.f, 40.f);
				io->physics.cyl.radius = io->original_radius * io->scale;
			} else {
				ScriptWarning << "unknown command: " << type;
				return Failed;
			}
			
		}
		
		return Success;
	}
	
};

class LinkObjToMeCommand : public Command {
	
public:
	
	LinkObjToMeCommand() : Command("linkobjtome", ANY_IO) { }
	
	Result execute(Context & context) {
		
		string name = context.getStringVar(context.getWord());
		
		string attach = context.getWord();
		
		DebugScript(' ' << name << ' ' << attach);
		
		long t = inter.getById(name);
		if(!ValidIONum(t)) {
			ScriptWarning << "unknown target: " << name;
			return Failed;
		}
		
		LinkObjToMe(context.getIO(), inter.iobj[t], attach);
		
		return Success;
	}
	
};

class IfExistInternalCommand : public Command {
	
public:
	
	IfExistInternalCommand() : Command("ifexistinternal") { }
	
	Result execute(Context & context) {
		
		string target = context.getWord();
		
		DebugScript(' ' << target);
		
		long t = inter.getById(target);
		
		if(t == -1) {
			context.skipStatement();
		}
		
		return Success;
	}
	
};

class IfVisibleCommand : public Command {
	
	static bool hasVisibility(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * ioo) {
		
		if(distSqr(io->pos, ioo->pos) > square(20000)) {
			return false;
		}
		
		float ab = MAKEANGLE(io->angle.b);
		float aa = getAngle(io->pos.x, io->pos.z, ioo->pos.x, ioo->pos.z);
		aa = MAKEANGLE(degrees(aa));
		
		if((aa < ab + 90.f) && (aa > ab - 90.f)) {
			//font
			return true;
		}
		
		return false;
	}
	
public:
	
	IfVisibleCommand() : Command("ifvisible", ANY_IO) { }
	
	Result execute(Context & context) {
		
		string target = context.getWord();
		
		DebugScript(' ' << target);
		
		long t = inter.getById(target);
		
		if(!ValidIONum(t) || !hasVisibility(context.getIO(), inter.iobj[t])) {
			context.skipStatement();
		}
		
		return Success;
	}
	
};

class ObjectHideCommand : public Command {
	
public:
	
	ObjectHideCommand() : Command("objecthide") { }
	
	Result execute(Context & context) {
		
		bool megahide = false;
		HandleFlags("m") {
			megahide = test_flag(flg, 'm');
		}
		
		string target = context.getWord();
		INTERACTIVE_OBJ * t = inter.getById(target, context.getIO());
		
		bool hide = context.getBool();
		
		DebugScript(' ' << options << ' ' << target << ' ' << hide);
		
		if(!t) {
			return Failed;
		}
		
		t->GameFlags &= ~GFLAG_MEGAHIDE;
		if(hide) {
			if(megahide) {
				t->GameFlags |= GFLAG_MEGAHIDE;
				t->show = SHOW_FLAG_MEGAHIDE;
			} else {
				t->show = SHOW_FLAG_HIDDEN;
			}
		} else if(t->show == SHOW_FLAG_MEGAHIDE || t->show == SHOW_FLAG_HIDDEN) {
			t->show = SHOW_FLAG_IN_SCENE;
			if((t->ioflags & IO_NPC) && t->_npcdata->life <= 0.f) {
				t->animlayer[0].cur_anim = t->anims[ANIM_DIE];
				t->animlayer[1].cur_anim = NULL;
				t->animlayer[2].cur_anim = NULL;
				t->animlayer[0].ctime = 9999999;
			}
		}
		
		return Success;
	}
	
};

class TeleportCommand : public Command {
	
public:
	
	TeleportCommand() : Command("teleport") { }
	
	Result execute(Context & context) {
		
		bool confirm = true;
		
		bool teleport_player = false, initpos = false;
		HandleFlags("alnpi") {
			
			long angle = -1;
			
			if(flg & flag('a')) {
				float fangle = context.getFloat();
				angle = static_cast<long>(fangle);
				if(!(flg & flag('l'))) {
					player.desiredangle.b = player.angle.b = fangle;
				}
			}
			
			if(flg & flag('n')) {
				confirm = false;
			}
			
			if(flg & flag('l')) {
				
				string level = context.getWord();
				string target = context.getWord();
				
				strcpy(TELEPORT_TO_LEVEL, level.c_str());
				strcpy(TELEPORT_TO_POSITION, target.c_str());
				
				if(angle == -1) {
					TELEPORT_TO_ANGLE	=	static_cast<long>(player.angle.b);
				} else {
					TELEPORT_TO_ANGLE = angle;
				}
				
				CHANGE_LEVEL_ICON =  confirm ? 1 : 200;
				
				DebugScript(' ' << options << ' ' << angle << ' ' << level << ' ' << target);
				
				return Success;
			}
			
			teleport_player = test_flag(flg, 'p');
			initpos = test_flag(flg, 'i');
		}
		
		string target;
		if(!initpos) {
			target = context.getWord();
		}
		
		DebugScript(' ' << options << ' ' << player.angle.b << ' ' << target);
		
		if(target == "behind") {
			ARX_INTERACTIVE_TeleportBehindTarget(context.getIO());
			return Success;
		}
		
		INTERACTIVE_OBJ * io = context.getIO();
		if(!teleport_player && !io) {
			ScriptWarning << "must either use -p or use in IO context";
			return Failed;
		}
		
		if(!initpos) {
			
			INTERACTIVE_OBJ * t = inter.getById(target, context.getIO());
			if(!t) {
				ScriptWarning << "unknown target: " << target;
				return Failed;
			}
			
			Vec3f pos;
			if(!GetItemWorldPosition(t, &pos)) {
				ScriptWarning << "could not get world position";
				return Failed;
			}
			
			if(teleport_player) {
				ARX_INTERACTIVE_Teleport(inter.iobj[0], &pos);
				return Success;
			}
			
			if(!(io->ioflags & IO_NPC) || io->_npcdata->life > 0) {
				if(io->show != SHOW_FLAG_HIDDEN && io->show != SHOW_FLAG_MEGAHIDE) {
					io->show = SHOW_FLAG_IN_SCENE;
				}
				ARX_INTERACTIVE_Teleport(io, &pos);
			}
			
		} else {
			
			if(!io) {
				ScriptWarning << "must be in IO context to teleport -i";
				return Failed;
			}
			
			if(teleport_player) {
				Vec3f pos;
				if(GetItemWorldPosition(io, &pos)) {
					ARX_INTERACTIVE_Teleport(inter.iobj[0], &pos);
				}
			} else if(!(io->ioflags & IO_NPC) || io->_npcdata->life > 0) {
				if(io->show != SHOW_FLAG_HIDDEN && io->show != SHOW_FLAG_MEGAHIDE) {
					io->show = SHOW_FLAG_IN_SCENE;
				}
				ARX_INTERACTIVE_Teleport(io, &io->initpos);
			}
		}
		
		return Success;
	}
	
};

class TargetPlayerPosCommand : public Command {
	
public:
	
	TargetPlayerPosCommand() : Command("targetplayerpos", ANY_IO) { }
	
	Result execute(Context & context) {
		
		DebugScript("");
		
		context.getIO()->targetinfo = TARGET_PLAYER;
		GetTargetPos(context.getIO());
		
		return Success;
	}
	
};

class DestroyCommand : public Command {
	
public:
	
	DestroyCommand() : Command("destroy") { }
	
	Result execute(Context & context) {
		
		string target = context.getStringVar(context.getWord());
		
		DebugScript(' ' << target);
		
		INTERACTIVE_OBJ * t = inter.getById(target, context.getIO());
		if(!t) {
			return Success;
		}
		
		bool self = (t == context.getIO());
		
		ARX_INTERACTIVE_DestroyIO(t);
		
		return self ? AbortAccept : Success; // Cannot process further if we destroyed the script's IO
	}
	
};

class AbstractDamageCommand : public Command {
	
protected:
	
	AbstractDamageCommand(const string & name, long ioflags = 0) : Command(name, ioflags) { }
	
	DamageType getDamageType(Context & context) {
		
		DamageType type = 0;
		HandleFlags("fmplcgewsaornu") {
			type |= (flg & flag('f')) ? DAMAGE_TYPE_FIRE : DamageType(0);
			type |= (flg & flag('m')) ? DAMAGE_TYPE_MAGICAL : DamageType(0);
			type |= (flg & flag('p')) ? DAMAGE_TYPE_POISON : DamageType(0);
			type |= (flg & flag('l')) ? DAMAGE_TYPE_LIGHTNING : DamageType(0);
			type |= (flg & flag('c')) ? DAMAGE_TYPE_COLD : DamageType(0);
			type |= (flg & flag('g')) ? DAMAGE_TYPE_GAS : DamageType(0);
			type |= (flg & flag('e')) ? DAMAGE_TYPE_METAL : DamageType(0);
			type |= (flg & flag('w')) ? DAMAGE_TYPE_WOOD : DamageType(0);
			type |= (flg & flag('s')) ? DAMAGE_TYPE_STONE : DamageType(0);
			type |= (flg & flag('a')) ? DAMAGE_TYPE_ACID : DamageType(0);
			type |= (flg & flag('o')) ? DAMAGE_TYPE_ORGANIC : DamageType(0);
			type |= (flg & flag('r')) ? DAMAGE_TYPE_DRAIN_LIFE : DamageType(0);
			type |= (flg & flag('n')) ? DAMAGE_TYPE_DRAIN_MANA : DamageType(0);
			type |= (flg & flag('u')) ? DAMAGE_TYPE_PUSH : DamageType(0);
		}
		
		return type;
	}
	
};

class DoDamageCommand : public AbstractDamageCommand {
	
public:
	
	DoDamageCommand() : AbstractDamageCommand("dodamage") { }
	
	Result execute(Context & context) {
		
		DamageType type = getDamageType(context);
		
		string target = context.getWord();
		
		float damage = context.getFloat();
		
		DebugScript(' ' << type << ' ' << target);
		
		INTERACTIVE_OBJ * t = inter.getById(target, context.getIO());
		if(!t) {
			ScriptWarning << "unknown target: " << target;
			return Failed;
		}
		
		ARX_DAMAGES_DealDamages(GetInterNum(t), damage, GetInterNum(context.getIO()), type, &t->pos);
		
		return Success;
	}
	
};

class DamagerCommand : public AbstractDamageCommand {
	
public:
	
	DamagerCommand() : AbstractDamageCommand("damager", ANY_IO) { }
	
	Result execute(Context & context) {
		
		INTERACTIVE_OBJ * io = context.getIO();
		
		io->damager_type = getDamageType(context) | DAMAGE_TYPE_PER_SECOND;
		
		float damages = context.getFloat();
		
		DebugScript(' ' << io->damager_type << damages);
		
		io->damager_damages = checked_range_cast<short>(damages);
		
		return Success;
	}
	
};

}

void setupScriptedIOControl() {
	
	ScriptEvent::registerCommand(new ReplaceMeCommand);
	ScriptEvent::registerCommand(new CollisionCommand);
	ScriptEvent::registerCommand(new SpawnCommand);
	ScriptEvent::registerCommand(new KillMeCommand);
	ScriptEvent::registerCommand(new PhysicalCommand);
	ScriptEvent::registerCommand(new LinkObjToMeCommand);
	ScriptEvent::registerCommand(new IfExistInternalCommand);
	ScriptEvent::registerCommand(new IfVisibleCommand);
	ScriptEvent::registerCommand(new ObjectHideCommand);
	ScriptEvent::registerCommand(new TeleportCommand);
	ScriptEvent::registerCommand(new TargetPlayerPosCommand);
	ScriptEvent::registerCommand(new DestroyCommand);
	ScriptEvent::registerCommand(new DoDamageCommand);
	ScriptEvent::registerCommand(new DamagerCommand);
	
}

} // namespace script
