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

#include "script/ScriptedIOProperties.h"

#include <cstring>
#include <string>
#include <string_view>

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
	
	Result execute(Context & context) override {
		
		context.getEntity()->shop_category = context.getWord();
		
		DebugScript(' ' << context.getEntity()->shop_category);
		
		return Success;
	}
	
};

class ShopMultiplyCommand : public Command {
	
public:
	
	ShopMultiplyCommand() : Command("shopmultiply", AnyEntity) { }
	
	Result execute(Context & context) override {
		
		context.getEntity()->shop_multiply = context.getFloat();
		
		DebugScript(' ' << context.getEntity()->shop_multiply);
		
		return Success;
	}
	
};

class GameFlagCommand : public Command {
	
	GameFlag flag;
	bool inv;
	
public:
	
	GameFlagCommand(std::string_view name, GameFlag _flag, bool _inv = false)
		: Command(name, AnyEntity), flag(_flag), inv(_inv) { }
	
	Result execute(Context & context) override {
		
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
	
	IOFlagCommand(std::string_view name, EntityFlag _flag, bool _inv = false)
		: Command(name, AnyEntity), flag(_flag), inv(_inv) { }
	
	Result execute(Context & context) override {
		
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
	
	Result execute(Context & context) override {
		
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
	
	Result execute(Context & context) override {
		
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
	
	typedef std::map<std::string_view, Material> Materials;
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
	
	Result execute(Context & context) override {
		
		std::string name = context.getWord();
		
		DebugScript(' ' << name);
		
		if(auto it = materials.find(name); it != materials.end()) {
			context.getEntity()->material = it->second;
		} else {
			ScriptWarning << "unknown material: " << name;
			context.getEntity()->material = MATERIAL_NONE;
		}
		
		return Success;
	}
	
};

class SetNameCommand : public Command {
	
public:
	
	SetNameCommand() : Command("setname", AnyEntity) { }
	
	Result execute(Context & context) override {
		
		context.getEntity()->locname = toLocalizationKey(context.getWord());
		
		DebugScript(' ' << context.getEntity()->locname);
		
		return Success;
	}
	
};

class SetInteractivityCommand : public Command {
	
public:
	
	SetInteractivityCommand() : Command("setinteractivity", AnyEntity) { }
	
	Result execute(Context & context) override {
		
		std::string interactivity = context.getWord();
		
		Entity * io = context.getEntity();
		if(interactivity == "none" || interactivity == "hide") {
			io->gameFlags &= ~GFLAG_INTERACTIVITY;
		} else {
			io->gameFlags |= GFLAG_INTERACTIVITY;
		}
		
		return Success;
	}
	
};

class SetStepMaterialCommand : public Command {
	
public:
	
	SetStepMaterialCommand() : Command("setstepmaterial", AnyEntity) { }
	
	Result execute(Context & context) override {
		
		context.getEntity()->stepmaterial = context.getWord();
		
		DebugScript(' ' << context.getEntity()->stepmaterial);
		
		return Success;
	}
	
};

class SetArmorMaterialCommand : public Command {
	
public:
	
	SetArmorMaterialCommand() : Command("setarmormaterial", AnyEntity) { }
	
	Result execute(Context & context) override {
		
		context.getEntity()->armormaterial = context.getWord();
		
		DebugScript(' ' << context.getEntity()->armormaterial);
		
		return Success;
	}
	
};

class SetWeaponMaterialCommand : public Command {
	
public:
	
	SetWeaponMaterialCommand() : Command("setweaponmaterial", AnyEntity) { }
	
	Result execute(Context & context) override {
		
		context.getEntity()->weaponmaterial = context.getWord();
		
		DebugScript(' ' << context.getEntity()->weaponmaterial);
		
		return Success;
	}
	
};

class SetCollisionCommand : public Command {
	
	IOCollisionFlags::Enum flag;
	
public:
	
	SetCollisionCommand(std::string_view command, IOCollisionFlags::Enum _flag)
		: Command(command, AnyEntity), flag(_flag)
	{ }
	
	Result execute(Context & context) override {
		
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
	
	Result execute(Context & context) override {
		
		context.getEntity()->weight = std::max(context.getFloat(), 0.f);
		
		DebugScript(' ' << context.getEntity()->weight);
		
		return Success;
	}
	
};

class SetTransparencyCommand : public Command {
	
public:
	
	SetTransparencyCommand() : Command("settransparency", AnyEntity) { }
	
	Result execute(Context & context) override {
		
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
	
	Result execute(Context & context) override {
		
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
	
	Result execute(Context & context) override {
		
		context.getEntity()->scale = context.getFloat() * 0.01f;
		
		DebugScript(' ' << context.getEntity()->scale);
		
		return Success;
	}
	
};

class HaloCommand : public Command {
	
public:
	
	HaloCommand() : Command("halo", AnyEntity) { }
	
	Result execute(Context & context) override {
		
		Entity * io = context.getEntity();
		
		HandleFlags("ofncs") {
		
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
	
	Result execute(Context & context) override {
		
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
	
	Result execute(Context & context) override {
		
		res::path mesh = res::path::load(context.getWord());
		
		DebugScript(' ' << mesh);
		
		ARX_INTERACTIVE_MEMO_TWEAK(context.getEntity(), TWEAK_TYPE_MESH, mesh, res::path());
		ARX_INTERACTIVE_USEMESH(context.getEntity(), mesh);
		
		return Success;
	}
	
};

} // anonymous namespace

void setupScriptedIOProperties() {
	
	ScriptEvent::registerCommand(std::make_unique<ShopCategoryCommand>());
	ScriptEvent::registerCommand(std::make_unique<ShopMultiplyCommand>());
	ScriptEvent::registerCommand(std::make_unique<GameFlagCommand>("setplatform", GFLAG_PLATFORM));
	ScriptEvent::registerCommand(std::make_unique<GameFlagCommand>("setgore", GFLAG_NOGORE, true));
	ScriptEvent::registerCommand(std::make_unique<GameFlagCommand>("setelevator", GFLAG_ELEVATOR));
	ScriptEvent::registerCommand(std::make_unique<GameFlagCommand>("viewblock", GFLAG_VIEW_BLOCKER));
	ScriptEvent::registerCommand(std::make_unique<IOFlagCommand>("setunique", IO_UNIQUE));
	ScriptEvent::registerCommand(std::make_unique<IOFlagCommand>("setblacksmith", IO_BLACKSMITH));
	ScriptEvent::registerCommand(std::make_unique<IOFlagCommand>("setangular", IO_ANGULAR));
	ScriptEvent::registerCommand(std::make_unique<IOFlagCommand>("setshadow", IO_NOSHADOW, true));
	ScriptEvent::registerCommand(std::make_unique<IOFlagCommand>("setshop", IO_SHOP));
	ScriptEvent::registerCommand(std::make_unique<IOFlagCommand>("setbump", IO_BUMP));
	// IO_ZMAP Currently has no effect, but keep for now as it affects save state
	ScriptEvent::registerCommand(std::make_unique<IOFlagCommand>("setzmap", IO_ZMAP));
	ScriptEvent::registerCommand(std::make_unique<IOFlagCommand>("invertedobject", IO_INVERTED));
	ScriptEvent::registerCommand(std::make_unique<SetTrapCommand>());
	ScriptEvent::registerCommand(std::make_unique<SetSecretCommand>());
	ScriptEvent::registerCommand(std::make_unique<SetMaterialCommand>());
	ScriptEvent::registerCommand(std::make_unique<SetNameCommand>());
	ScriptEvent::registerCommand(std::make_unique<SetInteractivityCommand>());
	ScriptEvent::registerCommand(std::make_unique<SetStepMaterialCommand>());
	ScriptEvent::registerCommand(std::make_unique<SetArmorMaterialCommand>());
	ScriptEvent::registerCommand(std::make_unique<SetWeaponMaterialCommand>());
	ScriptEvent::registerCommand(std::make_unique<SetCollisionCommand>("setplayercollision", COLLIDE_WITH_PLAYER));
	ScriptEvent::registerCommand(std::make_unique<SetCollisionCommand>("setworldcollision", COLLIDE_WITH_WORLD));
	ScriptEvent::registerCommand(std::make_unique<SetWeightCommand>());
	ScriptEvent::registerCommand(std::make_unique<SetTransparencyCommand>());
	ScriptEvent::registerCommand(std::make_unique<SetIRColorCommand>());
	ScriptEvent::registerCommand(std::make_unique<SetScaleCommand>());
	ScriptEvent::registerCommand(std::make_unique<HaloCommand>());
	ScriptEvent::registerCommand(std::make_unique<TweakCommand>());
	ScriptEvent::registerCommand(std::make_unique<UseMeshCommand>());
	
}

} // namespace script
