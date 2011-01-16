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
#ifndef MERCURY_EXTERN_H
#define MERCURY_EXTERN_H

#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <conio.h>
#include <direct.h>
#include <ARX_Common.h>

#define INITGUID
#define DIRECTINPUT_VERSION 0x0700
#include <dinput.h>

#include "Mercury_dx_input.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

/*---------------------------------------------------------*/
#define MALLOC(a)		DI_Init.malloc(a)
#define REALLOC(a,b)	DI_Init.realloc(a,b)
#define FREE(a)			DI_Init.free(a)
#define RELEASE(a){\
	if(a)\
	{\
		a->Release();\
	}\
}
#define INITSTRUCT(a){\
		memset((void*)&a,0,sizeof(a));\
		a.dwSize=sizeof(a);\
	}

#define DEVICENOACTIF	0
#define DEVICEACTIF		1

#define DFDIMOUSE		0
#define DFDIMOUSE2		1
#define DFDIKEYBOARD	2
#define DFDIJOYSTICK	3
#define DFDIJOYSTICK2	4

#define MAXKEYBOARD		1
#define MAXMOUSE		2
#define MAXJOY			2
#define MAXSCID			1
/*---------------------------------------------------------*/
typedef struct
{
	int						actif;
	char			*		name;
	GUID			*		guid;
	int						type;
	int						nbbuttons;
	int						nbaxes;
	int						info;
	int						datasid;
	int						nbele;				//pour la mouse
	LPDIRECTINPUTDEVICE7	inputdevice7;
	union
	{
		char		*		bufferstate;
		DIDEVICEOBJECTDATA	* mousestate;
		DIJOYSTATE		*	joystate;
		DIJOYSTATE2		*	joystate2;
		DIDEVICEOBJECTDATA	* SCIDstate;
	};
	union
	{
		char		*		old_bufferstate;
		DIDEVICEOBJECTDATA	* old_mousestate;
		DIJOYSTATE		*	old_joystate;
		DIJOYSTATE2		*	old_joystate2;
		DIDEVICEOBJECTDATA	* old_SCIDstate;
	};
} INPUT_INFO;
/*---------------------------------------------------------*/
extern HRESULT			DI_Hr;
extern DXI_INIT			DI_Init;
extern int				DI_NbInputInfo;
extern INPUT_INFO		DI_InputInfo[];
extern IDirectInput7	* DI_DInput7;
extern int				DI_NbKeyboard;
extern int				DI_NbMouse;
extern int				DI_NbJoy;
extern INPUT_INFO		*	DI_KeyBoardBuffer[];
extern INPUT_INFO		*	DI_MouseState[];
extern INPUT_INFO		*	DI_JoyState[];
extern INPUT_INFO		*	DI_SCIDState[];
/*---------------------------------------------------------*/

#endif
