
#include "script/ScriptedControl.h"

#include "ai/Paths.h"
#include "graphics/data/Mesh.h"
#include "io/Logger.h"
#include "platform/String.h"
#include "scene/Interactive.h"
#include "script/ScriptEvent.h"
#include "script/ScriptUtils.h"

using std::string;

extern INTERACTIVE_OBJ * CAMERACONTROLLER;
extern long FRAME_COUNT;

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

class CameraActivateCommand : public Command {
	
public:
	
	Result execute(Context & context) {
		
		string target = context.getLowercase();
		
		LogDebug << "cameraactivate " << target;
		
		if(target == "none") {
			FRAME_COUNT = -1;
			MasterCamera.exist = 0;
			return Success;
		}
		
		FRAME_COUNT = 0;
		
		long t = GetTargetByNameTarget(target);
		if(t == -2) {
			t = GetInterNum(context.getIO());
		}
		
		if(t == -1 || !(inter.iobj[t]->ioflags & IO_CAMERA)) {
			return Failed;
		}
		
		MasterCamera.exist |= 2;
		MasterCamera.want_io = inter.iobj[t];
		MasterCamera.want_aup = inter.iobj[t]->usepath;
		MasterCamera.want_cam = &inter.iobj[t]->_camdata->cam;
		
		return Success;
	}
	
	~CameraActivateCommand() {}
	
};

}

void setupScriptedCamera() {
	
	ScriptEvent::registerCommand("cameracontrol", new CameraControlCommand);
	ScriptEvent::registerCommand("cameraactivate", new CameraActivateCommand);
	
}

} // namespace script
