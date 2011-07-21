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

#ifndef ARX_CORE_CONFIG_H
#define ARX_CORE_CONFIG_H

#include <string>

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
	CONTROLS_CUST_MOUSELOOK,
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
	
	CONTROLS_CUST_MINIMAP
	
};

typedef int InputKeyId; // TODO use InputKey from Application.h?

const size_t NUM_ACTION_KEY = CONTROLS_CUST_MINIMAP + 1;

struct ActionKey {
	
	ActionKey(InputKeyId key_0 = -1, InputKeyId key_1 = -1) {
		key[0] = key_0;
		key[1] = key_1;
	}
	
	InputKeyId key[2];
	
};

class Config {
	
public:
	
	// section FIRSTRUN
	bool firstRun;
	
	// section LANGUAGE
	std::string language;
	
	// section VIDEO
	struct {
		
		int width;
		int height;
		int bpp;
		bool fullscreen;
		bool bumpmap;
		int textureSize;
		int meshReduction;
		int levelOfDetail;
		float fogDistance;
		float gamma;
		float luminosity;
		float contrast;
		bool showCrosshair;
		bool antialiasing;
	
	} video;
	
	// section AUDIO
	struct {
		
		float volume;
		float sfxVolume;
		float speechVolume;
		float ambianceVolume;
		
		bool eax;
		
		std::string backend;
	
	} audio;
	
	// section INPUT
	struct {
		
		bool invertMouse;
		bool autoReadyWeapon;
		bool mouseLookToggle;
		bool autoDescription;
		int mouseSensitivity;
		bool mouseSmoothing;
		bool linkMouseLookToUse;
		
	} input;
	
	// section KEY
	ActionKey actions[NUM_ACTION_KEY];
	
	enum MigrationStatus {
		OriginalAssets = 0,
		CaseSensitiveFilenames = 1
	};
	
	// section MISC
	struct {
		
		bool forceZBias; // should be in video? TODO can we remove this?
		bool forceToggle; // should be in input?
		bool gore;
		bool newControl; // what is this?
		
		MigrationStatus migration;
		
		int quicksaveSlots;
		
	} misc;
	
public:
	
	bool setActionKey(ControlAction action, int index, InputKeyId key);
	void setDefaultActionKeys();
	
	/*!
	 * Saves all config entries to a file.
	 * @return true if the config was saved successfully.
	 */
	bool save();
	
	bool init(const std::string & file, const std::string & default_file);
	
private:
	
	std::string file;
	
	InputKeyId GetDIKWithASCII( const std::string& _pcTouch) const;
	
};

extern Config config;

#endif // ARX_CORE_CONFIG_H
