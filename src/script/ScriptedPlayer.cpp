
#include "script/ScriptedPlayer.h"

#include "game/Player.h"
#include "game/Inventory.h"
#include "graphics/data/Mesh.h"
#include "gui/Interface.h"
#include "io/Logger.h"
#include "io/FilePath.h"
#include "scene/Interactive.h"
#include "scene/GameSound.h"
#include "script/ScriptEvent.h"
#include "script/ScriptUtils.h"

using std::string;

extern float InventoryDir;

namespace script {

namespace {

class AddBagCommand : public Command {
	
public:
	
	AddBagCommand() : Command("addbag") { }
	
	Result execute(Context & context) {
		
		ARX_UNUSED(context);
		
		ARX_PLAYER_AddBag();
		
		LogDebug << "addbag";
		
		return Success;
	}
	
};

class AddXpCommand : public Command {
	
public:
	
	AddXpCommand() : Command("addxp") { }
	
	Result execute(Context & context) {
		
		float val = context.getFloat();
		
		ARX_PLAYER_Modify_XP(static_cast<long>(val));
		
		LogDebug << "addxp " << val;
		
		return Success;
	}
	
};

class AddGoldCommand : public Command {
	
public:
	
	AddGoldCommand() : Command("addgold") { }
	
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
	
};

class RidiculousCommand : public Command {
	
public:
	
	RidiculousCommand() : Command("ridiculous") { }
	
	Result execute(Context & context) {
		
		ARX_UNUSED(context);
		
		ARX_PLAYER_MakeFreshHero();
		
		LogDebug << "ridiculous";
		
		return Success;
	}
	
};

class RuneCommand : public Command {
	
	typedef std::map<string, RuneFlag> Runes;
	Runes runes;
	
public:
	
	RuneCommand() : Command("rune") {
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
	
};

class QuestCommand : public Command {
	
public:
	
	QuestCommand() : Command("quest") { }
	
	Result execute(Context & context) {
		
		string name = loadUnlocalized(context.getLowercase());
		
		LogDebug << "quest " << name;
		
		ARX_PLAYER_Quest_Add(name);
		
		return Success;
	}
	
};

class SetPlayerTweakCommand : public Command {
	
public:
	
	SetPlayerTweakCommand() : Command("setplayertweak", ANY_IO) { }
	
	Result execute(Context & context) {
		
		string command = context.getLowercase();
		
		INTERACTIVE_OBJ * io = context.getIO();
		if(!io->tweakerinfo) {
			io->tweakerinfo = (IO_TWEAKER_INFO *)malloc(sizeof(IO_TWEAKER_INFO));
			if(!(io->tweakerinfo)) {
				return Failed;
			}
			memset(io->tweakerinfo, 0, sizeof(IO_TWEAKER_INFO));
		}
		
		if(command == "skin") {
			
			string src = loadPath(context.getWord());
			string dst = loadPath(context.getWord());
			
			LogDebug << "setplayertweak skin " << src << ' ' << dst;
			
			strcpy(io->tweakerinfo->skintochange, src.c_str());
			strcpy(io->tweakerinfo->skinchangeto, dst.c_str());
			
		} else {
			
			string mesh = loadPath(context.getWord());
			
			LogDebug << "setplayertweak mesh " << mesh;
			
			strcpy(io->tweakerinfo->filename, mesh.c_str());
		}
		
		return Success;
	}
	
};

class SetHungerCommand : public Command {
	
public:
	
	SetHungerCommand() : Command("sethunger") { }
	
	Result execute(Context & context) {
		
		player.hunger = context.getFloat();
		
		LogDebug << "sethunger " << player.hunger;
		
		return Success;
	}
	
};

class SetPlayerControlsCommand : public Command {
	
	static void Stack_SendMsgToAllNPC_IO(ScriptMessage msg, const char * dat) {
		for(long i = 0; i < inter.nbmax; i++) {
			if(inter.iobj[i] && (inter.iobj[i]->ioflags & IO_NPC)) {
				Stack_SendIOScriptEvent(inter.iobj[i], msg, dat);
			}
		}
	}
	
public:
	
	SetPlayerControlsCommand() : Command("setplayercontrols") { }
	
	Result execute(Context & context) {
		
		INTERACTIVE_OBJ * oes = EVENT_SENDER;
		EVENT_SENDER = context.getIO();
		
		bool enable = context.getBool();
		
		LogDebug << "setplayercontrols " << enable;
		
		if(enable) {
			if(BLOCK_PLAYER_CONTROLS) {
				Stack_SendMsgToAllNPC_IO(SM_CONTROLS_ON, "");
			}
			BLOCK_PLAYER_CONTROLS = 0;
		} else {
			if(!BLOCK_PLAYER_CONTROLS) {
				ARX_PLAYER_PutPlayerInNormalStance(0);
				Stack_SendMsgToAllNPC_IO(SM_CONTROLS_OFF, "");
				ARX_SPELLS_FizzleAllSpellsFromCaster(0);
			}
			BLOCK_PLAYER_CONTROLS = 1;
			player.Interface &= ~INTER_COMBATMODE;
		}
		
		EVENT_SENDER = oes;
		
		return Success;
	}
	
};

class StealNPCCommand : public Command {
	
public:
	
	StealNPCCommand() : Command("stealnpc") { }
	
	Result execute(Context & context) {
		
		LogDebug << "stealnpc";
		
		if(player.Interface & INTER_STEAL) {
			SendIOScriptEvent(ioSteal, SM_STEAL, "OFF");
		}
		
		player.Interface |= INTER_STEAL;
		InventoryDir = 1;
		ioSteal = context.getIO();
		
		return Success;
	}
	
};

}

void setupScriptedPlayer() {
	
	ScriptEvent::registerCommand(new AddBagCommand);
	ScriptEvent::registerCommand(new AddXpCommand);
	ScriptEvent::registerCommand(new AddGoldCommand);
	ScriptEvent::registerCommand(new RidiculousCommand);
	ScriptEvent::registerCommand(new RuneCommand);
	ScriptEvent::registerCommand(new QuestCommand);
	ScriptEvent::registerCommand(new SetPlayerTweakCommand);
	ScriptEvent::registerCommand(new SetHungerCommand);
	ScriptEvent::registerCommand(new SetPlayerControlsCommand);
	ScriptEvent::registerCommand(new StealNPCCommand);
	
}

} // namespace script
