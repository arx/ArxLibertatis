
#include "script/ScriptedVariable.h"

#include "ai/Paths.h"
#include "core/GameTime.h"
#include "game/Inventory.h"
#include "graphics/Math.h"
#include "platform/String.h"
#include "scene/Interactive.h"
#include "script/ScriptEvent.h"
#include "script/ScriptUtils.h"

using std::string;

namespace script {

namespace {

class SetCommand : public Command {
	
public:
	
	SetCommand() : Command("set") { }
	
	Result execute(Context & context) {
		
		HandleFlags("a") {
			if(flg & flag('a')) {
				ScriptWarning << "broken 'set -a' script command used";
			}
		}
		
		string var = context.getWord();
		string val = context.getWord();
		
		DebugScript(' ' << var << " \"" << val << '"');
		
		if(var.empty()) {
			ScriptWarning << "missing var name";
			return Failed;
		}
		
		EERIE_SCRIPT & es = *context.getMaster();
		
		switch(var[0]) {
			
			case '$': { // global text
				string v = context.getStringVar(val);
				SCRIPT_VAR * sv = SETVarValueText(svar, NB_GLOBALS, var, v);
				if(!sv) {
					ScriptWarning << "unable to set var " << var << " to \"" << v << '"';
					return Failed;
				}
				sv->type = TYPE_G_TEXT;
				break;
			}
			
			case '\xA3': { // local text
				string v = context.getStringVar(val);
				SCRIPT_VAR * sv = SETVarValueText(es.lvar, es.nblvar, var, v);
				if(!sv) {
					ScriptWarning << "unable to set var " << var << " to \"" << v << '"';
					return Failed;
				}
				sv->type = TYPE_L_TEXT;
				break;
			}
			
			case '#': { // global long
				long v = (long)context.getFloatVar(val);
				SCRIPT_VAR * sv = SETVarValueLong(svar, NB_GLOBALS, var, v);
				if(!sv) {
					ScriptWarning << "unable to set var " << var << " to \"" << v << '"';
					return Failed;
				}
				sv->type = TYPE_G_LONG;
				break;
			}
			
			case '\xA7': { // local long
				long v = (long)context.getFloatVar(val);
				SCRIPT_VAR * sv = SETVarValueLong(es.lvar, es.nblvar, var, v);
				if(!sv) {
					ScriptWarning << "unable to set var " << var << " to \"" << v << '"';
					return Failed;
				}
				sv->type = TYPE_L_LONG;
				break;
			}
			
			case '&': { // global float
				float v = context.getFloatVar(val);
				SCRIPT_VAR * sv = SETVarValueFloat(svar, NB_GLOBALS, var, v);
				if(!sv) {
					ScriptWarning << "unable to set var " << var << " to \"" << v << '"';
					return Failed;
				}
				sv->type = TYPE_G_FLOAT;
				break;
			}
			
			case '@': { // local float
				float v = context.getFloatVar(val);
				SCRIPT_VAR * sv = SETVarValueFloat(es.lvar, es.nblvar, var, v);
				if(!sv) {
					ScriptWarning << "unable to set var " << var << " to \"" << v << '"';
					return Failed;
				}
				sv->type = TYPE_L_FLOAT;
				break;
			}
			
			default: {
				ScriptWarning << "unknown variable type: " << var;
				return Failed;
			}
			
		}
		
		return Success;
	}
	
};

class ArithmeticCommand : public Command {
	
public:
	
	enum Operator {
		Add,
		Subtract,
		Multiply,
		Divide
	};
	
private:
	
	float calculate(float left, float right) {
		switch(op) {
			case Add: return left + right;
			case Subtract: return left - right;
			case Multiply: return left * right;
			case Divide: return (right == 0.f) ? 0.f : left / right;
		}
		arx_assert_msg(false, "Invalid op used in ArithmeticCommand: %d", (int)op);
		return 0.f;
	}
	
	Operator op;
	
public:
	
	ArithmeticCommand(const string & name, Operator _op) : Command(name), op(_op) { }
	
	Result execute(Context & context) {
		
		string var = context.getWord();
		float val = context.getFloat();
		
		DebugScript(' ' << var << ' ' << val);
		
		if(var.empty()) {
			ScriptWarning << "missing variable name";
			return Failed;
		}
		
		EERIE_SCRIPT * es = context.getMaster();
		
		switch(var[0]) {
			
			case '$': // global text
			case '\xA3': { // local text
				ScriptWarning << "cannot increment string variables";
				return Failed;
			}
			
			case '#':  {// global long
				float old = (float)GETVarValueLong(svar, NB_GLOBALS, var);
				SCRIPT_VAR * sv = SETVarValueLong(svar, NB_GLOBALS, var, (long)calculate(old, val));
				if(!sv) {
					ScriptWarning << "unable to set var " << var;
					return Failed;
				}
				sv->type = TYPE_G_LONG;
				break;
			}
			
			case '\xA7': { // local long
				float old = (float)GETVarValueLong(es->lvar, es->nblvar, var);
				SCRIPT_VAR * sv = SETVarValueLong(es->lvar, es->nblvar, var, (long)calculate(old, val));
				if(!sv) {
					ScriptWarning << "unable to set var " << var;
					return Failed;
				}
				sv->type = TYPE_L_LONG;
				break;
			}
			
			case '&': { // global float
				float old = GETVarValueFloat(svar, NB_GLOBALS, var);
				SCRIPT_VAR * sv = SETVarValueFloat(svar, NB_GLOBALS, var, calculate(old, val));
				if(!sv) {
					ScriptWarning << "unable to set var " << var;
					return Failed;
				}
				sv->type = TYPE_G_FLOAT;
				break;
			}
			
			case '@': { // local float
				float old = GETVarValueFloat(es->lvar, es->nblvar, var);
				SCRIPT_VAR * sv = SETVarValueFloat(es->lvar, es->nblvar, var, calculate(old, val));
				if(!sv) {
					ScriptWarning << "unable to set var " << var;
					return Failed;
				}
				sv->type = TYPE_L_FLOAT;
				break;
			}
			
			default: {
				ScriptWarning << "unknown variable type: " << var;
				return Failed;
			}
			
		}
		
		return Success;
	}
	
};

class UnsetCommand : public Command {
	
	static long GetVarNum(SCRIPT_VAR * svf, size_t nb, const string & name) {
		
		if(!svf) {
			return -1;
		}
		
		for(size_t i = 0; i < nb; i++) {
			if(svf[i].type != 0 && name == svf[i].name) {
				return i;
			}
		}
		
		return -1;
	}
	
	static bool isGlobal(char c) {
		return (c == '$' || c == '#' || c == '&');
	}
	
	// TODO move to variable context
	static bool UNSETVar(SCRIPT_VAR * & svf, long & nb, const string & name) {
		
		long i = GetVarNum(svf, nb, name);
		if(i < 0) {
			return false;
		}
		
		if(svf[i].text) {
			free((void *)svf[i].text), svf[i].text = NULL;
		}
		
		if(i + 1 < nb) {
			memcpy(&svf[i], &svf[i + 1], sizeof(SCRIPT_VAR) * (nb - i - 1));
		}
		
		svf = (SCRIPT_VAR *)realloc(svf, sizeof(SCRIPT_VAR) * (nb - 1));
		nb--;
		return true;
	}
	
public:
	
	UnsetCommand() : Command("unset") { }
	
	Result execute(Context & context) {
		
		string var = context.getWord();
		
		DebugScript(' ' << var);
		
		if(var.empty()) {
			ScriptWarning << "missing variable name";
			return Failed;
		}
		
		if(isGlobal(var[0])) {
			UNSETVar(svar, NB_GLOBALS, var);
		} else {
			UNSETVar(context.getMaster()->lvar, context.getMaster()->nblvar, var);
		}
		
		return Success;
	}
	
};

class IncrementCommand : public Command {
	
	float diff;
	
public:
	
	IncrementCommand(const string & name, float _diff) : Command(name), diff(_diff) { }
	
	Result execute(Context & context) {
		
		string var = context.getWord();
		
		DebugScript(' ' << var);
		
		if(var.empty()) {
			ScriptWarning << "missing variable name";
			return Failed;
		}
		
		EERIE_SCRIPT& es = *context.getMaster();
		
		switch(var[0]) {
			
			case '#': {
				long ival = GETVarValueLong(svar, NB_GLOBALS, var);
				SETVarValueLong(svar, NB_GLOBALS, var, ival + (long)diff);
				break;
			}
			
			case '\xA3': {
				long ival = GETVarValueLong(es.lvar, es.nblvar, var);
				SETVarValueLong(es.lvar, es.nblvar, var, ival + (long)diff);
				break;
			}
			
			case '&': {
				float fval = GETVarValueFloat(svar, NB_GLOBALS, var);
				SETVarValueFloat(svar, NB_GLOBALS, var, fval + diff);
				break;
			}
			
			case '@': {
				float fval = GETVarValueFloat(es.lvar, es.nblvar, var);
				SETVarValueFloat(es.lvar, es.nblvar, var, fval + diff);
				break;
			}
			
			default: {
				ScriptWarning << "can only use " << getName() << " with number variables, got " << var;
				return Failed;
			}
			
		}
		
		return Success;
	}
	
};

}

void setupScriptedVariable() {
	
	ScriptEvent::registerCommand(new SetCommand);
	ScriptEvent::registerCommand(new ArithmeticCommand("inc", ArithmeticCommand::Add));
	ScriptEvent::registerCommand(new ArithmeticCommand("dec", ArithmeticCommand::Subtract));
	ScriptEvent::registerCommand(new ArithmeticCommand("mul", ArithmeticCommand::Multiply));
	ScriptEvent::registerCommand(new ArithmeticCommand("div", ArithmeticCommand::Divide));
	ScriptEvent::registerCommand(new UnsetCommand);
	ScriptEvent::registerCommand(new IncrementCommand("++", 1.f));
	ScriptEvent::registerCommand(new IncrementCommand("--", -1.f));
	
}

} // namespace script
