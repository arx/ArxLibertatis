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

#include <sstream>

#include "core/Localization.h"
#include "graphics/data/Texture.h"
#include "gui/MenuPublic.h"
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
		new_control = false;
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
			magic_mode = "magic_mode"
			;
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
Config *pMenuConfig;

Config::Config()
{
	First();
}

//-----------------------------------------------------------------------------

void Config::First()
{
	ResetActionKey();

	sakActionDefaultKey[CONTROLS_CUST_JUMP].iKey[0]=DIK_SPACE;
	sakActionDefaultKey[CONTROLS_CUST_JUMP].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_MAGICMODE].iKey[0]=DIK_LCONTROL;
	sakActionDefaultKey[CONTROLS_CUST_MAGICMODE].iKey[1]=DIK_RCONTROL;
	sakActionDefaultKey[CONTROLS_CUST_STEALTHMODE].iKey[0]=DIK_LSHIFT;
	sakActionDefaultKey[CONTROLS_CUST_STEALTHMODE].iKey[1]=DIK_RSHIFT;
	sakActionDefaultKey[CONTROLS_CUST_WALKFORWARD].iKey[0] = DIK_W;
	sakActionDefaultKey[CONTROLS_CUST_WALKFORWARD].iKey[1]=DIK_UP;
	sakActionDefaultKey[CONTROLS_CUST_WALKBACKWARD].iKey[0] = DIK_S;
	sakActionDefaultKey[CONTROLS_CUST_WALKBACKWARD].iKey[1]=DIK_DOWN;
	sakActionDefaultKey[CONTROLS_CUST_STRAFELEFT].iKey[0] = DIK_A;
	sakActionDefaultKey[CONTROLS_CUST_STRAFELEFT].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_STRAFERIGHT].iKey[0] = DIK_D;
	sakActionDefaultKey[CONTROLS_CUST_STRAFERIGHT].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_LEANLEFT].iKey[0] = DIK_Q;
	sakActionDefaultKey[CONTROLS_CUST_LEANLEFT].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_LEANRIGHT].iKey[0] = DIK_E;
	sakActionDefaultKey[CONTROLS_CUST_LEANRIGHT].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_CROUCH].iKey[0] = DIK_X;
	sakActionDefaultKey[CONTROLS_CUST_CROUCH].iKey[1]=-1;

	if (INTERNATIONAL_MODE)
	{
		sakActionDefaultKey[CONTROLS_CUST_MOUSELOOK].iKey[0]=DIK_F;
		sakActionDefaultKey[CONTROLS_CUST_MOUSELOOK].iKey[1]=DIK_RETURN;
	}
	else
	{
		sakActionDefaultKey[CONTROLS_CUST_MOUSELOOK].iKey[0]=DIK_BUTTON2;
		sakActionDefaultKey[CONTROLS_CUST_MOUSELOOK].iKey[1]=-1;
	}

	sakActionDefaultKey[CONTROLS_CUST_ACTION].iKey[0]=DIK_BUTTON1;
	sakActionDefaultKey[CONTROLS_CUST_ACTION].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_INVENTORY].iKey[0] = DIK_I;
	sakActionDefaultKey[CONTROLS_CUST_INVENTORY].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_INVENTORY].iPage=1;
	sakActionDefaultKey[CONTROLS_CUST_BOOK].iKey[0]=DIK_BACKSPACE;
	sakActionDefaultKey[CONTROLS_CUST_BOOK].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_BOOK].iPage=1;
	sakActionDefaultKey[CONTROLS_CUST_BOOKCHARSHEET].iKey[0] = DIK_F1;
	sakActionDefaultKey[CONTROLS_CUST_BOOKCHARSHEET].iPage=1;
	sakActionDefaultKey[CONTROLS_CUST_BOOKSPELL].iKey[0] = DIK_F2;
	sakActionDefaultKey[CONTROLS_CUST_BOOKSPELL].iPage=1;
	sakActionDefaultKey[CONTROLS_CUST_BOOKMAP].iKey[0] = DIK_F3;
	sakActionDefaultKey[CONTROLS_CUST_BOOKMAP].iPage=1;
	sakActionDefaultKey[CONTROLS_CUST_BOOKQUEST].iKey[0] = DIK_F4;
	sakActionDefaultKey[CONTROLS_CUST_BOOKQUEST].iPage=1;
	sakActionDefaultKey[CONTROLS_CUST_DRINKPOTIONLIFE].iKey[0] = DIK_H;
	sakActionDefaultKey[CONTROLS_CUST_DRINKPOTIONLIFE].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_DRINKPOTIONLIFE].iPage=1;
	sakActionDefaultKey[CONTROLS_CUST_DRINKPOTIONMANA].iKey[0] = DIK_G;
	sakActionDefaultKey[CONTROLS_CUST_DRINKPOTIONMANA].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_DRINKPOTIONMANA].iPage=1;
	sakActionDefaultKey[CONTROLS_CUST_TORCH].iKey[0]=DIK_T;
	sakActionDefaultKey[CONTROLS_CUST_TORCH].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_TORCH].iPage=1;

	sakActionDefaultKey[CONTROLS_CUST_CANCELCURSPELL].iKey[0] = DIK_4;
	sakActionDefaultKey[CONTROLS_CUST_CANCELCURSPELL].iKey[1] = -1;
	sakActionDefaultKey[CONTROLS_CUST_CANCELCURSPELL].iPage=1;

	sakActionDefaultKey[CONTROLS_CUST_PRECAST1].iKey[0] = DIK_1;
	sakActionDefaultKey[CONTROLS_CUST_PRECAST1].iPage=1;
	sakActionDefaultKey[CONTROLS_CUST_PRECAST2].iKey[0] = DIK_2;
	sakActionDefaultKey[CONTROLS_CUST_PRECAST2].iPage=1;
	sakActionDefaultKey[CONTROLS_CUST_PRECAST3].iKey[0] = DIK_3;
	sakActionDefaultKey[CONTROLS_CUST_PRECAST3].iPage=1;
	sakActionDefaultKey[CONTROLS_CUST_WEAPON].iKey[0] = DIK_TAB;
	sakActionDefaultKey[CONTROLS_CUST_WEAPON].iKey[1] = DIK_NUMPAD0;
	sakActionDefaultKey[CONTROLS_CUST_WEAPON].iPage=1;
	sakActionDefaultKey[CONTROLS_CUST_QUICKLOAD].iKey[0] = DIK_F9;
	sakActionDefaultKey[CONTROLS_CUST_QUICKLOAD].iPage=1;
	sakActionDefaultKey[CONTROLS_CUST_QUICKSAVE].iKey[0] = DIK_F5;
	sakActionDefaultKey[CONTROLS_CUST_QUICKSAVE].iPage=1;

	sakActionDefaultKey[CONTROLS_CUST_TURNLEFT].iKey[0]=DIK_LEFT;
	sakActionDefaultKey[CONTROLS_CUST_TURNLEFT].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_TURNRIGHT].iKey[0]=DIK_RIGHT;
	sakActionDefaultKey[CONTROLS_CUST_TURNRIGHT].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_LOOKUP].iKey[0]=DIK_PGUP;
	sakActionDefaultKey[CONTROLS_CUST_LOOKUP].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_LOOKDOWN].iKey[0]=DIK_PGDN;
	sakActionDefaultKey[CONTROLS_CUST_LOOKDOWN].iKey[1]=-1;

	sakActionDefaultKey[CONTROLS_CUST_STRAFE].iKey[0]=DIK_LALT;
	sakActionDefaultKey[CONTROLS_CUST_STRAFE].iKey[1]=-1;
	sakActionDefaultKey[CONTROLS_CUST_CENTERVIEW].iKey[0]=DIK_END;
	sakActionDefaultKey[CONTROLS_CUST_CENTERVIEW].iKey[1]=-1;

	if (INTERNATIONAL_MODE)
	{
		sakActionDefaultKey[CONTROLS_CUST_FREELOOK].iKey[0]=DIK_L;
		sakActionDefaultKey[CONTROLS_CUST_FREELOOK].iKey[1]=DIK_BUTTON2;
	}
	else
	{
		sakActionDefaultKey[CONTROLS_CUST_FREELOOK].iKey[0]=DIK_L;
		sakActionDefaultKey[CONTROLS_CUST_FREELOOK].iKey[1]=-1;
	}

	sakActionDefaultKey[CONTROLS_CUST_PREVIOUS].iKey[0] = GetDIKWithASCII(")");
	sakActionDefaultKey[CONTROLS_CUST_PREVIOUS].iPage=1;
	sakActionDefaultKey[CONTROLS_CUST_NEXT].iKey[0] = GetDIKWithASCII("=");
	sakActionDefaultKey[CONTROLS_CUST_NEXT].iPage=1;

	sakActionDefaultKey[CONTROLS_CUST_CROUCHTOGGLE].iKey[0] = GetDIKWithASCII("C");

	sakActionDefaultKey[CONTROLS_CUST_UNEQUIPWEAPON].iKey[0] = DIK_B;
	sakActionDefaultKey[CONTROLS_CUST_UNEQUIPWEAPON].iKey[1] = -1;
	sakActionDefaultKey[CONTROLS_CUST_UNEQUIPWEAPON].iPage=1;

	sakActionDefaultKey[CONTROLS_CUST_MINIMAP].iKey[0] = DIK_R;
	sakActionDefaultKey[CONTROLS_CUST_MINIMAP].iKey[1] = DIK_M;
	sakActionDefaultKey[CONTROLS_CUST_MINIMAP].iPage=1;

	bChangeResolution = false;
	bChangeTextures = false;
	bNoReturnToWindows=false;
	bLinkMouseLookToUse=false;
	bForceZBias=false;

	SetDefaultKey();
	DefaultValue();
}

//-----------------------------------------------------------------------------

Config::Config( const std::string& _pName)
{
	// if _pName equals exactly "cfg"
	if ( strcasecmp( _pName.c_str(), "cfg" ) == 0 )
	{
		pcName="cfg.ini";
	}
	else
	{
		pcName = _pName;
	}
	
	// TODO GetPrivateProfileString needs an absolute path
	if(pcName.length() > 2 && pcName[1] != ':') {
		
		char cwd[512];
		GetCurrentDirectory(512, cwd);
		if(cwd[strlen(cwd)-1] != '\\' && pcName[0] != '\\') {
			pcName = cwd + ('\\' + pcName);
		} else {
			pcName = cwd + pcName;
		}
		
	}

	First();
}

//-----------------------------------------------------------------------------

void Config::DefaultValue()
{
	//VIDEO
	iWidth=640;
	iHeight=480;
	iNewWidth=iWidth;
	iNewHeight=iHeight;
	bpp=16;
	iNewBpp=bpp;
	bFullScreen=true;
	bBumpMapping=false;
	bNewBumpMapping=bBumpMapping;
	iTextureResol=2;
	iNewTextureResol=iTextureResol;
	iMeshReduction=0;
	iLevelOfDetails=2;
	iFogDistance=5;
	iLuminosite=4;
	iContrast=5;
	iGamma=5;
	bShowCrossHair=true;
	//AUDIO
	iMasterVolume=10;
	iSFXVolume=10;
	iSpeechVolume=10;
	iAmbianceVolume=8;
	bEAX=false;
	//INPUT
	bInvertMouse=false;
	bAutoReadyWeapon=false;
	bMouseLookToggle=false;
	bAutoDescription=true;
	iMouseSensitivity=4;
	bMouseSmoothing=false;
	//MISC
	INTERNATIONAL_MODE=1;
}

Config::~Config()
{
}

//-----------------------------------------------------------------------------

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
	{
		bLinkMouseLookToUse=true;
	}
	else
	{
		bLinkMouseLookToUse=false;
	}
}

int Config::GetDIKWithASCII( const std::string& _pcTouch)
{
	std::string pcT = _pcTouch;
	

	if( strcasecmp(pcT.c_str(), "---"  ) == 0 )
	{
		return -1;
	}

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

	for(int iI=DIK_WHEELUP;iI<=DIK_WHEELDOWN;iI++)
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
	
	// TODO unify with localisation loading (and make platform-independent)
	char text[256];
	GetPrivateProfileString( _section.c_str(), _key.c_str(), "", text, 256, pcName.c_str());
	
	LogDebug << "Read section: " << _section << " key: " << _key << " from " << pcName << " as:" << text;

	return std::string( text );
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

//-----------------------------------------------------------------------------

bool Config::WriteConfig( const std::string& _pcSection, const std::string& _pcKey, const std::string& _pcDatas)
{
	int iErreur=0;

	char tcText[256];

	if(!GetPrivateProfileSection(_pcSection.c_str(),tcText,256,pcName.c_str()))
	{
		if(WritePrivateProfileSection(_pcSection.c_str(),"",pcName.c_str())) iErreur++;
	}

	if(WritePrivateProfileString(_pcSection.c_str(),_pcKey.c_str(),_pcDatas.c_str(),pcName.c_str())) iErreur++;

	return (iErreur==2);
}

//-----------------------------------------------------------------------------

bool Config::WriteConfigInt( const std::string& _pcSection, const std::string& _pcKey, int data )
{
	return WriteConfig(_pcSection,_pcKey, itoa( data ) );
}

//-----------------------------------------------------------------------------

bool Config::WriteConfigString( const std::string& _pcSection, const std::string& _pcKey, const std::string& _pcDatas)
{
	return WriteConfig(_pcSection,_pcKey,_pcDatas);
}

//-----------------------------------------------------------------------------

void Config::ResetActionKey()
{
	for (unsigned int iI=0; iI<MAX_ACTION_KEY; iI++)
	{
		sakActionKey[iI].iKey[0] = sakActionKey[iI].iKey[1]=-1;

		sakActionDefaultKey[iI].iKey[0] = sakActionDefaultKey[iI].iKey[1]=-1;

		sakActionKey[iI].iPage=0;
		sakActionDefaultKey[iI].iPage=0;
	}
}

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

//-----------------------------------------------------------------------------

bool Config::WriteConfigKey( const std::string& _pcKey,int _iAction)
{
	char tcTxt[256];
	char tcTxt2[256];
	std::string pcText;
	bool bOk=true;
	std::string pcText1;

	strcpy(tcTxt,_pcKey.c_str());

	int iL;
	pcText1 = pGetInfoDirectInput->GetFullNameTouch(sakActionKey[_iAction].iKey[0]);
	iL = pcText1.length() + 1;

	pcText = pcText1;

	if( !pcText.empty() )
	{
		strcpy(tcTxt2,tcTxt);
		strcat(tcTxt2,"_k0");
		bOk&=WriteConfigString("KEY",tcTxt2,pcText);
	}

	pcText1 = pGetInfoDirectInput->GetFullNameTouch(sakActionKey[_iAction].iKey[1]);
	iL = pcText1.length() + 1;
		
	pcText = pcText1;

	if( !pcText.empty() )
	{
		strcpy(tcTxt2,tcTxt);
		strcat(tcTxt2,"_k1");
		bOk&=WriteConfigString("KEY",tcTxt2,pcText);
	}

	return bOk;
}

//-----------------------------------------------------------------------------

void Config::ReInitActionKey(CWindowMenuConsole *_pwmcWindowMenuConsole)
{
	int iID=BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP1;
	int iI=MAX_ACTION_KEY;
	bool bOldTouch=pGetInfoDirectInput->bTouch;
	int iOldVirtualKey=pGetInfoDirectInput->iKeyId;
	pGetInfoDirectInput->bTouch=true;

	while(iI--)
	{
		int iTab=(iID-BUTTON_MENUOPTIONS_CONTROLS_CUST_JUMP1)>>1;

		CMenuZone *pmzMenuZone = _pwmcWindowMenuConsole->MenuAllZone.GetZoneWithID(iID);

		if (pmzMenuZone)
		{
			if(pmzMenuZone)
			{
				_pwmcWindowMenuConsole->pZoneClick=(CMenuElement*)pmzMenuZone;
				pGetInfoDirectInput->iKeyId=sakActionKey[iTab].iKey[0];
				_pwmcWindowMenuConsole->GetTouch();
			}

			pmzMenuZone=_pwmcWindowMenuConsole->MenuAllZone.GetZoneWithID(iID+1);

			if(pmzMenuZone)
			{
				_pwmcWindowMenuConsole->pZoneClick=(CMenuElement*)pmzMenuZone;
				pGetInfoDirectInput->iKeyId=sakActionKey[iTab].iKey[1];
				_pwmcWindowMenuConsole->GetTouch();
			}
		}

		iID+=2;
	}

	pGetInfoDirectInput->bTouch=bOldTouch;
	pGetInfoDirectInput->iKeyId=iOldVirtualKey;
}

//-----------------------------------------------------------------------------

bool Config::ReadConfigKey( const std::string& _pcKey, int _iAction )
{
	char tcTxt[256];
	char tcTxt2[256];
	std::string pcText;
	bool bOk=true;
	strcpy(tcTxt, _pcKey.c_str());


	int iDIK;
	strcpy(tcTxt2,tcTxt);
	strcat(tcTxt2,"_k0");
	pcText = ReadConfig( "KEY", tcTxt2 );

	if( pcText.empty() )
		bOk=false;
	else
	{
		iDIK = GetDIKWithASCII( pcText );

		if( iDIK == -1 )
			sakActionKey[_iAction].iKey[0]=-1;
		else
			SetActionKey( _iAction, 0, iDIK) ;
	}

	strcpy( tcTxt2, tcTxt );
	strcat( tcTxt2, "_k1" );
	pcText = ReadConfig( "KEY", tcTxt2 );

	if( pcText.empty() )
		bOk = false;
	else
	{
		iDIK = GetDIKWithASCII( pcText );

		if( iDIK == -1 )
		{
			sakActionKey[_iAction].iKey[1]=-1;
		}
		else
		{
			SetActionKey( _iAction, 1, iDIK );
		}
	}

	return bOk;
}

//-----------------------------------------------------------------------------

bool Config::SaveAll()
{
	char tcTxt[256];
	bool bOk=true;

	//language
	strcpy(tcTxt,"\"");
	strcat(tcTxt,Project.localisationpath.c_str());
	strcat(tcTxt,"\"");
	bOk&=WriteConfigString( Section::Language, Key::language_string, tcTxt);
	bOk&=WriteConfigInt("FIRSTRUN","int", first_launch?1:0);
	//video
	sprintf(tcTxt,"%dx%d",iWidth,iHeight);
	bOk&=WriteConfigString("VIDEO","resolution",tcTxt);
	bOk&=WriteConfigInt("VIDEO","bpp",bpp);
	bOk&=WriteConfigInt("VIDEO","full_screen",(bFullScreen)?1:0);
	bOk&=WriteConfigInt("VIDEO","bump",(bBumpMapping)?1:0);
	bOk&=WriteConfigInt("VIDEO","texture",iTextureResol);
	bOk&=WriteConfigInt("VIDEO","mesh_reduction",iMeshReduction);
	bOk&=WriteConfigInt("VIDEO","others_details",iLevelOfDetails);
	bOk&=WriteConfigInt("VIDEO","fog",iFogDistance);
	bOk&=WriteConfigInt("VIDEO","gamma",iGamma);
	bOk&=WriteConfigInt("VIDEO","luminosity",iLuminosite);
	bOk&=WriteConfigInt("VIDEO","contrast",iContrast);
	bOk&=WriteConfigInt("VIDEO","show_crosshair",bShowCrossHair?1:0);
	bOk&=WriteConfigInt("VIDEO","antialiasing",bAntiAliasing?1:0);
	//audio
	bOk&=WriteConfigInt("AUDIO","master_volume",iMasterVolume);
	bOk&=WriteConfigInt("AUDIO","effects_volume",iSFXVolume);
	bOk&=WriteConfigInt("AUDIO","speech_volume",iSpeechVolume);
	bOk&=WriteConfigInt("AUDIO","ambiance_volume",iAmbianceVolume);
	bOk&=WriteConfigInt("AUDIO","EAX",(bEAX)?1:0);
	//input
	bOk&=WriteConfigInt("INPUT","invert_mouse",(bInvertMouse)?1:0);
	bOk&=WriteConfigInt("INPUT","auto_ready_weapon",(bAutoReadyWeapon)?1:0);
	bOk&=WriteConfigInt("INPUT","mouse_look_toggle",(bMouseLookToggle)?1:0);
	bOk&=WriteConfigInt("INPUT","mouse_sensitivity",iMouseSensitivity);
	bOk&=WriteConfigInt("INPUT","mouse_smoothing",(bMouseSmoothing)?1:0);
	bOk&=WriteConfigInt("INPUT","auto_description",(bAutoDescription)?1:0);
	//key
	bOk&=WriteConfigKey("jump",CONTROLS_CUST_JUMP);
	bOk&=WriteConfigKey("magic_mode",CONTROLS_CUST_MAGICMODE);
	bOk&=WriteConfigKey("stealth_mode",CONTROLS_CUST_STEALTHMODE);
	bOk&=WriteConfigKey("walk_forward",CONTROLS_CUST_WALKFORWARD);
	bOk&=WriteConfigKey("walk_backward",CONTROLS_CUST_WALKBACKWARD);
	bOk&=WriteConfigKey("strafe_left",CONTROLS_CUST_STRAFELEFT);
	bOk&=WriteConfigKey("strafe_right",CONTROLS_CUST_STRAFERIGHT);
	bOk&=WriteConfigKey("lean_left",CONTROLS_CUST_LEANLEFT);
	bOk&=WriteConfigKey("lean_right",CONTROLS_CUST_LEANRIGHT);
	bOk&=WriteConfigKey("crouch",CONTROLS_CUST_CROUCH);
	bOk&=WriteConfigKey("mouselook",CONTROLS_CUST_MOUSELOOK);
	bOk&=WriteConfigInt("INPUT","link_mouse_look_to_use",(bLinkMouseLookToUse)?1:0);
	bOk&=WriteConfigKey("action_combine",CONTROLS_CUST_ACTION);
	bOk&=WriteConfigKey("inventory",CONTROLS_CUST_INVENTORY);
	bOk&=WriteConfigKey("book",CONTROLS_CUST_BOOK);
	bOk&=WriteConfigKey("char_sheet",CONTROLS_CUST_BOOKCHARSHEET);
	bOk&=WriteConfigKey("magic_book",CONTROLS_CUST_BOOKSPELL);
	bOk&=WriteConfigKey("map",CONTROLS_CUST_BOOKMAP);
	bOk&=WriteConfigKey("quest_book",CONTROLS_CUST_BOOKQUEST);
	bOk&=WriteConfigKey("drink_potion_life",CONTROLS_CUST_DRINKPOTIONLIFE);
	bOk&=WriteConfigKey("drink_potion_mana",CONTROLS_CUST_DRINKPOTIONMANA);
	bOk&=WriteConfigKey("torch",CONTROLS_CUST_TORCH);

	bOk&=WriteConfigKey("cancel_current_spell",CONTROLS_CUST_CANCELCURSPELL);
	bOk&=WriteConfigKey("precast_1",CONTROLS_CUST_PRECAST1);
	bOk&=WriteConfigKey("precast_2",CONTROLS_CUST_PRECAST2);
	bOk&=WriteConfigKey("precast_3",CONTROLS_CUST_PRECAST3);
	bOk&=WriteConfigKey("draw_weapon",CONTROLS_CUST_WEAPON);
	bOk&=WriteConfigKey("quicksave",CONTROLS_CUST_QUICKSAVE);
	bOk&=WriteConfigKey("quickload",CONTROLS_CUST_QUICKLOAD);

	bOk&=WriteConfigKey("turn_left",CONTROLS_CUST_TURNLEFT);
	bOk&=WriteConfigKey("turn_right",CONTROLS_CUST_TURNRIGHT);
	bOk&=WriteConfigKey("look_up",CONTROLS_CUST_LOOKUP);
	bOk&=WriteConfigKey("look_down",CONTROLS_CUST_LOOKDOWN);

	bOk&=WriteConfigKey("strafe",CONTROLS_CUST_STRAFE);
	bOk&=WriteConfigKey("center_view",CONTROLS_CUST_CENTERVIEW);

	bOk&=WriteConfigKey("freelook",CONTROLS_CUST_FREELOOK);

	bOk&=WriteConfigKey("previous",CONTROLS_CUST_PREVIOUS);
	bOk&=WriteConfigKey("next",CONTROLS_CUST_NEXT);

	bOk&=WriteConfigKey("crouch_toggle",CONTROLS_CUST_CROUCHTOGGLE);

	bOk&=WriteConfigKey("unequip_weapon",CONTROLS_CUST_UNEQUIPWEAPON);

	bOk&=WriteConfigKey("minimap",CONTROLS_CUST_MINIMAP);

	//misc
	bOk&=WriteConfigInt(Section::Misc,"softfog",(bATI)?1:0);
	bOk&=WriteConfigInt(Section::Misc,"forcenoeax",(bForceNoEAX)?1:0);
	bOk&=WriteConfigInt(Section::Misc,"forcezbias",(bForceZBias)?1:0);
	bOk&=WriteConfigInt(Section::Misc,"newcontrol",(INTERNATIONAL_MODE)?1:0);
	bOk&=WriteConfigInt(Section::Misc,"forcetoggle",(bOneHanded)?1:0);
	bOk&=WriteConfigInt(Section::Misc,"fg",uiGoreMode);
	return bOk;
}

extern bool IsNoGore( void );

bool Config::ReadAll()
{
	bool bOk = false; 
	bool bOkTemp;
	int iTemp;
	bool bWarningGore=false;

	// Check if this is the first run of the game
	first_launch = ReadConfig( Section::FirstRun, Key::first_run_int, 1 );

	// Get the locale language
	Project.localisationpath = ReadConfig( Section::Language, Key::language_string, Default::language );

	// Get the video settings
	std::string resolution = ReadConfig( Section::Video, Key::resolution, Default::resolution );
	iWidth = atoi( resolution.substr( 0, resolution.find( 'x' ) ) );
	iHeight = atoi( resolution.substr( resolution.find('x') + 1 ) );
	bpp = ReadConfig( Section::Video, Key::bpp, Default::bpp );
	bFullScreen = ReadConfig( Section::Video, Key::full_screen, 1 );
	bBumpMapping = ReadConfig( Section::Video, Key::bump, 0 );
	iTextureResol = ReadConfig( Section::Video, Key::texture, Default::texture );
	iMeshReduction = ReadConfig( Section::Video, Key::mesh_reduction, Default::mesh_reduction );
	iLevelOfDetails = ReadConfig( Section::Video, Key::others_details, Default::others_details );
	iFogDistance = ReadConfig( Section::Video, Key::fog, Default::fog );
	iGamma = ReadConfig( Section::Video, Key::gamma, Default::gamma );
	iLuminosite = ReadConfig( Section::Video, Key::luminosity, Default::luminosity );
	iContrast = ReadConfig( Section::Video, Key::contrast, Default::contrast );
	bShowCrossHair = ReadConfig( Section::Video, Key::show_crosshair, 1 );
	bAntiAliasing = ReadConfig( Section::Video, Key::antialiasing, 0 );

	// Get audio settings
	iSFXVolume = ReadConfig( Section::Audio, Key::master_volume, Default::master_volume );
	iSFXVolume = ReadConfig( Section::Audio, Key::effects_volume, Default::effects_volume );
	iSpeechVolume = ReadConfig( Section::Audio, Key::speech_volume, Default::speech_volume );
	iAmbianceVolume = ReadConfig( Section::Audio, Key::ambiance_volume, Default::ambiance_volume );
	bEAX = ReadConfig( Section::Audio, Key::EAX, 0 );

	// Get input settings
	bInvertMouse = ReadConfig( Section::Input, Key::invert_mouse, 0 );
	bAutoReadyWeapon = ReadConfig( Section::Input, Key::auto_ready_weapon, 0 );
	bMouseLookToggle = ReadConfig( Section::Input, Key::mouse_look_toggle, 0 );
	iMouseSensitivity = ReadConfig( Section::Input, Key::mouse_sensitivity, Default::mouse_sensitivity );
	bMouseSmoothing = ReadConfig( Section::Input, Key::mouse_smoothing, 0 );
	bAutoDescription = ReadConfig( Section::Input, Key::auto_description, 1 );
	bLinkMouseLookToUse = ReadConfig( Section::Input, Key::link_mouse_look_to_use, 0 );

	//key
	bool bOk2=true;
	bOk2&=ReadConfigKey("jump",CONTROLS_CUST_JUMP);
	bOk2&=ReadConfigKey("magic_mode",CONTROLS_CUST_MAGICMODE);
	bOk2&=ReadConfigKey("stealth_mode",CONTROLS_CUST_STEALTHMODE);
	bOk2&=ReadConfigKey("walk_forward",CONTROLS_CUST_WALKFORWARD);
	bOk2&=ReadConfigKey("walk_backward",CONTROLS_CUST_WALKBACKWARD);
	bOk2&=ReadConfigKey("strafe_left",CONTROLS_CUST_STRAFELEFT);
	bOk2&=ReadConfigKey("strafe_right",CONTROLS_CUST_STRAFERIGHT);
	bOk2&=ReadConfigKey("lean_left",CONTROLS_CUST_LEANLEFT);
	bOk2&=ReadConfigKey("lean_right",CONTROLS_CUST_LEANRIGHT);
	bOk2&=ReadConfigKey("crouch",CONTROLS_CUST_CROUCH);
	bOk2&=ReadConfigKey("mouselook",CONTROLS_CUST_MOUSELOOK);


	bOk2&=ReadConfigKey("action_combine",CONTROLS_CUST_ACTION);
	bOk2&=ReadConfigKey("inventory",CONTROLS_CUST_INVENTORY);
	bOk2&=ReadConfigKey("book",CONTROLS_CUST_BOOK);
	bOk2&=ReadConfigKey("char_sheet",CONTROLS_CUST_BOOKCHARSHEET);
	bOk2&=ReadConfigKey("magic_book",CONTROLS_CUST_BOOKSPELL);
	bOk2&=ReadConfigKey("map",CONTROLS_CUST_BOOKMAP);
	bOk2&=ReadConfigKey("quest_book",CONTROLS_CUST_BOOKQUEST);
	bOk2&=ReadConfigKey("drink_potion_life",CONTROLS_CUST_DRINKPOTIONLIFE);
	bOk2&=ReadConfigKey("drink_potion_mana",CONTROLS_CUST_DRINKPOTIONMANA);
	bOk2&=ReadConfigKey("torch",CONTROLS_CUST_TORCH);

	bOk2&=ReadConfigKey("cancel_current_spell",CONTROLS_CUST_CANCELCURSPELL);
	bOk2&=ReadConfigKey("precast_1",CONTROLS_CUST_PRECAST1);
	bOk2&=ReadConfigKey("precast_2",CONTROLS_CUST_PRECAST2);
	bOk2&=ReadConfigKey("precast_3",CONTROLS_CUST_PRECAST3);
	bOk2&=ReadConfigKey("draw_weapon",CONTROLS_CUST_WEAPON);
	bOk2&=ReadConfigKey("quicksave",CONTROLS_CUST_QUICKSAVE);
	bOk2&=ReadConfigKey("quickload",CONTROLS_CUST_QUICKLOAD);
	bOk2&=ReadConfigKey("turn_left",CONTROLS_CUST_TURNLEFT);
	bOk2&=ReadConfigKey("turn_right",CONTROLS_CUST_TURNRIGHT);
	bOk2&=ReadConfigKey("look_up",CONTROLS_CUST_LOOKUP);
	bOk2&=ReadConfigKey("look_down",CONTROLS_CUST_LOOKDOWN);
	bOk2&=ReadConfigKey("strafe",CONTROLS_CUST_STRAFE);
	bOk2&=ReadConfigKey("center_view",CONTROLS_CUST_CENTERVIEW);

	bOk2&=ReadConfigKey("freelook",CONTROLS_CUST_FREELOOK);

	bOk2&=ReadConfigKey("previous",CONTROLS_CUST_PREVIOUS);
	bOk2&=ReadConfigKey("next",CONTROLS_CUST_NEXT);

	bOk2&=ReadConfigKey("crouch_toggle",CONTROLS_CUST_CROUCHTOGGLE);

	bOk2&=ReadConfigKey("unequip_weapon",CONTROLS_CUST_UNEQUIPWEAPON);

	bOk2&=ReadConfigKey("minimap",CONTROLS_CUST_MINIMAP);

	bOk2&=bOk;

	if(!bOk2)
	{
		int iI=MAX_ACTION_KEY;

		while(iI--)
		{
			sakActionKey[iI].iKey[0]=sakActionDefaultKey[iI].iKey[0];
			sakActionKey[iI].iKey[1]=sakActionDefaultKey[iI].iKey[1];
		}
	}

	// Get miscellaneous settings
	bATI = ReadConfig( Section::Misc, "softfog", 0 );
	bForceNoEAX = ReadConfig( Section::Misc, "forcenoeax", 0 );
	bForceZBias = ReadConfig( Section::Misc, "forcezbias", 0 );
	bOneHanded = ReadConfig( Section::Misc, "forcetoggle", 0 );
	uiGoreMode = ReadConfig(Section::Misc, "fg", 1 );
	pStringMod = ReadConfig( Section::Misc, "mod", Default::modpak );
	pStringModSfx = ReadConfig(Section::Misc, "modsfx", Default::modsfxpak );
	pStringModSpeech = ReadConfig(Section::Misc, "modspeech", Default::speechmodpak );

	iTemp=ReadConfig(Section::Misc,"newcontrol", 0);
	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		INTERNATIONAL_MODE=1;
	}
	else
	{
		INTERNATIONAL_MODE=(iTemp)?1:0;
	}

	if(INTERNATIONAL_MODE)
	{
		bLinkMouseLookToUse=false;
	}

	bOk&=bOkTemp;

	if(!bOkTemp)
	{
		uiGoreMode=bWarningGore?0:1;
	}

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
		{
			GORE_MODE=1;
		}
		break;
	case 2:
		{
			GORE_MODE=0;
		}
		break;
	default:
		{
			uiGoreMode=0;
			GORE_MODE=0;
		}
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

	pMenuConfig->bEAX=bEAX;
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
	{
		EERIE_ActivateBump();
	}
	else
	{
		EERIE_DesactivateBump();
	}

	if( iTextureResol==2 ) Project.TextureSize=0;

	if( iTextureResol==1 ) Project.TextureSize=2;

	if( iTextureResol==0 ) Project.TextureSize=64;

	return bOk;
}

