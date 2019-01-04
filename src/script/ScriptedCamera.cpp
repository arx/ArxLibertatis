/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */
/* Based on:
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/

#include "script/ScriptedCamera.h"

#include "ai/Paths.h"
#include "core/Core.h"
#include "core/GameTime.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/Camera.h"
#include "game/effect/Quake.h"
#include "graphics/Math.h"
#include "graphics/data/Mesh.h"
#include "graphics/effects/Fade.h"
#include "gui/Interface.h"
#include "scene/Interactive.h"
#include "script/ScriptUtils.h"


namespace script {

namespace {

class CameraActivateCommand : public Command {
	
public:
	
	CameraActivateCommand() : Command("cameraactivate") { }
	
	Result execute(Context & context) {
		
		std::string target = context.getWord();
		
		DebugScript(' ' << target);
		
		if(target == "none") {
			g_cameraEntity = NULL;
			return Success;
		}
		
		Entity * t = entities.getById(target, context.getEntity());
		if(!t || !(t->ioflags & IO_CAMERA)) {
			return Failed;
		}
		
		g_cameraEntity = t;
		
		return Success;
	}
	
};

class CameraSmoothingCommand : public Command {
	
public:
	
	CameraSmoothingCommand() : Command("camerasmoothing", IO_CAMERA) { }
	
	Result execute(Context & context) {
		
		float smoothing = context.getFloat();
		
		DebugScript(' ' << smoothing);
		
		context.getEntity()->_camdata->smoothing = smoothing;
		
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
		
		cinematicBorder.set(enable, smooth);
		
		return Success;
	}
	
};

class CameraFocalCommand : public Command {
	
public:
	
	CameraFocalCommand() : Command("camerafocal", IO_CAMERA) { }
	
	Result execute(Context & context) {
		
		float focal = glm::clamp(context.getFloat(), 100.f, 800.f);
		
		DebugScript(' ' << focal);
		
		context.getEntity()->_camdata->cam.focal = focal;
		
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
		
		context.getEntity()->_camdata->translatetarget = Vec3f(x, y, z);
		
		return Success;
	}
	
};

class WorldFadeCommand : public Command {
	
public:
	
	WorldFadeCommand() : Command("worldfade") { }
	
	Result execute(Context & context) {
		
		std::string inout = context.getWord();
		const PlatformDuration duration = PlatformDurationMsf(context.getFloat());
		
		if(inout == "out") {
			
			Color3f color;
			color.r = context.getFloat();
			color.g = context.getFloat();
			color.b = context.getFloat();
			fadeSetColor(color);
			
			fadeRequestStart(FadeType_Out, duration);
			
			DebugScript(" out " << toMs(duration) << ' ' << color.r << ' ' << color.g << ' ' << color.b);
		} else if(inout == "in") {
			
			fadeRequestStart(FadeType_In, duration);
			
			DebugScript(" in " << toMs(duration));
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
		
		AddQuakeFX(intensity, GameDurationMsf(duration), period, true);
		
		return Success;
	}
	
};

} // anonymous namespace

void setupScriptedCamera() {
	
	ScriptEvent::registerCommand(new CameraActivateCommand);
	ScriptEvent::registerCommand(new CameraSmoothingCommand);
	ScriptEvent::registerCommand(new CinemascopeCommand);
	ScriptEvent::registerCommand(new CameraFocalCommand);
	ScriptEvent::registerCommand(new CameraTranslateTargetCommand);
	ScriptEvent::registerCommand(new WorldFadeCommand);
	ScriptEvent::registerCommand(new QuakeCommand);
	
}

} // namespace script
