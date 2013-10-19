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

#include "script/ScriptedInterface.h"

#include "game/Inventory.h"
#include "game/Entity.h"
#include "game/Player.h"
#include "gui/Interface.h"
#include "gui/Menu.h"
#include "gui/MiniMap.h"
#include "scene/GameSound.h"
#include "script/ScriptEvent.h"
#include "script/ScriptUtils.h"

using std::string;

extern float InventoryDir;

namespace script {

namespace {

class BookCommand : public Command {
	
public:
	
	BookCommand() : Command("book") { }
	
	Result execute(Context & context) {
		
		HandleFlags("aem") {
			if(flg & flag('a')) { // Magic
				Book_Mode = BOOKMODE_MINIMAP;
			}
			if(flg & flag('e')) { // Equip
				Book_Mode = BOOKMODE_SPELLS;
			}
			if(flg & flag('m')) { // Map
				Book_Mode = BOOKMODE_QUESTS;
			}
		}
		
		string command = context.getWord();
		
		if(command == "open") {
			ARX_INTERFACE_BookOpenClose(1);
		} else if(command == "close") {
			ARX_INTERFACE_BookOpenClose(2);
		} else if(command == "change") {
			// Nothing to do, mode already changed by flags.
		} else {
			ScriptWarning << "unexpected command: " << options << " \"" << command << "\"";
		}
		
		DebugScript(' ' << options << ' ' << command);
		
		return Success;
	}
	
};

class CloseStealBagCommand : public Command {
	
public:
	
	CloseStealBagCommand() : Command("closestealbag", IO_NPC) { }
	
	Result execute(Context & context) {
		
		ARX_UNUSED(context);
		
		if(!(player.Interface & INTER_STEAL)) {
			return Success;
		}
		
		Entity * pio = (SecondaryInventory) ? SecondaryInventory->io : ioSteal;
		if(pio && pio == ioSteal) {
			InventoryDir = -1;
			SendIOScriptEvent(pio, SM_INVENTORY2_CLOSE);
			TSecondaryInventory = SecondaryInventory;
			SecondaryInventory = NULL;
		}
		
		return Success;
	}
	
};

class NoteCommand : public Command {
	
public:
	
	NoteCommand() : Command("note") { }
	
	Result execute(Context & context) {
		
		gui::Note::Type type;
		string tpname = context.getWord();
		if(tpname == "note") {
			type = gui::Note::SmallNote;
		} else if(tpname == "notice") {
			type = gui::Note::Notice;
		} else if(tpname == "book") {
			type = gui::Note::Book;
		} else {
			ScriptWarning << "unexpected note type: " << tpname;
			type = gui::Note::SmallNote;
		}
		
		string text = loadUnlocalized(context.getWord());
		
		DebugScript(' ' << tpname << ' ' << text);
		
		ARX_INTERFACE_NoteOpen(type, text);
		
		return Success;
	}
	
};

class ShowGlobalsCommand : public Command {
	
public:
	
	ShowGlobalsCommand() : Command("showglobals") { }
	
	Result execute(Context & context) {
		
		ARX_UNUSED(context);
		
		DebugScript("");
		
		string text;
		MakeGlobalText(text);
		LogInfo << "Global vars:\n" << text;
		
		return Success;
	}
	
};

class ShowLocalsCommand : public Command {
	
public:
	
	ShowLocalsCommand() : Command("showlocals") { }
	
	Result execute(Context & context) {
		
		DebugScript("");
		
		string text;
		MakeLocalText(context.getScript(), text);
		LogInfo << "Local vars:\n" << text;
		
		return Success;
	}
	
};

class ShowVarsCommand : public Command {
	
public:
	
	ShowVarsCommand() : Command("showvars") { }
	
	Result execute(Context & context) {
		
		DebugScript("");
		
		string text;
		MakeGlobalText(text);
		text += "--------------------------\n";
		MakeLocalText(context.getScript(), text);
		LogInfo << "Vars:\n" << text;
		
		return Success;
	}
	
};

class PlayerInterfaceCommand : public Command {
	
public:
	
	PlayerInterfaceCommand() : Command("playerinterface") { }
	
	Result execute(Context & context) {
		
		bool smooth = false;
		HandleFlags("s") {
			smooth = test_flag(flg, 's');
		}
		
		string command = context.getWord();
		
		DebugScript(' ' << options << ' ' << command);
		
		if(command == "hide") {
			ARX_INTERFACE_PlayerInterfaceModify(0, smooth);
		} else if(command == "show") {
			ARX_INTERFACE_PlayerInterfaceModify(1, smooth);
		} else {
			ScriptWarning << "unknown command: " << command;
			return Failed;
		}
		
		return Success;
	}
	
};

class PopupCommand : public Command {
	
public:
	
	PopupCommand() : Command("popup") { }
	
	Result execute(Context & context) {
		
		string message = context.getWord();
		
		DebugScript(' ' << message);
		
		return Success;
	}
	
};

class EndIntroCommand : public Command {
	
public:
	
	EndIntroCommand() : Command("endintro") { }
	
	Result execute(Context & context) {
		
		ARX_UNUSED(context);
		
		DebugScript("");
		
		ARX_INTERFACE_EndIntro();
		
		return Success;
	}
	
};

class EndGameCommand : public Command {
	
public:
	
	EndGameCommand() : Command("endgame") { }
	
	Result execute(Context & context) {
		
		ARX_UNUSED(context);
		
		DebugScript("");
		
		ARX_SOUND_MixerStop(ARX_SOUND_MixerGame);
		ARX_MENU_Launch(false);
		ARX_MENU_Clicked_CREDITS();
		
		return Success;
	}
	
};

class MapMarkerCommand : public Command {
	
public:
	
	MapMarkerCommand() : Command("mapmarker") { }
	
	Result execute(Context & context) {
		
		bool remove = false;
		HandleFlags("r") {
			remove = test_flag(flg, 'r');
		}
		
		if(remove) {
			
			string marker = loadUnlocalized(context.getWord());
			
			DebugScript(' ' << options << ' ' << marker);
			
			g_miniMap.mapMarkerRemove(marker);
			
		} else {
			
			float x = context.getFloat();
			float y = context.getFloat();
			long level = (long)context.getFloat();
			
			string marker = loadUnlocalized(context.getWord());
			
			DebugScript(' ' << options << ' ' << x << ' ' << y << ' ' << level << ' ' << marker);
			
			g_miniMap.mapMarkerAdd(x, y, level, marker);
			
		}
		
		return Success;
	}
	
};

class DrawSymbolCommand : public Command {
	
public:
	
	DrawSymbolCommand() : Command("drawsymbol", AnyEntity) { }
	
	Result execute(Context & context) {
		
		string symbol = context.getWord();
		
		float duration = context.getFloat();
		
		DebugScript(' ' << symbol << ' ' << duration);
		
		ARX_SPELLS_RequestSymbolDraw(context.getEntity(), symbol, duration);
		
		return Success;
	}
	
};

}

void setupScriptedInterface() {
	
	ScriptEvent::registerCommand(new BookCommand);
	ScriptEvent::registerCommand(new CloseStealBagCommand);
	ScriptEvent::registerCommand(new NoteCommand);
	ScriptEvent::registerCommand(new ShowGlobalsCommand);
	ScriptEvent::registerCommand(new ShowLocalsCommand);
	ScriptEvent::registerCommand(new ShowVarsCommand);
	ScriptEvent::registerCommand(new PlayerInterfaceCommand);
	ScriptEvent::registerCommand(new PopupCommand);
	ScriptEvent::registerCommand(new EndIntroCommand);
	ScriptEvent::registerCommand(new EndGameCommand);
	ScriptEvent::registerCommand(new MapMarkerCommand);
	ScriptEvent::registerCommand(new DrawSymbolCommand);
	
}

} // namespace script
