
#include "script/ScriptUtils.h"

#include "graphics/data/Mesh.h"
#include "io/Logger.h"
#include "platform/String.h"

using std::string;

namespace script {

static inline bool isWhitespace(char c) {
	return (((unsigned char)c) <= 32 || c == '(' || c == ')');
}

string loadUnlocalized(const std::string & str) {
	
	// if the section name has the qualifying brackets "[]", cut them off
	if(!str.empty() && str[0] == '[' && str[str.length() - 1] == ']') {
		return str.substr(1, str.length() - 2);
	}
	
	return str;
}

Context::Context(EERIE_SCRIPT * _script, size_t _pos, INTERACTIVE_OBJ * _io, ScriptMessage msg) : script(_script), pos(_pos), io(_io), message(msg) { };

string Context::getStringVar(const string & var) const {
	return GetVarValueInterpretedAsText(var, getMaster(), io);
}

std::string Context::getCommand(bool skipNewlines) {
	
	const char * esdat = script->data;
	
	skipWhitespace(skipNewlines);
	
	string word;
	
	// now take chars until it finds a space or unused char
	for(; pos != script->size && !isWhitespace(esdat[pos]); pos++) {
		
		char c = esdat[pos];
		if(c == '"') {
			LogWarning << ScriptContextPrefix(*this) << ": unexpected '\"' in command name";
		} else if(c == '~') {
			LogWarning << ScriptContextPrefix(*this) << ": unexpected '~' in command name";
		} else if(c == '\n') {
			break;
		} else if(c == '/' && pos + 1 != script->size && esdat[pos + 1] == '/') {
			pos = std::find(esdat + pos + 2, esdat + script->size, '\n') - esdat;
			if(!word.empty()) {
				break;
			}
			skipWhitespace(skipNewlines), pos--;
		} else {
			word.push_back(c);
		}
	}
	
	return word;
}

string Context::getWord() {
	
	skipWhitespace();
	
	if(pos >= script->size) {
		return string();
	}
	
	const char * esdat = script->data;
	
	bool tilde = false; // number of tildes
	
	string word;
	string var;
	
	if(pos != script->size && esdat[pos] == '"') {
		
		for(pos++; pos != script->size && esdat[pos] != '"'; pos++) {
			if(esdat[pos] == '\n') {
				if(tilde) {
					LogWarning << ScriptContextPrefix(*this) << ": unmatched '\"' before end of line";
				}
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
		}
		
		if(pos != script->size) {
			pos++;
		} else {
			LogWarning << ScriptContextPrefix(*this) << ": unmatched '\"'";
		}
		
	} else {
		
		// now take chars until it finds a space or unused char
		for(; pos != script->size && !isWhitespace(esdat[pos]); pos++) {
			
			if(esdat[pos] == '"') {
				LogWarning << ScriptContextPrefix(*this) << ": unexpected '\"' inside token";
			} else if(esdat[pos] == '~') {
				if(tilde) {
					word += GetVarValueInterpretedAsText(var, getMaster(), NULL);
					var.clear();
				}
				tilde = !tilde;
			} else if(tilde) {
				var.push_back(esdat[pos]);
			} else if(esdat[pos] == '/' && pos + 1 != script->size && esdat[pos + 1] == '/') {
				pos = std::find(esdat + pos + 2, esdat + script->size, '\n') - esdat;
				break;
			} else {
				word.push_back(esdat[pos]);
			}
			
		}
		
	}
	
	if(tilde) {
		LogWarning << ScriptContextPrefix(*this) << ": unmatched '~'";
	}
	
	return word;
}

void Context::skipWord() {
	
	skipWhitespace();
	
	const char * esdat = script->data;
	
	if(pos != script->size && esdat[pos] == '"') {
		
		for(pos++; pos != script->size && esdat[pos] != '"'; pos++) {
			if(esdat[pos] == '\n') {
				LogWarning << ScriptContextPrefix(*this) << ": missing '\"' before end of line";
				return;
			}
		}
		
		if(pos != script->size) {
			pos++;
		} else {
			LogWarning << ScriptContextPrefix(*this) << ": unmatched '\"'";
		}
		
	} else {
		
		// now take chars until it finds a space or unused char
		for(; pos != script->size && !isWhitespace(esdat[pos]); pos++) {
			if(esdat[pos] == '"') {
				LogWarning << ScriptContextPrefix(*this) << ": unexpected '\"' inside token";
			} else if(esdat[pos] == '/' && pos + 1 != script->size && esdat[pos + 1] == '/') {
				pos = std::find(esdat + pos + 2, esdat + script->size, '\n') - esdat;
				break;
			}
		}
		
	}
}

void Context::skipWhitespace(bool skipNewlines) {
	
	const char * esdat = script->data;
	
	// First ignores spaces & unused chars
	for(; pos != script->size && isWhitespace(esdat[pos]); pos++) {
		if(!skipNewlines && esdat[pos] == '\n') {
			return;
		}
	}
}

string Context::getFlags() {
	
	skipWhitespace();
	
	if(pos < script->size && script->data[pos] == '-') {
		return getWord();
	}
	
	return string();
}

float Context::getFloat() {
	return getFloatVar(getWord());
}

bool Context::getBool() {
	
	string word = getWord();
	
	return (word == "on" || word == "yes");
}

float Context::getFloatVar(const std::string & name) const {
	return GetVarValueInterpretedAsFloat(name, getMaster(), io);
}

size_t Context::skipCommand() {
	
	skipWhitespace();
	
	const char * esdat = script->data;
	
	if(pos == script->size || esdat[pos] == '\n') {
		return (size_t)-1;
	}
	
	size_t oldpos = pos;
	
	if(esdat[pos] == '/' && pos + 1 != script->size && esdat[pos + 1] == '/') {
		oldpos = (size_t)-1;
		pos += 2;
	}
	
	pos = std::find(esdat + pos, esdat + script->size, '\n') - esdat;
	
	return oldpos;
}

bool Context::jumpToLabel(const string & target, bool substack) {
	
	if(substack) {
		stack.push_back(pos);
	}
	
	long targetpos = FindScriptPos(script, ">>" + target);
	if(targetpos == -1) {
		return false;
	}
	
	pos = targetpos + target.length() + 2;
	return true;
}

bool Context::returnToCaller() {
	
	if(stack.empty()) {
		return false;
	}
	
	pos = stack.back();
	stack.pop_back();
	return true;
}

void Context::skipStatement() {
	
	string word = getCommand();
	if(pos == script->size) {
		LogWarning << ScriptContextPrefix(*this) << ": missing statement before end of script";
		return;
	}
	
	if(word == "{") {
		long brackets = 1;
		while(brackets > 0) {
			
			if(script->data[pos] == '\n') {
				pos++;
			}
			word = getWord(); // TODO should not evaluate ~var~
			if(pos == script->size) {
				LogWarning << ScriptContextPrefix(*this) << ": missing '}' before end of script";
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
	
	skipWhitespace(true);
	size_t oldpos = pos;
	word = getCommand();
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
	
	suppressions[7725]["player"]["loadanim"] = true; // bad animation id: "cast_hold"
	suppressions[8463]["player"]["loadanim"] = true; // bad animation id: "lean_left_cycle"
	suppressions[8531]["player"]["loadanim"] = true; // bad animation id: "lean_left_out"
	suppressions[8666]["player"]["loadanim"] = true; // bad animation id: "lean_right_cycle"
	suppressions[8733]["player"]["loadanim"] = true; // bad animation id: "lean_right_out"
	suppressions[9284]["player"]["loadanim"] = true; // missing animation "human_death_cool"
	suppressions[9558]["player"]["loadanim"] = true; // missing animation: "human_talk_happyneutral_headonly"
	
	suppressions[5872]["human_base"]["loadanim"] = true; // bad animation id: "bae_ready"
	suppressions[45586]["human_base"]["goto"] = true; // missing label "main_alert"
	
	suppressions[422]["light_door"]["set"] = true; // bad variable name: "durability"
	
	suppressions[162]["light_door_0030"]["setevent"] = true; // unsupported event: "npc_open"
	
	suppressions[152]["jail_wood_grid"]["set"] = true; // bad variable name: "material"
	
	suppressions[5107]["troll_base"]["loadanim"] = true; // missing animation: "troll_fight_ready_toponly"
	suppressions[5175]["troll_base"]["loadanim"] = true; // missing animation: "troll_fight_unready_toponly"
	suppressions[19054]["troll_base"]["goto"] = true; // missing label "main_alert"
	
	suppressions[93]["dragon_ice_0001"]["loadanim"] = true; // missing animation: "dragon_talk_head"
	
	suppressions[30010]["goblin_base"]["goto"] = true; // missing label "main_alert"
	
	suppressions[399]["corpse_0003"]["inventory addfromscene"] = true; // bad target ident: "magic\\potion_life\\potion_life"
	
	suppressions[274]["corpse_0084"]["inventory add"] = true; // missing object: "graph/obj3d/interactive/items/weapons/chest_leather_ac/chest_leather_ac.teo"
	
	suppressions[242]["chest_metal_0045"]["inventory addfromscene"] = true; // bad target ident: "magic\\potion_life\\potion_life"
	
	suppressions[17145]["rat_base"]["play"] = true; // sound number is sometimes too high
	
	suppressions[18479]["demon"]["play"] = true; // sound number is sometimes too high
	
	suppressions[523]["wall_breakable"]["}"] = true; // missing accept/refuse before end of event block
	
	suppressions[66]["hammer_club"]["settwohanded"] = true; // obsolete command
	
	suppressions[2410]["chicken_base"]["}"] = true; // missing accept/refuse before end of event block
	
	suppressions[2409]["pig"]["}"] = true; // missing accept/refuse before end of event block
	
	suppressions[3703]["black_thing"]["play"] = true; // variable is never set
	
}

bool isSuppressed(const Context & context, const string & command) {
	
	SuppressionsForPos::const_iterator i0 = suppressions.find(context.getPosition());
	if(i0 == suppressions.end()) {
		return false;
	}
	
	SuppressionsForFile::const_iterator i1 = i0->second.find(context.getIO() ? ((context.getScript() == &context.getIO()->script) ? context.getIO()->short_name() : context.getIO()->long_name()) : "unknown");
	if(i1 == i0->second.end()) {
		return false;
	}
	
	SuppressedCommands::const_iterator i2 = i1->second.find(command);
	
	return (i2 != i1->second.end() && i2->second);
}

}
