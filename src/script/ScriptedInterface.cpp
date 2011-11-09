
#include "script/ScriptedInterface.h"

#include "game/Player.h"
#include "game/Inventory.h"
#include "gui/Interface.h"
#include "gui/Menu.h"
#include "gui/MiniMap.h"
#include "io/PakReader.h"
#include "scene/GameSound.h"
#include "script/ScriptEvent.h"
#include "script/ScriptUtils.h"

using std::string;

extern float InventoryDir;
extern long REFUSE_GAME_RETURN;

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
		
		INTERACTIVE_OBJ * pio = (SecondaryInventory) ? SecondaryInventory->io : ioSteal;
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
		
		ARX_INTERFACE_NOTE_TYPE type = NOTE_TYPE_UNDEFINED;
		string tpname = context.getWord();
		if(tpname == "note") {
			type = NOTE_TYPE_NOTE;
		} else if(tpname == "notice") {
			type = NOTE_TYPE_NOTICE;
		} else if(tpname == "book") {
			type = NOTE_TYPE_BOOK;
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
		LogInfo << "global vars:\n" << text;
		
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
		LogInfo << "local vars:\n" << text;
		
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
		LogInfo << "vars:\n" << text;
		
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
		
		REFUSE_GAME_RETURN = 1;
		
		ARX_SOUND_MixerStop(ARX_SOUND_MixerGame);
		ARX_MENU_Launch();
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
			
			ARX_MAPMARKER_Remove(marker);
			
		} else {
			
			float x = context.getFloat();
			float y = context.getFloat();
			long level = (long)context.getFloat();
			
			string marker = loadUnlocalized(context.getWord());
			
			DebugScript(' ' << options << ' ' << x << ' ' << y << ' ' << level << ' ' << marker);
			
			ARX_MAPMARKER_Add(x, y, level, marker);
			
		}
		
		return Success;
	}
	
};

class DrawSymbolCommand : public Command {
	
public:
	
	DrawSymbolCommand() : Command("drawsymbol", ANY_IO) { }
	
	Result execute(Context & context) {
		
		string symbol = context.getWord();
		
		float duration = context.getFloat();
		
		DebugScript(' ' << symbol << ' ' << duration);
		
		ARX_SPELLS_RequestSymbolDraw(context.getIO(), symbol, duration);
		
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
