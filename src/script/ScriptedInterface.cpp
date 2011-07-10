
#include "script/ScriptedInterface.h"

#include "game/Player.h"
#include "game/Inventory.h"
#include "gui/Interface.h"
#include "io/Logger.h"
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
		
		string options = context.getFlags();
		
		string command = context.getLowercase();
		
		if(!options.empty()) {
			u64 flg = flags(options);
			if(flg & flag('a')) { // Magic
				Book_Mode = BOOKMODE_MINIMAP;
			}
			if(flg & flag('e')) { // Equip
				Book_Mode = BOOKMODE_SPELLS;
			}
			if(flg & flag('m')) { // Map
				Book_Mode = BOOKMODE_QUESTS;
			}
			if(!flg || (flg & ~flags("aem"))) {
				LogWarning << "unexpected flags: book " << options;
			}
		}
		
		if(command == "open") {
			ARX_INTERFACE_BookOpenClose(1);
		} else if(command == "close") {
			ARX_INTERFACE_BookOpenClose(2);
		} else {
			LogWarning << "unexpected command: book " << options << " \"" << command << "\"";
		}
		
		LogDebug << "book " << options << " \"" << command << "\"";
		
		return Success;
	}
	
	~BookCommand() { }
	
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
	
	~CloseStealBagCommand() { }
	
};

class NoteCommand : public Command {
	
public:
	
	NoteCommand() : Command("note") { }
	
	Result execute(Context & context) {
		
		ARX_INTERFACE_NOTE_TYPE type = NOTE_TYPE_UNDEFINED;
		string tpname = context.getLowercase();
		if(tpname == "note") {
			type = NOTE_TYPE_NOTE;
		} else if(tpname == "notice") {
			type = NOTE_TYPE_NOTICE;
		} else if(tpname == "book") {
			type = NOTE_TYPE_BOOK;
		}
		
		string text = loadUnlocalized(context.getLowercase());
		
		LogDebug << "note " << tpname << ' ' << text;
		
		ARX_INTERFACE_NoteOpen(type, text);
		
		return Success;
	}
	
	~NoteCommand() { }
	
};

}

void setupScriptedInterface() {
	
	ScriptEvent::registerCommand(new BookCommand);
	ScriptEvent::registerCommand(new CloseStealBagCommand);
	ScriptEvent::registerCommand(new NoteCommand);
	
}

} // namespace script
