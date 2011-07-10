
#include "script/ScriptedLang.h"

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
	
	Result execute(Context & context) {
		
		ARX_UNUSED(context);
		
		LogDebug << "nop";
		
		return Success;
	}
	
	~NopCommand() { }
	
};

class GotoCommand : public Command {
	
	string command;
	bool sub;
	
public:
	
	GotoCommand(string _command, bool _sub = false) : command(_command), sub(_sub) { }
	
	Result execute(Context & context) {
		
		string label = context.getWord();
		
		LogDebug << command << ' ' << label;
		
		size_t pos = context.skipCommand();
		if(pos != (size_t)-1) {
			LogWarning << "unexpected text after " << command << " at " << pos;
		}
		
		if(!context.jumpToLabel(label)) {
			// TODO should be a warning, but some scripts do this
			LogInfo << "error jumping to label \"" << label << '"';
			return AbortError;
		}
		
		return Jumped;
	}
	
	~GotoCommand() { }
	
};

class AbortCommand : public Command {
	
	string command;
	Result result;
	
public:
	
	AbortCommand(string _command, Result _result) : command(_command), result(_result) { }
	
	Result execute(Context & context) {
		
		ARX_UNUSED(context);
		
		LogDebug << command;
		
		return result;
	}
	
	~AbortCommand() { }
	
};

class RandomCommand : public Command {
	
public:
	
	Result execute(Context & context) {
		
		float chance = clamp(context.getFloat(), 0.f, 100.f);
		
		LogDebug << "random " << chance;
		
		float t = rnd() * 100.f;
		if(chance < t) {
			context.skipStatement();
		}
		
		return Jumped;
	}
	
	~RandomCommand() { }
	
};

class ReturnCommand : public Command {
	
public:
	
	Result execute(Context & context) {
		
		LogDebug << "return";
		
		if(!context.returnToCaller()) {
			LogError << "return failed";
			return AbortError;
		}
		
		return Jumped;
	}
	
	~ReturnCommand() { }
	
};

}

void setupScriptedLang() {
	
	ScriptEvent::registerCommand("nop", new NopCommand);
	ScriptEvent::registerCommand("goto", new GotoCommand("goto"));
	ScriptEvent::registerCommand("gosub", new GotoCommand("gosub", true));
	ScriptEvent::registerCommand("accept", new AbortCommand("accept", Command::AbortAccept));
	ScriptEvent::registerCommand("refuse", new AbortCommand("refuse", Command::AbortRefuse));
	ScriptEvent::registerCommand("random", new RandomCommand);
	ScriptEvent::registerCommand("return", new ReturnCommand);
	
}

} // namespace script
