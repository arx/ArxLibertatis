
#include "scripting/ScriptedNPC.h"

#include "gui/Interface.h"
#include "io/Logger.h"
#include "scripting/ScriptEvent.h"

using std::string;

namespace {

class BookCommand : public ScriptCommand {
	
public:
	
	ScriptResult execute(ScriptContext & context) {
		
		string flags = context.getFlags();
		
		string command = context.getLowercase();
		
		if(!flags.empty()) {
			if(CharIn(flags, 'a')) { // Magic
				Book_Mode = BOOKMODE_MINIMAP;
			}
			if(CharIn(flags, 'e')) { // Equip
				Book_Mode = BOOKMODE_SPELLS;
			}
			if(CharIn(flags, 'm')) { // Map
				Book_Mode = BOOKMODE_QUESTS;
			}
		}
		
		if(command == "open") {
			ARX_INTERFACE_BookOpenClose(1);
		} else if(command == "close") {
			ARX_INTERFACE_BookOpenClose(2);
		}
		
		LogDebug << "book " << flags << " \"" << command << "\"";
		
		return ACCEPT;
	}
	
	~BookCommand() { }
	
};

}

void setupScriptedInterface() {
	
	ScriptEvent::registerCommand("book", new BookCommand);
	
}
