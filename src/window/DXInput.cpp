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

#include "window/DXInput.h"

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

bool DXI_Init() {
	
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
	
	return true;
}

static void DXI_ReleaseDevice(INPUT_INFO & info) {
	
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

void DXI_Release() {
	
	for(InputList::iterator i = DI_InputInfo.begin(); i != DI_InputInfo.end(); ++i) {
		DXI_ReleaseDevice(*i);
	}
	DI_InputInfo.clear();
	
	if(DI_DInput7) {
		DI_DInput7->Release();
	}
	DI_DInput7 = NULL;
}

void DXI_RestoreAllDevices() {
	
	for(InputList::iterator i = DI_InputInfo.begin(); i != DI_InputInfo.end(); ++i) {
		if(i->active) {
			i->inputdevice7->Acquire();
		}
	}
}

void DXI_SleepAllDevices() {
	
	for(InputList::iterator i = DI_InputInfo.begin(); i != DI_InputInfo.end(); ++i) {
		if(i->active) {
			i->inputdevice7->Unacquire();
		}
	}
}

static bool DXI_ChooseInputDevice(HWND hwnd, INPUT_INFO & info, DXIMode mode) {
	
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
	
	DXI_ReleaseDevice(info);
	
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
			info.bufferstate = new char[256];
			memset(info.bufferstate, 0, 256);
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

bool DXI_GetKeyboardInputDevice(DXIMode mode) {
	
	if(DI_KeyBoardBuffer) {
		DXI_ReleaseDevice(*DI_KeyBoardBuffer);
		DI_KeyBoardBuffer = NULL;
	}
	
	for(InputList::iterator i = DI_InputInfo.begin(); i != DI_InputInfo.end(); ++i) {
		
		if(GET_DIDEVICE_TYPE(i->type) != DIDEVTYPE_KEYBOARD || i->active) {
			continue;
		}
		
		if(DXI_ChooseInputDevice(mainWindow, *i, mode)) {
			return true;
		}
		
	}
	
	return false;
}

bool DXI_GetMouseInputDevice(DXIMode mode, int minbutton, int minaxe) {
	
	if(DI_MouseState) {
		DXI_ReleaseDevice(*DI_MouseState);
		DI_MouseState = NULL;
	}
	
	for(InputList::iterator i = DI_InputInfo.begin(); i != DI_InputInfo.end(); ++i) {
		
		if(GET_DIDEVICE_TYPE(i->type) != DIDEVTYPE_MOUSE || i->active) {
			continue;
		}
		
		if(DXI_ChooseInputDevice(mainWindow, *i, mode)) {
			if(i->nbbuttons >= minbutton && i->nbaxes >= minaxe) {
				return true;
			} else {
				DXI_ReleaseDevice(*i);
			}
		}
		
	}
	
	return false;
}

bool DXI_ExecuteAllDevices() {
	
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
				if(FAILED(i->inputdevice7->GetDeviceState(256, i->bufferstate))) {
					DXI_RestoreAllDevices(); 
					if(FAILED(i->inputdevice7->GetDeviceState(256, i->bufferstate)))  {
						memset(i->bufferstate, 0, 256);
						success = false;
					}
				}
				break;
			}
			
		}
	}
	
	return success;
}

bool DXI_KeyPressed(int dikkey) {
	return (DI_KeyBoardBuffer->bufferstate[dikkey] & 0x80) == 0x80;
}

int DXI_GetKeyIDPressed() {
	
	char * buf = DI_KeyBoardBuffer->bufferstate;
	for(int i = 0; i < 256; i++) {
		if(buf[i] & 0x80) {
			return i;
		}
	}
	return -1;
}

bool DXI_GetAxeMouseXYZ(int & mx, int & my, int & mz) {
	
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

static bool isMouseButton(int numb, DWORD dwOfs) {
	switch(numb) {
	case DXI_BUTTON0:
		return (dwOfs == (DWORD)DIMOFS_BUTTON0);
	case DXI_BUTTON1:
		return (dwOfs == (DWORD)DIMOFS_BUTTON1);
	case DXI_BUTTON2:
		return (dwOfs == (DWORD)DIMOFS_BUTTON2);
	case DXI_BUTTON3:
		return (dwOfs == (DWORD)DIMOFS_BUTTON3);
	case DXI_BUTTON4:
		return (dwOfs == (DWORD)DIMOFS_BUTTON4);
	case DXI_BUTTON5:
		return (dwOfs == (DWORD)DIMOFS_BUTTON5);
	case DXI_BUTTON6:
		return (dwOfs == (DWORD)DIMOFS_BUTTON6);
	case DXI_BUTTON7:
		return (dwOfs == (DWORD)DIMOFS_BUTTON7);
	default:
		return false;
	}
}

void DXI_MouseButtonCountClick(int numb, int & _iNumClick, int & _iNumUnClick) {
	
	_iNumClick = 0;
	_iNumUnClick = 0;
	
	const DIDEVICEOBJECTDATA * od = DI_MouseState->mousestate;
	for(int nb = DI_MouseState->nbele; nb; nb--, od++) {
		if(isMouseButton(numb, od->dwOfs)) {
			if(od->dwData & 0x80) {
				_iNumClick += 1;
			} else {
				_iNumUnClick += 1;
			}
		}
	}
	
}

bool DXI_MouseButtonPressed(int numb, int & _iDeltaTime) {
	
	int iTime1 = 0;
	int iTime2 = 0;
	
	const DIDEVICEOBJECTDATA * od = DI_MouseState->mousestate;
	for(int nb = DI_MouseState->nbele; nb; nb--, od++) {
		if(isMouseButton(numb, od->dwOfs) && od->dwData & 0x80) {
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

bool DXI_SetMouseRelative() {
	
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
