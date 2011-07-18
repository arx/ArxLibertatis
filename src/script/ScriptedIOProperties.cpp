
#include "script/ScriptedIOProperties.h"

#include "graphics/Math.h"
#include "graphics/data/Mesh.h"
#include "io/FilePath.h"
#include "scene/Light.h"
#include "scene/Interactive.h"
#include "script/ScriptUtils.h"

using std::string;

namespace script {

namespace {

class ShopCategoryCommand : public Command {
	
public:
	
	ShopCategoryCommand() : Command("shopcategory", ANY_IO) { }
	
	Result execute(Context & context) {
		
		string category = context.getWord();
		
		DebugScript(' ' << category);
		
		INTERACTIVE_OBJ * io = context.getIO();
		if(io->shop_category) {
			free(io->shop_category);
		}
		io->shop_category = strdup(category.c_str());
		
		return Success;
	}
	
};

class ShopMultiplyCommand : public Command {
	
public:
	
	ShopMultiplyCommand() : Command("shopmultiply", ANY_IO) { }
	
	Result execute(Context & context) {
		
		float multiply = context.getFloat();
		
		DebugScript(' ' << multiply);
		
		context.getIO()->shop_multiply = multiply;
		
		return Success;
	}
	
};

class GameFlagCommand : public Command {
	
	unsigned short flag;
	bool inv;
	
public:
	
	GameFlagCommand(string name, short _flag, bool _inv = false) : Command(name, ANY_IO), flag(_flag), inv(_inv) { }
	
	Result execute(Context & context) {
		
		bool enable = context.getBool();
		
		DebugScript(' ' << enable);
		
		INTERACTIVE_OBJ * io = context.getIO();
		
		if(enable xor inv) {
			io->GameFlags |= flag;
		} else {
			io->GameFlags &= ~flag;
		}
		
		return Success;
	}
	
};

class IOFlagCommand : public Command {
	
	long flag;
	bool inv;
	
public:
	
	IOFlagCommand(string name, long _flag, bool _inv = false) : Command(name, ANY_IO), flag(_flag), inv(_inv) { }
	
	Result execute(Context & context) {
		
		bool enable = context.getBool();
		
		DebugScript(' ' << enable);
		
		INTERACTIVE_OBJ * io = context.getIO();
		
		if(enable xor inv) {
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
		
		string trapvalue = context.getWord();
		
		DebugScript(' ' << trapvalue);
		
		if(trapvalue == "off") {
			context.getIO()->_fixdata->trapvalue = -1;
		} else {
			context.getIO()->_fixdata->trapvalue = clamp((int)context.getFloatVar(trapvalue), -1, 100);
		}
		
		return Success;
	}
	
};

class SetSecretCommand : public Command {
	
public:
	
	SetSecretCommand() : Command("setsecret", ANY_IO) { }
	
	Result execute(Context & context) {
		
		string secretvalue = context.getWord();
		
		DebugScript(' ' << secretvalue);
		
		if(secretvalue == "off") {
			context.getIO()->secretvalue = -1;
		} else {
			context.getIO()->secretvalue = clamp((int)context.getFloatVar(secretvalue), -1, 100);
		}
		
		return Success;
	}
	
};

class SetMaterialCommand : public Command {
	
	typedef std::map<std::string, Material> Materials;
	Materials materials;
	
public:
	
	SetMaterialCommand() : Command("setmaterial", ANY_IO) {
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
		
		string name = context.getWord();
		
		DebugScript(' ' << name);
		
		Materials::const_iterator it = materials.find(name);
		if(it == materials.end()) {
			ScriptWarning << "unknown material: " << name;
			context.getIO()->material = MATERIAL_NONE;
		} else {
			context.getIO()->material = it->second;
		}
		
		return Success;
	}
	
};

class SetNameCommand : public Command {
	
public:
	
	SetNameCommand() : Command("setname", ANY_IO) { }
	
	Result execute(Context & context) {
		
		string name = loadUnlocalized(context.getWord());
		
		DebugScript(' ' << name);
		
		strcpy(context.getIO()->locname, name.c_str());
		
		return Success;
	}
	
};

class SetInteractivityCommand : public Command {
	
public:
	
	SetInteractivityCommand() : Command("setinteractivity", ANY_IO) { }
	
	Result execute(Context & context) {
		
		string interactivity = context.getWord();
		
		INTERACTIVE_OBJ * io = context.getIO();
		if(interactivity == "none") {
			io->GameFlags &= ~GFLAG_INTERACTIVITY;
			io->GameFlags &= ~GFLAG_INTERACTIVITYHIDE;
		} else if(interactivity == "hide") {
			io->GameFlags &= ~GFLAG_INTERACTIVITY;
			io->GameFlags |= GFLAG_INTERACTIVITYHIDE;
		} else {
			io->GameFlags |= GFLAG_INTERACTIVITY;
			io->GameFlags &= ~GFLAG_INTERACTIVITYHIDE;
		}
		
		return Success;
	}
	
};

class SetStepMaterialCommand : public Command {
	
public:
	
	SetStepMaterialCommand() : Command("setstepmaterial", ANY_IO) { }
	
	Result execute(Context & context) {
		
		string material = context.getWord();
		
		DebugScript(' ' << material);
		
		INTERACTIVE_OBJ * io = context.getIO();
		if(io->stepmaterial) {
			free(io->stepmaterial);
		}
		io->stepmaterial = strdup(material.c_str());
		
		return Success;
	}
	
};

class SetArmorMaterialCommand : public Command {
	
public:
	
	SetArmorMaterialCommand() : Command("setarmormaterial", ANY_IO) { }
	
	Result execute(Context & context) {
		
		string material = context.getWord();
		
		DebugScript(' ' << material);
		
		INTERACTIVE_OBJ * io = context.getIO();
		if(io->armormaterial) {
			free(io->armormaterial);
		}
		io->armormaterial = strdup(material.c_str());
		
		return Success;
	}
	
};

class SetWeaponMaterialCommand : public Command {
	
public:
	
	SetWeaponMaterialCommand() : Command("setweaponmaterial", ANY_IO) { }
	
	Result execute(Context & context) {
		
		string material = context.getWord();
		
		DebugScript(' ' << material);
		
		INTERACTIVE_OBJ * io = context.getIO();
		if(io->weaponmaterial) {
			free(io->weaponmaterial);
		}
		io->weaponmaterial = strdup(material.c_str());
		
		return Success;
	}
	
};

class SetCollisionCommand : public Command {
	
	short flag;
	
public:
	
	SetCollisionCommand(const string & command, short _flag) : Command(command, ANY_IO), flag(_flag) { }
	
	Result execute(Context & context) {
		
		bool enable = context.getBool();
		
		DebugScript(' ' << enable);
		
		if(enable) {
			context.getIO()->collision |= flag;
		} else {
			context.getIO()->collision &= ~flag;
		}
		
		return Success;
	}
	
};

class SetWeightCommand : public Command {
	
public:
	
	SetWeightCommand() : Command("setweight", ANY_IO) { }
	
	Result execute(Context & context) {
		
		float weight = context.getFloat();
		if(weight < 0.f) {
			weight = 0.f;
		}
		
		DebugScript(' ' << weight);
		
		context.getIO()->weight = weight;
		
		return Success;
	}
	
};

class SetTransparencyCommand : public Command {
	
public:
	
	SetTransparencyCommand() : Command("settransparency", ANY_IO) { }
	
	Result execute(Context & context) {
		
		float trans = context.getFloat();
		
		DebugScript(' ' << trans);
		
		INTERACTIVE_OBJ * io = context.getIO();
		io->invisibility = 1.f + trans * 0.01f;
		if(io->invisibility == 1.f) {
			io->invisibility = 0;
		}
		
		return Success;
	}
	
};

class SetIRColorCommand : public Command {
	
public:
	
	SetIRColorCommand() : Command("setircolor", ANY_IO) { }
	
	Result execute(Context & context) {
		
		float r = context.getFloat();
		float g = context.getFloat();
		float b = context.getFloat();
		
		DebugScript(' ' << r << ' ' << g << ' ' << b);
		
		context.getIO()->infracolor = Color3f(r, g, b);
		
		return Success;
	}
	
};

class SetScaleCommand : public Command {
	
public:
	
	SetScaleCommand() : Command("setscale", ANY_IO) { }
	
	Result execute(Context & context) {
		
		float scale = context.getFloat();
		
		DebugScript(' ' << scale);
		
		context.getIO()->scale = scale * 0.01f;
		
		return Success;
	}
	
};

class HaloCommand : public Command {
	
public:
	
	HaloCommand() : Command("halo", ANY_IO) { }
	
	Result execute(Context & context) {
		
		INTERACTIVE_OBJ * io = context.getIO();
		
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
			
			if(flg & flag('l')) {
				io->halo_native.flags |= HALO_DYNLIGHT;
			} else {
				io->halo_native.flags &= ~HALO_DYNLIGHT;
				if(ValidDynLight(io->halo_native.dynlight)) {
					DynLight[io->halo_native.dynlight].exist = 0;
				}
				io->halo_native.dynlight = -1;
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
	
	TweakCommand() : Command("tweak", ANY_IO) { }
	
	Result execute(Context & context) {
		
		INTERACTIVE_OBJ * io = context.getIO();
		
		string type = context.getWord();
		
		if(type == "skin") {
			
			string oldskin = loadPath(context.getWord());
			string newskin = loadPath(context.getWord());
			
			DebugScript(" skin \"" << oldskin << "\" \"" << newskin << '"');
			
			ARX_INTERACTIVE_MEMO_TWEAK(io, TWEAK_TYPE_SKIN, oldskin, newskin);
			EERIE_MESH_TWEAK_Skin(io->obj, oldskin, newskin);
			
		} else if(type == "icon") {
			
			string icon = loadPath(context.getWord());
			
			DebugScript(" icon \"" << icon << '"');
			
			ARX_INTERACTIVE_MEMO_TWEAK(io, TWEAK_TYPE_ICON, icon, string());
			ARX_INTERACTIVE_TWEAK_Icon(io, icon);
			
		} else if(type == "remove") {
			
			DebugScript(" remove");
			
			ARX_INTERACTIVE_MEMO_TWEAK(io, TWEAK_REMOVE, string(), string());
			EERIE_MESH_TWEAK_Do(io, TWEAK_REMOVE, string());
			
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
			
			string mesh = loadPath(context.getWord());
			
			DebugScript(' ' << type << " \"" << mesh << '"');
			
			string path = io->usemesh ? io->usemesh : io->filename;
			RemoveName(path);
			path += "tweaks\\" + mesh + ".teo";
			
			ARX_INTERACTIVE_MEMO_TWEAK(io, tw, path, string());
			EERIE_MESH_TWEAK_Do(io, tw, path);
		}
		
		return Success;
	}
	
};

class UseMeshCommand : public Command {
	
public:
	
	UseMeshCommand() : Command("usemesh", ANY_IO) { }
	
	Result execute(Context & context) {
		
		string mesh = loadPath(context.getWord());
		
		DebugScript(" \"" << mesh << '"');
		
		ARX_INTERACTIVE_MEMO_TWEAK(context.getIO(), TWEAK_TYPE_MESH, mesh, string());
		ARX_INTERACTIVE_USEMESH(context.getIO(), mesh);
		
		return Success;
	}
	
};

}

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
	ScriptEvent::registerCommand(new IOFlagCommand("satangular", IO_ANGULAR)); // TODO(broken-scripts)
	ScriptEvent::registerCommand(new IOFlagCommand("setshadow", IO_NOSHADOW, true));
	ScriptEvent::registerCommand(new IOFlagCommand("setshop", IO_SHOP));
	ScriptEvent::registerCommand(new IOFlagCommand("setbump", IO_BUMP));
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
	ScriptEvent::registerCommand(new SetCollisionCommand("setplayercollision", 1));
	ScriptEvent::registerCommand(new SetCollisionCommand("setworldcollision", 2));
	ScriptEvent::registerCommand(new SetWeightCommand);
	ScriptEvent::registerCommand(new SetTransparencyCommand);
	ScriptEvent::registerCommand(new SetIRColorCommand);
	ScriptEvent::registerCommand(new SetScaleCommand);
	ScriptEvent::registerCommand(new HaloCommand);
	ScriptEvent::registerCommand(new TweakCommand);
	ScriptEvent::registerCommand(new UseMeshCommand);
	
}

} // namespace script
