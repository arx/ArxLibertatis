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

#include "script/ScriptedIOProperties.h"

#include <cstring>

#include "game/Entity.h"
#include "graphics/Math.h"
#include "graphics/data/Mesh.h"
#include "io/resource/ResourcePath.h"
#include "scene/Light.h"
#include "scene/Interactive.h"
#include "script/ScriptUtils.h"


namespace script {

namespace {

class ShopCategoryCommand : public Command {
	
public:
	
	ShopCategoryCommand() : Command("shopcategory", AnyEntity) { }
	
	Result execute(Context & context) {
		
		context.getEntity()->shop_category = context.getWord();
		
		DebugScript(' ' << context.getEntity()->shop_category);
		
		return Success;
	}
	
};

class ShopMultiplyCommand : public Command {
	
public:
	
	ShopMultiplyCommand() : Command("shopmultiply", AnyEntity) { }
	
	Result execute(Context & context) {
		
		context.getEntity()->shop_multiply = context.getFloat();
		
		DebugScript(' ' << context.getEntity()->shop_multiply);
		
		return Success;
	}
	
};

class GameFlagCommand : public Command {
	
	GameFlag flag;
	bool inv;
	
public:
	
	GameFlagCommand(const std::string & name, GameFlag _flag, bool _inv = false)
		: Command(name, AnyEntity), flag(_flag), inv(_inv) { }
	
	Result execute(Context & context) {
		
		bool enable = context.getBool();
		
		DebugScript(' ' << enable);
		
		Entity * io = context.getEntity();
		
		if(enable != inv) {
			io->gameFlags |= flag;
		} else {
			io->gameFlags &= ~flag;
		}
		
		return Success;
	}
	
};

class IOFlagCommand : public Command {
	
	EntityFlag flag;
	bool inv;
	
public:
	
	IOFlagCommand(const std::string & name, EntityFlag _flag, bool _inv = false)
		: Command(name, AnyEntity), flag(_flag), inv(_inv) { }
	
	Result execute(Context & context) {
		
		bool enable = context.getBool();
		
		DebugScript(' ' << enable);
		
		Entity * io = context.getEntity();
		
		if(enable != inv) {
			io->ioflags |= flag;
		} else {
			io->ioflags &= ~flag;
		}
		
		return Success;
	}
	
};

class SetTrapCommand : public Command {
	
public:
	
	SetTrapCommand() : Command("settrap", IO_FIX) { }
	
	Result execute(Context & context) {
		
		std::string trapvalue = context.getWord();
		
		DebugScript(' ' << trapvalue);
		
		if(trapvalue == "off") {
			context.getEntity()->_fixdata->trapvalue = -1;
		} else {
			context.getEntity()->_fixdata->trapvalue = glm::clamp(int(context.getFloatVar(trapvalue)), -1, 100);
		}
		
		return Success;
	}
	
};

class SetSecretCommand : public Command {
	
public:
	
	SetSecretCommand() : Command("setsecret", AnyEntity) { }
	
	Result execute(Context & context) {
		
		std::string secretvalue = context.getWord();
		
		DebugScript(' ' << secretvalue);
		
		if(secretvalue == "off") {
			context.getEntity()->secretvalue = -1;
		} else {
			context.getEntity()->secretvalue = glm::clamp(int(context.getFloatVar(secretvalue)), -1, 100);
		}
		
		return Success;
	}
	
};

class SetMaterialCommand : public Command {
	
	typedef std::map<std::string, Material> Materials;
	Materials materials;
	
public:
	
	SetMaterialCommand() : Command("setmaterial", AnyEntity) {
		materials["weapon"] = MATERIAL_WEAPON;
		materials["flesh"] = MATERIAL_FLESH;
		materials["metal"] = MATERIAL_METAL;
		materials["glass"] = MATERIAL_GLASS;
		materials["cloth"] = MATERIAL_CLOTH;
		materials["wood"] = MATERIAL_WOOD;
		materials["earth"] = MATERIAL_EARTH;
		materials["water"] = MATERIAL_WATER;
		materials["ice"] = MATERIAL_ICE;
		materials["gravel"] = MATERIAL_GRAVEL;
		materials["stone"] = MATERIAL_STONE;
		materials["foot_large"] = MATERIAL_FOOT_LARGE;
		materials["foot_bare"] = MATERIAL_FOOT_BARE;
		materials["foot_shoe"] = MATERIAL_FOOT_SHOE;
		materials["foot_metal"] = MATERIAL_FOOT_METAL;
		materials["foot_stealth"] = MATERIAL_FOOT_STEALTH;
	}
	
	Result execute(Context & context) {
		
		std::string name = context.getWord();
		
		DebugScript(' ' << name);
		
		Materials::const_iterator it = materials.find(name);
		if(it == materials.end()) {
			ScriptWarning << "unknown material: " << name;
			context.getEntity()->material = MATERIAL_NONE;
		} else {
			context.getEntity()->material = it->second;
		}
		
		return Success;
	}
	
};

class SetNameCommand : public Command {
	
public:
	
	SetNameCommand() : Command("setname", AnyEntity) { }
	
	Result execute(Context & context) {
		
		context.getEntity()->locname = loadUnlocalized(context.getWord());
		
		DebugScript(' ' << context.getEntity()->locname);
		
		return Success;
	}
	
};

class SetInteractivityCommand : public Command {
	
public:
	
	SetInteractivityCommand() : Command("setinteractivity", AnyEntity) { }
	
	Result execute(Context & context) {
		
		std::string interactivity = context.getWord();
		
		Entity * io = context.getEntity();
		if(interactivity == "none") {
			io->gameFlags &= ~GFLAG_INTERACTIVITY;
			io->gameFlags &= ~GFLAG_INTERACTIVITYHIDE;
		} else if(interactivity == "hide") {
			io->gameFlags &= ~GFLAG_INTERACTIVITY;
			io->gameFlags |= GFLAG_INTERACTIVITYHIDE;
		} else {
			io->gameFlags |= GFLAG_INTERACTIVITY;
			io->gameFlags &= ~GFLAG_INTERACTIVITYHIDE;
		}
		
		return Success;
	}
	
};

class SetStepMaterialCommand : public Command {
	
public:
	
	SetStepMaterialCommand() : Command("setstepmaterial", AnyEntity) { }
	
	Result execute(Context & context) {
		
		context.getEntity()->stepmaterial = context.getWord();
		
		DebugScript(' ' << context.getEntity()->stepmaterial);
		
		return Success;
	}
	
};

class SetArmorMaterialCommand : public Command {
	
public:
	
	SetArmorMaterialCommand() : Command("setarmormaterial", AnyEntity) { }
	
	Result execute(Context & context) {
		
		context.getEntity()->armormaterial = context.getWord();
		
		DebugScript(' ' << context.getEntity()->armormaterial);
		
		return Success;
	}
	
};

class SetWeaponMaterialCommand : public Command {
	
public:
	
	SetWeaponMaterialCommand() : Command("setweaponmaterial", AnyEntity) { }
	
	Result execute(Context & context) {
		
		context.getEntity()->weaponmaterial = context.getWord();
		
		DebugScript(' ' << context.getEntity()->weaponmaterial);
		
		return Success;
	}
	
};

class SetCollisionCommand : public Command {
	
	IOCollisionFlags::Enum flag;
	
public:
	
	SetCollisionCommand(const std::string & command, IOCollisionFlags::Enum _flag) : Command(command, AnyEntity), flag(_flag) { }
	
	Result execute(Context & context) {
		
		bool enable = context.getBool();
		
		DebugScript(' ' << enable);
		
		if(enable) {
			context.getEntity()->collision |= flag;
		} else {
			context.getEntity()->collision &= ~flag;
		}
		
		return Success;
	}
	
};

class SetWeightCommand : public Command {
	
public:
	
	SetWeightCommand() : Command("setweight", AnyEntity) { }
	
	Result execute(Context & context) {
		
		context.getEntity()->weight = std::max(context.getFloat(), 0.f);
		
		DebugScript(' ' << context.getEntity()->weight);
		
		return Success;
	}
	
};

class SetTransparencyCommand : public Command {
	
public:
	
	SetTransparencyCommand() : Command("settransparency", AnyEntity) { }
	
	Result execute(Context & context) {
		
		float trans = context.getFloat();
		
		DebugScript(' ' << trans);
		
		Entity * io = context.getEntity();
		io->invisibility = 1.f + trans * 0.01f;
		if(io->invisibility == 1.f) {
			io->invisibility = 0;
		}
		
		return Success;
	}
	
};

class SetIRColorCommand : public Command {
	
public:
	
	SetIRColorCommand() : Command("setircolor", AnyEntity) { }
	
	Result execute(Context & context) {
		
		float r = context.getFloat();
		float g = context.getFloat();
		float b = context.getFloat();
		
		DebugScript(' ' << r << ' ' << g << ' ' << b);
		
		context.getEntity()->infracolor = Color3f(r, g, b);
		
		return Success;
	}
	
};

class SetScaleCommand : public Command {
	
public:
	
	SetScaleCommand() : Command("setscale", AnyEntity) { }
	
	Result execute(Context & context) {
		
		context.getEntity()->scale = context.getFloat() * 0.01f;
		
		DebugScript(' ' << context.getEntity()->scale);
		
		return Success;
	}
	
};

class HaloCommand : public Command {
	
public:
	
	HaloCommand() : Command("halo", AnyEntity) { }
	
	Result execute(Context & context) {
		
		Entity * io = context.getEntity();
		
		HandleFlags("ofnlcs") {
		
			if(flg & flag('o')) {
				io->halo_native.flags |= HALO_ACTIVE;
			}
			
			if(flg & flag('f')) {
				io->halo_native.flags &= ~HALO_ACTIVE;
			}
			
			if(flg & flag('n')) {
				io->halo_native.flags |= HALO_NEGATIVE;
			} else {
				io->halo_native.flags &= ~HALO_NEGATIVE;
			}
			
			if(flg & flag('c')) {
				io->halo_native.color.r = context.getFloat();
				io->halo_native.color.g = context.getFloat();
				io->halo_native.color.b = context.getFloat();
			}
			
			if(flg & flag('s')) {
				io->halo_native.radius = context.getFloat();
			}
			
		}
		
		DebugScript(' ' << options);
		
		ARX_HALO_SetToNative(io);
		
		return Success;
	}
	
};

class TweakCommand : public Command {
	
public:
	
	TweakCommand() : Command("tweak", AnyEntity) { }
	
	Result execute(Context & context) {
		
		Entity * io = context.getEntity();
		
		std::string type = context.getWord();
		
		if(type == "skin") {
			
			res::path oldskin = res::path::load(context.getWord());
			res::path newskin = res::path::load(context.getWord());
			
			DebugScript(" skin " << oldskin << ' ' << newskin);
			
			ARX_INTERACTIVE_MEMO_TWEAK(io, TWEAK_TYPE_SKIN, oldskin, newskin);
			EERIE_MESH_TWEAK_Skin(io->obj, oldskin, newskin);
			
		} else if(type == "icon") {
			
			res::path icon = res::path::load(context.getWord());
			
			DebugScript(" icon " << icon);
			
			ARX_INTERACTIVE_MEMO_TWEAK(io, TWEAK_TYPE_ICON, icon, res::path());
			ARX_INTERACTIVE_TWEAK_Icon(io, icon);
			
		} else if(type == "remove") {
			
			DebugScript(" remove");
			
			ARX_INTERACTIVE_MEMO_TWEAK(io, TWEAK_REMOVE, res::path(), res::path());
			EERIE_MESH_TWEAK_Do(io, TWEAK_REMOVE, res::path());
			
		} else {
			
			TweakType tw;
			if(type == "head") {
				tw = TWEAK_HEAD;
			} else if(type == "torso") {
				tw = TWEAK_TORSO;
			} else if(type == "legs") {
				tw = TWEAK_LEGS;
			} else if(type == "all") {
				tw = TWEAK_HEAD | TWEAK_TORSO | TWEAK_LEGS;
			} else if(type == "upper") {
				tw = TWEAK_HEAD | TWEAK_TORSO;
			} else if(type == "lower") {
				tw = TWEAK_TORSO | TWEAK_LEGS;
			} else if(type == "up_lo") {
				tw = TWEAK_HEAD | TWEAK_LEGS;
			} else {
				ScriptWarning << "unknown tweak type: " << type;
				return Failed;
			}
			
			res::path mesh = res::path::load(context.getWord()).append(".teo");
			
			DebugScript(' ' << type << ' ' << mesh);
			
			if(io->usemesh.empty()) {
				mesh = io->classPath().parent() / "tweaks" / mesh;
			} else {
				mesh = io->usemesh.parent() / "tweaks" / mesh;
			}
			
			ARX_INTERACTIVE_MEMO_TWEAK(io, tw, mesh, res::path());
			EERIE_MESH_TWEAK_Do(io, tw, mesh);
		}
		
		return Success;
	}
	
};

class UseMeshCommand : public Command {
	
public:
	
	UseMeshCommand() : Command("usemesh", AnyEntity) { }
	
	Result execute(Context & context) {
		
		res::path mesh = res::path::load(context.getWord());
		
		DebugScript(' ' << mesh);
		
		ARX_INTERACTIVE_MEMO_TWEAK(context.getEntity(), TWEAK_TYPE_MESH, mesh, res::path());
		ARX_INTERACTIVE_USEMESH(context.getEntity(), mesh);
		
		return Success;
	}
	
};

} // anonymous namespace

void setupScriptedIOProperties() {
	
	ScriptEvent::registerCommand(new ShopCategoryCommand);
	ScriptEvent::registerCommand(new ShopMultiplyCommand);
	ScriptEvent::registerCommand(new GameFlagCommand("setplatform", GFLAG_PLATFORM));
	ScriptEvent::registerCommand(new GameFlagCommand("setgore", GFLAG_NOGORE, true));
	ScriptEvent::registerCommand(new GameFlagCommand("setelevator", GFLAG_ELEVATOR));
	ScriptEvent::registerCommand(new GameFlagCommand("viewblock", GFLAG_VIEW_BLOCKER));
	ScriptEvent::registerCommand(new IOFlagCommand("setunique", IO_UNIQUE));
	ScriptEvent::registerCommand(new IOFlagCommand("setblacksmith", IO_BLACKSMITH));
	ScriptEvent::registerCommand(new IOFlagCommand("setangular", IO_ANGULAR));
	ScriptEvent::registerCommand(new IOFlagCommand("setshadow", IO_NOSHADOW, true));
	ScriptEvent::registerCommand(new IOFlagCommand("setshop", IO_SHOP));
	ScriptEvent::registerCommand(new IOFlagCommand("setbump", IO_BUMP));
	// IO_ZMAP Currently has no effect, but keep for now as it affects save state
	ScriptEvent::registerCommand(new IOFlagCommand("setzmap", IO_ZMAP));
	ScriptEvent::registerCommand(new IOFlagCommand("invertedobject", IO_INVERTED));
	ScriptEvent::registerCommand(new SetTrapCommand);
	ScriptEvent::registerCommand(new SetSecretCommand);
	ScriptEvent::registerCommand(new SetMaterialCommand);
	ScriptEvent::registerCommand(new SetNameCommand);
	ScriptEvent::registerCommand(new SetInteractivityCommand);
	ScriptEvent::registerCommand(new SetStepMaterialCommand);
	ScriptEvent::registerCommand(new SetArmorMaterialCommand);
	ScriptEvent::registerCommand(new SetWeaponMaterialCommand);
	ScriptEvent::registerCommand(new SetCollisionCommand("setplayercollision", COLLIDE_WITH_PLAYER));
	ScriptEvent::registerCommand(new SetCollisionCommand("setworldcollision", COLLIDE_WITH_WORLD));
	ScriptEvent::registerCommand(new SetWeightCommand);
	ScriptEvent::registerCommand(new SetTransparencyCommand);
	ScriptEvent::registerCommand(new SetIRColorCommand);
	ScriptEvent::registerCommand(new SetScaleCommand);
	ScriptEvent::registerCommand(new HaloCommand);
	ScriptEvent::registerCommand(new TweakCommand);
	ScriptEvent::registerCommand(new UseMeshCommand);
	
}

} // namespace script
