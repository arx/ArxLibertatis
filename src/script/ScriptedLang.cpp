
#include "script/ScriptedLang.h"

#include "core/GameTime.h"
#include "graphics/Math.h"
#include "platform/String.h"
#include "io/Logger.h"
#include "script/ScriptEvent.h"
#include "script/ScriptUtils.h"

using std::string;

namespace script {

namespace {

class NopCommand : public Command {
	
public:
	
	NopCommand() : Command("nop") { }
	
	Result execute(Context & context) {
		
		ARX_UNUSED(context);
		
		LogDebug << "nop";
		
		return Success;
	}
	
};

class GotoCommand : public Command {
	
	bool sub;
	
public:
	
	GotoCommand(string command, bool _sub = false) : Command(command), sub(_sub) { }
	
	Result execute(Context & context) {
		
		string label = context.getLowercase();
		
		LogDebug << getName() << ' ' << label;
		
		size_t pos = context.skipCommand();
		if(pos != (size_t)-1) {
			LogWarning << "unexpected text after " << getName() << " at " << pos;
		}
		
		if(!context.jumpToLabel(label)) {
			// TODO should be a warning, but some scripts do this
			LogDebug << "error jumping to label \"" << label << '"';
			return AbortError;
		}
		
		return Jumped;
	}
	
};

class AbortCommand : public Command {
	
	Result result;
	
public:
	
	AbortCommand(string command, Result _result) : Command(command), result(_result) { }
	
	Result execute(Context & context) {
		
		ARX_UNUSED(context);
		
		LogDebug << getName();
		
		return result;
	}
	
};

class RandomCommand : public Command {
	
public:
	
	RandomCommand() : Command("random") { }
	
	Result execute(Context & context) {
		
		float chance = clamp(context.getFloat(), 0.f, 100.f);
		
		LogDebug << "random " << chance;
		
		float t = rnd() * 100.f;
		if(chance < t) {
			context.skipStatement();
		}
		
		return Jumped;
	}
	
};

class ReturnCommand : public Command {
	
public:
	
	ReturnCommand() : Command("return") { }
	
	Result execute(Context & context) {
		
		LogDebug << getName();
		
		if(!context.returnToCaller()) {
			LogError << "return failed";
			return AbortError;
		}
		
		return Jumped;
	}
	
};

class SetMainEventCommand : public Command {
	
public:
	
	SetMainEventCommand(const string & command) : Command(command, ANY_IO) { }
	
	Result execute(Context & context) {
		
		string event = context.getLowercase();
		
		LogDebug << "setmainevent " << event;
		
		ARX_SCRIPT_SetMainEvent(context.getIO(), event);
		
		return Success;
	}
	
};

class StartStopTimerCommand : public Command {
	
	const bool start;
	
public:
	
	StartStopTimerCommand(const string & command, bool _start) : Command(command), start(_start) { }
	
	Result execute(Context & context) {
		
		string timer = context.getLowercase();
		
		LogDebug << getName() << ' ' << timer;
		
		long t;
		if(timer == "timer1") {
			t = 0;
		} else if(timer == "timer2") {
			t = 1;
		} else if(timer == "timer3") {
			t = 2;
		} else if(timer == "timer4") {
			t = 3;
		} else {
			LogWarning << "invalid timer: " << timer;
			return Failed;
		}
		
		if(start) {
			context.getScript()->timers[t] = ARXTimeUL();
			if(context.getScript()->timers[t] == 0) {
				context.getScript()->timers[t] = 1;
			}
		} else {
			context.getScript()->timers[t] = 0;
		}
		
		return Success;
	}
	
};

}

void setupScriptedLang() {
	
	ScriptEvent::registerCommand(new NopCommand);
	ScriptEvent::registerCommand(new GotoCommand("goto"));
	ScriptEvent::registerCommand(new GotoCommand("gosub", true));
	ScriptEvent::registerCommand(new AbortCommand("accept", Command::AbortAccept));
	ScriptEvent::registerCommand(new AbortCommand("refuse", Command::AbortRefuse));
	ScriptEvent::registerCommand(new RandomCommand);
	ScriptEvent::registerCommand(new ReturnCommand);
	ScriptEvent::registerCommand(new SetMainEventCommand("setstatus"));
	ScriptEvent::registerCommand(new SetMainEventCommand("setmainevent"));
	ScriptEvent::registerCommand(new StartStopTimerCommand("starttimer", true));
	ScriptEvent::registerCommand(new StartStopTimerCommand("stoptimer", false));
	
}

} // namespace script
