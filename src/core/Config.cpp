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

#include <cstdio>

#include <fstream>
#include <sstream>

#include "core/Localization.h"
#include "graphics/data/Texture.h"
#include "gui/MenuPublic.h"
#include "gui/MenuWidgets.h"
#include "io/Logger.h"
#include "io/String.h"
#include "scene/GameSound.h"
#include "window/Input.h"

// To avoid conflicts with potential other classes/namespaces
namespace
{

	/* Default values for config */
	namespace Default
	{
		std::string
			language = "english",
			resolution = "640x480",
			modpak = "mod.pak",
			modsfxpak = "modsfx.pak",
			speechmodpak = "modspeech.pak",
			empty_string = "";

		int
			bpp = 16,
			texture = 2,
			mesh_reduction = 0,
			others_details = 2,
			fog = 5,
			gamma = 5,
			luminosity = 4,
			contrast = 5,
			master_volume = 10,
			effects_volume = 10,
			speech_volume = 10,
			ambiance_volume = 10,
			mouse_sensitivity = 4;

		bool
			first_run = true,
			full_screen = true,
			bump = false,
			show_crosshair = true,
			antialiasing = false,
			EAX = false,
			invert_mouse = false,
			auto_ready_weapon = false,
			mouse_look_toggle = false,
			mouse_smoothing = false,
			auto_description = true,
			link_mouse_look_to_use = false,
			softfog = false,
			forcenoeax = false,
			forcezbias = false,
			forcetoggle = false,
			fg = true,
			newcontrol = false;
	};

	namespace Section
	{
		std::string
			Language = "LANGUAGE",
			FirstRun = "FIRSTRUN",
			Video = "VIDEO",
			Audio = "AUDIO",
			Input = "INPUT",
			Key = "KEY",
			Misc = "MISC";
	};

	namespace Key
	{
		std::string
			// Language options
			language_string = "string",
			// First run options
			first_run_int = "int",
			// Video options
			resolution = "resolution",
			bpp = "bpp",
			full_screen = "full_screen",
			bump = "bump",
			texture = "texture",
			mesh_reduction = "mesh_reduction",
			others_details = "others_details",
			fog = "fog",
			gamma = "gamma",
			luminosity = "luminosity",
			contrast = "contrast",
			show_crosshair = "show_crosshair",
			antialiasing = "antialiasing",
			// Audio options
			master_volume = "master_volume",
			effects_volume = "effects_volume",
			speech_volume = "speech_volume",
			ambiance_volume = "ambiance_volume",
			EAX = "EAX",
			// Input options
			invert_mouse = "invert_mouse",
			auto_ready_weapon = "auto_ready_weapon",
			mouse_look_toggle = "mouse_look_toggle",
			mouse_sensitivity = "mouse_sensitivity",
			mouse_smoothing = "mouse_smoothing",
			auto_description = "auto_description",
			link_mouse_look_to_use = "link_mouse_look_to_use",
			// Input key options
			jump = "jump",
			magic_mode = "magic_mode",
			stealth_mode = "stealth_mode",
			walk_forward = "walk_forward",
			walk_backward = "walk_backward",
			strafe_left = "strafe_left",
			strafe_right = "strafe_right",
			lean_left = "lean_left",
			lean_right = "lean_right",
			crouch = "crouch",
			mouselook = "mouselook",
			action_combine = "action_combine",
			inventory = "inventory",
			book = "book",
			char_sheet = "char_sheet",
			magic_book = "magic_book",
			map = "map",
			quest_book = "quest_book",
			drink_potion_life = "drink_potion_life",
			drink_potion_mana = "drink_potion_mana",
			torch = "torch",
			cancel_current_spell = "cancel_current_spell",
			precast_1 = "precast_1",
			precast_2 = "precast_2",
			precast_3 = "precast_3",
			draw_weapon = "draw_weapon",
			quicksave = "quicksave",
			quickload = "quickload",
			turn_left = "turn_left",
			turn_right = "turn_right",
			look_up = "look_up",
			look_down = "look_down",
			strafe = "strafe",
			center_view = "center_view",
			freelook = "freelook",
			previous = "previous",
			next = "next",
			crouch_toggle = "crouch_toggle",
			unequip_weapon = "unequip_weapon",
			minimap = "minimap",

			// Misc options
			softfog = "softfog",
			forcenoeax = "forcenoeax",
			forcezbias = "forcezbias",
			forcetoggle = "forcetoggle",
			fg = "fg",
			modpak = "mod",
			modsfxpak = "modsfx",
			modspeechpak = "modspeech",
			newcontrol = "newcontrol"
			;
	};

	ActionKey sakActionDefaultKey[MAX_ACTION_KEY] =
	{
		ActionKey( DIK_SPACE ), // JUMP
		ActionKey( DIK_LCONTROL, DIK_RCONTROL ), // MAGICMODE
		ActionKey( DIK_LSHIFT, DIK_RSHIFT ), // STEALTHMODE
		ActionKey( DIK_W, DIK_UP ), // WALKFORWARD
		ActionKey( DIK_S, DIK_DOWN ), // WALKBACKWARD
		ActionKey( DIK_A ), // STRAFELEFT
		ActionKey( DIK_D ), // STRAFERIGHT
		ActionKey( DIK_Q ), // LEANLEFT
		ActionKey( DIK_E ), // LEANRIGHT
		ActionKey( DIK_X ), // CROUCH
		ActionKey( DIK_F, DIK_RETURN ), // MOUSELOOK
		ActionKey( DIK_BUTTON1 ), // ACTION
		ActionKey( DIK_I, -1, 1 ), // INVENTORY
		ActionKey( DIK_BACKSPACE, -1, 1 ), // BOOK
		ActionKey( DIK_F1, -1, 1 ), // BOOKCHARSHEET
		ActionKey( DIK_F2, -1, 1 ), // BOOKSPELL
		ActionKey( DIK_F3, -1, 1 ), // BOOKMAP
		ActionKey( DIK_F4, -1, 1 ), // BOOKQUEST
		ActionKey( DIK_H, -1, 1 ), // DRINKPOTIONLIFE
		ActionKey( DIK_G, -1, 1 ), // DRINKPOTIONMANA
		ActionKey( DIK_T, -1, 1 ), // TORCH
		ActionKey( DIK_1, -1, 1 ), // PRECAST1
		ActionKey( DIK_2, -1, 1 ), // PRECAST2
		ActionKey( DIK_3, -1, 1 ), // PRECAST3
		ActionKey( DIK_TAB, DIK_NUMPAD0, 1 ), // WEAPON
		ActionKey( DIK_F9, -1, 1 ), // QUICKLOAD
		ActionKey( DIK_F5, -1, 1 ), // QUICKSAVE
		ActionKey( DIK_LEFT ), // TURNLEFT
		ActionKey( DIK_RIGHT ), // TURNRIGHT
		ActionKey( DIK_PGUP ), // LOOKUP
		ActionKey( DIK_PGDN ), // LOOKDOWN
		ActionKey( DIK_LALT ), // STRAFE
		ActionKey( DIK_END ), // CENTERVIEW
		ActionKey( DIK_L, DIK_BUTTON2 ), // FREELOOK

		// TODO Get these intialized const, not through DInput calls
		ActionKey( -1, -1, 1 ), // PREVIOUS
		ActionKey( -1, -1, 1 ), // NEXT
		ActionKey( -1, -1, 1 ), // CROUCHTOGGLE

		ActionKey( DIK_B, -1, 1 ), // UNEQUIPWEAPON
		ActionKey( DIK_4, -1, 1 ), // CANCELCURSPELL
		ActionKey( DIK_R, DIK_M, 1 ), // MINIMAP
	};
};

void ARX_SetAntiAliasing();

	/* Externs */
extern long INTERNATIONAL_MODE;
extern CDirectInput * pGetInfoDirectInput;
extern long GORE_MODE;
extern long GERMAN_VERSION;
extern long FRENCH_VERSION;
extern bool bForceNoEAX;
extern CMenuState *pMenu;
extern bool bALLOW_BUMP;
extern long WILL_RELOAD_ALL_TEXTURES;

/* Global variables */
std::string pStringMod;
std::string pStringModSfx;
std::string pStringModSpeech;
Config *pMenuConfig = 0;

Config::Config()
{
	init();
}

Config::Config( const std::string& _filename)
{
	// if _pName equals exactly "cfg"
	if ( _filename == "cfg" )
		pcName = "cfg.ini";
	else
		pcName = _filename;

	// Load the config file
	std::ifstream ifs( pcName.c_str() );
	config_map = ConfigHashMap( ifs );

	init();
}

Config::~Config()
{

}

void Config::init()
{

	// TODO Remove these and constify sakActionDefaultKey
	sakActionDefaultKey[CONTROLS_CUST_PREVIOUS].iKey[0] = GetDIKWithASCII(")");
	sakActionDefaultKey[CONTROLS_CUST_PREVIOUS].iPage=1;
	sakActionDefaultKey[CONTROLS_CUST_NEXT].iKey[0] = GetDIKWithASCII("=");
	sakActionDefaultKey[CONTROLS_CUST_NEXT].iPage=1;

	sakActionDefaultKey[CONTROLS_CUST_CROUCHTOGGLE].iKey[0] = GetDIKWithASCII("C");

	bChangeResolution = false;
	bChangeTextures = false;
	bNoReturnToWindows=false;
	bLinkMouseLookToUse=false;
	bForceZBias=false;

	SetDefaultKey();
}

void Config::SetDefaultKey()
{
	int iI=MAX_ACTION_KEY;

	while(iI--)
	{
		sakActionKey[iI].iKey[0]=sakActionDefaultKey[iI].iKey[0];
		sakActionKey[iI].iKey[1]=sakActionDefaultKey[iI].iKey[1];
		sakActionKey[iI].iPage=sakActionDefaultKey[iI].iPage;
	}

	if (!INTERNATIONAL_MODE)
		bLinkMouseLookToUse=true;
	else
		bLinkMouseLookToUse=false;
}

int Config::GetDIKWithASCII( const std::string& _pcTouch) const
{
	std::string pcT = _pcTouch;

	// If a noneset key, return -1
	if( strcasecmp( pcT, "---"  ) == 0 )
		return -1;

	for(int iI=0;iI<256;iI++)
	{
		std::string pcT1 = pGetInfoDirectInput->GetFullNameTouch(iI);

		if( !pcT.compare( pcT1 ) )
			return iI;

		pcT1 = pGetInfoDirectInput->GetFullNameTouch(iI | (DIK_LSHIFT << 16));

		if( !pcT.compare( pcT1 ) )
			return iI|(DIK_LSHIFT<<16);

		pcT1 = pGetInfoDirectInput->GetFullNameTouch(iI|(DIK_RSHIFT<<16));

		if( !pcT.compare( pcT1 ) )
			return iI|(DIK_RSHIFT<<16);

		pcT1 = pGetInfoDirectInput->GetFullNameTouch(iI|(DIK_LCONTROL<<16));

		if( !pcT.compare( pcT1 ) )
			return iI|(DIK_LCONTROL<<16);

		pcT1 = pGetInfoDirectInput->GetFullNameTouch(iI|(DIK_RCONTROL<<16));

		if( !pcT.compare( pcT1 ) )
			return iI|(DIK_RCONTROL<<16);

		pcT1 = pGetInfoDirectInput->GetFullNameTouch(iI|(DIK_LALT<<16));

		if( ! pcT.compare( pcT1 ) )
			return iI|(DIK_LALT<<16);

		pcT1 = pGetInfoDirectInput->GetFullNameTouch(iI|(DIK_RALT<<16));

		if( !pcT.compare( pcT1 ) )
			return iI|(DIK_RALT<<16);
	}

	for(int iI=DIK_BUTTON1;iI<=(int)DIK_BUTTON32;iI++)
	{
		std::string pcT1 = pGetInfoDirectInput->GetFullNameTouch(iI);

		if( !pcT.compare( pcT1 ) )
			return iI;
	}

	for( unsigned iI=DIK_WHEELUP;iI<=DIK_WHEELDOWN;iI++)
	{
		std::string pcT1 = pGetInfoDirectInput->GetFullNameTouch(iI);

		if( !pcT.compare( pcT1 ) )
			return iI;
	}

	return -1;
}

//-----------------------------------------------------------------------------
std::string Config::ReadConfig( const std::string& _section, const std::string& _key) const
{
	
	LogDebug << "Read section: " << _section << " key: " << _key << " from " << pcName << " as:" << config_map.getConfigValue( _section, "", _key );
	return config_map.getConfigValue( _section, "", _key );
}

//-----------------------------------------------------------------------------

int Config::ReadConfigInt( const std::string& _pcSection, const std::string& _pcKey, bool &_bOk )
{
	std::string pcText=ReadConfig(_pcSection,_pcKey);

	if ( pcText.empty() )
	{
		_bOk = false;
		return 0;
	}

	std::stringstream ss( pcText );

	int iI;
	ss >> iI;
	_bOk=true;
	return iI;
}

/**
 * Reads a string from the Config and returns it, returning the default value if an empty string was found
 * @param section The section to read from
 * @param key The key in the section to return
 * @param default_value The default value to be returned in the case of an empty string
 */
std::string Config::ReadConfig( const std::string& section, const std::string& key, const std::string& default_value ) const
{
	std::string temp( ReadConfig( section, key ) );

	if ( temp.empty() )
		return default_value;
	else
		return temp;
}

/**
 * Reads a string from the Config and returns its converted int value,
 * return the default value if an empty string is found.
 * @param section The section to read from
 * @param key The key in the section to return
 * @param default_value The default value to return in the case of an empty string
 */
int Config::ReadConfig( const std::string& section, const std::string& key, int default_value ) const
{
	std::string temp( ReadConfig( section, key ) );

	if ( temp.empty() )
		return default_value;
	else
		return atoi( temp );
}

/**
 * Reads a string from the Config and returns its converted bool value,
 * return the default value if an empty string is found.
 * @param section The section to read from
 * @param key The key in the section to return
 * @param default_value The default value to return in the case of an empty string
 */
bool Config::ReadConfig( const std::string& section, const std::string& key, bool default_value ) const
{
	std::string temp( ReadConfig( section, key ) );

	if ( temp.empty() )
		return default_value;
	else
		return atob( temp );
}

//-----------------------------------------------------------------------------

void Config::WriteConfig( const std::string& section, const std::string& key, const std::string& value )
{
	LogDebug << "Updating map with Section: " << section << " Key: " << key << " Value: " << value;
	config_map.updateConfigValue( section, key, value );
}

void Config::WriteConfig( const std::string& section, const std::string& key, int value )
{
	WriteConfig( section, key, itoa( value ) );
}

/**
 * Writes a key to the section map after converting the bool value
 * @param section The section to write to
 * @param key The Key in the section to write
 * @param value The bool value to be written
 */
void Config::WriteConfig( const std::string& section, const std::string& key, bool value )
{
	WriteConfig( section, key, btoa( value ) );
}

//-----------------------------------------------------------------------------

bool Config::WriteConfigInt( const std::string& _pcSection, const std::string& _pcKey, int data )
{
	WriteConfig(_pcSection,_pcKey, itoa( data ) );
	return false;
}

//-----------------------------------------------------------------------------

bool Config::WriteConfigString( const std::string& _pcSection, const std::string& _pcKey, const std::string& _pcDatas)
{
	WriteConfig(_pcSection,_pcKey,_pcDatas);
	return false;
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

bool Config::SetActionKey(int _iAction,int _iActionNum,int _iVirtualKey)
{
	if(    (_iAction>=MAX_ACTION_KEY)||
		(_iActionNum>1) ) return false;

	bool bChange=false;
	bool bSecondChoice=false;
	int iOldVirtualKey=sakActionKey[_iAction].iKey[_iActionNum];
	sakActionKey[_iAction].iKey[_iActionNum]=_iVirtualKey;

	if(_iActionNum)
	{
		if(sakActionKey[_iAction].iKey[0]==-1)
		{
			sakActionKey[_iAction].iKey[0]=iOldVirtualKey;
			bSecondChoice=true;
		}

		if(sakActionKey[_iAction].iKey[0]==_iVirtualKey)
		{
			sakActionKey[_iAction].iKey[0]=-1;
		}

		bChange=true;
	}
	else
	{
		if(sakActionKey[_iAction].iKey[1]==-1)
		{
			sakActionKey[_iAction].iKey[1]=iOldVirtualKey;
			bSecondChoice=true;
		}

		if(sakActionKey[_iAction].iKey[1]==_iVirtualKey)
		{
			sakActionKey[_iAction].iKey[1]=-1;
		}
		bChange=true;
	}

	if(bSecondChoice)
	{
		bChange=true;
		iOldVirtualKey=-1;
	}

	//on remove les doublons de keys
	int iI=MAX_ACTION_KEY;

	while(iI--)
	{
		if(iI==_iAction) continue;

		if(sakActionKey[iI].iPage!=sakActionKey[_iAction].iPage) continue;

		if(sakActionKey[iI].iKey[0]==_iVirtualKey)
		{
			sakActionKey[iI].iKey[0]=iOldVirtualKey;
			bChange=true;
			break;
		}
		else
		{
			if(sakActionKey[iI].iKey[1]==_iVirtualKey)
			{
				sakActionKey[iI].iKey[1]=iOldVirtualKey;
				bChange=true;
				break;
			}
		}
	}

	return bChange;
}

void Config::WriteConfig( const std::string& section, const std::string& key, ControlAction index )
{
	std::string value;

	value = pGetInfoDirectInput->GetFullNameTouch( sakActionKey[index].iKey[0] );

	if ( value.empty() )
		value = pGetInfoDirectInput->GetFullNameTouch( sakActionDefaultKey[index].iKey[0] );

	WriteConfig( section, key + "_k0", value );

	value = pGetInfoDirectInput->GetFullNameTouch( sakActionKey[index].iKey[1] );

	if ( value.empty() )
		value = pGetInfoDirectInput->GetFullNameTouch( sakActionDefaultKey[index].iKey[1] );

	WriteConfig( section, key + "_k1", value );
}

ActionKey Config::ReadConfig( const std::string& section, const std::string& key, ControlAction index ) const
{
	ActionKey action_key = sakActionDefaultKey[index];

	std::string key_value;

	key_value = ReadConfig( section, key + "_k0" );

	if ( !key_value.empty() )
		action_key.iKey[0] = GetDIKWithASCII( key_value );

	key_value = ReadConfig( section, key + "_k1" );

	if ( !key_value.empty() )
		action_key.iKey[1] = GetDIKWithASCII( key_value );

	return action_key;
}

/**
 * Writes all the data in the config to the section map
 * and then saves the data to file
 */
void Config::SaveAll()
{
	/** Update all the relevant sections in the config with current config data **/

	//language
	WriteConfig( Section::Language, Key::language_string, Project.localisationpath );
	WriteConfig( Section::FirstRun, Key::first_run_int, first_launch );

	//video
	WriteConfig( Section::Video, Key::resolution, itoa( iWidth ) + 'x' + itoa( iHeight ) );
	WriteConfig( Section::Video, Key::bpp, bpp);
	WriteConfig( Section::Video, Key::full_screen, bFullScreen );
	WriteConfig( Section::Video, Key::bump, bBumpMapping );
	WriteConfig( Section::Video, Key::texture, iTextureResol);
	WriteConfig( Section::Video, Key::mesh_reduction, iMeshReduction);
	WriteConfig( Section::Video, Key::others_details, iLevelOfDetails);
	WriteConfig( Section::Video, Key::fog, iFogDistance);
	WriteConfig( Section::Video, Key::gamma, iGamma);
	WriteConfig( Section::Video, Key::luminosity, iLuminosite);
	WriteConfig( Section::Video, Key::contrast, iContrast);
	WriteConfig( Section::Video, Key::show_crosshair, bShowCrossHair );
	WriteConfig( Section::Video, Key::antialiasing, bAntiAliasing );

	//audio
	WriteConfig( Section::Audio, Key::master_volume, iMasterVolume );
	WriteConfig( Section::Audio, Key::effects_volume, iSFXVolume);
	WriteConfig( Section::Audio, Key::speech_volume, iSpeechVolume);
	WriteConfig( Section::Audio, Key::ambiance_volume, iAmbianceVolume);
	WriteConfig( Section::Audio, Key::EAX, bEAX );

	//input
	WriteConfig( Section::Input, Key::invert_mouse, bInvertMouse );
	WriteConfig( Section::Input, Key::auto_ready_weapon, bAutoReadyWeapon );
	WriteConfig( Section::Input, Key::mouse_look_toggle, bMouseLookToggle );
	WriteConfig( Section::Input, Key::mouse_sensitivity, iMouseSensitivity );
	WriteConfig( Section::Input, Key::mouse_smoothing, bMouseSmoothing );
	WriteConfig( Section::Input, Key::auto_description, bAutoDescription );
	WriteConfigInt( Section::Input, Key::link_mouse_look_to_use, bLinkMouseLookToUse );

	//key
	WriteConfig( Section::Key, Key::jump, CONTROLS_CUST_JUMP );
	WriteConfig( Section::Key, Key::magic_mode, CONTROLS_CUST_MAGICMODE );
	WriteConfig( Section::Key, Key::stealth_mode, CONTROLS_CUST_STEALTHMODE);
	WriteConfig( Section::Key, Key::walk_forward, CONTROLS_CUST_WALKFORWARD);
	WriteConfig( Section::Key, Key::walk_backward, CONTROLS_CUST_WALKBACKWARD);
	WriteConfig( Section::Key, Key::strafe_left, CONTROLS_CUST_STRAFELEFT);
	WriteConfig( Section::Key, Key::strafe_right, CONTROLS_CUST_STRAFERIGHT);
	WriteConfig( Section::Key, Key::lean_left, CONTROLS_CUST_LEANLEFT);
	WriteConfig( Section::Key, Key::lean_right, CONTROLS_CUST_LEANRIGHT);
	WriteConfig( Section::Key, Key::crouch, CONTROLS_CUST_CROUCH);
	WriteConfig( Section::Key, Key::mouselook, CONTROLS_CUST_MOUSELOOK);
	WriteConfig( Section::Key, Key::action_combine, CONTROLS_CUST_ACTION);
	WriteConfig( Section::Key, Key::inventory, CONTROLS_CUST_INVENTORY);
	WriteConfig( Section::Key, Key::book, CONTROLS_CUST_BOOK);
	WriteConfig( Section::Key, Key::char_sheet, CONTROLS_CUST_BOOKCHARSHEET);
	WriteConfig( Section::Key, Key::magic_book, CONTROLS_CUST_BOOKSPELL);
	WriteConfig( Section::Key, Key::map, CONTROLS_CUST_BOOKMAP);
	WriteConfig( Section::Key, Key::quest_book, CONTROLS_CUST_BOOKQUEST);
	WriteConfig( Section::Key, Key::drink_potion_life, CONTROLS_CUST_DRINKPOTIONLIFE);
	WriteConfig( Section::Key, Key::drink_potion_mana, CONTROLS_CUST_DRINKPOTIONMANA);
	WriteConfig( Section::Key, Key::torch, CONTROLS_CUST_TORCH);

	WriteConfig( Section::Key, Key::cancel_current_spell, CONTROLS_CUST_CANCELCURSPELL);
	WriteConfig( Section::Key, Key::precast_1, CONTROLS_CUST_PRECAST1);
	WriteConfig( Section::Key, Key::precast_2, CONTROLS_CUST_PRECAST2);
	WriteConfig( Section::Key, Key::precast_3, CONTROLS_CUST_PRECAST3);
	WriteConfig( Section::Key, Key::draw_weapon, CONTROLS_CUST_WEAPON);
	WriteConfig( Section::Key, Key::quicksave, CONTROLS_CUST_QUICKSAVE);
	WriteConfig( Section::Key, Key::quickload, CONTROLS_CUST_QUICKLOAD);

	WriteConfig( Section::Key, Key::turn_left, CONTROLS_CUST_TURNLEFT);
	WriteConfig( Section::Key, Key::turn_right, CONTROLS_CUST_TURNRIGHT);
	WriteConfig( Section::Key, Key::look_up, CONTROLS_CUST_LOOKUP);
	WriteConfig( Section::Key, Key::look_down, CONTROLS_CUST_LOOKDOWN);

	WriteConfig( Section::Key, Key::strafe, CONTROLS_CUST_STRAFE);
	WriteConfig( Section::Key, Key::center_view, CONTROLS_CUST_CENTERVIEW);

	WriteConfig( Section::Key, Key::freelook, CONTROLS_CUST_FREELOOK);

	WriteConfig( Section::Key, Key::previous, CONTROLS_CUST_PREVIOUS);
	WriteConfig( Section::Key, Key::next, CONTROLS_CUST_NEXT);

	WriteConfig( Section::Key, Key::crouch_toggle, CONTROLS_CUST_CROUCHTOGGLE);

	WriteConfig( Section::Key, Key::unequip_weapon, CONTROLS_CUST_UNEQUIPWEAPON);

	WriteConfig( Section::Key, Key::minimap, CONTROLS_CUST_MINIMAP);

	//misc
	WriteConfig( Section::Misc, Key::softfog, bATI );
	WriteConfig( Section::Misc, Key::forcenoeax, bForceNoEAX );
	WriteConfig( Section::Misc, Key::forcezbias, bForceZBias );
	WriteConfig( Section::Misc, Key::newcontrol, (bool)INTERNATIONAL_MODE );
	WriteConfig( Section::Misc, Key::forcetoggle, bOneHanded );
	WriteConfig( Section::Misc, Key::fg, (bool)uiGoreMode );

	// Finally save it all to file/stream
	std::ofstream out( pcName.c_str() );
	config_map.save_all( out );
}

extern bool IsNoGore( void );

void Config::ReadAll()
{
	bool bWarningGore=false;

	// Check if this is the first run of the game
	first_launch = ReadConfig( Section::FirstRun, Key::first_run_int, Default::first_run );

	// Get the locale language
	Project.localisationpath = ReadConfig( Section::Language, Key::language_string, Default::language );

	// TODO Config should have no business calling this function. Is needed since gettings keys from the config will call localization functions
	Localisation_Init();

	// Get the video settings
	std::string resolution = ReadConfig( Section::Video, Key::resolution, Default::resolution );
	iWidth = atoi( resolution.substr( 0, resolution.find( 'x' ) ) );
	iHeight = atoi( resolution.substr( resolution.find('x') + 1 ) );
	bpp = ReadConfig( Section::Video, Key::bpp, Default::bpp );
	bFullScreen = ReadConfig( Section::Video, Key::full_screen, Default::full_screen );
	bBumpMapping = ReadConfig( Section::Video, Key::bump, Default::bump );
	iTextureResol = ReadConfig( Section::Video, Key::texture, Default::texture );
	iMeshReduction = ReadConfig( Section::Video, Key::mesh_reduction, Default::mesh_reduction );
	iLevelOfDetails = ReadConfig( Section::Video, Key::others_details, Default::others_details );
	iFogDistance = ReadConfig( Section::Video, Key::fog, Default::fog );
	iGamma = ReadConfig( Section::Video, Key::gamma, Default::gamma );
	iLuminosite = ReadConfig( Section::Video, Key::luminosity, Default::luminosity );
	iContrast = ReadConfig( Section::Video, Key::contrast, Default::contrast );
	bShowCrossHair = ReadConfig( Section::Video, Key::show_crosshair, Default::show_crosshair );
	bAntiAliasing = ReadConfig( Section::Video, Key::antialiasing, Default::antialiasing );

	// Get audio settings
	iMasterVolume = ReadConfig( Section::Audio, Key::master_volume, Default::master_volume );
	iSFXVolume = ReadConfig( Section::Audio, Key::effects_volume, Default::effects_volume );
	iSpeechVolume = ReadConfig( Section::Audio, Key::speech_volume, Default::speech_volume );
	iAmbianceVolume = ReadConfig( Section::Audio, Key::ambiance_volume, Default::ambiance_volume );
	bEAX = ReadConfig( Section::Audio, Key::EAX, Default::EAX );

	// Get input settings
	bInvertMouse = ReadConfig( Section::Input, Key::invert_mouse, Default::invert_mouse );
	bAutoReadyWeapon = ReadConfig( Section::Input, Key::auto_ready_weapon, Default::auto_ready_weapon );
	bMouseLookToggle = ReadConfig( Section::Input, Key::mouse_look_toggle, Default::mouse_look_toggle );
	iMouseSensitivity = ReadConfig( Section::Input, Key::mouse_sensitivity, Default::mouse_sensitivity );
	bMouseSmoothing = ReadConfig( Section::Input, Key::mouse_smoothing, Default::mouse_smoothing );
	bAutoDescription = ReadConfig( Section::Input, Key::auto_description, Default::auto_description );
	bLinkMouseLookToUse = ReadConfig( Section::Input, Key::link_mouse_look_to_use, Default::link_mouse_look_to_use );

	// Get action key settings
	sakActionKey[CONTROLS_CUST_JUMP] = ReadConfig( Section::Key, Key::jump, CONTROLS_CUST_JUMP );
	sakActionKey[CONTROLS_CUST_MAGICMODE] = ReadConfig( Section::Key, Key::magic_mode, CONTROLS_CUST_MAGICMODE );
	sakActionKey[CONTROLS_CUST_STEALTHMODE] = ReadConfig( Section::Key, Key::stealth_mode, CONTROLS_CUST_STEALTHMODE );
	sakActionKey[CONTROLS_CUST_WALKFORWARD] = ReadConfig( Section::Key, Key::walk_forward, CONTROLS_CUST_WALKFORWARD );
	sakActionKey[CONTROLS_CUST_WALKBACKWARD] = ReadConfig( Section::Key, Key::walk_backward, CONTROLS_CUST_WALKBACKWARD );
	sakActionKey[CONTROLS_CUST_STRAFELEFT] = ReadConfig( Section::Key, Key::strafe_left, CONTROLS_CUST_STRAFELEFT );
	sakActionKey[CONTROLS_CUST_STRAFERIGHT] = ReadConfig( Section::Key, Key::strafe_right, CONTROLS_CUST_STRAFERIGHT );
	sakActionKey[CONTROLS_CUST_LEANLEFT] = ReadConfig( Section::Key, Key::lean_left, CONTROLS_CUST_LEANLEFT );
	sakActionKey[CONTROLS_CUST_LEANRIGHT] = ReadConfig( Section::Key, Key::lean_right, CONTROLS_CUST_LEANRIGHT );
	sakActionKey[CONTROLS_CUST_CROUCH] = ReadConfig( Section::Key, Key::crouch, CONTROLS_CUST_CROUCH );
	sakActionKey[CONTROLS_CUST_MOUSELOOK] = ReadConfig( Section::Key, Key::mouselook, CONTROLS_CUST_MOUSELOOK );


	sakActionKey[CONTROLS_CUST_ACTION] = ReadConfig( Section::Key, Key::action_combine, CONTROLS_CUST_ACTION );
	sakActionKey[CONTROLS_CUST_INVENTORY] = ReadConfig( Section::Key, Key::inventory, CONTROLS_CUST_INVENTORY );
	sakActionKey[CONTROLS_CUST_BOOK] = ReadConfig( Section::Key, Key::book, CONTROLS_CUST_BOOK );
	sakActionKey[CONTROLS_CUST_BOOKCHARSHEET] = ReadConfig( Section::Key, Key::char_sheet, CONTROLS_CUST_BOOKCHARSHEET );
	sakActionKey[CONTROLS_CUST_BOOKSPELL] = ReadConfig( Section::Key, Key::magic_book, CONTROLS_CUST_BOOKSPELL );
	sakActionKey[CONTROLS_CUST_BOOKMAP] = ReadConfig( Section::Key, Key::map, CONTROLS_CUST_BOOKMAP );
	sakActionKey[CONTROLS_CUST_BOOKQUEST] = ReadConfig( Section::Key, Key::quest_book, CONTROLS_CUST_BOOKQUEST );
	sakActionKey[CONTROLS_CUST_DRINKPOTIONLIFE] = ReadConfig( Section::Key, Key::drink_potion_life, CONTROLS_CUST_DRINKPOTIONLIFE );
	sakActionKey[CONTROLS_CUST_DRINKPOTIONMANA] = ReadConfig( Section::Key, Key::drink_potion_mana, CONTROLS_CUST_DRINKPOTIONMANA );
	sakActionKey[CONTROLS_CUST_TORCH] = ReadConfig( Section::Key, Key::torch, CONTROLS_CUST_TORCH );

	sakActionKey[CONTROLS_CUST_CANCELCURSPELL] = ReadConfig( Section::Key, Key::cancel_current_spell, CONTROLS_CUST_CANCELCURSPELL );
	sakActionKey[CONTROLS_CUST_PRECAST1] = ReadConfig( Section::Key, Key::precast_1, CONTROLS_CUST_PRECAST1 );
	sakActionKey[CONTROLS_CUST_PRECAST2] = ReadConfig( Section::Key, Key::precast_2, CONTROLS_CUST_PRECAST2 );
	sakActionKey[CONTROLS_CUST_PRECAST3] = ReadConfig( Section::Key, Key::precast_3, CONTROLS_CUST_PRECAST3 );
	sakActionKey[CONTROLS_CUST_WEAPON] = ReadConfig( Section::Key, Key::draw_weapon, CONTROLS_CUST_WEAPON );
	sakActionKey[CONTROLS_CUST_QUICKSAVE] = ReadConfig( Section::Key, Key::quicksave, CONTROLS_CUST_QUICKSAVE );
	sakActionKey[CONTROLS_CUST_QUICKLOAD] = ReadConfig( Section::Key, Key::quickload, CONTROLS_CUST_QUICKLOAD );
	sakActionKey[CONTROLS_CUST_TURNLEFT] = ReadConfig( Section::Key, Key::turn_left, CONTROLS_CUST_TURNLEFT );
	sakActionKey[CONTROLS_CUST_TURNRIGHT] = ReadConfig( Section::Key, Key::turn_right, CONTROLS_CUST_TURNRIGHT );
	sakActionKey[CONTROLS_CUST_LOOKUP] = ReadConfig( Section::Key, Key::look_up, CONTROLS_CUST_LOOKUP );
	sakActionKey[CONTROLS_CUST_LOOKDOWN] = ReadConfig( Section::Key, Key::look_down, CONTROLS_CUST_LOOKDOWN );
	sakActionKey[CONTROLS_CUST_STRAFE] = ReadConfig( Section::Key, Key::strafe, CONTROLS_CUST_STRAFE );
	sakActionKey[CONTROLS_CUST_CENTERVIEW] = ReadConfig( Section::Key, Key::center_view, CONTROLS_CUST_CENTERVIEW );
	sakActionKey[CONTROLS_CUST_FREELOOK] = ReadConfig( Section::Key, Key::freelook, CONTROLS_CUST_FREELOOK );

	sakActionKey[CONTROLS_CUST_PREVIOUS] = ReadConfig( Section::Key, Key::previous, CONTROLS_CUST_PREVIOUS );
	sakActionKey[CONTROLS_CUST_NEXT] = ReadConfig( Section::Key, Key::next, CONTROLS_CUST_NEXT );
	sakActionKey[CONTROLS_CUST_CROUCHTOGGLE] = ReadConfig( Section::Key, Key::crouch_toggle, CONTROLS_CUST_CROUCHTOGGLE );

	sakActionKey[CONTROLS_CUST_UNEQUIPWEAPON] = ReadConfig( Section::Key, Key::unequip_weapon, CONTROLS_CUST_UNEQUIPWEAPON );

	sakActionKey[CONTROLS_CUST_MINIMAP] = ReadConfig( Section::Key, Key::minimap, CONTROLS_CUST_MINIMAP );

	// Get miscellaneous settings
	bATI = ReadConfig( Section::Misc, Key::softfog, Default::softfog );
	bForceNoEAX = ReadConfig( Section::Misc, Key::forcenoeax, Default::forcenoeax );
	bForceZBias = ReadConfig( Section::Misc, Key::forcezbias, Default::forcezbias );
	bOneHanded = ReadConfig( Section::Misc, Key::forcetoggle, Default::forcetoggle );
	uiGoreMode = ReadConfig(Section::Misc, Key::fg, Default::fg );
	pStringMod = ReadConfig( Section::Misc, Key::modpak, Default::modpak );
	pStringModSfx = ReadConfig(Section::Misc, Key::modsfxpak, Default::modsfxpak );
	pStringModSpeech = ReadConfig(Section::Misc, Key::modspeechpak, Default::speechmodpak );

	INTERNATIONAL_MODE = ReadConfig(Section::Misc, Key::newcontrol, Default::newcontrol );

	switch(uiGoreMode)
	{
	case 0:
		{
			if(bWarningGore)
			{
				uiGoreMode=0;
				GORE_MODE=0;
			}
			else
			{
				uiGoreMode=1;
				GORE_MODE=1;
			}
		}
		break;

	case 1:
		GORE_MODE=1;
		break;

	case 2:
		GORE_MODE=0;
		break;

	default:
		uiGoreMode=0;
		GORE_MODE=0;
		break;
	}

	//on set les options

	ARXMenu_Options_Video_SetFogDistance(iFogDistance);
	ARXMenu_Options_Video_SetTextureQuality(iTextureResol);
	ARXMenu_Options_Video_SetBump(bBumpMapping);
	ARXMenu_Options_Video_SetLODQuality(iMeshReduction);
	ARXMenu_Options_Video_SetDetailsQuality(iLevelOfDetails);
	ARXMenu_Options_Video_SetGamma(iGamma);
	ARX_SetAntiAliasing();
	ARXMenu_Options_Audio_SetMasterVolume(iMasterVolume);
	ARXMenu_Options_Audio_SetSfxVolume(iSFXVolume);
	ARXMenu_Options_Audio_SetSpeechVolume(iSpeechVolume);
	ARXMenu_Options_Audio_SetAmbianceVolume(iAmbianceVolume);

	ARXMenu_Options_Control_SetInvertMouse(bInvertMouse);
	ARXMenu_Options_Control_SetAutoReadyWeapon(bAutoReadyWeapon);
	ARXMenu_Options_Control_SetMouseLookToggleMode(bMouseLookToggle);
	ARXMenu_Options_Control_SetMouseSensitivity(iMouseSensitivity);
	ARXMenu_Options_Control_SetAutoDescription(bAutoDescription);

	if(pMenu)
	{
		pMenu->bReInitAll=true;
	}

	//mixer Game
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerGame, ARX_SOUND_MixerGetVolume(ARX_SOUND_MixerMenu));
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerGameSample, ARX_SOUND_MixerGetVolume(ARX_SOUND_MixerMenuSample));
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerGameSpeech, ARX_SOUND_MixerGetVolume(ARX_SOUND_MixerMenuSpeech));
	ARX_SOUND_MixerSetVolume(ARX_SOUND_MixerGameAmbiance, ARX_SOUND_MixerGetVolume(ARX_SOUND_MixerMenuAmbiance));

	Localisation_Close();

	bALLOW_BUMP=bBumpMapping;
	WILL_RELOAD_ALL_TEXTURES=1;
	GORE_MODE = IsNoGore()? 0 : 1;
	Localisation_Init();

	if(bBumpMapping)
		EERIE_ActivateBump();
	else
		EERIE_DesactivateBump();

	if( iTextureResol==2 ) Project.TextureSize=0;

	if( iTextureResol==1 ) Project.TextureSize=2;

	if( iTextureResol==0 ) Project.TextureSize=64;
}

