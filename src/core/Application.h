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

#ifndef  ARX_CORE_APPLICATION_H
#define  ARX_CORE_APPLICATION_H

#include <string>

#include <windows.h>
#include <d3d.h>

#include "graphics/Renderer.h"

#include "platform/Flags.h"

#include "Configure.h"

struct D3DEnum_DeviceInfo;
class CD3DFramework7;

enum HideFlag {
	HIDE_BACKGROUND = (1<<0),
	HIDE_NPC        = (1<<1),
	HIDE_FIXINTER   = (1<<2),
	HIDE_ITEMS      = (1<<3),
	HIDE_PARTICLES  = (1<<4),
	HIDE_INTERFACE  = (1<<5),
	HIDE_NODES      = (1<<6),
	HIDE_CAMERAS    = (1<<7)
};
DECLARE_FLAGS(HideFlag, HideFlags);
DECLARE_FLAGS_OPERATORS(HideFlags);

enum ViewModeFlag {
	VIEWMODE_WIRE           = (1<<0),
	VIEWMODE_NORMALS        = (1<<1),
	VIEWMODE_FLAT           = (1<<2),
	VIEWMODE_NOLIGHTSOURCES = (1<<3),
	VIEWMODE_INFOTEXT       = (1<<4)
};
DECLARE_FLAGS(ViewModeFlag, ViewModeFlags);
DECLARE_FLAGS_OPERATORS(ViewModeFlags);

enum LightModeFlag {
	MODE_STATICLIGHT  = (1<<0),
	MODE_DEPTHCUEING  = (1<<1),
	MODE_DYNAMICLIGHT = (1<<2),
	MODE_NORMALS      = (1<<3),
	MODE_RAYLAUNCH    = (1<<4),
	MODE_SMOOTH       = (1<<5)
};
DECLARE_FLAGS(LightModeFlag, LightMode);
DECLARE_FLAGS_OPERATORS(LightMode);

enum WindowCreationFlag {
	WCF_NORESIZE     = (1<<0),
	WCF_NOFRAMEINFOS = (1<<1),
	WCF_NOSTDPOPUP   = (1<<2),
	WCF_CHILD        = (1<<3),
	WCF_ACCEPTFILES  = (1<<4)
};
DECLARE_FLAGS(WindowCreationFlag, WindowCreationFlags);
DECLARE_FLAGS_OPERATORS(WindowCreationFlags);

enum InputKey {
	
	INKEY_ESCAPE = 1,
	INKEY_ARROW_UP = 172,
	INKEY_ARROW_LEFT = 175,
	INKEY_ARROW_RIGHT = 177,
	INKEY_ARROW_DOWN = 180,
	
	INKEY_PAD0 = 82,
	INKEY_PAD1 = 79,
	INKEY_PAD2 = 80,
	INKEY_PAD3 = 81,
	INKEY_PAD4 = 75,
	INKEY_PAD5 = 76,
	INKEY_PAD6 = 77,
	INKEY_PAD7 = 71,
	INKEY_PAD8 = 72,
	INKEY_PAD9 = 73,
	INKEY_PADMINUS = 74,
	INKEY_PADADD = 78,
	INKEY_PADMULTIPLY = 55,
	INKEY_PADDIVIDE = 153,
	INKEY_PADENTER = 128,
	INKEY_PADNUMLOCK = 169,
	
	INKEY_F1 = 59,
	INKEY_F2 = 60,
	INKEY_F3 = 61,
	INKEY_F4 = 62,
	INKEY_F5 = 63,
	INKEY_F6 = 64,
	INKEY_F7 = 65,
	INKEY_F8 = 66,
	INKEY_F9 = 67,
	INKEY_F10 = 68,
	
	INKEY_RETURN = 28,
	INKEY_SPACE = 57,
	INKEY_LEFTCTRL = 29,
	INKEY_RIGHTCTRL = 129,
	INKEY_LEFTSHIFT = 42,
	INKEY_RIGHTSHIFT = 54,
	INKEY_DEL = 183,
	INKEY_INSERT = 182,
	INKEY_PAGEUP = 173,
	INKEY_PAGEDOWN = 181,
	INKEY_HOME = 171,
	INKEY_END = 179,
	INKEY_BACKSPACE = 14,
	
	INKEY_CAPSLOCK = 58,
	INKEY_TAB = 15,
	INKEY_EXPONENT = 41,
	INKEY_1 = 2,
	INKEY_2 = 3,
	INKEY_3 = 4,
	INKEY_4 = 5,
	INKEY_5 = 6,
	INKEY_6 = 7,
	INKEY_7 = 8,
	INKEY_8 = 9,
	INKEY_9 = 10,
	INKEY_0 = 11,
	INKEY_CLOSEBRACKET = 12,
	INKEY_ADD = 13,
	
	INKEY_A = 16,
	INKEY_Z = 17,
	INKEY_E = 18,
	INKEY_R = 19,
	INKEY_T = 20,
	INKEY_Y = 21,
	INKEY_U = 22,
	INKEY_I = 23,
	INKEY_O = 24,
	INKEY_P = 25,
	INKEY_Q = 30,
	INKEY_S = 31,
	INKEY_D = 32,
	INKEY_F = 33,
	INKEY_G = 34,
	INKEY_H = 35,
	INKEY_J = 36,
	INKEY_K = 37,
	INKEY_L = 38,
	INKEY_M = 39,
	INKEY_W = 44,
	INKEY_X = 45,
	INKEY_C = 46,
	INKEY_V = 47,
	INKEY_B = 48,
	INKEY_N = 49
	
};

struct PROJECT {
	
	PROJECT()
		:
		  compatibility(0), ambient(0),
		  improve(0), detectliving(0), improvespeed(0),
		  telekinesis(0), multiplayer(0), demo(0),
		  bits(0), hide(0), TextureSize(0), TextureBits(0),
		  interfacergb(Color3f::black), torch(Color3f::black),
		  interpolatemouse(0), vsync(0) {
	}
	
	long compatibility;
	long ambient;
	long improve;
	long detectliving;
	long improvespeed;
	long telekinesis;
	long multiplayer;
	long demo;
	long bits;
	HideFlags hide;
	long TextureSize;
	long TextureBits;
	Color3f interfacergb;
	Color3f torch;
	long interpolatemouse;
	long vsync;
	
};

#ifdef BUILD_EDITOR

#include <commctrl.h>

enum ToolbarPosition {
	EERIE_TOOLBAR_TOP,
	EERIE_TOOLBAR_LEFT
};

struct EERIETOOLBAR {
	HWND hWnd;
	long CreationToolBar;
	long ToolBarNb;
	LPCTBBUTTON Buttons;
	long		Bitmap;
	std::string		String;
	long Type;
};

#endif

struct KEYBOARD_MNG {
	short nbkeydown;
	unsigned char inkey[255];
	char _CAPS;
	short lastkey;
};

extern PROJECT Project;
extern float FPS;
extern LPDIRECT3DDEVICE7 GDevice;
extern LightMode ModeLight;

extern ViewModeFlags ViewMode;

extern long EERIEMouseButton, EERIEMouseGrab;
extern HWND MSGhwnd;

class CD3DApplication {
	
	// Internal variables and member functions
	bool m_bSingleStep;
	DWORD m_dwBaseTime;
	DWORD m_dwStopTime;
	
	HRESULT Render3DEnvironment();
	void DisplayFrameworkError(HRESULT, DWORD);
	
public:
	
	float fMouseSensibility;
	
protected:
	
	// Overridable variables for the app
		std::string m_strWindowTitle;
	bool m_bAppUseZBuffer;
	bool m_bAppUseStereo;
	bool m_bShowStats;
	HRESULT(*m_fnConfirmDevice)(DDCAPS *, D3DDEVICEDESC7 *);
	
#ifdef BUILD_EDITOR
	HWND CreateToolBar(HWND hWndParent, HINSTANCE hInst);
#endif
	
	// Overridable functions for the 3D scene created by the app
	virtual HRESULT OneTimeSceneInit() {
		return S_OK;
	}
	virtual HRESULT DeleteDeviceObjects() {
		return S_OK;
	}
	virtual HRESULT FrameMove() {
		return S_OK;
	}
	virtual HRESULT RestoreSurfaces() {
		return S_OK;
	}
	virtual HRESULT FinalCleanup() {
		return S_OK;
	}
	
	// Overridable power management (APM) functions
	virtual LRESULT OnQuerySuspend(DWORD dwFlags);
	virtual LRESULT OnResumeSuspend(DWORD dwData);
	virtual HRESULT BeforeRun() {
		return S_OK;
	}
	
	//zbuffer
	short w_zdecal;
	long dw_zmask;
	float f_zmul;
	long dw_zXmodulo;
	
public:
	
	LPDIRECTDRAW7 m_pDD;
	LPDIRECTDRAWSURFACE7 m_pddsRenderTarget;
	LPDIRECTDRAWSURFACE7 m_pddsRenderTargetLeft;	// For stereo modes
	DDSURFACEDESC2 m_ddsdRenderTarget;
	int WinManageMess();
	void Cleanup3DEnvironment();
	LPDIRECT3D7 m_pD3D;
	void EvictManagedTextures();
	virtual HRESULT Render() = 0;
	virtual HRESULT InitDeviceObjects() {
		return S_OK;
	}
	void OutputText(DWORD x, DWORD y, const std::string& str);
	
	bool m_bActive;
	HRESULT Change3DEnvironment();
	HRESULT Initialize3DEnvironment();
	bool m_bReady;
	D3DEnum_DeviceInfo * m_pDeviceInfo;
	HWND m_hWnd;
	HWND m_hWndRender;
	WNDPROC m_OldProc;
	long d_dlgframe;
	
	// Functions to create, run, pause, and clean up the application
	virtual HRESULT Create(HINSTANCE);
	virtual INT Run();
	virtual LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void Pause(bool bPause);
	LRESULT SwitchFullScreen() ;
	
	CD3DFramework7 * m_pFramework;
	KEYBOARD_MNG kbd;
	
	char StatusText[512];
	short WndSizeX;
	short WndSizeY;
	bool  Fullscreen;
	WindowCreationFlags CreationFlags;
	long CreationMenu;
#ifdef BUILD_EDITOR
	EERIETOOLBAR * ToolBar;
#endif
	HWND owner;
	
	void * logical;
	void * zbuf;
	long zbits;
	long nbbits;
	
	void * Lock();
	bool Unlock();
	DDSURFACEDESC2 ddsd;
	DDSURFACEDESC2 ddsd2;
	
	LPDIRECTDRAWGAMMACONTROL lpDDGammaControl; // gamma control
	DDGAMMARAMP DDGammaRamp; // modified ramp value
	DDGAMMARAMP DDGammaOld; // backup gamma values
	HRESULT UpdateGamma();
	
	float GetZBufferMax();
	float zbuffer_max;
	float zbuffer_max_div;
	int menu;
	
	// Class constructor
	CD3DApplication();
	
};

extern CD3DApplication * g_pD3DApp;

/**
 * RAII for Lock() Unlock() on CD3DApplication class
 * Can be used in conditions to test whether the lock was successfully acquired
 **/
class CD3DApplicationScopedLock {
public:
	explicit CD3DApplicationScopedLock(CD3DApplication& app) : instance_(&app) { if (!instance_->Lock()) instance_ = NULL; }
	~CD3DApplicationScopedLock() { if (instance_) instance_->Unlock(); }
	operator void*() const { return instance_; }
private:
	CD3DApplication* instance_;
	// Prevent instances from being copied.
	CD3DApplicationScopedLock(const CD3DApplicationScopedLock&);
	CD3DApplicationScopedLock& operator=(const CD3DApplicationScopedLock&);
};

bool OKBox(const std::string& text, const std::string& title);

void CalcFPS(bool reset = false);

void SetZBias(int);

#endif // ARX_CORE_APPLICATION_H
