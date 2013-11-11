/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_CORE_CONFIG_H
#define ARX_CORE_CONFIG_H

#include <string>

#include "input/InputKey.h"

#include "io/fs/FilePath.h"

#include "math/Types.h"
#include "math/Vector.h"

//! Enum for all the controlling actions
enum ControlAction {
	
	CONTROLS_CUST_JUMP = 0,
	CONTROLS_CUST_MAGICMODE,
	CONTROLS_CUST_STEALTHMODE,
	CONTROLS_CUST_WALKFORWARD,
	CONTROLS_CUST_WALKBACKWARD,
	CONTROLS_CUST_STRAFELEFT,
	CONTROLS_CUST_STRAFERIGHT,
	CONTROLS_CUST_LEANLEFT,
	CONTROLS_CUST_LEANRIGHT,
	CONTROLS_CUST_CROUCH,
	CONTROLS_CUST_USE,
	CONTROLS_CUST_ACTION,
	CONTROLS_CUST_INVENTORY,
	CONTROLS_CUST_BOOK,
	CONTROLS_CUST_BOOKCHARSHEET,
	CONTROLS_CUST_BOOKSPELL,
	CONTROLS_CUST_BOOKMAP,
	CONTROLS_CUST_BOOKQUEST,
	CONTROLS_CUST_DRINKPOTIONLIFE,
	CONTROLS_CUST_DRINKPOTIONMANA,
	CONTROLS_CUST_TORCH,
	
	CONTROLS_CUST_PRECAST1,
	CONTROLS_CUST_PRECAST2,
	CONTROLS_CUST_PRECAST3,
	CONTROLS_CUST_WEAPON,
	CONTROLS_CUST_QUICKLOAD,
	CONTROLS_CUST_QUICKSAVE,
	
	CONTROLS_CUST_TURNLEFT,
	CONTROLS_CUST_TURNRIGHT,
	CONTROLS_CUST_LOOKUP,
	CONTROLS_CUST_LOOKDOWN,
	
	CONTROLS_CUST_STRAFE,
	CONTROLS_CUST_CENTERVIEW,
	
	CONTROLS_CUST_FREELOOK,
	
	CONTROLS_CUST_PREVIOUS,
	CONTROLS_CUST_NEXT,
	
	CONTROLS_CUST_CROUCHTOGGLE,
	
	CONTROLS_CUST_UNEQUIPWEAPON,
	
	CONTROLS_CUST_CANCELCURSPELL,
	
	CONTROLS_CUST_MINIMAP,
	
	CONTROLS_CUST_TOGGLE_FULLSCREEN,
	
	NUM_ACTION_KEY
};

struct ActionKey {
	
	ActionKey(InputKeyId key_0 = -1, InputKeyId key_1 = -1) {
		key[0] = key_0;
		key[1] = key_1;
	}
	
	InputKeyId key[2];
	
};

class Config {
	
public:
	
	// section 'language'
	std::string language;
	
	// section 'video'
	struct {
		
		Vec2i resolution;
		
		bool fullscreen;
		int levelOfDetail;
		float fogDistance;
		bool showCrosshair;
		bool antialiasing;
		bool vsync;
		
	} video;
	
	// section 'window'
	struct {
		
		Vec2i size;
		
		std::string framework;
		
	} window;
	
	// section 'audio'
	struct {
		
		float volume;
		float sfxVolume;
		float speechVolume;
		float ambianceVolume;
		
		bool eax;
		
		std::string backend;
	
	} audio;
	
	// section 'input'
	struct {
		
		bool invertMouse;
		bool autoReadyWeapon;
		bool mouseLookToggle;
		bool autoDescription;
		int mouseSensitivity;
		bool linkMouseLookToUse;
		
	} input;
	
	// section 'key'
	ActionKey actions[NUM_ACTION_KEY];
	
	enum MigrationStatus {
		OriginalAssets = 0,
		CaseSensitiveFilenames = 1
	};
	
	// section 'misc'
	struct {
		
		bool forceToggle; // should be in input?
		
		MigrationStatus migration;
		
		int quicksaveSlots;
		
		std::string debug; //!< Logger debug levels.
		
	} misc;
	
public:
	
	bool setActionKey(ControlAction action, int index, InputKeyId key);
	void setDefaultActionKeys();
	
	/*!
	 * Saves all config entries to a file.
	 * @return true if the config was saved successfully.
	 */
	bool save();
	
	bool init(const fs::path & file);
	
	void setOutputFile(const fs::path & _file);
	
private:
	
	fs::path file;
};

extern Config config;

#endif // ARX_CORE_CONFIG_H
