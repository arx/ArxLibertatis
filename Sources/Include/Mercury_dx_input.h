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
/*-----------------------------------------------------------*/
#ifndef MERCURY_DX_INPUT_H
#define MERCURY_DX_INPUT_H

#include <ARX_Common.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DXI_FAIL	0
#define DXI_OK		1

#define DXI_MODE_EXCLUSIF_ALLMSG		0
#define DXI_MODE_EXCLUSIF_OURMSG		1
#define DXI_MODE_NONEXCLUSIF_ALLMSG		2
#define DXI_MODE_NONEXCLUSIF_OURMSG		3

#define DXI_XAxis	1
#define DXI_YAxis	2
#define DXI_ZAxis	4
#define DXI_RxAxis	8
#define DXI_RyAxis	16
#define DXI_RzAxis	32
#define DXI_Slider	64
#define DXI_Button	128
#define DXI_Key		256
#define DXI_POV		512
#define DXI_Unknown 1024

#define DXI_KEYBOARD1	0
#define DXI_KEYBOARD2	1
#define DXI_MOUSE1		0
#define DXI_MOUSE2		1
#define DXI_JOY1		0
#define DXI_JOY2		1
#define DXI_SCID		0

#define DXI_BUTTON0		0
#define DXI_BUTTON1		1
#define DXI_BUTTON2		2
#define DXI_BUTTON3		3
#define DXI_BUTTON4		4
#define DXI_BUTTON5		5
#define DXI_BUTTON6		6
#define DXI_BUTTON7		7
#define DXI_BUTTON8		8
#define DXI_BUTTON9		9
#define DXI_BUTTON10	10
#define DXI_BUTTON11	11
#define DXI_BUTTON12	12
#define DXI_BUTTON13	13
#define DXI_BUTTON14	14
#define DXI_BUTTON15	15
#define DXI_BUTTON16	16
#define DXI_BUTTON17	17
#define DXI_BUTTON18	18
#define DXI_BUTTON19	19
#define DXI_BUTTON20	20
#define DXI_BUTTON21	21
#define DXI_BUTTON22	22
#define DXI_BUTTON23	23
#define DXI_BUTTON24	24
#define DXI_BUTTON25	25
#define DXI_BUTTON26	26
#define DXI_BUTTON27	27
#define DXI_BUTTON28	28
#define DXI_BUTTON29	29
#define DXI_BUTTON30	30
#define DXI_BUTTON31	31

#define DXI_JOYNONE		0
#define DXI_JOYUP		1
#define DXI_JOYDOWN		2
#define DXI_JOYLEFT		4
#define DXI_JOYRIGHT	8
	/*-----------------------------------------------------------*/

	typedef struct
	{
		void *(*malloc)(int size);
		void *(*realloc)(void * mem, int size);
		void (*free)(void * buff);
	} DXI_INIT;

	typedef struct
	{
		char			*		name;
		int						type;
		int						nbbuttons;
		int						nbaxes;
		int						info;
		int						numlist;
	} DXI_INPUT_INFO;
	/*-----------------------------------------------------------*/
	int DXI_Init(HINSTANCE h, DXI_INIT * i);
	void DXI_Release(void);
	int DXI_ChooseInputDevice(HWND hwnd, int id, int num, int mode);
	void DXI_FreeInfoDevice(DXI_INPUT_INFO * dinf);
	BOOL DXI_GetAxeMouseXYZ(int id, int * mx, int * my, int * mz);
	BOOL DXI_ExecuteAllDevices(BOOL _bKeept);
	int DXI_GetSCIDInputDevice(HWND hwnd, int id, int mode, int minbutton, int minaxe);
	int DXI_GetKeyboardInputDevice(HWND hwnd, int id, int mode);
	int DXI_GetMouseInputDevice(HWND hwnd, int id, int mode, int minbutton, int minaxe);
	int DXI_GetJoyInputDevice(HWND hwnd, int id, int mode, int minbutton, int minaxe);
	BOOL DXI_KeyPressed(int id, int dikkey);
	int DXI_GetKeyIDPressed(int id);
	void DXI_RestoreAllDevices(void);
	void DXI_SleepAllDevices(void);
	BOOL DXI_MouseButtonPressed(int id, int numb, int * _iDeltaTime);
	void DXI_MouseButtonCountClick(int id, int numb, int * _iNumClick, int * _iNumUnClick);
	BOOL DXI_IsSCIDButtonPressed(int id, int numb);
	int DXI_GetSCIDAxis(int id, int * jx, int * jy, int * jz);
	int DXI_SetMouseRelative(int id);
	int DXI_SetRangeJoy(int id, int axe, int range);
	BOOL DXI_GetJoyButtonPressed(int id, int numb);
	BOOL DXI_OldKeyPressed(int id, int dikkey);
	BOOL DXI_OldGetJoyButtonPressed(int id, int numb);
 
	/*-----------------------------------------------------------*/

#ifdef __cplusplus
}
#endif
#endif
