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
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// ARX_Input
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Input interface (uses MERCURY input system)
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "window/Input.h"

#include <cstdio>

#include "core/Config.h"

#include "gui/MenuWidgets.h" //controls

#include "io/Logger.h"
#include "window/DXInput.h"

extern CDirectInput * pGetInfoDirectInput;
extern long STOP_KEYBOARD_INPUT;

bool ARX_INPUT_Init() {
	return DX7Input::init();
}

void ARX_INPUT_Release() {
	DX7Input::release();
}
 
//-----------------------------------------------------------------------------
bool ARX_IMPULSE_NowPressed(long ident)
{
	switch (ident)
	{
		case CONTROLS_CUST_MOUSELOOK:
		case CONTROLS_CUST_ACTION:
			break;
		default:
		{
			for (long j = 0; j < 2; j++)
			{
				if (config.actions[ident].key[j] != -1)
				{
					if (config.actions[ident].key[j] & Mouse::ButtonBase)
					{
						if (pGetInfoDirectInput->GetMouseButtonNowPressed(config.actions[ident].key[j] & ~Mouse::ButtonBase))
							return true;
					}
					else if (config.actions[ident].key[j] & Mouse::WheelBase)
					{
						if (config.actions[ident].key[j] == Mouse::Wheel_Down)
						{
							if (pGetInfoDirectInput->iWheelSens < 0) return true;
						}
						else
						{
							if (pGetInfoDirectInput->iWheelSens > 0) return true;
						}
					}
					else
					{
						bool bCombine = true;

						if (config.actions[ident].key[j] & 0x7FFF0000)
						{
							if (!pGetInfoDirectInput->IsVirtualKeyPressed((config.actions[ident].key[j] >> 16) & 0xFFFF))
								bCombine = false;
						}

						if (pGetInfoDirectInput->IsVirtualKeyPressedNowPressed(config.actions[ident].key[j] & 0xFFFF))
							return true & bCombine;
					}
				}
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
static unsigned int uiOneHandedMagicMode = 0;
static unsigned int uiOneHandedStealth = 0;

bool ARX_IMPULSE_Pressed(long ident)
{
	switch (ident)
	{
		case CONTROLS_CUST_MOUSELOOK:
		case CONTROLS_CUST_ACTION:
			break;
		default:
		{
			if (config.misc.forceToggle)
			{
				for (long j = 0; j < 2; j++)
				{
					if (config.actions[ident].key[j] != -1)
					{
						if (config.actions[ident].key[j] & Mouse::ButtonBase)
						{
							if (pGetInfoDirectInput->GetMouseButtonRepeat(config.actions[ident].key[j] & ~Mouse::ButtonBase))
								return true;
						}
						else if (config.actions[ident].key[j] & Mouse::WheelBase)
						{
							if (config.actions[ident].key[j] == Mouse::Wheel_Down)
							{
								if (pGetInfoDirectInput->iWheelSens < 0) return true;
							}
							else
							{
								if (pGetInfoDirectInput->iWheelSens > 0) return true;
							}
						}
						else
						{
							bool bCombine = true;

							if (config.actions[ident].key[j] & 0x7FFF0000)
							{
								if (!pGetInfoDirectInput->IsVirtualKeyPressed((config.actions[ident].key[j] >> 16) & 0xFFFF))
									bCombine = false;
							}

							if (pGetInfoDirectInput->IsVirtualKeyPressed(config.actions[ident].key[j] & 0xFFFF))
							{
								bool bQuit = false;

								switch (ident)
								{
									case CONTROLS_CUST_MAGICMODE:
									{
										if (bCombine)
										{
											if (!uiOneHandedMagicMode)
											{
												uiOneHandedMagicMode = 1;
											}
											else
											{
												if (uiOneHandedMagicMode == 2)
												{
													uiOneHandedMagicMode = 3;
												}
											}

											bQuit = true;
										}
									}
									break;
									case CONTROLS_CUST_STEALTHMODE:
									{
										if (bCombine)
										{
											if (!uiOneHandedStealth)
											{
												uiOneHandedStealth = 1;
											}
											else
											{
												if (uiOneHandedStealth == 2)
												{
													uiOneHandedStealth = 3;
												}
											}

											bQuit = true;
										}
									}
									break;
									default:
									{
										return true & bCombine;
									}
									break;
								}

								if (bQuit)
								{
									break;
								}
							}
							else
							{
								switch (ident)
								{
									case CONTROLS_CUST_MAGICMODE:
									{
										if ((!j) &&
											    (pGetInfoDirectInput->IsVirtualKeyPressed(config.actions[ident].key[j+1] & 0xFFFF)))
										{
											continue;
										}

										if (uiOneHandedMagicMode == 1)
										{
											uiOneHandedMagicMode = 2;
										}
										else
										{
											if (uiOneHandedMagicMode == 3)
											{
												uiOneHandedMagicMode = 0;
											}
										}
									}
									break;
									case CONTROLS_CUST_STEALTHMODE:
									{
										if ((!j) &&
											    (pGetInfoDirectInput->IsVirtualKeyPressed(config.actions[ident].key[j+1] & 0xFFFF)))
										{
											continue;
										}

										if (uiOneHandedStealth == 1)
										{
											uiOneHandedStealth = 2;
										}
										else
										{
											if (uiOneHandedStealth == 3)
											{
												uiOneHandedStealth = 0;
											}
										}
									}
									break;
								}
							}
						}
					}
				}

				switch (ident)
				{
					case CONTROLS_CUST_MAGICMODE:

						if ((uiOneHandedMagicMode == 1) || (uiOneHandedMagicMode == 2))
						{
							return true;
						}

						break;
					case CONTROLS_CUST_STEALTHMODE:

						if ((uiOneHandedStealth == 1) || (uiOneHandedStealth == 2))
						{
							return true;
						}

						break;
				}
			}
			else
			{
				for (long j = 0; j < 2; j++)
				{
					if (config.actions[ident].key[j] != -1)
					{
						if (config.actions[ident].key[j] & Mouse::ButtonBase)
						{
							if (pGetInfoDirectInput->GetMouseButtonRepeat(config.actions[ident].key[j] & ~Mouse::ButtonBase))
								return true;
						}
						else if (config.actions[ident].key[j] & Mouse::WheelBase)
						{
							if (config.actions[ident].key[j] == Mouse::Wheel_Down)
							{
								if (pGetInfoDirectInput->iWheelSens < 0) return true;
							}
							else
							{
								if (pGetInfoDirectInput->iWheelSens > 0) return true;
							}
						}
						else
						{
							bool bCombine = true;

							if (config.actions[ident].key[j] & 0x7FFF0000)
							{
								if (!pGetInfoDirectInput->IsVirtualKeyPressed((config.actions[ident].key[j] >> 16) & 0xFFFF))
									bCombine = false;
							}

							if (pGetInfoDirectInput->IsVirtualKeyPressed(config.actions[ident].key[j] & 0xFFFF))
								return true & bCombine;
						}
					}
				}
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
bool ARX_IMPULSE_NowUnPressed(long ident)
{
	switch (ident)
	{
		case CONTROLS_CUST_MOUSELOOK:
		case CONTROLS_CUST_ACTION:
			break;
		default:
		{
			for (long j = 0; j < 2; j++)
			{
				if (config.actions[ident].key[j] != -1)
				{
					if (config.actions[ident].key[j] & Mouse::ButtonBase)
					{
						if (pGetInfoDirectInput->GetMouseButtonNowUnPressed(config.actions[ident].key[j] & Mouse::ButtonBase))
							return true;
					}
					else
					{
						bool bCombine = true;

						if (config.actions[ident].key[j] & 0x7FFF0000)
						{
							if (!pGetInfoDirectInput->IsVirtualKeyPressed((config.actions[ident].key[j] >> 16) & 0xFFFF))
								bCombine = false;
						}

						if (pGetInfoDirectInput->IsVirtualKeyPressedNowUnPressed(config.actions[ident].key[j] & 0xFFFF))
							return true & bCombine;
					}
				}
			}
		}
	}

	return false;
}


// All standard keys
// "+" should not appear in names as it is used as a separator
static const KeyDescription keysDescriptions[] = {
	{ Keyboard::Key_0, "0" },
	{ Keyboard::Key_1, "1" },
	{ Keyboard::Key_2, "2" },
	{ Keyboard::Key_3, "3" },
	{ Keyboard::Key_4, "4" },
	{ Keyboard::Key_5, "5" },
	{ Keyboard::Key_6, "6" },
	{ Keyboard::Key_7, "7" },
	{ Keyboard::Key_8, "8" },
	{ Keyboard::Key_9, "9" },
	{ Keyboard::Key_A, "A" },
	{ Keyboard::Key_B, "B" },
	{ Keyboard::Key_C, "C" },
	{ Keyboard::Key_D, "D" },
	{ Keyboard::Key_E, "E" },
	{ Keyboard::Key_F, "F" },
	{ Keyboard::Key_G, "G" },
	{ Keyboard::Key_H, "H" },
	{ Keyboard::Key_I, "I" },
	{ Keyboard::Key_J, "J" },
	{ Keyboard::Key_K, "K" },
	{ Keyboard::Key_L, "L" },
	{ Keyboard::Key_M, "M" },
	{ Keyboard::Key_N, "N" },
	{ Keyboard::Key_O, "O" },
	{ Keyboard::Key_P, "P" },
	{ Keyboard::Key_Q, "Q" },
	{ Keyboard::Key_R, "R" },
	{ Keyboard::Key_S, "S" },
	{ Keyboard::Key_T, "T" },
	{ Keyboard::Key_U, "U" },
	{ Keyboard::Key_V, "V" },
	{ Keyboard::Key_W, "W" },
	{ Keyboard::Key_X, "X" },
	{ Keyboard::Key_Y, "Y" },
	{ Keyboard::Key_Z, "Z" },
	{ Keyboard::Key_F1, "F1" },
	{ Keyboard::Key_F2, "F2" },
	{ Keyboard::Key_F3, "F3" },
	{ Keyboard::Key_F4, "F4" },
	{ Keyboard::Key_F5, "F5" },
	{ Keyboard::Key_F6, "F6" },
	{ Keyboard::Key_F7, "F7" },
	{ Keyboard::Key_F8, "F8" },
	{ Keyboard::Key_F9, "F9" },
	{ Keyboard::Key_F10, "F10" },
	{ Keyboard::Key_F11, "F11" },
	{ Keyboard::Key_F12, "F12" },
	{ Keyboard::Key_F13, "F13" },
	{ Keyboard::Key_F14, "F14" },
	{ Keyboard::Key_F15, "F15" },
	{ Keyboard::Key_UpArrow, "Up" },
	{ Keyboard::Key_DownArrow, "Down" },
	{ Keyboard::Key_LeftArrow, "Left" },
	{ Keyboard::Key_RightArrow, "Right" },
	{ Keyboard::Key_Home, "Home" },
	{ Keyboard::Key_End, "End" },
	{ Keyboard::Key_PageUp, "PageUp" },
	{ Keyboard::Key_PageDown, "PageDown" },
	{ Keyboard::Key_Insert, "Insert" },
	{ Keyboard::Key_Delete, "Delete" },
	{ Keyboard::Key_Escape, "Escape" },
	{ Keyboard::Key_NumLock, "NumLock" },
	{ Keyboard::Key_NumPad0, "Numpad0" },
	{ Keyboard::Key_NumPad1, "Numpad1" },
	{ Keyboard::Key_NumPad2, "Numpad2" },
	{ Keyboard::Key_NumPad3, "Numpad3" },
	{ Keyboard::Key_NumPad4, "Numpad4" },
	{ Keyboard::Key_NumPad5, "Numpad5" },
	{ Keyboard::Key_NumPad6, "Numpad6" },
	{ Keyboard::Key_NumPad7, "Numpad7" },
	{ Keyboard::Key_NumPad8, "Numpad8" },
	{ Keyboard::Key_NumPad9, "Numpad9" },
	{ Keyboard::Key_NumPadEnter, "NumpadReturn" },
	{ Keyboard::Key_NumSubtract, "Numpad-" },
	{ Keyboard::Key_NumAdd, "NumpadPlus" },
	{ Keyboard::Key_NumMultiply, "Multiply" },
	{ Keyboard::Key_NumDivide, "Numpad/" },
	{ Keyboard::Key_NumPoint, "Numpad." },
	{ Keyboard::Key_LeftBracket, "[" },
	{ Keyboard::Key_LeftCtrl, "LeftControl" },
	{ Keyboard::Key_LeftAlt, "LeftAlt" },
	{ Keyboard::Key_LeftShift, "LeftShift" },
	{ Keyboard::Key_LeftWin, "LeftStart" },
	{ Keyboard::Key_RightBracket, "]" },
	{ Keyboard::Key_RightCtrl, "RightControl" },
	{ Keyboard::Key_RightAlt, "RightAlt" },
	{ Keyboard::Key_RightShift, "RightShift" },
	{ Keyboard::Key_RightWin, "RightStart" },
	{ Keyboard::Key_PrintScreen, "PrintScreen" },
	{ Keyboard::Key_ScrollLock, "ScrollLock" },
	{ Keyboard::Key_Pause, "Pause" },
	{ Keyboard::Key_Spacebar, "Space" },
	{ Keyboard::Key_Backspace, "Backspace" },
	{ Keyboard::Key_Enter, "Return" },
	{ Keyboard::Key_Tab, "Tab" },
	{ Keyboard::Key_Apps, "AppMenu" },
	{ Keyboard::Key_CapsLock, "Capital" },
	{ Keyboard::Key_Slash, "/" },
	{ Keyboard::Key_Backslash, "Backslash" },
	{ Keyboard::Key_Comma, "," },
	{ Keyboard::Key_Semicolon, ";" },
	{ Keyboard::Key_Period, "." },
	{ Keyboard::Key_Grave, "`" },
	{ Keyboard::Key_Apostrophe, "'" },
	{ Keyboard::Key_Minus, "-" },
	{ Keyboard::Key_Equals, "=" },

	{ Mouse::Wheel_Up, "WheelUp" },
	{ Mouse::Wheel_Down, "WheelDown" }
};

const std::string PREFIX_KEY = "Key_";
const std::string PREFIX_BUTTON = "Button";
const char SEPARATOR = '+';
const std::string Input::KEY_NONE = "---";

std::string Input::getKeyName(InputKeyId key, bool localizedName) {
	ARX_UNUSED(localizedName);

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
	
	arx_assert(Mouse::Button_32 > Mouse::Button_1 && Mouse::Button_32 - Mouse::Button_1 == 31);
	if(key >= (InputKeyId)Mouse::Button_1 && key <= (InputKeyId)Mouse::Button_32) {
		
		ostringstream oss;
		oss << PREFIX_BUTTON << (int)(key - Mouse::Button_1 + 1);
		name = oss.str();
		
	} else {
		arx_assert(key >= 0 && key < ARX_ARRAY_NB_ITEMS(keysDescriptions));
		const KeyDescription & entity = keysDescriptions[key];
		
		arx_assert(entity.id == key);
		name = entity.name;
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

std::map<std::string, InputKeyId> keyNames;

InputKeyId Input::getKeyId(const std::string & name) {
	
	// If a noneset key, return -1
	if(name.empty() || name == KEY_NONE) {
		return -1;
	}
	
	size_t sep = name.find(SEPARATOR);
	if(sep != string::npos) {
		InputKeyId modifier = getKeyId(name.substr(0, sep));
		InputKeyId key = getKeyId(name.substr(sep + 1));
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
	
	arx_assert(Mouse::Button_32 > Mouse::Button_1 && Mouse::Button_32 - Mouse::Button_1 == 31);
	if(!name.compare(0, PREFIX_BUTTON.length(), PREFIX_BUTTON)) {
		istringstream iss(name.substr(PREFIX_BUTTON.length()));
		int key;
		iss >> key;
		if(!iss.bad() && key >= 0 && key <= (int)(Mouse::Button_32 - Mouse::Button_1)) {
			return Mouse::Button_1 + key - 1;
		}
	}
	
	if(keyNames.empty()) {
		// Initialize the key name -> id map.
		for(size_t i = 0; i < ARX_ARRAY_NB_ITEMS(keysDescriptions); i++) {
			keyNames[keysDescriptions[i].name] = keysDescriptions[i].id;
		}
	}
	
	map<string, InputKeyId>::const_iterator it = keyNames.find(name);
	if(it != keyNames.end()) {
		return it->second;
	}
	
	return -1;
}

//-----------------------------------------------------------------------------

CDirectInput *pGetInfoDirectInput=NULL;

//-----------------------------------------------------------------------------

CDirectInput::CDirectInput()
{
	SetSensibility(2);

	iMouseAX=0;
	iMouseAY=0;
	iMouseAZ=0;
	fMouseAXTemp=fMouseAYTemp=0.f;
	
	for(size_t i = 0; i < ARX_MAXBUTTON; i++) {
		iOldMouseButton[i] = 0;
		iMouseTime[i] = 0;
		iMouseTimeSet[i] = 0;
		bMouseButton[i] = bOldMouseButton[i] = false;
		iOldNumClick[i] = iOldNumUnClick[i] = 0;
	}

	bTouch=false;
	for(int iI=0;iI<256;iI++)
		iOneTouch[iI]=0;

	bActive=false;

	iWheelSens=0;
}

//-----------------------------------------------------------------------------

CDirectInput::~CDirectInput()
{
}

//-----------------------------------------------------------------------------

void CDirectInput::ResetAll()
{
	for(size_t i = 0; i < ARX_MAXBUTTON; i++) {
		iOldMouseButton[i] = 0;
		iMouseTime[i] = 0;
		iMouseTimeSet[i] = 0;
		bMouseButton[i] = bOldMouseButton[i] = false;
		iOldNumClick[i] = iOldNumUnClick[i] = 0;
	}

	iKeyId=-1;
	bTouch=false;

	for(int i=0;i<256;i++)
	{
		iOneTouch[i]=0;
	}

	EERIEMouseButton=LastEERIEMouseButton=0;

	iWheelSens=0;
}

//-----------------------------------------------------------------------------

void CDirectInput::GetInput()
{
	int iDTime;

	DX7Input::update();

	iKeyId= DX7Input::getKeyboardKeyPressed();
	bTouch=(iKeyId>=0)?true:false;

	for(int i=0;i<256;i++)
	{
		if(IsVirtualKeyPressed(i))
		{
			switch(i)
			{
			case Keyboard::Key_LeftShift:
			case Keyboard::Key_RightShift:
			case Keyboard::Key_LeftCtrl:
			case Keyboard::Key_RightCtrl:
			case Keyboard::Key_LeftAlt:
			case Keyboard::Key_RightAlt:

				if(i!=iKeyId)
					iKeyId|=(i<<16);

				break;
			}

			if(iOneTouch[i]<2)
			{
				iOneTouch[i]++;
			}
		}
		else
		{
			if(iOneTouch[i]>0)
			{
				iOneTouch[i]--;
			}
		}
	}

	if(bTouch)    //keys priority
	{
		switch(iKeyId)
		{
		case Keyboard::Key_LeftShift:
		case Keyboard::Key_RightShift:
		case Keyboard::Key_LeftCtrl:
		case Keyboard::Key_RightCtrl:
		case Keyboard::Key_LeftAlt:
		case Keyboard::Key_RightAlt:
			{
				bool bFound=false;

				for(int i=0;i<256;i++)
				{
					if(bFound)
					{
						break;
					}

					switch(i&0xFFFF)
					{
					case Keyboard::Key_LeftShift:
					case Keyboard::Key_RightShift:
					case Keyboard::Key_LeftCtrl:
					case Keyboard::Key_RightCtrl:
					case Keyboard::Key_LeftAlt:
					case Keyboard::Key_RightAlt:
						continue;
					default:
						{
							if(iOneTouch[i])
							{
								bFound=true;
								iKeyId&=~0xFFFF;
								iKeyId|=i;
							}
						}
						break;
					}
				}
			}
		}
	}


	ARX_CHECK_INT(ARX_TIME_Get( false ));
	const int iArxTime = ARX_CLEAN_WARN_CAST_INT(ARX_TIME_Get( false )) ;


	for(size_t i = 0; i < ARX_MAXBUTTON; i++)
	{
		int iNumClick;
		int iNumUnClick;
		DX7Input::getMouseButtonClickCount(i, iNumClick, iNumUnClick);

		iOldNumClick[i]+=iNumClick+iNumUnClick;

		if(    (!bMouseButton[i])&&(iOldNumClick[i]==iNumUnClick) )
		{
			iOldNumClick[i]=0;
		}

		bOldMouseButton[i]=bMouseButton[i];

		if(bMouseButton[i])
		{
			if(iOldNumClick[i])
			{
				bMouseButton[i]=false;
			}
		}
		else
		{
			if(iOldNumClick[i])
			{
				bMouseButton[i]=true;
			}
		}

		if(iOldNumClick[i]) iOldNumClick[i]--;

		DX7Input::isMouseButtonPressed(i,iDTime);

		if(iDTime)
		{
			iMouseTime[i]=iDTime;
			iMouseTimeSet[i]=2;
		}
		else
		{
			if(    (iMouseTimeSet[i]>0)&&
				((ARX_TIME_Get( false )-iMouseTime[i])>300)
				)
			{
				iMouseTime[i]=0;
				iMouseTimeSet[i]=0;
			}

			if(GetMouseButtonNowPressed(i))
			{
				switch(iMouseTimeSet[i])
				{
				case 0:
					iMouseTime[i] = iArxTime;
					iMouseTimeSet[i]++;
					break;
				case 1:
					iMouseTime[i] = iArxTime - iMouseTime[i];
					iMouseTimeSet[i]++;
					break;
				}
			}
		}
		}

	iWheelSens=pGetInfoDirectInput->GetWheelSens();

	if(danaeApp.m_pFramework->m_bIsFullscreen && bGLOBAL_DINPUT_MENU) {
		float fDX = 0.f;
		float fDY = 0.f;
		iMouseRX = iMouseRY = iMouseRZ = 0;

		if(DX7Input::getMouseCoordinates(iMouseRX, iMouseRY, iMouseRZ)) {
			float fSensMax = 1.f / 6.f;
			float fSensMin = 2.f;
			float fSens = ( ( fSensMax - fSensMin ) * ( (float)iSensibility ) / 10.f ) + fSensMin;
			fSens = pow( .7f, fSens ) * 2.f;

			fDX=( (float)iMouseRX ) * fSens * ( ( (float)DANAESIZX ) / 640.f );
			fDY=( (float)iMouseRY ) * fSens * ( ( (float)DANAESIZY ) / 480.f );
			fMouseAXTemp += fDX;
			fMouseAYTemp += fDY;

			ARX_CHECK_INT(fMouseAXTemp);
			ARX_CHECK_INT(fMouseAYTemp);
			iMouseAX  = ARX_CLEAN_WARN_CAST_INT(fMouseAXTemp);
			iMouseAY  = ARX_CLEAN_WARN_CAST_INT(fMouseAYTemp);

			iMouseAZ += iMouseRZ;


			if(iMouseAX<0)
			{
				iMouseAX     = 0;
				fMouseAXTemp = 0.f; 
			}


			ARX_CHECK_NOT_NEG( iMouseAX );

			if( ARX_CAST_ULONG( iMouseAX ) >= danaeApp.m_pFramework->m_dwRenderWidth )
			{

				iMouseAX = danaeApp.m_pFramework->m_dwRenderWidth - 1;
				fMouseAXTemp = ARX_CLEAN_WARN_CAST_FLOAT( iMouseAX );
			}

			if(iMouseAY<0)
			{
				fMouseAYTemp=    0.f;
				iMouseAY    =    0;
			}


			ARX_CHECK_NOT_NEG( iMouseAY );

			if( ARX_CAST_ULONG( iMouseAY ) >= danaeApp.m_pFramework->m_dwRenderHeight )
			{

				iMouseAY        = danaeApp.m_pFramework->m_dwRenderHeight - 1;
				fMouseAYTemp    = ARX_CLEAN_WARN_CAST_FLOAT( iMouseAY ); 
			}



			bMouseMove=true;
		}
		else
		{
			bMouseMove=false;
		}

		if(bGLOBAL_DINPUT_GAME)
		{
			_EERIEMouseXdep=(int)fDX;
			_EERIEMouseYdep=(int)fDY;
			EERIEMouseX=iMouseAX;
			EERIEMouseY=iMouseAY;
		}
	}
	else
	{
		bMouseMove = ((iMouseAX != DANAEMouse.x) || (iMouseAY != DANAEMouse.y));
		iMouseAX=DANAEMouse.x;
		iMouseAY=DANAEMouse.y;
		iMouseAZ=0;
	}
}

//-----------------------------------------------------------------------------

void CDirectInput::SetSensibility(int _iSensibility)
{
	iSensibility=_iSensibility;
}

//-----------------------------------------------------------------------------

int CDirectInput::GetSensibility() const
{
	return iSensibility;
}

//-----------------------------------------------------------------------------

int CDirectInput::GetWheelSens() const {
	
	int iX, iY, iZ = 0;
	DX7Input::getMouseCoordinates(iX, iY, iZ);
	
	return iZ;
}

//-----------------------------------------------------------------------------

bool CDirectInput::IsVirtualKeyPressed(int _iVirtualKey) const
{
	return DX7Input::isKeyboardKeyPressed(_iVirtualKey)?true:false;
}

//-----------------------------------------------------------------------------

bool CDirectInput::IsVirtualKeyPressedNowPressed(int _iVirtualKey) const
{
	return ((DX7Input::isKeyboardKeyPressed(_iVirtualKey)) &&
		    (iOneTouch[_iVirtualKey]==1));
}

//-----------------------------------------------------------------------------

bool CDirectInput::IsVirtualKeyPressedNowUnPressed(int _iVirtualKey) const
{
	return ((!DX7Input::isKeyboardKeyPressed(_iVirtualKey))&&
			(iOneTouch[_iVirtualKey]==1) );
}

//-----------------------------------------------------------------------------

bool CDirectInput::GetMouseButton(int _iNumButton) const
{

	return(    (bMouseButton[_iNumButton])&&(!bOldMouseButton[_iNumButton]));
}

//-----------------------------------------------------------------------------

bool CDirectInput::GetMouseButtonRepeat(int _iNumButton) const
{
	return( bMouseButton[_iNumButton] );
}

//-----------------------------------------------------------------------------

bool CDirectInput::GetMouseButtonNowPressed(int _iNumButton) const
{

	return(    (bMouseButton[_iNumButton])&&(!bOldMouseButton[_iNumButton]));
}

//-----------------------------------------------------------------------------

bool CDirectInput::GetMouseButtonNowUnPressed(int _iNumButton) const
{

	return( (!bMouseButton[_iNumButton])&&(bOldMouseButton[_iNumButton]) );
}

//-----------------------------------------------------------------------------

bool CDirectInput::GetMouseButtonDoubleClick(int _iNumButton,int _iTime) const
{
	return ((iMouseTimeSet[_iNumButton]==2)&&(iMouseTime[_iNumButton]<_iTime)) ;
}

//-----------------------------------------------------------------------------

int CDirectInput::GetMouseButtonClicked() const
{
	//MouseButton
	for(size_t i = 0; i < CDirectInput::ARX_MAXBUTTON; i++) {
		if(GetMouseButtonNowPressed(i)) {
			return Mouse::Button_1+i-DXI_BUTTON0;
		}
	}

	//Wheel UP/DOWN
	if(iWheelSens<0)
	{
		return Mouse::Wheel_Down;
	}
	else
	{
		if(iWheelSens>0)
		{
			return Mouse::Wheel_Up;
		}
	}

	return 0;
}