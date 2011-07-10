
#include "script/ScriptedInteractiveObject.h"

#include "game/Inventory.h"
#include "game/Player.h"
#include "graphics/data/Mesh.h"
#include "gui/Interface.h"
#include "io/Logger.h"
#include "io/FilePath.h"
#include "physics/Collisions.h"
#include "scene/Interactive.h"
#include "script/ScriptEvent.h"

using std::string;

extern INTERACTIVE_OBJ * LASTSPAWNED;

namespace script {

namespace {

class ReplaceMeCommand : public Command {
	
public:
	
	Result execute(Context & context) {
		
		string object = context.getLowercase();
		
		LogDebug << "replaceme \"" << object << '"';
		
		INTERACTIVE_OBJ * io = context.getIO();
		if(!io) {
			LogDebug << "replaceme called for non-io";
			return Failed;
		}
		
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
	
	~ReplaceMeCommand() { }
	
};

class RotateCommand : public Command {
	
public:
	
	Result execute(Context & context) {
		
		INTERACTIVE_OBJ * io = context.getIO();
		if(!io) {
			LogDebug << "rotate called without IO";
			return Failed;
		}
		
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
	
	~RotateCommand() { }
	
};

class CollisionCommand : public Command {
	
public:
	
	Result execute(Context & context) {
		
		bool choice = context.getBool();
		
		INTERACTIVE_OBJ * io = context.getIO();
		if(!io) {
			LogWarning << "collision script command executed without IO";
			return Failed;
		}
		
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
	
	~CollisionCommand() { }
	
};

class ShopCategoryCommand : public Command {
	
public:
	
	Result execute(Context & context) {
		
		string category = context.getLowercase();
		
		INTERACTIVE_OBJ * io = context.getIO();
		if(!io) {
			return Failed;
		}
		
		if(io->shop_category) {
			free(io->shop_category);
		}
		io->shop_category = strdup(category.c_str());
		
		return Success;
	}
	
	~ShopCategoryCommand() { }
	
};

class ShopMultiplyCommand : public Command {
	
public:
	
	Result execute(Context & context) {
		
		float multiply = context.getFloat();
		
		INTERACTIVE_OBJ * io = context.getIO();
		if(!io) {
			return Failed;
		}
		
		io->shop_multiply = multiply;
		
		return Success;
	}
	
	~ShopMultiplyCommand() { }
	
};

}

void setupScriptedInteractiveObject() {
	
	ScriptEvent::registerCommand("replaceme", new ReplaceMeCommand);
	ScriptEvent::registerCommand("rotate", new RotateCommand);
	ScriptEvent::registerCommand("collision", new CollisionCommand);
	ScriptEvent::registerCommand("shopcategory", new ShopCategoryCommand);
	ScriptEvent::registerCommand("shopmultiply", new ShopMultiplyCommand);
	
}

} // namespace script
