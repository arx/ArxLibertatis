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

#include "script/ScriptedIOControl.h"

#include "core/Core.h"
#include "game/EntityManager.h"
#include "game/Equipment.h"
#include "game/Inventory.h"
#include "game/Item.h"
#include "game/Missile.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "graphics/Math.h"
#include "graphics/data/Mesh.h"
#include "gui/Interface.h"
#include "io/resource/ResourcePath.h"
#include "physics/Collisions.h"
#include "scene/Interactive.h"
#include "script/ScriptUtils.h"


extern Entity * LASTSPAWNED;

namespace script {

namespace {

class ReplaceMeCommand : public Command {
	
public:
	
	ReplaceMeCommand() : Command("replaceme", AnyEntity) { }
	
	Result execute(Context & context) {
		
		res::path object = res::path::load(context.getWord());
		
		DebugScript(' ' << object);
		
		Entity * io = context.getEntity();
		
		res::path file;
		if(io->ioflags & IO_NPC) {
			file = "graph/obj3d/interactive/npc" / object;
		} else if(io->ioflags & IO_FIX) {
			file = "graph/obj3d/interactive/fix_inter" / object;
		} else {
			file = "graph/obj3d/interactive/items" / object;
		}
		
		Anglef last_angle = io->angle;
		Entity * ioo = AddInteractive(file);
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
		
		EntityHandle neww = ioo->index();
		EntityHandle oldd = io->index();
		
		if((io->ioflags & IO_ITEM) && io->_itemdata->count > 1) {
			io->_itemdata->count--;
			SendInitScriptEvent(ioo);
			
			if(playerInventory.locate(io)) {
				giveToPlayer(ioo);
			} else {
				CheckForInventoryReplaceMe(ioo, io);
			}
		} else {
			spells.replaceCaster(oldd, neww);
			
			InventoryPos oldPos = removeFromInventories(io);
			
			SendInitScriptEvent(ioo);
			ioo->angle = last_angle;
			TREATZONE_AddIO(ioo);
			
			// check that the init script didn't put the item anywhere
			// if we ignore this we might create duplucate references
			bool reInsert = true; // should the new item be inserted at the old items position?
			if(locateInInventories(ioo)) {
				// the init script already inserted the item into an inventory
				reInsert = false;
			}
			for(size_t i = 0; i < MAX_EQUIPED; i++) {
				if(ValidIONum(player.equiped[i])) {
					if(entities[player.equiped[i]] == ioo) {
						// the init script was sneaky and equiped the item
						reInsert = false;
					}
				}
			}
			
			if(reInsert) {
				if(oldPos) {
					insertIntoInventory(ioo, oldPos);
				} else {
					for(size_t i = 0; i < MAX_EQUIPED; i++) {
						if(ValidIONum(player.equiped[i])) {
							if(entities[player.equiped[i]] == io) {
								ARX_EQUIPMENT_UnEquip(entities.player(), io, 1);
								ARX_EQUIPMENT_Equip(entities.player(), ioo);
							}
						}
					}
				}
			}
			
			// Delay destruction of the object to avoid invalid references
			ARX_INTERACTIVE_DestroyIOdelayed(io);
			
			// Prevent further script events as the object has been destroyed!
			io->show = SHOW_FLAG_MEGAHIDE;
			io->ioflags |= IO_FREEZESCRIPT;
			return AbortRefuse;
		}
		
		return Success;
	}
	
};

class CollisionCommand : public Command {
	
public:
	
	CollisionCommand() : Command("collision", AnyEntity) { }
	
	Result execute(Context & context) {
		
		bool choice = context.getBool();
		
		DebugScript(' ' << choice);
		
		Entity * io = context.getEntity();
		
		if(!choice) {
			io->ioflags |= IO_NO_COLLISIONS;
			return Success;
		}
		
		if(io->ioflags & IO_NO_COLLISIONS) {
			
			bool colliding = false;
			for(size_t k = 0; k < entities.size(); k++) {
				const EntityHandle handle = EntityHandle(k);
				Entity * ioo = entities[handle];
				if(ioo && IsCollidingIO(io, ioo)) {
					Stack_SendIOScriptEvent(ioo, io, SM_COLLISION_ERROR_DETAIL);
					colliding = true;
				}
			}
			
			if(colliding) {
				Stack_SendIOScriptEvent(NULL, io, SM_COLLISION_ERROR);
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
		
		std::string type = context.getWord();
		
		if(type == "npc" || type == "item") {
			
			res::path file = res::path::load(context.getWord()); // object to spawn.
			file.remove_ext();
			
			std::string target = context.getWord(); // object ident for position
			Entity * t = entities.getById(target, context.getEntity());
			if(!t) {
				ScriptWarning << "unknown target: npc " << file << ' ' << target;
				return Failed;
			}
			
			DebugScript(" npc " << file << ' ' << target);
			
			if(FORBID_SCRIPT_IO_CREATION) {
				return Failed;
			}
			
			if(type == "npc") {
				
				res::path path = "graph/obj3d/interactive/npc" / file;
				
				Entity * ioo = AddNPC(path, -1, IO_IMMEDIATELOAD);
				if(!ioo) {
					ScriptWarning << "failed to create npc " << path;
					return Failed;
				}
				
				LASTSPAWNED = ioo;
				ioo->scriptload = 1;
				ioo->pos = t->pos;
				
				ioo->angle = t->angle;
				SendInitScriptEvent(ioo);
				
				if(t->ioflags & IO_NPC) {
					float dist = t->physics.cyl.radius + ioo->physics.cyl.radius + 10;
					
					ioo->pos += angleToVectorXZ(t->angle.getYaw()) * dist;
				}
				
				TREATZONE_AddIO(ioo);
				
			} else {
				
				res::path path = "graph/obj3d/interactive/items" / file;
				
				Entity * ioo = AddItem(path);
				if(!ioo) {
					ScriptWarning << "failed to create item " << path;
					return Failed;
				}
				
				LASTSPAWNED = ioo;
				ioo->scriptload = 1;
				ioo->pos = t->pos;
				ioo->angle = t->angle;
				SendInitScriptEvent(ioo);
				
				TREATZONE_AddIO(ioo);
				
			}
			
		} else if(type == "fireball") {
			
			Entity * io = context.getEntity();
			if(!io) {
				ScriptWarning << "must be npc to spawn fireballs";
				return  Failed;
			}
			
			GetTargetPos(io);
			Vec3f pos = io->pos;
			
			if(io->ioflags & IO_NPC) {
				pos.y -= 80.f;
			}
			
			ARX_MISSILES_Spawn(io, MISSILE_FIREBALL, pos, io->target);
			
		} else {
			ScriptWarning << "unexpected type: " << type;
			return Failed;
		}
		
		return Success;
	}
	
};

class PhysicalCommand : public Command {
	
public:
	
	PhysicalCommand() : Command("physical", AnyEntity) { }
	
	Result execute(Context & context) {
		
		std::string type = context.getWord();
		
		Entity * io = context.getEntity();
		
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
				io->original_height = glm::clamp(-fval, -165.f, -30.f);
				io->physics.cyl.height = io->original_height * io->scale;
			} else if(type == "radius") {
				io->original_radius = glm::clamp(fval, 10.f, 40.f);
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
	
	LinkObjToMeCommand() : Command("linkobjtome", AnyEntity) { }
	
	Result execute(Context & context) {
		
		std::string name = context.getStringVar(context.getWord());
		
		std::string attach = context.getWord();
		
		DebugScript(' ' << name << ' ' << attach);
		
		EntityHandle t = entities.getById(name);
		if(!ValidIONum(t)) {
			ScriptWarning << "unknown target: " << name;
			return Failed;
		}
		
		LinkObjToMe(context.getEntity(), entities[t], attach);
		
		return Success;
	}
	
};

class IfExistInternalCommand : public Command {
	
public:
	
	IfExistInternalCommand() : Command("ifexistinternal") { }
	
	Result execute(Context & context) {
		
		std::string target = context.getWord();
		
		DebugScript(' ' << target);
		
		EntityHandle t = entities.getById(target);
		
		if(t == EntityHandle()) {
			context.skipStatement();
		}
		
		return Success;
	}
	
};

class IfVisibleCommand : public Command {
	
	static bool hasVisibility(Entity * io, Entity * ioo) {
		
		if(fartherThan(io->pos, ioo->pos, 20000)) {
			return false;
		}
		
		float ab = MAKEANGLE(io->angle.getYaw());
		float aa = getAngle(io->pos.x, io->pos.z, ioo->pos.x, ioo->pos.z);
		aa = MAKEANGLE(glm::degrees(aa));
		
		return (aa < ab + 90.f && aa > ab - 90.f);
	}
	
public:
	
	IfVisibleCommand() : Command("ifvisible", AnyEntity) { }
	
	Result execute(Context & context) {
		
		std::string target = context.getWord();
		
		DebugScript(' ' << target);
		
		EntityHandle t = entities.getById(target);
		
		if(!ValidIONum(t) || !hasVisibility(context.getEntity(), entities[t])) {
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
		
		std::string target = context.getWord();
		Entity * t = entities.getById(target, context.getEntity());
		
		bool hide = context.getBool();
		
		DebugScript(' ' << options << ' ' << target << ' ' << hide);
		
		if(!t) {
			return Failed;
		}
		
		t->gameFlags &= ~GFLAG_MEGAHIDE;
		if(hide) {
			if(megahide) {
				t->gameFlags |= GFLAG_MEGAHIDE;
				t->show = SHOW_FLAG_MEGAHIDE;
			} else {
				t->show = SHOW_FLAG_HIDDEN;
			}
		} else if(t->show == SHOW_FLAG_MEGAHIDE || t->show == SHOW_FLAG_HIDDEN) {
			t->show = SHOW_FLAG_IN_SCENE;
			if((t->ioflags & IO_NPC) && t->_npcdata->lifePool.current <= 0.f) {
				t->animlayer[0].cur_anim = t->anims[ANIM_DIE];
				t->animlayer[1].cur_anim = NULL;
				t->animlayer[2].cur_anim = NULL;
				t->animlayer[0].ctime = AnimationDurationMs(9999999);
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
					player.desiredangle.setYaw(fangle);
					player.angle.setYaw(fangle);
				}
			}
			
			if(flg & flag('n')) {
				confirm = false;
			}
			
			if(flg & flag('l')) {
				
				std::string level = context.getWord();
				std::string target = context.getWord();
				
				TELEPORT_TO_LEVEL = level;
				TELEPORT_TO_POSITION = target;
				
				if(angle == -1) {
					TELEPORT_TO_ANGLE = static_cast<long>(player.angle.getYaw());
				} else {
					TELEPORT_TO_ANGLE = angle;
				}
				
				CHANGE_LEVEL_ICON = confirm ? ConfirmChangeLevel : ChangeLevelNow;
				
				DebugScript(' ' << options << ' ' << angle << ' ' << level << ' ' << target);
				
				return Success;
			}
			
			teleport_player = test_flag(flg, 'p');
			initpos = test_flag(flg, 'i');
		}
		
		std::string target;
		if(!initpos) {
			target = context.getWord();
		}
		
		DebugScript(' ' << options << ' ' << player.angle.getYaw() << ' ' << target);
		
		if(target == "behind") {
			ARX_INTERACTIVE_TeleportBehindTarget(context.getEntity());
			return Success;
		}
		
		Entity * io = context.getEntity();
		if(!teleport_player && !io) {
			ScriptWarning << "must either use -p or use in IO context";
			return Failed;
		}
		
		if(!initpos) {
			
			Entity * t = entities.getById(target, context.getEntity());
			if(!t) {
				ScriptWarning << "unknown target: " << target;
				return Failed;
			}
			
			Vec3f pos = GetItemWorldPosition(t);
			
			if(teleport_player) {
				ARX_INTERACTIVE_Teleport(entities.player(), pos);
				return Success;
			}
			
			if(!(io->ioflags & IO_NPC) || io->_npcdata->lifePool.current > 0) {
				if(io->show != SHOW_FLAG_HIDDEN && io->show != SHOW_FLAG_MEGAHIDE) {
					io->show = SHOW_FLAG_IN_SCENE;
				}
				ARX_INTERACTIVE_Teleport(io, pos);
			}
			
		} else {
			
			if(!io) {
				ScriptWarning << "must be in IO context to teleport -i";
				return Failed;
			}
			
			if(teleport_player) {
				Vec3f pos = GetItemWorldPosition(io);
				ARX_INTERACTIVE_Teleport(entities.player(), pos);
			} else if(!(io->ioflags & IO_NPC) || io->_npcdata->lifePool.current > 0) {
				if(io->show != SHOW_FLAG_HIDDEN && io->show != SHOW_FLAG_MEGAHIDE) {
					io->show = SHOW_FLAG_IN_SCENE;
				}
				ARX_INTERACTIVE_Teleport(io, io->initpos);
			}
		}
		
		return Success;
	}
	
};

class TargetPlayerPosCommand : public Command {
	
public:
	
	TargetPlayerPosCommand() : Command("targetplayerpos", AnyEntity) { }
	
	Result execute(Context & context) {
		
		DebugScript("");
		
		context.getEntity()->targetinfo = EntityHandle(TARGET_PLAYER);
		GetTargetPos(context.getEntity());
		
		return Success;
	}
	
};

class DestroyCommand : public Command {
	
public:
	
	DestroyCommand() : Command("destroy") { }
	
	Result execute(Context & context) {
		
		std::string target = context.getStringVar(context.getWord());
		
		DebugScript(' ' << target);
		
		Entity * entity = entities.getById(target, context.getEntity());
		if(!entity) {
			return Success;
		}
		
		// Delay destruction of the object to avoid invalid references
		bool destroyed = ARX_INTERACTIVE_DestroyIOdelayed(entity);
		
		// Prevent further script events as the object has been destroyed!
		if(destroyed) {
			entity->show = SHOW_FLAG_MEGAHIDE;
			entity->ioflags |= IO_FREEZESCRIPT;
			if(entity == context.getEntity()) {
				return AbortAccept;
			}
		}
		
		return Success;
	}
	
};

class AbstractDamageCommand : public Command {
	
protected:
	
	AbstractDamageCommand(const std::string & name, long ioflags = 0) : Command(name, ioflags) { }
	
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
		
		std::string target = context.getWord();
		
		float damage = context.getFloat();
		
		DebugScript(' ' << type << ' ' << target);
		
		Entity * t = entities.getById(target, context.getEntity());
		if(!t) {
			ScriptWarning << "unknown target: " << target;
			return Failed;
		}
		
		EntityHandle self = (context.getEntity() == NULL) ? EntityHandle() : context.getEntity()->index();
		ARX_DAMAGES_DealDamages(t->index(), damage, self, type, &t->pos);
		
		return Success;
	}
	
};

class DamagerCommand : public AbstractDamageCommand {
	
public:
	
	DamagerCommand() : AbstractDamageCommand("damager", AnyEntity) { }
	
	Result execute(Context & context) {
		
		Entity * io = context.getEntity();
		
		io->damager_type = getDamageType(context) | DAMAGE_TYPE_PER_SECOND;
		
		float damages = context.getFloat();
		
		DebugScript(' ' << io->damager_type << damages);
		
		io->damager_damages = checked_range_cast<short>(damages);
		
		return Success;
	}
	
};

} // anonymous namespace

void setupScriptedIOControl() {
	
	ScriptEvent::registerCommand(new ReplaceMeCommand);
	ScriptEvent::registerCommand(new CollisionCommand);
	ScriptEvent::registerCommand(new SpawnCommand);
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
