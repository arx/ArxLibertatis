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
// ARX_Common
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		All preprocessor directives set for all the solution.
//
// Updates: (07-23-2010) (xrichter)		File extension change from .c to .cpp
//										Comments which start with //Old : are the old way to call directx 7 functions in C
//
// Code:	Xavier RICHTER
//
// Copyright (c) 1999-2010 ARKANE Studios SA. All rights reserved
/////////////////////////////////////////////////////////////////////////////////////

#include "input/DX7InputBackend.h"

#include <cstdlib>
#include <vector>

#ifndef DIRECTINPUT_VERSION
	#define DIRECTINPUT_VERSION 0x0700
#endif
#include <dinput.h>

#include "io/Logger.h"

#include "platform/Platform.h"

using std::vector;

#define INPUT_STATE_ADD (512)

enum DXIMode {
	DXI_MODE_EXCLUSIF_ALLMSG = 0,
	DXI_MODE_EXCLUSIF_OURMSG = 1,
	DXI_MODE_NONEXCLUSIF_ALLMSG = 2,
	DXI_MODE_NONEXCLUSIF_OURMSG = 3
};

struct INPUT_INFO {
	bool active;
	GUID guid;
	int type;
	int nbbuttons;
	int nbaxes;
	int nbele; // for mouse
	LPDIRECTINPUTDEVICE7 inputdevice7;
	union {
		char * bufferstate;
		DIDEVICEOBJECTDATA * mousestate;
	};
};

typedef vector<INPUT_INFO> InputList;
static InputList DI_InputInfo;
static IDirectInput7A * DI_DInput7;

static INPUT_INFO * DI_KeyBoardBuffer;
static INPUT_INFO * DI_MouseState;

const int DI_KEY_ARRAY_SIZE = 256;

static int DInputToArxKeyTable[DI_KEY_ARRAY_SIZE];

static void initDInputToArxKeyTable();
static void releaseDevice(INPUT_INFO & info);
static bool chooseInputDevice(HWND hwnd, INPUT_INFO & info, DXIMode mode);
static bool getKeyboardInputDevice(DXIMode mode);
static bool getMouseInputDevice(DXIMode mode, int minbutton, int minaxe);
static bool setMouseRelative();

void initDInputToArxKeyTable()
{
	for(int i = 0; i < DI_KEY_ARRAY_SIZE;++i)
		DInputToArxKeyTable[i] = -1;

    DInputToArxKeyTable[DIK_0]            = Keyboard::Key_0;              
    DInputToArxKeyTable[DIK_1]            = Keyboard::Key_1;
    DInputToArxKeyTable[DIK_2]            = Keyboard::Key_2;
    DInputToArxKeyTable[DIK_3]            = Keyboard::Key_3;
    DInputToArxKeyTable[DIK_4]            = Keyboard::Key_4;
    DInputToArxKeyTable[DIK_5]            = Keyboard::Key_5;
    DInputToArxKeyTable[DIK_6]            = Keyboard::Key_6;
    DInputToArxKeyTable[DIK_7]            = Keyboard::Key_7;
    DInputToArxKeyTable[DIK_8]            = Keyboard::Key_8;
    DInputToArxKeyTable[DIK_9]            = Keyboard::Key_9;
    DInputToArxKeyTable[DIK_A]            = Keyboard::Key_A;
    DInputToArxKeyTable[DIK_ADD]          = Keyboard::Key_NumAdd;
    DInputToArxKeyTable[DIK_APOSTROPHE]   = Keyboard::Key_Apostrophe;
    DInputToArxKeyTable[DIK_APPS]         = Keyboard::Key_Apps;
    DInputToArxKeyTable[DIK_B]            = Keyboard::Key_B;
    DInputToArxKeyTable[DIK_BACK]         = Keyboard::Key_Backspace;
    DInputToArxKeyTable[DIK_BACKSLASH]    = Keyboard::Key_Backslash;
    DInputToArxKeyTable[DIK_C]            = Keyboard::Key_C;
    DInputToArxKeyTable[DIK_CAPITAL]      = Keyboard::Key_CapsLock;
    DInputToArxKeyTable[DIK_COMMA]        = Keyboard::Key_Comma;
    DInputToArxKeyTable[DIK_D]            = Keyboard::Key_D;
    DInputToArxKeyTable[DIK_DECIMAL]      = Keyboard::Key_NumPoint;
    DInputToArxKeyTable[DIK_DELETE]       = Keyboard::Key_Delete;
    DInputToArxKeyTable[DIK_DIVIDE]       = Keyboard::Key_NumDivide;
    DInputToArxKeyTable[DIK_DOWN]         = Keyboard::Key_DownArrow;
    DInputToArxKeyTable[DIK_E]            = Keyboard::Key_E;
    DInputToArxKeyTable[DIK_END]          = Keyboard::Key_End;
    DInputToArxKeyTable[DIK_EQUALS]       = Keyboard::Key_Equals;
    DInputToArxKeyTable[DIK_ESCAPE]       = Keyboard::Key_Escape;
    DInputToArxKeyTable[DIK_F]            = Keyboard::Key_F;
    DInputToArxKeyTable[DIK_F1]           = Keyboard::Key_F1;
    DInputToArxKeyTable[DIK_F2]           = Keyboard::Key_F2;
    DInputToArxKeyTable[DIK_F3]           = Keyboard::Key_F3;
    DInputToArxKeyTable[DIK_F4]           = Keyboard::Key_F4;
    DInputToArxKeyTable[DIK_F5]           = Keyboard::Key_F5;
    DInputToArxKeyTable[DIK_F6]           = Keyboard::Key_F6;
    DInputToArxKeyTable[DIK_F7]           = Keyboard::Key_F7;
    DInputToArxKeyTable[DIK_F8]           = Keyboard::Key_F8;
    DInputToArxKeyTable[DIK_F9]           = Keyboard::Key_F9;
    DInputToArxKeyTable[DIK_F10]          = Keyboard::Key_F10;
    DInputToArxKeyTable[DIK_F11]          = Keyboard::Key_F11;
    DInputToArxKeyTable[DIK_F12]          = Keyboard::Key_F12;
    DInputToArxKeyTable[DIK_F13]          = Keyboard::Key_F13;
    DInputToArxKeyTable[DIK_F14]          = Keyboard::Key_F14;
    DInputToArxKeyTable[DIK_F15]          = Keyboard::Key_F15;
    DInputToArxKeyTable[DIK_G]            = Keyboard::Key_G;
    DInputToArxKeyTable[DIK_GRAVE]        = Keyboard::Key_Grave;
    DInputToArxKeyTable[DIK_H]            = Keyboard::Key_H;
    DInputToArxKeyTable[DIK_HOME]         = Keyboard::Key_Home;
    DInputToArxKeyTable[DIK_I]            = Keyboard::Key_I;
    DInputToArxKeyTable[DIK_INSERT]       = Keyboard::Key_Insert;
    DInputToArxKeyTable[DIK_J]            = Keyboard::Key_J;
    DInputToArxKeyTable[DIK_K]            = Keyboard::Key_K;
    DInputToArxKeyTable[DIK_L]            = Keyboard::Key_L;
    DInputToArxKeyTable[DIK_LBRACKET]     = Keyboard::Key_LeftBracket;
    DInputToArxKeyTable[DIK_LCONTROL]     = Keyboard::Key_LeftCtrl;
    DInputToArxKeyTable[DIK_LEFT]         = Keyboard::Key_LeftArrow;
    DInputToArxKeyTable[DIK_LMENU]        = Keyboard::Key_LeftAlt;
    DInputToArxKeyTable[DIK_LSHIFT]       = Keyboard::Key_LeftShift;
    DInputToArxKeyTable[DIK_LWIN]         = Keyboard::Key_LeftWin;
    DInputToArxKeyTable[DIK_M]            = Keyboard::Key_M;
    DInputToArxKeyTable[DIK_MINUS]        = Keyboard::Key_Minus;
    DInputToArxKeyTable[DIK_MULTIPLY]     = Keyboard::Key_NumMultiply;
    DInputToArxKeyTable[DIK_N]            = Keyboard::Key_N;
    DInputToArxKeyTable[DIK_NEXT]         = Keyboard::Key_PageDown;
    DInputToArxKeyTable[DIK_NUMLOCK]      = Keyboard::Key_NumLock;
    DInputToArxKeyTable[DIK_NUMPAD0]      = Keyboard::Key_NumPad0;
    DInputToArxKeyTable[DIK_NUMPAD1]      = Keyboard::Key_NumPad1;
    DInputToArxKeyTable[DIK_NUMPAD2]      = Keyboard::Key_NumPad2;
    DInputToArxKeyTable[DIK_NUMPAD3]      = Keyboard::Key_NumPad3;
    DInputToArxKeyTable[DIK_NUMPAD4]      = Keyboard::Key_NumPad4;
    DInputToArxKeyTable[DIK_NUMPAD5]      = Keyboard::Key_NumPad5;
    DInputToArxKeyTable[DIK_NUMPAD6]      = Keyboard::Key_NumPad6;
    DInputToArxKeyTable[DIK_NUMPAD7]      = Keyboard::Key_NumPad7;
    DInputToArxKeyTable[DIK_NUMPAD8]      = Keyboard::Key_NumPad8;
    DInputToArxKeyTable[DIK_NUMPAD9]      = Keyboard::Key_NumPad9;
    DInputToArxKeyTable[DIK_NUMPADENTER]  = Keyboard::Key_NumPadEnter;
    DInputToArxKeyTable[DIK_O]            = Keyboard::Key_O;
    DInputToArxKeyTable[DIK_P]            = Keyboard::Key_P;
    DInputToArxKeyTable[DIK_PAUSE]        = Keyboard::Key_Pause;
    DInputToArxKeyTable[DIK_PERIOD]       = Keyboard::Key_Period;
    DInputToArxKeyTable[DIK_PRIOR]        = Keyboard::Key_PageUp;
    DInputToArxKeyTable[DIK_Q]            = Keyboard::Key_Q;
    DInputToArxKeyTable[DIK_R]            = Keyboard::Key_R;
    DInputToArxKeyTable[DIK_RBRACKET]     = Keyboard::Key_RightBracket;
    DInputToArxKeyTable[DIK_RCONTROL]     = Keyboard::Key_RightCtrl;
    DInputToArxKeyTable[DIK_RETURN]       = Keyboard::Key_Enter;
    DInputToArxKeyTable[DIK_RIGHT]        = Keyboard::Key_RightArrow;
    DInputToArxKeyTable[DIK_RMENU]        = Keyboard::Key_RightAlt;
    DInputToArxKeyTable[DIK_RSHIFT]       = Keyboard::Key_RightShift;
    DInputToArxKeyTable[DIK_RWIN]         = Keyboard::Key_RightWin;
    DInputToArxKeyTable[DIK_S]            = Keyboard::Key_S;
    DInputToArxKeyTable[DIK_SCROLL]       = Keyboard::Key_ScrollLock;
    DInputToArxKeyTable[DIK_SEMICOLON]    = Keyboard::Key_Semicolon;
    DInputToArxKeyTable[DIK_SLASH]        = Keyboard::Key_Slash;
    DInputToArxKeyTable[DIK_SPACE]        = Keyboard::Key_Spacebar;
    DInputToArxKeyTable[DIK_SUBTRACT]     = Keyboard::Key_NumSubtract;
    DInputToArxKeyTable[DIK_SYSRQ]        = Keyboard::Key_PrintScreen;
    DInputToArxKeyTable[DIK_T]            = Keyboard::Key_T;
    DInputToArxKeyTable[DIK_TAB]          = Keyboard::Key_Tab;
    DInputToArxKeyTable[DIK_U]            = Keyboard::Key_U;
    DInputToArxKeyTable[DIK_UP]           = Keyboard::Key_UpArrow;
    DInputToArxKeyTable[DIK_V]            = Keyboard::Key_V;
    DInputToArxKeyTable[DIK_W]            = Keyboard::Key_W;
    DInputToArxKeyTable[DIK_X]            = Keyboard::Key_X;
    DInputToArxKeyTable[DIK_Y]            = Keyboard::Key_Y;
    DInputToArxKeyTable[DIK_Z]            = Keyboard::Key_Z;
}

// must be BOOL to be passed to DX
static BOOL CALLBACK DIEnumDevicesCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef) {
	
	(void)pvRef;
	
	DI_InputInfo.resize(DI_InputInfo.size() + 1);
	
	INPUT_INFO & info = DI_InputInfo.back();
	memset(&info,0,sizeof(INPUT_INFO));
	
	//guid
	memcpy(&info.guid, &lpddi->guidInstance, sizeof(GUID));
	
	//type
	info.type = lpddi->dwDevType;
	
	return DIENUM_CONTINUE;
}

DX7InputBackend::DX7InputBackend()
{
}

DX7InputBackend::~DX7InputBackend()
{
	for(InputList::iterator i = DI_InputInfo.begin(); i != DI_InputInfo.end(); ++i) {
		releaseDevice(*i);
	}

	DI_InputInfo.clear();
	
	if(DI_DInput7) {
		DI_DInput7->Release();
	}

	DI_DInput7 = NULL;
}

bool DX7InputBackend::init() {
	
	initDInputToArxKeyTable();

	HINSTANCE h = GetModuleHandle(0);
	if(!h) {
		return false;
	}
	
	DI_DInput7 = NULL;
	if(FAILED(DirectInputCreateEx(h, DIRECTINPUT_VERSION, IID_IDirectInput7, (void**)&DI_DInput7, NULL))) {
		return false;
	}
	
	DI_InputInfo.clear();
	if(FAILED(DI_DInput7->EnumDevices(0, DIEnumDevicesCallback, NULL, DIEDFL_ATTACHEDONLY))) {
		return false;
	}
	
	DI_KeyBoardBuffer = NULL;
	DI_MouseState = NULL;

	if(!getKeyboardInputDevice(DXI_MODE_NONEXCLUSIF_OURMSG)) {
		LogWarning << "could not grab the keyboeard";
		return false;
	}
	
	if(!getMouseInputDevice(DXI_MODE_NONEXCLUSIF_ALLMSG, 2, 2)) {
		LogWarning << "could not grab the mouse";
		return false;
	}
	
	if(!setMouseRelative()) {
		LogWarning << "could not set mouse relative mode";
		return false;
	}
	
	return true;
}

void releaseDevice(INPUT_INFO & info) {
	
	if(!info.active) {
		return;
	}
	
	info.active = false;
	
	if(info.inputdevice7) {
		info.inputdevice7->Unacquire();
		info.inputdevice7->Release();
	}
	info.inputdevice7=NULL;
	
	switch(GET_DIDEVICE_TYPE(info.type)) {
		case DIDEVTYPE_MOUSE: {
			if(info.mousestate) {
				delete[] info.mousestate;
				info.mousestate = NULL;
			}
			break;
		}
		case DIDEVTYPE_KEYBOARD: {
			if(info.bufferstate) {
				delete[] info.bufferstate;
				info.bufferstate = NULL;
			}
			break;
		}
	}
}

void DX7InputBackend::acquireDevices() {
	
	for(InputList::iterator i = DI_InputInfo.begin(); i != DI_InputInfo.end(); ++i) {
		if(i->active) {
			i->inputdevice7->Acquire();
		}
	}
}

void DX7InputBackend::unacquireDevices() {
	
	for(InputList::iterator i = DI_InputInfo.begin(); i != DI_InputInfo.end(); ++i) {
		if(i->active) {
			i->inputdevice7->Unacquire();
		}
	}
}

bool chooseInputDevice(HWND hwnd, INPUT_INFO & info, DXIMode mode) {
	
	int flag;
	switch(mode) {
		case DXI_MODE_EXCLUSIF_ALLMSG:
			flag = DISCL_EXCLUSIVE | DISCL_BACKGROUND;
			break;
		case DXI_MODE_EXCLUSIF_OURMSG:
			flag = DISCL_EXCLUSIVE | DISCL_FOREGROUND;
			break;
		case DXI_MODE_NONEXCLUSIF_ALLMSG:
			flag = DISCL_NONEXCLUSIVE | DISCL_BACKGROUND;
			break;
		case DXI_MODE_NONEXCLUSIF_OURMSG:
			flag = DISCL_NONEXCLUSIVE | DISCL_FOREGROUND;
			break;
		default:
			ARX_CHECK_NO_ENTRY();
			return false;
	}
	
	releaseDevice(info);
	
	if(FAILED(DI_DInput7->CreateDeviceEx(info.guid, IID_IDirectInputDevice7, (void**)&info.inputdevice7, NULL))) {
		return false;
	}
	
	DIDEVCAPS devcaps;
	memset(&devcaps, 0, sizeof(devcaps));
	devcaps.dwSize = sizeof(devcaps);
	if(FAILED(info.inputdevice7->GetCapabilities(&devcaps))) {
		return false;
	}
	
	info.nbbuttons = devcaps.dwButtons;
	info.nbaxes = devcaps.dwAxes;
	
	if(FAILED(info.inputdevice7->SetCooperativeLevel(hwnd, flag))) {
		return false;
	}
	
	const DIDATAFORMAT * dformat;
	switch(GET_DIDEVICE_TYPE(info.type)) {
		
		case DIDEVTYPE_MOUSE: {
			info.mousestate = new DIDEVICEOBJECTDATA[info.nbbuttons + info.nbaxes + INPUT_STATE_ADD];
			memset(info.mousestate, 0, (sizeof(DIDEVICEOBJECTDATA) * (info.nbbuttons + info.nbaxes + INPUT_STATE_ADD)));
			DI_MouseState = &info;
			if(info.nbbuttons > 4) {
				dformat = &c_dfDIMouse2;
			} else {
				dformat = &c_dfDIMouse;
			}
			
			DIPROPDWORD dipdw = {
				{
					sizeof(DIPROPDWORD),  // diph.dwSize
					sizeof(DIPROPHEADER), // diph.dwHeaderSize
					0,                    // diph.dwObj
					DIPH_DEVICE,          // diph.dwHow
				},
				128,                    // dwData
			};
			if(FAILED(info.inputdevice7->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph))) {
				return false;
			}
			
			break;
		}
		
		case DIDEVTYPE_KEYBOARD: {
			info.bufferstate = new char[DI_KEY_ARRAY_SIZE];
			memset(info.bufferstate, 0, DI_KEY_ARRAY_SIZE);
			DI_KeyBoardBuffer = &info;
			dformat = &c_dfDIKeyboard;
			break;
		}
		
		default:
			return false;
	}
	
	if(FAILED(info.inputdevice7->SetDataFormat(dformat))) {
		return false;
	}
	
	(void)info.inputdevice7->Acquire();
	
	info.active = true;
	return true;
}

// TODO
extern HWND mainWindow;

bool getKeyboardInputDevice(DXIMode mode) {
	
	if(DI_KeyBoardBuffer) {
		releaseDevice(*DI_KeyBoardBuffer);
		DI_KeyBoardBuffer = NULL;
	}
	
	for(InputList::iterator i = DI_InputInfo.begin(); i != DI_InputInfo.end(); ++i) {
		
		if(GET_DIDEVICE_TYPE(i->type) != DIDEVTYPE_KEYBOARD || i->active) {
			continue;
		}
		
		if(chooseInputDevice(mainWindow, *i, mode)) {
			return true;
		}
		
	}
	
	return false;
}

bool getMouseInputDevice(DXIMode mode, int minbutton, int minaxe) {
	
	if(DI_MouseState) {
		releaseDevice(*DI_MouseState);
		DI_MouseState = NULL;
	}
	
	for(InputList::iterator i = DI_InputInfo.begin(); i != DI_InputInfo.end(); ++i) {
		
		if(GET_DIDEVICE_TYPE(i->type) != DIDEVTYPE_MOUSE || i->active) {
			continue;
		}
		
		if(chooseInputDevice(mainWindow, *i, mode)) {
			if(i->nbbuttons >= minbutton && i->nbaxes >= minaxe) {
				return true;
			} else {
				releaseDevice(*i);
			}
		}
		
	}
	
	return false;
}

bool DX7InputBackend::update() {
	
	bool success = true;
	for(InputList::iterator i = DI_InputInfo.begin(); i != DI_InputInfo.end(); ++i) {
		
		if(!i->active) {
			continue;
		}
		
		switch(GET_DIDEVICE_TYPE(i->type)) {
			
			case DIDEVTYPE_MOUSE: {
				DWORD dwNbele = (DWORD)(i->nbbuttons + i->nbaxes + INPUT_STATE_ADD);
				if(FAILED(i->inputdevice7->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), i->mousestate, &dwNbele, 0))) {
					dwNbele = (DWORD)(i->nbbuttons + i->nbaxes + INPUT_STATE_ADD);
					if(FAILED(i->inputdevice7->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), i->mousestate, &dwNbele, 0))) {
						i->inputdevice7->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), i->mousestate, &dwNbele, 0);
						success = false;
					}
				}
				
				i->nbele = (int)dwNbele;
				break;
			}
			
			case DIDEVTYPE_KEYBOARD: {
				char keyBuffer[DI_KEY_ARRAY_SIZE];
				if(FAILED(i->inputdevice7->GetDeviceState(DI_KEY_ARRAY_SIZE, keyBuffer))) {
					acquireDevices(); 
					if(FAILED(i->inputdevice7->GetDeviceState(DI_KEY_ARRAY_SIZE, keyBuffer)))  {
						memset(i->bufferstate, 0, DI_KEY_ARRAY_SIZE);
						success = false;
					}
				} else {
					Keyboard::Key currentKey;
					for( int iKey = 0; iKey < DI_KEY_ARRAY_SIZE; iKey++ )
					{
						// Ignore unused keys
						if( DInputToArxKeyTable[iKey] == -1 )
							continue;
						else 
							currentKey = (Keyboard::Key)DInputToArxKeyTable[iKey];

						i->bufferstate[currentKey - Keyboard::KeyBase] = keyBuffer[iKey];
					}
				}
				break;
			}
			
		}
	}
	
	return success;
}

bool DX7InputBackend::isKeyboardKeyPressed(int dikkey) const {
	return (DI_KeyBoardBuffer->bufferstate[dikkey - Keyboard::KeyBase] & 0x80) == 0x80;
}

int DX7InputBackend::getKeyboardKeyPressed() const {
	
	char * buf = DI_KeyBoardBuffer->bufferstate;
	for(int i = 0; i < DI_KEY_ARRAY_SIZE; i++) {
		if(buf[i] & 0x80) {
			return i;
		}
	}
	return -1;
}

bool DX7InputBackend::getMouseCoordinates(int & mx, int & my, int & mz) const {
	
	mx = my = mz = 0;
	
	bool flg = false;
	const DIDEVICEOBJECTDATA * od = DI_MouseState->mousestate;
	for(int nb = DI_MouseState->nbele; nb; nb--, od++) {
		if(od->dwOfs == (DWORD)DIMOFS_X) {
			mx += od->dwData;
			flg = true;
		}
		else if(od->dwOfs == (DWORD)DIMOFS_Y) {
			my += od->dwData;
			flg = true;
		}
		else if(od->dwOfs == (DWORD)DIMOFS_Z) {
			mz += od->dwData;
			flg = true;
		}
	}
	
	return flg;
}

static bool isMouseButton(int buttonId, DWORD dwOfs) {
	switch(buttonId) {
	case Mouse::Button_0:
		return (dwOfs == (DWORD)DIMOFS_BUTTON0);
	case Mouse::Button_1:
		return (dwOfs == (DWORD)DIMOFS_BUTTON1);
	case Mouse::Button_2:
		return (dwOfs == (DWORD)DIMOFS_BUTTON2);
	case Mouse::Button_3:
		return (dwOfs == (DWORD)DIMOFS_BUTTON3);
	case Mouse::Button_4:
		return (dwOfs == (DWORD)DIMOFS_BUTTON4);
	case Mouse::Button_5:
		return (dwOfs == (DWORD)DIMOFS_BUTTON5);
	case Mouse::Button_6:
		return (dwOfs == (DWORD)DIMOFS_BUTTON6);
	case Mouse::Button_7:
		return (dwOfs == (DWORD)DIMOFS_BUTTON7);
	default:
		return false;
	}
}

void DX7InputBackend::getMouseButtonClickCount(int buttonId, int & _iNumClick, int & _iNumUnClick) const {
	arx_assert(buttonId >= Mouse::ButtonBase && buttonId < Mouse::ButtonMax);

	_iNumClick = 0;
	_iNumUnClick = 0;
	
	const DIDEVICEOBJECTDATA * od = DI_MouseState->mousestate;
	for(int nb = DI_MouseState->nbele; nb; nb--, od++) {
		if(isMouseButton(buttonId, od->dwOfs)) {
			if(od->dwData & 0x80) {
				_iNumClick += 1;
			} else {
				_iNumUnClick += 1;
			}
		}
	}
	
}

bool DX7InputBackend::isMouseButtonPressed(int buttonId, int & _iDeltaTime) const {
	arx_assert(buttonId >= Mouse::ButtonBase && buttonId < Mouse::ButtonMax);

	int iTime1 = 0;
	int iTime2 = 0;
	
	const DIDEVICEOBJECTDATA * od = DI_MouseState->mousestate;
	for(int nb = DI_MouseState->nbele; nb; nb--, od++) {
		if(isMouseButton(buttonId, od->dwOfs) && od->dwData & 0x80) {
			if(!iTime1) {
				iTime1 = od->dwTimeStamp;
			} else {
				iTime2 = od->dwTimeStamp;
			}
		}
	}
	
	if(!iTime2) {
		_iDeltaTime = 0;
	} else {
		_iDeltaTime = iTime2 - iTime1;
	}
	
	return (iTime1 != 0);
}

bool setMouseRelative() {
	
	if(FAILED(DI_MouseState->inputdevice7->Unacquire())) {
		return false;
	}
	
	DIPROPDWORD dipdw = {
		{
			sizeof(DIPROPDWORD),  // diph.dwSize
			sizeof(DIPROPHEADER), // diph.dwHeaderSize
			0,                    // diph.dwObj
			DIPH_DEVICE,          // diph.dwHow
		},
		DIPROPAXISMODE_REL,     // dwData
	};
	if(FAILED(DI_MouseState->inputdevice7->SetProperty(DIPROP_AXISMODE, &dipdw.diph))) {
		return false;
	}
	
	return !FAILED(DI_MouseState->inputdevice7->Acquire());
}
