
#include "script/ScriptedControl.h"

#include <sstream>

#include "core/Core.h"
#include "core/GameTime.h"
#include "gui/Speech.h"
#include "physics/Attractors.h"
#include "physics/Collisions.h"
#include "io/Logger.h"
#include "io/PakReader.h"
#include "platform/String.h"
#include "scene/Interactive.h"
#include "scene/GameSound.h"
#include "script/ScriptEvent.h"
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
	
	Result execute(Context & context) {
		
		ARX_INTERACTIVE_ActivatePhysics(GetInterNum(context.getIO()));
		
		LogDebug << "activatephysics";
		
		return Success;
	}
	
	~ActivatePhysicsCommand() { }
	
};

class AttractorCommand : public Command {
	
public:
	
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
	
	~AttractorCommand() { }
	
};

class AmbianceCommand : public Command {
	
public:
	
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
	
	~AmbianceCommand() { }
	
};

class AnchorBlockCommand : public Command {
	
public:
	
	Result execute(Context & context) {
		
		bool choice = context.getBool();
		
		ANCHOR_BLOCK_By_IO(context.getIO(), choice ? 1 : 0);
		
		LogDebug << "anchorblock \"" << choice << "\"";
		
		return Success;
	}
	
	~AnchorBlockCommand() { }
	
};

class AttachCommand : public Command {
	
public:
	
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
	
	~AttachCommand() {}
	
};

class CineCommand : public Command {
	
public:
	
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
	
	~CineCommand() {}
	
};

class ConversationCommand : public Command {
	
public:
	
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
	
	~ConversationCommand() {}
	
};

}

void setupScriptedControl() {
	
	ScriptEvent::registerCommand("activatephysics", new ActivatePhysicsCommand);
	ScriptEvent::registerCommand("attractor", new AttractorCommand);
	ScriptEvent::registerCommand("ambiance", new AmbianceCommand);
	ScriptEvent::registerCommand("anchorblock", new AnchorBlockCommand);
	ScriptEvent::registerCommand("attach", new AttachCommand);
	ScriptEvent::registerCommand("cine", new CineCommand);
	ScriptEvent::registerCommand("conversation", new ConversationCommand);
	
}

} // namespace script
