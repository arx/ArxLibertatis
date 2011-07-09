
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

}

void setupScriptedItem() {
	
	ScriptEvent::registerCommand("repair", new RepairCommand);
	
}

} // namespace script
