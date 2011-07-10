
#include "script/ScriptUtils.h"

#include "platform/String.h"

using std::string;

namespace script {

string loadUnlocalized(const std::string & str) {
	
	// if the section name has the qualifying brackets "[]", cut them off
	if(!str.empty() && str[0] == '[' && str[str.length() - 1] == ']') {
		return str.substr(1, str.length() - 2);
	}
	
	return str;
}

Context::Context(EERIE_SCRIPT * _script, size_t _pos, INTERACTIVE_OBJ * _io) : script(_script), pos(_pos), io(_io) { };

string Context::getStringVar(const string & var) {
	return GetVarValueInterpretedAsText(var, script, io);
}

string Context::getWord() {
	
	skipWhitespace();
	
	if(pos >= script->size) {
		return string();
	}
	
	const char * esdat = script->data;
	
	bool tilde = false; // number of tildes
	
	string word;
	string var; // TODO(case-sensitive) should sometimes be lowercased
	
	// now take chars until it finds a space or unused char
	while(((unsigned char)esdat[pos]) > 32 && esdat[pos] != '(' && esdat[pos] != ')') {
		
		if(esdat[pos] == '"') {
			pos++;
			if(pos == script->size) {
				return word;
			}
			
			while(esdat[pos] != '"') {
				if(esdat[pos] == '\n') {
					return word;
				} else if(esdat[pos] == '~') {
					if(tilde) {
						if(script->master) {
							word += GetVarValueInterpretedAsText(var, script->master, NULL);
						} else {
							word += GetVarValueInterpretedAsText(var, script, NULL);
						}
					}
					tilde = !tilde;
				} else if(tilde) {
					var.push_back(esdat[pos]);
				} else {
					word.push_back(esdat[pos]);
				}
				pos++;
				if(pos == script->size) {
					return word;
				}
			}
			
			pos++;
			return word;
			
		} else if(esdat[pos] == '~') {
			if(tilde) {
				if(script->master) {
					word += GetVarValueInterpretedAsText(var, script->master, NULL);
				} else {
					word += GetVarValueInterpretedAsText(var, script, NULL);
				}
			}
			tilde = !tilde;
		} else if(tilde) {
			var.push_back(esdat[pos]);
		} else {
			word.push_back(esdat[pos]);
		}
		
		pos++;
		if(pos == script->size) {
			return word;
		}
	}
	
	return word;
}

void Context::skipWord() {
	
	skipWhitespace();
	
	if(pos >= script->size) {
		return;
	}
	
	const char * esdat = script->data;
	
	// now take chars until it finds a space or unused char
	while(((unsigned char)esdat[pos]) > 32 && esdat[pos] != '(' && esdat[pos] != ')') {
		
		if(esdat[pos] == '"') {
			pos++;
			if(pos == script->size) {
				return;
			}
			
			while(esdat[pos] != '"') {
				if(esdat[pos] == '\n') {
					return;
				}
				pos++;
				if(pos == script->size) {
					return;
				}
			}
			
			pos++;
			return;
			
		}
		
		pos++;
		if(pos == script->size) {
			return;
		}
	}
	
	return;
}

void Context::skipWhitespace() {
	
	const char * esdat = script->data;
	
	// First ignores spaces & unused chars
	while(pos < script->size && (((unsigned char)esdat[pos]) <= 32 || esdat[pos] == '(' || esdat[pos] == ')')) {
		if(esdat[pos] == '\n') {
			return;
		}
		pos++;
	}
}

string Context::getFlags() {
	
	skipWhitespace();
	
	if(pos < script->size && script->data[pos] == '-') {
		return getLowercase();
	}
	
	return string();
}

float Context::getFloat() {
	return GetVarValueInterpretedAsFloat(getLowercase(), script, io);
}

string Context::getLowercase() {
	return toLowercase(getWord());
}

bool Context::getBool() {
	
	string word = getLowercase();
	
	return (word == "on" || word == "yes");
}

float Context::getFloatVar(const std::string & name) {
	return GetVarValueInterpretedAsFloat(name, script, io);
}

size_t Context::skipCommand() {
	
	skipWhitespace();
	
	const char * esdat = script->data;
	
	if(pos >= script->size || esdat[pos] == '\n') {
		return (size_t)-1;
	}
	
	size_t oldpos = pos;
	
	if(esdat[pos] == '/' && pos + 1 < script->size && esdat[pos + 1] == '/') {
		oldpos = (size_t)-1;
		pos += 2;
	}
	
	while(pos < script->size && esdat[pos] != '\n') {
		pos++;
	}
	
	return oldpos;
}

bool Context::jumpToLabel(const string & target, bool substack) {
	
	if(!substack && !InSubStack(script, pos)) {
		return false;
	}
	
	long targetpos = FindLabelPos(script, target);
	if(targetpos == -1) {
		return false;
	}
	
	pos = targetpos;
	return true;
}

bool Context::returnToCaller() {
	
	long oldpos = GetSubStack(script);
	if(oldpos == -1) {
		return false;
	}
	
	pos = oldpos;
	return true;
}

void Context::skipStatement() {
	
	string word = getWord();
	if(pos == script->size) {
		return;
	}
	
	while(word.empty() && pos != script->size && script->data[pos] == '\n') {
		pos++;
		word = getWord();
	}
	
	if(word == "{") {
		long brackets = 1;
		while(brackets > 0) {
			
			if(script->size && script->data[pos] == '\n') {
				pos++;
			}
			word = getWord();
			if(pos == script->size) {
				return;
			}
			
			if(word == "{") {
				brackets++;
			} else if(word == "}") {
				brackets--;
			}
		}
	} else {
		skipCommand();
	}
	
	size_t oldpos = pos;
	if(pos != script->size && script->data[pos] == '\n') {
		do {
			pos++;
			word = getWord();
		} while(word.empty() && pos != script->size && script->data[pos] == '\n');
	}
	word = getLowercase();
	if(word != "else") {
		pos = oldpos;
	}
}

}