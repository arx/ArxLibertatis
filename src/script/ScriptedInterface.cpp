
#include "script/ScriptedInterface.h"

#include "gui/Interface.h"
#include "io/Logger.h"
#include "script/ScriptEvent.h"
#include "script/ScriptUtils.h"

using std::string;

namespace script {

namespace {

class BookCommand : public Command {
	
public:
	
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

}

void setupScriptedInterface() {
	
	ScriptEvent::registerCommand("book", new BookCommand);
	
}

} // namespace script
