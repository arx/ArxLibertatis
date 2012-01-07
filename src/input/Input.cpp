/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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
/* Based on:
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
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include "input/Input.h"

#include <string>
#include <map>

#include "core/Application.h"
#include "core/Config.h"
#include "core/GameTime.h"
#include "graphics/Math.h"
#include "input/InputBackend.h"
#ifdef HAVE_DINPUT8
#include "input/DInput8Backend.h"
#endif
#ifdef HAVE_SDL
#include "input/SDLInputBackend.h"
#endif
#include "io/log/Logger.h"
#include "window/RenderWindow.h"

Input * GInput = NULL;

// TODO-input: Clean me!
extern long EERIEMouseButton;
extern long LastEERIEMouseButton;

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
};

const std::string PREFIX_KEY = "Key_";
const std::string PREFIX_BUTTON = "Button";
const char SEPARATOR = '+';
const std::string Input::KEY_NONE = "---";

//-----------------------------------------------------------------------------

bool ARX_INPUT_Init() {
	GInput = new Input();
	
	bool ret = GInput->init();
	if(!ret)
	{
		delete GInput;
		GInput = NULL;
	}

	return ret;
}

//-----------------------------------------------------------------------------

void ARX_INPUT_Release() {
	delete GInput;
	GInput = NULL;
}

//-----------------------------------------------------------------------------

Input::Input() : backend(NULL) {
	
	setMouseSensitivity(2);
	
	reset();
}

//-----------------------------------------------------------------------------

bool Input::init() {
	
	arx_assert(backend == NULL);
	
	bool autoBackend = (config.input.backend == "auto");
	
	for(int i = 0; i < 2 && !backend; i++) {
		bool first = (i == 0);
		
		bool matched = false;
		
		#ifdef HAVE_SDL
		if(!backend && first == (autoBackend || config.input.backend == "SDL")) {
			matched = true;
			backend = new SDLInputBackend;
			if(!backend->init()) {
				delete backend, backend = NULL;
			}
		}
		#endif
		
		#ifdef HAVE_DINPUT8
		if(!backend && first == (autoBackend || config.input.backend == "DirectInput8")) {
			matched = true;
			backend = new DInput8Backend;
			if(!backend->init()) {
				delete backend, backend = NULL;
			}
		}
		#endif
		
		if(first && !matched) {
			LogError << "unknown backend: " << config.input.backend;
		}
	}
	
	return (backend != NULL);
}

//-----------------------------------------------------------------------------

Input::~Input()
{
	delete backend;
}

//-----------------------------------------------------------------------------

void Input::reset()
{
	iMouseR = Vec2s::ZERO;

	Vec2s wndSize((short)mainApp->GetWindow()->GetSize().x, (short)mainApp->GetWindow()->GetSize().y); 
	Vec2s absPos = wndSize / 2;
	setMousePosAbs(absPos);
	
	for(size_t i = 0; i < Mouse::ButtonCount; i++) {
		iMouseTime[i] = 0;
		iMouseTimeSet[i] = 0;
		bMouseButton[i] = bOldMouseButton[i] = false;
		iOldNumClick[i] = 0;
	}

	iKeyId=-1;

	for(int i = 0; i < Keyboard::KeyCount; i++)
	{
		keysStates[i]=0;
	}

	EERIEMouseButton=LastEERIEMouseButton=0;

	iWheelDir = 0;
}

void Input::acquireDevices()
{
	backend->acquireDevices();
}

void Input::unacquireDevices()
{
	backend->unacquireDevices();
}

//-----------------------------------------------------------------------------

const Vec2s& Input::getMousePosAbs() const {
	return iMouseA;
}

//-----------------------------------------------------------------------------

const Vec2s& Input::getMousePosRel() const {
	return iMouseR;
}

//-----------------------------------------------------------------------------
void Input::setMousePosAbs(const Vec2s& mousePos)
{
	if(backend)
		backend->setMouseCoordinates(mousePos.x, mousePos.y);

	iMouseA = mousePos;
	iMouseARaw = mousePos;
}

//-----------------------------------------------------------------------------

void Input::update()
{
	int iDTime;

	backend->update();

	bool keyJustPressed = false;
	iKeyId = -1;
	int modifier = 0;

	for(int i = 0; i < Keyboard::KeyCount; i++)
	{
		if(isKeyPressed(i))
		{
			switch(i)
			{
			case Keyboard::Key_LeftShift:
			case Keyboard::Key_RightShift:
			case Keyboard::Key_LeftCtrl:
			case Keyboard::Key_RightCtrl:
			case Keyboard::Key_LeftAlt:
			case Keyboard::Key_RightAlt:
				modifier = i;
				break;
			}

			if(keysStates[i]<2)
			{
				keysStates[i]++;
			}

			if(!keyJustPressed)
			{
				if(keysStates[i] == 1)
				{
					iKeyId = i;
					keyJustPressed = true;
				}
				else
				{
					iKeyId = i;
				}
			}
		}
		else
		{
			if(keysStates[i]>0)
			{
				keysStates[i]--;
			}
		}
	}

	if(modifier != 0 && iKeyId != modifier)
	{
		iKeyId |= (modifier << 16);
	}

	if(iKeyId >= 0)    //keys priority
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

				for(int i = 0; i < Keyboard::KeyCount; i++)
				{
					if(bFound)
					{
						break;
					}

					switch(i & 0xFFFF)
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
							if(keysStates[i])
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

	const int iArxTime = checked_range_cast<int>(arxtime.get_updated(false));

	for(int buttonId = Mouse::ButtonBase; buttonId < Mouse::ButtonMax; buttonId++)
	{
		int i = buttonId - Mouse::ButtonBase;

		int iNumClick;
		int iNumUnClick;
		backend->getMouseButtonClickCount(buttonId, iNumClick, iNumUnClick);

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

		if(iOldNumClick[i]) 
			iOldNumClick[i]--;

		backend->isMouseButtonPressed(buttonId,iDTime);

		if(iDTime)
		{
			iMouseTime[i]=iDTime;
			iMouseTimeSet[i]=2;
		}
		else
		{
			if( (iMouseTimeSet[i]>0)&&
					((arxtime.get_updated( false )-iMouseTime[i])>300))
			{
				iMouseTime[i]=0;
				iMouseTimeSet[i]=0;
			}

			if(getMouseButtonNowPressed(buttonId))
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

	Vec2s iLastMouseARaw = iMouseARaw;
	
	// Get the new coordinates
	int absX, absY;
	backend->getMouseCoordinates(absX, absY, iWheelDir);

	Vec2i wndSize = mainApp->GetWindow()->GetSize();

	// Do not update mouse position when it is outside of the window
	if(absX >= 0 && absX < wndSize.x && absY >= 0 && absY < wndSize.y)
	{
		iMouseARaw = Vec2s((short)absX, (short)absY);

		// In fullscreen, use the sensitivity config value to adjust mouse mouvements
		if(mainApp->GetWindow()->IsFullScreen()) {
			float fSensMax = 1.f / 6.f;
			float fSensMin = 2.f;
			float fSens = ( ( fSensMax - fSensMin ) * ( (float)iSensibility ) / 10.f ) + fSensMin;
			fSens = pow( .7f, fSens ) * 2.f;

			Vec2f fD;
			fD.x=( iMouseARaw.x - iLastMouseARaw.x ) * fSens;
			fD.y=( iMouseARaw.y - iLastMouseARaw.y ) * fSens;

			iMouseR.x = (int)fD.x;
			iMouseR.y = (int)fD.y;
			iMouseA += iMouseR;
		} else {
			iMouseR = iMouseARaw - iMouseA;
			iMouseA = iMouseARaw;		
		}

		// Clamp to window rect
		iMouseA.x = clamp(iMouseA.x, 0, (short)wndSize.x - 1);
		iMouseA.y = clamp(iMouseA.y, 0, (short)wndSize.y - 1);
	}
}

//-----------------------------------------------------------------------------
std::map<std::string, InputKeyId> keyNames;

std::string Input::getKeyName(InputKeyId key, bool localizedName) {
	
	if(key == -1) {
		return std::string();
	}
	
	std::string name;
	
	std::string modifier;
	if(key & INPUT_COMBINATION_MASK) {
		// key combination
		modifier = getKeyName((key >> 16) & 0x0fff, localizedName);
		key &= INPUT_MASK;
	}
	
	if(key >= (InputKeyId)Mouse::ButtonBase && key < (InputKeyId)Mouse::ButtonMax) {
		
		std::ostringstream oss;
		oss << PREFIX_BUTTON << (int)(key - Mouse::ButtonBase + 1);
		name = oss.str();
	
	} else if(key == (InputKeyId)Mouse::Wheel_Up) {
		name = "WheelUp";

	} else if(key == (InputKeyId)Mouse::Wheel_Down) {
		name = "WheelDown";

	} else {
		arx_assert(key >= 0 && key < (int)ARRAY_SIZE(keysDescriptions));
		const KeyDescription & entity = keysDescriptions[key];
		
		arx_assert(entity.id == key);
		name = entity.name;
	}
	
	if(name.empty()) {
		std::ostringstream oss;
		oss << PREFIX_KEY << (int)key;
		name = oss.str();
	}
	
	if(!modifier.empty()) {
		return modifier + SEPARATOR + name;
	} else {
		return name;
	}
}

//-----------------------------------------------------------------------------

InputKeyId Input::getKeyId(const std::string & name) {
	
	// If a noneset key, return -1
	if(name.empty() || name == KEY_NONE) {
		return -1;
	}
	
	size_t sep = name.find(SEPARATOR);
	if(sep != std::string::npos) {
		InputKeyId modifier = getKeyId(name.substr(0, sep));
		InputKeyId key = getKeyId(name.substr(sep + 1));
		return (modifier << 16 | key);
	}
	
	if(!name.compare(0, PREFIX_KEY.length(), PREFIX_KEY)) {
		std::istringstream iss(name.substr(PREFIX_KEY.length()));
		int key;
		iss >> key;
		if(!iss.bad()) {
			return key;
		}
	}
	
	if(!name.compare(0, PREFIX_BUTTON.length(), PREFIX_BUTTON)) {
		std::istringstream iss(name.substr(PREFIX_BUTTON.length()));
		int key;
		iss >> key;
		if(!iss.bad() && key >= 0 && key < Mouse::ButtonCount) {
			return Mouse::ButtonBase + key - 1;
		}
	}
	
	if(keyNames.empty()) {
		// Initialize the key name -> id map.
		for(size_t i = 0; i < ARRAY_SIZE(keysDescriptions); i++) {
			keyNames[keysDescriptions[i].name] = keysDescriptions[i].id;
		}
		keyNames["WheelUp"] = (InputKeyId)Mouse::Wheel_Up;
		keyNames["WheelDown"] = (InputKeyId)Mouse::Wheel_Down;
	}
	
	std::map<std::string, InputKeyId>::const_iterator it = keyNames.find(name);
	if(it != keyNames.end()) {
		return it->second;
	}
	
	return -1;
}

//-----------------------------------------------------------------------------

void Input::setMouseSensitivity(int _iSensibility)
{
	iSensibility = _iSensibility;
}

//-----------------------------------------------------------------------------

int Input::getMouseSensitivity() const {
	return iSensibility;
}

//-----------------------------------------------------------------------------

int Input::getMouseWheelDir() const {
	return iWheelDir;
}

//-----------------------------------------------------------------------------
int Input::getKeyPressed() const {
	return iKeyId;
}

//-----------------------------------------------------------------------------
bool Input::isAnyKeyPressed() const {
	return iKeyId >= 0;
}

bool Input::isKeyPressed(int keyId) const {
	arx_assert(keyId >= Keyboard::KeyBase && keyId < Keyboard::KeyMax);

	return backend->isKeyboardKeyPressed(keyId);
}

//-----------------------------------------------------------------------------

bool Input::isKeyPressedNowPressed(int keyId) const {
	arx_assert(keyId >= Keyboard::KeyBase && keyId < Keyboard::KeyMax);

	return backend->isKeyboardKeyPressed(keyId) && (keysStates[keyId] == 1);
}

//-----------------------------------------------------------------------------

bool Input::isKeyPressedNowUnPressed(int keyId) const {
	arx_assert(keyId >= Keyboard::KeyBase && keyId < Keyboard::KeyMax);

	return !backend->isKeyboardKeyPressed(keyId) && (keysStates[keyId] == 1);
}

//-----------------------------------------------------------------------------

bool Input::getKeyAsText(int keyId, char& result) const {
	return backend->getKeyAsText(keyId, result);
}

//-----------------------------------------------------------------------------

bool Input::getMouseButton(int buttonId) const {
	arx_assert(buttonId >= Mouse::ButtonBase && buttonId < Mouse::ButtonMax);

	int buttonIdx = buttonId - Mouse::ButtonBase;
	return bMouseButton[buttonIdx] && !bOldMouseButton[buttonIdx];
}

//-----------------------------------------------------------------------------

bool Input::getMouseButtonRepeat(int buttonId) const {
	arx_assert(buttonId >= Mouse::ButtonBase && buttonId < Mouse::ButtonMax);

	int buttonIdx = buttonId - Mouse::ButtonBase;
	return bMouseButton[buttonIdx];
}

//-----------------------------------------------------------------------------

bool Input::getMouseButtonNowPressed(int buttonId) const {
	arx_assert(buttonId >= Mouse::ButtonBase && buttonId < Mouse::ButtonMax);

	int buttonIdx = buttonId - Mouse::ButtonBase;
	return bMouseButton[buttonIdx] && !bOldMouseButton[buttonIdx];
}

//-----------------------------------------------------------------------------

bool Input::getMouseButtonNowUnPressed(int buttonId) const {
	arx_assert(buttonId >= Mouse::ButtonBase && buttonId < Mouse::ButtonMax);

	int buttonIdx = buttonId - Mouse::ButtonBase;
	return !bMouseButton[buttonIdx] && bOldMouseButton[buttonIdx];
}

//-----------------------------------------------------------------------------

bool Input::getMouseButtonDoubleClick(int buttonId, int timeMs) const {
	arx_assert(buttonId >= Mouse::ButtonBase && buttonId < Mouse::ButtonMax);

	int buttonIdx = buttonId - Mouse::ButtonBase;
	return (iMouseTimeSet[buttonIdx] == 2) && (iMouseTime[buttonIdx] < timeMs);
}

//-----------------------------------------------------------------------------
bool Input::hasMouseMoved() const {
	return iMouseR.x != 0 || iMouseR.y != 0;
}

//-----------------------------------------------------------------------------
int Input::getMouseButtonClicked() const {

	//MouseButton
	for(int i = Mouse::ButtonBase; i < Mouse::ButtonMax; i++) {
		if(getMouseButtonNowPressed(i)) {
			return i;
		}
	}

	//Wheel UP/DOWN
	if(iWheelDir < 0)
	{
		return Mouse::Wheel_Down;
	}
	else
	{
		if(iWheelDir > 0)
		{
			return Mouse::Wheel_Up;
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------

bool Input::actionNowPressed(int actionId) const {
	
	for(size_t j = 0; j < ARRAY_SIZE(config.actions[actionId].key); j++) {
		
		InputKeyId key = config.actions[actionId].key[j];
		if(key == -1) {
			continue;
		}
		
		if(key & Mouse::ButtonBase) {
			if(getMouseButtonNowPressed(key)) {
				return true;
			}
			continue;
		}
		
		if(key & Mouse::WheelBase) {
			if((key == Mouse::Wheel_Down) ? (getMouseWheelDir() < 0) : (getMouseWheelDir() > 0)) {
				return true;
			}
			continue;
		}
		
		bool bCombine = true;
		if(config.actions[actionId].key[j] & INPUT_COMBINATION_MASK) {
			if(!isKeyPressed((config.actions[actionId].key[j] >> 16) & INPUT_KEYBOARD_MASK)) {
				bCombine = false;
			}
		}
		
		if(isKeyPressedNowPressed(config.actions[actionId].key[j] & INPUT_KEYBOARD_MASK)) {
			return true && bCombine;
		}
	}
	
	return false;
}

//-----------------------------------------------------------------------------
static unsigned int uiOneHandedMagicMode = 0;
static unsigned int uiOneHandedStealth = 0;

bool Input::actionPressed(int actionId) const
{
	switch (actionId)
	{
		case CONTROLS_CUST_MOUSELOOK:
		case CONTROLS_CUST_ACTION:
			break;
		default:
		{
			if (config.misc.forceToggle)
			{
				for (int j = 0; j < 2; j++)
				{
					if (config.actions[actionId].key[j] != -1)
					{
						if (config.actions[actionId].key[j] & Mouse::ButtonBase)
						{
							if (getMouseButtonRepeat(config.actions[actionId].key[j]))
								return true;
						}
						else if (config.actions[actionId].key[j] & Mouse::WheelBase)
						{
							if (config.actions[actionId].key[j] == Mouse::Wheel_Down)
							{
								if (getMouseWheelDir() < 0) return true;
							}
							else
							{
								if (getMouseWheelDir() > 0) return true;
							}
						}
						else
						{
							bool bCombine = true;

							if (config.actions[actionId].key[j] & INPUT_COMBINATION_MASK)
							{
								if (!isKeyPressed((config.actions[actionId].key[j] >> 16) & 0xFFFF))
									bCombine = false;
							}

							if (isKeyPressed(config.actions[actionId].key[j] & 0xFFFF))
							{
								bool bQuit = false;

								switch (actionId)
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
								switch (actionId)
								{
									case CONTROLS_CUST_MAGICMODE:
									{
										if ((!j) &&
											    (isKeyPressed(config.actions[actionId].key[1] & 0xFFFF)))
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
											    (isKeyPressed(config.actions[actionId].key[1] & 0xFFFF)))
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

				switch (actionId)
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
				for (int j = 0; j < 2; j++)
				{
					if (config.actions[actionId].key[j] != -1)
					{
						if (config.actions[actionId].key[j] & Mouse::ButtonBase)
						{
							if (getMouseButtonRepeat(config.actions[actionId].key[j]))
								return true;
						}
						else if (config.actions[actionId].key[j] & Mouse::WheelBase)
						{
							if (config.actions[actionId].key[j] == Mouse::Wheel_Down)
							{
								if (getMouseWheelDir() < 0) return true;
							}
							else
							{
								if (getMouseWheelDir() > 0) return true;
							}
						}
						else
						{
							bool bCombine = true;

							if (config.actions[actionId].key[j] & INPUT_COMBINATION_MASK)
							{
								if (!isKeyPressed((config.actions[actionId].key[j] >> 16) & 0xFFFF))
									bCombine = false;
							}

							if (isKeyPressed(config.actions[actionId].key[j] & 0xFFFF))
								return true & bCombine;
						}
					}
				}
			}
		}
	}

	return false;
}

bool Input::actionNowReleased(int actionId) const {
	
	for(size_t j = 0; j < ARRAY_SIZE(config.actions[actionId].key); j++) {
		
		InputKeyId key = config.actions[actionId].key[j];
		if(key == -1) {
			continue;
		}
		
		if(key & Mouse::ButtonBase) {
			if(getMouseButtonNowUnPressed(key)) {
				return true;
			}
			continue;
		}
		
		if(key & Mouse::WheelBase) {
			continue;
		}
		
		bool bCombine = true;
		if(config.actions[actionId].key[j] & INPUT_COMBINATION_MASK) {
			if(!isKeyPressed((config.actions[actionId].key[j] >> 16) & INPUT_KEYBOARD_MASK)) {
				bCombine = false;
			}
		}
		
		if(isKeyPressedNowUnPressed(config.actions[actionId].key[j] & INPUT_KEYBOARD_MASK)) {
			return true && bCombine;
		}
	}
	
	return false;
}
