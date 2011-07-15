
#include "script/ScriptUtils.h"

#include "graphics/data/Mesh.h"
#include "io/Logger.h"
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

string Context::getStringVar(const string & var) const {
	return GetVarValueInterpretedAsText(var, getMaster(), io);
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
						word += GetVarValueInterpretedAsText(var, getMaster(), NULL);
						var.clear();
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
				word += GetVarValueInterpretedAsText(var, getMaster(), NULL);
				var.clear();
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
	return getFloatVar(getLowercase());
}

string Context::getLowercase() {
	return toLowercase(getWord());
}

bool Context::getBool() {
	
	string word = getLowercase();
	
	return (word == "on" || word == "yes");
}

float Context::getFloatVar(const std::string & name) const {
	return GetVarValueInterpretedAsFloat(name, getMaster(), io);
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
	
	if(substack && !InSubStack(script, pos)) {
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

namespace {

typedef std::map<string, bool> SuppressedCommands;
typedef std::map<string, SuppressedCommands> SuppressionsForFile;
typedef std::map<size_t, SuppressionsForFile> SuppressionsForPos;

SuppressionsForPos suppressions;

}

void initSuppressions() {
	
	// TODO(broken-scripts)
	
	suppressions[7725]["Player"]["loadanim"] = true;
	suppressions[8463]["Player"]["loadanim"] = true;
	suppressions[8531]["Player"]["loadanim"] = true;
	suppressions[8666]["Player"]["loadanim"] = true;
	suppressions[8733]["Player"]["loadanim"] = true;
	suppressions[9284]["Player"]["loadanim"] = true;
	suppressions[9558]["Player"]["loadanim"] = true;
	
	suppressions[5872]["human_base"]["loadanim"] = true;
	
	suppressions[422]["light_door"]["set"] = true;
	
	suppressions[152]["jail_wood_grid"]["set"] = true;
	
	suppressions[5107]["troll_base"]["loadanim"] = true;
	suppressions[5175]["troll_base"]["loadanim"] = true;
	
	suppressions[93]["dragon_ice_0001"]["loadanim"] = true;
	
}

bool isSuppressed(const Context & context, const Command & command) {
	
	SuppressionsForPos::const_iterator i0 = suppressions.find(context.getPosition());
	if(i0 == suppressions.end()) {
		return false;
	}
	
	SuppressionsForFile::const_iterator i1 = i0->second.find(context.getIO() ? ((context.getScript() == &context.getIO()->script) ? context.getIO()->short_name() : context.getIO()->long_name()) : "unknown");
	if(i1 == i0->second.end()) {
		return false;
	}
	
	SuppressedCommands::const_iterator i2 = i1->second.find(command.getName());
	
	return (i2 != i1->second.end() && i2->second);
}

}
