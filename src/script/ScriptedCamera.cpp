
#include "script/ScriptedControl.h"

#include "ai/Paths.h"
#include "core/Core.h"
#include "core/GameTime.h"
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
		
		DebugScript(' ' << enable);
		
		CAMERACONTROLLER = enable ? context.getIO() : NULL;
		
		return Success;
	}
	
};

class CameraActivateCommand : public Command {
	
public:
	
	CameraActivateCommand() : Command("cameraactivate") { }
	
	Result execute(Context & context) {
		
		string target = context.getLowercase();
		
		DebugScript(' ' << target);
		
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
	
};

class CameraSmoothingCommand : public Command {
	
public:
	
	CameraSmoothingCommand() : Command("camerasmoothing", IO_CAMERA) { }
	
	Result execute(Context & context) {
		
		float smoothing = context.getFloat();
		
		DebugScript(' ' << smoothing);
		
		context.getIO()->_camdata->cam.smoothing = smoothing;
		
		return Success;
	}
	
};

class CinemascopeCommand : public Command {
	
public:
	
	CinemascopeCommand() : Command("cinemascope") { }
	
	Result execute(Context & context) {
		
		bool smooth = false;
		HandleFlags("s") {
			if(flg & flag('s')) {
				smooth = true;
			}
		}
		
		bool enable = context.getBool();
		
		DebugScript(' ' << options << ' ' << enable);
		
		ARX_INTERFACE_SetCinemascope(enable ? 1 : 0, smooth);
		
		return Success;
	}
	
};

class CameraFocalCommand : public Command {
	
public:
	
	CameraFocalCommand() : Command("camerafocal", IO_CAMERA) { }
	
	Result execute(Context & context) {
		
		float focal = clamp(context.getFloat(), 100.f, 800.f);
		
		DebugScript(' ' << focal);
		
		context.getIO()->_camdata->cam.focal = focal;
		
		return Success;
	}
	
};

class CameraTranslateTargetCommand : public Command {
	
public:
	
	CameraTranslateTargetCommand() : Command("cameratranslatetarget", IO_CAMERA) { }
	
	Result execute(Context & context) {
		
		float x = context.getFloat();
		float y = context.getFloat();
		float z = context.getFloat();
		
		DebugScript(' ' << x << ' ' << y << ' ' << z);
		
		context.getIO()->_camdata->cam.translatetarget = Vec3f(x, y, z);
		
		return Success;
	}
	
};

class WorldFadeCommand : public Command {
	
public:
	
	WorldFadeCommand() : Command("worldfade") { }
	
	Result execute(Context & context) {
		
		string inout = context.getLowercase();
		
		FADEDURATION = context.getFloat();
		FADESTART = ARX_TIME_GetUL();
		
		if(inout == "out") {
			
			FADECOLOR.r = context.getFloat();
			FADECOLOR.g = context.getFloat();
			FADECOLOR.b = context.getFloat();
			FADEDIR = -1;
			
			DebugScript(" out " << FADEDURATION << ' ' << FADECOLOR.r << ' ' << FADECOLOR.g << ' ' << FADECOLOR.b);
			
		} else if(inout == "in") {
			
			FADEDIR = 1;
			
			DebugScript(" in " << FADEDURATION);
			
		} else {
			ScriptWarning << "unexpected fade direction: " << inout;
			return Failed;
		}
		
		return Success;
	}
	
};

class QuakeCommand : public Command {
	
public:
	
	QuakeCommand() : Command("quake") { }
	
	Result execute(Context & context) {
		
		float intensity = context.getFloat();
		float duration = context.getFloat();
		float period = context.getFloat();
		
		DebugScript(' ' << intensity << ' ' << duration << ' ' << period);
		
		AddQuakeFX(intensity, duration, period, 1);
		
		return Success;
	}
	
};

}

void setupScriptedCamera() {
	
	ScriptEvent::registerCommand(new CameraControlCommand);
	ScriptEvent::registerCommand(new CameraActivateCommand);
	ScriptEvent::registerCommand(new CameraSmoothingCommand);
	ScriptEvent::registerCommand(new CinemascopeCommand);
	ScriptEvent::registerCommand(new CameraFocalCommand);
	ScriptEvent::registerCommand(new CameraTranslateTargetCommand);
	ScriptEvent::registerCommand(new WorldFadeCommand);
	ScriptEvent::registerCommand(new QuakeCommand);
	
}

} // namespace script
