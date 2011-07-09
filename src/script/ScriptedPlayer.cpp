
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
	
	Result execute(Context & context) {
		
		ARX_UNUSED(context);
		
		ARX_PLAYER_AddBag();
		
		LogDebug << "addbag";
		
		return Success;
	}
	
	~AddBagCommand() { }
	
};

class AddXpCommand : public Command {
	
public:
	
	Result execute(Context & context) {
		
		float val = context.getFloat();
		
		ARX_PLAYER_Modify_XP(static_cast<long>(val));
		
		LogDebug << "addxp " << val;
		
		return Success;
	}
	
	~AddXpCommand() { }
	
};

class AddGoldCommand : public Command {
	
public:
	
	Result execute(Context & context) {
		
		float val = context.getFloat();
		
		if(val != 0) {
			ARX_SOUND_PlayInterface(SND_GOLD);
		}
		
		ARX_CHECK_LONG(val);
		ARX_PLAYER_AddGold(static_cast<long>(val));
		
		LogDebug << "addgold " << val;
		
		return Success;
	}
	
	~AddGoldCommand() { }
	
};

class RidiculousCommand : public Command {
	
public:
	
	Result execute(Context & context) {
		
		ARX_UNUSED(context);
		
		ARX_PLAYER_MakeFreshHero();
		
		LogDebug << "ridiculous";
		
		return Success;
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
