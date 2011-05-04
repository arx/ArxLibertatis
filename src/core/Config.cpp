/*
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

#include "core/Config.h"

#include <fstream>
#include <sstream>

#include "io/IniReader.h"
#include "io/IniWriter.h"
#include "io/Logger.h"
#include "platform/String.h"
#include "window/Input.h" // for key codes TODO remove

using std::string;
using std::istream;
using std::ifstream;
using std::ostream;
using std::ofstream;
using std::istringstream;
using std::ostringstream;
using std::boolalpha;
using std::map;

// To avoid conflicts with potential other classes/namespaces
namespace {

/* Default values for config */
namespace Default {

#undef _LITERALSTR
#undef _VALSTR
#define _LITERALSTR(x) # x
#define _VALSTR(x) _LITERALSTR(x)

#define DEFAULT_WIDTH 640
#define DEFAULT_HEIGHT 480

const std::string
	language = "english",
	resolution = _VALSTR(DEFAULT_WIDTH) "x" _VALSTR(DEFAULT_HEIGHT),
	audioBackend = "auto";

const int
	bpp = 16,
	textureSize = 2,
	meshReduction = 0,
	levelOfDetail = 2,
	fogDistance = 5,
	gamma = 5,
	luminosity = 4,
	contrast = 5,
	volume = 10,
	sfxVolume = 10,
	speechVolume = 10,
	ambianceVolume = 10,
	mouseSensitivity = 4;

const bool
	first_run = true,
	fullscreen = true,
	bumpmap = false,
	showCrosshair = true,
	antialiasing = false,
	eax = false,
	invertMouse = false,
	autoReadyWeapon = false,
	mouseLookToggle = false,
	mouseSmoothing = false,
	autoDescription = true,
	linkMouseLookToUse = false,
	forceZBias = false,
	forceToggle = false,
	gore = true,
	newControl = false;

ActionKey actions[NUM_ACTION_KEY] = {
	ActionKey(DIK_SPACE), // JUMP
	ActionKey(DIK_LCONTROL, DIK_RCONTROL), // MAGICMODE
	ActionKey(DIK_LSHIFT, DIK_RSHIFT), // STEALTHMODE
	ActionKey(DIK_W, DIK_UP), // WALKFORWARD
	ActionKey(DIK_S, DIK_DOWN), // WALKBACKWARD
	ActionKey(DIK_A), // STRAFELEFT
	ActionKey(DIK_D), // STRAFERIGHT
	ActionKey(DIK_Q), // LEANLEFT
	ActionKey(DIK_E), // LEANRIGHT
	ActionKey(DIK_X), // CROUCH
	ActionKey(DIK_F, DIK_RETURN), // MOUSELOOK
	ActionKey(DIK_BUTTON1), // ACTION
	ActionKey(DIK_I), // INVENTORY
	ActionKey(DIK_BACKSPACE), // BOOK
	ActionKey(DIK_F1), // BOOKCHARSHEET
	ActionKey(DIK_F2), // BOOKSPELL
	ActionKey(DIK_F3), // BOOKMAP
	ActionKey(DIK_F4), // BOOKQUEST
	ActionKey(DIK_H), // DRINKPOTIONLIFE
	ActionKey(DIK_G), // DRINKPOTIONMANA
	ActionKey(DIK_T), // TORCH
	ActionKey(DIK_1), // PRECAST1
	ActionKey(DIK_2), // PRECAST2
	ActionKey(DIK_3), // PRECAST3
	ActionKey(DIK_TAB, DIK_NUMPAD0), // WEAPON
	ActionKey(DIK_F9), // QUICKLOAD
	ActionKey(DIK_F5), // QUICKSAVE
	ActionKey(DIK_LEFT), // TURNLEFT
	ActionKey(DIK_RIGHT), // TURNRIGHT
	ActionKey(DIK_PGUP), // LOOKUP
	ActionKey(DIK_PGDN), // LOOKDOWN
	ActionKey(DIK_LALT), // STRAFE
	ActionKey(DIK_END), // CENTERVIEW
	ActionKey(DIK_L, DIK_BUTTON2), // FREELOOK
	ActionKey(DIK_MINUS), // PREVIOUS
	ActionKey(DIK_EQUALS), // NEXT
	ActionKey(DIK_C), // CROUCHTOGGLE
	ActionKey(DIK_B), // UNEQUIPWEAPON
	ActionKey(DIK_4), // CANCELCURSPELL
	ActionKey(DIK_R, DIK_M), // MINIMAP
};

} // namespace Default

namespace Section {

const string
	Language = "LANGUAGE",
	FirstRun = "FIRSTRUN",
	Video = "VIDEO",
	Audio = "AUDIO",
	Input = "INPUT",
	Key = "KEY",
	Misc = "MISC";
}

namespace Key {

// First run options
const string firstRun = "int";

// Language options
const string language = "string";

// Video options
const string
	resolution = "resolution",
	bpp = "bpp",
	fullscreen = "full_screen",
	bumpmap = "bump",
	textureSize = "texture",
	meshReduction = "mesh_reduction",
	levelOfDetail = "others_details",
	fogDistance = "fog",
	gamma = "gamma",
	luminosity = "luminosity",
	contrast = "contrast",
	showCrosshair = "show_crosshair",
	antialiasing = "antialiasing";

// Audio options
const string
	volume = "master_volume",
	sfxVolume = "effects_volume",
	speechVolume = "speech_volume",
	ambianceVolume = "ambiance_volume",
	eax = "EAX",
	audioBackend = "backend";

// Input options
const string
	invertMouse = "invert_mouse",
	autoReadyWeapon = "auto_ready_weapon",
	mouseLookToggle = "mouse_look_toggle",
	mouseSensitivity = "mouse_sensitivity",
	mouseSmoothing = "mouse_smoothing",
	autoDescription = "auto_description",
	linkMouseLookToUse = "link_mouse_look_to_use";

// Input key options
const string actions[NUM_ACTION_KEY] = {
	"jump",
	"magic_mode",
	"stealth_mode",
	"walk_forward",
	"walk_backward",
	"strafe_left",
	"strafe_right",
	"lean_left",
	"lean_right",
	"crouch",
	"mouselook",
	"action_combine",
	"inventory",
	"book",
	"char_sheet",
	"magic_book",
	"map",
	"quest_book",
	"drink_potion_life",
	"drink_potion_mana",
	"torch",
	"precast_1",
	"precast_2",
	"precast_3",
	"draw_weapon",
	"quickload",
	"quicksave",
	"turn_left",
	"turn_right",
	"look_up",
	"look_down",
	"strafe",
	"center_view",
	"freelook",
	"previous",
	"next",
	"crouch_toggle",
	"unequip_weapon",
	"cancel_current_spell",
	"minimap"
};

// Misc options
const string
	forceZBias = "forcezbias",
	forceToggle = "forcetoggle",
	gore = "fg",
	newControl = "newcontrol";

} // namespace Key

class ConfigReader {
	
private:
	
	IniReader ini;
	mutable map<string, InputKeyId> keyNames;
	
	InputKeyId getKeyId(const string & name) const;
	
public:
	
	ConfigReader(ifstream & input) : ini(input) { }
	
	const string & get(const string & section, const string & key, const string & defaultValue) const;
	int get(const string & section, const string & key, int defaultValue) const;
	float get(const string & section, const string & key, float defaultValue) const;
	bool get(const string & section, const string & key, bool defaultValue) const;
	ActionKey get(const string & section, ControlAction index) const;
	
};

class ConfigWriter : public IniWriter {
	
public:
	
	ConfigWriter(ostream & _output) : IniWriter(_output) { }
	
	void writeActionKey(ControlAction index, const ActionKey & value);
	
};

struct KeyDescription {
	InputKeyId id;
	std::string name;
};

// All standard keys, sorted by key ID
// + should not appear in names as it is used as a separator
static const KeyDescription keys[] = {
	{ DIK_ESCAPE, "Escape" },
	{ DIK_1, "1" },
	{ DIK_2, "2" },
	{ DIK_3, "3" },
	{ DIK_4, "4" },
	{ DIK_5, "5" },
	{ DIK_6, "6" },
	{ DIK_7, "7" },
	{ DIK_8, "8" },
	{ DIK_9, "9" },
	{ DIK_0, "0" },
	{ DIK_MINUS, "-" },
	{ DIK_EQUALS, "=" },
	{ DIK_BACK, "Backspace" },
	{ DIK_TAB, "Tab" },
	{ DIK_Q, "Q" },
	{ DIK_W, "W" },
	{ DIK_E, "E" },
	{ DIK_R, "R" },
	{ DIK_T, "T" },
	{ DIK_Y, "Y" },
	{ DIK_U, "U" },
	{ DIK_I, "I" },
	{ DIK_O, "O" },
	{ DIK_P, "P" },
	{ DIK_LBRACKET, "[" },
	{ DIK_RBRACKET, "]" },
	{ DIK_RETURN, "Return" },
	{ DIK_LCONTROL, "LeftControl" },
	{ DIK_A, "A" },
	{ DIK_S, "S" },
	{ DIK_D, "D" },
	{ DIK_F, "F" },
	{ DIK_G, "G" },
	{ DIK_H, "H" },
	{ DIK_J, "J" },
	{ DIK_K, "K" },
	{ DIK_L, "L" },
	{ DIK_SEMICOLON, ";" },
	{ DIK_APOSTROPHE, "'" },
	{ DIK_GRAVE, "`" },
	{ DIK_LSHIFT, "LeftShift" },
	{ DIK_BACKSLASH, "Backslash" },
	{ DIK_Z, "Z" },
	{ DIK_X, "X" },
	{ DIK_C, "C" },
	{ DIK_V, "V" },
	{ DIK_B, "B" },
	{ DIK_N, "N" },
	{ DIK_M, "M" },
	{ DIK_COMMA, "," },
	{ DIK_PERIOD, "." },
	{ DIK_SLASH, "/" },
	{ DIK_RSHIFT, "RightShift" },
	{ DIK_MULTIPLY, "Multiply" },
	{ DIK_LMENU, "LeftAlt" },
	{ DIK_SPACE, "Space" },
	{ DIK_CAPITAL, "Capital" },
	{ DIK_F1, "F1" },
	{ DIK_F2, "F2" },
	{ DIK_F3, "F3" },
	{ DIK_F4, "F4" },
	{ DIK_F5, "F5" },
	{ DIK_F6, "F6" },
	{ DIK_F7, "F7" },
	{ DIK_F8, "F8" },
	{ DIK_F9, "F9" },
	{ DIK_F10, "F10" },
	{ DIK_NUMLOCK, "NumLock" },
	{ DIK_SCROLL, "ScrollLock" },
	{ DIK_NUMPAD7, "Numpad7" },
	{ DIK_NUMPAD8, "Numpad8" },
	{ DIK_NUMPAD9, "Numpad9" },
	{ DIK_SUBTRACT, "Numpad-" },
	{ DIK_NUMPAD4, "Numpad4" },
	{ DIK_NUMPAD5, "Numpad5" },
	{ DIK_NUMPAD6, "Numpad6" },
	{ DIK_ADD, "NumpadPlus" },
	{ DIK_NUMPAD1, "Numpad1" },
	{ DIK_NUMPAD2, "Numpad2" },
	{ DIK_NUMPAD3, "Numpad3" },
	{ DIK_NUMPAD0, "Numpad0" },
	{ DIK_DECIMAL, "Numpad." },
	{ DIK_F11, "F11" },
	{ DIK_F12, "F12" },
	{ DIK_F13, "F13" },
	{ DIK_F14, "F14" },
	{ DIK_F15, "F15" },
	{ DIK_KANA, "Kana" },
	{ DIK_CONVERT, "Convert" },
	{ DIK_NOCONVERT, "NoConvert" },
	{ DIK_YEN, "Yen" },
	{ DIK_NUMPADEQUALS, "Numpad=" },
	{ DIK_CIRCUMFLEX, "Circumflex" },
	{ DIK_AT, "@" },
	{ DIK_COLON, ":" },
	{ DIK_UNDERLINE, "_" },
	{ DIK_KANJI, "Kanji" },
	{ DIK_STOP, "Stop" },
	{ DIK_AX, "Ax" },
	{ DIK_NUMPADENTER, "NumpadReturn" },
	{ DIK_RCONTROL, "RightControl" },
	{ DIK_NUMPADCOMMA, "Numpad," },
	{ DIK_DIVIDE, "Numpad/" },
	{ DIK_SYSRQ, "?" },
	{ DIK_RMENU, "RightAlt" },
	{ DIK_PAUSE, "Pause" },
	{ DIK_HOME, "Home" },
	{ DIK_UP, "Up" },
	{ DIK_PRIOR, "PageUp" },
	{ DIK_LEFT, "Left" },
	{ DIK_RIGHT, "Right" },
	{ DIK_END, "End" },
	{ DIK_DOWN, "Down" },
	{ DIK_NEXT, "PageDown" },
	{ DIK_INSERT, "Insert" },
	{ DIK_DELETE, "Delete" },
	{ DIK_LWIN, "LeftStart" },
	{ DIK_RWIN, "RightStart" },
	{ DIK_APPS, "AppMenu" },
	{ DIK_POWER, "Power" },
	{ DIK_SLEEP, "Sleep" },
	{ DIK_WHEELUP, "WheelUp" },
	{ DIK_WHEELDOWN, "WheelDown" }
};

static const string PREFIX_KEY = "Key_";
static const string PREFIX_BUTTON = "Button";
static const char SEPARATOR = '+';
static const string KEY_NONE = "---";

static int keyCompare(const void * a, const void * b) {
	return *((const InputKeyId *)a) - ((const KeyDescription *)b)->id;
}

static string getKeyName(InputKeyId key) {
	
	if(key == -1) {
		return string();
	}
	
	std::string name;
	
	std::string modifier;
	if(key & ~0xC000ffff) {
		// key combination
		modifier = getKeyName((key >> 16) & 0x3fff);
		key &= 0xC000ffff;
	}
	
	arx_assert(DIK_BUTTON32 > DIK_BUTTON1 && DIK_BUTTON32 - DIK_BUTTON1 == 31);
	if(key >= (InputKeyId)DIK_BUTTON1 && key <= (InputKeyId)DIK_BUTTON32) {
		
		ostringstream oss;
		oss << PREFIX_BUTTON << (int)(key - DIK_BUTTON1 + 1);
		name = oss.str();
		
	} else {
		
		const KeyDescription * entity = (const KeyDescription *)bsearch(&key, keys, sizeof(keys) / sizeof(*keys), sizeof(*keys), keyCompare);
		
		if(entity != NULL) {
			arx_assert(entity->id == key);
			name = entity->name;
		}
	}
	
	if(name.empty()) {
		ostringstream oss;
		oss << PREFIX_KEY << (int)key;
		name = oss.str();
	}
	
	if(!modifier.empty()) {
		return modifier + SEPARATOR + name;
	} else {
		return name;
	}
}

InputKeyId ConfigReader::getKeyId(const string & name) const {
	
	// If a noneset key, return -1
	if(name.empty() || name == KEY_NONE) {
		return -1;
	}
	
	size_t sep = name.find(SEPARATOR);
	if(sep != string::npos) {
		InputKeyId modifier = getKeyId(name.substr(0, sep));
		InputKeyId key = getKeyId(name.substr(sep + 1));
		if(modifier & key) {
			// bits overlap, something went wrong
			return key;
		}
		return (modifier << 16 | key);
	}
	
	if(!name.compare(0, PREFIX_KEY.length(), PREFIX_KEY)) {
		istringstream iss(name.substr(PREFIX_KEY.length()));
		int key;
		iss >> key;
		if(!iss.bad()) {
			return key;
		}
	}
	
	arx_assert(DIK_BUTTON32 > DIK_BUTTON1 && DIK_BUTTON32 - DIK_BUTTON1 == 31);
	if(!name.compare(0, PREFIX_BUTTON.length(), PREFIX_BUTTON)) {
		istringstream iss(name.substr(PREFIX_BUTTON.length()));
		int key;
		iss >> key;
		key = DIK_BUTTON1 + key - 1;
		if(!iss.bad() && key >= (int)DIK_BUTTON1 && key <= (int)DIK_BUTTON32) {
			return key;
		}
	}
	
	if(keyNames.empty()) {
		// Initialize the key name -> id map.
		for(size_t i = 0; i < sizeof(keys)/sizeof(*keys); i++) {
			keyNames[keys[i].name] = keys[i].id;
		}
	}
	
	map<string, InputKeyId>::const_iterator it = keyNames.find(name);
	if(it != keyNames.end()) {
		return it->second;
	}
	
	return -1;
}

/**
 * Reads a string from the config and returns it, returning the default value if an empty string was found
 * @param section The section to read from
 * @param key The key in the section to return
 * @param default_value The default value to be returned in the case of an empty string
 */
const string & ConfigReader::get(const string & section, const string & key, const string & defaultValue) const {
	
	const string * temp = ini.getConfigValue(section, key);
	
	if(!temp) {
		LogDebug << "[" << section << "] " << key << " = \"" << defaultValue << "\" [default]";
		return defaultValue;
	} else {
		LogDebug << "[" << section << "] " << key << " = \"" << *temp << "\"";
		return *temp;
	}
}

/**
 * Reads an int from the config and returns its converted int value,
 * return the default value if an empty string is found.
 * @param section The section to read from
 * @param key The key in the section to return
 * @param default_value The default value to return in the case of an empty string
 */
int ConfigReader::get(const string & section, const string & key, int defaultValue) const {
	
	const string * temp = ini.getConfigValue(section, key);
	if(!temp) {
		LogDebug << "[" << section << "] " << key << " = " << defaultValue << " [default]";
		return defaultValue;
	}
	
	istringstream iss(*temp);
	
	int val;
	if((iss >> val).bad()) {
		LogWarning << "bad integer value for [" << section << "] " << key << ": " << *temp << ", resetting to " << defaultValue;
		return defaultValue;
	}
	
	LogDebug << "[" << section << "] " << key << " = " << val;
	
	return val;
}

/**
 * Reads a float from the config and returns its converted int value,
 * return the default value if an empty string is found.
 * @param section The section to read from
 * @param key The key in the section to return
 * @param default_value The default value to return in the case of an empty string
 */
float ConfigReader::get(const string & section, const string & key, float defaultValue) const {
	
	const string * temp = ini.getConfigValue(section, key);
	if(!temp) {
		LogDebug << "[" << section << "] " << key << " = " << defaultValue << " [default]";
		return defaultValue;
	}
	
	istringstream iss(*temp);
	
	float val;
	if((iss >> val).bad()) {
		LogWarning << "bad float value for [" << section << "] " << key << ": " << *temp << ", resetting to " << defaultValue;
		return defaultValue;
	}
	
	LogDebug << "[" << section << "] " << key << " = " << val;
	
	return val;
}

/**
 * Reads a bool from the config and returns its converted bool value,
 * return the default value if an empty string is found.
 * @param section The section to read from
 * @param key The key in the section to return
 * @param default_value The default value to return in the case of an empty string
 */
bool ConfigReader::get(const string & section, const string & key, bool defaultValue) const {
	
	const string * temp = ini.getConfigValue(section, key);
	if(!temp) {
		LogDebug << "[" << section << "] " << key << " = " << boolalpha << defaultValue << " [default]";
		return defaultValue;
	}
	
	istringstream iss(*temp);
	
	bool val;
	if((iss >> val).bad()) {
		if((iss >> boolalpha >> val).bad()) {
			LogWarning << "bad integer value for [" << section << "] " << key << ": " << *temp << ", resetting to " << boolalpha << defaultValue;
		}
	}
	
	LogDebug << "[" << section << "] " << key << " = " << boolalpha << val;
	
	return val;
}

void ConfigWriter::writeActionKey(ControlAction index, const ActionKey & actionKey) {
	
	string v1 = getKeyName(actionKey.key[0]);
	writeKey(Key::actions[index] + "_k0", v1);
	
	string v2 = getKeyName(actionKey.key[1]);
	writeKey(Key::actions[index] + "_k1", v2);
}

ActionKey ConfigReader::get(const string & section, ControlAction index) const {
	
	const string & key = Key::actions[index];
	ActionKey action_key = Default::actions[index];
	
	const string * k0 = ini.getConfigValue(section, key + "_k0");
	if(k0) {
		InputKeyId id = getKeyId(*k0);
		if(id == -1 && !k0->empty() && *k0 != KEY_NONE) {
			LogWarning << "error parsing key name for " <<  key << "_k0: \"" << (*k0) << "\", resetting to \"" << getKeyName(action_key.key[0]) << "\"";
		} else {
			action_key.key[0] = id;
		}
	}
	
	const string * k1 = ini.getConfigValue(section, key + "_k1");
	if(k1) {
		InputKeyId id = getKeyId(*k1);
		if(id == -1 && !k1->empty() && *k1 != KEY_NONE) {
			LogWarning << "error parsing key name for " <<  key << "_k1: \"" << (*k1) << "\", resetting to \"" << getKeyName(action_key.key[1]) << "\"";
		} else {
			action_key.key[1] = id;
		}
	}
	
	LogDebug << "[" << section << "] " << key << " = \"" << getKeyName(action_key.key[0]) << "\", \"" << getKeyName(action_key.key[1]) << "\"";
	
	return action_key;
}

} // anonymous namespace

Config config;

void Config::setDefaultActionKeys() {
	
	for(size_t i = 0; i < NUM_ACTION_KEY; i++) {
		actions[i] = Default::actions[i];
	}
	
	config.input.linkMouseLookToUse = !config.misc.newControl;
}

bool Config::setActionKey(ControlAction actionId, int index, InputKeyId key) {
	
	if(actionId < 0 || actionId >= NUM_ACTION_KEY || index > 1 || index < 0) {
		return false;
	}
	
	ActionKey & action = actions[actionId];
	
	InputKeyId oldKey = action.key[index];
	action.key[index] = key;
	
	int otherIndex = 1 - index;
	
	if(action.key[otherIndex] == -1) {
		action.key[otherIndex] = oldKey;
		oldKey = -1;
	}
	
	if(action.key[otherIndex] == key) {
		action.key[otherIndex] = -1;
	}
	
	// remove double key assignments
	for(size_t i = 0; i < NUM_ACTION_KEY; i++) {
		
		if(i == actionId) {
			continue;
		}
		
		for(int k = 0; i < 2; i++) {
			if(actions[i].key[k] == key) {
				actions[i].key[k] = oldKey;
				oldKey = -1;
			}
		}
		
	}
	
	return true;
}

bool Config::save() {
	
	// Finally save it all to file/stream
	ofstream out(file.c_str());
	if(!out.is_open()) {
		return false;
	}
	
	ConfigWriter writer(out);
	
	// firstrun
	writer.beginSection(Section::FirstRun);
	writer.writeKey(Key::firstRun, firstRun);
	
	// language
	writer.beginSection(Section::Language);
	writer.writeKey(Key::language, language);
	
	// video
	writer.beginSection(Section::Video);
	ostringstream oss;
	oss << video.width << 'x' << video.height;
	writer.writeKey(Key::resolution, oss.str());
	writer.writeKey(Key::bpp, video.bpp);
	writer.writeKey(Key::fullscreen, video.fullscreen);
	writer.writeKey(Key::bumpmap, video.bumpmap);
	writer.writeKey(Key::textureSize, video.textureSize);
	writer.writeKey(Key::meshReduction, video.meshReduction);
	writer.writeKey(Key::levelOfDetail, video.levelOfDetail);
	writer.writeKey(Key::fogDistance, video.fogDistance);
	writer.writeKey(Key::gamma, video.gamma);
	writer.writeKey(Key::luminosity, video.luminosity);
	writer.writeKey(Key::contrast, video.contrast);
	writer.writeKey(Key::showCrosshair, video.showCrosshair);
	writer.writeKey(Key::antialiasing, video.antialiasing);
	
	// audio
	writer.beginSection(Section::Audio);
	writer.writeKey(Key::volume, audio.volume);
	writer.writeKey(Key::sfxVolume, audio.sfxVolume);
	writer.writeKey(Key::speechVolume, audio.speechVolume);
	writer.writeKey(Key::ambianceVolume, audio.ambianceVolume);
	writer.writeKey(Key::eax, audio.eax);
	writer.writeKey(Key::audioBackend, audio.backend);
	
	// input
	writer.beginSection(Section::Input);
	writer.writeKey(Key::invertMouse, input.invertMouse);
	writer.writeKey(Key::autoReadyWeapon, input.autoReadyWeapon);
	writer.writeKey(Key::mouseLookToggle, input.mouseLookToggle);
	writer.writeKey(Key::mouseSensitivity, input.mouseSensitivity);
	writer.writeKey(Key::mouseSmoothing, input.mouseSmoothing);
	writer.writeKey(Key::autoDescription, input.autoDescription);
	writer.writeKey(Key::linkMouseLookToUse, input.linkMouseLookToUse);
	
	// key
	writer.beginSection(Section::Key);
	for(size_t i = 0; i < NUM_ACTION_KEY; i++) {
		writer.writeActionKey((ControlAction)i, actions[i]);
	}
	
	// misc
	writer.beginSection(Section::Misc);
	writer.writeKey(Key::forceZBias, misc.forceZBias);
	writer.writeKey(Key::newControl, misc.newControl);
	writer.writeKey(Key::forceToggle, misc.forceToggle);
	writer.writeKey(Key::gore, misc.gore);
	
	return writer.flush();
}

bool Config::init(const string & file, const string & defaultFile) {
	
	this->file = file;
	
	std::ifstream ifs;
	ifs.open(file.c_str());
	if(!ifs.is_open()) {
		ifs.open(defaultFile.c_str());
	}
	bool loaded = ifs.is_open();
	
	ConfigReader reader(ifs);
	
	// Check if this is the first run of the game
	firstRun = reader.get(Section::FirstRun, Key::firstRun, Default::first_run);
	
	// Get locale language
	language = reader.get(Section::Language, Key::language, Default::language);
	
	// Get video settings
	
	string resolution = reader.get(Section::Video, Key::resolution, Default::resolution);
	istringstream iss(resolution);
	iss >> video.width;
	char x;
	iss >> x;
	iss >> video.height;
	if(x != 'x' || iss.bad() || video.width <= 0 || video.height <= 0) {
		LogWarning << "bad resolution string: " << resolution;
		video.width = DEFAULT_WIDTH;
		video.height = DEFAULT_HEIGHT;
	}
	
	video.bpp = reader.get(Section::Video, Key::bpp, Default::bpp);
	video.fullscreen = reader.get(Section::Video, Key::fullscreen, Default::fullscreen);
	video.bumpmap = reader.get(Section::Video, Key::bumpmap, Default::bumpmap);
	video.textureSize = reader.get(Section::Video, Key::textureSize, Default::textureSize);
	video.meshReduction = reader.get(Section::Video, Key::meshReduction, Default::meshReduction);
	video.levelOfDetail = reader.get(Section::Video, Key::levelOfDetail, Default::levelOfDetail);
	video.fogDistance = reader.get(Section::Video, Key::fogDistance, Default::fogDistance);
	video.gamma = reader.get(Section::Video, Key::gamma, Default::gamma);
	video.luminosity = reader.get(Section::Video, Key::luminosity, Default::luminosity);
	video.contrast = reader.get(Section::Video, Key::contrast, Default::contrast);
	video.showCrosshair = reader.get(Section::Video, Key::showCrosshair, Default::showCrosshair);
	video.antialiasing = reader.get(Section::Video, Key::antialiasing, Default::antialiasing);
	
	// Get audio settings
	audio.volume = reader.get(Section::Audio, Key::volume, Default::volume);
	audio.sfxVolume = reader.get(Section::Audio, Key::sfxVolume, Default::sfxVolume);
	audio.speechVolume = reader.get(Section::Audio, Key::speechVolume, Default::speechVolume);
	audio.ambianceVolume = reader.get(Section::Audio, Key::ambianceVolume, Default::ambianceVolume);
	audio.eax = reader.get(Section::Audio, Key::eax, Default::eax);
	audio.backend = reader.get(Section::Audio, Key::audioBackend, Default::audioBackend);
	
	// Get input settings
	input.invertMouse = reader.get(Section::Input, Key::invertMouse, Default::invertMouse);
	input.autoReadyWeapon = reader.get(Section::Input, Key::autoReadyWeapon, Default::autoReadyWeapon);
	input.mouseLookToggle = reader.get(Section::Input, Key::mouseLookToggle, Default::mouseLookToggle);
	input.mouseSensitivity = reader.get(Section::Input, Key::mouseSensitivity, Default::mouseSensitivity);
	input.mouseSmoothing = reader.get(Section::Input, Key::mouseSmoothing, Default::mouseSmoothing);
	input.autoDescription = reader.get(Section::Input, Key::autoDescription, Default::autoDescription);
	input.linkMouseLookToUse = reader.get(Section::Input, Key::linkMouseLookToUse, Default::linkMouseLookToUse);
	
	// Get action key settings
	for(size_t i = 0; i < NUM_ACTION_KEY; i++) {
		actions[i] = reader.get(Section::Key, (ControlAction)i);
	}
	
	// Get miscellaneous settings
	misc.forceZBias = reader.get(Section::Misc, Key::forceZBias, Default::forceZBias);
	misc.forceToggle = reader.get(Section::Misc, Key::forceToggle, Default::forceToggle);
	misc.gore = reader.get(Section::Misc, Key::gore, Default::gore);
	misc.newControl = reader.get(Section::Misc, Key::newControl, Default::newControl);
	
	return loaded;
}
