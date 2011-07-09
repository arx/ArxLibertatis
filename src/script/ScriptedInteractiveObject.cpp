
#include "script/ScriptedInteractiveObject.h"

#include "scene/Interactive.h"
#include "io/Logger.h"
#include "script/ScriptEvent.h"

using std::string;

namespace script {

namespace {

class ActivatePhysicsCommand : public Command {
	
public:
	
	ScriptResult execute(Context & context) {
		
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

} // namespace script
