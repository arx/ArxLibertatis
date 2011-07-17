
#include "script/ScriptedControl.h"

#include <sstream>

#include "core/Core.h"
#include "core/GameTime.h"
#include "core/Localisation.h"
#include "graphics/Math.h"
#include "graphics/GraphicsModes.h"
#include "gui/Speech.h"
#include "physics/Attractors.h"
#include "physics/Collisions.h"
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
extern long GLOBAL_MAGIC_MODE;

namespace script {

namespace {

class ActivatePhysicsCommand : public Command {
	
public:
	
	ActivatePhysicsCommand() : Command("activatephysics", ANY_IO) { }
	
	Result execute(Context & context) {
		
		DebugScript("");
		
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
		
		DebugScript(" \"" << target << "\" " << val << ' ' << radius);
		
		ARX_SPECIAL_ATTRACTORS_Add(t, val, radius);
		
		return Success;
	}
	
};

class AmbianceCommand : public Command {
	
public:
	
	AmbianceCommand() : Command("ambiance") { }
	
	Result execute(Context & context) {
		
		HandleFlags("v") {
			if(flg & flag('v')) {
				float volume = context.getFloat();
				string ambiance = context.getLowercase();
				DebugScript(' ' << options << ' ' << volume << " \"" << ambiance << '"');
				bool ret = ARX_SOUND_PlayScriptAmbiance(ambiance, ARX_SOUND_PLAY_LOOPED, volume * 0.01f);
				if(!ret) {
					ScriptWarning << "unable to find ambiance \"" << ambiance << '"';
					return Failed;
				}
				return Success;
			}
			return Failed;
		}
		
		string ambiance = context.getLowercase();
		DebugScript(" \"" << ambiance << '"');
		if(ambiance == "kill") {
			ARX_SOUND_KillAmbiances();
		} else {
			bool ret = ARX_SOUND_PlayScriptAmbiance(ambiance);
			if(!ret) {
				ScriptWarning << "unable to find \"" << ambiance << '"';
				return Failed;
			}
		}
		
		return Success;
	}
	
};

class AnchorBlockCommand : public Command {
	
public:
	
	AnchorBlockCommand() : Command("anchorblock", ANY_IO) { }
	
	Result execute(Context & context) {
		
		bool choice = context.getBool();
		
		DebugScript(" \"" << choice << "\"");
		
		ANCHOR_BLOCK_By_IO(context.getIO(), choice ? 1 : 0);
		
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
		
		DebugScript(' ' << sourceio << ' ' << source << ' ' << targetio << ' ' << target);
		
		ARX_INTERACTIVE_Attach(t, t2, source, target);
		
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
		
		string name = context.getLowercase();
		
		DebugScript(' ' << options << " \"" << name << '"');
		
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
				ScriptWarning << "unable to load cinematic \"" << file << '"';
				return Failed;
			}
		}
		
		return Success;
	}
	
};

class SetGroupCommand : public Command {
	
	static void add(INTERACTIVE_OBJ & io, const string & group) {
		
		if(IsIOGroup(&io, group)) {
			return;
		}
		
		io.iogroups = (IO_GROUP_DATA *)realloc(io.iogroups, sizeof(IO_GROUP_DATA) * (io.nb_iogroups + 1));
		strcpy(io.iogroups[io.nb_iogroups].name, group.c_str());
		io.nb_iogroups++;
	}
	
	static void remove(INTERACTIVE_OBJ & io, const string & group) {
		
		long toremove = -1;
		for(long i = 0; i < io.nb_iogroups; i++) {
			if(!strcasecmp(group, io.iogroups[i].name)) {
				toremove = i;
			}
		}
		
		if(toremove == -1) {
			return;
		}
		
		if(io.nb_iogroups == 1) {
			free(io.iogroups);
			io.iogroups = NULL;
			io.nb_iogroups = 0;
			return;
		}
		
		IO_GROUP_DATA * temporary = (IO_GROUP_DATA *)malloc(sizeof(IO_GROUP_DATA) * (io.nb_iogroups - 1));
		long pos = 0;
		for(int i = 0; i < io.nb_iogroups; i++) {
			if(i != toremove) {
				strcpy(temporary[pos++].name, io.iogroups[i].name);
			}
		}
		
		free(io.iogroups);
		io.iogroups = temporary;
		io.nb_iogroups--;
	}
	
public:
	
	SetGroupCommand() : Command("setgroup", ANY_IO) { }
	
	Result execute(Context & context) {
		
		bool rem = false;
		HandleFlags("r") {
			rem = (flg & flag('r'));
		}
		
		string group = toLowercase(context.getStringVar(context.getLowercase()));
		
		DebugScript(' ' << options << ' ' << group);
		
		INTERACTIVE_OBJ & io = *context.getIO();
		if(group == "door") {
			if(rem) {
				io.GameFlags &= ~GFLAG_DOOR;
			} else {
				io.GameFlags |= GFLAG_DOOR;
			}
		}
		
		if(group.empty()) {
			ScriptWarning << "missing group";
			return Failed;
		}
		
		if(rem) {
			remove(io, group);
		} else {
			add(io, group);
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
			DebugScript(" stack");
			ARX_GLOBALMODS_Stack();
			
		} else if(command == "unstack") {
			DebugScript(" unstack");
			ARX_GLOBALMODS_UnStack();
			
		} else if(command == "rgb") {
			
			desired.depthcolor.r = context.getFloat();
			desired.depthcolor.g = context.getFloat();
			desired.depthcolor.b = context.getFloat();
			desired.flags |= GMOD_DCOLOR;
			
			DebugScript(" rgb " << desired.depthcolor.r << ' ' << desired.depthcolor.g << ' ' << desired.depthcolor.b);
			
		} else if(command == "zclip") {
				
			desired.zclip = context.getFloat();
			desired.flags |= GMOD_ZCLIP;
			
			DebugScript(" zclip " << desired.zclip);
			
		} else if(command == "ambiance") {
			
			string ambiance = loadPath(context.getLowercase());
			
			DebugScript(" ambiance " << ambiance);
			
			bool ret = ARX_SOUND_PlayZoneAmbiance(ambiance);
			if(!ret) {
				ScriptWarning << "unable to find ambiance \"" << ambiance << '"';
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
		
		GLOBAL_MAGIC_MODE = context.getBool() ? 1 : 0;
		
		DebugScript(' ' << GLOBAL_MAGIC_MODE);
		
		return Success;
	}
	
};

class DetachCommand : public Command {
	
public:
	
	DetachCommand() : Command("detach") { }
	
	Result execute(Context & context) {
		
		string source = context.getLowercase(); // source IO
		string target = context.getLowercase(); // target IO
		
		DebugScript(' ' << source << ' ' << target);
		
		long t = GetTargetByNameTarget(source);
		if(t == -2) {
			t = GetInterNum(context.getIO()); //self
		}
		if(!ValidIONum(t)) {
			ScriptWarning << "unknown source: " << source;
			return Failed;
		}
		
		long t2 = GetTargetByNameTarget(target);
		if(t2 == -2) {
			t2 = GetInterNum(context.getIO()); //self
		}
		if(!ValidIONum(t2)) {
			ScriptWarning << "unknown target: " << target;
			return Failed;
		}
		
		ARX_INTERACTIVE_Detach(t, t2);
		
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
	ScriptEvent::registerCommand(new SetGroupCommand);
	ScriptEvent::registerCommand(new ZoneParamCommand);
	ScriptEvent::registerCommand(new MagicCommand);
	ScriptEvent::registerCommand(new DetachCommand);
	
}

} // namespace script
