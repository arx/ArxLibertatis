
#include "script/ScriptedControl.h"

#include "ai/Paths.h"
#include "graphics/data/Mesh.h"
#include "gui/Interface.h"
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

class CameraSmoothingCommand : public Command {
	
public:
	
	Result execute(Context & context) {
		
		float smoothing = context.getFloat();
		
		LogDebug << "camerasmoothing " << smoothing;
		
		INTERACTIVE_OBJ * io = context.getIO();
		if(!io || !(io->ioflags & IO_CAMERA)) {
			return Failed;
		}
		
		io->_camdata->cam.smoothing = smoothing;
		
		return Success;
	}
	
	~CameraSmoothingCommand() {}
	
};

class CinemascopeCommand : public Command {
	
public:
	
	Result execute(Context & context) {
		
		string options = context.getFlags();
		bool smooth = false;
		if(!options.empty()) {
			u64 flg = flags(options);
			if(flg & flag('s')) {
				smooth = true;
			} else if(!flg || (flg & ~flag('s'))) {
				LogWarning << "unexpected flags: cinemascope " << options;
			}
		}
		
		bool enable = context.getBool();
		
		ARX_INTERFACE_SetCinemascope(enable ? 1 : 0, smooth);
		
		return Success;
	}
	
	~CinemascopeCommand() {}
	
};

}

void setupScriptedCamera() {
	
	ScriptEvent::registerCommand("cameracontrol", new CameraControlCommand);
	ScriptEvent::registerCommand("cameraactivate", new CameraActivateCommand);
	ScriptEvent::registerCommand("camerasmoothing", new CameraSmoothingCommand);
	ScriptEvent::registerCommand("cinemascope", new CinemascopeCommand);
	
}

} // namespace script
