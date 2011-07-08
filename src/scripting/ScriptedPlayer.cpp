
#include "scripting/ScriptedPlayer.h"

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

class AddXpCommand : public ScriptCommand {
	
public:
	
	ScriptResult execute(ScriptContext & context) {
		
		float val = context.getFloat();
		
		ARX_PLAYER_Modify_XP((long)val);
		
		LogDebug << "addxp " << val;
		
		return ACCEPT;
	}
	
	~AddXpCommand() { }
	
};

}

void setupScriptedPlayer() {
	
	ScriptEvent::registerCommand("addbag", new AddBagCommand);
	ScriptEvent::registerCommand("addxp", new AddXpCommand);
	
}
