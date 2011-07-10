
#include "script/ScriptedControl.h"

#include <sstream>

#include "ai/Paths.h"
#include "core/Core.h"
#include "core/GameTime.h"
#include "graphics/GraphicsModes.h"
#include "gui/Speech.h"
#include "physics/Attractors.h"
#include "physics/Collisions.h"
#include "io/Logger.h"
#include "io/PakReader.h"
#include "io/FilePath.h"
#include "platform/String.h"
#include "scene/Interactive.h"
#include "scene/GameSound.h"
#include "script/ScriptUtils.h"

using std::string;

extern long PLAY_LOADED_CINEMATIC;
extern char WILL_LAUNCH_CINE[256];
extern long CINE_PRELOAD;
extern long ARX_CONVERSATION;

namespace script {

namespace {

class ActivatePhysicsCommand : public Command {
	
public:
	
	ActivatePhysicsCommand() : Command("activatephysics", ANY_IO) { }
	
	Result execute(Context & context) {
		
		LogDebug << "activatephysics";
		
		ARX_INTERACTIVE_ActivatePhysics(GetInterNum(context.getIO()));
		
		return Success;
	}
	
};

class AttractorCommand : public Command {
	
public:
	
	AttractorCommand() : Command("attractor") { }
	
	Result execute(Context & context) {
		
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
		
		return Success;
	}
	
};

class AmbianceCommand : public Command {
	
public:
	
	AmbianceCommand() : Command("ambiance") { }
	
	Result execute(Context & context) {
		
		string options = context.getFlags();
		
		if(!options.empty()) {
			u64 flg = flags(options);
			if(flg & flag('v')) {
				float volume = context.getFloat();
				string ambiance = context.getLowercase();
				ARX_SOUND_PlayScriptAmbiance(ambiance, ARX_SOUND_PLAY_LOOPED, volume * 0.01f);
				LogDebug << "ambiance " << options << ' ' << volume << " \"" << ambiance << "\"";
			} else if(!flg || (flg & ~flag('v'))) {
				LogWarning << "unexpected flags: ambiance " << options;
				return Failed;
			}
		} else {
			string ambiance = context.getLowercase();
			if(ambiance == "kill") {
				ARX_SOUND_KillAmbiances();
			} else {
				ARX_SOUND_PlayScriptAmbiance(ambiance);
			}
			LogDebug << "ambiance \"" << ambiance << "\"";
		}
		
		return Success;
	}
	
};

class AnchorBlockCommand : public Command {
	
public:
	
	AnchorBlockCommand() : Command("anchorblock", ANY_IO) { }
	
	Result execute(Context & context) {
		
		bool choice = context.getBool();
		
		ANCHOR_BLOCK_By_IO(context.getIO(), choice ? 1 : 0);
		
		LogDebug << "anchorblock \"" << choice << "\"";
		
		return Success;
	}
	
};

class AttachCommand : public Command {
	
public:
	
	AttachCommand() : Command("attach") { }
	
	Result execute(Context & context) {
		
		string sourceio = context.getLowercase();
		long t = GetTargetByNameTarget(sourceio);
		if(t == -2) {
			t = GetInterNum(context.getIO()); //self
		}
		
		string source = context.getLowercase(); // source action_point
		
		string targetio = context.getLowercase();
		long t2 = GetTargetByNameTarget(targetio);
		if(t2 == -2) {
			t2 = GetInterNum(context.getIO()); //self
		}
		
		string target = context.getLowercase();
		
		ARX_INTERACTIVE_Attach(t, t2, source, target);
		
		LogDebug << "attach " << sourceio << ' ' << source << ' ' << targetio << ' ' << target;
		
		return Success;
	}
	
};

class CineCommand : public Command {
	
public:
	
	CineCommand() : Command("cine") { }
	
	Result execute(Context & context) {
		
		string options = context.getFlags();
		
		bool preload = false;
		if(!options.empty()) {
			u64 flg = flags(options);
			if(flg & flag('p')) {
				preload = true;
			} else if(!flg || (flg & !flag('p'))) {
				LogWarning << "unexpected flags: cine " << options;
			}
		}
		
		string name = context.getLowercase();
		
		LogDebug << "cine " << options << " \"" << name << '"';
		
		if(name == "kill") {
			DANAE_KillCinematic();
		} else if(name == "play") {
			PLAY_LOADED_CINEMATIC = 1;
			ARX_TIME_Pause();
		} else {
			
			string file = "graph\\interface\\illustrations\\" + name + ".cin";
			
			if(resources->getFile(file)) {
				strcpy(WILL_LAUNCH_CINE, name.c_str());
				strcat(WILL_LAUNCH_CINE, ".cin");
				CINE_PRELOAD = preload;
			} else {
				LogError << "unable to load cinematic " << file;
				return Failed;
			}
		}
		
		return Success;
	}
	
};

class ConversationCommand : public Command {
	
public:
	
	ConversationCommand() : Command("conversation") { }
	
	Result execute(Context & context) {
		
		string nbpeople = context.getWord();
		long nb_people = 0;
		if(!nbpeople.empty() && nbpeople[0] == '-') {
			std::istringstream iss(nbpeople.substr(1));
			iss >> nb_people;
			if(iss.bad()) {
				nb_people = 0;
			}
		}
		
		bool enabled = context.getBool();
		ARX_CONVERSATION = enabled ? 1 : 0;
		
		if(!nb_people || !enabled) {
			LogDebug << "conversation " << nbpeople << ' ' << enabled;
			return Success;
		}
		
		main_conversation.actors_nb = nb_people;
		
		std::ostringstream oss;
		oss << "conversation " << nbpeople << ' ' << enabled;
		
		for(long j = 0; j < nb_people; j++) {
			
			string target = context.getLowercase();
			long t = GetTargetByNameTarget(target);
			if(t == -2) {
				t = GetInterNum(context.getIO()); // self
			}
			
			oss << ' ' << target;
			
			main_conversation.actors[j] = t;
		}
		
		LogDebug << oss;
		
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
		
		LogDebug << "quake " << intensity << ' ' << duration << ' ' << period;
		
		AddQuakeFX(intensity, duration, period, 1);
		
		return Success;
	}
	
};

class SetGroupCommand : public Command {
	
public:
	
	SetGroupCommand() : Command("setgroup", ANY_IO) { }
	
	Result execute(Context & context) {
		
		bool remove = false;
		string options = context.getFlags();
		if(!options.empty()) {
			u64 flg = flags(options);
			remove = (flg & flag('r'));
			if(!flg || (flg & ~flag('r'))) {
				LogWarning << "unexpected flags: setgroup " << options;
			}
		}
		
		string group = toLowercase(context.getStringVar(context.getLowercase()));
		
		LogDebug << "setgroup " << options << ' ' << group;
		
		INTERACTIVE_OBJ * io = context.getIO();
		if(group == "door") {
			if(remove) {
				io->GameFlags &= ~GFLAG_DOOR;
			} else {
				io->GameFlags |= GFLAG_DOOR;
			}
		}
		
		if(remove) {
			ARX_IOGROUP_Remove(io, group);
		} else {
			ARX_IOGROUP_Add(io, group);
		}
		
		return Success;
	}
	
};

class SetControlledZoneCommand : public Command {
	
public:
	
	SetControlledZoneCommand() : Command("setcontrolledzone") { }
	
	Result execute(Context & context) {
		
		string name = context.getLowercase();
		
		LogDebug << "setcontrolledzone " << name;
		
		ARX_PATH * ap = ARX_PATH_GetAddressByName(name);
		if(!ap) {
			LogWarning << "unknown zone: setcontrolledzone " << name;
			return Failed;
		}
		
		strcpy(ap->controled, context.getIO()->long_name().c_str());
		
		return Success;
	}
	
};

class SetPathCommand : public Command {
	
public:
	
	SetPathCommand() : Command("setpath", ANY_IO) { }
	
	Result execute(Context & context) {
		
		bool wormspecific = false;
		bool followdir = false;
		string options = context.getFlags();
		if(!options.empty()) {
			u64 flg = flags(options);
			wormspecific = (flg & flag('w'));
			followdir = (flg & flag('f'));
			if(!flg || (flg & ~flags("wf"))) {
				LogWarning << "unexpected flags: setpath " << options;
			}
		}
		
		string name = context.getLowercase();
		
		LogDebug << "setpath " << options << ' ' << name;
		
		INTERACTIVE_OBJ * io = context.getIO();
		if(name == "none") {
			if(io->usepath) {
				free(io->usepath), io->usepath = NULL;
			}
		} else {
			
			ARX_PATH * ap = ARX_PATH_GetAddressByName(name);
			if(!ap) {
				LogWarning << "unknown path: " << name;
				return Failed;
			}
			
			if(io->usepath != NULL) {
				free(io->usepath), io->usepath = NULL;
			}
			
			ARX_USE_PATH * aup = (ARX_USE_PATH *)malloc(sizeof(ARX_USE_PATH));
			aup->_starttime = aup->_curtime = ARXTime;
			aup->aupflags = ARX_USEPATH_FORWARD;
			if(wormspecific) {
				aup->aupflags |= ARX_USEPATH_WORM_SPECIFIC | ARX_USEPATH_FLAG_ADDSTARTPOS;
			}
			if(followdir) {
				aup->aupflags |= ARX_USEPATH_FOLLOW_DIRECTION;
			}
			aup->initpos = io->initpos;
			aup->lastWP = -1;
			aup->path = ap;
			io->usepath = aup;
		}
		
		return Success;
	}
	
};

class ZoneParamCommand : public Command {
	
public:
	
	ZoneParamCommand() : Command("zoneparam") { }
	
	Result execute(Context & context) {
		
		string options = context.getFlags();
		string command = context.getLowercase();
		
		if(command == "stack") {
			LogDebug << "zoeparam stack";
			ARX_GLOBALMODS_Stack();
			
		} else if(command == "unstack") {
			LogDebug << "zoeparam unstack";
			ARX_GLOBALMODS_UnStack();
			
		} else if(command == "rgb") {
			
			desired.depthcolor.r = context.getFloat();
			desired.depthcolor.g = context.getFloat();
			desired.depthcolor.b = context.getFloat();
			desired.flags |= GMOD_DCOLOR;
			
			LogDebug << "zoneparam rgb " << desired.depthcolor.r << ' ' << desired.depthcolor.g << ' ' << desired.depthcolor.b;
			
		} else if(command == "zclip") {
				
			desired.zclip = context.getFloat();
			desired.flags |= GMOD_ZCLIP;
			
			LogDebug << "zoneparam zclip " << desired.zclip;
			
		} else if(command == "ambiance") {
			
			string ambiance = loadPath(context.getWord());
			
			LogDebug << "zoneparam ambiance " << ambiance;
			
			ARX_SOUND_PlayZoneAmbiance(ambiance);
			
		} else {
			LogWarning << "unknwon zoneparam command: " << command;
			return Failed;
		}
		
		return Success;
	}
	
};

}

void setupScriptedControl() {
	
	ScriptEvent::registerCommand(new ActivatePhysicsCommand);
	ScriptEvent::registerCommand(new AttractorCommand);
	ScriptEvent::registerCommand(new AmbianceCommand);
	ScriptEvent::registerCommand(new AnchorBlockCommand);
	ScriptEvent::registerCommand(new AttachCommand);
	ScriptEvent::registerCommand(new CineCommand);
	ScriptEvent::registerCommand(new ConversationCommand);
	ScriptEvent::registerCommand(new QuakeCommand);
	ScriptEvent::registerCommand(new SetGroupCommand);
	ScriptEvent::registerCommand(new SetControlledZoneCommand);
	ScriptEvent::registerCommand(new SetPathCommand);
	ScriptEvent::registerCommand(new ZoneParamCommand);
	
}

} // namespace script
