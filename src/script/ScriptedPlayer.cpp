
#include "script/ScriptedPlayer.h"

#include "game/Player.h"
#include "io/Logger.h"
#include "scene/GameSound.h"
#include "script/ScriptEvent.h"

using std::string;

namespace script {

namespace {

class AddBagCommand : public Command {
	
public:
	
	ScriptResult execute(Context & context) {
		
		ARX_UNUSED(context);
		
		ARX_PLAYER_AddBag();
		
		LogDebug << "addbag";
		
		return ACCEPT;
	}
	
	~AddBagCommand() { }
	
};

class AddXpCommand : public Command {
	
public:
	
	ScriptResult execute(Context & context) {
		
		float val = context.getFloat();
		
		ARX_PLAYER_Modify_XP(static_cast<long>(val));
		
		LogDebug << "addxp " << val;
		
		return ACCEPT;
	}
	
	~AddXpCommand() { }
	
};

class AddGoldCommand : public Command {
	
public:
	
	ScriptResult execute(Context & context) {
		
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

class RidiculousCommand : public Command {
	
public:
	
	ScriptResult execute(Context & context) {
		
		ARX_UNUSED(context);
		
		ARX_PLAYER_MakeFreshHero();
		
		LogDebug << "ridiculous";
		
		return ACCEPT;
	}
	
	~RidiculousCommand() { }
	
};

}

void setupScriptedPlayer() {
	
	ScriptEvent::registerCommand("addbag", new AddBagCommand);
	ScriptEvent::registerCommand("addxp", new AddXpCommand);
	ScriptEvent::registerCommand("addgold", new AddGoldCommand);
	ScriptEvent::registerCommand("ridiculous", new RidiculousCommand);
	
}

} // namespace script
