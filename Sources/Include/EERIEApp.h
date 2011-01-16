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
// EERIEApp
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//
// Updates: (date) (person) (update)
//
// Code:	Cyril Meynier
//			Sébastien Scieux	(Zbuffer)
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#ifndef  EERIEAPP_H
#define  EERIEAPP_H

#define  D3D_OVERLOADS

#include <windows.h>
#include <commctrl.h>
#include <d3d.h>
#include "EERIEFrame.h"
#include "EERIEEnum.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

//-----------------------------------------------------------------------------
#define HIDE_BACKGROUND 1
#define HIDE_NPC		2
#define HIDE_FIXINTER	4
#define HIDE_ITEMS		8
#define HIDE_PARTICLES	16
#define HIDE_INTERFACE	32
#define HIDE_NODES		64
#define HIDE_CAMERAS	128

#define VIEWMODE_WIRE			1
#define VIEWMODE_NORMALS		2
#define VIEWMODE_FLAT			4
#define VIEWMODE_NOLIGHTSOURCES 8
#define VIEWMODE_INFOTEXT		16

#define MODE_STATICLIGHT	1
#define MODE_DEPTHCUEING	2
#define MODE_DYNAMICLIGHT	4
#define MODE_NORMALS		8
#define MODE_RAYLAUNCH		16
#define MODE_SMOOTH			32

#define WCF_NORESIZE		1
#define WCF_NOFRAMEINFOS	(1<<1)
#define WCF_NOSTDPOPUP		(1<<2)
#define	WCF_CHILD			(1<<3)
#define WCF_ACCEPTFILES		(1<<4)

#define EERIE_TOOLBAR_TOP	0
#define EERIE_TOOLBAR_LEFT	1

#define	INKEY_ESCAPE		1
#define INKEY_ARROW_UP		172
#define INKEY_ARROW_LEFT	175
#define INKEY_ARROW_RIGHT	177
#define INKEY_ARROW_DOWN	180

#define INKEY_PAD0			82
#define INKEY_PAD1			79
#define INKEY_PAD2			80
#define INKEY_PAD3			81
#define INKEY_PAD4			75
#define INKEY_PAD5			76
#define INKEY_PAD6			77
#define INKEY_PAD7			71
#define INKEY_PAD8			72
#define INKEY_PAD9			73
#define INKEY_PADMINUS		74
#define INKEY_PADADD		78
#define INKEY_PADMULTIPLY	55
#define INKEY_PADDIVIDE		153
#define INKEY_PADENTER		128
#define INKEY_PADNUMLOCK	169

#define INKEY_F1			59
#define INKEY_F2			60
#define INKEY_F3			61
#define INKEY_F4			62
#define INKEY_F5			63
#define INKEY_F6			64
#define INKEY_F7			65
#define INKEY_F8			66
#define INKEY_F9			67
#define INKEY_F10			68

#define INKEY_RETURN		28
#define INKEY_SPACE			57
#define INKEY_LEFTCTRL		29
#define INKEY_RIGHTCTRL		129
#define INKEY_LEFTSHIFT		42
#define INKEY_RIGHTSHIFT	54
#define INKEY_DEL			183
#define INKEY_INSERT		182
#define INKEY_PAGEUP		173
#define INKEY_PAGEDOWN		181
#define INKEY_HOME			171
#define INKEY_END			179
#define INKEY_BACKSPACE		14

#define INKEY_CAPSLOCK		58
#define INKEY_TAB			15
#define INKEY_EXPONENT		41
#define INKEY_1				2
#define INKEY_2				3
#define INKEY_3				4
#define INKEY_4				5
#define INKEY_5				6
#define INKEY_6				7
#define INKEY_7				8
#define INKEY_8				9
#define INKEY_9				10
#define INKEY_0				11
#define INKEY_CLOSEBRACKET	12
#define INKEY_ADD			13

#define INKEY_A				16
#define INKEY_Z				17
#define INKEY_E				18
#define INKEY_R				19
#define INKEY_T				20
#define INKEY_Y				21
#define INKEY_U				22
#define INKEY_I				23
#define INKEY_O				24
#define INKEY_P				25
#define INKEY_Q				30
#define INKEY_S				31
#define INKEY_D				32
#define INKEY_F				33
#define INKEY_G				34
#define INKEY_H				35
#define INKEY_J				36
#define INKEY_K				37
#define INKEY_L				38
#define INKEY_M				39
#define INKEY_W				44
#define INKEY_X				45
#define INKEY_C				46
#define INKEY_V				47
#define INKEY_B				48
#define INKEY_N				49

//-----------------------------------------------------------------------------
typedef struct
{
	float r;
	float g;
	float b;
} EERIE_RGBB;

typedef struct
{
	long		soundmode;
	long		compatibility;
	long		ambient;
	long		improve;
	long		detectliving;
	long		improvespeed;
	long		telekinesis;
	long		multiplayer;
	long		demo;
	long		bits;
	long		hide;
	long		TextureSize;
	long		TextureBits;
	EERIE_RGBB	interfacergb;
	EERIE_RGBB	torch;
	long		interpolatemouse;
	long		vsync;
	char		workingdir[256];
	char		localisationpath[256];
} PROJECT;

typedef struct
{
	HWND		hWnd;
	long		CreationToolBar;
	long		ToolBarNb;
	LPCTBBUTTON Buttons;
	long		Bitmap;
	char*		String;
	long		Type;
} EERIETOOLBAR;

typedef struct
{
	short			nbkeydown;
	unsigned char	inkey[255];
	char			_CAPS;
	short			lastkey;
} KEYBOARD_MNG;


//-----------------------------------------------------------------------------
extern PROJECT			Project;
extern float			FPS;
extern LPDIRECT3DDEVICE7 GDevice;
extern int				ModeLight;

extern short WINDOWCREATIONSIZEX;
extern short WINDOWCREATIONSIZEY;
extern long ViewMode;

extern long EERIEMouseXdep, EERIEMouseYdep, EERIEMouseX, EERIEMouseY, EERIEWheel;
extern long EERIEMouseButton, EERIEMouseGrab;
extern HWND MSGhwnd;

//-----------------------------------------------------------------------------
// Name: class CD3DApplication
// Desc:
//-----------------------------------------------------------------------------
class CD3DApplication
{
		// Internal variables and member functions
		BOOL            m_bSingleStep;
		DWORD           m_dwBaseTime;
		DWORD           m_dwStopTime;

		HRESULT Render3DEnvironment();
		VOID    DisplayFrameworkError(HRESULT, DWORD);

public:
		float			fMouseSensibility;
protected:
		// Overridable variables for the app
		TCHAR*			m_strWindowTitle;
		BOOL			m_bAppUseZBuffer;
		BOOL			m_bAppUseStereo;
		BOOL			m_bShowStats;
		HRESULT(*m_fnConfirmDevice)(DDCAPS *, D3DDEVICEDESC7 *);
		HWND				 CreateToolBar(HWND hWndParent, long tbb, HINSTANCE hInst);

		// Overridable functions for the 3D scene created by the app
		virtual HRESULT OneTimeSceneInit()
		{
			return S_OK;
		}
		virtual HRESULT DeleteDeviceObjects()
		{
			return S_OK;
		}
		virtual HRESULT FrameMove(FLOAT)
		{
			return S_OK;
		}
		virtual HRESULT RestoreSurfaces()
		{
			return S_OK;
		}
		virtual HRESULT FinalCleanup()
		{
			return S_OK;
		}

		// Overridable power management (APM) functions
		virtual LRESULT OnQuerySuspend(DWORD dwFlags);
		virtual LRESULT OnResumeSuspend(DWORD dwData);
		virtual HRESULT BeforeRun()
		{
			return S_OK;
		}

		//zbuffer
		short			w_zdecal;
		long			dw_zmask;
		float			f_zmul;
		long			dw_zXmodulo;

	public:
		LPDIRECTDRAW7			m_pDD;
		LPDIRECT3DDEVICE7		m_pd3dDevice;
		LPDIRECTDRAWSURFACE7	m_pddsRenderTarget;
		LPDIRECTDRAWSURFACE7	m_pddsRenderTargetLeft;	// For stereo modes
		DDSURFACEDESC2			m_ddsdRenderTarget;
		int						WinManageMess();
		VOID					Cleanup3DEnvironment();
		LPDIRECT3D7				m_pD3D;
		void					EvictManagedTextures();
		virtual HRESULT Render()
		{
			return S_OK;
		}
		virtual HRESULT InitDeviceObjects()
		{
			return S_OK;
		}
		VOID					OutputText( DWORD x, DWORD y, TCHAR * str );
 
		HRESULT	SetClipping( float x1, float y1, float x2, float y2 );
 
		BOOL					m_bFrameMoving;
		BOOL					m_bActive;
		HRESULT					Change3DEnvironment();
		HRESULT					Initialize3DEnvironment();
		BOOL					m_bReady;
		D3DEnum_DeviceInfo*		m_pDeviceInfo;
		HWND					m_hWnd;
		HWND					m_hWndRender;
		WNDPROC					m_OldProc;
		HWND					m_dlghWnd;
		BOOL					b_dlg;
		long					d_dlgframe;
		void					EERIEMouseUpdate(short x, short y);

		// Functions to create, run, pause, and clean up the application
		virtual HRESULT			Create(HINSTANCE, TCHAR *);
		virtual INT				Run();
		virtual LRESULT			MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual VOID			Pause(BOOL bPause);
		LRESULT					SwitchFullScreen() ;

		CD3DFramework7*			m_pFramework;
		KEYBOARD_MNG			kbd;

		char					StatusText[512];
		short					CreationSizeX;
		short					CreationSizeY;
		long					CreationFlags;
		long					CreationMenu;
		EERIETOOLBAR*			ToolBar;
		HWND					owner;
 
		void*					logical;
		void*					zbuf;
		long					zbits;
		long					nbbits;
 
		void*					Lock();
		bool					Unlock();
		DDSURFACEDESC2			ddsd;
		DDSURFACEDESC2			ddsd2;

		void					EnableZBuffer();

		LPDIRECTDRAWGAMMACONTROL lpDDGammaControl;	//gamma control
		DDGAMMARAMP				DDGammaRamp;		//modified ramp value
		DDGAMMARAMP				DDGammaOld;			//backup gamma values
		HRESULT					UpdateGamma();
 
		float					GetZBufferMax();
		float					zbuffer_max;
		float					zbuffer_max_div;

		// Class constructor
		CD3DApplication();
};

extern CD3DApplication * g_pD3DApp;

//******************************************************************************
// MESSAGE BOXES
//******************************************************************************
bool	OKBox(char * text, char * title);
void	ShowPopup(char * text);
int		ShowError(char * funcname, char * message, long fatality);

void	ExitApp(int v);
char*	MakeDir(char * tex, char * tex2);
void	CalcFPS(BOOL reset = FALSE);

void	SetZBias(const LPDIRECT3DDEVICE7, int);

#endif // D3DAPP_H
