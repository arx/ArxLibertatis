
#include "script/ScriptedItem.h"

#include "graphics/Math.h"
#include "io/Logger.h"
#include "platform/String.h"
#include "scene/Interactive.h"
#include "script/ScriptEvent.h"
#include "script/ScriptUtils.h"

using std::string;

namespace script {

namespace {

class RepairCommand : public Command {
	
public:
	
	Result execute(Context & context) {
		
		string target = context.getLowercase();
		long t = GetTargetByNameTarget(target);
		if(t == -2) {
			t = GetInterNum(context.getIO()); //self
		}
		
		float val = clamp(context.getFloat(), 0.f, 100.f);
		
		if(ValidIONum(t)) {
			ARX_DAMAGES_DurabilityRestore(inter.iobj[t], val);
		}
		
		LogDebug << "repair " << target << ' ' << val;
		
		return Success;
	}
	
	~RepairCommand() { }
	
};

class SetPoisonousCommand : public Command {
	
public:
	
	Result execute(Context & context) {
		
		float poisonous = context.getFloat();
		float poisonous_count = context.getFloat();
		
		LogDebug << "setpoisonous " << poisonous << ' ' << poisonous_count;
		
		INTERACTIVE_OBJ * io = context.getIO();
		if(!io) {
			return Failed;
		}
		
		if(poisonous_count == 0) {
			io->poisonous_count = 0;
		} else {
			ARX_CHECK_SHORT(poisonous);
			ARX_CHECK_SHORT(poisonous_count);
			io->poisonous = static_cast<short>(poisonous);
			io->poisonous_count = static_cast<short>(poisonous_count);
		}
		
		return Success;
	}
	
	~SetPoisonousCommand() { }
	
};

}

void setupScriptedItem() {
	
	ScriptEvent::registerCommand("repair", new RepairCommand);
	ScriptEvent::registerCommand("setpoisonous", new SetPoisonousCommand);
	
}

} // namespace script
