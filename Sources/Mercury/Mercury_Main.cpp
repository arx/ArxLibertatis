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


#include "Mercury_extern.h"


#include <stdlib.h>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

//-----------------------------------------------------------------------------
#define INPUT_STATE_ADD	(512)

/*-------------------------------------------------------------*/
/*static void * DXI_malloc(int t)
{ 
	return malloc(t);
}
/*-------------------------------------------------------------*/
/*static void * DXI_Realloc(void *mem,int t)
{
	return realloc(mem,t);
}*/
/*-------------------------------------------------------------*/
/*static void DXI_free(void *mem)
{
	free(mem);
}*/
/*-------------------------------------------------------------*/
BOOL CALLBACK DIEnumDevicesCallback(LPCDIDEVICEINSTANCE lpddi,LPVOID pvRef)
{
	INPUT_INFO	*info;

	info=&DI_InputInfo[DI_NbInputInfo];
	memset((void*)info,0,sizeof(INPUT_INFO));

	//nom
	info->name=(char *)malloc(strlen(lpddi->tszInstanceName)+1);
	if(!info->name) return DIENUM_CONTINUE;
	strcpy((char*)info->name,(const char*)lpddi->tszInstanceName);

	//guid
	info->guid=(GUID*)malloc(sizeof(GUID));
	if(!info->guid)
	{
		free(info->name);
		return DIENUM_CONTINUE;
	}
	memcpy((void*)info->guid,(void*)&lpddi->guidInstance,sizeof(GUID));

	//type
	info->type=lpddi->dwDevType;

	DI_NbInputInfo++;
	return DIENUM_CONTINUE;
}
/*-------------------------------------------------------------*/
int DXI_Init(HINSTANCE h,DXI_INIT *i)
{
int	nb;

	if(!h) return DXI_FAIL;

	DI_DInput7=NULL;

	memcpy((void*)&DI_Init,(void*)i,sizeof(DXI_INIT));
/*	if(!DI_Init.malloc||!DI_Init.realloc||!DI_Init.free)
	{
//		DI_Init.malloc=DXI_malloc;
//		DI_Init.realloc=DXI_Realloc;
//		DI_Init.free=DXI_free;
	}
*/
	//Old : if(FAILED(DI_Hr=DirectInputCreateEx(h,DIRECTINPUT_VERSION,&IID_IDirectInput7,&DI_DInput7,NULL))) return DXI_FAIL;
	if(FAILED(DI_Hr=DirectInputCreateEx(h,DIRECTINPUT_VERSION,IID_IDirectInput7,(void**)&DI_DInput7,NULL))) return DXI_FAIL;

	DI_NbInputInfo=0;
	//Old : if(FAILED(DI_Hr=DI_DInput7->lpVtbl->EnumDevices(DI_DInput7,0,DIEnumDevicesCallback,NULL,DIEDFL_ATTACHEDONLY))) return DXI_FAIL;
	if(FAILED(DI_Hr=DI_DInput7->EnumDevices(0,DIEnumDevicesCallback,NULL,DIEDFL_ATTACHEDONLY))) return DXI_FAIL;

	nb=MAXKEYBOARD;
	while(nb--) DI_KeyBoardBuffer[nb]=NULL;
	nb=MAXMOUSE;
	while(nb--) DI_MouseState[nb]=NULL;
	nb=MAXJOY;
	while(nb--) DI_JoyState[nb]=NULL;
	nb=MAXSCID;
	while(nb--) DI_SCIDState[nb]=NULL;

	return DXI_OK;
}
/*-------------------------------------------------------------*/
void DXI_ReleaseDevice(INPUT_INFO *info)
{
	if(!info->actif) return;

	info->actif=DEVICENOACTIF;

	//Old : if(info->inputdevice7) info->inputdevice7->lpVtbl->Unacquire(info->inputdevice7);
	if(info->inputdevice7) info->inputdevice7->Unacquire();
	RELEASE(info->inputdevice7);
	info->inputdevice7=NULL;

	switch(GET_DIDEVICE_TYPE(info->type))
	{
	case DIDEVTYPE_MOUSE:
		if(info->mousestate)
		{
			free((void*)info->mousestate);
//			free((void*)info->old_mousestate);
			info->mousestate=NULL;
//			info->old_mousestate=NULL;
		}
		break;
	case DIDEVTYPE_KEYBOARD:
		if(info->bufferstate)
		{
			free((void*)info->bufferstate);
//			free((void*)info->old_bufferstate);
			info->bufferstate=NULL;
//			info->old_bufferstate=NULL;
		}
		break;
	case DIDEVTYPE_JOYSTICK:
		if(info->datasid==DFDIJOYSTICK)
		{
			if(info->joystate)
			{
				free((void*)info->joystate);
//				free((void*)info->old_joystate);
				info->joystate=NULL;
//				info->old_joystate=NULL;
			}
		}
		else
		{
			if(info->joystate2)
			{
				free((void*)info->joystate2);
				info->joystate2=NULL;
//				free((void*)info->old_joystate2);
//				info->old_joystate2=NULL;
			}
		}
		break;
	default:
	case DIDEVTYPE_DEVICE:
		if(info->datasid==DFDIJOYSTICK)
		{
			if(info->joystate)
			{
				free((void*)info->joystate);
//				free((void*)info->old_joystate);
				info->joystate=NULL;
//				info->old_joystate=NULL;
			}
		}
		else
		{
			if(info->joystate2)
			{
				free((void*)info->joystate2);
				info->joystate2=NULL;
//				free((void*)info->old_joystate2);
//				info->old_joystate2=NULL;
			}
		}
		break;
	}
}
/*-------------------------------------------------------------*/
void DXI_ReleaseAllDevices(void)
{
INPUT_INFO	*info;
int			nb;

	info=DI_InputInfo;
	nb=DI_NbInputInfo;
	while(nb)
	{
		DXI_ReleaseDevice(info);
		info++;
		nb--;
	}
}
/*-------------------------------------------------------------*/
void DXI_DeleteAllDevices(void)
{
INPUT_INFO	*info;

	info=DI_InputInfo;
	while(DI_NbInputInfo)
	{
		free(info->guid);
		info->guid=NULL;
		free(info->name);
		info->name=NULL;
		DXI_ReleaseDevice(info);
		info++;
		DI_NbInputInfo--;
	}
}
/*-------------------------------------------------------------*/
void DXI_Release(void)
{
INPUT_INFO	*info;

	info=DI_InputInfo;
	while(DI_NbInputInfo)
	{
		free(info->guid);
		info->guid=NULL;
		free(info->name);
		info->name=NULL;
		DXI_ReleaseDevice(info);
		info++;
		DI_NbInputInfo--;
	}

	RELEASE(DI_DInput7);
	DI_DInput7 = NULL;
}
/*-------------------------------------------------------------*/
BOOL CompareGUID(GUID *g1,GUID *g2)
{
int		i,j,*m1,*m2;
char	*mm1,*mm2;

	i=sizeof(GUID);
	j=i&3;
	i>>=2;

	m1=(int*)g1;
	m2=(int*)g2;
	while(i)
	{
		if(*m1++!=*m2++) return FALSE;
		i--;
	}

	mm1=(char*)m1;
	mm2=(char*)m2;
	while(j)
	{
		if(*mm1++!=*mm2++) return FALSE;
		j--;
	}

	return TRUE;
}
/*-------------------------------------------------------------*/
BOOL CALLBACK DIEnumDeviceObjectsCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi,LPVOID pvRef)
{
INPUT_INFO			*info;

	info=(INPUT_INFO *)pvRef;
	if(CompareGUID((GUID*)&lpddoi->guidType,(GUID*)&GUID_XAxis)) info->info|=DXI_XAxis;
	if(CompareGUID((GUID*)&lpddoi->guidType,(GUID*)&GUID_YAxis)) info->info|=DXI_YAxis;
	if(CompareGUID((GUID*)&lpddoi->guidType,(GUID*)&GUID_ZAxis)) info->info|=DXI_ZAxis;
	if(CompareGUID((GUID*)&lpddoi->guidType,(GUID*)&GUID_RxAxis)) info->info|=DXI_RxAxis;
	if(CompareGUID((GUID*)&lpddoi->guidType,(GUID*)&GUID_RyAxis)) info->info|=DXI_RyAxis;
	if(CompareGUID((GUID*)&lpddoi->guidType,(GUID*)&GUID_RzAxis)) info->info|=DXI_RzAxis;
	if(CompareGUID((GUID*)&lpddoi->guidType,(GUID*)&GUID_Slider)) info->info|=DXI_Slider;
	if(CompareGUID((GUID*)&lpddoi->guidType,(GUID*)&GUID_Button)) info->info|=DXI_Button;
	if(CompareGUID((GUID*)&lpddoi->guidType,(GUID*)&GUID_Key)) info->info|=DXI_Key;
	if(CompareGUID((GUID*)&lpddoi->guidType,(GUID*)&GUID_POV)) info->info|=DXI_POV;
	if(CompareGUID((GUID*)&lpddoi->guidType,(GUID*)&GUID_Unknown)) info->info|=DXI_Unknown;

	return DIENUM_CONTINUE;
}
/*-------------------------------------------------------------*/
static INPUT_INFO * DXI_GetInputInfoWithState(void *state,int type)
{
int			nbdev;
INPUT_INFO	*info;

	info=DI_InputInfo;
	nbdev=DI_NbInputInfo;
	while(nbdev)
	{
		if(GET_DIDEVICE_TYPE(info->type)==type)
		{
			switch(type)	
			{
			case DIDEVTYPE_MOUSE:
//				if(info->mousestate==state) return info;
				if(state==info) return info;
				break;
			case DIDEVTYPE_KEYBOARD:
				if(info->bufferstate==state) return info;
				break;
			case DIDEVTYPE_JOYSTICK:
				if(state==info) return info;
				break;
			default:
			case DIDEVTYPE_DEVICE:
				break;
			}
		}
		info++;
		nbdev--;
	}

	return NULL;
}
/*-------------------------------------------------------------*/
void DXI_RestoreAllDevices(void)
{
int			nbdev;
INPUT_INFO	*info;

	info=DI_InputInfo;
	nbdev=DI_NbInputInfo;
	while(nbdev)
	{
		if(info->actif)
		{
			//Old : info->inputdevice7->lpVtbl->Acquire(info->inputdevice7);
			info->inputdevice7->Acquire();
		}
		info++;
		nbdev--;
	}
}
/*-------------------------------------------------------------*/
void DXI_SleepAllDevices(void)
{
int			nbdev;
INPUT_INFO	*info;

	info=DI_InputInfo;
	nbdev=DI_NbInputInfo;
	while(nbdev)
	{
		if(info->actif)
		{
			//Old : info->inputdevice7->lpVtbl->Unacquire(info->inputdevice7);
			info->inputdevice7->Unacquire();
		}
		info++;
		nbdev--;
	}
}
/*-------------------------------------------------------------*/
int DXI_GetKeyboardInputDevice(HWND hwnd,int id,int mode)
{
int			nbdev,num=0;
INPUT_INFO	*info;

	if(id>=MAXKEYBOARD) 
		return DXI_FAIL;
	if(DI_KeyBoardBuffer[id])
	{
		info=DXI_GetInputInfoWithState(DI_KeyBoardBuffer[id],DIDEVTYPE_KEYBOARD);
		if(info) DXI_ReleaseDevice(info);
		DI_KeyBoardBuffer[id]=NULL;
	}

	info=DI_InputInfo;
	nbdev=DI_NbInputInfo;
	while(nbdev)
	{
		if((GET_DIDEVICE_TYPE(info->type)==DIDEVTYPE_KEYBOARD)&&(!info->actif))
		{
			if(DXI_ChooseInputDevice(hwnd,id,num,mode)==DXI_OK) 
				return DXI_OK;
		}
		num++;
		info++;
		nbdev--;
	}

	return DXI_FAIL;
}
/*-------------------------------------------------------------*/
int DXI_GetMouseInputDevice(HWND hwnd,int id,int mode,int minbutton,int minaxe)
{
int			nbdev,num=0;
INPUT_INFO	*info;

	if(id>=MAXMOUSE) return DXI_FAIL;
	if(DI_MouseState[id])
	{
		info=DXI_GetInputInfoWithState(DI_MouseState[id],DIDEVTYPE_MOUSE);
		if(info) DXI_ReleaseDevice(info);
		DI_MouseState[id]=NULL;
	}

	info=DI_InputInfo;
	nbdev=DI_NbInputInfo;
	while(nbdev)
	{
		if((GET_DIDEVICE_TYPE(info->type)==DIDEVTYPE_MOUSE)&&(!info->actif))
		{
			if(DXI_ChooseInputDevice(hwnd,id,num,mode)==DXI_OK)
			{
				if((info->nbbuttons>=minbutton)&&(info->nbaxes>=minaxe)) return DXI_OK;
				else
				{
					DXI_ReleaseDevice(info);
				}
			}
		}
		num++;
		info++;
		nbdev--;
	}

	return DXI_FAIL;
}
/*-------------------------------------------------------------*/
int DXI_GetJoyInputDevice(HWND hwnd,int id,int mode,int minbutton,int minaxe)
{
int			nbdev,num=0;
INPUT_INFO	*info;

	if(id>=MAXJOY) return DXI_FAIL;
	if(DI_JoyState[id])
	{
		info=DXI_GetInputInfoWithState(DI_JoyState[id],DIDEVTYPE_JOYSTICK);
		if(info) DXI_ReleaseDevice(info);
		DI_JoyState[id]=NULL;
	}

	info=DI_InputInfo;
	nbdev=DI_NbInputInfo;
	while(nbdev)
	{
		if((GET_DIDEVICE_TYPE(info->type)==DIDEVTYPE_JOYSTICK)&&(!info->actif))
		{
			if(DXI_ChooseInputDevice(hwnd,id,num,mode)==DXI_OK)
			{
				if((info->nbbuttons>=minbutton)&&(info->nbaxes>=minaxe)) return DXI_OK;
				else
				{
					DXI_ReleaseDevice(info);
				}
			}
		}
	
		num++;
		info++;
		nbdev--;
	}

	return DXI_FAIL;
}
int DXI_GetSCIDInputDevice(HWND hwnd,int id,int mode,int minbutton,int minaxe)
{
int			nbdev,num=0;
INPUT_INFO	*info;

/*	if(id>=MAXJOY) return DXI_FAIL;
	if(DI_JoyState[id])
	{
		info=DXI_GetInputInfoWithState(DI_JoyState[id],DIDEVTYPE_JOYSTICK);
		if(info) DXI_ReleaseDevice(info);
		DI_JoyState[id]=NULL;
	}
*/  // A checker....
	info=DI_InputInfo;
	nbdev=DI_NbInputInfo;
	while(nbdev)
	{
		if((GET_DIDEVICE_TYPE(info->type)==DIDEVTYPE_DEVICE )&&(!info->actif))
		{
			if (!strcmp("Microsoft SideWinder Strategic Commander",info->name))
			if(DXI_ChooseInputDevice(hwnd,id,num,mode)==DXI_OK)
			{
				if((info->nbbuttons>=minbutton)&&(info->nbaxes>=minaxe)) return DXI_OK;
				else
				{
					DXI_ReleaseDevice(info);
				}
			}
		}
		
		
		num++;
		info++;
		nbdev--;
	}

	return DXI_FAIL;
}
/*-------------------------------------------------------------*/
int DXI_ChooseInputDevice( HWND hwnd, int id, int num, int mode )
{
DIDEVCAPS		devcaps;
INPUT_INFO*		info;
int				flag;
DIDATAFORMAT*	dformat;

	if( num >= DI_NbInputInfo ) return DXI_FAIL;
	info = &DI_InputInfo[num];

	DXI_ReleaseDevice( info );
	//Old : if( FAILED( DI_Hr = DI_DInput7->lpVtbl->CreateDeviceEx( DI_DInput7, info->guid, &IID_IDirectInputDevice7, &info->inputdevice7, NULL ) ) ) return DXI_FAIL;
	if( FAILED( DI_Hr = DI_DInput7->CreateDeviceEx(*(info->guid), IID_IDirectInputDevice7, (void**)&info->inputdevice7, NULL ) ) ) return DXI_FAIL;

	INITSTRUCT( devcaps );
	//Old : if( FAILED( DI_Hr = info->inputdevice7->lpVtbl->GetCapabilities( info->inputdevice7, &devcaps ) ) ) return DXI_FAIL;
	if( FAILED( DI_Hr = info->inputdevice7->GetCapabilities(&devcaps ) ) ) return DXI_FAIL;

	info->nbbuttons	=	devcaps.dwButtons;
	info->nbaxes	=	devcaps.dwAxes;

	switch( mode )
	{
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
		//ARX_BEGIN: jycorbel (2010-06-30) - clean warning on not-initialized variable
		// flag should be always set unless 'mode' doesn't match any case which could resume on a fatal error
		default:
			ARX_CHECK_NO_ENTRY();
			flag = 0; //clean warning
		//ARX_END: jycorbel (2010-06-30)
	}

	//Old : 	if( FAILED( DI_Hr = info->inputdevice7->lpVtbl->SetCooperativeLevel( info->inputdevice7, hwnd, flag ) ) ) return DXI_FAIL;
	if( FAILED( DI_Hr = info->inputdevice7->SetCooperativeLevel(hwnd, flag ) ) ) return DXI_FAIL;
	//Old : 	if( FAILED( DI_Hr = info->inputdevice7->lpVtbl->EnumObjects( info->inputdevice7, DIEnumDeviceObjectsCallback, (void*)info, DIDFT_ALL ) ) ) return DXI_FAIL;
	if( FAILED( DI_Hr = info->inputdevice7->EnumObjects(DIEnumDeviceObjectsCallback, (void*)info, DIDFT_ALL ) ) ) return DXI_FAIL;

	switch( GET_DIDEVICE_TYPE( info->type ) )
	{
	case DIDEVTYPE_MOUSE:
		{
			DIPROPDWORD dipdw={
				// the header
				{
					sizeof(DIPROPDWORD),        // diph.dwSize
					sizeof(DIPROPHEADER),       // diph.dwHeaderSize
					0,                          // diph.dwObj
					DIPH_DEVICE,                // diph.dwHow
				},
		        // the data
				128,              // dwData
			};

			info->mousestate=(DIDEVICEOBJECTDATA*)malloc(sizeof(DIDEVICEOBJECTDATA)*(info->nbbuttons+info->nbaxes+INPUT_STATE_ADD));
			memset(info->mousestate,0,(sizeof(DIDEVICEOBJECTDATA)*(info->nbbuttons+info->nbaxes+INPUT_STATE_ADD)));
//			info->old_mousestate=(DIDEVICEOBJECTDATA*)malloc(sizeof(DIDEVICEOBJECTDATA)*(info->nbbuttons+info->nbaxes));
//			memset(info->old_mousestate,0,(sizeof(DIDEVICEOBJECTDATA)*(info->nbbuttons+info->nbaxes)));
//			DI_MouseState[id]=info->mousestate;
			DI_MouseState[id]=info;
			if(info->nbbuttons>4)
			{
				info->datasid=DFDIMOUSE2;
				dformat=(DIDATAFORMAT*)&c_dfDIMouse2;
			}
			else
			{
				info->datasid=DFDIMOUSE;
				dformat=(DIDATAFORMAT*)&c_dfDIMouse;
			}
			//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->SetProperty(info->inputdevice7,DIPROP_BUFFERSIZE,&dipdw.diph))) return DXI_FAIL;
			if(FAILED(DI_Hr=info->inputdevice7->SetProperty(DIPROP_BUFFERSIZE,&dipdw.diph))) return DXI_FAIL;
		}
		break;
	case DIDEVTYPE_KEYBOARD:
		info->datasid=DFDIKEYBOARD;
		info->bufferstate=(char*)malloc(256);
		memset(info->bufferstate,0,256);
//		info->old_bufferstate=(char*)malloc(256);
//		memset(info->old_bufferstate,0,256);
		DI_KeyBoardBuffer[id]=info;//->bufferstate;
		//DI_OldKeyBoardBuffer[id]=info->old_bufferstate;
		dformat=(DIDATAFORMAT*)&c_dfDIKeyboard;
		break;
	case DIDEVTYPE_JOYSTICK:
		DI_JoyState[id]=info;
		if(info->nbaxes>2)
		{
			info->joystate2=(DIJOYSTATE2*)malloc(sizeof(DIJOYSTATE2));
			memset(info->joystate2,0,sizeof(DIJOYSTATE2));
//			info->old_joystate2=(DIJOYSTATE2*)malloc(sizeof(DIJOYSTATE2));
//			memset(info->old_joystate2,0,sizeof(DIJOYSTATE2));
			info->datasid=DFDIJOYSTICK2;
			dformat=(DIDATAFORMAT*)&c_dfDIJoystick2;
		}
		else
		{
			info->joystate=(DIJOYSTATE*)malloc(sizeof(DIJOYSTATE));
			memset(info->joystate,0,sizeof(DIJOYSTATE));
//			info->old_joystate=(DIJOYSTATE*)malloc(sizeof(DIJOYSTATE));
//			memset(info->old_joystate,0,sizeof(DIJOYSTATE));
			info->datasid=DFDIJOYSTICK;
			dformat=(DIDATAFORMAT*)&c_dfDIJoystick;
		}
		break;
	default:
	case DIDEVTYPE_DEVICE:
		if (!strcmp("Microsoft SideWinder Strategic Commander",info->name))
		{
			DI_SCIDState[id]=info;
			if(info->nbaxes>2)
			{
				info->joystate2=(DIJOYSTATE2*)malloc(sizeof(DIJOYSTATE2));
				memset(info->joystate2,0,sizeof(DIJOYSTATE2));
//				info->old_joystate2=(DIJOYSTATE2*)malloc(sizeof(DIJOYSTATE2));
//				memset(info->old_joystate2,0,sizeof(DIJOYSTATE2));
				info->datasid=DFDIJOYSTICK2;
				dformat=(DIDATAFORMAT*)&c_dfDIJoystick2;
			}
			else
			{
				info->joystate=(DIJOYSTATE*)malloc(sizeof(DIJOYSTATE));
				memset(info->joystate,0,sizeof(DIJOYSTATE));
//				info->old_joystate=(DIJOYSTATE*)malloc(sizeof(DIJOYSTATE));
//				memset(info->old_joystate,0,sizeof(DIJOYSTATE));
				info->datasid=DFDIJOYSTICK;
				dformat=(DIDATAFORMAT*)&c_dfDIJoystick;
			}
		
			/*
			DIPROPDWORD dipdw={
				// the header
				{
					sizeof(DIPROPDWORD),        // diph.dwSize
					sizeof(DIPROPHEADER),       // diph.dwHeaderSize
					0,                          // diph.dwObj
					DIPH_DEVICE,                // diph.dwHow
				},
		        // the data
				16,              // dwData
			};

			info->SCIDstate=(DIDEVICEOBJECTDATA*)malloc(sizeof(DIDEVICEOBJECTDATA)*(info->nbbuttons+info->nbaxes));
			info->old_SCIDstate=(DIDEVICEOBJECTDATA*)malloc(sizeof(DIDEVICEOBJECTDATA)*(info->nbbuttons+info->nbaxes));
//			DI_MouseState[id]=info->mousestate;
			DI_SCIDState[id]=info;
			if(info->nbbuttons>4)
			{
				info->datasid=DFDIMOUSE2;
				dformat=(DIDATAFORMAT*)&c_dfDIMouse2;
			}
			else
			{
				info->datasid=DFDIMOUSE;
				dformat=(DIDATAFORMAT*)&c_dfDIMouse;
			}
			
			if(FAILED(DI_Hr=info->inputdevice7->SetProperty(info->inputdevice7,DIPROP_BUFFERSIZE,&dipdw.diph))) return DXI_FAIL;
*/
		}
		else dformat=NULL;
		break;
	}
	if(!dformat) return DXI_FAIL;
	//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->SetDataFormat(info->inputdevice7,dformat))) return DXI_FAIL;
	if(FAILED(DI_Hr=info->inputdevice7->SetDataFormat(dformat))) return DXI_FAIL;
	//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->Acquire(info->inputdevice7))) 
	if(FAILED(DI_Hr=info->inputdevice7->Acquire())) 
	{}
	//	info->actif=DEVICENOACTIF;
	//	return DXI_FAIL;
	//else 
	info->actif=DEVICEACTIF;
	return DXI_OK;
}
/*-------------------------------------------------------------*/
DXI_INPUT_INFO * DXI_GetInfoDevice(int num)
{
DXI_INPUT_INFO	*dinf;
INPUT_INFO		*info;

	if(num>=DI_NbInputInfo) return NULL;
	dinf=(DXI_INPUT_INFO*)malloc(sizeof(DXI_INPUT_INFO));
	if(!dinf) return NULL;

	info=&DI_InputInfo[num];
	dinf->name=(char*)malloc(strlen(info->name)+1);
	if(!dinf)
	{
		free((void*)dinf);
		return NULL;
	}

	strcpy(dinf->name,info->name);
	dinf->type=info->type;
	dinf->numlist=num;
	if(info->inputdevice7)
	{
		dinf->nbbuttons=info->nbbuttons;
		dinf->nbaxes=info->nbaxes;
		dinf->info=info->info;
	}
	else
	{
		dinf->nbbuttons=-1;
		dinf->nbaxes=-1;
		dinf->info=-1;
	}

	return dinf;
}
BOOL DXI_CleanAxeMouseZ(int id)
{
DIDEVICEOBJECTDATA	*od;
DIDEVICEOBJECTDATA	*od2;
int					nb,flg=0;

return FALSE;

	nb=DI_MouseState[id]->nbele;
	if(!nb) return FALSE;
	od=DI_MouseState[id]->mousestate;
//	od2=DI_MouseState[id]->old_mousestate;
	while(nb)
	{
		switch(od->dwOfs)
		{
		case DIMOFS_X:
			flg++;
			break;
		case DIMOFS_Y:
			flg++;
			break;
		case DIMOFS_Z:
			od->dwData-=od2->dwData;
			flg++;
			break;
		default:
			break;
		}
		od++;
		od2++;
		nb--;
	}
	return (flg>0);
}

/*-------------------------------------------------------------*/
void DXI_freeInfoDevice(DXI_INPUT_INFO *dinf)
{
	if(!dinf) return;
	if(dinf->name) free((void*)dinf->name);
	free((void*)dinf);
}
/*-------------------------------------------------------------*/
BOOL DXI_ExecuteAllDevices(BOOL _bKeept)
{
int			nb,nbele;
DWORD		dwNbele;//ARX: xrichter (2010-06-30) - treat warnings C4057 for 'LPDWORD' differs in indirection to slightly different base types from 'int *'
INPUT_INFO	*info;
BOOL		flg=TRUE;
void * temp;
	
//DIDEVICEOBJECTDATA	* od;
//DIDEVICEOBJECTDATA	* odd;
				

	info=DI_InputInfo;
	nb=DI_NbInputInfo;
	
	while(nb)
	{
		if(info->actif)
		{
			// union!!!
			temp=info->mousestate;
//			info->mousestate=info->old_mousestate;
//			info->old_mousestate=temp;
			switch(GET_DIDEVICE_TYPE(info->type))
			{

			//ARX_BEGIN: xrichter (2010-06-30) - treat warnings C4057 for 'LPDWORD' differs in indirection to slightly different base types from 'int *'
			case DIDEVTYPE_MOUSE:
				nbele=info->nbbuttons+info->nbaxes+INPUT_STATE_ADD;
				dwNbele=(DWORD)nbele; 
				//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->GetDeviceData(info->inputdevice7,sizeof(DIDEVICEOBJECTDATA),info->mousestate,&dwNbele,(_bKeept)?DIGDD_PEEK:0 ))) 
				if(FAILED(DI_Hr=info->inputdevice7->GetDeviceData(sizeof(DIDEVICEOBJECTDATA),info->mousestate,&dwNbele,(_bKeept)?DIGDD_PEEK:0 ))) 
				{
					nbele=info->nbbuttons+info->nbaxes+INPUT_STATE_ADD;
					dwNbele=(DWORD)nbele; 
					//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->GetDeviceData(info->inputdevice7,sizeof(DIDEVICEOBJECTDATA),info->mousestate,&dwNbele,(_bKeept)?DIGDD_PEEK:0 ))) 
					if(FAILED(DI_Hr=info->inputdevice7->GetDeviceData(sizeof(DIDEVICEOBJECTDATA),info->mousestate,&dwNbele,(_bKeept)?DIGDD_PEEK:0 ))) 
					{
						//Old : info->inputdevice7->lpVtbl->GetDeviceData(info->inputdevice7,sizeof(DIDEVICEOBJECTDATA),info->mousestate,&dwNbele,(_bKeept)?DIGDD_PEEK:0 );
						info->inputdevice7->GetDeviceData(sizeof(DIDEVICEOBJECTDATA),info->mousestate,&dwNbele,(_bKeept)?DIGDD_PEEK:0 );
			//ARX_END: xrichter (2010-06-30)	

//					DXI_RestoreAllDevices();
				//	info->actif=DEVICENOACTIF;
					/*
					if (DIERR_INPUTLOST == DI_Hr) 
					{
				//		DXI_RestoreAllDevices();
					}
					if (DIERR_NOTACQUIRED  == DI_Hr)
					{
				//		DXI_RestoreAllDevices();
						//info->inputdevice7->Acquire
					}
				/*	if (DIERR_INVALIDPARAM  == DI_Hr) MessageBox(NULL,"DIERR_INVALIDPARAM ","",0);
					if (DIERR_NOTACQUIRED  == DI_Hr) MessageBox(NULL,"DIERR_NOTACQUIRED ","",0);
					if (DIERR_NOTINITIALIZED  == DI_Hr) MessageBox(NULL,"DIERR_NOTINITIALIZED ","",0);
					if (E_PENDING  == DI_Hr) MessageBox(NULL,"E_PENDING ","",0);
*/
					flg=FALSE;
					}
				}
				DXI_CleanAxeMouseZ(DXI_MOUSE1); //////////////////////////
			//	od=info->mousestate;
			//	odd=info->old_mousestate;
			//	od++;od++;
			//	odd++;odd++;
			//	od->dwData-=odd->dwData;
				
				nbele=(int)dwNbele; //ARX: xrichter (2010-06-30) - treat warnings C4057 for 'LPDWORD' differs in indirection to slightly different base types from 'int *'
				info->nbele=nbele;
				break;
			case DIDEVTYPE_KEYBOARD: 
				
				//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->GetDeviceState(info->inputdevice7,256,(void*)info->bufferstate))) 
				if(FAILED(DI_Hr=info->inputdevice7->GetDeviceState(256,(void*)info->bufferstate))) 
				{
					DXI_RestoreAllDevices(); 
					
					//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->GetDeviceState(info->inputdevice7,256,(void*)info->bufferstate))) 
					if(FAILED(DI_Hr=info->inputdevice7->GetDeviceState(256,(void*)info->bufferstate))) 
					{
//					DXI_RestoreAllDevices();

					/*
					if (DIERR_INPUTLOST == DI_Hr) 
					{
						//DXI_RestoreAllDevices(); 
						//Old : info->inputdevice7->lpVtbl->Acquire(info->inputdevice7);
						info->inputdevice7->Acquire(info->inputdevice7);
					}
					if (DIERR_NOTACQUIRED  == DI_Hr)
					{
						//DXI_RestoreAllDevices(); 
						//Old : info->inputdevice7->lpVtbl->Acquire(info->inputdevice7);
						info->inputdevice7->Acquire(info->inputdevice7);
					}
					if (DIERR_NOTINITIALIZED  == DI_Hr) 
					{	//Old : info->inputdevice7->lpVtbl->Acquire(info->inputdevice7);
						info->inputdevice7->Acquire(info->inputdevice7);
						//MessageBox(NULL,"DIERR_NOTINITIALIZED ","",0);
					}
					/*
					if (DIERR_INVALIDPARAM  == DI_Hr) MessageBox(NULL,"DIERR_INVALIDPARAM ","",0);
					if (DIERR_NOTACQUIRED  == DI_Hr) MessageBox(NULL,"DIERR_NOTACQUIRED ","",0);
					if (DIERR_NOTINITIALIZED  == DI_Hr) MessageBox(NULL,"DIERR_NOTINITIALIZED ","",0);
					if (E_PENDING  == DI_Hr) MessageBox(NULL,"E_PENDING ","",0);
					*/		
					//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->GetDeviceState(info->inputdevice7,256,(void*)info->bufferstate))) 
//					if(FAILED(DI_Hr=info->inputdevice7->GetDeviceState(256,(void*)info->bufferstate))) 
						memset(info->bufferstate,0,256); //seb 27/03/2002
						flg=FALSE;					
					}
					
				}
				break;
			case DIDEVTYPE_JOYSTICK: 
				//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->Poll(info->inputdevice7))) flg=FALSE;
				if(FAILED(DI_Hr=info->inputdevice7->Poll())) flg=FALSE;

				if(info->datasid==DFDIJOYSTICK2)
				{	
					//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->GetDeviceState(info->inputdevice7,sizeof(DIJOYSTATE2),(void*)info->joystate2))) 
					if(FAILED(DI_Hr=info->inputdevice7->GetDeviceState(sizeof(DIJOYSTATE2),(void*)info->joystate2))) 
						flg=FALSE;
				}
				else
				{	
					//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->GetDeviceState(info->inputdevice7,sizeof(DIJOYSTATE),(void*)info->joystate))) 
					if(FAILED(DI_Hr=info->inputdevice7->GetDeviceState(sizeof(DIJOYSTATE),(void*)info->joystate))) 
						flg=FALSE;						
				}
				break;
			default:
			case DIDEVTYPE_DEVICE: 
				//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->Poll(info->inputdevice7))) 
				if(FAILED(DI_Hr=info->inputdevice7->Poll())) 
					flg=FALSE;
					
				if(info->datasid==DFDIJOYSTICK2)
				{	
					//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->GetDeviceState(info->inputdevice7,sizeof(DIJOYSTATE2),(void*)info->joystate2))) 
					if(FAILED(DI_Hr=info->inputdevice7->GetDeviceState(sizeof(DIJOYSTATE2),(void*)info->joystate2))) 
						flg=FALSE;
				/*	else
					{
						long togo=sizeof(DIJOYSTATE2);
						long ii=0;
						char * dat1=(char *)info->joystate2;
						char * dat2=(char *)info->old_joystate2;
						while (ii<togo)
						{							
							if (dat1[ii]!=dat2[ii]) 
								dat1[ii]=dat1[ii];
							ii++;
						}
					}*/

				}
				else
				{	
					//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->GetDeviceState(info->inputdevice7,sizeof(DIJOYSTATE),(void*)info->SCIDstate))) 
					if(FAILED(DI_Hr=info->inputdevice7->GetDeviceState(sizeof(DIJOYSTATE),(void*)info->SCIDstate))) 
						flg=FALSE;							
				}
				break;
				/*
				nbele=info->nbbuttons+info->nbaxes; 
				// Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->GetDeviceData(info->inputdevice7,sizeof(DIDEVICEOBJECTDATA),info->SCIDstate,&nbele,0))) 
				if(FAILED(DI_Hr=info->inputdevice7->GetDeviceData(sizeof(DIDEVICEOBJECTDATA),info->SCIDstate,&nbele,0))) 
					flg=FALSE;			
				info->nbele=nbele;
				break;*/
			}
		}
		info++;
		nb--;
	}

	return flg;
}
/*-------------------------------------------------------------*/
BOOL DXI_KeyPressed(int id,int dikkey)
{
	if(DI_KeyBoardBuffer[id]->bufferstate[dikkey]&0x80) return TRUE;
	return FALSE;
}
BOOL DXI_OldKeyPressed(int id,int dikkey)
{
	//if(DI_InputInfo->old_bufferstate[id*dikkey]&0x80) return TRUE;
//	if(DI_KeyBoardBuffer[id]->old_bufferstate[dikkey]&0x80) return TRUE;
	return FALSE;
}
/*-------------------------------------------------------------*/
int DXI_GetKeyIDPressed(int id)
{
int		nb;
char	*buf;

	buf=DI_KeyBoardBuffer[id]->bufferstate;
	nb=256;
	while(nb)
	{
		if((*buf)&0x80) 
			return 256-nb;
		buf++;
		nb--;
	}
	return -1;
}
void DXI_ClearKeys(int id)
{
	memset(DI_KeyBoardBuffer[id],0,256);	
}
/*-------------------------------------------------------------*/
BOOL DXI_GetAxeMouseXY(int id,int *mx,int *my)
{
DIDEVICEOBJECTDATA	*od;
int					nb,flg=0;

	nb=DI_MouseState[id]->nbele;
	if(!nb) return FALSE;
	od=DI_MouseState[id]->mousestate;
	while(nb)
	{
		switch(od->dwOfs)
		{
		case DIMOFS_X:
			*mx=od->dwData;
			flg++;
			break;
		case DIMOFS_Y:
			*my=od->dwData;
			flg++;
			break;
		default:
			break;
		}
		od++;
		nb--;
	}
	return(flg>0);
}
/*-------------------------------------------------------------*/
BOOL DXI_GetAxeMouseXYZ(int id,int *mx,int *my,int *mz)
{
DIDEVICEOBJECTDATA	*od;
int					nb,flg=0;

	*mx=*my=*mz=0;

	nb=DI_MouseState[id]->nbele;
	if(!nb) return FALSE;
	od=DI_MouseState[id]->mousestate;
	while(nb)
	{
		switch(od->dwOfs)
		{
		case DIMOFS_X:
			*mx+=od->dwData;
			flg++;
			break;
		case DIMOFS_Y:
			*my+=od->dwData;
			flg++;
			break;
		case DIMOFS_Z:
			*mz+=od->dwData;
			flg++;
			break;
		default:
			break;
		}
		od++;
		nb--;
	}
	return (flg>0);
}

/*-------------------------------------------------------------*/
BOOL DXI_MouseButtonImage(int id,int numb)
{
DIDEVICEOBJECTDATA	*od;
int					state,nb;
static FILE *fTemp=NULL;

	if(!fTemp)
	{
		fTemp=fopen("c:\\temp\\dinput.txt","wb");
	}

	nb=DI_MouseState[id]->nbele;
	if(!nb) return FALSE;
	od=DI_MouseState[id]->mousestate;
	while(nb)
	{
		switch(numb)
		{
		case DXI_BUTTON0:
			if(od->dwOfs==DIMOFS_BUTTON0)
			{
				state=od->dwData;
				if(state&0x80)
				{
					fprintf(fTemp,"1");
				}
				else
				{
					fprintf(fTemp,"0");
				}
			}
			break;
		case DXI_BUTTON1:
			if(od->dwOfs==DIMOFS_BUTTON1)
			{
				state=od->dwData;
				if(state&0x80)
				{
					fprintf(fTemp,"1");
				}
				else
				{
					fprintf(fTemp,"0");
				}
			}
			break;
		case DXI_BUTTON2:
			break;
		case DXI_BUTTON3:
			break;
		case DXI_BUTTON4:
			break;
		case DXI_BUTTON5:
			break;
		case DXI_BUTTON6:
			break;
		case DXI_BUTTON7:
			break;
		default:
			return FALSE;
		}

		od++;
		nb--;
	}

	fprintf(fTemp,"\r\n---------\r\n");
	return TRUE;
}

/*-------------------------------------------------------------*/
void DXI_MouseButtonCountClick(int id,int numb,int *_iNumClick,int *_iNumUnClick)
{
DIDEVICEOBJECTDATA	*od;
int					state,nb;

	*_iNumClick=0;
	*_iNumUnClick=0;

	nb=DI_MouseState[id]->nbele;
	if(!nb) return;
	od=DI_MouseState[id]->mousestate;
	while(nb)
	{
		switch(numb)
		{
		case DXI_BUTTON0:
			if(od->dwOfs==DIMOFS_BUTTON0)
			{
				state=od->dwData;
				if(state&0x80)
				{
					*_iNumClick+=1;
				}
				else
				{
					*_iNumUnClick+=1;
				}
			}
			break;
		case DXI_BUTTON1:
			if(od->dwOfs==DIMOFS_BUTTON1)
			{
				state=od->dwData;
				if(state&0x80)
				{
					*_iNumClick+=1;
				}
				else
				{
					*_iNumUnClick+=1;
				}
			}
			break;
		case DXI_BUTTON2:
			if(od->dwOfs==DIMOFS_BUTTON2)
			{
				state=od->dwData;
				if(state&0x80)
				{
					*_iNumClick+=1;
				}
				else
				{
					*_iNumUnClick+=1;
				}
			}
			break;
		case DXI_BUTTON3:
			if(od->dwOfs==DIMOFS_BUTTON3)
			{
				state=od->dwData;
				if(state&0x80)
				{
					*_iNumClick+=1;
				}
				else
				{
					*_iNumUnClick+=1;
				}
			}
			break;
		case DXI_BUTTON4:
			if(od->dwOfs==DIMOFS_BUTTON4)
			{
				state=od->dwData;
				if(state&0x80)
				{
					*_iNumClick+=1;
				}
				else
				{
					*_iNumUnClick+=1;
				}
			}
			break;
		case DXI_BUTTON5:
			if(od->dwOfs==DIMOFS_BUTTON5)
			{
				state=od->dwData;
				if(state&0x80)
				{
					*_iNumClick+=1;
				}
				else
				{
					*_iNumUnClick+=1;
				}
			}
			break;
		case DXI_BUTTON6:
			if(od->dwOfs==DIMOFS_BUTTON6)
			{
				state=od->dwData;
				if(state&0x80)
				{
					*_iNumClick+=1;
				}
				else
				{
					*_iNumUnClick+=1;
				}
			}
			break;
		case DXI_BUTTON7:
			if(od->dwOfs==DIMOFS_BUTTON7)
			{
				state=od->dwData;
				if(state&0x80)
				{
					*_iNumClick+=1;
				}
				else
				{
					*_iNumUnClick+=1;
				}
			}
			break;
		default:
			break;
		}

		od++;
		nb--;
	}
}

/*-------------------------------------------------------------*/
BOOL DXI_MouseButtonPressed(int id,int numb,int *_iDeltaTime)
{
DIDEVICEOBJECTDATA	*od;
int					state,iTime1,iTime2,nb;
BOOL				bResult;

	nb=DI_MouseState[id]->nbele;
	if(!nb) return FALSE;
	od=DI_MouseState[id]->mousestate;
	iTime1=iTime2=0;
	while(nb)
	{
		bResult=FALSE;
		switch(numb)
		{
		case DXI_BUTTON0:
			if(od->dwOfs==DIMOFS_BUTTON0)
			{
				state=od->dwData;
				if(state&0x80)
				{
					bResult=TRUE;
				}
			}
			break;
		case DXI_BUTTON1:
			if(od->dwOfs==DIMOFS_BUTTON1)
			{
				state=od->dwData;
				if(state&0x80)
				{
					bResult=TRUE;
				}
			}
			break;
		case DXI_BUTTON2:
			if(od->dwOfs==DIMOFS_BUTTON2)
			{
				state=od->dwData;
				if(state&0x80)
				{
					bResult=TRUE;
				}
			}
			break;
		case DXI_BUTTON3:
			if(od->dwOfs==DIMOFS_BUTTON3)
			{
				state=od->dwData;
				if(state&0x80)
				{
					bResult=TRUE;
				}
			}
			break;
		case DXI_BUTTON4:
			if(od->dwOfs==DIMOFS_BUTTON4)
			{
				state=od->dwData;
				if(state&0x80)
				{
					bResult=TRUE;
				}
			}
			break;
		case DXI_BUTTON5:
			if(od->dwOfs==DIMOFS_BUTTON5)
			{
				state=od->dwData;
				if(state&0x80)
				{
					bResult=TRUE;
				}
			}
			break;
		case DXI_BUTTON6:
			if(od->dwOfs==DIMOFS_BUTTON6)
			{
				state=od->dwData;
				if(state&0x80)
				{
					bResult=TRUE;
				}
			}
			break;
		case DXI_BUTTON7:
			if(od->dwOfs==DIMOFS_BUTTON7)
			{
				state=od->dwData;
				if(state&0x80)
				{
					bResult=TRUE;
				}
			}
			break;
		default:
			return FALSE;
		}

		if(bResult)
		{
			if(!iTime1)
			{
				iTime1=od->dwTimeStamp;
			}
			else
			{
				iTime2=od->dwTimeStamp;
			}
		}

		od++;
		nb--;
	}

	if(!iTime2)
	{
		*_iDeltaTime=0;
	}
	else
	{
		*_iDeltaTime=iTime2-iTime1;
	}

	return (iTime1)?TRUE:FALSE;
}
/*-------------------------------------------------------------*/
BOOL DXI_MouseButtonUnPressed(int id,int numb)
{
DIDEVICEOBJECTDATA	*od;
int					state,nb;

	nb=DI_MouseState[id]->nbele;
	if(!nb) return FALSE;
	od=DI_MouseState[id]->mousestate;
	state=0x80;
	while(nb)
	{
//		state=0x80;
		switch(numb)
		{
		case DXI_BUTTON0:
			if(od->dwOfs==DIMOFS_BUTTON0) state=od->dwData;
			break;
		case DXI_BUTTON1:
			if(od->dwOfs==DIMOFS_BUTTON1) state=od->dwData;
			break;
		case DXI_BUTTON2:
			if(od->dwOfs==DIMOFS_BUTTON2) state=od->dwData;
			break;
		case DXI_BUTTON3:
			if(od->dwOfs==DIMOFS_BUTTON3) state=od->dwData;
			break;
		case DXI_BUTTON4:
			if(od->dwOfs==DIMOFS_BUTTON4) state=od->dwData;
			break;
		case DXI_BUTTON5:
			if(od->dwOfs==DIMOFS_BUTTON5) state=od->dwData;
			break;
		case DXI_BUTTON6:
			if(od->dwOfs==DIMOFS_BUTTON6) state=od->dwData;
			break;
		case DXI_BUTTON7:
			if(od->dwOfs==DIMOFS_BUTTON7) state=od->dwData;
			break;
		default:
			return FALSE;
		}
//		if(!(state&0x80)) return TRUE;
		od++;
		nb--;
	}
	if(!(state&0x80)) return TRUE;
	return FALSE;
}

BOOL DXI_OldMouseButtonPressed(int id,int numb)
{
DIDEVICEOBJECTDATA	*od;
int					state,nb;

return FALSE;
	nb=DI_MouseState[id]->nbele;
	if(!nb) return FALSE;
//	od=DI_MouseState[id]->old_mousestate;
	while(nb)
	{
		state=0;
		switch(numb)
		{
		case DXI_BUTTON0:
			if(od->dwOfs==DIMOFS_BUTTON0) state=od->dwData;
			break;
		case DXI_BUTTON1:
			if(od->dwOfs==DIMOFS_BUTTON1) state=od->dwData;
			break;
		case DXI_BUTTON2:
			if(od->dwOfs==DIMOFS_BUTTON2) state=od->dwData;
			break;
		case DXI_BUTTON3:
			if(od->dwOfs==DIMOFS_BUTTON3) state=od->dwData;
			break;
		case DXI_BUTTON4:
			if(od->dwOfs==DIMOFS_BUTTON4) state=od->dwData;
			break;
		case DXI_BUTTON5:
			if(od->dwOfs==DIMOFS_BUTTON5) state=od->dwData;
			break;
		case DXI_BUTTON6:
			if(od->dwOfs==DIMOFS_BUTTON6) state=od->dwData;
			break;
		case DXI_BUTTON7:
			if(od->dwOfs==DIMOFS_BUTTON7) state=od->dwData;
			break;
		default:
			return FALSE;
		}
		if(state&0x80) return TRUE;
		od++;
		nb--;
	}
	return FALSE;
}

int DXI_GetSCIDAxis(int id,int *jx,int *jy,int *jz)
{
INPUT_INFO	*io;
int			dir;

	dir=DXI_JOYNONE;

	io=DI_SCIDState[id];
	if(io->datasid==DFDIJOYSTICK2)
	{
		{
			DIJOYSTATE2	*js;
			js=io->joystate2;
			*jx=js->lX;
			*jy=js->lY;
			*jz=js->lRz;//>lZ;
			if(js->lX>0)
			{
				dir|=DXI_JOYRIGHT;
			}
			else
			{
				if(js->lX<0)
				{
					dir|=DXI_JOYLEFT;
				}
			}

			if(js->lY>0)
			{
				dir|=DXI_JOYDOWN;
			}
			else
			{
				if(js->lY<0)
				{
					dir|=DXI_JOYUP;
				}
			}
		}
	}
	else
	{
		{
			DIJOYSTATE	*js;
			js=io->joystate;
			*jx=js->lX;
			*jy=js->lY;
			*jz=js->lZ;
			if(js->lX>0)
			{
				dir|=DXI_JOYRIGHT;
			}
			else
			{
				if(js->lX<0)
				{
					dir|=DXI_JOYLEFT;
				}
			}

			if(js->lY>0)
			{
				dir|=DXI_JOYDOWN;
			}
			else
			{
				if(js->lY<0)
				{
					dir|=DXI_JOYUP;
				}
			}
		}
	}

	return dir;
}
BOOL DXI_IsSCIDButtonPressed(int id,int numb)
{
INPUT_INFO	*io;

	io=DI_SCIDState[id];
	if(io->datasid==DFDIJOYSTICK2)
	{
		{
			DIJOYSTATE2	*js;
			js=io->joystate2;
			if(js->rgbButtons[numb]&0x80) return TRUE;
		}
	}
	else
	{
		{
			DIJOYSTATE	*js;
			js=io->joystate;
			if(js->rgbButtons[numb]&0x80) return TRUE;
		}
	}
	return FALSE;
}

int DXI_GetSCIDButtonPressed(int id)
{
	INPUT_INFO	*io;
	int			nb;

	io=DI_SCIDState[id];
	if(io->datasid==DFDIJOYSTICK2)
	{
		{
			DIJOYSTATE2	*js;
			js=io->joystate2;
			nb=128;
			while(nb--)
			{
				if (nb==9) 
					nb=9;
				if(js->rgbButtons[nb]&0x80) return nb;
			}
		}
	}
	else
	{
		{
			DIJOYSTATE	*js;
			js=io->joystate;
			nb=32;
			while(nb--)
			{
				if(js->rgbButtons[nb]&0x80) return nb;
			}
		}
	}
	return -1;
}
/*
DIDEVICEOBJECTDATA	*od;
int					nb;

	nb=DI_SCIDState[id]->nbele;
	if(!nb) return -1;
	od=DI_SCIDState[id]->SCIDstate;
	while(nb)
	{
		switch(od->dwOfs)
		{
		case DIMOFS_BUTTON0:
			if(od->dwData&0x80) return DXI_BUTTON0;
			break;
		case DIMOFS_BUTTON1:
			if(od->dwData&0x80) return DXI_BUTTON1;
			break;
		case DIMOFS_BUTTON2:
			if(od->dwData&0x80) return DXI_BUTTON2;
			break;
		case DIMOFS_BUTTON3:
			if(od->dwData&0x80) return DXI_BUTTON3;
			break;
		case DIMOFS_BUTTON4:
			if(od->dwData&0x80) return DXI_BUTTON4;
			break;
		case DIMOFS_BUTTON5:
			if(od->dwData&0x80) return DXI_BUTTON5;
			break;
		case DIMOFS_BUTTON6:
			if(od->dwData&0x80) return DXI_BUTTON6;
			break;
		case DIMOFS_BUTTON7:
			if(od->dwData&0x80) return DXI_BUTTON7;
			break;
		case DIMOFS_BUTTON8:
			if(od->dwData&0x80) return DXI_BUTTON8;
			break;
		case DIMOFS_BUTTON9:
			if(od->dwData&0x80) return DXI_BUTTON9;
			break;
		case DIMOFS_BUTTON10:
			if(od->dwData&0x80) return DXI_BUTTON10;
			break;
		case DIMOFS_BUTTON11:
			if(od->dwData&0x80) return DXI_BUTTON11;
			break;
		case DIMOFS_BUTTON12:
			if(od->dwData&0x80) return DXI_BUTTON12;
			break;
		case DIMOFS_BUTTON13:
			if(od->dwData&0x80) return DXI_BUTTON13;
			break;
		case DIMOFS_BUTTON14:
			if(od->dwData&0x80) return DXI_BUTTON14;
			break;
		case DIMOFS_BUTTON15:
			if(od->dwData&0x80) return DXI_BUTTON15;
			break; 
		default:
			break;
		}
		od++;
		nb--;
	}

	return -1;
}
*/
/*-------------------------------------------------------------*/
int DXI_GetIDButtonPressed(int id)
{
DIDEVICEOBJECTDATA	*od;
int					nb;

	nb=DI_MouseState[id]->nbele;
	if(!nb) return -1;
	od=DI_MouseState[id]->mousestate;
	while(nb)
	{
		switch(od->dwOfs)
		{
		case DIMOFS_BUTTON0:
			if(od->dwData&0x80) return DXI_BUTTON0;
			break;
		case DIMOFS_BUTTON1:
			if(od->dwData&0x80) return DXI_BUTTON1;
			break;
		case DIMOFS_BUTTON2:
			if(od->dwData&0x80) return DXI_BUTTON2;
			break;
		case DIMOFS_BUTTON3:
			if(od->dwData&0x80) return DXI_BUTTON3;
			break;
		case DIMOFS_BUTTON4:
			if(od->dwData&0x80) return DXI_BUTTON4;
			break;
		case DIMOFS_BUTTON5:
			if(od->dwData&0x80) return DXI_BUTTON5;
			break;
		case DIMOFS_BUTTON6:
			if(od->dwData&0x80) return DXI_BUTTON6;
			break;
		case DIMOFS_BUTTON7:
			if(od->dwData&0x80) return DXI_BUTTON7;
			break;
		default:
			break;
		}
		od++;
		nb--;
	}

	return -1;
}
/*-------------------------------------------------------------*/
int DXI_SetMouseRelative(int id)
{
INPUT_INFO		*info;
DIPROPDWORD		dipdw={
				{
					sizeof(DIPROPDWORD),        // diph.dwSize
					sizeof(DIPROPHEADER),       // diph.dwHeaderSize
					0,			                // diph.dwObj
					DIPH_DEVICE,	            // diph.dwHow
				},
				DIPROPAXISMODE_REL,				// dwData
				};

	info=DXI_GetInputInfoWithState((void*)DI_MouseState[id],DIDEVTYPE_MOUSE);
	//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->Unacquire(info->inputdevice7))) return DXI_FAIL;
	if(FAILED(DI_Hr=info->inputdevice7->Unacquire())) return DXI_FAIL;
	//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->SetProperty(info->inputdevice7,DIPROP_AXISMODE,&dipdw.diph))) return DXI_FAIL;
	if(FAILED(DI_Hr=info->inputdevice7->SetProperty(DIPROP_AXISMODE,&dipdw.diph))) return DXI_FAIL;
	//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->Acquire(info->inputdevice7))) return DXI_FAIL;
	if(FAILED(DI_Hr=info->inputdevice7->Acquire())) return DXI_FAIL;
	return DXI_OK;
}
/*-------------------------------------------------------------*/
int DXI_SetMouseAbsolue(int id)
{
INPUT_INFO		*info;
DIPROPDWORD		dipdw={
				{
					sizeof(DIPROPDWORD),        // diph.dwSize
					sizeof(DIPROPHEADER),       // diph.dwHeaderSize
					0,			                // diph.dwObj
					DIPH_DEVICE,	            // diph.dwHow
				},
				DIPROPAXISMODE_ABS,				// dwData
				};

	info=DXI_GetInputInfoWithState((void*)DI_MouseState[id],DIDEVTYPE_MOUSE);
	//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->Unacquire(info->inputdevice7))) return DXI_FAIL;
	if(FAILED(DI_Hr=info->inputdevice7->Unacquire())) return DXI_FAIL;
	//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->SetProperty(info->inputdevice7,DIPROP_AXISMODE,&dipdw.diph))) return DXI_FAIL;
	if(FAILED(DI_Hr=info->inputdevice7->SetProperty(DIPROP_AXISMODE,&dipdw.diph))) return DXI_FAIL;
	//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->Acquire(info->inputdevice7))) return DXI_FAIL;
	if(FAILED(DI_Hr=info->inputdevice7->Acquire())) return DXI_FAIL;
	return DXI_OK;
}
/*-------------------------------------------------------------*/
int DXI_GetAxeJoyXY(int id,int *jx,int *jy)
{
INPUT_INFO	*io;
int			dir;

	dir=DXI_JOYNONE;

	io=DI_JoyState[id];
	if(io->datasid==DFDIJOYSTICK2)
	{
		{
			DIJOYSTATE2	*js;
			js=io->joystate2;
			*jx=js->lX;
			*jy=js->lY;
			if(js->lX>0)
			{
				dir|=DXI_JOYRIGHT;
			}
			else
			{
				if(js->lX<0)
				{
					dir|=DXI_JOYLEFT;
				}
			}

			if(js->lY>0)
			{
				dir|=DXI_JOYDOWN;
			}
			else
			{
				if(js->lY<0)
				{
					dir|=DXI_JOYUP;
				}
			}
		}
	}
	else
	{
		{
			DIJOYSTATE	*js;
			js=io->joystate;
			*jx=js->lX;
			*jy=js->lY;
			if(js->lX>0)
			{
				dir|=DXI_JOYRIGHT;
			}
			else
			{
				if(js->lX<0)
				{
					dir|=DXI_JOYLEFT;
				}
			}

			if(js->lY>0)
			{
				dir|=DXI_JOYDOWN;
			}
			else
			{
				if(js->lY<0)
				{
					dir|=DXI_JOYUP;
				}
			}
		}
	}

	return dir;
}

/*-------------------------------------------------------------*/
int DXI_GetAxeJoyXYZ(int id,int *jx,int *jy,int *jz)
{
INPUT_INFO	*io;
int			dir;

	dir=DXI_JOYNONE;

	io=DI_JoyState[id];
	if(io->datasid==DFDIJOYSTICK2)
	{
		{
			DIJOYSTATE2	*js;
			js=io->joystate2;
			*jx=js->lX;
			*jy=js->lY;
			*jz=js->lZ;
			if(js->lX>0)
			{
				dir|=DXI_JOYRIGHT;
			}
			else
			{
				if(js->lX<0)
				{
					dir|=DXI_JOYLEFT;
				}
			}

			if(js->lY>0)
			{
				dir|=DXI_JOYDOWN;
			}
			else
			{
				if(js->lY<0)
				{
					dir|=DXI_JOYUP;
				}
			}
		}
	}
	else
	{
		{
			DIJOYSTATE	*js;
			js=io->joystate;
			*jx=js->lX;
			*jy=js->lY;
			*jz=js->lZ;
			if(js->lX>0)
			{
				dir|=DXI_JOYRIGHT;
			}
			else
			{
				if(js->lX<0)
				{
					dir|=DXI_JOYLEFT;
				}
			}

			if(js->lY>0)
			{
				dir|=DXI_JOYDOWN;
			}
			else
			{
				if(js->lY<0)
				{
					dir|=DXI_JOYUP;
				}
			}
		}
	}

	return dir;
}
/*-------------------------------------------------------------*/
int DXI_GetAxeJoyXYZW(int id,int *jx,int *jy,int *jz,int * jw)
{
INPUT_INFO	*io;
//int			dir;

//	dir=DXI_JOYNONE;

	io=DI_JoyState[id];
	if(io->datasid==DFDIJOYSTICK2)
	{
		DIJOYSTATE2	*js;
		js=io->joystate2;
		*jx=js->lX;
		*jy=js->lY;
		*jz=js->lRz;
		*jw=js->rglSlider[0];			
	}
	else
	{
		DIJOYSTATE	*js;
		js=io->joystate;
		*jx=js->lX;
		*jy=js->lY;
		*jz=js->lZ;		
		*jw=0;//js->rglSlider;			
	}

	return 1;
}

/*-------------------------------------------------------------*/
int DXI_SetJoyRelative(int id)
{
INPUT_INFO		*info;
DIPROPDWORD		dipdw={
				{
					sizeof(DIPROPDWORD),        // diph.dwSize
					sizeof(DIPROPHEADER),       // diph.dwHeaderSize
					0,			                // diph.dwObj
					DIPH_DEVICE,	            // diph.dwHow
				},
				DIPROPAXISMODE_REL,				// dwData
				};

	info=DXI_GetInputInfoWithState((void*)DI_JoyState[id],DIDEVTYPE_JOYSTICK);
	//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->Unacquire(info->inputdevice7))) return DXI_FAIL;
	if(FAILED(DI_Hr=info->inputdevice7->Unacquire())) return DXI_FAIL;
	//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->SetProperty(info->inputdevice7,DIPROP_AXISMODE,&dipdw.diph))) return DXI_FAIL;
	if(FAILED(DI_Hr=info->inputdevice7->SetProperty(DIPROP_AXISMODE,&dipdw.diph))) return DXI_FAIL;
	//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->Acquire(info->inputdevice7))) return DXI_FAIL;
	if(FAILED(DI_Hr=info->inputdevice7->Acquire())) return DXI_FAIL;
	return DXI_OK;
}
/*-------------------------------------------------------------*/
int DXI_SetJoyAbsolue(int id)
{
INPUT_INFO		*info;
DIPROPDWORD		dipdw={
				{
					sizeof(DIPROPDWORD),        // diph.dwSize
					sizeof(DIPROPHEADER),       // diph.dwHeaderSize
					0,			                // diph.dwObj
					DIPH_DEVICE,	            // diph.dwHow
				},
				DIPROPAXISMODE_ABS,				// dwData
				};

	info=DXI_GetInputInfoWithState((void*)DI_JoyState[id],DIDEVTYPE_JOYSTICK);
	//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->Unacquire(info->inputdevice7))) return DXI_FAIL;
	if(FAILED(DI_Hr=info->inputdevice7->Unacquire())) return DXI_FAIL;
	//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->SetProperty(info->inputdevice7,DIPROP_AXISMODE,&dipdw.diph))) return DXI_FAIL;
	if(FAILED(DI_Hr=info->inputdevice7->SetProperty(DIPROP_AXISMODE,&dipdw.diph))) return DXI_FAIL;
	//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->Acquire(info->inputdevice7))) return DXI_FAIL;
	if(FAILED(DI_Hr=info->inputdevice7->Acquire())) return DXI_FAIL;
	return DXI_OK;
}
/*-------------------------------------------------------------*/
int DXI_SetRangeJoy(int id,int axe,int range)
{
INPUT_INFO		*info;
DIPROPRANGE		diprg; 
DIPROPDWORD		dipdw={
				{
					sizeof(DIPROPDWORD),        // diph.dwSize
					sizeof(DIPROPHEADER),       // diph.dwHeaderSize
					0,			                // diph.dwObj
					DIPH_BYOFFSET,	            // diph.dwHow
				},
				0,								// dwData
				};

	if(!range) return DXI_FAIL;
	
	info=DXI_GetInputInfoWithState((void*)DI_JoyState[id],DIDEVTYPE_JOYSTICK);

	diprg.diph.dwSize=sizeof(diprg); 
	diprg.diph.dwHeaderSize=sizeof(diprg.diph); 
	switch(axe)
	{
	case DXI_XAxis:
		diprg.diph.dwObj=DIJOFS_X; 
		dipdw.diph.dwObj=DIJOFS_X;
		break;
	case DXI_YAxis:
		diprg.diph.dwObj=DIJOFS_Y; 
		dipdw.diph.dwObj=DIJOFS_Y;
		break;
	case DXI_ZAxis:
		diprg.diph.dwObj=DIJOFS_Z; 
		dipdw.diph.dwObj=DIJOFS_Z;
		break;
	case DXI_RzAxis:
		diprg.diph.dwObj=DIJOFS_RZ; 
		dipdw.diph.dwObj=DIJOFS_RZ;
		break;
	case DXI_Slider:
		diprg.diph.dwObj=DIJOFS_SLIDER(0); 
		dipdw.diph.dwObj=DIJOFS_SLIDER(0);
		break;
	default:
		return DXI_FAIL;
	}
	diprg.diph.dwHow=DIPH_BYOFFSET; 
	diprg.lMin=-range; 
	diprg.lMax=range; 
	dipdw.dwData=5000;
	//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->SetProperty(info->inputdevice7,DIPROP_RANGE,&diprg.diph))) return DXI_FAIL;
	if(FAILED(DI_Hr=info->inputdevice7->SetProperty(DIPROP_RANGE,&diprg.diph))) return DXI_FAIL;
	//Old : if(FAILED(DI_Hr=info->inputdevice7->lpVtbl->SetProperty(info->inputdevice7,DIPROP_DEADZONE,&dipdw.diph))) return DXI_FAIL;
	if(FAILED(DI_Hr=info->inputdevice7->SetProperty(DIPROP_DEADZONE,&dipdw.diph))) return DXI_FAIL;

	return DXI_OK;
}
/*-------------------------------------------------------------*/
BOOL DXI_GetJoyButtonPressed(int id,int numb)
{
INPUT_INFO	*io;

	io=DI_JoyState[id];
	if(io->datasid==DFDIJOYSTICK2)
	{
		{
			DIJOYSTATE2	*js;
			js=io->joystate2;
			if(js->rgbButtons[numb]&0x80) return TRUE;
		}
	}
	else
	{
		{
			DIJOYSTATE	*js;
			js=io->joystate;
			if(js->rgbButtons[numb]&0x80) return TRUE;
		}
	}
	return FALSE;
}
BOOL DXI_OldGetJoyButtonPressed(int id,int numb)
{
INPUT_INFO	*io;

return FALSE;
	io=DI_JoyState[id];
	if(io->datasid==DFDIJOYSTICK2)
	{
		{
			DIJOYSTATE2	*js;
//			js=io->old_joystate2;
			if(js->rgbButtons[numb]&0x80) return TRUE;
		}
	}
	else
	{
		{
			DIJOYSTATE	*js;
//			js=io->old_joystate;
			if(js->rgbButtons[numb]&0x80) return TRUE;
		}
	}
	return FALSE;
}
/*-------------------------------------------------------------*/
int DXI_GetIDJoyButtonPressed(int id)
{
INPUT_INFO	*io;
int			nb;

	io=DI_JoyState[id];
	if(io->datasid==DFDIJOYSTICK2)
	{
		{
			DIJOYSTATE2	*js;
			js=io->joystate2;
			nb=128;
			while(nb--)
			{
				if(js->rgbButtons[nb]&0x80) return nb;
			}
		}
	}
	else
	{
		{
			DIJOYSTATE	*js;
			js=io->joystate;
			nb=32;
			while(nb--)
			{
				if(js->rgbButtons[nb]&0x80) return nb;
			}
		}
	}
	return -1;
}
/*-------------------------------------------------------------*/

