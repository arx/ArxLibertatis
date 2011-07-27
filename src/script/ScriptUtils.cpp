
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

void suppress(const string & script, size_t pos, const string & command) {
	suppressions[pos][script][command] = true;
}

void initSuppressions() {
	
	// TODO(broken-scripts)
	// TODO move to external file
	
	suppress("axe_2handed", 26, "settwohanded"); // obsolete command
	
	suppress("black_thing", 3703, "play"); // variable is never set
	
	suppress("chest_metal_0020", 54, "inventory add"); // missing object: "graph/obj3d/interactive/items/provisions/candle/candle.teo" (should be 'candle/candel'?)
	suppress("chest_metal_0020", 99, "inventory add"); // missing object: "graph/obj3d/interactive/items/provisions/candle/candle.teo" (should be 'candle/candel'?)
	suppress("chest_metal_0020", 224, "inventory add"); // missing object: "graph/obj3d/interactive/items/magic/ring_darkaa/ring_darkaa.teo" (should be 'ring_daarka/ring_daarka'?)
	
	suppress("chest_metal_0029", 224, "inventory add"); // missing object: "graph/obj3d/interactive/items/provisions/candle/candle.teo" (should be 'candle/candel'?)
	suppress("chest_metal_0029", 317, "inventory add"); // missing object: "graph/obj3d/interactive/items/provisions/candle/candle.teo" (should be 'candle/candel'?)
	suppress("chest_metal_0029", 461, "inventory add"); // missing object: "graph/obj3d/interactive/items/provisions/candle/candle.teo" (should be 'candle/candel'?)
	suppress("chest_metal_0029", 557, "inventory add"); // missing object: "graph/obj3d/interactive/items/provisions/candle/candle.teo" (should be 'candle/candel'?)
	suppress("chest_metal_0029", 650, "inventory add"); // missing object: "graph/obj3d/interactive/items/provisions/candle/candle.teo" (should be 'candle/candel'?)
	
	suppress("chest_metal_0045", 242, "inventory addfromscene"); // bad target ident: "magic\\potion_life\\potion_life"
	
	suppress("chest_metal_0095", 143, "inventory add"); // missing object: "graph/obj3d/interactive/items/armor/legging_leatherac/legging_leatherac.teo" (should be 'legging_leather_ac'?)
	
	suppress("chicken_base", 2410, "}"); // missing accept/refuse before end of event block
	
	suppress("corpse_0003", 399, "inventory addfromscene"); // bad target ident: "magic\\potion_life\\potion_life" (should be 'inventory add'?)
	
	suppress("corpse_0084", 274, "inventory add"); // missing object: "graph/obj3d/interactive/items/weapons/chest_leather_ac/chest_leather_ac.teo"
	
	suppress("demon", 18479, "play"); // sound number is sometimes too high
	
	suppress("dog", 19669, "play"); // sound number is sometimes too high
	
	suppress("dog_0011", 31, "playanim"); // animation 'action2' not loaded
	
	suppress("dragon_ice_0001", 93, "loadanim"); // missing animation: "dragon_talk_head"
	
	suppress("gem_inwall", 114, "play"); // unknown flag -e (ignored)
	
	suppress("goblin_base", 30010, "goto"); // missing label "main_alert"
	
	suppress("goblin_base_0046", 2924, "if"); // unknown operator '=>' (should be '>='?), interpreted as constant true
	
	suppress("hammer_club", 66, "settwohanded"); // obsolete command
	
	suppress("human_base", 5872, "loadanim"); // bad animation id: "bae_ready"
	suppress("human_base", 13711, "loadanim"); // missing animation "child_get_hit", should be "child_hit"?
	suppress("human_base", 13751, "loadanim"); // missing animation "child_get_hit", should be "child_hit"?
	suppress("human_base", 45586, "goto"); // missing label "main_alert"
	
	suppress("human_base_0079", 239, "inventory add"); // missing object: "graph/obj3d/interactive/items/armor/chest_leatherac/chest_leatherac.teo" (should be 'chest_leather_ac'?)
	suppress("human_base_0079", 303, "inventory add"); // missing object: "graph/obj3d/interactive/items/armor/leggings_leatherac/leggings_leatherac.teo" (should be 'legging_leather_ac'?)
	
	suppress("human_base_0095", 722, "setcontrolledzone"); // unknown zone 'maria_shop'
	
	suppress("human_base_0118", 101, "collisions"); // unknown command 'collisions', should be 'collision'
	
	suppress("jail_wood_grid", 152, "set"); // bad variable name: "material"
	
	suppress("light_door", 422, "set"); // bad variable name: "durability"
	
	suppress("light_door_0019", 105, "setspeakpitch"); // setspeakpitch only applies to NPCs
	
	suppress("light_door_0020", 230, "setspeakpitch"); // setspeakpitch only applies to NPCs
	
	suppress("light_door_0021", 234, "setspeakpitch"); // setspeakpitch only applies to NPCs
	
	suppress("light_door_0029", 88, "setspeakpitch"); // setspeakpitch only applies to NPCs
	
	suppress("light_door_0030", 162, "setevent"); // unsupported event: "npc_open"
	suppress("light_door_0030", 488, "setevent"); // unsupported event: "npc_open"
	
	suppress("light_door_0100", 69, "setspeakpitch"); // setspeakpitch only applies to NPCs
	
	suppress("light_door_0102", 88, "setspeakpitch"); // setspeakpitch only applies to NPCs
	
	suppress("light_door_0106", 110, "setcontrolledzone"); // unknown zone 'city_entrance'
	
	suppress("light_door_0121", 88, "setspeakpitch"); // setspeakpitch only applies to NPCs
	
	suppress("marker_0025", 288, "sendevent"); // unknown zone 'cooking' (should be 'cook_gary'?)
	
	suppress("pig", 2409, "}"); // missing accept/refuse before end of event block
	
	suppress("player", 7725, "loadanim"); // bad animation id: "cast_hold"
	suppress("player", 8463, "loadanim"); // bad animation id: "lean_left_cycle"
	suppress("player", 8531, "loadanim"); // bad animation id: "lean_left_out"
	suppress("player", 8666, "loadanim"); // bad animation id: "lean_right_cycle"
	suppress("player", 8733, "loadanim"); // bad animation id: "lean_right_out"
	suppress("player", 9284, "loadanim"); // missing animation "human_death_cool"
	suppress("player", 9558, "loadanim"); // missing animation "human_talk_happyneutral_headonly"
	
	suppress("rat_base", 17145, "play"); // sound number is sometimes too high
	
	suppress("rat_base_0059", 62, "behavior"); // unknown behavior 'firendly', should be 'friendly'
	suppress("rat_base_0059", 160, "behavior"); // unknown behavior 'firendly', should be 'friendly'
	
	suppress("secret_door_council_2b", 609, "}"); // extraneous '}'
	
	suppress("snake_woman_base", 26358, "goto"); // missing label 'main_alert'
	
	suppress("snake_woman_base_0010", 122, "collions"); // unknown command 'collions', should be 'collision'
	
	suppress("troll_base", 5107, "loadanim"); // missing animation: "troll_fight_ready_toponly"
	suppress("troll_base", 5175, "loadanim"); // missing animation: "troll_fight_unready_toponly"
	suppress("troll_base", 19054, "goto"); // missing label "main_alert"
	
	suppress("wall_breakable", 523, "}"); // missing accept/refuse before end of event block
	
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
