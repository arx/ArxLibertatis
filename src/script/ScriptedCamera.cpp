
#include "script/ScriptedControl.h"

#include "ai/Paths.h"
#include "graphics/Math.h"
#include "graphics/data/Mesh.h"
#include "gui/Interface.h"
#include "io/Logger.h"
#include "platform/String.h"
#include "scene/Interactive.h"
#include "script/ScriptUtils.h"

using std::string;

extern INTERACTIVE_OBJ * CAMERACONTROLLER;
extern long FRAME_COUNT;

namespace script {

namespace {

class CameraControlCommand : public Command {
	
public:
	
	CameraControlCommand() : Command("cameracontrol", IO_CAMERA) { }
	
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
	
	CameraActivateCommand() : Command("cameraactivate") { }
	
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
	
	CameraSmoothingCommand() : Command("camerasmoothing", IO_CAMERA) { }
	
	Result execute(Context & context) {
		
		float smoothing = context.getFloat();
		
		LogDebug << "camerasmoothing " << smoothing;
		
		context.getIO()->_camdata->cam.smoothing = smoothing;
		
		return Success;
	}
	
	~CameraSmoothingCommand() {}
	
};

class CinemascopeCommand : public Command {
	
public:
	
	CinemascopeCommand() : Command("cinemascope") { }
	
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
		
		LogDebug << "cinemascope " << options << ' ' << enable;
		
		ARX_INTERFACE_SetCinemascope(enable ? 1 : 0, smooth);
		
		return Success;
	}
	
	~CinemascopeCommand() {}
	
};

class CameraFocalCommand : public Command {
	
public:
	
	CameraFocalCommand() : Command("camerafocal", IO_CAMERA) { }
	
	Result execute(Context & context) {
		
		float focal = clamp(context.getFloat(), 100.f, 800.f);
		
		LogDebug << "camerafocal " << focal;
		
		context.getIO()->_camdata->cam.focal = focal;
		
		return Success;
	}
	
	~CameraFocalCommand() {}
	
};

class CameraTranslateTargetCommand : public Command {
	
public:
	
	CameraTranslateTargetCommand() : Command("cameratranslatetarget", IO_CAMERA) { }
	
	Result execute(Context & context) {
		
		float x = context.getFloat();
		float y = context.getFloat();
		float z = context.getFloat();
		
		LogDebug << "cameratranslatetarget " << x << ' ' << y << ' ' << z;
		
		context.getIO()->_camdata->cam.translatetarget = Vec3f(x, y, z);
		
		return Success;
	}
	
	~CameraTranslateTargetCommand() {}
	
};

}

void setupScriptedCamera() {
	
	ScriptEvent::registerCommand(new CameraControlCommand);
	ScriptEvent::registerCommand(new CameraActivateCommand);
	ScriptEvent::registerCommand(new CameraSmoothingCommand);
	ScriptEvent::registerCommand(new CinemascopeCommand);
	ScriptEvent::registerCommand(new CameraFocalCommand);
	ScriptEvent::registerCommand(new CameraTranslateTargetCommand);
	
}

} // namespace script
