/*
 * Copyright 2011-2016 Arx Libertatis Team (see the AUTHORS file)
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

#include "script/ScriptedVariable.h"

#include <cstring>

#include "game/Entity.h"
#include "graphics/data/Mesh.h"
#include "script/ScriptEvent.h"
#include "script/ScriptUtils.h"


namespace script {

namespace {

class SetCommand : public Command {
	
public:
	
	SetCommand() : Command("set") { }
	
	Result execute(Context & context) {
		
		std::string var = context.getWord();
		std::string val = context.getWord();
		
		DebugScript(' ' << var << " \"" << val << '"');
		
		if(var.empty()) {
			ScriptWarning << "Missing variable name";
			return Failed;
		}
		
		SCRIPT_VARIABLES & variables = isLocalVariable(var) ? context.getMaster()->lvar : svar;
		
		SCRIPT_VAR * sv = NULL;
		switch(var[0]) {
			
			case '$':      // global text
			case '\xA3': { // local text
				sv = SETVarValueText(variables, var, context.getStringVar(val));
				break;
			}
			
			case '#':      // global long
			case '\xA7': { // local long
				sv = SETVarValueLong(variables, var, long(context.getFloatVar(val)));
				break;
			}
			
			case '&':      // global float
			case '@': {    // local float
				sv = SETVarValueFloat(variables, var, context.getFloatVar(val));
				break;
			}
			
			default: {
				ScriptWarning << "Unknown variable type: " << var;
				return Failed;
			}
			
		}
		
		if(!sv) {
			ScriptWarning << "Unable to set variable " << var;
			return Failed;
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
	
	ArithmeticCommand(const std::string & name, Operator _op) : Command(name), op(_op) { }
	
	Result execute(Context & context) {
		
		std::string var = context.getWord();
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
				float old = (float)GETVarValueLong(svar, var);
				SCRIPT_VAR * sv = SETVarValueLong(svar, var, (long)calculate(old, val));
				if(!sv) {
					ScriptWarning << "unable to set var " << var;
					return Failed;
				}
				break;
			}
			
			case '\xA7': { // local long
				float old = (float)GETVarValueLong(es->lvar, var);
				SCRIPT_VAR * sv = SETVarValueLong(es->lvar, var, (long)calculate(old, val));
				if(!sv) {
					ScriptWarning << "unable to set var " << var;
					return Failed;
				}
				break;
			}
			
			case '&': { // global float
				float old = GETVarValueFloat(svar, var);
				SCRIPT_VAR * sv = SETVarValueFloat(svar, var, calculate(old, val));
				if(!sv) {
					ScriptWarning << "unable to set var " << var;
					return Failed;
				}
				break;
			}
			
			case '@': { // local float
				float old = GETVarValueFloat(es->lvar, var);
				SCRIPT_VAR * sv = SETVarValueFloat(es->lvar, var, calculate(old, val));
				if(!sv) {
					ScriptWarning << "unable to set var " << var;
					return Failed;
				}
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
	
	static bool isGlobal(char c) {
		return (c == '$' || c == '#' || c == '&');
	}
	
	// TODO move to variable context
	static bool UNSETVar(SCRIPT_VARIABLES & svf, const std::string & name) {
		
		SCRIPT_VARIABLES::iterator it;
		for(it = svf.begin(); it != svf.end(); ++it) {
			if(name == it->name) {
				break;
			}
		}
		
		if(it == svf.end())
			return false;
		
		svf.erase(it);
		
		return true;
	}
	
public:
	
	UnsetCommand() : Command("unset") { }
	
	Result execute(Context & context) {
		
		std::string var = context.getWord();
		
		DebugScript(' ' << var);
		
		if(var.empty()) {
			ScriptWarning << "missing variable name";
			return Failed;
		}
		
		if(isGlobal(var[0])) {
			UNSETVar(svar, var);
		} else {
			UNSETVar(context.getMaster()->lvar, var);
		}
		
		return Success;
	}
	
};

class IncrementCommand : public Command {
	
	float diff;
	
public:
	
	IncrementCommand(const std::string & name, float _diff) : Command(name), diff(_diff) { }
	
	Result execute(Context & context) {
		
		std::string var = context.getWord();
		
		DebugScript(' ' << var);
		
		if(var.empty()) {
			ScriptWarning << "missing variable name";
			return Failed;
		}
		
		EERIE_SCRIPT & es = *context.getMaster();
		
		switch(var[0]) {
			
			case '#': {
				long ival = GETVarValueLong(svar, var);
				SETVarValueLong(svar, var, ival + (long)diff);
				break;
			}
			
			case '\xA3': {
				long ival = GETVarValueLong(es.lvar, var);
				SETVarValueLong(es.lvar, var, ival + (long)diff);
				break;
			}
			
			case '&': {
				float fval = GETVarValueFloat(svar, var);
				SETVarValueFloat(svar, var, fval + diff);
				break;
			}
			
			case '@': {
				float fval = GETVarValueFloat(es.lvar, var);
				SETVarValueFloat(es.lvar, var, fval + diff);
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

} // anonymous namespace

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
