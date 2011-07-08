
#include "scripting/ScriptedControl.h"

#include "physics/Attractors.h"
#include "io/Logger.h"
#include "platform/String.h"
#include "scene/Interactive.h"
#include "scripting/ScriptEvent.h"

using std::string;

namespace {

class AttractorCommand : public ScriptCommand {
	
public:
	
	ScriptResult execute(ScriptContext & context) {
		
		string target = context.getLowercase();
		
		long t = GetTargetByNameTarget(target);
		if(t == -2) {
			t = GetInterNum(context.getIO());
		}
		
		string power = context.getLowercase();
		
		float val = 0.f;
		float radius = 0.f;
		
		if(power != "off") {
			val = context.getFloatVar(power);
			radius = context.getFloat();
		}
		
		ARX_SPECIAL_ATTRACTORS_Add(t, val, radius);
		
		LogDebug << "attractor \"" << target << "\" " << val << ' ' << radius;
		
		
		return ACCEPT;
	}
	
	~AttractorCommand() { }
	
};

}

void setupScriptedControl() {
	
	ScriptEvent::registerCommand("attractor", new AttractorCommand);
	
}
