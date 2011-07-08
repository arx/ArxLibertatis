
#include "scripting/ScriptedNPC.h"

#include "game/NPC.h"
#include "graphics/data/Mesh.h"
#include "io/Logger.h"
#include "platform/String.h"
#include "scripting/ScriptEvent.h"

using std::string;

class BehaviourCommand : public ScriptCommand {
	
public:
	
	ScriptResult execute(ScriptContext & context) {
		
		string flags = context.getFlags();
		
		string command = context.getLowercase();
		
		INTERACTIVE_OBJ * io = context.getIO();
		
		if(flags.empty()) {
			if(command == "stack") {
				LogDebug << "behavior " << flags << ' ' << command; 
				ARX_NPC_Behaviour_Stack(io);
				return ACCEPT;
			} else if(command == "unstack") {
				LogDebug << "behavior " << flags << ' ' << command; 
				ARX_NPC_Behaviour_UnStack(io);
				return ACCEPT;
			} else if(command == "unstackall") {
				LogDebug << "behavior " << flags << ' ' << command; 
				ARX_NPC_Behaviour_Reset(io);
				return ACCEPT;
			}
		}
		
		Behaviour behavior = 0;
		
		if(!flags.empty()) {
			if(CharIn(flags, 'l')) {
				behavior |= BEHAVIOUR_LOOK_AROUND;
			}
			if(CharIn(flags, 's')) {
				behavior |= BEHAVIOUR_SNEAK;
			}
			if(CharIn(flags, 'd')) {
				behavior |= BEHAVIOUR_DISTANT;
			}
			if(CharIn(flags, 'm')) {
				behavior |= BEHAVIOUR_MAGIC;
			}
			if(CharIn(flags, 'f')) {
				behavior |= BEHAVIOUR_FIGHT;
			}
			if(CharIn(flags, 'a')) {
				behavior |= BEHAVIOUR_STARE_AT;
			}
			if(io && (io->ioflags & IO_NPC)) {
				if(CharIn(flags, '0')) {
					io->_npcdata->tactics = 0;
				}
				if(CharIn(flags, '1')) {
					io->_npcdata->tactics = 0;
				}
				if(CharIn(flags, '2')) {
					io->_npcdata->tactics = 0;
				}
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
		}
		
		LogDebug << "behavior " << flags << " \"" << command << "\" " << behavior_param; 
		
		if(io && (io->ioflags & IO_NPC)) {
			ARX_CHECK_LONG(behavior_param);
			ARX_NPC_Behaviour_Change(io, behavior, static_cast<long>(behavior_param));
		}
		
		return ACCEPT;
	}
	
	~BehaviourCommand() { }
	
};

void setupScriptedNPC() {
	
	ScriptEvent::registerCommand("behavior", new BehaviourCommand);
	
}
