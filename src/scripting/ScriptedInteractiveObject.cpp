
#include "scripting/ScriptedInteractiveObject.h"

#include "scene/Interactive.h"
#include "io/Logger.h"
#include "scripting/ScriptEvent.h"

using std::string;

namespace {

class ActivatePhysicsCommand : public ScriptCommand {
	
public:
	
	ScriptResult execute(ScriptContext & context) {
		
		ARX_INTERACTIVE_ActivatePhysics(GetInterNum(context.getIO()));
		
		LogDebug << "activatephysics";
		
		return ACCEPT;
	}
	
	~ActivatePhysicsCommand() { }
	
};

}

void setupScriptedInteractiveObject() {
	
	ScriptEvent::registerCommand("activatephysics", new ActivatePhysicsCommand);
	
}
