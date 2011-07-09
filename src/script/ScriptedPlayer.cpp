
#include "script/ScriptedPlayer.h"

#include "game/Player.h"
#include "io/Logger.h"
#include "scene/GameSound.h"
#include "script/ScriptEvent.h"
#include "script/ScriptUtils.h"

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

class RuneCommand : public Command {
	
	typedef std::map<string, RuneFlag> Runes;
	Runes runes;
	
public:
	
	RuneCommand() {
		runes["aam"] = FLAG_AAM;
		runes["cetrius"] = FLAG_CETRIUS;
		runes["comunicatum"] = FLAG_COMUNICATUM;
		runes["cosum"] = FLAG_COSUM;
		runes["folgora"] = FLAG_FOLGORA;
		runes["fridd"] = FLAG_FRIDD;
		runes["kaom"] = FLAG_KAOM;
		runes["mega"] = FLAG_MEGA;
		runes["morte"] = FLAG_MORTE;
		runes["movis"] = FLAG_MOVIS;
		runes["nhi"] = FLAG_NHI;
		runes["rhaa"] = FLAG_RHAA;
		runes["spacium"] = FLAG_SPACIUM;
		runes["stregum"] = FLAG_STREGUM;
		runes["taar"] = FLAG_TAAR;
		runes["tempus"] = FLAG_TEMPUS;
		runes["tera"] = FLAG_TERA;
		runes["vista"] = FLAG_VISTA;
		runes["vitae"] = FLAG_VITAE;
		runes["yok"] = FLAG_YOK;
	}
	
	Result execute(Context & context) {
		
		string options = context.getFlags();
		
		long add = 0;
		if(!options.empty()) {
			u64 flg = flags(options);
			if(flg & flag('a')) {
				add = 1;
			}
			if(flg & flag('r')) {
				add = -1;
			}
			if(!flg || (flg & ~flags("ar"))) {
				LogWarning << "unexpected flags: rotate " << options;
			}
		}
		
		string name = context.getLowercase();
		
		LogDebug << "rune " << options << ' ' << name;
		
		if(name == "all") {
			
			if(add != 0) {
				LogWarning << "unexpected flags: rotate " << options << " all";
				return Failed;
			}
			
			ARX_PLAYER_Rune_Add_All();
			
		} else {
			
			if(add == 0) {
				LogWarning << "missing flags:  rotate " << options << ' ' << name << "; expected -a or -r";
				return Failed;
			}
			
			Runes::const_iterator it = runes.find(name);
			if(it == runes.end()) {
				LogWarning << "unknown rune name: rune " << options << ' ' << name;
				return Failed;
			}
			
			if(add == 1) {
				ARX_Player_Rune_Add(it->second);
			} else if(add == -1) {
				ARX_Player_Rune_Remove(it->second);
			}
		}
		
		return Success;
	}
	
	~RuneCommand() { }
	
};

class QuestCommand : public Command {
	
public:
	
	Result execute(Context & context) {
		
		string name = loadUnlocalized(context.getLowercase());
		
		LogDebug << "quest " << name;
		
		ARX_PLAYER_Quest_Add(name);
		
		return Success;
	}
	
	~QuestCommand() { }
	
};

}

void setupScriptedPlayer() {
	
	ScriptEvent::registerCommand("addbag", new AddBagCommand);
	ScriptEvent::registerCommand("addxp", new AddXpCommand);
	ScriptEvent::registerCommand("addgold", new AddGoldCommand);
	ScriptEvent::registerCommand("ridiculous", new RidiculousCommand);
	ScriptEvent::registerCommand("rune", new RuneCommand);
	ScriptEvent::registerCommand("quest", new QuestCommand);
	
}

} // namespace script
