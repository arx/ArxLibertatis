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

#include "script/ScriptedControl.h"

#include "ai/Anchors.h"

#include "core/Core.h"
#include "core/GameTime.h"
#include "game/EntityManager.h"
#include "physics/Attractors.h"
#include "physics/Collisions.h"
#include "io/resource/PakReader.h"
#include "io/resource/ResourcePath.h"
#include "scene/Interactive.h"
#include "scene/GameSound.h"
#include "script/ScriptUtils.h"
#include "cinematic/CinematicController.h"


extern bool GLOBAL_MAGIC_MODE;

namespace script {

namespace {

class ActivatePhysicsCommand : public Command {
	
public:
	
	ActivatePhysicsCommand() : Command("activatephysics", AnyEntity) { }
	
	Result execute(Context & context) {
		
		DebugScript("");
		
		ARX_INTERACTIVE_ActivatePhysics(context.getEntity()->index());
		
		return Success;
	}
	
};

class AttractorCommand : public Command {
	
public:
	
	AttractorCommand() : Command("attractor") { }
	
	Result execute(Context & context) {
		
		std::string target = context.getWord();
		
		Entity * t = entities.getById(target, context.getEntity());
		
		std::string power = context.getWord();
		
		float val = 0.f;
		float radius = 0.f;
		
		if(power != "off") {
			val = context.getFloatVar(power);
			radius = context.getFloat();
		}
		
		DebugScript(' ' << target << ' ' << val << ' ' << radius);
		
		ARX_SPECIAL_ATTRACTORS_Add((t == NULL) ? EntityHandle() : t->index(), val, radius);
		
		return Success;
	}
	
};

class AmbianceCommand : public Command {
	
public:
	
	AmbianceCommand() : Command("ambiance") { }
	
	Result execute(Context & context) {
		
		HandleFlags("nv") {
			
			if(flg & flag('v')) {
				float volume = context.getFloat();
				res::path ambiance = res::path::load(context.getWord());
				DebugScript(' ' << options << ' ' << volume << ' ' << ambiance);
				bool ret = ARX_SOUND_PlayScriptAmbiance(ambiance, ARX_SOUND_PLAY_LOOPED, volume * 0.01f);
				if(!ret) {
					ScriptWarning << "unable to find ambiance " << ambiance;
					return Failed;
				}
				return Success;
			}
			
			if(flg & flag('n')) {
				res::path ambiance = res::path::load(context.getWord());
				DebugScript(' ' << options << ' ' << ambiance);
				ARX_SOUND_PlayScriptAmbiance(ambiance, ARX_SOUND_PLAY_ONCE);
				return Success;
			}
		}
		
		res::path ambiance = res::path::load(context.getWord());
		DebugScript(' ' << ambiance);
		if(ambiance == "kill") {
			ARX_SOUND_KillAmbiances();
		} else {
			bool ret = ARX_SOUND_PlayScriptAmbiance(ambiance);
			if(!ret) {
				ScriptWarning << "unable to find " << ambiance;
				return Failed;
			}
		}
		
		return Success;
	}
	
};

class AnchorBlockCommand : public Command {
	
public:
	
	AnchorBlockCommand() : Command("anchorblock", AnyEntity) { }
	
	Result execute(Context & context) {
		
		bool blocked = context.getBool();
		
		DebugScript(' ' << blocked);
		
		ANCHOR_BLOCK_By_IO(context.getEntity(), blocked);
		
		return Success;
	}
	
};

class AttachCommand : public Command {
	
public:
	
	AttachCommand() : Command("attach") { }
	
	Result execute(Context & context) {
		
		std::string sourceio = context.getWord();
		Entity * t = entities.getById(sourceio, context.getEntity());
		
		std::string source = context.getWord(); // source action_point
		
		std::string targetio = context.getWord();
		Entity * t2 = entities.getById(targetio, context.getEntity());
		
		std::string target = context.getWord();
		
		DebugScript(' ' << sourceio << ' ' << source << ' ' << targetio << ' ' << target);
		
		EntityHandle i = (t == NULL) ? EntityHandle() : t->index();
		EntityHandle i2 = (t2 == NULL) ? EntityHandle() : t2->index();
		
		ARX_INTERACTIVE_Attach(i, i2, source, target);
		
		return Success;
	}
	
};

class CineCommand : public Command {
	
public:
	
	CineCommand() : Command("cine") { }
	
	Result execute(Context & context) {
		
		bool preload = false;
		HandleFlags("p") {
			if(flg & flag('p')) {
				preload = true;
			}
		}
		
		std::string name = context.getWord();
		
		DebugScript(' ' << options << " \"" << name << '"');
		
		if(name == "kill") {
			cinematicKill();
		} else if(name == "play") {
			cinematicRequestStart();
		} else {
			
			if(g_resources->getFile(res::path("graph/interface/illustrations") / (name + ".cin"))) {
				cinematicPrepare(name + ".cin", preload);
			} else {
				ScriptWarning << "unable to find cinematic \"" << name << '"';
				return Failed;
			}
		}
		
		return Success;
	}
	
};

class SetGroupCommand : public Command {
	
public:
	
	SetGroupCommand() : Command("setgroup", AnyEntity) { }
	
	Result execute(Context & context) {
		
		bool rem = false;
		HandleFlags("r") {
			rem = test_flag(flg, 'r');
		}
		
		std::string group = context.getStringVar(context.getWord());
		
		DebugScript(' ' << options << ' ' << group);
		
		Entity & io = *context.getEntity();
		if(group == "door") {
			if(rem) {
				io.gameFlags &= ~GFLAG_DOOR;
			} else {
				io.gameFlags |= GFLAG_DOOR;
			}
		}
		
		if(group.empty()) {
			ScriptWarning << "missing group";
			return Failed;
		}
		
		if(rem) {
			io.groups.erase(group);
		} else {
			io.groups.insert(group);
		}
		
		return Success;
	}
	
};

class ZoneParamCommand : public Command {
	
public:
	
	ZoneParamCommand() : Command("zoneparam") { }
	
	Result execute(Context & context) {
		
		std::string command = context.getWord();
		
		if(command == "ambiance") {
			
			res::path ambiance = res::path::load(context.getWord());
			
			DebugScript(" ambiance " << ambiance);
			
			bool ret = ARX_SOUND_PlayZoneAmbiance(ambiance);
			if(!ret) {
				ScriptWarning << "unable to find ambiance " << ambiance;
			}
			
		} else {
			ScriptWarning << "unknown command: " << command;
			return Failed;
		}
		
		return Success;
	}
	
};

class MagicCommand : public Command {
	
public:
	
	MagicCommand() : Command("magic") { }
	
	Result execute(Context & context) {
		
		GLOBAL_MAGIC_MODE = context.getBool();
		
		DebugScript(' ' << GLOBAL_MAGIC_MODE);
		
		return Success;
	}
	
};

class DetachCommand : public Command {
	
public:
	
	DetachCommand() : Command("detach") { }
	
	Result execute(Context & context) {
		
		std::string source = context.getWord(); // source IO
		std::string target = context.getWord(); // target IO
		
		DebugScript(' ' << source << ' ' << target);
		
		Entity * t = entities.getById(source, context.getEntity());
		if(!t) {
			ScriptWarning << "unknown source: " << source;
			return Failed;
		}
		
		Entity * t2 = entities.getById(target, context.getEntity());
		if(!t2) {
			ScriptWarning << "unknown target: " << target;
			return Failed;
		}
		
		ARX_INTERACTIVE_Detach(t->index(), t2->index());
		
		return Success;
	}
	
};

} // anonymous namespace

void setupScriptedControl() {
	
	ScriptEvent::registerCommand(new ActivatePhysicsCommand);
	ScriptEvent::registerCommand(new AttractorCommand);
	ScriptEvent::registerCommand(new AmbianceCommand);
	ScriptEvent::registerCommand(new AnchorBlockCommand);
	ScriptEvent::registerCommand(new AttachCommand);
	ScriptEvent::registerCommand(new CineCommand);
	ScriptEvent::registerCommand(new SetGroupCommand);
	ScriptEvent::registerCommand(new ZoneParamCommand);
	ScriptEvent::registerCommand(new MagicCommand);
	ScriptEvent::registerCommand(new DetachCommand);
	
}

} // namespace script
