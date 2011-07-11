
#include "script/ScriptedInteractiveObject.h"

#include "core/GameTime.h"
#include "game/Inventory.h"
#include "game/Player.h"
#include "game/Missile.h"
#include "game/NPC.h"
#include "graphics/Math.h"
#include "graphics/data/Mesh.h"
#include "gui/Interface.h"
#include "io/Logger.h"
#include "io/FilePath.h"
#include "physics/Collisions.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"
#include "script/ScriptUtils.h"

using std::string;

extern INTERACTIVE_OBJ * LASTSPAWNED;

namespace script {

namespace {

class ReplaceMeCommand : public Command {
	
public:
	
	ReplaceMeCommand() : Command("replaceme", ANY_IO) { }
	
	Result execute(Context & context) {
		
		string object = context.getLowercase();
		
		LogDebug << "replaceme \"" << object << '"';
		
		INTERACTIVE_OBJ * io = context.getIO();
		
		string tex2;
		if(io->ioflags & IO_NPC) {
			tex2 = "Graph\\Obj3D\\Interactive\\NPC\\" + object + ".teo";
		} else if(io->ioflags & IO_FIX) {
			tex2 = "Graph\\Obj3D\\Interactive\\FIX_INTER\\" + object + ".teo";
		} else {
			tex2 = "Graph\\Obj3D\\Interactive\\Items\\" + object + ".teo";
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
		
		io->angle.a += t1;
		io->angle.b += t2;
		io->angle.g += t3;
		
		if((size_t)io->nb_lastanimvertex != io->obj->vertexlist.size()) {
			free(io->lastanimvertex);
			io->lastanimvertex = NULL;
		}
		io->lastanimtime = 0;
		
		LogDebug << "rotate " << t1 << ' ' << t2 << ' ' << t3;
		
		return Success;
	}
	
};

class CollisionCommand : public Command {
	
public:
	
	CollisionCommand(const string & command) : Command(command, ANY_IO) { }
	
	Result execute(Context & context) {
		
		bool choice = context.getBool();
		
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
		
		LogDebug << "shopcategory " << category;
		
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
		
		LogDebug << "shopmultiply " << multiply;
		
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
		
		LogDebug << getName() << ' ' << enable;
		
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
		
		LogDebug << getName() << ' ' << enable;
		
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
		
		LogDebug << "settrap " << trapvalue;
		
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
		
		LogDebug << "setsecret " << secretvalue;
		
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
		
		LogDebug << "setmaterial " << name;
		
		Materials::const_iterator it = materials.find(name);
		if(it == materials.end()) {
			LogWarning << "unknown material: setmaterial " << name;
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
		
		LogDebug << "setname " << name;
		
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
				LogWarning << "unknown target: spawn npc " << file << ' ' << target;
				return Failed;
			}
			
			LogDebug << "spawn npc " << file << ' ' << target;
			
			if(FORBID_SCRIPT_IO_CREATION) {
				return Failed;
			}
			
			string path;
			File_Standardize("graph\\obj3d\\interactive\\npc\\" + file, path);
			
			if(type == "npc") {
				
				INTERACTIVE_OBJ * ioo = AddNPC(path, IO_IMMEDIATELOAD);
				if(!ioo) {
					LogWarning << "failed to create npc " << path;
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
					LogWarning << "failed to create item " << path;
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
				LogWarning << "must be npc to spawn fireballs";
				return  Failed;
			}
			
			GetTargetPos(io);
			Vec3f pos = io->pos;
			
			if(io->ioflags & IO_NPC) {
				pos.y -= 80.f;
			}
			
			ARX_MISSILES_Spawn(io, MISSILE_FIREBALL, &pos, &io->target);
			
		} else {
			LogWarning << "unexpected spawn type: " << type;
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
		
		LogDebug << "setstepmaterial " << material;
		
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
		
		LogDebug << "setarmormaterial " << material;
		
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
		
		LogDebug << "setweaponmaterial " << material;
		
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
		
		LogDebug << "setstrikespeech " << speech;
		
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
		
		LogDebug << getName() << ' ' << enable;
		
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
		
		LogDebug << "setweight " << weight;
		
		context.getIO()->weight = weight;
		
		return Success;
	}
	
};

class SetTransparencyCommand : public Command {
	
public:
	
	SetTransparencyCommand() : Command("settransparency", ANY_IO) { }
	
	Result execute(Context & context) {
		
		float trans = context.getFloat();
		
		LogDebug << "settransparency " << trans;
		
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
		
		LogDebug << "setircolor " << r << ' ' << g << ' ' << b;
		
		context.getIO()->infracolor = Color3f(r, g, b);
		
		return Success;
	}
	
};

class SetScaleCommand : public Command {
	
public:
	
	SetScaleCommand() : Command("setscale", ANY_IO) { }
	
	Result execute(Context & context) {
		
		float scale = context.getFloat();
		
		LogDebug << "setscale " << scale;
		
		context.getIO()->scale = scale * 0.01f;
		
		return Success;
	}
	
};

class KillMeCommand : public Command {
	
public:
	
	KillMeCommand() : Command("killme", ANY_IO) { }
	
	Result execute(Context & context) {
		
		LogDebug << "killme";
		
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
		
		LogDebug << "forceanim " << anim;
		
		AnimationNumber num = GetNumAnim(anim);
		if(num == ANIM_NONE) {
			LogWarning << "forceanim: unknown animation: " << anim;
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
		
		LogDebug << "forceangle " << angle;
		
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
		
		string options = context.getFlags();
		if(!options.empty()) {
			u64 flg = flags(options);
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
			if(!flg || (flg & ~flags("123lnep"))) {
				LogWarning << "unexpected flags: playanim " << options;
			}
		}
		
		string anim = context.getLowercase();
		
		LogDebug << "playanim " << options << ' ' << anim;
		
		if(!iot) {
			LogWarning << "playanim: must either use -p or use with IO";
			return Failed;
		}
		
		if(anim == "none") {
			iot->animlayer[nu].cur_anim = NULL;
			iot->animlayer[nu].next_anim = NULL;
			return Success;
		}
		
		AnimationNumber num = GetNumAnim(anim);
		if(num == ANIM_NONE) {
			LogWarning << "playanim: unknown anim: " << anim;
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
				LogError << "no free timer";
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
			LogDebug << "physical on";
			
		} else if(type == "off") {
			io->ioflags |= IO_PHYSICAL_OFF;
			LogDebug << "physical off";
			
		} else {
			
			float fval = context.getFloat();
			
			LogDebug << "physical " << type << ' ' << fval;
			
			if(type == "height") {
				io->original_height = clamp(-fval, -165.f, -30.f);
				io->physics.cyl.height = io->original_height * io->scale;
			} else if(type == "radius") {
				io->original_radius = clamp(fval, 10.f, 40.f);
				io->physics.cyl.radius = io->original_radius * io->scale;
			} else {
				LogWarning << "physical: unknown command: " << type;
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
		
		string options = context.getFlags();
		if(!options.empty()) {
			u64 flg = flags(options);
			if(flg & flag('p')) {
				iot = inter.iobj[0];
			}
			if(!flg || (flg & ~flag('p'))) {
				LogWarning << "unexpected flags: playanim " << options;
			}
		}
		
		string anim = context.getLowercase();
		
		string file = loadPath(context.getWord());
		
		LogDebug << "loadanim " << options << ' ' << anim << ' ' << file;
		
		if(!iot) {
			LogWarning << "playanim: must either use -p or use with IO";
			return Failed;
		}
		
		AnimationNumber num = GetNumAnim(anim);
		if(num == ANIM_NONE) {
			LogWarning << "loadanim: unknown anim: " << anim;
			return Failed;
		}
		
		if(iot->anims[num]) {
			ReleaseAnimFromIO(iot, num);
		}
		
		if(iot == inter.iobj[0] || (iot->ioflags & IO_NPC)) {
			file = "Graph\\Obj3D\\Anims\\npc\\" + file;
		} else {
			file = "Graph\\Obj3D\\Anims\\Fix_Inter\\" + file;
		}
		SetExt(file, ".tea");
		string path;
		File_Standardize(file, path);
		
		iot->anims[num] = EERIE_ANIMMANAGER_Load(path);
		
		if(!iot->anims[num]) {
			LogWarning << "loadanim: file not found: " << path;
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
		
		LogDebug << "linkobjtome " << name << ' ' << attach;
		
		long t = GetTargetByNameTarget(name);
		if(!ValidIONum(t)) {
			LogWarning << "linkobjtome: unknown target: " << name;
			return Failed;
		}
		
		LinkObjToMe(context.getIO(), inter.iobj[t], attach);
		
		return Success;
	}
	
};

}

void setupScriptedInteractiveObject() {
	
	ScriptEvent::registerCommand(new ReplaceMeCommand);
	ScriptEvent::registerCommand(new RotateCommand);
	ScriptEvent::registerCommand(new CollisionCommand("collision"));
	ScriptEvent::registerCommand(new CollisionCommand("collison"));
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
	
}

} // namespace script
