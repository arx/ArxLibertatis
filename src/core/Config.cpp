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

// To avoid conflicts with potential other classes/namespaces
namespace {

/* Default values for config */
namespace Default {

#define ARX_DEFAULT_WIDTH 640
#define ARX_DEFAULT_HEIGHT 480
#define THUMBNAIL_DEFAULT_WIDTH 320
#define THUMBNAIL_DEFAULT_HEIGHT 200

const std::string
	language = std::string(),
	resolution = "auto",
	audioBackend = "auto",
	audioDevice = "auto",
	windowFramework = "auto",
	windowSize = BOOST_PP_STRINGIZE(ARX_DEFAULT_WIDTH) "x"
	             BOOST_PP_STRINGIZE(ARX_DEFAULT_HEIGHT),
	debugLevels = "",
	bufferUpload = "",
	
	thumbnailSize = BOOST_PP_STRINGIZE(THUMBNAIL_DEFAULT_WIDTH) "x"
					BOOST_PP_STRINGIZE(THUMBNAIL_DEFAULT_HEIGHT);

const int
	levelOfDetail = 2,
	fogDistance = 10,
	maxAnisotropicFiltering = 9001,
	cinematicWidescreenMode = CinematicFadeEdges,
	hudScaleFilter = UIFilterBilinear,
	volume = 10,
	sfxVolume = 10,
	speechVolume = 10,
	ambianceVolume = 10,
	mouseSensitivity = 6,
	mouseAcceleration = 0,
	migration = Config::OriginalAssets,
	quicksaveSlots = 3,
	bufferSize = 0;

const bool
	fullscreen = true,
	showCrosshair = true,
	antialiasing = true,
	vsync = true,
	colorkeyAlphaToCoverage = true,
	colorkeyAntialiasing = true,
	limitSpeechWidth = true,
	hudScaleInteger = true,
	minimizeOnFocusLost = true,
	eax = true,
	muteOnFocusLost = false,
	invertMouse = false,
	autoReadyWeapon = false,
	mouseLookToggle = true,
	autoDescription = true,
	forceToggle = false,
	rawMouseInput = true,
	borderTurning = true;

const float
	hudScale = 0.5f;

const ActionKey actions[NUM_ACTION_KEY] = {
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

const std::string
	Language = "language",
	Video = "video",
	Interface = "interface",
	Window = "window",
	Audio = "audio",
	Input = "input",
	Key = "key",
	Misc = "misc";
}

namespace Key {

// Language options
const std::string language = "string";

// Video options
const std::string
	resolution = "resolution",
	fullscreen = "full_screen",
	levelOfDetail = "others_details",
	fogDistance = "fog",
	antialiasing = "antialiasing",
	vsync = "vsync",
	maxAnisotropicFiltering = "max_anisotropic_filtering",
	colorkeyAlphaToCoverage = "colorkey_alpha_to_coverage",
	colorkeyAntialiasing = "colorkey_antialiasing",
	bufferSize = "buffer_size",
	bufferUpload = "buffer_upload";

// Interface options
const std::string
	showCrosshair = "show_crosshair",
	limitSpeechWidth = "limit_speech_width",
	cinematicWidescreenMode = "cinematic_widescreen_mode",
	hudScale = "hud_scale",
	hudScaleInteger = "hud_scale_integer",
	hudScaleFilter = "hud_scale_filter",
	thumbnailSize = "save_thumbnail_size";

// Window options
const std::string
	windowFramework = "framework",
	windowSize = "size",
	minimizeOnFocusLost = "minimize_on_focus_lost";

// Audio options
const std::string
	audioBackend = "backend",
	audioDevice = "device",
	volume = "master_volume",
	sfxVolume = "effects_volume",
	speechVolume = "speech_volume",
	ambianceVolume = "ambiance_volume",
	eax = "eax",
	muteOnFocusLost = "mute_on_focus_lost";

// Input options
const std::string
	invertMouse = "invert_mouse",
	autoReadyWeapon = "auto_ready_weapon",
	mouseLookToggle = "mouse_look_toggle",
	mouseSensitivity = "mouse_sensitivity",
	mouseAcceleration = "mouse_acceleration",
	rawMouseInput = "raw_mouse_input",
	autoDescription = "auto_description",
	borderTurning = "border_turning";

// Input key options
const std::string actions[NUM_ACTION_KEY] = {
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
const std::string
	forceToggle = "forcetoggle",
	migration = "migration",
	quicksaveSlots = "quicksave_slots",
	debugLevels = "debug";

} // namespace Key

class ConfigReader : public IniReader {
	
public:
	ActionKey getActionKey(const std::string & section, ControlAction index) const;
	
};

class ConfigWriter : public IniWriter {
	
public:
	
	explicit ConfigWriter(std::ostream & _output) : IniWriter(_output) { }
	
	void writeActionKey(ControlAction index, const ActionKey & value);

};

void ConfigWriter::writeActionKey(ControlAction index, const ActionKey & actionKey) {
	
	std::string v1 = Input::getKeyName(actionKey.key[0]);
	writeKey(Key::actions[index] + "_k0", v1);
	
	std::string v2 = Input::getKeyName(actionKey.key[1]);
	writeKey(Key::actions[index] + "_k1", v2);
}

ActionKey ConfigReader::getActionKey(const std::string & section, ControlAction index) const {
	
	const std::string & key = Key::actions[index];
	ActionKey action_key = Default::actions[index];
	
	const IniKey * k0 = getKey(section, key + "_k0");
	if(k0) {
		InputKeyId id = Input::getKeyId(k0->getValue());
		if(id == ActionKey::UNUSED && !k0->getValue().empty() && k0->getValue() != Input::KEY_NONE) {
			LogWarning << "Error parsing key name for " <<  key << "_k0: \"" << k0->getValue() << "\", resetting to \"" << Input::getKeyName(action_key.key[0]) << "\"";
		} else {
			action_key.key[0] = id;
		}
	}
	
	const IniKey * k1 = getKey(section, key + "_k1");
	if(k1) {
		InputKeyId id = Input::getKeyId(k1->getValue());
		if(id == ActionKey::UNUSED && !k1->getValue().empty() && k1->getValue() != Input::KEY_NONE) {
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
}

void Config::setActionKey(ControlAction actionId, size_t index, InputKeyId key) {
	
	if(actionId < 0 || actionId >= NUM_ACTION_KEY || index > 1) {
		arx_assert(false);
		return;
	}
	
	ActionKey & action = actions[actionId];
	action.key[index] = key;
	
	int otherIndex = 1 - index;
	
	if(action.key[otherIndex] == key) {
		action.key[otherIndex] = ActionKey::UNUSED;
	}
	
	// remove double key assignments
	for(size_t i = 0; i < NUM_ACTION_KEY; i++) {
		
		if(i == (size_t)actionId) {
			continue;
		}
		
		for(int k = 0; k < 2; k++) {
			if(actions[i].key[k] == key) {
				actions[i].key[k] = ActionKey::UNUSED;
			}
		}
		
	}
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
	writer.writeKey(Key::antialiasing, video.antialiasing);
	writer.writeKey(Key::vsync, video.vsync);
	writer.writeKey(Key::maxAnisotropicFiltering, video.maxAnisotropicFiltering);
	writer.writeKey(Key::colorkeyAlphaToCoverage, video.colorkeyAlphaToCoverage);
	writer.writeKey(Key::colorkeyAntialiasing, video.colorkeyAntialiasing);
	writer.writeKey(Key::bufferSize, video.bufferSize);
	writer.writeKey(Key::bufferUpload, video.bufferUpload);
	
	// interface
	writer.beginSection(Section::Interface);
	writer.writeKey(Key::showCrosshair, interface.showCrosshair);
	writer.writeKey(Key::limitSpeechWidth, interface.limitSpeechWidth);
	writer.writeKey(Key::cinematicWidescreenMode, int(interface.cinematicWidescreenMode));
	writer.writeKey(Key::hudScale, interface.hudScale);
	writer.writeKey(Key::hudScaleInteger, interface.hudScaleInteger);
	writer.writeKey(Key::hudScaleFilter, interface.hudScaleFilter);
	std::ostringstream osst;
	osst << interface.thumbnailSize.x << 'x' << interface.thumbnailSize.y;
	writer.writeKey(Key::thumbnailSize, osst.str());
	
	// window
	writer.beginSection(Section::Window);
	writer.writeKey(Key::windowFramework, window.framework);
	std::ostringstream oss;
	oss << window.size.x << 'x' << window.size.y;
	writer.writeKey(Key::windowSize, oss.str());
	writer.writeKey(Key::minimizeOnFocusLost, window.minimizeOnFocusLost);
	
	// audio
	writer.beginSection(Section::Audio);
	writer.writeKey(Key::audioBackend, audio.backend);
	writer.writeKey(Key::audioDevice, audio.device);
	writer.writeKey(Key::volume, audio.volume);
	writer.writeKey(Key::sfxVolume, audio.sfxVolume);
	writer.writeKey(Key::speechVolume, audio.speechVolume);
	writer.writeKey(Key::ambianceVolume, audio.ambianceVolume);
	writer.writeKey(Key::eax, audio.eax);
	writer.writeKey(Key::muteOnFocusLost, audio.muteOnFocusLost);
	
	// input
	writer.beginSection(Section::Input);
	writer.writeKey(Key::invertMouse, input.invertMouse);
	writer.writeKey(Key::autoReadyWeapon, input.autoReadyWeapon);
	writer.writeKey(Key::mouseLookToggle, input.mouseLookToggle);
	writer.writeKey(Key::mouseSensitivity, input.mouseSensitivity);
	writer.writeKey(Key::mouseAcceleration, input.mouseAcceleration);
	writer.writeKey(Key::rawMouseInput, input.rawMouseInput);
	writer.writeKey(Key::autoDescription, input.autoDescription);
	writer.writeKey(Key::borderTurning, input.borderTurning);
	
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

static Vec2i parseResolution(const std::string & resolution) {
	
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

static Vec2i parseThumbnailSize(const std::string & thumbnailSize) {
	
	Vec2i res;
	
	std::istringstream iss(thumbnailSize);
	iss >> res.x;
	char x = '\0';
	iss >> x;
	iss >> res.y;
	if (iss.fail() || x != 'x' || res.x <= 0 || res.y <= 0) {
		LogWarning << "Bad thumbnail resolution string: " << thumbnailSize;
		return Vec2i(THUMBNAIL_DEFAULT_WIDTH, THUMBNAIL_DEFAULT_HEIGHT);
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
	std::string resolution = reader.getKey(Section::Video, Key::resolution, Default::resolution);
	if(resolution == "auto") {
		video.resolution = Vec2i_ZERO;
	} else {
		video.resolution = parseResolution(resolution);
	}
	video.fullscreen = reader.getKey(Section::Video, Key::fullscreen, Default::fullscreen);
	video.levelOfDetail = reader.getKey(Section::Video, Key::levelOfDetail, Default::levelOfDetail);
	video.fogDistance = reader.getKey(Section::Video, Key::fogDistance, Default::fogDistance);;
	video.antialiasing = reader.getKey(Section::Video, Key::antialiasing, Default::antialiasing);
	video.vsync = reader.getKey(Section::Video, Key::vsync, Default::vsync);
	int anisoFiltering = reader.getKey(Section::Video, Key::maxAnisotropicFiltering, Default::maxAnisotropicFiltering);
	video.maxAnisotropicFiltering = std::max(anisoFiltering, 1);
	video.maxAnisotropicFiltering = std::max(0, video.maxAnisotropicFiltering);
	video.colorkeyAlphaToCoverage = reader.getKey(Section::Video, Key::colorkeyAlphaToCoverage, Default::colorkeyAlphaToCoverage);
	video.colorkeyAntialiasing = reader.getKey(Section::Video, Key::colorkeyAntialiasing, Default::colorkeyAntialiasing);
	video.bufferSize = std::max(reader.getKey(Section::Video, Key::bufferSize, Default::bufferSize), 0);
	video.bufferUpload = reader.getKey(Section::Video, Key::bufferUpload, Default::bufferUpload);
	
	// Get interface settings
	bool oldCrosshair = reader.getKey(Section::Video, Key::showCrosshair, Default::showCrosshair);
	interface.showCrosshair = reader.getKey(Section::Interface, Key::showCrosshair, oldCrosshair);
	interface.limitSpeechWidth = reader.getKey(Section::Interface, Key::limitSpeechWidth, Default::limitSpeechWidth);
	int cinematicMode = reader.getKey(Section::Interface, Key::cinematicWidescreenMode, Default::cinematicWidescreenMode);
	interface.cinematicWidescreenMode = CinematicWidescreenMode(glm::clamp(cinematicMode, 0, 2));
	float hudScale = reader.getKey(Section::Interface, Key::hudScale, Default::hudScale);
	interface.hudScale = glm::clamp(hudScale, 0.f, 1.f);
	interface.hudScaleInteger = reader.getKey(Section::Interface, Key::hudScaleInteger, Default::hudScaleInteger);
	int hudScaleFilter = reader.getKey(Section::Interface, Key::hudScaleFilter, Default::hudScaleFilter);
	interface.hudScaleFilter = UIScaleFilter(glm::clamp(hudScaleFilter, 0, 1));
	std::string thumbnailSize = reader.getKey(Section::Interface, Key::thumbnailSize, Default::thumbnailSize);
	interface.thumbnailSize = parseThumbnailSize(thumbnailSize);
	
	// Get window settings
	window.framework = reader.getKey(Section::Window, Key::windowFramework, Default::windowFramework);
	std::string windowSize = reader.getKey(Section::Window, Key::windowSize, Default::windowSize);
	window.size = parseResolution(windowSize);
	window.minimizeOnFocusLost = reader.getKey(Section::Window, Key::minimizeOnFocusLost,
	                                           Default::minimizeOnFocusLost);
	
	// Get audio settings
	audio.backend = reader.getKey(Section::Audio, Key::audioBackend, Default::audioBackend);
	audio.device = reader.getKey(Section::Audio, Key::audioDevice, Default::audioDevice);
	audio.volume = reader.getKey(Section::Audio, Key::volume, Default::volume);
	audio.sfxVolume = reader.getKey(Section::Audio, Key::sfxVolume, Default::sfxVolume);
	audio.speechVolume = reader.getKey(Section::Audio, Key::speechVolume, Default::speechVolume);
	audio.ambianceVolume = reader.getKey(Section::Audio, Key::ambianceVolume, Default::ambianceVolume);
	audio.eax = reader.getKey(Section::Audio, Key::eax, Default::eax);
	audio.muteOnFocusLost = reader.getKey(Section::Audio, Key::muteOnFocusLost, Default::muteOnFocusLost);
	
	// Get input settings
	input.invertMouse = reader.getKey(Section::Input, Key::invertMouse, Default::invertMouse);
	input.autoReadyWeapon = reader.getKey(Section::Input, Key::autoReadyWeapon, Default::autoReadyWeapon);
	input.mouseLookToggle = reader.getKey(Section::Input, Key::mouseLookToggle, Default::mouseLookToggle);
	input.mouseSensitivity = reader.getKey(Section::Input, Key::mouseSensitivity, Default::mouseSensitivity);
	input.mouseAcceleration = reader.getKey(Section::Input, Key::mouseAcceleration, Default::mouseAcceleration);
	input.rawMouseInput = reader.getKey(Section::Input, Key::rawMouseInput, Default::rawMouseInput);
	input.autoDescription = reader.getKey(Section::Input, Key::autoDescription, Default::autoDescription);
	input.borderTurning = reader.getKey(Section::Input, Key::borderTurning, Default::borderTurning);
	
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
