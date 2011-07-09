
#include "script/ScriptedNPC.h"

#include "game/NPC.h"
#include "graphics/Math.h"
#include "graphics/data/Mesh.h"
#include "io/Logger.h"
#include "platform/String.h"
#include "scene/Interactive.h"
#include "script/ScriptEvent.h"
#include "script/ScriptUtils.h"

using std::string;

namespace script {

namespace {

class BehaviourCommand : public Command {
	
public:
	
	Result execute(Context & context) {
		
		string options = context.getFlags();
		
		string command = context.getLowercase();
		
		INTERACTIVE_OBJ * io = context.getIO();
		
		if(options.empty()) {
			if(command == "stack") {
				LogDebug << "behavior " << options << ' ' << command; 
				ARX_NPC_Behaviour_Stack(io);
				return Success;
			} else if(command == "unstack") {
				LogDebug << "behavior " << options << ' ' << command; 
				ARX_NPC_Behaviour_UnStack(io);
				return Success;
			} else if(command == "unstackall") {
				LogDebug << "behavior " << options << ' ' << command; 
				ARX_NPC_Behaviour_Reset(io);
				return Success;
			}
		}
		
		Behaviour behavior = 0;
		
		if(!options.empty()) {
			u64 flg = flags(options);
			if(flg & flag('l')) {
				behavior |= BEHAVIOUR_LOOK_AROUND;
			}
			if(flg & flag('s')) {
				behavior |= BEHAVIOUR_SNEAK;
			}
			if(flg & flag('d')) {
				behavior |= BEHAVIOUR_DISTANT;
			}
			if(flg & flag('m')) {
				behavior |= BEHAVIOUR_MAGIC;
			}
			if(flg & flag('f')) {
				behavior |= BEHAVIOUR_FIGHT;
			}
			if(flg & flag('a')) {
				behavior |= BEHAVIOUR_STARE_AT;
			}
			if(io && (io->ioflags & IO_NPC)) {
				if(flg & flags("012")) {
					io->_npcdata->tactics = 0;
				}
			}
			if(!flg || (flg & ~flags("lsdmfa012"))) {
				LogWarning << "unexpected flags: behavior " << options;
			}
		}
		
		float behavior_param = 0.f;
		if(command == "go_home") {
			behavior |= BEHAVIOUR_GO_HOME;
		} else if(command == "friendly") {
			if(io && (io->ioflags & IO_NPC)) {
				io->_npcdata->movemode = NOMOVEMODE;
			}
			behavior |= BEHAVIOUR_FRIENDLY;
		} else if(command == "move_to") {
			if(io && (io->ioflags & IO_NPC)) {
				io->_npcdata->movemode = WALKMODE;
			}
			behavior |= BEHAVIOUR_MOVE_TO;
		} else if(command == "flee") {
			behavior_param = context.getFloat();
			if(io && (io->ioflags & IO_NPC)) {
				io->_npcdata->movemode = RUNMODE;
			}
			behavior |= BEHAVIOUR_FLEE;
		} else if(command == "look_for") {
			behavior_param = context.getFloat();
			if(io && (io->ioflags & IO_NPC)) {
				io->_npcdata->movemode = WALKMODE;
			}
			behavior |= BEHAVIOUR_LOOK_FOR;
		} else if(command == "hide") {
			behavior_param = context.getFloat();
			if(io && (io->ioflags & IO_NPC)) {
				io->_npcdata->movemode = WALKMODE;
			}
			behavior |= BEHAVIOUR_HIDE;
		} else if(command == "wander_around") {
			behavior_param = context.getFloat();
			if(io && (io->ioflags & IO_NPC)) {
				io->_npcdata->movemode = WALKMODE;
			}
			behavior |= BEHAVIOUR_WANDER_AROUND;
		} else if(command == "guard") {
			behavior |= BEHAVIOUR_GUARD;
			if(io) {
				io->targetinfo = -2;
				if(io->ioflags & IO_NPC) {
					io->_npcdata->movemode = NOMOVEMODE;
				}
			}
		} else {
			LogWarning << "unexpected command: behavior " << options << " \"" << command << '"';
		}
		
		LogDebug << "behavior " << options << " \"" << command << "\" " << behavior_param;
		
		if(io && (io->ioflags & IO_NPC)) {
			ARX_CHECK_LONG(behavior_param);
			ARX_NPC_Behaviour_Change(io, behavior, static_cast<long>(behavior_param));
		}
		
		return Success;
	}
	
	~BehaviourCommand() { }
	
};

class ReviveCommand : public Command {
	
public:
	
	Result execute(Context & context) {
		
		string options = context.getFlags();
		
		bool init = false;
		
		if(!options.empty()) {
			u64 flg = flags(options);
			if(flg & flag('i')) {
				init = true;
			} else if(!flg || (flg & ~flag('i'))) {
				LogWarning << "unexpected flags: revive " << options;
			}
		}
		
		ARX_NPC_Revive(context.getIO(), init ? 1 : 0);
		
		LogDebug << "revive " << options;
		
		return Success;
	}
	
	~ReviveCommand() { }
	
};

class SpellcastCommand : public Command {
	
public:
	
	Result execute(Context & context) {
		
		SpellcastFlags spflags = 0;
		long duration = -1;
		bool haveDuration = 0;
		
		string options = context.getFlags();
		if(!options.empty()) {
			u64 flg = flags(options);
			
			if(!flg || (flg & ~flags("kdxmsfz"))) {
				LogWarning << "unexpected flags: spellcast " << options;
			}
			
			if(flg & flag('k')) {
				
				string spellname = context.getLowercase();
				Spell spellid = GetSpellId(spellname);
				
				LogDebug << "spellcast " << options << ' ' << spellname;
				
				long from = GetInterNum(context.getIO());
				if(ValidIONum(from)) {
					long sp = ARX_SPELLS_GetInstanceForThisCaster(spellid, from);
					if(sp >= 0) {
						spells[sp].tolive = 0;
					}
				}
				
				return Success;
			}
			
			if(flg & flag('d')) {
				spflags |= SPELLCAST_FLAG_NOCHECKCANCAST;
				duration = context.getFloat();
				if(duration <= 0) {
					duration = 99999999; // TODO should this be FLT_MAX?
				}
				haveDuration = 1;
			}
			if(flg & flag('x')) {
				spflags |= SPELLCAST_FLAG_NOSOUND;
			}
			if(flg & flag('m')) {
				spflags |= SPELLCAST_FLAG_NOCHECKCANCAST | SPELLCAST_FLAG_NODRAW;
			}
			if(flg & flag('s')) {
				spflags |= SPELLCAST_FLAG_NOCHECKCANCAST | SPELLCAST_FLAG_NOANIM;
			}
			if(flg & flag('f')) {
				spflags |= SPELLCAST_FLAG_NOCHECKCANCAST | SPELLCAST_FLAG_NOMANA;
			}
			if(flg & flag('z')) {
				spflags |= SPELLCAST_FLAG_RESTORE;
			}
		}
		
		long level = clamp(static_cast<long>(context.getFloat()), 1l, 10l);
		if(!haveDuration) {
			duration = 1000 + level * 2000;
		}
		
		string spellname = context.getLowercase();
		Spell spellid = GetSpellId(spellname);
		
		string target = context.getLowercase();
		long t = GetTargetByNameTarget(target);
		if(t <= -1) {
			t = GetInterNum(context.getIO());
		}
		
		if(!ValidIONum(t) || spellid == SPELL_NONE) {
			return Failed;
		}
		
		if(context.getIO() != inter.iobj[0]) {
			spflags |= SPELLCAST_FLAG_NOCHECKCANCAST;
		}
		
		LogDebug << "spellcast " << spellname << ' ' << level << ' ' << target << ' ' << spflags << ' ' << duration; 
		
		TryToCastSpell(context.getIO(), spellid, level, t, spflags, duration);
		
		return Success;
	}
	
	~SpellcastCommand() { }
	
};

}

void setupScriptedNPC() {
	
	ScriptEvent::registerCommand("behavior", new BehaviourCommand);
	ScriptEvent::registerCommand("revive", new ReviveCommand);
	ScriptEvent::registerCommand("spellcast", new SpellcastCommand);
	
}

} // namespace script
