
#include "scripting/ScriptedControl.h"

#include "physics/Attractors.h"
#include "io/Logger.h"
#include "platform/String.h"
#include "scene/Interactive.h"
#include "scene/GameSound.h"
#include "scripting/ScriptEvent.h"

using std::string;

namespace {

class AttractorCommand : public ScriptCommand {
	
public:
	
	ScriptResult execute(ScriptContext & context) {
		
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
		
		
		return ACCEPT;
	}
	
	~AttractorCommand() { }
	
};

class AmbianceCommand : public ScriptCommand {
	
public:
	
	ScriptResult execute(ScriptContext & context) {
		
		string flags = context.getFlags();
		
		if(!flags.empty()) {
			if(CharIn(flags, 'v')) {
				float volume = context.getFloat();
				string ambiance = context.getLowercase();
				ARX_SOUND_PlayScriptAmbiance(ambiance, ARX_SOUND_PLAY_LOOPED, volume * 0.01f);
				LogDebug << "ambiance " << flags << ' ' << volume << " \"" << ambiance << "\"";
			} else {
				LogDebug << "ambiance " << flags;
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
		
		return ACCEPT;
	}
	
	~AmbianceCommand() { }
	
};

}

void setupScriptedControl() {
	
	ScriptEvent::registerCommand("attractor", new AttractorCommand);
	ScriptEvent::registerCommand("ambiance", new AmbianceCommand);
	
}
