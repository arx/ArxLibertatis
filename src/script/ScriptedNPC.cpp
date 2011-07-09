
#include "script/ScriptedNPC.h"

#include "game/NPC.h"
#include "graphics/data/Mesh.h"
#include "io/Logger.h"
#include "platform/String.h"
#include "script/ScriptEvent.h"
#include "script/ScriptUtils.h"

using std::string;

namespace script {

namespace {

class BehaviourCommand : public Command {
	
public:
	
	ScriptResult execute(Context & context) {
		
		string options = context.getFlags();
		
		string command = context.getLowercase();
		
		INTERACTIVE_OBJ * io = context.getIO();
		
		if(options.empty()) {
			if(command == "stack") {
				LogDebug << "behavior " << options << ' ' << command; 
				ARX_NPC_Behaviour_Stack(io);
				return ACCEPT;
			} else if(command == "unstack") {
				LogDebug << "behavior " << options << ' ' << command; 
				ARX_NPC_Behaviour_UnStack(io);
				return ACCEPT;
			} else if(command == "unstackall") {
				LogDebug << "behavior " << options << ' ' << command; 
				ARX_NPC_Behaviour_Reset(io);
				return ACCEPT;
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
		
		return ACCEPT;
	}
	
	~BehaviourCommand() { }
	
};

}

void setupScriptedNPC() {
	
	ScriptEvent::registerCommand("behavior", new BehaviourCommand);
	
}

} // namespace script
