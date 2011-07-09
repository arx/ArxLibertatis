
#include "script/ScriptedControl.h"

#include "io/Logger.h"
#include "platform/String.h"
#include "script/ScriptEvent.h"
#include "script/ScriptUtils.h"

using std::string;

extern INTERACTIVE_OBJ * CAMERACONTROLLER;

namespace script {

namespace {

class CameraControlCommand : public Command {
	
public:
	
	Result execute(Context & context) {
		
		bool enable = context.getBool();
		
		LogDebug << "cameracontrol " << enable;
		
		CAMERACONTROLLER = enable ? context.getIO() : NULL;
		
		return Success;
	}
	
	~CameraControlCommand() {}
	
};

}

void setupScriptedCamera() {
	
	ScriptEvent::registerCommand("cameracontrol", new CameraControlCommand);
	
}

} // namespace script
