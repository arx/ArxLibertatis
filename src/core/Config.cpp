/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#include "core/Config.h"

#include <stddef.h>
#include <algorithm>
#include <sstream>

#include <boost/preprocessor/stringize.hpp>

#include "input/Input.h"
#include "input/Keyboard.h"
#include "input/Mouse.h"
#include "io/fs/FileStream.h"
#include "io/IniReader.h"
#include "io/IniSection.h"
#include "io/IniWriter.h"
#include "io/log/Logger.h"
#include "math/Vector.h"
#include "platform/CrashHandler.h"

using std::string;

// To avoid conflicts with potential other classes/namespaces
namespace {

/* Default values for config */
namespace Default {

#define ARX_DEFAULT_WIDTH 640
#define ARX_DEFAULT_HEIGHT 480

const string
	language = string(),
	resolution = "auto",
	audioBackend = "auto",
	windowFramework = "auto",
	windowSize = BOOST_PP_STRINGIZE(ARX_DEFAULT_WIDTH) "x"
	             BOOST_PP_STRINGIZE(ARX_DEFAULT_HEIGHT),
	debugLevels = "";

const int
	levelOfDetail = 2,
	fogDistance = 10,
	volume = 10,
	sfxVolume = 10,
	speechVolume = 10,
	ambianceVolume = 10,
	mouseSensitivity = 6,
	migration = Config::OriginalAssets,
	quicksaveSlots = 3;

const bool
	fullscreen = true,
	showCrosshair = true,
	antialiasing = true,
	vsync = true,
	eax = false,
	invertMouse = false,
	autoReadyWeapon = false,
	mouseLookToggle = true,
	autoDescription = true,
	linkMouseLookToUse = false,
	forceToggle = false;

ActionKey actions[NUM_ACTION_KEY] = {
	ActionKey(Keyboard::Key_Spacebar), // JUMP
	ActionKey(Keyboard::Key_LeftCtrl, Keyboard::Key_RightCtrl), // MAGICMODE
	ActionKey(Keyboard::Key_LeftShift, Keyboard::Key_RightShift), // STEALTHMODE
	ActionKey(Keyboard::Key_W, Keyboard::Key_UpArrow), // WALKFORWARD
	ActionKey(Keyboard::Key_S, Keyboard::Key_DownArrow), // WALKBACKWARD
	ActionKey(Keyboard::Key_A), // STRAFELEFT
	ActionKey(Keyboard::Key_D), // STRAFERIGHT
	ActionKey(Keyboard::Key_Q), // LEANLEFT
	ActionKey(Keyboard::Key_E), // LEANRIGHT
	ActionKey(Keyboard::Key_X), // CROUCH
	ActionKey(Keyboard::Key_F, Keyboard::Key_Enter), // USE
	ActionKey(Mouse::Button_0), // ACTION
	ActionKey(Keyboard::Key_I), // INVENTORY
	ActionKey(Keyboard::Key_Backspace), // BOOK
	ActionKey(Keyboard::Key_F1), // BOOKCHARSHEET
	ActionKey(Keyboard::Key_F2), // BOOKSPELL
	ActionKey(Keyboard::Key_F3), // BOOKMAP
	ActionKey(Keyboard::Key_F4), // BOOKQUEST
	ActionKey(Keyboard::Key_H), // DRINKPOTIONLIFE
	ActionKey(Keyboard::Key_G), // DRINKPOTIONMANA
	ActionKey(Keyboard::Key_T), // TORCH
	ActionKey(Keyboard::Key_1), // PRECAST1
	ActionKey(Keyboard::Key_2), // PRECAST2
	ActionKey(Keyboard::Key_3), // PRECAST3
	ActionKey(Keyboard::Key_Tab, Keyboard::Key_NumPad0), // WEAPON
	ActionKey(Keyboard::Key_F9), // QUICKLOAD
	ActionKey(Keyboard::Key_F5), // QUICKSAVE
	ActionKey(Keyboard::Key_LeftArrow), // TURNLEFT
	ActionKey(Keyboard::Key_RightArrow), // TURNRIGHT
	ActionKey(Keyboard::Key_PageUp), // LOOKUP
	ActionKey(Keyboard::Key_PageDown), // LOOKDOWN
	ActionKey(Keyboard::Key_LeftAlt), // STRAFE
	ActionKey(Keyboard::Key_End), // CENTERVIEW
	ActionKey(Keyboard::Key_L, Mouse::Button_1), // FREELOOK
	ActionKey(Keyboard::Key_Minus), // PREVIOUS
	ActionKey(Keyboard::Key_Equals), // NEXT
	ActionKey(Keyboard::Key_C), // CROUCHTOGGLE
	ActionKey(Keyboard::Key_B), // UNEQUIPWEAPON
	ActionKey(Keyboard::Key_4), // CANCELCURSPELL
	ActionKey(Keyboard::Key_R, Keyboard::Key_M), // MINIMAP
	ActionKey((Keyboard::Key_LeftAlt << 16) | Keyboard::Key_Enter, (Keyboard::Key_RightAlt << 16) | Keyboard::Key_Enter), // TOGGLE_FULLSCREEN
};

} // namespace Default

namespace Section {

const string
	Language = "language",
	Video = "video",
	Window = "window",
	Audio = "audio",
	Input = "input",
	Key = "key",
	Misc = "misc";
}

namespace Key {

// Language options
const string language = "string";

// Video options
const string
	resolution = "resolution",
	fullscreen = "full_screen",
	levelOfDetail = "others_details",
	fogDistance = "fog",
	showCrosshair = "show_crosshair",
	antialiasing = "antialiasing",
	vsync = "vsync";

// Window options
const string
	windowSize = "size",
	windowFramework = "framework";

// Audio options
const string
	volume = "master_volume",
	sfxVolume = "effects_volume",
	speechVolume = "speech_volume",
	ambianceVolume = "ambiance_volume",
	eax = "eax",
	audioBackend = "backend";

// Input options
const string
	invertMouse = "invert_mouse",
	autoReadyWeapon = "auto_ready_weapon",
	mouseLookToggle = "mouse_look_toggle",
	mouseSensitivity = "mouse_sensitivity",
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
	"mouselook", // TODO rename to "use"?
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
	"minimap",
	"toggle_fullscreen"
};

// Misc options
const string
	forceToggle = "forcetoggle",
	migration = "migration",
	quicksaveSlots = "quicksave_slots",
	debugLevels = "debug";

} // namespace Key

class ConfigReader : public IniReader {
	
public:
	ActionKey getActionKey(const string & section, ControlAction index) const;
	
};

class ConfigWriter : public IniWriter {
	
public:
	
	explicit ConfigWriter(std::ostream & _output) : IniWriter(_output) { }
	
	void writeActionKey(ControlAction index, const ActionKey & value);

};

void ConfigWriter::writeActionKey(ControlAction index, const ActionKey & actionKey) {
	
	string v1 = Input::getKeyName(actionKey.key[0]);
	writeKey(Key::actions[index] + "_k0", v1);
	
	string v2 = Input::getKeyName(actionKey.key[1]);
	writeKey(Key::actions[index] + "_k1", v2);
}

ActionKey ConfigReader::getActionKey(const string & section, ControlAction index) const {
	
	const string & key = Key::actions[index];
	ActionKey action_key = Default::actions[index];
	
	const IniKey * k0 = getKey(section, key + "_k0");
	if(k0) {
		InputKeyId id = Input::getKeyId(k0->getValue());
		if(id == -1 && !k0->getValue().empty() && k0->getValue() != Input::KEY_NONE) {
			LogWarning << "Error parsing key name for " <<  key << "_k0: \"" << k0->getValue() << "\", resetting to \"" << Input::getKeyName(action_key.key[0]) << "\"";
		} else {
			action_key.key[0] = id;
		}
	}
	
	const IniKey * k1 = getKey(section, key + "_k1");
	if(k1) {
		InputKeyId id = Input::getKeyId(k1->getValue());
		if(id == -1 && !k1->getValue().empty() && k1->getValue() != Input::KEY_NONE) {
			LogWarning << "Error parsing key name for " <<  key << "_k1: \"" << k1->getValue() << "\", resetting to \"" << Input::getKeyName(action_key.key[1]) << "\"";
		} else {
			action_key.key[1] = id;
		}
	}
	
	LogDebug("[" << section << "] " << key << " = \"" << Input::getKeyName(action_key.key[0]) << "\", \"" << Input::getKeyName(action_key.key[1]) << "\"");
	
	return action_key;
}

} // anonymous namespace

Config config;

void Config::setDefaultActionKeys() {
	
	for(size_t i = 0; i < NUM_ACTION_KEY; i++) {
		actions[i] = Default::actions[i];
	}
	
	config.input.linkMouseLookToUse = false;
}

bool Config::setActionKey(ControlAction actionId, int index, InputKeyId key) {
	
	if(actionId < 0 || (size_t)actionId >= NUM_ACTION_KEY || index > 1 || index < 0) {
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
		
		if(i == (size_t)actionId) {
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

void Config::setOutputFile(const fs::path & _file) {
	file = _file;
	CrashHandler::addAttachedFile(file);
}

bool Config::save() {
	
	// Finally save it all to file/stream
	fs::ofstream out(file);
	if(!out.is_open()) {
		return false;
	}
	
	ConfigWriter writer(out);
	
	// language
	writer.beginSection(Section::Language);
	writer.writeKey(Key::language, language);
	
	// video
	writer.beginSection(Section::Video);
	if(video.resolution == Vec2i_ZERO) {
		writer.writeKey(Key::resolution, Default::resolution);
	} else {
		std::ostringstream oss;
		oss << video.resolution.x << 'x' << video.resolution.y;
		writer.writeKey(Key::resolution, oss.str());
	}
	writer.writeKey(Key::fullscreen, video.fullscreen);
	writer.writeKey(Key::levelOfDetail, video.levelOfDetail);
	writer.writeKey(Key::fogDistance, video.fogDistance);
	writer.writeKey(Key::showCrosshair, video.showCrosshair);
	writer.writeKey(Key::antialiasing, video.antialiasing);
	writer.writeKey(Key::vsync, video.vsync);
	
	// window
	writer.beginSection(Section::Window);
	std::ostringstream oss;
	oss << window.size.x << 'x' << window.size.y;
	writer.writeKey(Key::windowSize, oss.str());
	writer.writeKey(Key::windowFramework, window.framework);
	
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
	writer.writeKey(Key::autoDescription, input.autoDescription);
	writer.writeKey(Key::linkMouseLookToUse, input.linkMouseLookToUse);
	
	// key
	writer.beginSection(Section::Key);
	for(size_t i = 0; i < NUM_ACTION_KEY; i++) {
		writer.writeActionKey((ControlAction)i, actions[i]);
	}
	
	// misc
	writer.beginSection(Section::Misc);
	writer.writeKey(Key::forceToggle, misc.forceToggle);
	writer.writeKey(Key::migration, misc.migration);
	writer.writeKey(Key::quicksaveSlots, misc.quicksaveSlots);
	writer.writeKey(Key::debugLevels, misc.debug);
	
	return writer.flush();
}

static Vec2i parseResolution(const string & resolution) {
	
	Vec2i res;
	
	std::istringstream iss(resolution);
	iss >> res.x;
	char x = '\0';
	iss >> x;
	iss >> res.y;
	if(iss.fail() || x != 'x' || res.x <= 0 || res.y <= 0) {
		LogWarning << "Bad resolution string: " << resolution;
		return Vec2i(ARX_DEFAULT_WIDTH, ARX_DEFAULT_HEIGHT);
	} else {
		return res;
	}
}

bool Config::init(const fs::path & file) {
	
	fs::ifstream ifs;
	ifs.open(file);
	bool loaded = ifs.is_open();
	
	ConfigReader reader;
	
	if(!reader.read(ifs)) {
		LogWarning << "Errors while parsing config file";
	}
	
	// Get locale language
	language = reader.getKey(Section::Language, Key::language, Default::language);
	
	// Get video settings
	string resolution = reader.getKey(Section::Video, Key::resolution, Default::resolution);
	if(resolution == "auto") {
		video.resolution = Vec2i_ZERO;
	} else {
		video.resolution = parseResolution(resolution);
	}
	video.fullscreen = reader.getKey(Section::Video, Key::fullscreen, Default::fullscreen);
	video.levelOfDetail = reader.getKey(Section::Video, Key::levelOfDetail, Default::levelOfDetail);
	video.fogDistance = reader.getKey(Section::Video, Key::fogDistance, Default::fogDistance);
	video.showCrosshair = reader.getKey(Section::Video, Key::showCrosshair, Default::showCrosshair);
	video.antialiasing = reader.getKey(Section::Video, Key::antialiasing, Default::antialiasing);
	video.vsync = reader.getKey(Section::Video, Key::vsync, Default::vsync);
	
	// Get window settings
	string windowSize = reader.getKey(Section::Window, Key::windowSize, Default::windowSize);
	window.size = parseResolution(windowSize);
	window.framework = reader.getKey(Section::Window, Key::windowFramework, Default::windowFramework);
	
	// Get audio settings
	audio.volume = reader.getKey(Section::Audio, Key::volume, Default::volume);
	audio.sfxVolume = reader.getKey(Section::Audio, Key::sfxVolume, Default::sfxVolume);
	audio.speechVolume = reader.getKey(Section::Audio, Key::speechVolume, Default::speechVolume);
	audio.ambianceVolume = reader.getKey(Section::Audio, Key::ambianceVolume, Default::ambianceVolume);
	audio.eax = reader.getKey(Section::Audio, Key::eax, Default::eax);
	audio.backend = reader.getKey(Section::Audio, Key::audioBackend, Default::audioBackend);
	
	// Get input settings
	input.invertMouse = reader.getKey(Section::Input, Key::invertMouse, Default::invertMouse);
	input.autoReadyWeapon = reader.getKey(Section::Input, Key::autoReadyWeapon, Default::autoReadyWeapon);
	input.mouseLookToggle = reader.getKey(Section::Input, Key::mouseLookToggle, Default::mouseLookToggle);
	input.mouseSensitivity = reader.getKey(Section::Input, Key::mouseSensitivity, Default::mouseSensitivity);
	input.autoDescription = reader.getKey(Section::Input, Key::autoDescription, Default::autoDescription);
	input.linkMouseLookToUse = reader.getKey(Section::Input, Key::linkMouseLookToUse, Default::linkMouseLookToUse);
	
	// Get action key settings
	for(size_t i = 0; i < NUM_ACTION_KEY; i++) {
		actions[i] = reader.getActionKey(Section::Key, (ControlAction)i);
	}
	
	// Get miscellaneous settings
	misc.forceToggle = reader.getKey(Section::Misc, Key::forceToggle, Default::forceToggle);
	misc.migration = (MigrationStatus)reader.getKey(Section::Misc, Key::migration, Default::migration);
	misc.quicksaveSlots = std::max(reader.getKey(Section::Misc, Key::quicksaveSlots, Default::quicksaveSlots), 1);
	misc.debug = reader.getKey(Section::Misc, Key::debugLevels, Default::debugLevels);
	
	return loaded;
}
