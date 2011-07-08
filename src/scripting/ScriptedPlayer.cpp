
#include "scripting/ScriptedNPC.h"

#include "game/Player.h"
#include "io/Logger.h"
#include "scripting/ScriptEvent.h"

using std::string;

namespace {

class AddBagCommand : public ScriptCommand {
	
public:
	
	ScriptResult execute(ScriptContext & context) {
		
		ARX_UNUSED(context);
		
		ARX_PLAYER_AddBag();
		
		LogDebug << "addbag";
		
		return ACCEPT;
	}
	
	~AddBagCommand() { }
	
};

}

void setupScriptedPlayer() {
	
	ScriptEvent::registerCommand("addbag", new AddBagCommand);
	
}
