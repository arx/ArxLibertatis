
#include "scripting/ScriptedPlayer.h"

#include "game/Player.h"
#include "io/Logger.h"
#include "scene/GameSound.h"
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
		
		ARX_PLAYER_Modify_XP(static_cast<long>(val));
		
		LogDebug << "addxp " << val;
		
		return ACCEPT;
	}
	
	~AddXpCommand() { }
	
};

class AddGoldCommand : public ScriptCommand {
	
public:
	
	ScriptResult execute(ScriptContext & context) {
		
		float val = context.getFloat();
		
		if(val != 0) {
			ARX_SOUND_PlayInterface(SND_GOLD);
		}
		
		ARX_CHECK_LONG(val);
		ARX_PLAYER_AddGold(static_cast<long>(val));
		
		LogDebug << "addgold " << val;
		
		return ACCEPT;
	}
	
	~AddGoldCommand() { }
	
};

}

void setupScriptedPlayer() {
	
	ScriptEvent::registerCommand("addbag", new AddBagCommand);
	ScriptEvent::registerCommand("addxp", new AddXpCommand);
	ScriptEvent::registerCommand("addgold", new AddGoldCommand);
	
}
