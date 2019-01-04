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

#include "script/ScriptedItem.h"

#include "game/EntityManager.h"
#include "game/Equipment.h"
#include "game/Inventory.h"
#include "game/Item.h"
#include "game/Player.h"
#include "graphics/Math.h"
#include "gui/Console.h"
#include "gui/Credits.h"
#include "gui/Menu.h"
#include "scene/Interactive.h"
#include "script/ScriptEvent.h"
#include "script/ScriptUtils.h"


namespace script {

namespace {

class RepairCommand : public Command {
	
public:
	
	RepairCommand() : Command("repair") { }
	
	Result execute(Context & context) {
		
		std::string target = context.getWord();
		Entity * t = entities.getById(target, context.getEntity());
		
		float val = glm::clamp(context.getFloat(), 0.f, 100.f);
		
		if(t != NULL) {
			ARX_DAMAGES_DurabilityRestore(t, val);
		}
		
		DebugScript(' ' << target << ' ' << val);
		
		return Success;
	}
	
};

class SetPoisonousCommand : public Command {
	
public:
	
	SetPoisonousCommand() : Command("setpoisonous", AnyEntity) { }
	
	Result execute(Context & context) {
		
		float poisonous = context.getFloat();
		float poisonous_count = context.getFloat();
		
		DebugScript(' ' << poisonous << ' ' << poisonous_count);
		
		Entity * io = context.getEntity();
		if(poisonous_count == 0) {
			io->poisonous_count = 0;
		} else {
			io->poisonous = checked_range_cast<short>(poisonous);
			io->poisonous_count = checked_range_cast<short>(poisonous_count);
		}
		
		return Success;
	}
	
};

class SetStealCommand : public Command {
	
public:
	
	SetStealCommand() : Command("setsteal", IO_ITEM) { }
	
	Result execute(Context & context) {
		
		std::string stealvalue = context.getWord();
		
		DebugScript(' ' << stealvalue);
		
		Entity * io = context.getEntity();
		if(stealvalue == "off") {
			io->_itemdata->stealvalue = -1;
		} else {
			io->_itemdata->stealvalue = glm::clamp(int(context.getFloatVar(stealvalue)), -1, 100);
			if(io->_itemdata->stealvalue == 100) {
				io->_itemdata->stealvalue = -1;
			}
		}
		
		return Success;
	}
	
};

class SetLightCommand : public Command {
	
public:
	
	SetLightCommand() : Command("setlight", IO_ITEM) { }
	
	Result execute(Context & context) {
		
		std::string lightvalue = context.getWord();
		
		DebugScript(' ' << lightvalue);
		
		if(lightvalue == "off") {
			context.getEntity()->_itemdata->LightValue = -1;
		} else {
			context.getEntity()->_itemdata->LightValue = glm::clamp(int(context.getFloatVar(lightvalue)), -1, 1);
		}
		
		return Success;
	}
	
};

class SetFoodCommand : public Command {
	
public:
	
	SetFoodCommand() : Command("setfood", IO_ITEM) { }
	
	Result execute(Context & context) {
		
		float food_value = context.getFloat();
		
		DebugScript(' ' << food_value);
		
		context.getEntity()->_itemdata->food_value = char(food_value);
		
		return Success;
	}
	
};

class SetObjectTypeCommand : public Command {
	
public:
	
	SetObjectTypeCommand() : Command("setobjecttype", AnyEntity) { }
	
	Result execute(Context & context) {
		
		bool set = true;
		HandleFlags("r") {
			set = !(flg & flag('r'));
		}
		
		std::string type = context.getWord();
		
		DebugScript(' ' << type << ' ' << set);
		
		if(!ARX_EQUIPMENT_SetObjectType(*context.getEntity(), type, set)) {
			ScriptWarning << "unknown object type: " << type;
			return Failed;
		}
		
		return Success;
	}
	
};

class SetEquipCommand : public Command {
	
public:
	
	SetEquipCommand() : Command("setequip", IO_ITEM) { }
	
	Result execute(Context & context) {
		
		bool special = false;
		HandleFlags("rs") {
			if(flg & flag('r')) {
				ARX_EQUIPMENT_Remove_All_Special(context.getEntity());
			}
			if(flg & flag('s')) {
				special = true;
			}
		}
		
		std::string modifierName = context.getWord();
		std::string val = context.getWord();
		
		EquipmentModifierFlags flag = 0;
		if(!val.empty() && val[val.length() - 1] == '%') {
			flag |= IO_ELEMENT_FLAG_PERCENT;
		}
		float fval = context.getFloatVar(val);
		
		DebugScript(' ' << options << ' ' << modifierName << ' ' << fval << ' ' << flag);
		
		ARX_EQUIPMENT_SetEquip(context.getEntity(), special, modifierName, fval, flag);
		
		return Success;
	}
	
};

class SetDurabilityCommand : public Command {
	
public:
	
	SetDurabilityCommand() : Command("setdurability", AnyEntity) { }
	
	Result execute(Context & context) {
		
		Entity * io = context.getEntity();
		if(io->ioflags & IO_NPC) {
			ScriptWarning << "cannot set durability on NPCs";
			return Failed;
		}
		
		bool current = false;
		HandleFlags("c") {
			current = test_flag(flg, 'c');
		}
		
		float durability = context.getFloat();
		
		DebugScript(' ' << options << ' ' << durability);
		
		io->durability = durability;
		if(!current) {
			io->max_durability = durability;
		}
		
		return Success;
	}
	
};

class SetMaxCountCommand : public Command {
	
public:
	
	SetMaxCountCommand() : Command("setmaxcount", IO_ITEM) { }
	
	Result execute(Context & context) {
		
		short count = std::max(short(context.getFloat()), short(1));
		
		DebugScript(' ' << count);
		
		context.getEntity()->_itemdata->maxcount = count;
		
		return Success;
	}
	
};

class SetCountCommand : public Command {
	
public:
	
	SetCountCommand() : Command("setcount", IO_ITEM) { }
	
	Result execute(Context & context) {
		
		short count = glm::clamp(short(context.getFloat()), short(1), context.getEntity()->_itemdata->maxcount);
		
		DebugScript(' ' << count);
		
		context.getEntity()->_itemdata->count = count;
		
		return Success;
	}
	
};

class SetPriceCommand : public Command {
	
public:
	
	SetPriceCommand() : Command("setprice", IO_ITEM) { }
	
	Result execute(Context & context) {
		
		long price = std::max(long(context.getFloat()), 0l);
		
		DebugScript(' ' << price);
		
		context.getEntity()->_itemdata->price = price;
		
		return Success;
	}
	
};

class PlayerStackSizeCommand : public Command {
	
public:
	
	PlayerStackSizeCommand() : Command("playerstacksize", IO_ITEM) { }
	
	Result execute(Context & context) {
		
		short size = short(glm::clamp(int(context.getFloat()), 1, 100));
		
		DebugScript(' ' << size);
		
		context.getEntity()->_itemdata->playerstacksize = size;
		
		return Success;
	}
	
};

class EatMeCommand : public Command {
	
public:
	
	EatMeCommand() : Command("eatme", AnyEntity) { }
	
	Result execute(Context & context) {
		
		DebugScript("");
		
		Entity * entity = context.getEntity();
		
		if(entity->ioflags & IO_ITEM) {
			player.hunger += entity->_itemdata->food_value * 4.f;
			player.hunger = std::min(player.hunger, 100.f);
		}
		
		if(entity == entities.player()) {
			// The player entity must not be destroyed!
			g_console.close();
			ARX_MENU_Launch(g_canResumeGame);
			ARX_MENU_Clicked_CREDITS();
			credits::setMessage("You have been eaten by a Grue!");
		} else {
			ARX_INTERACTIVE_DestroyIOdelayed(entity);
		}
		
		return Success;
	}
	
};

} // anonymous namespace

void setupScriptedItem() {
	
	ScriptEvent::registerCommand(new RepairCommand);
	ScriptEvent::registerCommand(new SetPoisonousCommand);
	ScriptEvent::registerCommand(new SetStealCommand);
	ScriptEvent::registerCommand(new SetLightCommand);
	ScriptEvent::registerCommand(new SetFoodCommand);
	ScriptEvent::registerCommand(new SetObjectTypeCommand);
	ScriptEvent::registerCommand(new SetEquipCommand);
	ScriptEvent::registerCommand(new SetDurabilityCommand);
	ScriptEvent::registerCommand(new SetMaxCountCommand);
	ScriptEvent::registerCommand(new SetCountCommand);
	ScriptEvent::registerCommand(new SetPriceCommand);
	ScriptEvent::registerCommand(new PlayerStackSizeCommand);
	ScriptEvent::registerCommand(new EatMeCommand);
	
}

} // namespace script
