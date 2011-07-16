
#include "script/ScriptedControl.h"

#include <sstream>

#include "ai/Paths.h"
#include "core/Core.h"
#include "core/GameTime.h"
#include "core/Localisation.h"
#include "graphics/Math.h"
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
extern long FINAL_RELEASE;
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
			DebugScript(' ' << nbpeople << ' ' << enabled);
			return Success;
		}
		
		main_conversation.actors_nb = nb_people;
		
		std::ostringstream oss;
		oss << ' ' << nbpeople << ' ' << enabled;
		
		for(long j = 0; j < nb_people; j++) {
			
			string target = context.getLowercase();
			long t = GetTargetByNameTarget(target);
			if(t == -2) {
				t = GetInterNum(context.getIO()); // self
			}
			
			oss << ' ' << target;
			
			main_conversation.actors[j] = t;
		}
		
		DebugScript(oss);
		
		return Success;
	}
	
};

class SetGroupCommand : public Command {
	
public:
	
	SetGroupCommand() : Command("setgroup", ANY_IO) { }
	
	Result execute(Context & context) {
		
		bool remove = false;
		HandleFlags("r") {
			remove = (flg & flag('r'));
		}
		
		string group = toLowercase(context.getStringVar(context.getLowercase()));
		
		DebugScript(' ' << options << ' ' << group);
		
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
		
		DebugScript(' ' << name);
		
		ARX_PATH * ap = ARX_PATH_GetAddressByName(name);
		if(!ap) {
			ScriptWarning << "unknown zone: " << name;
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
		HandleFlags("wf") {
			wormspecific = (flg & flag('w'));
			followdir = (flg & flag('f'));
		}
		
		string name = context.getLowercase();
		
		DebugScript(' ' << options << ' ' << name);
		
		INTERACTIVE_OBJ * io = context.getIO();
		if(name == "none") {
			if(io->usepath) {
				free(io->usepath), io->usepath = NULL;
			}
		} else {
			
			ARX_PATH * ap = ARX_PATH_GetAddressByName(name);
			if(!ap) {
				ScriptWarning << "unknown path: " << name;
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
			
			string ambiance = loadPath(context.getWord());
			
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

class PlayCommand : public Command {
	
public:
	
	PlayCommand() : Command("play", ANY_IO) { }
	
	Result execute(Context & context) {
		
		bool unique = false;
		SoundLoopMode loop = ARX_SOUND_PLAY_ONCE;
		float pitch = 1.f;
		bool stop = false;
		bool no_pos = false;
		
		HandleFlags("ilpso") {
			unique = (flg & flag('i'));
			if(flg & flag('l')) {
				loop = ARX_SOUND_PLAY_LOOPED;
			}
			if(flg & flag('p')) {
				pitch = 0.9F + 0.2F * rnd();
			}
			stop = (flg & flag('s'));
			no_pos = (flg & flag('o'));
		}
		
		string sample = loadPath(context.getStringVar(context.getLowercase()));
		SetExt(sample, ".wav");
		
		DebugScript(' ' << options << " \"" << sample << '"');
		
		INTERACTIVE_OBJ * io = context.getIO();
		if(stop) {
			ARX_SOUND_Stop(io->sound);
			io->sound = ARX_SOUND_INVALID_RESOURCE;
			
		} else {
			
			if(unique && io->sound != ARX_SOUND_INVALID_RESOURCE) {
				ARX_SOUND_Stop(io->sound);
			}
			
			audio::SampleId num;
			// TODO (case-sensitive) should be a flag instead of depending on the event
			if(no_pos || SM_INVENTORYUSE == context.getMessage()) {
				num = ARX_SOUND_PlayScript(sample, NULL, pitch, loop);
			} else {
				num = ARX_SOUND_PlayScript(sample, io, pitch, loop);
			}
			
			if(unique) {
				io->sound = (num == ARX_SOUND_TOO_FAR) ? audio::INVALID_ID : num;
			}
			
			if(num == audio::INVALID_ID) {
				ScriptWarning << "unable to load sound file \"" << sample << '"';
				return Failed;
			}
			
		}
		
		return Success;
	}
	
};

class PlaySpeechCommand : public Command {
	
public:
	
	PlaySpeechCommand() : Command("playspeech") { }
	
	Result execute(Context & context) {
		
		string sample = loadPath(context.getLowercase());
		
		DebugScript(' ' << sample);
		
		INTERACTIVE_OBJ * io = context.getIO();
		audio::SampleId num = ARX_SOUND_PlaySpeech(sample, io && io->show == 1 ? io : NULL);
		
		if(num == audio::INVALID_ID) {
			ScriptWarning << "unable to load sound file \"" << sample << '"';
			return Failed;
		}
		
		return Success;
	}
	
};

class HeroSayCommand : public Command {
	
public:
	
	HeroSayCommand() : Command("herosay") { }
	
	Result execute(Context & context) {
		
		HandleFlags("d") {
			if((flg & flag('d')) && FINAL_RELEASE) {
				context.skipWord();
				return Success;
			}
		}
		
		string text = toLowercase(context.getStringVar(context.getLowercase()));
		
		DebugScript(' ' << options << " \"" << text << '"');
		
		if(!text.empty() && text[0] == '[') {
			text = getLocalised(loadUnlocalized(text), "Not Found");
		}
		
		ARX_SPEECH_Add(text);
		
		return Success;
	}
	
};

class UsePathCommand : public Command {
	
public:
	
	UsePathCommand() : Command("usepath", ANY_IO) { }
	
	Result execute(Context & context) {
		
		string type = context.getLowercase();
		
		DebugScript(' ' << type);
		
		ARX_USE_PATH * aup = context.getIO()->usepath;
		if(!aup) {
			ScriptWarning << "no path set";
			return Failed;
		}
		
		if(type == "b") {
			aup->aupflags &= ~ARX_USEPATH_PAUSE;
			aup->aupflags &= ~ARX_USEPATH_FORWARD;
			aup->aupflags |= ARX_USEPATH_BACKWARD;
		} else if(type == "f") {
			aup->aupflags &= ~ARX_USEPATH_PAUSE;
			aup->aupflags |= ARX_USEPATH_FORWARD;
			aup->aupflags &= ~ARX_USEPATH_BACKWARD;
		} else if(type == "p") {
			aup->aupflags |= ARX_USEPATH_PAUSE;
			aup->aupflags &= ~ARX_USEPATH_FORWARD;
			aup->aupflags &= ~ARX_USEPATH_BACKWARD;
		} else {
			ScriptWarning << "unknown usepath type: " << type;
			return Failed;
		}
		
		return Success;
	}
	
};

class UnsetControlledZoneCommand : public Command {
	
public:
	
	UnsetControlledZoneCommand() : Command("unsetcontrolledzone") { }
	
	Result execute(Context & context) {
		
		string zone = context.getLowercase();
		
		DebugScript(' ' << zone);
		
		ARX_PATH * ap = ARX_PATH_GetAddressByName(zone);
		if(ap) {
			ap->controled[0] = 0;
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
	ScriptEvent::registerCommand(new ConversationCommand);
	ScriptEvent::registerCommand(new SetGroupCommand);
	ScriptEvent::registerCommand(new SetControlledZoneCommand);
	ScriptEvent::registerCommand(new SetPathCommand);
	ScriptEvent::registerCommand(new ZoneParamCommand);
	ScriptEvent::registerCommand(new PlayCommand);
	ScriptEvent::registerCommand(new PlaySpeechCommand);
	ScriptEvent::registerCommand(new HeroSayCommand);
	ScriptEvent::registerCommand(new UsePathCommand);
	ScriptEvent::registerCommand(new UnsetControlledZoneCommand);
	ScriptEvent::registerCommand(new MagicCommand);
	ScriptEvent::registerCommand(new DetachCommand);
	
}

} // namespace script
