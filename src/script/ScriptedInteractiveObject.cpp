
#include "script/ScriptedInteractiveObject.h"

#include "core/Core.h"
#include "core/GameTime.h"
#include "game/Inventory.h"
#include "game/Player.h"
#include "game/Missile.h"
#include "game/NPC.h"
#include "graphics/Math.h"
#include "graphics/data/MeshManipulation.h"
#include "graphics/data/Mesh.h"
#include "gui/Interface.h"
#include "io/Logger.h"
#include "io/FilePath.h"
#include "physics/Collisions.h"
#include "scene/Light.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"
#include "script/ScriptUtils.h"

using std::string;

extern INTERACTIVE_OBJ * LASTSPAWNED;
extern long TELEPORT_TO_CONFIRM;
extern long CHANGE_LEVEL_ICON;

namespace script {

namespace {

class ReplaceMeCommand : public Command {
	
public:
	
	ReplaceMeCommand() : Command("replaceme", ANY_IO) { }
	
	Result execute(Context & context) {
		
		string object = context.getLowercase();
		
		DebugScript(" \"" << object << '"');
		
		INTERACTIVE_OBJ * io = context.getIO();
		
		string tex2;
		if(io->ioflags & IO_NPC) {
			tex2 = "graph\\obj3d\\interactive\\npc\\" + object + ".teo";
		} else if(io->ioflags & IO_FIX) {
			tex2 = "graph\\obj3d\\interactive\\fix_inter\\" + object + ".teo";
		} else {
			tex2 = "graph\\obj3d\\interactive\\items\\" + object + ".teo";
		}
		string tex;
		File_Standardize(tex2, tex);
		
		Anglef last_angle = io->angle;
		INTERACTIVE_OBJ * ioo = AddInteractive(tex, -1);
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

class RotateCommand : public Command {
	
public:
	
	RotateCommand() : Command("rotate", ANY_IO) { }
	
	Result execute(Context & context) {
		
		INTERACTIVE_OBJ * io = context.getIO();
		
		float t1 = context.getFloat();
		float t2 = context.getFloat();
		float t3 = context.getFloat();
		
		DebugScript(' ' << t1 << ' ' << t2 << ' ' << t3);
		
		io->angle.a += t1;
		io->angle.b += t2;
		io->angle.g += t3;
		
		if((size_t)io->nb_lastanimvertex != io->obj->vertexlist.size()) {
			free(io->lastanimvertex);
			io->lastanimvertex = NULL;
		}
		io->lastanimtime = 0;
		
		return Success;
	}
	
};

class CollisionCommand : public Command {
	
public:
	
	CollisionCommand(const string & command) : Command(command, ANY_IO) { }
	
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

class ShopCategoryCommand : public Command {
	
public:
	
	ShopCategoryCommand() : Command("shopcategory", ANY_IO) { }
	
	Result execute(Context & context) {
		
		string category = context.getLowercase();
		
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
		
		string trapvalue = context.getLowercase();
		
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
		
		string secretvalue = context.getLowercase();
		
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
		
		string name = context.getLowercase();
		
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
		
		string name = loadUnlocalized(context.getLowercase());
		
		DebugScript(' ' << name);
		
		strcpy(context.getIO()->locname, name.c_str());
		
		return Success;
	}
	
};

class SpawnCommand : public Command {
	
public:
	
	SpawnCommand() : Command("spawn") { }
	
	Result execute(Context & context) {
		
		string type = context.getLowercase();
		
		if(type == "npc" || type == "item") {
			
			string file = loadPath(context.getWord()); // object to spawn.
			
			string target = context.getLowercase(); // object ident for position
			long t = GetTargetByNameTarget(target);
			if(t == -2) {
				t = GetInterNum(context.getIO());
			}
			if(!ValidIONum(t)) {
				ScriptWarning << "unknown target: npc " << file << ' ' << target;
				return Failed;
			}
			
			DebugScript(" npc " << file << ' ' << target);
			
			if(FORBID_SCRIPT_IO_CREATION) {
				return Failed;
			}
			
			string path;
			File_Standardize("graph\\obj3d\\interactive\\npc\\" + file, path);
			
			if(type == "npc") {
				
				INTERACTIVE_OBJ * ioo = AddNPC(path, IO_IMMEDIATELOAD);
				if(!ioo) {
					ScriptWarning << "failed to create npc " << path;
					return Failed;
				}
				
				LASTSPAWNED = ioo;
				ioo->scriptload = 1;
				ioo->pos = inter.iobj[t]->pos;
				
				ioo->angle = inter.iobj[t]->angle;
				MakeTemporaryIOIdent(ioo);
				SendInitScriptEvent(ioo);
				
				if(inter.iobj[t]->ioflags & IO_NPC) {
					float dist = inter.iobj[t]->physics.cyl.radius + ioo->physics.cyl.radius + 10;
					ioo->pos.x += -EEsin(radians(inter.iobj[t]->angle.b)) * dist;
					ioo->pos.z += EEcos(radians(inter.iobj[t]->angle.b)) * dist;
				}
				
				TREATZONE_AddIO(ioo, GetInterNum(ioo));
				
			} else {
				
				INTERACTIVE_OBJ * ioo = AddItem(path, IO_IMMEDIATELOAD);
				if(!ioo) {
					ScriptWarning << "failed to create item " << path;
					return Failed;
				}
				
				MakeTemporaryIOIdent(ioo);
				LASTSPAWNED = ioo;
				ioo->scriptload = 1;
				ioo->pos = inter.iobj[t]->pos;
				ioo->angle = inter.iobj[t]->angle;
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

class SetInteractivityCommand : public Command {
	
public:
	
	SetInteractivityCommand() : Command("setinteractivity", ANY_IO) { }
	
	Result execute(Context & context) {
		
		string interactivity = context.getLowercase();
		
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
		
		string material = context.getLowercase();
		
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
		
		string material = context.getLowercase();
		
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
		
		string material = context.getLowercase();
		
		DebugScript(' ' << material);
		
		INTERACTIVE_OBJ * io = context.getIO();
		if(io->weaponmaterial) {
			free(io->weaponmaterial);
		}
		io->weaponmaterial = strdup(material.c_str());
		
		return Success;
	}
	
};

class SetStrikeSpeechCommand : public Command {
	
public:
	
	SetStrikeSpeechCommand() : Command("setstrikespeech", ANY_IO) { }
	
	Result execute(Context & context) {
		
		string speech = loadPath(context.getWord());
		
		DebugScript(' ' << speech);
		
		INTERACTIVE_OBJ * io = context.getIO();
		if(io->strikespeech) {
			free(io->strikespeech);
		}
		io->strikespeech = strdup(speech.c_str());
		
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

class ForceAnimCommand : public Command {
	
public:
	
	ForceAnimCommand() : Command("forceanim", ANY_IO) { }
	
	Result execute(Context & context) {
		
		string anim = context.getLowercase();
		
		DebugScript(' ' << anim);
		
		AnimationNumber num = GetNumAnim(anim);
		if(num == ANIM_NONE) {
			ScriptWarning << "unknown animation: " << anim;
			return Failed;
		}
		
		INTERACTIVE_OBJ * io = context.getIO();
		if(io->anims[num]) {
			ForceAnim(io, io->anims[num]);
			CheckSetAnimOutOfTreatZone(io, 0);
		}
		
		return Success;
	}
	
};

class ForceAngleCommand : public Command {
	
public:
	
	ForceAngleCommand() : Command("forceangle", ANY_IO) { }
	
	Result execute(Context & context) {
		
		float angle = MAKEANGLE(context.getFloat());
		
		DebugScript(' ' << angle);
		
		context.getIO()->angle.b = angle;
		
		return Success;
	}
	
};

class PlayAnimCommand : public Command {
	
	static void SetNextAnim(INTERACTIVE_OBJ * io, ANIM_HANDLE * ea, long layer, bool loop, bool nointerpol) {
		
		if(IsDeadNPC(io)) {
			return;
		}
		
		if(nointerpol) {
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
		
		INTERACTIVE_OBJ * iot = context.getIO();
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
			loop = (flg & flag('l'));
			nointerpol = (flg & flag('n'));
			execute = (flg & flag('e'));
			if(flg & flag('p')) {
				iot = inter.iobj[0];
				iot->move = iot->lastmove = Vec3f::ZERO;
			}
		}
		
		string anim = context.getLowercase();
		
		DebugScript(' ' << options << ' ' << anim);
		
		if(!iot) {
			ScriptWarning << "must either use -p or use with IO";
			return Failed;
		}
		
		if(anim == "none") {
			iot->animlayer[nu].cur_anim = NULL;
			iot->animlayer[nu].next_anim = NULL;
			return Success;
		}
		
		AnimationNumber num = GetNumAnim(anim);
		if(num == ANIM_NONE) {
			ScriptWarning << "unknown anim: " << anim;
			return Failed;
		}
		
		if(!iot->anims[num]) {
			return Success;
		}
		
		iot->ioflags |= IO_NO_PHYSICS_INTERPOL;
		SetNextAnim(iot, iot->anims[num], nu, loop, nointerpol);
		
		if(!loop) {
			CheckSetAnimOutOfTreatZone(iot, nu);
		}
		
		if(iot == inter.iobj[0]) {
			iot->animlayer[nu].flags &= ~EA_STATICANIM;
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
				scr_timer[num2].io = context.getIO();
				scr_timer[num2].msecs = max(iot->anims[num]->anims[iot->animlayer[nu].altidx_cur]->anim_time, 1000.f);
				scr_timer[num2].name = timername;
				scr_timer[num2].pos = pos;
				scr_timer[num2].tim = ARXTimeUL();
				scr_timer[num2].times = 1;
				scr_timer[num2].longinfo = 0;
			}
		}
		
		return Success;
	}
	
};

class PhysicalCommand : public Command {
	
public:
	
	PhysicalCommand() : Command("physical", ANY_IO) { }
	
	Result execute(Context & context) {
		
		string type = context.getLowercase();
		
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

class LoadAnimCommand : public Command {
	
public:
	
	LoadAnimCommand() : Command("loadanim") { }
	
	Result execute(Context & context) {
		
		INTERACTIVE_OBJ * iot = context.getIO();
		
		HandleFlags("p") {
			if(flg & flag('p')) {
				iot = inter.iobj[0];
			}
		}
		
		string anim = context.getLowercase();
		
		string file = loadPath(context.getWord());
		
		DebugScript(' ' << options << ' ' << anim << ' ' << file);
		
		
		if(!iot) {
			ScriptWarning << "must either use -p or use with IO";
			return Failed;
		}
		
		AnimationNumber num = GetNumAnim(anim);
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
		
		if(iot == inter.iobj[0] || (iot->ioflags & IO_NPC)) {
			file = "graph\\obj3d\\anims\\npc\\" + file;
		} else {
			file = "graph\\obj3d\\anims\\fix_inter\\" + file;
		}
		SetExt(file, ".tea");
		string path;
		File_Standardize(file, path);
		
		iot->anims[num] = EERIE_ANIMMANAGER_Load_NoWarning(path);
		
		if(!iot->anims[num]) {
			ScriptWarning << "animation not found: " << path;
			return Failed;
		}
		
		return Success;
	}
	
};

class LinkObjToMeCommand : public Command {
	
public:
	
	LinkObjToMeCommand() : Command("linkobjtome", ANY_IO) { }
	
	Result execute(Context & context) {
		
		string name = toLowercase(context.getStringVar(context.getLowercase()));
		
		string attach = context.getLowercase();
		
		DebugScript(' ' << name << ' ' << attach);
		
		long t = GetTargetByNameTarget(name);
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
		
		string target = context.getLowercase();
		
		DebugScript(' ' << target);
		
		long t = GetTargetByNameTarget(target);
		
		if(t == -1) {
			context.skipStatement();
		}
		
		return Jumped;
	}
	
};

class IfVisibleCommand : public Command {
	
	static bool hasVisibility(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * ioo) {
		
		if(distSqr(io->pos, ioo->pos) > square(20000)) {
			return false;
		}
		
		float ab = MAKEANGLE(io->angle.b);
		float aa = GetAngle(io->pos.x, io->pos.z, ioo->pos.x, ioo->pos.z);
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
		
		string target = context.getLowercase();
		
		DebugScript(' ' << target);
		
		long t = GetTargetByNameTarget(target);
		
		if(!ValidIONum(t) || !hasVisibility(context.getIO(), inter.iobj[t])) {
			context.skipStatement();
		}
		
		return Jumped;
	}
	
};

class InventoryCommand : public Command {
	
	class SubCommand : public Command {
		
		const string command;
		
	public:
		
		SubCommand(const std::string & name) : Command("inventory " + name, ANY_IO), command(name) { }
		
		inline const string & getCommand() { return command; }
		
	};
	
	typedef std::map<string, SubCommand *> Commands;
	Commands commands;
	
	void addCommand(SubCommand * command) {
		
		typedef std::pair<Commands::iterator, bool> Res;
		
		Res res = commands.insert(std::make_pair(command->getCommand(), command));
		
		if(!res.second) {
			LogError << "duplicate script inventory command name: " + command->getCommand();
			delete command;
		}
		
	}
	
	class CreateCommand : public SubCommand {
		
	public:
		
		CreateCommand() : SubCommand("create") { }
		
		Result execute(Context & context) {
			
			DebugScript("");
			
			INTERACTIVE_OBJ * io = context.getIO();
			
			if(io->inventory) {
				
				INVENTORY_DATA * id = io->inventory;
				
				for(long nj = 0; nj < id->sizey; nj++) {
					for(long ni = 0; ni < id->sizex; ni++) {
						
						INTERACTIVE_OBJ * item = id->slot[ni][nj].io;
						if(!item) {
							continue;
						}
						
						if(item->scriptload) {
							long tmp = GetInterNum(item);
							arx_assert(ValidIONum(tmp) && inter.iobj[tmp] == item);
							RemoveFromAllInventories(item);
							ReleaseInter(item);
							inter.iobj[tmp] = NULL;
						} else {
							item->show = SHOW_FLAG_KILLED;
						}
						
						id->slot[ni][nj].io = NULL;
					}
				}
				
				free(io->inventory);
			}
			
			io->inventory = (INVENTORY_DATA *)malloc(sizeof(INVENTORY_DATA));
			memset(io->inventory, 0, sizeof(INVENTORY_DATA));
			io->inventory->sizex = 3;
			io->inventory->sizey = 11;
			io->inventory->io = io;
			
			return Success;
		}
		
	};
	
	class SkinCommand : public SubCommand {
		
	public:
		
		SkinCommand() : SubCommand("skin") { }
		
		Result execute(Context & context) {
			
			string skin = loadPath(context.getLowercase());
			
			DebugScript(" \"" << skin << '"');
			
			INTERACTIVE_OBJ * io = context.getIO();
			if(io->inventory_skin) {
				free(io->inventory_skin);
			}
			io->inventory_skin = strdup(skin.c_str());
			
			return Success;
		}
		
	};
	
	class PlayerAddFromSceneCommand : public SubCommand {
		
	public:
		
		PlayerAddFromSceneCommand() : SubCommand("playeraddfromscene") { }
		
		Result execute(Context & context) {
			
			string target = context.getLowercase();
			
			DebugScript(' ' << target);
			
			long t = GetTargetByNameTarget(target);
			if(t == -2) {
				t = GetInterNum(context.getIO());
			}
			if(!ValidIONum(t)) {
				ScriptWarning << "unknown target: " << target;
				return Failed;
			}
			
			RemoveFromAllInventories(inter.iobj[t]);
			inter.iobj[t]->show = SHOW_FLAG_IN_INVENTORY;
			if(!CanBePutInInventory(inter.iobj[t])) {
				PutInFrontOfPlayer(inter.iobj[t]);
			}
			
			return Success;
		}
		
	};
	
	class PlayerAddCommand : public SubCommand {
		
		const bool multi;
		
	public:
		
		PlayerAddCommand(const string & name, bool _multi) : SubCommand(name), multi(_multi) { }
		
		Result execute(Context & context) {
			
			string file = loadPath(context.getLowercase());
			
			if(FORBID_SCRIPT_IO_CREATION) {
				if(multi) {
					context.skipWord();
				}
				return Failed;
			}
			
			File_Standardize("graph\\obj3d\\interactive\\items\\" + file + ".teo", file);
			
			INTERACTIVE_OBJ * ioo = (INTERACTIVE_OBJ *)AddItem(file, IO_IMMEDIATELOAD);
			if(!ioo) {
				ScriptWarning << "could not add item \"" << file << '"';
				return Failed;
			}
			
			LASTSPAWNED = ioo;
			ioo->scriptload = 1;
			MakeTemporaryIOIdent(ioo);
			SendInitScriptEvent(ioo);
			
			if(multi) {
				
				float count = context.getFloat();
				
				DebugScript(' ' << file << ' ' << count);
				
				if(ioo->ioflags & IO_GOLD) {
					ioo->_itemdata->price = static_cast<long>(count);
				} else {
					ioo->_itemdata->maxcount = 9999;
					ARX_CHECK_SHORT(count);
					ioo->_itemdata->count = std::max(static_cast<short>(count), (short)1);
				}
				
			} else {
				DebugScript(' ' << file);
			}
			
			ioo->show = SHOW_FLAG_IN_INVENTORY;
			
			if(!CanBePutInInventory(ioo)) {
				PutInFrontOfPlayer(ioo);
			}
			
			return Success;
		}
		
	};
	
	class AddFromSceneCommand : public SubCommand {
		
	public:
		
		AddFromSceneCommand() : SubCommand("addfromscene") { }
		
		Result execute(Context & context) {
			
			string target = context.getLowercase();
			
			DebugScript(' ' << target);
			
			long t = GetTargetByNameTarget(target);
			if(t == -2) {
				t = GetInterNum(context.getIO());
			}
			if(!ValidIONum(t)) {
				ScriptWarning << "unknown target: " << target;
				return Failed;
			}
			
			if(ARX_EQUIPMENT_IsPlayerEquip(inter.iobj[t])) {
				ARX_EQUIPMENT_UnEquip(inter.iobj[0], inter.iobj[t], 1);
			} else {
				RemoveFromAllInventories(inter.iobj[t]);
			}
			
			inter.iobj[t]->scriptload = 0;
			inter.iobj[t]->show = SHOW_FLAG_IN_INVENTORY;
			
			long xx, yy;
			if(!CanBePutInSecondaryInventory(context.getIO()->inventory, inter.iobj[t], &xx, &yy)) {
				PutInFrontOfPlayer(inter.iobj[t]);
			}
			
			return Success;
		}
		
	};
	
	class AddCommand : public SubCommand {
		
		const bool multi;
		
	public:
		
		AddCommand(const string & name, bool _multi) : SubCommand(name), multi(_multi) { }
		
		Result execute(Context & context) {
			
			string file = loadPath(context.getLowercase());
			
			INTERACTIVE_OBJ * io = context.getIO();
			
			if(FORBID_SCRIPT_IO_CREATION || !io->inventory) {
				if(multi) {
					context.skipWord();
				}
				return Failed;
			}
			
			File_Standardize("graph\\obj3d\\interactive\\items\\" + file + ".teo", file);
			
			long count = -1;
			if(multi) {
				float val = context.getFloat();
				
				DebugScript(" \"" << file << "\" " << val);
				
				count = static_cast<long>(val);
				
			} else {
				DebugScript(" \"" << file << '"');
			}
			
			INTERACTIVE_OBJ * ioo = (INTERACTIVE_OBJ *)AddItem(file, IO_IMMEDIATELOAD);
			if(!ioo) {
				ScriptWarning << "could not add item \"" << file << '"';
				return Failed;
			}
			
			if(!count) {
				return Success;
			}
			
			LASTSPAWNED = ioo;
			ioo->scriptload = 1;
			MakeTemporaryIOIdent(ioo);
			SendInitScriptEvent(ioo);
			ioo->show = SHOW_FLAG_IN_INVENTORY;
			
			if(multi) {
				if(ioo->ioflags & IO_GOLD) {
					ioo->_itemdata->price = count;
				} else {
					ioo->_itemdata->maxcount = 9999;
					ARX_CHECK_SHORT(count);
					ioo->_itemdata->count = std::max(static_cast<short>(count), (short)1);
				}
			}
			
			long xx, yy;
			if(!CanBePutInSecondaryInventory(context.getIO()->inventory, ioo, &xx, &yy)) {
					PutInFrontOfPlayer(ioo);
			}
			
			return Success;
		}
		
	};
	
	class DestroyCommand : public SubCommand {
		
	public:
		
		DestroyCommand() : SubCommand("destroy") { }
		
		Result execute(Context & context) {
			
			DebugScript("");
			
			INTERACTIVE_OBJ * io = context.getIO();
			if(io->inventory) {
				if(SecondaryInventory == io->inventory) {
					SecondaryInventory = NULL;
				}
				free(io->inventory), io->inventory = NULL;
			}
			
			return Success;
		}
		
	};
	
	class OpenCommand : public SubCommand {
		
	public:
		
		OpenCommand() : SubCommand("open") { }
		
		Result execute(Context & context) {
			
			DebugScript("");
			
			if(SecondaryInventory != context.getIO()->inventory) {
				SecondaryInventory = context.getIO()->inventory;
				ARX_SOUND_PlayInterface(SND_BACKPACK);
			}
			
			return Success;
		}
		
	};
	
	class CloseCommand : public SubCommand {
		
	public:
		
		CloseCommand() : SubCommand("close") { }
		
		Result execute(Context & context) {
			
			DebugScript("");
			
			if(context.getIO()->inventory != NULL) {
				SecondaryInventory = NULL;
				ARX_SOUND_PlayInterface(SND_BACKPACK);
			}
			
			return Success;
		}
		
	};
	
public:
	
	InventoryCommand() : Command("inventory", ANY_IO) {
		addCommand(new CreateCommand);
		addCommand(new SkinCommand);
		addCommand(new PlayerAddFromSceneCommand);
		addCommand(new PlayerAddCommand("playeradd", false));
		addCommand(new PlayerAddCommand("playeraddmulti", true));
		addCommand(new AddFromSceneCommand);
		addCommand(new AddCommand("add", false));
		addCommand(new AddCommand("addmulti", true));
		addCommand(new DestroyCommand);
		addCommand(new OpenCommand);
		addCommand(new CloseCommand);
	}
	
	Result execute(Context & context) {
		
		string cmdname = context.getLowercase();
		
		// Remove all underscores from the command.
		cmdname.resize(std::remove(cmdname.begin(), cmdname.end(), '_') - cmdname.begin());
		
		Commands::const_iterator it = commands.find(cmdname);
		if(it == commands.end()) {
			ScriptWarning << "unknown inventory command: " << cmdname;
			return Failed;
		}
		
		return it->second->execute(context);
	}
	
};

class ObjectHideCommand : public Command {
	
public:
	
	ObjectHideCommand() : Command("objecthide") { }
	
	Result execute(Context & context) {
		
		bool megahide = false;
		HandleFlags("m") {
			megahide = (flg & flag('m'));
		}
		
		string target = context.getLowercase();
		long t = GetTargetByNameTarget(target);
		if(t == -2) {
			t = GetInterNum(context.getIO());
		}
		
		bool hide = context.getBool();
		
		DebugScript(' ' << options << ' ' << target << ' ' << hide);
		
		if(!ValidIONum(t)) {
			ScriptWarning << "unknown target: " << target;
			return Failed;
		}
		
		INTERACTIVE_OBJ * io = inter.iobj[t];
		io->GameFlags &= ~GFLAG_MEGAHIDE;
		if(hide) {
			if(megahide) {
				io->GameFlags |= GFLAG_MEGAHIDE;
				io->show = SHOW_FLAG_MEGAHIDE;
			} else {
				io->show = SHOW_FLAG_HIDDEN;
			}
		} else if(io->show == SHOW_FLAG_MEGAHIDE || io->show == SHOW_FLAG_HIDDEN) {
			io->show = SHOW_FLAG_IN_SCENE;
			if((io->ioflags & IO_NPC) && io->_npcdata->life <= 0.f) {
				inter.iobj[t]->animlayer[0].cur_anim = inter.iobj[t]->anims[ANIM_DIE];
				inter.iobj[t]->animlayer[1].cur_anim = NULL;
				inter.iobj[t]->animlayer[2].cur_anim = NULL;
				inter.iobj[t]->animlayer[0].ctime = 9999999;
			}
		}
		
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

class TeleportCommand : public Command {
	
public:
	
	TeleportCommand() : Command("teleport") { }
	
	Result execute(Context & context) {
		
		TELEPORT_TO_CONFIRM = 1;
		
		bool teleport_player = false, initpos = false;
		HandleFlags("alnpi") {
			
			long angle = -1;
			
			if(flg & flag('a')) {
				float fangle = context.getFloat();
				angle = static_cast<long>(fangle);
				if(!flg & flag('l')) {
					player.desiredangle.b = player.angle.b = fangle;
				}
			}
			
			if(flg & flag('n')) {
				TELEPORT_TO_CONFIRM = 0;
			}
			
			if(flg & flag('l')) {
				
				string level = context.getLowercase();
				string target = context.getLowercase();
				
				strcpy(TELEPORT_TO_LEVEL, level.c_str());
				strcpy(TELEPORT_TO_POSITION, target.c_str());
				
				if(angle == -1) {
					TELEPORT_TO_ANGLE	=	static_cast<long>(player.angle.b);
				} else {
					TELEPORT_TO_ANGLE = angle;
				}
				
				CHANGE_LEVEL_ICON = 1;
				if(!TELEPORT_TO_CONFIRM) {
					CHANGE_LEVEL_ICON = 200;
				}
				
				DebugScript(' ' << options << ' ' << level << ' ' << target);
				
				return Success;
			}
			
			teleport_player = (flg & flag('p'));
			initpos = (flg & flag('i'));
		}
		
		string target;
		if(!initpos) {
			target = context.getLowercase();
		}
		
		DebugScript(' ' << options << ' ' << target);
		
		if(target == "behind") {
			TELEPORT_TO_CONFIRM = 0;
			ARX_INTERACTIVE_TeleportBehindTarget(context.getIO());
			return Success;
		}
		
#ifdef BUILD_EDITOR
		if(!GAME_EDITOR) {
#endif
			TELEPORT_TO_CONFIRM = 0;
#ifdef BUILD_EDITOR
		}
#endif
		
		INTERACTIVE_OBJ * io = context.getIO();
		if(!teleport_player && !io) {
			LogWarning << "must either use -p or use in IO context";
			return Failed;
		}
		
		if(!initpos) {
			
			long t = GetTargetByNameTarget(target);
			if(t == -2) {
				t = GetInterNum(context.getIO());
			}
			arx_assert(t != -3);
			if(!ValidIONum(t)) {
				ScriptWarning << "unknown target: " << target;
				return Failed;
			}
			
			Vec3f pos;
			if(!GetItemWorldPosition(inter.iobj[t], &pos)) {
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
				LogWarning << "must be in IO context to teleport -i";
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

class TweakCommand : public Command {
	
public:
	
	TweakCommand() : Command("tweak", ANY_IO) { }
	
	Result execute(Context & context) {
		
		INTERACTIVE_OBJ * io = context.getIO();
		
		string type = context.getLowercase();
		
		if(type == "skin") {
			
			string oldskin = loadPath(context.getLowercase());
			string newskin = loadPath(context.getLowercase());
			
			DebugScript(" skin \"" << oldskin << "\" \"" << newskin << '"');
			
			ARX_INTERACTIVE_MEMO_TWEAK(io, TWEAK_TYPE_SKIN, oldskin, newskin);
			EERIE_MESH_TWEAK_Skin(io->obj, oldskin, newskin);
			
		} else if(type == "icon") {
			
			string icon = loadPath(context.getLowercase());
			
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
			
			string mesh = loadPath(context.getLowercase());
			
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
		
		string mesh = loadPath(context.getLowercase());
		
		DebugScript(" \"" << mesh << '"');
		
		ARX_INTERACTIVE_MEMO_TWEAK(context.getIO(), TWEAK_TYPE_MESH, mesh, string());
		ARX_INTERACTIVE_USEMESH(context.getIO(), mesh);
		
		return Success;
	}
	
};

class MoveCommand : public Command {
	
public:
	
	MoveCommand() : Command("move", ANY_IO) { }
	
	Result execute(Context & context) {
		
		float dx = context.getFloat();
		float dy = context.getFloat();
		float dz = context.getFloat();
		
		DebugScript(' ' << dx << ' ' << dy << ' ' << dz);
		
		context.getIO()->pos += Vec3f(dx, dy, dz);
		
		return Success;
	}
	
};

class DestroyCommand : public Command {
	
public:
	
	DestroyCommand() : Command("destroy") { }
	
	Result execute(Context & context) {
		
		string target = toLowercase(context.getStringVar(context.getLowercase()));
		
		DebugScript(' ' << target);
		
		long t = GetTargetByNameTarget(target);
		if(t == -2) {
			t = GetInterNum(context.getIO()); //self
		}
		if(!ValidIONum(t)) {
			ScriptWarning << "unknown target: " << target;
			return Failed;
		}
		
		bool self = (inter.iobj[t] == context.getIO());
		
		ARX_INTERACTIVE_DestroyIO(inter.iobj[t]);
		
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
		
		string target = context.getLowercase();
		
		float damage = context.getFloat();
		
		DebugScript(' ' << type << ' ' << target);
		
		long t = GetTargetByNameTarget(target);
		if(t == -2) {
			t = GetInterNum(context.getIO()); //self
		}
		if(!ValidIONum(t)) {
			ScriptWarning << "unknown target: " << target;
			return Failed;
		}
		
		ARX_DAMAGES_DealDamages(t, damage, GetInterNum(context.getIO()), type, &inter.iobj[t]->pos);
		
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
		
		ARX_CHECK_SHORT(damages);
		io->damager_damages = static_cast<short>(damages);
		
		return Success;
	}
	
};

}

void setupScriptedInteractiveObject() {
	
	ScriptEvent::registerCommand(new ReplaceMeCommand);
	ScriptEvent::registerCommand(new RotateCommand);
	ScriptEvent::registerCommand(new CollisionCommand("collision"));
	ScriptEvent::registerCommand(new CollisionCommand("collison")); // TODO(broken-scripts)
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
	ScriptEvent::registerCommand(new SpawnCommand);
	ScriptEvent::registerCommand(new SetInteractivityCommand);
	ScriptEvent::registerCommand(new SetStepMaterialCommand);
	ScriptEvent::registerCommand(new SetArmorMaterialCommand);
	ScriptEvent::registerCommand(new SetWeaponMaterialCommand);
	ScriptEvent::registerCommand(new SetStrikeSpeechCommand);
	ScriptEvent::registerCommand(new SetCollisionCommand("setplayercollision", 1));
	ScriptEvent::registerCommand(new SetCollisionCommand("setworldcollision", 2));
	ScriptEvent::registerCommand(new SetWeightCommand);
	ScriptEvent::registerCommand(new SetTransparencyCommand);
	ScriptEvent::registerCommand(new SetIRColorCommand);
	ScriptEvent::registerCommand(new SetScaleCommand);
	ScriptEvent::registerCommand(new KillMeCommand);
	ScriptEvent::registerCommand(new ForceAnimCommand);
	ScriptEvent::registerCommand(new ForceAngleCommand);
	ScriptEvent::registerCommand(new PlayAnimCommand);
	ScriptEvent::registerCommand(new PhysicalCommand);
	ScriptEvent::registerCommand(new LoadAnimCommand);
	ScriptEvent::registerCommand(new LinkObjToMeCommand);
	ScriptEvent::registerCommand(new IfExistInternalCommand);
	ScriptEvent::registerCommand(new IfVisibleCommand);
	ScriptEvent::registerCommand(new InventoryCommand);
	ScriptEvent::registerCommand(new ObjectHideCommand);
	ScriptEvent::registerCommand(new HaloCommand);
	ScriptEvent::registerCommand(new TeleportCommand);
	ScriptEvent::registerCommand(new TargetPlayerPosCommand);
	ScriptEvent::registerCommand(new TweakCommand);
	ScriptEvent::registerCommand(new UseMeshCommand);
	ScriptEvent::registerCommand(new MoveCommand);
	ScriptEvent::registerCommand(new DestroyCommand);
	ScriptEvent::registerCommand(new DoDamageCommand);
	ScriptEvent::registerCommand(new DamagerCommand);
	
}

} // namespace script
